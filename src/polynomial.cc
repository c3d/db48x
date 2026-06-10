// ****************************************************************************
//  polynomial.c                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Dense representation of multivariate polynomials
//
//    Some operations on polynomials are much easier or faster if done
//    with a numerical representation of the coefficients.
//    We choose a dense representation here in line with the primary objective
//    of DB48X to run on very memory-constrainted machines like the DM42
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "polynomial.h"

#include "arithmetic.h"
#include "array.h"
#include "compare.h"
#include "complex.h"
#include "decimal.h"
#include "expression.h"
#include "factor.h"
#include "grob.h"
#include "integer.h"
#include "leb128.h"
#include "fraction.h"
#include "list.h"
#include "parser.h"
#include "runtime.h"
#include "settings.h"
#include "functions.h"
#include "unit.h"
#include "variables.h"
#include "util.h"

#include <cstdlib>


polynomial_p polynomial::make(algebraic_p value)
// ----------------------------------------------------------------------------
//   Convert a value into an algebraic with zero variables
// ----------------------------------------------------------------------------
{
    if (!value || value->type() == ID_polynomial)
        return polynomial_p(value);

    if (expression_g expr = value->as<expression>())
    {
        value = nullptr;
        if (object_p quoted = expr->as_quoted())
            if (algebraic_p alg = quoted->as_algebraic())
                value = alg;
        if (!value)
            return make(expr);
    }

    if (symbol_g sym = value->as<symbol>())
        return make(sym);
    if (!value->is_numeric_constant())
        return nullptr;

    // Case where we have a numerical constant
    scribble    scr;
    algebraic_g avalue = value;
    size_t      sz     = value->size();
    byte       *p      = rt.allocate(1 + sz);
    if (!p)
        return nullptr;
    *p++ = 0; // Number of variables = 0
    memcpy(p, +avalue, sz);
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::make(symbol_p name)
// ----------------------------------------------------------------------------
//   Convert a name into an algebraic with a single variable
// ----------------------------------------------------------------------------
{
    if (!name || name->type() != ID_symbol)
        return nullptr;

    scribble scr;
    symbol_g aname  = name;
    byte_p   src    = name->payload();
    byte_p   p      = src;
    size_t   len    = leb128<size_t>(p);
    size_t   namesz = p + len - src;
    size_t   polysz = namesz + integer::required_memory(ID_integer, 1) + 2;
    byte    *dst    = rt.allocate(polysz);
    if (!dst)
        return nullptr;
    dst = leb128(dst, 1);     // Number of variables = 1
    memcpy(dst, src, namesz); // Copy name
    dst += namesz;
    dst = leb128(dst, ID_integer); // Encode constant 1 (scaling factor)
    dst = leb128(dst, 1);
    dst = leb128(dst, 1); // Encode exponent 1
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::make(algebraic_r factor, symbol_r sym, ularge exp)
// ----------------------------------------------------------------------------
//   Convert a value into an algebraic with zero variables
// ----------------------------------------------------------------------------
{
    if (!factor || !sym)
        return nullptr;
    if (exp == 0)
        return make(factor);

    // Case where we have a numerical constant
    scribble scr;
    size_t   len = sym->length();
    size_t   fsz = factor->size();
    size_t   asz = 1 + fsz + len + leb128size(len) + leb128size(exp);
    byte    *p   = rt.allocate(asz);
    if (!p)
        return nullptr;
    *p++ = 1; // Number of variables = 1
    p = leb128(p, len);
    memcpy(p, sym->value(), len);
    p += len;
    memcpy(p, +factor, fsz);
    p += fsz;
    p = leb128(p, exp);
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


static bool polynomial_op(size_t depth, polynomial_p (*op)(polynomial_r x))
// ----------------------------------------------------------------------------
//   Unary operation
// ----------------------------------------------------------------------------
{
    if (rt.depth() - depth >= 1)
        if (polynomial_g arg = rt.top()->as<polynomial>())
            if (polynomial_p result = op(arg))
                if (rt.top(result))
                    return true;
    return false;
}


static bool polynomial_op(size_t depth,
                          polynomial_p (*op)(polynomial_r x, polynomial_r y))
// ----------------------------------------------------------------------------
//   Binary operation
// ----------------------------------------------------------------------------
{
    if (rt.depth() - depth >= 2)
        if (polynomial_g x = rt.pop()->as<polynomial>())
            if (polynomial_g y = rt.top()->as<polynomial>())
                if (polynomial_p result = op(y, x))
                    if (rt.top(result))
                        return true;
    return false;
}


static bool polynomial_op(size_t depth,
                          polynomial_p (*op)(polynomial_r y, integer_r x),
                          integer_r xi)
// ----------------------------------------------------------------------------
//   Binary power operation
// ----------------------------------------------------------------------------
{
    if (xi)
        if (rt.depth() - depth >= 2)
            if (polynomial_g x = rt.pop()->as<polynomial>())
                if (polynomial_g y = rt.top()->as<polynomial>())
                    if (polynomial_p result = op(y, xi))
                        if (rt.top(result))
                            return true;
    return false;
}


static bool polynomial_op(size_t depth,
                          polynomial_p (*op)(polynomial_r y, ularge x),
                          ularge xi)
// ----------------------------------------------------------------------------
//   Binary power operation
// ----------------------------------------------------------------------------
{
    if (rt.depth() - depth >= 1)
        if (polynomial_g y = rt.top()->as<polynomial>())
            if (polynomial_p result = op(y, xi))
                if (rt.top(result))
                    return true;
    return false;
}


polynomial_p polynomial::make(expression_p expr, bool error)
// ----------------------------------------------------------------------------
//   Check if an expression has the right structure for a polynomial
// ----------------------------------------------------------------------------
{
    // If the expression is already a polynomial, return it
    if (!expr || expr->type() == ID_polynomial)
        return polynomial_p(expr);
    if (expr->type() != ID_expression)
    {
        if (error)
            rt.type_error();
        return nullptr;
    }

    // First check that what we have is compatible with expectations
    size_t    depth = rt.depth();
    integer_g power = nullptr;
    for (object_p obj : *expr)
    {
        ASSERT(obj && "We must have valid objects in expressions");
        id ty = obj->type();

        // Save integer exponents for `pow`
        if (ty == ID_integer)
            power = integer_p(obj);
        else if (ty != ID_pow)
            power = nullptr;

        // Check which types are valid in a polynomial
        if (is_real(ty) || (ty == ID_polar || ty == ID_rectangular))
        {
            algebraic_g  arg  = algebraic_p(obj);
            polynomial_g poly = make(arg);
            if (!poly)
                goto error;
            rt.push(+poly);
        }
        else if (ty == ID_symbol)
        {
            symbol_g     sym  = symbol_p(obj);
            polynomial_g poly = make(sym);
            if (!poly)
                goto error;
            rt.push(+poly);
        }
        else if (ty == ID_neg)
        {
            if (!polynomial_op(depth, neg))
                goto error;
        }
        else if (ty == ID_add)
        {
            if (!polynomial_op(depth, add))
                goto error;
        }
        else if (ty == ID_subtract)
        {
            if (!polynomial_op(depth, sub))
                goto error;
        }
        else if (ty == ID_multiply)
        {
            if (!polynomial_op(depth, mul))
                goto error;
        }
        else if (ty == ID_pow)
        {
            if (!polynomial_op(depth, pow, power))
                goto error;
        }
        else if (ty == ID_sq)
        {
            if (!polynomial_op(depth, pow, 2))
                goto error;
        }
        else if (ty == ID_cubed)
        {
            if (!polynomial_op(depth, pow, 3))
                goto error;
        }
        else if (ty == ID_sqrt)
        {
            bool ok = false;
            if (object_p pobj = rt.top())
                if (polynomial_p poly = pobj->as<polynomial>())
                    if (poly->variables() == 0)
                        if (algebraic_p expr = poly->as_expression())
                            if (algebraic_g v = expr->evaluate())
                                if (algebraic_g s = sqrt::run(v))
                                    if (polynomial_p repl = make(s))
                                        ok = rt.top(repl);
            if (!ok)
            {
                rt.value_error();
                goto error;
            }
        }
        else
        {
            // All other operators are invalid in a polynom
            if (error)
                rt.value_error();
            goto error;
        }
    }

    if (rt.depth() == depth + 1)
        if (polynomial_p result = rt.pop()->as<polynomial>())
            return result;

error:
    // Case where we had an error: drop anything we pushed on the stack
    if (size_t removing = rt.depth() - depth)
        rt.drop(removing);
    return nullptr;
}


byte *polynomial::copy_variables(polynomial_r x, byte *prev)
// ----------------------------------------------------------------------------
//   Copy variables from an existing polynomial, return pointer at end
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    gcmbytes gprev  = prev;
    size_t   ovars  = prev ? leb128<size_t>(prev) : 0;
    size_t   ovoffs = prev - +gprev;

    byte_p   xp     = x->payload();
    size_t   xsz    = leb128<size_t>(xp);
    size_t   nvars  = leb128<size_t>(xp);
    size_t   offset = xp - byte_p(+x);

    // Insert variables in copy
    for (size_t v = 0; v < nvars; v++)
    {
        if (offset >= xsz)
            return nullptr;

        // Scan next variable in polynomial x
        xp          = byte_p(+x) + offset;
        size_t vlen = leb128<size_t>(xp);

        // Check if a copy of that variable already exists
        byte_p old  = nullptr;
        int    cmp  = -1;
        if (prev)
        {
            // Restart from beginning of variables
            prev = gprev + ovoffs;
            for (size_t ov = 0; ov < ovars; ov++)
            {
                byte_p oldvar = prev;
                size_t ovlen  = leb128<size_t>(prev);
                cmp = symbol::compare(prev, xp, std::min(ovlen, vlen));
                if (cmp >= 0)
                {
                    old = oldvar;
                    if (cmp == 0)
                        cmp = ovlen - vlen;
                    break;
                }
                prev += ovlen;
            }
        }

        size_t vsz = leb128size(vlen) + vlen;
        if (cmp)
        {
            // Size needed for variable
            size_t offs   = old - +gprev;
            bool   vszchg = !prev || leb128size(ovars + 1) != leb128size(ovars);
            byte  *copy   = rt.allocate(vsz + vszchg);
            if (!copy)
                return nullptr;
            ovars++;
            if (!prev)
            {
                gprev = prev = copy;
                copy         = (byte *) leb128(+gprev, ovars);
            }
            else
            {
                if (vszchg)
                    memmove((byte *) +gprev + 1, +gprev, copy - +gprev);
                leb128(+gprev, ovars);
            }
            if (!old)
            {
                memcpy(copy, byte_p(+x) + offset, vsz);
            }
            else
            {
                old           = +gprev + offs;
                size_t copysz = copy - old;
                memmove((byte *) old + vsz, old, copysz);
                memcpy((byte *) old, byte_p(+x) + offset, vsz);
            }
        }
        offset += vsz;
    }

    if (!gprev)
    {
        byte *p = rt.allocate(1);
        if (p)
            *p = 0;
        gprev = p;
    }

    return (byte *) +gprev;
}


polynomial_p polynomial::neg(polynomial_r x)
// ----------------------------------------------------------------------------
//  Negate a polynomial by negating the constant in all terms
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;

    scribble scr;
    gcbytes  polycopy = copy_variables(x);
    size_t   nvars    = x->variables();
    for (auto term : *x)
    {
        algebraic_g factor = term.factor();
        factor             = -factor;
        size_t sz          = factor->size();
        byte  *np          = rt.allocate(sz);
        if (!np)
            return nullptr;
        memcpy(np, +factor, sz);
        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = term.exponent();
            byte  *ep       = rt.allocate(leb128size(exponent));
            if (!ep)
                return nullptr;
            leb128(ep, exponent);
        }
    }
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::addsub(polynomial_r x, polynomial_r y, bool sub)
// ----------------------------------------------------------------------------
//  Add or subtract two polynomials
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    scribble scr;
    gcbytes  result = copy_variables(x);
    if (!result)                // Special case of empty x
        rt.free(scr.growth());
    result          = copy_variables(y, (byte *) +result);
    if (!result)
        return nullptr;

    byte_p p     = +result;
    size_t nvars = leb128<size_t>(p);
    size_t xvars = x->variables();
    size_t yvars = y->variables();
    ularge xexp[nvars];
    ularge yexp[nvars];
    size_t xvar[xvars];
    size_t yvar[yvars];

    // Map variables in x and y to variables in the result
    for (size_t v = 0; v < nvars; v++)
    {
        size_t nlen = leb128<size_t>(p);
        for (size_t xv = 0; xv < xvars; xv++)
        {
            size_t xlen  = 0;
            utf8   xname = x->variable(xv, &xlen);
            if (xlen == nlen && symbol::compare(xname, p, xlen) == 0)
                xvar[xv] = v;
        }
        for (size_t yv = 0; yv < yvars; yv++)
        {
            size_t ylen  = 0;
            utf8   yname = y->variable(yv, &ylen);
            if (ylen == nlen && symbol::compare(yname, p, ylen) == 0)
                yvar[yv] = v;
        }
        p += nlen;
    }

    // Add all the terms in X
    for (auto xterm : *x)
    {
        for (size_t v = 0; v < nvars; v++)
            xexp[v] = 0;

        // Computer the factor of the variables in polynomial x
        algebraic_g xfactor = xterm.factor();
        for (size_t xv = 0; xv < xvars; xv++)
            xexp[xvar[xv]] = xterm.exponent();

        // Check if we have the same factors in polynomial y
        for (auto yterm : *y)
        {
            for (size_t v = 0; v < nvars; v++)
                yexp[v] = 0;

            algebraic_g yfactor = yterm.factor();
            for (size_t yv = 0; yv < yvars; yv++)
                yexp[yvar[yv]] = yterm.exponent();

            bool sameexps = true;
            for (size_t v = 0; sameexps && v < nvars; v++)
                sameexps = xexp[v] == yexp[v];
            if (sameexps)
                xfactor = sub ? xfactor - yfactor : xfactor + yfactor;
        }
        if (!xfactor)
            return nullptr;
        if (!xfactor->is_zero(false))
        {
            size_t sz = xfactor->size();
            byte  *p  = rt.allocate(sz);
            if (!p)
                return nullptr;
            memcpy(p, +xfactor, sz);
            p += sz;
            for (size_t v = 0; v < nvars; v++)
            {
                p = rt.allocate(leb128size(xexp[v]));
                if (!p)
                    return nullptr;
                leb128(p, xexp[v]);
            }
        }
    }

    // Add all the terms in Y
    for (auto yterm : *y)
    {
        for (size_t v = 0; v < nvars; v++)
            yexp[v] = 0;

        // Compute the factor of the variables in polynomial y
        algebraic_g yfactor = yterm.factor();
        for (size_t yv = 0; yv < yvars; yv++)
            yexp[yvar[yv]] = yterm.exponent();

        // Check if we have the same factors in polynomial X
        for (auto xterm : *x)
        {
            for (size_t v = 0; v < nvars; v++)
                xexp[v] = 0;

            algebraic_g xfactor = xterm.factor();
            for (size_t xv = 0; xv < xvars; xv++)
                xexp[xvar[xv]] = xterm.exponent();

            bool sameexps = true;
            for (size_t v = 0; sameexps && v < nvars; v++)
                sameexps = xexp[v] == yexp[v];
            if (sameexps)
                yfactor = nullptr; // Already done in the X loop
        }

        if (yfactor && !yfactor->is_zero(false))
        {
            if (sub)
                yfactor = -yfactor;

            size_t sz = yfactor->size();
            byte  *p  = rt.allocate(sz);
            if (!p)
                return nullptr;
            memcpy(p, +yfactor, sz);
            p += sz;
            for (size_t v = 0; v < nvars; v++)
            {
                p = rt.allocate(leb128size(yexp[v]));
                if (!p)
                    return nullptr;
                leb128(p, yexp[v]);
            }
        }
    }

    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::add(polynomial_r x, polynomial_r y)
// ----------------------------------------------------------------------------
//  Add two polynomials
// ----------------------------------------------------------------------------
{
    return addsub(x, y, false);
}


polynomial_p polynomial::sub(polynomial_r x, polynomial_r y)
// ----------------------------------------------------------------------------
//  Subtract two polynomials
// ----------------------------------------------------------------------------
{
    return addsub(x, y, true);
}


polynomial_p polynomial::mul(polynomial_r x, polynomial_r y)
// ----------------------------------------------------------------------------
//   Multiply two polynomials
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    scribble scr;
    gcbytes  result = copy_variables(x);
    if (!result)
        rt.free(scr.growth());
    result          = copy_variables(y, (byte *) +result);
    if (!result)
        return nullptr;

    byte_p p     = +result;
    size_t nvars = leb128<size_t>(p);
    size_t xvars = x->variables();
    size_t yvars = y->variables();
    ularge xexp[nvars];
    ularge yexp[nvars];
    size_t xvar[xvars];
    size_t yvar[yvars];

    // Map variables in x and y to variables in the result
    for (size_t v = 0; v < nvars; v++)
    {
        size_t nlen = leb128<size_t>(p);
        for (size_t xv = 0; xv < xvars; xv++)
        {
            size_t xlen  = 0;
            utf8   xname = x->variable(xv, &xlen);
            if (xlen == nlen && symbol::compare(xname, p, xlen) == 0)
                xvar[xv] = v;
        }
        for (size_t yv = 0; yv < yvars; yv++)
        {
            size_t ylen  = 0;
            utf8   yname = y->variable(yv, &ylen);
            if (ylen == nlen && symbol::compare(yname, p, ylen) == 0)
                yvar[yv] = v;
        }
        p += nlen;
    }

    // Loop over all the terms in X
    gcbytes terms = p;
    for (auto xterm : *x)
    {
        for (size_t v = 0; v < nvars; v++)
            xexp[v] = 0;

        // Computer the factor of the variables in polynomial x
        algebraic_g xfactor = xterm.factor();
        for (size_t xv = 0; xv < xvars; xv++)
            xexp[xvar[xv]] = xterm.exponent();

        // Check if we have the same factors in polynomial y
        for (auto yterm : *y)
        {
            for (size_t v = 0; v < nvars; v++)
                yexp[v] = 0;

            algebraic_g yfactor = yterm.factor();
            for (size_t yv = 0; yv < yvars; yv++)
                yexp[yvar[yv]] = yterm.exponent();

            algebraic_g rfactor = xfactor * yfactor;
            if (!rfactor)
                return nullptr;
            if (!rfactor->is_zero(false))
            {
                // Check if there is an existing term with same exponents
                gcbytes end = rt.allocate(0);
                byte_p next = end;
                for (byte_p check = terms; check < end; check = next)
                {
                    algebraic_g existing = algebraic_p(check);
                    bool sameexps = true;
                    byte_p expp = byte_p(existing->skip());
                    for (size_t v = 0; v < nvars; v++)
                    {
                        ularge eexp = leb128<size_t>(expp);
                        if (eexp != xexp[v] + yexp[v])
                            sameexps = false;
                    }
                    next = expp;
                    if (sameexps)
                    {
                        size_t remove = size_t(expp - check);
                        rfactor = rfactor + existing;
                        memmove((byte *) +existing,
                                byte_p(existing) + remove,
                                end - byte_p(+existing));
                        rt.free(remove);
                        break;
                    }
                }
            }

            if (!rfactor->is_zero(false))
            {
                size_t sz = rfactor->size();
                byte  *p  = rt.allocate(sz);
                if (!p)
                    return nullptr;
                memcpy(p, +rfactor, sz);
                p += sz;
                for (size_t v = 0; v < nvars; v++)
                {
                    ularge exp = xexp[v] + yexp[v];
                    p = rt.allocate(leb128size(exp));
                    p = leb128(p, exp);
                }
            }
        }
    }

    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::div(polynomial_r x, polynomial_r y)
// ----------------------------------------------------------------------------
//  Euclidean divide of polynomials
// ----------------------------------------------------------------------------
{
    polynomial_g q, r;
    if (quorem(x, y, q, r))
        return q;
    return nullptr;
}


polynomial_p polynomial::mod(polynomial_r x, polynomial_r y)
// ----------------------------------------------------------------------------
//  Euclidean remainder of polynomials
// ----------------------------------------------------------------------------
{
    polynomial_g q, r;
    if (quorem(x, y, q, r))
        return r;
    return nullptr;
}


bool polynomial::quorem(polynomial_r  x,
                        polynomial_r  y,
                        polynomial_g &q,
                        polynomial_g &r)
// ----------------------------------------------------------------------------
//  Quotient and remainder of two polynomials
// ----------------------------------------------------------------------------
//  The quotient is computed based on the polynomial::main_variable
//
//  Consider x = A^3-B^3 and y=A-B
//  We start with q=0, r=(A^3-B^3) and y=A-B
//
//      q               r               high R  ratio HR/HY     prod
//
//      0               A^3-B^3         A^3     A^2             A^3-A^2*B
//      A^2             A^2*B-B^3       A^2*B   A*B             A^2*B-A*B^2
//      A^2+A*B         A*B^2-B^3       A*B^2   B^2             A*B^2-B^3
//      A^2+A*B+B^2     0               0
//
//
//  Consider x=A^3+B^3 and y=A-B
//      q               r               high R  ratio HR/HY     prod
//
//      0               A^3+B^3         A^3     A^2             A^3+A^2*B
//      A^2             A^2*B+B^3       A^2*B   A*B             A^2*B+A*B^2
//      A^2+A*B        -A*B^2+B^3      -A*B^2  -B^2             -A*B^2+B^3
//      A^2+A*B+B^2     2*B^3           0
//
{
    if (!x || !y)
        return false;

    // Initial remainder and quotient
    r = x;
    q = polynomial::make(integer::make(0));
    if (!q)
        return false;

    // Find highest rank in the terms
    symbol_g     var    = main_variable();
    size_t       rvar   = r->variable(+var);
    size_t       yvar   = y->variable(+var);
    iterator     ri     = r->ranking(rvar);
    iterator     yi     = y->ranking(yvar);
    ularge       rorder = ri.rank(rvar);
    ularge       yorder = yi.rank(yvar);

    symbol_g rvars[ri.variables];
    for (size_t rv = 0; rv < ri.variables; rv++)
        rvars[rv] = r->variable(rv);

    while (rorder >= yorder && yi != y->end())
    {
        iterator     yterm = yi;
        algebraic_g  yf    = yterm.factor();

        // Compute term factor for ratio of highest-ranking terms in var
        polynomial_g rpoly = polynomial::make(integer::make(0));
        for (auto rterm : *r)
        {
            algebraic_g  rf    = rterm.factor();
            polynomial_g ratio = polynomial::make(rf / yf);
            if (!ratio)
                return false;
            bool match = true;
            for (size_t rv = 0; rv < rterm.variables; rv++)
            {
                ularge rexp = rterm.exponent();
                if (rv == rvar)
                {
                    match = rexp == rorder;
                    rexp = rorder - yorder;
                }
                if (match)
                {
                    algebraic_g rf = integer::make(1);
                    polynomial_g rp = polynomial::make(rf, rvars[rv], rexp);
                    ratio = mul(ratio, rp);
                    if (!ratio)
                        return false;
                }
            }
            if (match)
            {
                rpoly = add(rpoly, ratio);
                if (!rpoly)
                    return false;
            }
        }
        q = add(q, rpoly);
        rpoly = mul(rpoly, y);
        r     = sub(r, rpoly);
        if (!r)
            return false;
        if (r->is_zero(false))
            break;

        // Restart with rest
        rvar = r->variable(+var);
        ri = r->ranking(rvar);
        rorder = ri.rank(rvar);
    }

    return true;
}


polynomial_p polynomial::pow(polynomial_r x, integer_r y)
// ----------------------------------------------------------------------------
//  Elevate a polynomial to some integer power
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;
    ularge exp = y->value<ularge>();
    return pow(x, exp);
}


