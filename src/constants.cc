// ****************************************************************************
//  constants.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Constant values loaded from a constants file
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

#include "constants.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "compare.h"
#include "equations.h"
#include "expression.h"
#include "file.h"
#include "files.h"
#include "functions.h"
#include "grob.h"
#include "library.h"
#include "parser.h"
#include "renderer.h"
#include "settings.h"
#include "unit.h"
#include "user_interface.h"
#include "utf8.h"
#include "variables.h"

RECORDER(constants,         16, "Constant objects");
RECORDER(constants_error,   16, "Error on constant objects");


// ============================================================================
//
//   Parsing the constant from the constant file
//
// ============================================================================

PARSE_BODY(constant)
// ----------------------------------------------------------------------------
//    Skip, the actual parsing is done in the symbol parser
// ----------------------------------------------------------------------------
{
    return do_parsing(constants, p);
}


SIZE_BODY(constant)
// ----------------------------------------------------------------------------
//   Compute the size
// ----------------------------------------------------------------------------
{
    object_p p = object_p(payload(o));
    p += leb128size(p);
    return byte_p(p) - byte_p(o);
}


RENDER_BODY(constant)
// ----------------------------------------------------------------------------
//   Render the constant into the given constant buffer
// ----------------------------------------------------------------------------
{
    return do_rendering(constants, o, r);
}


GRAPH_BODY(constant)
// ----------------------------------------------------------------------------
//   Do not italicize constants, but render as bold
// ----------------------------------------------------------------------------
{
    using pixsize = grob::pixsize;

    grob_g sym = object::do_graph(o, g);
    if (!sym)
        return nullptr;

    pixsize sw    = sym->width();
    pixsize sh    = sym->height();
    pixsize rw    = sw + 1;
    pixsize rh    = sh;
    grob_g result = g.grob(rw, rh);
    if (!result)
        return nullptr;

    grob::surface ss = sym->pixels();
    grob::surface rs = result->pixels();

    rs.fill(0, 0, rw, rh, g.background);
    rs.copy(ss, 0, 0);
    blitter::blit<blitter::DRAW>(rs, ss,
                                 rect(1, 0, sw, sh-1), point(),
                                 blitter::blitop_and, pattern::black);

    return result;
}


EVAL_BODY(constant)
// ----------------------------------------------------------------------------
//   Check if we need to convert to numeric
// ----------------------------------------------------------------------------
{
    // Check if we should preserve the constant as is
    if (!Settings.NumericalConstants() && !Settings.NumericalResults())
        return rt.push(o) ? OK : ERROR;
    object_p value = o->numerical_value();
    return value && rt.push(value) ? OK : ERROR;
}


HELP_BODY(constant)
// ----------------------------------------------------------------------------
//   Help topic for constants
// ----------------------------------------------------------------------------
{
    return o->do_instance_help(constant::constants);
}


algebraic_p constant::numerical_value() const
// ----------------------------------------------------------------------------
//   Evaluate a constant as a numerical value
// ----------------------------------------------------------------------------
{
    save<bool> nodates(unit::nodates, true);
    if (algebraic_g a = value())
    {
        to_decimal(a, true);
        return a;
    }
    return nullptr;
}


algebraic_p constant::range() const
// ----------------------------------------------------------------------------
//   Evaluate a constant as a range
// ----------------------------------------------------------------------------
{
    save<bool> nodates(unit::nodates, true);
    if (algebraic_g value = specification())
    {
        algebraic_g uncertainty;
        if (array_p spec = value->as<array>())
        {
            if (object_p obj = spec->at(0))
                if (algebraic_p alg = obj->as_extended_algebraic())
                    value = alg;
            if (object_p obj = spec->at(1))
                if (algebraic_p alg = obj->as_extended_algebraic())
                    uncertainty = alg;
        }
        if (value)
            to_decimal(value, true);
        if (uncertainty)
            to_decimal(uncertainty, true);

        algebraic_g uexpr;
        if (unit_g uval = unit::get(value))
        {
            uexpr = uval->uexpr();
            if (!uval->convert(uncertainty))
                return nullptr;
            unit_g uunc = unit::get(uncertainty);
            if (!uunc)
                return nullptr;
            value = uval->value();
            uncertainty = uunc->value();
        }
        if (value && uncertainty)
        {
            if (range::adjust_input(ID_drange, value, uncertainty))
                if (range_p range = range::make(ID_drange, value, uncertainty))
                    value = range;
            if (uexpr && value)
                value = unit::make(value, uexpr);
        }
        return value;
    }
    return nullptr;
}


MENU_BODY(constant_menu)
// ----------------------------------------------------------------------------
//   Build a constants menu
// ----------------------------------------------------------------------------
{
    return o->do_submenu(constant::constants, mi);
}


HELP_BODY(constant_menu)
// ----------------------------------------------------------------------------
//   Show the help for the given constant
// ----------------------------------------------------------------------------
{
    return o->do_menu_help(constant::constants, o);
}


EVAL_BODY(constant_menu_name)
// ----------------------------------------------------------------------------
//   Put the name of a constant on the stack
// ----------------------------------------------------------------------------
{
    rt.command(static_object(ID_ConstantName));
    int key = ui.evaluating;
    unicode pfx = ui.character_left_of_cursor();
    const constant::config &cfg = pfx == L'Ⓡ' ? relative_uncertainty::relative
                                : pfx == L'Ⓢ' ? standard_uncertainty::standard
                                              : constant::constants;
    if (object_p cstobj = constant::do_key(cfg, key))
        if (constant_p cst = cstobj->as<constant>())
            if (rt.push(cst))
                return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(constant_menu_name)
// ----------------------------------------------------------------------------
//   Put the name of a constant in the editor
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    unicode prefix = ui.character_left_of_cursor();
    bool noprefix = prefix == L'Ⓒ' || prefix == L'Ⓡ' || prefix == L'Ⓢ';
    return ui.insert_softkey(key, noprefix ? "" : " Ⓒ", " ", false);
}


HELP_BODY(constant_menu_name)
// ----------------------------------------------------------------------------
//   Put the help for a given constant name
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
        if (constant_p cst = cstobj->as<constant>())
            return cst->help();
    return utf8("Constants");
}


EVAL_BODY(constant_menu_value)
// ----------------------------------------------------------------------------
//   Put the value of a constant on the stack
// ----------------------------------------------------------------------------
{
    rt.command(static_object(ID_ConstantValue));
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
        if (constant_p cst = cstobj->as<constant>())
            if (algebraic_p value = cst->numerical_value())
                if (rt.push(value))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(constant_menu_value)
// ----------------------------------------------------------------------------
//   Insert the value of a constant
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
        if (constant_p cst = cstobj->as<constant>())
            if (object_p value = cst->numerical_value())
                return ui.insert_object(value, " ", " ");
    return ERROR;
}


HELP_BODY(constant_menu_value)
// ----------------------------------------------------------------------------
//   Put the help for a given constant name
// ----------------------------------------------------------------------------
{
    return constant_menu_name::do_help(nullptr);
}


EVAL_BODY(constant_menu_range)
// ----------------------------------------------------------------------------
//   Put the value of a constant on the stack
// ----------------------------------------------------------------------------
{
    rt.command(static_object(ID_ConstantRange));
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
        if (constant_p cst = cstobj->as<constant>())
            if (algebraic_p range = cst->range())
                if (rt.push(range))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(constant_menu_range)
// ----------------------------------------------------------------------------
//   Insert the range associated to a constant
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
        if (constant_p cst = cstobj->as<constant>())
            if (object_p range = cst->range())
                return ui.insert_object(range, " ", " ");
    return ERROR;
}


HELP_BODY(constant_menu_range)
// ----------------------------------------------------------------------------
//   Put the help for a given constant name
// ----------------------------------------------------------------------------
{
    return constant_menu_name::do_help(nullptr);
}


MENU_BODY(ConstantsMenu)
// ----------------------------------------------------------------------------
//   The constants menu is dynamically populated
// ----------------------------------------------------------------------------
{
    bool ok = constant::do_collection_menu(constant::constants, mi);
    if (ok)
        items(mi,
              "Ⓒ",              ID_SelfInsert,
              "Const",          ID_Const,
              "Ⓢ",              ID_SelfInsert,
              "StdUnc",         ID_StandardUncertainty,
              "Ⓡ",              ID_SelfInsert,
              "RelUnc",         ID_RelativeUncertainty);
    return ok;
}


utf8 constant_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name for a menu entry
// ----------------------------------------------------------------------------
{
    return do_name(constant::constants, type, len);
}


