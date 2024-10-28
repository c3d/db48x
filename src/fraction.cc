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
    ularge nv = leb128<ularge>(p);
    ularge dv = leb128<ularge>(p) + 0 * nv;
    return rt.make<bignum>(ID_bignum, dv);
}


integer_g fraction::numerator(int) const
// ----------------------------------------------------------------------------
//   Return the numerator as an integer
// ----------------------------------------------------------------------------
{
    id ty = (type() == ID_neg_fraction) ? ID_neg_integer : ID_integer;
    return rt.make<integer>(ty, numerator_value());
}


integer_g fraction::denominator(int) const
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
    while (b && !b->is_zero())
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
