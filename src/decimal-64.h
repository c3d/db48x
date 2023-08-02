#ifndef DECIMAL64_H
#define DECIMAL64_H
// ****************************************************************************
//  decimal64.h                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Real numbers in decimal64 representation
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
//
// Payload format:
//
//   A copy of the 64-bit representation of the object follows the type
//
//   Since it is unclear that Intel's library is robust to misaligned data,
//   We presently copy that payload when operating on objects. This may be
//   unnecessary.


#include "algebraic.h"
#include "runtime.h"
#include "settings.h"

#include <bid_conf.h>
#include <bid_functions.h>
#include <cstring>

GCP(bignum);
GCP(fraction);

struct decimal64 : algebraic
// ----------------------------------------------------------------------------
//    Floating-point numbers in 64-bit decimal64 representation
// ----------------------------------------------------------------------------
{
    decimal64(gcstring value, id type = ID_decimal64): algebraic(type)
    {
        bid64 num;
        bid64_from_string(&num.value, (cstring) value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal64(const bid64 &value, id type = ID_decimal64): algebraic(type)
    {
        byte *p = (byte *) payload(this);
        memcpy(p, &value, sizeof(value));
    }

    decimal64(uint64_t value, id type = ID_decimal64): algebraic(type)
    {
        BID_UINT64 bval = BID_UINT64(value);
        bid64 num;
        bid64_from_uint64(&num.value, &bval);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal64(uint64_t value, bool neg, id ty = ID_decimal64): algebraic(ty)
    {
        BID_UINT64 bval = BID_UINT64(value);
        bid64 num, negated;
        byte *p = (byte *) payload();
        bid64_from_uint64(&num.value, &bval);
        if (neg)
            bid64_negate(&negated.value, &num.value);
        memcpy(p, neg ? &negated : &num, sizeof(num));
    }

    decimal64(int64_t value, id type = ID_decimal64): algebraic(type)
    {
        BID_SINT64 bval = BID_SINT64(value);
        bid64 num;
        bid64_from_int64(&num.value, &bval);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal64(uint32_t value, id type = ID_decimal64): algebraic(type)
    {
        bid64 num;
        // Bug in the BID library, which uses int and not int32_t
        bid64_from_uint32(&num.value, (uint *) &value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal64(int32_t value, id type = ID_decimal64): algebraic(type)
    {
        bid64 num;
        // Bug in the BID library, which uses int and not int32_t
        bid64_from_int32(&num.value, (int *) &value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal64(bignum_p value, id type = ID_decimal64);
    decimal64(fraction_p value, id type = ID_decimal64);

#if 64 > 64
    decimal64(const bid64 &value, id type = ID_decimal64): algebraic(type)
    {
        bid64 num;
        bid64_to_bid64(&num.value, (BID_UINT64 *) &value.value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }
#endif

#if 64 > 32
    decimal64(const bid32 &value, id type = ID_decimal64): algebraic(type)
    {
        bid64 num;
        bid32_to_bid64(&num.value, (BID_UINT32 *) &value.value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }
#endif

    template <typename Value>
    static size_t required_memory(id i, Value UNUSED value)
    {
        return leb128size(i) + sizeof(bid64);
    }

    template <typename Value>
    static size_t required_memory(id i, Value UNUSED value, bool UNUSED neg)
    {
        return leb128size(i) + sizeof(bid64);
    }

    bid64 value() const
    {
        bid64 result;
        byte_p p = payload(this);
        memcpy(&result, p, sizeof(result));
        return result;
    }

    enum class_type
    // ------------------------------------------------------------------------
    //   Class type for bid64 numbers
    // ------------------------------------------------------------------------
    //   This should really be exported in the header, since it's the result of
    //   the bid64_class function. Lifted from Inte's source code
    {
        signalingNaN,
        quietNaN,
        negativeInfinity,
        negativeNormal,
        negativeSubnormal,
        negativeZero,
        positiveZero,
        positiveSubnormal,
        positiveNormal,
        positiveInfinity
    };

    static class_type fpclass(const BID_UINT64 *b)
    {
        int c = 0;
        bid64_class(&c, (BID_UINT64 *) b);
        return (class_type) c;
    }

    static class_type fpclass(const bid64 &x)
    {
        return fpclass(&x.value);
    }

    class_type fpclass() const
    {
        return fpclass(value());
    }

    static bool is_zero(const BID_UINT64 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeZero && c <= positiveZero;
    }

    static bool is_zero(const bid64 &x)
    {
        return is_zero(&x.value);
    }

    bool is_zero() const
    {
        return is_zero(value());
    }

    bool is_one() const
    {
        uint oneint = 1;
        bid64 one;
        bid64_from_uint32(&one.value, &oneint);
        bid64 num = value();
        bid64 zero;
        bid64_sub(&zero.value, &num.value, &one.value);
        return is_zero(zero);
    }

    static bool is_negative(const BID_UINT64 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeInfinity && c <= negativeZero;
    }

    static bool is_negative(const bid64 &x)
    {
        return is_negative(&x.value);
    }

    bool is_negative() const
    {
        return is_negative(value());
    }

    static bool is_negative_or_zero(const BID_UINT64 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeInfinity && c <= positiveZero;
    }

    static bool is_negative_or_zero(const bid64 &x)
    {
        return is_negative_or_zero(&x.value);
    }

    bool is_negative_or_zero() const
    {
        return is_negative_or_zero(value());
    }

    algebraic_p to_fraction(uint count = Settings.fraciter,
                            uint decimals = Settings.fracprec) const;

public:
    OBJECT_DECL(decimal64);
    PARSE_DECL(decimal64);
    SIZE_DECL(decimal64);
    RENDER_DECL(decimal64);
};

typedef const decimal64 *decimal64_p;

// Functions used by arithmetic.h
void bid64_mod(BID_UINT64 *pres, BID_UINT64 *px, BID_UINT64 *py);
void bid64_rem(BID_UINT64 *pres, BID_UINT64 *px, BID_UINT64 *py);

// Utlity common to all formats to format a number for display
size_t decimal_format(char *buf, size_t len, bool editing, bool raw);

#endif // DECIMAL64_H
