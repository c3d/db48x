#ifndef UNIT_H
#  define UNIT_H
// ****************************************************************************
//  unit.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Unit objects represent objects such as 1_km/s.
//
//    The representation is a complex number where the x() part is the value
//    and the y() part is the unit. This makes it faster to extract them.
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
#include "complex.h"
#include "file.h"
#include "functions.h"
#include "menu.h"
#include "symbol.h"

GCP(unit);

struct unit : complex
// ----------------------------------------------------------------------------
//   A unit object is mostly like an equation, except for parsing
// ----------------------------------------------------------------------------
{
    unit(id type, algebraic_r value, algebraic_r uexpr):
        complex(type, value, uexpr) {}

    static unit_p make(algebraic_g v, algebraic_g u, id ty = ID_unit);
    static algebraic_p simple(algebraic_g v, algebraic_g u, id ty = ID_unit);

    algebraic_p value() const   { return x(); }
    algebraic_p uexpr() const   { return y(); }

    bool convert(algebraic_g &x, bool error = true) const;
    bool convert(unit_g &x, bool error = true) const;

    static bool convert_to_linear(algebraic_g &value, algebraic_g &uexpr);
    algebraic_p convert_to_real() const;

    static algebraic_p parse_uexpr(gcutf8 source, size_t &len);

    static unit_p lookup(symbol_p name, int *prefix_index = nullptr);

    unit_p cycle() const;
    unit_p custom_cycle(symbol_r sym) const;

    static unit_p get(object_p obj);
    static unit_p get_after_evaluation(object_p obj);

    static object_p si_prefixes_variable()
    {
        return object::static_object(object::ID_UnitsSIPrefixCycle);
    }

    static bool mode;           // Set to true to evaluate units
    static bool factoring;      // Set to true when factoring out units
    static bool nodates;        // Disable conversions about dates

public:
    OBJECT_DECL(unit);
    EVAL_DECL(unit);
    PARSE_DECL(unit);
    RENDER_DECL(unit);
    GRAPH_DECL(unit);
    HELP_DECL(unit);
};


struct unit_menu : menu
// ----------------------------------------------------------------------------
//   A unit menu is like a standard menu, but with conversion / functions
// ----------------------------------------------------------------------------
{
    unit_menu(id type) : menu(type) { }
    static utf8 name(id type, size_t &len);

public:
    MENU_DECL(unit_menu);
#if 0
    PARSE_DECL(unit_menu);
    RENDER_DECL(unit_menu);
#endif
};


struct unit_file : file
// ----------------------------------------------------------------------------
//   Manage a unit file
// ----------------------------------------------------------------------------
{
    unit_file(cstring name = "config/units.csv"): file(name, READING) {}
    ~unit_file() {}

    symbol_p    lookup(gcutf8 what,size_t len,bool menu=false,bool seek0=true);
    symbol_p    next(bool menu = false);
};


#define ID(i)
#define UNIT_MENU(UnitMenu)     struct UnitMenu : unit_menu {};
#include "ids.tbl"

COMMAND_DECLARE(Convert,2);
FUNCTION(UBase);
COMMAND_DECLARE(UFact,2);
COMMAND_DECLARE_INSERT(ConvertToUnitPrefix,-1);
FUNCTION(UVal);
COMMAND_DECLARE(ToUnit,2);
COMMAND_DECLARE_INSERT(ApplyUnit,1);
COMMAND_DECLARE_INSERT(ConvertToUnit,1);
COMMAND_DECLARE_INSERT(ApplyInverseUnit, 1);
COMMAND_DECLARE(UnitsSIPrefixCycle, 1);

FUNCTION(ToDegrees);
FUNCTION(ToRadians);
FUNCTION(ToGrads);
FUNCTION(ToPiRadians);

#endif // UNIT_H
