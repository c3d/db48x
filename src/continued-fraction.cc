// ****************************************************************************
//  continued-fraction.cc                                         DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Continued fraction decomposition of a real number
//
//     DFC decomposes a real number x into a continued fraction:
//       x = a0 + 1/(a1 + 1/(a2 + 1/...))
//     and returns the list of coefficients [a0, a1, a2, ...].
//
//
//
//
//
// ****************************************************************************
//   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
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
//     Algorithm for integers:   trivial – one-element list.
//     Algorithm for fractions:  exact Euclidean algorithm on num/denominator.
//     Algorithm for decimals:   exact bignum Euclidean algorithm.
//       The decimal x is converted to an exact rational mant/10^denom_exp,
//       and the standard Euclidean GCD algorithm is applied.  Iteration stops
//       (before appending) when the next convergent denominator Q_{k+1} would
//       exceed 10^(eff_exp/2), where eff_exp = significant_digits(x) − xe is
//       the effective denominator exponent.  Coefficients beyond that threshold
//       belong to the finite decimal approximation, not the true mathematical
//       value, and are discarded.

#include "continued-fraction.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "bignum.h"
#include "decimal.h"
#include "fraction.h"
#include "integer.h"
#include "list.h"


static algebraic_p bignum_to_algebraic(bignum_r n)
// ----------------------------------------------------------------------------
//   Convert a bignum coefficient to integer when it fits, bignum otherwise.
// ----------------------------------------------------------------------------
//   Required because == does byte-level comparison: bignum(2) ≠ integer(2).
{
    if (!n)
        return nullptr;
    if (integer_p small = n->as_integer())
        return algebraic_p(small);
    return algebraic_p(+n);
}


static object::result dfc_decimal(decimal_r dec, bool neg)
// ----------------------------------------------------------------------------
//  Core DFC algorithm for a decimal value
// ----------------------------------------------------------------------------
{
    if (!dec)
        return object::ERROR;
    // Build the mantissa integer from kigits.
    // Re-fetch shape() each iteration: bignum allocations can trigger GC and
    // move the decimal object (dec is a GC root so +dec stays valid, but a
    // locally cached xi.base pointer would go stale).
    decimal::info xi    = dec->shape();
    size_t        xs    = xi.nkigits;
    large         xe    = xi.exponent;
    bignum_g      mant  = bignum::make(0);
    bignum_g      k1000 = bignum::make(1000);
    bignum_g      k10   = bignum::make(10);
    bignum_g      one   = bignum::make(1);
    if (!mant || !k1000 || !k10 || !one)
        return object::ERROR;
    for (size_t i = 0; i < xs; i++)
    {
        decimal::info xi_now = dec->shape();
        bignum_g      kbig   = bignum::make(decimal::kigit(xi_now.base, i));
        bignum_g      prod   = mant * k1000;
        mant = prod + kbig;
        if (!kbig || !prod || !mant)
            return object::ERROR;
    }

    // Represent value as exact rational mant / 10^denom_exp.
    bignum_g p, q;
    large    denom_exp = (large) (3 * xs) - xe;
    if (denom_exp < 0)
    {
        p = bignum::pow(k10, -denom_exp);
        p = mant * p;
        q = one;
    }
    else
    {
        p = mant;
        q = bignum::pow(k10, denom_exp);
    }
    if (!p || !q || q->is_zero())
        return object::ERROR;

    // First coefficient a0 = floor(±p/q), keeping fp = fractional part ≥ 0.
    bignum_g quot, rem;
    if (!bignum::quorem(p, q, bignum::ID_bignum, &quot, &rem))
        return object::ERROR;
    algebraic_g a0;
    bignum_g    next_p, next_q;
    if (neg && !rem->is_zero())
    {
        // floor(-p/q) = −(quot+1);  fractional part = (q − rem)/q
        bignum_g quot1 = quot + one;
        bignum_g neg_a = -quot1;
        if (!one || !quot1 || !neg_a)
            return object::ERROR;
        a0     = bignum_to_algebraic(neg_a);
        next_p = q - rem;
        next_q = q;
        if (!next_p || !next_q)
            return object::ERROR;
    }
    else if (neg)
    {
        // Exact negative integer
        bignum_g neg_a = -quot;
        next_p = bignum::make(0);
        next_q = bignum::make(1);
        if (!neg_a || !next_p || !next_q)
            return object::ERROR;
        a0     = bignum_to_algebraic(neg_a);
    }
    else
    {
        a0     = bignum_to_algebraic(quot);
        next_p = rem;
        next_q = q;
    }
    if (!a0 || !rt.append(a0))
        return object::ERROR;

    // Stopping threshold: 10^(eff_exp/2).
    // eff_exp = significant_digits(x) − xe is the exponent of the effective
    // denominator (after cancelling trailing kigit-padding zeros from the
    // mantissa).  When precision P is not a multiple of 3, kigit alignment
    // adds trailing zeros, making the kigit-based denom_exp too large; using
    // significant_digits() corrects this and prevents garbage coefficients.
    large    eff_exp = (large) dec->significant_digits() - xe;
    size_t   tlim    = (eff_exp > 0) ? (size_t)(eff_exp / 2) : 0;
    bignum_g thresh  = bignum::pow(k10, tlim);
    if (!thresh)
        return object::ERROR;

    // Euclidean loop.  Q_prev = Q_{k-1}, Q_curr = Q_k are the convergent
    // denominators.  Stop BEFORE appending a_k when Q_{k+1} > thresh: such
    // coefficients belong to the finite decimal approximation, not to the
    // true mathematical value.
    bignum_g Q_prev = bignum::make(0);
    bignum_g Q_curr = bignum::make(1);
    if (!Q_prev || !Q_curr)
        return object::ERROR;
    p = next_q;
    q = next_p;
    while (q && !q->is_zero())
    {
        bignum_g ai_big, r;
        if (!bignum::quorem(p, q, bignum::ID_bignum, &ai_big, &r))
            return object::ERROR;
        if (!ai_big || !r)
            return object::ERROR;
        bignum_g tmp    = ai_big * Q_curr;
        bignum_g Q_new  = tmp + Q_prev;
        if (!tmp || !Q_new)
            return object::ERROR;
        if (Q_new > thresh)
            break;
        algebraic_g ai = bignum_to_algebraic(ai_big);
        if (!ai || !rt.append(ai))
            return object::ERROR;
        Q_prev = Q_curr;
        Q_curr = Q_new;
        p = q;
        q = r;
    }
    return object::OK;
}


