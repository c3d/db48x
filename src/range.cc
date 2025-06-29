// ****************************************************************************
//  range.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Range arithmetic
//
//    This defines two range types:
//    - range is an interval of numbers, showing as 1.245…1.267, or,
//      alternatively, as 1.256±0.011 or 1.256±0.876%. All three forms are
//      accepted as input.
//    - uncertain is mean and standard deviation, showing as 1.245±σ0.067
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

#include "range.h"

#include "arithmetic.h"
#include "compare.h"
#include "functions.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "tag.h"
#include "unit.h"
#include "utf8.h"


// ============================================================================
//
//   Generic range operations
//
// ============================================================================
//
//  The generic operations optimize for the most efficient operation
//  if there is a difference between uncertain and polar
//

SIZE_BODY(range)
// ----------------------------------------------------------------------------
//   Size of a complex number
// ----------------------------------------------------------------------------
{
    object_p p = object_p(payload(o));
    object_p e = p->skip()->skip();
    return byte_p(e) - byte_p(o);
}


HELP_BODY(range)
// ----------------------------------------------------------------------------
//   Help topic for range numbers
// ----------------------------------------------------------------------------
{
    return utf8("Ranges");
}


HELP_BODY(uncertain)
// ----------------------------------------------------------------------------
//   Help topic for range numbers
// ----------------------------------------------------------------------------
{
    return utf8("Uncertain numbers");
}



// ============================================================================
//
//   Specific code for uncertain form
//
// ============================================================================

PARSE_BODY(range)
// ----------------------------------------------------------------------------
//   Parse the range form for range numbers
// ----------------------------------------------------------------------------
//   We accept the following formats: a…b, a±b and a±b%
{
    // Check if we have a first expression
    algebraic_g xexpr = algebraic_p(+p.out);
    if (!xexpr)
        return ERROR;
    if (!xexpr->is_real())
        return SKIP;

    size_t max = p.length;
    if (!max)
        return SKIP;

    // First character must be compatible with a range
    size_t  offs  = 0;
    unicode cp    = p.separator;
    bool    imark = cp == range::INTERVAL_MARK;
    bool    pmark = cp == drange::PLUSMINUS_MARK;
    bool    smark = p.precedence != PARENTHESES && cp == uncertain::SIGMA_MARK;
    if (!imark && !pmark && !smark)
        return SKIP;
    offs = utf8_next(p.source, offs, max);
    if (!smark && offs < max)
    {
        cp = utf8_codepoint(p.source + offs);
        smark = cp == uncertain::SIGMA_MARK;
        if (smark)
            offs = utf8_next(p.source, offs, max);
    }

    size_t   ysz  = max - offs;
    object_p yobj = parse(p.source + offs, ysz, PARENTHESES);
    if (!yobj)
        return ERROR;
    algebraic_g yexpr = yobj->as_algebraic();
    if (!yexpr || !yexpr->is_real())
        return SKIP;
    offs += ysz;

    // Check if we give a percentage
    cp = !smark && offs < max ? utf8_codepoint(p.source + offs) : 0;
    bool cmark = cp == '%';
    if (!smark)
    {
        smark = cp == uncertain::SIGMA_MARK;
        if (smark)
            offs = utf8_next(p.source, offs, max);
    }

    if (smark)
    {
        p.out = uncertain::make(xexpr, yexpr);
    }
    else
    {
        id type = object::ID_range;
        if (cmark || pmark)
        {
            algebraic_g div = yexpr;
            if (cmark)
            {
                div = integer::make(100);
                div = yexpr / div;
                if (!xexpr->is_zero(false))
                    div = xexpr * div;
                offs = utf8_next(p.source, offs, max);
            }
            if (div->is_negative(false))
                div = -div;
            yexpr = xexpr + div;
            xexpr = xexpr - div;
            type = cmark ? ID_prange : ID_drange;
        }
        range::sort(xexpr, yexpr);
        p.out = range::make(type, xexpr, yexpr);
    }

    p.length = offs;
    return p.out ? OK : ERROR;
}


RENDER_BODY(range)
// ----------------------------------------------------------------------------
//   Render a range
// ----------------------------------------------------------------------------
{
    range_g     go = o;
    algebraic_g lo = go->lo();
    algebraic_g hi = go->hi();
    lo->render(r);
    r.put(unicode(range::INTERVAL_MARK));
    hi->render(r);
    return r.size();
}


RENDER_BODY(drange)
// ----------------------------------------------------------------------------
//   Render a delta range
// ----------------------------------------------------------------------------
{
    range_g     go   = o;
    algebraic_g lo   = go->lo();
    algebraic_g hi   = go->hi();
    algebraic_g two  = integer::make(2);
    algebraic_g disp = (lo + hi) / two;
    if (disp)
        disp->render(r);
    r.put(unicode(drange::PLUSMINUS_MARK));
    algebraic_g delta = (hi - lo) / two;
    if (delta)
        delta->render(r);
    return r.size();
}


