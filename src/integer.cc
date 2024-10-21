// ****************************************************************************
//  integer.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of basic integer operations
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

#include "integer.h"

#include "arithmetic.h"
#include "bignum.h"
#include "fraction.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "utf8.h"

#include <stdio.h>


RECORDER(integer, 16, "Integers");

SIZE_BODY(integer)
// ----------------------------------------------------------------------------
//   Compute size for all integers
// ----------------------------------------------------------------------------
{
    byte_p p = o->payload();
    return ptrdiff(p, o) + leb128size(p);
}


HELP_BODY(integer)
// ----------------------------------------------------------------------------
//   Help topic for integers
// ----------------------------------------------------------------------------
{
    return utf8("Integers");
}


PARSE_BODY(integer)
// ----------------------------------------------------------------------------
//    Try to parse this as an integer
// ----------------------------------------------------------------------------
//    For simplicity, this deals with all kinds of integers, including bignum
{
    int        base        = 10;
    id         type        = ID_integer;
    const byte NODIGIT     = (byte) -1;
    size_t     is_fraction = 0;
    uint       is_dms      = 0;
    size_t     dms_end     = 0;
    object_g   number      = nullptr;
    object_g   numerator   = nullptr;

    record(integer, "Parsing [%s]", (utf8) p.source);

    // Array of values for digits
    static byte value[256] = { 0 };
    if (!value[(byte) 'A'])
    {
        // Initialize value array on first use
        for (int c = 0; c < 256; c++)
            value[c] = NODIGIT;
        for (int c = '0'; c <= '9'; c++)
            value[c] = c - '0';
        for (int c = 'A'; c <= 'Z'; c++)
            value[c] = c - 'A' + 10;
        for (int c = 'a'; c <= 'z'; c++)
            value[c] = c - 'a' + 10;
    }

    byte_p s      = (byte_p) p.source;
    size_t length = p.length;
    byte_p last   = s + length;
    byte_p endp   = nullptr;

    if (*s == '-')
    {
        // In an equation, '1+3' should interpret '+' as an infix command
        if (p.precedence < 0)
            return SKIP;
        type = ID_neg_integer;
        s++;
    }
    else if (*s == '+')
    {
        if (p.precedence < 0)
            return SKIP;
        s++;
    }
    else if (*s == '#')
    {
        s++;
        for (byte_p e = s; !endp; e++)
            if (e >= last || (value[*e] == NODIGIT && *e != '#'))
                endp = e;

        if (endp > s)
        {
            // The HP syntax takes A-F as digits, and b/d as bases
            // Prefer to accept B and D suffixes, but only if no
            // digit above was found in the base
            base     = Settings.Base();
            type     = ID_based_integer;

            uint max = 0;
            for (byte_p e = s; e < endp - 1; e++)
                if (max < value[*e])
                    max = value[*e];

            switch (endp[-1])
            {
            case 'b':
#ifdef CONFIG_UPPERCASE_BASE_SUFFIXES
            case 'B':
#endif // CONFIG_UPPERCASE_BASE_SUFFIXES
                if (max < 2)
                {
                    base = 2;
                    endp--;
#if CONFIG_FIXED_BASED_OBJECTS
                    type = ID_bin_integer;
#endif // CONFIG_FIXED_BASED_OBJECTS

                }
                else
                {
                    endp = nullptr;
                }
                break;
            case 'o':
#ifdef CONFIG_UPPERCASE_BASE_SUFFIXES
            case 'O':
#endif // CONFIG_UPPERCASE_BASE_SUFFIXES
                base = 8;
                endp--;
#if CONFIG_FIXED_BASED_OBJECTS
                type = ID_oct_integer;
#endif // CONFIG_FIXED_BASED_OBJECTS
                break;
            case 'd':
#ifdef CONFIG_UPPERCASE_BASE_SUFFIXES
            case 'D':
#endif // CONFIG_UPPERCASE_BASE_SUFFIXES
                if (max < 10)
                {
                    base = 10;
                    endp--;
#if CONFIG_FIXED_BASED_OBJECTS
                    type = ID_dec_integer;
#endif // CONFIG_FIXED_BASED_OBJECTS
                }
                else
                {
                    endp = nullptr;
                }
                break;
            case 'h':
#ifdef CONFIG_UPPERCASE_BASE_SUFFIXES
            case 'H':
#endif // CONFIG_UPPERCASE_BASE_SUFFIXES
                base = 16;
                endp--;
#if CONFIG_FIXED_BASED_OBJECTS
                type = ID_hex_integer;
#endif // CONFIG_FIXED_BASED_OBJECTS
                break;
            default:
                // Use current default base
                endp = nullptr;
                break;
            }
            if (endp && s >= endp)
            {
                rt.based_number_error().source(s);
                return ERROR;
            }
        }
    }

    // If this is a + or - operator, skip
    if (s >= last || value[*s] >= base)
        return SKIP;

    do
    {
        // Loop on digits
        ularge result = 0;
        bool   big    = false;
        size_t digits = 0;
        byte   v;
        unicode sep = Settings.NumberSeparator();

        if (is_fraction && value[*s] == NODIGIT)
        {
            // This can be something like `1/(1+x)`
            number = numerator;
            s = +p.source + is_fraction;
            break;
        }

        while (!endp || s < endp)
        {
            unicode cp = utf8_codepoint(s);

            // Check new syntax for based numbers
            if (cp == '#')
            {
                if (result < 2 || result > 36)
                {
                    rt.invalid_base_error().source(s);
                    return ERROR;
                }
                base = result;
                result = 0;
                type = ID_based_integer;
                sep = Settings.BasedSeparator();
                s++;
                continue;
            }
            if (cp == sep)
            {
                s = utf8_next(s);
                continue;
            }

            v = value[*s++];
            if (v == NODIGIT)
                break;

            if (v >= base)
            {
                object::result err = ERROR;
                if (type == ID_integer || type == ID_neg_integer)
                {
                    if (v == 0xE) // Exponent
                        return SKIP;
                    else
                        break;
                }
                rt.based_digit_error().source(s - 1);
                return err;
            }
            ularge next = result * base + v;
            record(integer,
                   "Digit %c value %u value=%llu next=%llu",
                   s[-1],
                   v,
                   result,
                   next);
            digits++;

            // If the value does not fit in an integer, defer to bignum / real
            big = next / base != result;
            if (big)
                break;

            result = next;
        }

        // Exit quickly if we had no digits
        if (!digits)
        {
            if (is_fraction)
                // Something like `2/s`, we parsed 2 successfully
                s = p.source + is_fraction;
            if (is_dms)
                // Unterminated DMS number, e.g. 1°3
                s = p.source + dms_end;

            // Parsed no digit: try something else
            if (!is_fraction && !is_dms)
                return WARN;

            // Check incomplete fractions or DMS
            if (is_fraction || is_dms)
            {
                number = numerator;
                if (is_dms < 1)
                    break;
                else
                    s++;
            }

        }

        // Check if we need bignum
        bignum_g bresult = nullptr;
        if (big)
        {
            // We may cause garbage collection in bignum arithmetic
            gcbytes gs    = s;
            gcbytes ge    = endp;
            size_t  count = endp - s;

            switch (type)
            {
            case ID_integer:       type = ID_bignum; break;
            case ID_neg_integer:   type = ID_neg_bignum; break;
#if CONFIG_FIXED_BASED_OBJECTS
            case ID_hex_integer:   type = ID_hex_bignum; break;
            case ID_dec_integer:   type = ID_dec_bignum; break;
            case ID_oct_integer:   type = ID_oct_bignum; break;
            case ID_bin_integer:   type = ID_bin_bignum; break;
#endif // CONFIG_FIXED_BASED_OBJECTS
            case ID_based_integer: type = ID_based_bignum; break;
            default: break;
            }

            // Integrate last digit that overflowed above
            bignum_g bbase  = rt.make<bignum>(ID_bignum, base);
            bignum_g bvalue = rt.make<bignum>(type, v);
            bresult         = rt.make<bignum>(type, result);
            bresult = bvalue + bbase * bresult; // Order matters for types

            while (count--)
            {
                v = value[*gs];
                ++gs;
                if (v == NODIGIT)
                    break;

                if (v >= base)
                {
                    object::result err = ERROR;
                    if (type == ID_bignum || type == ID_neg_bignum)
                    {
                        if (v == 0xE) // Exponent, switch to decimal
                            err = WARN;
                        else
                            break;
                    }
                    rt.based_digit_error().source(s - 1);
                    return err;
                }
                record(integer, "Digit %c value %u in bignum", s[-1], v);
                bvalue  = rt.make<bignum>(type, v);
                bresult = bvalue + bbase * bresult;
            }

            s    = gs;
            endp = ge;
        }


        // Skip base if one was given, else point at char that got us out
        if (endp && s == endp)
            s++;
        else
            s--;

        // Create the intermediate result, which may GC
        {
            gcutf8 gs = s;
            number = big ? object_p(bresult) : rt.make<integer>(type, result);
            s = gs;
        }
        if (!number)
            return ERROR;

        // Check if we parse a DMS fraction
        if (is_real(type) && !p.separator && (s < last || is_dms))
        {
            if (s < last)
            {
                unicode cp = utf8_codepoint(s);
                uint want_dms = 0;
                switch(cp)
                {
                case L'°':  want_dms = 1;           break;
                case L'′':  want_dms = 2;           break;
                case L'″':  want_dms = 3;           break;
                default:                            break;
                }
                if (want_dms)
                {
                    if (is_dms != want_dms - 1)
                    {
                        rt.syntax_error().source(s);
                        return ERROR;
                    }
                    s = utf8_next(s);
                    is_dms = want_dms;
                }
                else if (is_dms)
                {
                    is_dms++;
                }
            }
            else
            {
                is_dms++;
            }

            if (is_dms)
            {
                dms_end = s - +p.source;
                if (is_dms == 1)
                {
                    numerator   = number;
                    number      = nullptr;
                    type        = ID_integer;
                }
                else
                {
                    gcutf8      gs       = s;
                    algebraic_g existing = algebraic_p(+numerator);
                    algebraic_g current  = algebraic_p(+number);
                    uint        div      = is_dms == 2 ? 60 : 3600;
                    algebraic_g scale    = +fraction::make(integer::make(1),
                                                           integer::make(div));
                    existing = existing + current * scale;

                    // If we are at the end, check if there is a fraction
                    if (is_dms == 3)
                    {
                        size_t hasfrac = false;
                        s = gs;
                        last = p.source + length;
                        utf8   p;
                        for (p = s; p < last; p++)
                        {
                            if (*p == '/')
                                hasfrac = true;
                            else if (*p < '0' || *p > '9')
                                break;
                        }
                        if (hasfrac)
                        {
                            size_t   sz   = p - s;
                            object_p frac = object::parse(s, sz);
                            if (!frac || !frac->is_fraction())
                            {
                                if (!rt.error())
                                    rt.syntax_error().source(+gs);
                                return ERROR;
                            }
                            current = algebraic_p(frac);
                            existing = existing + current * scale;
                            gs += sz;
                        }
                        else if (p > s)
                        {
                            // Something like 1°2'3"4_dms is not valid
                            if (!rt.error())
                                rt.syntax_error().source(+gs);
                            return ERROR;
                        }

                        is_dms = 0;
                        is_fraction = 0;
                        number = +existing;
                    }
                    numerator = +existing;
                    s = gs;
                }
            }
        }

        // Check if we parse a fraction
        if (is_fraction)
        {
            if (integer_p(object_p(number))->is_zero())
            {
                rt.zero_divide_error().source(p.source + (is_fraction + 1));
                return ERROR;
            }
            else if (numerator->is_bignum() || number->is_bignum())
            {
                // We rely here on the fact that an integer can also be read as
                // a bignum (they share the same payload format)
                bignum_g n = (bignum *) (object_p) numerator;
                bignum_g d = (bignum *) (object_p) number;
                number     = (object_p) big_fraction::make(n, d);
            }
            else
            {
                // We rely here on the fact that an integer can also be read as
                // a bignum (they share the same payload format)
                integer_g n = (integer *) (object_p) numerator;
                integer_g d = (integer *) (object_p) number;
                number      = (object_p) fraction::make(n, d);
            }
            is_fraction = false;
        }
        else if (*s == '/' && p.precedence <= MULTIPLICATIVE && is_real(type))
        {
            is_fraction = s - +p.source;
            numerator   = number;
            number      = nullptr;
            type        = ID_integer;
            s++;
        }
    } while (is_fraction || is_dms);

    // Check if we finish with something indicative of a fraction or real number
    if (!endp)
    {
        if (*s == '.' || *s == ',' ||
            utf8_codepoint(s) == Settings.ExponentSeparator())
            return SKIP;
    }

    // Record output
    p.length = (utf8) s - (utf8) p.source;
    p.out = number;

    return OK;
}


