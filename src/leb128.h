#ifndef LEB128_H
#define LEB128_H
// ****************************************************************************
//  leb128.h                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Operations on LEB128-encoded data
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

#include "types.h"
#include <cstdint>


#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

template <typename Int = uint, typename Data>
NOINLINE Data *leb128(Int &res, Data *p)
// ----------------------------------------------------------------------------
//   Return the leb128 value at pointer
// ----------------------------------------------------------------------------
{
    const bool is_signed = Int(~0ULL) < Int(0);
    byte      *bp        = (byte *) p;
    Int        result    = 0;
    unsigned   shift     = 0;
    do
    {
        result |= Int(*bp & 0x7F) << shift;
        shift += 7;
    } while (*bp++ & 0x80);

    if (is_signed && (bp[-1] & 0x40))
        result |= Int(~0ULL << (shift - 1));
    res = result;
    return (Data *) bp;
}

template <typename Int = uint, typename Data>
inline INLINE Int leb128(Data *&p)
// ----------------------------------------------------------------------------
//   Return the leb128 value at pointer
// ----------------------------------------------------------------------------
{
    Int result;
    p = leb128(result, p);
    return result;
}


inline INLINE uint16_t leb128_u16(byte *bp)
// ----------------------------------------------------------------------------
//   Return the leb128 value at pointer
// ----------------------------------------------------------------------------
{
    uint16_t b1 = *bp;
    if (b1 < 0x80)
        return b1;
    return (b1 & 0x7F) | (uint16_t(bp[1]) << 7);
}


template<typename Data, typename Int = uint>
inline Data *leb128(Data *p, Int value)
// ----------------------------------------------------------------------------
//   Write the LEB value at pointer
// ----------------------------------------------------------------------------
{
    const bool is_signed = Int(~0ULL) < Int(0);
    byte *bp = (byte *) p;
    do
    {
        *bp++ = (value & 0x7F) | 0x80;
        value = Int(value >> 7);
    } while ((!is_signed && value != 0) ||
             (is_signed
              && (value != Int(~0ULL) || (~bp[-1] & 0x40))
              && (value != 0          || ( bp[-1] & 0x40))));
    bp[-1] &= ~0x80;
    return (Data *) bp;
}


template<typename Int>
inline size_t leb128size(Int value)
// ----------------------------------------------------------------------------
//   Compute the size required for a given integer value
// ----------------------------------------------------------------------------
{
    const bool is_signed = Int(~0ULL) < Int(0);
    size_t     result    = 0;
    bool       signbit   = false;
    do
    {
        signbit = value & 0x40;
        value = Int(value >> 7);
        result++;
    } while ((!is_signed && value != 0) ||
             (is_signed
              && (value != Int(~0ULL)   || !signbit)
              && (value != 0            ||  signbit)));
    return result;
}


template<typename Data>
inline size_t leb128size(Data *ptr)
// ----------------------------------------------------------------------------
//   Compute the size of an LEB128 value at pointer
// ----------------------------------------------------------------------------
{
    byte *s = (byte *) ptr;
    byte *p = s;
    do { } while (*p++ & 0x80);
    return p - s;
}


template<typename Data>
inline Data *leb128skip(Data *ptr)
// ----------------------------------------------------------------------------
//   Skip LEB128 data
// ----------------------------------------------------------------------------
{
    const byte *p = (const byte *) ptr;
    while ((*p++) & 0x80);
    return (Data *) p;
}

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42

#endif // LEB128_H
