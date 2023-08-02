// ****************************************************************************
//  decimal32.cc                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of decimal floating point using Intel's library
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
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

#include "decimal-32.h"

#include "arithmetic.h"
#include "bignum.h"
#include "fraction.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "utf8.h"

#include <algorithm>
#include <bid_conf.h>
#include <bid_functions.h>
#include <cstdio>
#include <cstdlib>

using std::min;
using std::max;

RECORDER(decimal32, 32, "Decimal32 data type");


decimal32::decimal32(bignum_p num, id type)
// ----------------------------------------------------------------------------
//   Create a decimal32 from a bignum value
// ----------------------------------------------------------------------------
    : algebraic(type)
{
    bid32 result;
    bid32 mul;
    unsigned z = 0;
    bid32_from_uint32(&result.value, &z);
    z = 256;
    bid32_from_uint32(&mul.value, &z);

    size_t size = 0;
    byte_p n = num->value(&size);
    for (uint i = 0; i < size; i++)
    {
        unsigned digits = n[size - i - 1];
        bid32 step;
        bid32_mul(&step.value, &result.value, &mul.value);
        bid32 add;
        bid32_from_uint32(&add.value, &digits);
        bid32_add(&result.value, &step.value, &add.value);
    }
    if (num->type() == ID_neg_bignum)
        bid32_negate(&result.value, &result.value);
    byte *p = (byte *) payload();
    memcpy(p, &result, sizeof(result));
}


decimal32::decimal32(fraction_p f, id type)
// ----------------------------------------------------------------------------
//   Create a decimal32 from a bignum value
// ----------------------------------------------------------------------------
    : algebraic(type)
{
    bid32 result, numerator, divisor;
    bid32 mul;
    unsigned z = 0;
    bid32_from_uint32(&result.value, &z);
    numerator = divisor = result;
    z = 256;
    bid32_from_uint32(&mul.value, &z);

    size_t size = 0;
    bignum_p num = f->numerator();
    byte_p n = num->value(&size);
    for (uint i = 0; i < size; i++)
    {
        unsigned digits = n[size - i - 1];
        bid32 step;
        bid32_mul(&step.value, &numerator.value, &mul.value);
        bid32 add;
        bid32_from_uint32(&add.value, &digits);
        bid32_add(&numerator.value, &step.value, &add.value);
    }

    bignum_p den = f->denominator();
    byte_p d = den->value(&size);
    for (uint i = 0; i < size; i++)
    {
        unsigned digits = d[size - i - 1];
        bid32 step;
        bid32_mul(&step.value, &divisor.value, &mul.value);
        bid32 add;
        bid32_from_uint32(&add.value, &digits);
        bid32_add(&divisor.value, &step.value, &add.value);
    }

    bid32_div(&result.value, &numerator.value, &divisor.value);
    if (f->type() == ID_neg_fraction)
        bid32_negate(&result.value, &result.value);
    byte *p = (byte *) payload();
    memcpy(p, &result, sizeof(result));

}


SIZE_BODY(decimal32)
// ----------------------------------------------------------------------------
//    Compute size for a decimal32 payload
// ----------------------------------------------------------------------------
{
    return ptrdiff(o->payload(), o) + sizeof(bid32);
}