polynomial_p polynomial::pow(polynomial_r x, ularge exp)
// ----------------------------------------------------------------------------
//  Elevate a polynomial to some integer power
// ----------------------------------------------------------------------------
{
    polynomial_g r   = nullptr;
    polynomial_g m   = x;
    while (exp)
    {
        if (exp & 1)
        {
            r = r ? mul(r, m) : +m;
            if (!r)
                return nullptr;
        }
        m = mul(m, m);
        if (!m)
            return nullptr;
        exp >>= 1;
    }

    if (!r)
    {
        algebraic_g one = integer::make(1);
        r               = polynomial::make(one);
    }
    return r;
}


size_t polynomial::variables() const
// ----------------------------------------------------------------------------
//   Return the number of variables
// ----------------------------------------------------------------------------
{
    byte_p first  = byte_p(this);
    byte_p p      = payload();
    size_t length = leb128<size_t>(p);
    size_t nvars  = leb128<size_t>(p);
    return (size_t(p - first) < length) ? nvars : 0;
}


symbol_p polynomial::variable(size_t index) const
// ----------------------------------------------------------------------------
//   Return the variable at the given index as a symbol
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8   p   = variable(index, &len);
    return symbol::make(p, len);
}


utf8 polynomial::variable(size_t index, size_t *len) const
// ----------------------------------------------------------------------------
//   Return the variable at the given index as a symbol
// ----------------------------------------------------------------------------
{
    byte_p first  = byte_p(this);
    byte_p p      = payload();
    size_t length = leb128<size_t>(p);
    size_t nvars  = leb128<size_t>(p);
    if (index >= nvars)
        return nullptr;

    for (size_t v = 0; v < index; v++)
    {
        size_t vlen = leb128<size_t>(p);
        p += vlen;
    }
    if (size_t(p - first) >= length)
        return nullptr;
    size_t vlen = leb128<size_t>(p);
    if (len)
        *len = vlen;
    return p;
}


