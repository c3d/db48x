#ifndef PLOT_H
#define PLOT_H
// ****************************************************************************
//  plot.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Function and curve plotting
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

#include "command.h"
#include "graphics.h"

COMMAND_DECLARE(Function,1);
COMMAND_DECLARE(Polar,1);
COMMAND_DECLARE(Parametric,1);
COMMAND_DECLARE(Scatter,1);
COMMAND_DECLARE(Bar,1);
COMMAND_DECLARE(Histogram,1);
COMMAND_DECLARE(Draw,0);
COMMAND_DECLARE(Drax,0);

struct Equation : command
// ----------------------------------------------------------------------------
//   A shortcut name for `EQ`
// ----------------------------------------------------------------------------
{
    Equation(id ty = ID_Equation): command(ty) {}
};

#endif // PLOT_H