COMMAND_BODY(Pi)
// ----------------------------------------------------------------------------
//   Return the value of constant pi
// ----------------------------------------------------------------------------
{
    if (algebraic_p pi = constant::lookup("π"))
    {
        if (Settings.NumericalConstants() || Settings.NumericalResults())
            pi = constant_p(pi)->value();
        if (pi && rt.push(pi))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(EulerianNumber)
// ----------------------------------------------------------------------------
//   Return the value of constant e
// ----------------------------------------------------------------------------
{
    if (algebraic_p e = constant::lookup("e"))
    {
        if (Settings.NumericalConstants() || Settings.NumericalResults())
            e = constant_p(e)->value();
        if (e && rt.push(e))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(Infinity)
// ----------------------------------------------------------------------------
//   Return the value of positive infinity
// ----------------------------------------------------------------------------
{
    if (object_p inf = rt.infinity(false))
        if (rt.push(inf))
            return OK;
    return ERROR;
}


COMMAND_BODY(NegativeInfinity)
// ----------------------------------------------------------------------------
//   Return the value of negative infinity
// ----------------------------------------------------------------------------
{
    if (object_p inf = rt.infinity(true))
        if (rt.push(inf))
            return OK;
    return ERROR;
}


FUNCTION_BODY(ConstantName)
// ----------------------------------------------------------------------------
//   Put the name of a constant on the stack
// ----------------------------------------------------------------------------
{
    utf8   name = nullptr;
    size_t sz   = 0;
    switch (x->type())
    {
#define CASE(c)    case ID_##c: name = c##_p(+x)->name(&sz); break
        CASE(constant);
        CASE(equation);
        CASE(xlib);
        CASE(standard_uncertainty);
        CASE(relative_uncertainty);
#undef CASE
    default: rt.type_error();
    }
    if (name)
        if (text_p sym = text::make(name, sz))
            return sym;
    return nullptr;
}


FUNCTION_BODY(ConstantValue)
// ----------------------------------------------------------------------------
//   Put the value of a constant on the stack
// ----------------------------------------------------------------------------
{
    object_p value = nullptr;
    switch (x->type())
    {
#define CASE(c)    case ID_##c: value = c##_p(+x)->value(); break
        CASE(constant);
        CASE(equation);
        CASE(xlib);
        CASE(standard_uncertainty);
        CASE(relative_uncertainty);
#undef CASE
    default:
        rt.type_error();
    }
    if (value)
        return algebraic_p(value);
    return nullptr;
}


FUNCTION_BODY(ConstantRange)
// ----------------------------------------------------------------------------
//   Put the range associated to a constant on the stack
// ----------------------------------------------------------------------------
{
    if (constant_p cst = x->as<constant>())
        if (algebraic_p value = cst->range())
            return value;
    if (!rt.error())
        rt.type_error();
    return nullptr;
}


COMMAND_BODY(Const)
// ----------------------------------------------------------------------------
//   Evaluate a library constant
// ----------------------------------------------------------------------------
{
    return constant::lookup_command(constant::constants, true);
}


COMMAND_BODY(StandardUncertainty)
// ----------------------------------------------------------------------------
//   Evaluate the standard uncertainty for a library constant
// ----------------------------------------------------------------------------
{
    return constant::lookup_command(standard_uncertainty::standard, true);
}


COMMAND_BODY(RelativeUncertainty)
// ----------------------------------------------------------------------------
//   Evaluate the relative uncertainty for a library constant
// ----------------------------------------------------------------------------
{
    return constant::lookup_command(relative_uncertainty::relative, true);
}


COMMAND_BODY(Constants)
// ----------------------------------------------------------------------------
//   Select the mathematics constants menu
// ----------------------------------------------------------------------------
{
    if (object_p menu = constant::lookup_menu(constant::constants,
                                              "Mathematics"))
        return menu->evaluate();
    rt.invalid_constant_error();
    return ERROR;
}



// ============================================================================
//
//   Constant definitions - Algebrified and in conformity with CODATA2022
//
// ============================================================================
//
//  The HP48 constants are quite outdated. A lot of progress happened in the
//  field of physics since then. This file reflects this progress.
//
//  In particular, physical constants now have uncertainty values, and
//  they are connected to one another according to the laws of physics.
//  As a result, many constants are now treated as *fundamental* that we obtain
//  from measurements, while others are *derived*, i.e. *computed* from the
//  value of measured constants. This ensures that better measurements will
//  automatically improve the value of computed constants. This process is
//  what we call "algebrified constants" in the comment above.
//

static const cstring basic_constants[] =
// ----------------------------------------------------------------------------
//   List of basic constants (including standard and relative uncertainty)
// ----------------------------------------------------------------------------
//   Revision: 2026-05-11  Jean Wilson (calculations & references)
//                         with the aid of Claude's AI
//
//   clang-format off
{
    // ------------------------------------------------------------------------
    // Mathematics
    // ------------------------------------------------------------------------
    "Mathematics",     nullptr,

    // *Pi constant - Theory approximation - Evaluated specially (decimal-pi.h)
    "π",      "[3.14159"
                "  0 "
                "  0 ]",
    // *Euler number - Theory approximation - Evaluated specially (decimal-e.h)
    "e",      "[2.71828"
                "  0 "
                "  0 ]",
    // *Imaginary unit - Exact definition
    "ⅈ",       "[0+ⅈ1"
                "  0 "
                "  0 ]",
    // *Infinity - A small numeric version of infinity
    "∞",      "[9.99999E999999"
                "  0 "
                "  0 ]",
    // *Undefined constant - Exact convention
    "?",      "[Undefined"
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    // *Imaginary unit in physics - Exact definition
    "ⅉ",       "[0+ⅈ1"
                "  0 "
                "  0 ]",
    // *One radian - Exact definition
    "rad",      "[1_r"
                "  0_r "
                "  0 ]",
    // *Two pi radians - Exact definition
    "twoπ",     "['2*Ⓒπ'_r"
                "  0_r "
                "  0 ]",
    // *Half turn (180°) - Exact definition
    "angl",     "[ 180_°"
                "  0_° "
                "  0 ]",
    // *Minus infinity - A small numeric version of minus infinity
    "−∞",    "[-9.99999E999999"
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    // *Tau (alias for twoπ) - The circle constant
    "τ",        "['2*Ⓒπ'_r"
                "  0_r "
                "  0 ]",
    // *Euler-Mascheroni constant - Theory approximation
    "γ",       "[ 0.5772156649015328606065120900824024 "
                "  0 "
                "  0 ]",
    // *Golden ratio - Exact calculation
    "ϕ",       "[ 1.618033988749894848204586834365638 "
                "  0 "
                "  0 ]",
    // *Catalan's constant - Theory approximation
    "Gⓒ",      "[ 9.159655941772190150546035149323841E-1 "
                "  0 "
                "  0 ]",
    // *Khinchin's constant - Theory approximation
    "K₀",      "[ 2.685452001065306445309714835481796 "
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    // *Feigenbaum alpha - Theory approximation
    "Fα",      "[ 2.502907875095892822283902873218216 "
                "  0 "
                "  0 ]",
    // *Feigenbaum delta - Theory approximation
    "Fδ",      "[ 4.669201609102990671853203820466202 "
                "  0 "
                "  0 ]",
    // *Silver ratio - Exact calculation
    "δₛ",      "[ 2.414213562373095048801688724209698 "
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    //   Chemistry
    // ------------------------------------------------------------------------

    "Chemistry",     nullptr,

    // *Avogadro's number - Exact definition [0]
    "NA",       "[ 6.02214076E23_mol⁻¹ "
                "  0_mol⁻¹ "
                "  0 ]",
    // *Boltzmann constant - Exact definition [0]
    "k",        "[ 1.380649E-23_J/K "
                "  0_J/K "
                "  0 ]",
    // *Molar volume - Exact calculation [0]
    "Vm",       "[ 'CONVERT(ⒸR*ⒸStdT/ⒸStdP;1_m^3/mol)' "
                "  0_m^3/mol "
                "  0 "
                "  2.241E-2_m^3/mol ]",
    // *Universal gas constant - Exact calculation [0]
    "R",        "[ 'CONVERT(ⒸNA*Ⓒk;1_J/(mol*K))' "
                "  0_J/(mol*K) "
                "  0 "
                "  8.314_J/(mol·K) ]",
    // *Faraday constant - Exact calculation [0]
    "F",        "[ 'CONVERT(ⒸNA*Ⓒqe;1_C/mol)' "
                "  0_C/mol "
                "  0 "
                "  96485._C/mol ]",

    // ------------------------------------------------------------------------
    // *Standard temperature - Definition convention
    "StdT",     "[ 273.15_K "
                "  0_K "
                "  0 ]",
    // *Standard pressure - Definition convention
    "StdP",     "[ 101.325_kPa "
                "  0_kPa "
                "  0 ]",
    // *Molar mass constant - Calculation from measurement [0]
    "Mu",       "[ 'ROUND(CONVERT(ⒸNA*Ⓒu;1_g/mol);XPON(ⓇMu*ⒸNA*Ⓒu)-XPON(ⒸNA*Ⓒu)-2)' "
                "  'ROUND(CONVERT(ⓇMu*ⒸMu;1_g/mol);-2)' "
                "  'Ⓡu' "
                "  1_g/mol ]",
    // *C12 molar mass - Calculation from measurement [0]
    "MC12",     "[ 'ROUND(CONVERT(12*ⒸMu;1_g/mol);XPON(UVAL(ⓇMC12*12*ⒸMu))-XPON(UVAL(12*ⒸMu))-2)' "
                "  'ROUND(CONVERT(ⓇMC12*ⒸMC12;1_g/mol);-2)' "
                "  'ⓇMu' "
                "  12_g/mol ]",
    // *Mass unit (Dalton) - Calculation from measurement [0]
    "Da",       "[ 'Ⓒu' "
                "  'Ⓢu' "
                "  'Ⓡu' "
                "  1.661E-27_kg ]",

    // ------------------------------------------------------------------------
    // *Loschmidt constant - Exact calculation [0]
    "n0",       "[ 'CONVERT(ⒸNA/ⒸVm;1_m^-3)' "
                "  0_m^-3 "
                "  0 "
                "  2.687E25_(m↑3)⁻¹ ]",
    // *Sackur-Tetrode constant - Calculation from measurement [0]
    "SoR",      "[ 'ROUND((5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP));XPON(ⓇSoR*(5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP)))-XPON((5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP)))-2)' "
                "  'ROUND(ⓇSoR*ABS(ⒸSoR);-2)' "
                "  4.0E-10 "
                "  -1.165 ]",
    // *kq ratio - Exact calculation [0]
    "kq",       "[ 'CONVERT(Ⓒk/Ⓒqe;1_J/(K*C))' "
                "  0_J/(K*C) "
                "  0 "
                "  8.617E-5_J/(K·C) ]",

    // ------------------------------------------------------------------------
    //   Physics
    // ------------------------------------------------------------------------

    "Physics",      nullptr,

    // *Imaginary unit in physics - Exact definition
    "ⅉ",       "[0+ⅈ1"
                "  0 "
                "  0 ]",
    // *Speed of light - Exact definition [0]
    "c",        "[ 299792458_m/s "
                "  0_m/s "
                "  0 ]",
    // *Gravitation constant - Measurement [0]
    "G",        "[ 6.67430E-11_m^3/(s²*kg) "
                "  0.00015E-11_m^3/(s²*kg) "
                "  'ROUND(UBASE(ⓈG/ⒸG);-2)' ]",
    // *Acceleration of Earth gravity - Definition convention
    "g",        "[ 9.80665_m/s² "
                "  0_m/s² "
                "  0 ]",
    // *Stefan-Boltzmann constant - Exact calculation [0]
    "σ",        "[ 'CONVERT(Ⓒπ²/60*Ⓒk^4/(Ⓒℏ^3*Ⓒc²);1_W/(m²*K^4))' "
                "  0_W/(m²*K^4) "
                "  0 "
                "  5.670E-8_W/(m↑2·K↑4) ]",

    // ------------------------------------------------------------------------
    // *Vacuum permittivity - Calculation from measurement [0]
    "ε₀",       "[ 'ROUND(CONVERT(1/(Ⓒμ₀*Ⓒc²);1_F/m);XPON(UVAL(Ⓡε₀/(Ⓒμ₀*Ⓒc²)))-XPON(UVAL(1/(Ⓒμ₀*Ⓒc²)))-2)' "
                "  'Convert(ROUND(UBASE(Ⓡε₀*Ⓒε₀);-2); 1_F/m)' "
                "  'Ⓡμ₀' "
                "  8.854E-12_F/m ]",
    // *Vacuum permeability - Calculation from measurement [0]
    "μ₀",       "[ 'ROUND(CONVERT(4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc);1_H/m);XPON(UVAL(Ⓡμ₀*4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc)))-XPON(UVAL(4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡμ₀*Ⓒμ₀);-2);1_H/m)' "
                "  'Ⓡα' "
                "  1.257E-6_H/m ]",
    // *Coulomb constant - Calculation from measurement [0]
    "ke",      "[ 'ROUND(CONVERT(1/(4*Ⓒπ*Ⓒε₀);1_(N*(m/C)²));XPON(UVAL(Ⓡke/(4*Ⓒπ*Ⓒε₀)))-XPON(UVAL(1/(4*Ⓒπ*Ⓒε₀)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡke*Ⓒke);-2);1_(N*(m/C)²))' "
                "  'Ⓡε₀' "
                "  8.988E9_N·m↑2/C↑2 ]",
    // *Vacuum characteristic impedance - Calculation from measurement [0]
    "Z₀",       "[ 'ROUND(CONVERT(Ⓒμ₀*Ⓒc;1_Ω);XPON(UVAL(ⓇZ₀*Ⓒμ₀*Ⓒc))-XPON(UVAL(Ⓒμ₀*Ⓒc))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇZ₀*ⒸZ₀);-2);1_Ω)' "
                "  'Ⓡμ₀' "
                "  376.7_Ω ]",
    // *ε₀q ratio - Calculation from measurement [0]
    "ε₀q",      "[ 'ROUND(CONVERT(Ⓒε₀/Ⓒqe;1_F/(m*C));XPON(UVAL(Ⓡε₀q*Ⓒε₀/Ⓒqe))-XPON(UVAL(Ⓒε₀/Ⓒqe))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡε₀q*Ⓒε₀q);-2);1_F/(m*C))' "
                "  'Ⓡε₀' "
                "  5.526E7_F/(m·C) ]",

    // ------------------------------------------------------------------------
    // *qε₀ product - Calculation from measurement [0]
    "qε₀",      "[ 'ROUND(CONVERT(Ⓒqe*Ⓒε₀;1_F*C/m);XPON(UVAL(Ⓡqε₀*Ⓒqe*Ⓒε₀))-XPON(UVAL(Ⓒqe*Ⓒε₀))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡqε₀*Ⓒqε₀);-2);1_F*C/m)' "
                "  'Ⓡε₀' "
                "  1.419E-30_F·C/m ]",
    // *Ref intensity - Definition convention
    "I₀",       "[ 0.000000000001_W/m² "
                "  0_W/m² "
                "  0 ]",


    // ------------------------------------------------------------------------
    //   Electromagnetism
    // ------------------------------------------------------------------------

    "Electromagnetism",     nullptr,

    // *Electronic charge - Exact definition [0]
    "qe",       "[ 1.602176634E-19_C "
                "  0_C "
                "  0 ]",
    // *Rydberg constant - Measurement [0]
    "R∞",       "[ 10973731.568157_m⁻¹ "
                "  0.000012_m⁻¹ "
                "  'ROUND(UBASE(ⓈR∞/ⒸR∞);-2)' "
                "  1.097E7_m⁻¹ ]",
    // *Photon wavelength - Exact calculation [0]
    "λ₀",       "[ 'CONVERT(Ⓒh*Ⓒc/Ⓒqe/(1_V);1_nm)' "
                "  0_nm "
                "  0 ]",
    // *Photon frequency - Exact calculation [0]
    "f₀",       "[ 'CONVERT(Ⓒc/Ⓒλ₀;1_Hz)' "
                "  0_Hz "
                "  0 "
                "  2.418E14_Hz ]",
    // *Electron g-factor - Measurement [0]
    "ge",       "[ -2.00231930436092 "
                "  0.00000000000036 "
                "  'ROUND(UBASE(ABS(Ⓢge/Ⓒge));-2)' ]",

    // ------------------------------------------------------------------------
    // *qme ratio - Calculation from measurement [0]
    "qme",      "[ 'ROUND(CONVERT(Ⓒqe/Ⓒme;1_C/kg);XPON(UVAL(Ⓡqme*Ⓒqe/Ⓒme))-XPON(UVAL(Ⓒqe/Ⓒme))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡqme*Ⓒqme);-2);1_C/kg)' "
                "  'Ⓡme' "
                "  1.759E11_C/kg ]",
    // *Electron magnetic moment - Measurement [0]
    "μe",       "[ -9.2847646917E-24_J/T "
                "  0.0000000029E-24_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμe/Ⓒμe));-2)' ]",
    // *Electron magnetic moment / Bohr magneton - Measurement [0]
    "μeμB",     "[ -1.00115965218046 "
                "  0.00000000000011 "
                "  'ROUND(UBASE(ABS(Ⓢμe/Ⓒμe));-2)' ]",
    // *Proton magnetic moment - Measurement [0]
    "μp",       "[ 1.41060679545E-26_J/T "
                "  0.00000000060E-26_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμp/Ⓒμp));-2)' ]",
    // *Neutron magnetic moment - Measurement [0]
    "μn",       "[ -9.6623653E-27_J/T "
                "  0.0000023E-27_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμn/Ⓒμn));-2)' ]",
    // *Muon magnetic moment - Measurement [0]
    "μμ",       "[ -4.49044830E-26_J/T "
                "  0.00000010E-26_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμμ/Ⓒμμ));-2)' ]",

    // ------------------------------------------------------------------------
    // *Bohr magneton - Calculation from measurement [0]
    "μB",       "[ 'ROUND(CONVERT(Ⓒqe*Ⓒℏ/(2*Ⓒme);1_J/T);XPON(UVAL(ⓇμB*Ⓒqe*Ⓒℏ/(2*Ⓒme)))-XPON(UVAL(Ⓒqe*Ⓒℏ/(2*Ⓒme)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇμB*ⒸμB);-2);1_J/T)' "
                "  'Ⓡme' "
                "  9.274E-24_J/T ]",
    // *Nuclear magneton - Calculation from measurement [0]
    "μN",       "[ 'ROUND(CONVERT(Ⓒqe*Ⓒℏ/(2*Ⓒmp);1_J/T);XPON(UVAL(ⓇμN*Ⓒqe*Ⓒℏ/(2*Ⓒmp)))-XPON(UVAL(Ⓒqe*Ⓒℏ/(2*Ⓒmp)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇμN*ⒸμN);-2);1_J/T)' "
                "  'Ⓡmp' "
                "  5.051E-27_J/T ]",
    // *Electron gyromagnetic ratio - Calculation from measurement [0]
    "γe",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμe)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγe*2*ABS(Ⓒμe)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμe)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγe*Ⓒγe);-2);1_(s*T)^-1)' "
                "  'Ⓡμe' "
                "  1.761E11_(s·T)⁻¹ ]",
    // *Proton gyromagnetic ratio - Calculation from measurement [0]
    "γp",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμp)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγp*2*ABS(Ⓒμp)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμp)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγp*Ⓒγp);-2);1_(s*T)^-1)' "
                "  'Ⓡμp' "
                "  2.675E8_(s·T)⁻¹ ]",
    // *Neutron gyromagnetic ratio - Calculation from measurement [0]
    "γn",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμn)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγn*2*ABS(Ⓒμn)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμn)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγn*Ⓒγn);-2);1_(s*T)^-1)' "
                "  'Ⓡμn' "
                "  1.832E8_(s·T)⁻¹ ]",

    // ------------------------------------------------------------------------
    // *First radiation constant - Exact calculation [0]
    "c1",       "[ 'CONVERT(2*Ⓒπ*Ⓒh*Ⓒc²;1_(W*m²))' "
                "  0_(W*m²) "
                "  0 "
                "  3.742E-16_W·m↑2 ]",
    // *Second radiation constant - Exact calculation [0]
    "c2",       "[ 'CONVERT(Ⓒh*Ⓒc/Ⓒk;1_(m*K))' "
                "  0_(m*K) "
                "  0 "
                "  0.01439_m·K ]",
    // *Wien's wavelength constant - Theory approximation [0]
    "c3",       "[ 2.897771955185172661478605448092885_mm*K "
                "  0_mm*K "
                "  0 ]",
    // *Wien's frequency constant - Theory approximation [0]
    "c3f",      "[ 0.05878925757646824946606130795309722_THz/K "
                "  0_THz/K "
                "  0 ]",
    // *von Klitzing constant - Exact calculation [0]
    "Rk",       "[ 'CONVERT(2*Ⓒπ*Ⓒℏ/Ⓒqe²;1_Ω)' "
                "  0_Ω "
                "  0 "
                "  25813._Ω ]",

    // ------------------------------------------------------------------------
    // *Conductance quantum - Exact calculation [0]
    "G0",       "[ 'CONVERT(Ⓒqe²/(Ⓒπ*Ⓒℏ);1_S)' "
                "  0_S "
                "  0 "
                "  7.748E-5_S ]",
    // *Fermi reduced coupling constant - Measurement [0]
    "G0F",       "[ 1.1663787E-5_GeV^-2 "
                "  0.0000006E-5_GeV^-2 "
                "  'ROUND(UBASE(ABS(ⓈG0F/ⒸG0F));-2)' ]",
    // *Magnetic flux quantum - Exact calculation [0]
    "ø",        "[ 'CONVERT(Ⓒπ*Ⓒℏ/Ⓒqe;1_Wb)' "
                "  0_Wb "
                "  0 "
                "  2.068E-15_Wb ]",
    // *Josephson constant - Exact calculation [0]
    "KJ",       "[ 'CONVERT(2*Ⓒqe/Ⓒh;1_Hz/V)' "
                "  0_Hz/V "
                "  0 "
                "  4.836E14_Hz/V ]",
    // *Quantum of circulation - Calculation from measurement [0]
    "Kc",       "[ 'ROUND(CONVERT(Ⓒπ*Ⓒℏ/Ⓒme;1_m²/s);XPON(UVAL(ⓇKc*Ⓒπ*Ⓒℏ/Ⓒme))-XPON(UVAL(Ⓒπ*Ⓒℏ/Ⓒme))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇKc*ⒸKc);-2);1_m²/s)' "
                "  'Ⓡme' "
                "  3.637E-4_m↑2/s ]",

    // ------------------------------------------------------------------------
    //   Quantum mechanics
    // ------------------------------------------------------------------------

    "Quantum",    nullptr,

    // *Planck constant - Exact definition [0]
    "h",        "[ 6.62607015E-34_J*s "
                "  0_J*s "
                "  0 ]",
    // *Dirac constant - Exact definition [0]
    "ℏ",        "[ 'CONVERT(Ⓒh/(2*Ⓒπ);1_J*s)' "
                "  0_J*s "
                "  0 "
                "  1.055E-34_J·s ]",
    // *Fine structure constant - Measurement [0]
    "α",        "[ 0.00729735256434 "
                "  0.00000000000114 "
                "  'ROUND(UBASE(Ⓢα/Ⓒα);-2)' ]",
    // *Cs hyperfine transition frequency - Exact definition [0]
    "ΔfCs",     "[ 9192631770_Hz "
                "  0_Hz "
                "  0 ]",
    // *Weak mixing angle - Calculation from measurement [0]
    "θw",       "[ 'ROUND(CONVERT(ASIN(√(0.22305));1_°);XPON(UVAL(Ⓡθw*ASIN(√(0.22305))))-XPON(UVAL(ASIN(√(0.22305))))-2)' "
                   "'CONVERT(ROUND((ASIN(√(0.22305+0.00023))-ASIN(√(0.22305-0.00023)))/2;-2);1_°)' "
                "  'ROUND(UBASE(ABS(Ⓢθw/CONVERT(ASIN(√(0.22305));1_°)));-2)' "
                "  28.18_° ]",

    // ------------------------------------------------------------------------
    // *Planck length - Calculation from measurement [0]
    "Lpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*ⒸG/Ⓒc^3);1_m);XPON(UVAL(ⓇLpl*√(Ⓒℏ*ⒸG/Ⓒc^3)))-XPON(UVAL(√(Ⓒℏ*ⒸG/Ⓒc^3)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇLpl*ⒸLpl);-2);1_m)' "
                "  'ⓇG/2' "
                "  1.616E-35_m ]",
    // *Planck time - Calculation from measurement [0]
    "Tpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*ⒸG/Ⓒc^5);1_s);XPON(UVAL(ⓇTpl*√(Ⓒℏ*ⒸG/Ⓒc^5)))-XPON(UVAL(√(Ⓒℏ*ⒸG/Ⓒc^5)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇTpl*ⒸTpl);-2);1_s)' "
                "  'ⓇG/2' "
                "  5.391E-44_s ]",
    // *Planck mass - Calculation from measurement [0]
    "Mpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*Ⓒc/ⒸG);1_kg);XPON(UVAL(ⓇMpl*√(Ⓒℏ*Ⓒc/ⒸG)))-XPON(UVAL(√(Ⓒℏ*Ⓒc/ⒸG)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇMpl*ⒸMpl);-2);1_kg)' "
                "  'ⓇG/2' "
                "  2.176E-8_kg ]",
    // *Planck energy - Calculation from measurement [0]
    "Epl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*Ⓒc^5/ⒸG);1_GeV);XPON(UVAL(ⓇEpl*√(Ⓒℏ*Ⓒc^5/ⒸG)))-XPON(UVAL(√(Ⓒℏ*Ⓒc^5/ⒸG)))-2)' "
                "  'ROUND(CONVERT(ROUND(UBASE(ⓇEpl*ⒸEpl);-2);1_GeV);-2)' "
                "  'ⓇG/2' "
                "  1.221E19_GeV ]",
    // *Planck temperature - Calculation from measurement [0]
    "T°pl",     "[ 'ROUND(CONVERT(√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk;1_K);XPON(UVAL(ⓇT°pl*√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk))-XPON(UVAL(√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇT°pl*ⒸT°pl);-2);1_K)' "
                "  'ⓇG/2' "
                "  1.417E32_K ]",

    // ------------------------------------------------------------------------
    // *Planck charge - Calculation from measurement [0]
    "qpl",     "[ 'ROUND(CONVERT(√(4*Ⓒπ*Ⓒε₀*Ⓒℏ*Ⓒc);1_C);XPON(UVAL(Ⓡqpl*√(4*Ⓒπ*Ⓒε₀*Ⓒℏ*Ⓒc)))-XPON(UVAL(√(4*Ⓒπ*Ⓒε₀*Ⓒℏ*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡqpl*Ⓒqpl);-2);1_C)' "
                "  'Ⓡε₀/2' "
                "  1.876E-18_C ]",
    // *Planck momentum - Calculation from measurement [0]
    "ppl",     "[ 'ROUND(CONVERT(√(Ⓒℏ*Ⓒc↑3/ⒸG);1_kg·m/s);XPON(UVAL(Ⓡppl*√(Ⓒℏ*Ⓒc↑3/ⒸG)))-XPON(UVAL(√(Ⓒℏ*Ⓒc↑3/ⒸG)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡppl*Ⓒppl);-2);1_kg·m/s)' "
                "  'ⓇG/2' "
                "  6.525_kg·m/s ]",
    // *Planck force - Calculation from measurement [0]
    "Fpl",     "[ 'ROUND(CONVERT(Ⓒc↑4/ⒸG;1_N);XPON(UVAL(Ⓡppl*Ⓒc↑4/ⒸG))-XPON(UVAL(Ⓒc↑4/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇFpl*ⒸFpl);-2);1_N)' "
                "  'ⓇG' "
                "  1.210E44_N ]",
    // *Planck density - Calculation from measurement [0]
    "ρvpl",     "[ 'ROUND(CONVERT(Ⓒc↑5/(Ⓒℏ*ⒸG²);1_kg/m³);XPON(UVAL(Ⓡρvpl*Ⓒc↑5/(Ⓒℏ*ⒸG²)))-XPON(UVAL(Ⓒc↑5/(Ⓒℏ*ⒸG²)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρvpl*Ⓒρvpl);-2);1_kg/m³)' "
                "  'ⓇG*2' "
                "  5.155E96_kg/m³ ]",
    // *Hartree energy - Calculation from measurement [0]
    "Eh",       "[ 'ROUND(CONVERT(2*Ⓒh*Ⓒc*ⒸR∞;1_J);XPON(UVAL(ⓇEh*2*Ⓒh*Ⓒc*ⒸR∞))-XPON(UVAL(2*Ⓒh*Ⓒc*ⒸR∞))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇEh*ⒸEh);-2);1_J)' "
                "  'ⓇR∞' "
                "  4.360E-18_J ]",

    // ------------------------------------------------------------------------
    //   Particle mass constants
    // ------------------------------------------------------------------------

    "Mass",     nullptr,

    // *Mass unit (u) - Calculation from measurement [0]
    "u",        "[ 'ROUND(CONVERT(Ⓒme/ⒸAre;1_kg);XPON(UVAL(Ⓡu*Ⓒme/ⒸAre))-XPON(UVAL(Ⓒme/ⒸAre))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡu*Ⓒu);-2);1_kg)' "
                "  3.1E-10 "
                "  1.661E-27_kg ]",
    // *Energy equivalent of atomic mass unit - Exact calculation [0]
    "uc²",      "[ 'CONVERT(Ⓒu*Ⓒc²;1_J)' "
                "  0_J "
                "  0 "
                "  1.492E-10_J ]",
    // *Electron mass - Calculation from measurement [0]
    "me",       "[ 'ROUND(CONVERT(2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc);1_kg);XPON(UVAL(Ⓡme*2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc)))-XPON(UVAL(2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡme*Ⓒme);-2);1_kg)' "
                "  'Ⓡu' "
                "  9.109E-31_kg ]",
    // *Neutron mass - Measurement [0]
    "mn",       "[ 1.67492750056E-27_kg "
                "  0.00000000085E-27_kg "
                "  'ROUND(UBASE(Ⓢmn/Ⓒmn);-2)' "
                "  1.675E-27_kg ]",
    // *Proton mass - Measurement [0]
    "mp",       "[ 1.67262192595E-27_kg "
                "  0.00000000052E-27_kg "
                "  'ROUND(UBASE(Ⓢmp/Ⓒmp);-2)' "
                "  1.673E-27_kg ]",
    // *Hydrogen mass - Measurement [0]
    "mH",       "[ 1.00782503223_u "
                "  0.00000000009_u "
                "  'ROUND(UBASE(ⓈmH/ⒸmH);-2)' "
                "  1.008_u ]",

    // ------------------------------------------------------------------------
    // *Deuterium mass - Measurement [0]
    "mD",       "[ 2.01410177812_u "
                "  0.00000000012_u "
                "  'ROUND(UBASE(ⓈmD/ⒸmD);-2)' "
                "  2.014_u ]",
    // *Tritium mass - Measurement [0]
    "mT",       "[ 3.0160492779_u "
                "  0.0000000024_u "
                "  'ROUND(UBASE(ⓈmT/ⒸmT);-2)' "
                "  3.016_u ]",
    // *Helium atomic mass - Measurement [0]
    "mHe",      "[ 4.00260325413_u "
                "  0.00000000006_u "
                "  'ROUND(UBASE(ⓈmHe/ⒸmHe);-2)' "
                "  4.003_u ]",
    // *Muon mass - Measurement [0]
    "mμ",       "[ 0.1134289257_u "
                "  0.0000000025_u "
                "  'ROUND(UBASE(Ⓢmμ/Ⓒmμ);-2)' "
                "  0.1134_u ]",
    // *Tau mass - Measurement [0]
    "mτ",       "[ 1.90754_u "
                "  0.00013_u "
                "  'ROUND(UBASE(Ⓢmτ/Ⓒmτ);-2)' "
                "  1.908_u ]",

    // ------------------------------------------------------------------------
    // *mpme ratio - Measurement [0]
    "mpme",     "[ 1836.152673426 "
                "  0.000000032 "
                "  'ROUND(UBASE(Ⓢmpme/Ⓒmpme);-2)' "
                "  1836. ]",
    // *mnmp ratio - Neutron/proton mass ratio - Measurement [0]
    "mnmp",     "[ 1.00137841946 "
                "  0.00000000051 "
                "  'ROUND(UBASE(Ⓢmn/Ⓒmn+Ⓢmp/Ⓒmp);-2)' "
                "  1.001 ]",
    // *Electron relative atomic mass - Measurement [0]
    "Are",       "[ 5.485799090441E-4 "
                "  0.000000000097E-4 "
                "  'ROUND(ⓈAre/ⒸAre;-2)' "
                "  5.486E-4 ]",
    // *Top quark mass - Measurement [2]
    "mQtop",       "[ 172.52_GeV "
                "  0.33_GeV "
                "  'ROUND(ⓈmQtop/ⒸmQtop;-2)' "
                "  173._GeV ]",
    // *Bottom quark mass - Measurement [2]
    "mQbot",       "[ 4.183_GeV "
                "  0.007_GeV "
                "  'ROUND(ⓈmQbot/ⒸmQbot;-2)' "
                "  4.183_GeV ]",
    // *Charm quark mass - Measurement [2]
    "mQcha",       "[ 1.2730_GeV "
                "  0.0046_GeV "
                "  'ROUND(ⓈmQcha/ⒸmQcha;-2)' "
                "  1.273_GeV ]",

    // ------------------------------------------------------------------------
    // *Strange quark mass - Measurement [2]
    "mQstr",       "[ 93.5_MeV "
                "  0.8_MeV "
                "  'ROUND(ⓈmQstr/ⒸmQstr;-2)' ]",
    // *Down quark mass - Measurement [2]
    "mQdwn",       "[ 4.70_MeV "
                "  0.07_MeV "
                "  'ROUND(ⓈmQdwn/ⒸmQdwn;-2)' ]",
    // *Up quark mass - Measurement [2]
    "mQup",       "[ 2.16_MeV "
                "  0.07_MeV "
                "  'ROUND(ⓈmQup/ⒸmQup;-2)' ]",
    // *Maximum neutrino mass - Measurement [2]
    "mνmax",       "[ 0.45_eV/c² "
                "  0_eV/c² "
                "  0 ]",
    // *Minimum neutrino mass - Measurement [2]
    "mνmin",       "[ 0.001_eV/c² "
                "  0_eV/c² "
                "  0 ]",

    // ------------------------------------------------------------------------
    // *Higgs boson mass - Measurement [15]
    "mHig",       "[ 125.11_GeV "
                "  0.11_GeV "
                "  'ROUND(ⓈmHig/ⒸmHig;-2)' "
                "  125._GeV ]",
    // *W boson mass - Measurement [16]
    "mW",       "[ 80.3602_GeV "
                "  0.0099_GeV "
                "  'ROUND(ⓈmW/ⒸmW;-2)' "
                "  80.36_GeV ]",
    // *Z° boson mass - Measurement [17]
    "mZ°",       "[ 91.1876_GeV "
                "  0.0021_GeV "
                "  'ROUND(ⓈmZ°/ⒸmZ°;-2)' "
                "  91.19_GeV ]",


    // ------------------------------------------------------------------------
    //   Particle sizes
    // ------------------------------------------------------------------------

    "Size",     nullptr,

    // *Classical electron radius - Calculation from measurement [0]
    "re",       "[ 'ROUND(CONVERT(Ⓒα^2*Ⓒa0; 1_fm); XPON(Ⓡre*Ⓒα^2*Ⓒa0) - XPON(Ⓒα^2*Ⓒa0) - 2)' "
                "  'ROUND(Ⓡre*Ⓒre; -2)' "
                "  'ROUND(3*Ⓢα/Ⓒα; -2)' "
                "  2.818_fm ]",
    // *Proton charge radius - Measurement [0]
    "rp",       "[ 8.4075E-16_m "
                "  0.0064E-16_m "
                "  'ROUND(Ⓢrp/Ⓒrp;-2)' "
                "  0.841_fm ]",
    // *Bohr radius - Calculation from measurement [0]
    "a0",       "[ 'ROUND(CONVERT(4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²);1_nm);XPON(UVAL(Ⓡa0*4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²)))-XPON(UVAL(4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓢα/Ⓒα*Ⓒa0);-2);1_nm)' "
                "  'Ⓡα' "
                "  5.292E-2_nm ]",

    // ------------------------------------------------------------------------
    //    Compton effect
    // ------------------------------------------------------------------------

    "Scattering",   nullptr,

    // *Thomson cross-section - Calculation from measurement [0]
    "σe",       "[ 'ROUND(CONVERT(8*Ⓒπ*Ⓒre²/3;1_m²);XPON(UVAL(Ⓡσe*8*Ⓒπ*Ⓒre²/3))-XPON(UVAL(8*Ⓒπ*Ⓒre²/3))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡσe*Ⓒσe);-2);1_m²)' "
                "  'ROUND(6*Ⓢα/Ⓒα;-2)' "
                "  6.652E-29_m² ]",
    // *Electron Compton wavelength - Calculation from measurement [0]
    "λc",       "[ 'ROUND(CONVERT(Ⓒh/(Ⓒme*Ⓒc);1_nm);XPON(UVAL(Ⓡλc*Ⓒh/(Ⓒme*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒme*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλc*Ⓒλc);-2);1_nm)' "
                "  'Ⓡme' "
                "  2.426E-3_nm ]",
    // *Proton Compton wavelength - Calculation from measurement [0]
    "λcp",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmp*Ⓒc);1_nm);XPON(UVAL(Ⓡλcp*Ⓒh/(Ⓒmp*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmp*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcp*Ⓒλcp);-2);1_nm)' "
                "  'Ⓡmp' "
                "  1.321E-6_nm ]",
    // *Neutron Compton wavelength - Calculation from measurement [0]
    "λcn",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmn*Ⓒc);1_nm);XPON(UVAL(Ⓡλcn*Ⓒh/(Ⓒmn*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmn*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcn*Ⓒλcn);-2);1_nm)' "
                "  'Ⓡmn' "
                "  1.320E-6_nm ]",
    // *Muon Compton wavelength - Calculation from measurement [0]
    "λcμ",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmμ*Ⓒc);1_nm);XPON(UVAL(Ⓡλcμ*Ⓒh/(Ⓒmμ*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmμ*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcμ*Ⓒλcμ);-2);1_nm)' "
                "  'Ⓡmμ' "
                "  1.173E-5_nm ]",
    // ------------------------------------------------------------------------
    // *Tau Compton wavelength - Calculation from measurement [0]
    "λcτ",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmτ*Ⓒc);1_nm);XPON(UVAL(Ⓡλcτ*Ⓒh/(Ⓒmτ*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmτ*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcτ*Ⓒλcτ);-2);1_nm)' "
                "  'Ⓡmτ' "
                "  6.978E-7_nm ]",


    // ------------------------------------------------------------------------
    //  Astronomy & Astrophysics
    // ------------------------------------------------------------------------

    "Astronomy",      nullptr,

    "Astronomy/Mercury",     nullptr,

    // *Mercury gravitational parameter - Measurement [4]
    "GM☿",      "[ 2.2031868551E13_m³/s² "
                "  0.0000000091E13_m³/s² "
                "  'ROUND(ⓈGM☿/ⒸGM☿;-2)' "
                "  2.203E13_m³/s² ]",
    // *Mercury mass - Calculation from GM and G
    "M☿",       "[ 'ROUND(CONVERT(ⒸGM☿/ⒸG;1_kg);XPON(UVAL(ⒸGM☿/ⒸG·ⓇG))-XPON(UVAL(ⒸGM☿/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM☿*ⒸM☿);-2);1_kg)' "
                "  'ⓇG' "
                "  3.301E23_kg ]",
    // *Mercury equatorial radius - Measurement [22]
    "Req☿",     "[ 2439.7_km "
                "  0.1_km "
                "  'ROUND(ⓈReq☿/ⒸReq☿;-2)' "
                "  2440._km ]",
    // *Mercury polar radius - Measurement [22]
    "Rp☿",      "[ 2439.7_km "
                "  0.1_km "
                "  'ROUND(ⓈRp☿/ⒸRp☿;-2)' "
                "  2440._km ]",
    // *Mercury oblateness - Calculation from measurement [22]
    "f☿",       "[ 'ROUND(1-ⒸRp☿/ⒸReq☿;XPON(UVAL(Ⓡf☿*(1-ⒸRp☿/ⒸReq☿)))-XPON(1-ⒸRp☿/ⒸReq☿)-2)' "
                "  'ROUND(Ⓡf☿*Ⓒf☿;-2)' "
                "  'ⓇRp☿+ⓇReq☿' ]",
    // *Mercury mean density - Calculation from measurement [4] [22]
    "ρv☿",       "[ 'ROUND(CONVERT(ⒸGM☿/(ⒸG·4/3·Ⓒπ·ⒸReq☿²·ⒸRp☿);1_kg/m³);XPON(UVAL(Ⓡρv☿*ⒸGM☿/(ⒸG·4/3·Ⓒπ·ⒸReq☿²·ⒸRp☿)))-XPON(UVAL(ⒸGM☿/(ⒸG·4/3·Ⓒπ·ⒸReq☿²·ⒸRp☿)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv☿*Ⓒρv☿);-2);1_kg/m³)' "
                "  'ⓇGM☿+ⓇG+2·ⓇReq☿+ⓇRp☿' ]",

    // ------------------------------------------------------------------------
    // *Mercury surface gravity - Calculation from measurement [4] [22]
    "g☿",       "[ 'ROUND(CONVERT(ⒸGM☿/ⒸReq☿²;1_m/s²);XPON(UVAL(Ⓡg☿*ⒸGM☿/ⒸReq☿²))-XPON(UVAL(ⒸGM☿/ⒸReq☿²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg☿*Ⓒg☿);-2);1_m/s²)' "
                "  'ⓇGM☿+2·ⓇReq☿' ]",
    // *Mercury escape velocity - Calculation from measurement [4] [22]
    "Ve☿",      "[ 'ROUND(CONVERT(√(2·ⒸGM☿/(ⒸReq☿²·ⒸRp☿)↑(1/3));1_m/s);XPON(UVAL(ⓇVe☿*√(2·ⒸGM☿/(ⒸReq☿²·ⒸRp☿)↑(1/3))))-XPON(UVAL(√(2·ⒸGM☿/(ⒸReq☿²·ⒸRp☿)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe☿*ⒸVe☿);-2);1_m/s)' "
                "  0.02 ]",
    // *Mercury sidereal rotation period - Measurement [22]
    "Prot☿",    "[ 5067014.4_s "
                "  0.1_s "
                "  'ROUND(ⓈProt☿/ⒸProt☿;-2)' "
                "  5.067E6_s ]",
    // *Mercury orbital period - Computed [27]
    "Porb☿",    "[ 7600551.8_s "
                "  0_s "
                "  0 ]",
    // *Mercury rotation angular velocity - Calculation from Prot
    "ωrot☿",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt☿;1_r/s);XPON(UVAL(Ⓡωrot☿*(2*Ⓒπ*1_r)/ⒸProt☿))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt☿))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot☿*(2*Ⓒπ*1_r)/ⒸProt☿;-2);1_r/s)' "
                "  'ⓇProt☿' ]",
    // *Mercury axial tilt - Measurement [22]
    "ϵ☿",       "[ 0.034_° "
                "  0.001_° "
                "  'ROUND(Ⓢϵ☿/Ⓒϵ☿;-2)' ]",
    // *Mercury semi-major axis - Measurement [23]
    "a☿",       "[ 57909050_km "
                "  10_km "
                "  'ROUND(Ⓢa☿/Ⓒa☿;-2)' "
                "  5.791E7_km ]",

    // ------------------------------------------------------------------------
    // *Mercury orbital eccentricity - Measurement [23]
    "e☿",       "[ 0.205630 "
                "  0.000001 "
                "  'ROUND(Ⓢe☿/Ⓒe☿;-2)' "
                "  0.2056 ]",
    // *Mercury orbital inclination - Measurement [23]
    "i☿",       "[ 7.005_° "
                "  0.001_° "
                "  'ROUND(Ⓢi☿/Ⓒi☿;-2)' ]",
    // *Mercury longitude of ascending node - Measurement [23]
    "Ω☿",       "[ 48.331_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ☿/ⒸΩ☿;-2)' "
                "  48.33_° ]",
    // *Mercury argument of perihelion - Measurement [23]
    "ω☿",       "[ 29.124_° "
                "  0.001_° "
                "  'ROUND(Ⓢω☿/Ⓒω☿;-2)' "
                "  29.12_° ]",
    // *Mercury time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀☿",      "[ 'JDN→(IFTE((JDN(DateTime)-2461178.915934)/UVAL(CONVERT(ⒸPorb☿;1_d))≥1;2461178.915934+IP((JDN(DateTime)-2461178.915934)/UVAL(CONVERT(ⒸPorb☿;1_d)))*UVAL(CONVERT(ⒸPorb☿;1_d));2461178.915934))' "
                "  0.0080_d 0 ]",

    "Astronomy/Venus",     nullptr,

    // *Venus gravitational parameter - Measurement [4]
    "GM♀",      "[ 3.24858592E14_m³/s² "
                "  0.00000012E14_m³/s² "
                "  'ROUND(ⓈGM♀/ⒸGM♀;-2)' "
                "  3.249E14_m³/s² ]",
    // *Venus mass - Calculation from GM and G
    "M♀",       "[ 'ROUND(CONVERT(ⒸGM♀/ⒸG;1_kg);XPON(UVAL(ⒸGM♀/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♀/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♀*ⒸM♀);-2);1_kg)' "
                "  'ⓇG' "
                "  4.867E24_kg ]",
    // *Venus equatorial radius - Measurement [22]
    "Req♀",     "[ 6051.8_km "
                "  0.1_km "
                "  'ROUND(ⓈReq♀/ⒸReq♀;-2)' "
                "  6052._km ]",
    // *Venus polar radius - Measurement [22]
    "Rp♀",      "[ 6051.8_km "
                "  0.1_km "
                "  'ROUND(ⓈRp♀/ⒸRp♀;-2)' "
                "  6052._km ]",
    // *Venus oblateness - Calculation from measurement [22]
    "f♀",       "[ 'ROUND(1-ⒸRp♀/ⒸReq♀;XPON(UVAL(Ⓡf♀*(1-ⒸRp♀/ⒸReq♀)))-XPON(1-ⒸRp♀/ⒸReq♀)-2)' "
                "  'ROUND(Ⓡf♀*Ⓒf♀;-2)' "
                "  'ⓇRp♀+ⓇReq♀' ]",
    // *Venus mean density - Calculation from measurement [4] [22]
    "ρv♀",       "[ 'ROUND(CONVERT(ⒸGM♀/(ⒸG·4/3·Ⓒπ·ⒸReq♀²·ⒸRp♀);1_kg/m³);XPON(UVAL(Ⓡρv♀*ⒸGM♀/(ⒸG·4/3·Ⓒπ·ⒸReq♀²·ⒸRp♀)))-XPON(UVAL(ⒸGM♀/(ⒸG·4/3·Ⓒπ·ⒸReq♀²·ⒸRp♀)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♀*Ⓒρv♀);-2);1_kg/m³)' "
                "  'ⓇGM♀ +ⓇG+2·ⓇReq♀+ⓇRp♀' ]",

    // ------------------------------------------------------------------------
    // *Venus surface gravity - Calculation from measurement [4] [22]
    "g♀",       "[ 'ROUND(CONVERT(ⒸGM♀/ⒸReq♀²;1_m/s²);XPON(UVAL(Ⓡg♀*ⒸGM♀/ⒸReq♀²))-XPON(UVAL(ⒸGM♀/ⒸReq♀²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♀*Ⓒg♀);-2);1_m/s²)' "
                "  'ⓇGM♀+2·ⓇReq♀' ]",
    // *Venus escape velocity - Calculation from measurement [4] [22]
    "Ve♀",      "[ 'ROUND(CONVERT(√(2·ⒸGM♀/(ⒸReq♀²·ⒸRp♀)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♀*√(2·ⒸGM♀/(ⒸReq♀²·ⒸRp♀)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♀/(ⒸReq♀²·ⒸRp♀)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♀*ⒸVe♀);-2);1_m/s)' "
                "  '1/2·(ⓇGM♀+(2/3·(2·ⓇReq♀+ⓇRp♀)))' ]",
    // *Venus sidereal rotation period - Measurement [22]
    "Prot♀",    "[ 20996772.8_s "
                "  8.64_s "
                "  'ROUND(ⓈProt♀/ⒸProt♀;-2)' "
                "  2.100E7_s ]",
    // *Venus orbital period - Computed [27]
    "Porb♀",    "[ 19414071.4_s "
                "  0_s "
                "  0 ]",
    // *Venus rotation angular velocity - Calculation from Prot  [retrograde]
    "ωrot♀",    "[ 'ROUND(CONVERT(-(2*Ⓒπ*1_r)/ⒸProt♀;1_r/s);XPON(UVAL(Ⓡωrot♀*(2*Ⓒπ*1_r)/ⒸProt♀))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♀))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♀*(2*Ⓒπ*1_r)/ⒸProt♀;-2);1_r/s)' "
                "  'ⓇProt♀' ]",
    // *Venus axial tilt - Measurement [22]
    "ϵ♀",       "[ 177.36_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ♀/Ⓒϵ♀;-2)' "
                "  177.4_° ]",
    // *Venus semi-major axis - Measurement [23]
    "a♀",       "[ 108208000_km "
                "  10_km "
                "  'ROUND(Ⓢa♀/Ⓒa♀;-2)' "
                "  1.082E8_km ]",

    // ------------------------------------------------------------------------
    // *Venus orbital eccentricity - Measurement [23]
    "e♀",       "[ 0.006772 "
                "  0.000001 "
                "  'ROUND(Ⓢe♀/Ⓒe♀;-2)' ]",
    // *Venus orbital inclination - Measurement [23]
    "i♀",       "[ 3.39458_° "
                "  0.00001_° "
                "  'ROUND(Ⓢi♀/Ⓒi♀;-2)' "
                "  3.395_° ]",
    // *Venus longitude of ascending node - Measurement [23]
    "Ω♀",       "[ 76.680_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♀/ⒸΩ♀;-2)' "
                "  76.68_° ]",
    // *Venus argument of perihelion - Measurement [23]
    "ω♀",       "[ 54.884_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♀/Ⓒω♀;-2)' "
                "  54.88_° ]",
    // *Venus time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀♀",      "[ 'JDN→(IFTE((JDN(DateTime)-2461175.615653)/UVAL(CONVERT(ⒸPorb♀;1_d))≥1;2461175.615653+IP((JDN(DateTime)-2461175.615653)/UVAL(CONVERT(ⒸPorb♀;1_d)))*UVAL(CONVERT(ⒸPorb♀;1_d));2461175.615653))' "
                "  0.11_d 0 ]",

    "Astronomy/Earth",     nullptr,

    // *Earth gravitational parameter - Exact nominal value [3]
    "GM♁",      "[ 3.986004E14_m³/s² "
                "  0_m³/s² "
                "  0 ]",
    // *Earth mass - Calculation from GM and G
    "M♁",       "[ 'ROUND(CONVERT(ⒸGM♁/ⒸG;1_kg);XPON(UVAL(ⒸGM♁/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♁/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♁*ⒸM♁);-2);1_kg)' "
                "  'ⓇG' "
                "  5.972E24_kg ]",
    // *Earth equatorial radius - Exact nominal value [3]
    "Req♁",     "[ 6378.1_km "
                "  0_km "
                "  0 ]",
    // *Earth polar radius - Exact nominal value [3]
    "Rp♁",      "[ 6356.8_km "
                "  0_km "
                "  0 ]",
    // *Earth oblateness - Calculation from nominal value [3]
    "f♁",       "[ 'ROUND(1-ⒸRp♁/ⒸReq♁;-5)' "
                "  'ROUND(Ⓡf♁*Ⓒf♁;-2)' "
                "  'ⓇRp♁+ⓇReq♁' ]",
    // *Earth mean density - Calculation from nominal value [3]
    "ρv♁",       "[ 'ROUND(CONVERT(ⒸGM♁/(ⒸG·4/3·Ⓒπ·ⒸReq♁²·ⒸRp♁);1_kg/m³);XPON(UVAL(Ⓡρv♁*ⒸGM♁/(ⒸG·4/3·Ⓒπ·ⒸReq♁²·ⒸRp♁)))-XPON(UVAL(ⒸGM♁/(ⒸG·4/3·Ⓒπ·ⒸReq♁²·ⒸRp♁)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♁*Ⓒρv♁);-2);1_kg/m³)' "
                "  'ⓇGM♁ +ⓇG+2·ⓇReq♁+ⓇRp♁' ]",

    // ------------------------------------------------------------------------
    // *Earth surface gravity - Calculation from nominal value [3]
    "g♁",       "[ 'ROUND(CONVERT(ⒸGM♁/ⒸReq♁²;1_m/s²);XPON(UVAL(Ⓡg♁*ⒸGM♁/ⒸReq♁²))-XPON(UVAL(ⒸGM♁/ⒸReq♁²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♁*Ⓒg♁);-2);1_m/s²)' "
                "  'ⓇGM♁+2·ⓇReq♁' ]",
    // *Earth escape velocity - Calculation from nominal value [3]
    "Ve♁",      "[ 'ROUND(CONVERT(√(2·ⒸGM♁/(ⒸReq♁²·ⒸRp♁)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♁*√(2·ⒸGM♁/(ⒸReq♁²·ⒸRp♁)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♁/(ⒸReq♁²·ⒸRp♁)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♁*ⒸVe♁);-2);1_m/s)' "
                "  '1/2·(ⓇGM♁+(2/3·(2·ⓇReq♁+ⓇRp♁)))' ]",
    // *Earth sidereal rotation period - Measurement [24]
    "Prot♁",    "[ 86164.0905_s "
                "  0.0001_s "
                "  'ROUND(ⓈProt♁/ⒸProt♁;-2)' "
                "  8.616E4_s ]",
    // *Earth orbital period - Computed [27]
    "Porb♁",    "[ 31558956.5_s "
                "  0_s "
                "  0 ]",
    // *Earth rotation angular velocity - Calculation from Prot
    "ωrot♁",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt♁;1_r/s);XPON(UVAL(Ⓡωrot♁*(2*Ⓒπ*1_r)/ⒸProt♁))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♁))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♁*(2*Ⓒπ*1_r)/ⒸProt♁;-2);1_r/s)' "
                "  'ⓇProt♁' ]",
    // *Earth axial tilt - Measurement [24]
    "ϵ♁",       "[ 23.4393_° "
                "  0.0001_° "
                "  'ROUND(Ⓢϵ♁/Ⓒϵ♁;-2)' "
                "  23.44_° ]",
    // *Earth semi-major axis - Exact nominal value [3]
    "a♁",       "[ 149597870.7_km "
                "  0_km "
                "  0 "
                "  1.496E8_km ]",

    // ------------------------------------------------------------------------
    // *Earth orbital eccentricity - Measurement [23]
    "e♁",       "[ 0.0167086 "
                "  0.0000001 "
                "  'ROUND(Ⓢe♁/Ⓒe♁;-2)' "
                "  0.01671 ]",
    // *Earth orbital inclination - Measurement [23]
    "i♁",       "[ 0.00005_° "
                "  0.00001_° "
                "  'ROUND(Ⓢi♁/Ⓒi♁;-2)' ]",
    // *Earth longitude of ascending node - Measurement [23]
    "Ω♁",       "[ 348.74_° "
                "  0.01_° "
                "  'ROUND(ⓈΩ♁/ⒸΩ♁;-2)' "
                "  348.7_° ]",
    // *Earth argument of perihelion - Measurement [23]
    "ω♁",       "[ 114.20783_° "
                "  0.00001_° "
                "  'ROUND(Ⓢω♁/Ⓒω♁;-2)' "
                "  114.2_° ]",
    // *Earth time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀♁",      "[ 'JDN→(IFTE((JDN(DateTime)-2461044.220333)/UVAL(CONVERT(ⒸPorb♁;1_d))≥1;2461044.220333+IP((JDN(DateTime)-2461044.220333)/UVAL(CONVERT(ⒸPorb♁;1_d)))*UVAL(CONVERT(ⒸPorb♁;1_d));2461044.220333))' "
                "  0.83_d 0 ]",

    // ------------------------------------------------------------------------
    // WGS-84 / GPS reference ellipsoid (defining constants, exact)
    // *Earth equatorial radius - WGS-84 defining constant [WGS-84]
    "a♁GPS",    "[ 6378137_m "
                "  0_m "
                "  0 ]",
    // *Earth flattening - WGS-84 defining constant 1/298.257223563 [WGS-84]
    "f♁GPS",    "[ '1/298.257223563' "
                "  0 "
                "  0 ]",
    // *Earth nominal mean angular velocity - WGS-84 [WGS-84]
    "ω♁GPS",    "[ 7.2921150E-5_r/s "
                "  0_r/s "
                "  0 ]",
    // *Earth gravitational parameter (incl. atmosphere) - WGS-84 [WGS-84]
    "GM♁GPS",   "[ 3.986004418E14_m³/s² "
                "  0_m³/s² "
                "  0 ]",
    // *Earth first eccentricity squared - WGS-84 ellipsoid, from flattening [WGS-84]
    "e12♁GPS",  "[ '2*Ⓒf♁GPS-Ⓒf♁GPS²' "
                "  0 "
                "  0 ]",
    // *Earth second eccentricity squared - WGS-84 ellipsoid, from first [WGS-84]
    "e22♁GPS",  "[ 'Ⓒe12♁GPS/(1-Ⓒe12♁GPS)' "
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    // Calendar and astronomical years
    // *Tropical year (equinox to equinox, J2000) - seasonal year [24]
    "Ytrop♁",   "[ 365.24219_d "
                "  0_d "
                "  0 ]",
    // *Julian year - exact, defines the light-year [24]
    "YJul♁",    "[ 365.25_d "
                "  0_d "
                "  0 ]",
    // *Gregorian mean year - exact: 365 + 1/4 - 1/100 + 1/400 [24]
    "YGreg♁",   "[ 365.2425_d "
                "  0_d "
                "  0 ]",

    "Astronomy/Moon",     nullptr,

    // *Moon gravitational parameter - Measurement [4]
    "GM☽",      "[ 4.9028001184E12_m³/s² "
                "  0.0000000009E12_m³/s² "
                "  'ROUND(ⓈGM☽/ⒸGM☽;-2)' "
                "  4.903E12_m³/s² ]",
    // *Moon mass - Calculation from GM and G
    "M☽",       "[ 'ROUND(CONVERT(ⒸGM☽/ⒸG;1_kg);XPON(UVAL(ⒸGM☽/ⒸG·ⓇG))-XPON(UVAL(ⒸGM☽/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM☽*ⒸM☽);-2);1_kg)' "
                "  'ⓇG' "
                "  7.346E22_kg ]",
    // *Moon equatorial radius - Measurement [22]
    "Req☽",     "[ 1738.1_km "
                "  0.1_km "
                "  'ROUND(ⓈReq☽/ⒸReq☽;-2)' "
                "  1738._km ]",
    // *Moon polar radius - Measurement [22]
    "Rp☽",      "[ 1736.0_km "
                "  0.1_km "
                "  'ROUND(ⓈRp☽/ⒸRp☽;-2)' "
                "  1736._km ]",
    // *Moon oblateness - Calculation from measurement [22]
    "f☽",       "[ 'ROUND(1-ⒸRp☽/ⒸReq☽;XPON(UVAL(Ⓡf☽*(1-ⒸRp☽/ⒸReq☽)))-XPON(1-ⒸRp☽/ⒸReq☽)-2)' "
                "  'ROUND(Ⓡf☽*Ⓒf☽;-2)' "
                "  'ⓇRp☽+ⓇReq☽' ]",
    // *Moon mean density - Calculation from measurement [4] [22]
    "ρv☽",       "[ 'ROUND(CONVERT(ⒸGM☽/(ⒸG·4/3·Ⓒπ·ⒸReq☽²·ⒸRp☽);1_kg/m³);XPON(UVAL(Ⓡρv☽*ⒸGM☽/(ⒸG·4/3·Ⓒπ·ⒸReq☽²·ⒸRp☽)))-XPON(UVAL(ⒸGM☽/(ⒸG·4/3·Ⓒπ·ⒸReq☽²·ⒸRp☽)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv☽*Ⓒρv☽);-2);1_kg/m³)' "
                "  'ⓇGM☽ +ⓇG+2·ⓇReq☽+ⓇRp☽' ]",

    // ------------------------------------------------------------------------
    // *Moon surface gravity - Calculation from measurement [4] [22]
    "g☽",       "[ 'ROUND(CONVERT(ⒸGM☽/ⒸReq☽²;1_m/s²);XPON(UVAL(Ⓡg☽*ⒸGM☽/ⒸReq☽²))-XPON(UVAL(ⒸGM☽/ⒸReq☽²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg☽*Ⓒg☽);-2);1_m/s²)' "
                "  'ⓇGM☽+2·ⓇReq☽' ]",
    // *Moon escape velocity - Calculation from measurement [4] [22]
    "Ve☽",      "[ 'ROUND(CONVERT(√(2·ⒸGM☽/(ⒸReq☽²·ⒸRp☽)↑(1/3));1_m/s);XPON(UVAL(ⓇVe☽*√(2·ⒸGM☽/(ⒸReq☽²·ⒸRp☽)↑(1/3))))-XPON(UVAL(√(2·ⒸGM☽/(ⒸReq☽²·ⒸRp☽)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe☽*ⒸVe☽);-2);1_m/s)' "
                "  '1/2·(ⓇGM☽+(2/3·(2·ⓇReq☽+ⓇRp☽)))' ]",
    // *Moon sidereal rotation period - Measurement [22]
    "Prot☽",    "[ 2360591.5_s "
                "  0.1_s "
                "  'ROUND(ⓈProt☽/ⒸProt☽;-2)' "
                "  2.361E6_s ]",
    // *Moon orbital period - anomalistic month (perigee to perigee) [27]
    "Porb☽",    "[ 2380713.11_s "
                "  97165_s "
                "  0 ]",
    // *Moon rotation angular velocity - Calculation from Prot
    "ωrot☽",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt☽;1_r/s);XPON(UVAL(Ⓡωrot☽*(2*Ⓒπ*1_r)/ⒸProt☽))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt☽))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot☽*(2*Ⓒπ*1_r)/ⒸProt☽;-2);1_r/s)' "
                "  'ⓇProt☽' ]",
    // *Moon axial tilt - Measurement [22]
    "ϵ☽",       "[ 1.5424_° "
                "  0.0001_° "
                "  'ROUND(Ⓢϵ☽/Ⓒϵ☽;-2)' "
                "  1.542_° ]",
    // *Moon semi-major axis - Measurement [25]
    "a☽",       "[ 384399_km "
                "  1_km "
                "  'ROUND(Ⓢa☽/Ⓒa☽;-2)' "
                "  3.844E5_km ]",

    // ------------------------------------------------------------------------
    // *Moon orbital eccentricity - Measurement [25]
    "e☽",       "[ 0.0549 "
                "  0.0001 "
                "  'ROUND(Ⓢe☽/Ⓒe☽;-2)' ]",
    // *Moon orbital inclination - Measurement [25]
    "i☽",       "[ 5.145_° "
                "  0.001_° "
                "  'ROUND(Ⓢi☽/Ⓒi☽;-2)' ]",
    // *Moon longitude of ascending node - Measurement [25]
    "Ω☽",       "[ 125.08_° "
                "  0.01_° "
                "  'ROUND(ⓈΩ☽/ⒸΩ☽;-2)' "
                "  125.1_° ]",
    // *Moon argument of perigee - Measurement [25]
    "ω☽",       "[ 318.15_° "
                "  0.01_° "
                "  'ROUND(Ⓢω☽/Ⓒω☽;-2)' "
                "  318.2_° ]",
    // *Moon time of perigee passage - PeriSel (full Meeus ch.50); unc = ch.50
    //  method accuracy vs true perigee ~5 min (1σ), up to ~30 min rare [27]
    "T₀☽",      "[ 'JDN→(ⓁPeriSel(→Num(JDN(DateTime))))' "
                "  300_s "
                "  0 ]",

    "Astronomy/Mars",     nullptr,

    // *Mars gravitational parameter - Measurement [4]
    "GM♂",      "[ 4.28283758161E13_m³/s² "
                "  0.00000000091E13_m³/s² "
                "  'ROUND(ⓈGM♂/ⒸGM♂;-2)' "
                "  4.283E13_m³/s² ]",
    // *Mars mass - Calculation from GM and G
    "M♂",       "[ 'ROUND(CONVERT(ⒸGM♂/ⒸG;1_kg);XPON(UVAL(ⒸGM♂/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♂/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♂*ⒸM♂);-2);1_kg)' "
                "  'ⓇG' "
                "  6.417E23_kg ]",
    // *Mars equatorial radius - Measurement [22]
    "Req♂",     "[ 3396.2_km "
                "  0.1_km "
                "  'ROUND(ⓈReq♂/ⒸReq♂;-2)' "
                "  3396._km ]",
    // *Mars polar radius - Measurement [22]
    "Rp♂",      "[ 3376.2_km "
                "  0.1_km "
                "  'ROUND(ⓈRp♂/ⒸRp♂;-2)' "
                "  3376._km ]",
    // *Mars oblateness - Calculation from measurement [22]
    "f♂",       "[ 'ROUND(1-ⒸRp♂/ⒸReq♂;XPON(UVAL(Ⓡf♂*(1-ⒸRp♂/ⒸReq♂)))-XPON(1-ⒸRp♂/ⒸReq♂)-2)' "
                "  'ROUND(Ⓡf♂*Ⓒf♂;-2)' "
                "  'ⓇRp♂+ⓇReq♂' ]",
    // *Mars mean density - Calculation from measurement [4] [22]
    "ρv♂",       "[ 'ROUND(CONVERT(ⒸGM♂/(ⒸG·4/3·Ⓒπ·ⒸReq♂²·ⒸRp♂);1_kg/m³);XPON(UVAL(Ⓡρv♂*ⒸGM♂/(ⒸG·4/3·Ⓒπ·ⒸReq♂²·ⒸRp♂)))-XPON(UVAL(ⒸGM♂/(ⒸG·4/3·Ⓒπ·ⒸReq♂²·ⒸRp♂)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♂*Ⓒρv♂);-2);1_kg/m³)' "
                "  'ⓇGM♂ +ⓇG+2·ⓇReq♂+ⓇRp♂' ]",

    // ------------------------------------------------------------------------
    // *Mars surface gravity - Calculation from measurement [4] [22]
    "g♂",       "[ 'ROUND(CONVERT(ⒸGM♂/ⒸReq♂²;1_m/s²);XPON(UVAL(Ⓡg♂*ⒸGM♂/ⒸReq♂²))-XPON(UVAL(ⒸGM♂/ⒸReq♂²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♂*Ⓒg♂);-2);1_m/s²)' "
                "  'ⓇGM♂+2·ⓇReq♂' ]",
    // *Mars escape velocity - Calculation from measurement [4] [22]
    "Ve♂",      "[ 'ROUND(CONVERT(√(2·ⒸGM♂/(ⒸReq♂²·ⒸRp♂)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♂*√(2·ⒸGM♂/(ⒸReq♂²·ⒸRp♂)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♂/(ⒸReq♂²·ⒸRp♂)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♂*ⒸVe♂);-2);1_m/s)' "
                "  '1/2·(ⓇGM♂+(2/3·(2·ⓇReq♂+ⓇRp♂)))' ]",
    // *Mars sidereal rotation period - Measurement [22]
    "Prot♂",    "[ 88642.7_s "
                "  0.1_s "
                "  'ROUND(ⓈProt♂/ⒸProt♂;-2)' "
                "  8.864E4_s ]",
    // *Mars orbital period - Computed [27]
    "Porb♂",    "[ 59356065.6_s "
                "  0_s "
                "  0 ]",
    // *Mars rotation angular velocity - Calculation from Prot
    "ωrot♂",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt♂;1_r/s);XPON(UVAL(Ⓡωrot♂*(2*Ⓒπ*1_r)/ⒸProt♂))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♂))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♂*(2*Ⓒπ*1_r)/ⒸProt♂;-2);1_r/s)' "
                "  'ⓇProt♂' ]",
    // *Mars axial tilt - Measurement [22]
    "ϵ♂",       "[ 25.19_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ♂/Ⓒϵ♂;-2)' ]",
    // *Mars semi-major axis - Measurement [23]
    "a♂",       "[ 227939200_km "
                "  100_km "
                "  'ROUND(Ⓢa♂/Ⓒa♂;-2)' "
                "  2.279E8_km ]",

    // ------------------------------------------------------------------------
    // *Mars orbital eccentricity - Measurement [23]
    "e♂",       "[ 0.0934 "
                "  0.0001 "
                "  'ROUND(Ⓢe♂/Ⓒe♂;-2)' ]",
    // *Mars orbital inclination - Measurement [23]
    "i♂",       "[ 1.850_° "
                "  0.001_° "
                "  'ROUND(Ⓢi♂/Ⓒi♂;-2)' ]",
    // *Mars longitude of ascending node - Measurement [23]
    "Ω♂",       "[ 49.558_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♂/ⒸΩ♂;-2)' "
                "  49.56_° ]",
    // *Mars argument of perihelion - Measurement [23]
    "ω♂",       "[ 286.502_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♂/Ⓒω♂;-2)' "
                "  286.5_° ]",
    // *Mars time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀♂",      "[ 'JDN→(IFTE((JDN(DateTime)-2461125.798009)/UVAL(CONVERT(ⒸPorb♂;1_d))≥1;2461125.798009+IP((JDN(DateTime)-2461125.798009)/UVAL(CONVERT(ⒸPorb♂;1_d)))*UVAL(CONVERT(ⒸPorb♂;1_d));2461125.798009))' "
                "  0.16_d 0 ]",

    "Astronomy/Jupiter",     nullptr,

    // *Jupiter gravitational parameter - Exact nominal value [3]
    "GM♃",      "[ 1.26686534E17_m³/s² "
                "  0_m³/s² "
                "  0 ]",
    // *Jupiter mass - Calculation from GM and G
    "M♃",       "[ 'ROUND(CONVERT(ⒸGM♃/ⒸG;1_kg);XPON(UVAL(ⒸGM♃/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♃/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♃*ⒸM♃);-2);1_kg)' "
                "  'ⓇG' "
                "  1.898E27_kg ]",
    // *Jupiter equatorial radius - Exact nominal value [3]
    "Req♃",     "[ 71492_km "
                "  0_km "
                "  0 ]",
    // *Jupiter polar radius - Exact nominal value [3]
    "Rp♃",      "[ 66854_km "
                "  0_km "
                "  0 ]",
    // *Jupiter oblateness - Calculation from nominal value [3]
    "f♃",       "[ 'ROUND(1-ⒸRp♃/ⒸReq♃;-5)' "
                "  'ROUND(Ⓡf♃*Ⓒf♃;-2)' "
                "  'ⓇRp♃+ⓇReq♃' ]",
    // *Jupiter mean density - Calculation from nominal value [3]
    "ρv♃",       "[ 'ROUND(CONVERT(ⒸGM♃/(ⒸG·4/3·Ⓒπ·ⒸReq♃²·ⒸRp♃);1_kg/m³);XPON(UVAL(Ⓡρv♃*ⒸGM♃/(ⒸG·4/3·Ⓒπ·ⒸReq♃²·ⒸRp♃)))-XPON(UVAL(ⒸGM♃/(ⒸG·4/3·Ⓒπ·ⒸReq♃²·ⒸRp♃)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♃*Ⓒρv♃);-2);1_kg/m³)' "
                "  'ⓇGM♃ +ⓇG+2·ⓇReq♃+ⓇRp♃' ]",

    // ------------------------------------------------------------------------
    // *Jupiter surface gravity - Calculation from nominal value [3]
    "g♃",       "[ 'ROUND(CONVERT(ⒸGM♃/ⒸReq♃²;1_m/s²);XPON(UVAL(Ⓡg♃*ⒸGM♃/ⒸReq♃²))-XPON(UVAL(ⒸGM♃/ⒸReq♃²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♃*Ⓒg♃);-2);1_m/s²)' "
                "  'ⓇGM♃+2·ⓇReq♃' ]",
    // *Jupiter escape velocity - Calculation from nominal value [3]
    "Ve♃",      "[ 'ROUND(CONVERT(√(2·ⒸGM♃/(ⒸReq♃²·ⒸRp♃)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♃*√(2·ⒸGM♃/(ⒸReq♃²·ⒸRp♃)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♃/(ⒸReq♃²·ⒸRp♃)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♃*ⒸVe♃);-2);1_m/s)' "
                "  '1/2·(ⓇGM♃+(2/3·(2·ⓇReq♃+ⓇRp♃)))' ]",
    // *Jupiter sidereal rotation period - Measurement [22]
    "Prot♃",    "[ 35730_s "
                "  1_s "
                "  'ROUND(ⓈProt♃/ⒸProt♃;-2)' ]",
    // *Jupiter orbital period - Computed, anomalistic perigee interval [27]
    "Porb♃",    "[ 374360783_s "
                "  0_s "
                "  0 ]",
    // *Jupiter rotation angular velocity - Calculation from Prot
    "ωrot♃",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt♃;1_r/s);XPON(UVAL(Ⓡωrot♃*(2*Ⓒπ*1_r)/ⒸProt♃))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♃))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♃*(2*Ⓒπ*1_r)/ⒸProt♃;-2);1_r/s)' "
                "  'ⓇProt♃' ]",
    // *Jupiter axial tilt - Measurement [22]
    "ϵ♃",       "[ 3.13_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ♃/Ⓒϵ♃;-2)' ]",
    // *Jupiter semi-major axis - Measurement [23]
    "a♃",       "[ 778570000_km "
                "  1000_km "
                "  'ROUND(Ⓢa♃/Ⓒa♃;-2)' "
                "  7.786E8_km ]",

    // ------------------------------------------------------------------------
    // *Jupiter orbital eccentricity - Measurement [23]
    "e♃",       "[ 0.0489 "
                "  0.0001 "
                "  'ROUND(Ⓢe♃/Ⓒe♃;-2)' ]",
    // *Jupiter orbital inclination - Measurement [23]
    "i♃",       "[ 1.303_° "
                "  0.001_° "
                "  'ROUND(Ⓢi♃/Ⓒi♃;-2)' ]",
    // *Jupiter longitude of ascending node - Measurement [23]
    "Ω♃",       "[ 100.464_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♃/ⒸΩ♃;-2)' "
                "  100.5_° ]",
    // *Jupiter argument of perihelion - Measurement [23]
    "ω♃",       "[ 273.867_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♃/Ⓒω♃;-2)' "
                "  273.9_° ]",
    // *Jupiter time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀♃",      "[ 'JDN→(IFTE((JDN(DateTime)-2459964.991260)/UVAL(CONVERT(ⒸPorb♃;1_d))≥1;2459964.991260+IP((JDN(DateTime)-2459964.991260)/UVAL(CONVERT(ⒸPorb♃;1_d)))*UVAL(CONVERT(ⒸPorb♃;1_d));2459964.991260))' "
                "  8.9_d 0 ]",

    "Astronomy/Saturn",     nullptr,

    // *Saturn gravitational parameter - Measurement [4]
    "GM♄",      "[ 3.79405848418E16_m³/s² "
                "  0.00000000091E16_m³/s² "
                "  'ROUND(ⓈGM♄/ⒸGM♄;-2)' "
                "  3.794E16_m³/s² ]",
    // *Saturn mass - Calculation from GM and G
    "M♄",       "[ 'ROUND(CONVERT(ⒸGM♄/ⒸG;1_kg);XPON(UVAL(ⒸGM♄/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♄/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♄*ⒸM♄);-2);1_kg)' "
                "  'ⓇG' "
                "  5.685E26_kg ]",
    // *Saturn equatorial radius - Measurement [22]
    "Req♄",     "[ 60268_km "
                "  4_km "
                "  'ROUND(ⓈReq♄/ⒸReq♄;-2)' "
                "  6.027E4_km ]",
    // *Saturn polar radius - Measurement [22]
    "Rp♄",      "[ 54364_km "
                "  10_km "
                "  'ROUND(ⓈRp♄/ⒸRp♄;-2)' "
                "  5.436E4_km ]",
    // *Saturn oblateness - Calculation from measurement [22]
    "f♄",       "[ 'ROUND(1-ⒸRp♄/ⒸReq♄;XPON(UVAL(Ⓡf♄*(1-ⒸRp♄/ⒸReq♄)))-XPON(1-ⒸRp♄/ⒸReq♄)-2)' "
                "  'ROUND(Ⓡf♄*Ⓒf♄;-2)' "
                "  'ⓇRp♄+ⓇReq♄' ]",
    // *Saturn mean density - Calculation from measurement [4] [22]
    "ρv♄",       "[ 'ROUND(CONVERT(ⒸGM♄/(ⒸG·4/3·Ⓒπ·ⒸReq♄²·ⒸRp♄);1_kg/m³);XPON(UVAL(Ⓡρv♄*ⒸGM♄/(ⒸG·4/3·Ⓒπ·ⒸReq♄²·ⒸRp♄)))-XPON(UVAL(ⒸGM♄/(ⒸG·4/3·Ⓒπ·ⒸReq♄²·ⒸRp♄)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♄*Ⓒρv♄);-2);1_kg/m³)' "
                "  'ⓇGM♄ +ⓇG+2·ⓇReq♄+ⓇRp♄' ]",

    // ------------------------------------------------------------------------
    // *Saturn surface gravity - Calculation from measurement [4] [22]
    "g♄",       "[ 'ROUND(CONVERT(ⒸGM♄/ⒸReq♄²;1_m/s²);XPON(UVAL(Ⓡg♄*ⒸGM♄/ⒸReq♄²))-XPON(UVAL(ⒸGM♄/ⒸReq♄²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♄*Ⓒg♄);-2);1_m/s²)' "
                "  'ⓇGM♄+2·ⓇReq♄' ]",
    // *Saturn escape velocity - Calculation from measurement [4] [22]
    "Ve♄",      "[ 'ROUND(CONVERT(√(2·ⒸGM♄/(ⒸReq♄²·ⒸRp♄)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♄*√(2·ⒸGM♄/(ⒸReq♄²·ⒸRp♄)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♄/(ⒸReq♄²·ⒸRp♄)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♄*ⒸVe♄);-2);1_m/s)' "
                "  '1/2·(ⓇGM♄+(2/3·(2·ⓇReq♄+ⓇRp♄)))' ]",
    // *Saturn sidereal rotation period - Measurement [22]
    "Prot♄",    "[ 38361_s "
                "  50_s "
                "  'ROUND(ⓈProt♄/ⒸProt♄;-2)' "
                "  3.836E4_s ]",
    // *Saturn orbital period - Computed, anomalistic perigee interval [27]
    "Porb♄",    "[ 928565359_s "
                "  0_s "
                "  0 ]",
    // *Saturn rotation angular velocity - Calculation from Prot
    "ωrot♄",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt♄;1_r/s);XPON(UVAL(Ⓡωrot♄*(2*Ⓒπ*1_r)/ⒸProt♄))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♄))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♄*(2*Ⓒπ*1_r)/ⒸProt♄;-2);1_r/s)' "
                "  'ⓇProt♄' ]",
    // *Saturn axial tilt - Measurement [22]
    "ϵ♄",       "[ 26.73_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ♄/Ⓒϵ♄;-2)' ]",
    // *Saturn semi-major axis - Measurement [23]
    "a♄",       "[ 1433530000_km "
                "  10000_km "
                "  'ROUND(Ⓢa♄/Ⓒa♄;-2)' "
                "  1.434E9_km ]",

    // ------------------------------------------------------------------------
    // *Saturn orbital eccentricity - Measurement [23]
    "e♄",       "[ 0.0565 "
                "  0.0001 "
                "  'ROUND(Ⓢe♄/Ⓒe♄;-2)' ]",
    // *Saturn orbital inclination - Measurement [23]
    "i♄",       "[ 2.485_° "
                "  0.001_° "
                "  'ROUND(Ⓢi♄/Ⓒi♄;-2)' ]",
    // *Saturn longitude of ascending node - Measurement [23]
    "Ω♄",       "[ 113.665_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♄/ⒸΩ♄;-2)' "
                "  113.7_° ]",
    // *Saturn argument of perihelion - Measurement [23]
    "ω♄",       "[ 339.392_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♄/Ⓒω♄;-2)' "
                "  339.4_° ]",
    // *Saturn time of perihelion passage - Computed via IFTE from Tp and Porb [27]
    "T₀♄",      "[ 'JDN→(IFTE((JDN(DateTime)-2452847.154242)/UVAL(CONVERT(ⒸPorb♄;1_d))≥1;2452847.154242+IP((JDN(DateTime)-2452847.154242)/UVAL(CONVERT(ⒸPorb♄;1_d)))*UVAL(CONVERT(ⒸPorb♄;1_d));2452847.154242))' "
                "  17_d 0 ]",

    "Astronomy/Uranus",     nullptr,

    // *Uranus gravitational parameter - Measurement [4]
    "GM⛢",      "[ 5.7945564E15_m³/s² "
                "  0.0000040E15_m³/s² "
                "  'ROUND(ⓈGM⛢/ⒸGM⛢;-2)' "
                "  5.795E15_m³/s² ]",
    // *Uranus mass - Calculation from GM and G
    "M⛢",       "[ 'ROUND(CONVERT(ⒸGM⛢/ⒸG;1_kg);XPON(UVAL(ⒸGM⛢/ⒸG·ⓇG))-XPON(UVAL(ⒸGM⛢/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM⛢*ⒸM⛢);-2);1_kg)' "
                "  'ⓇG' "
                "  8.682E25_kg ]",
    // *Uranus equatorial radius - Measurement [22]
    "Req⛢",     "[ 25559_km "
                "  4_km "
                "  'ROUND(ⓈReq⛢/ⒸReq⛢;-2)' "
                "  2.556E4_km ]",
    // *Uranus polar radius - Measurement [22]
    "Rp⛢",      "[ 24973_km "
                "  20_km "
                "  'ROUND(ⓈRp⛢/ⒸRp⛢;-2)' "
                "  2.497E4_km ]",
    // *Uranus oblateness - Calculation from measurement [22]
    "f⛢",       "[ 'ROUND(1-ⒸRp⛢/ⒸReq⛢;XPON(UVAL(Ⓡf⛢*(1-ⒸRp⛢/ⒸReq⛢)))-XPON(1-ⒸRp⛢/ⒸReq⛢)-2)' "
                "  'ROUND(Ⓡf⛢*Ⓒf⛢;-2)' "
                "  'ⓇRp⛢+ⓇReq⛢' ]",
    // *Uranus mean density - Calculation from measurement [4] [22]
    "ρv⛢",       "[ 'ROUND(CONVERT(ⒸGM⛢/(ⒸG·4/3·Ⓒπ·ⒸReq⛢²·ⒸRp⛢);1_kg/m³);XPON(UVAL(Ⓡρv⛢*ⒸGM⛢/(ⒸG·4/3·Ⓒπ·ⒸReq⛢²·ⒸRp⛢)))-XPON(UVAL(ⒸGM⛢/(ⒸG·4/3·Ⓒπ·ⒸReq⛢²·ⒸRp⛢)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv⛢*Ⓒρv⛢);-2);1_kg/m³)' "
                "  'ⓇGM⛢ +ⓇG+2·ⓇReq⛢+ⓇRp⛢' ]",

    // ------------------------------------------------------------------------
    // *Uranus surface gravity - Calculation from measurement [4] [22]
    "g⛢",       "[ 'ROUND(CONVERT(ⒸGM⛢/ⒸReq⛢²;1_m/s²);XPON(UVAL(Ⓡg⛢*ⒸGM⛢/ⒸReq⛢²))-XPON(UVAL(ⒸGM⛢/ⒸReq⛢²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg⛢*Ⓒg⛢);-2);1_m/s²)' "
                "  'ⓇGM⛢+2·ⓇReq⛢' ]",
    // *Uranus escape velocity - Calculation from measurement [4] [22]
    "Ve⛢",      "[ 'ROUND(CONVERT(√(2·ⒸGM⛢/(ⒸReq⛢²·ⒸRp⛢)↑(1/3));1_m/s);XPON(UVAL(ⓇVe⛢*√(2·ⒸGM⛢/(ⒸReq⛢²·ⒸRp⛢)↑(1/3))))-XPON(UVAL(√(2·ⒸGM⛢/(ⒸReq⛢²·ⒸRp⛢)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe⛢*ⒸVe⛢);-2);1_m/s)' "
                "  '1/2·(ⓇGM⛢+(2/3·(2·ⓇReq⛢+ⓇRp⛢)))' ]",
    // *Uranus sidereal rotation period - Measurement [22]
    "Prot⛢",    "[ 62064_s "
                "  10_s "
                "  'ROUND(ⓈProt⛢/ⒸProt⛢;-2)' "
                "  6.206E4_s ]",
    // *Uranus orbital period - Computed, anomalistic perigee interval [27]
    "Porb⛢",    "[ 2658520424_s "
                "  0_s "
                "  0 ]",
    // *Uranus rotation angular velocity - Calculation from Prot  [retrograde]
    "ωrot⛢",    "[ 'ROUND(CONVERT(-(2*Ⓒπ*1_r)/ⒸProt⛢;1_r/s);XPON(UVAL(Ⓡωrot⛢*(2*Ⓒπ*1_r)/ⒸProt⛢))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt⛢))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot⛢*(2*Ⓒπ*1_r)/ⒸProt⛢;-2);1_r/s)' "
                "  'ⓇProt⛢' ]",
    // *Uranus axial tilt - Measurement [22]
    "ϵ⛢",       "[ 97.77_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ⛢/Ⓒϵ⛢;-2)' ]",
    // *Uranus semi-major axis - Measurement [23]
    "a⛢",       "[ 2872460000_km "
                "  10000_km "
                "  'ROUND(Ⓢa⛢/Ⓒa⛢;-2)' "
                "  2.872E9_km ]",

    // ------------------------------------------------------------------------
    // *Uranus orbital eccentricity - Measurement [23]
    "e⛢",       "[ 0.04717 "
                "  0.00001 "
                "  'ROUND(Ⓢe⛢/Ⓒe⛢;-2)' ]",
    // *Uranus orbital inclination - Measurement [23]
    "i⛢",       "[ 0.773_° "
                "  0.001_° "
                "  'ROUND(Ⓢi⛢/Ⓒi⛢;-2)' ]",
    // *Uranus longitude of ascending node - Measurement [23]
    "Ω⛢",       "[ 74.006_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ⛢/ⒸΩ⛢;-2)' "
                "  74.01_° ]",
    // *Uranus argument of perihelion - Measurement [23]
    "ω⛢",       "[ 96.998857_° "
                "  0.000001_° "
                "  'ROUND(Ⓢω⛢/Ⓒω⛢;-2)' "
                "  97.00_° ]",
    // *Uranus time of perihelion passage - Computed via IFTE from Tp and Porb [27]  (last perihelion 1966; next ~2050)
    "T₀⛢",      "[ 'JDN→(IFTE((JDN(DateTime)-2439264.256742)/UVAL(CONVERT(ⒸPorb⛢;1_d))≥1;2439264.256742+IP((JDN(DateTime)-2439264.256742)/UVAL(CONVERT(ⒸPorb⛢;1_d)))*UVAL(CONVERT(ⒸPorb⛢;1_d));2439264.256742))' "
                "  40_d 0 ]",

    "Astronomy/Neptune",     nullptr,

    // *Neptune gravitational parameter - Measurement [4]
    "GM♆",      "[ 6.83652710058E15_m³/s² "
                "  0.00000010058E15_m³/s² "
                "  'ROUND(ⓈGM♆/ⒸGM♆;-2)' "
                "  6.837E15_m³/s² ]",
    // *Neptune mass - Calculation from GM and G
    "M♆",       "[ 'ROUND(CONVERT(ⒸGM♆/ⒸG;1_kg);XPON(UVAL(ⒸGM♆/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♆/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♆*ⒸM♆);-2);1_kg)' "
                "  'ⓇG' "
                "  1.024E26_kg ]",
    // *Neptune equatorial radius - Measurement [22]
    "Req♆",     "[ 24764_km "
                "  15_km "
                "  'ROUND(ⓈReq♆/ⒸReq♆;-2)' "
                "  2.476E4_km ]",
    // *Neptune polar radius - Measurement [22]
    "Rp♆",      "[ 24341_km "
                "  30_km "
                "  'ROUND(ⓈRp♆/ⒸRp♆;-2)' "
                "  2.434E4_km ]",
    // *Neptune oblateness - Calculation from measurement [22]
    "f♆",       "[ 'ROUND(1-ⒸRp♆/ⒸReq♆;XPON(UVAL(Ⓡf♆*(1-ⒸRp♆/ⒸReq♆)))-XPON(1-ⒸRp♆/ⒸReq♆)-2)' "
                "  'ROUND(Ⓡf♆*Ⓒf♆;-2)' "
                "  'ⓇRp♆+ⓇReq♆' ]",
    // *Neptune mean density - Calculation from measurement [4] [22]
    "ρv♆",       "[ 'ROUND(CONVERT(ⒸGM♆/(ⒸG·4/3·Ⓒπ·ⒸReq♆²·ⒸRp♆);1_kg/m³);XPON(UVAL(Ⓡρv♆*ⒸGM♆/(ⒸG·4/3·Ⓒπ·ⒸReq♆²·ⒸRp♆)))-XPON(UVAL(ⒸGM♆/(ⒸG·4/3·Ⓒπ·ⒸReq♆²·ⒸRp♆)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♆*Ⓒρv♆);-2);1_kg/m³)' "
                "  'ⓇGM♆ +ⓇG+2·ⓇReq♆+ⓇRp♆' ]",

    // ------------------------------------------------------------------------
    // *Neptune surface gravity - Calculation from measurement [4] [22]
    "g♆",       "[ 'ROUND(CONVERT(ⒸGM♆/ⒸReq♆²;1_m/s²);XPON(UVAL(Ⓡg♆*ⒸGM♆/ⒸReq♆²))-XPON(UVAL(ⒸGM♆/ⒸReq♆²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♆*Ⓒg♆);-2);1_m/s²)' "
                "  'ⓇGM♆+2·ⓇReq♆' ]",
    // *Neptune escape velocity - Calculation from measurement [4] [22]
    "Ve♆",      "[ 'ROUND(CONVERT(√(2·ⒸGM♆/(ⒸReq♆²·ⒸRp♆)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♆*√(2·ⒸGM♆/(ⒸReq♆²·ⒸRp♆)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♆/(ⒸReq♆²·ⒸRp♆)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♆*ⒸVe♆);-2);1_m/s)' "
                "  '1/2·(ⓇGM♆+(2/3·(2·ⓇReq♆+ⓇRp♆)))' ]",
    // *Neptune sidereal rotation period - Measurement [22]
    "Prot♆",    "[ 58000_s "
                "  100_s "
                "  'ROUND(ⓈProt♆/ⒸProt♆;-2)' ]",
    // *Neptune orbital period - Sidereal [28]
    "Porb♆",    "[ 5200416000_s "
                "  0_s "
                "  0 ]",
    // *Neptune rotation angular velocity - Calculation from Prot
    "ωrot♆",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt♆;1_r/s);XPON(UVAL(Ⓡωrot♆*(2*Ⓒπ*1_r)/ⒸProt♆))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♆))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♆*(2*Ⓒπ*1_r)/ⒸProt♆;-2);1_r/s)' "
                "  'ⓇProt♆' ]",
    // *Neptune axial tilt - Measurement [22]
    "ϵ♆",       "[ 28.32_° "
                "  0.01_° "
                "  'ROUND(Ⓢϵ♆/Ⓒϵ♆;-2)' ]",
    // *Neptune semi-major axis - Measurement [23]
    "a♆",       "[ 4495060000_km "
                "  10000_km "
                "  'ROUND(Ⓢa♆/Ⓒa♆;-2)' "
                "  4.495E9_km ]",

    // ------------------------------------------------------------------------
    // *Neptune orbital eccentricity - Measurement [23]
    "e♆",       "[ 0.00859 "
                "  0.00001 "
                "  'ROUND(Ⓢe♆/Ⓒe♆;-2)' ]",
    // *Neptune orbital inclination - Measurement [23]
    "i♆",       "[ 1.770_° "
                "  0.001_° "
                "  'ROUND(Ⓢi♆/Ⓒi♆;-2)' ]",
    // *Neptune longitude of ascending node - Measurement [23]
    "Ω♆",       "[ 131.784_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♆/ⒸΩ♆;-2)' "
                "  131.8_° ]",
    // *Neptune argument of perihelion - Measurement [23]
    "ω♆",       "[ 273.187_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♆/Ⓒω♆;-2)' "
                "  273.2_° ]",
    // *Neptune time of perihelion passage - Computed via IFTE from Tp and Porb [4]  (EXCEPTION: Tp = next perihelion 2042; floor-IFTE returns it unchanged)
    "T₀♆",      "[ 'JDN→(IFTE((JDN(DateTime)-2467131.5)/UVAL(CONVERT(ⒸPorb♆;1_d))≥1;2467131.5+IP((JDN(DateTime)-2467131.5)/UVAL(CONVERT(ⒸPorb♆;1_d)))*UVAL(CONVERT(ⒸPorb♆;1_d));2467131.5))' "
                "  10_d 0 ]",

    "Astronomy/Pluto",     nullptr,

    // *Pluto gravitational parameter - Measurement [4]
    "GM♇",      "[ 9.755E11_m³/s² "
                "  0.005E11_m³/s² "
                "  'ROUND(ⓈGM♇/ⒸGM♇;-2)' ]",
    // *Pluto mass - Calculation from GM and G
    "M♇",       "[ 'ROUND(CONVERT(ⒸGM♇/ⒸG;1_kg);XPON(UVAL(ⒸGM♇/ⒸG·ⓇG))-XPON(UVAL(ⒸGM♇/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM♇*ⒸM♇);-2);1_kg)' "
                "  'ⓇG' "
                "  1.462E22_kg ]",
    // *Pluto equatorial radius - Measurement [26]
    "Req♇",     "[ 1188.3_km "
                "  1.6_km "
                "  'ROUND(ⓈReq♇/ⒸReq♇;-2)' "
                "  1188._km ]",
    // *Pluto polar radius - Measurement [26]
    "Rp♇",      "[ 1188.3_km "
                "  1.6_km "
                "  'ROUND(ⓈRp♇/ⒸRp♇;-2)' "
                "  1188._km ]",
    // *Pluto oblateness - Calculation from measurement [26]
    "f♇",       "[ 'ROUND(1-ⒸRp♇/ⒸReq♇;XPON(UVAL(Ⓡf♇*(1-ⒸRp♇/ⒸReq♇)))-XPON(1-ⒸRp♇/ⒸReq♇)-2)' "
                "  'ROUND(Ⓡf♇*Ⓒf♇;-2)' "
                "  'ⓇRp♇+ⓇReq♇' ]",
    // *Pluto mean density - Calculation from measurement [4] [26]
    "ρv♇",       "[ 'ROUND(CONVERT(ⒸGM♇/(ⒸG·4/3·Ⓒπ·ⒸReq♇²·ⒸRp♇);1_kg/m³);XPON(UVAL(Ⓡρv♇*ⒸGM♇/(ⒸG·4/3·Ⓒπ·ⒸReq♇²·ⒸRp♇)))-XPON(UVAL(ⒸGM♇/(ⒸG·4/3·Ⓒπ·ⒸReq♇²·ⒸRp♇)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv♇*Ⓒρv♇);-2);1_kg/m³)' "
                "  'ⓇGM♇ +ⓇG+2·ⓇReq♇+ⓇRp♇' ]",

    // ------------------------------------------------------------------------
    // *Pluto surface gravity - Calculation from measurement [4] [26]
    "g♇",       "[ 'ROUND(CONVERT(ⒸGM♇/ⒸReq♇²;1_m/s²);XPON(UVAL(Ⓡg♇*ⒸGM♇/ⒸReq♇²))-XPON(UVAL(ⒸGM♇/ⒸReq♇²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg♇*Ⓒg♇);-2);1_m/s²)' "
                "  'ⓇGM♇+2·ⓇReq♇' ]",
    // *Pluto escape velocity - Calculation from measurement [4] [26]
    "Ve♇",      "[ 'ROUND(CONVERT(√(2·ⒸGM♇/(ⒸReq♇²·ⒸRp♇)↑(1/3));1_m/s);XPON(UVAL(ⓇVe♇*√(2·ⒸGM♇/(ⒸReq♇²·ⒸRp♇)↑(1/3))))-XPON(UVAL(√(2·ⒸGM♇/(ⒸReq♇²·ⒸRp♇)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe♇*ⒸVe♇);-2);1_m/s)' "
                "  '1/2·(ⓇGM♇+(2/3·(2·ⓇReq♇+ⓇRp♇)))' ]",
    // *Pluto sidereal rotation period - Measurement [26]
    "Prot♇",    "[ 551856.7_s "
                "  0.1_s "
                "  'ROUND(ⓈProt♇/ⒸProt♇;-2)' "
                "  5.519E5_s ]",
    // *Pluto orbital period - Sidereal [28]
    "Porb♇",    "[ 7824384000_s "
                "  0_s "
                "  0 ]",
    // *Pluto rotation angular velocity - Calculation from Prot  [retrograde]
    "ωrot♇",    "[ 'ROUND(CONVERT(-(2*Ⓒπ*1_r)/ⒸProt♇;1_r/s);XPON(UVAL(Ⓡωrot♇*(2*Ⓒπ*1_r)/ⒸProt♇))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt♇))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot♇*(2*Ⓒπ*1_r)/ⒸProt♇;-2);1_r/s)' "
                "  'ⓇProt♇' ]",
    // *Pluto axial tilt - Measurement [26]
    "ϵ♇",       "[ 119.591_° "
                "  0.001_° "
                "  'ROUND(Ⓢϵ♇/Ⓒϵ♇;-2)' "
                "  119.6_° ]",
    // *Pluto semi-major axis - Measurement [23]
    "a♇",       "[ 5906380000_km "
                "  10000_km "
                "  'ROUND(Ⓢa♇/Ⓒa♇;-2)' "
                "  5.906E9_km ]",

    // ------------------------------------------------------------------------
    // *Pluto orbital eccentricity - Measurement [23]
    "e♇",       "[ 0.2488 "
                "  0.0001 "
                "  'ROUND(Ⓢe♇/Ⓒe♇;-2)' ]",
    // *Pluto orbital inclination - Measurement [23]
    "i♇",       "[ 17.16_° "
                "  0.01_° "
                "  'ROUND(Ⓢi♇/Ⓒi♇;-2)' ]",
    // *Pluto longitude of ascending node - Measurement [23]
    "Ω♇",       "[ 110.299_° "
                "  0.001_° "
                "  'ROUND(ⓈΩ♇/ⒸΩ♇;-2)' "
                "  110.3_° ]",
    // *Pluto argument of perihelion - Measurement [23]
    "ω♇",       "[ 113.834_° "
                "  0.001_° "
                "  'ROUND(Ⓢω♇/Ⓒω♇;-2)' "
                "  113.8_° ]",
    // *Pluto time of perihelion passage - Computed via IFTE from Tp and Porb [4]  (last perihelion 1989)
    "T₀♇",      "[ 'JDN→(IFTE((JDN(DateTime)-2447774.5)/UVAL(CONVERT(ⒸPorb♇;1_d))≥1;2447774.5+IP((JDN(DateTime)-2447774.5)/UVAL(CONVERT(ⒸPorb♇;1_d)))*UVAL(CONVERT(ⒸPorb♇;1_d));2447774.5))' "
                "  5_d 0 ]",

    "Astronomy/Sun",     nullptr,

    // *Solar mass - Calculation from nominal value [2] [3] [5]
    "M☉",       "[ 'ROUND(CONVERT(ⒸGM☉/ⒸG;1_kg);XPON(UVAL(ⒸGM☉/ⒸG·ⓇG))-XPON(UVAL(ⒸGM☉/ⒸG))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇM☉*ⒸM☉);-2);1_kg)' "
                "  'ⓇG' "
                "  1.989E30_kg ]",
    // *Solar gravitational parameter - Exact nominal value [3]
    "GM☉",      "[ 1.3271244E20_m³/s² "
                "  0_m³/s² "
                "  0 ]",
    // *Solar luminosity - Exact nominal value [3]
    "L☉",       "[ 3.828E26_W "
                "  0_W "
                "  0 ]",
    // *Solar equatorial radius - Exact nominal value [3]
    "Req☉",     "[ 695700_km "
                "  0_km "
                "  0 ]",
    // *Solar polar radius - Measurement [18] [19]
    "Rp☉",      "[ 695508_km "
                "  140_km "
                "  'ROUND(ⓈRp☉/ⒸRp☉;-2)' "
                "  6.955E5_km ]",

    // ------------------------------------------------------------------------
    // *Solar oblateness - Calculation from measurement [18] [19]
    "f☉",       "[ 'ROUND(1-ⒸRp☉/ⒸReq☉;XPON(UVAL(Ⓡf☉*(1-ⒸRp☉/ⒸReq☉)))-XPON(1-ⒸRp☉/ⒸReq☉)-2)' "
                "  'ROUND(Ⓡf☉*Ⓒf☉;-2)' "
                "  'ⓇRp☉+ⓇReq☉' ]",
    // *Solar mean density - Calculation from nominal value [3]
    "ρv☉",       "[ 'ROUND(CONVERT(ⒸGM☉/(ⒸG·4/3·Ⓒπ·ⒸReq☉²·ⒸRp☉);1_kg/m³);XPON(UVAL(Ⓡρv☉*ⒸGM☉/(ⒸG·4/3·Ⓒπ·ⒸReq☉²·ⒸRp☉)))-XPON(UVAL(ⒸGM☉/(ⒸG·4/3·Ⓒπ·ⒸReq☉²·ⒸRp☉)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡρv☉*Ⓒρv☉);-2);1_kg/m³)' "
                "  'ⓇGM☉ +ⓇG+2·ⓇReq☉+ⓇRp☉' ]",
    // *Solar surface gravity - Calculation from nominal value [3]
    "g☉",       "[ 'ROUND(CONVERT(ⒸGM☉/ⒸReq☉²;1_m/s²);XPON(UVAL(Ⓡg☉*ⒸGM☉/ⒸReq☉²))-XPON(UVAL(ⒸGM☉/ⒸReq☉²))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡg☉*Ⓒg☉);-2);1_m/s²)' "
                "  'ⓇGM☉+2·ⓇReq☉' ]",
    // *Solar escape velocity - Calculation from nominal value [3]
    "Ve☉",      "[ 'ROUND(CONVERT(√(2·ⒸGM☉/(ⒸReq☉²·ⒸRp☉)↑(1/3));1_m/s);XPON(UVAL(ⓇVe☉*√(2·ⒸGM☉/(ⒸReq☉²·ⒸRp☉)↑(1/3))))-XPON(UVAL(√(2·ⒸGM☉/(ⒸReq☉²·ⒸRp☉)↑(1/3))))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇVe☉*ⒸVe☉);-2);1_m/s)' "
                "  '1/2·(ⓇGM☉+(2/3·(2·ⓇReq☉+ⓇRp☉)))' ]",
    // *Solar sidereal rotation period - Measurement [20] [21]
    "Prot☉",    "[ 2192832_s "
                "  864_s "
                "  'ROUND(ⓈProt☉/ⒸProt☉;-2)' "
                "  2.193E6_s ]",
    // *Sun rotation angular velocity - Calculation from Prot
    "ωrot☉",    "[ 'ROUND(CONVERT((2*Ⓒπ*1_r)/ⒸProt☉;1_r/s);XPON(UVAL(Ⓡωrot☉*(2*Ⓒπ*1_r)/ⒸProt☉))-XPON(UVAL((2*Ⓒπ*1_r)/ⒸProt☉))-2)' "
                "  'CONVERT(ROUND(Ⓡωrot☉*(2*Ⓒπ*1_r)/ⒸProt☉;-2);1_r/s)' "
                "  'ⓇProt☉' ]",

    "Astronomy/Cosmology",     nullptr,

    // *Cosmological constant - Measurement [1] [2]
    "Λ",       "[ 1.088E-52_1/m² "
                "  0.030E-52_1/m² "
                "  'ROUND(ⓈΛ/ⒸΛ;-2)' ]",
    // *Hubble constant - Measurement [1]
    "H₀",       "[ 67.4_km/(s·Mpc) "
                "  0.5_km/(s·Mpc) "
                "  'ROUND(ⓈH₀/ⒸH₀;-2)' ]",
    // *Baryon density parameter - Measurement [1]
    "ωb",       "[ 0.02237 "
                "  0.00015 "
                "  'ROUND(Ⓢωb/Ⓒωb;-2)' ]",
    // *Cold dark matter density parameter - Measurement [1]
    "ωc",       "[ 0.1200 "
                "  0.0012 "
                "  'ROUND(Ⓢωc/Ⓒωc;-2)' ]",
    // *CMB acoustic scale - Measurement [1]
    "100θ*",    "[ 1.04109 "
                "  0.00030 "
                "  'ROUND(0.00030/1.04109;-2)' "
                "  1.041 ]",

    // ------------------------------------------------------------------------
    // *Reionization optical depth - Measurement [1]
    "τr",       "[ 0.0544 "
                "  0.0073 "
                "  'ROUND(Ⓢτr/Ⓒτr;-2)' ]",
    // *Scalar spectral index - Measurement [1]
    "ns",       "[ 0.9665 "
                "  0.0038 "
                "  'ROUND(Ⓢns/Ⓒns;-2)' ]",
    // *Power spectrum amplitude - Measurement [1]
    "(ln(10¹⁰As))", "[ 3.044 "
                "  0.014 "
                "  'ROUND(0.014/3.044;-2)' ]",
    // *Milky Way total mass - Measurement [8] [9] [10]
    "MG",       "[ 1.5E12_M☉ "
                "  0.5E12_M☉ "
                "  'ROUND(ⓈMG/ⒸMG;-2)' ]",
    // *Milky Way dark matter fraction - Measurement [9] [32]
    "fDMG",     "[ 0.95 "
                "  0.02 "
                "  'ROUND(ⓈfDMG/ⒸfDMG;-2)' ]",

    // ------------------------------------------------------------------------
    // *Milky Way disk radius - Measurement [8] [9]
    "RG",       "[ 50000_ly "
                "  5000_ly "
                "  'ROUND(ⓈRG/ⒸRG;-2)' ]",
    // *Distance to Sgr A* - Measurement [6] [7]
    "D●SgrA*",  "[ 26673_ly "
                "  42_ly "
                "  'ROUND(42/26673;-2)' "
                "  2.667E4_ly ]",
    // *Sgr A* Schwarzschild radius - Measurement [7]
    "R●SgrA*",  "[ 12.0E6_km "
                "  0.4E6_km "
                "  'ROUND(0.4E6/12.0E6;-2)' ]",
    // *Sgr A* black hole mass - Measurement [7]
    "M●SgrA*",  "[ 4.297E6_M☉ "
                "  0.012E6_M☉ "
                "  'ROUND(0.012E6/4.297E6;-2)' ]",
    // *Andromeda galaxy total mass - Measurement [35] [36]
    "MNGC224",  "[ 1.4E12_M☉ "
                "  0.4E12_M☉ "
                "  'ROUND(ⓈMNGC224/ⒸMNGC224;-2)' ]",

    // ------------------------------------------------------------------------
    // *Andromeda dark matter fraction - Measurement [31] [35]
    "fDMNGC224", "[ 0.92 "
                "  0.03 "
                "  'ROUND(ⓈfDMNGC224/ⒸfDMNGC224;-2)' ]",
    // *Andromeda galaxy disk radius - Measurement [34]
    "RNGC224",  "[ 110000_ly "
                "  10000_ly "
                "  'ROUND(ⓈRNGC224/ⒸRNGC224;-2)' "
                "  1.100E5_ly ]",
    // *Distance to Andromeda galaxy - Measurement [29]
    "DNGC224",  "[ 2482000_ly "
                "  36000_ly "
                "  'ROUND(ⓈDNGC224/ⒸDNGC224;-2)' "
                "  2.482E6_ly ]",
    // *Vega gravitational parameter - Measurement [13] [14]
    "GM★Vega",  "[ 1.89E19_m³/s² "
                "  0.05E19_m³/s² "
                "  'ROUND(ⓈGM★Vega/ⒸGM★Vega;-2)' ]",
    // *Vega luminosity - Measurement [13] [14]
    "L★Vega",   "[ 1.51E27_W "
                "  0.05E27_W "
                "  'ROUND(ⓈL★Vega/ⒸL★Vega;-2)' ]",

    "Materials",     nullptr,

    // ========================================================================
    //   A. GASES (at StdT = 273.15 K = 0°C, StdP = 101.325 kPa)
    //
    //   Parameters: ρv, n*, cp, cv, γ, R, ε, μ, ρ, Ed
    //     *: where relevant
    //   γ = cp/cv and R = cp − cv apply to gases only.
    //   γ is given as a calculation 'ROUND(ⒸcpX/ⒸcvX;4)' for diatomic gases.
    //   R is given as a calculation 'CONVERT(ⒸcpX-ⒸcvX;1_J/(kg*K))' for all gases.
    // ========================================================================

    "Materials/Dry Air",     nullptr,

    // *Dry air volumetric density - Measurement [M1] [M2]
    "ρvAir",     "[ 1.2929_kg/m³ "
                "  0.0001_kg/m³ "
                "  'ROUND(ⓈρvAir/ⒸρvAir;-2)' ]",
    // *Dry air refractive index - Measurement [M2]
    "nAir",     "[ 1.000293 "
                "  0.000001 "
                "  'ROUND(ⓈnAir/ⒸnAir;-2)' ]",
    // *Dry air specific heat cp - Measurement [M1] [M2]
    "cpAir",    "[ 1006._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcpAir/ⒸcpAir;-2)' ]",
    // *Dry air specific heat cv - Calculation from measurement [M1] [M2]
    "cvAir",    "[ 717._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcvAir/ⒸcvAir;-2)' ]",
    // *Dry air adiabatic index - Calculation: γ=cp/cv [M1] [M2]
    "γAir",     "[ 'ROUND(ⒸcpAir/ⒸcvAir;XPON(UVAL(ⓇγAir*ⒸcpAir/ⒸcvAir))-XPON(UVAL(ⒸcpAir/ⒸcvAir))-2)' "
                "  'ROUND(ⓇγAir*ⒸγAir;-2)' "
                "  'ROUND(ⓈcpAir/ⒸcpAir+ⓈcvAir/ⒸcvAir;-2)' ]",

    // ------------------------------------------------------------------------
    // *Dry air specific gas constant - Calculation: R=cp-cv [M1]
    "RAir",     "[ 'ROUND(CONVERT(ⒸcpAir-ⒸcvAir;1_J/(kg*K));XPON(UVAL(ⓇRAir*(ⒸcpAir-ⒸcvAir)))-XPON(UVAL(ⒸcpAir-ⒸcvAir))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇRAir*ⒸRAir);-2);1_J/(kg*K))' "
                "  'ROUND((ⓈcpAir+ⓈcvAir)/ABS(ⒸcpAir-ⒸcvAir);-2)' ]",
    // *Dry air electric permittivity - Measurement [M1] [M2]
    "εAir",     "[ 8.8542E-12_F/m "
                "  0.0005E-12_F/m "
                "  'ROUND(ⓈεAir/ⒸεAir;-2)' "
                "  8.854E-12_F/m ]",
    // *Dry air magnetic permeability - Measurement [M1] [M2]
    "μAir",     "[ 1.2566E-6_H/m "
                "  0.0001E-6_H/m "
                "  'ROUND(ⓈμAir/ⒸμAir;-2)' "
                "  1.257E-6_H/m ]",
    // *Dry air resistivity - Measurement [M1] [M2]
    "ρAir",     "[ 1.5E15_Ω·m "
                "  0.5E15_Ω·m "
                "  'ROUND(ⓈρAir/ⒸρAir;-2)' ]",
    // *Dry air dielectric rigidity - Measurement [M1] [M2]
    "EdAir",    "[ 3.0E6_V/m "
                "  0.2E6_V/m "
                "  'ROUND(ⓈEdAir/ⒸEdAir;-2)' ]",

    "Materials/N2",     nullptr,

    // *Dinitrogen N2 volumetric density - Measurement [M2] [M3]
    "ρvN2",      "[ 1.2506_kg/m³ "
                "  0.0001_kg/m³ "
                "  'ROUND(ⓈρvN2/ⒸρvN2;-2)' ]",
    // *Dinitrogen N2 specific heat cp - Measurement [M2] [M3]
    "cpN2",     "[ 1039._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcpN2/ⒸcpN2;-2)' ]",
    // *Dinitrogen N2 specific heat cv - Measurement [M2] [M3]
    "cvN2",     "[ 742._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcvN2/ⒸcvN2;-2)' ]",
    // *Dinitrogen N2 adiabatic index - Calculation: γ=cp/cv [M2] [M3]
    "γN2",      "[ 'ROUND(ⒸcpN2/ⒸcvN2;XPON(UVAL(ⓇγN2*ⒸcpN2/ⒸcvN2))-XPON(UVAL(ⒸcpN2/ⒸcvN2))-2)' "
                "  'ROUND(ⓇγN2*ⒸγN2;-2)' "
                "  'ROUND(ⓈcpN2/ⒸcpN2+ⓈcvN2/ⒸcvN2;-2)' ]",
    // *Dinitrogen N2 specific gas constant - Calculation: R=cp-cv [M2]
    "RN2",      "[ 'ROUND(CONVERT(ⒸcpN2-ⒸcvN2;1_J/(kg*K));XPON(UVAL(ⓇRN2*(ⒸcpN2-ⒸcvN2)))-XPON(UVAL(ⒸcpN2-ⒸcvN2))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇRN2*ⒸRN2);-2);1_J/(kg*K))' "
                "  'ROUND((ⓈcpN2+ⓈcvN2)/ABS(ⒸcpN2-ⒸcvN2);-2)' ]",

    // ------------------------------------------------------------------------
    // *Dinitrogen N2 electric permittivity - Measurement [M2]
    "εN2",      "[ 8.859E-12_F/m "        // εr ≈ 1.000580 at STP
                "  0.001E-12_F/m "
                "  'ROUND(ⓈεN2/ⒸεN2;-2)' ]",
    // *Dinitrogen N2 magnetic permeability - Measurement [M2]
    "μN2",      "[ 1.2566E-6_H/m "        // μr ≈ 1.0000000 (diamagnetic)
                "  0.0001E-6_H/m "
                "  'ROUND(ⓈμN2/ⒸμN2;-2)' "
                "  1.257E-6_H/m ]",
    // *Dinitrogen N2 resistivity - Measurement [M2]
    "ρN2",      "[ 1.0E15_Ω·m "           // dry N2 at STP; excellent insulator
                "  0.5E15_Ω·m "
                "  'ROUND(ⓈρN2/ⒸρN2;-2)' ]",
    // *Dinitrogen N2 dielectric rigidity - Measurement [M2]
    "EdN2",     "[ 3.5E6_V/m "            // slightly higher than dry air
                "  0.3E6_V/m "
                "  'ROUND(ⓈEdN2/ⒸEdN2;-2)' ]",

    "Materials/O2",     nullptr,

    // *Dioxygen O2 volumetric density - Measurement [M2] [M3]
    "ρvO2",      "[ 1.4290_kg/m³ "
                "  0.0001_kg/m³ "
                "  'ROUND(ⓈρvO2/ⒸρvO2;-2)' ]",
    // *Dioxygen O2 specific heat cp - Measurement [M2] [M3]
    "cpO2",     "[ 919._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcpO2/ⒸcpO2;-2)' ]",
    // *Dioxygen O2 specific heat cv - Measurement [M2] [M3]
    "cvO2",     "[ 659._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcvO2/ⒸcvO2;-2)' ]",
    // *Dioxygen O2 adiabatic index - Calculation: γ=cp/cv [M2] [M3]
    "γO2",      "[ 'ROUND(ⒸcpO2/ⒸcvO2;XPON(UVAL(ⓇγO2*ⒸcpO2/ⒸcvO2))-XPON(UVAL(ⒸcpO2/ⒸcvO2))-2)' "
                "  'ROUND(ⓇγO2*ⒸγO2;-2)' "
                "  'ROUND(ⓈcpO2/ⒸcpO2+ⓈcvO2/ⒸcvO2;-2)' ]",
    // *Dioxygen O2 specific gas constant - Calculation: R=cp-cv [M2]
    "RO2",      "[ 'ROUND(CONVERT(ⒸcpO2-ⒸcvO2;1_J/(kg*K));XPON(UVAL(ⓇRO2*(ⒸcpO2-ⒸcvO2)))-XPON(UVAL(ⒸcpO2-ⒸcvO2))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇRO2*ⒸRO2);-2);1_J/(kg*K))' "
                "  'ROUND((ⓈcpO2+ⓈcvO2)/ABS(ⒸcpO2-ⒸcvO2);-2)' ]",

    // ------------------------------------------------------------------------
    // *Dioxygen O2 electric permittivity - Measurement [M2]
    "εO2",      "[ 8.859E-12_F/m "        // εr ≈ 1.000530 at STP
                "  0.001E-12_F/m "
                "  'ROUND(ⓈεO2/ⒸεO2;-2)' ]",
    // *Dioxygen O2 magnetic permeability - Measurement [M2]
    "μO2",      "[ 1.2566E-6_H/m "        // μr ≈ 1.0000019 (paramagnetic)
                "  0.0001E-6_H/m "
                "  'ROUND(ⓈμO2/ⒸμO2;-2)' "
                "  1.257E-6_H/m ]",
    // *Dioxygen O2 resistivity - Measurement [M2]
    "ρO2",      "[ 1.0E15_Ω·m "           // dry O2 at STP
                "  0.5E15_Ω·m "
                "  'ROUND(ⓈρO2/ⒸρO2;-2)' ]",
    // *Dioxygen O2 dielectric rigidity - Measurement [M2]
    "EdO2",     "[ 2.9E6_V/m "            // slightly lower than N2
                "  0.3E6_V/m "
                "  'ROUND(ⓈEdO2/ⒸEdO2;-2)' ]",

    "Materials/Argon",     nullptr,

    // *Argon volumetric density - Measurement [M2]
    "ρvAr",      "[ 1.7839_kg/m³ "
                "  0.0001_kg/m³ "
                "  'ROUND(ⓈρvAr/ⒸρvAr;-2)' ]",
    // *Argon specific heat cp - Exact calculation [M2]
    "cpAr",     "[ 520.3_J/(kg*K) "
                "  0.1_J/(kg*K) "
                "  'ROUND(ⓈcpAr/ⒸcpAr;-2)' ]",
    // *Argon specific heat cv - Exact calculation [M2]
    "cvAr",     "[ 312.2_J/(kg*K) "
                "  0.1_J/(kg*K) "
                "  'ROUND(ⓈcvAr/ⒸcvAr;-2)' ]",
    // *Argon adiabatic index - Exact definition: 5/3 for monatomic ideal gas [M2]
    "γAr",      "[ '5/3' "
                "  0 "
                "  0 ]",
    // *Argon specific gas constant - Exact calculation: R=cp-cv [M2]
    "RAr",      "[ 'CONVERT(ⒸcpAr-ⒸcvAr;1_J/(kg*K))' "
                "  'CONVERT(ROUND(UBASE(ⓇRAr*ⒸRAr);-2);1_J/(kg*K))' "
                "  'ROUND((ⓈcpAr+ⓈcvAr)/ABS(ⒸcpAr-ⒸcvAr);-2)' ]",

    // ------------------------------------------------------------------------
    // *Argon electric permittivity - Measurement [M2]
    "εAr",      "[ 8.855E-12_F/m "        // εr ≈ 1.000516 at STP
                "  0.001E-12_F/m "
                "  'ROUND(ⓈεAr/ⒸεAr;-2)' ]",
    // *Argon magnetic permeability - Measurement [M2]
    "μAr",      "[ 1.2566E-6_H/m "        // μr ≈ 1.0000000 (diamagnetic)
                "  0.0001E-6_H/m "
                "  'ROUND(ⓈμAr/ⒸμAr;-2)' "
                "  1.257E-6_H/m ]",
    // *Argon resistivity - Measurement [M2]
    "ρAr",      "[ 1.0E15_Ω·m "           // noble gas; excellent insulator
                "  0.5E15_Ω·m "
                "  'ROUND(ⓈρAr/ⒸρAr;-2)' ]",
    // *Argon dielectric rigidity - Measurement [M2]
    "EdAr",     "[ 2.0E6_V/m "            // lower than air (no polar molecules)
                "  0.2E6_V/m "
                "  'ROUND(ⓈEdAr/ⒸEdAr;-2)' ]",

    "Materials/CO2",     nullptr,

    // *Carbon dioxide CO2 volumetric density - Measurement [M2] [M3]
    "ρvCO2",     "[ 1.9768_kg/m³ "
                "  0.0001_kg/m³ "
                "  'ROUND(ⓈρvCO2/ⒸρvCO2;-2)' ]",
    // *Carbon dioxide CO2 specific heat cp - Measurement [M2] [M3]
    "cpCO2",    "[ 819._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcpCO2/ⒸcpCO2;-2)' ]",
    // *Carbon dioxide CO2 specific heat cv - Measurement [M2] [M3]
    "cvCO2",    "[ 630._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcvCO2/ⒸcvCO2;-2)' ]",
    // *Carbon dioxide CO2 adiabatic index - Calculation: γ=cp/cv [M2] [M3]
    "γCO2",     "[ 'ROUND(ⒸcpCO2/ⒸcvCO2;XPON(UVAL(ⓇγCO2*ⒸcpCO2/ⒸcvCO2))-XPON(UVAL(ⒸcpCO2/ⒸcvCO2))-2)' "
                "  'ROUND(ⓇγCO2*ⒸγCO2;-2)' "
                "  'ROUND(ⓈcpCO2/ⒸcpCO2+ⓈcvCO2/ⒸcvCO2;-2)' ]",
    // *Carbon dioxide CO2 specific gas constant - Calculation: R=cp-cv [M2]
    "RCO2",     "[ 'ROUND(CONVERT(ⒸcpCO2-ⒸcvCO2;1_J/(kg*K));XPON(UVAL(ⓇRCO2*(ⒸcpCO2-ⒸcvCO2)))-XPON(UVAL(ⒸcpCO2-ⒸcvCO2))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇRCO2*ⒸRCO2);-2);1_J/(kg*K))' "
                "  'ROUND((ⓈcpCO2+ⓈcvCO2)/ABS(ⒸcpCO2-ⒸcvCO2);-2)' ]",

    // ------------------------------------------------------------------------
    // *Carbon dioxide CO2 electric permittivity - Measurement [M2] [M3]
    "εCO2",     "[ 8.920E-12_F/m "        // εr ≈ 1.000921 at STP
                "  0.001E-12_F/m "
                "  'ROUND(ⓈεCO2/ⒸεCO2;-2)' ]",
    // *Carbon dioxide CO2 magnetic permeability - Measurement [M2]
    "μCO2",     "[ 1.2566E-6_H/m "        // μr ≈ 1.0000000 (diamagnetic)
                "  0.0001E-6_H/m "
                "  'ROUND(ⓈμCO2/ⒸμCO2;-2)' "
                "  1.257E-6_H/m ]",
    // *Carbon dioxide CO2 resistivity - Measurement [M2]
    "ρCO2",     "[ 1.0E13_Ω·m "           // lower than N2/Ar; polar molecule
                "  0.5E13_Ω·m "
                "  'ROUND(ⓈρCO2/ⒸρCO2;-2)' ]",
    // *Carbon dioxide CO2 dielectric rigidity - Measurement [M2]
    "EdCO2",    "[ 2.9E6_V/m "            // similar to air
                "  0.3E6_V/m "
                "  'ROUND(ⓈEdCO2/ⒸEdCO2;-2)' ]",

    // ========================================================================
    //   B. LIQUIDS (at T = 293.15 K = 20°C, P = 101.325 kPa)
    //
    //   For liquids: cv ≈ cp (difference < 1%), γ ≈ 1, R_id ≈ 0.
    //   These parameters are therefore not listed for liquids.
    // ========================================================================

    "Materials/H2O",     nullptr,

    // *Liquid water volumetric density - Measurement [M4] [M2]
    "ρvH2O",     "[ 998.2_kg/m³ "
                "  0.1_kg/m³ "
                "  'ROUND(ⓈρvH2O/ⒸρvH2O;-2)' ]",
    // *Liquid water refractive index - Measurement [M4]
    "nH2O",     "[ 1.3330 "
                "  0.0005 "
                "  'ROUND(ⓈnH2O/ⒸnH2O;-2)' "
                "  1.333 ]",
    // *Liquid water specific heat cp - Measurement [M4] [M2]
    "cpH2O",    "[ 4182._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcpH2O/ⒸcpH2O;-2)' ]",
    // *Liquid water resistivity - Measurement [M4] [M2]
    "ρH2O",     "[ 1.8E5_Ω·m "            // ultra-pure water at 25°C; 18 MΩ·cm
                "  0.2E5_Ω·m "
                "  'ROUND(ⓈρH2O/ⒸρH2O;-2)' ]",

    // ========================================================================
    //   C. SOLIDS (at T = 293.15 K = 20°C, P = 101.325 kPa)
    //
    //   Sub-sections:
    //     C1. Materials for optical fibers and waveguides
    //     C2. Reference optical glasses
    //     C3. Optical crystals (UV and IR)
    //     C4. Reference metals (thermal and mechanical engineering)
    // ========================================================================

    // ------------------------------------------------------------------------
    //   C1. Materials for optical fibers and waveguides
    // ------------------------------------------------------------------------

    "Materials/Silicon",     nullptr,

    // *Crystalline silicon volumetric density - Measurement [M10] [M11]
    "ρvSi",      "[ 2330._kg/m³ "
                "  1._kg/m³ "
                "  'ROUND(ⓈρvSi/ⒸρvSi;-2)' ]",
    // *Crystalline silicon refractive index - Measurement [M11]
    "nSi",      "[ 3.4179 "
                "  0.0002 "
                "  'ROUND(ⓈnSi/ⒸnSi;-2)' ]",
    // *Crystalline silicon specific heat - Measurement [M10]
    "cpSi",     "[ 711._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcpSi/ⒸcpSi;-2)' ]",
    // *Dielectric constant of silicon - Definition convention [HP50g manual]
    "εsi",      "[ 11.9 "
                "  0 "
                "  0 ]",
    // *Crystalline silicon resistivity - Measurement [M10] [M11]
    "ρSi",      "[ 2.3E3_Ω·m "            // intrinsic Si at 300 K; range 10²–10⁴ Ω·m
                "  0.5E3_Ω·m "
                "  'ROUND(ⓈρSi/ⒸρSi;-2)' ]",

    "Materials/SiO2",     nullptr,

    // *Fused silica volumetric density - Measurement [M7]
    "ρvSiO2",    "[ 2201._kg/m³ "
                "  2._kg/m³ "
                "  'ROUND(ⓈρvSiO2/ⒸρvSiO2;-2)' ]",
    // *Fused silica refractive index - Measurement [M5]
    "nSiO2",    "[ 1.45701 "
                "  0.00005 "
                "  'ROUND(ⓈnSiO2/ⒸnSiO2;-2)' ]",
    // *Fused silica specific heat - Measurement [M7]
    "cpSiO2",   "[ 703._J/(kg*K) "
                "  5._J/(kg*K) "
                "  'ROUND(ⓈcpSiO2/ⒸcpSiO2;-2)' ]",
    // *Dielectric constant of SiO2 - Definition convention [HP50g manual]
    "εox",      "[ 3.9 "
                "  0 "
                "  0 ]",
    // *Fused silica resistivity - Measurement [M7]
    "ρSiO2",    "[ 1.0E18_Ω·m "           // fused silica; excellent insulator
                "  0.5E18_Ω·m "
                "  'ROUND(ⓈρSiO2/ⒸρSiO2;-2)' ]",
    // *Fused silica dielectric rigidity - Measurement [M7]
    "EdSiO2",   "[ 5.0E8_V/m "            // fused silica bulk breakdown
                "  0.5E8_V/m "
                "  'ROUND(ⓈEdSiO2/ⒸEdSiO2;-2)' ]",

    "Materials/GeO2",     nullptr,

    // *Germanate glass volumetric density - Measurement [M8] [M9]
    "ρvGeO2",    "[ 3650._kg/m³ "
                "  10._kg/m³ "
                "  'ROUND(ⓈρvGeO2/ⒸρvGeO2;-2)' ]",
    // *Germanate glass refractive index - Measurement [M8]
    "nGeO2",    "[ 1.607 "
                "  0.002 "
                "  'ROUND(ⓈnGeO2/ⒸnGeO2;-2)' ]",
    // *Germanate glass specific heat - Measurement [M9]
    "cpGeO2",   "[ 440._J/(kg*K) "
                "  10._J/(kg*K) "
                "  'ROUND(ⓈcpGeO2/ⒸcpGeO2;-2)' ]",
    // *Germanate glass resistivity - Measurement [M9]
    "ρGeO2",    "[ 1.0E12_Ω·m "           // pure GeO2 glass; lower than SiO2
                "  0.5E12_Ω·m "
                "  'ROUND(ⓈρGeO2/ⒸρGeO2;-2)' ]",
    // *Germanate glass dielectric rigidity - Measurement [M9]
    "EdGeO2",   "[ 4.0E8_V/m "            // estimated from glass family
                "  0.5E8_V/m "
                "  'ROUND(ⓈEdGeO2/ⒸEdGeO2;-2)' ]",

    "Materials/Si3N4",     nullptr,

    // *Silicon nitride volumetric density - Measurement [M18] [M19]
    "ρvSi3N4",   "[ 3170._kg/m³ "
                "  10._kg/m³ "
                "  'ROUND(ⓈρvSi3N4/ⒸρvSi3N4;-2)' ]",
    // *LPCVD Si3N4 refractive index - Measurement [M18]
    "nSi3N4",   "[ 2.016 "
                "  0.005 "
                "  'ROUND(ⓈnSi3N4/ⒸnSi3N4;-2)' ]",
    // *Silicon nitride specific heat - Measurement [M19]
    "cpSi3N4",  "[ 691._J/(kg*K) "
                "  10._J/(kg*K) "
                "  'ROUND(ⓈcpSi3N4/ⒸcpSi3N4;-2)' ]",
    // *Silicon nitride resistivity - Measurement [M19]
    "ρSi3N4",   "[ 1.0E14_Ω·m "           // LPCVD Si3N4; good insulator
                "  0.5E14_Ω·m "
                "  'ROUND(ⓈρSi3N4/ⒸρSi3N4;-2)' ]",
    // *Silicon nitride dielectric rigidity - Measurement [M19]
    "EdSi3N4",  "[ 1.0E9_V/m "            // thin-film breakdown field
                "  0.2E9_V/m "
                "  'ROUND(ⓈEdSi3N4/ⒸEdSi3N4;-2)' ]",

    // ------------------------------------------------------------------------
    //   C2. Reference optical glasses
    // ------------------------------------------------------------------------

    "Materials/BK7 Glass",     nullptr,

    // *Schott BK7 glass volumetric density - Measurement [M15]
    "ρvBK7",     "[ 2510._kg/m³ "
                "  5._kg/m³ "
                "  'ROUND(ⓈρvBK7/ⒸρvBK7;-2)' ]",
    // *Schott BK7 glass refractive index - Measurement [M15]
    "nBK7",     "[ 1.51680 "
                "  0.00002 "
                "  'ROUND(ⓈnBK7/ⒸnBK7;-2)' ]",
    // *Schott BK7 glass specific heat - Measurement [M15]
    "cpBK7",    "[ 858._J/(kg*K) "
                "  10._J/(kg*K) "
                "  'ROUND(ⓈcpBK7/ⒸcpBK7;-2)' ]",
    // *Schott BK7 glass resistivity - Measurement [M15]
    "ρBK7",     "[ 1.0E13_Ω·m "           // borosilicate glass at 20°C
                "  0.5E13_Ω·m "
                "  'ROUND(ⓈρBK7/ⒸρBK7;-2)' ]",
    // *Schott BK7 glass dielectric rigidity - Measurement [M15]
    "EdBK7",    "[ 2.5E7_V/m "            // bulk optical glass breakdown
                "  0.5E7_V/m "
                "  'ROUND(ⓈEdBK7/ⒸEdBK7;-2)' ]",

    // ------------------------------------------------------------------------
    //   C3. Optical crystals (UV and IR)
    // ------------------------------------------------------------------------

    "Materials/Al2O3 Sapphire",     nullptr,

    // *Sapphire volumetric density - Measurement [M16]
    "ρvAl2O3",   "[ 3980._kg/m³ "
                "  5._kg/m³ "
                "  'ROUND(ⓈρvAl2O3/ⒸρvAl2O3;-2)' ]",
    // *Sapphire ordinary refractive index - Measurement [M16]
    "nAl2O3",   "[ 1.7659 "
                "  0.0002 "
                "  'ROUND(ⓈnAl2O3/ⒸnAl2O3;-2)' ]",
    // *Sapphire specific heat - Measurement [M16] [M2]
    "cpAl2O3",  "[ 765._J/(kg*K) "
                "  5._J/(kg*K) "
                "  'ROUND(ⓈcpAl2O3/ⒸcpAl2O3;-2)' ]",
    // *Sapphire resistivity - Measurement [M16]
    "ρAl2O3",   "[ 1.0E14_Ω·m "           // single-crystal sapphire at 20°C
                "  0.5E14_Ω·m "
                "  'ROUND(ⓈρAl2O3/ⒸρAl2O3;-2)' ]",
    // *Sapphire dielectric rigidity - Measurement [M16]
    "EdAl2O3",  "[ 4.0E8_V/m "            // bulk sapphire breakdown field
                "  0.5E8_V/m "
                "  'ROUND(ⓈEdAl2O3/ⒸEdAl2O3;-2)' ]",

    "Materials/CaF2",     nullptr,

    // *Calcium fluoride volumetric density - Measurement [M17] [M15]
    "ρvCaF2",    "[ 3180._kg/m³ "
                "  5._kg/m³ "
                "  'ROUND(ⓈρvCaF2/ⒸρvCaF2;-2)' ]",
    // *Calcium fluoride refractive index - Measurement [M17]
    "nCaF2",    "[ 1.43381 "
                "  0.00005 "
                "  'ROUND(ⓈnCaF2/ⒸnCaF2;-2)' ]",
    // *Calcium fluoride specific heat - Measurement [M15]
    "cpCaF2",   "[ 854._J/(kg*K) "
                "  5._J/(kg*K) "
                "  'ROUND(ⓈcpCaF2/ⒸcpCaF2;-2)' ]",
    // *Calcium fluoride resistivity - Measurement [M17]
    "ρCaF2",    "[ 1.0E15_Ω·m "           // single-crystal CaF2 at 20°C
                "  0.5E15_Ω·m "
                "  'ROUND(ⓈρCaF2/ⒸρCaF2;-2)' ]",
    // *Calcium fluoride dielectric rigidity - Measurement [M17]
    "EdCaF2",   "[ 1.0E9_V/m "            // CaF2 crystal bulk breakdown
                "  0.2E9_V/m "
                "  'ROUND(ⓈEdCaF2/ⒸEdCaF2;-2)' ]",

    "Materials/C Diamond",     nullptr,

    // *Diamond volumetric density - Measurement [M13] [M14]
    "ρvC",       "[ 3515._kg/m³ "
                "  1._kg/m³ "
                "  'ROUND(ⓈρvC/ⒸρvC;-2)' ]",
    // *Diamond refractive index - Measurement [M14]
    "nC",       "[ 2.4180 "
                "  0.0002 "
                "  'ROUND(ⓈnC/ⒸnC;-2)' ]",
    // *Diamond specific heat - Measurement [M13]
    "cpC",      "[ 502._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcpC/ⒸcpC;-2)' ]",
    // *Diamond resistivity - Measurement [M13] [M14]
    "ρC",       "[ 1.0E13_Ω·m "           // type IIa natural diamond; ~10¹¹–10¹⁴ Ω·m
                "  0.5E13_Ω·m "
                "  'ROUND(ⓈρC/ⒸρC;-2)' ]",
    // *Diamond dielectric rigidity - Measurement [M13] [M14]
    "EdC",      "[ 1.0E9_V/m "            // type IIa diamond; highest of any solid
                "  0.2E9_V/m "
                "  'ROUND(ⓈEdC/ⒸEdC;-2)' ]",

    // ------------------------------------------------------------------------
    //   C4. Reference metals (thermal and mechanical engineering)
    //
    //   For metals: n not listed (complex n, strongly wavelength-dependent).
    //   cv ≈ cp for all metals at room temperature (difference < 0.5%).
    // ------------------------------------------------------------------------

    "Materials/Al Metal",     nullptr,

    // *Aluminium volumetric density - Measurement [M12] [M2]
    "ρvAl",      "[ 2700._kg/m³ "
                "  1._kg/m³ "
                "  'ROUND(ⓈρvAl/ⒸρvAl;-2)' ]",
    // *Aluminium specific heat - Measurement [M12] [M2]
    "cpAl",     "[ 902._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcpAl/ⒸcpAl;-2)' ]",

    "Materials/Cu Metal",     nullptr,

    // *Copper volumetric density - Measurement [M12] [M2]
    "ρvCu",      "[ 8960._kg/m³ "
                "  5._kg/m³ "
                "  'ROUND(ⓈρvCu/ⒸρvCu;-2)' ]",
    // *Copper specific heat - Measurement [M12] [M2]
    "cpCu",     "[ 385._J/(kg*K) "
                "  1._J/(kg*K) "
                "  'ROUND(ⓈcpCu/ⒸcpCu;-2)' ]",

    "Materials/Fe Metal",     nullptr,

    // *Iron volumetric density (α phase) - Measurement [M12] [M2]
    "ρvFe",      "[ 7874._kg/m³ "
                "  3._kg/m³ "
                "  'ROUND(ⓈρvFe/ⒸρvFe;-2)' ]",
    // *Iron specific heat (α phase, BCC) - Measurement [M12] [M2]
    "cpFe",     "[ 450._J/(kg*K) "
                "  2._J/(kg*K) "
                "  'ROUND(ⓈcpFe/ⒸcpFe;-2)' ]",

    // ------------------------------------------------------------------------
    //  Computing
    // ------------------------------------------------------------------------
    "Computing",   nullptr,

   // *Boolean false value - Exact convention
    "No",                       "False",                // No value = false
    // *Boolean true value - Exact convention
    "Yes",                      "True",                 // Yes value = true
    // *Unix time epoch - Exact convention
    "UnixEpoch",                "[ 19700101_date 0_date 0 ]",
    // *Sinclair ZX81 RAM size - Exact definition
    "SinclairZX81RAM",          "[ 1_KiB 0_KiB 0 ]",
    // *Virtual memory page size - Exact convention
    "PageSize",                 "[ 4_KiB 0_KiB 0 ]",

    // ------------------------------------------------------------------------
    // *Hello World string - Exact convention
    "HelloWorld",               "\"Hello World\"",
};
// ------------------------------------------------------------------------
//   References are in constants.md — Bibliographic references section
//   NUMERIC CONSTANTS TABLE (actual value as of 2026-05-11)
//
//   Section                    Count    %
//   Mathematics                 11    2.8%
//   Chemistry                   13    3.3%
//   Physics                     12    3.1%
//   Mass                        23    5.9%
//   Electromagnetism            25    6.4%
//   Size                         3    0.8%
//   Scattering                   6    1.5%
//   Quantum                     15    3.8%
//   Astronomy                  180   46.0%
//   Materials                   97   24.8%
//   Computing                    6    1.5%
//   TOTAL                      391  100.0%
//
//   Exact or Definition         40   11.3%
//   Calculated                 112   28.6%
//   Measured                   239   61.1%
// ------------------------------------------------------------------------
//   clang-format on


