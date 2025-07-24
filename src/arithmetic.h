#ifndef ARITHMETIC_H
#define ARITHMETIC_H
// ****************************************************************************
//  arithmetic.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Basic arithmetic operations
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
#include "bignum.h"
#include "complex.h"
#include "decimal.h"
#include "fraction.h"
#include "hwfp.h"
#include "range.h"
#include "runtime.h"


RECORDER_DECLARE(arithmetic);

struct arithmetic : algebraic
// ----------------------------------------------------------------------------
//   Shared logic for all arithmetic operations
// ----------------------------------------------------------------------------
{
    arithmetic(id i): algebraic(i) {}

    static bool decimal_promotion(algebraic_g &x, algebraic_g &y)
    {
        if (!x || !y || !x->is_real() || !y->is_real())
            return false;
        return decimal_promotion(x) && decimal_promotion(y);
    }

    static bool decimal_promotion(algebraic_g &x)
    {
        return algebraic::decimal_promotion(x);
    }

    static bool hwfp_promotion(algebraic_g &x, algebraic_g &y)
    {
        if (!x || !y || !x->is_real() || !y->is_real())
            return false;
        if (hwfp_promotion(x) && hwfp_promotion(y))
        {
            // It's possible for the two to have distinct types
            if (hwfloat_p xf = x->as<hwfloat>())
            {
                if (y->type() != ID_hwfloat)
                    x = hwdouble::make(xf->value());
            }
            else if (hwfloat_p yf = y->as<hwfloat>())
            {
                y = hwdouble::make(yf->value());
            }
            return x->type() == y->type();
        }
        return false;
    }

    static bool hwfp_promotion(algebraic_g &x)
    {
        return algebraic::hwfp_promotion(x);
    }

    static bool complex_promotion(algebraic_g &x, id type = ID_rectangular)
    {
        return algebraic::complex_promotion(x, type);
    }
    static bool complex_promotion(algebraic_g &x, algebraic_g &y);
    static bool range_promotion(algebraic_g &x, id type = ID_range)
    {
        return algebraic::range_promotion(x, type);
    }
    static bool range_promotion(algebraic_g &x, algebraic_g &y);
    static fraction_p fraction_promotion(algebraic_g &x);
    static algebraic_p zero_divide(algebraic_r y, algebraic_r x);



    // We do not insert parentheses for algebraic values
    INSERT_DECL(arithmetic);

protected:
    typedef bool (*integer_fn)(id &xt, id &yt, ularge &xv, ularge &yv);
    typedef bool (*bignum_fn)(bignum_g &x, bignum_g &y);
    typedef bool (*fraction_fn)(fraction_g &x, fraction_g &y);
    typedef bool (*complex_fn)(complex_g &x, complex_g &y);
    typedef bool (*range_fn)(range_g &x, range_g &y);

    // Function pointers used by generic evaluation code
    typedef hwfloat_p (*hwfloat_fn)(hwfloat_r, hwfloat_r y);
    typedef hwdouble_p (*hwdouble_fn)(hwdouble_r, hwdouble_r y);
    typedef decimal_p (*decimal_fn)(decimal_r x, decimal_r y);
    typedef arithmetic_fn (*target_fn)(algebraic_r x, algebraic_r y);

    // Structure holding the function pointers called by generic code
    struct ops
    {
        decimal_fn    decop;
        hwfloat_fn    fop;
        hwdouble_fn   dop;
        integer_fn    integer_ok;
        bignum_fn     bignum_ok;
        fraction_fn   fraction_ok;
        complex_fn    complex_ok;
        range_fn      range_ok;
        arithmetic_fn non_numeric;
    };
    typedef const ops &ops_t;
    template <typename Op> static ops_t Ops();

    static result evaluate(id op, ops_t ops);

    template <typename Op> static result evaluate();
    // ------------------------------------------------------------------------
    //   Stack-based evaluation for all binary operators
    // ------------------------------------------------------------------------

    static algebraic_p evaluate(id op, algebraic_r x, algebraic_r y, ops_t ops);

    template <typename Op>
    static algebraic_p evaluate(algebraic_r x, algebraic_r y);
    // ------------------------------------------------------------------------
    //   C++ wrapper for the operation
    // ------------------------------------------------------------------------

    template <typename Op>
    static algebraic_p non_numeric(algebraic_r UNUSED x, algebraic_r UNUSED y)
    // ------------------------------------------------------------------------
    //   Return value if we can process non-numeric objects of the type
    // ------------------------------------------------------------------------
    {
        return nullptr;
    }

    template <typename Op>
    static algebraic_p optimize(algebraic_r UNUSED x, algebraic_r UNUSED y)
    // ------------------------------------------------------------------------
    //   Return optimizations that are shared between fast and slow path
    // ------------------------------------------------------------------------
    {
        return nullptr;
    }

};


