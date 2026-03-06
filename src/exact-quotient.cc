// ****************************************************************************
//  exact-quotient.cc                                             DB48X project
// ****************************************************************************
//
//   File Description:
//
//     XQ: find the simplest symbolic representation of a real number.
//
//     Algorithm ported and adapted from QPI 4.3 for the HP 48G/GX
//     by Mika Heiskanen & Andre Schoorl, re-implemented by Han Duong
//     for the HP Prime, then ported to DB48X.
//
//     Given a decimal number, XQ tries to express it as one of:
//       1. p/q              — rational fraction
//       2. a/b × √(c/d)    — rational multiple of a square root
//       3. p/q × π         — rational multiple of pi
//       4. ln(p/q)         — natural log of a rational
//       5. e^(p/q)         — exponential of a rational
//     The representation with the smallest denominator that matches
//     the input within the current precision is returned.
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

#include "exact-quotient.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "bignum.h"
#include "constants.h"
#include "decimal.h"
#include "expression.h"
#include "fraction.h"
#include "functions.h"
#include "integer.h"
#include "list.h"
#include "settings.h"


// ============================================================================
//
//   Rational approximation via continued fractions
//
// ============================================================================

static bool xq_approx(decimal_r dec, bignum_g &p_out, bignum_g &q_out)
// ----------------------------------------------------------------------------
//   Find best rational p/q ≈ abs(dec) within current precision.
//   Delegates to decimal::to_fraction with precision-based stopping criterion,
//   then extracts p (numerator) and q (denominator) as bignums.
// ----------------------------------------------------------------------------
{
    if (!dec)
        return false;

    // Special case: zero.
    if (dec->is_zero())
    {
        p_out = bignum::make(0);
        q_out = bignum::make(1);
        return p_out && q_out;
    }

    uint prec = Settings.Precision();
    uint tol  = (prec >= 4) ? prec - 4 : 0;

    // to_fraction(count, decimals): runs CF, stops when |remainder| < 10^(-decimals).
    // We allow up to prec iterations and stop when the residual is below our
    // accuracy floor, matching the precision-based threshold used for XQ.
    algebraic_g result = dec->to_fraction(prec, tol);
    if (!result)
        return false;

    bignum_g one = bignum::make(1);
    if (!one)
        return false;

    object::id ty = result->type();
    if (ty == object::ID_big_fraction)
    {
        big_fraction_p bf = big_fraction_p(+result);
        p_out = bf->numerator();
        q_out = bf->denominator();
    }
    else if (ty == object::ID_fraction)
    {
        fraction_p f = fraction_p(+result);
        p_out = f->numerator();    // bignum_p overload
        q_out = f->denominator();  // bignum_p overload
    }
    else if (object::is_bignum(ty))
    {
        p_out = bignum_p(+result);
        q_out = one;
    }
    else if (object::is_integer(ty))
    {
        p_out = bignum::make(integer_p(+result)->value<ularge>());
        q_out = one;
    }
    else
    {
        return false;
    }

    return p_out && q_out;
}


static bool xq_approx_of(decimal_p d, bignum_g &p, bignum_g &q)
// ----------------------------------------------------------------------------
//   Approx of absolute value of d. Returns false on failure.
// ----------------------------------------------------------------------------
{
    if (!d)
        return false;
    decimal_g abs_d = decimal::abs(d);
    if (!abs_d)
        return false;
    return xq_approx(abs_d, p, q);
}


// ============================================================================
//
//   Helper: build algebraic from bignums
//
// ============================================================================

static algebraic_p make_number(bignum_r p, bignum_r q)
// ----------------------------------------------------------------------------
//   Build algebraic p/q from two non-negative bignums (p ≥ 0, q ≥ 1).
//   Returns integer when q==1, fraction otherwise.
// ----------------------------------------------------------------------------
{
    if (!p || !q)
        return nullptr;
    if (q->is_one())
    {
        if (integer_p i = p->as_integer())
            return algebraic_p(i);
        return algebraic_p(+p);
    }
    return algebraic_p(big_fraction::make(p, q));
}


