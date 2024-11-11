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

#include "array.h"
#include "decimal.h"
#include "integer.h"
#include "program.h"
#include "target.h"
#include "variables.h"


COMMAND_BODY(ToggleCustomMenu)
// ----------------------------------------------------------------------------
//   List custom in current directory matching the type
// ----------------------------------------------------------------------------
{
    menu_p m = ui.menu();
    if (m && m->type() == ID_VariablesMenu)
    {
        if (CustomMenu::custom())
            return run<CustomMenu>();
        if (rt.error())
            return ERROR;
    }
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
    {
        list_p cstl = cst->as_array_or_list();
        if (!cstl)
            if (algebraic_p alg = cst->as_extended_algebraic())
                if (algebraic_p eval = alg->evaluate())
                    cstl = eval->as_array_or_list();
        if (cstl)
            return cstl;
        rt.invalid_custom_menu_error();
    }
    return nullptr;
}


uint CustomMenu::count_custom()
// ----------------------------------------------------------------------------
//    Count the custom entries in the current directory
// ----------------------------------------------------------------------------
{
    if (list_p cst = custom())
    {
        if (array_p ary = cst->as<array>())
        {
            size_t rows = 0, columns = 0;
            if (ary->is_matrix(&rows, &columns, false))
                if (rows <= 3)
                    return columns;
        }
        return cst->items();
    }
    return 0;
}


void CustomMenu::list_custom(info &mi, list_p cst)
// ----------------------------------------------------------------------------
//   Fill the menu with variable names
// ----------------------------------------------------------------------------
{
    if (!cst)
        cst = custom();
    if (cst)
    {
        if (array_p ary = cst->as<array>())
        {
            size_t rows = ary->items();
            if (rows <= 3)
            {
                size_t columns = 0;
                for (size_t r = 0; r < rows; r++)
                {
                    if (array_p rvec = ary->at(r)->as<array>())
                    {
                        size_t cols = rvec->items();
                        if (columns < cols)
                            columns = cols;
                    }
                }

                size_t page = rows * columns <= 18 ? 6 : 5;
                for (size_t r = 0; r < rows; r++)
                {
                    array_p rvec = ary->at(r)->as<array>();
                    size_t cols = rvec->items();
                    for (size_t c = 0; c < cols; c++)
                        add_custom_item(mi, rvec->at(c));
                    for (size_t c = cols; c % page; c++)
                        add_custom_item(mi, nullptr);
                }
                return;
            }
        }
        for (object_g obj : *cst)
            add_custom_item(mi, obj);
    }
}


void CustomMenu::add_custom_item(info &mi, object_p obj)
// ----------------------------------------------------------------------------
//   Add an object to the list
// ----------------------------------------------------------------------------
{
    bool separator = !obj;
    symbol_p sym;

    if (obj)
    {
        sym = obj->as_quoted<symbol>();
        if (sym)
        {
            if (object_p value = directory::recall_all(sym, false))
                if (value->type() == object::ID_directory)
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
                else if (name)
                {
                    obj = name;
                }
                else
                {
                    // Accept {} as a separator
                    separator = true;
                }
            }
            else if (text_p txt = obj->as<text>())
            {
                if (txt->length() == 0)
                    separator = true;
            }
        }
    }
    if (separator)
    {
        sym = symbol::make("");
        obj = program::make(ID_program, nullptr, 0);
    }
    if (!sym)
        sym = obj->as_symbol(false);
    if (obj->type() == object::ID_directory)
        mi.marker = L'◥';
    menu::items(mi, sym, obj);
}


object::result CustomMenu::run_menu_command(bool tmp)
// ----------------------------------------------------------------------------
//   Shared code between Menu and TMenu
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        id oty = obj->type();
        if (object::is_real(oty))
        {
            if (obj->is_zero(false))
                if (rt.drop())
                    return run<LastMenu>();

            // Translation of MENU values is not implemented
            rt.unimplemented_error();
            return ERROR;
        }

        // Check names
        if (symbol_p sym = obj->as_quoted<symbol>())
        {
            obj = directory::recall_all(sym, true);
            if (!obj)
                return ERROR;
            oty = obj->type();
        }

        // Consume the object
        if (!rt.drop())
            return ERROR;

        // Result from RclMenu: apply as is
        if (is_menu(oty))
        {
            ui.menu(menu_p(obj));
            return OK;
        }

        // Check the input (differs from HP50G documented behaviour)
        if (!obj->is_array_or_list())
        {
            rt.push(obj);
            rt.type_error();
            return ERROR;
        }

        // Check if we have a persistent menu
        if (!tmp)
        {
            // Store in CST and evaluate the custom menu
            object_p cst = static_object(ID_CustomMenu);
            if (directory::store_here(cst, obj))
                return run<CustomMenu>();
        }

        // Run the custom menu
        list_g tmenu  = list_p(obj);
        uint   nitems = tmenu->items();
        info   mi(0);
        items_init(mi, nitems);
        list_custom(mi, tmenu);
        return OK;
    }
    return ERROR;

}



COMMAND_BODY(Menu)
// ----------------------------------------------------------------------------
//   Define the Custom menu from the input
// ----------------------------------------------------------------------------
{
    return CustomMenu::run_menu_command(false);
}


COMMAND_BODY(TemporaryMenu)
// ----------------------------------------------------------------------------
//   Define a temporary menu
// ----------------------------------------------------------------------------
{
    return CustomMenu::run_menu_command(true);
}


COMMAND_BODY(RecallMenu)
// ----------------------------------------------------------------------------
//   Recall current menu
// ----------------------------------------------------------------------------
{
    if (menu_p m = ui.menu())
    {
        if (rt.push(m))
            return OK;
    }
    else
    {
        if (integer_p z = integer::make(0))
            if (rt.push(z))
                return OK;
    }
    return ERROR;
}