size_t polynomial::variable(utf8 name, size_t len) const
// ----------------------------------------------------------------------------
//   Find a variable by name
// ----------------------------------------------------------------------------
{
    byte_p first  = byte_p(this);
    byte_p p      = payload();
    size_t length = leb128<size_t>(p);
    size_t nvars  = leb128<size_t>(p);

    for (size_t v = 0; v < nvars; v++)
    {
        size_t vlen = leb128<size_t>(p);
        if (vlen == len && symbol::compare(p, name, len) == 0)
            return v;
        p += vlen;
        if (size_t(p - first) >= length)
            break;
    }
    return ~0UL;
}


size_t polynomial::variable(symbol_p sym) const
// ----------------------------------------------------------------------------
//   Find a variable by name
// ----------------------------------------------------------------------------
{
    if (!sym)
        return ~0UL;
    size_t len  = 0;
    utf8   name = sym->value(&len);
    return variable(name, len);
}


ularge polynomial::order(size_t *var) const
// ----------------------------------------------------------------------------
//   Compute the order of a polynomial, as the highest exponent of any variable
// ----------------------------------------------------------------------------
{
    iterator    where   = ranking(var);
    size_t      mainvar = 0;
    ularge      maxexp  = 0;
    if (where != end())
    {

        algebraic_g factor  = where.factor();
        for (size_t v = 0; v < where.variables; v++)
        {
            ularge vexp = where.exponent();
            if (vexp > maxexp)
            {
                maxexp = vexp;
                mainvar = v;
            }
        }
    }
    if (var)
        *var = mainvar;

    return maxexp;
}


