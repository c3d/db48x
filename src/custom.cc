// ****************************************************************************
//  custom.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Implementation of custom
//
//      Global custom are stored in mutable directory objects that occupy
//      a reserved area of the runtime, and can grow/shrinnk as you store
//      or purge global custom
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

#include "custom.h"
#include "variables.h"



COMMAND_BODY(ToggleCustomMenu)
// ----------------------------------------------------------------------------
//   List custom in current directory matching the type
// ----------------------------------------------------------------------------
{
    menu_p m = ui.menu();
    if (m && m->type() == ID_VariablesMenu)
        if (CustomMenu::custom())
            return run<CustomMenu>();
    return run<VariablesMenu>();
}



// ============================================================================
//
//    Custom menu
//
// ============================================================================

MENU_BODY(CustomMenu)
// ----------------------------------------------------------------------------
//   Process the MENU command for CustomMenu
// ----------------------------------------------------------------------------
{
    uint  nitems = count_custom();
    items_init(mi, nitems);
    list_custom(mi);
    return OK;
}


list_p CustomMenu::custom()
// ----------------------------------------------------------------------------
//   Return current custom menu from array or list
// ----------------------------------------------------------------------------
{
    object_p name = static_object(ID_CustomMenu);
    if (object_p cst = directory::recall_all(name, false))
        if (list_p cstl = cst->as_array_or_list())
            return cstl;
    return nullptr;
}


uint CustomMenu::count_custom()
// ----------------------------------------------------------------------------
//    Count the custom entries in the current directory
// ----------------------------------------------------------------------------
{
    if (list_p cst = custom())
        return cst->items();
    return 0;
}


void CustomMenu::list_custom(info &mi)
// ----------------------------------------------------------------------------
//   Fill the menu with variable names
// ----------------------------------------------------------------------------
{
    if (list_p cst = custom())
    {
        for (object_g obj : *cst)
        {
            symbol_p sym = obj->as_quoted<symbol>();
            if (sym)
            {
                if (object_p obj = directory::recall_all(sym, false))
                    if (obj->type() == object::ID_directory)
                        mi.marker = L'◥';
            }
            else
            {
                if (list_p lst = obj->as_array_or_list())
                {
                    object_p name = lst->at(0);
                    object_p payload = lst->at(1);
                    if (name && payload)
                    {
                        id nty = name->type();
                        if (nty == ID_text || nty == ID_symbol)
                        {
                            sym = symbol_p(name);
                            obj = payload;
                        }
                    }
                }

            }
            if (!sym)
                sym = obj->as_symbol(false);
            if (obj->type() == object::ID_directory)
                mi.marker = L'◥';
            menu::items(mi, sym, obj);
        }
    }
}
