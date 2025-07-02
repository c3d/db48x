#ifndef CONSTANTS_H
#define CONSTANTS_H
// ****************************************************************************
//  constants.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Constant values loaded from a constants file
//
//     Constants are loaded from a `config/constants.csv` file.
//     This makes it possible to define them with arbitrary precision
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

#include "algebraic.h"
#include "array.h"
#include "command.h"
#include "menu.h"


GCP(constant);
GCP(constant_menu);
GCP(standard_uncertainty);
GCP(relative_uncertainty);


struct constant : algebraic
// ----------------------------------------------------------------------------
//   A constant is a symbol where the value is looked up from a file
// ----------------------------------------------------------------------------
{
    constant(id type, uint index) : algebraic(type)
    {
        byte *p = (byte *) payload();
        leb128(p, index);
    }

    static size_t required_memory(id i, uint index)
    {
        return leb128size(i) + leb128size(index);
    }


    typedef const cstring *builtins_p;
    struct config
    // ------------------------------------------------------------------------
    //   Configuration for a kind of file-based constants
    // ------------------------------------------------------------------------
    {
        cstring    menu_help;   // Help base for menus
        cstring    help;        // Help base for objects of the category
        unicode    prefix;      // Prefix identifying constant type (Ⓒ, Ⓔ, Ⓛ)
        id         type;        // Type for constants, e.g. ID_xlib
        id         first_menu;  // First possible menu, e.g. ID_EquationsMenu00
        id         last_menu;   // Last possible menu, e.g. ID_EquationsMenu99
        id         name;        // Menu command for the name
        id         value;       // Menu command for the value
        id         command;     // Menu command for third-level row
        cstring    file;        // CSV file for names and definitions
        cstring    library;     // Path for library files
        builtins_p builtins;    // Builtins definitions
        size_t     nbuiltins;   // Number of entries in builtins[]
        runtime &  (*error)();  // Emit error message
        symbol_p   (*label)(symbol_r); // Menu label adustment
        bool       (*show_builtins)(); // How to check if we show builtins
        bool       stack_prefix;       // Show prefix on the stack
    };
    typedef const config &config_r;

    static constant_p make(uint index)
    {
        return rt.make<constant>(ID_constant, index);
    }

    static constant_p make(id type, uint index)
    {
        return rt.make<constant>(type, index);
    }

    static constant_p lookup(utf8 name, size_t len, bool error)
    {
        return do_lookup(constants, name, len, error);
    }

    static constant_p lookup(cstring name, bool error = true)
    {
        return lookup(utf8(name), strlen(name), error);
    }

    uint        index() const
    {
        byte_p p = payload();
        return leb128<uint>(p);
    }

    utf8        name(size_t *size = nullptr) const
    {
        return do_name(constants, size);
    }
    algebraic_p specification() const
    {
        if (object_p obj = cache())
            if (obj->is_extended_algebraic())
                return algebraic_p(obj);
        return nullptr;
    }
    algebraic_p value() const
    {
        uint index = value_index();
        algebraic_p val = specification();
        if (val)
            if (array_p spec = val->as<array>())
                if (object_p obj = spec->at(index))
                    if (algebraic_p alg = obj->as_extended_algebraic())
                        val = alg;
        return val;
    }
    algebraic_p numerical_value() const;
    uint value_index() const
    {
        switch (type())
        {
        default:
        case ID_constant:               return 0;
        case ID_standard_uncertainty:   return 1;
        case ID_relative_uncertainty:   return 2;
        }
    }
    bool is_imaginary_unit() const
    {
        return matches("ⅈ") || matches("ⅉ");
    }
    bool        is_pi() const
    {
        return matches("π");
    }
    bool is_simplifiable() const
    {
        return !matches("∞") && !matches("?");
    }
    bool        matches(cstring ref) const
    {
        size_t nlen = strlen(ref);
        size_t len = 0;
        utf8 txt = name(&len);
        return len == nlen && memcmp(ref, txt, len) == 0;
    }

    static result    lookup_command(config_r cfg, bool numerical);
    static object_p  lookup_menu(config_r cfg, utf8 name, size_t len);
    static object_p  lookup_menu(config_r cfg, cstring name);

    object_p          cache() const;
    object_p          uncache() const;

protected:
    static result     do_parsing(config_r cfg, parser &p);
    static size_t     do_rendering(config_r cfg, constant_p o, renderer &r);
    static constant_p do_lookup(config_r cfg, utf8 name, size_t len, bool error);
    utf8              do_name(config_r cfg, size_t *size = nullptr) const;
    object_p          do_value(config_r cfg) const;
    utf8              do_instance_help(config_r cfg) const;

public:
    static bool       do_collection_menu(config_r cfg, menu_info &mi);
    static constant_p do_key(config_r cfg, int key);

public:
    OBJECT_DECL(constant);
    PARSE_DECL(constant);
    SIZE_DECL(constant);
    EVAL_DECL(constant);
    RENDER_DECL(constant);
    GRAPH_DECL(constant);
    HELP_DECL(constant);

public:
    static const config constants;
};


struct constant_menu : menu
// ----------------------------------------------------------------------------
//   A constant menu is like a standard menu, but with constants
// ----------------------------------------------------------------------------
{
    constant_menu(id type) : menu(type) { }
    static utf8 name(id type, size_t &len);

protected:
    using config_r = constant::config_r;
    bool        do_submenu(config_r cfg, menu_info &mi) const;
    static utf8 do_name(config_r cfg, id base, size_t &len);
    utf8        do_menu_help(config_r cfg, constant_menu_p o) const;

public:
    MENU_DECL(constant_menu);
    HELP_DECL(constant_menu);
};


#define ID(i)
#define CONSTANT_MENU(ConstantMenu)     struct ConstantMenu : constant_menu {};
#include "ids.tbl"

COMMAND_DECLARE_INSERT_HELP(ConstantName,-1);
COMMAND_DECLARE_INSERT_HELP(ConstantValue,-1);
COMMAND_DECLARE(Const, 1);
COMMAND_DECLARE(StandardUncertainty, 1);
COMMAND_DECLARE(RelativeUncertainty, 1);
COMMAND_DECLARE(Constants, 0);


struct standard_uncertainty : constant
// ----------------------------------------------------------------------------
//  The standard uncertainty for a constant
// ----------------------------------------------------------------------------
{
    standard_uncertainty(id type, uint index): constant(type, index) {}

    OBJECT_DECL(standard_uncertainty);
    SIZE_DECL(standard_uncertainty);
    PARSE_DECL(standard_uncertainty);
    RENDER_DECL(standard_uncertainty);

public:
    static const config standard;
};


struct relative_uncertainty : constant
// ----------------------------------------------------------------------------
//  The relative uncertainty for a constant
// ----------------------------------------------------------------------------
{
    relative_uncertainty(id type, uint index): constant(type, index) {}

    OBJECT_DECL(relative_uncertainty);
    SIZE_DECL(relative_uncertainty);
    PARSE_DECL(relative_uncertainty);
    RENDER_DECL(relative_uncertainty);

public:
    static const config relative;
};


#endif // CONSTANT_H
