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
#include "decimal-32.h"
#include "decimal-64.h"
#include "decimal128.h"
#include "equation.h"
#include "fraction.h"
#include "functions.h"
#include "integer.h"
#include "list.h"
#include "runtime.h"
#include "settings.h"
#include "text.h"

#include <bit>
#include <bitset>


RECORDER(arithmetic,            16, "Arithmetic");
RECORDER(arithmetic_error,      16, "Errors from arithmetic code");

bool arithmetic::real_promotion(algebraic_g &x, algebraic_g &y)
// ----------------------------------------------------------------------------
//   Promote x or y to the largest of both types
// ----------------------------------------------------------------------------
{
    if (!x.Safe() || !y.Safe())
        return false;

    id xt = x->type();
    id yt = y->type();
    if (is_integer(xt) && is_integer(yt))
        // If we got here, we failed an integer op, e.g. 2/3, promote to real
        return real_promotion(x) && real_promotion(y);

    if (!is_real(xt) || !is_real(yt))
        return false;

    uint16_t prec  = Settings.precision;
    id       minty = prec > BID64_MAXDIGITS ? ID_decimal128
                   : prec > BID32_MAXDIGITS ? ID_decimal64
                                            : ID_decimal32;
    if (is_decimal(xt) && xt > minty)
        minty = xt;
    if (is_decimal(yt) && yt > minty)
        minty = yt;

    return (xt == minty || real_promotion(x, minty))
        && (yt == minty || real_promotion(y, minty));
}


bool arithmetic::complex_promotion(algebraic_g &x, algebraic_g &y)
// ----------------------------------------------------------------------------
//   Return true if one type is complex and the other can be promoted
// ----------------------------------------------------------------------------
{
    if (!x.Safe() || !y.Safe())
        return false;

    id xt = x->type();
    id yt = y->type();

    if (is_complex(xt))
    {
        // Success if the two complex types are identical
        if (yt == xt)
            return true;

        // Try to convert both types to the same complex type
        return complex_promotion(y, xt);
    }

    if (is_complex(yt))
    {
        // Try to convert both types to the same complex type
        return complex_promotion(x, yt);
    }

    // Neither type is complex, no point to promote
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
algebraic_p arithmetic::non_numeric<add>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for addition
// ----------------------------------------------------------------------------
//   This deals with:
//   - Text + text: Concatenation of text
//   - Text + object: Concatenation of text + object text
//   - Object + text: Concatenation of object text + text
{
    // Deal with basic auto-simplifications rules
    if (Settings.auto_simplify && x->is_algebraic() && y->is_algebraic())
    {
        if (x->is_zero(false))                  // 0 + X = X
            return y;
        if (y->is_zero(false))                  // X + 0 = X
            return x;
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

    // list + ...
    else if (list_g xl = x->as<list>())
    {
        if (list_g yl = y->as<list>())
            return xl + yl;
        if (list_g yl = rt.make<list>(byte_p(y.Safe()), y->size()))
            return xl + yl;
    }
    else if (list_g yl = y->as<list>())
    {
        if (list_g xl = rt.make<list>(byte_p(x.Safe()), x->size()))
            return xl + yl;
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

    // Not yet implemented
    return nullptr;
}


inline bool add::integer_ok(object::id &xt, object::id &yt,
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


inline bool add::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always add two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x + y;
    return true;
}


inline bool add::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always add two fractions
// ----------------------------------------------------------------------------
{
    x = x + y;
    return true;
}


inline bool add::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Add complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x + y;
    return true;
}


template <>
algebraic_p arithmetic::non_numeric<sub>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
//   This deals with vector and matrix operations
{
    // Deal with basic auto-simplifications rules
    if (Settings.auto_simplify && x->is_algebraic() && y->is_algebraic())
    {
        if (y->is_zero(false))                  // X - 0 = X
            return x;
        if (x->is_same_as(y))                   // X - X = 0
            return integer::make(0);
        if (x->is_zero(false) && y->is_strictly_symbolic())
            return neg::run(y);                 // 0 - X = -X
    }

    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa - ya;
        return xa->map(sub::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, sub::evaluate);
    }

    // Not yet implemented
    return nullptr;
}


inline bool sub::integer_ok(object::id &xt, object::id &yt,
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


inline bool sub::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always subtract two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x - y;
    return true;
}


inline bool sub::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always subtract two fractions (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x - y;
    return true;
}


inline bool sub::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Subtract complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x - y;
    return true;
}


