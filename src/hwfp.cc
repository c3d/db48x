// ****************************************************************************
//  hwfp.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Support code for hardware floating-point support
//
//
//
//
//
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

#include "hwfp.h"

#include "arithmetic.h"
#include "parser.h"
#include "settings.h"

#include <cmath>


size_t hwfp_base::render(renderer &r, double x, char suffix)
// ----------------------------------------------------------------------------
//   Render the value, ignoring formatting for now
// ----------------------------------------------------------------------------
{
    if (std::isfinite(x))
    {
        decimal_g dec = decimal::from(x);
        if (dec && dec->render(r))
        {
            r.put(suffix);
            return r.size();
        }
    }
    else if (std::isinf(x))
    {
        r.put(x < 0 ? "-∞" : "∞");
    }
    else
    {
        r.put("NaN");
    }
    return r.size();
}


template <typename hw>
algebraic_p hwfp<hw>::to_fraction(uint count, uint prec) const
// ----------------------------------------------------------------------------
//   Convert hwfp number to fraction
// ----------------------------------------------------------------------------
{
    hw   num = value();
    bool neg = num < 0;
    if (neg)
        num = -num;

    hw whole_part   = std::floor(num);
    hw decimal_part = num - whole_part;
    if (decimal_part == 0.0)
        return to_integer();

    hw   v1num  = whole_part;
    hw   v1den  = 1.0;
    hw   v2num  = 1.0;
    hw   v2den  = 0.0;

    uint maxdec = Settings.Precision() - 3;
    if (prec > maxdec)
        prec = maxdec;

    // Limit fraction precision to displayed digits (DisplayDigits) like HP50G
    uint dispdig = Settings.DisplayDigits();
    if (dispdig > 0 && prec > dispdig)
        prec = dispdig;

    hw eps = std::exp(-hw(prec) * M_LN10);

    while (count--)
    {
        // Check if the decimal part is small enough
        if (decimal_part == 0.0)
            break;

        hw next = 1.0 / decimal_part;
        whole_part = std::floor(next);

        hw s = v1num;
        v1num = whole_part * v1num + v2num;
        v2num = s;

        s = v1den;
        v1den = whole_part * v1den + v2den;
        v2den = s;

        // Check convergence: break when |num - n/d| < 10^(-prec)
        hw convergent = v1num / v1den;
        hw err = num - convergent;
        if (err < 0)
            err = -err;
        if (err < eps)
            break;

        decimal_part = next - whole_part;
    }

    ularge      numerator   = ularge(v1num);
    ularge      denominator = ularge(v1den);
    algebraic_g result;
    if (denominator == 1)
        result = +integer::make(numerator);
    else
        result = +fraction::make(integer::make(numerator),
                                 integer::make(denominator));
    if (neg)
        result = -result;
    return +result;

}



