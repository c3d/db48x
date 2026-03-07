// ****************************************************************************
//  algebraic.cc                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Shared code for all algebraic commands
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

#include "algebraic.h"

#include "arithmetic.h"
#include "factor.h"
#include "list.h"
#include "array.h"
#include "bignum.h"
#include "compare.h"
#include "complex.h"
#include "constants.h"
#include "decimal.h"
#include "equations.h"
#include "expression.h"
#include "functions.h"
#include "hwfp.h"
#include "integer.h"
#include "parser.h"
#include "range.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "tag.h"
#include "unit.h"
#include "user_interface.h"

#include <cctype>
#include <cmath>
#include <cstdio>


RECORDER(algebraic,       16, "RPL Algebraics");
RECORDER(algebraic_error, 16, "Errors processing a algebraic");


INSERT_BODY(algebraic)
// ----------------------------------------------------------------------------
//   Enter data in algebraic mode
// ----------------------------------------------------------------------------
{
    return ui.insert_object(o, o->arity() ? ui.ALGEBRAIC : ui.CONSTANT);
}


bool algebraic::decimal_promotion(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to a decimal / floating-point type
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    id xt = x->type();
    record(algebraic, "Real promotion of %p from %+s to decimal",
           (object_p) x, object::name(xt));

    switch(xt)
    {
    case ID_hwfloat:
        if (algebraic_p xx = decimal::from(hwfloat_p(+x)->value()))
        {
            x = xx;
            return true;
        }
        break;
    case ID_hwdouble:
        if (algebraic_p xx = decimal::from(hwdouble_p(+x)->value()))
        {
            x = xx;
            return true;
        }
        break;
    case ID_decimal:
    case ID_neg_decimal:
        return true;

    case ID_integer:
    case ID_neg_integer:
        if (algebraic_p xx = decimal::from_integer(integer_p(+x)))
        {
            x = xx;
            return true;
        }
        break;
    case ID_bignum:
    case ID_neg_bignum:
        if (algebraic_p xx = decimal::from_bignum(bignum_p(+x)))
        {
            x = xx;
            return true;
        }
        break;
    case ID_fraction:
    case ID_neg_fraction:
        if (algebraic_p xx = decimal::from_fraction(fraction_p(+x)))
        {
            x = xx;
            return true;
        }
        break;
    case ID_big_fraction:
    case ID_neg_big_fraction:
        if (algebraic_p xx = decimal::from_big_fraction(big_fraction_p(+x)))
        {
            x = xx;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

template<typename value>
algebraic_p algebraic::as_hwfp(value x)
// ----------------------------------------------------------------------------
//   Return a hardware floating-point value if possible
// ----------------------------------------------------------------------------
{
    if (Settings.HardwareFloatingPoint())
    {
        uint prec = Settings.Precision();
        if (prec <= 7)
            return hwfloat::make(float(x));
        if (prec <= 16)
            return hwdouble::make(double(x));
    }
    return nullptr;
}


bool algebraic::hwfp_promotion(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to a hardware floating-point type if possible
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    if (!Settings.HardwareFloatingPoint())
        return false;
    uint prec = Settings.Precision();
    if (prec > 16)
        return false;
    return prec > 7 ? to_hwdouble(x) : to_hwfloat(x);
}


bool algebraic::complex_promotion(algebraic_g &x, object::id type)
// ----------------------------------------------------------------------------
//   Promote the value x to the given complex type
// ----------------------------------------------------------------------------
{
    id xt = x->type();
    if (xt == type)
        return true;

    record(algebraic, "Complex promotion of %p from %+s to %+s",
           (object_p) x, object::name(xt), object::name(type));

    if (!is_complex(type))
    {
        record(algebraic_error, "Complex promotion to invalid type %+s",
               object::name(type));
        return false;
    }

    if (xt == ID_polar)
    {
        // Convert from polar to rectangular
        polar_g z = polar_p(algebraic_p(x));
        x = rectangular_p(z->as_rectangular());
        return +x;
    }
    else if (xt == ID_rectangular)
    {
        // Convert from rectangular to polar
        rectangular_g z = rectangular_p(algebraic_p(x));
        x = polar_p(z->as_polar());
        return +x;
    }
    else if (is_symbolic(xt))
    {
        // Assume a symbolic value is complex for now
        // TODO: Implement `REALASSUME`
        return false;
    }
    else if (is_symbolic_arg(xt) || is_algebraic(xt))
    {
        algebraic_g zero = algebraic_p(integer::make(0));
        if (type == ID_polar)
            x = polar::make(x, zero, object::ID_PiRadians);
        else
            x = rectangular::make(x, zero);
        return +x;
    }

    return false;
}


bool algebraic::range_promotion(algebraic_g &x, object::id type)
// ----------------------------------------------------------------------------
//   Promote the value x to the given range type
// ----------------------------------------------------------------------------
{
    id xt = x->type();
    if (xt == type)
        return true;

    record(algebraic, "Range promotion of %p from %+s to %+s",
           (object_p) x, object::name(xt), object::name(type));

    if (!is_range(type))
    {
        record(algebraic_error, "Range promotion to invalid type %+s",
               object::name(type));
        return false;
    }

    // Can convert between ranges but not to/from uncertain
    if (is_range(xt))
        return (xt == ID_uncertain) == (type == ID_uncertain);

    // Symbolic values cannot be promoted to ranges
    if (is_symbolic(xt))
        return false;

    // Convert numerical values to uncertain range
    if (is_symbolic_arg(xt) || is_algebraic(xt))
    {
        if (type == ID_uncertain)
        {
            algebraic_g zero = integer::make(0);
            x = uncertain::make(x, zero);
        }
        else
        {
            x = range::make(type, x, x);
        }
        return +x;
    }

    return false;
}


object::id algebraic::bignum_promotion(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to the corresponding bignum
// ----------------------------------------------------------------------------
{
    id xt = x->type();
    id ty = xt;

    switch(xt)
    {
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_integer:        ty = ID_hex_bignum;     break;
    case ID_dec_integer:        ty = ID_dec_bignum;     break;
    case ID_oct_integer:        ty = ID_oct_bignum;     break;
    case ID_bin_integer:        ty = ID_bin_bignum;     break;
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:      ty = ID_based_bignum;   break;
    case ID_neg_integer:        ty = ID_neg_bignum;     break;
    case ID_integer:            ty = ID_bignum;         break;
    default:
        break;
    }
    if (ty != xt)
    {
        integer_g i = (integer *) object_p(x);
        x = rt.make<bignum>(ty, i);
    }
    return ty;
}


object::id algebraic::based_promotion(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to a based number
// ----------------------------------------------------------------------------
{
    if (!x)
        return object::ID_object;

    id xt = x->type();

    switch (xt)
    {
    case ID_integer:
    case ID_neg_integer:
        if (Settings.WordSize() < 64)
        {
            ularge value = integer_p(+x)->value<ularge>();
            if (xt == ID_neg_integer)
                value = -value;
            x = rt.make<based_integer>(value);
            return ID_based_integer;
        }
        else
        {
            xt = xt == ID_neg_integer ? ID_neg_bignum : ID_bignum;
            integer_g xi = integer_p(+x);
            bignum_g  xb = rt.make<bignum>(xt, xi);
            x = +xb;
        }
        // fallthrough

    case ID_bignum:
    case ID_neg_bignum:
    {
        size_t   sz   = 0;
        gcbytes  data = bignum_p(+x)->value(&sz);
        bignum_g xb   = rt.make<bignum>(ID_based_bignum, data, sz);
        if (xt == ID_neg_bignum)
        {
            bignum_g zero = rt.make<based_bignum>(0);
            xb = zero - xb;
        }
        x = +xb;
        return ID_based_bignum;
    }

    default:
        break;
    }
    return xt;
}


bool algebraic::to_integer(algebraic_g &x)
// ----------------------------------------------------------------------------
//  Check if we can convert the number to an integer (or big integer)
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    id ty = x->type();
    switch(ty)
    {
    case ID_hwfloat:
        x = hwfloat_p(+x)->to_integer();
        break;
    case ID_hwdouble:
        x = hwdouble_p(+x)->to_integer();
        break;
    case ID_decimal:
    case ID_neg_decimal:
        x = decimal_p(+x)->to_integer();
        break;

    case ID_integer:
    case ID_neg_integer:
    case ID_bignum:
    case ID_neg_bignum:
    case ID_fraction:
    case ID_neg_fraction:
    case ID_big_fraction:
    case ID_neg_big_fraction:
        break;

    case ID_unit:
    {
        unit_p ux = unit_p(+x);
        algebraic_g v = ux->value();
        algebraic_g u = ux->uexpr();
        if (to_integer(v))
        {
            x = unit::simple(v, u);
            break;
        }
    }
    // fallthrough
    default:
        return false;
    }
    return x;                   // Need x to be non-null
}


// ============================================================================
//
//   to_fraction / to_fraction_pi shared dispatch
//
// ============================================================================

struct to_fraction_context
// ----------------------------------------------------------------------------
//   Callbacks for real conversion and list mapping
// ----------------------------------------------------------------------------
{
    bool (*convert_real)(algebraic_g &);
    algebraic_p (*map_fn)(algebraic_r);
};


static bool to_fraction_real(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Convert hwfloat/hwdouble/decimal to fraction
// ----------------------------------------------------------------------------
{
    object::id ty = x->type();
    switch(ty)
    {
    case object::ID_hwfloat:
        x = hwfloat_p(+x)->to_fraction();
        break;
    case object::ID_hwdouble:
        x = hwdouble_p(+x)->to_fraction();
        break;
    case object::ID_decimal:
    case object::ID_neg_decimal:
        x = decimal_p(+x)->to_fraction();
        break;
    default:
        return false;
    }
    return x;
}


static bool to_fraction_dispatch(algebraic_g &x, const to_fraction_context &ctx)
// ----------------------------------------------------------------------------
//   Shared type switch; real types via callback, compound types recursed/mapped
// ----------------------------------------------------------------------------
{
    object::id ty = x->type();
    switch(ty)
    {
    case object::ID_hwfloat:
    case object::ID_hwdouble:
    case object::ID_decimal:
    case object::ID_neg_decimal:
        return ctx.convert_real(x);

    case object::ID_integer:
    case object::ID_neg_integer:
    case object::ID_bignum:
    case object::ID_neg_bignum:
    case object::ID_fraction:
    case object::ID_neg_fraction:
    case object::ID_big_fraction:
    case object::ID_neg_big_fraction:
        break;

    case object::ID_rectangular:
    {
        rectangular_p z = rectangular_p(+x);
        algebraic_g re = z->re();
        algebraic_g im = z->im();
        if (!to_fraction_dispatch(re, ctx) || !to_fraction_dispatch(im, ctx))
            return false;
        x = rectangular::make(re, im);
        break;
    }
    case object::ID_polar:
    {
        polar_p z = polar_p(+x);
        algebraic_g mod = z->mod();
        algebraic_g arg = z->pifrac();
        if (!to_fraction_dispatch(mod, ctx) || !to_fraction_dispatch(arg, ctx))
            return false;
        x = polar::make(mod, arg, object::ID_PiRadians);
        break;
    }
    case object::ID_range:
    case object::ID_drange:
    case object::ID_prange:
    case object::ID_uncertain:
    {
        range_p r = range_p(+x);
        algebraic_g lo = r->lo();
        algebraic_g hi = r->hi();
        if (!to_fraction_dispatch(lo, ctx) || !to_fraction_dispatch(hi, ctx))
            return false;
        x = range::make(r->type(), lo, hi);
        break;
    }
    case object::ID_unit:
    {
        unit_p ux = unit_p(+x);
        algebraic_g v = ux->value();
        algebraic_g u = ux->uexpr();
        if (to_fraction_dispatch(v, ctx))
        {
            x = unit::simple(v, u);
            break;
        }
        return false;
    }
    case object::ID_equation:
    {
        object_p inner = equation_p(+x)->value();
        if (!inner || !inner->is_algebraic())
            return false;
        x = algebraic_p(inner);
        // fall through
    }
    case object::ID_array:
    case object::ID_list:
    case object::ID_expression:
    {
        list_g mapped = list_p(+x)->map(ctx.map_fn);
        if (!mapped)
            return false;
        record(algebraic, "to_fraction mapped %p type %+s size %u",
               +mapped, object::name(mapped->type()), mapped->size());
        x = +mapped;
        break;
    }
    default:
        return false;
    }
    return x;
}


static algebraic_p to_fraction_map_fn(algebraic_r a)
// ----------------------------------------------------------------------------
//   Map callback for list::map in to_fraction
// ----------------------------------------------------------------------------
{
    algebraic_g ag = a;
    if (ag->is_algebraic_num())
        algebraic::to_fraction(ag);
    return +ag;
}


bool algebraic::to_fraction(algebraic_g &x)
// ----------------------------------------------------------------------------
//  Check if we can promote the number to a fraction
// ----------------------------------------------------------------------------
{
    to_fraction_context ctx = { to_fraction_real, to_fraction_map_fn };
    return to_fraction_dispatch(x, ctx);
}


static ularge fraction_denominator(algebraic_r x)
// ----------------------------------------------------------------------------
//   Return the denominator of a fraction result, or 1 for integers
// ----------------------------------------------------------------------------
{
    object::id ty = x->type();
    if (object::is_integer(ty))
        return 1;
    if (object::is_fraction(ty))
        return fraction_p(+x)->denominator_value();
    return ~ularge(0);
}


static void extract_square_factor(ularge n, ularge &sq, ularge &rem)
// ----------------------------------------------------------------------------
//   Factor n as sq²·rem where rem is square-free
// ----------------------------------------------------------------------------
{
    sq = 1;
    rem = n;
    ularge max_prime = Settings.FractionLargestPrime();
    for (size_t i = 0; i < NUM_SMALL_PRIMES; i++)
    {
        ularge p = small_primes[i];
        ularge pp = p * p;
        if (p == 0 || p > max_prime || rem < pp)
            return;
        while (rem % pp == 0)
        {
            rem /= pp;
            sq *= p;
            if (rem == 0)
                return;
        }
    }
}


static algebraic_p fraction_times_symbolic(algebraic_r frac, algebraic_r sym)
// ----------------------------------------------------------------------------
//   Build result as frac * symbolic, simplifying 1*X and (-1)*X
// ----------------------------------------------------------------------------
{
    if (frac->is_one(false))
        return expression::make(sym);
    algebraic_g nfrac = -frac;
    if (nfrac && nfrac->is_one(false))
        return expression::make(object::ID_neg, sym);
    return expression::make(object::ID_multiply, frac, sym);
}


static algebraic_p wrap_symbolic(algebraic_r sym)
// ----------------------------------------------------------------------------
//   Wrap a value in an expression if not already one
// ----------------------------------------------------------------------------
{
    if (sym->type() == object::ID_expression)
        return algebraic_p(sym);
    return expression::make(sym);
}


static algebraic_p to_fraction_pi_map_fn(algebraic_r a)
// ----------------------------------------------------------------------------
//   Wrapper for list::map to apply to_fraction_pi to expression elements
// ----------------------------------------------------------------------------
{
    algebraic_g ag = a;
    if (ag->is_algebraic_num())
        algebraic::to_fraction_pi(ag);
    return +ag;
}


static bool to_fraction_pi_real(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Convert real to fraction with π, √n, ln(n), e factors (→Qπ)
// ----------------------------------------------------------------------------
{
    algebraic_g val = x;
    algebraic_g plain = val;
    if (!algebraic::to_fraction(plain))
        return false;

    ularge      best_denom = fraction_denominator(plain);
    algebraic_g best_result = plain;
    record(algebraic, "→Qπ plain fraction denom %u type %+s",
           best_denom, object::name(plain->type()));

    if (best_denom <= 1)
    {
        x = best_result;
        return x;
    }

    constant_p  pi_sym  = constant::lookup("π", false);
    constant_p  e_sym   = constant::lookup("e", false);
    algebraic_g pi_val  = algebraic::pi();
    if (!pi_val)
        return false;

    auto try_factor = [&](algebraic_r num_val, algebraic_r sym)
    {
        if (!num_val || !sym || rt.error())
            return;
        algebraic_g divided = val / num_val;
        if (!divided || rt.error())
        {
            rt.clear_error();
            return;
        }
        algebraic_g frac = divided;
        if (!algebraic::to_fraction(frac))
        {
            rt.clear_error();
            return;
        }
        ularge d = fraction_denominator(frac);
        if (d < best_denom)
        {
            best_denom = d;
            best_result = fraction_times_symbolic(frac, sym);
        }
    };

    // Try x = (p/q) · π
    if (pi_sym)
        try_factor(pi_val, pi_sym);
    if (best_denom <= 1)
        goto done;

    // Try x = (p/q) · √n by squaring x and factoring out perfect squares
    {
        bool        neg = val->is_negative(false);
        algebraic_g val_sq = val * val;
        if (val_sq && !rt.error())
        {
            algebraic_g frac = val_sq;
            if (algebraic::to_fraction(frac))
            {
                ularge num = 0, den = 1;
                object::id fty = frac->type();
                if (object::is_bignum(fty))
                    num = bignum_p(+frac)->value<ularge>();
                else if (object::is_integer(fty))
                    num = integer_p(+frac)->value<ularge>();
                else if (object::is_fraction(fty))
                {
                    num = fraction_p(+frac)->numerator_value();
                    den = fraction_p(+frac)->denominator_value();
                }
                record(algebraic,
                       "Sqrt factoring x²→frac %+s num %u den %u",
                       object::name(fty), num, den);

                if (num > 0)
                {
                    ularge sq_num, free_num, sq_den, free_den;
                    extract_square_factor(num, sq_num, free_num);
                    extract_square_factor(den, sq_den, free_den);

                    constexpr ularge ulmax = ~ularge(0);
                    ularge sqrt_part = 0;
                    if (free_den > 0 && free_num <= ulmax / free_den)
                        sqrt_part = free_num * free_den;
                    record(algebraic,
                           "Sqrt factors sq=%u free=%u / sq=%u free=%u"
                           " → √%u denom %u (best %u)",
                           sq_num, free_num, sq_den, free_den,
                           sqrt_part, sq_den * free_den, best_denom);
                    if (sqrt_part > 1)
                    {
                        ularge rational_den = sq_den * free_den;
                        if (rational_den < best_denom)
                        {
                            algebraic_g p = integer::make(sq_num);
                            algebraic_g q = integer::make(rational_den);
                            algebraic_g rf = (rational_den == 1)
                                ? p
                                : fraction::make(
                                      integer_p(+p), integer_p(+q));
                            if (neg)
                                rf = -rf;
                            algebraic_g sv = integer::make(sqrt_part);
                            algebraic_g ssym = expression::make(object::ID_sqrt, sv);
                            best_denom = rational_den;
                            best_result =
                                fraction_times_symbolic(rf, ssym);
                        }
                    }
                }
            }
            else
            {
                rt.clear_error();
            }
        }
        else
        {
            rt.clear_error();
        }
    }
    if (best_denom <= 1)
        goto done;

    // Try x = (p/q) · ln(n)
    static const uint ln_factors[] = { 2, 3, 5, 7, 10 };
    for (uint n : ln_factors)
    {
        algebraic_g nval = integer::make(n);
        algebraic_g lval = ln::run(nval);
        algebraic_g lsym = expression::make(object::ID_ln, nval);
        try_factor(lval, lsym);
    }
    if (best_denom <= 1)
        goto done;

    // Try x = (p/q) · e
    if (e_sym)
    {
        algebraic_g e_val = decimal::e();
        if (e_val)
            try_factor(e_val, e_sym);
    }
    if (best_denom <= 1)
        goto done;

    // Try x = (p/q) · π·√n for common combinations
    if (pi_sym)
    {
        static const uint pi_sqrt_n[] = { 2, 3 };
        for (uint n : pi_sqrt_n)
        {
            algebraic_g nval = integer::make(n);
            algebraic_g sqrt_n = expression::make(object::ID_sqrt, nval);
            algebraic_g prod = pi_val * sqrt_n;
            algebraic_g sym =
                expression::make(object::ID_multiply, pi_sym, sqrt_n);
            if (prod && sym)
                try_factor(prod, sym);
        }
    }
    if (best_denom <= 1)
        goto done;

    // Try x = e^(p/q) by checking if ln(x) is rational
    if (e_sym && !val->is_negative(false) && !val->is_zero(false))
    {
        algebraic_g lx = ln::run(val);
        if (lx && !rt.error())
        {
            algebraic_g frac = lx;
            if (algebraic::to_fraction(frac))
            {
                ularge d = fraction_denominator(frac);
                if (d < best_denom)
                {
                    best_denom = d;
                    if (frac->is_one(false))
                    {
                        best_result = wrap_symbolic(e_sym);
                    }
                    else
                    {
                        algebraic_g nfrac = -frac;
                        if (nfrac && nfrac->is_one(false))
                        {
                            best_result =
                                expression::make(object::ID_inv, e_sym);
                        }
                        else
                        {
                            best_result =
                                expression::make(object::ID_exp, frac);
                        }
                    }
                }
            }
            else
            {
                rt.clear_error();
            }
        }
        else
        {
            rt.clear_error();
        }
    }

done:
    x = best_result;
    return x;
}


bool algebraic::to_fraction_pi(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Convert to fraction, trying π, √n, ln(n), and e as factors
// ----------------------------------------------------------------------------
{
    to_fraction_context ctx = { to_fraction_pi_real, to_fraction_pi_map_fn };
    return to_fraction_dispatch(x, ctx);
}


static algebraic_p to_decimal_callback(algebraic_r x, bool weak)
// ----------------------------------------------------------------------------
//  Callback for to_decimal applied to arrays
// ----------------------------------------------------------------------------
{
    algebraic_g v = x;
    return algebraic::to_decimal(v, weak) ? v : nullptr;
}


static algebraic_p to_decimal_strong(algebraic_r x)
// ----------------------------------------------------------------------------
//   For the string case (error emitting)
// ----------------------------------------------------------------------------
{
    return to_decimal_callback(x, false);
}


static algebraic_p to_decimal_weak(algebraic_r x)
// ----------------------------------------------------------------------------
//   For the weak case (no error emission)
// ----------------------------------------------------------------------------
{
    return to_decimal_callback(x, true);
}


bool algebraic::to_decimal(algebraic_g &x, bool weak)
// ----------------------------------------------------------------------------
//   Convert a value to decimal
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    id xt = x->type();

    switch(xt)
    {
    case ID_rectangular:
    {
        rectangular_p z = rectangular_p(+x);
        algebraic_g re = z->re();
        algebraic_g im = z->im();
        if (to_decimal(re, weak) && to_decimal(im, weak))
        {
            x = rectangular::make(re, im);
            return x;
        }
        break;
    }
    case ID_polar:
    {
        polar_p z = polar_p(+x);
        algebraic_g mod = z->mod();
        algebraic_g arg = z->pifrac();
        if (to_decimal(mod, weak) &&
            (mod->is_fraction() || to_decimal(arg, weak)))
        {
            x = polar::make(mod, arg, object::ID_PiRadians);
            return x;
        }
        break;
    }
    case ID_range:
    case ID_prange:
    case ID_drange:
    {
        range_p r = range_p(+x);
        algebraic_g lo = r->lo();
        algebraic_g hi = r->hi();
        if (to_decimal(lo, weak) && to_decimal(hi, weak))
        {
            x = range::make(r->type(), lo, hi);
            return true;
        }
        break;
    }
    case ID_uncertain:
    {
        uncertain_p u = uncertain_p(+x);
        algebraic_g a = u->average();
        algebraic_g s = u->stddev();
        if (to_decimal(a, weak) && to_decimal(s, weak))
        {
            x = uncertain::make(a, s);
            return true;
        }
        break;
    }
    case ID_unit:
    {
        unit_p ux = unit_p(+x);
        algebraic_g v = ux->value();
        algebraic_g u = ux->uexpr();
        if (to_decimal(v, weak))
        {
            x = unit::simple(v, u);
            return x;
        }
        break;
    }
    case ID_integer:
    case ID_neg_integer:
        if (weak)
            return true;
        // fallthrough
    case ID_bignum:
    case ID_neg_bignum:
    case ID_fraction:
    case ID_neg_fraction:
    case ID_big_fraction:
    case ID_neg_big_fraction:
    case ID_hwfloat:
    case ID_hwdouble:
    case ID_decimal:
    case ID_neg_decimal:
    case ID_True:
    case ID_False:
        return decimal_promotion(x);
    case ID_constant:
    case ID_standard_uncertainty:
    case ID_relative_uncertainty:
    case ID_xlib:
    {
        settings::SaveNumericalResults save(true);
        x = constant_p(+x)->evaluate();
        return x && !rt.error();
    }

    case ID_array:
    case ID_list:
    {
        bool ok = true;
        if (list_p res = list_p(+x)->map(weak
                                         ? to_decimal_weak
                                         : to_decimal_strong))
            x = res;
        else
            ok = false;
        return ok;
    }

    case ID_expression:
        if (!unit::mode)
        {
            expression_p eq = expression_p(+x);
            settings::SaveNumericalResults save(true);
            x = eq->evaluate();
            return x && !rt.error();
        }
        // fallthrough
    default:
        if (!weak)
            rt.type_error();
    }
    return false;
}


bool algebraic::to_hwfloat(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to a hardware floating-point type if possible
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    id xt = x->type();
    record(algebraic, "Convert %p from %+s to hwfloat",
           (object_p) x, object::name(xt));

    switch(xt)
    {
    case ID_hwfloat:
        return true;
    case ID_hwdouble:
        x = hwfloat::make(hwdouble_p(+x)->as_float());
        return x;

    case ID_decimal:
    case ID_neg_decimal:
        x = hwfloat::make(decimal_p(+x)->to_float());
        return x;

    case ID_integer:
        x = as_hwfp(float(integer_p(+x)->value<ularge>()));
        return x;
    case ID_neg_integer:
        x = as_hwfp(-float(integer_p(+x)->value<ularge>()));
        return x;
    case ID_bignum:
    case ID_neg_bignum:
        x = decimal::from_bignum(bignum_p(+x));
        if (x && x->is_decimal())
            x = as_hwfp(decimal_p(+x)->to_float());
        return x;

    case ID_fraction:
        x = as_hwfp(float(fraction_p(+x)->numerator_value()) /
                    float(fraction_p(+x)->denominator_value()));
        return x;
    case ID_neg_fraction:
        x = as_hwfp(-float(fraction_p(+x)->numerator_value()) /
                    float(fraction_p(+x)->denominator_value()));
        return x;
    case ID_big_fraction:
    case ID_neg_big_fraction:
        x = decimal::from_big_fraction(big_fraction_p(+x));
        if (x && x->is_decimal())
                x = as_hwfp(decimal_p(+x)->to_float());
        return x;
    default:
        return false;
    }
}


bool algebraic::to_hwdouble(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Promote the value x to a hardware floating-point type if possible
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    id xt = x->type();
    record(algebraic, "Convert %p from %+s to hwdouble",
           (object_p) x, object::name(xt));

    switch(xt)
    {
    case ID_hwfloat:
        x = hwfloat::make(hwfloat_p(+x)->as_double());
        return x;
    case ID_hwdouble:
        return true;

    case ID_decimal:
    case ID_neg_decimal:
        x = hwdouble::make(decimal_p(+x)->to_double());
        return x;

    case ID_integer:
        x = as_hwfp(double(integer_p(+x)->value<ularge>()));
        return x;
    case ID_neg_integer:
        x = as_hwfp(-double(integer_p(+x)->value<ularge>()));
        return x;
    case ID_bignum:
    case ID_neg_bignum:
        x = decimal::from_bignum(bignum_p(+x));
        if (x && x->is_decimal())
            x = as_hwfp(decimal_p(+x)->to_double());
        return x;

    case ID_fraction:
        x = as_hwfp(double(fraction_p(+x)->numerator_value()) /
                    double(fraction_p(+x)->denominator_value()));
        return x;
    case ID_neg_fraction:
        x = as_hwfp(-double(fraction_p(+x)->numerator_value()) /
                    double(fraction_p(+x)->denominator_value()));
        return x;
    case ID_big_fraction:
    case ID_neg_big_fraction:
        x = decimal::from_big_fraction(big_fraction_p(+x));
        if (x && x->is_decimal())
                x = as_hwfp(decimal_p(+x)->to_double());
        return x;
    default:
        return false;
    }
}


algebraic_g algebraic::pi()
// ----------------------------------------------------------------------------
//   Return the value of pi
// ----------------------------------------------------------------------------
{
    if (algebraic_p result = as_hwfp(M_PI))
        return result;
    return decimal::pi();
}


algebraic::angle_unit algebraic::adjust_angle(algebraic_g &x)
// ----------------------------------------------------------------------------
//   If we have an angle unit, use it for the computation
// ----------------------------------------------------------------------------
{
retry:
    angle_unit amode = ID_object;
    if (unit_p uobj = unit::get(x))
    {
        algebraic_g uexpr = uobj->uexpr();
        if (symbol_p sym = uexpr->as_quoted<symbol>())
        {
            if (sym->matches("dms") || sym->matches("°"))
                amode = ID_Deg;
            else if (sym->matches("r"))
                amode = ID_Rad;
            else if (sym->matches("pir") || sym->matches("πr"))
                amode = ID_PiRadians;
            else if (sym->matches("grad"))
                amode = ID_Grad;

        }
        if (amode == ID_object)
        {
            algebraic_g aunit = integer::make(1);
            if (add_angle(aunit))
                if (unit_p(+aunit)->convert(x, false))
                    goto retry;
        }
        if (amode)
            x = uobj->value();
    }
    return amode;
}


bool algebraic::add_angle(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Add an angle unit if this is required
// ----------------------------------------------------------------------------
{
    cstring uname;

    switch(Settings.AngleMode())
    {
    case ID_Deg:        uname = "°";    break;
    case ID_Grad:       uname = "grad"; break;
    case ID_PiRadians:  uname = "πr";   break;
    case ID_Rad:        uname = "r";    break;
    default:
        return false;
    }

    symbol_p uexpr = symbol::make(uname);
    if (algebraic_p angle = unit::make(x, uexpr))
    {
        x = angle;
        return true;
    }
    return false;
}


algebraic_p algebraic::convert_angle(algebraic_r ra,
                                     angle_unit  from,
                                     angle_unit  to,
                                     bool        negmod,
                                     bool        domodulo)
// ----------------------------------------------------------------------------
//   Convert to angle in current angle mode.
// ----------------------------------------------------------------------------
//   If radians is set, input is in radians.
//   Otherwise, input is in fractions of pi (internal format for y() in polar).
{
    algebraic_g a = ra;
    if (a->is_real() && (from != to || negmod))
    {
        switch (from)
        {
        case ID_Deg:
            a = a / integer::make(180);
            break;
        case ID_Grad:
            a = a / integer::make(200);
            break;
        case ID_Rad:
        {
            algebraic_g pi = algebraic::pi();
            if (a->is_fraction())
            {
                fraction_g  f = fraction_p(+a);
                algebraic_g n = algebraic_p(f->numerator());
                algebraic_g d = algebraic_p(f->denominator());
                a = n / pi / d;
            }
            else
            {
                a = a / pi;
            }
            break;
        }
        case ID_PiRadians:
        default:
            break;
        }

        // Check if we have (-1, 0π), change it to (1, 1π)
        if (negmod)
            a = a + algebraic_g(integer::make(1));

        // Bring the result between -1 and 1
        algebraic_g one = integer::make(1);
        algebraic_g two = integer::make(2);
        if (domodulo)
        {
            a = (one - a) % two;
            if (!a)
                return nullptr;
            if (a->is_negative(false))
                a = a + two;
            a = one - a;
        }

        switch (to)
        {
        case ID_Deg:
            a = a * integer::make(180);
            break;
        case ID_Grad:
            a = a * integer::make(200);
            break;
        case ID_Rad:
        {
            algebraic_g pi = algebraic::pi();
            if (a->is_fraction())
            {
                fraction_g f = fraction_p(+a);
                algebraic_g n = algebraic_p(f->numerator());
                algebraic_g d = algebraic_p(f->denominator());
                a = pi * n / d;
            }
            else
            {
                a = a * pi;
            }
            break;
        }
        case ID_PiRadians:
        default:
            break;
        }
    }
    return a;
}


algebraic_p algebraic::exact_angle(int num, int denom, angle_unit aunit)
// ----------------------------------------------------------------------------
//   Generate a fraction of a turn in the given unit
// ----------------------------------------------------------------------------
{
    if (aunit != ID_Deg && aunit != ID_Grad && aunit != ID_PiRadians)
        return nullptr;

    int hturn = aunit == ID_Deg ? 180 : aunit == ID_Grad ? 200 : 1;
    num *= hturn;
    if (num % denom == 0)
        return integer::make(num/denom);
    return fraction::make(integer::make(num), integer::make(denom));
}


algebraic_p algebraic::evaluate_function(program_r eq, algebraic_r x)
// ----------------------------------------------------------------------------
//   Evaluate the eq object as a function
// ----------------------------------------------------------------------------
//   Equation objects can be one of:
//   - Something that takes value from the stack and returns it on the stack
//     for example << 1 + >>
//   - Something that evaluates using the indep and returns it on the stack,
//     for example 'X + 1' (assuming X is the independent variable)
{
    stack_depth_restore sdr;
    if (!rt.push(+x))
        return nullptr;
    rt.clear_error();
    save<object_g *> ival(expression::independent_value, (object_g *) &x);
    result           err    = eq->run();
    if (err != OK)
        return nullptr;

    object_p result = rt.pop();
    if (!result)
        return nullptr;

    size_t added = sdr.extra_depth();
    if (added == 1)
    {
        // Case where we evaluated from indep without consuming the stack
        object_p indep  = rt.pop();
        if (indep == +x)
            added = 0;
    }
    if (added != 0)
    {
        rt.invalid_function_error();
        return nullptr;
    }

    // Check that we have an acceptable return type
    if (!result->is_extended_algebraic())
    {
        rt.type_error();
        return nullptr;
    }

    return algebraic_p(result);
}


algebraic_p algebraic::evaluate_function(program_r   eq,
                                         algebraic_r x,
                                         algebraic_r y)
// ----------------------------------------------------------------------------
//   Evaluate the eq object as a function of two variables
// ----------------------------------------------------------------------------
{
    stack_depth_restore sdr;
    if (!rt.push(+x) || !rt.push(+y))
        return nullptr;
    rt.clear_error();
    save<object_g *> ival(expression::independent_value, (object_g *) &x);
    save<object_g *> dval(expression::dependent_value, (object_g *) &y);
    result           err    = eq->run();
    if (err != OK)
        return nullptr;

    object_p result = rt.pop();
    if (!result)
        return nullptr;

    size_t added   = sdr.extra_depth();
    if (added == 2)
    {
        // Case where we evaluated from indep without consuming the stack
        object_p dep  = rt.pop();
        object_p indep  = rt.pop();
        if (indep == +x && dep == +y)
            added = 1;
    }
    if (added != 1)
    {
        rt.invalid_function_error();
        return nullptr;
    }

    // Check that we have an acceptable return type
    if (!result->is_extended_algebraic())
    {
        rt.type_error();
        return nullptr;
    }
    return algebraic_p(result);
}


algebraic_p algebraic::evaluate() const
// ----------------------------------------------------------------------------
//   Evaluate an algebraic value as an algebraic
// ----------------------------------------------------------------------------
{
    stack_depth_restore sdr;
    if (program::run(this) != OK)
        return nullptr;

    if (rt.depth() != sdr.depth + 1)
    {
        rt.invalid_algebraic_error();
        return nullptr;
    }

    if (object_p obj = rt.pop())
    {
        while (tag_p tagged = obj->as<tag>())
            obj = tagged->tagged_object();
        if (obj->is_extended_algebraic())
            return algebraic_p(obj);
    }

    rt.type_error();
    return nullptr;
}


bool algebraic::is_numeric_constant() const
// ----------------------------------------------------------------------------
//  Return true if a value is a valid numerical constant in polynomials
// ----------------------------------------------------------------------------
{
    id ty = type();
    if (is_real(ty))
        return true;
    if (ty == ID_polar || ty == ID_rectangular)
    {
        complex_p z = complex_p(this);
        return z->x()->is_real() && z->y()->is_real();
    }
    return false;
}


algebraic_p algebraic::as_numeric_constant() const
// ----------------------------------------------------------------------------
//   Check if a value is a valid numerical constant (real or complex)
// ----------------------------------------------------------------------------
{
    if (is_numeric_constant())
        return this;
    return nullptr;
}


algebraic_p algebraic::zero_divide(algebraic_r x)
// ----------------------------------------------------------------------------
//   Deal with division by zero
// ----------------------------------------------------------------------------
{
    return rt.zero_divide(x && x->is_negative(false));
}


algebraic_p algebraic::epsilon(int impr)
// ----------------------------------------------------------------------------
//   Compute an epsilon value e.g. for numerical solver or integrator
// ----------------------------------------------------------------------------
{
    int         disp = Settings.DisplayDigits();
    int         prec = Settings.Precision();
    int         dig  = std::min(disp + 1, std::max(prec - impr, 3));
    algebraic_p eps  = decimal::make(1, -dig);
    return eps;
}


int algebraic::compare(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Return a comparison number beteen two valeus
// ----------------------------------------------------------------------------
{
    int result;
    if (x && y && comparison::compare(&result, x, y))
        return result;
    return 777;
}
