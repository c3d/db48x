// ****************************************************************************
//  arithmetic.cc                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of basic arithmetic operations
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

#include "arithmetic.h"

#include "array.h"
#include "bignum.h"
#include "compare.h"
#include "constants.h"
#include "datetime.h"
#include "decimal.h"
#include "expression.h"
#include "fraction.h"
#include "functions.h"
#include "integer.h"
#include "list.h"
#include "polynomial.h"
#include "runtime.h"
#include "settings.h"
#include "tag.h"
#include "text.h"
#include "unit.h"

#include <bit>
#include <bitset>


RECORDER(arithmetic,            16, "Arithmetic");
RECORDER(arithmetic_error,      16, "Errors from arithmetic code");

bool arithmetic::complex_promotion(algebraic_g &x, algebraic_g &y)
// ----------------------------------------------------------------------------
//   Return true if one type is complex and the other can be promoted
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return false;

    id xt = x->type();
    id yt = y->type();

    // If both are complex, we do not do anything: Complex ops know best how
    // to handle mixed inputs (mix of rectangular and polar). We should leave
    // it to them to handle the different representations.
    if (is_complex(xt) && is_complex(yt))
        return true;

    // Try to convert both types to the same complex type
    if (is_complex(xt))
        return complex_promotion(y, xt);
    if (is_complex(yt))
        return complex_promotion(x, yt);

    // Neither type is complex, no point to promote
    return false;
}


bool arithmetic::range_promotion(algebraic_g &x, algebraic_g &y)
// ----------------------------------------------------------------------------
//   Return true if one type is range and the other can be promoted
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return false;

    id xt = x->type();
    id yt = y->type();

    // If both are ranges, we do not do anything
    if (is_range(xt) && is_range(yt))
        return true;

    // Try to convert both types to the same range type
    if (is_range(xt))
        return range_promotion(y, xt);
    if (is_range(yt))
        return range_promotion(x, yt);

    // Neither type is range, no point to promote
    return false;
}


fraction_p arithmetic::fraction_promotion(algebraic_g &x)
// ----------------------------------------------------------------------------
//  Check if we can promote the number to a fraction
// ----------------------------------------------------------------------------
{
    id ty = x->type();
    if (is_fraction(ty))
        return fraction_g((fraction *) object_p(x));
    if (ty >= ID_integer && ty <= ID_neg_integer)
    {
        integer_g n = integer_p(object_p(x));
        integer_g d = integer::make(1);
        fraction_p f = fraction::make(n, d);
        return f;
    }
    if (ty >= ID_bignum && ty <= ID_neg_bignum)
    {
        bignum_g n = bignum_p(object_p(x));
        bignum_g d = bignum::make(1);
        fraction_p f = big_fraction::make(n, d);
        return f;
    }
    return nullptr;
}


template<>
algebraic_p arithmetic::optimize<add>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Auto-simplification rules for addition
// ----------------------------------------------------------------------------
{
    // Deal with basic auto-simplifications rules
    if (Settings.AutoSimplify())
    {
        int xinf = x->is_infinity();
        int yinf = y->is_infinity();
        if (xinf || yinf)
        {
            if (xinf && yinf)
            {
                if (xinf == yinf)
                    return x;
                rt.undefined_operation_error();
                return nullptr;
            }
            return xinf ? x : y;
        }
        if  (x->is_simplifiable() && y->is_simplifiable())
        {
            if (x->is_zero(false) && !x->is_based()) // 0 + X = X
                return y;
            if (y->is_zero(false) && !y->is_based()) // X + 0 = X
                return x;
        }
    }
    return nullptr;
}


template<>
algebraic_p arithmetic::non_numeric<add>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for addition
// ----------------------------------------------------------------------------
//   This deals with:
//   - Text + text: Concatenation of text
//   - Text + object: Concatenation of text + object text
//   - Object + text: Concatenation of object text + text
{
    // list + ...
    if (list_g xl = x->as<list>())
    {
        if (list_g yl = y->as<list>())
            return xl + yl;
        if (list_g yl = rt.make<list>(byte_p(+y), y->size()))
            return xl + yl;
    }
    else if (list_g yl = y->as<list>())
    {
        if (list_g xl = rt.make<list>(byte_p(+x), x->size()))
            return xl + yl;
    }

    // text + ...
    if (text_g xs = x->as<text>())
    {
        // text + text
        if (text_g ys = y->as<text>())
            return xs + ys;
        // text + object
        if (text_g ys = y->as_text())
            return xs + ys;
    }
    // ... + text
    else if (text_g ys = y->as<text>())
    {
        // object + text
        if (text_g xs = x->as_text())
            return xs + ys;
    }

    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa + ya;
        return xa->map(add::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, add::evaluate);
    }

    // Check addition of unit objects
    if (unit_g xu = unit::get(x))
    {
        // A zero can be generated by simplification rules
        if (y->is_zero(false))
            return xu;

        if (!unit::nodates)
        {
            if (algebraic_p daf = days_after(x, y, false))
                return daf;
            if (algebraic_p daf = days_after(y, x, false))
                return daf;
        }

        if (unit_g yu = unit::get_after_evaluation(y))
        {
            if (yu->convert(xu))
            {
                algebraic_g xv = xu->value();
                algebraic_g yv = yu->value();
                algebraic_g ye = yu->uexpr();
                xv = xv + yv;
                return unit::simple(xv, ye);
            }
            return nullptr;
        }
        save<bool> sumode(unit::mode, true);
        algebraic_g ubased = xu->evaluate();
        if (!ubased || ubased->type() == ID_unit)
        {
            rt.inconsistent_units_error();
            return nullptr;
        }
        return add::evaluate(ubased, y);
    }
    else if (unit_g yu = unit::get(y))
    {
        // A zero can be generated by simplification rules
        if (x->is_zero(false))
            return yu;
        if (!unit::nodates)
            if (algebraic_p daf = days_after(yu, x, false))
                return daf;

        save<bool> sumode(unit::mode, true);
        algebraic_g ubased = yu->evaluate();
        if (!ubased || ubased->type() == ID_unit)
        {
            rt.inconsistent_units_error();
            return nullptr;
        }
        return add::evaluate(x, ubased);
    }

    return optimize<add>(x, y);
}