PARSE_BODY(decimal32)
// ----------------------------------------------------------------------------
//    Try to parse this as an decimal32
// ----------------------------------------------------------------------------
{
    record(decimal32, "Parsing [%s]", (utf8) p.source);

    utf8 source = p.source;
    utf8 s = source;
    utf8 last = source + p.length;

    // Skip leading sign
    if (*s == '+' || *s == '-')
    {
        // In an equation, `1 + 3` should interpret `+` as an infix
        if (p.precedence < 0)
            return SKIP;
        s++;
    }

    // Skip digits
    utf8 digits = s;
    while (s < last && (*s >= '0' && *s <= '9'))
        s++;

    // Check decimal dot
    bool hadDecimalDot = *s == '.' || *s == ',';
    if (hadDecimalDot)
    {
        s++;
        while (s < last && (*s >= '0' && *s <= '9'))
            s++;
    }

    // If we had no digits, check for special names or exit
    if (s == digits)
    {
        if (strncasecmp(cstring(s), "inf", sizeof("inf") - 1) != 0 &&
            strncasecmp(cstring(s), "∞",   sizeof("∞")   - 1) != 0 &&
            strncasecmp(cstring(s), "NaN", sizeof("NaN") - 1) != 0)
            return SKIP;
        record(decimal32, "Recognized NaN or Inf", s);
    }

    // Check how many digits were given
    uint mantissa = s - digits - hadDecimalDot;
    record(decimal32, "Had %u digits, max %u", mantissa, BID32_MAXDIGITS);
    if (mantissa > BID32_MAXDIGITS)
    {
        rt.mantissa_error().source(digits + mantissa);
        return WARN;                    // Try again with higher-precision
    }

    // Check exponent
    utf8 exponent = nullptr;
    if (*s == 'e' || *s == 'E' || utf8_codepoint(s) == Settings.exponent_mark)
    {
        s = utf8_next(s);
        exponent = s;
        if (*s == '+' || *s == '-')
            s++;
        utf8 expval = s;
        while (s < last && (*s >= '0' && *s <= '9'))
            s++;
        if (s == expval)
        {
            rt.exponent_error().source(s);
            return ERROR;
        }
    }

    // Check if exponent is withing range, if not skip to wider format
    if (exponent)
    {
        int expval = atoi(cstring(exponent));
        int maxexp = 32 == 127+1 ? 6144 : 32 == 63+1 ? 384 : 96;
        record(decimal32, "Exponent is %d, max is %d", expval, maxexp);
        if (expval < -(maxexp-1) || expval > maxexp)
        {
            rt.exponent_range_error().source(s);
            return WARN;
        }
    }

    // Patch the input to the BID library
    char buf[50];
    char *b = buf;
    for (utf8 u = source; u < s && b < buf+sizeof(buf) - 1; u++)
    {
        if (*u == Settings.decimal_mark)
        {
            *b++ = '.';
        }
        else if (utf8_codepoint(u) == Settings.exponent_mark)
        {
            *b++ = 'E';
            u = utf8_next(u) - 1;
        }
        else
        {
            *b++ = *u;
        }
    }
    *b++ = 0;

    // Create the number
    p.end = s - source;
    p.out = rt.make<decimal32>(ID_decimal32, buf);

    return OK;
}

// Max number of characters written by BID32
// 1 sign
// 34 digits
// 1 exponent delimiter
// 1 exponent sign
// 4 exponent
// 1 decimal separator
// Total 42
// Intermediate spacing could double the mantissa
// Spacing can be unicode, only 3 byte encoding for now, so 4 bytes per digit
#define MAXBIDCHAR 256

// Trick to only put the decimal_format function inside decimal32.cc
#if 32 == 64 + 64                      // Check if we are in decimal32.cc

