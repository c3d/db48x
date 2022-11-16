// ****************************************************************************
//  bignum.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of basic bignum operations
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

#include "bignum.h"

#include "integer.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "utf8.h"

#include <stdio.h>


RECORDER(bignum, 16, "Bignums");

bignum::bignum(integer_p value, id type)
// ----------------------------------------------------------------------------
//   Create a bignum from an integer value
// ----------------------------------------------------------------------------
    : text(value->payload(), bytesize(value), type)
{
    byte *p = payload();
    size_t sz = leb128<size_t>(p);
    if (sz)
    {
        byte_p q = value->payload();
        uint c = 0;
        uint bits = 0;
        bool more;
        do
        {
            byte b = *q++;
            more = b & 0x80;
            c |= (b & 0x7F) << bits;
            bits += 7;
            if (bits >= 8)
            {
                *p++ = byte(c);
                c >>= 8;
                bits -= 8;
            }
        } while (more);
        if (c)
            *p++ = byte(c);
    }
}


size_t bignum::required_memory(id i, integer_p value)
// ----------------------------------------------------------------------------
//   Compute the size to copy an integer value
// ----------------------------------------------------------------------------
{
    size_t size = bytesize(value);
    return leb128size(i) + leb128size(size) + size;
}


OBJECT_HANDLER_BODY(bignum)
// ----------------------------------------------------------------------------
//    Handle commands for bignums
// ----------------------------------------------------------------------------
{
    record(bignum, "Command %+s on %p", name(op), obj);
    switch(op)
    {
    case EXEC:
    case EVAL:
        // Bignum values evaluate as self
        return rt.push(obj) ? OK : ERROR;
    case SIZE:
        return size(obj, payload);
    case PARSE:
        return object_parser(OBJECT_PARSER_ARG(), rt);
    case RENDER:
        return obj->object_renderer(OBJECT_RENDERER_ARG(), rt);
    case HELP:
        return (intptr_t) "bignum";

    default:
        // Check if anyone else knows how to deal with it
        return DELEGATE(text);
    }
}


OBJECT_PARSER_BODY(bignum)
// ----------------------------------------------------------------------------
//    Try to parse this as an bignum
// ----------------------------------------------------------------------------
//    For simplicity, this deals with all kinds of bignums
//    REVISIT
{
    int         base       = 10;
    id          type       = ID_bignum;
    const byte  NODIGIT    = (byte) -1;

    record(bignum, "Parsing [%s]", (utf8) p.source);

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

    utf8 s = (utf8) p.source;
    utf8 endp = (utf8) p.source + p.length;
    gcutf8 bp = nullptr;

    if (*s == '-')
    {
        type = ID_neg_bignum;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }
    else if (*s == '#')
    {
        s++;
        for (byte_p e = s; e < endp; e++)
        {
            if (value[*e] == NODIGIT)
            {
                endp = e;
                break;
            }
        }

        if (endp > s)
        {
            // The HP syntax takes A-F as digits, and b/d as bases (lowercase)
            // Prefer to accept B and D suffixes, but only if no
            // digit above the base was found in the mantissa
            base = Settings.base;

            uint max = 0;
            for (byte_p e = s; e < endp - 1; e++)
                if (max < value[*e])
                    max = value[*e];

            switch(endp[-1])
            {
            case 'b':
            case 'B':
                if (max < 2)
                    base = 2;
                else
                    endp++;
                break;
            case 'O':
            case 'o':
                base = 8;
                break;
            case 'd':
            case 'D':
                if (max < 10)
                    base = 10;
                else
                    endp++;
                break;
            case 'H':
            case 'h':
                base = 16;
                break;
            default:
                // Use current default base
                endp++;
                break;
            }
            switch(base)
            {
            case  2: type = ID_bin_bignum; break;
            case  8: type = ID_oct_bignum; break;
            case 10: type = ID_dec_bignum; break;
            case 16: type = ID_hex_bignum; break;
            }
            endp--;
            if (s >= endp)
            {
                rt.based_number_error(). source(s);
                return ERROR;
            }
            bp = endp;
        }
    }

    // If this is a + or - operator, skip
    if (s < endp && value[*s] >= base)
        return SKIP;

    // Check if end of number indicates a floating-point value
    for (byte_p e = s; e < endp; e++)
    {
        if (value[*e] == NODIGIT)
        {
            if (*e == Settings.decimalDot ||
                utf8_codepoint(e) == Settings.exponentChar)
                return SKIP;
            break;
        }
    }

    // Record info about where we are
    gcutf8 gs = s;                              // GC-safe pointer
    size_t count = endp - s;                    // Compute this before any GC
    bignum_g result = rt.make<bignum>(type, 0); // GC may occur here
    bignum_g bbase = bignum::make(base);        // ... or here
    byte v;

    // Loop on digits
    while (count--)
    {
        s = gs;                                 // Need GC-safe ptr here...
        v = value[*s++];
        gs = s;
        if (v == NODIGIT)
            break;
        if (v >= base)
        {
            rt.based_digit_error().source(s-1);
            return ERROR;
        }
        bignum_g bv = rt.make<bignum>(ID_bignum, &v, 1);
        result = bv + bbase * result;           // Order is important for type
    }

    // If we stopped with the base marker
    if (byte_p(gs) == byte_p(bp))
        gs = byte_p(gs) + 1;

    // Record output
    p.end = (utf8) gs - (utf8) p.source;
    p.out = object_p(result);

    return OK;
}


