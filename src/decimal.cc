// ****************************************************************************
//  decimal.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Variable-precision decimal implementation
//
//     This is intended to save some code space when running on DM42,
//     while improving the avaiable precision.
//     The bid128 implementation takes 59.7% of the PGM space and 79.7%
//     of the entire ELF file size. We can probably do better.
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "decimal.h"

#include "arithmetic.h"
#include "bignum.h"
#include "constants.h"
#include "fraction.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "utf8.h"
#include "util.h"

#include <inttypes.h>


RECORDER(decimal, 32, "Variable-precision decimal data type");
RECORDER(decimal_error, 32, "Variable-precision decimal data type");


// ============================================================================
//
//   Object interface
//
// ============================================================================

SIZE_BODY(decimal)
// ----------------------------------------------------------------------------
//   Compute the size of a decimal number
// ----------------------------------------------------------------------------
{
    byte_p p       = o->payload();
    large  exp     = leb128<large>(p); (void) exp;
    size_t nkigits = leb128<size_t>(p);
    p += (nkigits * 10 + 7) / 8;
    return ptrdiff(p, o);
}


HELP_BODY(decimal)
// ----------------------------------------------------------------------------
//   Help topic for decimal numbers
// ----------------------------------------------------------------------------
{
    return utf8("Decimal numbers");
}


static bool normalize(object::id      type,
                      decimal::kint *&rb,
                      size_t         &rs,
                      large          &re)
// ----------------------------------------------------------------------------
//   Normalize a result to have no leading or trailing zero
// ----------------------------------------------------------------------------
{
    // Strip leading zeroes three by three
    while (rs && *rb == 0)
    {
        re -= 3;
        rb++;
        rs--;
    }

    // Strip up to two individual leading zeroes
    if (rs && *rb < 100)
    {
        re -= 1 + (*rb < 10);
        uint hmul = *rb < 10 ? 100 : 10;
        uint lmul = 1000 / hmul;
        for (size_t ko = 0; ko < rs; ko++)
        {
            decimal::kint next = ko + 1 < rs ? rb[ko + 1] : 0;
            rb[ko] = (rb[ko] * hmul + next / lmul) % 1000;
        }
    }

    // Strip trailing zeroes
    while (rs && rb[rs-1] == 0)
        rs--;

    // If result is zero, set exponent to 0
    if (!rs)
        re = 0;

    // Check overflow and underflow
    large maxexp = large(Settings.MaximumDecimalExponent());
    if (re - 1 < -maxexp)
    {
        bool negative = type == object::ID_neg_decimal;
        if (Settings.UnderflowError())
        {
            if (negative)
                rt.negative_underflow_error();
            else
                rt.positive_underflow_error();
            return false;
        }
        if (negative)
            Settings.NegativeUnderflowIndicator(true);
        else
            Settings.PositiveUnderflowIndicator(true);
        re = 0;
        rs = 0;
    }
    else if (re - 1 > maxexp)
    {
        if (Settings.OverflowError())
        {
            rt.overflow_error();
            return false;
        }
        Settings.OverflowIndicator(true);
        re = maxexp + 2;
    }
    return true;
}


bool decimal::is_infinity() const
// ----------------------------------------------------------------------------
//  Check if the value overflowed and represents an infinity
// ----------------------------------------------------------------------------
{
    return exponent() > large(Settings.MaximumDecimalExponent());
}


PARSE_BODY(decimal)
// ----------------------------------------------------------------------------
//    Try to parse this as an decimal
// ----------------------------------------------------------------------------
//    Note that this does not try to parse named constants like "inf" or "NaN"
{
    gcutf8   source = p.source;
    gcutf8   s      = source;
    gcutf8   last   = source + p.length;
    id       type   = ID_decimal;
    scribble scr;

    // Skip leading sign
    if (*s == '+' || *s == '-')
    {
        // In an equation, `1 + 3` should interpret `+` as an infix
        if (p.precedence < 0)
            return SKIP;
        if (*s == '-')
            type = ID_neg_decimal;
        ++s;
    }

    // Scan digits and decimal dot
    kint    kigit      = 0;
    uint    kigc       = 0;
    large   exponent   = 0;
    int     decimalDot = -1;
    size_t  digits     = 0;
    bool    zeroes     = true;
    unicode sep        = Settings.NumberSeparator();
    unicode expsep     = Settings.ExponentSeparator();
    while (+s < +last)
    {
        unicode cp = utf8_codepoint(s);
        if (cp == sep)
        {
            s = utf8_next(+s);
            continue;
        }
        if (cp >= '0' && cp <= '9')
        {
            digits++;
            if (!zeroes || cp != '0')
            {
                if (decimalDot < 0)
                    exponent++;
                kigit = kigit * 10 + (cp - '0');
                if (++kigc == 3)
                {
                    kint *kigp = (kint *) rt.allocate(sizeof(kint));
                    if (!kigp)
                        return ERROR;
                    *kigp = kigit;
                    kigc  = 0;
                    kigit = 0;
                }
                zeroes = false;
            }
            else if (decimalDot >= 0)
            {
                exponent--;
            }
        }
        else if (decimalDot < 0 && (cp == '.' || cp == ','))
        {
            decimalDot = +s - +source;
        }
        else
        {
            break;
        }
        ++s;
    }
    if (!digits)
        return SKIP;

    if (kigc)
    {
        while (kigc++ < 3)
            kigit *= 10;
        kint *kigp = (kint *) rt.allocate(sizeof(kint));
        if (!kigp)
            return ERROR;
        *kigp = kigit;
        kigc = 0;
        kigit = 0;
    }

    // Check how many digits were given
    const size_t maxdigits = Settings.Precision();
    if (Settings.TooManyDigitsErrors() && digits > maxdigits)
    {
        rt.mantissa_error().source(source, digits + (decimalDot >= 0));
        return ERROR;
    }

    // Check if we were given an exponent
    if (+s < +last)
    {
        unicode cp = utf8_codepoint(+s);
        if (cp == 'e' || cp == 'E' || cp == expsep)
        {
            large expval = 0;
            bool  expneg = false;
            s = utf8_next(s);
            unicode sign = utf8_codepoint(s);
            if (sign == '+' || sign == '-' || sign == L'⁻')
            {
                expneg = sign == '-' || sign == L'⁻';
                s = utf8_next(s);
            }
            bool expok = false;
            while (+s < +last)
            {
                unicode expchar = utf8_codepoint(s);
                uint expdig = fancy_digit_value(expchar, true);
                if (expdig >= 10)
                    break;
                expval = expval * 10 + expdig;
                s = utf8_next(s);
                expok = true;
            }
            if (!expok)
            {
                rt.exponent_error().source(s);
                return ERROR;
            }
            exponent += expneg ? -expval : expval;
        }
    }

    // Normalize the parsed value (#762)
    kint *rb = (kint *) scr.scratch();
    size_t rs = scr.growth() / sizeof(kint);
    if (!normalize(type, rb, rs, exponent))
        return ERROR;

    // Success: build the resulting number
    gcp<kint> kigits = rb;
    size_t    nkigs  = rs;
    p.length         = +s - +source;
    p.out            = rt.make<decimal>(type, exponent, nkigs, kigits);

    return p.out ? OK : ERROR;
}