static runtime &invalid_constant_error()
// ----------------------------------------------------------------------------
//    Return the error message for invalid constants
// ----------------------------------------------------------------------------
{
    return rt.invalid_constant_error();
}


static bool show_builtin_constants()
// ----------------------------------------------------------------------------
//   Check if we show the builtin constants
// ----------------------------------------------------------------------------
{
    return Settings.ShowBuiltinConstants();
}


const constant::config constant::constants =
// ----------------------------------------------------------------------------
//  Define the configuration for the constants
// ----------------------------------------------------------------------------
{
    .menu_help     = " constants",
    .help          = " constant",
    .prefix        = L'Ⓒ',
    .type          = ID_constant,
    .first_menu    = ID_ConstantsMenu00,
    .last_menu     = ID_ConstantsMenu99,
    .name          = ID_constant_menu_name,
    .value         = ID_constant_menu_value,
    .command       = ID_constant_menu_range,
    .file          = "config/constants.csv",
    .library       = "library",
    .builtins      = basic_constants,
    .nbuiltins     = sizeof(basic_constants) / sizeof(*basic_constants),
    .vlabel        = "Value",
    .clabel        = "Range",
    .error         = invalid_constant_error,
    .show_builtins = show_builtin_constants,
    .stack_prefix  = false,
    .ignore_case   = false,
};


