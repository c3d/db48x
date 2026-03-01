#ifndef PRECEDENCE_H
#define PRECEDENCE_H
// ****************************************************************************
//  precedence.h                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Define operator precedence
//
//
//
//
//
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

enum precedence
// ----------------------------------------------------------------------------
//   Precedence for the various operators
// ----------------------------------------------------------------------------
//   Odd precedences indicate right-associativity
{
    NONE                = 0,    // No precedence
    LOWEST              = 1,    // Lowest precedence (when parsing parentheses)
    WHERE               = 4,    // | (where) operator
    COMPLEX             = 6,    // Complex numbers

    LOGICAL             = 10,    // and, or, xor
    RELATIONAL          = 12,    // <, >, =, etc
    ADDITIVE            = 14,    // +, -
    MULTIPLICATIVE      = 16,    // *, /
    POWER               = 29,    // ^ (right associative)

    FUNCTIONAL          = 30,   // Unknown operator
    FUNCTION            = 40,   // Functions, e.g. f(x)
    FUNCTION_POWER      = 50,   // X²
    SYMBOL              = 60,   // Names
    PARENTHESES         = 70,   // Parentheses
};

#endif // PRECEDENCE_H