static size_t render_num(renderer &r,
                         integer_p num,
                         uint      base,
                         cstring   fmt)
// ----------------------------------------------------------------------------
//   Convert an integer value to the proper format
// ----------------------------------------------------------------------------
//   This is necessary because the arm-none-eabi-gcc printf can't do 64-bit
//   I'm getting non-sensible output
{
    // If we render to a file, need to first render to scratchpad to be able to
    // revert the digits in memory before writing
    if (r.file_save())
    {
        renderer tmp(r.expression(), r.editing(), r.stack());
        size_t result = render_num(tmp, num, base, fmt);
        r.put(tmp.text(), result);
        return result;
    }

    // Upper / lower rendering
    bool upper = *fmt == '^';
    bool lower = *fmt == 'v';
    if (upper || lower)
        fmt++;
    if (!Settings.SmallFractions() || r.editing())
        upper = lower = false;
    static uint16_t fancy_upper_digits[10] =
    {
        L'⁰', L'¹', L'²', L'³', L'⁴',
        L'⁵', L'⁶', L'⁷', L'⁸', L'⁹'
    };
    static uint16_t fancy_lower_digits[10] =
    {
        L'₀', L'₁', L'₂', L'₃', L'₄',
        L'₅', L'₆', L'₇', L'₈', L'₉'
    };

    // Check which kind of spacing to use
    bool based = *fmt == '#';
    bool fancy_base = based && r.stack();
    uint spacing = based ? Settings.BasedSpacing() : Settings.MantissaSpacing();
    unicode space = based ? Settings.BasedSeparator() : Settings.NumberSeparator();

    // Copy the '#' or '-' sign
    if (*fmt)
        r.put(*fmt++);
    else
        r.flush();

    // Get denominator for the base
    size_t findex = r.size();
    ularge n      = num->value<ularge>();

    // Keep dividing by the base until we get 0
    uint sep = 0;
    do
    {
        ularge digit = n % base;
        n /= base;
        unicode c = upper        ? fancy_upper_digits[digit]
                  : lower        ? fancy_lower_digits[digit]
                  : (digit < 10) ? digit + '0'
                                 : digit + ('A' - 10);
        r.put(c);

        if (n && ++sep == spacing)
        {
            sep = 0;
            r.put(space);
        }
    } while (n);

    // Revert the digits
    byte *dest  = (byte *) r.text();
    bool multibyte = upper || lower || (spacing && space > 0xFF);
    utf8_reverse(dest + findex, dest + r.size(), multibyte);

    // Add suffix
    if (fancy_base)
    {
        if (base / 10)
            r.put(unicode(fancy_lower_digits[base/10]));
        r.put(unicode(fancy_lower_digits[base%10]));
    }
    else if (*fmt)
    {
        r.put(*fmt++);
    }

    return r.size();
}


