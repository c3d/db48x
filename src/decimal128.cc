// ****************************************************************************
//  decimal128.cc                                                 DB48X project
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

#include "decimal128.h"

#include "runtime.h"
#include "settings.h"

#include <bid_conf.h>
#include <bid_functions.h>
#include <cstdio>
#include <algorithm>
#include <cstdlib>

using std::min, std::max;


OBJECT_HANDLER_BODY(decimal128)
// ----------------------------------------------------------------------------
//    Handle commands for decimal128s
// ----------------------------------------------------------------------------
{
    switch(cmd)
    {
    case EVAL:
        // Decimal128 values evaluate as self
        rt.push(obj);
        return 0;
    case SIZE:
        return ptrdiff(payload, obj) + sizeof(bid128);
    case PARSE:
    {
        parser *p = (parser *) arg;
        return object_parser(p->begin, &p->end, &p->output, rt);
    }
    case RENDER:
    {
        renderer *r = (renderer *) arg;
        return obj->object_renderer(r->begin, r->end, rt);
    }

    default:
        // Check if anyone else knows how to deal with it
        return DELEGATE(object);
    }

}


OBJECT_PARSER_BODY(decimal128)
// ----------------------------------------------------------------------------
//    Try to parse this as an decimal128
// ----------------------------------------------------------------------------
{
    cstring p = begin;

    // Skip leading sign
    if (*p == '+' || *p == '-')
        p++;

    // Skip digits
    cstring digits = p;
    while (*p >= '0' && *p <= '9')
        p++;

    // If we had no digits, check for special names or exit
    if (p == digits)
    {
        if (strncasecmp(p, "infinity", sizeof("infinity") - 1) != 0 &&
            strncasecmp(p, "NaN",      sizeof("NaN")      - 1) != 0)
            return SKIP;
    }

    // Check decimal dot
    char *decimal = nullptr;
    if (*p == '.' || *p == ',')
    {
        decimal = (char *) p++;
        while (*p >= '0' && *p <= '9')
            p++;
    }

    // Check exponent
    char *exponent = nullptr;
    if (*p == 'e' || *p == 'E' || *p == Settings.exponentChar)
    {
        exponent = (char *) p++;
        if (*p == '+' || *p == '-')
            p++;
        cstring expval = p;
        while (*p >= '0' && *p <= '9')
            p++;
        if (p == expval)
        {
            rt.error("Malformed exponent");
            return ERROR;
        }
    }

    // Patch the input to the BID library
    char dot = '.';
    if (decimal)
    {
        dot = *decimal;
        *decimal = '.';
    }

    char exp = 'e';
    if (exponent)
    {
        exp = *exponent;
        *exponent = 'e';
    }

    // Parse the number
    if (end)
        *end = p;
    if (out)
        *out = rt.make<decimal128>(ID_decimal128, begin);

    // Restore the patched input
    if (decimal)
        *decimal = dot;
    if (exponent)
        *exponent = exp;

    return OK;
}


// Trick to only put the decimal_format function inside decimal128.cc
#if 128 == 64 + 64                      // Check if we are in decimal128.cc

#define MAX_NEG_FILL    4               // Limit before switching to SCI

static bool round_string(char *s, int digit, char rounding_digit)
// ----------------------------------------------------------------------------
//   Checks which digit was responsible for overflow
// ----------------------------------------------------------------------------

{
    if (rounding_digit + 5 <= '9')
        return 0;

    for (; digit >= 0; digit--)
    {
        if (s[digit] == '.')
            continue; // Skip decimal point
        s[digit]++;
        if (s[digit] <= '9')
            return false;
        s[digit] -= 10;
    }

    return 1;
}