RENDER_BODY(decimal)
// ----------------------------------------------------------------------------
//   Render the decimal number into the given renderer
// ----------------------------------------------------------------------------
{
    // Read information about the number
    info      sh       = o->shape();
    large     exponent = sh.exponent;
    size_t    nkigits  = sh.nkigits;
    gcbytes   base     = sh.base;
    decimal_g d        = o;
    bool      negative = o->type() == ID_neg_decimal;

    // Read formatting information from the renderer
    r.flush();
    bool      editing  = r.editing();
    size_t    rsize    = r.size();

    // Read settings
    settings &ds       = Settings;
    id        mode     = ds.DisplayMode();
    int       dispdigs = ds.DisplayDigits();
    int       digits   = dispdigs;
    int       std_exp  = ds.StandardExponent();
    bool      showdec  = ds.TrailingDecimal();
    unicode   space    = ds.NumberSeparator();
    uint      mant_spc = ds.MantissaSpacing();
    uint      frac_spc = ds.FractionSpacing();
    bool      fancy    = ds.FancyExponent();
    char      decimal  = ds.DecimalSeparator(); // Can be '.' or ','

    // Compute mantissa exponent, i.e. count of non-zero digits
    large     mexp     = nkigits * 3;
    int       rmdigit  = 0;
    while (mexp > 0)
    {
        kint k = kigit(+base, mexp / 3 - 1);
        if (k == 0)
        {
            mexp -= 3;
            continue;
        }
        rmdigit = k % 10;
        if (rmdigit == 0)
        {
            mexp--;
            k /= 10;
            rmdigit = k % 10;
            if (rmdigit == 0)
            {
                mexp--;
                k /= 10;
                rmdigit = k;
            }
        }
        break;
    }

    if (editing)
    {
        mode = object::ID_Std;
        digits += mexp;
        fancy = false;
        showdec = true;
    }
    if (mode == object::ID_Std)
        mode = object::ID_Sig;

    static uint16_t fancy_digit[10] =
    {
        L'⁰', L'¹', L'²', L'³', L'⁴',
        L'⁵', L'⁶', L'⁷', L'⁸', L'⁹'
    };

    // Emit sign if necessary
    if (negative)
    {
        r.put('-');
        rsize++;
    }

    // Loop checking for overflow
    bool overflow = false;
    do
    {
        // Position where we will emit the decimal dot when there is an exponent
        int decpos = 1;

        // Mantissa is between 0 and 1
        large  realexp  = exponent - 1;

        // Check if we need to switch to scientific notation in normal mode
        // On the negative exponents, we switch when digits would be lost on
        // display compared to actual digits. This is consistent with how HP
        // calculators do it. e.g 1.234556789 when divided by 10 repeatedly
        // switches to scientific notation at 1.23456789E-5, but 1.23 at
        // 1.23E-11 and 1.2 at 1.2E-12 (on an HP50G with 12 digits).
        // This is not symmetrical. Positive exponents switch at 1E12.
        // Note that the behaviour here is purposely different than HP's
        // when in FIX mode. In FIX 5, for example, 1.2345678E-5 is shown
        // on HP50s as 0.00001, and is shown here as 1.23457E-5, which I believe
        // is more useful. This behaviour is enabled by setting
        // MinimumSignificantDigits to a non-zero value. If the value is zero,
        // FIX works like on HP.  Also, since DB48X can compute on many digits,
        // and counting zeroes can be annoying, there is a separate setting,
        // StandardExponent, for when to switch to scientific notation.
        bool hasexp = mode == object::ID_Sci || mode == object::ID_Eng;
        if (!hasexp)
        {
            if (realexp < 0)
            {
                // Need to round up if last digit is above 5
                bool roundup = rmdigit >= 5;
                int shown = digits + realexp + roundup;
                int minfix = ds.MinimumSignificantDigits();
                if (minfix < 0)
                {
                    if (shown < 0)
                    {
                        nkigits = 0;
                        realexp = -digits;
                    }
                }
                else
                {
                    if (minfix > mexp)
                        minfix = mexp;
                    hasexp = shown < minfix || realexp < -std_exp;
                }
            }
            else
            {
                hasexp = realexp >= std_exp;
                if (!hasexp)
                    decpos = realexp + 1;
            }
        }

        // Position where we emit spacing (at sep == 0)
        //     10_000_000 with mant_spc = 3
        // sep=10-210-210
        uint sep = mant_spc ? (decpos - 1) % mant_spc : ~0U;

        // Number of decimals to show is given number of digits for most modes
        // (This counts *all* digits for standard / SIG mode)
        int decimals = digits;

        // Write leading zeroes if necessary
        if (!hasexp && realexp < 0)
        {
            // HP RPL calculators don't show leading 0, i.e. 0.5 shows as .5,
            // but this is only in STD mode, not in other modes.
            // This is pure evil and inconsistent with all older HP calculators
            // (which, granted, did not have STD mode) and later ones (Prime)
            // So let's decide that 0.3 will show as 0.3 in STD mode and not .3
            if (Settings.LeadingZero())
                r.put('0');
            decpos--;       // Don't emit the decimal separator twice

            // Emit decimal dot and leading zeros on fractional part
            if (showdec || realexp  < 0)
                r.put(decimal);
            sep = frac_spc-1;
            for (int zeroes = realexp + 1; zeroes < 0; zeroes++)
            {
                r.put('0');
                if (sep-- == 0)
                {
                    r.put(space);
                    sep = frac_spc - 1;
                }
                decimals--;
            }
        }

        // Adjust exponent being displayed for engineering mode
        large dispexp = realexp;
        bool engmode = mode == object::ID_Eng;
        if (engmode)
        {
            int offset = dispexp >= 0 ? dispexp % 3 : (dispexp - 2) % 3 + 2;
            decpos += offset;
            dispexp -= offset;
            if (mant_spc)
                sep = (sep + offset) % mant_spc;
            decimals += 1;
        }

        // Copy significant digits, inserting decimal separator when needed
        bool   sigmode = mode == object::ID_Sig;
        size_t lastnz  = r.size();
        size_t midx    = 0;
        uint   decade  = 0;
        kint   md      = 0;
        kint   d       = 0;
        while (midx < nkigits || decade)
        {
            // Find next digit and emit it
            if (decade == 0)
            {
                if (overflow)
                {
                    md = 1;
                    decade = 1;
                    midx = nkigits;
                }
                else
                {
                    md = kigit(+base, midx++);
                    decade = 3;
                }
            }
            decade--;

            d =  decade == 2 ? md / 100 : (decade == 1 ? (md / 10) : md) % 10;
            if (decpos <= 0 && decimals <= 0)
            {
                decade++;       // Enable rounding of last digit
                break;
            }

            r.put(char('0' + d));
            decpos--;

            // Check if we will need to eliminate trailing zeros
            if (decpos >= 0 || d)
                lastnz = r.size();

            // Insert spacing on the left of the decimal mark
            bool more = (midx < nkigits || decade) || !sigmode || decpos > 0;
            if (sep-- == 0 && more && decimals > 1)
            {
                if (decpos)
                {
                    r.put(space);
                    if (decpos > 0)
                        lastnz = r.size();
                }
                sep = (decpos > 0 ? mant_spc : frac_spc) - 1;
            }

            if (decpos == 0 && (more || showdec))
            {
                r.put(decimal);
                lastnz = r.size() - (!showdec && (sigmode || digits == 0));
                sep = frac_spc - 1;
            }

            // Count decimals after decimal separator, except in SIG mode
            // where we count all significant digits being displayed
            if (decpos < 0 || sigmode || engmode)
                decimals--;
        }

        // Check if we need some rounding on what is being displayed
        if ((midx < nkigits || decade) && d >= 5)
        {
            size_t rsz = r.size();
            char *start = (char *) r.text() + rsize;
            char *rptr = start + rsz - rsize;
            bool rounding = true;
            bool stripzeros = mode == object::ID_Sig;
            while (rounding && --rptr >= start)
            {
                if (*rptr >= '0' && *rptr <= '9')   // Do not convert '.' or '-'
                {
                    *rptr += 1;
                    rounding = *rptr > '9';
                    if (rounding)
                    {
                        *rptr -= 10;
                        if (stripzeros && *rptr == '0' && rptr > start)
                        {
                            r.unwrite(1);
                            decimals++;
                            decpos++;
                            uint spc = decpos > 0 ? mant_spc : frac_spc;
                            sep = (sep + 1) % spc;
                        }
                        else
                        {
                            stripzeros = false;
                        }
                    }
                }
                else if (*rptr == decimal)
                {
                    stripzeros = false;
                    if (!showdec)
                        r.unwrite(1);
                }
                else if (stripzeros) // Inserted separator
                {
                    r.unwrite(1);
                    sep = 0;
                }
            }

            // If we ran past the first digit, we overflowed during rounding
            // Need to re-run with the next larger exponent
            // This can only occur with a conversion of 9.9999 to 1
            if (rounding)
            {
                overflow = true;
                exponent++;
                r.reset_to(rsize);
                continue;
            }

            // Check if we need to reinsert the last separator
            if (sep-- == 0 && decpos > 0 && decimals > 1)
            {
                r.put(space);
                sep = (decpos > 0 ? mant_spc : frac_spc) - 1;
            }
        }

        // Return to position of last inserted zero
        else if ((!decpos || mode == object::ID_Sig) && r.size() > lastnz)
        {
            r.reset_to(lastnz);
        }


        // Do not add trailing zeroes in standard mode
        if (sigmode)
        {
            decimals = decpos > 0 ? decpos : 0;
        }
        else if (mode == object::ID_Fix && decpos > 0)
        {
            decimals = digits + decpos;
        }

        // Add trailing zeroes if necessary
        while (decimals > 0)
        {
            r.put('0');
            decpos--;

            if (sep-- == 0 && decimals > 1)
            {
                if (decpos)
                    r.put(space);
                sep = (decpos > 0 ? mant_spc : frac_spc) - 1;
            }

            if (decpos == 0 && showdec)
                r.put(decimal);
            decimals--;
        }

        // Add exponent if necessary
        if (hasexp)
        {
            r.put(ds.ExponentSeparator());
            char expbuf[32];
            char *end = render_i64(expbuf, dispexp);
            if (fancy)
            {
                for (char *ptr = expbuf; ptr < end; ptr++)
                {
                    char c = *ptr;
                    unicode u = c == '-' ? L'⁻' : fancy_digit[c - '0'];
                    r.put(u);
                }
            }
            else
            {
                r.put(cstring(expbuf), end - expbuf);
            }
        }
        return r.size();
    } while (overflow);
    return 0;
}



// ============================================================================
//
//   Conversions
//
// ============================================================================

ularge decimal::as_unsigned(bool magnitude) const
// ----------------------------------------------------------------------------
//   Convert a decimal value to an unsigned value
// ----------------------------------------------------------------------------
//   When magnitude is set, we return magnitude for negative values
{
    info   s       = shape();
    large  exp     = s.exponent;
    size_t nkigits = s.nkigits;
    byte_p bp      = s.base;
    if (exp < 0 || (!magnitude && type() == ID_neg_decimal))
        return 0;

    // If we overflow in the computation, return a "maxint"
    if (exp >= 19)
        return ~0UL;

    ularge xp  = exp;
    ularge pow = 1;
    ularge mul = 10;
    while (xp && pow)
    {
        if (xp & 1)
            pow *= mul;
        mul = mul * mul;
        xp /= 2;
    }
    if (!pow)
        return ~0ULL;

    ularge result = 0;
    for (size_t m = 0; m < nkigits && pow; m++)
    {
        kint d = kigit(bp, m);
        ularge next = result + d * pow / 1000;
        if (next < result)
            return ~0ULL;
        result = next;
        pow /= 1000;
    }
    return result;
}


large decimal::as_integer() const
// ----------------------------------------------------------------------------
//   Convert a decimal value to an integer
// ----------------------------------------------------------------------------
{
    large result = (large) as_unsigned(true);
    if (result == ~0L)
        result = 0x7FFFFFFFFFFFFFFFL;
    if (type() == ID_neg_decimal)
        result = -result;
    return result;
}


int32_t decimal::as_int32() const
// ----------------------------------------------------------------------------
//   Convert a decimal value to an int32_t
// ----------------------------------------------------------------------------
{
    large result = (large) as_unsigned(true);
    if (result == ~0L || result >= 0x80000000L)
        result = 0x7FFFFFFF;
    if (type() == ID_neg_decimal)
        result = -result;
    return (int32_t) result;
}


decimal_p decimal::from_integer(integer_p value)
// ----------------------------------------------------------------------------
//   Create a decimal value from an integer
// ----------------------------------------------------------------------------
{
    if (!value)
        return nullptr;
    id itype = value->type();
    id type = itype == ID_neg_integer ? ID_neg_decimal : ID_decimal;
    ularge magnitude = value->value<ularge>();
    return make(type, magnitude);
}


decimal_p decimal::from_bignum(bignum_p valuep)
// ----------------------------------------------------------------------------
//    Create a decimal number from a bignum
// ----------------------------------------------------------------------------
{
    if (!valuep)
        return nullptr;
    id        itype  = valuep->type();
    id        type   = itype == ID_neg_bignum ? ID_neg_decimal : ID_decimal;
    decimal_g result = make(type, 0);
    decimal_g digits;
    large     exp    = 0;
    bignum_g  value  = valuep;
    bignum_g  div    = bignum::make(1000000000000UL);
    bignum_g  kigit;

    while (!value->is_zero())
    {
        if (!bignum::quorem(value, div, itype, &value, &kigit))
            return nullptr;
        ularge kigval = kigit->value<ularge>();
        digits = make(type, kigval, exp);
        result = result + digits;
        exp += 12;
    }

    return result;
}


decimal_p decimal::from_random_seed(bignum_p valuep)
// ----------------------------------------------------------------------------
//    Create a decimal number from a bignum interpreted as a random seed
// ----------------------------------------------------------------------------
{
    if (!valuep)
        return nullptr;
    id        itype  = valuep->type();
    id        type   = ID_decimal;
    decimal_g result = make(type, 0);
    decimal_g digits;
    large     exp    = 0;
    bignum_g  value  = valuep;
    bignum_g  div    = bignum::make(1000000000000UL);
    bignum_g  kigit;

    while (!value->is_zero())
    {
        exp -= 12;
        if (!bignum::quorem(value, div, itype, &value, &kigit))
            return nullptr;
        ularge kigval = kigit->value<ularge>();
        digits = make(type, kigval, exp);
        result = result + digits;
    }

    return result;
}


decimal_p decimal::from_fraction(fraction_p value)
// ----------------------------------------------------------------------------
//   Build a decimal number from a fraction
// ----------------------------------------------------------------------------
{
    id type = value->type();
    if (type == ID_big_fraction || type == ID_neg_big_fraction)
        return from_big_fraction(big_fraction_p(value));
    decimal_g num = decimal::make(value->numerator_value());
    decimal_g den = decimal::make(value->denominator_value());
    if (type == ID_neg_fraction)
        num = -num;
    return num / den;
}



decimal_p decimal::from_big_fraction(big_fraction_p value)
// ------------------------------------------------------------------------
//   Build a decimal number from a big fraction
// ------------------------------------------------------------------------
{
    decimal_g num = decimal::from_bignum(value->numerator());
    decimal_g den = decimal::from_bignum(value->denominator());
    return num / den;
}


decimal::class_type decimal::fpclass() const
// ----------------------------------------------------------------------------
//   Return the floating-point class for the decimal number
// ----------------------------------------------------------------------------
{
    info   s       = shape();
    size_t nkigits = s.nkigits;
    byte_p bp      = s.base;
    bool   neg     = type() == ID_neg_decimal;
    if (nkigits == 0)
        return neg ? negativeZero : positiveZero;
    kint d = kigit(bp, 0);
    if (d >= 1000)
    {
        if (d == infinity)
            return neg ? negativeInfinity : positiveInfinity;
    }
    if (d < 100)
        return neg ? negativeSubnormal : positiveSubnormal;
    return neg ? negativeNormal : positiveNormal;
}