static size_t render_num(renderer &r,
                         bignum_p  num,
                         uint      base,
                         cstring   fmt)
// ----------------------------------------------------------------------------
//   Convert an bignum value to the proper format
// ----------------------------------------------------------------------------
//   This is necessary because the arm-none-eabi-gcc printf can't do 64-bit
//   I'm getting non-sensible output
{
    runtime &rt = runtime::RT;

    // Copy the '#' or '-' sign
    if (*fmt)
        r.put(*fmt++);

    // Get denominator for the base
    object::id ntype = num->type();
    size_t findex = r.size();
    bignum_g b = rt.make<bignum>(ntype, base);
    bignum_g n = (bignum *) num;

    // Keep dividing by the base until we get 0
    do
    {
        bignum_g remainder = nullptr;
        if (!bignum::quorem(n, b, bignum::ID_bignum, &n, &remainder))
            break;
        uint digit = remainder->value<uint>();
        char c = (digit < 10) ? digit + '0' : digit + ('A' - 10);
        r.put(c);
    } while (!n->zero());

    // Revert the digits
    char *dest = (char *) r.text();
    char *first = dest + findex;
    char *last = dest + r.size() - 1;
    while (first < last)
    {
        char tmp = *first;
        *first = *last;
        *last = tmp;
        last--;
        first++;
    }

    // Add suffix if there is one
    if (*fmt)
        r.put(*fmt++);

    // Return the number of items we need
    return r.size();
}


OBJECT_RENDERER_BODY(bignum)
// ----------------------------------------------------------------------------
//   Render the bignum into the given string buffer
// ----------------------------------------------------------------------------
{
    size_t result = render_num(r, this, 10, "");
    return result;
}


template<>
OBJECT_RENDERER_BODY(neg_bignum)
// ----------------------------------------------------------------------------
//   Render the negative bignum value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, this, 10, "-");
}


template<>
OBJECT_RENDERER_BODY(hex_bignum)
// ----------------------------------------------------------------------------
//   Render the hexadecimal bignum value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, this, 16, "#h");
}

template<>
OBJECT_RENDERER_BODY(dec_bignum)
// ----------------------------------------------------------------------------
//   Render the decimal based number
// ----------------------------------------------------------------------------
{
    return render_num(r, this, 10, "#d");
}

template<>
OBJECT_RENDERER_BODY(oct_bignum)
// ----------------------------------------------------------------------------
//   Render the octal bignum value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, this, 8, "#o");
}

template<>
OBJECT_RENDERER_BODY(bin_bignum)
// ----------------------------------------------------------------------------
//   Render the binary bignum value into the given string buffer
// ----------------------------------------------------------------------------
{
    return render_num(r, this, 2, "#b");
}



// ============================================================================
//
//    Big bignum comparisons
//
// ============================================================================

