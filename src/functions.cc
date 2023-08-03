// ****************************************************************************
//  functions.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//
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

#include "functions.h"

#include "arithmetic.h"
#include "array.h"
#include "bignum.h"
#include "decimal128.h"
#include "equation.h"
#include "fraction.h"
#include "integer.h"
#include "list.h"


bool function::should_be_symbolic(id type)
// ----------------------------------------------------------------------------
//   Check if we should treat the type symbolically
// ----------------------------------------------------------------------------
{
    return is_strictly_symbolic(type);
}


algebraic_p function::symbolic(id op, algebraic_r x)
// ----------------------------------------------------------------------------
//    Check if we should process this function symbolically
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    return equation::make(op, x);
}


object::result function::evaluate(id op, bid128_fn op128, complex_fn zop)
// ----------------------------------------------------------------------------
//   Shared code for evaluation of all common math functions
// ----------------------------------------------------------------------------
{
    algebraic_g x = algebraic_p(rt.top());
    if (!x)
        return ERROR;
    x = evaluate(x, op ,op128, zop);
    if (x && rt.top(x))
        return OK;
    return ERROR;
}


static bid128 from_deg, from_grad, from_ratio;
static bool   init = false;

static void adjust_init()
// ----------------------------------------------------------------------------
//   Initialize the constants used for adjustments
// ----------------------------------------------------------------------------
{
    if (!init)
    {
        bid128_from_string(&from_deg.value,
                           "1.745329251994329576923690768488613E-2");
        bid128_from_string(&from_grad.value,
                           "1.570796326794896619231321691639752E-2");
        bid128_from_string(&from_ratio.value,
                           "3.141592653589793238462643383279503");
        init = true;
    }
}

void function::adjust_from_angle(bid128 &x)
// ----------------------------------------------------------------------------
//   Adjust an angle value for sin/cos/tan
// ----------------------------------------------------------------------------
{
    if (!init)
        adjust_init();
    switch(Settings.angle_mode)
    {
    case Settings.DEGREES:
        bid128_mul(&x.value, &x.value, &from_deg.value); break;
    case Settings.GRADS:
        bid128_mul(&x.value, &x.value, &from_grad.value); break;
    case Settings.PI_RADIANS:
        bid128_mul(&x.value, &x.value, &from_ratio.value); break;
    default:
    case Settings.RADIANS:
        break;
    }
}


void function::adjust_to_angle(bid128 &x)
// ----------------------------------------------------------------------------
//   Adjust an angle value for asin/acos/atan
// ----------------------------------------------------------------------------
{
    if (!init)
        adjust_init();
    switch(Settings.angle_mode)
    {
    case Settings.DEGREES:
        bid128_div(&x.value, &x.value, &from_deg.value); break;
    case Settings.GRADS:
        bid128_div(&x.value, &x.value, &from_grad.value); break;
    case Settings.PI_RADIANS:
        bid128_div(&x.value, &x.value, &from_ratio.value); break;
    default:
    case Settings.RADIANS:
        break;
    }
}


bool function::adjust_to_angle(algebraic_g &x)
// ----------------------------------------------------------------------------
//   Adjust an angle value for asin/acos/atan
// ----------------------------------------------------------------------------
{
    if (!init)
        adjust_init();
    if (x->is_real())
    {
        bid128 *adjust = nullptr;
        switch(Settings.angle_mode)
        {
        case Settings.DEGREES:          adjust = &from_deg;     break;
        case Settings.GRADS:            adjust = &from_grad;    break;
        case Settings.PI_RADIANS:       adjust = &from_ratio;   break;
        default:                                                break;
        }

        if (adjust)
        {
            algebraic_g div = rt.make<decimal128>(*adjust);
            x = x / div;
            return true;
        }
    }
    return false;
}


