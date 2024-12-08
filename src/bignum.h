#ifndef BIGNUM_H
#define BIGNUM_H
// ****************************************************************************
//  bignum.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//      The bignum object type
//
//      Operations on bignum values
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
//
// Payload format:
//
//   Bignum types are distinguished by their type ID.
//   Negative bignums are represented by ID_neg_bignum.
//   They store their magnitude as a sized sequence of bytes
//
//   For 'integer' values, the present implementation limits itself to 64 bits,
//   and uses native CPU operations to do that (or at least, a fixed number of
//   CPU operations on 32-bit CPUs like on the DM42).
//
//   While an implementation of computations on larger values was implemented on
//   this LEB128 format, it is now replaced with the bignum format for both
//   computation and memory efficiency reasons:
//   - Computations do not need to mask 7 bits at every step
//   - Memory for 64 bits is 2 (ID + size) + 8 (payload) = 10, whereas LEB128
//     would use 1 (ID) + 10 (64 / 7 > 9), so starting at 63 bits, the LEB128
//     representation is 12.5% less memory-efficient.
//
//   See bignum.h for the 8-bit big-num implementation
//


#include "algebraic.h"
#include "integer.h"
#include "object.h"
#include "runtime.h"
#include "settings.h"
#include "text.h"

#include <algorithm>

GCP(bignum);

struct bignum : text
// ----------------------------------------------------------------------------
//    Represent bignum objects, i.e. integer values with more than 64 bits
// ----------------------------------------------------------------------------
{
    template <typename Int>
    static size_t bytesize(Int x)
    {
        size_t sz = 0;
        while (x > 0)                          // Defensive coding against x < 0
        {
            sz++;
            if (sizeof(x) > 1)
                x >>= 8;
            else
                break;
        }
        return sz;
    }

    static size_t bytesize(integer_p i)
    {
        byte_p p = payload(i);
        size_t bitsize = 0;
        while (*p & 0x80)
        {
            bitsize += 7;
            p++;
        }
        byte c = *p;
        while(c)
        {
            bitsize++;
            c >>= 1;
        }
        return (bitsize + 7) / 8;
    }

    static size_t bytesize(integer_r i)
    {
        return bytesize(integer_p(i));
    }

    template <typename Int>
    bignum(id type, Int value)
        : text(type, (utf8) &value, bytesize(value))
    {
        byte *p = (byte *) payload(this);
        size_t sz = leb128<size_t>(p);
        for (uint i = 0; i < sz; i++)
            p[i] = byte(value >> (8 * i));
    }

    template <typename Int>
    static size_t required_memory(id i, Int value)
    {
        size_t size = bytesize(value);
        return leb128size(i) + leb128size(size) + size;
    }

    bignum(id type, gcbytes ptr, size_t size): text(type, ptr, size) {}

    static size_t required_memory(id i, gcbytes UNUSED ptr, size_t size)
    {
        return leb128size(i) + leb128size(size) + size;
    }

    bignum(id type, integer_g value);
    static size_t required_memory(id i, integer_g value);

    template <typename Int>
    Int value() const
    {
        size_t size = 0;
        utf8 p = value(&size);
        Int result = 0;
        for (size_t i = 0; i < size; i++)
            result |= Int(*p++) << (8 * i);
        return result;
    }

    byte_p value(size_t *size) const
    {
        return text::value(size);
    }

    // Creating a small integer from a bignum, or return nullptr
    integer_p as_integer() const;

    // Check if it matches a given value
    bool is_zero() const        { return length() == 0; }
    bool is_one() const         { return is(1); }
    bool is(ularge test) const
    {
        size_t size = 0;
        byte_p data = value(&size);
        for (size_t i = 0; i < size; i++)
        {
            if (data[i] != byte(test))
                return false;
            test >>= 8;
        }
        return true;
    }

    template <typename Int>
    static bignum_p make(Int value);

public:
    // Arithmetic internal routines
    static int compare(bignum_r x, bignum_r y, bool magnitude = false);

    static size_t wordsize(id type);
    size_t wordsize() const             { return wordsize(type()); }
    static id opposite_type(id type);
    static id product_type(id yt, id xt);

    template<bool extend, typename Op>
    static bignum_p binary(Op op, bignum_r x, bignum_r y, id ty);
    template<bool extend, typename Op>
    static bignum_p unary(Op op, bignum_r x);

    static bignum_p add_sub(bignum_r y, bignum_r x, bool subtract);
    static bignum_p multiply(bignum_r y, bignum_r x, id ty);
    static bool quorem(bignum_r y, bignum_r x, id ty, bignum_g *q, bignum_g *r);
    static bignum_p pow(bignum_r y, bignum_r x);
    static bignum_p shift(bignum_r x, int bits, bool rotate, bool arith);

    static bignum_p promote(object_p ival);

public:
    OBJECT_DECL(bignum);
    RENDER_DECL(bignum);
    HELP_DECL(bignum);
};