bool decimal::is_normal() const
// ----------------------------------------------------------------------------
//   Return true if the number is normal (not NaN, not infinity)
// ----------------------------------------------------------------------------
{
    info   s       = shape();
    if (s.exponent > large(Settings.MaximumDecimalExponent())) // Infinity
        return false;
    size_t nkigits = s.nkigits;
    byte_p bp      = s.base;
    if (nkigits == 0)
        return true;
    kint d = kigit(bp, 0);
    return d < 1000;
}


bool decimal::is_zero() const
// ----------------------------------------------------------------------------
//   The normal zero has no digits
// ----------------------------------------------------------------------------
{
    return shape().nkigits == 0;
}


bool decimal::is_one() const
// ----------------------------------------------------------------------------
//   Normal representation for one
// ----------------------------------------------------------------------------
{
    if (type() == ID_neg_decimal)
        return false;
    info   s       = shape();
    large  exp     = s.exponent;
    size_t nkigits = s.nkigits;
    byte_p bp      = s.base;
    return exp == 1 && nkigits == 1 && kigit(bp, 0) == 100;
}


bool decimal::is_negative() const
// ----------------------------------------------------------------------------
//   Return true if the value is strictly negative
// ----------------------------------------------------------------------------
{
    if (type() == ID_decimal)
        return false;
    return shape().nkigits != 0;
}


bool decimal::is_negative_or_zero() const
// ----------------------------------------------------------------------------
//   Return true if the value is zero o rnegative
// ------------------------------------------------------------------------
{
    if (type() == ID_neg_decimal)
        return true;
    return shape().nkigits == 0;
}


bool decimal::is_magnitude_less_than(uint kig, large exponent) const
// ----------------------------------------------------------------------------
//   Check if a given number is less than specified number in magnitude
// ----------------------------------------------------------------------------
{
    info   s       = shape();
    large  exp     = s.exponent;
    size_t nkigits = s.nkigits;
    byte_p bp      = s.base;
    if (exp != exponent)
        return exp < exponent;

    return nkigits == 0 || kigit(bp, 0) <= kig;
}


decimal_p decimal::truncate(large to_exp) const
// ----------------------------------------------------------------------------
//   Truncate a given decimal number (round towards zero)
// ----------------------------------------------------------------------------
{
    info s   = shape();

    // If we have 1E-3 and round at 0, return zero
    large exp = s.exponent;
    if (exp < to_exp)
        return make(0);

    // If rounding 10000 (10^4) to 0, we can copy 1 kigit as is
    size_t copy    = (exp - to_exp) / 3;
    size_t nkigits = s.nkigits;
    if (copy >= nkigits)
        return this;            // We copy everything

    gcbytes  bp = s.base;
    id       ty = type();
    scribble scr;

    for (size_t i = 0; i <= copy; i++)
    {
        kint k = kigit(+bp, i);
        if (i == copy)
        {
            size_t rm = (exp - to_exp) % 3;
            if (rm == 0)
                k = 0;
            else if (rm == 1)
                k -= k % 100;
            else if (rm == 2)
                k -= k % 10;
        }
        kint *kp = (kint *) rt.allocate(sizeof(kint));
        if (!kp)
            return nullptr;
        *kp = k;
    }

    kint  *rp = (kint *) scr.scratch();
    size_t rs = copy + 1;
    if (!normalize(ty, rp, rs, exp))
        return nullptr;
    return rt.make<decimal>(ty, exp, rs, gcp<kint>(rp));
}


decimal_p decimal::round(large to_exp) const
// ----------------------------------------------------------------------------
//   Round a given decimal number (round to nearest)
// ----------------------------------------------------------------------------
{
    info s   = shape();

    // If we have 1E-3 and round at 0, return zero
    large exp = s.exponent;
    if (exp < to_exp)
        return make(0);

    // If rounding 10000 (10^4) to 0, we can copy 1 kigit as is
    size_t copy    = (exp - to_exp) / 3;
    size_t nkigits = s.nkigits;
    if (copy >= nkigits)
        return this;            // We copy everything

    gcbytes  bp = s.base;
    id       ty = type();
    kint     ld = 0;
    scribble scr;

    for (size_t i = 0; i <= copy; i++)
    {
        kint k = kigit(+bp, i);
        if (i == copy)
        {
            size_t rm = (exp - to_exp) % 3;
            switch(rm)
            {
            case 0:
                ld = k >= 500;
                k = 0;
                break;
            case 1:
                ld = k % 100;
                k -= ld;
                ld = ld >= 50;
                if (ld)
                {
                    k += 100;
                    ld = k >= 1000;
                    if (ld)
                        k = 0;
                }
                break;
            case 2:
                ld = k % 10;
                k -= ld;
                ld = ld >= 5;
                if (ld)
                {
                    k += 10;
                    ld = k >= 1000;
                    if (ld)
                        k = 0;
                }
                break;
            }
        }
        kint *kp = (kint *) rt.allocate(sizeof(kint));
        if (!kp)
            return nullptr;
        *kp = k;
    }

    // Check if rounding is needed
    kint  *rp = (kint *) scr.scratch();
    size_t rs = copy + 1;
    while (ld && copy --> 0)
    {
        ld = ++rp[copy];
        ld = ld >= 1000;
        if (ld)
            rp[copy] = 0;
    }
    if (ld)
    {
        exp++;
        for (size_t i = rs; i > 0; --i)
            rp[i] = rp[i] / 10 + rp[i-1] % 10 * 100;
        *rp /= 10;
        *rp += ld * 100;
    }

    if (!normalize(ty, rp, rs, exp))
        return nullptr;

    return rt.make<decimal>(ty, exp, rs, gcp<kint>(rp));
}


bool decimal::split(decimal_g &ip, decimal_g &fp, large to_exp) const
// ----------------------------------------------------------------------------
//   Split a mumber between integral and decimal part
// ----------------------------------------------------------------------------
{
    info  s   = shape();

    // If we have 1E-3 and round at 0, return zero
    large exp = s.exponent;
    if (exp < to_exp)
    {
        fp = this;
        ip = make(0);
        return ip && fp;;
    }

    // If rounding 10000 (10^4) to 0, we can copoy 1 kigit as is
    size_t copy    = (exp - to_exp) / 3;
    size_t nkigits = s.nkigits;
    if (copy >= nkigits)
    {
        ip = this;
        fp = make(0);
        return fp && ip;
    }

    // Copy integral and decimal parts
    gcbytes  bp      = s.base;
    id       ty      = type();
    scribble scr;

    kint     rest = 0;
    large    fexp = exp - copy * 3;
    for (size_t i = 0; i <= copy; i++)
    {
        kint k = kigit(+bp, i);
        if (i == copy)
        {
            size_t rm = (exp - to_exp) % 3;
            switch(rm)
            {
            default:
            case 0:     rest = k;       break;
            case 1:     rest = k % 100; break;
            case 2:     rest = k % 10;  break;
            }
            k -= rest;
        }
        kint *kp = (kint *) rt.allocate(sizeof(kint));
        if (!kp)
            return false;
        *kp = k;
    }

    for (size_t i = copy; i < nkigits; i++)
    {
        kint k = i == copy ? rest : kigit(+bp, i);
        kint *kp = (kint *) rt.allocate(sizeof(kint));
        if (!kp)
            return false;
        *kp = k;
    }

    kint     *irp   = (kint *) scr.scratch();
    size_t    irs   = copy + 1;
    kint     *frp   = irp + irs;
    size_t    frs   = nkigits - copy;
    if (!normalize(ty, irp, irs, exp) || !normalize(ty, frp, frs, fexp))
        return false;

    gcp<kint> ibuf = irp;
    gcp<kint> fbuf = frp;
    ip = rt.make<decimal>(ty, exp, irs, ibuf);
    fp = rt.make<decimal>(ty, fexp, frs, fbuf);
    return ip && fp;
}


bool decimal::split(large &ip, decimal_g &fp, large to_exp) const
// ----------------------------------------------------------------------------
//   Split integral and fractional part, convert integral part to large
// ----------------------------------------------------------------------------
{
    decimal_g dip;
    if (!split(dip, fp, to_exp))
        return false;
    ip = dip->as_integer();
    return true;
}


algebraic_p decimal::to_integer() const
// ----------------------------------------------------------------------------
//   Convert a decimal to an integer or bignum
// ----------------------------------------------------------------------------
{
    decimal_g x      = this;
    info      xi     = x->shape();
    large     xe     = xi.exponent;

    // The standard integer type can hold 18 digits,
    // but kigit scaling below tops out at 16 digits, then may overflow
    if (xe <= 16)
    {
        if (xe < 0)
            return integer::make(0);

        size_t  xs    = xi.nkigits;
        gcbytes xb    = xi.base;
        bool    neg   = x->type() == ID_neg_decimal && xe >= 0;
        large   xl    = xe - 3 * xs;
        ularge  scale = 1;
        ularge  mul   = 10;
        ularge  res   = 0;
        if (xl >= 0)
        {
            large p = xl;
            while (p)
            {
                if (p & 1)
                    scale *= mul;
                p >>= 1;
                mul *= mul;
            }
        }
        else if (large(xs) > -xl/3)
        {
            xs += xl/3;
            xl = xl % 3;
            if (xl < 0)
            {
                xs--;

                kint xk = kigit(xb, xs);
                scale = xl == -1 ? 100 : 10;
                res = xk / (1000 / scale);
            }
        }
        else
        {
            return integer::make(0);
        }

        for (size_t xd = xs; xd --> 0; xl += 3)
        {
            kint xk = kigit(xb, xd);
            res += xk * scale;
            scale *= 1000;
        }

        id ty = neg ? ID_neg_integer : ID_integer;
        return rt.make<integer>(ty, res);
    }
    return x->to_bignum();
}


bignum_p decimal::to_bignum() const
// ----------------------------------------------------------------------------
//   Return the decimal value as a bignum
// ----------------------------------------------------------------------------
{
    decimal_g x     = this;
    info      xi    = x->shape();
    size_t    xs    = xi.nkigits;
    large     xe    = xi.exponent;
    gcbytes   xb    = xi.base;
    bool      neg   = x->type() == ID_neg_decimal;

    bignum_g  scale = bignum::make(1);
    bignum_g  mul   = bignum::make(10);
    bignum_g  tmp;

    large     p = xe;
    if (p < 0)
        neg = false;
    while (p > 0 && scale && mul)
    {
        if (p & 1)
            scale = scale * mul;
        p >>= 1;
        mul = mul * mul;
    }

    id       ty  = neg ? ID_neg_bignum : ID_bignum;
    bignum_g res = rt.make<bignum>(ty, 0);
    mul = bignum::make(1000);
    for (size_t xd = 0; xd < xs; xd++)
    {
        kint xk = kigit(xb, xd);
        tmp = rt.make<bignum>(ty, xk);
        tmp = tmp * scale;
        res = res + tmp;
        scale = scale / mul;
        if (scale && scale->is_zero())
            break;
    }

    res = res / mul;
    return res;
}


