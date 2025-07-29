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
#include "variables.h"



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
    size_t max = p.length;
    if (!max)
        return SKIP;
    size_t  offs  = 0;
    unicode cp    = p.separator;

    // Check if we have a first expression
    algebraic_g xexpr = algebraic_p(+p.out);
    bool        xisinf = false;

    // Check if we start with −∞
    if (!xexpr && !cp)
    {
        const size_t refsz = sizeof("−∞") - 1;
        if (max < refsz || memcmp(+p.source, "−∞", refsz) != 0)
            return SKIP;
        offs += refsz;
        p.separator = cp = utf8_codepoint(p.source + offs);
        xexpr = rt.infinity(true);
        xisinf = true;
    }

    if (!xexpr)
        return ERROR;
    if (!xisinf && !xexpr->is_real())
        return SKIP;


    // First character must be compatible with a range
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

    size_t   ysz    = max - offs;
    bool     yisinf = utf8_codepoint(p.source + offs) == L'∞';
    object_p yobj = yisinf
        ? rt.infinity(false)
        : parse(p.source + offs, ysz, PARENTHESES, p.separator);
    if (!yobj)
        return ERROR;
    algebraic_g yexpr = yobj->as_algebraic();
    if (!yexpr || (!yisinf && !yexpr->is_real()))
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
    if (lo->is_infinity())
        r.put("−∞");
    else
        lo->render(r);
    r.put(unicode(range::INTERVAL_MARK));
    if (hi->is_infinity())
        r.put("∞");
    else
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
    if (lo->is_infinity() || hi->is_infinity())
        return range::do_render(o, r);
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
    if (lo->is_infinity() || hi->is_infinity())
        return range::do_render(o, r);
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
    r.put(unicode(uncertain::SIGMA_MARK));
    s->render(r);
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

range_p operator-(range_r x)
// ----------------------------------------------------------------------------
//  Unary minus for ranges
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    return range::make(x->type(), -x->hi(), -x->lo());
}


range_p operator+(range_r x, range_r y)
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


range_p operator-(range_r x, range_r y)
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


range_p operator*(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range multiplication - Here we need to compare things
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g xlo = x->lo();
    algebraic_g xhi = x->hi();
    algebraic_g ylo = y->lo();
    algebraic_g yhi = y->hi();
    algebraic_g a = xlo * ylo;
    algebraic_g b = xlo * yhi;
    algebraic_g c = xhi * ylo;
    algebraic_g d = xhi * yhi;

    range::sort(a, b);
    range::sort(a, c);
    range::sort(a, d);
    range::sort(b, d);
    range::sort(c, d);
    return range::make(y->type(), a, d);
}


range_p operator/(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Range division - Here we need to compare things too
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    algebraic_g xlo = x->lo();
    algebraic_g xhi = x->hi();
    algebraic_g ylo = y->lo();
    algebraic_g yhi = y->hi();
    if (ylo->is_zero(false) || yhi->is_zero(false) ||
        ylo->is_negative(false) != yhi->is_negative())
    {
        if (Settings.InfinityError())
        {
            rt.zero_divide_error();
            return nullptr;
        }
        Settings.InfiniteResultIndicator(true);
        ylo = rt.infinity(true);
        yhi = rt.infinity(false);
        return range::make(y->type(), ylo, yhi);
    }
    algebraic_g a = xlo / ylo;
    algebraic_g b = xlo / yhi;
    algebraic_g c = xhi / ylo;
    algebraic_g d = xhi / yhi;

    range::sort(a, b);
    range::sort(a, c);
    range::sort(a, d);
    range::sort(b, d);
    range::sort(c, d);
    return range::make(y->type(), a, d);
}


