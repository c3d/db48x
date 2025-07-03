// ****************************************************************************
//  expression.cc                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of algebraic equations
//
//     Equations are simply programs that are rendered and parsed specially
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

#include "expression.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "equations.h"
#include "finance.h"
#include "functions.h"
#include "grob.h"
#include "integer.h"
#include "parser.h"
#include "polynomial.h"
#include "precedence.h"
#include "renderer.h"
#include "settings.h"
#include "tag.h"
#include "unit.h"
#include "utf8.h"
#include "variables.h"

RECORDER(expression,            16, "Expressions and algebraic objects");
RECORDER(expression_error,      16, "Errors with expressions");
RECORDER(rewrites,              16, "Expression rewrites");
RECORDER(rewrites_done,         16, "Successful expression rewrites");

symbol_g *expression::independent                   = nullptr;
object_g *expression::independent_value             = nullptr;
symbol_g *expression::dependent                     = nullptr;
object_g *expression::dependent_value               = nullptr;
bool      expression::in_algebraic                  = false;
bool      expression::contains_independent_variable = false;
uint      expression::constant_index                = 0;


// Used to match and build user-defined function calls for deriv/integ
expression::funcall_match_fn expression::funcall_match = nullptr;
expression::funcall_build_fn expression::funcall_build = nullptr;



// ============================================================================
//
//    Expression
//
// ============================================================================

EVAL_BODY(expression)
// ----------------------------------------------------------------------------
//   Evaluate expressions, indicating that we are in algebraic mode
// ----------------------------------------------------------------------------
{
    save<bool> savealg(in_algebraic, true);
    if (running)
        return rt.push(o) ? OK : ERROR;
    return o->run_program();
}


PARSE_BODY(expression)
// ----------------------------------------------------------------------------
//    Try to parse this as an expression
// ----------------------------------------------------------------------------
{
    // If already parsing an expression, let upper parser deal with quote
    if (p.precedence)
        return SKIP;

    save<bool> nounits(unit::mode, false);
    p.precedence = 1;
    auto result = list_parse(ID_expression, p, '\'', '\'');
    p.precedence = 0;

    return result;
}


HELP_BODY(expression)
// ----------------------------------------------------------------------------
//   Help topic for expressions
// ----------------------------------------------------------------------------
{
    return utf8("Expressions");
}


expression_p expression::parse_all(utf8 src, size_t len)
// ----------------------------------------------------------------------------
//   Parse the object
// ----------------------------------------------------------------------------
{
    parser p(src, len);
    p.precedence = LOWEST;
    if (list_parse(ID_expression, p, 0, 0) == OK)
        if (p.length == len)
            if (expression_p expr = expression::get(p.out))
                return expr;
    return nullptr;
}


symbol_p expression::parentheses(symbol_g arg)
// ----------------------------------------------------------------------------
//   Render, putting parentheses around an argument
// ----------------------------------------------------------------------------
{
    symbol_g open = symbol::make('(');
    symbol_g close = symbol::make(')');
    return open + arg + close;
}


symbol_p expression::space(symbol_g arg)
// ----------------------------------------------------------------------------
//   Render, putting parentheses around an argument
// ----------------------------------------------------------------------------
{
    return symbol::make(' ') + arg;
}


symbol_p expression::render(uint depth, int &precedence, bool editing)
// ----------------------------------------------------------------------------
//   Render an object as a symbol at a given precedence
// ----------------------------------------------------------------------------
{
    if (rt.depth() <= depth)
    {
        record(expression_error, "Rendering at depth %u with stack depth %u",
               depth, rt.depth());
        return nullptr;
    }

    if (object_g obj = rt.pop())
    {
        uint arity = obj->arity();
        if (rt.depth() < depth + arity)
        {
            if (rt.depth() != depth)
            {
                record(expression_error,
                       "Rendering %s arity %u at depth %u with stack depth %u",
                       name(obj->type()), arity, depth, rt.depth());
                return nullptr;
            }
            arity = 0;
        }
        switch(arity)
        {
        case 0:
            // Symbols and other non-algebraics, e.g. numbers
            precedence = obj->precedence();
            if (precedence == precedence::NONE ||
                (!editing && obj->type() == ID_unit))
                precedence = precedence::SYMBOL;
            if (obj->type() == ID_symbol)
                return symbol_p(object_p(obj));
            return obj->as_symbol(editing);

        case 1:
        {
            int      argp = 0;
            id       oid  = obj->type();
            symbol_g fn   = obj->as_symbol(editing);
            symbol_g arg  = render(depth, argp, editing);
            int      maxp =
                oid == ID_neg ? precedence::FUNCTION : precedence::SYMBOL;
            if (argp < maxp)
                arg = parentheses(arg);
            precedence = precedence::FUNCTION;
            switch(oid)
            {
            case ID_sq:
                precedence = precedence::FUNCTION_POWER;
                return arg + symbol::make("²");
            case ID_cubed:
                precedence = precedence::FUNCTION_POWER;
                return arg + symbol::make("³");
            case ID_neg:
                precedence = precedence::ADDITIVE;
                return symbol::make('-') + arg;
            case ID_fact:
                precedence = precedence::SYMBOL;
                return arg + symbol::make("!");
            case ID_inv:
                precedence = precedence::FUNCTION_POWER;
                return arg + symbol::make("⁻¹");
            default:
                break;
            }
            if (argp >= precedence::FUNCTION &&
                argp != precedence::FUNCTION_POWER)
                arg = space(arg);
            return fn + arg;
        }
        break;
        case 2:
        {
            int      lprec = 0, rprec = 0;
            symbol_g op   = obj->as_symbol(editing);
            symbol_g rtxt = render(depth, rprec, editing);
            symbol_g ltxt = render(depth, lprec, editing);
            int      prec = obj->precedence();
            id       oid  = obj->type();
            if (oid == ID_Derivative || oid == ID_Primitive)
            {
                symbol_g arg = parentheses(ltxt);
                precedence   = precedence::FUNCTION;
                if (oid == ID_Primitive)
                    op = symbol::make("∫");
                return op + rtxt + arg;
            }
            else if (prec != precedence::FUNCTION &&
                     prec != precedence::NONE)
            {
                if (op->is_alpha())
                {
                    op = symbol::make(' ') + op;
                    op = op + symbol::make(' ');
                }
                if (lprec < prec)
                    ltxt = parentheses(ltxt);
                if (rprec <= prec)
                    rtxt = parentheses(rtxt);
                precedence = prec;
                return ltxt + op + rtxt;
            }
            else
            {
                if (oid == ID_xroot)
                    std::swap(ltxt, rtxt);
                symbol_g arg = ltxt + symbol::make(';') + rtxt;
                arg = parentheses(arg);
                precedence = precedence::FUNCTION;
                return op + arg;
            }
        }
        break;
        default:
        {
            symbol_g op = obj->as_symbol(editing);
            symbol_g args = nullptr;
            for (uint a = 0; a < arity; a++)
            {
                int prec = 0;
                symbol_g arg = render(depth, prec, editing);
                if (a)
                    args = arg + symbol::make(';') + args;
                else
                    args = arg;
            }
            args = parentheses(args);
            precedence = precedence::FUNCTION;
            return op + args;
        }
        }
    }

    return nullptr;
}


RENDER_BODY(expression)
// ----------------------------------------------------------------------------
//   Render the equation
// ----------------------------------------------------------------------------
{
    return render(o, r, !r.expression());
}


size_t expression::render(const expression *o, renderer &r, bool quoted)
// ----------------------------------------------------------------------------
//  Internal rendering code
// ----------------------------------------------------------------------------
//   1 2 3 5 * + - 2 3 * +
{
    size_t depth   = rt.depth();
    bool   funcall = o->type() == ID_funcall;

    // First push all things so that we have the outermost operators first
    if (!o->expand_without_size())
        return 0;

    int      prec   = 0;
    symbol_g result = render(depth, prec, r.editing());
    if (!result)
        return 0;
    if (quoted)
        r.put('\'');

    size_t len = 0;
    utf8   txt = result->value(&len);
    r.put(txt, len);
    bool more = rt.depth() > depth;
    if (more)
    {
        if (funcall)
        {
            symbol_g args = nullptr;
            symbol_g sep  = symbol::make(";");
            while (more)
            {
                int aprec = 0;
                symbol_g arg = render(depth, aprec, r.editing());
                if (!arg)
                    return 0;
                more = rt.depth() > depth;
                if (args)
                    args = arg + sep + args;
                else
                    args = arg;
            }

            r.put('(');
            txt = args->value(&len);
            r.put(txt, len);
            r.put(')');
        }
        else
        {
            size_t remove = rt.depth() - depth;
            record(expression_error, "Malformed equation, %u removed", remove);
            rt.drop(remove);
        }
    }
    if (quoted)
        r.put('\'');
    return r.size();
}


object_p expression::quoted(id ty) const
// ----------------------------------------------------------------------------
//   If an equation contains a single symbol, return that
// ----------------------------------------------------------------------------
{
    byte  *p = (byte *) payload();
    size_t size = leb128<size_t>(p);
    object_p first = (object_p) p;
    if ((ty == ID_object || first->type() == ty) && first->size() == size)
        return first;
    return nullptr;
}


size_t expression::size_in_expression(object_p obj)
// ----------------------------------------------------------------------------
//   Size of an object in an equation
// ----------------------------------------------------------------------------
//   Inside an equation object, equations are reduced to their payload
{
    if (obj->type() == ID_expression)
        return expression_p(obj)->length();
    return obj->size();
}


expression::expression(id type, algebraic_r arg)
// ----------------------------------------------------------------------------
//   Build an equation object from an object
// ----------------------------------------------------------------------------
    : program(type, nullptr, 0)
{
    byte *p = (byte *) payload();

    // Compute the size of the program
    size_t size =  size_in_expression(arg);

    // Write the size of the program
    p = leb128(p, size);

    // Write the arguments
    size_t objsize = 0;
    byte_p objptr = nullptr;
    if (expression_p eq = arg->as<expression>())
    {
        objptr = eq->value(&objsize);
    }
    else
    {
        objsize = arg->size();
        objptr = byte_p(arg);
    }
    memmove(p, objptr, objsize);
    p += objsize;
}


size_t expression::required_memory(id type, algebraic_r arg)
// ----------------------------------------------------------------------------
//   Size of an equation object from an object
// ----------------------------------------------------------------------------
{
    size_t size = size_in_expression(arg);
    size += leb128size(size);
    size += leb128size(type);
    return size;
}


expression::expression(id type, id op, algebraic_r arg)
// ----------------------------------------------------------------------------
//   Build an equation from one argument
// ----------------------------------------------------------------------------
    : program(type, nullptr, 0)
{
    byte *p = (byte *) payload();

    // Compute the size of the program
    size_t size =  leb128size(op) + size_in_expression(arg);

    // Write the size of the program
    p = leb128(p, size);

    // Write the arguments
    size_t objsize = 0;
    byte_p objptr = nullptr;
    if (expression_p eq = arg->as<expression>())
    {
        objptr = eq->value(&objsize);
    }
    else
    {
        objsize = arg->size();
        objptr = byte_p(arg);
    }
    memmove(p, objptr, objsize);
    p += objsize;

    // Write the opcode
    p = leb128(p, op);
}


size_t expression::required_memory(id type, id op, algebraic_r arg)
// ----------------------------------------------------------------------------
//   Size of an equation object with one argument
// ----------------------------------------------------------------------------
{
    size_t size = leb128size(op) + size_in_expression(arg);
    size += leb128size(size);
    size += leb128size(type);
    return size;
}


expression::expression(id type, id op, algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Build an equation from two arguments
// ----------------------------------------------------------------------------
    : program(type, nullptr, 0)
{
    byte *p = (byte *) payload();

    // Compute the size of the program
    size_t size =  leb128size(op) + size_in_expression(x) + size_in_expression(y);

    // Write the size of the program
    p = leb128(p, size);

    // Write the first argument
    size_t objsize = 0;
    byte_p objptr = nullptr;
    if (expression_p eq = x->as<expression>())
    {
        objptr = eq->value(&objsize);
    }
    else
    {
        objsize = x->size();
        objptr = byte_p(x);
    }
    memmove(p, objptr, objsize);
    p += objsize;

    // Write the second argument
    if (expression_p eq = y->as<expression>())
    {
        objptr = eq->value(&objsize);
    }
    else
    {
        objsize = y->size();
        objptr = byte_p(y);
    }
    memmove(p, objptr, objsize);
    p += objsize;

    // Write the opcode
    p = leb128(p, op);
}


size_t expression::required_memory(id type, id op, algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Size of an equation object with one argument
// ----------------------------------------------------------------------------
{
    size_t size = leb128size(op)+size_in_expression(x)+size_in_expression(y);
    size += leb128size(size);
    size += leb128size(type);
    return size;
}


expression::expression(id type, id op,
                       algebraic_g args[], uint arity, bool wrap)
// ----------------------------------------------------------------------------
//   Build an equation with 'arity' arguments
// ----------------------------------------------------------------------------
    : program(type, nullptr, 0)
{
    byte *p = (byte *) payload();

    // Compute the size of the program
    size_t size =  leb128size(op);
    for (uint a = 0; a < arity; a++)
        size += wrap ? args[a]->size() : size_in_expression(args[a]);

    // Write the size of the program
    p = leb128(p, size);

    // Write the arguments
    size_t objsize = 0;
    byte_p objptr = nullptr;
    for (uint a = 0; a < arity; a++)
    {
        algebraic_p arg = args[arity + ~a];
        expression_p eq = wrap ? nullptr : arg->as<expression>();
        if (eq)
        {
            objptr = eq->value(&objsize);
        }
        else
        {
            objsize = arg->size();
            objptr = byte_p(arg);
        }
        memmove(p, objptr, objsize);
        p += objsize;
    }

    // Write the opcode
    p = leb128(p, op);
}


size_t expression::required_memory(id type, id op,
                                   algebraic_g args[], uint arity, bool wrap)
// ----------------------------------------------------------------------------
//   Size of an equation object with 'arity' arguments
// ----------------------------------------------------------------------------
{
    size_t size = leb128size(op);
    for (uint a = 0; a < arity; a++)
        size += wrap ? args[a]->size() : size_in_expression(args[a]);
    size += leb128size(size);
    size += leb128size(type);
    return size;
}



// ============================================================================
//
//   Equation rewrite engine
//
// ============================================================================
//
//   The equation rewrite engine works by expanding equation objects on
//   the stack, and matching them step by step.
//
//   When a symbol is encountered, it is recorded in locals as a pair
//   of entries, one for the symbol, one for its value.
//   If a symbol is seen twice, it must match exactly for the rule to match.
//   For example, a pattern like `X - X` will match if the two 'X' are 'same'
//
//   During rewrite, the stack is used to build three arrays, each being
//   the exploded content of the respective equation
//   - The 'from' pattern       [from..from+fromsz]
//   - The 'eq' value           [eq  ..eq+eqsz]
//
//   Because of the order in which RPL expressions are stored, the first items
//   in the array correspond to the innermost portions, so that scanning
//   starting with the first one, we do a top-down search (i.e. ↓MATCH)
//
//   eq:        sin(a+3) - cos(a+3)             a 3 + sin a 3 + cos -
//   match:     sin x    - cos x                    x sin     x cos -
//
//   Names of wildcards have a special role based on the initial letter:
//   - a, b, c: Constant values (numbers), i.e. is_real returns true
//              c must not be zero when matching, is numbered when generating
//   - d, e, f: Expressions (non-constant), i.e. is_real returns false
//   - i, j   : Positive integer values,i.e. type is ID_integer
//   - k      : Non-zero positive integer values,i.e. type is ID_integer
//   - l      : Linear function of independent variable,
//              sets 'a' and 'b'in output for a*x+b
//   - n, o   : An expression containing the independent variable
//   - p, q, r: An expression not containing the independent variable
//              r must not be zero
//   - s, t   : Symbols, i.e. is_symbol returns true, primed in derivative
//   - u, v, w: Unique sub-expressions, i.e. u!=v, v!=w, u!=w
//   - x, y, z: Arbitrary expression, can be identical to one another
//
//   Lowercase names must be sorted, i.e. x<=y and u<v.

static object_p grab_arguments(size_t &eq, size_t &eqsz)
// ----------------------------------------------------------------------------
//   Fetch an argument using the arity to know how many things to use
// ----------------------------------------------------------------------------
{
    size_t   len   = 0;
    size_t   arity = 1;
    scribble scr;
    while (arity && len < eqsz)
    {
        object_p obj = rt.stack(eq + len);
        arity--;
        arity += obj->arity();
        len++;
    }
    if (arity)
    {
        record(expression, "Argument gets %u beyond size %u", arity, eqsz);
        return nullptr;
    }

    size_t sz = len;
    symbol_p sym = nullptr;
    if (symbol_g *ref = expression::independent)
        if (!expression::contains_independent_variable)
            sym = *ref;

    if (sz == 1)
    {
        object_p obj = rt.stack(eq);
        if (sym && sym->found_in(obj))
            expression::contains_independent_variable = true;
        eq += sz;
        eqsz -= sz;
        return obj;
    }

    if (sym)
    {
        while (len--)
        {
            object_p obj = rt.stack(eq + len);
            if (sym->found_in(obj))
                expression::contains_independent_variable = true;
            if (!rt.append(obj))
                return nullptr;
        }
    }
    else
    {
        while (len--)
        {
            object_p obj = rt.stack(eq + len);
            if (!rt.append(obj))
                return nullptr;
        }
    }
    eq += sz;
    eqsz -= sz;
    list_p a = list::make(object::ID_expression, scr.scratch(), scr.growth());
    return a;
}


static inline char wildcard_category(symbol_p symbol)
// ----------------------------------------------------------------------------
//  The wildcard category is defined by the first letter in the name
// ----------------------------------------------------------------------------
{
    uint idx   = 1 + Settings.ExplicitWildcards();
    char first = object::payload(symbol)[idx];
    return first;
}


static inline bool must_be(char first, char low, char high)
// ----------------------------------------------------------------------------
//   Check a convention about a symbol
// ----------------------------------------------------------------------------
{
    return first >= low && first <= high;
}


static inline bool must_be(char first, cstring list)
// ----------------------------------------------------------------------------
//   Check a convention about a symbol
// ----------------------------------------------------------------------------
{
    return strchr(list, first);
}


static inline bool must_be_constant(char first)
// ----------------------------------------------------------------------------
//   Convention for naming numerical constants in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, 'a', 'c');
}


static inline bool must_be_non_constant(char first)
// ----------------------------------------------------------------------------
//   Convention for naming non-numerical constants in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, 'd', 'f');
}