static algebraic_p make_neg_number(bignum_r p, bignum_r q)
// ----------------------------------------------------------------------------
//   Build algebraic -(p/q) from non-negative bignums.
// ----------------------------------------------------------------------------
{
    if (!p || !q)
        return nullptr;
    bignum_g np = bignum_p(-p);
    if (!np)
        return nullptr;
    if (q->is_one())
    {
        if (integer_p i = np->as_integer())
            return algebraic_p(i);
        return algebraic_p(+np);
    }
    return algebraic_p(big_fraction::make(np, q));
}


// ============================================================================
//
//   Perfect-square factoring: n = a^2 * b (b square-free)
//   Ported from qpi_asqrtb() in Han Duong's HP Prime QPI.
//
// ============================================================================

static bool xq_asqrtb(bignum_r n, bignum_g &a_out, bignum_g &b_out)
// ----------------------------------------------------------------------------
//   Factor n = a² × b where b is square-free, so √n = a × √b.
//   Returns {1, n} when n is too large to factor efficiently.
// ----------------------------------------------------------------------------
{
    if (!n)
        return false;

    // Only bother for small n; beyond ~10^12 the expression is too ugly anyway.
    if (n->more_bits_than(42)) // 2^42 ≈ 4×10^12
    {
        a_out = bignum::make(1);
        b_out = +n;
        return a_out && b_out;
    }

    bignum_g a   = bignum::make(1);
    bignum_g rem = +n;
    bignum_g one = bignum::make(1);
    bignum_g two = bignum::make(2);
    if (!a || !rem || !one || !two)
        return false;

    // Try divisors k² = 4, 9, 16, 25, ...  (k = 2, 3, 4, 5, ...)
    // encoded as: den=4, nodd=3; increment nodd by 2 each time den fails,
    // den += nodd after increment (so den = 4, 9, 16, 25, ...).
    bignum_g nodd = bignum::make(3);
    bignum_g den  = bignum::make(4);
    if (!nodd || !den)
        return false;

    while (true)
    {
        bignum_g quo, r;
        if (!bignum::quorem(rem, den, bignum::ID_bignum, &quo, &r))
            break;

        if (r->is_zero())
        {
            // rem is divisible by den = k²; extract factor k.
            // k = IP(nodd/2) + 1  (nodd = 2k-1, so IP((2k-1)/2) + 1 = k)
            bignum_g half, ignore;
            if (!bignum::quorem(nodd, two, bignum::ID_bignum, &half, &ignore))
                break;
            bignum_g k   = half + one;
            bignum_g atmp = a * k;
            if (!k || !atmp)
                break;
            a   = atmp;
            rem = quo;
            // Do NOT advance nodd/den: try the same divisor again.
        }
        else
        {
            // Not divisible: advance to next square.
            bignum_g nodd2 = nodd + two;
            bignum_g den2  = den + nodd2;
            if (!nodd2 || !den2)
                break;
            nodd = nodd2;
            den  = den2;
        }

        // Stop when quotient is 0 (rem < den) or den got huge.
        if (quo->is_zero() || den->more_bits_than(42))
            break;
    }

    a_out = a;
    b_out = rem;
    return a_out && b_out;
}


// ============================================================================
//
//   Expression builders
//
// ============================================================================

static algebraic_p build_sqrt(bignum_r p, bignum_r q, bool neg)
// ----------------------------------------------------------------------------
//   Build ±(a1/a2)*√(b1/b2) where p = a1²*b1, q = a2²*b2.
//   p and q are the numerator and denominator of r² approximation.
// ----------------------------------------------------------------------------
{
    bignum_g a1, b1, a2, b2;
    if (!xq_asqrtb(p, a1, b1) || !xq_asqrtb(q, a2, b2))
        return nullptr;

    // Inner argument: b1/b2 (auto-reduced by big_fraction::make).
    algebraic_g inner = make_number(b1, b2);
    if (!inner)
        return nullptr;

    // Sqrt expression: √(b1/b2).
    algebraic_g sqrtexpr = algebraic_p(
        expression::make(object::ID_sqrt, inner));
    if (!sqrtexpr)
        return nullptr;

    // Rational prefix (a1/a2); omit when it equals 1 (a1 == a2).
    algebraic_g result;
    if (bignum::compare(a1, a2) == 0)
    {
        // a1 == a2: prefix is 1, just return ±√(b1/b2).
        result = sqrtexpr;
    }
    else if (a1->is_one())
    {
        // a1 == 1: build √(b1/b2) ÷ a2 (HP-style: √p÷q rather than (1/q)·√p).
        bignum_g one = bignum::make(1);
        algebraic_g denom = make_number(a2, one);
        if (!denom)
            return nullptr;
        result = algebraic_p(
            expression::make(object::ID_divide, sqrtexpr, denom));
    }
    else
    {
        algebraic_g coeff = make_number(a1, a2);
        if (!coeff)
            return nullptr;
        result = algebraic_p(
            expression::make(object::ID_multiply, coeff, sqrtexpr));
    }

    if (!result)
        return nullptr;

    if (neg)
        result = algebraic_p(expression::make(object::ID_neg, result));

    return +result;
}


