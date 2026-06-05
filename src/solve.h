#ifndef SOLVE_H
#define SOLVE_H
// ****************************************************************************
//  solve.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Numerical solver and solver menu
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

#include "algebraic.h"
#include "functions.h"
#include "menu.h"
#include "symbol.h"

GCP(list);
GCP(expression);

NFUNCTION_EXT(Root,3,
          static bool can_be_symbolic(uint a)
          {
              return a == 1 || a == 2;
          }
          static algebraic_p solve(program_r   eq,
                                   algebraic_r name,
                                   algebraic_r guess);
          static algebraic_p solve(algebraic_g &eq,
                                   algebraic_g &vars,
                                   algebraic_g &guess);
          static list_p multiple_equation_solver(list_r eqs,
                                                 list_r names,
                                                 list_r guesses);
          static bool jacobi_solver(list_g &eqs,
                                    list_g &vars,
                                    list_g &guesses);
          static expression_p isolate(expression_p eq, symbol_p name);
    );
COMMAND_DECLARE(MultipleEquationsSolver, 1);

COMMAND_DECLARE_FN(MultipleVariablesSolver,3);

COMMAND_DECLARE(StEq, 1);
SYMBOL_DECLARE(RcEq);
COMMAND_DECLARE(NextEq, 0);
SYMBOL_DECLARE(EvalEq);


struct SolvingMenu : menu
// ----------------------------------------------------------------------------
//   The solving menu is built dynamically from current expression
// ----------------------------------------------------------------------------
//   The SolvingMenu shows expression variables in the menu
//   For each variable, the function key has the three following features:
//   - Unshifted sets the variable value
//   - Shifted solves for the variable
//   - XShifted recalls the variable value
{
    SolvingMenu(id type = ID_SolvingMenu) : menu(type) {}

    static bool build(menu_info &mi, list_p eq, bool withcmds = true);

public:
    OBJECT_DECL(SolvingMenu);
    MENU_DECL(SolvingMenu);
};

SPECIAL_MENU_DECLARE(solving_menu_store);
SPECIAL_MENU_DECLARE(solving_menu_solve);
SPECIAL_MENU_DECLARE(solving_menu_recall);

#endif // SOLVE_H