bool function::exact_trig(id op, algebraic_g &x)
// ----------------------------------------------------------------------------
//   Optimize cases where we can do exact trigonometry (avoid rounding)
// ----------------------------------------------------------------------------
//   This matters to get exact results for rectangular -> polar
{
    // When in radians mode, we cannot avoid rounding except for 0
    if (Settings.angle_mode == settings::RADIANS && !x->is_zero(false))
        return false;

    algebraic_g degrees = x;
    switch(Settings.angle_mode)
    {
    case settings::GRADS:
        degrees = degrees * integer::make(90) / integer::make(100);
        break;
    case settings::PI_RADIANS:
        degrees = degrees * integer::make(180);
        break;
    default:
        break;
    }

    ularge angle = 42;      // Not a special case...
    if (integer_p posint = degrees->as<integer>())
        angle = posint->value<ularge>();
    else if (const neg_integer *negint = degrees->as<neg_integer>())
        angle = 360 - negint->value<ularge>() % 360;
    else if (bignum_p posint = degrees->as<bignum>())
        angle = posint->value<ularge>();
    else if (const neg_bignum *negint = degrees->as<neg_bignum>())
        angle = 360 - negint->value<ularge>() % 360;
    angle %= 360;

    switch(op)
    {
    case ID_cos:
        angle = (angle + 90) % 360;
        // fallthrough
    case ID_sin:
        switch(angle)
        {
        case 0:
        case 180:       x = integer::make(0);  return true;
        case 270:       x = integer::make(-1); return true;
        case 90:        x = integer::make(1);  return true;
        case 30:
        case 150:       x = fraction::make(integer::make(1),
                                           integer::make(2)).Safe();
                        return true;
        case 210:
        case 330:       x = fraction::make(integer::make(-1),
                                           integer::make(2)).Safe();
                        return true;
        }
        return false;
    case ID_tan:
        switch(angle)
        {
        case 0:
        case 180:       x = integer::make(0);  return true;
        case 45:
        case 225:       x = integer::make(1);  return true;
        case 135:
        case 315:       x = integer::make(-1); return true;
        }
    default:
        break;
    }

    return false;
}


algebraic_p function::evaluate(algebraic_r xr,
                               id          op,
                               bid128_fn   op128,
                               complex_fn  zop)
// ----------------------------------------------------------------------------
//   Shared code for evaluation of all common math functions
// ----------------------------------------------------------------------------
{
    if (!xr.Safe())
        return nullptr;

    id xt = xr->type();
    algebraic_g x = xr;

    // Check if we are computing exact trigonometric values
    if (op >= ID_sin && op <= ID_tan)
        if (exact_trig(op, x))
            return x;

    if (should_be_symbolic(xt))
        return symbolic(op, xr);

    if (is_complex(xt))
    {
        complex_r z = (complex_r) xr;
        return algebraic_p(zop(z));
    }

    // Check if need to promote integer values to decimal
    if (is_integer(xt))
    {
        // Do not accept sin(#123h)
        if (!is_real(xt))
        {
            rt.type_error();
            return nullptr;
        }
    }

    // Call the right function
    // We need to only call the bid128 functions here, because the 32 and 64
    // variants are not in the DM42's QSPI, and take too much space here
    if (real_promotion(x, ID_decimal128))
    {
        bid128 xv = decimal128_p(algebraic_p(x))->value();
        bid128 res;
        if (op == ID_sin || op == ID_cos || op == ID_tan)
            adjust_from_angle(xv);
        op128(&res.value, &xv.value);
        int finite = false;
        bid128_isFinite(&finite, &res.value);
        if (!finite)
        {
            rt.domain_error();
            return nullptr;
        }
        if (op == ID_asin || op == ID_acos || op == ID_atan)
            adjust_to_angle(res);
        x = rt.make<decimal128>(ID_decimal128, res);
        return x;
    }

    // All other cases: report an error
    rt.type_error();
    return nullptr;
}


object::result function::evaluate(algebraic_fn op, bool mat)
// ----------------------------------------------------------------------------
//   Perform the operation from the stack, using a C++ operation
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id topty = top->type();
        if (topty == ID_list || (topty == ID_array && !mat))
        {
            top = list_p(top)->map(op);
        }
        else
        {
            algebraic_g x = algebraic_p(top);
            x = op(x);
            top = x.Safe();
        }
        if (top && rt.top(top))
            return OK;
    }
    return ERROR;
}



