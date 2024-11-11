// ****************************************************************************
//  hwfp.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Support code for hardware floating-point support
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "hwfp.h"

#include "arithmetic.h"
#include "parser.h"
#include "settings.h"

#include <cmath>

size_t hwfp_base::render(renderer &r, double x)
// ----------------------------------------------------------------------------
//   Render the value, ignoring formatting for now
// ----------------------------------------------------------------------------
{
    if (std::isfinite(x))
    {
        decimal_g dec = decimal::from(x);
        if (dec)
            return dec->render(r);
    }
    if (std::isinf(x))
    {
        r.put(x < 0 ? "-∞" : "∞");
    }
    else
    {
        r.put("NaN");
    }
    return r.size();
}


PARSE_BODY(hwfp_base)
// ----------------------------------------------------------------------------
//   Parse a floating point value if this is configured
// ----------------------------------------------------------------------------
{
    // Check if hardware floating point is enabled
    if (!Settings.HardwareFloatingPoint())
        return SKIP;

    // If hardware FP is enabled, we need to have low-enough precision
    uint prec = Settings.Precision();
    if (prec > 16)
        return SKIP;

    // Check if we use double or float
    gcutf8   source = p.source;
    gcutf8   s      = source;
    gcutf8   last   = source + p.length;
    scribble scr;

    // Skip leading sign
    if (*s == '+' || *s == '-')
    {
        // In an equation, `1 + 3` should interpret `+` as an infix
        if (p.precedence < 0)
            return SKIP;
        byte *p = rt.allocate(1);
        *p = *s;
        ++s;
    }

    // Scan digits and decimal dot
    int    decimalDot = -1;
    size_t digits     = 0;
    while (+s < +last)
    {
        byte c = *s;
        bool digit = c >= '0' && c <= '9';
        digits += digit ? 1 : 0;
        if (!(digit || (decimalDot < 0 && (c == '.' || c == ','))))
            break;

        byte *p = rt.allocate(1);
        if (!p)
            return ERROR;
        if (c == ',')
            c = '.';
        *p = c;
        ++s;
    }
    if (!digits)
        return SKIP;

    // Check how many digits were given
    if (Settings.TooManyDigitsErrors() && digits > prec)
    {
        rt.mantissa_error().source(source, +s - +source);
        return ERROR;
    }

    // Check if we were given an exponent
    if (+s < +last)
    {
        unicode cp = utf8_codepoint(s);
        if (cp == 'e' || cp == 'E' || cp == Settings.ExponentSeparator())
        {
            s = utf8_next(s);
            byte *p = rt.allocate(1);
            if (!p)
                return ERROR;
            *p = 'e';
            if (*s == '+' || *s == '-')
            {
                byte *p = rt.allocate(1);
                if (!p)
                    return ERROR;
                *p = *s;
                ++s;
            }
            utf8 expstart = s;
            while (+s < +last && (*s >= '0' && *s <= '9'))
            {
                byte *p = rt.allocate(1);
                if (!p)
                    return ERROR;
                *p = *s;
                ++s;
            }

            if (s == expstart)
            {
                rt.exponent_error().source(s);
                return ERROR;
            }
        }
    }

    // Add trailing zero
    byte *bp = rt.allocate(1);
    if (!bp)
        return ERROR;
    *bp = 0;

    // Convert to floating point
    cstring src = cstring(scr.scratch());
    p.length = +s - +source;
    if (prec > 7)
    {
        double fp = std::strtod(src, nullptr);
        p.out = hwdouble::make(fp);

    }
    else
    {
        float fp = std::strtof(src, nullptr);
        p.out = hwfloat::make(fp);
    }

    return p.out ? OK : ERROR;
}


template <typename hw>
algebraic_p hwfp<hw>::to_fraction(uint count, uint prec) const
// ----------------------------------------------------------------------------
//   Convert hwfp number to fraction
// ----------------------------------------------------------------------------
{
    hw   num = value();
    bool neg = num < 0;
    if (neg)
        num = -num;

    hw whole_part   = std::floor(num);
    hw decimal_part = num - whole_part;
    if (decimal_part == 0.0)
        return to_integer();

    hw   v1num  = whole_part;
    hw   v1den  = 1.0;
    hw   v2num  = 1.0;
    hw   v2den  = 0.0;

    uint maxdec = Settings.Precision() - 3;
    if (prec > maxdec)
        prec = maxdec;
    hw eps = std::exp(-prec * M_LN10);

    while (count--)
    {
        // Check if the decimal part is small enough
        if (decimal_part == 0.0)
            break;

        if (decimal_part < eps)
            break;

        hw next = 1.0 / decimal_part;
        whole_part = std::floor(next);

        hw s = v1num;
        v1num = whole_part * v1num + v2num;
        v2num = s;

        s = v1den;
        v1den = whole_part * v1den + v2den;
        v2den = s;

        decimal_part = next - whole_part;
    }

    ularge      numerator   = ularge(v1num);
    ularge      denominator = ularge(v1den);
    algebraic_g result;
    if (denominator == 1)
        result = +integer::make(numerator);
    else
        result = +fraction::make(integer::make(numerator),
                                 integer::make(denominator));
    if (neg)
        result = -result;
    return +result;

}


template algebraic_p hwfp<float>::to_fraction(uint count, uint prec) const;
template algebraic_p hwfp<double>::to_fraction(uint count, uint prec) const;
