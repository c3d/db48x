#ifndef FUNCTIONS_H
#define FUNCTIONS_H
// ****************************************************************************
//  functions.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Standard mathematoical functions
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
#include "complex.h"
#include "decimal.h"
#include "hwfp.h"
#include "range.h"
#include "runtime.h"

RECORDER_DECLARE(function);


struct function : algebraic
// ----------------------------------------------------------------------------
//   Shared logic for all standard functions
// ----------------------------------------------------------------------------
{
    function(id i): algebraic(i) {}


public:
    typedef complex_g  (*complex_fn)(complex_r x);
    typedef range_p    (*range_fn)(range_r x);
    typedef uncertain_p(*uncertain_fn)(uncertain_r x);
    typedef hwfloat_p  (*hwfloat_fn)(hwfloat_r x);
    typedef hwdouble_p (*hwdouble_fn)(hwdouble_r x);

    // Structure holding the function pointers called by generic code
    struct ops
    {
        decimal_fn    decop;
        hwfloat_fn    fop;
        hwdouble_fn   dop;
        complex_fn    zop;
        range_fn      rop;
        uncertain_fn  uop;
    };
    typedef const ops &ops_t;

    static result evaluate(id op, ops_t ops);
    // ------------------------------------------------------------------------
    //   Stack-based evaluation for all functions implemented in std library
    // ------------------------------------------------------------------------

    static algebraic_p evaluate(algebraic_r x, id op, ops_t ops);
    static algebraic_p evaluate_noclean(algebraic_r x, id op, ops_t ops);
    // ------------------------------------------------------------------------
    //   C++ evaluation for all functions implemented in BID library
    // ------------------------------------------------------------------------

    static result evaluate(algebraic_fn fn, uint seqtypes);
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
    // ------------------------------------------------------------------------
    //   Process exact trigonometry cases
    // ------------------------------------------------------------------------

    static const uint seqtypes = 0;


    typedef algebraic_p (*nfunction_fn)(id op, algebraic_g args[], uint arity);
    static result evaluate(id op, nfunction_fn fn, uint arity, uint symbolic);
    // ------------------------------------------------------------------------
    //   Evaluate a function with n arguments
    // ------------------------------------------------------------------------

};


