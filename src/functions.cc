// ****************************************************************************
//  functions.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Standard mathematical functions
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
#include "compare.h"
#include "conditionals.h"
#include "decimal.h"
#include "expression.h"
#include "fraction.h"
#include "integer.h"
#include "integrate.h"
#include "list.h"
#include "logical.h"
#include "polynomial.h"
#include "solve.h"
#include "tag.h"
#include "unit.h"


RECORDER(function,              16, "Evaluation of functions");
RECORDER(function_error,        16, "Errors during evaluation of functions");

bool function::should_be_symbolic(id type)
// ----------------------------------------------------------------------------
//   Check if we should treat the type symbolically
// ----------------------------------------------------------------------------
{
    return is_symbolic(type);
}


algebraic_p function::symbolic(id op, algebraic_r x)
// ----------------------------------------------------------------------------
//    Check if we should process this function symbolically
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (expression_p expr = x->as<expression>())
    {
        expression_g left, right;
        if (expr->split_equation(left, right))
        {
            algebraic_g l = +left;
            algebraic_g r = +right;
            l = expression::make(op, l);
            r = expression::make(op, r);
            return expression::make(ID_TestEQ, l, r);
        }
    }
    expression_p result = expression::make(op, x);
    if (result && !unit::factoring && Settings.AutoSimplify())
        result = result->simplify();
    return result;
}


bool function::has_symbolic_arguments(id type)
// ----------------------------------------------------------------------------
//   Check if the command has any symbolic arguments, e.g. Root or Sum
// ----------------------------------------------------------------------------
{
    return (type == ID_IFTE                     ||
            type == ID_Sum                      ||
            type == ID_Product                  ||
            type == ID_IFTE                     ||
            type == ID_Subst                    ||
            type == ID_Integrate                ||
            type == ID_Root                     ||
            type == ID_MultipleEquationsSolver  ||
            type == ID_Derivative               ||
            type == ID_Primitive);
}


bool function::is_symbolic_argument(id type, uint arg)
// ----------------------------------------------------------------------------
//   Check if the given argument needs to be stored in symbolic form
// ----------------------------------------------------------------------------
{
    switch(type)
    {
    case ID_Sum:
        return Sum::can_be_symbolic(arg);
    case ID_Product:
        return Product::can_be_symbolic(arg);
    case ID_IFTE:
        return IFTE::can_be_symbolic(arg);
    case ID_Subst:
        return Subst::can_be_symbolic(arg);
    case ID_Integrate:
        return Integrate::can_be_symbolic(arg);
    case ID_Root:
        return Root::can_be_symbolic(arg);
    case ID_MultipleEquationsSolver:
        return MultipleEquationsSolver::can_be_symbolic(arg);
    case ID_Derivative:
        return Derivative::can_be_symbolic(arg);
    case ID_Primitive:
        return Primitive::can_be_symbolic(arg);
    default:
        break;
    }
    return false;
}


object::result function::evaluate(id op, ops_t ops)
// ----------------------------------------------------------------------------
//   Shared code for evaluation of all common math functions
// ----------------------------------------------------------------------------
{
    algebraic_g x = algebraic_p(rt.top());
    if (!x)
        return ERROR;
    x = evaluate(x, op, ops);
    if (x && rt.top(x))
        return OK;
    return ERROR;
}


