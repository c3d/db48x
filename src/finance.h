#ifndef FINANCE_H
#define FINANCE_H
// ****************************************************************************
//  finance.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Finance menu
//
//
//
//
//
//
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

#include "algebraic.h"
#include "expression.h"
#include "menu.h"

COMMAND_DECLARE(TVMAmort, 1);
COMMAND_DECLARE(TVMAmortTable, 1);
COMMAND_DECLARE(TVMRoot, 1);
COMMAND_DECLARE(TVMEquation, 0);


struct FinanceSolverMenu : menu
// ----------------------------------------------------------------------------
//   Solving menu for the finance application
// ----------------------------------------------------------------------------
{
    FinanceSolverMenu(id type = ID_FinanceSolverMenu) : menu(type) {}

    static expression_p equation();
    static bool active();
    static bool round(algebraic_g &value);

public:
    OBJECT_DECL(FinanceSolverMenu);
    MENU_DECL(FinanceSolverMenu);
};

#endif // FINANCE_H