template <>
algebraic_p arithmetic::non_numeric<mul>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
//   This deals with:
//   - Text * integer: Repeat the text
//   - Integer * text: Repeat the text
{
    // Deal with basic auto-simplifications rules
    if (Settings.auto_simplify && x->is_algebraic() && y->is_algebraic())
    {
        if (x->is_zero(false))                  // 0 * X = 0
            return x;
        if (y->is_zero(false))                  // X * 0 = Y
            return y;
        if (x->is_one(false))                   // 1 * X = X
            return y;
        if (y->is_one(false))                   // X * 1 = X
            return x;
        if (x->is_strictly_symbolic() && x->is_same_as(y))
            return sq::run(x);                  // X * X = X²
    }

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
        return xa->map(mul::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, mul::evaluate);
    }

    // Not yet implemented
    return nullptr;
}


inline bool mul::integer_ok(object::id &xt, object::id &yt,
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


inline bool mul::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   We can always multiply two big integers (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x * y;
    return true;
}


inline bool mul::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   We can always multiply two fractions (memory permitting)
// ----------------------------------------------------------------------------
{
    x = x * y;
    return true;
}


inline bool mul::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Multiply complex numbers if we have them
// ----------------------------------------------------------------------------
{
    x = x * y;
    return true;
}


template <>
algebraic_p arithmetic::non_numeric<struct div>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for division
// ----------------------------------------------------------------------------
//   This deals with vector and matrix operations
{
    // Deal with basic auto-simplifications rules
    if (Settings.auto_simplify && x->is_algebraic() && y->is_algebraic())
    {
        if (x->is_zero(false))                  // 0 / X = 0
        {
            if (y->is_zero(false))
            {
                rt.zero_divide_error();
                return nullptr;
            }
            return x;
        }
        if (y->is_one(false))                   // X / 1 = X
            return x;
        if (x->is_one(false) && y->is_strictly_symbolic())
            return inv::run(y);                 // 1 / X = X⁻¹
        if (x->is_same_as(y))
            return integer::make(1);            // X / X = 1
    }

    // vector + vector or matrix + matrix
    if (array_g xa = x->as<array>())
    {
        if (array_g ya = y->as<array>())
            return xa / ya;
        return xa->map(div::evaluate, y);
    }
    else if (array_g ya = y->as<array>())
    {
        return ya->map(x, div::evaluate);
    }

    // Not yet implemented
    return nullptr;
}


inline bool div::integer_ok(object::id &xt, object::id &yt,
                            ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Check if dividing two integers works or if we need to promote to real
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (yv == 0)
    {
        rt.zero_divide_error();
        return false;
    }

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


inline bool div::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Division works if there is no remainder
// ----------------------------------------------------------------------------
{
    if (!y)
    {
        rt.zero_divide_error();
        return false;
    }
    bignum_g q = nullptr;
    bignum_g r = nullptr;
    id type = bignum::product_type(x->type(), y->type());
    bool result = bignum::quorem(x, y, type, &q, &r);
    if (result)
        result = bignum_p(r) != nullptr;
    if (result)
    {
        if (r->is_zero())
            x = q;                  // Integer result
        else
            x = bignum_p(fraction_p(big_fraction::make(x, y))); // Wrong-cast
    }
    return result;
}


inline bool div::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Division of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    if (!y->numerator())
    {
        rt.zero_divide_error();
        return false;
    }
    x = x / y;
    return true;
}


inline bool div::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Divide complex numbers if we have them
// ----------------------------------------------------------------------------
{
    if (y->is_zero())
    {
        rt.zero_divide_error();
        return false;
    }
    x = x / y;
    return true;
}


inline bool mod::integer_ok(object::id &xt, object::id &yt,
                            ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   The modulo of two integers is always an integer
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (yv == 0)
    {
        rt.zero_divide_error();
        return false;
    }

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


inline bool mod::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Modulo always works except divide by zero
// ----------------------------------------------------------------------------
{
    bignum_g r = x % y;
    if (byte_p(r) == nullptr)
        return false;
    if (y->type() == ID_neg_bignum && !r->is_zero())
        x = y - r;
    else
        x = r;
    return true;
}


inline bool mod::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Modulo of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    if (!y->numerator())
    {
        rt.zero_divide_error();
        return false;
    }
    x = x % y;
    if (y->type() == ID_neg_fraction && !x->is_zero())
        x = y - x;
    return true;
}


inline bool mod::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No modulo on complex numbers
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool rem::integer_ok(object::id &UNUSED xt, object::id &UNUSED yt,
                            ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   The reminder of two integers is always an integer
// ----------------------------------------------------------------------------
{
    // Check divide by zero
    if (yv == 0)
    {
        rt.zero_divide_error();
        return false;
    }

    // The type of the result is always the type of x
    xv = xv % yv;
    return true;
}


inline bool rem::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Remainder always works except divide by zero
// ----------------------------------------------------------------------------
{
    x = x % y;
    return true;
}


inline bool rem::fraction_ok(fraction_g &x, fraction_g &y)
// ----------------------------------------------------------------------------
//   Modulo of fractions, except division by zero
// ----------------------------------------------------------------------------
{
    if (!y->numerator())
    {
        rt.zero_divide_error();
        return false;
    }
    x = x % y;
    return true;
}


inline bool rem::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No remainder on complex numbers
// ----------------------------------------------------------------------------
{
    return false;
}


template <>
algebraic_p arithmetic::non_numeric<struct pow>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Deal with non-numerical data types for multiplication
// ----------------------------------------------------------------------------
{
    if (!x.Safe() || !y.Safe())
        return nullptr;

    // Deal with X^N where N is a positive  or negative integer
    id   yt   = y->type();
    bool negy = yt == ID_neg_integer;
    bool posy = yt == ID_integer;
    if (negy || posy)
    {
        // Do not expand X^3 or integers when y>=0
        if (x->is_strictly_symbolic() || (x->is_integer() && !negy))
            return nullptr;

        // Deal with X^N where N is a positive integer
        ularge yv = integer_p(y.Safe())->value<ularge>();
        if (yv == 0 && x->is_zero(false))
        {
            rt.undefined_operation_error();
            return nullptr;
        }

        algebraic_g r = integer::make(1);
        algebraic_g xx = x;
        while (yv)
        {
            if (yv & 1)
                r = r * xx;
            yv /= 2;
            xx = xx * xx;
        }
        if (negy)
            r = inv::run(r);
        return r;
    }

    // Not yet implemented
    return nullptr;
}


inline bool pow::integer_ok(object::id &xt, object::id &yt,
                            ularge &xv, ularge &yv)
// ----------------------------------------------------------------------------
//   Compute Y^X
// ----------------------------------------------------------------------------
{
    // Check 0^0
    if (xv == 0 && yv == 0)
    {
        rt.undefined_operation_error();
        return false;
    }

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


inline bool pow::bignum_ok(bignum_g &x, bignum_g &y)
// ----------------------------------------------------------------------------
//   Compute y^x, works if x >= 0
// ----------------------------------------------------------------------------
{
    // Compute result, check that it does not overflow
    if (y->type() == ID_neg_bignum)
        return false;
    x = bignum::pow(x, y);
    return true;
}


inline bool pow::complex_ok(complex_g &x, complex_g &y)
// ----------------------------------------------------------------------------
//   Implement x^y as exp(y * log(x))
// ----------------------------------------------------------------------------
{
    x = complex::exp(y * complex::log(x));
    return true;
}


inline bool pow::fraction_ok(fraction_g &UNUSED x, fraction_g &UNUSED y)
// ----------------------------------------------------------------------------
//   Compute y^x, works if x >= 0
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool hypot::integer_ok(object::id &UNUSED xt, object::id &UNUSED yt,
                              ularge &UNUSED xv, ularge &UNUSED yv)
// ----------------------------------------------------------------------------
//   hypot() involves a square root, so not working on integers
// ----------------------------------------------------------------------------
//   Not trying to optimize the few cases where it works, e.g. 3^2+4^2=5^2
{
    return false;
}


inline bool hypot::bignum_ok(bignum_g &UNUSED x, bignum_g &UNUSED y)
// ----------------------------------------------------------------------------
//   Hypot never works with big integers
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool hypot::fraction_ok(fraction_g &UNUSED x, fraction_g &UNUSED y)
// ----------------------------------------------------------------------------
//   Hypot never works with big integers
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool hypot::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No hypot on complex yet, to be defined as sqrt(x^2+y^2)
// ----------------------------------------------------------------------------
{
    return false;
}



// ============================================================================
//
//   atan2: Optimize exact cases when dealing with fractions of pi
//
// ============================================================================

inline bool atan2::integer_ok(object::id &UNUSED xt, object::id &UNUSED yt,
                              ularge &UNUSED xv, ularge &UNUSED yv)
// ----------------------------------------------------------------------------
//   Optimized for integers on the real axis
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool atan2::bignum_ok(bignum_g &UNUSED x, bignum_g &UNUSED y)
// ----------------------------------------------------------------------------
//   Optimize for bignums on the real axis
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool atan2::fraction_ok(fraction_g &UNUSED x, fraction_g &UNUSED y)
// ----------------------------------------------------------------------------
//   Optimize for fractions on the real and complex axis and for diagonals
// ----------------------------------------------------------------------------
{
    return false;
}


inline bool atan2::complex_ok(complex_g &, complex_g &)
// ----------------------------------------------------------------------------
//   No atan2 on complex numbers yet
// ----------------------------------------------------------------------------
{
    return false;
}


template <>
algebraic_p arithmetic::non_numeric<struct atan2>(algebraic_r y, algebraic_r x)
// ----------------------------------------------------------------------------
//   Deal with various exact angle optimizations for atan2
// ----------------------------------------------------------------------------
//   Note that the first argument to atan2 is traditionally called y,
//   and represents the imaginary axis for complex numbers
{
    if (Settings.angle_mode == settings::PI_RADIANS)
    {
        // Deal with special cases without rounding
        if (y->is_zero(false))
        {
            if (x->is_negative(false))
                return integer::make(1);
            return integer::make(0);
        }
        if (x->is_zero(false))
        {
            return fraction::make(integer::make(y->is_negative() ? -1 : 1),
                                  integer::make(2));
        }
        algebraic_g s = x + y;
        algebraic_g d = x - y;
        if (!s.Safe() || !d.Safe())
            return nullptr;
        bool posdiag = s->is_zero(false);
        bool negdiag = d->is_zero(false);
        if (posdiag || negdiag)
        {
            bool xneg = x->is_negative();
            return fraction::make(integer::make(posdiag
                                                ? (xneg ? -3 :  1)
                                                : (xneg ?  3 : -1)),
                                  integer::make(4));
        }
    }
    return nullptr;
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
    if (!xr.Safe() || !yr.Safe())
        return nullptr;

    id xt = xr->type();
    id yt = yr->type();

    // All non-numeric cases, e.g. string concatenation
    // Must come first, e.g. for optimization of X^3
    if (algebraic_p result = ops.non_numeric(xr, yr))
        return result;

    // Integer types
    if (is_integer(xt) && is_integer(yt))
    {
        if (!is_bignum(xt) && !is_bignum(yt))
        {
            // Perform conversion of integer values to the same base
            integer_p xi = integer_p(object_p(xr.Safe()));
            integer_p yi = integer_p(object_p(yr.Safe()));
            if (xi->native() && yi->native())
            {
                ularge xv = xi->value<ularge>();
                ularge yv = yi->value<ularge>();
                if (ops.integer_ok(xt, yt, xv, yv))
                    return rt.make<integer>(xt, xv);
            }
        }

        algebraic_g x = xr;
        algebraic_g y = yr;
        if (!is_bignum(xt))
            xt = bignum_promotion(x);
        if (!is_bignum(yt))
            yt = bignum_promotion(y);

        // Proceed with big integers if native did not fit
        bignum_g xg = bignum_p(x.Safe());
        bignum_g yg = bignum_p(y.Safe());
        if (ops.bignum_ok(xg, yg))
            return bignum_p(xg);
    }

    // Fraction types
    if ((xr->is_fraction() || yr->is_fraction() ||
         (op == ID_div && xr->is_fractionable() && yr->is_fractionable())))
    {
        algebraic_g x = xr;
        algebraic_g y = yr;
        if (fraction_g xf = fraction_promotion(x))
        {
            if (fraction_g yf = fraction_promotion(y))
            {
                if (ops.fraction_ok(xf, yf))
                {
                    x = algebraic_p(fraction_p(xf));
                    if (x.Safe())
                    {
                        bignum_g d = xf->denominator();
                        if (d->is(1))
                            return algebraic_p(bignum_p(xf->numerator()));
                    }
                    return x;
                }
            }
        }
    }

    // Real data types
    algebraic_g x = xr;
    algebraic_g y = yr;
    if (real_promotion(x, y))
    {
        // Here, x and y have the same type, a decimal type
        xt = x->type();
        switch(xt)
        {
        case ID_decimal32:
        {
            bid32 xv = x->as<decimal32>()->value();
            bid32 yv = y->as<decimal32>()->value();
            bid32 res;
            ops.op32(&res.value, &xv.value, &yv.value);
            x = rt.make<decimal32>(ID_decimal32, res);
        }
        case ID_decimal64:
        {
            bid64 xv = x->as<decimal64>()->value();
            bid64 yv = y->as<decimal64>()->value();
            bid64 res;
            ops.op64(&res.value, &xv.value, &yv.value);
            x = rt.make<decimal64>(ID_decimal64, res);
        }
        case ID_decimal128:
        {
            bid128 xv = x->as<decimal128>()->value();
            bid128 yv = y->as<decimal128>()->value();
            bid128 res;
            ops.op128(&res.value, &xv.value, &yv.value);
            x = rt.make<decimal128>(ID_decimal128, res);
        }
        default:
            break;
        }
        if (op == ID_atan2)
            function::adjust_to_angle(x);
        return x;
    }

    // Complex data types
    if (complex_promotion(x, y))
    {
        complex_g xc = complex_p(algebraic_p(x));
        complex_g yc = complex_p(algebraic_p(y));
        if (ops.complex_ok(xc, yc))
            return xc;
    }

    if (!x.Safe() || !y.Safe())
        return nullptr;

    if (x->is_symbolic() && y->is_symbolic())
    {
        x = equation::make(op, x, y);
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
    algebraic_g y = (algebraic_p) rt.stack(1);
    if (!y)
        return ERROR;
    algebraic_g x = (algebraic_p) rt.stack(0);
    if (!x)
        return ERROR;

    // Evaluate the operation
    algebraic_g r = evaluate(op, y, x, ops);

    // If result is valid, drop second argument and push result on stack
    if (r)
    {
        rt.drop();
        if (rt.top(r))
            return OK;
    }

    return ERROR;
}


// ============================================================================
//
//   128-bit stubs
//
// ============================================================================
//   The non-trivial functions like sqrt or exp are not present in the QSPI
//   on the DM42. Calling them causes a discrepancy with the QSPI content,
//   and increases the size of the in-flash image above what is allowed
//   So we need to stub out some bid64 and bid42 functions and compute them
//   using bid128

void bid64_pow(BID_UINT64 *pres, BID_UINT64 *px, BID_UINT64 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid64_to_bid128(&x128, px);
    bid64_to_bid128(&y128, py);
    bid128_pow(&res128, &x128, &y128);
    bid128_to_bid64(pres, &res128);
}


void bid32_pow(BID_UINT32 *pres, BID_UINT32 *px, BID_UINT32 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid32_to_bid128(&x128, px);
    bid32_to_bid128(&y128, py);
    bid128_pow(&res128, &x128, &y128);
    bid128_to_bid32(pres, &res128);
}


void bid64_hypot(BID_UINT64 *pres, BID_UINT64 *px, BID_UINT64 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid64_to_bid128(&x128, px);
    bid64_to_bid128(&y128, py);
    bid128_hypot(&res128, &x128, &y128);
    bid128_to_bid64(pres, &res128);
}


void bid32_hypot(BID_UINT32 *pres, BID_UINT32 *px, BID_UINT32 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid32_to_bid128(&x128, px);
    bid32_to_bid128(&y128, py);
    bid128_hypot(&res128, &x128, &y128);
    bid128_to_bid32(pres, &res128);
}


void bid64_atan2(BID_UINT64 *pres, BID_UINT64 *px, BID_UINT64 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid64_to_bid128(&x128, px);
    bid64_to_bid128(&y128, py);
    bid128_atan2(&res128, &x128, &y128);
    bid128_to_bid64(pres, &res128);
}


void bid32_atan2(BID_UINT32 *pres, BID_UINT32 *px, BID_UINT32 *py)
// ----------------------------------------------------------------------------
//   Perform the computation with bid128 code
// ----------------------------------------------------------------------------
{
    BID_UINT128 x128, y128, res128;
    bid32_to_bid128(&x128, px);
    bid32_to_bid128(&y128, py);
    bid128_atan2(&res128, &x128, &y128);
    bid128_to_bid32(pres, &res128);
}



// ============================================================================
//
//   Instantiations
//
// ============================================================================

template object::result arithmetic::evaluate<struct add>();
template object::result arithmetic::evaluate<struct sub>();
template object::result arithmetic::evaluate<struct mul>();
template object::result arithmetic::evaluate<struct div>();
template object::result arithmetic::evaluate<struct mod>();
template object::result arithmetic::evaluate<struct rem>();
template object::result arithmetic::evaluate<struct pow>();
template object::result arithmetic::evaluate<struct hypot>();
template object::result arithmetic::evaluate<struct atan2>();
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
        Op::bid128_op,
        Op::bid64_op,
        Op::bid32_op,
        Op::integer_ok,
        Op::bignum_ok,
        Op::fraction_ok,
        Op::complex_ok,
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
    return sub::evaluate(x, y);
}


algebraic_g operator*(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Multiplication
// ----------------------------------------------------------------------------
{
    return mul::evaluate(x, y);
}


algebraic_g operator/(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Division
// ----------------------------------------------------------------------------
{
    return div::evaluate(x, y);
}


algebraic_g operator%(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Modulo
// ----------------------------------------------------------------------------
{
    return mod::evaluate(x, y);
}


INSERT_BODY(arithmetic)
// ----------------------------------------------------------------------------
//   Arithmetic objects do not insert parentheses
// ----------------------------------------------------------------------------
{
    return ui.edit(o->fancy(), ui.INFIX);
}