algebraic_p decimal::to_fraction(uint count, uint decimals) const
// ----------------------------------------------------------------------------
//   Convert a decimal value to a fraction
// ----------------------------------------------------------------------------
{
    decimal_g   num = this;
    decimal_g   next, ip, fp, one;
    bignum_g    n1, d1, n2, d2, s, i;
    bool        neg = num->is_negative();
    if (!num->split(ip, fp))
        return nullptr;
    if (fp->is_zero())
        return ip->to_integer();

    if (neg)
    {
        ip = decimal::neg(ip);
        fp = decimal::neg(fp);
    }
    one = make(1);
    n1 = ip->to_bignum();
    d1 = bignum::make(1);
    n2 = d1;
    d2 = bignum::make(0);

    uint maxdec = Settings.Precision() - 3;
    if (decimals > maxdec)
        decimals = maxdec;

    while (count--)
    {
        // Check if the decimal part is small enough
        if (fp->is_zero())
            break;
        large exp = fp->exponent();
        if (-exp > large(decimals))
            break;

        next = one / fp;
        if (!next)
            return nullptr;
        ip = next->truncate();
        if (!ip)
            return nullptr;
        i = ip->to_bignum();

        s = n1;
        n1 = i * n1;
        n1 = n1 + n2;
        n2 = s;

        s = d1;
        d1 = i * d1;
        d1 = d1 + d2;
        d2 = s;

        fraction_g f = +big_fraction::make(n1, d1);
        fp = num - decimal_g(decimal::from_fraction(f));
        if (fp->is_zero())
            break;

        fp = next - ip;
    }

    algebraic_g result = d1->is_one()
                           ? algebraic_p(+n1)
                           : algebraic_p(+big_fraction::make(n1, d1));
    if (neg)
        result = -result;
    return +result;
}


float decimal::to_float() const
// ----------------------------------------------------------------------------
//   Convert decimal value to float
// ----------------------------------------------------------------------------
{
    settings::SaveFancyExponent   saveFancyExponent(false);
    settings::SaveDecimalComma    saveDecimalComma(false);
    settings::SaveMantissaSpacing saveMantissaSpacing(0);
    settings::SaveFractionSpacing saveFractionSpacing(0);
    settings::SaveDisplayDigits   saveDisplayMode(ID_Std);
    renderer                      r;
    size_t                        sz = render(r);
    r.put(char(0));
    char *txt = (char *) r.text();
    txt[sz] = 0;
    return std::strtof(txt, nullptr);
}


double decimal::to_double() const
// ----------------------------------------------------------------------------
//   Convert decimal value to double
// ----------------------------------------------------------------------------
{
    settings::SaveFancyExponent   saveFancyExponent(false);
    settings::SaveDecimalComma    saveDecimalComma(false);
    settings::SaveMantissaSpacing saveMantissaSpacing(0);
    settings::SaveFractionSpacing saveFractionSpacing(0);
    settings::SaveDisplayDigits   saveDisplayMode(ID_Std);
    renderer                      r;
    size_t                        sz = render(r);
    r.put(char(0));
    char *txt = (char *) r.text();
    txt[sz] = 0;
    return std::strtod(txt, nullptr);
}


decimal_p decimal::from(double x)
// ----------------------------------------------------------------------------
//   Conversion from hardware floating-point to decimal
// ----------------------------------------------------------------------------
{
    if (std::isfinite(x))
    {
        renderer r;
        r.printf("%.18g", x);
        parser p(r.text(), r.size());
        if (decimal::do_parse(p) == OK)
            return decimal_p(+p.out);
    }
    else if (std::isinf(x))
    {
        if (constant_p cst = constant::lookup("∞"))
            if (decimal_p inf = decimal_p(cst->value()))
                return x > 0 ? inf : neg(inf);

    }
    return nullptr;
}


int decimal::compare(decimal_r x, decimal_r y, uint epsilon)
// ----------------------------------------------------------------------------
//   Return -1, 0 or 1 for comparison
// ----------------------------------------------------------------------------
//   epsilon indicates how many digits we are considering
{
    // Quick exit if identical pointers
    if (+x == +y)
        return 0;

    // Check if input is nullptr - If so, nullptr is smaller than value
    if (!x || !y)
        return !!x - !!y;

    id   xty = x->type();
    id   yty = y->type();

    // Check negative vs. positive
    if (xty != yty)
        return (xty == ID_decimal) - (yty == ID_decimal);

    // Read information from both numbers
    int  sign = xty == ID_neg_decimal ? -1 : 1;
    info xi   = x->shape();
    info yi   = y->shape();

    // Check special case of zero
    size_t xs = xi.nkigits;
    size_t ys = yi.nkigits;

    // Number with largest exponent is larger
    large xe   = xs ? xi.exponent : std::numeric_limits<large>::min();
    large ye   = ys ? yi.exponent : std::numeric_limits<large>::min();
    if (xe != ye)
        return sign * (xe > ye ? 1 : -1);

    // If same exponent, compare mantissa digits starting with highest one
    byte_p xb = xi.base;
    byte_p yb = yi.base;

    // Compare up to the given precision
    if (epsilon)
    {
        // epsilon = 1 -> s = 1, m = 100
        // epsilon = 2 -> s = 1, m = 10
        size_t l = epsilon / 3;
        size_t m = epsilon % 3;
        size_t d = m == 1 ? 100 : m == 2 ? 10 : 1;
        size_t e = (epsilon + 2) / 3;
        for (size_t i = 0; i < e; i++)
        {
            uint xk = i < xs ? kigit(xb, i) : 0;
            uint yk = i < ys ? kigit(yb, i) : 0;
            if (i+1 == l)
            {
                xk /= d;
                yk /= d;
            }
            if (int diff = xk - yk)
                return sign * diff;
        }
    }
    else
    {
        size_t s  = std::min(xs, ys);
        for (size_t i = 0; i < s; i++)
            if (int diff = kigit(xb, i) - kigit(yb, i))
                return sign * diff;

        // If all kigits were the same, longest number is larger
        if (xs != ys)
            return sign * int(xs - ys);
    }

    // Otherwise, numbers are identical
    return 0;
}



// ============================================================================
//
//    Basic arithmetic operations
//
// ============================================================================

static inline object::id negtype(object::id type)
// ----------------------------------------------------------------------------
//   Return the opposite type
// ----------------------------------------------------------------------------
{
    return type == object::ID_decimal ? object::ID_neg_decimal
                                      : object::ID_decimal;
}


decimal_p decimal::neg(decimal_r x)
// ----------------------------------------------------------------------------
//   Negation
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    object::id type  = x->type();
    object::id ntype = negtype(type);
    gcbytes data = x->payload();
    size_t len = x->size() - leb128size(type);
    return rt.make<decimal>(ntype, len, data);
}