template <object::id Type>
struct special_bignum : bignum
// ----------------------------------------------------------------------------
//   Representation for other bignum types
// ----------------------------------------------------------------------------
{
    template <typename Int>
    special_bignum(id type, Int value): bignum(type, value) {}

public:
    // Can't use the OBJECT_DECL and RENDER_DECL macros here
    static const id static_id = Type;
    static size_t do_render(const special_bignum *o, renderer &r);
};

using neg_bignum   = special_bignum<object::ID_neg_bignum>;
#if CONFIG_FIXED_BASED_OBJECTS
using hex_bignum   = special_bignum<object::ID_hex_bignum>;
using oct_bignum   = special_bignum<object::ID_oct_bignum>;
using bin_bignum   = special_bignum<object::ID_bin_bignum>;
using dec_bignum   = special_bignum<object::ID_dec_bignum>;
#endif // CONFIG_FIXED_BASED_OBJECTS
using based_bignum = special_bignum<object::ID_based_bignum>;
GCP_EXISTING(based_bignum);

template <typename Int>
bignum_p bignum::make(Int value)
// ----------------------------------------------------------------------------
//   Make an bignum with the correct sign
// ----------------------------------------------------------------------------
{
    return value < 0 ? rt.make<neg_bignum>(-value) : rt.make<bignum>(value);
}



// ============================================================================
//
//    Bignum comparisons and arithmetic
//
// ============================================================================

inline bool operator< (bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) <  0;
}

inline bool operator==(bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) == 0;
}

inline bool operator> (bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) >  0;
}

inline bool operator<=(bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) <= 0;
}

inline bool operator>=(bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) >= 0;
}

inline bool operator!=(bignum_r y, bignum_r x)
{
    return bignum::compare(y,x) != 0;
}


bignum_p operator- (bignum_r x);
bignum_p operator~ (bignum_r x);
bignum_p operator+ (bignum_r y, bignum_r x);
bignum_p operator- (bignum_r y, bignum_r x);
bignum_p operator* (bignum_r y, bignum_r x);
bignum_p operator/ (bignum_r y, bignum_r x);
bignum_p operator% (bignum_r y, bignum_r x);
bignum_p operator& (bignum_r y, bignum_r x);
bignum_p operator| (bignum_r y, bignum_r x);
bignum_p operator^ (bignum_r y, bignum_r x);
bignum_p operator<<(bignum_r y, bignum_r x);
bignum_p operator>>(bignum_r y, bignum_r x);
bignum_p operator<<(bignum_r y, uint x);
bignum_p operator>>(bignum_r y, uint x);



// ============================================================================
//
//   Helper code
//
// ============================================================================

inline size_t bignum::wordsize(id type)
// ----------------------------------------------------------------------------
//   Return the word size for an bignum type in bits
// ----------------------------------------------------------------------------
{
    if (is_based(type))
        return Settings.WordSize();
    return 0;
}


