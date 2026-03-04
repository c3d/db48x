// ****************************************************************************
//  exact-quotient.cc                                             DB48X project
// ****************************************************************************
//
//   File Description:
//
//     XQ: find the simplest symbolic representation of a real number.
//
//     Algorithm:
//       For each template T ∈ { OPR(p/q · c), c · OPR(p/q) }
//         with OPR ∈ { id, √, ln, exp } and c ∈ { 1, π, … }:
//         1. Compute v = OPR⁻¹(x) / c  [Form A, c_inside]
//            or      v = OPR⁻¹(x / c)  [Form B, c_outside]
//         2. Find the best rational approximation p/q ≈ v using
//            decimal::to_fraction() (continued-fraction convergents).
//         3. Build the symbolic expression.
//       Select the template yielding the expression with fewest bytes.
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
#include "bignum.h"
#include "constants.h"
#include "decimal.h"
#include "expression.h"
#include "fraction.h"
#include "integer.h"
#include "list.h"
#include "settings.h"


// ============================================================================
//
//   Rational approximation via continued fractions
//
// ============================================================================

static bool xq_best_approx(decimal_r dec, bignum_g &out_p, bignum_g &out_q)
// ----------------------------------------------------------------------------
//   Find smallest-denominator p/q ≈ dec using decimal::to_fraction().
//   dec must be positive (absolute value).  Returns false if none found.
// ----------------------------------------------------------------------------
//   We use a tolerance of (Precision − 4) digits.  The four-digit slack
//   absorbs the ULP-level rounding that accumulates in derived quantities such
//   as (x/π)² or x²/π, where straightforward decimal arithmetic loses ~1–2
//   digits relative to the true algebraic value.  (DB48X stores mantissa ×
//   10^exp with mantissa ∈ [0.001, 1), so a remainder of 10^{-N} has exp =
//   -(N-1); the strict inequality -exp > decimals then requires decimals ≤ N-2,
//   hence prec-4 rather than prec-2.)
{
    if (!dec)
        return false;
    uint        prec     = Settings.Precision();
    uint        prec_tol = prec > 4 ? prec - 4 : 1;
    decimal_g   num      = dec;         // GC root: dec is a raw pointer
    algebraic_g frac     = num->to_fraction(prec, prec_tol);
    if (!frac)
        return false;
    object::id ty = frac->type();
    if (object::is_integer(ty) || object::is_bignum(ty))
    {
        out_p = bignum::promote(+frac);
        out_q = bignum::make(1);
        return out_p && out_q;
    }
    if (ty == object::ID_fraction || ty == object::ID_big_fraction)
    {
        fraction_p f = fraction_p(+frac);
        out_p = f->numerator();
        out_q = f->denominator();
        return out_p && out_q;
    }
    return false;
}


// ============================================================================
//
//   Template table
//
// ============================================================================
//
// Each template describes one symbolic form to probe.  Two forms exist:
//
//   Form A (c_inside = true):  OPR(p/q · c) = |x|
//   Form B (c_inside = false): c · OPR(p/q) = |x|
//
// For identity OPR both forms are equivalent; only one row per constant is
// needed.  The inverse computations are:
//
//   OPR   |  OPR⁻¹
//   ------+---------
//   id    |  id
//   √     |  (·)²
//   ln    |  exp
//   exp   |  ln
//
// To add a new operator: insert rows with its object::id and add its inverse
// in the switch inside xq_inv_arg.
// To add a new constant: insert rows with its name as recognised by
// constant::lookup.

struct xq_tmpl
{
    object::id  opr;      // ID_sqrt / ID_ln / ID_exp; ID_object = identity
    const char *c_name;   // constant name ("π"…) or nullptr (c = 1)
    bool        c_inside; // true → Form A: OPR(p/q·c); false → Form B: c·OPR(p/q)
};

static const xq_tmpl xq_templates[] =
{
    // identity
    { object::ID_object, nullptr, false },  // p/q
    { object::ID_object, "π",    false },   // p/q · π
    // square root
    { object::ID_sqrt,   nullptr, false },  // √(p/q)
    { object::ID_sqrt,   "π",    true  },   // √(p/q · π)
    { object::ID_sqrt,   "π",    false },   // √(p/q) · π
    // natural logarithm
    { object::ID_ln,     nullptr, false },  // ln(p/q)
    { object::ID_ln,     "π",    true  },   // ln(p/q · π)
    { object::ID_ln,     "π",    false },   // ln(p/q) · π
    // exponential
    { object::ID_exp,    nullptr, false },  // exp(p/q)
    { object::ID_exp,    "π",    true  },   // exp(p/q · π)
    { object::ID_exp,    "π",    false },   // exp(p/q) · π
};


// ============================================================================
//
//   Inverse computation and expression builder
//
// ============================================================================