template <decimal_p (*code)(decimal_r, decimal_r)>
algebraic_p decimal_wrapper(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Wrapper to silence warning about function casts in recent XCode
// ----------------------------------------------------------------------------
{
    decimal_r xb = (decimal_r) x;
    decimal_r yb = (decimal_r) y;
    decimal_p rb = code(xb, yb);
    return rb;
}


template <decimal_p (*code)(decimal_r, decimal_r)>
arithmetic_fn target(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//  Target function for decimal objects
// ----------------------------------------------------------------------------
{
    return x->is_decimal() && y->is_decimal() &&
        (!Settings.HardwareFloatingPoint() || Settings.Precision() > 16)
        ? decimal_wrapper<code> : nullptr;
}


decimal_p decimal::add(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Addition of two numbers with the same sign
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    id xty = x->type();
    id yty = y->type();
    if (xty != yty)
        return subtract(x, decimal_g(neg(y)));
    add::remember(target<add>);

    // Read information from both numbers
    info  xi = x->shape();
    info  yi = y->shape();
    large xe = xi.exponent;
    large ye = yi.exponent;
    id    ty = x->type();

    // Put the smallest exponent in y
    bool lt = xe < ye;
    if (lt)
    {
        std::swap(xe, ye);
        std::swap(xi, yi);
    }

    // Check dimensions
    size_t   xs     = xi.nkigits;
    size_t   ys     = yi.nkigits;
    gcbytes  xb     = xi.base;
    gcbytes  yb     = yi.base;
    size_t   yshift = xe - ye;
    size_t   kshift = yshift / 3;
    kint     mod3   = yshift % 3;

    // Size of result - y can be wider than x
    size_t   ps     = (Settings.Precision() + 2) / 3;
    size_t   rs     = std::min(ps, std::max(xs, ys + (yshift + 2) / 3));

    // Check if y is negligible relative to x
    if (rs < kshift)
        return lt ? y : x;

    // Allocate the mantissa
    scribble scr;
    kint    *rb = (kint *) rt.allocate(rs * sizeof(kint));
    if (!rb)
        return nullptr;

    // Addition loop
    kint   hmul  = mod3 == 2 ? 100 : mod3 == 1 ? 10 : 1;
    kint   lmul  = 1000 / hmul;
    kint   carry = 0;
    size_t ko    = rs;
    while (ko-- > 0)
    {
        kint xk = ko < xs ? kigit(+xb, ko) : 0;
        kint yk = carry;
        if (ko >= kshift)
        {
            size_t yo = ko - kshift;
            if (yo < ys)
                yk += kigit(+yb, yo) / hmul;
            if (mod3 && ko > kshift && --yo < ys)
                yk += kigit(+yb, yo) % hmul * lmul;
        }
        xk += yk;
        carry = xk >= 1000;
        if (carry)
            xk -= 1000;
        rb[ko] = xk;
    }

    // Check if a carry remains above top
    if (carry)
    {
        uint expincr = 1;
        hmul = 10;
        while (carry >= hmul)
        {
            hmul *= 10;
            expincr++;
        }
        xe += expincr;
        if (rs < ps)
        {
            rb = (kint *) rt.allocate(sizeof(kint)) - rs;
            if (!rb)
                return nullptr;
            rb[rs] = 0;
            rs++;
        }

        ko = rs;
        lmul = 1000 / hmul;
        while (ko --> 0)
        {
            kint above = ko ? rb[ko-1] : carry;
            rb[ko] = rb[ko] / hmul + (above % hmul) * lmul;
        }
    }

    // Normalize result
    if (!normalize(ty, rb, rs, xe))
        return nullptr;

    // Build the result
    gcp<kint> kigits = rb;
    decimal_p result = rt.make<decimal>(ty, xe, rs, kigits);
    return result;
}


decimal_p decimal::subtract(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Subtraction of two numbers with the same sign
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    id xty = x->type();
    id yty = y->type();
    if (xty != yty)
        return add(x, decimal_g(neg(y)));
    subtract::remember(target<subtract>);

    // Read information from both numbers
    info  xi = x->shape();
    info  yi = y->shape();
    large xe = xi.exponent;
    large ye = yi.exponent;
    id    ty = x->type();
    bool  lt = xe < ye;

    // Put the smallest exponent in y
    if (lt)
    {
        std::swap(xe, ye);
        std::swap(xi, yi);
    }

    // Check dimensions
    size_t   xs     = xi.nkigits;
    size_t   ys     = yi.nkigits;
    gcbytes  xb     = xi.base;
    gcbytes  yb     = yi.base;
    size_t   yshift = xe - ye;
    size_t   kshift = yshift / 3;
    kint     mod3   = yshift % 3;

    // Size of result - y can be wider than x
    size_t   ps     = (Settings.Precision() + 2) / 3;
    size_t   rs     = std::min(ps, std::max(xs, ys + (yshift + 2) / 3));

    // Check if y is negligible relative to x
    if (rs < kshift)
        return lt ? neg(y) : decimal_p(x);

    // Allocate the mantissa
    scribble scr;
    kint    *rb = (kint *) rt.allocate(rs * sizeof(kint));
    if (!rb)
        return nullptr;

    // Subtraction loop
    kint   hmul  = mod3 == 2 ? 100 : mod3 == 1 ? 10 : 1;
    kint   lmul  = 1000 / hmul;
    kint   carry = 0;
    size_t ko    = rs;
    while (ko-- > 0)
    {
        kint xk = ko < xs ? kigit(+xb, ko) : 0;
        kint yk = carry;
        if (ko >= kshift)
        {
            size_t yo = ko - kshift;
            if (yo < ys)
                yk += kigit(+yb, yo) / hmul;
            if (mod3 && ko > kshift && --yo < ys)
                yk += kigit(+yb, yo) % hmul * lmul;
        }
        carry = xk < yk;
        if (carry)
            xk += 1000;
        xk = xk - yk;
        rb[ko] = xk;
    }

    // Check if a carry remains above top, e.g. 0.5 - 0.6 = -0.1
    if (carry)
    {
        ko = rs;
        uint rev = 1000;
        while (ko --> 0)
        {
            rb[ko] = rev - rb[ko];
            rev = 999;
        }
        lt = !lt;
    }

    // Check if we need to change the sign
    if (lt)
        ty = negtype(ty);

    // Normalize result
    if (!normalize(ty, rb, rs, xe))
        return nullptr;

    // Build the result
    gcp<kint> kigits = rb;
    decimal_p result = rt.make<decimal>(ty, xe, rs, kigits);
    return result;
}


decimal_p decimal::multiply(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Multiplication of two decimal numbers
// ----------------------------------------------------------------------------
//  (a0+a1/1000) * (b0+b1/1000) = a0*b0 + (a0*b1+a1*b0) / 1000 + epsilon
//  Exponent is the sum of the two exponents
{
    if (!x || !y)
        return nullptr;
    multiply::remember(target<multiply>);

    // Read information from both numbers
    info     xi  = x->shape();
    info     yi  = y->shape();
    large    xe  = xi.exponent;
    large    ye  = yi.exponent;
    id       xty = x->type();
    id       yty = y->type();
    id       ty  = xty == yty ? ID_decimal : ID_neg_decimal;

    // Check dimensions
    size_t   xs  = xi.nkigits;
    size_t   ys  = yi.nkigits;
    gcbytes  xb  = xi.base;
    gcbytes  yb  = yi.base;
    large    re  = xe + ye - 3;

    // Size of result
    size_t   ps  = (Settings.Precision() + 2) / 3;
    size_t   rs  = std::min(ps, xs + ys + 1);

    // Allocate the mantissa
    scribble scr;
    kint    *rb = (kint *) rt.allocate(rs * sizeof(kint));
    if (!rb)
        return nullptr;

    // Zero the result before doing sums on it
    for (size_t ri = 0; ri < rs; ri++)
        rb[ri] = 0;

    // Sum on all digits
    uint carry = 0;
    for (size_t xi = 0; xi < xs; xi++)
    {
        kint xk = kigit(xb, xi);
        for (size_t yi = 0; yi < ys; yi++)
        {
            size_t ri = xi + yi;
            if (ri >= rs)
                break;
            kint yk = kigit(yb, yi);
            uint rk = xk * yk;
            while (rk)
            {
                rk += rb[ri];
                rb[ri] = rk % 1000;
                rk /= 1000;
                if (ri-- == 0)
                    break;
            }
            carry += rk;
        }
    }

    // Check if a carry remains above top
    while (carry)
    {
        // Round things up
        size_t ri = rs - 1;
        bool overflow = rb[ri] >= 500;
        while (overflow && ri --> 0)
        {
            overflow = ++rb[ri] >= 1000;
            if (overflow)
                rb[ri] %= 1000;
        }
        if (overflow)
            carry++;

        memmove(rb + 1, rb, sizeof(kint) * (rs - 1));
        *rb = carry % 1000;
        re += 3;
        carry = carry / 1000;
    }

    // Strip leading zeroes three by three
    while (rs && *rb == 0)
    {
        re -= 3;
        rb++;
        rs--;
    }

    // Strip up to two individual leading zeroes
    if (rs && *rb < 100)
    {
        re -= 1 + (*rb < 10);
        uint hmul = *rb < 10 ? 100 : 10;
        uint lmul = 1000 / hmul;
        for (size_t ko = 0; ko < rs; ko++)
        {
            kint next = ko + 1 < rs ? rb[ko + 1] : 0;
            rb[ko] = (rb[ko] * hmul + next / lmul) % 1000;
        }
    }

    // Normalize result
    if (!normalize(ty, rb, rs, re))
        return nullptr;

    // Build the result
    gcp<kint> kigits = rb;
    decimal_p result = rt.make<decimal>(ty, re, rs, kigits);
    return result;
}


decimal_p decimal::divide(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Division of two decimal numbers
// ----------------------------------------------------------------------------
//
//   This uses the traditional algorithm, but with digits between 0 and 999
//
//      Q = 0
//      R = 0
//      for i in digits(X)
//          R = R * 1000 + X[i]
//          Q[i] = R[0] / D[0]
//          R = R - Y * Q[i]
{
    if (!x || !y)
        return nullptr;
    divide::remember(target<divide>);

    // Check if we divide by zero
    if (y->is_zero())
    {
        rt.zero_divide_error();
        return nullptr;
    }

    // Read information from both numbers
    info     xi  = x->shape();
    info     yi  = y->shape();
    large    xe  = xi.exponent;
    large    ye  = yi.exponent;
    id       xty = x->type();
    id       yty = y->type();
    id       ty  = xty == yty ? ID_decimal : ID_neg_decimal;

    // Size of result
    size_t   rs  = (Settings.Precision() + 2) / 3 + 1;
    size_t   qs  = rs;

    // Check dimensions
    size_t   xs  = std::min(xi.nkigits, rs);
    size_t   ys  = std::min(yi.nkigits, rs);
    gcbytes  xb  = xi.base;
    gcbytes  yb  = yi.base;
    large    re  = xe - ye;

    // Allocate memory for the result
    scribble scr;
    kint    *rp = (kint *) rt.allocate((rs + qs + xs + ys) * sizeof(kint));
    if (!rp)
        return nullptr;

    // Read the kigits from both inputs
    kint    *qp = rp + rs;
    kint    *xp = qp + qs;
    kint    *yp = xp + xs;
    for (size_t xi = 0; xi < xs; xi++)
        xp[xi] = kigit(+xb, xi);
    for (size_t yi = 0; yi < ys; yi++)
        yp[yi] = kigit(+yb, yi);

    // Initialize remainder and quotient with 0
    size_t rqs = rs + qs;
    for (size_t xi = 0; xi < xs; xi++)
        rp[xi] = xp[xi];
    for (size_t rqi = xs; rqi < rqs; rqi++)
        rp[rqi] = 0;


    // Only the first kigit can overflow, e.g. 300 / 100.
    // After that, these are remainders, so always smaller than Y[0]
    uint yv = yp[0] + (ys > 0);

    // Loop on the numerator
    size_t qi = 0;
    while (qi < qs)
    {
        // R = R * 1000
        uint rv = rp[0];
        bool forward = rv < yv;
        if (forward)
            rv *= 1000;

        // Q[i] = R[0] / Y[0]
        uint q = rv / yv;
        if (q)
        {
            size_t qdi = qi - !forward;
            if (qdi + 1)
            {
                qp[qdi] += q;
                if (qp[qdi] >= 1000)
                {
                    size_t ci = qdi;
                    while (ci)
                    {
                        qp[ci] -= 1000;
                        ci--;
                        if (++qp[ci] < 1000)
                            break;
                    }
                }
            }
            else
            {
                // Special case of overflow on first iteration
                qdi++;
                qp[qdi] += 1000 * q;
            }

            // R = R - Y * q;
            uint mulcarry = 0;
            uint subcarry = 0;
            for (size_t yi = ys; yi --> 0; )
            {
                size_t ri = yi + forward;
                uint yk = q * yp[yi] + mulcarry;
                uint rk = ri < rs ? rp[ri] : 0;
                rk = 1000 + rp[ri] - yk % 1000 - subcarry;
                subcarry = 1 - rk / 1000;
                mulcarry = yk / 1000;
                if (ri < rs)
                    rp[ri] = rk % 1000;
            }

            // Check if we overflowed during subtraction. If so, adjust
            uint wanted = rv / 1000;
            uint achieved = mulcarry + subcarry;
            int diff = wanted - achieved;
            if (forward)
                rp[0] -= achieved;
            if (diff)
                forward = false;
        }

        if (forward)
        {
            qi++;
            memmove(rp, rp + 1, sizeof(kint) * (rs - 1));
        }
    }

    // Round up last digits
    if (qp[qi-1] > 500)
    {
        while (qi > 0)
        {
            --qi;
            qp[qi]++;
            if (!qi || qp[qi] < 1000)
                break;
            qp[qi] -= 1000;
        }
    }

    // Case where we started with an overflow, e.g. 300/100
    while(qp[0] >= 1000)
    {
        re++;
        for (size_t qi = rs; qi > 0; --qi)
            qp[qi] = qp[qi] / 10 + qp[qi-1] % 10 * 100;
        *qp = *qp / 10;
    }

    // Normalize result
    if (!normalize(ty, qp, qs, re))
        return nullptr;

    if (qs >= rs)
        qs = rs - 1;

    // Build the result
    gcp<kint> kigits = qp;
    decimal_p result = rt.make<decimal>(ty, re, qs, kigits);
    return result;
}


decimal_p decimal::rem(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Remainder
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    rem::remember(target<rem>);
    decimal_g q = x / y;
    if (!q)
        return nullptr;
    q = q->truncate();
    return x - q * y;
}


decimal_p decimal::mod(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Modulo
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    mod::remember(target<mod>);
    decimal_g r = rem(x, y);
    if (x->is_negative() && !r->is_zero())
        r = y->is_negative() ? r - y : r + y;
    return r;
}


decimal_p decimal::pow(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Power
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    pow::remember(target<pow>);
    precision_adjust prec;
    return prec(exp(y * ln(x)));
}


decimal_p decimal::hypot(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Hypothenuse
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    hypot::remember(target<hypot>);
    precision_adjust prec;
    return prec(sqrt(x*x + y*y));
}


decimal_p decimal::atan2(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Arc-tangent with two arguments (arctan(x/y))
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    atan2::remember(target<atan2>);
    precision_adjust prec;
    if (y->is_zero())
    {
        if (x->is_zero())
            return y->is_negative() ? exact_angle(1, 0) : +x;
        return prec(exact_angle(x->is_negative() ? -5 : 5, -1));
    }

    decimal_g result = atan(x/y);
    if (y->is_negative())
    {
        decimal_g offs = exact_angle(x->is_negative() ? -1 : 1, 0);
        result = result + offs;
    }
    return prec(result);
}


decimal_p decimal::Min(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Min of two decimal values
// ----------------------------------------------------------------------------
{
    return x < y ? x : y;
}


decimal_p decimal::Max(decimal_r x, decimal_r y)
// ----------------------------------------------------------------------------
//   Max of two decimal values
// ----------------------------------------------------------------------------
{
    return x > y ? x : y;
}



// ============================================================================
//
//   Math functions
//
// ============================================================================

uint decimal::precision_adjust::adjusted = 0;

decimal_p decimal::sqrt(decimal_r x)
// ----------------------------------------------------------------------------
//   Square root using Newton's method
// ----------------------------------------------------------------------------
{
    if (x->is_negative())
    {
        rt.domain_error();
        return nullptr;
    }

    large     exponent = x->exponent();
    decimal_g half     = make(5, -1);
    decimal_g next     = make(5, (-exponent - 1) / 2);
    decimal_g current  = x * next;
    if (current && !current->is_zero())
    {
        precision_adjust prec;
        for (uint i = 0; i < 2 * prec; i++)
        {
            next = (current + x / current) * half;
            if (!next || compare(next, current, prec) == 0)
                break;
            current = next;
        }
        current = prec(current);
    }
    return current;
}


decimal_p decimal::cbrt(decimal_r x)
// ----------------------------------------------------------------------------
//  Cube root
// ----------------------------------------------------------------------------
{
    large     exponent = x->exponent();
    decimal_g third    = inv(make(3));
    decimal_g next     = make(1, -2 * exponent / 3);
    decimal_g current  = x * next;
    if (current && !current->is_zero())
    {
        precision_adjust prec;
        for (uint i = 0; i < 2 * prec; i++)
        {
            next = ((current + current) + x / (current * current)) * third;
            if (!next || compare(next, current, prec) == 0)
                break;
            current = next;
        }
        current = prec(current);
    }
    return current;
}


decimal_p decimal::sin(decimal_r x)
// ----------------------------------------------------------------------------
//   Sine function
// ----------------------------------------------------------------------------
{
    uint qturns;
    decimal_g fp;
    precision_adjust prec;
    if (!x->adjust_from_angle(qturns, fp))
        return nullptr;
    return prec(sin_fracpi(qturns, fp));
}


decimal_p decimal::cos(decimal_r x)
// ----------------------------------------------------------------------------
//   Cosine function
// ----------------------------------------------------------------------------
{
    uint qturns;
    decimal_g fp;
    precision_adjust prec;
    if (!x->adjust_from_angle(qturns, fp))
        return nullptr;
    return prec(cos_fracpi(qturns, fp));
}


decimal_p decimal::sin_fracpi(uint qturns, decimal_r fp)
// ----------------------------------------------------------------------------
//   Compute the sine of input expressed as fraction of pi
// ----------------------------------------------------------------------------
//   'qturns` is the number of quarter turns (pi/2), between -3 and 3
//   The 'fp' input determines ratio of the quarter turn
{
    bool small = fp->is_magnitude_less_than_half();
    if (!small)
    {
        // sin(pi/2 - x) = cos(x)
        id fty = fp->type();
        decimal_g x = make(fty, 1);
        x = x - fp;
        if (fty == ID_neg_decimal)
            qturns += 2;
        return cos_fracpi(-qturns, x);
    }
    qturns %= 4;
    if (qturns % 2)
        // sin(x+pi/2) = cos x
        return cos_fracpi((qturns - 1U) % 4, fp);

    // Scale by pi / 2, sum is between 0 and pi/4
    decimal_g sum = fp;
    decimal_g fact = make(2);
    decimal_g tmp;
    sum = sum / fact;
    sum = sum * pi();
    fact = make(6); // 3!

    // Prepare power factor and square that we multiply by every time
    decimal_g power = sum;
    decimal_g square = sum * sum;

    uint prec = Settings.Precision();
    for (uint i = 3; i < prec; i += 2)
    {
        power = power * square; // First iteration is x^3
        tmp = power / fact;     // x^3 / 3!

        // Check if we ran out of memory
        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        if ((i / 2) & 1)
            sum = sum - tmp;
        else
            sum = sum + tmp;

        tmp = make((i+1) * (i+2)); // First iteration: 4 * 5
        fact = fact * tmp;
    }

    // sin(x+pi) = -si(x)
    if (qturns != 0)
        sum = -sum;
    return sum;
}


decimal_p decimal::cos_fracpi(uint qturns, decimal_r fp)
// ----------------------------------------------------------------------------
//   Compute the cosine of input expressed as fraction of pi
// ----------------------------------------------------------------------------
{
    bool small = fp->is_magnitude_less_than_half();
    if (!small)
    {
        // cos(pi/2 - x) = sin(x)
        id fty = fp->type();
        decimal_g x = make(fty, 1);
        x = x - fp;
        if (fty == ID_neg_decimal)
            qturns += 2;
        return sin_fracpi(-qturns, x);
    }
    qturns %= 4;
    if (qturns % 2)
        // cos(x+3*pi/2) = sin x
        return sin_fracpi((qturns - 3U) % 4, fp);

    // Scale by pi / 2, sum is between 0 and pi/4
    decimal_g sum = fp;
    decimal_g fact = make(2); // Also 2!
    decimal_g tmp;
    sum = sum / fact;
    sum = sum * pi();

    // Prepare power factor and square that we multiply by every time
    decimal_g square = sum * sum;
    decimal_g power = square;

    // For cosine, the sum starts at 1
    sum = make(1);

    uint prec = Settings.Precision();
    for (uint i = 2; i < prec; i += 2)
    {
        tmp = power / fact;     // x^2 / 2!

        // Check if we ran out of memory
        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        if ((i / 2) & 1)
            sum = sum - tmp;
        else
            sum = sum + tmp;

        power = power * square; // Next iteration is x^4
        tmp = make((i+1) * (i+2)); // First iteration: 4 * 5
        fact = fact * tmp;
    }

    // sin(x+pi) = -si(x)
    if (qturns != 0)
        sum = -sum;
    return sum;
}


decimal_p decimal::tan(decimal_r x)
// ----------------------------------------------------------------------------
//   Compute the tangent as ratio of sin/cos
// ----------------------------------------------------------------------------
{
    uint qturns;
    decimal_g fp;
    precision_adjust prec;
    if (!x->adjust_from_angle(qturns, fp))
        return nullptr;
    decimal_g s = sin_fracpi(qturns, fp);
    decimal_g c = cos_fracpi(qturns, fp);
    return prec(s / c);
}


decimal_p decimal::asin(decimal_r x)
// ----------------------------------------------------------------------------
//   Arc-sine, use asin(x) = atan(x / sqrt(1-x^2))
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g tmp = make(1);
    tmp = tmp - x * x;
    if (tmp && tmp->is_zero())
    {
        tmp = exact_angle(x->is_negative() ? -5 : 5, -1);
    }
    else
    {
        tmp = x / sqrt(tmp);
        tmp = atan(tmp);
    }
    return prec(tmp);
}


decimal_p decimal::acos(decimal_r x)
// ----------------------------------------------------------------------------
//   Arc-cosine, use acos(x) = atan(sqrt(1-x^2) / x)
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    precision_adjust prec;
    decimal_g tmp;
    if (!x->is_zero())
    {
        tmp = make(1);
        tmp = tmp - x * x;
        tmp = sqrt(tmp) / x;
        tmp = atan(tmp);

        if (x->is_negative())
        {
            decimal_g offs = exact_angle(1, 0);
            tmp = tmp + offs;
        }
    }
    else
    {
        tmp = exact_angle(5,-1);
    }
    return prec(tmp);
}


decimal_p decimal::atan(decimal_r x)
// ----------------------------------------------------------------------------
//  Implementation of arctan
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    // Special case of 0
    if (x->is_zero())
        return x;

    // Reduce negative values to simplify equalities below and converge better
    if (x->is_negative())
    {
        decimal_g tmp = atan(-x);
        return -tmp;
    }

    // Check if we have a value of x above 1, if so reduce for convergence
    precision_adjust prec;
    if (x->exponent() >= 1 && !x->is_one())
    {
        // atan(1/x) = pi/2 - arctan(x) when x > 0
        decimal_g i = make(1);
        i = i / x;
        i = atan(i);
        decimal_g half = exact_angle(5, -1);
        i = half - i;
        return prec(i);
    }

    // Check if above 0.5
    if (!x->is_magnitude_less_than_half())
    {
        // atan(x) = pi/4 + atan((x - 1) / (1 + x))
        decimal_g one = make(1);
        decimal_g nx = (x - one) / (x + one);
        nx = atan(nx);
        decimal_g fourth = exact_angle(25,-2);
        nx = fourth + nx;
        return prec(nx);
    }

    // Prepare power factor and square that we multiply by every time
    decimal_g tmp;
    decimal_g sum = x;
    decimal_g square = x * x;
    decimal_g power = x;

    record(decimal, "atan of %t", +x);
    record(decimal, "sum=   %t", +sum);
    record(decimal, "power= %t", +power);
    record(decimal, "square=%t", +square);

    for (uint i = 3; i < 3 * prec; i += 2)
    {
        power = power * square;
        record(decimal, "%u: power= %t", i, +power);
        tmp = make(i);
        tmp = power / tmp;     // x^2 / 2
        record(decimal, "%u: factor=%t exponent %lld", i, +tmp, tmp->exponent());
        // Check if we ran out of memory
        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        if ((i/2) & 1)
            sum = sum - tmp;
        else
            sum = sum + tmp;
        record(decimal, "%u: sum=   %t exponent %lld", i, +sum, sum->exponent());
    }

    // Convert to current angle mode
    sum = sum->adjust_to_angle();

    return prec(sum);
}


decimal_p decimal::sinh(decimal_r x)
// ----------------------------------------------------------------------------
//    Hyperbolic sine
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g half = make(5,-1);
    decimal_g ep = exp(x);
    decimal_g em = exp(-x);
    return prec((ep - em) * half);
}


decimal_p decimal::cosh(decimal_r x)
// ----------------------------------------------------------------------------
//  Hyperbolic cosine
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g half = make(5,-1);
    decimal_g ep = exp(x);
    decimal_g em = exp(-x);
    return prec((ep + em) * half);
}


