#ifndef CONTINUED_FRACTION_H
#define CONTINUED_FRACTION_H
// ****************************************************************************
//  continued-fraction.h                                          DB48X project
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
//
//
// ****************************************************************************
//   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
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

#include "command.h"


// DFC: decompose a real number into its continued fraction coefficients.
//   Input:  a real number (integer, fraction, or decimal)
//   Output: a list { a0, a1, a2, ... } of integers such that
//             x ≈ a0 + 1/(a1 + 1/(a2 + ...))
//   The sequence stops when the fractional remainder is smaller than
//   the precision of the input (based on significant decimal digits).
COMMAND_DECLARE(DFC, 1);

// DFC2F: reconstruct a real number from its continued fraction list.
//   Input:  a list { a0, a1, ..., an } of integers
//   Output: a0 + 1/(a1 + 1/(a2 + ... + 1/an))
//   Exact rational result when all coefficients are integers.
COMMAND_DECLARE(DFC2F, 1);

#endif // CONTINUED_FRACTION_H