RENDER_BODY(prange)
// ----------------------------------------------------------------------------
//   Render a delta range
// ----------------------------------------------------------------------------
{
    range_g     go   = o;
    algebraic_g lo   = go->lo();
    algebraic_g hi   = go->hi();
    algebraic_g two  = integer::make(2);
    algebraic_g disp = (lo + hi) / two;
    if (disp)
        disp->render(r);
    r.put(unicode(drange::PLUSMINUS_MARK));
    algebraic_g delta = (hi - disp) * integer::make(100);
    if (!disp->is_zero(false))
        delta = delta / disp;
    if (delta->is_negative(true))
        delta = -delta;
    if (delta)
        delta->render(r);
    r.put('%');
    return r.size();
}


RENDER_BODY(uncertain)
// ----------------------------------------------------------------------------
//   Render an uncertain number
// ----------------------------------------------------------------------------
{
    uncertain_g     go = o;
    algebraic_g a = go->average();
    algebraic_g s = go->stddev();
    a->render(r);
    r.put(unicode(drange::PLUSMINUS_MARK));
    s->render(r);
    r.put(unicode(uncertain::SIGMA_MARK));
    return r.size();
}


bool range::is_zero() const
// ----------------------------------------------------------------------------
//   A range in uncertain form is zero iff both lo and hi are zero
// ----------------------------------------------------------------------------
{
    range_g o = this;
    return o->x()->is_zero(false) && o->y()->is_zero(false);
}


bool range::is_one() const
// ----------------------------------------------------------------------------
//   A range is one iff both lo and hi are one
// ----------------------------------------------------------------------------
{
    range_g o = this;
    return o->x()->is_one(false) && o->y()->is_one(false);
}


bool range::sort(algebraic_g &x, algebraic_g &y)
// ----------------------------------------------------------------------------
//   Swap x and y if x > y
// ----------------------------------------------------------------------------
{
    int cmp = 0;
    bool result = comparison::compare(&cmp, x, y) && cmp > 0;
    if (result)
        std::swap(x, y);
    return result;
}


algebraic_p range::as_uncertain() const
// ----------------------------------------------------------------------------
//   Convert a range to an uncertainy representation
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return this;
}



// ============================================================================
//
//   Arithmetic on ranges
//
// ============================================================================

range_g operator-(range_r x)
// ----------------------------------------------------------------------------
//  Unary minus for ranges
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    return range::make(x->type(), -x->hi(), -x->lo());
}


range_g operator+(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range addition
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g lo = x->lo() + y->lo();
    algebraic_g hi = x->hi() + y->hi();
    return range::make(y->type(), lo, hi);
}


range_g operator-(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range subtraction
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g lo = x->lo() - y->hi();
    algebraic_g hi = x->hi() - y->lo();
    return range::make(y->type(), lo, hi);
}


range_g operator*(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range multiplication - Here we need to compare things
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g a = x->lo() * y->lo();
    algebraic_g b = x->lo() * y->hi();
    algebraic_g c = x->hi() * y->lo();
    algebraic_g d = x->hi() * y->hi();

    range::sort(a, b);
    range::sort(a, c);
    range::sort(a, d);
    range::sort(b, d);
    range::sort(c, d);
    return range::make(y->type(), a, d);
}


range_g operator/(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range division - Here we need to compare things too
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g a = x->lo() / y->lo();
    algebraic_g b = x->lo() / y->hi();
    algebraic_g c = x->hi() / y->lo();
    algebraic_g d = x->hi() / y->hi();

    range::sort(a, b);
    range::sort(a, c);
    range::sort(a, d);
    range::sort(b, d);
    range::sort(c, d);
    return range::make(y->type(), a, d);
}



// ============================================================================
//
//   Implementation of range functions
//
// ============================================================================

static range_p monotonic(algebraic_fn fn, range_r r, bool down = false)
// ----------------------------------------------------------------------------
//   Compute monotonic functions
// ----------------------------------------------------------------------------
{
    if (!r)
        return nullptr;
    algebraic_g lo   = r->lo();
    algebraic_g hi   = r->hi();
    object::id  type = r->type();
    lo               = fn(lo);
    hi = fn(hi);
    return down ? range::make(type, hi, lo) : range::make(type, lo, hi);
}


RANGE_BODY(sqrt)
// ----------------------------------------------------------------------------
//   Range implementation of sqrt (monotonic)
// ----------------------------------------------------------------------------
{
    return monotonic(sqrt::evaluate, r);
}


RANGE_BODY(cbrt)
// ----------------------------------------------------------------------------
//   Range implementation of cbrt (monotonic)
// ----------------------------------------------------------------------------
{
    return monotonic(cbrt::evaluate, r);
}