static inline bool must_be_integer(char first)
// ----------------------------------------------------------------------------
//   Convention for naming integers in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, 'i', 'k');
}


static inline bool must_be_nonzero(char first)
// ----------------------------------------------------------------------------
//   Convention for naming non-zero integers in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, "kcr");
}


static inline bool must_be_symbol(char first)
// ----------------------------------------------------------------------------
//   Convention for identifying symbols in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, 's', 't');
}


static inline bool must_be_unique(char first)
// ----------------------------------------------------------------------------
//   Convention for naming unique terms in rewrite rules
// ----------------------------------------------------------------------------
{
    return must_be(first, 'u', 'w');
}


static inline bool must_contain_the_independent_variable(char first)
// ----------------------------------------------------------------------------
//   Convention for naming an expression that contains independent variable
// ----------------------------------------------------------------------------
{
    return must_be(first, 'n', 'o');
}


static inline bool must_be_the_independent_variable(char first)
// ----------------------------------------------------------------------------
//   Convention for naming an expression that contains independent variable
// ----------------------------------------------------------------------------
{
    return must_be(first, '=', '=');
}


static inline bool must_not_contain_the_independent_variable(char first)
// ----------------------------------------------------------------------------
//   Convention for naming an expression that does not contain independent var
// ----------------------------------------------------------------------------
{
    return must_be(first, 'p', 'r');
}


static inline bool must_be_linear_function_of_independent_variable(char first)
// ----------------------------------------------------------------------------
//   Matches a linear function of the independent variable
// ----------------------------------------------------------------------------
{
    return first == 'l';
}


static inline char some_index(char first)
// ----------------------------------------------------------------------------
//   Convention for naming a constant when generating non-principal solutions
// ----------------------------------------------------------------------------
{
    switch(first)
    {
        // See definitions of intk, natk and signk below
    case '#':   return 'i';
    case '+':   return 'n';
    case '-':   return 's';
    case 'c':   return 'C';
    case '@':   return '@';     // Pi
    case '!':   return '!';     // i
    case '=':   return '=';     // Current independent variable
    default: return 0;
    }
}


static bool must_be_sorted(char first)
// ----------------------------------------------------------------------------
//   Convention for naming terms in rewrite rules that need to be sorted
// ----------------------------------------------------------------------------
{
    return first >= 'a' && first <= 'z';
}


static size_t check_match(size_t eq, size_t eqsz,
                          size_t from, size_t fromsz)
// ----------------------------------------------------------------------------
//   Check for structure match between eq and from
// ----------------------------------------------------------------------------
{
    size_t eqs = eq;
    size_t locals = rt.locals();

    while (fromsz && eqsz && !program::interrupted())
    {
        // Check what we match against
        object_g ftop = rt.stack(from);
        if (!ftop)
            return false;
        object::id fty = ftop->type();

        // Check if this this is a symbol.
        if (fty == object::ID_symbol &&
            symbol_p(+ftop)->starts_with("&") == Settings.ExplicitWildcards())
        {
            // Check if the symbol already exists
            symbol_g name    = symbol_p(+ftop);
            object_g found   = nullptr;
            size_t   symbols = rt.locals() - locals;
            for (size_t l = 0; !found && l < symbols; l += 2)
            {
                symbol_p ename = symbol_p(rt.local(l));
                if (!ename)
                    return 0;
                if (ename->is_same_as(name))
                    found = rt.local(l+1);
            }

            // Get the value matching the symbol
            expression::contains_independent_variable = false;
            ftop = grab_arguments(eq, eqsz);
            if (!ftop)
                return 0;

            if (!found)
            {
                // Check if we expect an integer value
                char wcat     = wildcard_category(name);
                char cat      = tolower(wcat);
                bool want_cst = must_be_constant(cat);
                bool want_int = must_be_integer(cat);
                bool want_var = must_be_non_constant(cat);
                if (want_cst || want_int || want_var)
                {
                    // At this point, if we have a numerical value, it was
                    // wrapped in an equation by grab_arguments.
                    size_t depth = rt.depth();
                    if (program::run(+ftop) != object::OK)
                        return 0;
                    if (rt.depth() != depth + 1)
                    {
                        if (rt.depth() > depth)
                            rt.drop(rt.depth() - depth);
                        return 0;
                    }
                    ftop = rt.pop();

                    // Check if what we get matches expectations
                    fty = ftop->type();
                    if ((want_int && fty != object::ID_integer)         ||
                        (want_cst && !object::is_real(fty))             ||
                        (want_var && object::is_real(fty))              ||
                        (must_be_nonzero(cat) && ftop->is_zero(false)))
                        return 0;
                }

                // Check if the name must be unique in the locals
                else if (must_be_unique(cat))
                {
                    for (size_t l = 0; l < symbols; l += 2)
                    {
                        object_p existing = rt.local(l+1);
                        if (!existing || existing->is_same_as(+ftop))
                            return 0;
                    }
                }
                else if (must_be_symbol(cat))
                {
                    if (!ftop->as_quoted<symbol>())
                        return 0;
                }
                else if (must_be_the_independent_variable(cat))
                {
                    bool isvar = false;
                    if (expression::independent)
                        if (symbol_p ivar = *expression::independent)
                            if (symbol_p sym = ftop->as_quoted<symbol>())
                                isvar = sym->is_same_as(ivar);
                    if (!isvar)
                        return 0;
                }
                else if (must_contain_the_independent_variable(cat))
                {
                    if (!expression::contains_independent_variable)
                        return 0;
                }
                else if (must_not_contain_the_independent_variable(cat))
                {
                    if (expression::contains_independent_variable)
                        return 0;
                    if (must_be_nonzero(cat) && ftop->is_zero(false))
                        return 0;
                }
                else if (must_be_linear_function_of_independent_variable(cat))
                {
                    if (!expression::contains_independent_variable)
                        return 0;
                    bool isok = false;
                    algebraic_g a, b;
                    if (expression::independent)
                    {
                        if (symbol_g ivar = *expression::independent)
                        {
                            if (symbol_p s = ftop->as_quoted<symbol>())
                            {
                                isok = s->is_same_as(ivar);
                                if (isok)
                                {
                                    a = integer::make(1);
                                    b = integer::make(0);
                                }
                            }
                            expression_p x = expression::get(ftop);
                            if (x && x->is_linear(ivar, a, b))
                                isok = true;
                        }
                    }
                    if (!isok)
                        return 0;
                    symbol_g an = symbol::make("A");
                    symbol_g bn = symbol::make("B");
                    if (!an || !bn ||
                        !rt.push(+an) || !rt.push(+a) ||
                        !rt.push(+bn) || !rt.push(+b) ||
                        !rt.locals(4))
                        return 0;
                }

                // Check if things must be sorted
                if (must_be_sorted(wcat))
                {
                    for (size_t l = 0; l < symbols; l += 2)
                    {
                        symbol_p ename = symbol_p(rt.local(l));
                        char ecat = wildcard_category(ename);
                        if (must_be_sorted(ecat))
                        {
                            object_p existing = rt.local(l+1);
                            if (!existing)
                                return 0;

                            // Check if order of names and values match
                            int cmpnames = name->compare_to(ename);
                            int cmpvals = ftop->compare_to(existing);
                            if (cmpnames * cmpvals < 0)
                                return 0;
                        }
                    }
                }

                // Grab the parameter that corresponds and store it
                if (!rt.push(+name) || !rt.push(+ftop) || !rt.locals(2))
                    return 0;
            }
            else
            {
                // If there is a mismatch, rewrite fails
                if (!found->is_same_as(ftop))
                    return 0;
            }
        }
        else if (integer_g itop = ftop->as<integer>())
        {
            // If we have an integer in the pattern, need to evaluate arg
            ftop = grab_arguments(eq, eqsz);
            if (!ftop)
                return 0;

            settings::SaveNumericalResults snr(true);
            settings::SaveNumericalConstants snc(true);
            settings::SaveAutoSimplify sas(true);
            size_t depth = rt.depth();
            if (program::run(+ftop) != object::OK)
                return 0;
            if (rt.depth() != depth + 1)
            {
                if (rt.depth() > depth)
                    rt.drop(rt.depth() - depth);
                return 0;
            }
            ftop = rt.pop();
            integer_g fval = ftop->as_quoted<integer>();
            if (!fval || fval->value<ularge>() != itop->value<ularge>())
                return 0;
        }
        else
        {
            // Not a symbol. Check if this exists at all
            object_p top = rt.stack(eq);
            if (!top)
                return 0;

            // If it's a function call, need to match its arguemnts
            if (fty == object::ID_funcall)
            {
                if (top->type() != object::ID_funcall)
                    return 0;
                if (!expression::funcall_match)
                    return 0;
                funcall_p fcftop = funcall_p(+ftop);
                funcall_p fctop = funcall_p(+top);
                size_t fcmatch = expression::funcall_match(fcftop, fctop);
                if (!fcmatch)
                    return 0;
            }
            // If not a symbol, we need an exact match
            else if (!top->is_same_as(ftop))
            {
                return 0;
            }
            eq++;
            eqsz--;
        }
        from++;
        fromsz--;
    }

    // If there is a leftover in `from`, then this is a mismatch
    if (fromsz)
        return false;

    // Otherwise, we covered the whole 'from' pattern, we have a match
    // Return size matched in source equation
    return eq - eqs;
}


static algebraic_p build_expr(expression_p eqin,
                              uint         eqst,
                              expression_r to,
                              uint         matchsz,
                              uint         locals,
                              uint        &rwcount,
                              bool        &replaced)
// ----------------------------------------------------------------------------
//   Build an expression by rewriting
// ----------------------------------------------------------------------------
{
    scribble     scr;
    expression_g eq       = eqin;
    size_t       where    = 0;
    object::id   eqty     = to->type();

    // Copy from the original
    for (expression::iterator it = eq->begin(); it != eq->end(); ++it)
    {
        ASSERT(*it);
        if (where < eqst || where >= eqst + matchsz)
        {
            // Copy from source equation directly
            object_p obj = *it;
            if (!rt.append(obj))
                return nullptr;
        }
        else if (!replaced)
        {
            // Insert a version of 'to' where symbols are replaced
            uint     nvars      = 0;
            uint     nconstants = 0;
            size_t   begin      = scr.growth();
            for (object_p tobj : *to)
            {
                if (tobj->type() == object::ID_symbol)
                {
                    // Check if we find the matching pattern in local
                    symbol_p name  = symbol_p(tobj);
                    char     cat   = tolower(wildcard_category(name));
                    object_p found = nullptr;

                    if (char c = some_index(cat))
                    {
                        if (c == '@')
                        {
                            found = constant::lookup("π");
                            nvars++;
                            nconstants++;
                        }
                        else if (c == '!')
                        {
                            found = constant::lookup("ⅈ");
                            nvars++;
                            nconstants++;
                        }
                        else if (c == '=')
                        {
                            found = expression::independent
                                ? *expression::independent
                                : nullptr;
                            nvars++;
                        }
                        else
                        {
                            char buf[24];
                            snprintf(buf, sizeof(buf),
                                     "%c%u", c, ++expression::constant_index);
                            found = symbol::make(buf);
                            nvars++;
                        }
                    }
                    else
                    {
                        size_t   symbols = rt.locals() - locals;
                        for (size_t l = 0; !found && l < symbols; l += 2)
                        {
                            symbol_p existing = symbol_p(rt.local(l));
                            if (!existing)
                                continue;
                            if (existing->is_same_as(name))
                                found = rt.local(l+1);
                        }
                        nvars++;
                    }

                    if (found)
                    {
                        if (must_be_integer(cat)|| must_be_constant(cat))
                            nconstants++;
                        tobj = found;

                    }
                }

                // Only copy the payload of equations
                size_t tobjsize = tobj->size();
                object::id tty = tobj->type();
                if (tty == object::ID_funcall)
                {
                    object_p srcobj = *it;
                    if (srcobj->type() != tty || !expression::funcall_build)
                        return nullptr;

                    funcall_p src = funcall_p(srcobj);
                    funcall_p repl = funcall_p(tobj);
                    algebraic_p a = expression::funcall_build(src, repl);
                    if (!a)
                        return nullptr;
                    tobj = +a;
                    tobjsize = tobj->size();
                    tty = tobj->type();
                }
                if (tty == object::ID_expression)
                {
                    tobj = expression_p(tobj)->objects(&tobjsize);
                }
                if (object::is_real(tty))
                {
                    nvars++;
                    nconstants++;
                }
                if (!rt.append(tobjsize, byte_p(tobj)))
                    return nullptr;
            }

            // Check if we need to compute a sub-expression, e.g 3-1 = 2
            if (nvars >= 2 && nvars == nconstants)
            {
                byte_p start = scr.scratch() + begin;
                size_t len = scr.growth() - begin;
                expression_g sub = expression_p(list::make(eqty, start, len));
                if (!sub)
                    return nullptr;
                algebraic_g value = sub->evaluate();
                if (!value)
                    return nullptr;
                rt.free(len);
                if (!rt.append(value))
                    return nullptr;
            }

            replaced = true;
            rwcount++;
        }
        where++;
    }

    // Restart anew with replaced equation
    eq = expression_p(list::make(eqty, scr.scratch(), scr.growth()));
    return +eq;
}


