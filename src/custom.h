#ifndef CUSTOM_H
#define CUSTOM_H
// ****************************************************************************
//  custom.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Custom menu, as defined by the Cst / CustomMenu variable
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
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

#include "list.h"
#include "runtime.h"
#include "command.h"
#include "menu.h"


struct CustomMenu : menu
// ----------------------------------------------------------------------------
//   The custom menu is a bit special
// ----------------------------------------------------------------------------
//   The CustomMenu shows custom in the current menu
//   For each variable, the function key evaluates it, shift recalls it,
//   and xshift stores it. In program mode, the function key shows the name
//   for evaluation purpose, and shifted, shows it between quotes
{
    CustomMenu(id type = ID_CustomMenu) : menu(type) {}

    static list_p custom();
    static uint count_custom();
    static void list_custom(info &mi);

public:
    OBJECT_DECL(CustomMenu);
    MENU_DECL(CustomMenu);
};

COMMAND_DECLARE(ToggleCustomMenu, 0);

#endif // CUSTOM_H