RENDER_BODY(integer)
// ----------------------------------------------------------------------------
//   Render the integer into the given string buffer
// ----------------------------------------------------------------------------
{
    size_t result = render_num(r, o, 10, "");
    return result;
}


template <>
HELP_BODY(neg_integer)
// ------------------------------------------------------------------------
//   Help topic for negative integers
// ----------------------------------------------------------------------------
{
    return utf8("Integers");
}

template <>
RENDER_BODY(neg_integer)
// ----------------------------------------------------------------------------
//   Render the negative integer value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, o, 10, "-");
}


#if CONFIG_FIXED_BASED_OBJECTS
template <>
RENDER_BODY(hex_integer)
// ----------------------------------------------------------------------------
//   Render the hexadecimal integer value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, o, 16, "#h");
}

template <>
RENDER_BODY(dec_integer)
// ----------------------------------------------------------------------------
//   Render the decimal based number
// ----------------------------------------------------------------------------
{
    return render_num(r, o, 10, "#d");
}

template <>
RENDER_BODY(oct_integer)
// ----------------------------------------------------------------------------
//   Render the octal integer value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, o, 8, "#o");
}

template <>
RENDER_BODY(bin_integer)
// ----------------------------------------------------------------------------
//   Render the binary integer value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, o, 2, "#b");
}