static size_t check_match(size_t eq, size_t eqsz,
                          size_t from, size_t fromsz,
                          expression_r cond, uint locals)
// ----------------------------------------------------------------------------
//   Check if the condition applies
// ----------------------------------------------------------------------------
{
    size_t match = check_match(eq, eqsz, from, fromsz);
    if (!match || !cond)
        return match;

    uint condrw = 0;
    bool condrepl = false;
    algebraic_g cval = build_expr(cond, 0, cond, ~0U,
                                  locals, condrw, condrepl);
    if (!cval)
        return 0;
    cval = cval->evaluate();
    if (!cval)
        return 0;
    int rc = cval->as_truth(false);
    if (rc <= 0)                // If this does not evaluate correctly, fail
        match = 0;
    return match;
}


expression_p expression::rewrite(expression_r from,
                                 expression_r to,
                                 expression_r cond,
                                 uint        *count,
                                 bool         down) const
// ----------------------------------------------------------------------------
//   If we match pattern in `from`, then rewrite using pattern in `to`
// ----------------------------------------------------------------------------
//   For example, if this equation is `3 + sin(X + Y)`, from is `A + B` and
//   to is `B + A`, then the output will be `sin(Y + X) + 3`.
//
{
    // Remember the current stack depth and locals
    size_t       locals   = rt.locals();
    size_t       depth    = rt.depth();

    // Need a GC pointer since stack operations may move us
    expression_g eq       = this;
    expression_g saved    = this;

    // Information about part we replace
    bool         replaced = false;
    size_t       matchsz  = 0;
    uint         rewrites = Settings.MaxRewrites();
    uint         rwcount  = 0;

    record(rewrites, "Rewrite %t applying %t->%t cond %t",
           +eq, +from, +to, +cond);

    settings::PrepareForProgramEvaluation willRunPrograms;

    // Loop while there are replacements found
    do
    {
        // Location of expanded equation
        size_t eqsz = 0, fromsz = 0;
        size_t eqst = 0, fromst = 0;

        replaced = false;

        // Expand 'from' on the stack and remember where it starts
        if (!from->expand_without_size())
            goto err;
        fromsz = rt.depth() - depth;

        // Expand this equation on the stack, and remember where it starts
        if (!eq->expand_without_size())
            goto err;
        eqsz = rt.depth() - depth - fromsz;

        // Keep checking sub-expressions until we find a match
        size_t eqlen = eqsz;
        fromst = eqst + eqsz;
        if (down)
        {
            // Check if there is a match in sub-equations going down
            for (eqsz = eqlen; eqsz; eqst++, eqsz--)
            {
                matchsz = check_match(eqst, eqsz, fromst, fromsz,
                                      cond, locals);
                if (matchsz || interrupted())
                    break;
            }
        }
        else
        {
            size_t eqstart = eqst;
            // Check if there is a match in sub-equations going up
            for (eqsz = fromsz; eqsz <= eqlen; eqsz++)
            {
                for (eqst = eqstart; eqst + eqsz <= eqlen; eqst++)
                {
                    matchsz = check_match(eqst, eqsz, fromst, fromsz,
                                          cond, locals);
                    if (matchsz || interrupted())
                        break;
                }
                if (matchsz || interrupted())
                    break;
            }
        }
        if (interrupted())
            break;

        // We don't need the on-stack copies of 'eq' and 'to' anymore
        ASSERT(rt.depth() >= depth);
        rt.drop(rt.depth() - depth);

        // If we matched a sub-equation, perform replacement
        if (matchsz)
        {
            // We matched from the back of the equation object
            eqst = eqlen - matchsz - eqst;

            algebraic_g eqa = build_expr(eq, eqst, to, matchsz, locals,
                                         rwcount, replaced);
            if (!eqa)
                goto err;

            // Wrap result in expression if needed
            eq = eqa->as<expression>();
            if (!eq)
                eq = expression::make(eqa);

            // Drop the local names, we will recreate them on next match
            rt.unlocals(rt.locals() - locals);

            // Check if we are looping forever
            if (rewrites-- == 0)
            {
                rt.too_many_rewrites_error();
                goto err;
            }
        } // matchz
    } while (replaced && Settings.FinalAlgebraResults() && !interrupted());

    if (count)
        *count += rwcount;

err:
    ASSERT(rt.depth() >= depth);
    rt.drop(rt.depth() - depth);
    rt.unlocals(rt.locals() - locals);

    record(rewrites, "%+s rewritten as %t", rwcount ? "Was" : "Not", +eq);
    if (rwcount)
        record(rewrites_done,
               "%t rewritten as %t applying %t->%t with condition %t %u times",
               +saved, +eq, +from, +to, +cond, rwcount);
    return eq;
}


algebraic_p expression::factor_out(algebraic_g expr,
                                   algebraic_g factor,
                                   algebraic_g &scale,
                                   algebraic_g &exponent)
// ----------------------------------------------------------------------------
//   Factor out the given factor from the equation
// ----------------------------------------------------------------------------
//   Given expr=A*X*(X*B)^3/(X*C)^6,
//   returns X^(-2) * (A*B^3/C^6), with factor=(A*B^3/C^6) and exponent=-2
{
    // Quick bail out in case of error down the line
    if (!expr || !factor)
        return nullptr;

    // Default is 1 * X ^ 0 * (rest-of-expr)
    scale = integer::make(1);
    exponent = integer::make(0);

    expression_g eq = expr->as<expression>();
    if (eq)
    {
        // Case where we have a single name or a constant, e.g. m or 1
        if (object_p inner = eq->quoted())
        {
            if (inner->is_algebraic())
            {
                expr = algebraic_p(inner);
                eq = nullptr;
            }
        }
    }

    // Check for anything that is not an equation
    if (!eq)
    {
        // We have a single element in the equation
        if (expr->is_same_as(factor))
        {
            // Factoring X as 1 * (1 * X ^ 1)
            exponent = scale;
            return factor;
        }

        // Factoring Y as Y * X ^ 0)
        scale = expr;
        return expr;
    }


    // Loop on all items in the equation, factoring out as we go
    algebraic_g x, y, xs, xe, ys, ye;
    algebraic_g one = integer::make(1);
    for (object_p obj : *eq)
    {
        id ty = obj->type();

        switch (ty)
        {
        case ID_multiply:
            x = algebraic_p(rt.pop());
            y = algebraic_p(rt.pop());
            y = factor_out(y, factor, ys, ye);
            x = factor_out(x, factor, xs, xe);
            scale = ys * xs;
            exponent = ye + xe;
            x = y * x;
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_divide:
            x = algebraic_p(rt.pop());
            y = algebraic_p(rt.pop());
            y = factor_out(y, factor, ys, ye);
            x = factor_out(x, factor, xs, xe);
            scale = ys / xs;
            exponent = ye - xe;
            x = y / x;
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_pow:
            x = algebraic_p(rt.pop());
            y = algebraic_p(rt.pop());
            if (!x || !x->is_real())
            {
                rt.invalid_unit_expression_error();
                return nullptr;
            }
            y = factor_out(y, factor, ys, ye);
            ye = ye * x;
            scale = pow(ys, x);
            exponent = ye;
            x = pow(y, x);
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_inv:
            x = algebraic_p(rt.pop());
            x = factor_out(x, factor, xs, xe);
            scale = inv::run(xs);
            exponent = -xe;
            x = inv::run(x);
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_sq:
            x = algebraic_p(rt.pop());
            x = factor_out(x, factor, xs, xe);
            scale = xs * xs;
            exponent = xe + xe;
            x = x * x;
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_cubed:
            x = algebraic_p(rt.pop());
            x = factor_out(x, factor, xs, xe);
            scale = xs * xs * xs;
            exponent = xe + xe + xe;
            x = x * x * x;
            if (!x|| !rt.push(+x))
                return nullptr;
            break;

        case ID_symbol:
        case ID_integer:
        case ID_neg_integer:
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
        case ID_neg:
            if (program::run(obj) != OK)
                return nullptr;
            break;

        default:
            rt.invalid_unit_expression_error();
            return nullptr;
        }
    }

    x = algebraic_p(rt.pop());
    return x;
}


algebraic_p expression::simplify_products() const
// ----------------------------------------------------------------------------
//   Simplify products, used notably to simplify units
// ----------------------------------------------------------------------------
//   Units are products and ratios of powers.
//   We rewrite that so that all terms are written at most once with the
//   corresponing (positive or negative) power
{
    // Case where we have a single name or a constant, e.g. 1_m or 1_1.
    if (object_p inner = quoted())
        if (inner->is_algebraic())
            return algebraic_p(inner);

    // Save auto-simplify and set it, and evaluate units normally
    settings::SaveAutoSimplify sas(true);
    settings::SaveNumericalResults snr(false);
    save<bool> smode(unit::mode, false);
    save<bool> sexpr(unit::factoring, true);

    // Need a GC pointer since stack operations may move us
    expression_g eq   = this;
    algebraic_g  num  = integer::make(1);
    algebraic_g  den  = integer::make(1);

    // Loop factoring out variables, until there is no variable left
    bool done = false;
    while (!done && !program::interrupted())
    {
        done = true;
        for (object_p obj : *eq)
        {
            if (symbol_g sym = obj->as<symbol>())
            {
                uint depth = rt.depth();
                algebraic_g scale, exponent;
                algebraic_g rest = factor_out(+eq, +sym, scale, exponent);
                if (!rest || !scale || !exponent)
                {
                    uint sdepth = rt.depth();
                    if (sdepth > depth)
                        rt.drop(sdepth - depth);
                    return nullptr;
                }
                if (exponent->is_negative(false))
                    den = den * pow(+sym, -exponent);
                else
                    num = num * pow(+sym, exponent);
                rest = scale;
                if (expression_p req = rest->as<expression>())
                {
                    done = req->is_same_as(eq);
                    eq = req;
                }
                else
                {
                    if (rest->is_real())
                        num = rest * num;
                    else
                        num = num * rest;
                    eq = nullptr;
                }
                break;
            }
        }

        if (done && eq)
        {
            algebraic_g rest = +eq;
            num = num * rest;
        }
    }

    num = num / den;
    return num;
}


object_p expression::outermost_operator() const
// ----------------------------------------------------------------------------
//   Return the last operator in the equation
// ----------------------------------------------------------------------------
{
    object_p result = nullptr;
    for (object_p o : *this)
        result = o;
    return result;
}



// ============================================================================
//
//   Graphic rendering
//
// ============================================================================

using pixsize  = grob::pixsize;


grob_p expression::parentheses(grapher &g, grob_g what, uint padding)
// ----------------------------------------------------------------------------
//   Render parentheses around a grob
// ----------------------------------------------------------------------------
{
    if (!what)
        return nullptr;

    pixsize inw    = what->width();
    pixsize inh    = what->height();
    pixsize prw    = inh / 8;
    pixsize rw     = inw + 2 * prw + 2 + padding;
    pixsize rh     = inh;
    pixsize hh     = rh / 2;
    pixsize hh2    = hh * hh;

    grob_g  result = g.grob(rw, rh);
    if (!result)
        return nullptr;

    // Copy inner content
    grob::surface ws = what->pixels();
    grob::surface rs = result->pixels();
    rs.fill(0, 0, rw, rh, g.background);
    rs.copy(ws, prw + padding, 0);

    // Draw the parentheses themselves
    for (coord y = 0; y < coord(hh); y++)
    {
        coord x = prw * y * y / hh2;
        pixsize pw = 1 + (prw - x) / 4;
        if (pw > 4)
            pw = 4;
        coord x1 = x + padding;
        coord x2 = x1 + pw;
        rs.fill(x1, hh+y, x2, hh+y, g.foreground);
        rs.fill(x1, hh-y, x2, hh-y, g.foreground);
        x = rw - 2 - x;
        x2 = x;
        x1 = x2 - pw;
        rs.fill(x1, hh+y, x2, hh+y, g.foreground);
        rs.fill(x1, hh-y, x2, hh-y, g.foreground);
    }

    return result;
}


grob_p expression::abs_norm(grapher &g, grob_g what, uint padding)
// ----------------------------------------------------------------------------
//   Render a norm / abs (vertical bars, like |x| )
// ----------------------------------------------------------------------------
{
    if (!what)
        return nullptr;

    pixsize inw    = what->width();
    pixsize inh    = what->height();
    pixsize asz    = 2;
    pixsize rw     = inw + 2 * (asz + 2*padding);
    pixsize rh     = inh;
    pixsize rx     = rw - asz - padding;

    grob_g result = g.grob(rw, rh);
    if (!result)
        return nullptr;

    // Copy inner content
    grob::surface ws = what->pixels();
    grob::surface rs = result->pixels();
    rs.fill(0, 0, rw, rh, g.background);
    rs.copy(ws, asz + 2*padding, 0);
    rs.fill(padding, 0, padding + asz-1, rh-3, g.foreground);
    rs.fill(rx, 0, rx+asz-1, rh-3, g.foreground);

    return result;
}


grob_p expression::root(grapher &g, grob_g what)
// ----------------------------------------------------------------------------
//  Draw a square root around the expression
// ----------------------------------------------------------------------------
{
    if (!what)
        return nullptr;

    pixsize inw    = what->width();
    pixsize inh    = what->height();
    pixsize prw    = inh / 4;
    pixsize prh    = 4;
    pixsize rw     = inw + prw + 4;
    pixsize rh     = inh + prh;

    grob_g result = g.grob(rw, rh);
    if (!result)
        return nullptr;

    // Copy inner content
    grob::surface ws = what->pixels();
    grob::surface rs = result->pixels();
    rs.fill(0, 0, rw, rh, g.background);
    rs.copy(ws, prw, prh);

    // Draw the square root itself
    rs.line(0,     rh/2 + prh, prw/2, rh-4,     2, g.foreground);
    rs.line(prw/2, rh-4,       prw-2, 0,        3, g.foreground);
    rs.line(prw-2, 0,          rw-3,  0,        2, g.foreground);
    rs.line(rw-3,  0,          rw-3,  prh,      2, g.foreground);

    g.voffset = prh / 2;

    return result;
}


