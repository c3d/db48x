#ifndef HWFP_H
#define HWFP_H
// ****************************************************************************
//  hwfp.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Data types using hardware floating point to accelerate computations
//
//    Computing cbrt(exp(sin(atan(x)))), we get the following duration in ms
//
//                   VP Decimal      float           double
//         DM32      25.0933         0.0120          0.2708
//         DM42      18.4995         0.0140          0.1876
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

#include "algebraic.h"
#include "fraction.h"
#include "integer.h"
#include "runtime.h"
#include "settings.h"

#include <cmath>
#include <limits>

GCP(integer);
GCP(bignum);
GCP(fraction);
GCP(big_fraction);


struct hwfp_base : algebraic
// ----------------------------------------------------------------------------
//   Shared base code between hardware floating-point implementations
// ----------------------------------------------------------------------------
{
    hwfp_base(id type) : algebraic(type) {}
    static size_t render(renderer &r, double d, char suffix);
};


template<typename hw>
struct hwfp : hwfp_base
// ----------------------------------------------------------------------------
//    Floating-point numbers represented with a hardware-accelerated FP
// ----------------------------------------------------------------------------
{
    using hwfp_p = const hwfp *;
    using hwfp_r = const gcp<hwfp> &;

    hwfp(id type, hw value): hwfp_base(type)
    // ------------------------------------------------------------------------
    //   Constructor from actual value
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload(this);
        memcpy(p, &value, sizeof(value));
    }
    static size_t required_memory(id type, hw value)
    {
        return leb128size(type) + sizeof(value);
    }


    template<typename val>
    static hwfp_p make(id ty, val x)
    // ------------------------------------------------------------------------
    //   Build a hwfp from an integer value
    // ------------------------------------------------------------------------
    {
        hw fp = x;
        if (!std::isfinite(fp))
        {
            if (std::isinf(fp))
            {
                if (Settings.OverflowError())
                {
                    rt.overflow_error();
                    return nullptr;
                }
                Settings.OverflowIndicator(true);
            }
            else
            {
                rt.domain_error();
                return nullptr;
            }
        }
        return rt.make<hwfp>(ty, fp);
    }

    static hwfp_p make(float x);
    static hwfp_p make(double x);
    // ------------------------------------------------------------------------
    //   Builld a hardware floating-point value from float or double
    // ------------------------------------------------------------------------



    hw value() const
    // ------------------------------------------------------------------------
    //   Return the payload as a floating-point value
    // ------------------------------------------------------------------------
    {
        hw fp = 0.0;
        byte_p p  = payload(this);
        memcpy(&fp, p, sizeof(fp));
        return fp;
    }


    static hwfp_p from_integer(integer_p value);
    static hwfp_p from_bignum(bignum_p value);
    static hwfp_p from_fraction(fraction_p value);
    static hwfp_p from_big_fraction(big_fraction_p value);
    // ------------------------------------------------------------------------
    //   Build from other data types
    // ------------------------------------------------------------------------

    ularge        as_unsigned(bool magnitude = false) const
    // ------------------------------------------------------------------------
    //   Convert to an unsigned value
    // ------------------------------------------------------------------------
    {
        hw fp = value();
        if (fp < 0.0 && magnitude)
            fp = -fp;
        return (ularge) fp;
    }

    large as_integer() const
    // ------------------------------------------------------------------------
    //   Convert to a signed value
    // ------------------------------------------------------------------------
    {
        hw fp = value();
        return (large) fp;
    }

    int32_t     as_int32()              const { return int32_t(as_integer()); }
    hw          as_hwfp()               const { return value(); }
    float       as_float()              const { return value(); }
    double      as_double()             const { return value(); }
    // ------------------------------------------------------------------------
    //   Conversion to machine values
    // ------------------------------------------------------------------------

    bool        is_zero()               const { return value() == 0.0; }
    bool        is_one()                const { return value() == 1.0; }
    bool        is_negative()           const { return value() < 0.0; }
    bool        is_negative_or_zero()   const { return value() <= 0.0; }
    bool        is_simplifiable()   const { return std::isfinite(value());}
    // ------------------------------------------------------------------------
    //   Tests about the value of a given hwfp number
    // ------------------------------------------------------------------------


    algebraic_p to_integer() const
    // ------------------------------------------------------------------------
    //   Convert floating point to integer value
    // ------------------------------------------------------------------------
    {
        hw fp = value();
        if (fp > hw(std::numeric_limits<large>::max()) ||
            fp < hw(std::numeric_limits<large>::min()))
        {
            rt.value_error();
            return nullptr;
        }
        return integer::make(large(value()));
    }


    algebraic_p   to_fraction(uint count = Settings.FractionIterations(),
                              uint prec  = Settings.FractionDigits()) const;
    // ------------------------------------------------------------------------
    //   Convert floating point to fraction
    // ------------------------------------------------------------------------


    template <hwfp_p (*code)(hwfp_r, hwfp_r)>
    static algebraic_p hwfp_wrapper(algebraic_r x, algebraic_r y)
    // ------------------------------------------------------------------------
    //   Wrapper to silence warning about function casts in recent XCode
    // ------------------------------------------------------------------------
    {
        hwfp_r xb = (hwfp_r) x;
        hwfp_r yb = (hwfp_r) y;
        hwfp_p rb = code(xb, yb);
        return rb;
    }


    template <hwfp_p (*code)(hwfp_r, hwfp_r)>
    static arithmetic_fn target(algebraic_r x, algebraic_r y)
    // ------------------------------------------------------------------------
    //  Target function for hwfp objects
    // ------------------------------------------------------------------------
    {
        bool       flt = sizeof(hw) == sizeof(float);
        object::id ty  = flt ? ID_hwfloat : ID_hwdouble;
        uint       prc = flt ? 7 : 16;
        return x->type() == ty && y->type() == ty &&
            Settings.HardwareFloatingPoint() && Settings.Precision() <= prc
            ? hwfp_wrapper<code> : nullptr;
    }



    // ========================================================================
    //
    //    Arithmetic
    //
    // ========================================================================

    static hwfp_p neg(hwfp_r x);
    static hwfp_p add(hwfp_r x, hwfp_r y);
    static hwfp_p subtract(hwfp_r x, hwfp_r y);
    static hwfp_p multiply(hwfp_r x, hwfp_r y);
    static hwfp_p divide(hwfp_r x, hwfp_r y);
    static hwfp_p mod(hwfp_r x, hwfp_r y);
    static hwfp_p rem(hwfp_r x, hwfp_r y);
    static hwfp_p pow(hwfp_r x, hwfp_r y);
    static hwfp_p hypot(hwfp_r x, hwfp_r y);
    static hwfp_p atan2(hwfp_r x, hwfp_r y);
    static hwfp_p Min(hwfp_r x, hwfp_r y);
    static hwfp_p Max(hwfp_r x, hwfp_r y);


    // ========================================================================
    //
    //    Math functions
    //
    // ========================================================================

    static hw from_angle(hw x)
    {
        switch (Settings.AngleMode())
        {
        case ID_Deg: return x * hw(M_PI / 180.);
        default:
        case ID_Rad: return x;
        case ID_Grad: return x * hw(M_PI / 200.);
        case ID_PiRadians: return x * hw(M_PI);
        }
    }
    static hw to_angle(hw x)
    {
        switch (Settings.AngleMode())
        {
        case ID_Deg: return x * hw(180. / M_PI);
        default:
        case ID_Rad: return x;
        case ID_Grad: return x * hw(200. / M_PI);
        case ID_PiRadians: return x * hw(1.0 / M_PI);
        }
    }

    static hwfp_p sqrt(hwfp_r x)
    {
        return make(std::sqrt(x->value()));
    }

    static hwfp_p cbrt(hwfp_r x)
    {
        return make(std::cbrt(x->value()));
    }


    static hwfp_p sin(hwfp_r x)
    {
        return make(std::sin(from_angle(x->value())));
    }

    static hwfp_p cos(hwfp_r x)
    {
        return make(std::cos(from_angle(x->value())));
    }

    static hwfp_p tan(hwfp_r x)
    {
        return make(std::tan(from_angle(x->value())));
    }

    static hwfp_p asin(hwfp_r x)
    {
        return make(to_angle(std::asin(x->value())));
    }

    static hwfp_p acos(hwfp_r x)
    {
        return make(to_angle(std::acos(x->value())));
    }

    static hwfp_p atan(hwfp_r x)
    {
        return make(to_angle(std::atan(x->value())));
    }

    static hwfp_p sinh(hwfp_r x)
    {
        return make(std::sinh(x->value()));
    }

    static hwfp_p cosh(hwfp_r x)
    {
        return make(std::cosh(x->value()));
    }

    static hwfp_p tanh(hwfp_r x)
    {
        return make(std::tanh(x->value()));
    }

    static hwfp_p asinh(hwfp_r x)
    {
        return make(std::asinh(x->value()));
    }

    static hwfp_p acosh(hwfp_r x)
    {
        return make(std::acosh(x->value()));
    }

    static hwfp_p atanh(hwfp_r x)
    {
        return make(to_angle(std::atanh(x->value())));
    }


    static hwfp_p ln1p(hwfp_r x)
    {
        return make(std::log1p(x->value()));
    }

    static hwfp_p expm1(hwfp_r x)
    {
        return make(std::expm1(x->value()));
    }

    static hwfp_p ln(hwfp_r x)
    {
        return make(std::log(x->value()));
    }

    static hwfp_p log10(hwfp_r x)
    {
        return make(std::log10(x->value()));
    }

    static hwfp_p log2(hwfp_r x)
    {
        return make(std::log2(x->value()));
    }

    static hwfp_p exp(hwfp_r x)
    {
        return make(std::exp(x->value()));
    }

    static hwfp_p exp10(hwfp_r x)
    {
        return make(std::exp(x->value() * hw(M_LN10)));
    }

    static hwfp_p exp2(hwfp_r x)
    {
        return make(std::exp2(x->value()));
    }

    static hwfp_p erf(hwfp_r x)
    {
        return make(std::erf(x->value()));
    }

    static hwfp_p erfc(hwfp_r x)
    {
        return make(std::erfc(x->value()));
    }

    static hwfp_p tgamma(hwfp_r x)
    {
        return make(std::tgamma(x->value()));
    }

    static hwfp_p lgamma(hwfp_r x)
    {
        return make(std::lgamma(x->value()));
    }

    static hwfp_p abs(hwfp_r x)
    {
        return make(std::abs(x->value()));
    }

    static hwfp_p sign(hwfp_r x)
    {
        return make(x < 0.0 ? -1.0 : x > 0.0 ? 1.0 : 0.0);
    }

    static hwfp_p IntPart(hwfp_r x)
    {
        hw fx = x->value();
        return make(x < 0 ? ceil(x) : floor(x));
    }

    static hwfp_p FracPart(hwfp_r x)
    {
        hw fx = x->value();
        return make(std::fmod(fx, 1.0));
    }

    static hwfp_p ceil(hwfp_r x)
    {
        return make(std::ceil(x->value()));
    }

    static hwfp_p floor(hwfp_r x)
    {
        return make(std::floor(x->value()));
    }

    static hwfp_p inv(hwfp_r x)
    {
        hw fx = x->value();
        if (fx == 0.0)
        {
            rt.zero_divide_error();
            return nullptr;
        }
        return make(1.0 / fx);
    }

    static hwfp_p sq(hwfp_r x)
    {
        hw fx = x->value();
        return make(fx * fx);
    }

    static hwfp_p cubed(hwfp_r x)
    {
        hw fx = x->value();
        return make(fx * fx * fx);
    }

    static hwfp_p xroot(hwfp_r y, hwfp_r x)
    {
        hw fx = x->value();
        hw fy = y->value();
        return make(pow(fy, 1.0 / x));
    }

    static hwfp_p fact(hwfp_r x)
    {
        return make(std::tgamma(x->value() + 1.0));
    }


  public:
    SIZE_DECL(hwfp)
    {
        byte_p p = o->payload();
        p += sizeof(hw);
        return ptrdiff(p, o);
    }
};