bool function::exact_trig(id op, algebraic_g &x)
// ----------------------------------------------------------------------------
//   Optimize cases where we can do exact trigonometry (avoid rounding)
// ----------------------------------------------------------------------------
//   This matters to get exact results for rectangular -> polar
{
    // When in radians mode, we cannot avoid rounding except for 0
    id amode = Settings.AngleMode();
    if (amode == ID_Rad && !x->is_zero(false))
        return false;

    algebraic_g degrees = x;
    switch(amode)
    {
    case object::ID_Grad:
        degrees = degrees * integer::make(90) / integer::make(100);
        break;
    case object::ID_PiRadians:
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
        case 150:       x = +fraction::make(integer::make(1),
                                            integer::make(2));
                        return true;
        case 210:
        case 330:       x = +fraction::make(integer::make(-1),
                                            integer::make(2));
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


algebraic_p function::evaluate(algebraic_r xr, id op, ops_t ops)
// ----------------------------------------------------------------------------
//   Shared code for evaluation of all common math functions
// ----------------------------------------------------------------------------
{
    if (!xr)
        return nullptr;

    algebraic_g x = xr;

    // Check if we are computing exact trigonometric values
    if (op >= ID_sin && op <= ID_tan)
    {
        if (id amode = adjust_angle(x))
        {
            settings::SaveAngleMode saved(amode);
            return evaluate(x, op, ops);
        }
        if (exact_trig(op, x))
            return x;
    }

    // Check if we need to add units
    if (op >= ID_asin && op <= ID_atan)
    {
        if (Settings.SetAngleUnits() && x->is_real())
        {
            settings::SaveSetAngleUnits save(false);
            x = evaluate(x, op, ops);
            add_angle(x);
            return x;
        }
    }

    // Check if we need to deal with units specially
    if (unit_p u = unit::get(x))
    {
        algebraic_g value = u->value();
        algebraic_g uexpr = u->uexpr();
        value = evaluate(value, op, ops);

        settings::SaveNumericalResults snr(false);
        save<bool> ueval(unit::mode, true);
        if (op == ID_sqrt || op == ID_cbrt)
        {
            save<bool> ufactor(unit::factoring, true);
            uint divisor = 2 + (op == ID_cbrt);
            algebraic_g exponent = +fraction::make(integer::make(1),
                                                   integer::make(divisor));
            uexpr = pow(uexpr, exponent);
            if (value && uexpr)
                return unit::make(value, uexpr);
            if (!rt.error())
                rt.inconsistent_units_error();
            return nullptr;
        }
        unit::factoring = false;
        uexpr = uexpr->evaluate();
        if (uexpr && uexpr->is_real())
        {
            value = value * uexpr;
            return value;
        }
        rt.inconsistent_units_error();
        return nullptr;
    }

    // Convert arguments to numeric if necessary
    if (Settings.NumericalResults())
        (void) to_decimal(x, true);   // May fail silently, and that's OK

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(op, x);

    if (is_complex(xt))
        return algebraic_p(ops.zop(complex_g(complex_p(+x))));

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

    // Call the right hardware-accelerated or decimal function
    algebraic_p result = nullptr;
    if (hwfp_promotion(x))
    {
        if (hwfloat_p fp = x->as<hwfloat>())
            result = ops.fop(fp);
        else if (hwdouble_p dp = x->as<hwdouble>())
            result = ops.dop(dp);
    }
    else if (decimal_promotion(x))
    {
        decimal_g xv = decimal_p(+x);
        xv = ops.decop(xv);
        if (xv && !xv->is_normal())
        {
            if (xv->is_infinity())
                return rt.numerical_overflow(xv->is_negative());
            rt.domain_error();
            return nullptr;
        }
        result = +xv;
    }
    else
    {
        // All other cases: report an error
        rt.type_error();
        return nullptr;
    }

    // If we got a null result, try promoting to complex
    if (!result && Settings.ComplexResults())
    {
        rt.clear_error();
        complex_g z = rectangular::make(x, integer::make(0));
        result = ops.zop(z);
    }

    return result;
}


object::result function::evaluate(algebraic_fn op, bool mat)
// ----------------------------------------------------------------------------
//   Perform the operation from the stack, using a C++ operation
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id topty = top->type();
        while(topty == ID_tag)
        {
            top = tag_p(top)->tagged_object();
            topty = top->type();
        }
        if (topty == ID_polynomial)
        {
            if (op == algebraic_fn(sq::evaluate) ||
                op == algebraic_fn(cubed::evaluate))
            {
                polynomial_g xp = polynomial_p(top);
                ularge exp = op == algebraic_fn(cubed::evaluate) ? 3 : 2;
                top = polynomial::pow(xp, exp);
                if (top && rt.top(top))
                    return OK;
                return ERROR;
            }
            else
            {
                top = polynomial_p(top)->as_expression();
            }
            topty = top ? top->type() : ID_expression;
        }
        if (topty == ID_list || (topty == ID_array && !mat))
        {
            top = list_p(top)->map(op);
        }
        else if (is_algebraic(topty) || (topty == ID_array && mat))
        {
            algebraic_g x = algebraic_p(top);
            x = op(x);
            top = +x;
        }
        else
        {
            rt.type_error();
            return ERROR;
        }
        if (top && rt.top(top))
            return OK;
    }
    return ERROR;
}