template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::neg(hwfp_r x)
// ----------------------------------------------------------------------------
//   Negation
// ----------------------------------------------------------------------------
{
    return make(-x->value());
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::add(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Addition
// ----------------------------------------------------------------------------
{
    add::remember(target<add>);
    return make(x->value() + y->value());
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::subtract(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Subtraction
// ----------------------------------------------------------------------------
{
    subtract::remember(target<subtract>);
    return make(x->value() - y->value());
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::multiply(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Multiplication
// ----------------------------------------------------------------------------
{
    multiply::remember(target<multiply>);
    return make(x->value() * y->value());
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::divide(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Division
// ----------------------------------------------------------------------------
{
    divide::remember(target<divide>);
    hw fy = y->value();
    if (fy == 0.0)
    {
        rt.zero_divide_error();
        return nullptr;
    }
    return make(x->value() / fy);
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::mod(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Modulo
// ----------------------------------------------------------------------------
{
    mod::remember(target<mod>);
    hw fy = y->value();
    if (fy == 0.0)
    {
        rt.zero_divide_error();
        return nullptr;
    }
    hw fx = x->value();
    fx    = ::fmod(fx, fy);
    if (fx < 0)
        fx = fy < 0 ? fx - fy : fx + fy;
    return make(fx);
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::rem(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Remainder
// ----------------------------------------------------------------------------
{
    rem::remember(target<rem>);
    hw fy = y->value();
    if (fy == 0.0)
    {
        rt.zero_divide_error();
        return nullptr;
    }
    return make(std::fmod(x->value(), fy));
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::pow(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Power
// ----------------------------------------------------------------------------
{
    pow::remember(target<pow>);
    return make(std::pow(x->value(), y->value()));
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::hypot(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Hypothenuse
// ----------------------------------------------------------------------------
{
    hypot::remember(target<hypot>);
    return make(std::hypot(x->value(), y->value()));
}

template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::atan2(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Arctangent for two lengths
// ----------------------------------------------------------------------------
{
    atan2::remember(target<atan2>);
    return make(to_angle(std::atan2(x->value(), y->value())));
}


template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::Min(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Minimum
// ----------------------------------------------------------------------------
{
    hw fx = x->value();
    hw fy = y->value();
    return make(fx < fy ? fx : fy);
}

template<typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::Max(hwfp_r x, hwfp_r y)
// ----------------------------------------------------------------------------
//   Maximum1
// ----------------------------------------------------------------------------
{
    hw fx = x->value();
    hw fy = y->value();
    return make(fx > fy ? fx : fy);
}


template algebraic_p hwfp<float>::to_fraction(uint count, uint prec) const;
template algebraic_p hwfp<double>::to_fraction(uint count, uint prec) const;

#define ARITH1(name)     ARITH1I(name, float); ARITH1I(name, double)
#define ARITH1I(name,ty)                                                 \
    template hwfp<ty>::hwfp_p hwfp<ty>::name(hwfp<ty>::hwfp_r);
#define ARITH2(name)     ARITH2I(name, float); ARITH2I(name, double)
#define ARITH2I(name,ty)                                                \
    template hwfp<ty>::hwfp_p hwfp<ty>::name(hwfp<ty>::hwfp_r,          \
                                             hwfp<ty>::hwfp_r);

ARITH1(neg);
ARITH2(add);
ARITH2(subtract);
ARITH2(multiply);
ARITH2(divide);
ARITH2(mod);
ARITH2(rem);
ARITH2(pow);
ARITH2(hypot);
ARITH2(atan2);
ARITH2(Min);
ARITH2(Max);


#if LGAMMA_CRASHES
// ****************************************************************************
//
//   Local implementation of lgamma and lgammaf from newlib (bug #1532)
//
// ****************************************************************************
//   This is based on fdlibm (Freely Distributable Math Library)
//   Source: https://github.com/bminor/newlib
//   The reason for that is a crash in lgamma / lgammaf on DM32 / DM42N

static const double
// ----------------------------------------------------------------------------
// Constants for lgamma computation
// ----------------------------------------------------------------------------
    two52       = 4.50359962737049600000e+15,   // 0x43300000, 0x00000000
    half        = 5.00000000000000000000e-01,   // 0x3FE00000, 0x00000000
    one         = 1.00000000000000000000e+00,
    pi          = 3.14159265358979311600e+00,   // 0x400921FB, 0x54442D18

    // Polynomial coefficients for lgamma(x) in the primary interval [2,3]
    a0          = 7.72156649015328655494e-02,   // 0x3FB3C467, 0xE37DB0C8
    a1          = 3.22467033424113591611e-01,   // 0x3FD4A34C, 0xC4A60FAD
    a2          = 6.73523010531292681824e-02,   // 0x3FB13E00, 0x1A5562A7
    a3          = 2.05808084325167332806e-02,   // 0x3F951322, 0xAC92547B
    a4          = 7.38555086081402883957e-03,   // 0x3F7E404F, 0xB68FEFE8
    a5          = 2.89051383673415629091e-03,   // 0x3F67ADD8, 0xCCB7926B
    a6          = 1.19270763183362067845e-03,   // 0x3F538A94, 0x116F3F5D
    a7          = 5.10069792153511336608e-04,   // 0x3F40B6C6, 0x89B99C00
    a8          = 2.20862790713908385557e-04,   // 0x3F2CF2EC, 0xED10E54D
    a9          = 1.08011567247583939954e-04,   // 0x3F1C5088, 0x987DFB07
    a10         = 2.52144565451257326939e-05,   // 0x3EFA7074, 0x428CFA52
    a11         = 4.48640949618915160150e-05,   // 0x3F07858E, 0x90A45837

    // Rational approximation coefficients for sin(pi*x)
    tc          = 1.46163214496836224576e+00,   // 0x3FF762D8, 0x6356BE3F
    tf          = -1.21486290535849611461e-01,  // 0xBFBF19B9, 0xBCC38A42

    // Coefficients for lgamma(x) in [-0.2, 0.2]
    tt          = -3.63867699703950536541e-18,  // 0xBC50C7CA, 0xA48A971F
    t0          = 4.83836122723810047042e-01,   // 0x3FDEF72B, 0xC8EE38A2
    t1          = -1.47587722994593911752e-01,  // 0xBFC2E427, 0x8DC6C509
    t2          = 6.46249402391333854778e-02,   // 0x3FB08B42, 0x94D5419B
    t3          = -3.27885410759859649565e-02,  // 0xBFA0C9A8, 0xDF35B713
    t4          = 1.79706750811820387126e-02,   // 0x3F9266E7, 0x970AF9EC
    t5          = -1.03142241298341437450e-02,  // 0xBF851F9F, 0xBA91EC6A
    t6          = 6.10053870246291332635e-03,   // 0x3F78FCE0, 0xAEFDDD27
    t7          = -3.68452016781138256760e-03,  // 0xBF6E2EFF, 0xB3E914D7
    t8          = 2.25964780900612472250e-03,   // 0x3F6282D3, 0x2E15C915
    t9          = -1.40346469989232843813e-03,  // 0xBF56FE8E, 0xBF2D1AF1
    t10         = 8.81081882437654011382e-04,   // 0x3F4CDF0C, 0xEF61A8E9
    t11         = -5.38595305356740546715e-04,  // 0xBF41A610, 0x9C73E0EC
    t12         = 3.15632070903625950361e-04,   // 0x3F34AF6D, 0x6C0EBBF7
    t13         = -3.12754168375120860518e-04,  // 0xBF347F24, 0xECC38C38
    t14         = 3.35529192635519073543e-04,   // 0x3F35FD3E, 0xE8C2D3F4

    // Coefficients for lgamma(x+13) = x*P(x) + y
    u0          = -7.72156649015328655494e-02,  // 0xBFB3C467, 0xE37DB0C8
    u1          = 6.32827064025093366517e-01,   // 0x3FE4401E, 0x8B005DFF
    u2          = 1.45492250137234768737e+00,   // 0x3FF7475C, 0xD119BD6F
    u3          = 9.77717527963372745603e-01,   // 0x3FEF4976, 0x44EA8450
    u4          = 2.28963728064692451092e-01,   // 0x3FCD4EAE, 0xF6010924
    u5          = 1.33810918536787660377e-02,   // 0x3F8B678B, 0xBF2BAB09

    v1          = 2.45597793713041134822e+00,   // 0x4003A5D7, 0xC2BD619C
    v2          = 2.12848976379893395361e+00,   // 0x40010725, 0xA42B18F5
    v3          = 7.69285150456672783825e-01,   // 0x3FE89DFB, 0xE45050AF
    v4          = 1.04222645593369134254e-01,   // 0x3FBAAE55, 0xD6537C88
    v5          = 3.21709242282423911810e-03,   // 0x3F6A5ABB, 0x57D0CF61

    // Coefficients for lgamma(2+s) = s*(s*R(s^2) + y)
    s0          = -7.72156649015328655494e-02,  // 0xBFB3C467, 0xE37DB0C8
    s1          = 2.14982415960608852501e-01,   // 0x3FCB848B, 0x36E20878
    s2          = 3.25778796408930981787e-01,   // 0x3FD4D98F, 0x4F139F59
    s3          = 1.46350472652464452805e-01,   // 0x3FC2BB9C, 0xBEE5F2F7
    s4          = 2.66422703033638609560e-02,   // 0x3F9B481C, 0x7E939961
    s5          = 1.84028451407337715652e-03,   // 0x3F5E26B6, 0x7368F239
    s6          = 3.19475326584100867617e-05,   // 0x3F00BFEC, 0xDD17E945

    r1          = 1.39200533666097526988e+00,   // 0x3FF645A7, 0x62C4AB74
    r2          = 7.21935547567138069525e-01,   // 0x3FE71A18, 0x93D3DCDC
    r3          = 1.71933865632803078993e-01,   // 0x3FC601ED, 0xCCFBDF27
    r4          = 1.86459191715652901344e-02,   // 0x3F9317EA, 0x742ED475
    r5          = 7.77942496381893596434e-04,   // 0x3F497DDA, 0xCA41A95B
    r6          = 7.32668430744625636189e-06,   // 0x3EDEBAF7, 0xA5B38140

    // Coefficients for lgamma(x) in [1.7316,2] (used in negative path)
    w0          = 4.18938533204672725052e-01,   // 0x3FDACFE3, 0x90C97D69
    w1          = 8.33333333333329678849e-02,   // 0x3FB55555, 0x5555553B
    w2          = -2.77777777728775536470e-03,  // 0xBF66C16C, 0x16B02E5C
    w3          = 7.93650558643019558500e-04,   // 0x3F4A019F, 0x98CF38B6
    w4          = -5.95187557450339963135e-04,  // 0xBF4380CB, 0x8C0FE741
    w5          = 8.36339918996282139126e-04,   // 0x3F4B67BA, 0x4CDAD5D1
    w6          = -1.63092934096575273989e-03;  // 0xBF5AB89D, 0x0B9E43E4


static double local_sin_pi(double x)
// ----------------------------------------------------------------------------
//   Compute sin(pi*x) for use in lgamma reflection formula
// ----------------------------------------------------------------------------
{
    double y, z;
    int n, ix;

    union { double d; uint64_t i; } u;
    u.d = x;
    ix = (u.i >> 32) & 0x7fffffff;

    if (ix < 0x3fd00000)  // |x| < 0.25
        return sin(pi * x);

    y = -x;  // negative x is assumed
    z = floor(y);
    if (y != z)
        y = y - z;
    y = 2.0 * y;
    n = (int)(y);
    y = y - n;
    switch(n)
    {
    case 0:  y =  sin(pi * y); break;
    case 1:
    case 2:  y =  cos(pi * (0.5 - y)); break;
    case 3:
    case 4:  y =  sin(pi * (1.0 - y)); break;
    case 5:
    case 6:  y = -cos(pi * (y - 1.5)); break;
    default: y =  sin(pi * (y - 2.0)); break;
    }
    return -y;
}


static double local_lgamma(double x)
// ----------------------------------------------------------------------------
//   Compute lgamma(x) = log|Gamma(x)|
// ----------------------------------------------------------------------------
{
    double t, y, z, nadj, p, p1, p2, p3, q, r, w;
    int i, hx, lx, ix;

    union { double d; uint64_t i; } u;
    u.d = x;
    hx = u.i >> 32;
    lx = u.i & 0xffffffff;

    // Purge off +-inf, NaN, +-0, and negative integers
    ix = hx & 0x7fffffff;
    if (ix >= 0x7ff00000)
        return x * x;
    if ((ix | lx) == 0 || hx < 0)
    {
        if (x == 0.0)
            return one / fabs(x);  // +-Inf
        nadj = 0.0;
        if (hx < 0)  // negative x
        {
            if (ix >= 0x43300000)  // |x| >= 2^52, must be -integer
                return one / (x - x);
            t = local_sin_pi(x);
            if (t == 0.0)
                return one / (x - x);  // -integer
            nadj = log(pi / fabs(t * x));
            x = -x;
        }

        // lgamma(x) for x < 2.0
        if (ix < 0x40000000)  // |x| < 2.0
        {
            if (ix <= 0x3feccccc)  // |x| <= 0.9
            {
                // lgamma(x) for |x| < 0.5
                if (ix < 0x3fe00000)  // |x| < 0.5
                {
                    if (ix >= 0x3f9d6289)  // |x| >= 2^-28
                    {
                        r = -log(x);
                        if (ix >= 0x3fcda661)  // |x| >= 0.4
                            y = one - x;
                        else
                        {
                            i = (int)x;
                            y = x - (double)i;
                        }
                    }
                    else
                    {
                        r = -log(x);
                        y = 0.0;
                    }

                    // Evaluate polynomial
                    z = y * y;
                    p1 = a0 +
                         z * (a2 + z * (a4 + z * (a6 + z * (a8 + z * a10))));
                    p2 = z *
                         (a1 +
                          z * (a3 + z * (a5 + z * (a7 + z * (a9 + z * a11)))));
                    p = y * p1 + p2;
                    r += (p - 0.5 * y);
                }
                else  // 0.5 <= |x| <= 0.9
                {
                    r = 0.0;
                    if (ix >= 0x3ff00000)  // x >= 1.0
                    {
                        y = x - one;
                        i = 0;
                    }
                    else if (ix >= 0x3fe76944)  // x >= 0.7316
                    {
                        y = x - (tc - one);
                        i = 1;
                    }
                    else  // 0.5 <= x < 0.7316
                    {
                        y = x;
                        i = 2;
                    }

                    switch(i)
                    {
                    case 0:
                        z = y * y;
                        p1 =
                            a0 +
                            z * (a2 + z * (a4 + z * (a6 + z * (a8 + z * a10))));
                        p2 = z *
                             (a1 +
                              z * (a3 +
                                   z * (a5 + z * (a7 + z * (a9 + z * a11)))));
                        p = y * p1 + p2;
                        r += (p - 0.5 * y);
                        break;
                    case 1:
                        z = y * y;
                        w = z * y;
                        p1 = t0 + w * (t3 + w * (t6 + w * (t9 + w * t12)));
                        p2 = t1 + w * (t4 + w * (t7 + w * (t10 + w * t13)));
                        p3 = t2 + w * (t5 + w * (t8 + w * (t11 + w * t14)));
                        p = z * p1 - (tt - w * (p2 + y * p3));
                        r += (tf + p);
                        break;
                    case 2:
                        p1 =
                            y * (u0 +
                                 y * (u1 +
                                      y * (u2 + y * (u3 + y * (u4 + y * u5)))));
                        p2 = one +
                             y * (v1 + y * (v2 + y * (v3 + y * (v4 + y * v5))));
                        r += (-0.5 * y + p1 / p2);
                        break;
                    }
                }
            }
            else  // 0.9 < x < 2.0
            {
                r = 0.0;
                if (ix >= 0x3ffbb4c3)  // x >= 1.7316
                {
                    // [1.7316,2]
                    y = 2.0 - x;
                    i = 0;
                }
                else if (ix >= 0x3ff3b4c4)  // x >= 1.23
                {
                    y = x - tc;
                    i = 1;
                }
                else  // 0.9 < x < 1.23
                {
                    y = x - one;
                    i = 2;
                }

                switch(i)
                {
                case 0:
                    z = y * y;
                    w = w0 +
                        z * (w1 +
                             z * (w2 +
                                  z * (w3 + z * (w4 + z * (w5 + z * w6)))));
                    r += -0.5 * y + w;
                    break;
                case 1:
                    z = y * y;
                    w = z * y;
                    p1 = t0 + w * (t3 + w * (t6 + w * (t9 + w * t12)));
                    p2 = t1 + w * (t4 + w * (t7 + w * (t10 + w * t13)));
                    p3 = t2 + w * (t5 + w * (t8 + w * (t11 + w * t14)));
                    p = z * p1 - (tt - w * (p2 + y * p3));
                    r += (tf + p);
                    break;
                case 2:
                    p1 = y *
                         (s0 +
                          y * (s1 +
                               y * (s2 +
                                    y * (s3 + y * (s4 + y * (s5 + y * s6))))));
                    p2 = one +
                         y * (r1 +
                              y * (r2 +
                                   y * (r3 + y * (r4 + y * (r5 + y * r6)))));
                    r += (-0.5 * y + p1 / p2);
                    break;
                }
            }
        }
        else  // 2.0 <= x < 8.0
        {
            if (ix < 0x40200000)
            {
                i = (int)x;
                t = 0.0;
                y = x - (double)i;
                p = y *
                    (s0 +
                     y * (s1 +
                          y * (s2 + y * (s3 + y * (s4 + y * (s5 + y * s6))))));
                q = one +
                    y * (r1 +
                         y * (r2 + y * (r3 + y * (r4 + y * (r5 + y * r6)))));
                r = half * y + p / q;
                z = one;  // lgamma(1+s) = log(s) + lgamma(s)
                switch(i)
                {
                case 7: z *= (y + 6.0); /* FALLTHRU */
                case 6: z *= (y + 5.0); /* FALLTHRU */
                case 5: z *= (y + 4.0); /* FALLTHRU */
                case 4: z *= (y + 3.0); /* FALLTHRU */
                case 3: z *= (y + 2.0); /* FALLTHRU */
                        r += log(z); break;
                }
            }
            else if (ix < 0x43900000)  // 8.0 <= x < 2^58
            {
                t = log(x);
                z = one / x;
                y = z * z;
                w = w0 +
                    z * (w1 +
                         y * (w2 + y * (w3 + y * (w4 + y * (w5 + y * w6)))));
                r = (x - half) * (t - one) + w;
            }
            else  // 2^58 <= x <= Inf
            {
                r = x * (log(x) - one);
            }
        }

        if (hx < 0)
            r = nadj - r;
        return r;
    }

    // x >= 2.0
    if (ix < 0x40200000)  // x < 8.0
    {
        i = (int)x;
        t = 0.0;
        y = x - (double)i;
        p = y * (s0 +
                 y * (s1 + y * (s2 + y * (s3 + y * (s4 + y * (s5 + y * s6))))));
        q = one + y * (r1 + y * (r2 + y * (r3 + y * (r4 + y * (r5 + y * r6)))));
        r = half * y + p / q;
        z = one;
        switch(i)
        {
        case 7: z *= (y + 6.0); /* FALLTHRU */
        case 6: z *= (y + 5.0); /* FALLTHRU */
        case 5: z *= (y + 4.0); /* FALLTHRU */
        case 4: z *= (y + 3.0); /* FALLTHRU */
        case 3: z *= (y + 2.0); /* FALLTHRU */
                r += log(z); break;
        }
    }
    else if (ix < 0x43900000)  // 8.0 <= x < 2^58
    {
        t = log(x);
        z = one / x;
        y = z * z;
        w = w0 + z * (w1 + y * (w2 + y * (w3 + y * (w4 + y * (w5 + y * w6)))));
        r = (x - half) * (t - one) + w;
    }
    else  // 2^58 <= x <= Inf
    {
        r = x * (log(x) - one);
    }
    return r;
}


static const float
// ----------------------------------------------------------------------------
//   Constants for the float implementation of lgammaf
// ----------------------------------------------------------------------------
    twof52      = 4.50359962737049600000e+15f,
    halff       = 5.00000000000000000000e-01f,
    onef        = 1.00000000000000000000e+00f,
    pif         = 3.14159265358979311600e+00f,
    a0f         = 7.72156649015328655494e-02f,
    a1f         = 3.22467033424113591611e-01f,
    a2f         = 6.73523010531292681824e-02f,
    a3f         = 2.05808084325167332806e-02f,
    a4f         = 7.38555086081402883957e-03f,
    a5f         = 2.89051383673415629091e-03f,
    a6f         = 1.19270763183362067845e-03f,
    a7f         = 5.10069792153511336608e-04f,
    a8f         = 2.20862790713908385557e-04f,
    a9f         = 1.08011567247583939954e-04f,
    a10f        = 2.52144565451257326939e-05f,
    a11f        = 4.48640949618915160150e-05f,
    tcf         = 1.46163214496836224576e+00f,
    tff         = -1.21486290535849611461e-01f,
    ttf         = -3.63867699703950536541e-18f,
    t0f         = 4.83836122723810047042e-01f,
    t1f         = -1.47587722994593911752e-01f,
    t2f         = 6.46249402391333854778e-02f,
    t3f         = -3.27885410759859649565e-02f,
    t4f         = 1.79706750811820387126e-02f,
    t5f         = -1.03142241298341437450e-02f,
    t6f         = 6.10053870246291332635e-03f,
    t7f         = -3.68452016781138256760e-03f,
    t8f         = 2.25964780900612472250e-03f,
    t9f         = -1.40346469989232843813e-03f,
    t10f        = 8.81081882437654011382e-04f,
    t11f        = -5.38595305356740546715e-04f,
    t12f        = 3.15632070903625950361e-04f,
    t13f        = -3.12754168375120860518e-04f,
    t14f        = 3.35529192635519073543e-04f,
    u0f         = -7.72156649015328655494e-02f,
    u1f         = 6.32827064025093366517e-01f,
    u2f         = 1.45492250137234768737e+00f,
    u3f         = 9.77717527963372745603e-01f,
    u4f         = 2.28963728064692451092e-01f,
    u5f         = 1.33810918536787660377e-02f,
    v1f         = 2.45597793713041134822e+00f,
    v2f         = 2.12848976379893395361e+00f,
    v3f         = 7.69285150456672783825e-01f,
    v4f         = 1.04222645593369134254e-01f,
    v5f         = 3.21709242282423911810e-03f,
    s0f         = -7.72156649015328655494e-02f,
    s1f         = 2.14982415960608852501e-01f,
    s2f         = 3.25778796408930981787e-01f,
    s3f         = 1.46350472652464452805e-01f,
    s4f         = 2.66422703033638609560e-02f,
    s5f         = 1.84028451407337715652e-03f,
    s6f         = 3.19475326584100867617e-05f,
    r1f         = 1.39200533666097526988e+00f,
    r2f         = 7.21935547567138069525e-01f,
    r3f         = 1.71933865632803078993e-01f,
    r4f         = 1.86459191715652901344e-02f,
    r5f         = 7.77942496381893596434e-04f,
    r6f         = 7.32668430744625636189e-06f,
    w0f         = 4.18938533204672725052e-01f,
    w1f         = 8.33333333333329678849e-02f,
    w2f         = -2.77777777728775536470e-03f,
    w3f         = 7.93650558643019558500e-04f,
    w4f         = -5.95187557450339963135e-04f,
    w5f         = 8.36339918996282139126e-04f,
    w6f         = -1.63092934096575273989e-03f;


static float local_sin_pif(float x)
// ----------------------------------------------------------------------------
//   Compute sin(pi*x) for use in lgammaf reflection formula
// ----------------------------------------------------------------------------
{
    float y, z;
    int n, ix;

    union { float f; uint32_t i; } u;
    u.f = x;
    ix = u.i & 0x7fffffff;

    if (ix < 0x3e800000)  // |x| < 0.25
        return sinf(pif * x);

    y = -x;
    z = floorf(y);
    if (y != z)
        y = y - z;
    y = 2.0f * y;
    n = (int)(y);
    y = y - n;
    switch(n)
    {
    case 0:  y =  sinf(pif * y); break;
    case 1:
    case 2:  y =  cosf(pif * (0.5f - y)); break;
    case 3:
    case 4:  y =  sinf(pif * (1.0f - y)); break;
    case 5:
    case 6:  y = -cosf(pif * (y - 1.5f)); break;
    default: y =  sinf(pif * (y - 2.0f)); break;
    }
    return -y;
}


static float local_lgammaf(float x)
// ----------------------------------------------------------------------------
//   Compute lgammaf(x) = log|Gamma(x)| for float
// ----------------------------------------------------------------------------
{
    float t, y, z, nadj, p, p1, p2, p3, q, r, w;
    int   i, hx, ix;

    union
    {
        float    f;
        uint32_t i;
    } u;
    u.f = x;
    hx  = u.i;

    ix  = hx & 0x7fffffff;
    if (ix >= 0x7f800000)
        return x * x;
    if (ix == 0)
        return onef / fabsf(x);

    if (hx < 0) // negative x
    {
        if (ix >= 0x4b000000) // |x| >= 2^23, must be -integer
            return onef / (x - x);
        t = local_sin_pif(x);
        if (t == 0.0f)
            return onef / (x - x);
        nadj = logf(pif / fabsf(t * x));
        x    = -x;
    }
    else
    {
        nadj = 0.0f;
    }

    // lgammaf(x) for x < 2.0
    if (ix < 0x40000000) // |x| < 2.0
    {
        if (ix <= 0x3f666666) // |x| <= 0.9
        {
            if (ix < 0x3f000000) // |x| < 0.5
            {
                r = -logf(x);
                if (ix >= 0x3eb00000) // |x| >= 2^-8
                {
                    if (ix >= 0x3f2aaaab)
                        y = onef - x;
                    else
                    {
                        i = (int) x;
                        y = x - (float) i;
                    }
                }
                else
                {
                    y = 0.0f;
                }

                z  = y * y;
                p1 = a0f +
                     z * (a2f + z * (a4f + z * (a6f + z * (a8f + z * a10f))));
                p2 = z *
                     (a1f +
                      z * (a3f + z * (a5f + z * (a7f + z * (a9f + z * a11f)))));
                p = y * p1 + p2;
                r += (p - 0.5f * y);
            }
            else // 0.5 <= |x| <= 0.9
            {
                r = 0.0f;
                if (ix >= 0x3f800000)
                {
                    y = x - onef;
                    i = 0;
                }
                else if (ix >= 0x3f3b4a20)
                {
                    y = x - (tcf - onef);
                    i = 1;
                }
                else
                {
                    y = x;
                    i = 2;
                }

                switch (i)
                {
                case 0:
                    z  = y * y;
                    p1 = a0f +
                         z * (a2f +
                              z * (a4f + z * (a6f + z * (a8f + z * a10f))));
                    p2 = z *
                         (a1f +
                          z * (a3f +
                               z * (a5f + z * (a7f + z * (a9f + z * a11f)))));
                    p = y * p1 + p2;
                    r += (p - 0.5f * y);
                    break;
                case 1:
                    z  = y * y;
                    w  = z * y;
                    p1 = t0f + w * (t3f + w * (t6f + w * (t9f + w * t12f)));
                    p2 = t1f + w * (t4f + w * (t7f + w * (t10f + w * t13f)));
                    p3 = t2f + w * (t5f + w * (t8f + w * (t11f + w * t14f)));
                    p  = z * p1 - (ttf - w * (p2 + y * p3));
                    r += (tff + p);
                    break;
                case 2:
                    p1 =
                        y * (u0f +
                             y * (u1f +
                                  y * (u2f + y * (u3f + y * (u4f + y * u5f)))));
                    p2 =
                        onef +
                        y * (v1f + y * (v2f + y * (v3f + y * (v4f + y * v5f))));
                    r += (-0.5f * y + p1 / p2);
                    break;
                }
            }
        }
        else // 0.9 < x < 2.0
        {
            r = 0.0f;
            if (ix >= 0x3fdda618)
            {
                y = 2.0f - x;
                i = 0;
            }
            else if (ix >= 0x3f9da620)
            {
                y = x - tcf;
                i = 1;
            }
            else
            {
                y = x - onef;
                i = 2;
            }

            switch (i)
            {
            case 0:
                z = y * y;
                w = w0f +
                    z * (w1f +
                         z * (w2f +
                              z * (w3f + z * (w4f + z * (w5f + z * w6f)))));
                r += -0.5f * y + w;
                break;
            case 1:
                z  = y * y;
                w  = z * y;
                p1 = t0f + w * (t3f + w * (t6f + w * (t9f + w * t12f)));
                p2 = t1f + w * (t4f + w * (t7f + w * (t10f + w * t13f)));
                p3 = t2f + w * (t5f + w * (t8f + w * (t11f + w * t14f)));
                p  = z * p1 - (ttf - w * (p2 + y * p3));
                r += (tff + p);
                break;
            case 2:
                p1 = y *
                     (s0f +
                      y * (s1f +
                           y * (s2f +
                                y * (s3f + y * (s4f + y * (s5f + y * s6f))))));
                p2 = onef +
                     y * (r1f +
                          y * (r2f +
                               y * (r3f + y * (r4f + y * (r5f + y * r6f)))));
                r += (-0.5f * y + p1 / p2);
                break;
            }
        }
    }
    else                     // x >= 2.0
        if (ix < 0x41000000) // x < 8.0
        {
            i = (int) x;
            t = 0.0f;
            y = x - (float) i;
            p = y *
                (s0f +
                 y * (s1f +
                      y * (s2f + y * (s3f + y * (s4f + y * (s5f + y * s6f))))));
            q = onef +
                y * (r1f +
                     y * (r2f + y * (r3f + y * (r4f + y * (r5f + y * r6f)))));
            r = halff * y + p / q;
            z = onef;
            switch (i)
            {
            case 7: z *= (y + 6.0f); /* FALLTHRU */
            case 6: z *= (y + 5.0f); /* FALLTHRU */
            case 5: z *= (y + 4.0f); /* FALLTHRU */
            case 4: z *= (y + 3.0f); /* FALLTHRU */
            case 3:
                z *= (y + 2.0f); /* FALLTHRU */
                r += logf(z);
                break;
            }
        }
        else if (ix < 0x5c800000) // 8.0 <= x < 2^58
        {
            t = logf(x);
            z = onef / x;
            y = z * z;
            w = w0f +
                z * (w1f +
                     y * (w2f + y * (w3f + y * (w4f + y * (w5f + y * w6f)))));
            r = (x - halff) * (t - onef) + w;
        }
        else // 2^58 <= x <= Inf
        {
            r = x * (logf(x) - onef);
        }

    if (hx < 0)
        r = nadj - r;
    return r;
}



// ============================================================================
//
//   hwfp::lgamma implementation using local functions
//
// ============================================================================

template<>
hwfp<double>::hwfp_p hwfp<double>::lgamma(hwfp<double>::hwfp_r x)
// ----------------------------------------------------------------------------
//   lgamma for double precision - uses local_lgamma
// ----------------------------------------------------------------------------
{
    return make(local_lgamma(x->value()));
}


template<>
hwfp<float>::hwfp_p hwfp<float>::lgamma(hwfp<float>::hwfp_r x)
// ----------------------------------------------------------------------------
//   lgamma for float precision - uses local_lgammaf
// ----------------------------------------------------------------------------
{
    return make(local_lgammaf(x->value()));
}
#endif // LGAMMA_CRASHES