template <bool extend, typename Op>
bignum_p bignum::binary(Op op, bignum_r xg, bignum_r yg, id ty)
// ----------------------------------------------------------------------------
//   Perform binary operation op on bignum values xg and yg
// ----------------------------------------------------------------------------
//   This uses the scratch pad AND can cause garbage collection
{
    if (!xg || !yg)
        return nullptr;

    size_t   xs     = 0;
    size_t   ys     = 0;
    byte_p   x      = xg->value(&xs);
    byte_p   y      = yg->value(&ys);
    size_t   wbits  = wordsize(ty);
    size_t   wbytes = (wbits + 7) / 8;
    uint16_t c      = 0;
    size_t   needed = std::max(xs, ys) + 1;
    if (!wbits && needed * 8 > Settings.MaxNumberBits())
    {
        rt.number_too_big_error();
        return nullptr;
    }
    if (needed < wbytes || wbits)
        needed = wbytes;
    byte *buffer = rt.allocate(needed);         // May GC here
    if (!buffer)
        return nullptr;                         // Out of memory
    x = xg->value(&xs);                         // Re-read after potential GC
    y = yg->value(&ys);

    // Process the part that is common to X and Y
    size_t i = 0;
    size_t max = std::min(std::min(xs, ys), needed);
    for (i = 0; i < max; i++)
    {
        byte xd = x[i];
        byte yd = y[i];
        c = op(xd, yd, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Process X-only part if there is one
    for (max = std::min(xs, needed); i < max; i++)
    {
        byte xd = x[i];
        c = op(xd, 0, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Process Y-only part if there is one
    for (max = std::min(ys, needed); i < max; i++)
    {
        byte yd = y[i];
        c = op(0, yd, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Process extension to wordsize (when op(0, 0, 0) can be non-zero)
    for (max = ((extend || c) && wbits) ? wbytes : 0; i < max; i++)
    {
        c = op(0, 0, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Write last carry if applicable
    if (c && i < needed)
        buffer[i++] = c;

    // Drop highest zeros (this can reach i == 0 for value zero)
    while (i > 0 && buffer[i - 1] == 0)
        i--;

    // Check if we have a word size like 12 and we need to truncate result
    if (i == wbytes && (wbits % 8))
        buffer[i-1] &= byte(0xFFu >> (8 - wbits % 8));

    // Create the resulting bignum
    gcbytes buf = buffer;
    bignum_g result = rt.make<bignum>(ty, buf, i);
    rt.free(needed);
    return result;
}


template<bool extend, typename Op>
bignum_p bignum::unary(Op op, bignum_r xg)
// ----------------------------------------------------------------------------
//   Perform a unary operation on a bignum
// ----------------------------------------------------------------------------
//   This uses the scratch pad AND can cause garbage collection
{
    if (!xg)
        return nullptr;
    size_t   xs     = 0;
    byte_p   x      = xg->value(&xs);
    id       xt     = xg->type();
    size_t   wbits  = wordsize(xt);
    size_t   wbytes = (wbits + 7) / 8;
    uint16_t c      = 0;
    size_t   needed = xs + 1;
    if (wbits && needed < wbytes)
        needed = wbytes;
    byte *buffer = rt.allocate(needed); // May GC here
    if (!buffer)
        return nullptr; // Out of memory
    x = xg->value(&xs); // Re-read after potential GC

    // Process the part in X
    size_t i   = 0;
    size_t max = std::min(xs, needed);
    for (i = 0; i < max; i++)
    {
        byte xd = x[i];
        c = op(xd, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Process extension to wordsize (when op(0, 0, 0) can be non-zero)
    for (max = (extend && wbits) ? wbytes : 0; i < max; i++)
    {
        c = op(0, c);
        buffer[i] = byte(c);
        c >>= 8;
    }

    // Write last carry if applicable
    if (c && i < needed)
        buffer[i++] = c;

    // Drop highest zeros (this can reach i == 0 for value 0)
    while (i > 0 && buffer[i - 1] == 0)
        i--;

    // Check if we have a word size like 12 and we need to truncate result
    if (i == wbytes && (wbits % 8))
        buffer[i-1] &= byte(0xFFu >> (8 - wbits % 8));

    // Create the resulting bignum
    gcbytes buf = buffer;
    bignum_g result = rt.make<bignum>(xt, buf, i);
    rt.free(needed);
    return result;
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
        if (is_based(yt))
            return yt;
        return ID_bignum;
    case ID_neg_bignum:
        if (yt == ID_neg_bignum)
            return ID_bignum;
        if (is_based(yt))
            return yt;
        return ID_neg_bignum;
    default:
        return xt;
    }
}


#endif // BIGNUM_H