COMMAND_BODY(DFC)
// ----------------------------------------------------------------------------
//   Decompose a real number into its continued fraction coefficients
// ----------------------------------------------------------------------------
{
    algebraic_g xo = algebraic_p(strip(rt.stack(0)));
    if (!xo)
        return ERROR;
    object::id ty = xo->type();

    // Fast path 1: integer input → { n }
    if (object::is_integer(ty))
    {
        scribble scr;
        if (!rt.append(xo))
            return ERROR;
        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // Fast path 2: small fraction → exact Euclidean (native integer arithmetic)
    //   Only for ID_fraction / ID_neg_fraction (num and denom fit in ularge).
    //   Big fractions fall through to the decimal path below.
    if (ty == object::ID_fraction || ty == object::ID_neg_fraction)
    {
        fraction_p frac   = fraction_p(+xo);
        ularge     p      = frac->numerator_value();
        ularge     q      = frac->denominator_value();
        bool       neg    = (ty == object::ID_neg_fraction);
        scribble   scr;

        ularge    quot  = p / q;
        ularge    rem   = p % q;
        ularge    next_p, next_q;
        integer_g a0i;
        if (neg && rem != 0)
        {
            // floor(−p/q) = −(quot+1);  fractional part = (q − rem)/q
            a0i    = rt.make<neg_integer>(quot + 1);
            next_p = q - rem;
            next_q = q;
        }
        else if (neg)
        {
            a0i    = rt.make<neg_integer>(quot);
            next_p = 0;
            next_q = 1;
        }
        else
        {
            a0i    = rt.make<integer>(quot);
            next_p = rem;
            next_q = q;
        }
        if (!a0i || !rt.append(a0i))
            return ERROR;

        // Remaining coefficients: reciprocal swap then standard GCD steps.
        p = next_q;
        q = next_p;
        while (q != 0)
        {
            ularge    a  = p / q;
            ularge    r  = p % q;
            integer_g ai = rt.make<integer>(a);
            if (!ai || !rt.append(ai))
                return ERROR;
            p = q;
            q = r;
        }
        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // Decimal and general path: ensure we have a decimal, then apply the
    // exact bignum Euclidean algorithm via dfc_decimal().
    //
    //   The floating-point iterative approach (x → 1/fp − floor(1/fp)) is
    //   numerically unstable: the CF map amplifies errors by 1/fp² ≈ 5.83
    //   per step, so after ~30 steps with 34-digit arithmetic the result is
    //   garbage.  Converting to an exact bignum rational first avoids this.
    if (ty != object::ID_decimal && ty != object::ID_neg_decimal)
    {
        if (!algebraic::to_decimal(xo))
        {
            rt.type_error();
            return ERROR;
        }
        ty = xo->type();
        xo = algebraic_p(+xo);
    }

    scribble  scr;
    decimal_g dec = decimal_p(+xo);
    if (!dec)
        return ERROR;
    result r = dfc_decimal(dec, ty == object::ID_neg_decimal);
    if (r != OK)
        return r;
    list_g lst = list::make(scr.scratch(), scr.growth());
    if (!lst || !rt.top(lst))
        return ERROR;
    return OK;
}


COMMAND_BODY(DFC2F)
// ----------------------------------------------------------------------------
//   Reconstruct a real number from its continued fraction coefficient list
// ----------------------------------------------------------------------------
//   Evaluates { a0, a1, ..., an } right-to-left:
//     x = an
//     x = a_{n-1} + 1/x
//     ...
//     x = a0 + 1/x
{
    object_p obj = strip(rt.stack(0));
    if (!obj)
        return ERROR;
    if (obj->type() != object::ID_list)
    {
        rt.type_error();
        return ERROR;
    }
    list_g lst = list_g(list_p(obj));
    size_t n   = lst->items();
    if (n == 0)
    {
        rt.error("Empty list");
        return ERROR;
    }

    algebraic_g x = algebraic_p(lst->at(n - 1));
    if (!x)
        return ERROR;

    algebraic_g one = integer::make(1);
    if (!one)
        return ERROR;

    for (size_t i = n - 1; i-- > 0;)
    {
        algebraic_g ai    = algebraic_p(lst->at(i));
        algebraic_g recip = divide::evaluate(one, x);
        if (!ai || !recip)
            return ERROR;
        x = add::evaluate(ai, recip);
        if (!x)
            return ERROR;
    }

    if (!rt.top(x))
        return ERROR;
    return OK;
}