grob_p expression::ratio(grapher &g, grob_g num, grob_g den)
// ----------------------------------------------------------------------------
//   Draw a ratio beween two grobs
// ----------------------------------------------------------------------------
{
    if (!num || !den)
        return nullptr;

    pixsize nw = num->width();
    pixsize nh = num->height();
    pixsize dw = den->width();
    pixsize dh = den->height();
    pixsize lh = 1 + (nh + dh) / 80;
    pixsize gw = nw > dw ? nw : dw;
    pixsize gh = nh + dh + 2 + lh;

    grob_g  result = g.grob(gw, gh);
    if (!result)
        return nullptr;

    grob::surface ns = num->pixels();
    grob::surface ds = den->pixels();
    grob::surface rs = result->pixels();

    rs.fill(0, 0, gw, gh, g.background);
    rs.copy(ns, (gw - nw) / 2, 0);
    rs.copy(ds, (gw - dw) / 2, nh + 2 + lh);
    rs.fill(0, nh, gw, nh + lh, g.foreground);

    g.voffset = 1 + coord(nh) + coord(lh) / 2 - coord(gh) / 2;

    return result;
}


grob_p expression::ratio(grapher &g, cstring num, grob_g den)
// ----------------------------------------------------------------------------
//   Ratio with a constant
// ----------------------------------------------------------------------------
{
    symbol_g nobj = symbol::make(num);
    if (!nobj)
        return nullptr;
    grob_g n = object::do_graph(nobj, g);
    return ratio(g, n, den);
}


grob_p expression::infix(grapher &g,
                         coord vx, grob_g x,
                         coord vs, cstring sep,
                         coord vy, grob_g y)
// ----------------------------------------------------------------------------
//   Draw an infix between two grobs, e.g. "A+B"
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    // Render the separator
    symbol_g sobj = symbol::make(sep);
    if (!sobj)
        return nullptr;
    grob_g s = object::do_graph(sobj, g);
    return infix(g, vx, x, vs, s, vy, y);
}


grob_p expression::infix(grapher &g,
                         coord vx, grob_g x,
                         coord vs, grob_g s,
                         coord vy, grob_g y)
// ----------------------------------------------------------------------------
//  Draw an infix between two grobs with a third one in the middle
// ----------------------------------------------------------------------------
{
    if (!x || !y || !s)
        return nullptr;

    pixsize xw = x->width();
    pixsize xh = x->height();
    pixsize yw = y->width();
    pixsize yh = y->height();
    pixsize sw = s->width();
    pixsize sh = s->height();
    pixsize gw = xw + sw + yw;

    coord   xt = -vx - xh / 2;
    coord   yt = -vy - yh / 2;
    coord   st = -vs - sh / 2;
    coord   xb = xt + xh - 1;
    coord   yb = yt + yh - 1;
    coord   sb = st + sh - 1;

    coord t = xt;
    if (t > yt)
        t = yt;
    if (t > st)
        t = st;

    coord b = xb;
    if (b < yb)
        b = yb;
    if (b < sb)
        b = sb;

    pixsize gh = b - t + 1;

    grob_g result = g.grob(gw, gh);
    if (!result)
        return nullptr;

    grob::surface xs = x->pixels();
    grob::surface ys = y->pixels();
    grob::surface ss = s->pixels();
    grob::surface rs = result->pixels();

    coord xx = 0;
    coord sx = xx + xw;
    coord yx = sx + sw;

    rs.fill(0, 0, gw, gh, g.background);
    rs.copy(xs, xx, xt - t);
    rs.copy(ss, sx, st - t);
    rs.copy(ys, yx, yt - t);

    g.voffset = st - t + coord(sh)/2 - coord(gh)/2;

    return result;
}


grob_p expression::suscript(grapher &g,
                            coord vx, grob_g x,
                            coord vy, grob_g y,
                            int dir, bool alignleft)
// ----------------------------------------------------------------------------
//  Position grob y on the right of x
// ----------------------------------------------------------------------------
//  dir can be 0 (centered), 1 (superscript) or -1 (subscript)
{
    if (!x || !y)
        return nullptr;

    pixsize xw     = x->width();
    pixsize xh     = x->height();
    pixsize yw     = y->width();
    pixsize yh     = y->height();
    pixsize gw     = xw + yw;

    coord   voff   = (1 - dir) * xh / 2 + vx;
    coord   xt     = -vx - xh / 2;
    coord   yt     = -vy - yh / 2 + xt + voff;
    coord   xb     = xt + xh - 1;
    coord   yb     = yt + yh - 1;
    coord   t      = xt < yt ? xt : yt;
    coord   b      = xb > yb ? xb : yb;

    pixsize gh = b - t + 1;
    grob_g result = g.grob(gw, gh);
    if (!result)
        return nullptr;

    grob::surface xs = x->pixels();
    grob::surface ys = y->pixels();
    grob::surface rs = result->pixels();

    rs.fill(0, 0, gw, gh, g.background);
    rs.copy(xs, 0,  xt - t);
    rs.copy(ys, xw, yt - t);
    if (alignleft)
        g.voffset = xt - t + coord(xh)/2 - coord(gh)/2;
    else
        g.voffset = yt - t + coord(yh)/2 - coord(gh)/2;

    return result;
}


grob_p expression::suscript(grapher &g,
                            coord vx, grob_g x,
                            coord vy, cstring exp,
                            int dir, bool alignleft)
// ----------------------------------------------------------------------------
//   Position a superscript
// ----------------------------------------------------------------------------
{
    symbol_g yobj = symbol::make(exp);
    if (!yobj)
        return nullptr;
    settings::font_id savef = g.font;
    g.reduce_font();
    grob_g y = object::do_graph(yobj, g);
    g.font = savef;
    return suscript(g, vx, x, vy, y, dir, alignleft);
}


grob_p expression::suscript(grapher &g,
                            coord vx, cstring xstr,
                            coord vy, grob_g y,
                            int dir, bool alignleft)
// ----------------------------------------------------------------------------
//   Position a superscript
// ----------------------------------------------------------------------------
{
    symbol_g xobj = symbol::make(xstr);
    if (!xobj)
        return nullptr;
    grob_g x = object::do_graph(xobj, g);
    return suscript(g, vx, x, vy, y, dir, alignleft);
}


grob_p expression::prefix(grapher &g,
                          coord vx, grob_g x,
                          coord vy, grob_g y, int dir)
// ----------------------------------------------------------------------------
//   Draw a prefix, e.g. "sin"
// ----------------------------------------------------------------------------
{
    return suscript(g, vx, x, vy, y, dir);
}


grob_p expression::prefix(grapher &g,
                          coord vx, cstring pfx,
                          coord vy, grob_g y, int dir)
// ----------------------------------------------------------------------------
//   Draw a prefix, e.g. "-"
// ----------------------------------------------------------------------------
{
    symbol_g xobj = symbol::make(pfx);
    if (!xobj)
        return nullptr;
    grob_g x = object::do_graph(xobj, g);
    return suscript(g, vx, x, vy, y, dir);
}


grob_p expression::sumprod(grapher &g, id oid, sumprod_fn shape,
                           coord vi, grob_g index,
                           coord vf, grob_g first,
                           coord vl, grob_g last,
                           coord ve, grob_g expr)
// ----------------------------------------------------------------------------
//  Create a sum or product expression
// ----------------------------------------------------------------------------
{
    using std::max;
    using size = blitter::size;


    if (!index || !first || !last || !expr || !shape)
        return nullptr;

    if (oid == ID_Integrate)
    {
        // Order of arguments is not identical between sum and integrate
        // sum(index;first;last;expr)
        // integrate(first;last;expr;index)
        std::swap(last, expr);          // integrate(first;last;index;expr)
        std::swap(first, last);         // integrate(first;index;last;expr)
        std::swap(index, first);        // integrate(index;first;last;expr)
    }

    auto fid = g.font;
    g.reduce_font();
    grob_g lower  = (oid == ID_Integrate
                     ? +first
                     : infix(g, vi, index, 0, "=", vf, first));
    g.font = fid;
    if (!lower)
        return nullptr;

    if (oid == ID_Integrate)
    {
        expr = infix(g, ve, expr, 0, "d", vi, index);
        ve = g.voffset;
        if (!expr)
            return nullptr;
    }
    grob::surface xs   = expr->pixels();
    size          xh   = xs.height();
    size          xw   = xs.width();
    grob_g        sign = shape(g, xh);
    if (!sign)
        return nullptr;

    size    lh     = lower->height() + sign->height() + last->height();
    size    lw     = max(max(lower->width(), sign->width()), last->width());
    size    gh     = max(lh, xh);
    size    gw     = lw + xw;

    grob_g  result = g.grob(gw, gh);
    if (!result)
        return nullptr;
    grob::surface rs = result->pixels();
    rs.fill(0, 0, gw, gh, g.background);

    grob::surface sgs = sign->pixels();
    rs.copy(sgs, (lw - sgs.width()) / 2, last->height());
    grob::surface las = last->pixels();
    rs.copy(las, (lw - las.width()) / 2, 0 + 0 * vl);
    grob::surface los = lower->pixels();
    rs.copy(los, (lw - los.width()) / 2, gh - los.height());
    coord xx = lw;
    coord xy = las.height() + sgs.height() / 2 - xs.height() / 2 - ve;
    rs.copy(xs, xx, xy);
    g.voffset = xy + (coord(xh) - coord(gh)) / 2 + ve;

    return result;
}


grob_p expression::sum(grapher &g, blitter::size h)
// ----------------------------------------------------------------------------
//   Create a 'sum' sign of height h
// ----------------------------------------------------------------------------
{
    using size    = blitter::size;

    size   w      = h * 3 / 4;
    grob_g result = g.grob(w, h);
    if (!result)
        return nullptr;

    grob::surface rs = result->pixels();
    rs.fill(0,          0,      w-1,    h-1,            g.background);

    rs.fill(0,          0,      w-1,    2,              g.foreground);
    rs.fill(w-2,        0,      w-1,    4,              g.foreground);
    rs.fill(w-2,        h-5,    w-1,    h-1,            g.foreground);
    rs.fill(0,          h-3,    w-1,    h-1,            g.foreground);
    rs.line(0,          0,    w/2-1,    h/2-1,  4,      g.foreground);
    rs.line(0,          h-3,  w/2-1,    h/2-1,  4,      g.foreground);

    return result;
}


grob_p expression::product(grapher &g, blitter::size h)
// ----------------------------------------------------------------------------
//   Create a 'product' sign of height h
// ----------------------------------------------------------------------------
{
    using size    = blitter::size;
    size   w      = h * 3 / 4;
    grob_g result = g.grob(w, h);
    if (!result)
        return nullptr;

    grob::surface rs = result->pixels();
    rs.fill(0,          0,      w-1,    h-1,    g.background);

    rs.fill(0,          0,      w-1,    2,      g.foreground);
    rs.fill(4,          0,      8,      h-1,    g.foreground);
    rs.fill(w-8,        0,      w-4,    h-1,    g.foreground);

    return result;
}


grob_p expression::integral(grapher &g, blitter::size h)
// ----------------------------------------------------------------------------
//   Create an 'integral' sign of height h
// ----------------------------------------------------------------------------
{
    using size    = blitter::size;
    size   w      = 32;
    grob_g result = g.grob(w, h);
    if (!result)
        return nullptr;

    grob::surface rs = result->pixels();
    rs.fill   (0,       0,        w-1,    h-1,          g.background);
    rs.fill   (w/2-2,   w/4,      w/2+1,  h-w/4,        g.foreground);
    rect rc = rs.clip();
    rs.clip(0, 0, w, w/4);
    rs.ellipse(w/2-1,   2,        w-5,    w/2-1,  3,    g.foreground);
    rs.clip(0, h-w/4,   w, h-1);
    rs.ellipse(4,       h-w/2,    w/2-0,  h-3,    3,    g.foreground);
    rs.clip(rc);

    return result;
}


static inline cstring mulsep()
// ----------------------------------------------------------------------------
//   Return the separator for multiplication
// ----------------------------------------------------------------------------
{
    return Settings.UseDotForMultiplication() ? "·" : "×";
}


inline expression::sumprod_fn expression::sumprod_shape(id oid)
// ----------------------------------------------------------------------------
//   Return the associated shape function
// ----------------------------------------------------------------------------
{
    return oid == ID_Sum       ? sum
         : oid == ID_Product   ? product
         : oid == ID_Integrate ? integral
                               : nullptr;
}