static algebraic_p bignum_to_alg(bignum_r n)
// ----------------------------------------------------------------------------
//   Return an integer if the bignum fits, else the bignum itself
// ----------------------------------------------------------------------------
{
    if (!n)
        return nullptr;
    if (integer_p small = n->as_integer())
        return algebraic_p(small);
    return algebraic_p(+n);
}


static decimal_g xq_inv_arg(decimal_r x, decimal_r c, object::id opr, bool c_inside)
// ----------------------------------------------------------------------------
//   Compute the rational value v = p/q to approximate for a given template.
//   Form A (c_inside): OPR(p/q·c)=x  →  v = OPR⁻¹(x) / c
//   Form B:            c·OPR(p/q)=x  →  v = OPR⁻¹(x/c)
//   Returns nullptr when the result is not a positive real.
// ----------------------------------------------------------------------------
{
    decimal_g v = x;        // GC root
    if (!v)
        return nullptr;

    if (c_inside)
    {
        // Form A: apply OPR⁻¹ first, then divide by c
        switch (opr)
        {
        case object::ID_sqrt:  v = v * v;            break;
        case object::ID_ln:    v = decimal::exp(v);  break;
        case object::ID_exp:   v = decimal::ln(v);   break;
        default:               /* identity */         break;
        }
        if (c && v)
            v = v / c;
    }
    else
    {
        // Form B: divide by c first, then apply OPR⁻¹
        if (c && v)
            v = v / c;
        switch (opr)
        {
        case object::ID_sqrt:  v = v * v;            break;
        case object::ID_ln:    v = decimal::exp(v);  break;
        case object::ID_exp:   v = decimal::ln(v);   break;
        default:               /* identity */         break;
        }
    }

    // Reject non-positive results (e.g. ln of value < 1 yields neg_decimal)
    if (!v || v->type() != object::ID_decimal)
        return nullptr;
    return v;
}


static algebraic_p xq_build(bignum_r p, bignum_r q, const xq_tmpl &tmpl, bool neg)
// ----------------------------------------------------------------------------
//   Build the symbolic expression OPR(p/q·c) or c·OPR(p/q), negated if needed.
// ----------------------------------------------------------------------------
{
    bignum_g pn = p, qn = q;       // GC roots

    // Build p/q (or plain integer p when q = 1).
    // For a plain rational (identity OPR, no constant), fold the sign into the
    // numerator so we produce a proper negative fraction (-1/2) rather than the
    // expression form (-(1/2)), which renders with unwanted parentheses.
    if (tmpl.opr == object::ID_object && !tmpl.c_name)
    {
        algebraic_g result;
        if (qn->is_one())
        {
            result = bignum_to_alg(pn);
            if (neg && result)
                result = expression::make(object::ID_neg, result);
        }
        else
        {
            bignum_g signed_p = neg ? bignum_g(-pn) : pn;
            fraction_p frac = big_fraction::make(signed_p, qn);
            if (!frac)
                return nullptr;
            result = frac;
        }
        return +result;
    }

    algebraic_g pq;
    if (qn->is_one())
        pq = bignum_to_alg(pn);
    else
    {
        fraction_p frac = big_fraction::make(pn, qn);
        if (!frac)
            return nullptr;
        pq = frac;
    }
    if (!pq)
        return nullptr;

    // Get constant symbol (nullptr when c = 1)
    algebraic_g c_sym;
    if (tmpl.c_name)
    {
        c_sym = constant::lookup(tmpl.c_name);
        if (!c_sym)
            return nullptr;
    }

    // Build the OPR argument: p/q, or p/q·c (simplifying 1·c → c)
    algebraic_g arg;
    if (c_sym && tmpl.c_inside)
    {
        if (pn->is_one() && qn->is_one())
            arg = c_sym;
        else
            arg = expression::make(object::ID_multiply, pq, c_sym);
        if (!arg)
            return nullptr;
    }
    else
    {
        arg = pq;
    }

    // Apply OPR (identity leaves arg unchanged)
    algebraic_g result;
    if (tmpl.opr == object::ID_object)
        result = arg;
    else
    {
        result = expression::make(tmpl.opr, arg);
        if (!result)
            return nullptr;
    }

    // Multiply by c when c goes outside (simplifying 1·c → c)
    if (c_sym && !tmpl.c_inside)
    {
        if (tmpl.opr == object::ID_object && pn->is_one() && qn->is_one())
            result = c_sym;
        else
            result = expression::make(object::ID_multiply, result, c_sym);
        if (!result)
            return nullptr;
    }

    if (neg)
        result = expression::make(object::ID_neg, result);

    return +result;
}


// ============================================================================
//
//   Generic XQ engine
//
// ============================================================================