object::result function::evaluate(id op, nfunction_fn fn, uint arity,
                                  bool (*can_be_symbolic)(uint arg))
// ----------------------------------------------------------------------------
//   Perform the operation from the stack for n-ary functions
// ----------------------------------------------------------------------------
{
    if (!rt.args(arity))
        return ERROR;

    bool is_symbolic = false;
    algebraic_g args[arity];
    for (uint a = 0; a < arity; a++)
    {
        object_g oarg = rt.stack(a);
        while (tag_p tagged = oarg->as<tag>())
            oarg = tagged->tagged_object();
        algebraic_p arg = oarg->as_extended_algebraic();
        if (!arg)
        {
            rt.type_error();
            return ERROR;
        }
        args[a] = arg;
        if (!can_be_symbolic(a) && arg->is_symbolic())
            is_symbolic = true;

        // Conversion to numerical if needed (may fail silently)
        if (Settings.NumericalResults())
        {
            (void) to_decimal(args[a], true);
            if (!args[a])
                return ERROR;
        }
    }

    algebraic_g result;

    // Check the symbolic case
    if (is_symbolic)
        result = expression::make(op, args, arity);
    else
        result = fn(op, args, arity);

    if (result && rt.drop(arity) && rt.push(+result))
        return OK;
    return ERROR;
}


FUNCTION_BODY(neg)
// ----------------------------------------------------------------------------
//   Implementation of 'neg'
// ----------------------------------------------------------------------------
//   Special case where we don't need to promote argument to decimal
{
    if (!x)
        return nullptr;

    id xt = x->type();
    switch(xt)
    {
    case ID_expression:
    case ID_local:
    case ID_symbol:
    case ID_constant:
        return symbolic(ID_neg, x);

    case ID_integer:
    case ID_bignum:
    case ID_fraction:
    case ID_big_fraction:
    case ID_decimal:
    {
        // We can keep the object, just changing the type
        id negty = id(xt + 1);
        algebraic_p clone = algebraic_p(rt.clone(x));
        byte *tp = (byte *) clone;
        *tp = negty;
        return clone;
    }

    case ID_neg_integer:
    case ID_neg_bignum:
    case ID_neg_fraction:
    case ID_neg_big_fraction:
    case ID_neg_decimal:
    {
        // We can keep the object, just changing the type
        id negty = id(xt - 1);
        algebraic_p clone = algebraic_p(rt.clone(x));
        byte *tp = (byte *) clone;
        *tp = negty;
        return clone;
    }

    case ID_rectangular:
        return rectangular::make(-rectangular_p(+x)->re(),
                                 -rectangular_p(+x)->im());
    case ID_polar:
        return polar::make(-polar_p(+x)->mod(),
                           polar_p(+x)->arg(object::ID_PiRadians),
                           object::ID_PiRadians);

    case ID_unit:
        return unit::simple(neg::run(unit_p(+x)->value()),
                            unit_p(+x)->uexpr());
    case ID_tag:
    {
        algebraic_g tagged = tag_p(+x)->tagged_object()->as_algebraic();
        return evaluate(tagged);
    }

    case ID_array:
    case ID_list:
        return list_p(+x)->map(neg::evaluate);

    case ID_hwfloat:
        return hwfloat::neg((hwfloat::hwfp_r) x);
    case ID_hwdouble:
        return hwdouble::neg((hwdouble::hwfp_r) x);

    default:
        break;
    }

    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(abs)
// ----------------------------------------------------------------------------
//   Implementation of 'abs'
// ----------------------------------------------------------------------------
//   Special case where we don't need to promote argument to decimal
{
    if (!x)
        return nullptr;

    id xt = x->type();
    switch(xt)
    {
    case ID_expression:
    case ID_local:
    case ID_symbol:
    case ID_constant:
        return symbolic(ID_abs, x);

    case ID_integer:
    case ID_bignum:
    case ID_fraction:
    case ID_big_fraction:
    case ID_decimal:
        return x;

    case ID_neg_integer:
    case ID_neg_bignum:
    case ID_neg_fraction:
    case ID_neg_big_fraction:
    case ID_neg_decimal:
    {
        // We can keep the object, just changing the type
        id absty = id(xt - 1);
        algebraic_p clone = algebraic_p(rt.clone(x));
        byte *tp = (byte *) clone;
        *tp = absty;
        return clone;
    }

    case ID_rectangular:
    case ID_polar:
        return complex_p(+x)->mod();

    case ID_unit:
        return unit::simple(abs::run(unit_p(+x)->value()),
                            unit_p(+x)->uexpr());
    case ID_tag:
    {
        algebraic_g tagged = tag_p(+x)->tagged_object()->as_algebraic_or_list();
        return evaluate(tagged);
    }

    case ID_array:
        return array_p(+x)->norm();
    case ID_list:
        return list_p(+x)->map(abs::evaluate);

    case ID_hwfloat:
        return hwfloat::abs((hwfloat::hwfp_r) x);
    case ID_hwdouble:
        return hwdouble::abs((hwdouble::hwfp_r) x);

    default:
        break;
    }

    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(arg)
// ----------------------------------------------------------------------------
//   Implementation of the complex argument (0 for non-complex values)
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_arg, x);
    auto angle_mode = Settings.AngleMode();
    algebraic_g a;
    if (is_complex(xt))
    {
        a = complex_p(algebraic_p(x))->arg(angle_mode);
    }
    else
    {
        bool negative = x->is_negative(false);
        a = integer::make(0);
        a = complex::convert_angle(a, angle_mode, angle_mode, negative);
    }
    if (a && Settings.SetAngleUnits() && a->is_real())
        add_angle(a);
    return a;
}


