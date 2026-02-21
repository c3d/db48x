// ****************************************************************************
//  ContinuedFraction.cc                                         DB48X project
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
//     Algorithm for integers and fractions (exact):
//       Standard Euclidean algorithm on the numerator and denominator.
//
//     Algorithm for decimals and other reals:
//       1. Split x into integer part ip (truncated toward zero) and fp = x - ip
//       2. For negative non-integer x, adjust so fp stays in [0, 1):
//            ip = floor(x) = truncate(x) - 1
//            fp = 1 + fractional_part   (which is 1 - |fractional_part|)
//       3. Append ip as a0.
//       4. Iterate: next = 1/fp, ip = floor(next), fp = next - ip.
//          Append ip as next coefficient.
//       5. Stop when fp is zero or when |fp| < 10^(-sdigs)
//          where sdigs = significant_digits(input).
//
// ****************************************************************************
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#include "continued-fraction.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "bignum.h"
#include "decimal.h"
#include "fraction.h"
#include "integer.h"
#include "list.h"

COMMAND_BODY(DFC)
// ----------------------------------------------------------------------------
//   Decompose a real number into its continued fraction coefficients
// ----------------------------------------------------------------------------
{
    // Fetch the input from the stack
    algebraic_g xo = algebraic_p(strip(rt.stack(0)));
    if (!xo)
        return ERROR;

    object::id ty = xo->type();

    // -----------------------------------------------------------------------
    // Fast path 1: integer input → { n }
    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    // Fast path 2: small fraction → exact Euclidean algorithm
    //   Only for ID_fraction / ID_neg_fraction (numerator and denominator
    //   both fit in ularge).  Big fractions fall through to the decimal path.
    // -----------------------------------------------------------------------
    if (ty == object::ID_fraction || ty == object::ID_neg_fraction)
    {
        fraction_p frac  = fraction_p(+xo);
        ularge     p     = frac->numerator_value();   // absolute value
        ularge     q     = frac->denominator_value(); // always positive
        bool       neg   = (ty == object::ID_neg_fraction);

        scribble scr;

        // First coefficient: floor(±p/q) with correct sign handling
        ularge    quot0 = p / q;
        ularge    rem0  = p % q;
        ularge    next_p, next_q;
        integer_g a0i;

        if (neg && rem0 != 0)
        {
            // floor(-p/q) = -(p/q + 1); fractional part = (q - rem0)/q
            a0i    = rt.make<neg_integer>(quot0 + 1);
            next_p = q - rem0;
            next_q = q;
        }
        else if (neg)
        {
            // Exact negative integer: floor(-p/q) = -(p/q), fp = 0
            a0i    = rt.make<neg_integer>(quot0);
            next_p = 0;
            next_q = 1;
        }
        else
        {
            // Non-negative: floor(p/q) = p/q
            a0i    = rt.make<integer>(quot0);
            next_p = rem0;
            next_q = q;
        }
        if (!a0i || !rt.append(a0i))
            return ERROR;

        // Remaining coefficients via the Euclidean algorithm.
        // The fractional part is next_p/next_q; its reciprocal is next_q/next_p.
        p = next_q;
        q = next_p;
        while (q != 0)
        {
            ularge    a_val = p / q;
            ularge    r     = p % q;
            integer_g ai    = rt.make<integer>(a_val);
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

    // -----------------------------------------------------------------------
    // Fast path 3: decimal input → exact Euclidean algorithm
    //
    //   The floating-point iterative algorithm is numerically unstable for
    //   slowly-converging CFs (e.g. sqrt(2) = [1;2,2,2,…]) because the CF
    //   map x→1/x−floor(1/x) amplifies errors by 1/fp² ≈ 5.83 per step.
    //   After about 30 steps with 34-digit arithmetic the result is garbage.
    //
    //   Instead we convert the decimal exactly to a bignum fraction
    //     value = mantissa_integer × 10^(exponent − 3×nkigits)
    //   and apply the same Euclidean GCD algorithm used for ID_fraction.
    //   This gives a perfectly stable, terminating CF with zero rounding
    //   error, so DFC2F(DFC(x)) == x for any decimal x.
    // -----------------------------------------------------------------------
    if (ty == object::ID_decimal || ty == object::ID_neg_decimal)
    {
        decimal_g dec = decimal_p(+xo);
        decimal::info xi = (+dec)->shape();
        size_t xs  = xi.nkigits;
        large  xe  = xi.exponent;
        bool   neg = (ty == object::ID_neg_decimal);

        // Build the mantissa integer: mant = kigit(0)×1000^(xs-1) + … + kigit(xs-1)
        // kigit(0) is the most-significant (value × 10^xe).
        // We re-fetch shape() inside the loop because bignum allocations can
        // trigger GC, which may move the decimal object (dec is GC-tracked,
        // so +dec is always updated; xi.base would become stale after a GC).
        bignum_g mant  = bignum::make(0);
        bignum_g k1000 = bignum::make(1000);
        bignum_g k10   = bignum::make(10);
        for (size_t i = 0; i < xs; i++)
        {
            decimal::info    xi_now = (+dec)->shape();
            decimal::kint    xk     = decimal::kigit(xi_now.base, i);
            bignum_g         kbig   = bignum::make(xk);
            bignum_g         prod   = mant * k1000;
            mant = prod + kbig;
        }

        // Exact value = mant / 10^(3×xs − xe)   (denominator side)
        //             = mant × 10^(xe − 3×xs)    (numerator side, if xe > 3×xs)
        bignum_g p, q;
        large denom_exp = (large)(3 * xs) - xe; // positive → denominator = 10^denom_exp
        if (denom_exp > 0)
        {
            q = bignum::make(1);
            for (large i = 0; i < denom_exp; i++)
                q = q * k10;
            p = mant;
        }
        else
        {
            p = mant;
            for (large i = 0; i < -denom_exp; i++)
                p = p * k10;
            q = bignum::make(1);
        }

        // First coefficient: floor(±p/q) with correct sign handling
        scribble scr;
        bignum_g  quot   = p / q;
        bignum_g  rem    = p % q;
        algebraic_g a0;
        bignum_g next_p, next_q;

        if (neg && !rem->is_zero())
        {
            // floor(-p/q) = -(quot+1),  fractional part = (q−rem)/q
            bignum_g one1  = bignum::make(1);
            bignum_g quot1 = quot + one1;
            bignum_g neg_q1 = -quot1;
            a0     = algebraic_p(+neg_q1);
            next_p = q - rem;
            next_q = q;
        }
        else if (neg)
        {
            // Exact negative integer
            bignum_g neg_q = -quot;
            a0     = algebraic_p(+neg_q);
            next_p = bignum::make(0);
            next_q = bignum::make(1);
        }
        else
        {
            a0     = algebraic_p(+quot);
            next_p = rem;
            next_q = q;
        }
        if (!a0 || !rt.append(a0))
            return ERROR;

        // Remaining coefficients: Euclidean GCD on (next_q, next_p).
        // Track convergent denominators Q_prev/Q_curr: stop when Q_curr
        // exceeds 10^(denom_exp/2).  The CF of the decimal rational p/q
        // agrees with the CF of the true value x only while
        //   Q_k < sqrt(q)  =  10^(denom_exp/2),
        // beyond that the coefficients are garbage (specific to the finite
        // decimal approximation).  Using denom_exp/2, not 3*xs/2, correctly
        // accounts for the actual denominator exponent (xe shifts the boundary).
        bignum_g Q_prev = bignum::make(0);   // Q_{-1} = 0
        bignum_g Q_curr = bignum::make(1);   // Q_0 = 1
        bignum_g thresh = bignum::make(1);
        size_t   tlim   = (denom_exp > 0) ? (size_t)(denom_exp / 2) : 0;
        for (size_t ti = 0; ti < tlim; ti++)
            thresh = thresh * k10;

        p = next_q;
        q = next_p;
        while (!q->is_zero())
        {
            bignum_g ai_big = p / q;
            bignum_g r      = p % q;
            algebraic_g ai  = algebraic_p(+ai_big);
            if (!ai || !rt.append(ai))
                return ERROR;
            // Q_{k+1} = a_k * Q_k + Q_{k-1}
            bignum_g tmp   = ai_big * Q_curr;
            bignum_g Q_new = tmp + Q_prev;
            Q_prev = Q_curr;
            Q_curr = Q_new;
            if (Q_curr > thresh)
                break;
            p = q;
            q = r;
        }

        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // -----------------------------------------------------------------------
    // General path: convert to decimal, then use the exact Euclidean algorithm.
    // This handles symbolic constants (pi, e, …) and any other type that can
    // be evaluated numerically.  Converting to decimal first, then applying
    // the exact bignum Euclidean algorithm, avoids the floating-point
    // instability of the iterative 1/fp approach.
    // -----------------------------------------------------------------------
    if (!algebraic::to_decimal(xo))
    {
        rt.type_error();
        return ERROR;
    }

    // Re-read the type and fall through to the exact decimal path below.
    // We reuse fast path 3's logic directly on the converted decimal.
    ty  = xo->type();
    xo  = algebraic_p(+xo);

    // Intentional fall-through: the decimal case (fast path 3) handles this.
    // For clarity the code is repeated here rather than introducing a goto.
    {
        decimal_g dec = decimal_p(+xo);
        decimal::info xi = (+dec)->shape();
        size_t xs  = xi.nkigits;
        large  xe  = xi.exponent;
        bool   neg = (ty == object::ID_neg_decimal);

        bignum_g mant  = bignum::make(0);
        bignum_g k1000 = bignum::make(1000);
        bignum_g k10   = bignum::make(10);
        for (size_t i = 0; i < xs; i++)
        {
            decimal::info    xi_now = (+dec)->shape();
            decimal::kint    xk     = decimal::kigit(xi_now.base, i);
            bignum_g         kbig   = bignum::make(xk);
            bignum_g         prod   = mant * k1000;
            mant = prod + kbig;
        }

        bignum_g p, q;
        large denom_exp = (large)(3 * xs) - xe;
        if (denom_exp > 0)
        {
            q = bignum::make(1);
            for (large i = 0; i < denom_exp; i++)
                q = q * k10;
            p = mant;
        }
        else
        {
            p = mant;
            for (large i = 0; i < -denom_exp; i++)
                p = p * k10;
            q = bignum::make(1);
        }

        scribble scr;
        bignum_g  quot   = p / q;
        bignum_g  rem    = p % q;
        algebraic_g a0;
        bignum_g next_p, next_q;

        if (neg && !rem->is_zero())
        {
            bignum_g one1   = bignum::make(1);
            bignum_g quot1  = quot + one1;
            bignum_g neg_q1 = -quot1;
            a0     = algebraic_p(+neg_q1);
            next_p = q - rem;
            next_q = q;
        }
        else if (neg)
        {
            bignum_g neg_q = -quot;
            a0     = algebraic_p(+neg_q);
            next_p = bignum::make(0);
            next_q = bignum::make(1);
        }
        else
        {
            a0     = algebraic_p(+quot);
            next_p = rem;
            next_q = q;
        }
        if (!a0 || !rt.append(a0))
            return ERROR;

        // Same precision-limited stopping criterion as fast path 3:
        // stop when Q_curr > 10^(denom_exp/2) = sqrt(denominator).
        bignum_g Q_prev = bignum::make(0);
        bignum_g Q_curr = bignum::make(1);
        bignum_g thresh = bignum::make(1);
        size_t   tlim   = (denom_exp > 0) ? (size_t)(denom_exp / 2) : 0;
        for (size_t ti = 0; ti < tlim; ti++)
            thresh = thresh * k10;

        p = next_q;
        q = next_p;
        while (!q->is_zero())
        {
            bignum_g ai_big = p / q;
            bignum_g r      = p % q;
            algebraic_g ai  = algebraic_p(+ai_big);
            if (!ai || !rt.append(ai))
                return ERROR;
            bignum_g tmp   = ai_big * Q_curr;
            bignum_g Q_new = tmp + Q_prev;
            Q_prev = Q_curr;
            Q_curr = Q_new;
            if (Q_curr > thresh)
                break;
            p = q;
            q = r;
        }

        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }
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
// ----------------------------------------------------------------------------
{
    // Fetch the list from the stack
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

    // Start with the last coefficient
    algebraic_g x = algebraic_p(lst->at(n - 1));
    if (!x)
        return ERROR;

    // Process right to left: x = a_i + 1/x
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
