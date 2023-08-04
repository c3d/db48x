#ifndef FRACTION_H
#define FRACTION_H
// ****************************************************************************
//  fraction.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Representation of mathematical fractions
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
//   Payload representation:
//   - The ID is one of four formats:
//     + ID_fraction:           Positive ratio of two LEB128-encoded numbers
//     + ID_neg_fraction:       Negative ratio of two LEB128-encoded numbers
//     + ID_big_fraction:       Positive ratio of two bignum-encoded numbers
//     + ID_neg_big_fraction:   Negative ratio of two bignum-encoded numbers
//   - Following the ID are the two payloads for the matching integer type
//
//   A lot of the code in fraction is carefully written to work both with
//   integer (LEB128) and bignum (sized + bytes) payloads

#include "bignum.h"
#include "integer.h"
#include "object.h"
#include "runtime.h"

GCP(fraction);
GCP(big_fraction);


struct fraction : algebraic
// ----------------------------------------------------------------------------
//   A fraction is a ratio of two integers
// ----------------------------------------------------------------------------
{
    fraction(integer_g n, integer_g d, id type)
    // ------------------------------------------------------------------------
    //   Constructs a fraction from two integers or two bignums
    // ------------------------------------------------------------------------
        : algebraic(type)
    {
        // This is written so that it works with integer_g and bignum_g
        byte *p = (byte *) payload();
        byte_p np = n->payload();
        byte_p dp = d->payload();
        size_t ns = n->skip() - object_p(np);
        size_t ds = d->skip() - object_p(dp);
        memcpy(p, np, ns);
        memcpy(p + ns, dp, ds);
    }

    static size_t required_memory(id i, integer_g n, integer_g d)
    // ------------------------------------------------------------------------
    //  Compute the amount of memory required for an object
    // ------------------------------------------------------------------------
    {
        return leb128size(i)
            + n->size() - leb128size(n->type())
            + d->size() - leb128size(d->type());
    }

    bignum_g numerator() const;
    bignum_g denominator() const;
    integer_g numerator(int) const;
    integer_g denominator(int) const;

    bool is_zero() const { return numerator()->is_zero(); }
    bool is_one()  const { return (numerator() - denominator())->is_zero(); }

    static fraction_g make(integer_g n, integer_g d);

public:
    OBJECT_DECL(fraction);
    SIZE_DECL(fraction);
    RENDER_DECL(fraction);
    PREC_DECL(MULTIPLICATIVE);
};


struct neg_fraction : fraction
// ----------------------------------------------------------------------------
//   Negative fraction, the numerator is seen as negative
// ----------------------------------------------------------------------------
{
    neg_fraction(integer_g num, integer_g den, id type = ID_neg_fraction)
        : fraction(num, den, type) {}

public:
    OBJECT_DECL(neg_fraction);
    RENDER_DECL(neg_fraction);
};

struct big_fraction : fraction
// ----------------------------------------------------------------------------
//   A fraction where numerator and denominator are bignum
// ----------------------------------------------------------------------------
{
    big_fraction(bignum_g n, bignum_g d, id type):
    // ------------------------------------------------------------------------
    //   Constructor for a big fraction
    // ------------------------------------------------------------------------
        // We play a rather ugly wrong-cast game here...
        fraction((integer *) bignum_p(n), (integer *) bignum_p(d), type)
    {}

    static size_t required_memory(id i, bignum_g n, bignum_g d)
    // ------------------------------------------------------------------------
    //  Compute the amount of memory required for an object
    // ------------------------------------------------------------------------
    {
        return leb128size(i)
            + n->size() - leb128size(n->type())
            + d->size() - leb128size(d->type());
    }

    static fraction_g make(bignum_g n, bignum_g d);

    bignum_g numerator() const;
    bignum_g denominator() const;

public:
    OBJECT_DECL(big_fraction);
    SIZE_DECL(big_fraction);
    RENDER_DECL(big_fraction);
};


struct neg_big_fraction : big_fraction
// ----------------------------------------------------------------------------
//   A negative fraction where numerator and denominator are bignum
// ----------------------------------------------------------------------------
{
    neg_big_fraction(bignum_g num, bignum_g den, id type = ID_neg_big_fraction)
        : big_fraction(num, den, type) {}
public:
    OBJECT_DECL(neg_big_fraction);
    RENDER_DECL(neg_big_fraction);
};

fraction_g operator-(fraction_r x);
fraction_g operator+(fraction_r x, fraction_r y);
fraction_g operator-(fraction_r x, fraction_r y);
fraction_g operator*(fraction_r x, fraction_r y);
fraction_g operator/(fraction_r x, fraction_r y);
fraction_g operator%(fraction_r x, fraction_r y);


#endif // FRACTION_H