bool add::integer_ok(object::id &xt, object::id &yt,
                     ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Check if adding two integers works or if we need to promote to real
// ----------------------------------------------------------------------------
{
    // For integer types of the same sign, promote to real if we overflow
    if ((xt == ID_neg_integer) == (yt == ID_neg_integer))
    {
        ularge sum = xv + yv;

        // Do not promot to real if we have based numbers as input
        if ((sum < xv || sum < yv) && is_real(xt) && is_real(yt))
            return false;

        xv = sum;
        // Here, the type of x is the type of the result
        return true;
    }

    // Opposite sign: the difference in magnitude always fit in an integer type
    if (!is_real(xt))
    {
        // Based numbers keep the base of the number in X
        xv = xv - yv;
    }
    else if (yv >= xv)
    {
        // Case of (-3) + (+2) or (+3) + (-2): Change the sign of X
        xv = yv - xv;
        xt = (xv == 0 || xt == ID_neg_integer) ? ID_integer : ID_neg_integer;
    }
    else
    {
        // Case of (-3) + (+4) or (+3) + (-4): Keep the sign of X
        xv = xv - yv;
    }
    return true;
}


bool add::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always add two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x + y;
    return x;
}


bool add::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always add two fractions
// ----------------------------------------------------------------------------
{
    x = x + y;
    return x;
}


bool add::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Add complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x + y;
    return x;
}


static bool range_binary(range_g &x, range_g &y,
                         range_p (*rfn)(range_r, range_r),
                         uncertain_p (*ufn)(uncertain_r, uncertain_r))
// ----------------------------------------------------------------------------
//   Check if we deal with an uncertain number or with regular ranges
// ----------------------------------------------------------------------------
{
    if (x->type() == object::ID_uncertain)
    {
        if (y->type() == object::ID_uncertain)
        {
            x = ufn((uncertain_r) x, (uncertain_r) y);
            return x;
        }
    }
    x = rfn(x, y);
    return x;
}


bool add::range_ok(range_g &x, range_g &y)
// ----------------------------------------------------------------------------
//   Add ranges if we have them
// ----------------------------------------------------------------------------
{
    return range_binary(x, y, operator+, operator+);
}


template <>
algebraic_p arithmetic::optimize<subtract>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Optimizations for subtractions
// ----------------------------------------------------------------------------
{
    // Deal with basic auto-simplifications rules
    if (Settings.AutoSimplify())
    {
        int xinf = x->is_infinity();
        int yinf = y->is_infinity();
        if (xinf || yinf)
        {
            if (xinf && yinf)
            {
                if (xinf != yinf)
                    return x;
                rt.undefined_operation_error();
                return nullptr;
            }
            return xinf ? +x : rt.infinity(yinf > 0);
        }
        if  (x->is_simplifiable() && y->is_simplifiable())
        {
            if (y->is_zero(false) && !y->is_based()) // X - 0 = X
                return x;
            if (x->is_same_as(y))                   // X - X = 0
                return integer::make(0);
            if (x->is_zero(false) && !x->is_based() && y->is_symbolic())
                return neg::run(y);                 // 0 - X = -X
        }
    }

    // Not yet implemented
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<subtract>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
//   This deals with vector and matrix operations
{
    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa - ya;
        return xa->map(subtract::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, subtract::evaluate);
    }

    // Check subtraction of unit objects
    if (unit_g xu = unit::get(x))
    {
        // A zero can be generted by simplification rules
        if (y->is_zero(false))
            return xu;

        if (!unit::nodates)
            if (algebraic_p dbef = days_before(x, y, false))
                return dbef;
        if (unit_g yu = unit::get_after_evaluation(y))
        {
            if (!unit::nodates)
                if (algebraic_p ddays = days_between_dates(x, y, false))
                    return ddays;

            if (yu->convert(xu))
            {
                algebraic_g xv = xu->value();
                algebraic_g yv = yu->value();
                algebraic_g ye = yu->uexpr();
                xv = xv - yv;
                return unit::simple(xv, ye);
            }
        }
        save<bool> sumode(unit::mode, true);
        algebraic_g ubased = xu->evaluate();
        if (!ubased || ubased->type() == ID_unit)
        {
            rt.inconsistent_units_error();
            return nullptr;
        }
        return subtract::evaluate(ubased, y);
    }
    else if (unit_g yu = unit::get(y))
    {
        // A zero can be generated by simplification rules
        if (x->is_zero(false))
        {
            algebraic_g value = yu->value();
            return unit::simple(-value, yu->uexpr());
        }

        save<bool> sumode(unit::mode, true);
        algebraic_g ubased = yu->evaluate();
        if (!ubased || ubased->type() == ID_unit)
        {
            rt.inconsistent_units_error();
            return nullptr;
        }
        return subtract::evaluate(x, ubased);
    }
    return optimize<subtract>(x, y);
}


bool subtract::integer_ok(object::id &xt, object::id &yt,
                          ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Check if subtracting two integers works or if we need to promote to real
// ----------------------------------------------------------------------------
{
    // For integer types of opposite sign, promote to real if we overflow
    if ((xt == ID_neg_integer) != (yt == ID_neg_integer))
    {
        ularge sum = xv + yv;
        if ((sum < xv || sum < yv) && is_real(xt) && is_real(yt))
            return false;
        xv = sum;

        // The type of x gives us the correct sign for the difference:
        //   -2 - 3 is -5, 2 - (-3) is 5:
        return true;
    }

    // Same sign: the difference in magnitude always fit in an integer type
    if (!is_real(xt))
    {
        // Based numbers keep the base of the number in X
        xv = xv - yv;
    }
    else if (yv >= xv)
    {
        // Case of (+3) - (+4) or (-3) - (-4): Change the sign of X
        xv = yv - xv;
        xt = (xv == 0 || xt == ID_neg_integer) ? ID_integer : ID_neg_integer;
    }
    else
    {
        // Case of (-3) - (-2) or (+3) - (+2): Keep the sign of X
        xv = xv - yv;
    }
    return true;
}


bool subtract::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always subtract two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x - y;
    return x;
}


bool subtract::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always subtract two fractions (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x - y;
    return x;
}


bool subtract::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Subtract complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x - y;
    return x;
}


bool subtract::range_ok(range_g &x, range_g &y)
// ----------------------------------------------------------------------------
//   Subtract ranges if we have them
// ----------------------------------------------------------------------------
{
    return range_binary(x, y, operator-, operator-);
}