static algebraic_p xq_one(algebraic_r xo, const xq_tmpl *tmpls, size_t count)
// ----------------------------------------------------------------------------
//   Apply templates to a single value; return best match or xo unchanged.
//   Returns nullptr only on type error (non-numeric input).
// ----------------------------------------------------------------------------
{
    if (!xo)
        return nullptr;

    // Integer or fraction: already exact, return unchanged
    object::id ty = xo->type();
    if (object::is_integer(ty) || object::is_fraction(ty))
        return +xo;

    // Convert to decimal
    algebraic_g xg = xo;
    if (!algebraic::to_decimal(xg))
    {
        rt.type_error();
        return nullptr;
    }
    if (!xg)
        return nullptr;

    // Work with |x|, remember sign
    bool      neg  = xg->type() == object::ID_neg_decimal;
    decimal_g adec = decimal_p(+xg);
    if (neg)
        adec = decimal::abs(adec);
    if (!adec)
        return nullptr;

    // Try every template; keep the expression with the smallest byte count
    algebraic_g best    = nullptr;
    size_t      best_sz = SIZE_MAX;

    for (size_t i = 0; i < count; i++)
    {
        const xq_tmpl &tmpl = tmpls[i];

        // Resolve constant to its decimal value via →NUM (nullptr when c = 1)
        decimal_g c_dec;
        if (tmpl.c_name)
        {
            algebraic_g c_alg = constant::lookup(tmpl.c_name);
            if (!c_alg || !algebraic::to_decimal(c_alg))
                continue;
            c_dec = decimal_p(+c_alg);
            if (!c_dec)
                continue;
        }

        // Compute the rational value to approximate for this template
        decimal_g arg = xq_inv_arg(adec, c_dec, tmpl.opr, tmpl.c_inside);
        if (!arg)
            continue;

        // Find best rational p/q ≈ arg
        bignum_g p, q;
        if (!xq_best_approx(arg, p, q))
            continue;

        // Build the expression and track the smallest
        algebraic_g expr = xq_build(p, q, tmpl, neg);
        if (!expr)
            continue;

        size_t sz = expr->size();
        if (sz < best_sz)
        {
            best_sz = sz;
            best    = expr;
        }
    }

    // No simple form found: return original value unchanged
    return best ? +best : +xo;
}


static object::result xq_run(const xq_tmpl *tmpls, size_t count,
                              algebraic_fn   map_fn)
// ----------------------------------------------------------------------------
//   Find the simplest symbolic representation of the stack top.
//   map_fn is the algebraic_fn adapter used when the stack top is a list.
// ----------------------------------------------------------------------------
{
    object_p top = object::strip(rt.stack(0));
    if (!top)
        return object::ERROR;

    // List: apply element-wise
    if (top->type() == object::ID_list)
    {
        list_p result = list_p(top)->map(map_fn);
        if (!result || !rt.top(result))
            return object::ERROR;
        return object::OK;
    }

    algebraic_g xo    = algebraic_p(top);
    algebraic_g result = xq_one(xo, tmpls, count);
    if (!result)
        return object::ERROR;

    // Replace stack top only if the value changed
    if (+result != top)
        if (!rt.top(+result))
            return object::ERROR;

    return object::OK;
}


// ============================================================================
//
//   XQ command
//
// ============================================================================

static algebraic_p xq_map_xq(algebraic_r x)
{
    return xq_one(x, xq_templates, std::size(xq_templates));
}

COMMAND_BODY(XQ)
// ----------------------------------------------------------------------------
//   Find the simplest symbolic representation of a real number
// ----------------------------------------------------------------------------
{
    return xq_run(xq_templates, std::size(xq_templates), xq_map_xq);
}


// ============================================================================
//
//   →Qπ command
//
// ============================================================================

static const xq_tmpl qpi_templates[] =
{
    // identity
    { object::ID_object, "π",    false },   // p/q · π
    // square root
    { object::ID_sqrt,   "π",    true  },   // √(p/q · π)
    { object::ID_sqrt,   "π",    false },   // √(p/q) · π
    // natural logarithm
    { object::ID_ln,     "π",    true  },   // ln(p/q · π)
    { object::ID_ln,     "π",    false },   // ln(p/q) · π
    // exponential
    { object::ID_exp,    "π",    true  },   // exp(p/q · π)
    { object::ID_exp,    "π",    false },   // exp(p/q) · π
};

static algebraic_p xq_map_qpi(algebraic_r x)
{
    return xq_one(x, qpi_templates, std::size(qpi_templates));
}

COMMAND_BODY(QPI)
// ----------------------------------------------------------------------------
//   Find the simplest symbolic representation of a real number
// ----------------------------------------------------------------------------
{
    return xq_run(qpi_templates, std::size(qpi_templates), xq_map_qpi);
}