static algebraic_p build_pi_mult(bignum_r p, bignum_r q, bool neg)
// ----------------------------------------------------------------------------
//   Build ±(p/q)*π.  p and q must be positive (sign handled via neg).
// ----------------------------------------------------------------------------
{
    algebraic_g pi = constant::lookup("π");
    if (!pi)
        return nullptr;

    if (q->is_one() && p->is_one())
    {
        // Just ±π.
        if (neg)
            return algebraic_p(expression::make(object::ID_neg, pi));
        return +pi;
    }

    algebraic_g coeff = neg ? make_neg_number(p, q) : make_number(p, q);
    if (!coeff)
        return nullptr;

    return algebraic_p(expression::make(object::ID_multiply, coeff, pi));
}


static algebraic_p build_ln(bignum_r p, bignum_r q)
// ----------------------------------------------------------------------------
//   Build LN(p/q).  Sign is implicit: if p/q < 1 the result is negative.
// ----------------------------------------------------------------------------
{
    algebraic_g arg = make_number(p, q);
    if (!arg)
        return nullptr;
    return algebraic_p(expression::make(object::ID_ln, arg));
}


static algebraic_p build_exp(bignum_r p, bignum_r q)
// ----------------------------------------------------------------------------
//   Build e^(p/q).
// ----------------------------------------------------------------------------
{
    algebraic_g arg = make_number(p, q);
    if (!arg)
        return nullptr;
    return algebraic_p(expression::make(object::ID_exp, arg));
}


// ============================================================================
//
//   XQ main logic — ported from Han Duong's HP Prime QPI algorithm
//
// ============================================================================

// Threshold constants (matching HP Prime QPI behaviour):
static const ularge XQ_DIRECT  = 100;       // Output directly as rational if q < this
static const ularge XQ_TRYALT  = 10000000;  // Try alternatives if alt_q < this
static const ularge XQ_DIRECT2 = 10;        // Output alternative directly if q < this
static const ularge XQ_EXPLN   = 100;       // Max denominator for LN/EXP forms

static bool q_fits(bignum_r q, ularge limit)
// ----------------------------------------------------------------------------
//   True if q < limit (limit fits in a single ularge word).
// ----------------------------------------------------------------------------
{
    return !q->more_bits_than(63) && q->value<ularge>() < limit;
}

static bool q_le(bignum_r q1, bignum_r q2)
// ----------------------------------------------------------------------------
//   True if q1 <= q2 (both non-negative bignums).
// ----------------------------------------------------------------------------
{
    return !(q1 > q2);
}