grob_p expression::graph(grapher &g, uint depth, int &precedence)
// ----------------------------------------------------------------------------
//   Render a single object as a graphical object
// ----------------------------------------------------------------------------
{
    if (rt.depth() <= depth)
    {
        record(expression_error, "Graphing at depth %u with stack depth %u",
               depth, rt.depth());
        return nullptr;
    }

    if (object_g obj = rt.pop())
    {
        uint arity = obj->arity();
        if (rt.depth() < depth + arity)
        {
            if (rt.depth() != depth)
            {
                record(expression_error,
                       "Graphing %s arity %u at depth %u with stack depth %u",
                       name(obj->type()), arity, depth, rt.depth());
                return nullptr;
            }
            arity = 0;
        }
        switch(arity)
        {
        case 0:
            // Symbols and other non-algebraics, e.g. numbers
            precedence = obj->precedence();
            if (precedence == precedence::NONE || obj->type() == ID_unit)
                precedence = precedence::SYMBOL;
            g.voffset = 0;
            if (expression_p expr = obj->as<expression>())
            {
                if (!expr->expand_without_size())
                    return nullptr;
                return expr->graph(g, depth, precedence);
            }
            return obj->graph(g);

        case 1:
        {
            int      argp = 0;
            id       oid  = obj->type();
            auto     savef = g.font;
            if (oid == ID_exp || oid == ID_exp10 || oid == ID_exp2)
                g.reduce_font();
            grob_g   arg  = graph(g, depth, argp);
            coord    va   = g.voffset;
            int      maxp = (oid == ID_neg
                             ? precedence::MULTIPLICATIVE
                             : precedence::SYMBOL);
            bool paren = (argp < maxp &&
                          oid != ID_sqrt && oid != ID_inv && oid != ID_abs &&
                          oid != ID_exp && oid != ID_exp10 && oid != ID_exp2 &&
                          oid != ID_cbrt);
            if (paren)
                arg = parentheses(g, arg, 3);
            precedence = precedence::FUNCTION;

            g.font = savef;

            switch(oid)
            {
            case ID_sq:
                precedence = precedence::FUNCTION_POWER;
                return suscript(g, va, arg, 0, "2");
            case ID_cubed:
                precedence = precedence::FUNCTION_POWER;
                return suscript(g, va, arg, 0, "3");
            case ID_abs:
                precedence = precedence::SYMBOL;
                return abs_norm(g, arg);
            case ID_exp:
                precedence = precedence::FUNCTION_POWER;
                return suscript(g, 0, "e", va, arg);
            case ID_exp10:
                precedence = precedence::FUNCTION_POWER;
                return suscript(g, 0, "10", va, arg);
            case ID_exp2:
                precedence = precedence::FUNCTION_POWER;
                return suscript(g, 0, "2", va, arg);
            case ID_neg:
                precedence = precedence::ADDITIVE;
                return prefix(g, 0, "-", va, arg);
            case ID_fact:
                precedence = precedence::SYMBOL;
                return suscript(g, va, arg, 0, "!", 0);
            case ID_sqrt:
                precedence = precedence::FUNCTION_POWER;
                return root(g, arg);
            case ID_inv:
                precedence = precedence::FUNCTION_POWER;
                return ratio(g, "1", arg);
            case ID_cbrt:
            {
                auto fid = g.font;
                arg = root(g, arg);
                g.reduce_font();
                arg = suscript(g, 0, "3", va, arg, -1, false);
                g.font = fid;
                return arg;
            }

            default:
                break;
            }

            g.voffset = 0;
            grob_g fn = obj->graph(g);
            coord  vf = g.voffset;
            return paren ? prefix(g, vf, fn, va, arg)
                         : infix(g, vf, fn, 0, " ", va, arg);
        }
        break;
        case 2:
        {
            int    lprec = 0, rprec = 0;
            id     oid = obj->type();
            auto   fid  = g.font;
            if (oid == ID_pow || oid == ID_xroot ||
                oid == ID_comb || oid == ID_perm)
                g.reduce_font();
            grob_g rg   = graph(g, depth, rprec);
            coord  rv   = g.voffset;
            if (oid != ID_comb && oid != ID_perm)
                g.font      = fid;
            grob_g lg   = graph(g, depth, lprec);
            coord  lv   = g.voffset;
            int    prec = obj->precedence();
            g.font = fid;
            if ((prec == precedence::FUNCTION || prec == precedence::NONE) &&
                oid != ID_xroot && oid != ID_comb && oid != ID_perm)
            {
                grob_g arg = infix(g, lv, lg, 0, ";", rv, rg);
                coord  av  = g.voffset;
                arg = parentheses(g, arg);
                precedence = precedence::FUNCTION;
                g.voffset = 0;
                grob_g op  = obj->graph(g);
                coord  ov  = g.voffset;
                return prefix(g, ov, op, av, arg);
            }

            if ((oid != ID_divide || unit::mode) && oid != ID_xroot &&
                oid != ID_comb && oid != ID_perm)
            {
                if (lprec < prec)
                    lg = parentheses(g, lg);
                if (oid != ID_pow &&
                    (rprec < prec ||
                     (rprec == prec &&
                      (oid == ID_subtract || oid == ID_divide))))
                    rg = parentheses(g, rg);
            }
            precedence = prec;
            switch (oid)
            {
            case ID_pow: return suscript(g, lv, lg, rv, rg);
            case ID_divide: return unit::mode
                    ? infix(g, lv, lg, 0, "/", rv, rg)
                    : ratio(g, lg, rg);
            case ID_multiply: return infix(g, lv, lg, 0, mulsep(), rv, rg);
            case ID_xroot:
            {
                lg = root(g, lg);
                rg = suscript(g, rv, rg, lv, lg, -1, false);
                return rg;
            }
            case ID_comb:
            case ID_perm:
                rg = infix(g, lv, lg, 0, ",", rv, rg);
                rv = g.voffset;
                lg = suscript(g, 0, oid == ID_comb ? "C" : "P", rv, rg, -1);
                return lg;
            case ID_Derivative:
                rg = prefix(g, 0, "∂", rv, rg);
                rg = ratio(g, "∂", rg);
                rv = g.voffset;
                lg = prefix(g, rv, rg, lv, lg);
                return lg;
            case ID_Primitive:
                rg = prefix(g, 0, "d", rv, rg);
                rv = g.voffset;
                rg = prefix(g, lv, lg, rv, rg);
                rv = g.voffset;
                if (rg)
                {
                    lg = integral(g, rg->height());
                    rg = suscript(g, 0, lg, rv, rg, false);
                }
                return rg;

            default: break;
            }
            g.voffset = 0;
            grob_g op = obj->graph(g);
            coord ov = g.voffset;
            return infix(g, lv, lg, ov, op, rv, rg);
        }
        break;

        case 4:
        {
            id oid = obj->type();
            if (sumprod_fn shape = sumprod_shape(oid))
            {
                int    eprec = 0;
                auto   fid   = g.font;
                grob_g expr  = graph(g, depth, eprec);
                coord  ve    = g.voffset;
                if (oid != ID_Integrate)
                    g.reduce_font();
                grob_g last  = graph(g, depth, eprec);
                coord  vl    = g.voffset;
                if (oid == ID_Integrate)
                {
                    // 'last' is really the expression
                    if (eprec < MULTIPLICATIVE)
                        last = parentheses(g, last);
                    g.reduce_font();
                }
                grob_g first = graph(g, depth, eprec);
                coord  vf    = g.voffset;
                grob_g index = graph(g, depth, eprec);
                coord  vi    = g.voffset;
                g.font = fid;

                return sumprod(g, oid, shape,
                               vi, index,
                               vf, first,
                               vl, last,
                               ve, expr);
            }
        }

        [[fallthrough]];
        default:
        {
            grob_g args = nullptr;
            coord argsv = 0;
            for (uint a = 0; a < arity; a++)
            {
                int    prec = 0;
                grob_g arg  = graph(g, depth, prec);
                coord  argv = g.voffset;
                if (a)
                    args = infix(g, argv, arg, 0, ";", argsv, args);
                else
                    args = arg;
                argsv = g.voffset;
            }
            args = parentheses(g, args);
            precedence = precedence::FUNCTION;
            g.voffset = 0;
            grob_g op = obj->graph(g);
            coord ov = g.voffset;
            return prefix(g, ov, op, argsv, args);
        }
        }
    }

    return nullptr;
}


GRAPH_BODY(expression)
// ----------------------------------------------------------------------------
//   Render an expression graphically
// ----------------------------------------------------------------------------
{
    expression_g expr  = o;
    size_t       depth = rt.depth();
    bool         funcall = o->type() == ID_funcall;
    save<bool>   sexpr(g.expression, true);

    // First push all things so that we have the outermost operators first
    if (!expr->expand_without_size())
        return nullptr;

    int    prec   = 0;
    grob_g result = graph(g, depth, prec);
    bool   more   = rt.depth() > depth;
    if (more)
    {
        if (funcall)
        {
            grob_g args  = nullptr;
            coord  vr    = g.voffset;
            coord  voffs = 0;
            while (more)
            {
                int    aprec = 0;
                grob_p argg  = graph(g, depth, aprec);
                if (!argg)
                    return nullptr;
                int va = g.voffset;
                more = rt.depth() > depth;
                if (args)
                {
                    args = infix(g, va, argg, 0, ";", voffs, args);
                    voffs = g.voffset;
                    if (!args)
                        return nullptr;
                }
                else
                {
                    args = argg;
                    voffs = va;
                }
            }
            args = parentheses(g, args);
            if (!args)
                return nullptr;
            result = prefix(g, vr, result, voffs, args);
        }
        else
        {
            size_t remove = rt.depth() - depth;
            record(expression_error, "Malformed equation, %u removed", remove);
            rt.drop(remove);
        }
    }
    return result;
}


expression_p expression::get(object_p obj)
// ----------------------------------------------------------------------------
//   Convert an object to an expression, including polynomials and equations
// ----------------------------------------------------------------------------
{
    object_p old = nullptr;
    while (old != obj && obj)
    {
        old = obj;
        if (expression_p expr = obj->as<expression>())
            return expr;
        if (tag_p tagged = obj->as<tag>())
            obj = tagged->tagged_object();
        else if (equation_p eqn = obj->as<equation>())
            obj = eqn->value();
        else if (polynomial_p poly = obj->as<polynomial>())
            obj = poly->as_expression();
        else if (assignment_p asn = obj->as<assignment>())
            return expression::make(ID_TestEQ, asn->name(), asn->value());
        else if (algebraic_g alg = obj->as_algebraic())
            return make(alg);
    }
    return nullptr;
}


bool expression::is_simplifiable() const
// ----------------------------------------------------------------------------
//   Return true if the expression is simplifiable
// ----------------------------------------------------------------------------
{
    for (object_p obj : *this)
        if (!obj->is_simplifiable())
            return false;
    return true;
}


list_p expression::current_equation(bool all, bool error)
// ----------------------------------------------------------------------------
//   Return content of EQ variable
// ----------------------------------------------------------------------------
{
    object_p obj  = FinanceSolverMenu::active()
        ? FinanceSolverMenu::equation()
        : directory::recall_all(static_object(ID_Equation), false);
    bool more = true;
    while (more)
    {
        obj = strip(obj);
        if (!obj)
        {
            if (error && !rt.error())
                rt.no_equation_error();
            return nullptr;
        }
        switch (obj->type())
        {
        case ID_equation:
            obj = equation_p(obj)->value();
            break;
        case ID_list:
        case ID_array:
            if (all)
                more = false;
            else
                obj = list_p(obj)->at(0);
            break;
        case ID_expression:
            if (equation_p eq = obj->as_quoted<equation>())
            {
                obj = eq;
                break;
            }
        case ID_polynomial:
            more = false;
            break;
        case ID_symbol:
            if (object_p named =  directory::recall_all(obj, false))
            {
                // Make sure we can exit from an infinite loop
                if (!program::interrupted())
                {
                    obj = named;
                    continue;
                }
            }

        default:
            rt.type_error();
            return nullptr;
        }
    }
    list_p eq = list_p(obj);
    return eq;
}


bool expression::is_well_defined(symbol_p solving,
                                 bool     error,
                                 list_p   unknowns) const
// ----------------------------------------------------------------------------
//   Check if all variables but the one we solve for are defined
// ----------------------------------------------------------------------------
{
    expression_g expr  = this;
    symbol_g     sym   = solving;
    list_g       unset = unknowns;
    list_p       vars  = expr->names();
    bool         found = false;
    if (!vars)
        return false;

    for (auto var : *vars)
    {
        if (symbol_p vsym = var->as<symbol>())
        {
            if (sym && sym->is_same_as(vsym))
            {
                found = true;
            }
            else if ((unset && unset->contains(vsym)) ||
                     !directory::recall_all(var, false))
            {
                if (error)
                    rt.some_undefined_name_error();
                return false;
            }
        }
        else
        {
            if (error)
                rt.some_invalid_name_error();
            return false;
        }
    }
    return !sym || found;
}


object::result expression::variable_command(command_fn callback)
// ----------------------------------------------------------------------------
//   Process a command that applies to a variable name
// ----------------------------------------------------------------------------
{
    if (object_p exprobj = rt.stack(1))
        if (expression_g expr = expression::get(exprobj))
            if (object_p varobj = rt.stack(0))
                if (symbol_g var = varobj->as_quoted<symbol>())
                    if (expression_p res = (expr->*callback)(var))
                        if (rt.drop() && rt.top(res))
                            return OK;

    if (!rt.error())
        rt.type_error();
    return ERROR;
}


bool expression::is_linear(symbol_r sym, algebraic_g &a, algebraic_g &b) const
// ----------------------------------------------------------------------------
//   Check if the expression is linear in the current independent variable
// ----------------------------------------------------------------------------
//   Note that this returns false if we only have constants
{
    if (!depends_on(sym))
        return false;

    object_p op = outermost_operator();
    if (!op)
        return false;

    id type = op->type();

    if (type == ID_symbol)
    {
        bool ok = symbol_p(op)->is_same_as(sym);
        if (ok)
        {
            a = integer::make(1);
            b = integer::make(0);
        }
        return ok;

    }

    if (type == ID_add || type == ID_subtract ||
        type == ID_multiply || type == ID_divide)
    {
        expression_g l, r;
        if (split(type, l, r))
        {
            bool lcst = !l->depends_on(sym);
            bool rcst = !r->depends_on(sym);
            if (lcst)
            {
                if (rcst)
                    return false;
                if (r->is_linear(sym, a, b))
                {
                    algebraic_g t = l->evaluate();
                    switch(type)
                    {
                    case ID_add:        b = t + b; break;
                    case ID_subtract:   b = t - b; break;
                    case ID_multiply:   b = t * b; a = t * a; break;
                    default:
                    case ID_divide:     return false;
                    }
                    return a && b;
                }
                return false;
            }
            if (rcst)
            {
                if (l->is_linear(sym, a, b))
                {
                    algebraic_g t = r->evaluate();
                    switch(type)
                    {
                    case ID_add:        b = b + t; break;
                    case ID_subtract:   b = b - t; break;
                    case ID_multiply:   b = b * t; a = a * t; break;
                    case ID_divide:     b = b / t; a = a / t; break;
                    default:            return false;
                    }
                    return a && b;
                }
                return false;
            }

            if (l->is_linear(sym, a, b))
            {
                algebraic_g ra, rb;
                if (r->is_linear(sym, ra, rb))
                {
                    switch(type)
                    {
                    case ID_add:        a = a + ra; b = b + rb; break;
                    case ID_subtract:   a = a - ra; b = b - rb; break;
                    case ID_multiply:
                    case ID_divide:
                    default:            return false;
                    }
                    return a && b;
                }
            }
        }
        return false;
    }

    return false;
}


bool expression::depends_on(symbol_r sym) const
// ----------------------------------------------------------------------------
//   Return true if the symbol is referenced in expression
// ----------------------------------------------------------------------------
{
    return sym->found_in(this);
}



// ============================================================================
//
//   User-defined function calls
//
// ============================================================================

PARSE_BODY(funcall)
// ----------------------------------------------------------------------------
//    Parse a function call within an expression
// ----------------------------------------------------------------------------
{
    // If not inside an expression or no callee, we can't have a function call
    if (!p.precedence || !p.out)
        return SKIP;

    // We need to have a valid callee as a function call
    // Note that the list of valid callees is larger than for RPL,
    // since we also accept arrays, lists and expressions
    id nty = p.out->type();
    if (nty != ID_symbol && nty != ID_local &&
        nty != ID_expression && nty != ID_funcall &&
        nty != ID_list && nty != ID_array && nty != ID_xlib)
        return SKIP;

    // Cannot parse functions while parsing a unit
    if (unit::mode)
    {
        rt.invalid_unit_expression_error().source(p.source);
        return ERROR;
    }

    // Skip whitespace
    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = utf8_skip_whitespace(source, max);

    // Skip if this is not a function call
    if (parsed >= max || utf8_codepoint(source + parsed) != '(')
        return SKIP;

    // Record the callee
    object_g callee = p.out;

    // Skip the opening parenthese
    parsed = utf8_next(source, parsed, max);

    scribble scr;
    unicode  cp = utf8_codepoint(source + parsed);
    while (parsed < max && cp != ')')
    {
        parser  child(p, source + parsed, LOWEST);
        result rc = list_parse(ID_expression, child, 0, 0);
        if (rc != OK)
            return rc;
        object_g obj = child.out;
        if (!obj)
            return ERROR;
        parsed += child.length;

        if (!rt.append_expression(obj))
            return ERROR;

        source = p.source;      // In case of GC
        cp = utf8_codepoint(source + parsed);
        if (cp != ')' && cp != ';')
        {
            rt.syntax_error().source(source + parsed);
            return ERROR;
        }
        parsed = utf8_next(source, parsed, max);
    }

    if (cp != ')')
    {
        rt.unterminated_error().source(p.source, parsed);
        return ERROR;
    }

    // Copy the name last
    if (!rt.append(callee))
        return ERROR;

    // Create the function call object
    gcbytes scratch = scr.scratch();
    size_t  alloc   = scr.growth();
    p.length        = parsed;
    p.out           = rt.make<funcall>(ID_funcall, scratch, alloc);
    return p.out ? OK : ERROR;
}


EVAL_BODY(funcall)
// ----------------------------------------------------------------------------
//   Function calls get evaluated immediately
// ----------------------------------------------------------------------------
{
    return o->run(true);
}


