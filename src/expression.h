#ifndef EXPRESSION_H
#define EXPRESSION_H
// ****************************************************************************
//  expression.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of algebraic expressions
//
//     Expressions are simply programs that are rendered and parsed specially
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
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
#include "program.h"
#include "settings.h"
#include "symbol.h"

GCP(expression);
GCP(funcall);
GCP(grob);
struct grapher;

struct expression : program
// ----------------------------------------------------------------------------
//   An expression is a program with ' and ' as delimiters
// ----------------------------------------------------------------------------
//   We also need special parsing and rendering of algebraic objects
{
    expression(id type, gcbytes bytes, size_t len): program(type, bytes, len) {}
    static size_t required_memory(id i, gcbytes UNUSED bytes, size_t len)
    {
        return program::required_memory(i, bytes, len);
    }

    // Building an expression from an object
    expression(id type, algebraic_r arg);
    static size_t required_memory(id i, algebraic_r arg);

    // Building expressions from one or two arguments
    expression(id type, id op, algebraic_r arg);
    static size_t required_memory(id i, id op, algebraic_r arg);
    expression(id type, id op, algebraic_r x, algebraic_r y);
    static size_t required_memory(id i, id op, algebraic_r x, algebraic_r y);

    // Building expressions from an array of arguments
    expression(id type, id op, algebraic_g arg[], uint arity);
    static size_t required_memory(id i, id op,  algebraic_g arg[], uint arity);

    object_p quoted(id type = ID_object) const;
    static size_t size_in_expression(object_p obj);

    static expression_p make(algebraic_r x, id type = ID_expression)
    {
        if (!x)
            return nullptr;
        return rt.make<expression>(type, x);
    }

    static expression_p make(id op, algebraic_r x, id type = ID_expression)
    {
        if (!x)
            return nullptr;
        return rt.make<expression>(type, op, x);
    }

    static expression_p make(id op, algebraic_r x, algebraic_r y,
                           id type = ID_expression)
    {
        if (!x || !y)
            return nullptr;
        return rt.make<expression>(type, op, x, y);
    }

    static expression_p make(id op, algebraic_g args[], uint arity,
                             id type = ID_expression)
    {
        for (uint a = 0; a < arity; a++)
            if (!args[a])
                return nullptr;
        return rt.make<expression>(type, op, args, arity);
    }

    static expression_p get(object_p obj);
    static list_p current_equation(bool all, bool error);
    bool is_well_defined(symbol_p solving = nullptr, bool error = true) const;

    typedef expression_p (expression::*command_fn)(symbol_r name) const;
    static result variable_command(command_fn callback);



    // ========================================================================
    //
    //  Rewrite engine (↑MATCH et ↓MATCH)
    //
    // ========================================================================

    expression_p rewrite_up(expression_r from,
                            expression_r to,
                            expression_r cond,
                            uint *count = nullptr) const
    {
        return rewrite(from, to, cond, count, false);
    }
    expression_p rewrite_up(expression_p from,
                            expression_p to,
                            expression_p cond  = nullptr,
                            uint        *count = nullptr) const

    {
        return rewrite_up(expression_g(from),
                          expression_g(to),
                          expression_g(cond),
                          count);
    }
    expression_p rewrite_down(expression_r from,
                              expression_r to,
                              expression_r cond,
                              uint *count = nullptr) const
    {
        return rewrite(from, to, cond, count, true);
    }
    expression_p rewrite_down(expression_p from,
                              expression_p to,
                              expression_p cond  = nullptr,
                              uint        *count = nullptr) const
    {
        return rewrite_down(expression_g(from),
                            expression_g(to),
                            expression_g(cond),
                            count);
    }

    expression_p rewrite(expression_r from,
                         expression_r to,
                         expression_r cond,
                         uint        *count,
                         bool         down) const;
    expression_p rewrite(expression_p from,
                         expression_p to,
                         expression_p cond,
                         uint        *count,
                         bool         down) const
    {
        return rewrite(expression_g(from),
                       expression_g(to),
                       expression_g(cond),
                       count,
                       down);
    }
    static expression_p rewrite(expression_r eq,
                                expression_r from,
                                expression_r to,
                                expression_r cond,
                                uint        *count,
                                bool         down)
    {
        return eq->rewrite(from, to, cond, count, down);
    }

    enum rwrepeat       { ONCE,         REPEAT };
    enum rwconds        { ALWAYS,       CONDITIONAL };
    enum rwdir          { DOWN,         UP };

    template<rwdir down=DOWN, rwconds conds=ALWAYS, rwrepeat rep=REPEAT>
    expression_p do_rewrites(size_t       size,
                             const byte_p rewrites[],
                             uint        *count = nullptr) const
    // ------------------------------------------------------------------------
    //   Apply a series of rewrites
    // ------------------------------------------------------------------------
    {
        uint         rwcount = rep ? Settings.MaxRewrites() : 1;
        expression_g eq      = this;
        expression_g last    = nullptr;
        bool         intr    = false;
        settings::SaveExplicitWildcards ewc(false);
        settings::SaveAutoSimplify as(false);
        do
        {
            last = eq;

            for (size_t i = 0; i < size; i += conds ? 3 : 2)
            {
                eq = eq->rewrite(expression_p(rewrites[i+0]),
                                 expression_p(rewrites[i+1]),
                                 expression_p(conds ? rewrites[i+2] : nullptr),
                                 count, down);
                if (!eq)
                    return nullptr;
                intr = program::interrupted();
                if (intr)
                    break;
            }
            if (+eq == +last || intr)
                break;
        } while (--rwcount);

        if (rep && !rwcount)
            rt.too_many_rewrites_error();
        return eq;
    }

    template <rwdir down=DOWN, rwconds conds=ALWAYS, rwrepeat rep=REPEAT,
              typename ...args>
    expression_p rewrites(args... rest) const
    {
        static constexpr byte_p rwdata[] = { rest.as_bytes()... };
        return do_rewrites<down,conds,rep>(sizeof...(rest), rwdata, nullptr);
    }



    // ========================================================================
    //
    //   Common rewrite rules
    //
    // ========================================================================

    expression_p expand() const;
    expression_p collect() const;
    expression_p fold_constants() const;
    expression_p reorder_terms() const;
    expression_p simplify() const;
    expression_p as_difference_for_solve() const; // Transform A=B into A-B
    bool         split_equation(expression_g &left, expression_g &right) const;
    bool         split(id ty, expression_g &left, expression_g &right) const;
    object_p     outermost_operator() const;
    bool         is_linear(symbol_r sym, algebraic_g &a, algebraic_g &b) const;
    bool         depends_on(symbol_r sym) const;
    size_t       render(renderer &r, bool quoted = false) const
    {
        return render(this, r, quoted);
    }
    algebraic_p        simplify_products() const;
    static algebraic_p factor_out(algebraic_g  expr,
                                  algebraic_g  factor,
                                  algebraic_g &scale,
                                  algebraic_g &exponent);
    expression_p isolate(symbol_r sym) const;
    expression_p derivative(symbol_r sym) const;
    expression_p primitive(symbol_r sym) const;

    expression_p where(algebraic_r args) const
    {
        algebraic_g expr = this;
        if (algebraic_p obj = list::where(expr, args))
            if (expression_p result = obj->as<expression>())
                return result;
        return expression_p(+expr);
    }


    // ========================================================================
    //
    //   Graphical rendering of expressions
    //
    // ========================================================================

protected:
    static symbol_p render(uint depth, int &precedence, bool edit);
    static size_t   render(const expression *o, renderer &r, bool quoted);
    static symbol_p parentheses(symbol_g what);
    static symbol_p space(symbol_g what);

public:
    static grob_p   graph(grapher &g, uint depth, int &precedence);
    static grob_p   parentheses(grapher &g, grob_g x, uint padding = 0);
    static grob_p   abs_norm(grapher &g, grob_g x, uint padding = 2);
    static grob_p   root(grapher &g, grob_g x);
    static grob_p   ratio(grapher &g, grob_g x, grob_g y);
    static grob_p   ratio(grapher &g, cstring x, grob_g y);
    static grob_p   infix(grapher &g,
                          coord vx, grob_g x,
                          coord vs, grob_g sep,
                          coord vy, grob_g y);
    static grob_p   infix(grapher &g,
                          coord vx, grob_g x,
                          coord vs, cstring sep,
                          coord vy, grob_g y);
    static grob_p   suscript(grapher &g,
                             coord vx, grob_g x,
                             coord vy, grob_g y,
                             int dir=1, bool alignleft = true);
    static grob_p   suscript(grapher &g,
                             coord vx, grob_g x,
                             coord vy, cstring exp,
                             int dir=1, bool alignleft = true);
    static grob_p   suscript(grapher &g,
                             coord vx, cstring x,
                             coord vy, grob_g y,
                             int dir=1, bool alignleft = true);
    static grob_p   prefix(grapher &g,
                           coord vx, grob_g x,
                           coord vy, grob_g y,
                           int dir=0);
    static grob_p   prefix(grapher &g,
                           coord vx, cstring pfx,
                           coord vy, grob_g y,
                           int dir=0);

    typedef grob_p (*sumprod_fn)(grapher &g, blitter::size h);
    static grob_p     sumprod(grapher &g, id oid, sumprod_fn shape,
                              coord vi, grob_g index,
                              coord vf, grob_g first,
                              coord vl, grob_g last,
                              coord ve, grob_g expr);
    static grob_p     sum(grapher &g, blitter::size h);
    static grob_p     product(grapher &g, blitter::size h);
    static grob_p     integral(grapher &g, blitter::size h);
    static sumprod_fn sumprod_shape(id oid);


public:
    OBJECT_DECL(expression);
    PARSE_DECL(expression);
    RENDER_DECL(expression);
    GRAPH_DECL(expression);
    EVAL_DECL(expression);
    HELP_DECL(expression);

public:
    // Dependent and independent variables
    static symbol_g    *independent;
    static object_g    *independent_value;
    static symbol_g    *dependent;
    static object_g    *dependent_value;
    static bool         in_algebraic;
    static bool         contains_independent_variable;
    static uint         constant_index;

    typedef size_t (*funcall_match_fn)(funcall_p pat, funcall_p repl);
    typedef algebraic_p (*funcall_build_fn)(funcall_p src, funcall_p repl);
    static funcall_match_fn funcall_match;
    static funcall_build_fn funcall_build;
};