object::result constant::do_parsing(config_r cfg, parser &p)
// ----------------------------------------------------------------------------
//    Try to parse this as a constant
// ----------------------------------------------------------------------------
{
    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = 0;

    // First character must be a constant marker
    unicode cp     = utf8_codepoint(source);
    bool    needed = cp == cfg.prefix;
    if (!needed && unit::mode)
        return SKIP;
    if (needed)
        parsed = utf8_next(source, parsed, max);
    size_t first = parsed;

    // Other characters must be alphabetic
    while (parsed < max && is_valid_in_constant_name(source + parsed))
        parsed = utf8_next(source, parsed, max);
    if (parsed <= first)
        return SKIP;
    if (!needed)
    {
        // Check if the name exists in the current directory, prefer it if so
        if (symbol_p sym = directory::lookup_all(source, parsed))
        {
            p.length = parsed;
            p.out    = sym;
            return OK;
        }

        // Never use a constant in an assignent
        size_t remain  = max - parsed;
        size_t next    = parsed + utf8_skip_whitespace(source + parsed, remain);
        if (next < max)
        {
            unicode cp   = utf8_codepoint(source + next);
            bool    ineq = p.precedence;
            if (cp == '_'                               // Units
                || (!ineq && cp == '=')                 // In assignment
                || (ineq && (cp == '\'' || cp =='(')))  // 'A' or 'F(...)'
                return SKIP;
        }
    }

    size_t     len = parsed - first;
    constant_p cst = do_lookup(cfg, source + first, len, needed);
    p.length       = parsed;
    p.out          = cst;
    return cst ? OK : needed ? ERROR : SKIP;
}