array_p funcall::args() const
// ----------------------------------------------------------------------------
//   Return an array with the arguments to the funcall
// ----------------------------------------------------------------------------
{
    stack_depth_restore sdr;

    // Build list of arguments
    if (!expand_without_size())
        return nullptr;
    scribble scr;
    while (object_p obj = arg(sdr.depth))
        if (!rt.append(obj))
            return nullptr;
    if (rt.depth() > sdr.depth)
        return nullptr;

    array_g a = array_p(list::make(ID_array, scr.scratch(), scr.growth()));

    // Put arguments back in correct order
    if (!a->expand_without_size())
        return nullptr;
    size_t nitems = rt.depth() - sdr.depth - 1;
    size_t nhalf = nitems / 2;
    for (size_t i = 0; i < nhalf; i++)
    {
        object_p lo = rt.stack(i);
        object_p hi = rt.stack(nitems + ~i);
        rt.stack(i, hi);
        rt.stack(nitems + ~i, lo);
    }

    a = array::from_stack(nitems + 1, 0);
    return a;
}


object_p funcall::arg(uint depth) const
// ----------------------------------------------------------------------------
//   Return the outermost argument found in a function call
// ----------------------------------------------------------------------------
{
    if (rt.depth() <= depth)
        return nullptr;
    size_t sz = rt.depth() - depth;
    size_t eq = 0;
    object_p result = grab_arguments(eq, sz);
    rt.drop(eq);
    return result;
}


COMMAND_BODY(Apply)
// ----------------------------------------------------------------------------
//   Apply arguments to build a function call
// ----------------------------------------------------------------------------
{
    if (object_g callee = rt.stack(0))
    {
        if (object_p args = rt.stack(1))
        {
            if (list_g lst = args->as_array_or_list())
            {
                size_t arity = lst->items();

                if (object_p quoted = callee->as_quoted(ID_object))
                    callee = quoted;
                if (symbol_p sym = callee->as<symbol>())
                {
                    callee = sym;
                }
                else if (callee->is_algebraic_fn())
                {
                    if (arity != callee->arity())
                    {
                        rt.argument_count_error();
                        return ERROR;
                    }
                }
                else
                {
                    rt.type_error();
                    return ERROR;
                }

                scribble scr;
                size_t argsize = 0;
                object_p argsrc = lst->objects(&argsize);
                if (rt.append(argsrc, argsize))
                {
                    if (rt.append(callee))
                    {
                        gcbytes scratch = scr.scratch();
                        size_t  alloc   = scr.growth();
                        callee = rt.make<funcall>(ID_funcall, scratch, alloc);
                        if (rt.drop() && rt.top(callee))
                            return OK;
                    }
                }
            }
            else
            {
                rt.type_error();
            }
        }
    }
    return ERROR;
}



// ============================================================================
//
//   User-accessible match commands
//
// ============================================================================

static object::result match_up_down(bool down)
// ----------------------------------------------------------------------------
//   Run a rewrite up or down
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(0);
    object_p y = rt.stack(1);
    if (!x || !y)
        return object::ERROR;
    list_p transform = x->as<list>();
    expression_g eq = expression::get(y);
    if (!transform || !eq)
    {
        rt.type_error();
        return object::ERROR;
    }

    list::iterator it(transform);
    expression_g from = expression::get(*it++);
    expression_g to = expression::get(*it++);
    if (!from || !to)
    {
        rt.value_error();
        return object::ERROR;
    }
    expression_g cond = expression::get(*it++);
    settings::SaveAutoSimplify noas(false);
    uint rwcount = 0;
    cond = eq->rewrite(from, to, cond, &rwcount, down);
    if (!cond)
        return object::ERROR;
    integer_g changed = integer::make(rwcount);
    if (!rt.stack(1, cond) || !rt.stack(0, changed))
        return object::ERROR;

    return object::OK;
}


COMMAND_BODY(MatchUp)
// ----------------------------------------------------------------------------
//   Bottom-up match and replace command
// ----------------------------------------------------------------------------
{
    return match_up_down(false);
}


COMMAND_BODY(MatchDown)
// ----------------------------------------------------------------------------
//   Match pattern down command
// ----------------------------------------------------------------------------
{
    return match_up_down(true);
}



// ============================================================================
//
//    Actual rewrites for various rules
//
// ============================================================================

template <byte ...args>
constexpr byte eq<args...>::object_data[sizeof...(args)+2];

// Wildcards used to build patterns (type based on initial letter, see above)
static eq_symbol<'a'>     a;    // Numerical constants
static eq_symbol<'b'>     b;
static eq_symbol<'c'>     c;
static eq_symbol<'d'>     d;    // Non-constant expressions
static eq_symbol<'e'>     e;
static eq_symbol<'f'>     f;
static eq_symbol<'i'>     i;    // Positive or zero integer values
static eq_symbol<'j'>     j;
static eq_symbol<'k'>     k;    // Positive non-zero integers
static eq_symbol<'l'>     l;
static eq_symbol<'m'>     m;
static eq_symbol<'n'>     n;    // Contains independent variable
static eq_symbol<'o'>     o;
static eq_symbol<'p'>     p;    // Other, does not contain independent variable
static eq_symbol<'q'>     q;
static eq_symbol<'r'>     r;
static eq_symbol<'s'>     s;    // Symbols
static eq_symbol<'t'>     t;
static eq_symbol<'u'>     u;    // Unique subexpressions
static eq_symbol<'v'>     v;
static eq_symbol<'w'>     w;
static eq_symbol<'x'>     x;    // Any subexpression
static eq_symbol<'y'>     y;
static eq_symbol<'z'>     z;

// Wildcards that need not be sorted (e.g. matches if A<B or A>B)
static eq_symbol<'A'>     A;    // Numerical constants
static eq_symbol<'B'>     B;
static eq_symbol<'C'>     C;
static eq_symbol<'D'>     D;    // Non-constant expressions
static eq_symbol<'E'>     E;
static eq_symbol<'F'>     F;
static eq_symbol<'I'>     I;    // Positive or zero integer values
static eq_symbol<'J'>     J;
static eq_symbol<'K'>     K;    // Positive non-zero integers
static eq_symbol<'L'>     L;
static eq_symbol<'M'>     M;
static eq_symbol<'N'>     N;    // Contains independent variable
static eq_symbol<'O'>     O;
static eq_symbol<'P'>     P;    // Other, does not contain independent variable
static eq_symbol<'Q'>     Q;
static eq_symbol<'R'>     R;
static eq_symbol<'S'>     S;    // Symbols
static eq_symbol<'T'>     T;
static eq_symbol<'U'>     U;    // Unique subexpressions
static eq_symbol<'V'>     V;
static eq_symbol<'W'>     W;
static eq_symbol<'X'>     X;    // Any subexpression
static eq_symbol<'Y'>     Y;
static eq_symbol<'Z'>     Z;

// Numerical constants
static eq_integer<0>      zero;
static eq_neg_integer<-1> mone;
static eq_integer<1>      one;
static eq_integer<2>      two;
static eq_integer<3>      three;
static eq_integer<4>      four;
static eq_integer<10>     ten;
static eq_always          always;

// Sign and integer value for non-princpal solutions, see some_index() function
static eq_symbol<'#'>     intk;
static eq_symbol<'+'>     natk;
static eq_symbol<'-'>     signk;
static eq_symbol<'@'>     kpi;
static eq_symbol<'!'>     ki;
static eq_symbol<'='>     indep;


bool expression::split_equation(expression_g &left, expression_g &right) const
// ----------------------------------------------------------------------------
//   Split an expression between left and right parts
// ----------------------------------------------------------------------------
{
    return split(ID_TestEQ, left, right);
}


bool expression::split(id type, expression_g &left, expression_g &right) const
// ----------------------------------------------------------------------------
//   Split around a binary operator
// ----------------------------------------------------------------------------
{
    stack_depth_restore sdr;
    bool                result = false;

    if (!expand_without_size())
        return false;

    if (rt.depth() > sdr.depth)
    {
        if (object_p outer = rt.top())
        {
            if (outer->type() == type)
            {
                size_t eq = 1;
                size_t len = rt.depth() - sdr.depth - eq;
                if (object_g r = grab_arguments(eq, len))
                {
                    if (object_g l = grab_arguments(eq, len))
                    {
                        expression_g ra = expression::get(r);
                        expression_g la = expression::get(l);
                        if (la && ra)
                        {
                            right = ra;
                            left = la;
                            result = true;
                        }
                    }
                }
            }
        }
    }

    return result;
}


expression_p expression::as_difference_for_solve() const
// ----------------------------------------------------------------------------
//   For the solver, transform A=B into A-B
// ----------------------------------------------------------------------------
//   Revisit: how to transform A and B, A or B, e.g. A=B and C=D ?
{
    expression_g left = this;
    expression_g right;
    if (split_equation(left, right))
        return expression::make(ID_subtract, +left, +right);
    return left;
}


expression_p expression::expand() const
// ----------------------------------------------------------------------------
//   Run various rewrites to expand terms
// ----------------------------------------------------------------------------
{
    return rewrites<DOWN>(
        // Compute constants
        A+B,            A+B,
        A-B,            A-B,
        A*B,            A*B,
        A/B,            A/B,
        A^B,            A^B,
        -A,             -A,

        // Expand bult-in functions
        inv(x),         one/x,
        sq(x),          x^two,
        cubed(x),       x^three,
        sqrt(x),        x^(one/two),
        cbrt(x),        x^(one/three),

        // Distribute additions
        (X+Y)*Z,        X*Z+Y*Z,
        (X-Y)*Z,        X*Z-Y*Z,
        X*(Y+Z),        X*Y+X*Z,
        X*(Y-Z),        X*Y-X*Z,

        // Expand powers
        (X*Y)^Z,        (X^Z)*(Y^Z),
        (X/Y)^Z,        (X^Z)/(Y^Z),
        X^(Y+Z),        (X^Y)*(X^Z),
        X^(Y-Z),        (X^Y)/(X^Z),
        X^(Y*Z),        (X^Y)^Z,

        // Group terms
        v + u,          u + v,
        X + v + u,      X + u + v,
        v * u,          u * v,
        X * v * u,      X * u * v,

        // Sign change simplifications
        X + (-Y),       X - Y,
        X - (-Y),       X + Y,
        X * (-Y),       -(X*Y),
        X / (-Y),       -(X/Y),
        X ^ (-Y),       one / (X^Y),

        (-X) + Y,       Y - X,
        (-X) - Y,       -(X + Y),
        (-X) * Y,       -(X * Y),
        (-X) / Y,       -(X / Y),

        // Additive simplifications
        X + zero,       X,
        X + X,          two * X,
        X - X,          zero,
        X - zero,       X,
        zero - X,       -X,
        A * X + X,      (A + one) * X,
        X + A * X,      (A + one) * X,
        A*X + B*X,      (A + B) * X,

        // Multiplicative simplifications
        zero * X,       zero,
        one * X,        X,
        zero / X,       zero,
        X / one,        X,

        // Power simplifications
        X^zero,         one,
        X^one,          X,

        // Expansion of powers
        X^K,            (X^(K-one))*X);
}


expression_p expression::collect() const
// ----------------------------------------------------------------------------
//    Run various rewrites to collect terms (inverse of expand)
// ----------------------------------------------------------------------------
{
    return rewrites<UP>(
        // Collection of powers
        (X^K)*X,        X^(K+one),
        X*(X^K),        X^(K+one),
        (X^A)*(X^B),    X^(A+B),

        // Power simplifications
        X^one,          X,
        X^zero,         one,

        // Multiplicative simplifications
        X / one,        X,
        zero / X,       zero,
        one * X,        X,
        zero * X,       zero,

        // Additive simplifications
        A*X + B*X,      (A + B) * X,
        X + A * X,      (A + one) * X,
        A * X + X,      (A + one) * X,
        zero - X,       -X,
        X - zero,       X,
        X - X,          zero,
        X + X,          two * X,
        X + zero,       X,

        // Sign change simplifications
        (-X) / Y,       -(X / Y),
        (-X) * Y,       -(X * Y),
        (-X) - Y,       -(X + Y),
        (-X) + Y,       Y - X,

        X ^ (-Y),       one / (X^Y),
        x / (-Y),       -(X/Y),
        X * (-Y),       -(X*Y),
        X - (-Y),       X + Y,
        X + (-Y),       X - Y,

        // Group terms
        X * v * u,      X * u * v,
        v * u,          u * v,
        X + v + u,      X + u + v,
        v + u,          u + v,

        // Collect powers
        (X^Y)^Z,        X^(Y*Z),
        (X^Y)/(X^Z),    X^(Y-Z),
        (X^Y)*(X^Z),    X^(Y+Z),
        (X^Z)/(Y^Z),    (X/Y)^Z,
        (X^Z)*(Y^Z),    (X*Y)^Z,

        // Collect additions
        X*Y-X*Z,        X*(Y-Z),
        X*Y+X*Z,        X*(Y+Z),
        X*Z-Y*Z,        (X-Y)*Z,
        X*Z+Y*Z,        (X+Y)*Z,

        // Generate initial powers (must be last)
        X*X,            X^two,

        // Compute constants
        -A,          -A,
        A^B,         A^B,
        A/B,         A/B,
        A*B,         A*B,
        A-B,         A-B,
        A+B,         A+B);
}


expression_p expression::fold_constants() const
// ----------------------------------------------------------------------------
//  Fold constants
// ----------------------------------------------------------------------------
{
    return rewrites<DOWN>(
        // Compute constants
        A+B,         A+B,
        A-B,         A-B,
        A*B,         A*B,
        A/B,         A/B,
        A^B,         A^B,
        -A,          -A,

        // Group terms
        v + u,       u + v,
        X + v + u,   X + u + v,
        A + X,       X + A,
        v * u,       u * v,
        X * v * u,   X * u * v,
        X * A,       A * X,

        // Additive simplifications
        X + zero,    X,
        X + X,       two * X,
        X - X,       zero,
        X - zero,    X,
        zero - X,    -X,
        A * X + X,   (A + one) * X,
        X + A * X,   (A + one) * X,
        A*X + B*X,   (A + B) * X,

        // Multiplicative simplifications
        zero * X,    zero,
        one * X,     X,
        zero / X,    zero,
        X / one,     X,

        // Power simplifications
        X^zero,      one,
        X^one,       X);
}


expression_p expression::reorder_terms() const
// ----------------------------------------------------------------------------
//   Reorder terms
// ----------------------------------------------------------------------------
{
    return rewrites<DOWN>(
        // Group terms
        v + u,       u + v,
        X + v + u,   X + u + v,
        A + X,       X + A,
        v * u,       u * v,
        X * v * u,   X * u * v,
        X * A,       A * x);
}


