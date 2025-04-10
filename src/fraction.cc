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
#include "expression.h"
#include "grob.h"


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


HELP_BODY(fraction)
// ----------------------------------------------------------------------------
//   Help topic for fractions
// ----------------------------------------------------------------------------
{
    return utf8("Fractions");
}


EVAL_BODY(fraction)
// ----------------------------------------------------------------------------
//   Evaluate either as a fraction or decimal
// ----------------------------------------------------------------------------
{
    if (Settings.NumericalResults())
    {
        algebraic_g x = o;
        if (algebraic::decimal_promotion(x))
            if (rt.push(+x))
                return OK;
    }
    return rt.push(o) ? OK : ERROR;
}


GRAPH_BODY(fraction)
// ----------------------------------------------------------------------------
//   Render a fraction in graphical mode
// ----------------------------------------------------------------------------
{
    using font_id = settings::font_id;

    font_id font = g.font;
    if (Settings.SmallFractions())
        font = settings::smaller_font(font);
    save<font_id> fsave(g.font, font);
    fraction_g obj = o;

    // Render numerator and denominator
    bignum_g num = obj->numerator();
    bignum_g den = obj->denominator();
    if (!num || !den)
        return nullptr;

    if (num->is_negative())
        num = -num;

    // The denominator defines the overall width in the mixed case
    grob_g deng = den->graph(g);
    if (!deng)
        deng = object::do_graph(den, g);
    if (!deng)
        return nullptr;
    blitter::size dw = deng->width();

    grob_g ipart = nullptr;
    if (Settings.MixedFractions() && dw < g.maxw)
    {
        bignum_g quo, rem;
        if (bignum::quorem(num, den, bignum::ID_bignum, &quo, &rem))
        {
            if (!quo->is_zero())
            {
                save<blitter::size> wsave(g.maxw, g.maxw - dw);
                save<font_id> isave(g.font, fsave.saved);
                ipart = quo->graph(g);
                if (!ipart)
                    ipart = object::do_graph(quo, g);
                if (!ipart)
                    return nullptr;
                num = rem;
            }
        }
    }
    grob_g numg = num->graph(g);
    if (!numg)
        numg = object::do_graph(num, g);
    numg = expression::ratio(g, numg, deng);
    if (ipart && numg)
        numg = expression::prefix(g, 0, ipart, g.voffset, numg);
    if (obj->is_negative())
    {
        g.font = fsave.saved;
        numg = expression::prefix(g, 0, "-", g.voffset, numg);
    }
    return numg;
}


fraction_p fraction::make(integer_r n, integer_r d)
// ----------------------------------------------------------------------------
//   Create a reduced fraction from n and d
// ----------------------------------------------------------------------------
{
    ularge nv = n->value<ularge>();
    ularge dv = d->value<ularge>();
    ularge cd = gcd(nv, dv);
    bool neg = (n->type() == ID_neg_integer) != (d->type() == ID_neg_integer);
    id ty = neg ? ID_neg_fraction : ID_fraction;
    if (cd > 1)
    {
        integer_g nn = integer::make(nv / cd);
        integer_g dd = integer::make(dv / cd);
        return rt.make<fraction>(ty, nn, dd);
    }
    return rt.make<fraction>(ty, n, d);
}


bignum_p fraction::numerator() const
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


bignum_p fraction::denominator() const
// ----------------------------------------------------------------------------
//   Return the denominator as an integer (always positive)
// ----------------------------------------------------------------------------
{
    id ty = type();
    if (ty == ID_big_fraction || ty == ID_neg_big_fraction)
        return big_fraction_p(this)->denominator();

    byte_p p = payload();
    ularge nv = leb128<ularge>(p);
    ularge dv = leb128<ularge>(p) + 0 * nv;
    return rt.make<bignum>(ID_bignum, dv);
}


integer_p fraction::numerator(int) const
// ----------------------------------------------------------------------------
//   Return the numerator as an integer
// ----------------------------------------------------------------------------
{
    id ty = (type() == ID_neg_fraction) ? ID_neg_integer : ID_integer;
    return rt.make<integer>(ty, numerator_value());
}


integer_p fraction::denominator(int) const
// ----------------------------------------------------------------------------
//   Return the denominator as an integer (always positive)
// ----------------------------------------------------------------------------
{
    return rt.make<integer>(ID_integer, denominator_value());
}


