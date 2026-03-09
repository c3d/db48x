#ifndef EXACT_QUOTIENT_H
#define EXACT_QUOTIENT_H
// ****************************************************************************
//  exact-quotient.h                                              DB48X project
// ****************************************************************************
//
//   File Description:
//
//     XQ: find the simplest symbolic representation of a real number.
//
//     Given a real number x, XQ returns the expression matching x within
//     current precision (Settings.Precision()) that has the smallest
//     denominator among three templates:
//       - rational:    p/q
//       - square root: √(p/q)
//       - pi multiple: p/q × π
//
//     Examples:
//       XQ(0.3658)        → 1829/5000   (rational, higher precision)
//       XQ(0.3658)        → √(19/142)   (sqrt,     lower  precision)
//       XQ(1.04719755120) → 1/3×π
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


// XQ: exact quotient — simplest symbolic representation of a real number.
//   Input:  a real number x (integer, fraction, or decimal)
//   Output: the expression with the smallest denominator q matching x within
//           current precision, chosen among:
//             rational p/q, square root √(p/q), or pi multiple p/q×π.
//   If no template matches within precision, the original value is returned.
COMMAND_DECLARE(XQ, 1);

// ToFractionPi (→Qπ): like XQ but restricted to templates involving π.
COMMAND_DECLARE(ToFractionPi, 1);

#endif // EXACT_QUOTIENT_H
