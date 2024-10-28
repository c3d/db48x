#ifndef MENU_H
#define MENU_H
// ****************************************************************************
//  menu.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//     An RPL menu object defines the content of the soft menu keys
//
//     It is a directory which, when evaluated, updates the soft menu keys
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

#include "command.h"
#include "user_interface.h"
#include "symbol.h"


struct menu_info
// ----------------------------------------------------------------------------
//  Info filled by the menu() interface
// ----------------------------------------------------------------------------
{
    menu_info(uint page = 0, int planes = 0, uint skip = 0, int marker = 0)
        : page(page), skip(skip), marker(marker),
          pages(0), index(0), plane(0), planes(planes) {}
    uint page;   // In:  Page index
    uint skip;   // In:  Items to skip
    int  marker; // In:  Default marker
    uint pages;  // Out: Total number of pages
    uint index;  // Out: Last index written
    uint plane;  // Out: Last plane filled
    uint planes; // Out: Planes the menu wants
};


struct menu : command
// ----------------------------------------------------------------------------
//   An RPL menu object, can define menu keys
// ----------------------------------------------------------------------------
{
    menu(id type = ID_menu) : command(type)     { }

    typedef menu_info info;

    result update(uint page = 0) const
    {
        info mi(page);
        return ops().menu(this, mi) ? OK : ERROR;
    }

    static void items_init(info &mi, uint nitems, uint planes, uint vplanes);
    static void items_init(info &mi, uint nitems, uint planes = 3)
    {
        items_init(mi, nitems, planes, planes);
    }
    static void items(info &) { }
    static void items(info &mi, id action);
    static void items(info &mi, cstring label, object_p action);
    static void items(info &mi, cstring label, id action)
    {
        return items(mi, label, command::static_object(action));
    }
    static void items(info &mi, symbol_p label, object_p action)
    {
        return items(mi, cstring(label), action);
    }
    static void items(info &mi, symbol_p label, id action)
    {
        return items(mi, cstring(label), action);
    }

    template <typename... Args>
    static void items(info &mi, cstring label, id action, Args... args);

    template <typename... Args>
    static void items(info &mi, id action, Args... args);

    static uint count()
    {
        return 0;
    }
    template <typename... Args>
    static uint count(id UNUSED action, Args... args)
    {
        return 1 + count(args...);
    }
    template <typename... Args>
    static uint count(cstring UNUSED label, id UNUSED action, Args... args)
    {
        return 1 + count(args...);
    }

    // Dynamic menus
    typedef cstring (*menu_label_fn)(object::id ty);

    template <typename... Args>
    static uint count(menu_label_fn UNUSED lbl, id UNUSED action, Args... args)
    {
        return 1 + count(args...);
    }

    template <typename... Args>
    static void items(info &mi, menu_label_fn label, id action, Args... args);

public:
    OBJECT_DECL(menu);
    EVAL_DECL(menu);
    MARKER_DECL(menu);
};


template <typename... Args>
void menu::items(info &mi, cstring label, id type, Args... args)
// ----------------------------------------------------------------------------
//   Update menu items
// ----------------------------------------------------------------------------
{
    items(mi, label, type);
    items(mi, args...);
}


template <typename... Args>
void menu::items(info &mi, menu_label_fn lblfn, id type, Args... args)
// ----------------------------------------------------------------------------
//   Update menu items
// ----------------------------------------------------------------------------
{
    items(mi, lblfn(type), type);
    items(mi, args...);
}


template <typename... Args>
void menu::items(info &mi, id type, Args... args)
// ----------------------------------------------------------------------------
//   Update menu items
// ----------------------------------------------------------------------------
{
    items(mi, type);
    items(mi, args...);
}



// ============================================================================
//
//   Commands inserted in menus
//
// ============================================================================

COMMAND(MenuNextPage,-1)
// ----------------------------------------------------------------------------
//   Select the next page in the menu
// ----------------------------------------------------------------------------
{
    ui.page(ui.page() + 1);
    return OK;
}


COMMAND(MenuPreviousPage,-1)
// ----------------------------------------------------------------------------
//   Select the previous page in the menu
// ----------------------------------------------------------------------------
{
    ui.page(ui.page() - 1);
    return OK;
}


COMMAND(MenuFirstPage,-1)
// ----------------------------------------------------------------------------
//   Select the previous page in the menu
// ----------------------------------------------------------------------------
{
    ui.page(0);
    return OK;
}

#define ID(i)
#define MENU(SysMenu)                                                   \
struct SysMenu : menu                                                   \
/* ------------------------------------------------------------ */      \
/*   Create a system menu                                       */      \
/* ------------------------------------------------------------ */      \
{                                                                       \
    SysMenu(id type = ID_##SysMenu) : menu(type) { }                    \
    OBJECT_DECL(SysMenu);                                               \
    MENU_DECL(SysMenu);                                                 \
};
#include "ids.tbl"



// ============================================================================
//
//   Commands related to menus
//
// ============================================================================

COMMAND_DECLARE(LastMenu,-1);   // Return to previous menu
COMMAND_DECLARE(ToolsMenu,-1);  // Automatic selection of the right menu

#endif // MENU_H