COMMAND_BODY(KeyMap)
// ----------------------------------------------------------------------------
//   Store a complete keymap (like STOKEY with native key IDs)
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (directory_p keymap = obj->as<directory>())
            if (object_p name = static_object(ID_KeyMap))
                if (directory::store_here(name, keymap))
                    if (rt.drop())
                        return OK;
    if (!rt.error())
        rt.value_error();
    return ERROR;
}


static uint convert_key_position_to_id(object_p obj)
// ----------------------------------------------------------------------------
//    Convert the HP48 format (rc.p = row, column, plane) to native key
// ----------------------------------------------------------------------------
{
    uint rc = obj->as_uint32(0, true);

    if (decimal_g rcp = obj->as<decimal>())
    {
        if (rc < 100)
        {
            if (decimal_g rcps = decimal::make(1, 2)) // Decimal part * 100
            {
                if (decimal_g rcpf = (rcps * rcp) % rcps)
                {
                    ularge pf = rcpf->as_unsigned();
                    return platform_keyid(rc, uint(pf));
                }
            }
        }
        else
        {
            rt.value_error();
        }
    }
    return platform_keyid(rc, 0);
}


COMMAND_BODY(AssignKey)
// ----------------------------------------------------------------------------
//   Assign a single key
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.stack(1))
        if (object_g pos = rt.stack(0))
            if (uint keyid = convert_key_position_to_id(pos))
                if (ui.assign(keyid, obj))
                    if (rt.drop(2))
                        return OK;
    return ERROR;
}


COMMAND_BODY(DeleteKeys)
// ----------------------------------------------------------------------------
//  Delete user-defined keys
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (obj->is_zero(false))
        {
            object_p name = static_object(ID_KeyMap);
            directory *curdir = rt.variables(0);
            curdir->purge(name);
            return OK;
        }

        if (list_p keys = obj->as_array_or_list())
        {
            for (object_p key : *keys)
            {
                if (uint keyid = convert_key_position_to_id(key))
                    ui.assign(keyid, nullptr);
                else
                    return rt.value_error(), ERROR;
            }
            return OK;
        }
        if (uint keyid = convert_key_position_to_id(obj))
            if (ui.assign(keyid, nullptr))
                return OK;

        rt.value_error();
    }
    return ERROR;
}


COMMAND_BODY(StoreKeys)
// ----------------------------------------------------------------------------
//   Store a sequence of keys
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (list_p keys = obj->as_array_or_list())
        {
            object_g assigned;
            for (object_p obj : *keys)
            {
                if (assigned)
                {
                    // Got a RCLKEY with updir values, ignore them.
                    if (obj->type() == ID_UpDir &&
                        assigned->type() == ID_integer)
                        break;

                    uint keyid = convert_key_position_to_id(obj);
                    if (!keyid)
                    {
                        rt.value_error();
                        return ERROR;
                    }
                    if (!ui.assign(keyid, assigned))
                        return ERROR;
                    assigned = nullptr;
                }
                else
                {
                    assigned = obj;
                }
            }
            rt.drop();
            return OK;
        }
        rt.type_error();
    }
    return ERROR;
}


static bool compatible_keyid(object_p name, object_p obj, void *)
// ----------------------------------------------------------------------------
//   Append the compatible key ID to the list
// ----------------------------------------------------------------------------
{
    if (integer_p keyarg = name->as<integer>())
    {
        ularge keyid = keyarg->value<ularge>();
        object_p ckey = compatible_key_id(keyid);
        return ckey && obj && rt.append(obj) && rt.append(ckey);
    }
    return false;
}


COMMAND_BODY(RecallKeys)
// ----------------------------------------------------------------------------
//  Convert the key map to a format that is compatible with the HP48
// ----------------------------------------------------------------------------
{
    object_p name = object::static_object(object::ID_KeyMap);
    scribble scr;
    directory *dir = nullptr;
    size_t     used = 0;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
    {
        if (depth)
        {
            object_p updir = static_object(ID_UpDir);
            integer_p level = integer::make(depth);
            if (!rt.append(level) || !rt.append(updir))
                return ERROR;
        }
        if (object_p keymapvar = dir->recall(name))
        {
            if (directory_p keymap = keymapvar->as<directory>())
            {
                keymap->enumerate(compatible_keyid, nullptr);
                used = scr.growth();
            }
        }
    }
    if (!rt.error())
        if (list_p keys = list::make(ID_list, scr.scratch(), used))
            if (rt.push(keys))
                return OK;
    return ERROR;
}


COMMAND_BODY(StandardKey)
// ----------------------------------------------------------------------------
//   A no-op when evaluated
// ----------------------------------------------------------------------------
{
    return OK;
}


COMMAND_BODY(ToggleUserMode)
// ----------------------------------------------------------------------------
//   Immediate for the USER key
// ----------------------------------------------------------------------------
//   This cycles across 1USR USR and normal
{
    ui.toggle_user();
    return OK;
}


object_p compatible_key_id(uint keyid)
// ----------------------------------------------------------------------------
//   Return a key ID in an HP-compatible format
// ----------------------------------------------------------------------------
{
    uint rc = compatible_key_position(keyid);
    uint pl = compatible_key_plane(keyid);
    object_p ckey = nullptr;
    if (pl)
    {
        decimal_g rcd = decimal::make(rc, 0);
        decimal_g rcf = decimal::make(pl, -2);
        rcd = rcd + rcf;
        ckey = rcd;
    }
    else
    {
        ckey = integer::make(rc);
    }
    return ckey;
}