static range_p trig(algebraic_fn fn, range_p r, int issin, int istan)
// ----------------------------------------------------------------------------
//   Compute interval for a sin or a cos
// ----------------------------------------------------------------------------
{
    if (!r)
        return nullptr;
    algebraic_g lo    = r->lo();
    algebraic_g hi    = r->hi();
    object::id  amode = Settings.AngleMode();
    algebraic_g hpi = algebraic::convert_angle(lo, amode,
                                               object::ID_PiRadians, false);
    algebraic_g lpi = algebraic::convert_angle(hi, amode,
                                               object::ID_PiRadians, false);

    // Compute numbers of quadrants (90 degrees quarters)
    lpi = lpi + lpi;
    hpi = hpi + hpi;
    int32_t lip = lpi->as_int32(0, true) - issin;
    int32_t hip = hpi->as_int32(0, true) - issin;

    // Check if monotonic case or not
    if (hip / 2 == lip / 2)
    {
        // Monotonic section
        lo = fn(lo);
        hi = fn(hi);
        if (!istan && lip / 2 % 2 == 0)
            std::swap(lo, hi);
    }
    else if (istan || hip/2 - lip/2 > 1)
    {
        lo = istan ? rt.infinity(true)  : integer::make(-1);
        hi = istan ? rt.infinity(false) : integer::make( 1);
    }
    else
    {
        lo = fn(lo);
        hi = fn(hi);
        range::sort(lo, hi);
        if (lip / 2 % 2)
            hi = integer::make(1);
        else
            lo = integer::make(-1);
    }

    return range::make(r->type(), lo, hi);
}


RANGE_BODY(sin)
// ----------------------------------------------------------------------------
//   Range implementation of sin
// ----------------------------------------------------------------------------
{
    return trig(sin::evaluate, r, 1, false);
}

RANGE_BODY(cos)
// ----------------------------------------------------------------------------
//   Range implementation of cos
// ----------------------------------------------------------------------------
{
    return trig(cos::evaluate, r, 0, false);
}


RANGE_BODY(tan)
// ----------------------------------------------------------------------------
//   Range implementation of tan
// ----------------------------------------------------------------------------
{
    return trig(tan::evaluate, r, 1, true);
}


RANGE_BODY(asin)
// ----------------------------------------------------------------------------
//   Range implementation of asin
// ----------------------------------------------------------------------------
{
    return monotonic(asin::evaluate, r);
}


RANGE_BODY(acos)
// ----------------------------------------------------------------------------
//   Range implementation of acos
// ----------------------------------------------------------------------------
{
    return monotonic(acos::evaluate, r, true);
}


RANGE_BODY(atan)
// ----------------------------------------------------------------------------
//   Range implementation of atan
// ----------------------------------------------------------------------------
{
    return monotonic(atan::evaluate, r);
}


RANGE_BODY(sinh)
// ----------------------------------------------------------------------------
//   Range implementation of sinh
// ----------------------------------------------------------------------------
{
    return monotonic(sinh::evaluate, r);
}

RANGE_BODY(cosh)
// ----------------------------------------------------------------------------
//   Range implementation of cosh
// ----------------------------------------------------------------------------
{
    if (!r)
        return nullptr;
    algebraic_g lo   = r->lo();
    algebraic_g hi   = r->hi();
    bool        lneg = lo->is_negative(false);
    bool        hneg = hi->is_negative(false);
    if (lneg == hneg)
        return monotonic(cosh::evaluate, r, lneg);
    lo = cosh::evaluate(lo);
    hi = cosh::evaluate(hi);
    range::sort(lo, hi);
    lo = integer::make(1);
    return range::make(r->type(), lo, hi);
}


RANGE_BODY(tanh)
// ----------------------------------------------------------------------------
//   Range implementation of tanh
// ----------------------------------------------------------------------------
{
    return monotonic(tanh::evaluate, r);
}


RANGE_BODY(asinh)
// ----------------------------------------------------------------------------
//   Range implementation of asinh
// ----------------------------------------------------------------------------
{
    return monotonic(asinh::evaluate, r);
}


RANGE_BODY(acosh)
// ----------------------------------------------------------------------------
//   Range implementation of acosh
// ----------------------------------------------------------------------------
{
    return monotonic(acosh::evaluate, r);
}


RANGE_BODY(atanh)
// ----------------------------------------------------------------------------
//   Range implementation of atanh
// ----------------------------------------------------------------------------
{
    return monotonic(atanh::evaluate, r);
}


RANGE_BODY(log1p)
// ----------------------------------------------------------------------------
//   Range implementation of log1p
// ----------------------------------------------------------------------------
{
    return monotonic(log1p::evaluate, r);
}