decimal_p decimal::tanh(decimal_r x)
// ----------------------------------------------------------------------------
//   Hyperbolic tangent
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g hs = sinh(x);
    decimal_g hc = cosh(x);
    return prec(hs / hc);
}


decimal_p decimal::asinh(decimal_r x)
// ----------------------------------------------------------------------------
//  Inverse hyperbolic sine
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g one = make(1);
    return prec(ln(x + decimal_g(sqrt(x*x + one))));
}


decimal_p decimal::acosh(decimal_r x)
// ----------------------------------------------------------------------------
//  Inverse hyperbolic cosine
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g one = make(1);
    return prec(ln(x + decimal_g(sqrt(x*x - one))));
}


decimal_p decimal::atanh(decimal_r x)
// ----------------------------------------------------------------------------
//   Inverse hyperbolic tangent
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g one = make(1);
    decimal_g half = make(5, -1);
    return prec(half * ln((one + x) / (one - x)));
}


decimal_p decimal::ln1p(decimal_r x)
// ----------------------------------------------------------------------------
//   ln(1+x)
// ----------------------------------------------------------------------------
//   To reduce relatively efficiently to something that converges quickly.
//   we use the relation 10^2/e^3 ≈ 0.5, so that we can take the exponent,
//   divide by 3, multiply by 7, and that gives us a reasonable idea of
//   a power of e we can use as a divisor
{
    if (!x)
        return nullptr;
    if (x->is_zero())
        return x;

    precision_adjust prec;
    decimal_g one = make(1);
    decimal_g scaled = x + one;
    if (scaled->is_negative() || scaled->is_zero())
    {
        rt.domain_error();
        return nullptr;
    }

    large     texp  = x->exponent();
    large     eexp  = texp * 3 / 2;
    large     ipart = 0;
    decimal_g power, scale;

    scaled = x;
    record(decimal, "Start with %t exp=%ld eexp=%ld", +scaled, texp, eexp);
    while (eexp > 0)
    {
        power = constants().e;
        scale = one;
        ipart += eexp;

        record(decimal, "Exponent of e eexp=%ld", eexp);
        while (eexp)
        {
            if (eexp & 1)
                scale = scale * power;
            power = power * power;
            eexp >>= 1;
        }

        record(decimal, "Scale is %t", +scale);
        scaled = (one + scaled) / scale - one;

        texp = scaled->exponent();
        eexp = texp * 3 / 2;
        record(decimal, "Scaled is %t, exp=%ld eexp=%ld", +scaled, texp, eexp);
    }

    // If we end up with 0.999, convergence will be super slow
    while (!scaled->is_magnitude_less_than_half())
    {
        record(decimal, "Rescaling, %t, exp=%ld eexp=%ld ipart=%ld",
               +scaled, texp, eexp, ipart);

        scale = constants().e;
        if (scaled->is_negative())
        {
            scaled = (one + scaled) * scale - one;
            ipart -= 1;
        }
        else
        {
            scaled = (one + scaled) / scale - one;
            ipart += 1;
        }
    }

    record(decimal, "Taylor series with %t exp=%ld eexp=%ld ipart=%ld",
           +scaled, texp, eexp, ipart);

    // Taylor's serie
    decimal_g sum = scaled;
    power = scaled;
    for (uint i = 2; i < 3*prec; i++)
    {
        power = power * scaled;
        scale = make(i);
        scale = power / scale;

        if (!sum || !scale)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (scale->exponent() + large(prec) < sum->exponent())
        {
            record(decimal, "Taylor exits at %u exp=%ld", i, scale->exponent());
            break;
        }

        if (i & 1)
            sum = sum + scale;
        else
            sum = sum - scale;
    }
    record(decimal, "Power at exit %t exponent %ld", +power, power->exponent());
    record(decimal, "Sum   at exit %t exponent %ld", +sum, sum->exponent());

    if (ipart)
    {
        scale = make(ipart);
        sum = sum + scale;
    }
    return prec(sum);
}