bool polynomial::is_zero(bool error) const
// ----------------------------------------------------------------------------
//   Return true if the polynomial is zero
// ----------------------------------------------------------------------------
{
    iterator first = begin();
    if (first == end())
        return true;
    return false;
}


polynomial::iterator polynomial::ranking(size_t *var) const
// ----------------------------------------------------------------------------
//   Locate the highest-ranking term in the polynomial
// ----------------------------------------------------------------------------
{
    size_t vars    = variables();
    size_t mainvar = 0;
    ularge maxexp  = 0;
    iterator where = end();
    for (auto term : *this)
    {
        iterator here = term;
        algebraic_g factor = term.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v < vars; v++)
            {
                ularge vexp = term.exponent();
                if (maxexp < vexp)
                {
                    mainvar = v;
                    maxexp = vexp;
                    where = here;
                }
            }
        }
    }
    if (var)
        *var = mainvar;

    return where;
}


polynomial::iterator polynomial::ranking(size_t var) const
// ----------------------------------------------------------------------------
//   Locate the highest-ranking term for given variable in the polynomial
// ----------------------------------------------------------------------------
{
    size_t vars = variables();
    if (!vars)
        return begin();
    ularge   maxexp = 0;
    iterator where  = end();
    for (auto term : *this)
    {
        iterator here = term;   // Using term.exponent() below moves term
        algebraic_g factor = term.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v < vars; v++)
            {
                ularge vexp = term.exponent();
                if (v == var && maxexp < vexp)
                {
                    maxexp = vexp;
                    where = here;
                }
            }
        }
    }

    return where;
}


PARSE_BODY(polynomial)
// ----------------------------------------------------------------------------
//   No parsing for polynomials, they are only generated from expressions
// ----------------------------------------------------------------------------
{
    // If already parsing an equation, let upper parser deal with quote
    if (p.precedence)
        return SKIP;

    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = 0;

    // First character must be a constant marker
    unicode cp = utf8_codepoint(source);
    if (cp != L'Ⓟ')
        return SKIP;
    parsed = utf8_next(source, parsed, max);

    // Parse the expression itself
    p.source = +p.source + parsed;
    p.length -= parsed;
    p.precedence = 1;
    auto result = list_parse(ID_expression, p, '\'', '\'');
    p.precedence = 0;
    p.source = +p.source - parsed;
    p.length += parsed;

    if (result != OK)
        return result;
    if (p.out)
    {
        if (algebraic_p alg = p.out->as_algebraic())
        {
            if (polynomial_p poly = polynomial::make(alg))
            {
                p.out = +poly;
                return OK;
            }
        }
    }
    rt.invalid_polynomial_error().source(p.source, p.length);
    return ERROR;
}


EVAL_BODY(polynomial)
// ----------------------------------------------------------------------------
//   We can evaluate polynomials a bit faster than usual expressions
// ----------------------------------------------------------------------------
{
    if (running)
        return rt.push(o) ? OK : ERROR;

    polynomial_g poly  = o;
    size_t       nvars = poly->variables();
    algebraic_g  vars[nvars];

    // Evaluate each of the variables exactly once (this is where we save time)
    for (size_t v = 0; v < nvars; v++)
    {
        symbol_g var       = poly->variable(v);
        object_p evaluated = var->evaluate();
        if (!evaluated)
            return ERROR;
        algebraic_g alg = evaluated->as_extended_algebraic();
        if (!alg)
        {
            rt.type_error();
            return ERROR;
        }
        vars[v] = alg;
    }

    // Loop over all factors
    algebraic_g result = nullptr;
    for (auto term : *poly)
    {
        algebraic_g factor = term.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v < nvars; v++)
            {
                ularge exponent = term.exponent();
                if (exponent)
                {
                    algebraic_g value =
                        exponent == 1 ? vars[v] : ::pow(vars[v], exponent);
                    factor = factor * value;
                    if (!factor)
                        return ERROR;
                }
            }
            result = result ? result + factor : factor;
            if (!result)
                return ERROR;
        }
    }
    if (!result)
        result = +integer::make(0);

    // We are done, push the result
    return rt.push(+result) ? OK : ERROR;
}