FUNCTION_BODY(abs)
// ----------------------------------------------------------------------------
//   Implementation of 'abs'
// ----------------------------------------------------------------------------
//   Special case where we don't need to promote argument to decimal128
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_abs, x);

    if (xt == ID_neg_integer  ||
        xt == ID_neg_bignum   ||
        xt == ID_neg_fraction ||
        xt == ID_neg_big_fraction)
    {
        // We can keep the object, just changing the type
        id absty = id(xt - 1);
        algebraic_p clone = algebraic_p(rt.clone(x));
        byte *tp = (byte *) clone;
        *tp = absty;
        return clone;
    }
    else if (is_integer(xt) || is_bignum(xt) || is_fraction(xt))
    {
        // No-op
        return x;
    }
    else if (is_complex(xt))
    {
        return complex_p(algebraic_p(x))->mod();
    }
    else if (xt == ID_array)
    {
        return array_p(algebraic_p(x))->norm();
    }

    // Fall-back to floating-point abs
    return function::evaluate(x, ID_abs, bid128_abs, nullptr);
}


FUNCTION_BODY(arg)
// ----------------------------------------------------------------------------
//   Implementation of the complex argument (0 for non-complex values)
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_arg, x);
    auto angle_mode = Settings.angle_mode;
    if (is_complex(xt))
        return complex_p(algebraic_p(x))->arg(angle_mode);
    algebraic_g zero = integer::make(0);
    bool negative = x->is_negative(false);
    return complex::convert_angle(zero, angle_mode, angle_mode, negative);
}


FUNCTION_BODY(re)
// ----------------------------------------------------------------------------
//   Extract the real part of a number
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_re, x);
    if (is_complex(xt))
        return complex_p(algebraic_p(x))->re();
    if (!is_real(xt))
        rt.type_error();
    return x;
}


FUNCTION_BODY(im)
// ----------------------------------------------------------------------------
//   Extract the imaginary part of a number (0 for real values)
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_im, x);
    if (is_complex(xt))
        return complex_p(algebraic_p(x))->im();
    if (!is_real(xt))
        rt.type_error();
    return integer::make(0);
}


FUNCTION_BODY(conj)
// ----------------------------------------------------------------------------
//   Compute the conjugate of input
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_conj, x);
    if (is_complex(xt))
        return complex_p(algebraic_p(x))->conjugate();
    if (!is_real(xt))
        rt.type_error();
    return x;
}


FUNCTION_BODY(sign)
// ----------------------------------------------------------------------------
//   Implementation of 'sign'
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_sign, x);

    if (x->is_negative(false))
    {
        return integer::make(-1);
    }
    else if (x->is_zero(false))
    {
        return integer::make(0);
    }
    else if (is_integer(xt) || is_bignum(xt) || is_fraction(xt) || is_real(xt))
    {
        return integer::make(1);
    }
    else if (is_complex(xt))
    {
        return polar::make(integer::make(1),
                           complex_p(algebraic_p(x))->pifrac(),
                           settings::PI_RADIANS);
    }

    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(inv)
// ----------------------------------------------------------------------------
//   Invert is implemented as 1/x
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (x->is_strictly_symbolic())
        return symbolic(ID_inv, x);
    else if (x->type() == ID_array)
        return array_p(x.Safe())->invert();

    algebraic_g one = rt.make<integer>(ID_integer, 1);
    return one / x;
}


INSERT_BODY(inv)
// ----------------------------------------------------------------------------
//   x⁻¹ is a postfix
// ----------------------------------------------------------------------------
{
    return ui.edit(o->fancy(), ui.POSTFIX);

}


FUNCTION_BODY(neg)
// ----------------------------------------------------------------------------
//   Negate is implemented as 0-x
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (x->is_strictly_symbolic())
        return symbolic(ID_neg, x);
    algebraic_g zero = rt.make<integer>(ID_integer, 0);
    return zero - x;
}


FUNCTION_BODY(sq)
// ----------------------------------------------------------------------------
//   Square is implemented using a multiplication
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (x->is_strictly_symbolic())
        return equation::make(ID_sq, x);
    return x * x;
}


INSERT_BODY(sq)
// ----------------------------------------------------------------------------
//   x² is a postfix
// ----------------------------------------------------------------------------
{
    return ui.edit(o->fancy(), ui.POSTFIX);

}