template <>
HELP_BODY(hex_integer)
// ----------------------------------------------------------------------------
//   Help topic for based numbers
// ----------------------------------------------------------------------------
{
    return utf8("Based numbers");
}

template <>
HELP_BODY(oct_integer)
// ----------------------------------------------------------------------------
//   Help topic for based numbers
// ----------------------------------------------------------------------------
{
    return utf8("Based numbers");
}

template <>
HELP_BODY(dec_integer)
// ----------------------------------------------------------------------------
//   Help topic for based numbers
// ----------------------------------------------------------------------------
{
    return utf8("Based numbers");
}

template <>
HELP_BODY(bin_integer)
// ----------------------------------------------------------------------------
//   Help topic for based numbers
// ----------------------------------------------------------------------------
{
    return utf8("Based numbers");
}

#endif // CONFIG_FIXED_BASED_OBJECTS


template <>
RENDER_BODY(based_integer)
// ----------------------------------------------------------------------------
//   Render the based integer value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, o, Settings.Base(), "#");
}


template <>
HELP_BODY(based_integer)
// ----------------------------------------------------------------------------
//   Help topic for based numbers
// ----------------------------------------------------------------------------
{
    return utf8("Based numbers");
}


static size_t fraction_render(fraction_p o, renderer &r, bool negative)
// ----------------------------------------------------------------------------
//   Common code for positive and negative fractions
// ----------------------------------------------------------------------------
{
    integer_g n = o->numerator(1);
    integer_g d = o->denominator(1);
    if (negative)
        r.put('-');
    if (r.stack() && Settings.MixedFractions())
    {
        ularge nv = n->value<ularge>();
        ularge dv = d->value<ularge>();
        if (nv >= dv)
        {
            integer_g i = integer::make(nv / dv);
            render_num(r, i, 10, "");
            r.put(unicode(settings::SPACE_MEDIUM_MATH));
            n = integer::make(nv % dv);
        }
    }
    render_num(r, n, 10, "^");
    r.put('/');
    render_num(r, d, 10, "v");
    return r.size();
}


RENDER_BODY(fraction)
// ----------------------------------------------------------------------------
//   Render the fraction as 'num/den'
// ----------------------------------------------------------------------------
{
    return fraction_render(o, r, false);
}


RENDER_BODY(neg_fraction)
// ----------------------------------------------------------------------------
//   Render the fraction as '-num/den'
// ----------------------------------------------------------------------------
{
    return fraction_render(o, r, true);
}