size_t constant::do_rendering(config_r cfg, constant_p o, renderer &r)
// ----------------------------------------------------------------------------
//   Rendering of a constant
// ----------------------------------------------------------------------------
{
    constant_g cst = o;
    size_t     len = 0;
    utf8       txt = nullptr;
    txt = cst->do_name(cfg, &len);
    if (r.editing() || cfg.stack_prefix)
        r.put(cfg.prefix);
    r.put(txt, len);
    return r.size();
}


constant_p constant::do_lookup(config_r cfg, utf8 txt, size_t len, bool error)
// ----------------------------------------------------------------------------
//   Scan the table and file to see if there is matching constant
// ----------------------------------------------------------------------------
{
    if (unit::mode)
        return nullptr;

    unit_file cfile(cfg.file);
    size_t    maxb     = cfg.nbuiltins;
    auto      builtins = cfg.builtins;
    utf8      ctxt     = nullptr;
    size_t    clen     = 0;
    uint      idx      = 0;
    bool      icase    = cfg.ignore_case && Settings.IgnoreSymbolCase();
    settings::SaveIgnoreSymbolCase isc(icase);

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (cfile.next(true))                        // Category
        {
            while (symbol_p name = cfile.next(false))
            {
                ctxt = name->value(&clen);

                // Constant name comparison is case-sensitive
                if (len == clen && symbol::compare(txt, ctxt, len) == 0)
                    return constant::make(cfg.type, idx);
                idx++;
            }
        }
    }

    // Check built-in constants
    for (size_t b = 0; b < maxb; b += 2)
    {
        if (builtins[b+1] && *builtins[b+1])
        {
            ctxt = utf8(builtins[b]);
            if (ctxt[len] == 0 && symbol::compare(ctxt, txt, len) == 0)
                return constant::make(cfg.type, idx);
            idx++;
        }
    }

    if (error)
        cfg.error().source(txt, len);
    return nullptr;
}