#define STANDARD_FUNCTION(derived)                                      \
/* ----------------------------------------------------------------- */ \
/*  Macro to define a standard mathematical function (in library)    */ \
/* ----------------------------------------------------------------- */ \
struct derived : function                                               \
{                                                                       \
    derived(id i = ID_##derived) : function(i) {}                       \
                                                                        \
    static constexpr decimal_fn   decop = decimal::derived;             \
    static constexpr auto         fop   = hwfloat::derived;             \
    static constexpr auto         dop   = hwdouble::derived;            \
    static constexpr complex_fn   zop   = complex::derived;             \
    static constexpr range_fn     rop   = range::derived;               \
    static constexpr uncertain_fn uop   = uncertain::derived;           \
                                                                        \
public:                                                                 \
    OBJECT_DECL(derived);                                               \
    ARITY_DECL(1);                                                      \
    PREC_DECL(FUNCTION);                                                \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        record(function, "Evaluating " #derived " function %t", o);     \
        rt.command(o);                                                  \
        if (!rt.args(ARITY))                                            \
            return ERROR;                                               \
        return evaluate();                                              \
    }                                                                   \
    static result evaluate()                                            \
    {                                                                   \
        return function::evaluate(derived::evaluate, seqtypes);         \
    }                                                                   \
    static algebraic_g run(algebraic_r x) { return evaluate(x); }       \
    static algebraic_p evaluate(algebraic_r x)                          \
    {                                                                   \
        static const ops optable =                                      \
        {                                                               \
            decop, hwfloat_fn(fop), hwdouble_fn(dop), zop, rop, uop     \
        };                                                              \
        return function::evaluate(x, ID_##derived, optable);            \
    }                                                                   \
}

STANDARD_FUNCTION(sqrt);
STANDARD_FUNCTION(cbrt);

STANDARD_FUNCTION(sin);
STANDARD_FUNCTION(cos);
STANDARD_FUNCTION(tan);
STANDARD_FUNCTION(sec);
STANDARD_FUNCTION(csc);
STANDARD_FUNCTION(cot);
STANDARD_FUNCTION(asin);
STANDARD_FUNCTION(acos);
STANDARD_FUNCTION(atan);
STANDARD_FUNCTION(asec);
STANDARD_FUNCTION(acsc);
STANDARD_FUNCTION(acot);

STANDARD_FUNCTION(sinh);
STANDARD_FUNCTION(cosh);
STANDARD_FUNCTION(tanh);
STANDARD_FUNCTION(asinh);
STANDARD_FUNCTION(acosh);
STANDARD_FUNCTION(atanh);
STANDARD_FUNCTION(csch);
STANDARD_FUNCTION(sech);
STANDARD_FUNCTION(coth);
STANDARD_FUNCTION(acsch);
STANDARD_FUNCTION(asech);
STANDARD_FUNCTION(acoth);

STANDARD_FUNCTION(ln1p);
STANDARD_FUNCTION(expm1);
STANDARD_FUNCTION(ln);
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
    FUNCTION_EXT_PREC(derived, extra, FUNCTION)
#define FUNCTION_PREC(derived, prec)                                    \
    FUNCTION_EXT_PREC(derived,, prec)

#define FUNCTION_EXT_PREC(derived, extra, prec)                         \
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
    PREC_DECL(prec);                                                    \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        record(function, "Evaluating " #derived " function %t", o);     \
        rt.command(o);                                                  \
        if (!rt.args(ARITY))                                            \
            return ERROR;                                               \
        return evaluate();                                              \
    }                                                                   \
    extra                                                               \
public:                                                                 \
    static result evaluate()                                            \
    {                                                                   \
        return function::evaluate(derived::evaluate, seqtypes);         \
    }                                                                   \
    static algebraic_g run(algebraic_r x) { return evaluate(x); }       \
    static algebraic_p evaluate(algebraic_r x);                         \
};

#define FUNCTION(derived) FUNCTION_EXT(derived, )

#define FUNCTION_FANCY(derived)                                         \
    FUNCTION_EXT(derived, INSERT_DECL(derived);)
#define FUNCTION_MAT(derived)                                           \
    FUNCTION_EXT(derived,                                               \
                 static const uint seqtypes = 1UL<<ID_array;)
#define FUNCTION_FANCY_MAT(derived)                                     \
    FUNCTION_EXT(derived,                                               \
                 INSERT_DECL(derived);                                  \
                 static const uint seqtypes = 1UL<<ID_array;)

#define FUNCTION_BODY(derived)                  \
algebraic_p derived::evaluate(algebraic_r x)

FUNCTION_MAT(abs);
FUNCTION(sign);
FUNCTION_MAT(norm);
FUNCTION(IntPart);
FUNCTION(FracPart);
FUNCTION(ceil);
FUNCTION(floor);
FUNCTION(mant);
FUNCTION(xpon);
FUNCTION(SigDig);
FUNCTION_FANCY_MAT(inv);
FUNCTION_EXT_PREC(neg,
                  static const uint seqtypes = ((1UL << ID_array)
#if CONFIG_FIXED_BASED_OBJECTS
                                               |(1UL << ID_hex_integer)
                                               |(1UL << ID_dec_integer)
                                               |(1UL << ID_oct_integer)
                                               |(1UL << ID_bin_integer)
                                               |(1UL << ID_hex_bignum)
                                               |(1UL << ID_dec_bignum)
                                               |(1UL << ID_oct_bignum)
                                               |(1UL << ID_bin_bignum)
#endif // CONFIG_FIXED_BASED_OBJECTS
                                               |(1UL << ID_based_integer)
                                                |(1UL << ID_based_bignum)); ,
                  ADDITIVE);
FUNCTION_FANCY_MAT(sq);
FUNCTION_FANCY_MAT(cubed);
FUNCTION_FANCY(fact);

FUNCTION(re);
FUNCTION(im);
FUNCTION(arg);
FUNCTION(conj);

FUNCTION_EXT(ToDecimal,
             static const uint seqtypes = (1UL << ID_expression););
FUNCTION_EXT(ToFraction,
             static const uint seqtypes = (1UL << ID_expression););
FUNCTION_EXT(ToQuotient,
             static const uint seqtypes = (1UL << ID_expression););
FUNCTION(ToInteger);
FUNCTION(RadiansToDegrees);
FUNCTION(DegreesToRadians);



#define NFUNCTION(derived, fnarity)     NFUNCTION_EXT(derived, fnarity, )

#define NFUNCTION_EXT(derived, fnarity, extra)                          \
struct derived : function                                               \
/* ----------------------------------------------------------------- */ \
/*  Macro to define a mathematical function with more than 1 arg     */ \
/* ----------------------------------------------------------------- */ \
{                                                                       \
    derived(id i = ID_##derived) : function(i) {}                       \
                                                                        \
public:                                                                 \
    OBJECT_DECL(derived);                                               \
    ARITY_DECL(fnarity);                                                \
    PREC_DECL(FUNCTION);                                                \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        record(function, "Evaluating " #derived " nfunction %t", o);    \
        rt.command(o);                                                  \
        return evaluate();                                              \
    }                                                                   \
    extra                                                               \
public:                                                                 \
    static result evaluate()                                            \
    {                                                                   \
        return function::evaluate(derived::static_id,                   \
                                  derived::evaluate, fnarity,           \
                                  derived::SYMBOLIC_ARGS);              \
    }                                                                   \
    static algebraic_p evaluate(id op, algebraic_g args[], uint arity); \
}


#define NFUNCTION_BODY(derived)                              \
    algebraic_p derived::evaluate(id UNUSED          op,     \
                                  algebraic_g UNUSED args[], \
                                  uint UNUSED        arity)

NFUNCTION(Round, 2);
NFUNCTION(Truncate, 2);

NFUNCTION(ToStandardUncertainty, 2);
NFUNCTION(ToRelativeUncertainty, 2);
NFUNCTION(StandardRound, 2);
NFUNCTION(RelativeRound, 2);
NFUNCTION(PrecisionRound, 2);

NFUNCTION(xroot, 2);
NFUNCTION(comb, 2);
NFUNCTION(perm, 2);
NFUNCTION_EXT(Sum,     4, SYMARGS_DECL(SYMARG(1) SYMARG(4)); );
NFUNCTION_EXT(Product, 4, SYMARGS_DECL(SYMARG(1) SYMARG(4)); );
NFUNCTION_EXT(Min, 2,
              static algebraic_p evaluate(algebraic_r x, algebraic_r y);
    );
NFUNCTION_EXT(Max, 2,
              static algebraic_p evaluate(algebraic_r x, algebraic_r y);
    );


NFUNCTION(Percent, 2);
NFUNCTION(PercentChange, 2);
NFUNCTION(PercentTotal, 2);


#endif // FUNCTIONS_H