struct funcall : expression
// ----------------------------------------------------------------------------
//   Function call, indicating how many arguments we take from the stack
// ----------------------------------------------------------------------------
//   A function call F(1;2;3;4) is encoded as program `1 2 3 4 F`.
{
    funcall(id type, gcbytes bytes, size_t len)
        : expression(type, bytes, len) {}

    // Building expressions from an array of arguments
    funcall(id type, id op, algebraic_g args[], uint arity)
        : expression(type, op, args, arity) {}

    object_p arg(uint depth) const;
    array_p  args() const;

public:
    OBJECT_DECL(funcall);
    PARSE_DECL(funcall);
    EVAL_DECL(funcall);
};



// ============================================================================
//
//    C++ expression building (to create rules in C++ code)
//
// ============================================================================

template <byte ...args>
struct eq
// ----------------------------------------------------------------------------
//   A static expression builder for C++ code
// ----------------------------------------------------------------------------
{
    // Helper to ensure a compile error if we ever use a value above 128
    static constexpr byte leb(byte value)
    {
        return byte(value / byte(value < 128));
    }

    // Helper for low and high byte of a value that does not fit in one byte
    static constexpr byte lb(uint value)
    {
        return byte(leb(byte(value & 127)) | 128);
    }
    static constexpr byte hb(uint value)
    {
        return leb(byte((value / 128) & 127));
    }


    eq() {}
    static constexpr byte object_data[sizeof...(args) + 2] =
    {
        leb(object::ID_expression),
        leb(sizeof...(args)),  // Must be less than 128...
        args...
    };
    constexpr byte_p as_bytes() const
    {
        return object_data;
    }
    constexpr expression_p as_expression() const
    {
        return expression_p(object_data);
    }

    // Negation operation
    eq<args..., leb(object::ID_neg)>
    operator-()         { return eq<args..., leb(object::ID_neg)>(); }

    template <uint ty>
    using fntype = typename std::conditional<(ty < 128),
                                             eq<args..., byte(ty)>,
                                             eq<args..., lb(ty), hb(ty)>>::type;

#define EQ_FUNCTION(name)                                               \
    fntype<object::ID_##name> name()                                    \
    {                                                                   \
        return fntype<object::ID_##name>();                             \
    }

    EQ_FUNCTION(sqrt);
    EQ_FUNCTION(cbrt);

    EQ_FUNCTION(sin);
    EQ_FUNCTION(cos);
    EQ_FUNCTION(tan);
    EQ_FUNCTION(asin);
    EQ_FUNCTION(acos);
    EQ_FUNCTION(atan);

    EQ_FUNCTION(sinh);
    EQ_FUNCTION(cosh);
    EQ_FUNCTION(tanh);
    EQ_FUNCTION(asinh);
    EQ_FUNCTION(acosh);
    EQ_FUNCTION(atanh);

    EQ_FUNCTION(log1p);
    EQ_FUNCTION(expm1);
    EQ_FUNCTION(log);
    EQ_FUNCTION(log10);
    EQ_FUNCTION(log2);
    EQ_FUNCTION(exp);
    EQ_FUNCTION(exp10);
    EQ_FUNCTION(exp2);
    EQ_FUNCTION(erf);
    EQ_FUNCTION(erfc);
    EQ_FUNCTION(tgamma);
    EQ_FUNCTION(lgamma);

    EQ_FUNCTION(abs);
    EQ_FUNCTION(sign);
    EQ_FUNCTION(inv);
    EQ_FUNCTION(neg);
    EQ_FUNCTION(sq);
    EQ_FUNCTION(cubed);
    EQ_FUNCTION(fact);

    EQ_FUNCTION(re);
    EQ_FUNCTION(im);
    EQ_FUNCTION(arg);
    EQ_FUNCTION(conj);

#undef EQ_FUNCTION

    // Arithmetic
    template<byte ...y>
    eq<args..., y..., leb(object::ID_add)>
    operator+(eq<y...>) { return eq<args..., y..., leb(object::ID_add)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_sub)>
    operator-(eq<y...>) { return eq<args..., y..., leb(object::ID_sub)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_mul)>
    operator*(eq<y...>) { return eq<args..., y..., leb(object::ID_mul)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_div)>
    operator/(eq<y...>) { return eq<args..., y..., leb(object::ID_div)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_mod)>
    operator%(eq<y...>) { return eq<args..., y..., leb(object::ID_mod)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_rem)>
    rem(eq<y...>) { return eq<args..., y..., leb(object::ID_rem)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_pow)>
    operator^(eq<y...>) { return eq<args..., y..., leb(object::ID_pow)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_pow)>
    pow(eq<y...>) { return eq<args..., y..., leb(object::ID_pow)>(); }

    template<byte ...y>
    eq<args..., y..., lb(object::ID_Derivative), hb(object::ID_Derivative)>
    deriv(eq<y...>) { return eq<args..., y...,
                                lb(object::ID_Derivative),
                                hb(object::ID_Derivative)>(); }
    template<byte ...y>
    eq<args..., y..., lb(object::ID_Derivative), hb(object::ID_Derivative)>
    operator>>(eq<y...>) { return eq<args..., y...,
                                     lb(object::ID_Derivative),
                                     hb(object::ID_Derivative)>(); }
    template<byte ...y>
    eq<args..., y..., lb(object::ID_Primitive), hb(object::ID_Primitive)>
    prim(eq<y...>) { return eq<args..., y...,
                               lb(object::ID_Primitive),
                               hb(object::ID_Primitive)>(); }
    template<byte ...y>
    eq<args..., y..., lb(object::ID_Primitive), hb(object::ID_Primitive)>
    operator<<(eq<y...>) { return eq<args..., y...,
                                     lb(object::ID_Primitive),
                                     hb(object::ID_Primitive)>(); }

    template<byte ...y>
    eq<leb(object::ID_funcall),
       leb(sizeof...(args) + sizeof...(y)),
       y..., args...>
    call(eq<y...>) { return eq<leb(object::ID_funcall),
                               leb(sizeof...(args)+sizeof...(y)),
                               y..., args...>(); }
    template<byte ...y>
    eq<leb(object::ID_funcall),
       leb(sizeof...(args) + sizeof...(y)),
       y..., args...>
    operator()(eq<y...>) { return eq<leb(object::ID_funcall),
                                     leb(sizeof...(args)+sizeof...(y)),
                                     y..., args...>(); }

    // Comparisons
    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestLT)>
    operator<(eq<y...>) { return eq<args..., y..., leb(object::ID_TestLT)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestEQ)>
    operator==(eq<y...>) { return eq<args..., y..., leb(object::ID_TestEQ)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestGT)>
    operator>(eq<y...>) { return eq<args..., y..., leb(object::ID_TestGT)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestLE)>
    operator<=(eq<y...>) { return eq<args..., y..., leb(object::ID_TestLE)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestNE)>
    operator!=(eq<y...>) { return eq<args..., y..., leb(object::ID_TestNE)>(); }

    template<byte ...y>
    eq<args..., y..., leb(object::ID_TestGE)>
    operator>=(eq<y...>) { return eq<args..., y..., leb(object::ID_TestGE)>(); }

};