int bignum::compare(bignum_g xg, bignum_g yg)
// ----------------------------------------------------------------------------
//   Compare two bignum values
// ----------------------------------------------------------------------------
{
    id xt = xg->type();
    id yt = yg->type();

    // Negative bignums are always smaller than positive bignums
    if (xt == ID_neg_bignum && yt != ID_neg_bignum)
        return -1;
    else if (yt == ID_neg_bignum && xt != ID_neg_bignum)
        return 1;

    size_t xs = 0;
    size_t ys = 0;
    byte_p x = xg->value(&xs);
    byte_p y = yg->value(&ys);

    // First check if size difference is sufficient to let us decide
    int result = xs - ys;
    if (!result)
    {
        // Compare, starting with highest order
        for (int i = xs - 1; !result && i >= 0; i--)
            result = x[i] - y[i];
    }

    // If xt is ID_neg_bignum, then yt also must be, see test at top of function
    if (xt == ID_neg_bignum)
        result = -result;
    return result;
}



// ============================================================================
//
//    Big bignum arithmetic
//
// ============================================================================

// Operations with carry
static inline byte add_op(byte x, byte y, byte c)       { return x + y + c; }
static inline byte sub_op(byte x, byte y, byte c)       { return x - y - c; }
static inline byte neg_op(byte x, byte c)               { return -x - c; }
static inline byte not_op(byte x, byte  )               { return ~x; }
static inline byte and_op(byte x, byte y, byte  )       { return x & y; }
static inline byte or_op (byte x, byte y, byte  )       { return x | y; }
static inline byte xor_op(byte x, byte y, byte  )       { return x ^ y; }


inline object::id bignum::opposite_type(id type)
// ----------------------------------------------------------------------------
//   Return the type of the opposite
// ----------------------------------------------------------------------------
{
    switch(type)
    {
    case ID_bignum:            return ID_neg_bignum;
    case ID_neg_bignum:        return ID_bignum;
    default:                    return type;
    }
}


inline object::id bignum::product_type(id yt, id xt)
// ----------------------------------------------------------------------------
//   Return the type of the product of x and y
// ----------------------------------------------------------------------------
{
    switch(xt)
    {
    case ID_bignum:
        if (yt == ID_neg_bignum)
            return ID_neg_bignum;
        return ID_bignum;
    case ID_neg_bignum:
        if (yt == ID_neg_bignum)
            return ID_bignum;
        return ID_neg_bignum;
    default:
        return xt;
    }
}


bignum_g operator-(bignum_g xg)
// ----------------------------------------------------------------------------
//   Negate the input value
// ----------------------------------------------------------------------------
{
    runtime &rt = runtime::RT;
    object::id xt = xg->type();
    size_t xs = 0;
    byte_p x = xg->value(&xs);

    // Deal with simple case where we can simply copy the payload
    if (xt == object::ID_bignum)
        return rt.make<bignum>(object::ID_neg_bignum, x, xs);
    else if (xt == object::ID_neg_bignum)
        return rt.make<bignum>(object::ID_bignum, x, xs);

    // Complicated case of based numbers: need to actually compute the opposite
    return bignum::unary<true>(neg_op, xg);
}


bignum_g operator~(bignum_g x)
// ----------------------------------------------------------------------------
//   Boolean not
// ----------------------------------------------------------------------------
{
    runtime &rt = runtime::RT;
    object::id xt = x->type();

    // For bignum and neg_bignum, do a 0/1 logical not
    if (xt == object::ID_bignum || xt == object::ID_neg_bignum)
        return rt.make<bignum>(object::ID_bignum, !x);

    // For hex_bignum and other based numbers, do a binary not
    return bignum::unary<true>(not_op, x);
}