#define ARITHMETIC_DECLARE(derived, Precedence)                         \
/* ----------------------------------------------------------------- */ \
/*  Macro to define an arithmetic command                            */ \
/* ----------------------------------------------------------------- */ \
struct derived : arithmetic                                             \
{                                                                       \
    derived(id i = ID_##derived) : arithmetic(i)                        \
    { }                                                                 \
                                                                        \
    static bool integer_ok(id &xt, id &yt, ularge &xv, ularge &yv);     \
    static bool bignum_ok(bignum_g &x, bignum_g &y);                    \
    static bool fraction_ok(fraction_g &x, fraction_g &y);              \
    static bool complex_ok(complex_g &x, complex_g &y);                 \
    static bool range_ok(range_g &x, range_g &y);                       \
    static constexpr decimal_fn decop = decimal::derived;               \
                                                                        \
    static algebraic_p arith_float(algebraic_r x, algebraic_r y)        \
    {                                                                   \
        hwfloat_r xf = (hwfloat_r) x;                                   \
        hwfloat_r yf = (hwfloat_r) y;                                   \
        return hwfloat::derived(xf, yf);                                \
    }                                                                   \
                                                                        \
    static algebraic_p arith_double(algebraic_r x, algebraic_r y)       \
    {                                                                   \
        hwdouble_r xf = (hwdouble_r) x;                                 \
        hwdouble_r yf = (hwdouble_r) y;                                 \
        return hwdouble::derived(xf, yf);                               \
    }                                                                   \
                                                                        \
    static arithmetic_fn target_float(algebraic_r x, algebraic_r y)     \
    {                                                                   \
        return x->type() == ID_hwfloat && y->type() == ID_hwfloat       \
            ? arith_float : nullptr;                                    \
    }                                                                   \
    static arithmetic_fn target_double(algebraic_r x, algebraic_r y)    \
    {                                                                   \
        return x->type() == ID_hwdouble && y->type() == ID_hwdouble     \
            ? arith_double : nullptr;                                   \
    }                                                                   \
                                                                        \
    static hwfloat_p do_float(hwfloat_r x, hwfloat_r y)                 \
    {                                                                   \
        remember(target_float);                                         \
        return hwfloat::derived(x, y);                                  \
    }                                                                   \
    static hwdouble_p do_double(hwdouble_r x, hwdouble_r y)             \
    {                                                                   \
        remember(target_double);                                        \
        return hwdouble::derived(x, y);                                 \
    }                                                                   \
    static constexpr auto       fop   = do_float;                       \
    static constexpr auto       dop   = do_double;                      \
                                                                        \
    OBJECT_DECL(derived)                                                \
    ARITY_DECL(2);                                                      \
    PREC_DECL(Precedence);                                              \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        record(arithmetic, "Evaluating " #derived " arop %t", o);       \
        rt.command(o);                                                  \
        if (!rt.args(2))                                                \
            return ERROR;                                               \
        return arithmetic::evaluate<derived>();                         \
    }                                                                   \
    static algebraic_g run(algebraic_r x, algebraic_r y)                \
    {                                                                   \
        return evaluate(x, y);                                          \
    }                                                                   \
    static algebraic_p evaluate(algebraic_r x, algebraic_r y)           \
    {                                                                   \
        return arithmetic::evaluate<derived>(x, y);                     \
    }                                                                   \
    static void remember(target_fn tgt)                                 \
    {                                                                   \
        target = tgt;                                                   \
    }                                                                   \
                                                                        \
    static target_fn target;                                            \
}


ARITHMETIC_DECLARE(add,             ADDITIVE);
ARITHMETIC_DECLARE(subtract,        ADDITIVE);
ARITHMETIC_DECLARE(multiply,        MULTIPLICATIVE);
ARITHMETIC_DECLARE(divide,          MULTIPLICATIVE);
ARITHMETIC_DECLARE(mod,             MULTIPLICATIVE);
ARITHMETIC_DECLARE(rem,             MULTIPLICATIVE);
ARITHMETIC_DECLARE(pow,             POWER);
ARITHMETIC_DECLARE(hypot,           POWER);
ARITHMETIC_DECLARE(atan2,           POWER);

COMMAND_DECLARE(Div2, 2);



// ============================================================================
//
//    Arithmetic interface for C++
//
// ============================================================================

algebraic_g operator-(algebraic_r x);
algebraic_g operator+(algebraic_r x, algebraic_r y);
algebraic_g operator-(algebraic_r x, algebraic_r y);
algebraic_g operator*(algebraic_r x, algebraic_r y);
algebraic_g operator/(algebraic_r x, algebraic_r y);
algebraic_g operator%(algebraic_r x, algebraic_r y);
algebraic_g pow(algebraic_r x, algebraic_r y);
algebraic_g pow(algebraic_r x, ularge y);

#endif // ARITHMETIC
