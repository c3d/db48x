#ifndef COMPLEX_H
#define COMPLEX_H
// ****************************************************************************
//  complex.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Complex numbers
//
//      There are two representations for complex numbers:
//      - rectangular representation is one of X;Y, X+𝒊Y, X-𝒊Y, X+Y𝒊 or X-Y𝒊
//      - polar representation is X∡Y where X≥0 and Y is a ratio of π
//
//      Some settings control how complex numbers are rendered
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
//
// Payload format:
//
//   The payload is a simple sequence with the two parts of the complex


#include "algebraic.h"
#include "runtime.h"
#include "settings.h"

GCP(complex);
GCP(rectangular);
GCP(polar);

struct complex : algebraic
// ----------------------------------------------------------------------------
//    Base class shared by both rectangular and polar implementations
// ----------------------------------------------------------------------------
{
    complex(id type, algebraic_r x, algebraic_r y): algebraic(type)
    {
        byte *p = (byte *) this + leb128size(type);
        size_t xs = x->size();
        size_t ys = y->size();
        memcpy(p, byte_p(x), xs);
        p += xs;
        memcpy(p, byte_p(y), ys);
    }

    static size_t required_memory(id i, algebraic_r x, algebraic_r y)
    {
        return leb128size(i) + x->size() + y->size();
    }

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

    algebraic_g         re() const;
    algebraic_g         im() const;
    algebraic_g         mod() const;
    algebraic_g         arg(angle_unit unit) const;
    algebraic_g         pifrac() const;
    complex_g           conjugate() const;
    algebraic_p         is_real() const;

    polar_g             as_polar() const;
    rectangular_g       as_rectangular() const;
    complex_p           cycle() const;

    static complex_p    make(id type,
                             algebraic_r x, algebraic_r y,
                             angle_unit polar_unit);
    static rectangular_p make(int re = 0, int im = 1);

    enum { I_MARK = L'ⅈ', ANGLE_MARK = L'∡' };

public:
    SIZE_DECL(complex);
    PREC_DECL(COMPLEX);
    HELP_DECL(complex);

public:
    // Complex implementation for main functions
#define COMPLEX_FUNCTION(name)                  \
    static complex_g name(complex_r z)

#define COMPLEX_BODY(name)      complex_g complex::name(complex_r z)

    COMPLEX_FUNCTION(sqrt);
    COMPLEX_FUNCTION(cbrt);

    COMPLEX_FUNCTION(sin);
    COMPLEX_FUNCTION(cos);
    COMPLEX_FUNCTION(tan);
    COMPLEX_FUNCTION(asin);
    COMPLEX_FUNCTION(acos);
    COMPLEX_FUNCTION(atan);

    COMPLEX_FUNCTION(sinh);
    COMPLEX_FUNCTION(cosh);
    COMPLEX_FUNCTION(tanh);
    COMPLEX_FUNCTION(asinh);
    COMPLEX_FUNCTION(acosh);
    COMPLEX_FUNCTION(atanh);

    COMPLEX_FUNCTION(ln1p);
    COMPLEX_FUNCTION(expm1);
    COMPLEX_FUNCTION(ln);
    COMPLEX_FUNCTION(log10);
    COMPLEX_FUNCTION(log2);
    COMPLEX_FUNCTION(exp);
    COMPLEX_FUNCTION(exp10);
    COMPLEX_FUNCTION(exp2);
    COMPLEX_FUNCTION(erf);
    COMPLEX_FUNCTION(erfc);
    COMPLEX_FUNCTION(tgamma);
    COMPLEX_FUNCTION(lgamma);
};


complex_g operator-(complex_r x);
complex_g operator+(complex_r x, complex_r y);
complex_g operator-(complex_r x, complex_r y);
complex_g operator*(complex_r x, complex_r y);
complex_g operator/(complex_r x, complex_r y);


struct rectangular : complex
// ----------------------------------------------------------------------------
//   Rectangular representation for complex numbers
// ----------------------------------------------------------------------------
{
    rectangular(id type, algebraic_r re, algebraic_r im)
        : complex(type, re, im) {}

    algebraic_g re()  const     { return x(); }
    algebraic_g im()  const     { return y(); }
    algebraic_g mod() const;
    algebraic_g arg(angle_unit unit) const;
    algebraic_g pifrac() const;
    bool        is_zero() const;
    bool        is_one()  const;
    algebraic_p is_real() const;

    static rectangular_p make(algebraic_r r, algebraic_r i)
    {
        if (!r|| !i)
            return nullptr;
        return rt.make<rectangular>(r, i);
    }

public:
    OBJECT_DECL(rectangular);
    PARSE_DECL(rectangular);
    RENDER_DECL(rectangular);
};


struct polar : complex
// ----------------------------------------------------------------------------
//   Polar representation for complex numbers
// ----------------------------------------------------------------------------
{
    polar(id type, algebraic_r mod, algebraic_r pifrac)
        : complex(type, mod, pifrac) {}

    algebraic_g re()  const;
    algebraic_g im()  const;
    algebraic_g mod() const;
    algebraic_g arg(angle_unit unit) const;
    algebraic_g pifrac() const  { return y(); }
    bool        is_zero() const;
    bool        is_one()  const;
    algebraic_p is_real() const;

    static polar_p make(algebraic_r mod, algebraic_r arg, angle_unit unit);

public:
    OBJECT_DECL(polar);
    PARSE_DECL(polar);          // Just skips, actual work in 'rectangular'
    RENDER_DECL(polar);
};

COMMAND_DECLARE(RealToRectangular,2);
COMMAND_DECLARE(RealToPolar,2);
COMMAND_DECLARE(RectangularToReal,1);
COMMAND_DECLARE(PolarToReal,1);
COMMAND_DECLARE(ToRectangular,1);
COMMAND_DECLARE(ToPolar,1);

#endif // COMPLEX_H
