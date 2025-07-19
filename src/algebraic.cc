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
#include "array.h"
#include "bignum.h"
#include "compare.h"
#include "complex.h"
#include "constants.h"
#include "decimal.h"
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
    bool need_double = prec > 7;


    id xt = x->type();
    record(algebraic, "Real promotion of %p from %+s to hwfp",
           (object_p) x, object::name(xt));

    switch(xt)
    {
    case ID_hwfloat:
        if (need_double)
        {
            x = hwdouble::make(hwfloat_p(+x)->value());
            if (!x)
                return false;
        }
        return true;
    case ID_hwdouble:
        if (!need_double)
        {
            x = hwfloat::make(hwdouble_p(+x)->value());
            if (!x)
                return false;
        }
        return true;
    case ID_decimal:
    case ID_neg_decimal:
        if (need_double)
            x = hwdouble::make(decimal_p(+x)->to_double());
        else
            x = hwfloat::make(decimal_p(+x)->to_float());
        return x;

    case ID_integer:
        if (need_double)
            x = as_hwfp(double(integer_p(+x)->value<ularge>()));
        else
            x = as_hwfp(float(integer_p(+x)->value<ularge>()));
        return x;
    case ID_neg_integer:
        if (need_double)
            x = as_hwfp(-double(integer_p(+x)->value<ularge>()));
        else
            x = as_hwfp(-float(integer_p(+x)->value<ularge>()));
        return x;
    case ID_bignum:
    case ID_neg_bignum:
        x = decimal::from_bignum(bignum_p(+x));
        if (x && x->is_decimal())
        {
            if (need_double)
                x = as_hwfp(decimal_p(+x)->to_double());
            else
                x = as_hwfp(decimal_p(+x)->to_float());
        }
        return x;

    case ID_fraction:
        if (need_double)
            x = as_hwfp(double(fraction_p(+x)->numerator_value()) /
                        double(fraction_p(+x)->denominator_value()));
        else
            x = as_hwfp(float(fraction_p(+x)->numerator_value()) /
                        float(fraction_p(+x)->denominator_value()));
        return x;
    case ID_neg_fraction:
        if (need_double)
            x = as_hwfp(- double(fraction_p(+x)->numerator_value()) /
                        double(fraction_p(+x)->denominator_value()));
        else
            x = as_hwfp(- float(fraction_p(+x)->numerator_value()) /
                        float(fraction_p(+x)->denominator_value()));
        return x;
    case ID_big_fraction:
    case ID_neg_big_fraction:
        x = decimal::from_big_fraction(big_fraction_p(+x));
        if (x && x->is_decimal())
        {
            if (need_double)
                x = as_hwfp(decimal_p(+x)->to_double());
            else
                x = as_hwfp(decimal_p(+x)->to_float());
        }
        return x;
    default:
        return false;
    }
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

    if (xt == ID_uncertain)
    {
        // Convert from uncertain to range
        x = uncertain_p(+x)->as_range();
        return +x;
    }
    else if (xt == ID_range || xt == ID_drange || xt == ID_prange)
    {
        // Convert from range to uncertain
        x = range_p(+x)->as_uncertain();
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
        x = range::make(type, x, x);
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


bool algebraic::to_fraction(algebraic_g &x)
// ----------------------------------------------------------------------------
//  Check if we can promote the number to a fraction
// ----------------------------------------------------------------------------
{
    id ty = x->type();
    switch(ty)
    {
    case ID_hwfloat:
        x = hwfloat_p(+x)->to_fraction();
        break;
    case ID_hwdouble:
        x = hwdouble_p(+x)->to_fraction();
        break;
    case ID_decimal:
    case ID_neg_decimal:
        x = decimal_p(+x)->to_fraction();
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

    case ID_rectangular:
    {
        rectangular_p z = rectangular_p(+x);
        algebraic_g re = z->re();
        algebraic_g im = z->im();
        if (!to_fraction(re) || !to_fraction(im))
            return false;
        x = rectangular::make(re, im);
        break;
    }
    case ID_polar:
    {
        polar_p z = polar_p(+x);
        algebraic_g mod = z->mod();
        algebraic_g arg = z->pifrac();
        if (!to_fraction(mod) || !to_fraction(arg))
            return false;
        x = polar::make(mod, arg, object::ID_PiRadians);
        break;
    }
    case ID_range:
    case ID_drange:
    case ID_prange:
    case ID_uncertain:
    {
        range_p r = range_p(+x);
        algebraic_g lo = r->lo();
        algebraic_g hi = r->hi();
        if (!to_fraction(lo) || !to_fraction(hi))
            return false;
        x = range::make(r->type(), lo, hi);
        break;
    }
    case ID_unit:
    {
        unit_p ux = unit_p(+x);
        algebraic_g v = ux->value();
        algebraic_g u = ux->uexpr();
        if (to_fraction(v))
        {
            x = unit::simple(v, u);
            break;
        }
    }
    // fallthrough
    default:
        return false;
    }
    return x;                   // We need x to be non-null
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
    if (!rt.push(+x))
        return nullptr;
    rt.clear_error();
    save<object_g *> ival(expression::independent_value, (object_g *) &x);
    size_t   depth  = rt.depth();
    result   err    = eq->run();
    size_t   dnow   = rt.depth();
    object_p result = rt.pop();
    if (dnow == depth + 1)
    {
        object_p indep = rt.pop();
        dnow--;
        if (indep != +x)
        {
            rt.invalid_function_error();
            err = ERROR;
        }
    }
    if (!result || !result->is_algebraic())
    {
        rt.type_error();
        err = ERROR;
    }
    if (err != OK || (dnow != depth && dnow != depth + 1))
    {
        if (dnow > depth)
            rt.drop(dnow - depth);
        if (err == OK)
            rt.invalid_function_error();
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