RANGE_BODY(expm1)
// ----------------------------------------------------------------------------
//   Range implementation of expm1
// ----------------------------------------------------------------------------
{
    return monotonic(expm1::evaluate, r);
}


RANGE_BODY(log)
// ----------------------------------------------------------------------------
//   Range implementation of log
// ----------------------------------------------------------------------------
{
    return monotonic(log::evaluate, r);
}

RANGE_BODY(log10)
// ----------------------------------------------------------------------------
//   Range implementation of log10
// ----------------------------------------------------------------------------
{
    return monotonic(log10::evaluate, r);
}


RANGE_BODY(log2)
// ----------------------------------------------------------------------------
//   Range implementation of log2
// ----------------------------------------------------------------------------
{
    return monotonic(log2::evaluate, r);
}


RANGE_BODY(exp)
// ----------------------------------------------------------------------------
//   Range implementation of exp
// ----------------------------------------------------------------------------
{
    return monotonic(exp::evaluate, r);
}


RANGE_BODY(exp10)
// ----------------------------------------------------------------------------
//   Range implementation of exp10
// ----------------------------------------------------------------------------
{
    return monotonic(exp10::evaluate, r);
}


RANGE_BODY(exp2)
// ----------------------------------------------------------------------------
//   Range implementation of exp2
// ----------------------------------------------------------------------------
{
    return monotonic(exp2::evaluate, r);
}


RANGE_BODY(erf)
// ----------------------------------------------------------------------------
//   Range implementation of erf
// ----------------------------------------------------------------------------
{
    return monotonic(erf::evaluate, r);
}


RANGE_BODY(erfc)
// ----------------------------------------------------------------------------
//   Range implementation of erfc
// ----------------------------------------------------------------------------
{
    return monotonic(erfc::evaluate, r, true);
}


static range_p gamma(algebraic_fn fn, range_r r, bool aslog)
// ----------------------------------------------------------------------------
//   Range implementation of gamma and lgamm
// ----------------------------------------------------------------------------
{
    if (!r)
        return nullptr;
    algebraic_g lo   = r->lo();
    algebraic_g hi   = r->hi();
    bool        lneg = lo->is_negative(false);
    bool        hneg = hi->is_negative(false);
    if (!lneg && !hneg)
        return monotonic(fn, r);

    int32_t lip = lo->as_int32(0, true);
    if (lip == 0)
    {
        lo = tgamma::evaluate(lo);
        hi = tgamma::evaluate(hi);
        range::sort(lo, hi);
        lo = aslog ? decimal::make(-12, -2) : decimal::make(8855, -4);
    }
    else
    {
        int32_t hip = hi->as_int32(0, true);
        if (hip != lip)
        {
            lo = rt.infinity(true);
            hi = rt.infinity(false);
        }
        else
        {
            lo = tgamma::evaluate(lo);
            hi = tgamma::evaluate(hi);
            range::sort(lo, hi);
        }
    }
    return range::make(r->type(), lo, hi);
}


RANGE_BODY(tgamma)
// ----------------------------------------------------------------------------
//   Range implementation of tgamma
// ----------------------------------------------------------------------------
{
    return gamma(tgamma::evaluate, r, false);
}


RANGE_BODY(lgamma)
// ----------------------------------------------------------------------------
//   Range implementation of lgamma
// ----------------------------------------------------------------------------
{
    return gamma(lgamma::evaluate, r, true);
}


RANGE_BODY(abs)
// ----------------------------------------------------------------------------
//   Range implementation of abs
// ----------------------------------------------------------------------------
{
    algebraic_g lo   = r->lo();
    algebraic_g hi   = r->hi();
    bool        lneg = lo->is_negative(false);
    bool        hneg = hi->is_negative(false);
    if (lneg)
        lo = -lo;
    if (hneg)
        hi = -hi;
    if (lneg != hneg)
    {
        range::sort(lo, hi);
        lo = integer::make(0);
    }
    else if (lneg)
    {
        std::swap(lo, hi);
    }
    return range::make(r->type(), lo, hi);
}



// ============================================================================
//
//   Uncertain numbers
//
// ============================================================================

bool uncertain::is_zero() const
// ----------------------------------------------------------------------------
//   An uncertain range in zero if both value and deviation are zero
// ----------------------------------------------------------------------------
{
    range_g o = this;
    return o->x()->is_zero(false) && o->y()->is_zero(false);
}


bool uncertain::is_one() const
// ----------------------------------------------------------------------------
//   An uncertain range is zero iff the value is one and stddev is zero
// ----------------------------------------------------------------------------
{
    range_g o = this;
    return o->x()->is_one(false) && o->y()->is_zero(false);
}


algebraic_p uncertain::as_range(object::id type) const
// ----------------------------------------------------------------------------
//   Convert an uncertain number to a range
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return this;
}
