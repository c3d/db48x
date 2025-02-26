// ****************************************************************************
//  library.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the Library, defined by the config/library.csv file
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "library.h"

#include "program.h"
#include "renderer.h"


RECORDER(library,         16, "Xlib objects");
RECORDER(library_error,   16, "Error on xlib objects");



// ============================================================================
//
//   Xlib definitions
//
// ============================================================================

static const cstring basic_library[] =
// ----------------------------------------------------------------------------
//   List of basic entries in the library
// ----------------------------------------------------------------------------
//   clang-format off
{

    // ------------------------------------------------------------------------
    //   Physics
    // ------------------------------------------------------------------------

    "Secrets",          nullptr,

    "Dedicace", "\"À tous ceux qui se souviennent de Maubert électronique\"",
    "Dedicace", "\"À tous ceux qui se souviennent de Maubert électronique\"",
    "LibraryHelp",  "\"To modify the library, edit the config/library.csv file\"",
    "Physics",          nullptr,
    "SiDensity", "« → T '√((2.81e19)*(1.83e19))*(1_(1/cm^3))*exp(-(1.12_eV)/(2*Ⓒk*UBASE(T_K)))' »",
    "Fanning", "« → Roughness Reynolds 'IFTE(Reynolds≤2100;16/Reynolds;(4.781-((-2۰log((Roughness)/3.7+12/Reynolds)-4.781)^2/((-2۰log((Roughness)/3.7+2.51۰(-2۰log((Roughness)/3.7+12/Reynolds))/Reynolds))-2۰(-2۰log((Roughness)/3.7+12/Reynolds))+4.781)))^-2)' »",
};
//   clang-format on


static runtime &invalid_xlib_error()
// ----------------------------------------------------------------------------
//    Return the error message for invalid library
// ----------------------------------------------------------------------------
{
    return rt.invalid_xlib_error();
}


static bool show_builtin_library()
// ----------------------------------------------------------------------------
//   Show the builtin equations
// ----------------------------------------------------------------------------
{
    return Settings.ShowBuiltinLibrary();
}


const xlib::config xlib::library =
// ----------------------------------------------------------------------------
//  Define the configuration for the library
// ----------------------------------------------------------------------------
{
    .menu_help      = "Library",
    .help           = "",
    .prefix         = L'Ⓛ',
    .type           = ID_xlib,
    .first_menu     = ID_LibraryMenu00,
    .last_menu      = ID_LibraryMenu99,
    .name           = ID_XlibName,
    .value          = ID_XlibValue,
    .command        = ID_object,
    .file           = "config/library.csv",
    .library        = "library",
    .builtins       = basic_library,
    .nbuiltins      = sizeof(basic_library) / sizeof(*basic_library),
    .error          = invalid_xlib_error,
    .label          = nullptr,
    .show_builtins  = show_builtin_library,
    .stack_prefix   = false,
};



// ============================================================================
//
//   Menu implementation
//
// ============================================================================

PARSE_BODY(xlib)
// ----------------------------------------------------------------------------
//   Parse library entries
// ----------------------------------------------------------------------------
{
    return do_parsing(library, p);
}


EVAL_BODY(xlib)
// ----------------------------------------------------------------------------
//   Library entries evaluate like a program entry
// ----------------------------------------------------------------------------
{
    if (object_p value = o->attach())
        return program::run_program(value);
    if (!rt.error())
        rt.invalid_xlib_error();
    return ERROR;
}


RENDER_BODY(xlib)
// ----------------------------------------------------------------------------
//   Render the xlib into the given buffer
// ----------------------------------------------------------------------------
{
    return do_rendering(library, o, r);
}


GRAPH_BODY(xlib)
// ----------------------------------------------------------------------------
//   Render "normally"
// ----------------------------------------------------------------------------
{
    return object::do_graph(o, g);
}


HELP_BODY(xlib)
// ----------------------------------------------------------------------------
//   Help topic for libraries
// ----------------------------------------------------------------------------
{
    return o->do_instance_help(xlib::library);
}


MENU_BODY(library_menu)
// ----------------------------------------------------------------------------
//   Build a library menu
// ----------------------------------------------------------------------------
{
    return o->do_submenu(xlib::library, mi);
}


HELP_BODY(library_menu)
// ----------------------------------------------------------------------------
//   Show the help for the given library menu
// ----------------------------------------------------------------------------
{
    return o->do_menu_help(xlib::library, o);
}


MENU_BODY(Library)
// ----------------------------------------------------------------------------
//   The library menu is dynamically populated
// ----------------------------------------------------------------------------
{
    return xlib::do_collection_menu(xlib::library, mi);
}


utf8 library_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name for a menu entry
// ----------------------------------------------------------------------------
{
    return do_name(xlib::library, type, len);
}


