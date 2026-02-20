// ****************************************************************************
//  ContinuedFraction.cc                                         DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Continued fraction decomposition of a real number
//
//     DFC decomposes a real number x into a continued fraction:
//       x = a0 + 1/(a1 + 1/(a2 + 1/...))
//     and returns the list of coefficients [a0, a1, a2, ...].
//
//     Algorithm for integers and fractions (exact):
//       Standard Euclidean algorithm on the numerator and denominator.
//
//     Algorithm for decimals and other reals:
//       1. Split x into integer part ip (truncated toward zero) and fp = x - ip
//       2. For negative non-integer x, adjust so fp stays in [0, 1):
//            ip = floor(x) = truncate(x) - 1
//            fp = 1 + fractional_part   (which is 1 - |fractional_part|)
//       3. Append ip as a0.
//       4. Iterate: next = 1/fp, ip = floor(next), fp = next - ip.
//          Append ip as next coefficient.
//       5. Stop when fp is zero or when |fp| < 10^(-sdigs)
//          where sdigs = significant_digits(input).
//
// ****************************************************************************
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#include "continued-fraction.h"

#include "algebraic.h"
#include "decimal.h"
#include "fraction.h"
#include "integer.h"
#include "list.h"

COMMAND_BODY(DFC)
// ----------------------------------------------------------------------------
//   Decompose a real number into its continued fraction coefficients
// ----------------------------------------------------------------------------
{
    // Fetch the input from the stack
    algebraic_g xo = algebraic_p(strip(rt.stack(0)));
    if (!xo)
        return ERROR;

    object::id ty = xo->type();

    // -----------------------------------------------------------------------
    // Fast path 1: integer input → { n }
    // -----------------------------------------------------------------------
    if (object::is_integer(ty))
    {
        scribble scr;
        if (!rt.append(xo))
            return ERROR;
        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // -----------------------------------------------------------------------
    // Fast path 2: small fraction → exact Euclidean algorithm
    //   Only for ID_fraction / ID_neg_fraction (numerator and denominator
    //   both fit in ularge).  Big fractions fall through to the decimal path.
    // -----------------------------------------------------------------------
    if (ty == object::ID_fraction || ty == object::ID_neg_fraction)
    {
        fraction_p frac  = fraction_p(+xo);
        ularge     p     = frac->numerator_value();   // absolute value
        ularge     q     = frac->denominator_value(); // always positive
        bool       neg   = (ty == object::ID_neg_fraction);

        scribble scr;

        // First coefficient: floor(±p/q) with correct sign handling
        ularge    quot0 = p / q;
        ularge    rem0  = p % q;
        ularge    next_p, next_q;
        integer_g a0i;

        if (neg && rem0 != 0)
        {
            // floor(-p/q) = -(p/q + 1); fractional part = (q - rem0)/q
            a0i    = rt.make<neg_integer>(quot0 + 1);
            next_p = q - rem0;
            next_q = q;
        }
        else if (neg)
        {
            // Exact negative integer: floor(-p/q) = -(p/q), fp = 0
            a0i    = rt.make<neg_integer>(quot0);
            next_p = 0;
            next_q = 1;
        }
        else
        {
            // Non-negative: floor(p/q) = p/q
            a0i    = rt.make<integer>(quot0);
            next_p = rem0;
            next_q = q;
        }
        if (!a0i || !rt.append(a0i))
            return ERROR;

        // Remaining coefficients via the Euclidean algorithm.
        // The fractional part is next_p/next_q; its reciprocal is next_q/next_p.
        p = next_q;
        q = next_p;
        while (q != 0)
        {
            ularge    a_val = p / q;
            ularge    r     = p % q;
            integer_g ai    = rt.make<integer>(a_val);
            if (!ai || !rt.append(ai))
                return ERROR;
            p = q;
            q = r;
        }

        list_g lst = list::make(scr.scratch(), scr.growth());
        if (!lst || !rt.top(lst))
            return ERROR;
        return OK;
    }

    // -----------------------------------------------------------------------
    // General path: convert to decimal and use the iterative algorithm
    // -----------------------------------------------------------------------
    if (!algebraic::to_decimal(xo))
    {
        rt.type_error();
        return ERROR;
    }

    decimal_g num = decimal_p(+xo);
    if (!num)
        return ERROR;

    // Number of significant decimal digits in the input — used as the
    // precision limit to decide when to stop expanding the fraction.
    // We subtract a small margin (2 digits) to guard against rounding
    // errors in the last digits of the decimal representation, which
    // would otherwise produce a spurious large coefficient when the
    // remainder should ideally be zero.
    uint sdigs = num->significant_digits();
    if (sdigs == 0)
        sdigs = 1;      // Treat zero as having at least 1 significant digit
    if (sdigs > 2)
        sdigs -= 2;

    // Helper constant: 1
    decimal_g one = decimal::make(1);
    if (!one)
        return ERROR;

    // Split number into integer part (truncation toward zero) and
    // fractional part.  For negative non-integer inputs, split gives
    // fp < 0, so we adjust to keep fp in [0, 1).
    decimal_g ip, fp;
    if (!num->split(ip, fp))
        return ERROR;

    // Compute floor for the first coefficient a0:
    //   positive (or exact integer): floor = truncate = ip
    //   negative with fractional part: floor = truncate - 1, fp = 1 - |fp|
    algebraic_g a0;
    if (fp->is_negative() && !fp->is_zero())
    {
        decimal_g floor_ip = ip - one;   // truncate(x) - 1 = floor(x)
        a0 = floor_ip ? floor_ip->to_integer() : nullptr;
        fp = one + fp;                   // 1 + (-|fp|) = 1 - |fp| ∈ (0,1)
    }
    else
    {
        a0 = ip->to_integer();
    }

    // Build the output list on the scratchpad
    scribble scr;
    if (!a0 || !rt.append(a0))
        return ERROR;

    // Maximum iterations: a bit more than sdigs to handle rounding noise
    uint maxcount = sdigs + 4;

    for (uint iter = 0; iter < maxcount; iter++)
    {
        // Stop when the fractional part is exactly zero
        if (fp->is_zero())
            break;

        // Stop when |fp| is smaller than the input precision:
        // fp->exponent() gives the power-of-10 scale of fp;
        // when -exponent reaches sdigs the contribution is negligible.
        large exp = fp->exponent();
        if (-exp >= large(sdigs))
            break;

        // Next step: compute 1/fp and split into integer and new fraction
        decimal_g next = one / fp;
        if (!next)
            return ERROR;

        if (!next->split(ip, fp))
            return ERROR;

        algebraic_g ai = ip->to_integer();
        if (!ai || !rt.append(ai))
            return ERROR;
    }

    // Assemble the scratchpad into a list and replace the stack top
    list_g lst = list::make(scr.scratch(), scr.growth());
    if (!lst || !rt.top(lst))
        return ERROR;

    return OK;
}
