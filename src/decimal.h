#ifndef DECIMAL_H
#define DECIMAL_H
// ****************************************************************************
//  decimal.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Variable-precision decimal implementation
//
//     This is intended to save some code space when running on DM42,
//     while improving the avaiable precision.
//     The bid128 implementation takes 59.7% of the PGM space and 79.7%
//     of the entire ELF file size. We can probably do better.
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
//   The internal representation for variable decimal uses base 1000.
//   The idea is to preserve "decimal" properties while losing only about 2% in
//   memory compared to binary (1000 values out of 1024 total possible).
//   Values 0-999 represent numbers.
//   Values above 1000 are used for NaN and infinities
//
//   - The ID, which also providws the sign (decimal or neg_decimal)
//   - The signed LEB128 exponent, as a power of 10
//   - The unsigned LEB128 size of the mantissa in groups of 10 bits
//   - The mantissa bits, grouped 10 bits by 10 bits
//
//   Each group of 10 bits, a "kigit", can take normal values 000-999.
//   A number where the 10 bits are above 1000 is "denormal"
//   The mantissa bits represent a value between 0 and 1, 1 excluded
//
//   For example, the value 1.53 is 0.153 * 1000^1, which can be represented as:
//     <decimal><01><01>[153:10][*:6] = 5 bytes
//
//   The bits in the bytes are as follows, wrapping every 5 bytes:
//   |76543210|76543210|76543210|76543210|76543210|76543210|...
//   |98765542|10987654|32109876|54321098|76543210|98765432|...
//
//   The decimal exponent is signed, and can go up to 2^61 safely without any
//   risk of overflow when adding exponents to one another. The actual detection
//   of overflow happens at `MaximumDecimalExponent`. Overflows are reported
//   as an 'infinity' value, which is a value with a very large exponent.
//   Setting the `MaximumDecimalExponent` to 499 makes it possible to have
//   overflow detection at the same point than on the HP48.

#include "algebraic.h"
#include "runtime.h"
#include "settings.h"

GCP(integer);
GCP(bignum);
GCP(fraction);
GCP(big_fraction);
GCP(decimal);