FUNCTION_BODY(re)
// ----------------------------------------------------------------------------
//   Extract the real part of a number
// ----------------------------------------------------------------------------
{
    if (!x)
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
    if (!x)
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
    if (!x)
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
    if (!x)
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
                           object::ID_PiRadians);
    }

    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(IntPart)
// ----------------------------------------------------------------------------
//   Implementation of 'IP'
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_IntPart, x);

    if (is_decimal(xt))
        return decimal::IntPart(decimal_p(+x));
    if (is_real(xt))
    {
        // This code works for integer, fraction and decimal types
        algebraic_g one = integer::make(1);
        algebraic_g r = rem::evaluate(x, one);
        return x - r;
    }
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(FracPart)
// ----------------------------------------------------------------------------
//   Implementation of 'FP'
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_FracPart, x);

    if (is_decimal(xt))
        return decimal::FracPart(decimal_p(+x));
    if (is_real(xt))
    {
        algebraic_g one = integer::make(1);
        return rem::evaluate(x, one);
    }
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(ceil)
// ----------------------------------------------------------------------------
//   The `ceil` command returns the integer, or the integer immediately above
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_ceil, x);

    if (is_decimal(xt))
        return decimal::ceil(decimal_p(+x));
    if (is_real(xt))
    {
        algebraic_g one = integer::make(1);
        algebraic_g r = mod::evaluate(one - x, one);
        return x + r;
    }
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(floor)
// ----------------------------------------------------------------------------
//   The `floor` command returns the integer imediately below
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    id xt = x->type();
    if (should_be_symbolic(xt))
        return symbolic(ID_floor, x);

    if (is_decimal(xt))
        return decimal::floor(decimal_p(+x));
    if (is_real(xt))
    {
        algebraic_g one = integer::make(1);
        algebraic_g r = mod::evaluate(x, one);
        return x - r;
    }
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(inv)
// ----------------------------------------------------------------------------
//   Invert is implemented as 1/x
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_inv, x);
    else if (x->type() == ID_array)
        return array_p(+x)->invert();

    if (x->is_decimal())
        return decimal::inv(decimal_p(+x));
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