decimal_p decimal::expm1(decimal_r x)
// ----------------------------------------------------------------------------
//   Exponential minus one
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    large ip = 0;
    decimal_g fp;
    if (!x->split(ip, fp))
        return nullptr;

    // Prepare power factor and square that we multiply by every time
    decimal_g one =  make(1);
    decimal_g sum = fp;
    decimal_g fact = one;
    decimal_g power = fp;
    decimal_g tmp;

    precision_adjust prec;
    for (uint i = 2; i < prec; i++)
    {
        power = power * fp;
        tmp = make(i);
        fact = fact * tmp;

        tmp = power / fact;     // x^2 / 2!

        // Check if we ran out of memory
        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        sum = sum + tmp;
    }

    if (ip)
    {
        bool neg = ip < 0;
        if (neg)
            ip = -ip;
        fact = one;
        power = constants().e;
        while (ip)
        {
            if (ip & 1)
                fact = fact * power;
            ip >>= 1;
            if (ip)
                power = power * power;
        }
        if (neg)
            sum = (sum + one) / fact - one;
        else
            sum = (sum + one) * fact - one;
    }

    return prec(sum);
}


decimal_p decimal::ln(decimal_r x)
// ----------------------------------------------------------------------------
//   Compute natural logarithm of x
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_zero() || x->is_negative())
    {
        rt.domain_error();
        return nullptr;
    }

    precision_adjust prec;
    bool      negln  = x->exponent() <= -1;
    decimal_g one    = make(1);
    if (negln)
    {
        decimal_g scaled = one / x - one;
        scaled = ln1p(scaled);
        scaled = -scaled;
        return prec(scaled);
    }
    decimal_g scaled = x - one;
    scaled = ln1p(scaled);
    return prec(scaled);
}


decimal_p decimal::log10(decimal_r x)
// ----------------------------------------------------------------------------
//  Logarithm in base 10
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_zero() || x->is_negative())
    {
        rt.domain_error();
        return nullptr;
    }

    precision_adjust prec;
    large exp10 = x->exponent() - 1;
    decimal_g fp = x;
    if (exp10)
    {
        fp = make(1, -exp10);
        fp = fp * x;
    }
    decimal_g lnx = ln(fp);
    decimal_g ln10 = constants().ln10();
    ln10 = lnx / ln10;
    if (exp10)
    {
        fp = make(exp10);
        ln10 = ln10 + fp;
    }
    return prec(ln10);
}


decimal_p decimal::log2(decimal_r x)
// ----------------------------------------------------------------------------
//  Logarithm in base 2
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    decimal_g lnx = ln(x);
    decimal_g ln2 = constants().ln2();
    return prec(lnx / ln2);
}


decimal_p decimal::exp(decimal_r x)
// ----------------------------------------------------------------------------
//   Exponential
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    large ip = 0;
    decimal_g fp;
    if (!x->split(ip, fp))
        return nullptr;

    // Compute exponential for integral part
    precision_adjust prec;
    decimal_g one = make(1);
    decimal_g result = expm1(fp);
    result = (one + result);

    if (ip)
    {
        bool neg = ip < 0;
        if (neg)
            ip = - ip;
        decimal_g scale = one;
        decimal_g power = constants().e;
        while (ip)
        {
            if (ip & 1)
                scale = scale * power;
            ip >>= 1;
            if (ip)
                power = power * power;
        }
        if (neg)
            result = result / scale;
        else
            result = result * scale;
    }

    return prec(result);
}


decimal_p decimal::exp10(decimal_r x)
// ----------------------------------------------------------------------------
//  Exponential in base 10
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    large ip = 0;
    decimal_g fp;
    if (!x->split(ip, fp))
        return nullptr;
    precision_adjust prec;
    fp = constants().ln10() * fp;
    fp = exp(fp);
    if (ip)
    {
        decimal_g scale = make(1, ip);
        fp = scale * fp;
    }
    return prec(fp);
}


decimal_p decimal::exp2(decimal_r x)
// ----------------------------------------------------------------------------
//   Exponential in base 2
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(exp(constants().ln2() * x));
}


decimal_p decimal::erf(decimal_r x)
// ----------------------------------------------------------------------------
//   Error function
// ----------------------------------------------------------------------------
//   See http://www.mhtlab.uwaterloo.ca/courses/me755/web_chap2.pdf
{
    if (!x)
        return nullptr;

    if (x->is_negative())
        return -decimal_g(erf(-x));

    precision_adjust prec;
    if (!x->is_magnitude_less_than(300, 1))
    {
        // Use asymptotic expansion
        decimal_g one = make(1);
        decimal_g rest = erfc(x);
        return prec(one - rest);
    }

    // Taylor's serie
    decimal_g sum    = x;
    decimal_g square = x * x;
    decimal_g power  = sum;
    decimal_g fact = make(1);
    decimal_g tmp;

    for (uint i = 1; i < 2 * prec; i++)
    {
        // First term is x^3 / (3 * 1!), second is x^5 / (5 * 2!)
        power = power * square;         // x^3
        tmp = make(i);                  // 1
        fact = fact * tmp;              // 1!
        tmp = make(2*i+1);              // 3
        tmp = fact * tmp;               // 1! * 3
        tmp = power / tmp;              // x^3 / (1! * 3)

        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        if (i & 1)
            sum = sum - tmp;
        else
            sum = sum + tmp;
    }

    // Multiply result by 2 / sqrt(pi)
    sum = sum * constants().two_over_sqrt_pi();
    return prec(sum);
}


decimal_p decimal::erfc(decimal_r x)
// ----------------------------------------------------------------------------
//  Compute the complementary error function
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    // Use erf() for values below 2
    precision_adjust prec;
    if (x->is_negative() || x->is_magnitude_less_than(300, 1))
    {
        // Use asymptotic expansion
        decimal_g one = make(1);
        decimal_g rest = erf(x);
        return prec(one - rest);
    }

    // 1 - 1 / (2x^2) + (1*3) / (2x^2)^2 - (1*3*5) / (2x^2)^3 + ...
    decimal_g one    = make(1);
    decimal_g sum    = one;
    decimal_g square = x * x;
    decimal_g power  = one;
    decimal_g scale = make(1);
    decimal_g tmp;
    square = square + square;           // 2x^2

    for (uint i = 1; i < prec; i++)
    {
        // First term is x^3 / (3 * 1!), second is x^5 / (5 * 2!)
        power = power * square;         // (2x^3)^1
        tmp = make(2*i-1);              // 1, 3, 5, ...
        scale = scale * tmp;            // 1 * 1 * 3 * 5 ...
        tmp = scale / power;            // (1*3*5) / (2x^2)^3

        if (!sum || !tmp)
            return nullptr;

        // If what we add no longer has an impact, we can exit
        if (tmp->exponent() + large(prec) < sum->exponent())
            break;

        if (i & 1)
            sum = sum - tmp;
        else
            sum = sum + tmp;
    }

    // Divide result by sqrt(pi) * x * exp(x^2)
    sum = sum * constants().one_over_sqrt_pi();
    sum = sum / x;
    tmp = exp(-(x * x));
    sum = sum * tmp;
    return prec(sum);
}


decimal_p decimal::tgamma(decimal_r x)
// ----------------------------------------------------------------------------
//  Implementation of the gamma function
// ----------------------------------------------------------------------------
//   Largely inspired from https://deamentiaemundi.wordpress.com/2013/06/29
//      /the-gamma-function-with-spouges-approximation/comment-page-1/
{
    if (!x)
        return nullptr;

    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    if (fp->is_zero())
    {
        if (x->is_negative() || x->is_zero())
        {
            rt.domain_error();
            return nullptr;
        }
        fp = make(1);
        ip = ip - fp;
        return fact(ip);
    }

    precision_adjust prec;
    if (x->is_negative())
    {
        // gamma(x) = pi/(sin(pi*x) * gamma(1-x))
        ip = x + x;
        ip = sin_fracpi(0, ip);
        fp = make(1);
        fp = fp - x;
        fp = lgamma_internal(fp);
        fp = exp(fp);
        fp = fp * ip;
        ip = constants().pi;
        fp = ip / fp;
    }
    else
    {
        fp = exp(lgamma_internal(x));
    }
    fp = prec(fp);
    return fp;
}


decimal_p decimal::lgamma(decimal_r x)
// ----------------------------------------------------------------------------
//   Implementation of the log of the gamma function
// ----------------------------------------------------------------------------
//   Largely inspired from https://deamentiaemundi.wordpress.com/2013/06/29
//      /the-gamma-function-with-spouges-approximation/comment-page-1/
{
    precision_adjust prec;
    decimal_g result = lgamma_internal(x);
    result = prec(result);
    return result;
}


decimal_p decimal::lgamma_internal(decimal_r x)
// ----------------------------------------------------------------------------
//   Implementation of the log of the gamma function
// ----------------------------------------------------------------------------
//   Largely inspired from https://deamentiaemundi.wordpress.com/2013/06/29
//      /the-gamma-function-with-spouges-approximation/comment-page-1/
{
    if (!x)
        return nullptr;

    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    if (fp->is_zero())
    {
        if (x->is_negative() || x->is_zero())
        {
            rt.domain_error();
            return nullptr;
        }
        // Above 50, might as well use Stirling's approximation
        if (ip->exponent() < 50)
        {
            fp = make(1);
            ip = ip - fp;
            ip = fact(ip);
            return ln(ip);
        }
    }

    if (x->is_negative())
    {
        // gamma(x) = pi/(asin(pi*x) * gamma(1-x))
        ip = x + x;
        ip = sin_fracpi(0, ip);
        ip = ln(ip);
        fp = make(1);
        fp = lgamma_internal(fp - x);
        fp = fp + ip;
        ip = constants().lnpi();
        fp = ip - fp;
        return fp;
    }

    // Spouge's approximation uses a factor `a` - Compute it here
    uint digits = Settings.Precision();
    decimal_g tmp = make(digits + 4);
    decimal_g a = make(12528504409125680958ULL , -19);
    a = ceil(a * tmp);
    precision_adjust prec(digits < 24 ? 6 : digits/4, true);

    // Allocate number of ck elements we need
    uint na = a->as_unsigned();
    record(decimal, "a=%t na=%u", +a, na);
    decimal_g *cks = constants().gamma_realloc(na);

    // Loop for terms except first one
    decimal_g factorial = make(1);
    decimal_g sum       = constants().sqrt_2pi();
    decimal_g one       = make(1);
    decimal_g z         = x;
    decimal_g ck, power, scale;
    record(decimal, "First sum %t", +sum);
    for (uint i = 1; i < na; i++)
    {
        z   = z + one;
        record(decimal, "%u: z=%t", i, +z);

        tmp = cks[i-1];
        if (!tmp)
        {
            uint t  = na - i;
            uint xp = i - 1;
            tmp     = make(t);
            power   = tmp;
            scale   = exp(tmp);
            record(decimal, "%u: exp=%t", i, +scale);
            while (xp)
            {
                if (xp & 1)
                    scale = scale * power;
                xp >>= 1;
                if (xp)
                    power = power * power;
            }
            tmp = sqrt(tmp);
            record(decimal, "%u: sqrt=%t", i, +tmp);
            tmp = tmp * scale / factorial;

            cks[i-1] = tmp;
            cleaner::disable();
            if (!tmp)
                return nullptr;

            scale = make(i);
            factorial = factorial * scale;
            record(decimal, "%u: factorial=%t", i, +factorial);
        }
        record(decimal, "%u: ck=%t", i, +tmp);
        if (i & 1)
            sum = sum + tmp / z;
        else
            sum = sum - tmp / z;
        record(decimal, "%u: sum=%t", i, +sum);
    }

    sum = ln(sum);

    // Add first term
    tmp = x + a;
    z = make(5, -1);
    z = x + z;
    a = ln(x);
    tmp = ln(tmp) * z - tmp - a;
    sum = sum + tmp;

    return sum;
}