utf8 constant::do_name(config_r cfg, size_t *len) const
// ----------------------------------------------------------------------------
//   Return the name for the constant
// ----------------------------------------------------------------------------
{
    unit_file cfile(cfg.file);
    size_t    maxb     = cfg.nbuiltins;
    auto      builtins = cfg.builtins;
    cstring   ctxt     = nullptr;
    uint      idx      = index();

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (cfile.next(true))                        // Category
        {
            while (symbol_p sym = cfile.next(false))
            {
                if (!idx)
                    return sym->value(len);
                idx--;
            }
        }
    }

    // Check built-in constants
    for (size_t b = 0; b < maxb; b += 2)
    {
        if (builtins[b+1] && *builtins[b+1])
        {
            ctxt = builtins[b];
            if (!idx)
            {
                if (len)
                    *len = strlen(ctxt);
                return utf8(ctxt);
            }
            idx--;
        }
    }
    return nullptr;
}


object_p constant::do_value(config_r cfg) const
// ----------------------------------------------------------------------------
//   Lookup a built-in constant
// ----------------------------------------------------------------------------
{
    unit_file cfile(cfg.file);
    size_t    maxb     = cfg.nbuiltins;
    auto      builtins = cfg.builtins;
    symbol_g  csym     = nullptr;
    symbol_g  cname    = nullptr;
    size_t    clen     = 0;
    uint      idx      = index();

    settings::SaveAutomaticConstants sac(false);
    settings::SaveAutomaticXLibs     sax(false);

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (cfile.next(true))                        // Category
        {
            uint position = cfile.position();
            while (symbol_p sym = cfile.next(false))
            {
                if (!idx)
                {
                    cname = sym;
                    utf8 ctxt = sym->value(&clen);
                    cfile.seek(position);
                    csym = cfile.lookup(ctxt, clen, false, false);
                    break;
                }
                position = cfile.position();
                idx--;
            }
            if (csym)
                break;
        }
    }

    // Check built-in constants
    for (size_t b = 0; !csym && b < maxb; b += 2)
    {
        if (builtins[b+1] && *builtins[b+1])
        {
            if (!idx)
            {
                cname = symbol::make(builtins[b]);
                csym = symbol::make(builtins[b+1]);
                break;
            }
            idx--;
        }
    }

    // If we found a definition, use that
    if (csym)
    {
        // Special cases for pi and e where we have built-in constants
        if (cname->matches("π"))
            return decimal::pi();
        else if (cname->matches("e"))
            return decimal::e();

        utf8 cdef = csym->value(&clen);
        if (*cdef == '=')
        {
            if (text_g filename = clen>1 ? text::make(cdef+1, clen-1) : cname)
                if (files_g disk = files::make(cfg.library))
                    if (object_p obj = disk->recall(filename))
                        return obj;
        }
        else
        {
            error_save esave;
            if (object_p obj = object::parse(cdef, clen))
                return obj;
        }
    }
    cfg.error();
    return nullptr;
}