template <>
algebraic_p arithmetic::optimize<multiply>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Optimization rules for multiplication
// ----------------------------------------------------------------------------
{
    // Deal with basic auto-simplifications rules
    if (Settings.AutoSimplify())
    {
        int xinf = x->is_infinity();
        int yinf = y->is_infinity();
        if (xinf || yinf)
        {
            if (xinf && yinf)
                return rt.infinity(xinf * yinf < 0);
            if (x->is_zero(false) || y->is_zero(false))
            {
                rt.undefined_operation_error();
                return nullptr;
            }
            return rt.infinity(x->is_negative() ^ y->is_negative());
        }

        if (x->is_simplifiable() && y->is_simplifiable())
        {
            if (x->is_zero(false) && !x->is_based()) // 0 * X = 0
                return x;
            if (y->is_zero(false) && !y->is_based()) // X * 0 = Y
                return y;
            if (x->is_one(false) && !x->is_based()) // 1 * X = X
                return y;
            if (y->is_one(false) && !y->is_based()) // X * 1 = X
                return x;
            if (x->is_symbolic() && x->is_same_as(y))
            {
                if (constant_p cst = x->as<constant>())
                    if (cst->is_imaginary_unit())
                        return integer::make(-1);
                return sq::run(x);                  // X * X = X²
            }
        }
    }

    // Not yet implemented
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<multiply>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
//   This deals with:
//   - Text * integer: Repeat the text
//   - Integer * text: Repeat the text
{
    // Text multiplication
    if (text_g xs = x->as<text>())
        if (integer_g yi = y->as<integer>())
            return xs * yi->value<uint>();
    if (text_g ys = y->as<text>())
        if (integer_g xi = x->as<integer>())
            return ys * xi->value<uint>();
    if (list_g xl = x->as<list>())
        if (integer_g yi = y->as<integer>())
            return xl * yi->value<uint>();
    if (list_g yl = y->as<list>())
        if (integer_g xi = x->as<integer>())
            return yl * xi->value<uint>();

    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa * ya;
        return xa->map(multiply::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, multiply::evaluate);
    }

    // Check multiplication of unit objects
    if (unit_p xu = unit::get(x))
    {
        algebraic_g xv = xu->value();
        algebraic_g xe = xu->uexpr();

        if (unit_p yu = unit::get(y))
        {
            algebraic_g yv = yu->value();
            algebraic_g ye = yu->uexpr();

            unit::convert_to_linear(xv, xe);
            unit::convert_to_linear(yv, ye);

            xv = xv * yv;
            {
                save<bool> umode(unit::mode, true);
                xe = xe * ye;
            }
            return unit::simple(xv, xe);
        }
        else if (!y->is_symbolic() || xv->is_one())
        {
            xv = xv * y;
            return unit::simple(xv, xe);
        }
    }
    else if (unit_p yu = unit::get(y))
    {
        algebraic_g yv = yu->value();
        if (!x->is_symbolic() || yv->is_one())
        {
            algebraic_g ye = yu->uexpr();
            yv = x * yv;
            return unit::simple(yv, ye);
        }
    }
    return optimize<multiply>(x, y);
}

bool multiply::integer_ok(object::id &xt, object::id &yt,
                          ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Check if multiplying two integers works or if we need to promote to real
// ----------------------------------------------------------------------------
{
    // If one of the two objects is a based number, always use integer mul
    if (!is_real(xt) || !is_real(yt))
    {
        xv = xv * yv;
        return true;
    }

    // Check if there is an overflow
    // Can's use std::countl_zero yet (-std=c++20 breaks DMCP)
    if (std::__countl_zero(xv) + std::__countl_zero(yv) < int(8*sizeof(ularge)))
        return false;

    // Check if the multiplication generates a larger result. Is this correct?
    ularge product = xv * yv;

    // Check the sign of the product
    xt = (xt == ID_neg_integer) == (yt == ID_neg_integer)
        ? ID_integer
        : ID_neg_integer;
    xv = product;
    return true;
}


bool multiply::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always multiply two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x * y;
    return x;
}


bool multiply::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always multiply two fractions (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x * y;
    return x;
}


bool multiply::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Multiply complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x * y;
    return x;
}


bool multiply::range_ok(range_g &x, range_g &y)
// ----------------------------------------------------------------------------
//   Multiply ranges if we have them
// ----------------------------------------------------------------------------
{
    return range_binary(x, y, operator*, operator*);
}


template <>
algebraic_p arithmetic::optimize<divide>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Optimizations for division
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (y->is_zero(false))
        return zero_divide(x, y);

    // Deal with basic auto-simplifications rules
    if (Settings.AutoSimplify())
    {
        int xinf = x->is_infinity();
        int yinf = y->is_infinity();
        if (xinf || yinf)
        {
            if (xinf && yinf)
            {
                rt.undefined_operation_error();
                return nullptr;
            }
            if (yinf && x->is_real())
                return integer::make(0);            // 1 / ∞ = 0
            if (xinf && y->is_real())
                return rt.infinity((xinf < 0) ^ y->is_negative());
        }

        if (x->is_simplifiable() && y->is_simplifiable())
        {
            if (x->is_zero(false) && !x->is_based()) // 0 / X = 0
                return x;
            if (y->is_one(false) && !y->is_based()) // X / 1 = X
                return x;
            if (x->is_one(false) && y->is_symbolic())
                return inv::run(y);                 // 1 / X = X⁻¹
            if (x->is_same_as(y))
                return integer::make(1);            // X / X = 1
        }
    }

    // Not yet implemented
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<divide>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for division
// ----------------------------------------------------------------------------
//   This deals with vector and matrix operations
{
    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa / ya;
        return xa->map(divide::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, divide::evaluate);
    }

    // Check division of unit objects
    if (unit_p xu = unit::get(x))
    {
        algebraic_g xv = xu->value();
        algebraic_g xe = xu->uexpr();

        if (unit_p yu = unit::get(y))
        {
            algebraic_g yv = yu->value();
            algebraic_g ye = yu->uexpr();

            if (xe && ye)
            {
                if (xe->is_same_as(ye))
                {
                    // Two identical units: just return ratio
                    xv = xv / yv;
                    return xv;
                }
                else
                {
                    unit::convert_to_linear(xv, xe);
                    unit::convert_to_linear(yv, ye);
                }
            }
            xv = xv / yv;
            {
                save<bool> umode(unit::mode, true);
                xe = xe / ye;
            }
            return unit::simple(xv, xe);
        }
        else if (!y->is_symbolic())
        {
            xv = xv / y;
            return unit::simple(xv, xe);
        }
    }
    else if (unit_p yu = unit::get(y))
    {
        if (!x->is_symbolic())
        {
            algebraic_g yv = yu->value();
            algebraic_g ye = yu->uexpr();
            yv = x / yv;
            save<bool> umode(unit::mode, true);
            save<bool> ufact(unit::factoring, true);
            ye = inv::run(ye);
            return unit::simple(yv, ye);
        }
    }

    return optimize<divide>(x, y);
}