RENDER_BODY(polynomial)
// ----------------------------------------------------------------------------
//  Render a polynomial as text
// ----------------------------------------------------------------------------
{
    polynomial_g poly  = o;
    size_t       nvars = poly->variables();
    symbol_g     vars[nvars];

    // Get each of the variables
    for (size_t v = 0; v < nvars; v++)
        vars[v] = poly->variable(v);

    bool editing = r.editing();
    if (editing || Settings.PrefixPolynomialRender())
        r.put(unicode(L'Ⓟ'));
    if (editing)
        r.put('\'');

    // Loop over all factors
    bool    first = true;
    unicode mul   = Settings.UseDotForMultiplication() ? L'·' : L'×';
    for (auto term : *poly)
    {
        // Emit the factor
        algebraic_g factor = term.factor();
        bool isneg = factor->is_negative(false);
        if (isneg)
            factor = -factor;

        // Separate terms with + or -
        if (!first || isneg)
            r.put(isneg ? '-' : '+');
        first = false;

        bool hasmul = !factor->is_one(false);
        if (hasmul)
            factor->render(r);

        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = term.exponent();
            if (exponent)
            {
                if (hasmul)
                    r.put(mul);
                hasmul = true;
                vars[v]->render(r);
                if (exponent > 1)
                {
                    r.put(unicode(L'↑'));
                    char exptxt[24];
                    char *end = render_u64(exptxt, exponent);
                    r.put(exptxt, end - exptxt);
                }
            }
        }
        if (!hasmul)
            factor->render(r);
    }
    // Special-case of empty polynomial
    if (first)
        r.put('0');
    if (editing)
        r.put('\'');

    // We are done, push the result
    return r.size();
}


GRAPH_BODY(polynomial)
// ----------------------------------------------------------------------------
//  Render a polynomial as a graphic expression
// ----------------------------------------------------------------------------
{
    polynomial_g poly  = o;
    size_t       nvars = poly->variables();
    grob_g       vars[nvars];

    // Get each of the variables and render it graphically
    for (size_t v = 0; v < nvars; v++)
    {
        symbol_g sym = poly->variable(v);
        grob_g   var = sym->graph(g);
        vars[v]      = var;
    }


    // Loop over all factors
    grob_g  result = nullptr;
    coord   vr     = 0;
    cstring mul    = Settings.UseDotForMultiplication() ? "·" : "×";

    for (auto term : *poly)
    {
        // Render the factor
        algebraic_g factor = term.factor();
        bool        isneg  = factor->is_negative(false);
        if (isneg)
            factor = -factor;
        grob_g      factg  = factor->is_one(false) ? nullptr : factor->graph(g);
        coord       vf     = 0;

        // Render the terms
        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = term.exponent();
            if (exponent)
            {
                grob_g termg = vars[v];
                coord  vt    = 0;
                if (exponent > 1)
                {
                    char exptxt[24];
                    char *end = render_u64(exptxt, exponent);
                    *end = 0;
                    termg = suscript(g, vt, termg, 0, exptxt);
                    if (!termg)
                        return nullptr;
                    vt    = g.voffset;
                }
                if (factg)
                {
                    factg = infix(g, vf, factg, 0, mul, vt, termg);
                    if (!factg)
                        return nullptr;
                    vf = g.voffset;
                }
                else
                {
                    factg = termg;
                    vf = vt;
                }
            }
        }

        // Addition of terms
        if (result)
        {
            if (factor->is_one(false) && !factg)
                factg = factor->graph(g);
            result = infix(g, vr, result, 0, isneg ? "-" : "+", vf, factg);
            if (!result)
                return nullptr;
        }
        else
        {
            result = factg;
        }
        vr = g.voffset;
    }

    // Optionally display a little inverted [poly] to identify a polynomial
    if (Settings.PrefixPolynomialRender())
        result = prefix(g, 0, "Ⓟ", vr, result);

    // We are done, push the result
    return result;
}


FUNCTION_BODY(ToPolynomial)
// ----------------------------------------------------------------------------
//   Convert array/list/expression to a polynomial object
// ----------------------------------------------------------------------------
{
    if (polynomial_p poly = polynomial::get(+x))
        return poly;
    if (!rt.error())
        rt.invalid_polynomial_error();
    return nullptr;
}


COMMAND_BODY(FromPolynomial)
// ----------------------------------------------------------------------------
//   Convert a polynomial to an expression
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (polynomial_p poly = obj->as<polynomial>())
            if (algebraic_p result = poly->as_expression())
                if (rt.top(result))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


algebraic_p polynomial::as_expression() const
// ----------------------------------------------------------------------------
//   Rewrite a polynomial as a regular expression
// ----------------------------------------------------------------------------
{
    polynomial_g poly  = this;
    size_t       nvars = poly->variables();
    algebraic_g  vars[nvars];

    // Evaluate each of the variables exactly once (this is where we save time)
    for (size_t v = 0; v < nvars; v++)
    {
        symbol_p var = poly->variable(v);
        vars[v]      = var;
    }

    // Loop over all factors
    algebraic_g result = nullptr;
    for (auto term : *poly)
    {
        algebraic_g factor = term.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v < nvars; v++)
            {
                ularge exponent = term.exponent();
                if (exponent)
                {
                    algebraic_g value = exponent == 1
                        ? vars[v]
                        : ::pow(vars[v], exponent);
                    factor = factor->is_one(false) ? value : factor * value;
                    if (!factor)
                        return nullptr;
                    ;
                }
            }
            result = result ? result + factor : factor;
            if (!result)
                return nullptr;
        }
    }

    // If we did not have any term, just return 0
    if (!result)
        result = +integer::make(0);

    // We are done, return the result
    return result;
}



// ============================================================================
//
//   Polynomial iterator
//
// ============================================================================

polynomial::iterator::iterator(polynomial_p poly, bool at_end)
// ----------------------------------------------------------------------------
//   Constructor for an iterator over polynomials
// ----------------------------------------------------------------------------
    : poly(poly), size(), variables(), offset()
{
    byte_p first = byte_p(poly);
    byte_p p     = poly->payload();
    size       = leb128<size_t>(p);
    size += p - first;
    variables    = leb128<size_t>(p);
    if (at_end)
    {
        offset = size;
    }
    else
    {
        for (size_t v = 0; v < variables; v++)
        {
            // Skip each name
            size_t vlen = leb128<size_t>(p);
            p += vlen;
        }
        offset = p - first;
    }
}


algebraic_p polynomial::iterator::factor()
// ----------------------------------------------------------------------------
//   Consume the scaling factor in the iterator
// ----------------------------------------------------------------------------

{
    algebraic_p scalar    = algebraic_p(poly) + offset;
    object_p    exponents = scalar->skip();
    offset = exponents - object_p(poly);
    return scalar;
}


ularge polynomial::iterator::exponent()
// ----------------------------------------------------------------------------
//   Consume the next exponent in the iterator
// ----------------------------------------------------------------------------
{
    byte_p p = byte_p(poly) + offset;
    ularge exp = leb128<ularge>(p);
    offset = p - byte_p(poly);
    return exp;
}


