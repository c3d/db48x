#ifndef CONTINUED_FRACTION_H
#define CONTINUED_FRACTION_H
// ****************************************************************************
//  ContinuedFraction.h                                          DB48X project
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
//     The sequence terminates when the residual fractional part falls
//     below the precision of the input decimal value.
//
// ****************************************************************************
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#include "command.h"


// DFC: decompose a real number into its continued fraction coefficients.
//   Input:  a real number (integer, fraction, or decimal)
//   Output: a list { a0, a1, a2, ... } of integers such that
//             x ≈ a0 + 1/(a1 + 1/(a2 + ...))
//   The sequence stops when the fractional remainder is smaller than
//   the precision of the input (based on significant decimal digits).
COMMAND_DECLARE(DFC, 1);

#endif // CONTINUED_FRACTION_H