range_p operator^(range_r x, range_r y)
// ----------------------------------------------------------------------------
//   Power operator
// ----------------------------------------------------------------------------
{
    return range::exp(y * range::ln(x));
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


static inline bool increasing(int h)
// ----------------------------------------------------------------------------
//   Check if a quarter denotes an increasing or decreasing region
// ----------------------------------------------------------------------------
{
    return h & 1;
}


static range_p trig(algebraic_fn fn, range_p r, int issin, int istan)
// ----------------------------------------------------------------------------
//   Compute interval for a sin or a cos
// ----------------------------------------------------------------------------
//   The `cos` function is monotonically decreasing between 0° and 180°,
//   and then monotonically increasing between 180° and 360°, and then again.
//   If we are in a monotonic region, we can just compute low and high.
//   If the range covers more than one 180° peak, then the result is -1..1
//   If the range only covers one peak, then the min is -1 or the max is 1.
//   For sin/tan, everything is shifted left by 90° to get back to `cos`
{
    if (!r)
        return nullptr;
    algebraic_g lo    = r->lo();
    algebraic_g hi    = r->hi();
    object::id  amode = Settings.AngleMode();
    algebraic_g lpi = algebraic::convert_angle(lo, amode, object::ID_PiRadians,
                                               false, false);
    algebraic_g hpi = algebraic::convert_angle(hi, amode, object::ID_PiRadians,
                                               false, false);

    // Compute numbers of quadrants (90 degrees quarters)
    lpi = lpi + lpi;
    hpi = hpi + hpi;
    lpi = floor::run(lpi);
    hpi = floor::run(hpi);
    int32_t lq = lpi->as_int32(0, true) - issin;
    int32_t hq = hpi->as_int32(0, true) - issin;
    int32_t lh = (lq - (lq < 2)) / 2;
    int32_t hh = (hq - (hq < 2)) / 2;

    // Check if monotonic case or not
    if (hh == lh)
    {
        // Monotonic section
        lo = fn(lo);
        hi = fn(hi);
        if (!istan && !increasing(lh))
            std::swap(lo, hi);
    }
    else if (istan || hh - lh > 1)
    {
        lo = istan ? rt.infinity(true)  : integer::make(-1);
        hi = istan ? rt.infinity(false) : integer::make( 1);
    }
    else
    {
        lo = fn(lo);
        hi = fn(hi);
        range::sort(lo, hi);
        if (increasing(lh))
            hi = istan ? rt.infinity(false) : integer::make( 1);
        else
            lo = istan ? rt.infinity(true)  : integer::make(-1);
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


RANGE_BODY(ln1p)
// ----------------------------------------------------------------------------
//   Range implementation of ln1p
// ----------------------------------------------------------------------------
{
    return monotonic(ln1p::evaluate, r);
}

RANGE_BODY(expm1)
// ----------------------------------------------------------------------------
//   Range implementation of expm1
// ----------------------------------------------------------------------------
{
    return monotonic(expm1::evaluate, r);
}


RANGE_BODY(ln)
// ----------------------------------------------------------------------------
//   Range implementation of log
// ----------------------------------------------------------------------------
{
    return monotonic(ln::evaluate, r);
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

    int32_t lq = lo->as_int32(0, true);
    if (lq == 0)
    {
        lo = tgamma::evaluate(lo);
        hi = tgamma::evaluate(hi);
        range::sort(lo, hi);
        lo = aslog ? decimal::make(-12, -2) : decimal::make(8855, -4);
    }
    else
    {
        int32_t hq = hi->as_int32(0, true);
        if (hq != lq)
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



// ============================================================================
//
//   Commands implementing the ranges
//
// ============================================================================

static object::result to_range(object::id ty)
// ----------------------------------------------------------------------------
//   Build a range of the given type
// ----------------------------------------------------------------------------
{
    algebraic_g lo = algebraic_p(object::strip(rt.stack(1)));
    algebraic_g hi = algebraic_p(object::strip(rt.stack(0)));
    if (!lo || !hi)
        return object::ERROR;
    if (!(lo->is_real() || lo->is_infinity()) ||
        !(hi->is_real() || hi->is_infinity()) ||
        (lo->is_infinity() && lo->is_infinity() == hi->is_infinity()))
    {
        rt.type_error();
        return object::ERROR;
    }
    range::sort(lo, hi);
    range_g r = range::make(ty, lo, hi);
    if (!r || !rt.drop() || !rt.top(r))
        return object::ERROR;
    return object::OK;
}


COMMAND_BODY(ToRange)
// ----------------------------------------------------------------------------
//   Take two values and turn them into a range
// ----------------------------------------------------------------------------
{
    return to_range(ID_range);
}


COMMAND_BODY(ToDeltaRange)
// ----------------------------------------------------------------------------
//   Take two values and turn them into a delta range
// ----------------------------------------------------------------------------
{
    return to_range(ID_drange);
}


COMMAND_BODY(ToPercentRange)
// ----------------------------------------------------------------------------
//   Take two values and turn them into a percent range
// ----------------------------------------------------------------------------
{
    return to_range(ID_prange);
}


COMMAND_BODY(ToUncertain)
// ----------------------------------------------------------------------------
//  Take two values and turn them into an uncertain number
// ----------------------------------------------------------------------------
{
    return to_range(ID_uncertain);
}


static object::result range_op(bool intersect)
// ----------------------------------------------------------------------------
//   Shared code for union and intersection
// ----------------------------------------------------------------------------
{
    range_g a = range_p(object::strip(rt.stack(1)));
    range_g b = range_p(object::strip(rt.stack(0)));
    if (!a || !b)
        return object::ERROR;
    if (!a->is_strict_range() || !b->is_strict_range())
    {
        rt.type_error();
        return object::ERROR;
    }
    algebraic_g alo = a->lo();
    algebraic_g ahi = a->hi();
    algebraic_g blo = b->lo();
    algebraic_g bhi = b->hi();
    range::sort(alo, blo);
    range::sort(ahi, bhi);
    if (intersect && range::sort(blo, ahi))
        ahi = blo;
    range_g r = range::make(a->type(),
                            intersect ? blo : alo,
                            intersect ? ahi : bhi);
    if (!r || !rt.drop() || !rt.top(r))
        return object::ERROR;
    return object::OK;
}


COMMAND_BODY(RangeUnion)
// ----------------------------------------------------------------------------
//  Perform union between two ranges
// ----------------------------------------------------------------------------
{
    return range_op(false);
}


COMMAND_BODY(RangeIntersect)
// ----------------------------------------------------------------------------
//  Perform intersection between two ranges
// ----------------------------------------------------------------------------
{
    return range_op(true);
}


// ============================================================================
//
//   Arithmetic on uncertain numbers
//
// ============================================================================

static algebraic_p rho()
// ----------------------------------------------------------------------------
//   Return the value of the correlation coefficient ρ variable
// ----------------------------------------------------------------------------
{
    if (symbol_p name = symbol::make("ρ"))
        if (object_p value = directory::recall_all(name, false))
            if (value->is_real())
                return algebraic_p(value);
    return nullptr;
}


static uncertain_p bivariate(uncertain_r x, uncertain_r y,
                             arithmetic_fn f,
                             arithmetic_fn dfdx = nullptr,
                             arithmetic_fn dfdy = nullptr)
// ----------------------------------------------------------------------------
//   Compute bivariate function
// ----------------------------------------------------------------------------
//   If !dfdx or !dfdy, then the value is assumed to be 1
{
    if (!x || !y)
        return nullptr;
    algebraic_g xs = x->stddev();
    algebraic_g ys = y->stddev();
    algebraic_g xa = x->average();
    algebraic_g ya = y->average();

    algebraic_g fa = f(xa, ya);
    algebraic_g dxv = dfdx ? dfdx(xa, ya) : nullptr;
    algebraic_g dyv = dfdy ? dfdy(xa, ya) : nullptr;
    dxv = dxv ? dxv * xs : xs;
    dyv = dyv ? dyv * ys : ys;
    algebraic_g fs = dxv * dxv + dyv * dyv;
    if (algebraic_g r = rho())
    {
        algebraic_g cov = dxv * dyv * r;
        fs = fs + (cov + cov);
    }
    if (fs && !fs->is_infinity())
        fs = sqrt::run(fs);
    if (!fa || !fs)
        return nullptr;
    return uncertain::make(fa, fs);
}


uncertain_p operator-(uncertain_r x)
// ----------------------------------------------------------------------------
//  Unary minus for uncertains
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    return uncertain::make(-x->average(), x->stddev());
}


uncertain_p operator+(uncertain_r x, uncertain_r y)
// ----------------------------------------------------------------------------
//   Uncertain addition
// ----------------------------------------------------------------------------
{
    return bivariate(x, y, add::evaluate);
}


static algebraic_p subtract_dfdy(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   Return -1
// ----------------------------------------------------------------------------
{
    return integer::make(-1);
}

uncertain_p operator-(uncertain_r x, uncertain_r y)
// ----------------------------------------------------------------------------
//   Uncertain subtraction
// ----------------------------------------------------------------------------
{
    return bivariate(x, y, subtract::evaluate, nullptr, subtract_dfdy);
}


static algebraic_p mul_dfdx(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x*y)/dx = y
// ----------------------------------------------------------------------------
{
    return y;
}


static algebraic_p mul_dfdy(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x*y)/dy = x
// ----------------------------------------------------------------------------
{
    return x;
}


uncertain_p operator*(uncertain_r x, uncertain_r y)
// ----------------------------------------------------------------------------
//   Uncertain multiplication
// ----------------------------------------------------------------------------
{
    return bivariate(x, y, multiply::evaluate, mul_dfdx, mul_dfdy);
}


static algebraic_p div_dfdx(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x/y)/dx = 1/y
// ----------------------------------------------------------------------------
{
    return inv::evaluate(y);
}


static algebraic_p div_dfdy(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x/y)/dy = -x/y^2
// ----------------------------------------------------------------------------
{
    return -x / (y*y);
}


uncertain_p operator/(uncertain_r x, uncertain_r y)
// ----------------------------------------------------------------------------
//   Uncertain division
// ----------------------------------------------------------------------------
{
    return bivariate(x, y, divide::evaluate, div_dfdx, div_dfdy);
}


static algebraic_p pow_dfdx(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x^y)/dx = y*x^(y-1)
// ----------------------------------------------------------------------------
{
    algebraic_g one = integer::make(1);
    return y * pow::run(x, y - one);
}


static algebraic_p pow_dfdy(algebraic_r x, algebraic_r y)
// ----------------------------------------------------------------------------
//   d(x/y)/dy = ln(x) * x^y
// ----------------------------------------------------------------------------
{
    return ln::run(x) * pow::run(x, y);
}


uncertain_p operator^(uncertain_r x, uncertain_r y)
// ----------------------------------------------------------------------------
//   Power operator for uncertain numbers
// ----------------------------------------------------------------------------
{
    return bivariate(x, y, pow::evaluate, pow_dfdx, pow_dfdy);
}



// ============================================================================
//
//   Functions on uncertain numbers
//
// ============================================================================

static algebraic_p approx_dfdx(algebraic_r x, algebraic_fn f)
// ----------------------------------------------------------------------------
//   Approximate differentiation when no symbolic differential exists
// ----------------------------------------------------------------------------
{
    algebraic_g eps = decimal::make(1,-6);
    algebraic_g h = x * eps;
    algebraic_g h2 = h + h;
    algebraic_g fp = f(x + h);
    algebraic_g fn = f(x - h);
    algebraic_g df = fp - fn;
    algebraic_g fp2 = f(x + h2);
    algebraic_g fn2 = f(x - h2);
    algebraic_g df2 = fp2 - fn2;
    df = df + df;               // x2
    df = df + df;               // x4
    df = df + df;               // x8
    df = df2 - df;
    h = h + h2;                 // x3
    h = h + h;                  // x6
    h = h + h;                  // x12

    return df / h;
}


static uncertain_p univariate(algebraic_fn f,
                              uncertain_r   x,
                              algebraic_fn dfdx = nullptr)
// ----------------------------------------------------------------------------
//   Unary functions
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    algebraic_g xs = x->stddev();
    algebraic_g xa = x->average();

    algebraic_g fa = f(xa);
    algebraic_g dxv = dfdx ? dfdx(xa) : approx_dfdx(xa, f);
    algebraic_g fs = abs::evaluate(dxv * xs);

    if (!fa || !fs)
        return nullptr;
    return uncertain::make(fa, fs);
}



UNCERTAIN_BODY(sqrt)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of sqrt (monotonic)
// ----------------------------------------------------------------------------
{
    return univariate(sqrt::evaluate, u);
}


UNCERTAIN_BODY(cbrt)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of cbrt (monotonic)
// ----------------------------------------------------------------------------
{
    return univariate(cbrt::evaluate, u);
}


UNCERTAIN_BODY(sin)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of sin
// ----------------------------------------------------------------------------
{
    return univariate(sin::evaluate, u);
}

UNCERTAIN_BODY(cos)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of cos
// ----------------------------------------------------------------------------
{
    return univariate(cos::evaluate, u);
}


UNCERTAIN_BODY(tan)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of tan
// ----------------------------------------------------------------------------
{
    return univariate(tan::evaluate, u);
}


UNCERTAIN_BODY(asin)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of asin
// ----------------------------------------------------------------------------
{
    return univariate(asin::evaluate, u);
}


UNCERTAIN_BODY(acos)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of acos
// ----------------------------------------------------------------------------
{
    return univariate(acos::evaluate, u);
}


UNCERTAIN_BODY(atan)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of atan
// ----------------------------------------------------------------------------
{
    return univariate(atan::evaluate, u);
}


UNCERTAIN_BODY(sinh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of sinh
// ----------------------------------------------------------------------------
{
    return univariate(sinh::evaluate, u);
}


UNCERTAIN_BODY(cosh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of cosh
// ----------------------------------------------------------------------------
{
    return univariate(cosh::evaluate, u);
}


UNCERTAIN_BODY(tanh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of tanh
// ----------------------------------------------------------------------------
{
    return univariate(tanh::evaluate, u);
}


UNCERTAIN_BODY(asinh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of asinh
// ----------------------------------------------------------------------------
{
    return univariate(asinh::evaluate, u);
}


UNCERTAIN_BODY(acosh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of acosh
// ----------------------------------------------------------------------------
{
    return univariate(acosh::evaluate, u);
}


UNCERTAIN_BODY(atanh)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of atanh
// ----------------------------------------------------------------------------
{
    return univariate(atanh::evaluate, u);
}


UNCERTAIN_BODY(ln1p)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of ln1p
// ----------------------------------------------------------------------------
{
    return univariate(ln1p::evaluate, u);
}

UNCERTAIN_BODY(expm1)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of expm1
// ----------------------------------------------------------------------------
{
    return univariate(expm1::evaluate, u);
}


UNCERTAIN_BODY(ln)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of log
// ----------------------------------------------------------------------------
{
    return univariate(ln::evaluate, u);
}

UNCERTAIN_BODY(log10)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of log10
// ----------------------------------------------------------------------------
{
    return univariate(log10::evaluate, u);
}


UNCERTAIN_BODY(log2)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of log2
// ----------------------------------------------------------------------------
{
    return univariate(log2::evaluate, u);
}


UNCERTAIN_BODY(exp)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of exp
// ----------------------------------------------------------------------------
{
    return univariate(exp::evaluate, u);
}


UNCERTAIN_BODY(exp10)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of exp10
// ----------------------------------------------------------------------------
{
    return univariate(exp10::evaluate, u);
}


UNCERTAIN_BODY(exp2)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of exp2
// ----------------------------------------------------------------------------
{
    return univariate(exp2::evaluate, u);
}


UNCERTAIN_BODY(erf)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of erf
// ----------------------------------------------------------------------------
{
    return univariate(erf::evaluate, u);
}


UNCERTAIN_BODY(erfc)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of erfc
// ----------------------------------------------------------------------------
{
    return univariate(erfc::evaluate, u);
}


UNCERTAIN_BODY(tgamma)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of tgamma
// ----------------------------------------------------------------------------
{
    return univariate(tgamma::evaluate, u);
}


UNCERTAIN_BODY(lgamma)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of lgamma
// ----------------------------------------------------------------------------
{
    return univariate(lgamma::evaluate, u);
}


UNCERTAIN_BODY(abs)
// ----------------------------------------------------------------------------
//   Uncertain Number implementation of abs
// ----------------------------------------------------------------------------
{
    return univariate(abs::evaluate, u);
}