bool polynomial::iterator::operator==(const iterator &o) const
// ----------------------------------------------------------------------------
//   Check if two iterators are equal
// ----------------------------------------------------------------------------
{
    return +o.poly  == +poly
        &&  o.offset == offset
        &&  o.size == size
        &&  o.variables == variables;
}


bool polynomial::iterator::operator!=(const iterator &o) const
// ----------------------------------------------------------------------------
//   Check if two iterators are not equal
// ----------------------------------------------------------------------------
{
    return !(o==*this);
}


polynomial::iterator& polynomial::iterator::operator++()
// ----------------------------------------------------------------------------
//   Iterator pre-increment
// ----------------------------------------------------------------------------
{
    if (offset < size)
    {
        factor();
        for (size_t v = 0; v < variables; v++)
            exponent();
    }
    return *this;
}


polynomial::iterator polynomial::iterator::operator++(int)
// ----------------------------------------------------------------------------
//   Iterator post-increment
// ----------------------------------------------------------------------------
{
    iterator prev = *this;
    ++(*this);
    return prev;
}


polynomial::iterator::value_type polynomial::iterator::operator*()
// ----------------------------------------------------------------------------
//   Derefernecing an iterator return the iterator itself
// ----------------------------------------------------------------------------
{
    return *this;
}


ularge polynomial::iterator::rank(size_t *var) const
// ----------------------------------------------------------------------------
//   Return the highest rank at the iterator position
// ----------------------------------------------------------------------------
{
    ularge      maxexp = 0;
    ularge      mainvar = ~0U;
    if (offset < size)
    {
        iterator    it     = *this;
        algebraic_g factor = it.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v  < variables; v++)
            {
                ularge vexp = it.exponent();
                if (vexp > maxexp)
                {
                    mainvar = v;
                    maxexp = vexp;
                }

            }
        }
    }

    if (var)
        *var = mainvar;

    return maxexp;
}


ularge polynomial::iterator::rank(size_t var) const
// ----------------------------------------------------------------------------
//   Return the rank associated with a variable
// ----------------------------------------------------------------------------
{
    ularge maxexp = 0;
    if (offset < size)
    {
        iterator    it     = *this;
        algebraic_g factor = it.factor();
        if (!factor->is_zero(false))
        {
            for (size_t v = 0; v < variables; v++)
            {
                ularge vexp = it.exponent();
                if (var == v && vexp > maxexp)
                    maxexp = vexp;
            }
        }
    }
    return maxexp;
}


symbol_p polynomial::main_variable()
// ----------------------------------------------------------------------------
//   Return the current variable for polynomial evaluation
// ----------------------------------------------------------------------------
{
    if (directory_p dir = config())
        if (object_p name = static_object(ID_AlgebraVariable))
            if (object_p obj = dir->recall(name))
                if (symbol_p sym = obj->as_quoted<symbol>())
                    return sym;
    return symbol::make("x");
}



bool polynomial::main_variable(symbol_p sym)
// ----------------------------------------------------------------------------
//   Set the current variable for polynomial evaluation
// ----------------------------------------------------------------------------
{
    directory_g cfg = config();
    symbol_g    symg = sym;
    if (!cfg)
    {
        object_p name = static_object(ID_AlgebraConfiguration);
        directory *dir = rt.variables(0);
        if (!dir)
        {
            rt.no_directory_error();
            return false;
        }

        cfg = rt.make<directory>();
        if (!cfg)
            return false;
        cfg = (directory_p) dir->store(name, +cfg);
        if (!cfg || cfg->type() != ID_directory)
            return false;
    }

    if (object_p name = static_object(ID_AlgebraVariable))
        if (directory *wcfg = (directory *) +cfg)
            return wcfg->store(name, +symg);

    return false;
}


directory_p polynomial::config()
// ----------------------------------------------------------------------------
//   Return the directory for the current CAS configuration, or nullptr
// ----------------------------------------------------------------------------
{
    if (object_p name = static_object(ID_AlgebraConfiguration))
        if (object_p obj = directory::recall_all(name, false))
            if (directory_p dir = obj->as<directory>())
                return dir;
    return nullptr;
}



COMMAND_BODY(AlgebraConfiguration)
// ----------------------------------------------------------------------------
//   Recall the current algebra configuration directory
// ----------------------------------------------------------------------------
{
    if (directory_p config = polynomial::config())
        if (rt.push(config))
            return OK;
    return ERROR;
}


COMMAND_BODY(AlgebraVariable)
// ----------------------------------------------------------------------------
//   Recall the current algebra variable, defaults to `X`
// ----------------------------------------------------------------------------
{
    if (symbol_p var = polynomial::main_variable())
        if (rt.push(var))
            return OK;
    return ERROR;
}


COMMAND_BODY(StoreAlgebraVariable)
// ----------------------------------------------------------------------------
//   Store the current algebra varialbe
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.pop())
    {
        if (symbol_p sym = obj->as_quoted<symbol>())
        {
            if (polynomial::main_variable(sym))
                return OK;
        }
        rt.type_error();
    }
    return ERROR;
}


polynomial_p polynomial::from_coefficients(object_p coeffs, bool error)
// ----------------------------------------------------------------------------
//   Build a univariate polynomial from array/list coefficients
// ----------------------------------------------------------------------------
{
    if (!coeffs)
        return nullptr;
    if (polynomial_p poly = coeffs->as<polynomial>())
        return poly;

    list_p list = coeffs->as_array_or_list();
    if (!list)
    {
        if (error)
            rt.type_error();
        return nullptr;
    }

    size_t n = list->items();
    if (!n || n > Settings.MaxPolynomialDegree())
    {
        if (error)
            rt.dimension_error();
        return nullptr;
    }

    symbol_p var = polynomial::main_variable();
    if (!var)
        return nullptr;

    polynomial_g result = polynomial::make(integer::make(0));
    if (!result)
        return nullptr;

    size_t i = 0;
    for (object_p item : *list)
    {
        object_p o = object::strip(item);
        if (!o->is_real() && !o->is_complex())
        {
            if (error)
                rt.type_error();
            return nullptr;
        }
        algebraic_p coeff = o->as_algebraic();
        if (!coeff)
            return nullptr;

        ularge exp = n + ~i;
        if (!coeff->is_zero(false))
        {
            polynomial_g term = polynomial::make(coeff, var, exp);
            if (!term)
                return nullptr;
            result = polynomial::add(result, term);
            if (!result)
                return nullptr;
        }
        i++;
    }

    return result;
}


polynomial_p polynomial::get(object_p obj)
// ----------------------------------------------------------------------------
//   Turn an object into a polynomial
// ----------------------------------------------------------------------------
{
    for (uint retries = 0; retries < 2; retries++)
    {
        if (!obj)
            return nullptr;
        obj = object::strip(obj);
        if (polynomial_p poly = obj->as<polynomial>())
            return poly;
        if (polynomial_p poly = polynomial::from_coefficients(obj, false))
            return poly;
        if (expression_p expr = obj->as<expression>())
            obj = expr->as_difference_for_solve();
        if (algebraic_p alg = obj->as_algebraic())
            if (polynomial_p poly = polynomial::make(alg))
                return poly;
        if (!retries)
            if (algebraic_p alg = obj->as_algebraic())
                if (algebraic_p eval = alg->evaluate())
                    obj = eval;
    }
    return nullptr;
}


size_t polynomial::expand(bool error) const
// ----------------------------------------------------------------------------
//   Expand polynomial coefficients on the stack
// ----------------------------------------------------------------------------
{
    stack_buffer sbuf;
    size_t       result = expand(sbuf, error);
    if (result)
        sbuf.keep();                // Do not erase the stack
    return result;
}


