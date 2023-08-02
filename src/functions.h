#ifndef FUNCTIONS_H
#define FUNCTIONS_H
// ****************************************************************************
//  functions.h                                                  DB48X project
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

#include "algebraic.h"
#include "array.h"
#include "complex.h"
#include "list.h"
#include "runtime.h"


struct function : algebraic
// ----------------------------------------------------------------------------
//   Shared logic for all standard functions
// ----------------------------------------------------------------------------
{
    function(id i): algebraic(i) {}


public:
    typedef complex_g (*complex_fn)(complex_r x);

    static result evaluate(id op, bid128_fn op128, complex_fn zop);
    // ------------------------------------------------------------------------
    //   Stack-based evaluation for all functions implemented in BID library
    // ------------------------------------------------------------------------

    static algebraic_p evaluate(algebraic_r x, id op,
                                bid128_fn op128, complex_fn zop);
    // ------------------------------------------------------------------------
    //   C++ evaluation for all functions implemented in BID library
    // ------------------------------------------------------------------------

    static result evaluate(algebraic_fn fn, bool mat);
    // ------------------------------------------------------------------------
    //  Evaluate on the stack function a function doing the evaluation
    // ------------------------------------------------------------------------

    static bool should_be_symbolic(id type);
    // ------------------------------------------------------------------------
    //   Check if we should process it symbolically
    // ------------------------------------------------------------------------

    static algebraic_p symbolic(id op, algebraic_r x);
    // ------------------------------------------------------------------------
    //   Process it symbolically
    // ------------------------------------------------------------------------

    static bool exact_trig(id op, algebraic_g &x);

    static void adjust_from_angle(bid128 &x);
    static void adjust_to_angle(bid128 &x);
    static bool adjust_to_angle(algebraic_g &x);

    static const bool does_matrices = false;
};


#define STANDARD_FUNCTION(derived)                                      \
/* ----------------------------------------------------------------- */ \
/*  Macro to define a standard mathematical function (in library)    */ \
/* ----------------------------------------------------------------- */ \
struct derived : function                                               \
{                                                                       \
    derived(id i = ID_##derived) : function(i) {}                       \
                                                                        \
    static constexpr auto bid128_op = bid128_##derived;                 \
    static constexpr complex_fn zop = complex::derived;                 \
                                                                        \
public:                                                                 \
    OBJECT_DECL(derived);                                               \
    ARITY_DECL(1);                                                      \
    PREC_DECL(FUNCTION);                                                \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        rt.command(fancy(ID_##derived));                                \
        return evaluate();                                              \
    }                                                                   \
    static result evaluate()                                            \
    {                                                                   \
        return function::evaluate(derived::evaluate, does_matrices);    \
    }                                                                   \
    static algebraic_g run(algebraic_r x) { return evaluate(x); }       \
    static algebraic_p evaluate(algebraic_r x)                          \
    {                                                                   \
        return function::evaluate(x, ID_##derived, bid128_op, zop);     \
    }                                                                   \
}

STANDARD_FUNCTION(sqrt);
STANDARD_FUNCTION(cbrt);

STANDARD_FUNCTION(sin);
STANDARD_FUNCTION(cos);
STANDARD_FUNCTION(tan);
STANDARD_FUNCTION(asin);
STANDARD_FUNCTION(acos);
STANDARD_FUNCTION(atan);

STANDARD_FUNCTION(sinh);
STANDARD_FUNCTION(cosh);
STANDARD_FUNCTION(tanh);
STANDARD_FUNCTION(asinh);
STANDARD_FUNCTION(acosh);
STANDARD_FUNCTION(atanh);

STANDARD_FUNCTION(log1p);
STANDARD_FUNCTION(expm1);
STANDARD_FUNCTION(log);
STANDARD_FUNCTION(log10);
STANDARD_FUNCTION(log2);
STANDARD_FUNCTION(exp);
STANDARD_FUNCTION(exp10);
STANDARD_FUNCTION(exp2);
STANDARD_FUNCTION(erf);
STANDARD_FUNCTION(erfc);
STANDARD_FUNCTION(tgamma);
STANDARD_FUNCTION(lgamma);


#define FUNCTION_EXT(derived, extra)                                    \
struct derived : function                                               \
/* ----------------------------------------------------------------- */ \
/*  Macro to define a mathematical function not from the library     */ \
/* ----------------------------------------------------------------- */ \
{                                                                       \
    derived(id i = ID_##derived) : function(i) {}                       \
                                                                        \
public:                                                                 \
    OBJECT_DECL(derived);                                               \
    ARITY_DECL(1);                                                      \
    PREC_DECL(FUNCTION);                                                \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        rt.command(fancy(ID_##derived));                                \
        return evaluate();                                              \
    }                                                                   \
    extra                                                               \
public:                                                                 \
    static result evaluate()                                            \
    {                                                                   \
        return function::evaluate(derived::evaluate, does_matrices);    \
    }                                                                   \
    static algebraic_g run(algebraic_r x) { return evaluate(x); }       \
    static algebraic_p evaluate(algebraic_r x);                         \
};

#define FUNCTION(derived) FUNCTION_EXT(derived, )

#define FUNCTION_FANCY(derived)                         \
    FUNCTION_EXT(derived, INSERT_DECL(derived);)
#define FUNCTION_MAT(derived)                                           \
    FUNCTION_EXT(derived,                                               \
                 static const bool does_matrices = true;)
#define FUNCTION_FANCY_MAT(derived)                                     \
    FUNCTION_EXT(derived,                                               \
                 INSERT_DECL(derived);                                  \
                 static const bool does_matrices = true;)

#define FUNCTION_BODY(derived)                  \
algebraic_p derived::evaluate(algebraic_r x)

FUNCTION_MAT(abs);
FUNCTION(sign);
FUNCTION_FANCY_MAT(inv);
FUNCTION(neg);
FUNCTION_FANCY_MAT(sq);
FUNCTION_FANCY_MAT(cubed);
FUNCTION_FANCY(fact);

FUNCTION(re);
FUNCTION(im);
FUNCTION(arg);
FUNCTION(conj);

FUNCTION(Expand);
FUNCTION(Collect);
FUNCTION(Simplify);

FUNCTION(ToDecimal);
FUNCTION(ToFraction);

#endif // FUNCTIONS_H