bool divide::integer_ok(object::id &xt, object::id &yt,
                        ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Check if dividing two integers works or if we need to promote to real
// ----------------------------------------------------------------------------
{
    ASSERT(yv != 0 && "integer_ok divide by zero, optimize<divide> failed?");

    // If one of the two objects is a based number, always used integer div
    if (!is_real(xt) || !is_real(yt))
    {
        xv = xv / yv;
        return true;
    }

    // Check if there is a remainder - If so, switch to fraction
    if (xv % yv)
        return false;

    // Perform the division
    xv = xv / yv;

    // Check the sign of the ratio
    xt = (xt == ID_neg_integer) == (yt == ID_neg_integer)
        ? ID_integer
        : ID_neg_integer;
    return true;
}


bool divide::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Division works if there is no remainder
// ----------------------------------------------------------------------------
{
    ASSERT(y && "bignum divide by zero, optimize<divide> failed");

    bignum_g q = nullptr;
    bignum_g r = nullptr;
    id type = bignum::product_type(x->type(), y->type());
    bool result = bignum::quorem(x, y, type, &q, &r);
    if (result)
        result = bignum_p(r) != nullptr;
    if (result)
    {
        if (is_based(type) || r->is_zero())
            x = q;                  // Integer result
        else
            x = bignum_p(fraction_p(big_fraction::make(x, y))); // Wrong-cast
    }
    return result && x;
}


bool divide::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Division of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    ASSERT (!y->is_zero() &&
            "fraction divide by zero, optimize<divide> failed");
    x = x / y;
    return x;
}


bool divide::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Divide complex numbers if we have them
// ----------------------------------------------------------------------------
{
    ASSERT(!y->is_zero() && "complex divide by zero, optimize<divide> failed");
    x = x / y;
    return x;
}


bool divide::range_ok(range_g &x, range_g &y)
// ----------------------------------------------------------------------------
//   Divide ranges if we have them
// ----------------------------------------------------------------------------
{
    return range_binary(x, y, operator/, operator/);
}


template <>
algebraic_p arithmetic::optimize<mod>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Optimizations for modulo
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (y->is_zero(false))
        return zero_divide(x, y);
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<mod>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with modulo non-numerical cases
// ----------------------------------------------------------------------------
{
    return optimize<mod>(x, y);
}


bool mod::integer_ok(object::id &xt, object::id &yt,
                     ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   The modulo of two integers is always an integer
// ----------------------------------------------------------------------------
{
    ASSERT(yv != 0 && "integer mod divide by zero, optimize<mod> failed");

    // If one of the two objects is a based number, always used integer mod
    if (!is_real(xt) || !is_real(yt))
    {
        xv = xv % yv;
        return true;
    }

    // Perform the modulo
    xv = xv % yv;
    if (xt == ID_neg_integer && xv)
        xv = yv - xv;

    // The resulting type is always positive
    xt = ID_integer;
    return true;
}


bool mod::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Modulo always works except divide by zero
// ----------------------------------------------------------------------------
{
    bignum_g r = x % y;
    if (byte_p(r) == nullptr)
        return false;
    if (x->type() == ID_neg_bignum && !r->is_zero())
        x = y->type() == ID_neg_bignum ? r - y : r + y;
    else
        x = r;
    return x;
}


bool mod::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Modulo of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    ASSERT(!y->is_zero() &&
           "fraction mod divide by zero, optimize<mod> failed");
    x = x % y;
    if (x->is_negative() && !x->is_zero())
        x = y->is_negative() ? x - y : x + y;
    return x;
}


bool mod::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No modulo on complex numbers
// ----------------------------------------------------------------------------
{
    return false;
}


bool mod::range_ok(range_g &, range_g &)
// ----------------------------------------------------------------------------
//   No modulo on ranges
// ----------------------------------------------------------------------------
{
    return false;
}


template <>
algebraic_p arithmetic::optimize<rem>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Optimizations for modulo
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (y->is_zero(false))
        return zero_divide(x, y);
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<rem>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with modulo non-numerical cases
// ----------------------------------------------------------------------------
{
    return optimize<rem>(x, y);
}


bool rem::integer_ok(object::id &/* xt */, object::id &/* yt */,
                            ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   The reminder of two integers is always an integer
// ----------------------------------------------------------------------------
{
    ASSERT(yv != 0 && "integer rem divide by zero, optimize<rem> failed");

    // The type of the result is always the type of x
    xv = xv % yv;
    return true;
}


bool rem::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Remainder always works except divide by zero
// ----------------------------------------------------------------------------
{
    x = x % y;
    return x;
}


bool rem::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Modulo of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    ASSERT(!y->is_zero() &&
           "fraction rem divide by zero, optimize<rem> failed");
    x = x % y;
    return x;
}


bool rem::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No remainder on complex numbers
// ----------------------------------------------------------------------------
{
    return false;
}


bool rem::range_ok(range_g &, range_g &)
// ----------------------------------------------------------------------------
//   No remainder on ranges
// ----------------------------------------------------------------------------
{
    return false;
}


template <>
algebraic_p arithmetic::optimize<struct pow>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//    Optimizations for X^Y
// ----------------------------------------------------------------------------
{
    // Check 0^0 (but check compatibility flag, since HPs return 1)
    // See https://www.hpcalc.org/hp48/docs/faq/48faq-5.html#ss5.2 as
    // to rationale on why HP calculators compute 0^0 as 1.
    if (x->is_zero(false) && y->is_zero(false))
    {
        if (Settings.ZeroPowerZeroIsUndefined())
            return rt.undefined_result();
        return integer::make(1);
    }

    int xinf = x->is_infinity();
    int yinf = y->is_infinity();
    if (xinf || yinf)
    {
        if (xinf > 0 && yinf > 0)
            return rt.infinity(false);
        if (xinf > 0 && y->is_real() && !y->is_zero())
            return y->is_negative()
                ? algebraic_p(integer::make(0))
                : +x;
        if (x->is_real() && !x->is_negative() && !x->is_zero() && yinf)
            return yinf < 0
                ? algebraic_p(integer::make(0))
                : rt.infinity(false);
        rt.undefined_operation_error();
        return nullptr;
    }

    // Deal with X^N where N is a positive  or negative integer
    id   yt   = y->type();
    bool negy = yt == ID_neg_integer;
    bool posy = yt == ID_integer || y->is_zero(false) || y->is_one(false);
    if (negy || posy)
    {
        // Defer computations for integer values to integer_ok
        if (x->is_integer() && !negy)
            return nullptr;

        // Auto-simplify x^0 = 1 and x^1 = x (we already tested 0^0)
        if (Settings.AutoSimplify())
        {
            if (y->is_zero(false))
                return integer::make(1);
            if (y->is_one())
                return x;
        }

        // Do not expand X^3 or integers when y>=0
        if (x->is_symbolic() || x->type() == ID_uncertain)
            return nullptr;

        // Deal with X^N where N is a positive integer
        ularge yv = integer_p(+y)->value<ularge>();
        algebraic_g r = ::pow(x, yv);
        if (negy)
            r = inv::run(r);
        return r;
    }

    // Not yet implemented
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<struct pow>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    // Deal with arrays
    if (array_g xa = x->as<array>())
    {
        // The integer case is dealt with through optimize, and must not
        // be treated as element-by-element, so that we can raise a matrix
        // to the third power.
        if (!y->is_integer())
            return xa->map(pow::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, pow::evaluate);
    }


    // Deal with the case of units
    if (unit_p xu = unit::get(x))
    {
        algebraic_g xv = xu->value();
        algebraic_g xe = xu->uexpr();
        xv = pow(xv, y);
        {
            save<bool> umode(unit::mode, false);
            save<bool> ufact(unit::factoring, true);
            xe = pow(xe, y);
        }
        return unit::simple(xv, xe);
    }


    return optimize<struct pow>(x, y);
}


