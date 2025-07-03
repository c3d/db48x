#ifndef RANGE_H
#  define RANGE_H
// ****************************************************************************
//  range.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Range arithmetic
//
//    This defines two range types:
//    - range is an interval of numbers, showing as 1.245…1.267
//    - drange is a "delta" range that shows as 1.256±0.011
//    - prange is a "percent" range, shows as 1.256±0.876%
//    - uncertain is mean and standard deviation, showing as 1.245±σ0.067
//
//
// ****************************************************************************
//   (C) 2025 Christophe de Dinechin <christophe@dinechin.org>
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

#include "complex.h"

GCP(range);
GCP(uncertain);

struct range : complex
// ----------------------------------------------------------------------------
//    Number interval
// ----------------------------------------------------------------------------
{
    range(id type, algebraic_r x, algebraic_r y): complex(type, x, y)
    {}

    algebraic_g x() const
    {
        algebraic_p p = algebraic_p(payload(this));
        return p;
    }
    algebraic_g y() const
    {
        algebraic_p p = algebraic_p(payload(this));
        algebraic_p n = algebraic_p(byte_p(p) + p->size());
        return n;
    }
    algebraic_g         lo() const      { return x(); }
    algebraic_g         hi() const      { return y(); }
    bool                is_zero() const;
    bool                is_one()  const;
    algebraic_p         as_uncertain() const;

    static range_p      make(id type, algebraic_r x, algebraic_r y)
    {
        if (!x || !y)
            return nullptr;
        return rt.make<range>(type, x, y);
    }

    static bool sort(algebraic_g &x, algebraic_g &y);

    enum { INTERVAL_MARK = L'…' };

public:
    SIZE_DECL(range);
    OBJECT_DECL(range);
    PARSE_DECL(range);
    RENDER_DECL(range);
    HELP_DECL(range);

public:
    // Range implementation for main functions
#define RANGE_FUNCTION(name)    static range_p name(range_r r)

#define RANGE_BODY(name)        range_p range::name(range_r r)

    RANGE_FUNCTION(sqrt);
    RANGE_FUNCTION(cbrt);

    RANGE_FUNCTION(sin);
    RANGE_FUNCTION(cos);
    RANGE_FUNCTION(tan);
    RANGE_FUNCTION(asin);
    RANGE_FUNCTION(acos);
    RANGE_FUNCTION(atan);

    RANGE_FUNCTION(sinh);
    RANGE_FUNCTION(cosh);
    RANGE_FUNCTION(tanh);
    RANGE_FUNCTION(asinh);
    RANGE_FUNCTION(acosh);
    RANGE_FUNCTION(atanh);

    RANGE_FUNCTION(ln1p);
    RANGE_FUNCTION(expm1);
    RANGE_FUNCTION(ln);
    RANGE_FUNCTION(log10);
    RANGE_FUNCTION(log2);
    RANGE_FUNCTION(exp);
    RANGE_FUNCTION(exp10);
    RANGE_FUNCTION(exp2);
    RANGE_FUNCTION(erf);
    RANGE_FUNCTION(erfc);
    RANGE_FUNCTION(tgamma);
    RANGE_FUNCTION(lgamma);

    RANGE_FUNCTION(abs);
};


range_g operator-(range_r x);
range_g operator+(range_r x, range_r y);
range_g operator-(range_r x, range_r y);
range_g operator*(range_r x, range_r y);
range_g operator/(range_r x, range_r y);


struct drange : range
// ----------------------------------------------------------------------------
//   Delta range, like 1.256±0.011
// ----------------------------------------------------------------------------
{
    drange(id type, algebraic_r x, algebraic_r y);
    enum { PLUSMINUS_MARK = L'±' };

public:
    OBJECT_DECL(drange);
    RENDER_DECL(drange);
};


struct prange : drange
// ----------------------------------------------------------------------------
//   Percent range, like 1.256±0.876%
// ----------------------------------------------------------------------------
{
    prange(id type, algebraic_r x, algebraic_r y);

public:
    OBJECT_DECL(prange);
    RENDER_DECL(prange);
};


struct uncertain : range
// ----------------------------------------------------------------------------
//   Range based on the average and standard deviation
// ----------------------------------------------------------------------------
{
    uncertain(id type, algebraic_r avg, algebraic_r stddev)
        : range(type, avg, stddev) {}

    algebraic_g average()  const        { return x(); }
    algebraic_g stddev()  const         { return y(); }
    bool        is_zero() const;
    bool        is_one()  const;
    algebraic_p as_range(id type = ID_range) const;


    static uncertain_p make(algebraic_r a, algebraic_r s)
    {
        if (!a|| !s)
            return nullptr;
        return rt.make<uncertain>(a, s);
    }

    enum { SIGMA_MARK = L'σ' };


public:
    OBJECT_DECL(uncertain);
    RENDER_DECL(uncertain);
    HELP_DECL(uncertain);
};

#endif // RANGE_H