size_t polynomial::expand(stack_buffer &sbuf, bool error) const
// ----------------------------------------------------------------------------
//   Expand polynomial coefficients in a given stack buffer
// ----------------------------------------------------------------------------
{
    size_t var = variable(main_variable());
    if (variables() > 1 || !~var)
    {
        if (error)
            rt.invalid_polynomial_error();
        return 0;
    }

    algebraic_g  factor, existing;
    for (iterator it : *this)
    {
        ularge exp = it.rank(var);
        if (exp + 1 > sbuf.items())
        {
            if (exp > Settings.MaxPolynomialDegree())
            {
                if (error)
                    rt.dimension_error();
                return 0;
            }
            sbuf.grow(exp + 1 - sbuf.items());
        }
        factor = it.factor();
        object_p obj = sbuf[exp];
        existing = obj ? obj->as_algebraic() : nullptr;
        if (!existing)
        {
            if (error)
                rt.type_error();
            return 0;
        }
        factor = existing + factor;
        sbuf[exp] = +factor;
    }
    return sbuf.items();
}


size_t polynomial::expand(object_p pobj, bool error)
// ----------------------------------------------------------------------------
//   Expand a polynomial-convertible object (array, equation or polynomial)
// ----------------------------------------------------------------------------
{
    size_t sz = 0;
    if (list_p lst = pobj->as_array_or_list())
        lst->expand_without_size(&sz);
    else if (polynomial_p poly = polynomial::get(pobj))
        sz = poly->expand(true);
    else if (error)
        rt.type_error();
    return sz;
}


array_p polynomial::coefficients(bool error) const
// ----------------------------------------------------------------------------
//   Convert a univariate polynomial to coefficient array
// ----------------------------------------------------------------------------
{
    cleaner      purge;
    scribble     scr;
    stack_buffer sbuf;
    size_t       n = expand(sbuf, error);
    if (rt.error())
        return nullptr;
    while (n-- > 0)
        if (!rt.append(sbuf[n]))
            return nullptr;
    array_g result = array_p(list::make(ID_array, scr.scratch(), scr.growth()));
    result = purge(result);
    return result;
}


polynomial_p polynomial::derivative() const
// ----------------------------------------------------------------------------
//   Return derivative in the default variable
// ----------------------------------------------------------------------------
{
    return derivative(main_variable());
}


polynomial_p polynomial::derivative(symbol_p sym) const
// ----------------------------------------------------------------------------
//   Return derivative in the default variable
// ----------------------------------------------------------------------------
{
    return derivative(variable(sym));
}


polynomial_p polynomial::derivative(size_t var) const
// ----------------------------------------------------------------------------
//   Return derivative in the given variable
// ----------------------------------------------------------------------------
{
    scribble     scr;
    polynomial_g x        = this;
    gcbytes      polycopy = copy_variables(x);
    size_t       nvars    = x->variables();
    for (iterator term : *x)
    {
        algebraic_g factor = term.factor();
        iterator    it     = term;
        bool        skip   = false;
        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = it.exponent();
            if (v == var)
            {
                if (exponent > 0)
                    factor = factor * integer::make(exponent);
                else
                    skip = true;
            }
        }
        if (skip)
        {
            for (size_t v = 0; v < nvars; v++)
                term.exponent();
        }
        else
        {
            size_t sz = factor->size();
            byte  *np = rt.allocate(sz);
            if (!np)
                return nullptr;
            memcpy(np, +factor, sz);
            for (size_t v = 0; v < nvars; v++)
            {
                ularge exponent = term.exponent();
                if (v == var && exponent > 0)
                    exponent--;
                byte *ep = rt.allocate(leb128size(exponent));
                if (!ep)
                    return nullptr;
                leb128(ep, exponent);
            }
        }
    }
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


polynomial_p polynomial::primitive() const
// ----------------------------------------------------------------------------
//   Return primitive in the default variable
// ----------------------------------------------------------------------------
{
    return primitive(main_variable());
}


polynomial_p polynomial::primitive(symbol_p sym) const
// ----------------------------------------------------------------------------
//   Return primitive in the given variable
// ----------------------------------------------------------------------------
{
    return primitive(variable(sym));
}


polynomial_p polynomial::primitive(size_t var) const
// ----------------------------------------------------------------------------
//   Return primitive in the given variable
// ----------------------------------------------------------------------------
{
    scribble     scr;
    polynomial_g x        = this;
    gcbytes      polycopy = copy_variables(x);
    size_t       nvars    = x->variables();
    for (iterator term : *x)
    {
        algebraic_g factor = term.factor();
        iterator    it     = term;
        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = it.exponent();
            if (v == var)
            {
                // Integrate: divide by (exponent+1)
                factor = factor / integer::make(exponent + 1);
            }
        }
        if (!factor)
            return nullptr;

        size_t sz = factor->size();
        byte  *np = rt.allocate(sz);
        if (!np)
            return nullptr;
        memcpy(np, +factor, sz);
        for (size_t v = 0; v < nvars; v++)
        {
            ularge exponent = term.exponent();
            if (v == var)
                exponent++;  // Increase exponent by 1
            byte *ep = rt.allocate(leb128size(exponent));
            if (!ep)
                return nullptr;
            leb128(ep, exponent);
        }
    }
    gcbytes data   = scr.scratch();
    size_t  datasz = scr.growth();
    return rt.make<polynomial>(data, datasz);
}


RECORDER(polyroots, 16, "Polynomial roots");

list_p polynomial::roots(object::id ty, symbol_p var) const
// ----------------------------------------------------------------------------
//   Compute roots for polynomial and cleanup
// ----------------------------------------------------------------------------
{
    cleaner purge;
    list_p  result = roots_internal(ty, var);
    if (result)
        result = result->sort();
    if (result)
        result = result->unique();
    return purge(result);
}


