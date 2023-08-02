#ifndef DECIMAL128_H
#define DECIMAL128_H
// ****************************************************************************
//  decimal128.h                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Real numbers in decimal128 representation
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
//   A copy of the 128-bit representation of the object follows the type
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

struct decimal128 : algebraic
// ----------------------------------------------------------------------------
//    Floating-point numbers in 128-bit decimal128 representation
// ----------------------------------------------------------------------------
{
    decimal128(gcstring value, id type = ID_decimal128): algebraic(type)
    {
        bid128 num;
        bid128_from_string(&num.value, (cstring) value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal128(const bid128 &value, id type = ID_decimal128): algebraic(type)
    {
        byte *p = (byte *) payload(this);
        memcpy(p, &value, sizeof(value));
    }

    decimal128(uint64_t value, id type = ID_decimal128): algebraic(type)
    {
        BID_UINT64 bval = BID_UINT64(value);
        bid128 num;
        bid128_from_uint64(&num.value, &bval);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal128(uint64_t value, bool neg, id ty = ID_decimal128): algebraic(ty)
    {
        BID_UINT64 bval = BID_UINT64(value);
        bid128 num, negated;
        byte *p = (byte *) payload();
        bid128_from_uint64(&num.value, &bval);
        if (neg)
            bid128_negate(&negated.value, &num.value);
        memcpy(p, neg ? &negated : &num, sizeof(num));
    }

    decimal128(int64_t value, id type = ID_decimal128): algebraic(type)
    {
        BID_SINT64 bval = BID_SINT64(value);
        bid128 num;
        bid128_from_int64(&num.value, &bval);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal128(uint32_t value, id type = ID_decimal128): algebraic(type)
    {
        bid128 num;
        // Bug in the BID library, which uses int and not int32_t
        bid128_from_uint32(&num.value, (uint *) &value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal128(int32_t value, id type = ID_decimal128): algebraic(type)
    {
        bid128 num;
        // Bug in the BID library, which uses int and not int32_t
        bid128_from_int32(&num.value, (int *) &value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }

    decimal128(bignum_p value, id type = ID_decimal128);
    decimal128(fraction_p value, id type = ID_decimal128);

#if 128 > 64
    decimal128(const bid64 &value, id type = ID_decimal128): algebraic(type)
    {
        bid128 num;
        bid64_to_bid128(&num.value, (BID_UINT64 *) &value.value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }
#endif

#if 128 > 32
    decimal128(const bid32 &value, id type = ID_decimal128): algebraic(type)
    {
        bid128 num;
        bid32_to_bid128(&num.value, (BID_UINT32 *) &value.value);
        byte *p = (byte *) payload(this);
        memcpy(p, &num, sizeof(num));
    }
#endif

    template <typename Value>
    static size_t required_memory(id i, Value UNUSED value)
    {
        return leb128size(i) + sizeof(bid128);
    }

    template <typename Value>
    static size_t required_memory(id i, Value UNUSED value, bool UNUSED neg)
    {
        return leb128size(i) + sizeof(bid128);
    }

    bid128 value() const
    {
        bid128 result;
        byte_p p = payload(this);
        memcpy(&result, p, sizeof(result));
        return result;
    }

    enum class_type
    // ------------------------------------------------------------------------
    //   Class type for bid128 numbers
    // ------------------------------------------------------------------------
    //   This should really be exported in the header, since it's the result of
    //   the bid128_class function. Lifted from Inte's source code
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

    static class_type fpclass(const BID_UINT128 *b)
    {
        int c = 0;
        bid128_class(&c, (BID_UINT128 *) b);
        return (class_type) c;
    }

    static class_type fpclass(const bid128 &x)
    {
        return fpclass(&x.value);
    }

    class_type fpclass() const
    {
        return fpclass(value());
    }

    static bool is_zero(const BID_UINT128 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeZero && c <= positiveZero;
    }

    static bool is_zero(const bid128 &x)
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
        bid128 one;
        bid128_from_uint32(&one.value, &oneint);
        bid128 num = value();
        bid128 zero;
        bid128_sub(&zero.value, &num.value, &one.value);
        return is_zero(zero);
    }

    static bool is_negative(const BID_UINT128 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeInfinity && c <= negativeZero;
    }

    static bool is_negative(const bid128 &x)
    {
        return is_negative(&x.value);
    }

    bool is_negative() const
    {
        return is_negative(value());
    }

    static bool is_negative_or_zero(const BID_UINT128 *x)
    {
        class_type c = fpclass(x);
        return c >= negativeInfinity && c <= positiveZero;
    }

    static bool is_negative_or_zero(const bid128 &x)
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
    OBJECT_DECL(decimal128);
    PARSE_DECL(decimal128);
    SIZE_DECL(decimal128);
    RENDER_DECL(decimal128);
};

typedef const decimal128 *decimal128_p;

// Functions used by arithmetic.h
void bid128_mod(BID_UINT128 *pres, BID_UINT128 *px, BID_UINT128 *py);
void bid128_rem(BID_UINT128 *pres, BID_UINT128 *px, BID_UINT128 *py);

// Utlity common to all formats to format a number for display
size_t decimal_format(char *buf, size_t len, bool editing, bool raw);

#endif // DECIMAL128_H