FUNCTION_BODY(sq)
// ----------------------------------------------------------------------------
//   Square is implemented using a multiplication
// ----------------------------------------------------------------------------
{
    if (!+x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_sq, x);
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
    if (!x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_cubed, x);
    return x * x * x;
}


FUNCTION_BODY(mant)
// ----------------------------------------------------------------------------
//   Return mantissa of object
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_mant, x);
    algebraic_g a = x;
    if (!decimal_promotion(a))
    {
        rt.type_error();
        return nullptr;
    }
    decimal_p d = decimal_p(+a);
    decimal::info i = d->shape();
    // The mantissa is always positive on HP calculators
    gcbytes bytes = i.base;
    return rt.make<decimal>(1, i.nkigits, bytes);
}


FUNCTION_BODY(xpon)
// ----------------------------------------------------------------------------
//   Return exponent of object
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_xpon, x);
    algebraic_g a = x;
    if (!decimal_promotion(a))
    {
        rt.type_error();
        return nullptr;
    }
    decimal_p d = decimal_p(+a);
    return integer::make(d->exponent() - 1LL);
}


static decimal_p round(decimal_p value, int digits)
// ----------------------------------------------------------------------------
//   Round to given number of digits, in the manner of HP48 RND function
// ----------------------------------------------------------------------------
//   The HP50G documentation states that:
//   * 0 through 11 rounds to n decimal places
//   * -1 through -11 rounds to n significant digits
//   * 12 rounds to the current display format
//   Obviously, that does not work with variable precision, so we replace
//   12 with the current precision. Also, experiment with HP calculators shows
//   that any value above number of actual digits is the same as 12, and that
//   negative values below -11 do nothing.
{
    large rndexp = digits;
    if (rndexp >= Settings.Precision())
        rndexp = Settings.DisplayDigits();
    if (rndexp >= 0)
        rndexp = -rndexp;
    else
        rndexp = value->exponent() + rndexp;
    return value->round(rndexp);
}


static decimal_p truncate(decimal_p value, int digits)
// ----------------------------------------------------------------------------
//   Truncate to given number of digits, in the manner of HP48 TRNC function
// ----------------------------------------------------------------------------
//   Same interpretation as for 'round' / 'RND'
{
    large rndexp = digits;
    if (rndexp >= Settings.Precision())
        rndexp = Settings.DisplayDigits();
    if (rndexp >= 0)
        rndexp = -rndexp;
    else
        rndexp = value->exponent() + rndexp;
    return value->truncate(rndexp);
}


static algebraic_p rnd_or_trnc(algebraic_r value, int digits,
                               decimal_p (*func)(decimal_p value, int digits))
// ----------------------------------------------------------------------------
//   Implementation of round or truncate
// ----------------------------------------------------------------------------
{
    object::id ty = value->type();
    switch (ty)
    {
    case object::ID_polar:
    {
        polar_p p = polar_p(+value);
        auto angles = Settings.AngleMode();
        algebraic_g mod = p->mod();
        algebraic_g arg = p->arg(angles);
        mod = rnd_or_trnc(mod, digits, func);
        arg = rnd_or_trnc(arg, digits, func);
        return polar::make(mod, arg, angles);
    }
    case object::ID_rectangular:
    {
        rectangular_p r = rectangular_p(+value);
        algebraic_g re = r->re();
        algebraic_g im = r->im();
        re = rnd_or_trnc(re, digits, func);
        im = rnd_or_trnc(im, digits, func);
        return rectangular::make(re, im);
    }
    case object::ID_unit:
    {
        unit_p u = unit_p(+value);
        algebraic_g v = u->value();
        algebraic_g x = u->uexpr();
        v = rnd_or_trnc(v, digits, func);
        return unit::make(v, x);
    }
    case object::ID_tag:
    {
        tag_p t = tag_p(+value);
        if (object_p tv = t->tagged_object())
        {
            if (algebraic_g alg = tv->as_algebraic_or_list())
            {
                size_t sz = 0;
                if (gcutf8 lbl = t->label_value(&sz))
                {
                    alg = rnd_or_trnc(alg, digits, func);
                    if (tag_p tg = tag::make(lbl, sz, +alg))
                        return algebraic_p(tg);
                }
            }
        }
        return nullptr;
    }
    case object::ID_array:
    case object::ID_list:
    {
        scribble scr;
        list_p l = list_p(+value);
        for (object_p obj : *l)
        {
            algebraic_g a = obj->as_algebraic_or_list();
            if (!a)
            {
                rt.type_error();
                return nullptr;
            }
            a = rnd_or_trnc(a, digits, func);
            if (!a)
                return nullptr;
            obj = +a;
            if (!rt.append(obj))
                return nullptr;
        }
        return list::make(ty, scr.scratch(), scr.growth());
    }
    case object::ID_integer:
    case object::ID_neg_integer:
    case object::ID_bignum:
    case object::ID_neg_bignum:
    case object::ID_fraction:
    case object::ID_neg_fraction:
    case object::ID_big_fraction:
    case object::ID_neg_big_fraction:
    case object::ID_hwfloat:
    case object::ID_hwdouble:
    {
        algebraic_g a = value;
        if (algebraic::decimal_promotion(a))
            return rnd_or_trnc(a, digits, func);
        return nullptr;
    }
    case object::ID_decimal:
    case object::ID_neg_decimal:
        return func(decimal_p(+value), digits);

    default:
        rt.type_error();
        return nullptr;
    }
}