ularge fraction::numerator_value() const
// ----------------------------------------------------------------------------
//   Return the numerator as a native number
// ----------------------------------------------------------------------------
{
    byte_p p = payload();
    ularge nv = leb128<ularge>(p);
    return nv;
}


ularge fraction::denominator_value() const
// ----------------------------------------------------------------------------
//   Return the denominator as a native number
// ----------------------------------------------------------------------------
{
    byte_p p = payload();
    leb128<ularge>(p);
    ularge dv = leb128<ularge>(p);
    return dv;
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


bignum_p big_fraction::numerator() const
// ------------------------------------------------------------------------
//   Return the numerator as a bignum
// ------------------------------------------------------------------------
{
    id ty = type() == ID_neg_big_fraction ? ID_neg_bignum : ID_bignum;
    byte_p p = payload();
    size_t ns = leb128<size_t>(p);
    return rt.make<bignum>(ty, p, ns);
}


bignum_p big_fraction::denominator() const
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


static inline bignum_p gcd(bignum_g a, bignum_g b)
// ----------------------------------------------------------------------------
//   Compute the greatest common denominator between a and b
// ----------------------------------------------------------------------------
{
    while (b && !b->is_zero())
    {
        bignum_g na = b;
        b = a % b;
        a = na;
    }
    return a;
}


fraction_p big_fraction::make(bignum_r nn, bignum_r dd)
// ----------------------------------------------------------------------------
//   Create a reduced fraction from n and d
// ----------------------------------------------------------------------------
{
    bignum_g n = nn;
    bignum_g d = dd;
    bignum_g cd = gcd(n, d);
    if (!cd)
        return nullptr;
    if (!cd->is(1))
    {
        n = n / cd;
        d = d / cd;
    }
    if (!n || !d)
        return nullptr;

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

template <fraction_p (*code)(fraction_r, fraction_r)>
algebraic_p fraction_wrapper(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Wrapper to silence warning about function casts in recent XCode
// ----------------------------------------------------------------------------
{
    fraction_r xb = (fraction_r) x;
    fraction_r yb = (fraction_r) y;
    fraction_p rb = code(xb, yb);
    return rb;
}


template <fraction_p (*code)(fraction_r, fraction_r)>
arithmetic_fn target(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//  Target function for fraction objects
// ----------------------------------------------------------------------------
{
    return x->is_fraction() && y->is_fraction()
        ? fraction_wrapper<code>
        : nullptr;
}


fraction_p operator-(fraction_r x)
// ----------------------------------------------------------------------------
//    Negation of a fraction
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    xn = -xn;
    return big_fraction::make(xn, xd);
}


fraction_p operator+(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Add two fractions
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    add::remember(target< operator+ >);
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    bignum_g  a = xn * yd;
    bignum_g  b = yn * xd;
    xn = a + b;
    xd = xd * yd;
    return big_fraction::make(xn, xd);
}


fraction_p operator-(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Subtract two fractions
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    subtract::remember(target< operator- >);
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    bignum_g  a = xn * yd;
    bignum_g  b = yn * xd;
    xn = a - b;
    xd = xd * yd;
    return big_fraction::make(xn, xd);
}


fraction_p operator*(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Multiply two fractions
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    multiply::remember(target< operator* >);
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    xn = xn * yn;
    xd = xd * yd;
    return big_fraction::make(xn, xd);
}


fraction_p operator/(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Divide two fractions
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    divide::remember(target< operator/ >);
    bignum_g  xn = x->numerator();
    bignum_g  xd = x->denominator();
    bignum_g  yn = y->numerator();
    bignum_g  yd = y->denominator();
    xn = xn * yd;
    xd = xd * yn;
    return big_fraction::make(xn, xd);
}


fraction_p operator%(fraction_r x, fraction_r y)
// ----------------------------------------------------------------------------
//    Compute the remainder of two fractions
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    rem::remember(target< operator% >);
    bignum_g   xn = x->numerator();
    bignum_g   xd = x->denominator();
    bignum_g   yn = y->numerator();
    bignum_g   yd = y->denominator();
    bignum_g   qn = xn * yd;
    bignum_g   qd = xd * yn;
    fraction_g q  = big_fraction::make(qn, qd);
    if (!q)
        return nullptr;
    qn = q->numerator();
    qd = q->denominator();
    bignum_g   ir = qn / qd;
    if (!ir)
        return nullptr;
    fraction_g fr = big_fraction::make(ir, bignum::make(1));
    fr            = fr * y;
    q             = x - fr;
    return q;
}