bignum_g bignum::add_sub(bignum_g yg, bignum_g xg, bool issub)
// ----------------------------------------------------------------------------
//   Add the two bignum values, result has type of x
// ----------------------------------------------------------------------------
{
    id yt = yg->type();
    id xt = xg->type();

    // Check if we have opposite signs
    bool samesgn = (xt == ID_neg_bignum) == (yt == ID_neg_bignum);
    if (samesgn == issub)
    {
        int cmp = compare(yg, xg);
        if (cmp >= 0)
        {
            // abs Y > abs X: result has opposite type of X
            id ty = cmp == 0 ? ID_bignum: issub ? xt : opposite_type(xt);
            return binary<false>(sub_op, yg, xg, ty);
        }
        else
        {
            // abs Y < abs X: result has type of X
            id ty = issub ? opposite_type(xt) : xt;
            return binary<false>(sub_op, xg, yg, ty);
        }
    }

    // We have the same sign, add items
    id ty = issub ? opposite_type(xt) : xt;
    return binary<false>(add_op, yg, xg, ty);
}


bignum_g operator+(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Add the two bignum values, result has type of x
// ----------------------------------------------------------------------------
{
    return bignum::add_sub(y, x, false);
}


bignum_g operator-(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Subtract two bignum values, result has type of x
// ----------------------------------------------------------------------------
{
    return bignum::add_sub(y, x, true);
}


bignum_g operator&(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Perform a binary and operation
// ----------------------------------------------------------------------------
{
    return bignum::binary<false>(and_op, x, y, x->type());
}


bignum_g operator|(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Perform a binary or operation
// ----------------------------------------------------------------------------
{
    return bignum::binary<false>(or_op, x, y, x->type());
}


bignum_g operator^(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Perform a binary xor operation
// ----------------------------------------------------------------------------
{
    return bignum::binary<false>(xor_op, x, y, x->type());
}


bignum_g bignum::multiply(bignum_g yg, bignum_g xg, id ty)
// ----------------------------------------------------------------------------
//   Perform multiply operation on the two big nums, with result type ty
// ----------------------------------------------------------------------------
{
    runtime &rt = runtime::RT;
    size_t xs = 0;
    size_t ys = 0;
    byte_p x = xg->value(&xs);
    byte_p y = yg->value(&ys);
    id xt = xg->type();
    size_t wbits = wordsize(xt);
    size_t wbytes = (wbits + 7) / 8;
    size_t needed = wbits ? std::min(xs + ys, wbytes) : xs + ys;
    size_t available = rt.available(needed);    // May GC here
    if (available < needed)
        return nullptr;                         // Out of memory
    byte *buffer = rt.scratchpad();             // This must be below GC above

    // Zero-initialie the result
    for (size_t i = 0; i < needed; i++)
        buffer[i] = 0;

    // Loop on all bytes of x then y
    for (size_t xi = 0; xi < xs; xi++)
    {
        byte xd = x[xi];
        for (int bit = 0; xd && bit < 8; bit++)
        {
            if (xd & (1<<bit))
            {
                uint c = 0;
                size_t yi;
                for (yi = 0; yi < ys; yi++)
                {
                    c += buffer[xi + yi] + (y[yi] << bit);
                    buffer[xi + yi] = byte(c);
                    c >>= 8;
                }
                while (c)
                {
                    c += buffer[xi + yi];
                    buffer[xi + yi] = byte(c);
                    c >>= 8;
                    yi++;
                }
                xd &= ~(1<<bit);
            }
        }
    }

    size_t sz = xs + ys;
    while (sz > 0 && buffer[sz-1] == 0)
        sz--;
    gcbytes bytes = rt.allocate(sz);
    bignum_g result = rt.make<bignum>(ty, bytes, sz);
    rt.free(sz);
    return result;
}


bignum_g operator*(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Multiplication of bignums
// ----------------------------------------------------------------------------
{
    object::id xt = x->type();
    object::id yt = y->type();
    object::id prodtype = bignum::product_type(yt, xt);
    return bignum::multiply(y, x, prodtype);
}


bool bignum::quorem(bignum_g yg, bignum_g xg, id ty, bignum_g *q, bignum_g *r)
// ----------------------------------------------------------------------------
//   Perform divide  operation op on leb128 numbers x and y
// ----------------------------------------------------------------------------
//   Result is placed in scratchpad, the function returns the size in bytes
{
    runtime &rt = runtime::RT;
    if (!xg)
    {
        rt.zero_divide_error();
        return false;
    }

    // In the computations below (e.g. needed), the size of the quotient is
    // less than the size of y, and the size of the remainder is less than the
    // size of x, therefore, we need at most xs + ys for both
    size_t xs = 0;
    size_t ys = 0;
    byte_p x = xg->value(&xs);
    byte_p y = yg->value(&ys);
    id xt = xg->type();
    size_t wbits = wordsize(xt);
    size_t wbytes = (wbits + 7) / 8;
    size_t needed = wbits ? std::min(ys + xs, wbytes) : ys + xs;
    size_t available = rt.available(needed);    // May GC here
    if (available < needed)
        return false;                         // Out of memory
    byte *buffer = rt.scratchpad();             // This must be below GC above

    // Allocate size for the quotient (smaller than y), initialize to 0
    byte *quotient = buffer;
    byte *remainder = quotient + ys;
    size_t rs = 0;
    size_t qs = 0;
    for (uint i = 0; i < xs + ys; i++)
        buffer[i] = 0;

    // Loop on the numerator
    for (int yi = ys-1; yi >= 0; yi--)
    {
        for (int bit = 7; bit >= 0; bit--)
        {
            // Shift remainder left by one bit, add numerator bit
            uint16_t c = (y[yi] >> bit) & 1;
            int delta = 0;
            for (uint ri = 0; ri < rs; ri++)
            {
                c += (remainder[ri] << 1);
                remainder[ri] = byte(c);
                if (int d = remainder[ri] - x[ri])
                    delta = d;
                c >>= 8;
            }
            if (c)
            {
                if (int d = c - x[rs])
                    delta = d;
                remainder[rs++] = c;
            }
            if (rs != xs)
                delta = rs - xs;

            // If remainder >= denominator, add to quotient, subtract from rem
            if (delta >= 0)
            {
                quotient[yi] |= (1 << bit);
                if (qs < size_t(yi + 1))
                    qs = yi + 1;

                c = 0;
                for (uint ri = 0; ri < rs; ri++)
                {
                    c = remainder[ri] - x[ri] - c;
                    remainder[ri] = byte(c);
                    c >>= 8;
                }

                // Strip zeroes at top of remainder
                while (rs > 0 && remainder[rs-1] == 0)
                    rs--;
            }
        } // numerator bit loop
    } // numerator byte loop

    // Generate results
    gcutf8 qg = quotient;
    gcutf8 rg = remainder;
    rt.allocate(needed);
    bool ok = true;
    if (q)
    {
        *q = rt.make<bignum>(ty, qg, qs);
        ok = bignum_p(*q) != nullptr;
    }
    if (r && ok)
    {
        *r = rt.make<bignum>(ty, rg, rs);
        ok = bignum_p(*r) != nullptr;
    }
    rt.free(needed);
    return ok;
}


bignum_g operator/(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//   Perform long division of y by x
// ----------------------------------------------------------------------------
{
    object::id yt = y->type();
    object::id xt = x->type();
    object::id prodtype = bignum::product_type(yt, xt);

    bignum_g q = nullptr;
    bignum::quorem(y, x, prodtype, &q, nullptr);
    return q;
}


bignum_g operator%(bignum_g y, bignum_g x)
// ----------------------------------------------------------------------------
//  Perform long-remainder of y by x
// ----------------------------------------------------------------------------
{
    object::id yt = y->type();
    bignum_g r = nullptr;
    bignum::quorem(y, x, yt, nullptr, &r);
    return r;
}


bignum_g bignum::pow(bignum_g y, bignum_g xg)
// ----------------------------------------------------------------------------
//    Compute y^abs(x)
// ----------------------------------------------------------------------------
//   Note that the case where x is negative should be filtered by caller
{
    bignum_g r = bignum::make(1);
    size_t xs = 0;
    byte_p x = xg->value(&xs);

    for (size_t xi = 0; xi < xs; xi++)
    {
        byte xv = x[xi];
        for (uint bit = 0; xv && bit < 7; bit++)
        {
            if (xv & 1)
                r = r * y;
            xv >>= 1;
            if (xv || xi < xs-1)
                y = y * y;
        }
    }
    return r;
}