NFUNCTION_BODY(Round)
// ----------------------------------------------------------------------------
//   Round to a given number of decimal places
// ----------------------------------------------------------------------------
{
    int digits = args[0]->as_int32(0, true);
    if (rt.error())
        return nullptr;
    return rnd_or_trnc(args[1], digits, round);
}



NFUNCTION_BODY(Truncate)
// ----------------------------------------------------------------------------
//   Round to a given number of decimal places
// ----------------------------------------------------------------------------
{
    int digits = args[0]->as_int32(0, true);
    if (rt.error())
        return nullptr;
    return rnd_or_trnc(args[1], digits, truncate);
}



NFUNCTION_BODY(xroot)
// ----------------------------------------------------------------------------
//   Compute the x-th root
// ----------------------------------------------------------------------------
{
    algebraic_g &x = args[expression::in_algebraic ? 1 : 0];
    if (x->is_zero())
    {
        rt.domain_error();
    }
    else
    {
        algebraic_g &y = args[expression::in_algebraic ? 0 : 1];
        bool is_int = x->is_integer();
        bool is_neg = false;
        if (!is_int && x->is_decimal())
        {
            decimal_g ip, fp;
            decimal_p xd = decimal_p(+x);
            if (!xd->split(ip, fp))
                return nullptr;
            if (fp->is_zero())
                is_int = true;
        }
        if (is_int)
        {
            bool is_odd = x->as_int32(0, false) & 1;
            is_neg = y->is_negative(false);
            if (is_neg && !is_odd)
            {
                // Root of a negative number
                rt.domain_error();
                return nullptr;
            }
        }

        if (is_neg)
            x = -pow(-y, integer::make(1) / x);
        else
            x = pow(y, integer::make(1) / x);
        return x;
    }
    return nullptr;
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
    if (!x)
        return nullptr;

    if (x->is_symbolic())
        return symbolic(ID_fact, x);

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

    if (x->is_decimal())
    {
        decimal_g xd = decimal_p(+x);
        xd = decimal::fact(xd);
        return xd;
    }

    if (x->is_real() || x->is_complex())
        return tgamma::run(x + algebraic_g(integer::make(1)));

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


NFUNCTION_BODY(comb)
// ----------------------------------------------------------------------------
//   Compute number of combinations
// ----------------------------------------------------------------------------
{
    algebraic_g &n = args[1];
    algebraic_g &m = args[0];
    if (integer_g nval = n->as<integer>())
    {
        if (integer_g mval = m->as<integer>())
        {
            ularge ni = nval->value<ularge>();
            ularge mi = mval->value<ularge>();
            n = integer::make(ni < mi ? 0 : 1);
            for (ularge i = ni - mi + 1; i <= ni && n; i++)
                n = n * algebraic_g(integer::make(i));
            for (ularge i = 2; i <= mi && n; i++)
                n = n / algebraic_g(integer::make(i));
            return n;
        }
    }

    if (n->is_real() && m->is_real())
        rt.value_error();
    else
        rt.type_error();
    return nullptr;
}


NFUNCTION_BODY(perm)
// ----------------------------------------------------------------------------
//  Compute number of permutations (n! / (n - m)!)
// ----------------------------------------------------------------------------
{
    algebraic_g &n = args[1];
    algebraic_g &m = args[0];
    if (integer_g nval = n->as<integer>())
    {
        if (integer_g mval = m->as<integer>())
        {
            ularge ni = nval->value<ularge>();
            ularge mi = mval->value<ularge>();
            n = integer::make(ni < mi ? 0 : 1);
            for (ularge i = ni - mi + 1; i <= ni && n; i++)
                n = n * algebraic_g(integer::make(i));
            return n;
        }
    }

    if (n->is_real() && m->is_real())
        rt.value_error();
    else
        rt.type_error();
    return nullptr;
}


static algebraic_p sum_product(object::id op,
                               algebraic_g args[], uint arity)
// ----------------------------------------------------------------------------
//   Perform a sum or product on the operations
// ----------------------------------------------------------------------------
{
    if (arity != 4)
    {
        rt.internal_error();
        return nullptr;
    }
    symbol_g name = args[3]->as_quoted<symbol>();
    if (!name)
    {
        rt.type_error();
        return nullptr;
    }

    algebraic_g &init = args[2];
    algebraic_g &last = args[1];
    algebraic_g &expr = args[0];

    if (!expr->is_program())
    {
        rt.invalid_equation_error();
        return nullptr;
    }

    if (init->is_integer() && last->is_integer())
    {
        program_g        prg  = program_p(+expr);
        large            a    = init->as_int64(0, false);
        large            b    = last->as_int64(0, false);
        save<symbol_g *> iref(expression::independent, &name);

        if (op == object::ID_mul)
        {
            init = integer::make(1);
            for (large i = a; i <= b && init; i++)
            {
                last = integer::make(i);
                last = algebraic::evaluate_function(prg, last);
                if (!last || program::interrupted())
                    return nullptr;
                init = init * last;
            }
        }
        else
        {
            init = integer::make(0);
            for (large i = a; i <= b && init; i++)
            {
                last = integer::make(i);
                last = algebraic::evaluate_function(prg, last);
                if (!last || program::interrupted())
                    return nullptr;
                init = init + last;
            }
        }
        return init;
    }
    else if (init->is_real() && last->is_real())
    {
        program_g        prg  = program_p(+expr);
        save<symbol_g *> iref(expression::independent, &name);
        bool             product = op == object::ID_mul;
        algebraic_g      result  = integer::make(product ? 1 : 0);
        algebraic_g      one     = integer::make(1);
        while (!program::interrupted())
        {
            algebraic_g tmp = (init > last);
            if (!tmp || tmp->as_truth(false))
                break;

            tmp = algebraic::evaluate_function(prg, init);
            if (!tmp)
                return nullptr;
            result = product ? result * tmp : result + tmp;
            init = init + one;
        }
        return result;
    }
    else
    {
        rt.type_error();
    }
    return nullptr;
}


NFUNCTION_BODY(Sum)
// ----------------------------------------------------------------------------
//   Sum operation
// ----------------------------------------------------------------------------
{
    return sum_product(ID_add, args, arity);
}


NFUNCTION_BODY(Product)
// ----------------------------------------------------------------------------
//   Product operation
// ----------------------------------------------------------------------------
{
    return sum_product(ID_mul, args, arity);
}


FUNCTION_BODY(ToDecimal)
// ----------------------------------------------------------------------------
//   Convert numbers to a decimal value
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    algebraic_g xg = x;
    if (algebraic::to_decimal(xg, false))
        return +xg;
    return nullptr;
}