utf8 constant::do_instance_help(constant::config_r cfg) const
// ----------------------------------------------------------------------------
//   Generate the help topic for a given constant menu
// ----------------------------------------------------------------------------
{
    static char buf[64];
    size_t      len  = 0;
    utf8        base = do_name(cfg, &len);
    snprintf(buf, sizeof(buf), "%.*s%s", int(len), base, cfg.help);
    return utf8(buf);
}





// ============================================================================
//
//   Build a constants menu
//
// ============================================================================
//
//   In the following, we skip menus beginning with `=`
//   That feature is used by the `=Cycle` menu in config/units.csv,
//   which describes unit cycles but should not show in units menu

static bool path_top_level(cstring name)
// ----------------------------------------------------------------------------
//   True if name has no '/' (top-level category)
// ----------------------------------------------------------------------------
{
    return !strchr(name, '/');
}


static bool path_top_level(symbol_p sym)
// ----------------------------------------------------------------------------
//   True if name has no '/' (top-level category)
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8   txt = sym->value(&len);
    return !memchr(txt, '/', len);
}


static utf8 path_last_segment(utf8 path, size_t &len)
// ----------------------------------------------------------------------------
//   Return the final path segment (text after the last '/')
// ----------------------------------------------------------------------------
{
    size_t i = len;
    while (i-- > 0)
    {
        if (path[i] == '/')
        {
            len -= i + 1;
            return path + i + 1;
        }
    }
    return path;
}