void decimal_format(char *str, size_t len)
// ----------------------------------------------------------------------------
//   Format the number according to our preferences
// ----------------------------------------------------------------------------
//   This is largely inspired by the code in the DM42 SDKdemo
{
    // First make a copy of the raw output of the library
    char s[50];                 // Enough even for bid128
    strcpy(s, str);             // Make a local copy first

    int digits = Settings.displayed;
    settings::display mode = Settings.display_mode;
    for (;;)
    {
        char *ep = strchr(s, 'E');
        if (!ep)
        {
            // No exponent -> expecting special number like Inf or NaN
            // Just copy (we may have eliminated mantissa sign
            strcpy(str, s);
            return;
        }

        int   ms     = s[0] == '-'; // Mantissa negative
        int   mexp   = ep - s - 1;  // Mantissa exponent (e.g. 4 for +1234E+10)
        bool  hasexp = true;
        int   exp    = atoi(ep + 1) + mexp;

        // Terminate mantissa string
        char *mend   = ep - 1; // Mantissa end
        char *mant   = s + 1;  // Mantissa string

        // Ignore mantissa trailing zeros
        while (ep > mant && mend[0] == '0')
            mend--;
        *(++mend) = 0;

        // Check if we need to display an exponent ("scientific" mode)
        switch (mode)
        {
        case settings::display::FIX:
            if (exp <= 0 && Settings.displayed <= -exp)
                break;           // Zero fill bigger then mode digits

        case settings::display::NORMAL:
        {
            // Check if exponent is needed
            int sz = exp;
            if (exp >= (-MAX_NEG_FILL + 1))
            {
                // Check number requires a 0. and zero padding after decimal
                if (exp <= 0)
                    sz += 2 - exp + 1;
                if (ms)
                    sz++;                // One place for sign
                hasexp = sz > (int) Settings.displayed + 2;
            }
            break;
        }

        case settings::display::SCI:
        case settings::display::ENG:
            // Here, we always have an exponent
            break;
        }

        int digitsBeforePoint  = hasexp ? 1 : exp;
        int mantissaLen = strlen(s + 1);       // Available mantissa digits

        // Exponent correction for ENG mode
        exp--; // fix for digitsBeforePoint==1
        if (mode == settings::display::ENG)
        {
            // Lower the exponent to nearest multiple of 3
            int adjust = exp >= 0 ? exp % 3 : 2 + (exp - 2) % 3;
            exp -= adjust;
            digitsBeforePoint += adjust;
        }

        int zeroFillAfterDot = max(0, -digitsBeforePoint);

        // Prepare exponent
        int elen;
        if (hasexp)
        {
            // Do not interfere with mantissa end
            ep++;
            sprintf(ep, "%c%i", 'E', exp);
            elen = strlen(ep);
        }
        else
        {
            ep[0] = 0;
            elen  = 0;
        }

        //  // Frac digits available
        int frac = mantissaLen - digitsBeforePoint;

        // Add Mantissa
        char *p = str;
        if (ms)
            *p++ = '-';

        // Copy digits before point
        char *mp = s + 1;
        if (digitsBeforePoint > 0)
        {
            p = stpncpy(p, s + 1, digitsBeforePoint);
            mp += min(digitsBeforePoint, (int) strlen(s+1));
        }

        // Add trailing zeros in integer part
        for (int z = 0; z < -frac; z++)
            *p++ = '0';

        // Available space for fraction
        int avail = len - strlen(str) - elen;

        // Add fractional part
        digits = min(mode == settings::display::NORMAL ? frac : digits,
                     avail - 1 - (digitsBeforePoint > 0 ? 0 : 1));

        if (digits > 0)
        {
            // We have digits and have room for at least one frac digit
            p = stpcpy(p, digitsBeforePoint > 0 ? "." : "0.");
            frac = max(0, frac);
            for (int z = 0; z < zeroFillAfterDot; z++)
                *p++ = '0';
            digits -= zeroFillAfterDot;
            int msz = min(frac, digits);
            p = stpncpy(p, mp, msz);
            mp += msz;
            int trailingZeroes = max(digits + zeroFillAfterDot - frac, 0);
            for (int z = 0; z < trailingZeroes; z++)
                *p++ = '0';
        }

        if (*mp)
        {
            // More mantissa digits available -> rounding
            int rix  = mp - s;
            bool ovfl = round_string(str + ms, strlen(str + ms) - 1, s[rix]);
            if (ovfl)
            {
                // Retry -
                sprintf(s,
                        "%c1%c%c%i",
                        ms ? '-' : '+',
                        'E',
                        exp < 0 ? '-' : '+',
                        abs(exp + 1));
                continue;
            }
            if (mode == settings::display::NORMAL)
            {
                // Remove trailing zeros
                int ix = strlen(str) - 1;
                while (ix && str[ix] == '0')
                    ix--;
                if (str[ix] == '.')
                    ix--;
                str[ix + 1] = 0;
            }
        }

        // Add exponent
        p = strcpy(p, ep);
        break;
    }
}
#endif // In original decimal128.cc


OBJECT_RENDERER_BODY(decimal128)
// ----------------------------------------------------------------------------
//   Render the decimal128 into the given string buffer
// ----------------------------------------------------------------------------
{
    // Align the value
    bid128 num = value();

    // Render in a separate buffer to avoid overflows
    char buffer[50];            // bid128 with 34 digits takes at most 42 chars
    bid128_to_string(buffer, &num);
    decimal_format(buffer, sizeof(buffer));

    // Adjust special characters
    for (char *p = buffer; *p && p < buffer + sizeof(buffer); p++)
        if (*p == 'e' || *p == 'E')
            *p = Settings.exponentChar;
        else if (*p == '.')
            *p = Settings.decimalDot;

    // And return it to the caller
    return snprintf(begin, end - begin, "%s", buffer);
}