struct decimal : algebraic
// ----------------------------------------------------------------------------
//    Floating-point numbers with variable precision
// ----------------------------------------------------------------------------
{
    // A number between 0 and 1000 fits in 16 bits
    using kint = uint16_t;

    decimal(id type, size_t len, gcbytes bytes): algebraic(type)
    // ------------------------------------------------------------------------
    //   Constructor from raw data
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload(this);
        memcpy(p, bytes.Safe(), len);
    }
    static size_t required_memory(id type, size_t len, gcbytes UNUSED bytes)
    {
        return leb128size(type) + len;
    }


    decimal(id type, large exp, size_t nkig, gcbytes kig): algebraic(type)
    // ------------------------------------------------------------------------
    //   Constructor from exponent and mantissa data
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload(this);
        p = leb128(p, exp);
        p = leb128(p, nkig);
        memcpy(p, kig.Safe(), (nkig * 10 + 7) / 8);
    }
    static size_t required_memory(id type, large exp, size_t n, gcbytes UNUSED)
    {
        return leb128size(type) + leb128size(exp) + leb128size(n)
            + (n * 10 + 7) / 8;
    }

    decimal(id type, large exp, size_t nkigs, gcp<kint> kigs): algebraic(type)
    // ------------------------------------------------------------------------
    //   Constructor from exponent and mantissa digits
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload(this);
        p = leb128(p, exp);
        p = leb128(p, nkigs);
        const kint *kigsp = kigs.Safe();
        if (nkigs)
            p[(nkigs * 10 + 7) / 8 - 1] = 0;
        for (uint i = 0; i < nkigs; i++)
            kigit(p, i, kigsp[i]);
    }
    static size_t required_memory(id type, large exp, size_t n, gcp<kint>)
    {
        return leb128size(type) + leb128size(exp) + leb128size(n)
            + (n * 10 + 7) / 8;
    }

    template <typename Int>
    decimal(id type, Int value, large exp = 0): algebraic(type)
    // ------------------------------------------------------------------------
    //   Constructor from (unsigned) integer value
    // ------------------------------------------------------------------------
    {
        while (value && value % 10 == 0)
        {
            value /= 10;
            exp++;
        }
        Int  copy = value;
        Int  mul  = 1000;
        Int  div  = 1;
        int  iexp = 0;
        while (copy)
        {
            iexp++;
            copy /= 10;
            if (mul > 1)
                mul /= 10;
            else
                div *= 10;
        }
        byte *p = (byte *) payload(this);
        uint nkigits = (iexp + 2) / 3;
        exp += iexp;
        p = leb128(p, exp);
        p = leb128(p, nkigits);
        if (nkigits)
            p[(nkigits * 10 + 7) / 8 - 1] = 0;
        for (uint i = 0; i < nkigits; i++)
        {
            kigit(p, i, (value * mul / div) % 1000);
            if (div >= 1000)
                div /= 1000;
            else
                mul *= 1000;
        }
    }
    template<typename Int>
    static size_t required_memory(id type, Int value, large exp = 0)
    {
        size_t iexp = 0;
        while (value && value % 10 == 0)
        {
            value /= 10;
            exp++;
        }
        while (value)
        {
            iexp += 1;
            value /= 10;
        }
        exp += iexp;
        return required_memory(type, exp, size_t((iexp+2)/3), gcp<kint>());
    }


    template<typename Int>
    static decimal_p make(Int x, large exp = 0)
    // ------------------------------------------------------------------------
    //   Build a decimal from a signed integer
    // ------------------------------------------------------------------------
    {
        if (x < 0)
            return rt.make<decimal>(ID_neg_decimal, -x, exp);
        return rt.make<decimal>(x, exp);
    }


    template<typename Int>
    static decimal_p make(id type, Int x, large exp = 0)
    // ------------------------------------------------------------------------
    //   Build a decimal from a signed integer
    // ------------------------------------------------------------------------
    {
        if (x < 0)
        {
            type = type == ID_decimal ? ID_neg_decimal : ID_decimal;
            return rt.make<decimal>(type, -x, exp);
        }
        return rt.make<decimal>(type, x, exp);
    }


    static decimal_p from_integer(integer_p value);
    static decimal_p from_bignum(bignum_p value);
    static decimal_p from_random_seed(bignum_p value);
    static decimal_p from_fraction(fraction_p value);
    static decimal_p from_big_fraction(big_fraction_p value);
    // ------------------------------------------------------------------------
    //   Build from other data types
    // ------------------------------------------------------------------------


    large exponent() const
    // ------------------------------------------------------------------------
    //   Return the exponent value for the current decimal number
    // ------------------------------------------------------------------------
    {
        byte_p p = payload(this);
        return leb128<large>(p);
    }


    int kigits() const
    // ------------------------------------------------------------------------
    //   Return the number of kigits
    // ------------------------------------------------------------------------
    {
        byte_p p = payload(this);
        (void) leb128<large>(p);  // Exponent
        return leb128<size_t>(p);
    }


    struct info
    // ------------------------------------------------------------------------
    //   Return information about a decimal value
    // ------------------------------------------------------------------------
    {
        info(large exponent, size_t nkigits, byte_p base)
            : exponent(exponent), nkigits(nkigits), base(base) {}
        large   exponent;
        size_t  nkigits;
        byte_p  base;
    };


    info shape() const
    // ------------------------------------------------------------------------
    //   Return information about a given decimal number
    // ------------------------------------------------------------------------
    {
        byte_p p        = payload(this);
        large  exponent = leb128<large>(p);
        size_t nkigits  = leb128<size_t>(p);
        return info(exponent, nkigits, p);
    }


    byte_p base() const
    // ------------------------------------------------------------------------
    //   Return the base of all kigits for the current decimal number
    // ------------------------------------------------------------------------
    {
        return shape().base;
    }


    static kint kigit(byte_p base, size_t index)
    // ------------------------------------------------------------------------
    //    Find the given kigit (base-1000 digit)
    // ------------------------------------------------------------------------
    {
        base += (index * 10) / 8;
        index = (index % 4) * 2 + 2;
        return ((kint(base[0]) << index) | (base[1] >> (8 - index))) & 1023;
    }


    static void kigit(byte *base, size_t index, kint value)
    // ------------------------------------------------------------------------
    //    Write the given kigit (base-1000 digit)
    // ------------------------------------------------------------------------
    {
        base += (index * 10) / 8;
        index = (index % 4) * 2 + 2;
        base[0] = (base[0] & (0xFF << (10 - index)))   | (value >> index);
        index = (8 - index) % 8;
        base[1] = (base[1] & ~(0xFF << index)) | byte(value << index);
    }


    kint kigit(size_t index) const
    // ------------------------------------------------------------------------
    //   Return the given kigit for the current number
    // ------------------------------------------------------------------------
    {
        return kigit(base(), index);
    }


    void kigit(size_t index, kint kig)
    // ------------------------------------------------------------------------
    //   Set the given kigit for the current number
    // ------------------------------------------------------------------------
    {
        kigit((byte *) base(), index, kig);
    }


    unsigned significant_digits() const
    // ------------------------------------------------------------------------
    //   Return the number of significant digits in the number
    // ------------------------------------------------------------------------
    {
        info     i     = shape();
        unsigned nk    = i.nkigits;
        if (!nk)
            return 0;
        kint     k     = kigit(i.base, nk - 1);
        unsigned nlast = k % 10 ? 3 : k % 100 ? 2 : k ? 1 : 0;
        return 3 * (nk - 1) + nlast;
    }


    struct iterator
    // ------------------------------------------------------------------------
    // Iterator, built in a way that is robust to garbage collection in loops
    // ------------------------------------------------------------------------
    {
        typedef kint   value_type;
        typedef size_t difference_type;

    public:
        explicit iterator(decimal_p num, size_t skip = 0)
            : number(num),
              size(num->kigits()),
              index(skip < size ? skip : size)
        {}
        iterator() : number(), size(0), index(0) {}

    public:
        iterator& operator++()
        {
            if (index < size)
                index++;
            return *this;
        }
        iterator operator++(int)
        {
            iterator prev = *this;
            ++(*this);
            return prev;
        }
        bool operator==(const iterator &other) const
        {
            return !number.Safe() || !other.number.Safe() ||
                (index == other.index && size == other.size &&
                 number.Safe() == other.number.Safe());
        }
        bool operator!=(const iterator &other) const
        {
            return !(*this == other);
        }
        value_type operator*() const
        {
            return number->kigit(index);
        }
        void write(kint value) const
        {
            ((decimal *) number.Safe())->kigit(index, value);
        }
        bool valid() const
        {
            return index < size;
        }

    public:
      decimal_g number;
      size_t    size;
      size_t    index;
    };
    iterator begin() const      { return iterator(this); }
    iterator end() const        { return iterator(this, ~0U); }


    ularge   as_unsigned(bool magnitude = false) const;
    large    as_integer() const;
    int32_t  as_int32() const;
    // ------------------------------------------------------------------------
    //   Conversion to machine values
    // ------------------------------------------------------------------------


    enum class_type
    // ------------------------------------------------------------------------
    //   Class type for decimal numbers
    // ------------------------------------------------------------------------
    //   This should really be exported in the header, since it's the result of
    //   the bid128_class function. Lifted from Inte's source code
    {
        negativeNormal,
        negativeSubnormal,
        negativeZero,
        positiveZero,
        positiveSubnormal,
        positiveNormal,

        NaN                     = 1000,
        signalingNaN,
        quietNaN,
        negativeInfinity,
        positiveInfinity,
        infinity
    };

    class_type       fpclass() const;
    bool             is_normal() const;
    // ------------------------------------------------------------------------
    //   Return the floating-point class for the decimal number
    // ------------------------------------------------------------------------

    static int       compare(decimal_r x, decimal_r y, uint epsilon = 0);
    // ------------------------------------------------------------------------
    //   Return a comparision between the two values
    // ------------------------------------------------------------------------


    bool             is_zero() const;
    bool             is_one() const;
    bool             is_negative() const;
    bool             is_negative_or_zero() const;
    bool             is_magnitude_less_than(uint kigit, large exp) const;
    bool             is_magnitude_less_than_half() const;
    bool             is_infinity() const;
    bool             is_simplifiable() const { return is_normal(); }
    // ------------------------------------------------------------------------
    //   Tests about the value of a given decimal number
    // ------------------------------------------------------------------------


    decimal_p        truncate(large exp = 0) const;
    bool             split(decimal_g &ip, decimal_g &fp, large exp = 0) const;
    bool             split(large &ip, decimal_g &fp, large exp = 0) const;
    // ------------------------------------------------------------------------
    //   Truncate / round / split a decimal value to the given place
    // ------------------------------------------------------------------------


    decimal_p        round(large exp = 0) const;
    decimal_p        precision(size_t prec) const
    // ------------------------------------------------------------------------
    //   Round a number to the given precision
    // ------------------------------------------------------------------------
    {
        return round(exponent() - large(prec));
    }

    struct precision_adjust
    // ------------------------------------------------------------------------
    //   Helper to adjust precision during a computation
    // ------------------------------------------------------------------------
    {
        precision_adjust(uint extra = 3, bool force = false)
            : saved(Settings.Precision())
        {
            if (adjusted++ == 0 || force)
                Settings.Precision((saved + extra + 2) / 3 * 3);
        }
        ~precision_adjust()
        {
            --adjusted;
            Settings.Precision(saved);
        }
        operator uint()         { return saved; }
        decimal_p operator()(decimal_p dec)
        {
            return adjusted > 1 ? dec : dec ? dec->precision(saved) : nullptr;
        }

        uint saved;
        static uint adjusted;
    };

    algebraic_p      to_integer() const;
    bignum_p         to_bignum() const;
    algebraic_p      to_fraction(uint count = Settings.FractionIterations(),
                                 uint prec  = Settings.FractionDigits()) const;
    // ------------------------------------------------------------------------
    //   Convert decimal number to fraction
    // ------------------------------------------------------------------------

    float            to_float() const;
    double           to_double() const;
    static decimal_p from(float x)      { return from(double(x)); }
    static decimal_p from(double x);
    // ------------------------------------------------------------------------
    //   Conversion to/from hardware-accelerated floating-point types
    // ------------------------------------------------------------------------


    // ========================================================================
    //
    //    Arithmetic
    //
    // ========================================================================

    static decimal_p neg(decimal_r x);
    static decimal_p add(decimal_r x, decimal_r y);
    static decimal_p subtract(decimal_r x, decimal_r y);
    static decimal_p multiply(decimal_r x, decimal_r y);
    static decimal_p divide(decimal_r x, decimal_r y);
    static decimal_p mod(decimal_r x, decimal_r y);
    static decimal_p rem(decimal_r x, decimal_r y);
    static decimal_p pow(decimal_r x, decimal_r y);

    static decimal_p hypot(decimal_r x, decimal_r y);
    static decimal_p atan2(decimal_r x, decimal_r y);
    static decimal_p Min(decimal_r x, decimal_r y);
    static decimal_p Max(decimal_r x, decimal_r y);



    // ========================================================================
    //
    //    Math functions
    //
    // ========================================================================

    static decimal_p sqrt(decimal_r x);
    static decimal_p cbrt(decimal_r x);

    static decimal_p sin(decimal_r x);
    static decimal_p cos(decimal_r x);
    static decimal_p tan(decimal_r x);
    static decimal_p asin(decimal_r x);
    static decimal_p acos(decimal_r x);
    static decimal_p atan(decimal_r x);

    static decimal_p sin_fracpi(uint qturns, decimal_r fp);
    static decimal_p cos_fracpi(uint qturns, decimal_r fp);

    static decimal_p sinh(decimal_r x);
    static decimal_p cosh(decimal_r x);
    static decimal_p tanh(decimal_r x);
    static decimal_p asinh(decimal_r x);
    static decimal_p acosh(decimal_r x);
    static decimal_p atanh(decimal_r x);

    static decimal_p ln1p(decimal_r x);
    static decimal_p expm1(decimal_r x);
    static decimal_p ln(decimal_r x);
    static decimal_p log10(decimal_r x);
    static decimal_p log2(decimal_r x);
    static decimal_p exp(decimal_r x);
    static decimal_p exp10(decimal_r x);
    static decimal_p exp2(decimal_r x);
    static decimal_p erf(decimal_r x);
    static decimal_p erfc(decimal_r x);
    static decimal_p tgamma(decimal_r x);
    static decimal_p lgamma(decimal_r x);
    static decimal_p lgamma_internal(decimal_r x);

    static decimal_p abs(decimal_r x);
    static decimal_p sign(decimal_r x);
    static decimal_p IntPart(decimal_r x);
    static decimal_p FracPart(decimal_r x);
    static decimal_p ceil(decimal_r x);
    static decimal_p floor(decimal_r x);
    static decimal_p inv(decimal_r x);
    static decimal_p sq(decimal_r x);
    static decimal_p cubed(decimal_r x);
    static decimal_p xroot(decimal_r y, decimal_r x);
    static decimal_p fact(decimal_r x);



    // ========================================================================
    //
    //    Support for math functions
    //
    // ========================================================================

    struct ccache
    // ------------------------------------------------------------------------
    //  Constants are re-created whenever precision changes
    // ------------------------------------------------------------------------
    {
        ccache(): precision(), gamma_na(0), gamma_ck(nullptr) {}

        size_t  precision;
        decimal_g pi;
        decimal_g e;
        decimal_g log10;
        decimal_g log2;
        decimal_g sq2pi;
        decimal_g oosqpi;
        decimal_g lpi;

        size_t    gamma_na;
        decimal_g *gamma_ck;

        decimal_r ln10();
        decimal_r ln2();
        decimal_r lnpi();
        decimal_r sqrt_2pi();
        decimal_r one_over_sqrt_pi();
        decimal_g two_over_sqrt_pi();

        decimal_g *gamma_realloc(size_t na);
    };

    static ccache   &constants();


    static decimal_p pi();
    static decimal_p e();
    static decimal_p ln10();
    static decimal_p ln2();
    static decimal_p lnpi();
    bool             adjust_from_angle(uint &qturns, decimal_g &fp) const;
    decimal_p        adjust_to_angle() const;
    static decimal_p exact_angle(int digits, int exp);