expression_p expression::simplify() const
// ----------------------------------------------------------------------------
//   Run various rewrites to simplify equations
// ----------------------------------------------------------------------------
{
    record(expression, "Simplify %t", this);
    expression_g eq = this;
    if (!eq->is_simplifiable())
        return eq;
    return eq->rewrites(
        // Compute constant sub-expressions
        A+B,            A+B,
        A*B,            A*B,
        A-B,            A-B,
        A/B,            A/B,
        A^B,            A^B,
        sq(A),          A*A,
        cubed(A),       A*A*A,

        // Addition simplifications
        A+E,            E+A,
        X+zero,         X,
        X+X,            two*X,
        X+(Y+Z),        (X+Y)+Z,
        X+A+B,          X+(A+B),
        (X+A)+E,        (X+E)+A,

        // Subtraction simplifications
        X-zero,         X,
        zero-X,         -X,
        X-X,            zero,
        X+Y-Y,          X,
        X-Y+Y,          X,
        X+(Y-Z),        (X+Y)-Z,
        X-Y+E,          (X+E)-Y,
        X+A-B,          X+(A-B),
        (X-A)+E,        (X+E)-A,
        (X+A)-E,        (X-E)+A,
        (X-A)-E,        (X-E)-A,
        -(-X),          X,

        // Multiplication simplification
        X*A,            A*X,
        zero*X,         zero,
        one*X,          X,
        A*X+X,          (A+one)*X,
        X+A*X,          (A+one)*X,
        A*X+B*X,        (A+B)*X,
        A*X-B*X,        (A-B)*X,
        A*X-X,          (A-one)*X,
        X-A*X,          (one-A)*X,
        X*(Y*Z),        (X*Y)*Z,
        X*X*X,          cubed(X),
        X*X,            sq(X),
        sq(X)*X,        cubed(X),
        X*sq(X),        cubed(X),
        cubed(X)*X,     X^four,
        X*cubed(X),     X^four,
        sq(X)*sq(X),    X^four,
        sq(sq(X)),      X^four,

        // Division simplification
        X*(Y/Z),        (X*Y)/Z,
        A*X/B,          (A/B)*X,
        X/X,            one,
        A/(B/X),        (A/B)*X,
        one/X,          inv(X),
        inv(inv(X)),    X,

        // Power simplifications
        (X^A)*X,        X^(A+one),
        X*(X^A),        X^(A+one),
        sq(X)*(X^A),    X^(A+two),
        (X^A)*sq(X),    X^(A+two),
        cubed(X)*(X^A), X^(A+three),
        (X^A)*cubed(X), X^(A+three),
        (X^A)*(X^B),    X^(A+B),
        X^three,        cubed(X),
        X^two,          sq(X),
        X^one,          X,
        X^zero,         one,

        // Function simplifications
        sin(asin(X)),   X,
        cos(acos(X)),   X,
        tan(atan(X)),   X,
        sinh(asinh(X)), X,
        cosh(acosh(X)), X,
        tanh(atanh(X)), X,
        asinh(sinh(X)), X,
        acosh(cosh(X)), X,
        atanh(tanh(X)), X,
        abs(abs(X)),    abs(X),
        abs(-X),        abs(X),
        sqrt(abs(X)),   sqrt(X),
        sqrt(sq(X)),    abs(X),
        sq(sqrt(X)),    X,
        sq(X^Y),        X^(two*Y),
        sqrt(X^Y),      X^(Y/two),
        cubed(X^Y),     X^(three*Y),
        cbrt(X^Y),      X^(Y/three),
        cbrt(cubed(X)), X,
        cubed(cbrt(X)), X,
        ln(exp(X)),     X,
        exp(ln(X)),     X,
        log10(exp10(X)),X,
        exp10(log10(X)),X
        );
}


// ============================================================================
//
//   User-level expression rewrite commands
//
// ============================================================================

static algebraic_p do_rewrite(algebraic_r x,
                              expression_p (expression::*op)() const)
// ----------------------------------------------------------------------------
//   Shared code for all rewrite operations
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (expression_p eq = x->as<expression>())
        return (eq->*op)();
    if (x->is_algebraic())
        return x;
    rt.type_error();
    return nullptr;
}


FUNCTION_BODY(Expand)
// ----------------------------------------------------------------------------
//   Expand terms in expressions
// ----------------------------------------------------------------------------
{
    return do_rewrite(x, &expression::expand);
}


FUNCTION_BODY(Collect)
// ----------------------------------------------------------------------------
//   Collect terms in expressions (inverse of expand)
// ----------------------------------------------------------------------------
{
    return do_rewrite(x, &expression::collect);
}


FUNCTION_BODY(FoldConstants)
// ----------------------------------------------------------------------------
//   Evaluate constants in expressions
// ----------------------------------------------------------------------------
{
    return do_rewrite(x, &expression::fold_constants);
}


FUNCTION_BODY(ReorderTerms)
// ----------------------------------------------------------------------------
//   Sort terms in the expression
// ----------------------------------------------------------------------------
{
    return do_rewrite(x, &expression::reorder_terms);
}


FUNCTION_BODY(Simplify)
// ----------------------------------------------------------------------------
//   Simplify equations
// ----------------------------------------------------------------------------
{
    return do_rewrite(x, &expression::simplify);
}



NFUNCTION_BODY(Subst)
// ----------------------------------------------------------------------------
//   Perform a substitution without evaluating the resulting expression
// ----------------------------------------------------------------------------
{
    if (args[1]->is_real() || args[1]->is_complex())
        return args[1];

    if (expression_g pat = expression::get(args[1]))
        if (expression_g repl = expression::get(args[0]))
            return pat->substitute(repl);

    rt.type_error();
    return nullptr;
}


COMMAND_BODY(Where)
// ----------------------------------------------------------------------------
//   Perform a substitution and evaluate the resulting expression
// ----------------------------------------------------------------------------
{
    object_g obj = rt.stack(1);
    object_g args = rt.stack(0);
    obj = list::substitute(obj, args);
    if (obj && rt.drop() && rt.top(+obj))
        return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


expression_p expression::isolate(symbol_r sym) const
// ----------------------------------------------------------------------------
//   Isolate the variable in the expression
// ----------------------------------------------------------------------------
{
    save<symbol_g *> sindep(independent, (symbol_g *) &sym);
    save<object_g *> sindval(independent_value, nullptr);
    save<uint>       sconstant(constant_index, 0);
    expression_g eq = this;
    if (eq)
    {
        expression_g left, right;
        if (!split_equation(left, right))
        {
            algebraic_g l = +eq;
            algebraic_g r = zero.as_expression();
            eq = expression::make(ID_TestEQ, l, r);
        }
        if (left)
            if (symbol_p lsym = left->as_quoted<symbol>())
                if (lsym->is_same_as(sym))
                    if (eq->rewrites(N==P, one) != eq)
                        return eq;
    }
    if (!eq)
        return nullptr;

    expression_g result = Settings.PrincipalSolution()
        ? eq->rewrites(
            // Move the independent variable to the left
            P == N,                 N == P,
            N == O + P,             N - O == P,
            N == P + O,             N - O == P,
            N == O - P,             N - O == -P,
            N == P - O,             N + O == P,
            N == O * P,             N / O == P,
            N == P * O,             N / O == P,
            N == O / P,             N / O == inv(P),
            N == P / O,             N * O == P,

            // Binary operations
            N + Q == P,             N == P - Q,
            Q + N == P,             N == P - Q,
            N - Q == P,             N == P + Q,
            Q - N == P,             N == Q - P,
            N * Q == P,             N == P / Q,
            Q * N == P,             N == P / Q,
            N / Q == P,             N == P * Q,
            Q / N == P,             N == Q / P,
            (N ^ Q) == P,           N == (P ^ inv(Q)),
            (Q ^ N) == P,           N == ln(P) / ln(Q),

            // Basic simplifications
            N + N == P,             N == P / two,
            N + Q*N == P,           N == P / (one + Q),
            Q*N + N == P,           N == P / (one + Q),
            Q*N + R*N == P,         N == P / (Q+R),
            N - N == P,             zero == P,
            N - Q*N == P,           N == P / (one - Q),
            Q*N - N == P,           N == P / (Q - one),
            Q*N - R*N == P,         N == P / (Q-R),

            // Reversible functions
            inv(N) == P,            N == inv(P),
            sin(N) == P,            N == asin(P),
            cos(N) == P,            N == acos(P),
            tan(N) == P,            N == atan(P),
            sinh(N) == P,           N == asinh(P),
            cosh(N) == P,           N == acosh(P),
            tanh(N) == P,           N == atanh(P),
            asin(N) == P,           N == sin(P),
            acos(N) == P,           N == cos(P),
            atan(N) == P,           N == tan(P),
            asinh(N) == P,          N == sinh(P),
            acosh(N) == P,          N == cosh(P),
            atanh(N) == P,          N == tanh(P),

            ln(N) == P,             N == exp(P),
            exp(N) == P,            N == ln(P),
            log2(N) == P,           N == exp2(P),
            exp2(N) == P,           N == log2(P),
            log10(N) == P,          N == exp10(P),
            exp10(N) == P,          N == log10(P),
            ln1p(N) == P,           N == expm1(P),
            expm1(N) == P,          N == ln1p(P),

            sq(N) == P,             N == sqrt(P),
            sqrt(N) == P,           N == sq(P),
            cubed(N) == P,          N == cbrt(P),
            cbrt(N) == P,           N == cubed(P)
            )
        : eq->rewrites(
            // Move the independent variable to the left
            P == N,                 N == P,
            N == O + P,             N - O == P,
            N == P + O,             N - O == P,
            N == O - P,             N - O == -P,
            N == P - O,             N + O == P,
            N == O * P,             N / O == P,
            N == P * O,             N / O == P,
            N == O / P,             N / O == inv(P),
            N == P / O,             N * O == P,

            // Binary operations
            N + Q == P,             N == P - Q,
            Q + N == P,             N == P - Q,
            N - Q == P,             N == P + Q,
            Q - N == P,             N == Q - P,
            N * Q == P,             N == P / Q,
            Q * N == P,             N == P / Q,
            N / Q == P,             N == P * Q,
            Q / N == P,             N == Q / P,
            (N ^ Q) == P,           N == (P ^ inv(Q)) + exp(intk*kpi*ki/Q),
            (Q ^ N) == P,           N == ln(P) / ln(Q),

            // Basic simplifications
            N + N == P,             N == P / two,
            N + X*N == P,           N == P / (one + X),
            X*N + N == P,           N == P / (one + X),
            X*N + Y*N == P,         N == P / (X+Y),
            N - N == P,             zero == P,
            N - Q*N == P,           N == P / (one - Q),
            Q*N - N == P,           N == P / (Q - one),
            Q*N - R*N == P,         N == P / (Q-R),

            // Reversible functions
            inv(N) == P,            N == inv(P),
            sin(N) == P,            N == asin(P) + two*intk*kpi,
            cos(N) == P,            N == acos(P) + two*intk*kpi,
            tan(N) == P,            N == atan(P) + intk*kpi,
            sinh(N) == P,           N == asinh(P) + two*intk*kpi*ki,
            cosh(N) == P,           N == acosh(P) + two*intk*kpi*ki,
            tanh(N) == P,           N == atanh(P) + intk*kpi*ki,
            asin(N) == P,           N == sin(P),
            acos(N) == P,           N == cos(P),
            atan(N) == P,           N == tan(P),
            asinh(N) == P,          N == sinh(P),
            acosh(N) == P,          N == cosh(P),
            atanh(N) == P,          N == tanh(P),

            ln(N) == P,             N == exp(P),
            exp(N) == P,            N == ln(P) + two*intk*kpi*ki,
            log2(N) == P,           N == exp2(P),
            exp2(N) == P,           N == log2(P) + two*intk*kpi*ki/ln(two),
            log10(N) == P,          N == exp10(P),
            exp10(N) == P,          N == log10(P) + two*intk*kpi*ki/ln(ten),
            ln1p(N) == P,           N == expm1(P),
            expm1(N) == P,          N == ln1p(P) + two*intk*kpi*ki,

            sq(N) == P,             N == signk*sqrt(P),
            sqrt(N) == P,           N == sq(P),
            cubed(N) == P,          N == cbrt(P) + exp(intk*kpi*ki/three),
            cbrt(N) == P,           N == cubed(P)
            );

    if (+result == +eq)
    {
        rt.cannot_isolate_error();
        return nullptr;
    }
    if (result && Settings.AutoSimplify())
        result = result->simplify();
    return result;
}


COMMAND_BODY(Isolate)
// ----------------------------------------------------------------------------
//   Isolate a variable from an expression
// ----------------------------------------------------------------------------
{
    return expression::variable_command(&expression::isolate);
}


static size_t derivative_funcall_match(funcall_p pat, funcall_p repl)
// ----------------------------------------------------------------------------
//   Check if we match a function call
// ----------------------------------------------------------------------------
{
    size_t depth = rt.depth();
    if (!pat->expand_without_size())
        return 0;
    size_t patsz = rt.depth() - depth;
    if (!repl->expand_without_size())
    {
        rt.drop(patsz);
        return 0;
    }
    size_t replsz = rt.depth() - depth - patsz;
    size_t replst = 0;
    size_t patst = replsz;
    size_t match = check_match(replst, replsz, patst, patsz);
    rt.drop(rt.depth() - depth);
    return match;
}


static algebraic_p derivative_funcall_build(funcall_p src, funcall_p repl)
// ----------------------------------------------------------------------------
//   Build function calls for derivative command
// ----------------------------------------------------------------------------
//   The pattern for the funcall derivative only gives a single term.
//   Isolate the terms and run them one after the other.
//   When we enter this function while differentiating 'F(A*X;B*X^2)', we have
//   src set to that function call, and repl set to the replacement pattern,
//   which for differentiation is 'S(X)'.
{
    funcall_g   sg  = src;
    array_g     sa  = src->args();
    array_g     ra = repl->args();
    if (!sa || !ra)
        return nullptr;

    // Extract function and argument from replacement pattern
    object_p ro = ra->objects();
    if (!ro)
        return nullptr;

    symbol_g    rf = ro->as<symbol>();
    symbol_g    rx = ro->skip()->as<symbol>();
    if (!rf || !rx)
        return nullptr;

    // Extract function from source pattern
    size_t      sz = 0;
    object_g    so = sa->objects(&sz);
    symbol_g    sf = so->as<symbol>();
    if (!sf)
        return nullptr;
    size_t      sp = so->size();
    size_t      sn = sa->items();
    size_t      nobj = src->items();
    uint        aidx = 0;

    // If we have F(), then it does not depend on variable, return 0
    // (Should not happen because we already check for R before)
    ASSERT("User functions without args should not be seen here" && sn >= 2);

    // Transform F into F′
    symbol_g    sd = sf + symbol::make("′");

    // Variable we differentiate against
    ASSERT("Derivative should set independent variable" &&
           expression::independent);
    algebraic_g  dvar = +*expression::independent;

    // The sum we build
    algebraic_g result;

    // Loop on all arguments
    while (sp < sz)
    {
        object_p    si  = so + sp;
        algebraic_g arg = si->as_algebraic();
        if (!arg)
            return nullptr;

        // Function name for the partial term
        symbol_g st = sd;
        if (sn != 2)
        {
            // Build a function name like F′₁₅ for 15'th argument derivative
            static cstring lower_digits[10] =
            {
                "₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"
            };
            char buf[16];
            snprintf(buf, sizeof(buf), "%u", ++aidx);
            for (cstring p = buf; *p; p++)
                st = st + symbol::make(lower_digits[*p - '0']);
        }
        if (!st)
            return nullptr;

        // Differentiate that term
        algebraic_g dterm = expression::make(object::ID_Derivative, arg, dvar);

        // Build a copy of the source function call
        scribble scr;
        size_t a = 0;
        for (object_p obj : *sg)
        {
            if (++a == nobj)
                obj = +st;
            if (!rt.append(obj))
                return nullptr;
        }
        gcbytes bytes = scr.scratch();
        size_t size = scr.growth();
        algebraic_g dfunc = rt.make<funcall>(bytes, size);
        dterm = dterm * dfunc;
        if (!dterm)
            return nullptr;
        result = result ? result + dterm : dterm;
        if (!result)
            return nullptr;

        sp += si->size();
    }

    return result;
}


expression_p expression::derivative(symbol_r sym) const
// ----------------------------------------------------------------------------
//   Compute the derivative of the
// ----------------------------------------------------------------------------
{
    save<symbol_g *>       sindep(independent, (symbol_g *) &sym);
    save<object_g *>       sindval(independent_value, nullptr);
    save<uint>             sconstant(constant_index, 0);
    save<bool>             usave(unit::nodates, true);
    save<funcall_match_fn> smatch(funcall_match, derivative_funcall_match);
    save<funcall_build_fn> sbuild(funcall_build, derivative_funcall_build);
    expression_g           eq = this;
    eq = expression::make(ID_Derivative, algebraic_g(eq), algebraic_g(sym));
    expression_g result = eq->rewrites(
        P>>indep,               zero, // Expression not containing the variable

        (S(X))>>indep,          S(X), // Special handling for this form

        indep>>indep,           one,
        (X + Y)>>indep,         (X>>indep)+(Y>>indep),
        (X - Y)>>indep,         (X>>indep)-(Y>>indep),
        (X * Y)>>indep,         X*(Y>>indep) + (X>>indep)*Y,
        (X / Y)>>indep,         ((X>>indep)*Y-X*(Y>>indep))/sq(Y),
        (X ^ A)>>indep,         A*(X^(A-one)) * (X>>indep),

        A+B,                    A+B,
        A-B,                    A-B,
        A*B,                    A*B,
        A/B,                    A/B,
        A^B,                    A^B,
        zero*X,                 zero,
        X*zero,                 zero,
        zero/X,                 zero,
        zero+X,                 X,
        X+zero,                 X,
        zero-X,                 -X,
        X-zero,                 X,
        X*one,                  X,
        one*X,                  X,
        X/one,                  X,
        one/X,                  inv(X),
        X^zero,                 one,
        zero^X,                 zero,
        X^one,                  X,
        one^X,                  one,

        (X ^ E)>>indep,         (X^E)*((E>>indep)*ln(X) + ((X>>indep)*E)/X),

        (-X)>>indep,            -(X>>indep),
        inv(X)>>indep,          -(X>>indep) / sq(X),
        abs(X)>>indep,          (X>>indep)*sign(X),
        sign(X)>>indep,         zero,

        sin(X)>>indep,          (X>>indep)*cos(X),
        cos(X)>>indep,          -(X>>indep)*sin(X),
        tan(X)>>indep,          (X>>indep)/sq(cos(x)),
        sinh(X)>>indep,         (X>>indep)*cosh(X),
        cosh(X)>>indep,         (X>>indep)*sinh(X),
        tanh(X)>>indep,         (X>>indep)/sq(cosh(X)),

        asin(X)>>indep,         (X>>indep)/sqrt(one-sq(X)),
        acos(X)>>indep,         -(X>>indep)/sqrt(one-sq(X)),
        atan(X)>>indep,         (X>>indep)/(one+sq(X)),
        asinh(X)>>indep,        (X>>indep)/sqrt(one+sq(X)),
        acosh(X)>>indep,        (X>>indep)/sqrt(sq(X)-one),
        atanh(X)>>indep,        (X>>indep)/(one-sq(X)),

        ln(X)>>indep,           (X>>indep)/X,
        exp(X)>>indep,          (X>>indep)*exp(X),
        log2(X)>>indep,         (X>>indep)/(ln(two)*X),
        exp2(X)>>indep,         ln(two)*(X>>indep)*exp2(X),
        log10(X)>>indep,        (X>>indep)/(ln(ten)*X),
        exp10(X)>>indep,        ln(ten)*(X>>indep)*exp10(X),
        ln1p(X)>>indep,         (X>>indep)/(X+one),
        expm1(X)>>indep,        (X>>indep)*exp(X),

        sq(X)>>indep,           two*X*(X>>indep),
        sqrt(X)>>indep,         (X>>indep)/(two * sqrt(X)),
        cubed(X)>>indep,        three*sq(X)*(X>>indep),
        cbrt(X)>>indep,         (X>>indep)/(three*(sq(cbrt(X))))
        );

    bool unknown = +result == +eq;
    if (result)
    {
        if (!unknown)
        {
            expression_g hasder = result->rewrites(X>>indep, X);
            unknown = hasder && +hasder != +result;
        }
        if (unknown)
        {
            rt.unknown_derivative_error();
            return nullptr;
        }
        if (Settings.AutoSimplify())
            result = result->simplify();
    }
    return result;
}


COMMAND_BODY(Derivative)
// ----------------------------------------------------------------------------
//   Compute the derivative of an expression
// ----------------------------------------------------------------------------
{
    return expression::variable_command(&expression::derivative);
}


PARSE_BODY(Derivative)
// ----------------------------------------------------------------------------
//   The syntax for derivative is something like ∂X(sin X)
// ----------------------------------------------------------------------------
{
    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = 0;

    // First character must be a constant marker
    unicode cp = utf8_codepoint(source);
    if (cp != L'∂')
        return SKIP;
    parsed = utf8_next(source, parsed, max);

    // In command mode, just return the Derivative object
    if (!p.precedence)
    {
        p.length = parsed;
        p.out = object::static_object(ID_Derivative);
        return p.out ? OK : ERROR;
    }

    // Parse the name
    parser namep(p, source + parsed, LOWEST);
    result nres = symbol::do_parse(namep);
    if (nres != OK)
        return nres;
    algebraic_g name = symbol_p(+namep.out);
    ASSERT(name->type() == ID_symbol);

    // Parse the expression
    parser exprp(p, source + parsed + namep.length, LOWEST);
    result eres = expression::list_parse(ID_expression, exprp, '(', ')');
    if (eres != OK)
        return eres;
    algebraic_g expr = expression_p(+exprp.out);

    algebraic_g args[2] = { name, expr };
    expression_g res = expression::make(ID_Derivative, args, 2,
                                        ID_expression, true);
    p.out = +res;
    p.length = parsed + namep.length + exprp.length;
    return p.out ? OK : ERROR;
}


INSERT_BODY(Derivative)
// ----------------------------------------------------------------------------
//   Insert the derivative symbol
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, "", "", false);
}