decimal_p decimal::abs(decimal_r x)
// ----------------------------------------------------------------------------
//   Absolute value
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    return x->is_negative() ? neg(x) : +x;
}


decimal_p decimal::sign(decimal_r x)
// ----------------------------------------------------------------------------
//   Sign of the operand
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    int r = x->is_negative() ? -1 : x->is_zero() ? 0 : 1;
    return make(r);
}


decimal_p decimal::IntPart(decimal_r x)
// ----------------------------------------------------------------------------
//   Return integer part
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    return ip;
}


decimal_p decimal::FracPart(decimal_r x)
// ----------------------------------------------------------------------------
//   Return fractional part
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    return fp;
}


decimal_p decimal::ceil(decimal_r x)
// ----------------------------------------------------------------------------
//   Find the lowest integer just above this value
// ----------------------------------------------------------------------------
{
    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    if (fp->is_zero() || x->is_negative())
        return ip;
    fp = make(1);
    ip = ip + fp;
    return ip;
}


decimal_p decimal::floor(decimal_r x)
// ----------------------------------------------------------------------------
//  Return the highest integer just below this value
// ----------------------------------------------------------------------------
{
    decimal_g ip, fp;
    if (!x->split(ip, fp))
        return nullptr;
    if (fp->is_zero() || !x->is_negative())
        return ip;
    fp = make(1);
    ip = ip - fp;
    return ip;
}


decimal_p decimal::inv(decimal_r x)
// ----------------------------------------------------------------------------
//  Compute the inverse
// ----------------------------------------------------------------------------
{
    decimal_p one = make(1);
    return one / x;
}


decimal_p decimal::sq(decimal_r x)
// ----------------------------------------------------------------------------
//   Implement square
// ----------------------------------------------------------------------------
{
    return x * x;
}


decimal_p decimal::cubed(decimal_r x)
// ----------------------------------------------------------------------------
//  Implement cubed value
// ----------------------------------------------------------------------------
{
    return x * x * x;
}


decimal_p decimal::xroot(decimal_r y, decimal_r x)
// ----------------------------------------------------------------------------
//  Find the n-th root of a value
// ----------------------------------------------------------------------------
{
    large iip;
    decimal_g xfp;
    if (!x->split(iip, xfp))
        return nullptr;

    bool is_neg = false;
    bool is_int = xfp->is_zero();
    if (is_int)
    {
        is_neg = y->is_negative();
        if (is_neg && ((iip & 1) == 0))
        {
            // Root of a negative number
            rt.domain_error();
            return nullptr;
        }
    }

    xfp = inv(x);
    if (is_neg)
        xfp = neg(pow(neg(y), xfp));
    else
        xfp = pow(y, xfp);
    return xfp;
}


decimal_p decimal::fact(decimal_r x)
// ----------------------------------------------------------------------------
//   Compute the factorial of a number
// ----------------------------------------------------------------------------
{
    large ip;
    decimal_g fp;
    if (!x->split(ip, fp))
        return nullptr;
    if (!fp->is_zero() || x->is_negative())
    {
        fp = make(1);
        fp = x + fp;
        return tgamma(fp);
    }

    decimal_g r = make(1);
    for (large i = 2; i <= ip; i++)
    {
        fp = make(i);
        r = r * fp;
    }
    return r;
}



// ============================================================================
//
//   Support math functions
//
// ============================================================================

#include "decimal-pi.h"
#include "decimal-e.h"

decimal::ccache &decimal::constants()
// ----------------------------------------------------------------------------
//   Initialize the constants used for adjustments
// ----------------------------------------------------------------------------
{
    static ccache *cst = nullptr;
    if (!cst)
    {
        // operator new support purposefully not linked in embedded versions
        cst = (ccache *) malloc(sizeof(ccache));
        new(cst) ccache;
    }
    size_t precision = Settings.Precision();
    if (cst->precision != precision)
    {
        record(decimal, "Switched constants precision from %u to %u",
               cst->precision, precision);
        size_t nkigs   = (precision + 2) / 3;
        cst->pi        = rt.make<decimal>(1, nkigs, gcbytes(decimal_pi));
        cst->e         = rt.make<decimal>(1, nkigs, gcbytes(decimal_e));
        cst->log10     = nullptr;
        cst->log2      = nullptr;
        cst->sq2pi     = nullptr;
        cst->oosqpi    = nullptr;
        cst->lpi       = nullptr;
        cst->precision = precision;
        cleaner::disable();
    }
    return *cst;
}


decimal_r decimal::ccache::ln10()
// ----------------------------------------------------------------------------
//   Compute and cache the natural logarithm of 10
// ----------------------------------------------------------------------------
{
    if (!log10)
    {
        decimal_g ten = make(10);
        log10 = ln(ten);
        cleaner::disable();
    }
    return log10;
}


decimal_r decimal::ccache::ln2()
// ----------------------------------------------------------------------------
//   Compute and cache the natural logarithm of 2
// ----------------------------------------------------------------------------
{
    if (!log2)
    {
        decimal_g two = make(2);
        log2 = ln(two);
        cleaner::disable();
    }
    return log2;
}


decimal_r decimal::ccache::lnpi()
// ----------------------------------------------------------------------------
//   Compute and cache the natural logarithm of pi
// ----------------------------------------------------------------------------
{
    if (!lpi)
    {
        lpi = ln(pi);
        cleaner::disable();
    }
    return lpi;
}


decimal_r decimal::ccache::sqrt_2pi()
// ----------------------------------------------------------------------------
//   Compute and cache sqrt(pi)
// ----------------------------------------------------------------------------
{
    if (!sq2pi)
    {
        sq2pi = sqrt(pi + pi);
        cleaner::disable();
    }
    return sq2pi;
}


decimal_r decimal::ccache::one_over_sqrt_pi()
// ----------------------------------------------------------------------------
//   Compute and cache 1/sqrt(pi)
// ----------------------------------------------------------------------------
{
    if (!oosqpi)
    {
        decimal_g one = make(1);
        decimal_g sqpi = sqrt(pi);
        oosqpi = one / sqpi;
        cleaner::disable();
    }
    return oosqpi;
}


decimal_g decimal::ccache::two_over_sqrt_pi()
// ----------------------------------------------------------------------------
//   Compute and cache 2/sqrt(pi)
// ----------------------------------------------------------------------------
{
    decimal_g oosqpi = one_over_sqrt_pi();
    return oosqpi + oosqpi;
}


decimal_g *decimal::ccache::gamma_realloc(size_t na)
// ----------------------------------------------------------------------------
//    Reallocate the constants for gamma computation
// ----------------------------------------------------------------------------
{
    if (na != gamma_na)
    {
        size_t rna = gamma_na - 1;
        if (gamma_ck)
        {
            // No operator new[] nor operator delete[] in embedded runtime
            for (size_t i = rna; i --> 0; )
                (gamma_ck + i)->~decimal_g();
            free(gamma_ck);
            gamma_ck = nullptr;
        }
        if (na > 1)
        {
            // No operator new[] nor operator delete[] in embedded runtime
            rna = na - 1;
            gamma_ck = (decimal_g *) calloc(rna, sizeof(decimal_g));
            if (!gamma_ck)
                na = rna = 0;
            for (size_t i = 0; i < rna; i++)
                new(gamma_ck + i) decimal_g;
        }
        gamma_na = na;
    }
    return gamma_ck;
}


decimal_p decimal::pi()
// ----------------------------------------------------------------------------
//   Compute pi without clearing the constants
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(constants().pi);
}


decimal_p decimal::e()
// ----------------------------------------------------------------------------
//   Compute e without clearing the constants
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(constants().e);
}


decimal_p decimal::ln10()
// ----------------------------------------------------------------------------
//   Compute ln(10) without clearing the constants
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(constants().ln10());
}


decimal_p decimal::ln2()
// ----------------------------------------------------------------------------
//   Compute ln(2) without clearing the constants
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(constants().ln2());
}


decimal_p decimal::lnpi()
// ----------------------------------------------------------------------------
//   Compute ln(pi) without clearing the constants
// ----------------------------------------------------------------------------
{
    precision_adjust prec;
    return prec(constants().lnpi());
}


bool decimal::adjust_from_angle(uint &qturns, decimal_g &fp) const
// ----------------------------------------------------------------------------
//   Adjust an angle value for sin/cos/tan, qturns is number of quarter turns
// ----------------------------------------------------------------------------
{
    decimal_g x = this;
    switch(Settings.AngleMode())
    {
    case object::ID_Deg:       x = x / decimal_g(make(90)); break;
    case object::ID_Grad:      x = x * decimal_g(make(1,-2)); break;
    case object::ID_PiRadians: x = x + x; break;
    default:
    case object::ID_Rad:       x = x / pi(); x = x + x; break;
    }

    decimal_g ip;
    if (!x->split(ip, fp))
        return false;

    // Bring the integral part in 0-9 so that we can convert to int
    large iexp = ip->exponent();
    if (iexp > 1)
    {
        if (iexp > 4 && Settings.ReportPrecisionLoss())
        {
            rt.precision_loss_error();
            return false;
        }
        decimal_g turn = make(4);
        ip = rem(ip, turn);
        if (!ip)
            return false;
    }
    large q = ip->as_integer();
    qturns = uint(q);
    return ip;
}


decimal_p decimal::adjust_to_angle() const
// ----------------------------------------------------------------------------
//   Adjust an angle value for asin/acos/atan
// ----------------------------------------------------------------------------
{
    uint half_circle = 1;
    switch(Settings.AngleMode())
    {
    case object::ID_Deg:                half_circle = 180; break;
    case object::ID_Grad:               half_circle = 200; break;
    case object::ID_PiRadians:          half_circle =   1; break;
    default:
    case object::ID_Rad:                return this;
    }

    decimal_g x = this;
    decimal_g ratio = make(half_circle);
    x = x * ratio;
    x = x / pi();
    return x;
}



decimal_p decimal::exact_angle(int digits, int exponent)
// ----------------------------------------------------------------------------
//   Generate an exact fraction of pi for exact angles
// ----------------------------------------------------------------------------
{
    id        am    = Settings.AngleMode();
    auto      deg   = object::ID_Deg;
    auto      grad  = object::ID_Grad;
    int       half  = am == deg ? 180 : am == grad ? 200 : 1;
    decimal_g ratio = make(digits * half, exponent);
    if (am == object::ID_Rad)
        ratio = ratio * pi();
    return ratio;
}