object_p xlib::attach() const
// ----------------------------------------------------------------------------
//   Attach the library at the given index
// ----------------------------------------------------------------------------
{
    xlib_g   xl    = this;
    uint     idx   = xl->index();

    object_p value = rt.xlib(idx);
    if (!value)
    {
        // Resize the cache if needed
        if (idx >= rt.xlibs())
            if (!rt.attach(idx+1))
                return nullptr;;

        value = xl->value();
        if (!value)
        {
            rt.invalid_xlib_error();
            return nullptr;
        }
        rt.xlib(idx, value);
        cleaner::disable();
    }
    return value;
}


object_p xlib::detach() const
// ----------------------------------------------------------------------------
//   Detach the library at the given index
// ----------------------------------------------------------------------------
{
    xlib_g   xl    = this;
    uint     idx   = xl->index();
    if (idx < rt.xlibs())
        rt.xlib(idx, nullptr);
    return +xl;
}


xlib_p xlib::from_object(object_p obj)
// ----------------------------------------------------------------------------
//   Find the library item associated with an xlib, a name or an index
// ----------------------------------------------------------------------------
{
    xlib_g lib = obj->as_quoted<xlib>();
    if (!lib)
    {
        if (object_p inner = obj->as_quoted())
            obj = inner;
        if (text_g name = obj->as<text, symbol>())
        {
            size_t sz = 0;
            utf8 lname = name->value(&sz);
            lib = xlib_p(do_lookup(library, lname, sz, true));
        }
        else
        {
            uint idx = obj->as_uint32(0, true);
            if (rt.error())
                return nullptr;
            lib = xlib::make(idx);
        }
    }
    return lib;
}


bool xlib::operation(object_p obj, object_p (xlib::*op)() const)
// ----------------------------------------------------------------------------
//   Perform an operation on the given item or items
// ----------------------------------------------------------------------------
{
    if (list_p lst = obj->as_array_or_list())
    {
        for (object_p obj : *lst)
            if (!operation(obj, op))
                return false;
        return true;
    }

    if (xlib_p lib = xlib::from_object(obj))
        if ((lib->*op)())
            return true;
    if (!rt.error())
        rt.type_error();
    return false;
}


COMMAND_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the name of a xlib on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p cst = cstobj->as<xlib>())
            return xlib::do_evaluate(cst);
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the name of a xlib in the editor
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " Ⓛ", " ", false);
}


HELP_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the help for a given xlib function key
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    rt.command(o);
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p cst = cstobj->as<xlib>())
            return cst->help();
    return utf8("Library");
}


COMMAND_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Put the value of a xlib on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = xlib::do_key(xlib::library, key))
        if (xlib_p xl = cst->as<xlib>())
            if (object_p value = xl->value())
                if (rt.push(value))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Insert the value of a xlib
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p xl = cstobj->as<xlib>())
            if (object_p value = xl->value())
                return ui.insert_object(value, " ", " ");
    return ERROR;
}


HELP_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Put the help for a given library function ley
// ----------------------------------------------------------------------------
{
    return XlibName::do_help(nullptr);
}


COMMAND_BODY(XLib)
// ----------------------------------------------------------------------------
//   Evaluate a library item
// ----------------------------------------------------------------------------
{
    return xlib::lookup_command(xlib::library, false);
}


COMMAND_BODY(Attach)
// ----------------------------------------------------------------------------
//   Attach a library, i.e. ensure it is loaded in the cache
// ----------------------------------------------------------------------------
{
    object_g obj = rt.pop();
    if (xlib::operation(obj, &xlib::attach))
        return OK;
    rt.push(obj);
    return ERROR;
}


COMMAND_BODY(Detach)
// ----------------------------------------------------------------------------
//   Detach a library, i.e. ensure it is no longer loaded in the cache
// ----------------------------------------------------------------------------
{
    object_g obj = rt.pop();
    if (xlib::operation(obj, &xlib::detach))
    {
        size_t nlibs   = rt.xlibs();
        size_t highest = 0;
        for (size_t i = 0; i < nlibs; i++)
        {
            if (rt.xlib(nlibs + ~i))
            {
                highest = nlibs - i;
                break;
            }
        }
        if (highest < nlibs)
            rt.attach(highest);
        return OK;
    }
    rt.push(obj);
    return ERROR;
}


COMMAND_BODY(Libs)
// ----------------------------------------------------------------------------
//   List the currently attached libraries
// ----------------------------------------------------------------------------
{
    size_t depth = rt.depth();
    for (size_t i = 0; i < rt.xlibs(); i++)
    {
        if (rt.xlib(i))
        {
            xlib_p xl = xlib::make(i);
            if (!rt.push(xl))
            {
                rt.drop(rt.depth() - depth);
                return ERROR;
            }
        }
    }
    return list::push_list_from_stack(rt.depth() - depth);
}
