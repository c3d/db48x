// ****************************************************************************
//  compare.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of comparisons
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

#include "compare.h"

#include "decimal.h"
#include "expression.h"
#include "hwfp.h"
#include "integer.h"
#include "locals.h"


RECORDER(compare,       16, "Comparison operations");
RECORDER(compare_error, 16, "Errors during comparison operations");

template <typename Cmp>
object::result comparison::evaluate()
// ----------------------------------------------------------------------------
//   The actual evaluation for all binary operators
// ----------------------------------------------------------------------------
{
    return compare(Cmp::make_result, Cmp::static_id);
}


template <typename Cmp>
algebraic_p comparison::evaluate(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   The actual evaluation for all binary operators
// ----------------------------------------------------------------------------
{
    return compare(Cmp::make_result, Cmp::static_id, x, y);
}


bool comparison::compare(int *cmp, algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Compare objects left and right, return -1, 0 or +1
// ----------------------------------------------------------------------------
{
    // Check if we had some error earlier, if so propagate
    if (!x || !y)
        return false;

    id xt = x->type();
    id yt = y->type();

    // Check infinities
    if (xt == ID_constant || yt == ID_constant)
    {
        int xinf = x->is_infinity();
        int yinf = y->is_infinity();
        if (xinf || yinf)
        {
            if (is_real(xt) || is_real(yt) || xinf != yinf)
            {
                *cmp = xinf - yinf;
                return true;
            }
            // Compare two identical infinities: can't compare
            return false;
        }
    }

    /* Integer types */
    if (is_integer(xt) && is_integer(yt))
    {
        // Check if this is a bignum comparison
        if (is_bignum(xt) || is_bignum(yt))
        {
            algebraic_g xa = algebraic_p(+x);
            algebraic_g ya = algebraic_p(+y);
            if (!is_bignum(xt))
                xt = bignum_promotion(xa);
            if (!is_bignum(yt))
                yt = bignum_promotion(ya);
            bignum_g xb = bignum_p(+xa);
            bignum_g yb = bignum_p(+ya);
            int cmpval = bignum::compare(xb, yb);
            *cmp = cmpval < 0 ? -1 : cmpval > 0 ? 1 : 0;
            return true;
        }

        // Check if we have a neg_integer vs another integer type
        if ((xt == ID_neg_integer) != (yt == ID_neg_integer))
        {
            *cmp = xt == ID_neg_integer ? -1 : 1;
            return true;
        }

        integer_p xi     = integer_p(object_p(x));
        integer_p yi     = integer_p(object_p(y));
        ularge    xv     = xi->value<ularge>();
        ularge    yv     = yi->value<ularge>();
        int       cmpval = xv < yv ? -1 : xv > yv ? 1 : 0;
        if (xt == ID_neg_integer)
            cmpval = -cmpval;
        *cmp = cmpval;
        return true;
    }

    /* Real data types */
    algebraic_g xa = algebraic_p(+x);
    algebraic_g ya = algebraic_p(+y);
    if (hwfp_promotion(xa, ya))
    {
        // Here we have two identical hardware floats types
        if (hwfloat_p xf = xa->as<hwfloat>())
        {
            hwfloat_p yf = hwfloat_p(+ya);
            float xv = xf->value();
            float yv = yf->value();
            *cmp = (xv > yv) - (xv < yv);
            return true;
        }
        else
        {
            hwdouble_p xd = xa->as<hwdouble>();
            hwdouble_p yd = ya->as<hwdouble>();
            double xv = xd->value();
            double yv = yd->value();
            *cmp = (xv > yv) - (xv < yv);
            return true;
        }

        decimal_g xd = decimal_p(+xa);
        decimal_g yd = decimal_p(+ya);
        *cmp = decimal::compare(xd, yd);
        return true;
    }
    if (decimal_promotion(xa, ya))
    {
        /* Here, x and y have a decimal type */
        decimal_g xd = decimal_p(+xa);
        decimal_g yd = decimal_p(+ya);
        *cmp = decimal::compare(xd, yd);
        return true;
    }

    if (xt == ID_unit || yt == ID_unit)
    {
        algebraic_g diff = xa - ya; // Deal with cases such as ddays
        if (!diff)
            return false;
        *cmp = diff->is_zero() ? 0 : diff->is_negative() ? -1 : 1;
        return !rt.error();
    }

    if (((xt == ID_text && yt == ID_text) ||
         (xt == ID_symbol && yt == ID_symbol)))
    {
        // Lexical comparison
        size_t xl = 0;
        size_t yl = 0;
        utf8 xs = text_p(object_p(+x))->value(&xl);
        utf8 ys = text_p(object_p(+y))->value(&yl);
        size_t l = xl < yl ? xl : yl;

        // REVISIT: Unicode sorting?
        for (uint k = 0; k < l; k++)
        {
            if (int d = xs[k] - ys[k])
            {
                *cmp = (d > 0) - (d < 0);
                return true;
            }
        }

        *cmp = (xl > yl) - (xl < yl);
        return true;
    }

    if (((xt == ID_list && yt == ID_list) ||
         (xt == ID_array && yt == ID_array)))
    {
        list_p xl = list_p(+x);
        list_p yl = list_p(+y);
        list::iterator xi = xl->begin();
        list::iterator xe = xl->end();
        list::iterator yi = yl->begin();
        list::iterator ye = yl->end();

        // Lexicographic comparison of arrays and lists
        while(xi != xe && yi != ye)
        {
            object_p xo = *xi++;
            object_p yo = *yi++;
            if (xo->is_algebraic() && yo->is_algebraic())
            {
                algebraic_g xa = algebraic_p(xo);
                algebraic_g ya = algebraic_p(yo);
                if (compare(cmp, xa, ya))
                    if (*cmp)
                        return true;
            }
            else if (int d = xo->compare_to(yo))
            {
                *cmp = d;
                return true;
            }
        }
        *cmp = (xi != xe) - (yi != ye);
        return true;
    }

    if (xt == ID_True || xt == ID_False)
    {
        int xtruth = xt == ID_True;
        int ytruth = y->as_truth(false);
        if (ytruth >= 0)
        {
            *cmp = xtruth - ytruth;
            return true;
        }
    }
    if (yt == ID_True || yt == ID_False)
    {
        int xtruth = x->as_truth(false);
        int ytruth = yt == ID_True;
        if (xtruth >= 0)
        {
            *cmp = xtruth - ytruth;
            return true;
        }
    }

    // All other cases are type errors
    rt.type_error();
    return false;
}


object::result comparison::compare(comparison_fn comparator, id op)
// ----------------------------------------------------------------------------
//   Compare items from the stack
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(1);
    object_p y = rt.stack(0);
    if (!x || !y)
        return ERROR;
    if (!x->is_extended_algebraic() || !y->is_extended_algebraic())
    {
        rt.type_error();
        return ERROR;
    }

    algebraic_g xa = algebraic_p(x);
    algebraic_g ya = algebraic_p(y);

    // Convert arguments to numeric if necessary
    if (Settings.NumericalResults())
    {
        (void) to_decimal(xa, true);          // May fail silently
        (void) to_decimal(ya, true);
    }
    if (!xa || !ya)
        return ERROR;

    algebraic_g ra;
    if (xa->is_symbolic() || ya->is_symbolic())
        ra = expression::make(op, xa, ya);
    else
        ra = compare(comparator, op, xa, ya);


    if (ra)
        if (rt.drop(2))
            if (rt.push(+ra))
                return OK;

    return ERROR;
}


algebraic_p comparison::compare(comparison_fn comparator,
                                id            op,
                                algebraic_r   x,
                                algebraic_r   y)
// ----------------------------------------------------------------------------
//   Compare two algebraic values without using the stack
// ----------------------------------------------------------------------------
{
    int cmp = 0;
    if (compare(&cmp, x, y))
    {
        // Could evaluate the result, return True or False
        id type = comparator(cmp) ? ID_True : ID_False;
        return algebraic_p(command::static_object(type));
    }

    // Otherwise, need to build an equation with the comparison
    expression_p eq = expression::make(op, x, y);
    return eq;
}


object::result comparison::is_same(bool names)
// ----------------------------------------------------------------------------
//   Check if two objects are strictly identical
// ----------------------------------------------------------------------------
//   If 'names' is true, evaluate names (behavior of '==' aka TestSame)
//   If 'names' is false, do not evaluate names (behavior of 'same')
{
    object_p y = rt.stack(1);
    object_p x = rt.stack(0);
    if (!x || !y)
        return ERROR;

    // Check that the objects are strictly identical
    bool same = false;
    id xt = x->type();
    id yt = y->type();

    if (names)
    {
        if (symbol_p xs = x->as_quoted<symbol>())
        {
            x = xs->recall();
            xt = x->type();
        }
        else if (local_p xl = x->as_quoted<local>())
        {
            x = xl->recall();
            xt = x->type();
        }

        if (symbol_p ys = y->as_quoted<symbol>())
        {
            y = ys->recall();
            yt = y->type();
        }
        else if (local_p yl = x->as_quoted<local>())
        {
            y = yl->recall();
            yt = y->type();
        }
    }

    if (xt == yt)
    {
        size_t xs = x->size();
        size_t ys = y->size();
        if (xs == ys)
            same = memcmp(x, y, xs) == 0;
    }
    rt.drop(2);
    id type = same ? ID_True : ID_False;
    if (rt.push(command::static_object(type)))
        return OK;
    return ERROR;
}


template<>
object::result comparison::evaluate<TestSame>()
// ----------------------------------------------------------------------------
//   For "==", we want the same type, no promotion, but evaluate names
// ----------------------------------------------------------------------------
{
    return is_same(true);
}


template<>
object::result comparison::evaluate<same>()
// ----------------------------------------------------------------------------
//   For "same", we want the same type, no promotion
// ----------------------------------------------------------------------------
{
    return is_same(false);
}


bool smaller_magnitude(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Compare magnitude
// ----------------------------------------------------------------------------
{
    return algebraic::compare(abs::run(x), abs::run(y)) < 0;
}



// ============================================================================
//
//   Instantiations
//
// ============================================================================

template object::result comparison::evaluate<TestLT>();
template object::result comparison::evaluate<TestLE>();
template object::result comparison::evaluate<TestEQ>();
template object::result comparison::evaluate<TestGT>();
template object::result comparison::evaluate<TestGE>();
template object::result comparison::evaluate<TestNE>();



// ============================================================================
//
//    Commands for True and False
//
// ============================================================================

COMMAND_BODY(True)
// ----------------------------------------------------------------------------
//   Evaluate as a static (non-GC) version of True
// ----------------------------------------------------------------------------
{
    if (rt.push(True::static_self()))
        return OK;
    return ERROR;
}

COMMAND_BODY(False)
// ----------------------------------------------------------------------------
//   Evaluate as a static (non-GC) version of False
// ----------------------------------------------------------------------------
{
    if (rt.push(False::static_self()))
        return OK;
    return ERROR;
}



// ============================================================================
//
//   C++ interface
//
// ============================================================================

algebraic_p operator==(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Equality operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestEQ::evaluate(x, y);
}


algebraic_p operator<=(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Less or equal operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestLE::evaluate(x, y);
}


algebraic_p operator>=(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Gretter or equal operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestGE::evaluate(x, y);
}


algebraic_p operator!=(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Inequality operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestNE::evaluate(x, y);
}


algebraic_p operator<(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Less  operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestLT::evaluate(x, y);
}


algebraic_p operator>(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Gretter operation on algebraic objects
// ----------------------------------------------------------------------------
{
    return TestGT::evaluate(x, y);
}