#define GCP_HWFLOAT(T)                  \
    typedef const hwfp<T>   *hw##T##_p; \
    typedef gcp<hwfp<T>>     hw##T##_g; \
    typedef gcm<hwfp<T>>     hw##T##_m; \
    typedef const hw##T##_g &hw##T##_r;

GCP_HWFLOAT(float);
GCP_HWFLOAT(double);


struct hwfloat : hwfp<float>
// ----------------------------------------------------------------------------
//   A hardware-accelerated floating-point value represented by a `float`
// ----------------------------------------------------------------------------
{
    static hwfloat_p make(float x)
    {
        return hwfloat_p(hwfp<float>::make(ID_hwfloat, x));
    }
    OBJECT_DECL(hwfloat);
    HELP_DECL(hwfloat)
    {
        return utf8("hwfloat");
    }
    RENDER_DECL(hwfloat)
    {
        return render(r, o->value(), 'F');
    }
};


struct hwdouble : hwfp<double>
// ----------------------------------------------------------------------------
//   A hardware-accelerated floating-point value represented by a `double`
// ----------------------------------------------------------------------------
{
    static hwdouble_p make(double x)
    {
        return hwdouble_p(hwfp<double>::make(ID_hwdouble, x));
    }
    OBJECT_DECL(hwdouble);
    HELP_DECL(hwdouble)
    {
        return utf8("hwdouble");
    }
    RENDER_DECL(hwdouble)
    {
        return render(r, o->value(), 'D');
    }


};


template <typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::make(float x)
// ----------------------------------------------------------------------------
//  Make an object from a float
// ----------------------------------------------------------------------------
{
    return hwfloat::make(x);
}


template <typename hw>
typename hwfp<hw>::hwfp_p hwfp<hw>::make(double x)
// ----------------------------------------------------------------------------
//  Make an object from a double
// ----------------------------------------------------------------------------
{
    return hwdouble::make(x);
}


#endif // HWFP_H