FUNCTION_BODY(ToFraction)
// ----------------------------------------------------------------------------
//   Convert numbers to fractions
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    algebraic_g xg = x;
    if (arithmetic::decimal_to_fraction(xg))
        return xg;
    if (!rt.error())
        rt.type_error();
    return nullptr;
}


FUNCTION_BODY(RadiansToDegrees)
// ----------------------------------------------------------------------------
//   Compatibility function for R->D on HP-48
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    algebraic_g xg = integer::make(180);
    xg = xg / pi();
    xg = xg * x;
    return xg;
}


FUNCTION_BODY(DegreesToRadians)
// ----------------------------------------------------------------------------
//   Compatibility function for D->R on HP-48
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    algebraic_g xg = integer::make(180);
    xg = pi() / xg;
    xg = xg * x;
    return xg;
}



// ============================================================================
//
//   Min and Max operations
//
// ============================================================================

static algebraic_p min_max(algebraic_r x, algebraic_r y,
                           int sign, arithmetic_fn mapfn)
// ----------------------------------------------------------------------------
//   Compute min / max
// ----------------------------------------------------------------------------
{
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
        {
            auto xi = xa->begin();
            auto xe = xa->end();
            auto yi = ya->begin();
            auto ye = ya->end();
            array_g ra = array_p(rt.make<array>(nullptr, 0));
            algebraic_g xo, yo;
            while (xi != xe && yi != ye)
            {
                object_p xobj = *xi++;
                if (!xobj->is_algebraic())
                    return nullptr;
                object_p yobj = *yi++;
                if (!yobj->is_algebraic())
                    return nullptr;
                xo = algebraic_p(xobj);
                yo = algebraic_p(yobj);
                xo = min_max(xo, yo, sign, mapfn);
                if (!xo)
                    return nullptr;
                ra = ra->append(+xo);
            }
            if (xi != xe || yi != ye)
            {
                rt.dimension_error();
                return nullptr;
            }
            return ra;
        }
        return xa->map(mapfn, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, mapfn);
    }

    int cmp = 0;
    if (comparison::compare(&cmp, x, y))
        return sign * cmp > 0 ? x : y;
    return nullptr;
}