list_p polynomial::roots_internal(object::id ty, symbol_p var) const
// ----------------------------------------------------------------------------
//   Compute the roots from a polynomial
// ----------------------------------------------------------------------------
{
    // We can deal with degree <- 2 symbolically, otherwise must be numeric
    polynomial_g p      = this;
    symbol_g     vname  = var;
    bool         round  = Settings.SymbolicResults() && Settings.AutoSimplify();
    scribble     scr;
    record(polyroots, "Roots of %t for variable %t", +p, +vname);
    settings::SaveFractionDigits fd(4);

    while (p && !program::interrupted())
    {
        size_t       vars   = p->variables();
        size_t       vidx   = p->variable(+vname);
        iterator     r      = p->ranking(vidx);
        ularge       degree = r.rank(vidx);

        record(polyroots, "Degree %lu", degree);
        if (degree <= 2)
        {
            // ax^2+bx+c = 0, or bx+c=0
            algebraic_g a[3];
            algebraic_g x, y;
            for (iterator term : *p)
            {
                uint vexp = ~0;
                x = term.factor();
                for (size_t v = 0; v < vars; v++)
                {
                    ularge exp = term.exponent();
                    if (v == vidx)
                    {
                        vexp = exp;
                    }
                    else
                    {
                        y = variable(v);
                        y = ::pow(y, exp);
                        x = x * y;
                    }
                }
                if (vexp > 2)
                {
                    rt.invalid_polynomial_error();
                    return nullptr;
                }
                algebraic_g &vf = a[vexp];
                vf = vf ? vf + x : x;
            }

            record(polyroots, "Direct a=%t b=%t c=%t", +a[2], +a[1], +a[0]);
            if (!a[0])
                a[0] = integer::make(0);
            if (a[2] && !a[2]->is_zero(false))
            {
                if (!a[1])
                    a[1] = integer::make(0);
                y = a[2] * a[0];                  // ac
                y = y + y;                        // 2ac
                y = y + y;                        // 4ac
                y = a[1] * a[1] - y;              // b^2-4ac
                if (!y)
                    goto error;
                bool neg = y->is_negative(false);
                if (neg)
                {
                    // If negative, provide result only in complex mode
                    if (!Settings.ComplexResults())
                    {
                        record(polyroots, "No real solutions, delta=%t", +y);
                        return list::make(ty, scr.scratch(), scr.growth());
                    }
                    y = -y;
                }
                if (!y)
                    goto error;
                y = y->symbolic_sqrt();             // sqrt(b^2-4ac)
                a[2] = integer::make(2) * a[2];         // 2a
                a[1] = -a[1] / a[2];                    // -b/2a
                y  = y / a[2];                      // sqrt(b^2-4ac) / 2a
                if (!y)
                    goto error;
                if (neg)
                {
                    if (Settings.AutoSimplify())
                        if (expression_p expr = y->as<expression>())
                            y = expr->simplify();
                    x = rectangular::make(a[1], -y);
                    y = rectangular::make(a[1], y);
                }
                else
                {
                    x = a[1] - y;
                    y    = a[1] + y;
                }
                if (Settings.AutoSimplify())
                {
                    if (expression_p expr = y->as<expression>())
                        y = expr->simplify();
                    if (expression_p expr = x->as<expression>())
                        x = expr->simplify();
                }
                if (round && x && y)
                {
                    to_sqrt(x);
                    to_sqrt(y);
                }
                record(polyroots, "Solutions %t and %t", +x, +y);
                if (!rt.append(+x) || !rt.append(+y))
                    goto error;
            }
            else if (a[1] && !a[1]->is_zero(false))
            {
                y = -a[0] / a[1];
                record(polyroots, "Solution is %t", +y);
                if (!rt.append(+y))
                    goto error;
            }
            else
            {
                rt.invalid_polynomial_error();
                goto error;
            }
            return list::make(ty, scr.scratch(), scr.growth());
        }
        else
        {
            polynomial_g der1 = p->derivative(vidx);
            polynomial_g der2 = der1->derivative(vidx);
            stack_buffer spoly, sder1, sder2;
            if (!p->expand(spoly, true)     ||
                !der1->expand(sder1, true)  ||
                !der2->expand(sder2, true))
                goto error;

            record(polyroots, "p  =%t", +p);
            record(polyroots, "p' =%t", +der1);
            record(polyroots, "p''=%t", +der2);

            settings::SaveComplexResults scr(true);
            size_t      max = Settings.SolverIterations();
            algebraic_g eps = algebraic::epsilon();
            algebraic_g n   = integer::make(degree);
            algebraic_g n1  = integer::make(1);
            algebraic_g x   = integer::make(0);
            algebraic_g y, g, h, a;
            n1 = n - n1;

            // Laguerre iteration
            bool found = false;
            bool yneg  = false;
            bool yschg = false;
            decimal::precision_adjust prec(3);
            for (size_t i = 0; i < max; i++)
            {
                if (!x)
                    goto error;
                y = horner(spoly, x);
                record(polyroots, "Laguerre %zu x=%t y=%t", i, +x, +y);
                if (!y)
                    goto error;
                if (y->is_zero(false) || smaller_magnitude(y, eps))
                {
                    found = true;
                    record(polyroots, "Solution found x=%t y=%t", +x, +y);
                    break;
                }

                bool yn = y->is_negative(false);
                yschg   = yneg != yn;
                yneg    = yn;

                g = horner(sder1, x);
                g = g / y;
                h = horner(sder2, x);
                h = g * g - h / y;
                a = n1 * (n * h - g * g);
                a = sqrt::run(a);
                record(polyroots, "G=%t H=%t nH-G^2=%t", +g, +h, +a);
                y = g - a;
                a = g + a;
                record(polyroots, "denominators %t or %t", +a, +y);
                if (smaller_magnitude(a, y))
                    a = y;
                a = n / a;
                x = x - a;
                record(polyroots, "a=%t new x=%t", +a, +x);
                if (yschg && smaller_magnitude(a, eps * x))
                {
                    found = true;
                    record(polyroots, "Sign change at a=%t new x=%t", +a, +x);
                    break;
                }
            }

            sder1.cleanup();
            sder2.cleanup();
            spoly.cleanup();
            if (!found)
            {
                rt.no_solution_error();
                goto error;
            }
            if (round && x)
                to_sqrt(x);
            if (!rt.append(x))
                goto error;

            // Here p(x) is small enough, generate x-x0 polynomial
            der1 = make(vname);
            der2 = make(x);
            der1 = sub(der1, der2);
            der2 = div(p, der1);
            record(polyroots, "Dividing %t by %t is %t", +p, +der1, +der2);
            p = der2;
        }
    }

error:
    if (!rt.error())
        rt.invalid_polynomial_error();
    return nullptr;
}



// ============================================================================
//
//   Polynomial evaluation (Horner's method)
//
// ============================================================================

algebraic_p polynomial::horner(stack_buffer &s, algebraic_r x)
// ----------------------------------------------------------------------------
//   Horner evaluation from coefficients on the stack
// ----------------------------------------------------------------------------
{
    cleaner     purge;
    size_t      n = s.items();
    algebraic_g result, c;
    for (size_t i = 0; i < n; i++)
    {
        object_p o = s[n + ~i];
        c = o->as_algebraic();
        if (!c)
            return nullptr;
        result = result ? result * x + c : c;
        if (!result)
            return nullptr;
    }
    return purge(result);
}


algebraic_p polynomial::horner(algebraic_r x)
// ----------------------------------------------------------------------------
//   Horner evaluation from a polynomial
// ----------------------------------------------------------------------------
{
    stack_buffer s;
    expand(s, true);
    if (rt.error())
        return nullptr;
    return horner(s, x);
}


COMMAND_BODY(PEval)
// ----------------------------------------------------------------------------
//   Evaluate a polynomial at a point
// ----------------------------------------------------------------------------
{
    if (object_p xobj = rt.stack(0))
    {
        algebraic_p x = xobj->as_algebraic();
        if (!x)
        {
            rt.type_error();
            return ERROR;
        }

        if (object_p pobj = rt.stack(1))
        {
            size_t       sz = polynomial::expand(pobj, true);
            if (!sz)
                return ERROR;
            stack_buffer s(sz);
            algebraic_p result = polynomial::horner(s, x);
            s.cleanup();
            if (result && rt.drop() && rt.top(result))
                return OK;
        }
    }
    return ERROR;
}



// ============================================================================
//
//   Polynomial roots (PRoot / PCoef / Zeros)
//
// ============================================================================

COMMAND_BODY(PRoot)
// ----------------------------------------------------------------------------
//   All roots of a polynomial from coefficient vector
// ----------------------------------------------------------------------------
{
    if (object_p pobj = rt.top())
        if (polynomial_p poly = polynomial::get(pobj))
            if (symbol_p var = polynomial::main_variable())
                if (list_p roots = poly->roots(ID_array, var))
                    if (rt.top(roots))
                        return OK;

    if (!rt.error())
        rt.type_error();
    return ERROR;
}


COMMAND_BODY(PCoef)
// ----------------------------------------------------------------------------
//   Monic polynomial coefficients from roots
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        list_p roots = obj->as_array_or_list();
        if (!roots)
        {
            rt.type_error();
            return ERROR;
        }
        polynomial_g var  = polynomial::make(polynomial::main_variable());
        polynomial_g poly = nullptr;
        polynomial_g term = nullptr;
        for (object_p root : *roots)
        {
            algebraic_p r = root->as_algebraic();
            if (!r)
            {
                rt.invalid_polynomial_error();
                return ERROR;
            }
            term = polynomial::make(algebraic_p(r));
            term = polynomial::sub(var, term);
            poly = poly ? polynomial::mul(poly, term) : +term;
            if (!poly)
                return ERROR;
        }

        algebraic_p result = +poly;
        if (Settings.CompatiblePolynomials())
            result = poly->coefficients();
        if (result && rt.top(result))
            return OK;
    }
    return ERROR;
}