// ============================================================================
//
//   Primitive
//
// ============================================================================

expression_p expression::primitive(symbol_r sym) const
// ----------------------------------------------------------------------------
//   Compute the primitive of the
// ----------------------------------------------------------------------------
{
    save<symbol_g *>       sindep(independent, (symbol_g *) &sym);
    save<object_g *>       sindval(independent_value, nullptr);
    save<uint>             sconstant(constant_index, 0);
    save<bool>             usave(unit::nodates, true);
    expression_g           eq = this;
    eq = expression::make(ID_Primitive, algebraic_g(eq), algebraic_g(sym));
    expression_g result = eq->rewrites(
        P<<indep,               P*indep,

        indep<<indep,                   sq(indep)/two,
        (-X)<<indep,                    -(X<<indep),
        (X + Y)<<indep,                 (X<<indep)+(Y<<indep),
        (X - Y)<<indep,                 (X<<indep)-(Y<<indep),
        (P * X)<<indep,                 P*(X<<indep),
        (X * P)<<indep,                 P*(X<<indep),
        (X / P)<<indep,                 (X<<indep)/P,
        (indep ^ mone)<<indep,          ln(indep),
        (indep ^ P)<<indep,             (indep^(P+one)) / (P+one),

        A+B,                            A+B,
        A-B,                            A-B,
        A*B,                            A*B,
        A/B,                            A/B,
        A^B,                            A^B,
        X/A/B,                          X/(A*B),
        P*N*Q,                          (P*Q)*N,
        P*(N*Q),                        (P*Q)*N,
        P*N/Q,                          (P/Q)*N,
        P*(N/Q),                        (P/Q)*N,
        zero*X,                         zero,
        X*zero,                         zero,
        zero+X,                         X,
        X+zero,                         X,
        X*one,                          X,
        X/one,                          X,
        one*X,                          X,
        one/X,                          inv(X),
        X^mone,                         inv(X),
        A/X,                            A*inv(X),
        X^zero,                         one,
        X^one,                          X,
        inv(X^A),                       X^-A,
        (X^A)^B,                        X^(A*B),
        X*X*X,                          cubed(X),
        X*sq(X),                        cubed(X),
        sq(X)*X,                        cubed(X),
        X*X,                            sq(X),
        X^two,                          sq(X),
        X^three,                        cubed(X),

        // Patterns below in the order of section E-2 of HP50G ARM
        acos(L)<<indep,                 (L*acos(L)-sqrt(one-sq(L)))/A,
        acosh(L)<<indep,                (L*acosh(L)-sqrt(sq(L)-one))/A,
        asin(L)<<indep,                 (L*asin(L)+sqrt(one-sq(L)))/A,
        asinh(L)<<indep,                (L*asinh(L)-sqrt(one+sq(L)))/A,
        atan(L)<<indep,                 (L*atan(L)-ln(one+sq(L))/two)/A,
        atanh(L)<<indep,                (L*atan(L)-ln(one-sq(L))/two)/A,
        cos(L)<<indep,                  sin(L)/A,
        inv(cos(L))<<indep,             ln(abs(tan(L)+inv(cos(L))))/A,
        inv(cosh(L))<<indep,            atan(sinh(L))/A,
        inv(sin(L))<<indep,             ln(abs(tan(L/two)))/A,
        inv(sinh(L))<<indep,            ln(abs(tanh(L/two)))/A,
        inv(cos(L)*sin(L))<<indep,      ln(tan(L))/A,
        cosh(L)<<indep,                 sinh(L)/A,
        inv(cosh(L)*sinh(L))<<indep,    ln(tan(L))/A,
        inv(sinh(L)*cosh(L))<<indep,    ln(tan(L))/A,
        inv(sq(cosh(L)))<<indep,        tanh(L)/A,
        exp(L)<<indep,                  exp(L)/A,
        exp10(L)<<indep,                exp10(L)/(A*ln(ten)),
        exp2(L)<<indep,                 exp2(L)/(A*ln(two)),
        expm1(L)<<indep,                (expm1(L)-L+one)/A,
        ln(L)<<indep,                   (L*ln(L)-L)/A,
        log10(L)<<indep,                (L*log10(L)-L/ln(ten))/A,
        log2(L)<<indep,                 (L*log2(L)-L/ln(two))/A,
        ln1p(L)<<indep,                 ((L-one)*ln1p(L)-(L-one))/A,
        sign(L)<<indep,                 abs(L)/A,
        sin(L)<<indep,                  -cos(L)/A,
        inv(sin(L)*cos(L))<<indep,      ln(tan(L))/A,
        inv(sin(L)*tan(L))<<indep,      -inv(sin(L))/A,
        inv(sq(sin(L)))<<indep,         -inv(tan(L))/A,
        sinh(L)<<indep,                 cosh(L)/A,
        inv(sinh(L)*cosh(L))<<indep,    ln(tanh(L))/A,
        inv(sinh(L)*tanh(L))<<indep,    -inv(sinh(L))/A,
        (sq(tan(L)))<<indep,            (tan(L)-L)/A,
        tan(L)<<indep,                  -ln(cos(L))/A,
        (tan(L)/cos(L))<<indep,         inv(cos(L))/A,
        inv(tan(L))<<indep,             ln(sin(L))/A,
        inv(tan(L)*sin(L))<<indep,      -inv(sin(L))/A,
        tanh(L)<<indep,                 ln(cosh(L))/A,
        (tanh(L)/cosh(L))<<indep,       inv(cosh(L))/A,
        inv(tanh(L))<<indep,            ln(sinh(L))/A,
        inv(tanh(L)*sinh(L))<<indep,    -inv(sinh(L))/A,
        (L^zero)<<indep,                L/A,
        (P/L)<<indep,                   P*ln(abs(L))/A,
        ((P+N)/L)<<indep,               P*ln(abs(L))/A+((N/L)<<indep),
        ((N+P)/L)<<indep,               P*ln(abs(L))/A+((N/L)<<indep),
        ((P*indep)/L)<<indep,           (P*(A*indep+B-B*ln(abs(A*indep+B))))/sq(A),
        (P^L)<<indep,                   (P^L)/(A*ln(P)),
        inv(L)<<indep,                  ln(abs(L))/A,
        inv(one-(sq(L)))<<indep,        atanh(L)/A,
        inv(one+(sq(L)))<<indep,        atan(L)/A,
        inv(sqrt((sq(L))-one))<<indep,  acosh(L)/A,
        inv(sqrt(one-(sq(L))))<<indep,  asin(L)/A,
        inv(sqrt(one+(sq(L))))<<indep,  asinh(L)/A,
        inv((sqrt(sq(L))+one))<<indep,  asinh(L)/A,
        sq(L)<<indep,                   cubed(L)/(three*A),
        cubed(L)<<indep,                (L^four)/(four*A),
        sqrt(L)<<indep,                 ((two/three)*cubed(sqrt(L)))/A,
        inv(sqrt(L))<<indep,            two*sqrt(L)/A,
        cbrt(L)<<indep,                 ((three/four)*(cbrt(L)^four))/A,
        inv(cbrt(L))<<indep,            ((three/two)*sq(cbrt(L)))/A
        );

    bool unknown = +result == +eq;
    if (result)
    {
        if (!unknown)
        {
            expression_g hasprim = result->rewrites(X<<indep, X);
            unknown = hasprim && +hasprim != +result;
        }
        if (unknown)
        {
            rt.unknown_primitive_error();
            return nullptr;
        }
        if (Settings.AutoSimplify())
            result = result->simplify();
    }
    return result;
}


COMMAND_BODY(Primitive)
// ----------------------------------------------------------------------------
//   Compute the primitive of an expression
// ----------------------------------------------------------------------------
{
    return expression::variable_command(&expression::primitive);
}


PARSE_BODY(Primitive)
// ----------------------------------------------------------------------------
//   The syntax for primitive is something like ∫X(sin X)
// ----------------------------------------------------------------------------
{
    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = 0;

    // First character must be a constant marker
    unicode cp = utf8_codepoint(source);
    if (cp != L'∫')
        return SKIP;
    parsed = utf8_next(source, parsed, max);

    // If it's not in the form ∫X, then just return the Integrate object
    if (!p.precedence || parsed >= max ||
        !is_valid_as_name_initial(utf8_codepoint(source + parsed)))
    {
        p.length = parsed;
        p.out = object::static_object(ID_Integrate);
        return p.out ? OK : ERROR;
    }

    // Parse the name
    parser namep(p, source + parsed, LOWEST);
    result nres = symbol::do_parse(namep);
    if (nres != OK)
        return nres;
    algebraic_g name = symbol_p(+namep.out);
    ASSERT(name->type() == ID_symbol);

    // Parse the expression
    parser exprp(p, source + parsed + namep.length, LOWEST);
    result eres = expression::list_parse(ID_expression, exprp, '(', ')');
    if (eres != OK)
        return eres;
    algebraic_g expr = expression_p(+exprp.out);

    algebraic_g args[2] = { name, expr };
    expression_g res = expression::make(ID_Primitive, args, 2,
                                        ID_expression, true);
    p.out = +res;
    p.length = parsed + namep.length + exprp.length;
    return p.out ? OK : ERROR;
}


INSERT_BODY(Primitive)
// ----------------------------------------------------------------------------
//   Insert the primitive symbol
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, "", "", false);
}