algebraic_p Min::evaluate(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//  Evaluation with arithmetic arguments, e.g. within lists
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    if (x->is_symbolic() || y->is_symbolic())
        return expression::make(ID_Min, x, y);
    return min_max(x, y, -1, Min::evaluate);
}


algebraic_p Max::evaluate(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//  Evaluation with arithmetic arguments, e.g. within lists
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    if (x->is_symbolic() || y->is_symbolic())
        return expression::make(ID_Max, x, y);
    return min_max(x, y, 1, Max::evaluate);
}


NFUNCTION_BODY(Min)
// ----------------------------------------------------------------------------
//   Process the Min command
// ----------------------------------------------------------------------------
{
    algebraic_g x = args[0]->as_extended_algebraic();
    algebraic_g y = args[1]->as_extended_algebraic();
    return evaluate(x, y);
}


NFUNCTION_BODY(Max)
// ----------------------------------------------------------------------------
//   Process the Max command
// ----------------------------------------------------------------------------
{
    algebraic_g x = args[0]->as_extended_algebraic();
    algebraic_g y = args[1]->as_extended_algebraic();
    return evaluate(x, y);
}



// ============================================================================
//
//    Percentage operations
//
// ============================================================================

NFUNCTION_BODY(Percent)
// ----------------------------------------------------------------------------
//   Evaluate percentage operation
// ----------------------------------------------------------------------------
{
    algebraic_r x = args[0];
    algebraic_r y = args[1];
    algebraic_g hundred = integer::make(100);
    return x * (y / hundred);
}


NFUNCTION_BODY(PercentChange)
// ----------------------------------------------------------------------------
//   Evaluate percentage change operation
// ----------------------------------------------------------------------------
{
    algebraic_r x = args[0];
    algebraic_r y = args[1];
    algebraic_g one = integer::make(1);
    algebraic_g hundred = integer::make(100);
    return (x/y - one) * hundred;
}


NFUNCTION_BODY(PercentTotal)
// ----------------------------------------------------------------------------
//   Evaluate percentage total operation
// ----------------------------------------------------------------------------
{
    algebraic_r x = args[0];
    algebraic_r y = args[1];
    algebraic_g hundred = integer::make(100);
    return x/y * hundred;
}