static algebraic_p xq_decimal(decimal_r dec, bool neg, bool pi_only)
// ----------------------------------------------------------------------------
//   Core XQ algorithm.  dec is the absolute value of the input (no sign info).
//   neg carries the sign of the original input.
//   pi_only restricts output to pi-multiple and rational forms only (QPI).
// ----------------------------------------------------------------------------
{
    // Step 1: rational approximation of dec (= |r|).
    bignum_g p, q;
    if (!xq_approx(dec, p, q))
        return nullptr;

    // Step 1a: if q is tiny, return p/q directly (unless pi_only).
    if (!pi_only && q_fits(q, XQ_DIRECT))
    {
        algebraic_g frac = neg ? make_neg_number(p, q) : make_number(p, q);
        return +frac;
    }

    // Working "best denominator" tracks the best alternative found so far.
    bignum_g    best_q      = q;
    algebraic_g sqrt_result;
    algebraic_g pi_result;
    algebraic_g ln_result;
    algebraic_g exp_result;

    // ---- Step 2: try r² → √ form ----------------------------------------
    if (!pi_only)
    {
        decimal_g r2 = decimal::multiply(dec, dec);
        bignum_g  sq_p, sq_q;
        if (r2 && xq_approx_of(r2, sq_p, sq_q))
        {
            if (q_fits(sq_q, XQ_TRYALT) && q_le(sq_q, best_q))
            {
                if (q_fits(sq_q, XQ_DIRECT2))
                    return build_sqrt(sq_p, sq_q, neg);
                // Potentially nice: remember and keep trying.
                best_q      = sq_q;
                sqrt_result = build_sqrt(sq_p, sq_q, neg);
            }
        }
    }

    // ---- Step 3: try r/π → (p/q)*π form ---------------------------------
    {
        decimal_g pi_dec    = decimal::pi();
        decimal_g r_ov_pi  = pi_dec ? decimal::divide(dec, pi_dec) : decimal_p(nullptr);

        bignum_g pi_p, pi_q;
        // is_magnitude_less_than(100, 3): checks |value| < 100/1000 × 10^3 = 100
        if (r_ov_pi && r_ov_pi->is_magnitude_less_than(100, 3)
                    && xq_approx_of(r_ov_pi, pi_p, pi_q))
        {
            if (q_fits(pi_q, XQ_TRYALT) && q_le(pi_q, best_q))
            {
                if (q_fits(pi_q, XQ_DIRECT2))
                    return build_pi_mult(pi_p, pi_q, neg);
                best_q    = pi_q;
                pi_result = build_pi_mult(pi_p, pi_q, neg);
            }
        }
    }

    if (pi_only)
    {
        // QPI: only pi forms are allowed; fall back to rational otherwise.
        if (pi_result)
            return +pi_result;
        algebraic_g frac = neg ? make_neg_number(p, q) : make_number(p, q);
        return +frac;
    }

    // ---- Step 4: try r = ±LN(p/q) → check e^|r| -----------------------
    // Use e^|r| (absolute value): for r = -ln(q), e^|r| = q, giving build_ln(q,1).
    // The sign is handled by negating the ln expression when neg=true.
    // is_magnitude_less_than(100, 4): checks |value| < 100/1000 × 10^4 = 1000
    {
        decimal_g er = decimal::exp(dec);  // e^|r|
        bignum_g  ln_p, ln_q;
        if (er && er->is_magnitude_less_than(100, 4)
               && xq_approx_of(er, ln_p, ln_q))
        {
            // Exclude trivial case: e^|r| ≈ 1 (r ≈ 0, already handled above).
            bool trivial = ln_p->is_one() && ln_q->is_one();
            if (!trivial && q_fits(ln_q, XQ_EXPLN) && q_le(ln_q, best_q))
            {
                algebraic_g ln_expr = build_ln(ln_p, ln_q);
                if (neg && ln_expr)
                    ln_expr = algebraic_p(
                        expression::make(object::ID_neg, ln_expr));
                if (q_fits(ln_q, XQ_DIRECT2))
                    return +ln_expr;
                best_q    = ln_q;
                ln_result = ln_expr;
            }
        }
    }

    // ---- Step 5: try r = e^(p/q) → check LN(r) -------------------------
    if (!neg) // LN is only defined for r > 0.
    {
        decimal_g lnr = decimal::ln(dec);
        bignum_g  exp_p, exp_q;
        if (lnr && xq_approx_of(lnr, exp_p, exp_q))
        {
            if (q_fits(exp_q, XQ_EXPLN) && q_le(exp_q, best_q))
                exp_result = build_exp(exp_p, exp_q);
        }
    }

    // ---- Step 6: return the best result found ---------------------------
    // Priority: exp > ln > pi > sqrt > rational.
    if (exp_result)
        return +exp_result;
    if (ln_result)
        return +ln_result;
    if (pi_result)
        return +pi_result;
    if (sqrt_result)
        return +sqrt_result;

    // Fallback: return the rational approximation.
    algebraic_g frac = neg ? make_neg_number(p, q) : make_number(p, q);
    return +frac;
}


// ============================================================================
//
//   Per-element helpers (for list mapping)
//
// ============================================================================