public:
    OBJECT_DECL(decimal);
    PARSE_DECL(decimal);
    SIZE_DECL(decimal);
    HELP_DECL(decimal);
    RENDER_DECL(decimal);
};


struct neg_decimal : decimal
// ----------------------------------------------------------------------------
//   A negative decimal number is like a decimal number
// ----------------------------------------------------------------------------
{
    OBJECT_DECL(neg_decimal);
};



// ============================================================================
//
//   Arithmetic
//
// ============================================================================

decimal_g operator-(decimal_g x);
decimal_g operator+(decimal_g x, decimal_g y);
decimal_g operator-(decimal_g x, decimal_g y);
decimal_g operator*(decimal_g x, decimal_g y);
decimal_g operator/(decimal_g x, decimal_g y);

inline decimal_g operator-(decimal_g x)
// ----------------------------------------------------------------------------
//  Negate number
// ----------------------------------------------------------------------------
{
    return decimal::neg(x);
}


inline decimal_g operator+(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Addition
// ----------------------------------------------------------------------------
{
    return decimal::add(x, y);
}


inline decimal_g operator-(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//  Subtraction
// ----------------------------------------------------------------------------
{
    return decimal::subtract(x, y);
}


inline decimal_g operator*(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Multiplication
// ----------------------------------------------------------------------------
{
    return decimal::multiply(x, y);
}


inline decimal_g operator/(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Division
// ----------------------------------------------------------------------------
{
    return decimal::divide(x, y);
}


inline decimal_g operator%(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Remainder
// ----------------------------------------------------------------------------
{
    return decimal::rem(x, y);
}


inline bool operator==(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Equality between two decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) == 0;
}


inline bool operator!=(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Inequality between two decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) != 0;
}


inline bool operator<(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Less-than comparison between decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) < 0;
}


inline bool operator<=(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Less-than-or-equal between decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) <= 0;
}


inline bool operator>(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Greater-than comparison between decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) > 0;
}


inline bool operator>=(decimal_g x, decimal_g y)
// ----------------------------------------------------------------------------
//   Greater-than-or-equal comparison between decimal values
// ----------------------------------------------------------------------------
{
    return decimal::compare(x, y) >= 0;
}


inline bool decimal::is_magnitude_less_than_half() const
// ----------------------------------------------------------------------------
//  Check if magnitude is less than 0.5
// ----------------------------------------------------------------------------
{
    return is_magnitude_less_than(500, 0);
}

#endif // DECIMAL_H