size_t decimal_format(char *buf, size_t len, bool editing, bool raw)
// ----------------------------------------------------------------------------
//   Format the number according to our preferences
// ----------------------------------------------------------------------------
//   The decimal32 library has a very peculiar way to emit text:
//   it always uses scientific notation, and the mantissa is integral.
//   For example, 123.45 is emitted as 12345E-2.
//   However, it seems to carefully avoid exponent 0 for some reason,
//   so 123 is emitted as 1230E-1, whereas 12,3 is emitted as 123E-1.
//
//   I used to have code lifted from the DM42 SDK here, but it was hard
//   for me to comprehend and maintain, and I could not get it to do what
//   I wanted for SCI and FIX modes.
{
    // First make a copy of zany original input
    char copy[MAXBIDCHAR];
    strncpy(copy, buf, sizeof(copy)-1);

    // Read settings
    const settings &display = Settings;
    auto            mode    = editing ? display.NORMAL : display.display_mode;
    int             digits  = editing ? BID32_MAXDIGITS : display.displayed;
    int             std_exp = display.standard_exp;
    bool            showdec = display.show_decimal;
    unicode         space   = display.space;
    uint            mant_spc = display.spacing_mantissa;
    uint            frac_spc = display.spacing_fraction;
    bool            fancy   = !editing && display.fancy_exponent;
    char            decimal = display.decimal_mark; // Can be '.' or ','

    if (raw)
    {
        mode = display.NORMAL;
        digits = BID32_MAXDIGITS;
        std_exp = 9;
        showdec = true;
        space = 0;
        mant_spc = 0;
        frac_spc = 0;
        fancy = false;
        decimal = '.';
    }

    static uint16_t fancy_digit[10] =
    {
        L'⁰', L'¹', L'²', L'³', L'⁴',
        L'⁵', L'⁶', L'⁷', L'⁸', L'⁹'
    };

    bool overflow = false;
    do
    {
        char *in = copy;
        char *out = buf;
        char *expptr = strchr(in, 'E');
        if (!expptr)
        {
            // If there is no exponent, it's most likely a special number
            // like an infinity or a NaN
            if (strncasecmp(in, "+inf", sizeof("+inf") - 1) == 0)
                strncpy(out, "∞", len);
            else if (strncasecmp(in, "-inf", sizeof("-inf") - 1) == 0)
                strncpy(out, "-∞", len);

            // Otherwise, nothing to do, the buffer already is what we need
            return strlen(out);
        }

        // The first character is always + or -. Skip the '+'
        char sign = *in++;
        bool negative = sign == '-';

        // The exponent as given to us by the BID library
        int bidexp = atoi(expptr + 1);

        // Mantissa exponent, i.e. number of digits in mantissa (+1234E-1 -> 4)
        int mexp = expptr - in - 1;

        // Actual exponent is the sum of the two, e.g. 1234E-1 is 1.234E3
        int realexp = bidexp + mexp;

        // BID curiously emits 123.0 as 1230E-1, not even in a consistent way
        // (apparently, parsing "1." gives +1E+0, parsing "1.0" gives +10E-1...,
        // all the way to "1.000" giving "1000E-4"...).
        // This leads us to emit a useless trailing 0. Keep the 0 only for 0.0
        char *last = expptr;
        while (last > copy + 2 && last[-1] == '0')
        {
            last--;
            mexp--;
            bidexp++;
        }

        // BID generates things like 0E-12 as a result of some computations.
        // Turn that into a true 0
        if (last == copy + 2 && copy[1] == '0')
        {
            bidexp = realexp = 0;
            negative = false;
            sign = '+';
        }

        // Generate sign
        if (negative)
            out++;                  // Keep sign in copy
        else if (sign != '+')       // Defensive coding in case + is not present
            in--;

        // Position where we will emit the decimal dot when there is an exponent
        int decpos = 1;

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
        // is more useful.
        // Also, since DB48X can compute on 34 digits, and counting zeroes
        // can be annoying, there is a separate setting for when to switch
        // to scientific notation.
        bool hasexp = mode >= settings::display::SCI;
        if (!hasexp)
        {
            if (realexp < 0)
            {
                int minexp = digits < std_exp ? digits : std_exp;
                hasexp = mexp - realexp - 1 >= minexp;
            }
            else
            {
                hasexp = realexp >= std_exp;
                if (!hasexp)
                    decpos = realexp + 1;
            }
        }

        // Position where we emit spacing
        uint sep = mant_spc ? (~mant_spc - decpos) % mant_spc : 0;

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
            *out++ = '0';
            decpos--;               // Don't emit the decimal separator twice

            // Emit decimal dot and leading zeros on fractional part
            *out++ = decimal;
            sep = 0;
            for (int zeroes = realexp + 1; zeroes < 0; zeroes++)
            {
                *out++ = '0';
                if (++sep == frac_spc)
                {
                    out += utf8_encode(space, (byte *) out);
                    sep = 0;
                }
                decimals--;
            }
        }

        // Adjust exponent being displayed for engineering mode
        int dispexp = realexp;
        bool engmode = mode == display.ENG;
        if (engmode)
        {
            int offset = dispexp >= 0 ? dispexp % 3 : (dispexp - 2) % 3 + 2;
            decpos += offset;
            dispexp -= offset;
            sep += offset;
            decimals += 1;
        }

        // Copy significant digits, inserting decimal separator when needed
        bool sigmode = mode == display.NORMAL;
        char *lastnz = out;
        while (in < last && decimals > 0)
        {
            *out++ = *in++;
            decpos--;

            // Check if we will need to eliminate trailing zeros
            if (decpos >= 0 || out[-1] != '0')
                lastnz = out;

            // Insert spacing on the left of the decimal mark
            bool more = in < last || !sigmode || decpos > 0;
            uint limit = !more      ? 0
                       : decpos > 0 ? mant_spc
                       : decpos < 0 ? frac_spc
                                    : 0;
            if (++sep == limit && more && decimals > 1)
            {
                out += utf8_encode(space, (byte *) out);
                if (decpos > 0)
                    lastnz = out;
                sep = 0;
            }

            if (decpos == 0 && (more || showdec))
            {
                *out++ = decimal;
                lastnz = out;
                sep = 0;
            }

            // Count decimals after decimal separator, except in SIG mode
            // where we count all significant digits being displayed
            if (decpos < 0 || sigmode || engmode)
                decimals--;
        }

        // Check if we need some rounding on what is being displayed
        if (in < last && *in >= '5')
        {
            char *rptr = out;
            bool rounding = true;
            bool stripzeros = mode == display.NORMAL;
            while (rounding && --rptr >= buf)
            {
                if (*rptr >= '0' && *rptr <= '9')   // Do not convert '.' or '-'
                {
                    *rptr += 1;
                    rounding = *rptr > '9';
                    if (rounding)
                    {
                        *rptr -= 10;
                        if (stripzeros && *rptr == '0' && rptr != buf)
                        {
                            out--;
                            decimals++;
                            decpos++;
                            if (--sep == 0)
                                sep = decpos > 0 ? mant_spc - 1 : frac_spc - 1;
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
                }
                else if (stripzeros) // Inserted separator
                {
                    out--;
                    sep = decpos > 0 ? mant_spc - 1 : frac_spc - 1;
                }
            }

            // If we ran past the first digit, we overflowed during rounding
            // Need to re-run with the next larger exponent
            // This can only occur with a conversion of 9.9999 to 1
            if (rounding)
            {
                overflow = true;
                snprintf(copy, sizeof(copy),
                         "%c1E%d",
                         negative ? '-' : '+',
                         realexp + 1);
                continue;
            }

            // Check if we need to reinsert the last separator
            uint limit = decpos > 0 ? mant_spc : decpos < 0 ? frac_spc : 0;
            if (limit && ++sep == limit && decimals > 1)
            {
                out += utf8_encode(space, (byte *) out);
                sep = 0;
            }
        }

        // Return to position of last inserted zero
        else if (mode == display.NORMAL && out > lastnz)
        {
            out = lastnz;
            in = last;
        }


        // Do not add trailing zeroes in standard mode
        if (sigmode)
        {
            if (decpos > 0)
                decimals = decpos;
            else
                decimals = 0;
        }
        else if (mode == display.FIX && decpos > 0)
        {
            decimals = digits + decpos;
        }

        // Add trailing zeroes if necessary
        while (decimals > 0)
        {
            uint limit = decpos > 0 ? mant_spc : decpos < 0 ? frac_spc : 0;
            *out++ = '0';
            decpos--;

            if (++sep == limit && decimals > 1)
            {
                out += utf8_encode(space, (byte *) out);
                sep = 0;
            }

            if (decpos == 0 && showdec)
                *out++ = decimal;
            decimals--;
        }

        // Add exponent if necessary
        if (hasexp)
        {
            size_t sz = utf8_encode(display.exponent_mark, (byte *) out);
            out += sz;
            size_t remaining = buf + MAXBIDCHAR - out;
            if (fancy)
            {
                char expbuf[8];
                size_t written = snprintf(expbuf, 8, "%d", dispexp);
                for (uint e = 0; e < written; e++)
                {
                    char c = expbuf[e];
                    unicode u = c == '-' ? L'⁻' : fancy_digit[c - '0'];
                    out += utf8_encode(u, (byte *) out);
                }
            }
            else
            {
                size_t written = snprintf(out, remaining, "%d", dispexp);
                out += written;
            }
        }
        *out = 0;
        return out - buf;
    } while (overflow);
    return 0;
}
#endif // In original decimal32.cc


RENDER_BODY(decimal32)
// ----------------------------------------------------------------------------
//   Render the decimal32 into the given string buffer
// ----------------------------------------------------------------------------
{
    // Align the value
    bid32 num = o->value();

    // Render in a separate buffer to avoid overflows
    char buf[MAXBIDCHAR];
    bid32_to_string(buf, &num.value);
    record(decimal32, "Render raw output [%s] %+s",
           buf, r.editing() ? "editing" : "normal");

    size_t sz = decimal_format(buf, sizeof(buf), !r.stack(), r.file_save());
    record(decimal32, "Render formatted output [%s]", buf);

    // And return it to the caller
    return r.put(buf, sz) ? sz : 0;
}


RECORDER(debug32, 16, "Debug32 fractions");

algebraic_p decimal32::to_fraction(uint count, uint decimals) const
// ----------------------------------------------------------------------------
//   Convert a decimal value to a fraction
// ----------------------------------------------------------------------------
{
    int res;
    BID_UINT32 next, whole_part, decimal_part, one;
    BID_UINT32 v1num, v1den, v2num, v2den, t, s;

    bid32 num = value();
    bool neg = is_negative(&num.value);
    if (neg)
    {
        bid32_negate(&t, &num.value);
        num.value = t;
    }

    bid32_round_integral_zero(&whole_part, &num.value);
    bid32_sub(&decimal_part, &num.value, &whole_part);
    uint uone = 1;
    bid32_from_uint32(&one, &uone);
    v1num = whole_part;
    v1den = one;
    v2num = one;
    uint uzero = 0;
    bid32_from_uint32(&v2den, &uzero);

    if (decimals > BID32_MAXDIGITS - 3)
        decimals = BID32_MAXDIGITS - 3;

    while (count--)
    {
        // Check if the decimal part is small enough
        bid32_isZero(&res, &decimal_part);
        if (res)
            break;
        bid32_ilogb(&res, &decimal_part);
        if (-res > int(decimals))
            break;

        bid32_div(&next, &one, &decimal_part);
        bid32_round_integral_zero(&whole_part, &next);

        s = v1num;
        bid32_mul(&t, &whole_part, &v1num);
        bid32_add(&v1num, &t, &v2num);
        v2num = s;

        s = v1den;
        bid32_mul(&t, &whole_part, &v1den);
        bid32_add(&v1den, &t, &v2den);
        v2den = s;

        bid32_sub(&decimal_part, &next, &whole_part);
    }

    ularge numerator, denominator;
    bid32_to_uint64_floor(&numerator, &v1num);
    bid32_to_uint64_floor(&denominator, &v1den);
    algebraic_g result;
    if (denominator != 1)
        result = fraction::make(integer::make(numerator),
                                integer::make(denominator)).Safe();
    else
        result = integer::make(numerator);
    if (neg)
        result = -result;
    return result.Safe();
    }



// ============================================================================
//
//   Arithmetic wrappers
//
// ============================================================================
//   Define mod and rem in a way that matches mathematical definition

void bid32_mod(BID_UINT32 *pres, BID_UINT32 *px, BID_UINT32 *py)
// ----------------------------------------------------------------------------
//   The fmod function is really a remainder, adjust it for negative input
// ----------------------------------------------------------------------------
{
    int zero = 0;
    bid32_fmod(pres, px, py);
    bid32_isZero(&zero, pres);
    if (!zero)
    {
        bool xneg = decimal32::is_negative(px);
        bool yneg = decimal32::is_negative(py);
        if (xneg != yneg)
        {
            BID_UINT32 tmp = *pres;
            bid32_add(pres, &tmp, py);
        }
    }
}


void bid32_rem(BID_UINT32 *pres, BID_UINT32 *px, BID_UINT32 *py)
// ----------------------------------------------------------------------------
//   The fmod function is really a remainder, use it as is
// ----------------------------------------------------------------------------
{
    bid32_fmod(pres, px, py);
}