static bool path_is_direct_child(utf8   child,
                                 size_t clen,
                                 utf8   parent,
                                 size_t plen)
// ----------------------------------------------------------------------------
//   True if child is parent + '/' + one segment (no further slashes)
// ----------------------------------------------------------------------------
{
    if (clen <= plen || memcmp(child, parent, plen) != 0 || child[plen] != '/')
        return false;
    utf8 rest = child + plen + 1;
    return !memchr(rest, '/', clen - plen - 1);
}


static utf8 path_for_menu(constant::config_r cfg,
                          object::id         menu,
                          size_t            *len = nullptr)
// ----------------------------------------------------------------------------
//   Return the full category path for a menu id
// ----------------------------------------------------------------------------
{
    uint      count = menu - cfg.first_menu;
    unit_file cfile(cfg.file);

    if (cfile.valid())
        while (symbol_p mname = cfile.next(true))
            if (*mname->value() != '=')
                if (!count--)
                    return mname->value(len);

    if (cfg.show_builtins())
    {
        size_t maxb     = cfg.nbuiltins;
        auto   builtins = cfg.builtins;
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b + 1] || !*builtins[b + 1])
            {
                if (!count--)
                {
                    if (len)
                        *len = strlen(builtins[b]);
                    return utf8(builtins[b]);
                }
            }
        }
    }

    return nullptr;
}


static uint path_count_direct_children(constant::config_r cfg,
                                       utf8               parent,
                                       size_t             plen)
// ----------------------------------------------------------------------------
//   Count category headers that are direct children of parent
// ----------------------------------------------------------------------------
{
    uint      count = 0;
    unit_file cfile(cfg.file);

    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_p mname = cfile.next(true))
        {
            if (*mname->value() == '=')
                continue;
            size_t mlen = 0;
            utf8   path = mname->value(&mlen);
            if (path_is_direct_child(path, mlen, parent, plen))
                count++;
        }
    }

    if (!cfile.valid() || cfg.show_builtins())
    {
        size_t maxb     = cfg.nbuiltins;
        auto   builtins = cfg.builtins;
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b + 1] || !*builtins[b + 1])
            {
                utf8   child = utf8(builtins[b]);
                size_t clen  = strlen(builtins[b]);
                if (path_is_direct_child(child, clen, parent, plen))
                    count++;
            }
        }
    }

    return count;
}


utf8 constant_menu::do_name(constant::config_r cfg, id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the display name associated with the type
// ----------------------------------------------------------------------------
{
    if (utf8 path = path_for_menu(cfg, type, &len))
        return path_last_segment(path, len);
    return nullptr;
}


bool constant_menu::do_submenu(constant::config_r cfg, menu_info &mi) const
// ----------------------------------------------------------------------------
//   Load the menu from a file
// ----------------------------------------------------------------------------
{
    id     type    = this->type();
    size_t curlen  = 0;
    utf8   current = path_for_menu(cfg, type, &curlen);
    if (!current)
        return false;

    uint children = path_count_direct_children(cfg, current, curlen);
    if (children)
    {
        items_init(mi, children);
        uint idx = 0;
        unit_file cfile(cfg.file);
        if (cfile.valid())
        {
            cfile.seek(0);
            while (symbol_p mname = cfile.next(true))
            {
                if (*mname->value() == '=')
                    continue;
                size_t mlen = 0;
                utf8   path = mname->value(&mlen);
                if (path_is_direct_child(path, mlen, current, curlen))
                {
                    path = path_last_segment(path, mlen);
                    symbol_p label = symbol::make(path, mlen);
                    items(mi, label, id(cfg.first_menu + idx));
                }
                idx++;
            }
        }
        if (!cfile.valid() || cfg.show_builtins())
        {
            size_t maxb     = cfg.nbuiltins;
            auto   builtins = cfg.builtins;
            for (size_t b = 0; b < maxb; b += 2)
            {
                if (!builtins[b + 1] || !*builtins[b + 1])
                {
                    utf8   child = utf8(builtins[b]);
                    size_t clen  = strlen(builtins[b]);
                    id     cid   = id(cfg.first_menu + idx);
                    if (path_is_direct_child(child, clen, current, curlen))
                        items(mi, cstring(path_last_segment(child, clen)), cid);
                    idx++;
                }
            }
        }
        return true;
    }

    // Use the constants loaded from the constants file
    unit_file cfile(cfg.file);
    size_t    matching = 0;
    uint      position = 0;
    uint      count    = 0;
    id        menu     = cfg.first_menu;
    id        lastm    = cfg.last_menu;
    size_t    first    = 0;
    size_t    last     = cfg.nbuiltins;

    if (cfile.valid())
    {
        while (symbol_p mname = cfile.next(true))
        {
            if (*mname->value() == '=')
                continue;
            if (menu == type)
            {
                position = cfile.position();
                while (cfile.next(false))
                    matching++;
                menu = id(menu + 1);
                break;
            }
            menu = id(menu + 1);
            if (menu > lastm)
                break;
        }
    }

    // Disable built-in constants if we loaded a file
    if (!matching || cfg.show_builtins())
    {
        bool   found    = false;
        auto   builtins = cfg.builtins;
        size_t maxb     = cfg.nbuiltins;

        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b + 1] || !*builtins[b + 1])
            {
                if (found)
                {
                    last = b;
                    break;
                }
                if (menu == type)
                {
                    found = true;
                    first = b + 2;
                }
                menu = id(menu + 1);
                if (menu > lastm)
                    break;
            }
        }
        if (found)
            count = (last - first) / 2;
    }

    uint planes   = 1 + !!cfg.value + !!cfg.command;
    items_init(mi, count + matching, planes, 1);

    // Insert the built-in constants after the ones from the file
    uint skip     = mi.skip;
    id   ids[3]   = { cfg.name, cfg.value, cfg.command };
    auto builtins = cfg.builtins;

    for (uint plane = 0; plane < planes; plane++)
    {
        mi.plane  = plane;
        mi.planes = plane + 1;
        mi.index  = plane * ui.NUM_SOFTKEYS;
        mi.skip   = skip;
        id type = ids[plane];

        if (matching)
        {
            cfile.seek(position);

            if (plane)
            {
                cstring label = plane == 1 ? cfg.vlabel : cfg.clabel;
                while (cfile.next(false))
                    items(mi, label, type);
            }
            else
            {
                while (symbol_p mentry = cfile.next(false))
                    items(mi, mentry, type);
            }
        }
        for (uint i = 0; i < count; i++)
        {
            cstring label = plane == 1 ? cfg.vlabel
                          : plane == 2 ? cfg.clabel
                                       : builtins[first + 2 * i];
            items(mi, label, type);
        }
    }

    return true;
}


utf8 constant_menu::do_menu_help(constant::config_r cfg,
                                 constant_menu_p    cst) const
// ----------------------------------------------------------------------------
//   Generate the help topic for a given constant menu
// ----------------------------------------------------------------------------
{
    static char buf[64];
    size_t      len  = 0;
    utf8        base = do_name(cfg, cst->type(), len);
    snprintf(buf, sizeof(buf), "%.*s%s", int(len), base, cfg.menu_help);
    return utf8(buf);
}


bool constant::do_collection_menu(constant::config_r cfg, menu_info &mi)
// ----------------------------------------------------------------------------
//   Build the collection menu for the given config
// ----------------------------------------------------------------------------
{
    uint      menuID        = 0;
    uint      infile        = 0;
    uint      inbuiltins    = 0;
    uint      menus         = cfg.last_menu - cfg.first_menu;
    size_t    maxb          = cfg.nbuiltins;
    auto      builtins      = cfg.builtins;
    unit_file cfile(cfg.file);

    // List all top-level menu entries in the file (up to 100)
    if (cfile.valid())
    {
        while (symbol_p mname = cfile.next(true))
        {
            if (*mname->value() != '=')
            {
                if (menuID++ >= menus)
                {
                    record(constants_error,
                           "Too many entries in file %s, %u > %u, %u top level",
                           cfg.file, menuID, menus, infile);
                    break;
                }
                if (path_top_level(mname))
                    infile++;
            }
        }
    }

    // Count built-in top-level constant menu titles
    if (!infile || cfg.show_builtins())
    {
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b + 1] || !*builtins[b + 1])
            {
                if (menuID++ >= menus)
                {
                    record(constants_error,
                           "Too many builtins after %s, %u > %u, "
                           "%u + %u top level",
                           cfg.file, menuID, menus, infile, inbuiltins);
                    break;
                }
                if (path_top_level(builtins[b]))
                    inbuiltins++;
            }
        }
        // If too many file entries, we may need to truncate builtins
        if (infile + inbuiltins > menus)
        {
            record(constants_error,
                   "Too many entries in file %s, %u + %u > %u, truncating",
                   cfg.file, infile, inbuiltins, menus);
            inbuiltins = menus - infile;
        }
    }

    uint mitems = infile + inbuiltins;
    menu::items_init(mi, mitems);
    menuID = 0;
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_p mname = cfile.next(true))
        {
            if (*mname->value() == '=') // Skip =Cycle menu and similar
                continue;
            if (menuID++ >= menus)
                break;
            if (!path_top_level(mname))
                continue;
            menu::items(mi, mname, id(cfg.first_menu + menuID - 1));
        }
    }
    if (!infile || cfg.show_builtins())
    {
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b + 1] || !*builtins[b + 1])
            {
                if (menuID++ >= menus)
                    break;
                if (!path_top_level(builtins[b]))
                    continue;
                menu::items(mi, builtins[b], id(cfg.first_menu + menuID -1));
            }
        }
    }

    return true;
}



// ============================================================================
//
//   Constant-related commands
//
// ============================================================================

constant_p constant::do_key(config_r cfg, int key)
// ----------------------------------------------------------------------------
//   Return a softkey label as a constant value
// ----------------------------------------------------------------------------
{
    size_t   len = 0;
    if (utf8 txt = ui.label_for_function_key(key, &len))
        return do_lookup(cfg, txt, len, true);
    return nullptr;
}


object::result constant::lookup_command(config_r cfg, bool numerical)
// ----------------------------------------------------------------------------
//   Process a command that looks up in the given config (e.g. CONST)
// ----------------------------------------------------------------------------
{
    object_p name = rt.top();
    if (object_p sym = name->as_quoted(ID_object))
        name = sym;

    size_t len = 0;
    utf8   txt = nullptr;
    id     ty  = name->type();
    if (ty == ID_constant               ||
        ty == ID_standard_uncertainty   ||
        ty == ID_relative_uncertainty)
    {
        txt = constant_p(name)->name(&len);
    }
    else if (ty == ID_symbol || ty == ID_text)
    {
        txt = text_p(name)->value(&len);
    }
    else
    {
        rt.type_error();
        return ERROR;
    }

    if (constant_p cst = constant::do_lookup(cfg, txt, len, false))
    {
        if (object_p value = cst->do_value(cfg))
        {
            if (numerical)
            {
                if (array_p a = value->as<array>())
                {
                    if (object_p item = a->at(cst->value_index()))
                        value = item;
                }
                else if (cst->value_index() != 0)
                {
                    value = integer::make(0);
                    if (!value)
                        return ERROR;
                }

                if (expression_p expr = value->as<expression>())
                {
                    settings::SaveNumericalResults snr(true);
                    value = expr->evaluate();
                }
                else if (unit_p u = unit::get(value))
                {
                    if (algebraic_g expr = u->value()->as<expression>())
                    {
                        object_g gcvalue = value;
                        algebraic_g uexpr = u->uexpr();
                        if (algebraic::to_decimal(expr))
                            gcvalue = unit::simple(expr, uexpr);
                        value = gcvalue;
                    }
                }
            }
            if (rt.top(value))
                return OK;
        }
    }

    cfg.error();
    return ERROR;
}


object_p constant::lookup_menu(config_r cfg, utf8 name, size_t len)
// ----------------------------------------------------------------------------
//   Find the menu in the current configuratoin
// ----------------------------------------------------------------------------
{
    unit_file cfile(cfg.file);
    size_t    maxb     = cfg.nbuiltins;
    auto      builtins = cfg.builtins;
    uint      idx      = cfg.first_menu;

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_p category = cfile.next(true))
        {
            size_t clen = 0;
            utf8 ctxt = category->value(&clen);
            if (len == clen && memcmp(name, ctxt, len) == 0)
                return object::static_object(id(idx));
            idx++;
        }
    }

    // Check built-in constants
    for (size_t b = 0; b < maxb; b += 2)
    {
        if (!builtins[b+1] || !*builtins[b+1])
        {
            cstring ctxt = builtins[b];
            if (ctxt[len] == 0 && memcmp(ctxt, name, len) == 0)
                return object::static_object(id(idx));
            idx++;
        }
    }

    return nullptr;
}


object_p constant::lookup_menu(config_r cfg, cstring name)
// ----------------------------------------------------------------------------
//   Find the menu in the current configuratoin
// ----------------------------------------------------------------------------
{
    return lookup_menu(cfg, utf8(name), strlen(name));
}


object_p constant::cache() const
// ----------------------------------------------------------------------------
//   Cache the constant value in the runtime
// ----------------------------------------------------------------------------
{
    constant_g cst   = this;

    // Clear constant cache if precision changed
    static uint precision = 0;
    if (Settings.Precision() != precision)
    {
        record(constants, "Precision changed from %u to %u",
               precision, Settings.Precision());
        rt.constants(0);
        precision = Settings.Precision();
    }

    uint       idx   = cst->index();
    object_g   value = rt.constant(idx);
    if (!value)
    {
        // Resize the cache if needed
        if (idx >= rt.constants())
            if (!rt.constants(idx+1))
                return nullptr;;

        save<bool> nodates(unit::nodates, true);
        value = cst->do_value(constants);
        rt.constant(idx, value);

        if (value)
            if (algebraic_g alg = value->as_extended_algebraic())
                if (to_decimal(alg, true))
                    value = +alg;
        if (!value)
        {
            if (!rt.error())
                rt.invalid_constant_error();
            return nullptr;
        }
        rt.constant(idx, value);
        cleaner::disable();
    }
    return value;
}


object_p constant::uncache() const
// ----------------------------------------------------------------------------
//   Remove the cached value from the runtime
// ----------------------------------------------------------------------------
{
    constant_g cst   = this;
    uint       idx   = cst->index();
    if (idx < rt.constants())
        rt.constant(idx, nullptr);
    return +cst;
}



// ============================================================================
//
//   Standard and relative uncertainty
//
// ============================================================================

const constant::config standard_uncertainty::standard =
// ----------------------------------------------------------------------------
//  Define the configuration for the standard uncertainty of constants
// ----------------------------------------------------------------------------
{
    .menu_help      = " Constants",
    .help           = " Constant",
    .prefix         = L'Ⓢ',
    .type           = ID_standard_uncertainty,
    .first_menu     = ID_ConstantsMenu00,
    .last_menu      = ID_ConstantsMenu99,
    .name           = ID_constant_menu_name,
    .value          = ID_constant_menu_value,
    .command        = ID_constant_menu_range,
    .file           = "config/constants.csv",
    .library        = "library",
    .builtins       = basic_constants,
    .nbuiltins      = sizeof(basic_constants) / sizeof(*basic_constants),
    .vlabel         = "StdUnc",
    .clabel         = nullptr,
    .error          = invalid_constant_error,
    .show_builtins  = show_builtin_constants,
    .stack_prefix   = true,
    .ignore_case    = false,
};


SIZE_BODY(standard_uncertainty)
// ----------------------------------------------------------------------------
//   Compute the size
// ----------------------------------------------------------------------------
{
    object_p p = object_p(payload(o));
    p += leb128size(p);
    return byte_p(p) - byte_p(o);
}


PARSE_BODY(standard_uncertainty)
// ----------------------------------------------------------------------------
//    Skip, the actual parsing is done in the symbol parser
// ----------------------------------------------------------------------------
{
    return do_parsing(standard, p);
}


RENDER_BODY(standard_uncertainty)
// ----------------------------------------------------------------------------
//   Render the constant into the given constant buffer
// ----------------------------------------------------------------------------
{
    return do_rendering(standard, o, r);
}


const constant::config relative_uncertainty::relative =
// ----------------------------------------------------------------------------
//  Define the configuration for the relative uncertainty of constants
// ----------------------------------------------------------------------------
{
    .menu_help      = " Constants",
    .help           = " Constant",
    .prefix         = L'Ⓡ',
    .type           = ID_relative_uncertainty,
    .first_menu     = ID_ConstantsMenu00,
    .last_menu      = ID_ConstantsMenu99,
    .name           = ID_constant_menu_name,
    .value          = ID_constant_menu_value,
    .command        = ID_object,
    .file           = "config/constants.csv",
    .library        = "library",
    .builtins       = basic_constants,
    .nbuiltins      = sizeof(basic_constants) / sizeof(*basic_constants),
    .vlabel         = "RelUnc",
    .clabel         = nullptr,
    .error          = invalid_constant_error,
    .show_builtins  = show_builtin_constants,
    .stack_prefix   = true,
    .ignore_case    = false,
};


SIZE_BODY(relative_uncertainty)
// ----------------------------------------------------------------------------
//   Compute the size
// ----------------------------------------------------------------------------
{
    object_p p = object_p(payload(o));
    p += leb128size(p);
    return byte_p(p) - byte_p(o);
}


PARSE_BODY(relative_uncertainty)
// ----------------------------------------------------------------------------
//    Skip, the actual parsing is done in the symbol parser
// ----------------------------------------------------------------------------
{
    return do_parsing(relative, p);
}


RENDER_BODY(relative_uncertainty)
// ----------------------------------------------------------------------------
//   Render the constant into the given constant buffer
// ----------------------------------------------------------------------------
{
    return do_rendering(relative, o, r);
}