#define EQ_FUNCTION(name)                                       \
    template<byte ...x>                                         \
    typename eq<x...>::template fntype<object::ID_##name>       \
    name(eq<x...> xeq)         { return xeq.name(); }


EQ_FUNCTION(sqrt);
EQ_FUNCTION(cbrt);

EQ_FUNCTION(sin);
EQ_FUNCTION(cos);
EQ_FUNCTION(tan);
EQ_FUNCTION(asin);
EQ_FUNCTION(acos);
EQ_FUNCTION(atan);

EQ_FUNCTION(sinh);
EQ_FUNCTION(cosh);
EQ_FUNCTION(tanh);
EQ_FUNCTION(asinh);
EQ_FUNCTION(acosh);
EQ_FUNCTION(atanh);

EQ_FUNCTION(log1p);
EQ_FUNCTION(expm1);
EQ_FUNCTION(log);
EQ_FUNCTION(log10);
EQ_FUNCTION(log2);
EQ_FUNCTION(exp);
EQ_FUNCTION(exp10);
EQ_FUNCTION(exp2);
EQ_FUNCTION(erf);
EQ_FUNCTION(erfc);
EQ_FUNCTION(tgamma);
EQ_FUNCTION(lgamma);

EQ_FUNCTION(abs);
EQ_FUNCTION(sign);
EQ_FUNCTION(inv);
EQ_FUNCTION(neg);
EQ_FUNCTION(sq);
EQ_FUNCTION(cubed);
EQ_FUNCTION(fact);