FUNCTION_BODY(cubed)
// ----------------------------------------------------------------------------
//   Cubed is implemented as two multiplications
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (x->is_strictly_symbolic())
        return equation::make(ID_cubed, x);
    return x * x * x;
}


INSERT_BODY(cubed)
// ----------------------------------------------------------------------------
//   x³ is a postfix
// ----------------------------------------------------------------------------
{
    return ui.edit(o->fancy(), ui.POSTFIX);

}


FUNCTION_BODY(fact)
// ----------------------------------------------------------------------------
//   Perform factorial for integer values, fallback to gamma otherwise
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;

    if (x->is_strictly_symbolic())
        return equation::make(ID_fact, x);

    if (integer_p ival = x->as<integer>())
    {
        ularge maxl = ival->value<ularge>();
        uint max = uint(maxl);
        if (max != maxl)
        {
            rt.domain_error();
            return nullptr;
        }
        algebraic_g result = integer::make(1);
        for (uint i = 2; i <= max; i++)
            result = result * integer::make(i);
        return result;
    }

    if (x->is_real() || x->is_complex())
        return tgamma::run(x + integer::make(1));

    rt.type_error();
    return nullptr;
}


INSERT_BODY(fact)
// ----------------------------------------------------------------------------
//   A factorial is inserted in postfix form in
// ----------------------------------------------------------------------------
{
    // We need to pass "x!' because ui.edit() strips the x
    return ui.edit(utf8("x!"), 2, ui.POSTFIX);
}


FUNCTION_BODY(Expand)
// ----------------------------------------------------------------------------
//   Expand equations
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (equation_p eq = x->as<equation>())
        return algebraic_p(eq->expand());
    if (x->is_algebraic())
        return x;
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(Collect)
// ----------------------------------------------------------------------------
//   Collect equations
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (equation_p eq = x->as<equation>())
        return algebraic_p(eq->collect());
    if (x->is_algebraic())
        return x;
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(Simplify)
// ----------------------------------------------------------------------------
//   Simplify equations
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    if (equation_p eq = x->as<equation>())
        return algebraic_p(eq->simplify());
    if (x->is_algebraic())
        return x;
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(ToDecimal)
// ----------------------------------------------------------------------------
//   Convert numbers to a decimal value
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    algebraic_g xg = x;
    if (rectangular_p z = x->as<rectangular>())
    {
        algebraic_g re = z->re();
        algebraic_g im = z->im();
        if (arithmetic::real_promotion(re) &&
            arithmetic::real_promotion(im))
            return rectangular::make(re, im);
    }
    else if (polar_p z = x->as<polar>())
    {
        algebraic_g mod = z->mod();
        algebraic_g arg = z->pifrac();
        if (arithmetic::real_promotion(mod) &&
            (mod->is_fraction() || arithmetic::real_promotion(arg)))
            return polar::make(mod, arg, settings::PI_RADIANS);
    }
    else if (arithmetic::real_promotion(xg))
    {
        return xg;
    }
    else if (xg->type() == ID_pi)
    {
        return algebraic::pi();
    }
    else if (xg->type() == ID_ImaginaryUnit)
    {
        return rectangular::make(integer::make(0),integer::make(1));
    }
    else
    {
        rt.type_error();
    }
    return nullptr;
}


FUNCTION_BODY(ToFraction)
// ----------------------------------------------------------------------------
//   Convert numbers to fractions
// ----------------------------------------------------------------------------
{
    if (!x.Safe())
        return nullptr;
    algebraic_g xg = x;
    if (rectangular_p z = x->as<rectangular>())
    {
        algebraic_g re = z->re();
        algebraic_g im = z->im();
        re = ToFraction::run(re);
        im = ToFraction::run(im);
        if (re.Safe() && im.Safe())
            return rectangular::make(re, im);
    }
    else if (polar_p z = x->as<polar>())
    {
        algebraic_g mod = z->mod();
        algebraic_g arg = z->pifrac();
        mod = ToFraction::run(mod);
        arg = ToFraction::run(arg);
        if (mod.Safe() && arg.Safe())
            return polar::make(mod, arg, settings::PI_RADIANS);
    }
    else if (arithmetic::decimal_to_fraction(xg))
    {
        return xg;
    }
    else
    {
        rt.type_error();
    }
    return nullptr;
}