bool pow::integer_ok(object::id &xt, object::id &yt,
                     ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Compute Y^X
// ----------------------------------------------------------------------------
{
    // Cannot raise to a negative power as integer
    if (yt == ID_neg_integer)
        return false;

    // Check the type of the result
    if (xt == ID_neg_integer)
        xt = (yv & 1) ? ID_neg_integer : ID_integer;

    // Compute result, check that it does not overflow
    ularge r = 1;
    enum { MAXBITS = 8 * sizeof(ularge) };
    while (yv)
    {
        if (yv & 1)
        {
            if (std::__countl_zero(xv) + std::__countl_zero(r) < MAXBITS)
                return false;   // Integer overflow
            ularge p = r * xv;
            r = p;
        }
        yv /= 2;

        if (std::__countl_zero(xv) * 2 < MAXBITS)
            return false;   // Integer overflow
        ularge nxv = xv * xv;
        xv = nxv;
    }

    xv = r;
    return true;
}


bool pow::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Compute y^x, works if x >= 0
// ----------------------------------------------------------------------------
{
    // Compute result, check that it does not overflow
    if (y->type() == ID_neg_bignum)
        return false;
    x = bignum::pow(x, y);
    return x;
}


bool pow::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Implement x^y as exp(y * log(x))
// ----------------------------------------------------------------------------
{
    x = complex::exp(y * complex::ln(x));
    return x;
}


bool pow::range_ok(range_g &x, range_g &y)
// ----------------------------------------------------------------------------
//   Implement x^y as exp(y * log(x))
// ----------------------------------------------------------------------------
{
    return range_binary(x, y, operator^, operator^);
}


bool pow::fraction_ok(fraction_g &/* x */, fraction_g &/* y */)
// ----------------------------------------------------------------------------
//   Compute y^x, works if x >= 0
// ----------------------------------------------------------------------------
{
    return false;
}


bool hypot::integer_ok(object::id &/* xt */, object::id &/* yt */,
                              ularge &/* xv */, ularge &/* yv */)
// ----------------------------------------------------------------------------
//   hypot() involves a square root, so not working on integers
// ----------------------------------------------------------------------------
//   Not trying to optimize the few cases where it works, e.g. 3^2+4^2=5^2
{
    return false;
}


bool hypot::bignum_ok(bignum_g &/* x */, bignum_g &/* y */)
// ----------------------------------------------------------------------------
//   Hypot never works with big integers
// ----------------------------------------------------------------------------
{
    return false;
}


bool hypot::fraction_ok(fraction_g &/* x */, fraction_g &/* y */)
// ----------------------------------------------------------------------------
//   Hypot never works with big integers
// ----------------------------------------------------------------------------
{
    return false;
}


bool hypot::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No hypot on complex yet, to be defined as sqrt(x^2+y^2)
// ----------------------------------------------------------------------------
{
    return false;
}


bool hypot::range_ok(range_g &, range_g &)
// ----------------------------------------------------------------------------
//   No hypot on ranges yet
// ----------------------------------------------------------------------------
{
    return false;
}



// ============================================================================
//
//   atan2: Optimize exact cases when dealing with fractions of pi
//
// ============================================================================

bool atan2::integer_ok(object::id &/* xt */, object::id &/* yt */,
                              ularge &/* xv */, ularge &/* yv */)
// ----------------------------------------------------------------------------
//   Optimized for integers on the real axis
// ----------------------------------------------------------------------------
{
    return false;
}


bool atan2::bignum_ok(bignum_g &/* x */, bignum_g &/* y */)
// ----------------------------------------------------------------------------
//   Optimize for bignums on the real axis
// ----------------------------------------------------------------------------
{
    return false;
}


bool atan2::fraction_ok(fraction_g &/* x */, fraction_g &/* y */)
// ----------------------------------------------------------------------------
//   Optimize for fractions on the real and complex axis and for diagonals
// ----------------------------------------------------------------------------
{
    return false;
}


bool atan2::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No atan2 on complex numbers yet
// ----------------------------------------------------------------------------
{
    return false;
}


bool atan2::range_ok(range_g &, range_g &)
// ----------------------------------------------------------------------------
//   No atan2 on ranges yet
// ----------------------------------------------------------------------------
{
    return false;
}


template <>
algebraic_p arithmetic::optimize<struct atan2>(algebraic_r y, algebraic_r x)
// ----------------------------------------------------------------------------
//   Deal with various exact angle optimizations for atan2
// ----------------------------------------------------------------------------
//   Note that the first argument to atan2 is traditionally called y,
//   and represents the imaginary axis for complex numbers
{
    if (Settings.SetAngleUnits() && x->is_real() && y->is_real())
    {
        settings::SaveSetAngleUnits save(false);
        algebraic_g r = atan2::evaluate(y, x);
        add_angle(r);
        return r;
    }

    id angle_mode = Settings.AngleMode();
    if (angle_mode != object::ID_Rad)
    {
        // Deal with special cases without rounding
        if (y->is_zero(false))
            return exact_angle(x->is_negative() ? 1 : 0, 1, angle_mode);

        if (x->is_zero(false))
            return exact_angle(y->is_negative() ? -1 : 1, 2, angle_mode);

        algebraic_g s = x + y;
        algebraic_g d = x - y;
        if (!s || !d)
            return nullptr;
        bool posdiag = d->is_zero(false);
        bool negdiag = s->is_zero(false);
        if (posdiag || negdiag)
        {
            bool xneg = x->is_negative();
            int  num  = posdiag ? (xneg ? -3 : 1) : (xneg ? 3 : -1);
            return exact_angle(num, 4, angle_mode);
        }
    }
    return nullptr;
}


template <>
algebraic_p arithmetic::non_numeric<struct atan2>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with various exact angle optimizations for atan2
// ----------------------------------------------------------------------------
//   Note that the first argument to atan2 is traditionally called y,
//   and represents the imaginary axis for complex numbers
{
    return optimize<struct atan2>(x, y);
}



// ============================================================================
//
//   Shared evaluation code
//
// ============================================================================

algebraic_p arithmetic::evaluate(id          op,
                                 algebraic_r xr,
                                 algebraic_r yr,
                                 ops_t       ops)
// ----------------------------------------------------------------------------
//   Shared code for all forms of evaluation, does not use the RPL stack
// ----------------------------------------------------------------------------
{
    if (!xr || !yr || rt.error())
        return nullptr;

    record(arithmetic, "Op %u x=%t y=%t", op, +xr, +yr);
    algebraic_g x   = xr;
    algebraic_g y   = yr;

    // Convert arguments to numeric if necessary
    if (Settings.NumericalResults())
    {
        (void) to_decimal(x, true);          // May fail silently
        (void) to_decimal(y, true);
        if (!x || !y)
            return nullptr;
    }

    id xt = x->type();
    id yt = y->type();

    // All non-numeric cases, e.g. string concatenation
    if (algebraic_p result = ops.non_numeric(x, y))
        return result;
    if (rt.error())
        return nullptr;

    // Integer types
    if (is_integer(xt) && is_integer(yt))
    {
        bool based = is_based(xt) || is_based(yt);
        if (based)
        {
            xt = algebraic::based_promotion(x);
            yt = algebraic::based_promotion(y);
        }

        if (!is_bignum(xt) && !is_bignum(yt))
        {
            // Perform conversion of integer values to the same base
            integer_p xi = integer_p(object_p(+x));
            integer_p yi = integer_p(object_p(+y));
            uint      ws = Settings.WordSize();
            if (xi->native() && yi->native() && (ws < 64 || !based))
            {
                ularge xv = xi->value<ularge>();
                ularge yv = yi->value<ularge>();
                if (ops.integer_ok(xt, yt, xv, yv))
                {
                    if (based)
                        xv &= (1UL << ws) - 1UL;
                    return rt.make<integer>(xt, xv);
                }
            }
        }

        algebraic_g xb = x;
        algebraic_g yb = y;
        if (!is_bignum(xt))
            xt = bignum_promotion(xb);
        if (!is_bignum(yt))
            yt = bignum_promotion(yb);

        // Proceed with big integers if native did not fit
        bignum_g xg = bignum_p(+xb);
        bignum_g yg = bignum_p(+yb);
        if (ops.bignum_ok(xg, yg))
        {
            x = +xg;
            if (Settings.NumericalResults())
                (void) to_decimal(x, true);
            return x;
        }
    }

    // Fraction types
    if ((x->is_fraction() || y->is_fraction() ||
         (op == ID_divide && x->is_fractionable() && y->is_fractionable())))
    {
        if (fraction_g xf = fraction_promotion(x))
        {
            if (fraction_g yf = fraction_promotion(y))
            {
                if (ops.fraction_ok(xf, yf))
                {
                    x = algebraic_p(fraction_p(xf));
                    if (x)
                    {
                        bignum_g d = xf->denominator();
                        if (d->is(1))
                            return algebraic_p(bignum_p(xf->numerator()));
                    }
                    if (Settings.NumericalResults())
                        (void) to_decimal(x, true);
                    return x;
                }
            }
        }
    }

    // Hardware-accelerated floating-point data types
    if (hwfp_promotion(x, y))
    {
        if (hwfloat_g fx = x->as<hwfloat>())
            if (hwfloat_g fy = y->as<hwfloat>())
                return ops.fop(fx, fy);
        if (hwdouble_g dx = x->as<hwdouble>())
            if (hwdouble_g dy = y->as<hwdouble>())
                return ops.dop(dx, dy);
    }


    // Real data types
    if (decimal_promotion(x, y))
    {
        // Here, x and y have the same type, a decimal type
        decimal_g xv = decimal_p(+x);
        decimal_g yv = decimal_p(+y);
        xv = ops.decop(xv, yv);
        if (xv && !xv->is_normal())
        {
            if (xv->is_infinity())
                return rt.numerical_overflow(xv->is_negative());
            rt.domain_error();
            return nullptr;
        }
        return xv;
    }

    // Complex data types
    if (complex_promotion(x, y))
    {
        complex_g xc = complex_p(algebraic_p(x));
        complex_g yc = complex_p(algebraic_p(y));
        if (ops.complex_ok(xc, yc))
        {
            if (Settings.AutoSimplify())
                if (algebraic_p re = xc->is_real())
                    return re;
            return xc;
        }
        return nullptr;
    }

    // Range data types and uncertain numbers
    if (range_promotion(x, y))
    {
        range_g xr = range_p(algebraic_p(x));
        range_g yr = range_p(algebraic_p(y));
        if (ops.range_ok(xr, yr))
        {
            if (Settings.AutoSimplify())
            {
                if (uncertain_p xu = xr->as<uncertain>())
                {
                    if (xu->stddev()->is_zero(false))
                        return xu->average();
                }
                else
                {
                    if (algebraic_p diff = xr->hi() - xr->lo())
                        if (diff->is_zero(false))
                            return xr->hi();
                }
            }
            return xr;
        }
        return nullptr;
    }

    if (!x || !y)
        return nullptr;

    if (x->is_symbolic_arg() && y->is_symbolic_arg())
    {
        polynomial_g xp  = x->as<polynomial>();
        polynomial_g yp  = y->as<polynomial>();
        polynomial_p xpp = xp;
        polynomial_p ypp = yp;
        if (xpp || ypp)
        {
            if (!xp)
                xp = polynomial::make(x);
            if (xp)
            {
                if (!yp && op == ID_pow)
                    if (integer_g yi = y->as<integer>())
                        return polynomial::pow(xp, yi);
                if (!yp)
                    yp = polynomial::make(y);
                if (yp)
                {
                    switch(op)
                    {
                    case ID_add:        return polynomial::add(xp, yp); break;
                    case ID_subtract:   return polynomial::sub(xp, yp); break;
                    case ID_multiply:   return polynomial::mul(xp, yp); break;
                    case ID_divide:     return polynomial::div(xp, yp); break;
                    case ID_mod:
                    case ID_rem:        return polynomial::mod(xp, yp); break;
                    default: break;
                    }
                }
                if (ypp)
                    y = yp->as_expression();
                if (xpp)
                    x = xp->as_expression();
            }
        }

        // Deal with the special cases of (A=B) + (C=D)
        if (expression_p xe = x->as<expression>())
        {
            expression_g xl, xr;
            if (xe->split_equation(xl, xr))
            {
                algebraic_g xla = +xl;
                algebraic_g xra = +xr;
                if (expression_p ye = y->as<expression>())
                {
                    expression_g yl, yr;
                    if (ye->split_equation(yl, yr))
                    {
                        algebraic_g yla = +yl;
                        algebraic_g yra = +yr;
                        xla = expression::make(op, xla, yla);
                        xra = expression::make(op, xra, yra);
                        goto join;
                    }
                }
                xla = expression::make(op, xla, y);
                xra = expression::make(op, xra, y);
            join:
                x = expression::make(ID_TestEQ, xla, xra);
                goto done;
            }
        }
        if (expression_p ye = y->as<expression>())
        {
            expression_g yl, yr;
            if (ye->split_equation(yl, yr))
            {
                algebraic_g yla = +yl;
                algebraic_g yra = +yr;
                yla = expression::make(op, x, yla);
                yra = expression::make(op, x, yra);
                x = expression::make(ID_TestEQ, yla, yra);
                goto done;
            }
        }

        x = expression::make(op, x, y);
    done:
        record(arithmetic, "Done x=%t", +x);
        return x;
    }

    // Default error is "Bad argument type", unless we got something else
    if (!rt.error())
        rt.type_error();
    return nullptr;
}


object::result arithmetic::evaluate(id op, ops_t ops)
// ----------------------------------------------------------------------------
//   Shared code for all forms of evaluation using the RPL stack
// ----------------------------------------------------------------------------
{
    // Fetch arguments from the stack
    // Possibly wrong type, i.e. it migth not be an algebraic on the stack,
    // but since we tend to do extensive type checking later, don't overdo it
    object_p yo = strip(rt.stack(1));
    object_p xo = strip(rt.stack(0));
    if (!xo || !yo)
        return ERROR;
    algebraic_g y = yo->as_extended_algebraic();
    algebraic_g x = xo->as_extended_algebraic();
    if (!x || !y)
    {
        rt.type_error();
        return ERROR;
    }

    // Evaluate the operation
    cleaner     purge;
    algebraic_g r = evaluate(op, y, x, ops);
    if (+r != +x && +r != +y)
        r = purge(r);

    // If result is valid, drop second argument and push result on stack
    if (r)
    {
        rt.drop();
        if (rt.top(r))
            return OK;
    }

    // Default error is "Bad argument type", unless we got something else
    if (!rt.error())
        rt.type_error();
    return ERROR;
}



// ============================================================================
//
//   Instantiations
//
// ============================================================================

template object::result arithmetic::evaluate<add>();
template object::result arithmetic::evaluate<subtract>();
template object::result arithmetic::evaluate<multiply>();
template object::result arithmetic::evaluate<divide>();
template object::result arithmetic::evaluate<struct mod>();
template object::result arithmetic::evaluate<struct rem>();
template object::result arithmetic::evaluate<struct pow>();
template object::result arithmetic::evaluate<struct hypot>();
template object::result arithmetic::evaluate<struct atan2>();

template algebraic_p arithmetic::evaluate<struct mod>(algebraic_r x, algebraic_r y);
template algebraic_p arithmetic::evaluate<struct rem>(algebraic_r x, algebraic_r y);
template algebraic_p arithmetic::evaluate<struct hypot>(algebraic_r x, algebraic_r y);
template algebraic_p arithmetic::evaluate<struct atan2>(algebraic_r x, algebraic_r y);


template <typename Op>
arithmetic::ops_t arithmetic::Ops()
// ----------------------------------------------------------------------------
//   Return the operations for the given Op
// ----------------------------------------------------------------------------
{
    static const ops result =
    {
        Op::decop,
        hwfloat_fn(Op::fop),
        hwdouble_fn(Op::dop),
        Op::integer_ok,
        Op::bignum_ok,
        Op::fraction_ok,
        Op::complex_ok,
        Op::range_ok,
        non_numeric<Op>
    };
    return result;
}


template <typename Op>
algebraic_p arithmetic::evaluate(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Evaluate the operation for C++ use (not using RPL stack)
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    if (Op::target)
    {
        if (arithmetic_fn code = Op::target(x, y))
        {
            if (algebraic_p result = optimize<Op>(x, y))
                return result;
            else
                return code(x, y);
        }
    }
    return evaluate(Op::static_id, x, y, Ops<Op>());
}


template <typename Op>
object::result arithmetic::evaluate()
// ----------------------------------------------------------------------------
//   The stack-based evaluator for arithmetic operations
// ----------------------------------------------------------------------------
{
    return evaluate(Op::static_id, Ops<Op>());
}


algebraic_p arithmetic::zero_divide(algebraic_r num, algebraic_r den)
// ----------------------------------------------------------------------------
//   Deal with zero divide according to current configuration
// ----------------------------------------------------------------------------
{
    ASSERT(den && den->is_zero(false));
    if (num && num->is_zero(false))
    {
        if (Settings.ZeroOverZeroIsUndefined())
            return rt.undefined_result();
        bool negative = num->is_negative(false) != den->is_negative(false);
        return rt.zero_divide(negative);
    }
    bool negative = (num && num->is_negative(false)) != den->is_negative(false);
    return rt.zero_divide(negative);
}



// ============================================================================
//
//   C++ wrappers
//
// ============================================================================

algebraic_g operator-(algebraic_r x)
// ----------------------------------------------------------------------------
//   Negation
// ----------------------------------------------------------------------------
{
    return neg::evaluate(x);
}


algebraic_g operator+(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Addition
// ----------------------------------------------------------------------------
{
    return add::evaluate(x, y);
}


algebraic_g operator-(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Subtraction
// ----------------------------------------------------------------------------
{
    return subtract::evaluate(x, y);
}


algebraic_g operator*(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Multiplication
// ----------------------------------------------------------------------------
{
    return multiply::evaluate(x, y);
}


algebraic_g operator/(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Division
// ----------------------------------------------------------------------------
{
    return divide::evaluate(x, y);
}


algebraic_g operator%(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Modulo
// ----------------------------------------------------------------------------
{
    return mod::evaluate(x, y);
}


algebraic_g pow(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Power
// ----------------------------------------------------------------------------
{
    return pow::evaluate(x, y);
}


algebraic_g pow(algebraic_r xr, ularge y)
// ----------------------------------------------------------------------------
//   Power with a known integer value
// ----------------------------------------------------------------------------
{
    algebraic_g r = integer::make(1);
    algebraic_g x = xr;
    if (x->is_decimal() &&
        !decimal::precision_adjust::adjusted &&
        !(Settings.HardwareFloatingPoint() && Settings.Precision() <= 16))
    {
        decimal::precision_adjust prec(3);
        r = pow(xr, y);
        if (r && r->is_decimal())
            r = prec(decimal_p(+r));
        return r;
    }
    while (y)
    {
        if (y & 1)
            r = r * x;
        y /= 2;
        x = x * x;
    }
    return r;
}


INSERT_BODY(arithmetic)
// ----------------------------------------------------------------------------
//   Arithmetic objects do not insert parentheses
// ----------------------------------------------------------------------------
{
    if (o->type() == ID_multiply && Settings.UseDotForMultiplication())
    {
        auto mode = ui.editing_mode();
        if (mode == ui.ALGEBRAIC || mode == ui.PARENTHESES)
            return ui.insert(utf8("·"), ui.INFIX);
    }
    return ui.insert(o->fancy(), ui.INFIX);
}


// ============================================================================
//
//   Div2 operation (returning quotient and remainder)
//
// ============================================================================

COMMAND_BODY(Div2)
// ----------------------------------------------------------------------------
//   Process the Div2 command
// ----------------------------------------------------------------------------
{
    object_p xo = strip(rt.stack(0));
    object_p yo = strip(rt.stack(1));
    if (!xo || !yo)
        return ERROR;

    while (tag_p xt = xo->as<tag>())
        xo = xt->tagged_object();
    while (tag_p yt = yo->as<tag>())
        yo = yt->tagged_object();

    id xty = xo->type();
    id yty = yo->type();

    if (is_integer(xty) && is_integer(yty))
    {
        if (is_bignum(xty)  || is_bignum(yty))
        {
            bignum_g xi = bignum::promote(xo);
            bignum_g yi = bignum::promote(yo);
            if (!xi || !yi)
                return ERROR;

            if (xi->is_zero())
            {
                rt.zero_divide_error();
                return ERROR;
            }
            bignum_g q, r;
            id rty = bignum::product_type(xi->type(), yi->type());
            if (!bignum::quorem(yi, xi, rty, &q, &r))
                return ERROR;
            tag_g  qtag = tag::make("Q", +q);
            tag_g  rtag = tag::make("R", +r);
            if (qtag && rtag && rt.stack(0, rtag) && rt.stack(1, qtag))
                return OK;
            return ERROR;
        }
        else
        {
            integer_p xi = integer_p(xo);
            integer_p yi = integer_p(yo);
            ularge    xv = xi->value<ularge>();
            ularge    yv = yi->value<ularge>();
            if (!xv)
            {
                rt.zero_divide_error();
                return ERROR;
            }
            id rty = is_based(xty)
                ? xty
                : is_based(yty)
                ? yty
                : (xty == ID_neg_integer) != (yty == ID_neg_integer)
                ? ID_neg_integer
                : ID_integer;
            ularge qv   = yv / xv;
            ularge rv   = yv % xv;
            tag_g  qtag = tag::make("Q", rt.make<integer>(rty, qv));
            tag_g  rtag = tag::make("R", rt.make<integer>(rty, rv));
            if (qtag && rtag && rt.stack(0, rtag) && rt.stack(1, qtag))
                return OK;
            return ERROR;
        }
    }

    if (is_fraction(xty) || is_fraction(yty))
    {
        algebraic_g xa = algebraic_p(xo);
        algebraic_g ya = algebraic_p(yo);
        fraction_g xf = arithmetic::fraction_promotion(xa);
        fraction_g yf = arithmetic::fraction_promotion(ya);
        if (xf && yf)
        {
            bignum_g   xn = xf->numerator();
            bignum_g   xd = xf->denominator();
            bignum_g   yn = yf->numerator();
            bignum_g   yd = yf->denominator();
            bignum_g   qn = yn * xd;
            bignum_g   qd = yd * xn;
            if (fraction_g q  = big_fraction::make(qn, qd))
            {
                qn = q->numerator();
                qd = q->denominator();
                if (bignum_g ir = qn / qd)
                {
                    q = big_fraction::make(ir, bignum::make(1));
                    fraction_g r = q * xf;
                    r =  yf - r;
                    tag_g  qtag = tag::make("Q", +ir);
                    tag_g  rtag = tag::make("R", +r);
                    if (qtag && rtag && rt.stack(0, rtag) && rt.stack(1, qtag))
                        return OK;
                }
            }
            return ERROR;
        }
    }

    if (is_real(xty) && is_real(yty))
    {
        algebraic_g xa = algebraic_p(xo);
        algebraic_g ya = algebraic_p(yo);
        if (!arithmetic::decimal_promotion(xa, ya))
            return ERROR;
        decimal_g xd = decimal_p(+xa);
        decimal_g yd = decimal_p(+ya);
        decimal_g q = yd / xd;
        if (!q)
            return ERROR;
        q = q->truncate();
        decimal_g r = yd - q * xd;
        tag_g  qtag = tag::make("Q", +q);
        tag_g  rtag = tag::make("R", +r);
        if (qtag && rtag && rt.stack(0, rtag) && rt.stack(1, qtag))
            return OK;
        return ERROR;
    }

    algebraic_g xa = xo->as_algebraic();
    algebraic_g ya = yo->as_algebraic();
    if (!xa || !ya)
    {
        if (!rt.error())
            rt.type_error();
        return ERROR;
    }

    polynomial_g xp = polynomial::make(xa);
    polynomial_g yp = polynomial::make(ya);
    if (!xp || !yp)
    {
        if (!rt.error())
            rt.type_error();
        return ERROR;
    }

    polynomial_g q, r;
    if (!polynomial::quorem(yp, xp, q, r))
    {
        if (!rt.error())
            rt.invalid_polynomial_error();
        return ERROR;
    }

    tag_g  qtag = tag::make("Q", +q);
    tag_g  rtag = tag::make("R", +r);
    if (qtag && rtag && rt.stack(0, rtag) && rt.stack(1, qtag))
        return OK;
    return ERROR;
}


#define ARITHMETIC_DEFINE(derived)      arithmetic::target_fn derived::target;

ARITHMETIC_DEFINE(add);
ARITHMETIC_DEFINE(subtract);
ARITHMETIC_DEFINE(multiply);
ARITHMETIC_DEFINE(divide);
ARITHMETIC_DEFINE(mod);
ARITHMETIC_DEFINE(rem);
ARITHMETIC_DEFINE(pow);
ARITHMETIC_DEFINE(hypot);
ARITHMETIC_DEFINE(atan2);