static algebraic_p xq_one(algebraic_r x, bool pi_only)
// ----------------------------------------------------------------------------
//   Apply XQ or QPI logic to a single algebraic value; used for list mapping.
// ----------------------------------------------------------------------------
{
    algebraic_g xo = x;
    if (!xo)
        return nullptr;

    object::id ty = xo->type();

    if (!pi_only)
    {
        // XQ: if already exact, return unchanged.
        if (object::is_integer(ty) || object::is_bignum(ty) ||
            ty == object::ID_fraction || ty == object::ID_neg_fraction ||
            ty == object::ID_big_fraction || ty == object::ID_neg_big_fraction)
            return +xo;
    }

    // Convert to decimal if needed.
    if (ty != object::ID_decimal && ty != object::ID_neg_decimal)
    {
        if (!algebraic::to_decimal(xo))
            return nullptr;
        ty = xo->type();
        xo = algebraic_p(+xo);
    }

    bool      neg = (ty == object::ID_neg_decimal);
    decimal_g dec = decimal_p(+xo);
    if (!dec)
        return nullptr;

    if (dec->is_zero())
        return integer::make(0);

    decimal_g abs_dec = decimal::abs(dec);
    if (!abs_dec)
        return nullptr;

    return xq_decimal(abs_dec, neg, pi_only);
}


static algebraic_p xq_apply(algebraic_r x)
{
    return xq_one(x, false);
}


static algebraic_p qpi_apply(algebraic_r x)
{
    return xq_one(x, true);
}


// ============================================================================
//
//   XQ command
//
// ============================================================================


COMMAND_BODY(XQ)
// ----------------------------------------------------------------------------
//   Find the simplest symbolic representation of a real number
// ----------------------------------------------------------------------------
{
    algebraic_g xo = algebraic_p(strip(rt.stack(0)));
    if (!xo)
        return ERROR;

    object::id ty = xo->type();

    // List: apply element-wise.
    if (ty == object::ID_list)
    {
        list_g lst = list_p(+xo)->map(xq_apply);
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // If already exact, return unchanged.
    if (object::is_integer(ty) || object::is_bignum(ty) ||
        ty == object::ID_fraction || ty == object::ID_neg_fraction ||
        ty == object::ID_big_fraction || ty == object::ID_neg_big_fraction)
        return OK;

    // Convert to decimal if needed.
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

    bool      neg = (ty == object::ID_neg_decimal);
    decimal_g dec = decimal_p(+xo);
    if (!dec)
        return ERROR;

    if (dec->is_zero())
    {
        algebraic_g zero = integer::make(0);
        if (!zero || !rt.top(zero))
            return ERROR;
        return OK;
    }

    // Pass absolute value to xq_decimal; neg carries the sign.
    decimal_g abs_dec = decimal::abs(dec);
    if (!abs_dec)
        return ERROR;

    algebraic_g result = xq_decimal(abs_dec, neg, false);
    if (!result)
        return ERROR;

    if (!rt.top(result))
        return ERROR;
    return OK;
}


// ============================================================================
//
//   →Qπ command (QPI)
//
// ============================================================================


COMMAND_BODY(QPI)
// ----------------------------------------------------------------------------
//   Find the simplest symbolic representation involving π
// ----------------------------------------------------------------------------
{
    algebraic_g xo = algebraic_p(strip(rt.stack(0)));
    if (!xo)
        return ERROR;

    object::id ty = xo->type();

    // List: apply element-wise.
    if (ty == object::ID_list)
    {
        list_g lst = list_p(+xo)->map(qpi_apply);
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // Convert to decimal if needed.
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

    bool      neg = (ty == object::ID_neg_decimal);
    decimal_g dec = decimal_p(+xo);
    if (!dec)
        return ERROR;

    if (dec->is_zero())
    {
        algebraic_g zero = integer::make(0);
        if (!zero || !rt.top(zero))
            return ERROR;
        return OK;
    }

    decimal_g abs_dec = decimal::abs(dec);
    if (!abs_dec)
        return ERROR;

    algebraic_g result = xq_decimal(abs_dec, neg, true);
    if (!result)
        return ERROR;

    if (!rt.top(result))
        return ERROR;
    return OK;
}