EQ_FUNCTION(re);
EQ_FUNCTION(im);
EQ_FUNCTION(arg);
EQ_FUNCTION(conj);


#undef EQ_FUNCTION

// Pi constant
// struct eq_pi : eq<object::ID_pi> {};

// Build a symbol out of a character
template <byte c>       struct eq_symbol  : eq<object::ID_symbol,  1, c> {};

// Build an integer constant
template <uint c, std::enable_if_t<(c >= 0 && c < 128), bool> = true>
struct eq_integer : eq<object::ID_integer, byte(c)> {};
template <int c, std::enable_if_t<(c <= 0 && c > -128), bool> = true>
struct eq_neg_integer : eq<object::ID_neg_integer, byte(-c)> {};

// Build a conditional that always runs
struct eq_always : eq<object::ID_True>
{
    constexpr byte_p as_bytes() const
    {
        return nullptr;
    }
};



// ============================================================================
//
//   User commands
//
// ============================================================================

COMMAND_DECLARE(MatchUp,   2);
COMMAND_DECLARE(MatchDown, 2);

FUNCTION(Expand);
FUNCTION(Collect);
FUNCTION(FoldConstants);
FUNCTION(ReorderTerms);
FUNCTION(Simplify);

COMMAND_DECLARE(Apply, 2);
COMMAND_DECLARE(Isolate, 2);
COMMAND_DECLARE_SPECIAL(Derivative, algebraic, 2,
                        PREC_DECL(SYMBOL);
                        INSERT_DECL(Derivative);
                        PARSE_DECL(Derivative);
                        static bool can_be_symbolic(uint) { return true; });
COMMAND_DECLARE_SPECIAL(Primitive, algebraic, 2,
                        PREC_DECL(MULTIPLICATIVE);
                        INSERT_DECL(Primitive);
                        PARSE_DECL(Primitive);
                        static bool can_be_symbolic(uint) { return true; });
COMMAND_DECLARE_SPECIAL(Where, arithmetic, 2, PREC_DECL(WHERE); );
NFUNCTION(Subst, 2, static bool can_be_symbolic(uint) { return true; } );

#endif // EXPRESSION_H
