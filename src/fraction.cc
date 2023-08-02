// ****************************************************************************
//  fraction.cc                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Representation of fractions
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

#include "fraction.h"
#include "algebraic.h"

RECORDER(fraction, 16, "Fractions");


static ularge gcd(ularge a, ularge b)
// ----------------------------------------------------------------------------
//   Compute the greatest common denominator between a and b
// ----------------------------------------------------------------------------
{
    while (b)
    {
        ularge na = b;
        b = a % b;
        a = na;
    }
    return a;
}


SIZE_BODY(fraction)
// ----------------------------------------------------------------------------
//   Return the size of an LEB128-encoded fraction
// ----------------------------------------------------------------------------
{
    // LEB-128 encoded numerator and denominator
    byte_p p = o->payload();
    size_t ns = leb128size(p);
    p += ns;
    size_t ds = leb128size(p);
    p += ds;
    return ptrdiff(p, o);
}


fraction_g fraction::make(integer_g n, integer_g d)
// ----------------------------------------------------------------------------
//   Create a reduced fraction from n and d
// ----------------------------------------------------------------------------
{
    ularge nv = n->value<ularge>();
    ularge dv = d->value<ularge>();
    ularge cd = gcd(nv, dv);
    bool neg = (n->type() == ID_neg_integer) != (d->type() == ID_neg_integer);
    if (cd > 1)
    {
        n = integer::make(nv / cd);
        d = integer::make(dv / cd);
    }
    id ty = neg ? ID_neg_fraction : ID_fraction;
    return rt.make<fraction>(ty, n, d);
}


bignum_g fraction::numerator() const
// ----------------------------------------------------------------------------
//   Return the numerator as an integer
// ----------------------------------------------------------------------------
{
    id ty = type();
    if (ty == ID_big_fraction || ty == ID_neg_big_fraction)
        return big_fraction_p(this)->numerator();

    ty = (ty == ID_neg_fraction) ? ID_neg_bignum : ID_bignum;
    byte_p p = payload();
    ularge nv = leb128<ularge>(p);
    return rt.make<bignum>(ty, nv);
}


bignum_g fraction::denominator() const
// ----------------------------------------------------------------------------
//   Return the denominator as an integer (always positive)
// ----------------------------------------------------------------------------
{
    id ty = type();
    if (ty == ID_big_fraction || ty == ID_neg_big_fraction)
        return big_fraction_p(this)->denominator();

    byte_p p = payload();
    size_t nv = leb128<ularge>(p);
    size_t dv = leb128<ularge>(p) + 0 * nv;
    return rt.make<bignum>(ID_bignum, dv);
}


integer_g fraction::numerator(int) const
// ----------------------------------------------------------------------------
//   Return the numerator as an integer
// ----------------------------------------------------------------------------
{
    id ty = (type() == ID_neg_fraction) ? ID_neg_integer : ID_integer;
    byte_p p = payload();
    ularge nv = leb128<ularge>(p);
    return rt.make<integer>(ty, nv);
}


integer_g fraction::denominator(int) const
// ----------------------------------------------------------------------------
//   Return the denominator as an integer (always positive)
// ----------------------------------------------------------------------------
{
    byte_p p = payload();
    size_t nv = leb128<ularge>(p);
    size_t dv = leb128<ularge>(p) + 0 * nv;
    return rt.make<integer>(ID_integer, dv);
}




// ============================================================================
//
//   Bignum-based fractions
//
// ============================================================================

SIZE_BODY(big_fraction)
// ----------------------------------------------------------------------------
//   Big fractions use size and whole bytes
// ----------------------------------------------------------------------------
{
    // Bignum-encoded numerator and denominator
    byte_p p = o->payload();
    size_t ns = leb128<size_t>(p);
    p += ns;
    size_t ds = leb128<size_t>(p);
    p += ds;
    return ptrdiff(p, o);
}


bignum_g big_fraction::numerator() const
// ------------------------------------------------------------------------
//   Return the numerator as a bignum
// ------------------------------------------------------------------------
{
    id ty = type() == ID_neg_big_fraction ? ID_neg_bignum : ID_bignum;
    byte_p p = payload();
    size_t ns = leb128<size_t>(p);
    return rt.make<bignum>(ty, p, ns);
}


bignum_g big_fraction::denominator() const
// ------------------------------------------------------------------------
//   Return the denominator as bignum (always positive)
// ------------------------------------------------------------------------
{
    byte_p p = payload();
    size_t ns = leb128<size_t>(p);
    p += ns;
    size_t ds = leb128<size_t>(p);
    return rt.make<bignum>(ID_bignum, p, ds);
}


static bignum_g gcd(bignum_g a, bignum_g b)
// ----------------------------------------------------------------------------
//   Compute the greatest common denominator between a and b
// ----------------------------------------------------------------------------
{
    while (!b->is_zero())
    {
        bignum_g na = b;
        b = a % b;
        a = na;
    }
    return a;
}


fraction_g big_fraction::make(bignum_g n, bignum_g d)
// ----------------------------------------------------------------------------
//   Create a reduced fraction from n and d
// ----------------------------------------------------------------------------
{
    bignum_g cd = gcd(n, d);
    if (!cd->is(1))
    {
        n = n / cd;
        d = d / cd;
    }
    // Check if numerator and denominator are small enough to use LEB128
    if (integer_g ni = (integer *) n->as_integer())
        if (integer_g di = (integer *) d->as_integer())
            return fraction::make(ni, di);

    // Otherwise, use the bignum representation
    bool neg = (n->type() == ID_neg_bignum) != (d->type() == ID_neg_bignum);
    id ty = neg ? ID_neg_big_fraction : ID_big_fraction;
    return rt.make<big_fraction>(ty, n, d);
}



// ============================================================================
//
//   Arithmetic (which works both for bignum and LEB128 coding)
//
// ============================================================================

fraction_g operator-(fraction_r x)
// ----------------------------------------------------------------------------
//    Negation of a fraction
// ----------------------------------------------------------------------------
{
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    return big_fraction::make(-xn, xd);
}


fraction_g operator+(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Add two fractions
// ----------------------------------------------------------------------------
{
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    return big_fraction::make(xn * yd + yn * xd, xd * yd);
}


fraction_g operator-(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Subtract two fractions
// ----------------------------------------------------------------------------
{
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    return big_fraction::make(xn * yd - yn * xd, xd * yd);
}


fraction_g operator*(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Multiply two fractions
// ----------------------------------------------------------------------------
{
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    return big_fraction::make(xn * yn, xd * yd);
}


fraction_g operator/(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Divide two fractions
// ----------------------------------------------------------------------------
{
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    return big_fraction::make(xn * yd, xd * yn);
}


fraction_g operator%(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Compute the remainder of two fractions
// ----------------------------------------------------------------------------
{
    bignum_g   xn = x->numerator();
    bignum_g   xd = x->denominator();
    bignum_g   yn = y->numerator();
    bignum_g   yd = y->denominator();
    fraction_g q  = big_fraction::make(xn * yd, xd * yn);
    bignum_g   ir = q->numerator() / q->denominator();
    fraction_g fr = big_fraction::make(ir, bignum::make(1));
    fr            = fr * y;
    q             = x - fr;
    return q;
}
