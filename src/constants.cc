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
#include "expression.h"
#include "file.h"
#include "files.h"
#include "functions.h"
#include "grob.h"
#include "parser.h"
#include "renderer.h"
#include "settings.h"
#include "unit.h"
#include "user_interface.h"
#include "utf8.h"

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


COMMAND_BODY(ConstantName)
// ----------------------------------------------------------------------------
//   Put the name of a constant on the stack
// ----------------------------------------------------------------------------
{
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


INSERT_BODY(ConstantName)
// ----------------------------------------------------------------------------
//   Put the name of a constant in the editor
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    unicode prefix = ui.character_left_of_cursor();
    bool noprefix = prefix == L'Ⓒ' || prefix == L'Ⓡ' || prefix == L'Ⓢ';
    return ui.insert_softkey(key, noprefix ? "" : " Ⓒ", " ", false);
}


HELP_BODY(ConstantName)
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


COMMAND_BODY(ConstantValue)
// ----------------------------------------------------------------------------
//   Put the value of a constant on the stack
// ----------------------------------------------------------------------------
{
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


INSERT_BODY(ConstantValue)
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


HELP_BODY(ConstantValue)
// ----------------------------------------------------------------------------
//   Put the help for a given constant name
// ----------------------------------------------------------------------------
{
    return ConstantName::do_help(nullptr);
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
//   Constant definitions
//   *: Algefrification & Conformity to CODATA2022
// ============================================================================

static const cstring basic_constants[] =
// ----------------------------------------------------------------------------
//   List of basic constants (including standard and relative uncertainty)
// ----------------------------------------------------------------------------
//   clang-format off
{
    // ------------------------------------------------------------------------
    // Mathematics
    // ------------------------------------------------------------------------
    "Mathematics",     nullptr,

    "π",        "3.14159",              // Evaluated specially (decimal-pi.h)
    "e",        "2.71828",              // Evaluated specially (decimal-e.h)
    "ⅈ",        "0+ⅈ1",                 // Imaginary unit
    "∞",        "9.99999E999999",       // A small version of infinity
    "?",        "Undefined",            // Undefined result

    // ------------------------------------------------------------------------
    "ⅉ",        "0+ⅈ1",                 // Imaginary unit
    "rad",      "1_r",                  // One radian
    "twoπ",     "'2*Ⓒπ'_r",            // Two pi radian
    "angl",     "180_°",                // Half turn
    "−∞",       "-9.99999E999999",      // A small version of infinity


    // ------------------------------------------------------------------------
    //   Chemistry
    // ------------------------------------------------------------------------

    "Chemistry",     nullptr,

    // *Avogadro's number - Exact definition
    "NA",       "[ 6.02214076E23_mol⁻¹ "
                "  0_mol⁻¹ "
                "  0 ]",
    // *Boltzmann - Exact definition
    "k",        "[ 1.380649E-23_J/K "
                "  0_J/K "
                "  0 ]",
    // *Molar volume - Calculation convention
    "Vm",       "[ 'CONVERT(ⒸR*ⒸStdT/ⒸStdP;1_m^3/mol)' "
                "  0_m^3/mol "
                "  0 "
                "  2.241E-2_m^3/mol ]",
    // *Universal gas constant - Exact calculation
    "R",        "[ 'CONVERT(ⒸNA*Ⓒk;1_J/(mol*K))' "
                "  0_J/(mol*K) "
                "  0 "
                "  8.314 J/(mol·K) ]",
    // *Stefan-Boltzmann - Exact calculation
    "σ",        "[ 'CONVERT(Ⓒπ²/60*Ⓒk^4/(Ⓒℏ^3*Ⓒc²);1_W/(m²*K^4))' "
                "  0_W/(m²*K^4) "
                "  0 "
                "  5.67⁳⁻⁸ W/(m↑2·K↑4) ]",

    // ------------------------------------------------------------------------
    // *Standard temperature - Definition convention
    "StdT",     "[ 273.15_K "
                "  0_K "
                "  0 ]",
    // *Standard pressure - Definition convention
    "StdP",     "[ 101.325_kPa "
                "  0_kPa "
                "  0 ]",
    // *Molar Mass Constant - Calculation from measurement
    "Mu",       "[ 'ROUND(CONVERT(ⒸNA*Ⓒu;1_g/mol);XPON(ⓇMu*ⒸNA*Ⓒu)-XPON(ⒸNA*Ⓒu)-2)' "
                "  'ROUND(CONVERT(ⓇMu*ⒸMu;1_g/mol);-2)' "
                "  'Ⓡu' "
                "  1 g/mol ]",
    // *C12 Molar Mass - Calculation from measurement
    "MC12",     "[ 'ROUND(CONVERT(12*ⒸMu;1_g/mol);XPON(UVAL(ⓇMC12*12*ⒸMu))-XPON(UVAL(12*ⒸMu))-2)' "
                "  'ROUND(CONVERT(ⓇMC12*ⒸMC12;1_kg/mol);-2)' "
                "  'ⓇMu' "
                "  12 g/mol ]",

    // *Loschmidt constant - Exact calculation
    "n0",       "[ 'CONVERT(ⒸNA/ⒸVm;1_m^-3)' "
                "  0_m^-3 "
                "  0 "
                "  2.687⁳²⁵ (m↑3)⁻¹ ]",

    // ------------------------------------------------------------------------
    // *Sakur-Tetrode constant - Calculation from measurement
    "SoR",      "[ 'ROUND((5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP));XPON(ⓇSoR*(5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP)))-XPON((5/2+LN(UBASE(Ⓒu*Ⓒk*(1_K)/(2*Ⓒπ*Ⓒℏ²))^1.5*Ⓒk*(1_K)/ⒸStdP)))-2)' "
                "  'ROUND(ⓇSoR*ABS(ⒸSoR);-2)' "
                "  4.0E-10 "
                "  -1.165 ]",
    // *Mass unit (Dalton) - Calculation from measurement
    "Da",       "[ 'Ⓒu' "
                "  'Ⓢu' "
                "  'Ⓡu' "
                "  1.661⁳⁻²⁷ kg ]",
    // * kq ratio - Exact calculation
    "kq",       "[ 'CONVERT(Ⓒk/Ⓒqe;1_J/(K*C))' "
                "  0_J/(K*C) "
                "  0 "
                "  8.617⁳⁻⁵ J/(K·C) ]",

    // ------------------------------------------------------------------------
    //   Physics
    // ------------------------------------------------------------------------

    "Physics",      nullptr,

    // *Imaginary unit in physics - Definition convention
    "ⅉ",         "0+ⅈ1",
    // *Speed of light - Exact definition
    "c",        "[ 299792458_m/s "
                "  0_m/s "
                "  0 ]",
    // *Gravitation constant - Measurement
    "G",        "[ 6.67430E-11_m^3/(s²*kg) "
                "  0.00015E-11_m^3/(s²*kg) "
                "  'ROUND(UBASE(ⓈG/ⒸG);-2)' ]",
    // *Acceleration of Earth gravity - Definition convention
    "g",        "[ 9.80665_m/s² "
                "  0_m/s² "
                "  0 ]",
    // *Vacuum characteristic impedance - Calculation from measurement
    "Z₀",       "[ 'ROUND(CONVERT(Ⓒμ₀*Ⓒc;1_Ω);XPON(UVAL(ⓇZ₀*Ⓒμ₀*Ⓒc))-XPON(UVAL(Ⓒμ₀*Ⓒc))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇZ₀*ⒸZ₀);-2);1_Ω)' "
                "  'Ⓡμ₀' "
                "  376.7 Ω ]",

    // ------------------------------------------------------------------------
    // *Vaccuum permittivity - Calculation from measurement
    "ε₀",       "[ 'ROUND(CONVERT(1/(Ⓒμ₀*Ⓒc²);1_F/m);XPON(UVAL(Ⓡε₀/(Ⓒμ₀*Ⓒc²)))-XPON(UVAL(1/(Ⓒμ₀*Ⓒc²)))-2)' "
                "  'ROUND(UBASE(Ⓡε₀*Ⓒε₀);-2)' "
                "  'Ⓡμ₀' "
                "  8.854⁳⁻¹² F/m ]",
    // *Vaccuum permeability - Calculation from measurement
    "μ₀",       "[ 'ROUND(CONVERT(4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc);1_H/m);XPON(UVAL(Ⓡμ₀*4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc)))-XPON(UVAL(4*Ⓒπ*Ⓒα*Ⓒℏ/(Ⓒqe²*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡμ₀*Ⓒμ₀);-2);1_H/m)' "
                "  'Ⓡα' "
                "  1.257⁳⁻⁶ H/m ]",
    // *Coulomb constant - Calculation from measurement
    "ke",      "[ 'ROUND(CONVERT(1/(4*Ⓒπ*Ⓒε₀);1_(N*(m/C)²));XPON(UVAL(Ⓡke/(4*Ⓒπ*Ⓒε₀)))-XPON(UVAL(1/(4*Ⓒπ*Ⓒε₀)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡke*Ⓒke);-2);1_(N*(m/C)²))' "
                "  'Ⓡε₀' "
                "  8.988⁳⁹ N·m↑2/C↑2 ]",

    // ------------------------------------------------------------------------
    //   Particle masses
    // ------------------------------------------------------------------------

    "Mass",     nullptr,

    // ------------------------------------------------------------------------
    // *Electron mass - Calculation from measurement
    "me",       "[ 'ROUND(CONVERT(2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc);1_kg);XPON(UVAL(Ⓡme*2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc)))-XPON(UVAL(2*Ⓒh*ⒸR∞/((Ⓒα²)*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡme*Ⓒme);-2);1_kg)' "
                "  'Ⓡu' "
                "  9.109⁳⁻³¹ kg ]",
    // *Neutron mass - Measurement
    "mn",       "[ 1.67492750056E-27_kg "
                "  0.00000000085E-27_kg "
                "  'ROUND(UBASE(Ⓢmn/Ⓒmn);-2)' ]",
    // *Proton mass - Measurement
    "mp",       "[ 1.67262192595E-27_kg "
                "  0.00000000052E-27_kg "
                "  'ROUND(UBASE(Ⓢmp/Ⓒmp);-2)' ]",
    // *Hydrogen mass - Measurement
    "mH",       "[ 1.00782503223_u "
                "  0.00000000009_u "
                "  'ROUND(UBASE(ⓈmH/ⒸmH);-2)' ]",
    // *Mass unit (u) - Calculation from measurement
    "u",        "[ 'ROUND(CONVERT(Ⓒme/ⒸAre;1_kg);XPON(UVAL(Ⓡu*Ⓒme/ⒸAre))-XPON(UVAL(Ⓒme/ⒸAre))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡu*Ⓒu);-2);1_kg)' "
                "  3.1E-10 "
                "  1.661⁳⁻²⁷ kg ]",

    // ------------------------------------------------------------------------
    // *Deuterium mass - Measurement
    "mD",       "[ 2.01410177812_u "
                "  0.00000000012_u "
                "  'ROUND(UBASE(ⓈmD/ⒸmD);-2)' ]",
    // *Tritium mass - Measurement
    "mT",       "[ 3.0160492779_u "
                "  0.0000000024_u "
                "  'ROUND(UBASE(ⓈmT/ⒸmT);-2)' ]",
    // *Helium atomic mass - Measurement
    "mHe",      "[ 4.00260325413_u "
                "  0.00000000006_u "
                "  'ROUND(UBASE(ⓈmHe/ⒸmHe);-2)' ]",
    // *Muon mass - Measurement
    "mμ",       "[ 0.1134289257_u "
                "  0.0000000025_u "
                "  'ROUND(UBASE(Ⓢmμ/Ⓒmμ);-2)' ]",
    // *Tau mass - Measurement
    "mτ",       "[ 1.90754_u "
                "  0.00013_u "
                "  'ROUND(UBASE(Ⓢmμ/Ⓒmμ);-2)' ]",

    // ------------------------------------------------------------------------
    // *mpme ratio - Measurement
    "mpme",     "[ 1836.152673426 "
                "  0.000000032 "
                "  'ROUND(UBASE(Ⓢmpme/Ⓒmpme);-2)' ]",
    // *Electron relative atomic mass - Measurement
    "Are",       "[ 5.485799090441E-4 "
                "  0.000000000097E-4 "
                "  'ROUND(ⓈAre/ⒸAre;-2)' ]",


    // ------------------------------------------------------------------------
    //   Electromagnetism
    // ------------------------------------------------------------------------

    "Electromagnetism",     nullptr,

    // ------------------------------------------------------------------------
    // *Electronic charge - Exact definition
    "qe",       "[ 1.602176634E-19_C "
                "  0_C "
                "  0 ]",
    // *Photon wavelength - Exact calculation
    "λ0",       "[ 'CONVERT(Ⓒh*Ⓒc/Ⓒqe/(1_V);1_nm)' "
                "  0_nm "
                "  0 ]",
    // *Photon frequency - Exact calculation1 239.84198 43320 03 nm
    "f0",       "[ 'CONVERT(Ⓒc/Ⓒλ0;1_Hz)' "
                "  0_Hz "
                "  0 "
                "  2.418⁳¹⁴ Hz ]",
    // *Electron g-factor - Measurement
    "ge",       "[ -2.00231930436092 "
                "  0.00000000000036 "
                "  'ROUND(UBASE(ABS(Ⓢge/Ⓒge));-2)' ]",
    // *qme ratio - Calculation from measurement
    "qme",      "[ 'ROUND(CONVERT(Ⓒqe/Ⓒme;1_C/kg);XPON(UVAL(Ⓡqme*Ⓒqe/Ⓒme))-XPON(UVAL(Ⓒqe/Ⓒme))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡqme*Ⓒqme);-2);1_C/kg)' "
                "  'Ⓡme' "
                "  1.759⁳¹¹ C/kg ]",

    // ------------------------------------------------------------------------
    // *Electron magnetic moment - Measurement
    "μe",       "[ -9.2847646917E-24_J/T "
                "  0.0000000029E-24_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμe/Ⓒμe));-2)' ]",
    // *Proton magnetic moment - Measurement
    "μp",       "[ 1.41060679545E-26_J/T "
                "  0.00000000060E-26_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμp/Ⓒμp));-2)' ]",
    // *Neutron magnetic moment - Measurement
    "μn",       "[ -9.6623653E-27_J/T "
                "  0.0000023E-27_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμn/Ⓒμn));-2)' ]",
    // *Muon magnetic moment - Measurement
    "μμ",       "[ -4.49044830E-26_J/T "
                "  0.00000010E-26_J/T "
                "  'ROUND(UBASE(ABS(Ⓢμμ/Ⓒμμ));-2)' ]",

    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    //   Particle sizes
    // ------------------------------------------------------------------------

    "Size",     nullptr,

    // *Classical electron radius - Calculation from measurement
    "re",       "[ 'CONVERT(Ⓒα²*Ⓒa0;1_fm)' "
                "  'ROUND(Ⓡre*Ⓒre;-2)' "
                "  'ROUND(3*Ⓢα/α;-2)' "
                "  2.818 fm ]",
    // *Proton charge radius - Measurement
    "rp",       "[ 8.4075-16_m "
                "  0.0064-16_m "
                "  'ROUND(Ⓢrp/Ⓒrp;-2)' ]",
    // *Bohr radius - Calculation from measurement
    "a0",       "[ 'ROUND(CONVERT(4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²);1_nm);XPON(UVAL(Ⓡa0*4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²)))-XPON(UVAL(4*Ⓒπ*Ⓒε₀*Ⓒℏ²/(Ⓒme*Ⓒqe²)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓢα/Ⓒα*Ⓒa0);-2);1_nm)' "
                "  'Ⓡα' "
                "  5.292⁳⁻² nm ]",
    // *Thomson cross-section - Calculation from measurement
    "σe",       "[ 'ROUND(CONVERT(8*Ⓒπ*Ⓒre²/3;1_m²);XPON(UVAL(Ⓡσe*8*Ⓒπ*Ⓒre²/3))-XPON(UVAL(8*Ⓒπ*Ⓒre²/3))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡσe*Ⓒσe);-2);1_m²)' "
                "  'ROUND(6*Ⓢα/Ⓒα;-2)' "
                "  6.652⁳⁻²⁹ m↑2 ]",


    // ------------------------------------------------------------------------
    //    Compton effect
    // ------------------------------------------------------------------------

    "Scattering",   nullptr,
    // ------------------------------------------------------------------------
    // *Electron Compton wavelength - Calculation from measurement
    "λc",       "[ 'ROUND(CONVERT(Ⓒh/(Ⓒme*Ⓒc);1_nm);XPON(UVAL(Ⓡλc*Ⓒh/(Ⓒme*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒme*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλc*Ⓒλc);-2);1_nm)' "
                "  'Ⓡme' "
                "  2.426⁳⁻³ nm ]",
    // *Proton Compton wavelength - Calculation from measurement
    "λcp",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmp*Ⓒc);1_nm);XPON(UVAL(Ⓡλcp*Ⓒh/(Ⓒmp*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmp*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcp*Ⓒλcp);-2);1_nm)' "
                "  'Ⓡmp' "
                "  1.321⁳⁻⁶ nm ]",
    // *Neutron Compton wavelength - Calculation from measurement
    "λcn",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmn*Ⓒc);1_nm);XPON(UVAL(Ⓡλcn*Ⓒh/(Ⓒmn*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmn*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcn*Ⓒλcn);-2);1_nm)' "
                "  'Ⓡmn' "
                "  1.32⁳⁻⁶ nm ]",
    // *Muon Compton wavelength - Calculation from measurement
    "λcμ",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmμ*Ⓒc);1_nm);XPON(UVAL(Ⓡλcμ*Ⓒh/(Ⓒmμ*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmμ*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcμ*Ⓒλcμ);-2);1_nm)' "
                "  'Ⓡmμ' "
                "  1.173⁳⁻⁵ nm ]",
    // *Tau Compton wavelength - Calculation from measurement
    "λcτ",      "[ 'ROUND(CONVERT(Ⓒh/(Ⓒmτ*Ⓒc);1_nm);XPON(UVAL(Ⓡλcτ*Ⓒh/(Ⓒmτ*Ⓒc)))-XPON(UVAL(Ⓒh/(Ⓒmτ*Ⓒc)))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡλcτ*Ⓒλcτ);-2);1_nm)' "
                "  'Ⓡmτ' "
                "  6.978⁳⁻⁷ nm ]",

    // ------------------------------------------------------------------------
    //   Quantum mechanics
    // ------------------------------------------------------------------------

    "Quantum",    nullptr,

    // *Planck - Exact definition
    "h",        "[ 6.62607015E-34_J*s "
                "  0_J*s "
                "  0 ]",
    // *Dirac - Exact definition
    "ℏ",        "[ 'CONVERT(Ⓒh/(2*Ⓒπ);1_J*s)' "
                "  0_J*s "
                "  0 "
                "  1.055⁳⁻³⁴ J·s ]",
    // *fine structure constant - Measurement
    "α",        "[ 0.00729735256434 "
                "  0.00000000000114 "
                "  'ROUND(UBASE(Ⓢα/Ⓒα);-2)' ]",
    // *Cs hyperfine transition - Exact definition
    "ΔfCs",     "[ 9192631770_Hz "
                "  0_Hz "
                "  0 ]",
    // *Weak mixing angle - Measurement
    "θw",       "[ 'ROUND(CONVERT(ASIN(√(0.22305));1_°);XPON(UVAL(Ⓡθw*ASIN(√(0.22305))))-XPON(UVAL(ASIN(√(0.22305))))-2)' "
                   "'CONVERT(ROUND((ASIN(√(0.22305+0.00023))-ASIN(√(0.22305-0.00023)))/2;-2);1_°)' "
                "  'ROUND(UBASE(ABS(Ⓢθw/CONVERT(ASIN(√(0.22305));1_°)));-2)' "
                "  28.18 ° ]"
,

    // ------------------------------------------------------------------------
    // *Planck length - Calculation from measurement
    "Lpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*ⒸG/Ⓒc^3);1_m);XPON(UVAL(ⓇLpl*√(Ⓒℏ*ⒸG/Ⓒc^3)))-XPON(UVAL(√(Ⓒℏ*ⒸG/Ⓒc^3)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇLpl*ⒸLpl);-2);1_m)' "
                "  'ⓇG/2' "
                "  1.616⁳⁻³⁵ m ]",
    // *Planck time - Calculation from measurement
    "Tpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*ⒸG/Ⓒc^5);1_s);XPON(UVAL(ⓇTpl*√(Ⓒℏ*ⒸG/Ⓒc^5)))-XPON(UVAL(√(Ⓒℏ*ⒸG/Ⓒc^5)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇTpl*ⒸTpl);-2);1_s)' "
                "  'ⓇG/2' "
                "  5.391⁳⁻⁴⁴ s ]",
    // *Planck mass - Calculation from measurement
    "Mpl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*Ⓒc/ⒸG);1_kg);XPON(UVAL(ⓇMpl*√(Ⓒℏ*Ⓒc/ⒸG)))-XPON(UVAL(√(Ⓒℏ*Ⓒc/ⒸG)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇMpl*ⒸMpl);-2);1_kg)' "
                "  'ⓇG/2' "
                "  2.176⁳⁻⁸ kg ]",
    // *Planck energy - Calculation from measurement
    "Epl",      "[ 'ROUND(CONVERT(√(Ⓒℏ*Ⓒc^5/ⒸG);1_GeV);XPON(UVAL(ⓇEpl*√(Ⓒℏ*Ⓒc^5/ⒸG)))-XPON(UVAL(√(Ⓒℏ*Ⓒc^5/ⒸG)))-2)' "
                "  'ROUND(CONVERT(ROUND(UBASE(ⓇEpl*ⒸEpl);-2);1_GeV);-2)' "
                "  'ⓇG/2' "
                "  1.221⁳¹⁹ GeV ]",
    // *Planck temperature - Calculation from measurement
    "T°pl",     "[ 'ROUND(CONVERT(√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk;1_K);XPON(UVAL(ⓇT°pl*√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk))-XPON(UVAL(√((Ⓒℏ*Ⓒc^5/ⒸG))/Ⓒk))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇT°pl*ⒸT°pl);-2);1_K)' "
                "  'ⓇG/2' "
                "  1.417⁳³² K ]",

    // ------------------------------------------------------------------------
    // *Hartree energy - Calculation from measurement
    "Eh",       "[ 'ROUND(CONVERT(2*Ⓒh*Ⓒc*ⒸR∞;1_J);XPON(UVAL(ⓇEh*2*Ⓒh*Ⓒc*ⒸR∞))-XPON(UVAL(2*Ⓒh*Ⓒc*ⒸR∞))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇEh*ⒸEh);-2);1_J)' "
                "  'ⓇR∞' "
                "  4.36⁳⁻¹⁸ J ]",

    // ------------------------------------------------------------------------
    //   Quantum mechanics (electro) magnetic effects
    // ------------------------------------------------------------------------

    "Magnetism",      nullptr,

    // ------------------------------------------------------------------------
    // *Bohr magneton - Calculation from measurement
    "μB",       "[ 'ROUND(CONVERT(Ⓒqe*Ⓒℏ/(2*Ⓒme);1_J/T);XPON(UVAL(ⓇμB*Ⓒqe*Ⓒℏ/(2*Ⓒme)))-XPON(UVAL(Ⓒqe*Ⓒℏ/(2*Ⓒme)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇμB*ⒸμB);-2);1_J/T)' "
                "  'Ⓡme' "
                "  9.274⁳⁻²⁴ J/T ]",
    // *Nuclear magneton - Calculation from measurement
    "μN",       "[ 'ROUND(CONVERT(Ⓒqe*Ⓒℏ/(2*Ⓒmp);1_J/T);XPON(UVAL(ⓇμN*Ⓒqe*Ⓒℏ/(2*Ⓒmp)))-XPON(UVAL(Ⓒqe*Ⓒℏ/(2*Ⓒmp)))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇμN*ⒸμN);-2);1_J/T)' "
                "  'Ⓡmp' "
                "  5.051⁳⁻²⁷ J/T ]",
    // *Electron gyromagnetic ratio - Calculation from measurement
    "γe",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμe)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγe*2*ABS(Ⓒμe)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμe)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγe*Ⓒγe);-2);1_(s*T)^-1)' "
                "  'Ⓡμe' "
                "  1.761⁳¹¹ (s·T)⁻¹ ]",
    // *Proton gyromagnetic ratio - Calculation from measurement
    "γp",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμp)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγp*2*ABS(Ⓒμp)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμp)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγp*Ⓒγp);-2);1_(s*T)^-1)' "
                "  'Ⓡμp' "
                "  2.675⁳⁸ (s·T)⁻¹ ]",
    // *Neutron gyromagnetic ratio - Calculation from measurement
    "γn",       "[ 'ROUND(CONVERT(2*ABS(Ⓒμn)/Ⓒℏ;1_(s*T)^-1);XPON(UVAL(Ⓡγn*2*ABS(Ⓒμn)/Ⓒℏ))-XPON(UVAL(2*ABS(Ⓒμn)/Ⓒℏ))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡγn*Ⓒγn);-2);1_(s*T)^-1)' "
                "  'Ⓡμn' "
                "  1.832⁳⁸ (s·T)⁻¹ ]",

    // ------------------------------------------------------------------------
    // *Rydberg - Measurement
    "R∞",       "[ 10973731.568157_m⁻¹ "
                "  0.000012_m⁻¹ "
                "  'ROUND(UBASE(ⓈR∞/ⒸR∞);-2)' ]",
    // *von Klitzing constant - Exact calculation
    "Rk",       "[ 'CONVERT(2*Ⓒπ*Ⓒℏ/Ⓒqe²;1_Ω)' "
                "  0_Ω "
                "  0 "
                "  25 813. Ω ]",
    // *Faraday constant - Exact calculation
    "F",        "[ 'CONVERT(ⒸNA*Ⓒqe;1_C/mol)' "
                "  0_C/mol "
                "  0 "
                "  96 485. C/mol ]",
    // *Conductance quantum - Exact calculation
    "G0",       "[ 'CONVERT(Ⓒqe²/(Ⓒπ*Ⓒℏ);1_S)' "
                "  0_S "
                "  0 "
                "  7.748⁳⁻⁵ S ]",
    // *Fermi reduced coupling constant - Measurement
    "G0F",       "[ 1.1663787E-5_GeV^-2 "
                "  0.0000006E-5_GeV^-2 "
                "  'ROUND(UBASE(ABS(ⓈG0F/ⒸG0F));-2)' ]",

    // ------------------------------------------------------------------------
    // *First radiation constant - Exact calculation
    "c1",       "[ 'CONVERT(2*Ⓒπ*Ⓒh*Ⓒc²;1_(W*m²))' "
                "  0_(W*m²) "
                "  0 "
                "  3.742⁳⁻¹⁶ W·m↑2 ]",
    // *Second radiation constant - Exact calculation
    "c2",       "[ 'CONVERT(Ⓒh*Ⓒc/Ⓒk;1_(m*K))' "
                "  0_(m*K) "
                "  0 "
                "  0.0144 m·K ]",
    // *Wien's constant - Theory approximation
    "c3",       "[ 2.897771955185172661478605448092885_mm*K "
                "  0_mm*K "
                "  0 ]",
    // *Wien's frequency constant - Theory approximation
    "c3f",      "[ 0.05878925757646824946606130795309722_THz/K "
                "  0_THz/K "
                "  0 ]",
    // *Magnetic flux quantum - Exact calculation
    "ø",        "[ 'CONVERT(Ⓒπ*Ⓒℏ/Ⓒqe;1_Wb)' "
                "  0_Wb "
                "  0 "
                "  2.068⁳⁻¹⁵ Wb ]",

    // ------------------------------------------------------------------------
    // *Josephson constant - Exact calculation
    "KJ",       "[ 'CONVERT(2*Ⓒqe/Ⓒh;1_Hz/V)' "
                "  0_Hz/V "
                "  0 "
                "  4.836⁳¹⁴ Hz/V ]",
    // *Quantum of circulation - Calculation from measurement
    "Kc",       "[ 'ROUND(CONVERT(Ⓒπ*Ⓒℏ/Ⓒme;1_m²/s);XPON(UVAL(ⓇKc*Ⓒπ*Ⓒℏ/Ⓒme))-XPON(UVAL(Ⓒπ*Ⓒℏ/Ⓒme))-2)' "
                "  'CONVERT(ROUND(UBASE(ⓇKc*ⒸKc);-2);1_m²/s)' "
                "  'Ⓡme' "
                "  3.637⁳⁻⁴ m↑2/s ]",

    // ------------------------------------------------------------------------
    //  Materials
    // ------------------------------------------------------------------------

    "Materials",     nullptr,

    // * ε₀q ratio - Calculation from measurement
    "ε₀q",      "[ 'ROUND(CONVERT(Ⓒε₀/Ⓒqe;1_F/(m*C));XPON(UVAL(Ⓡε₀q*Ⓒε₀/Ⓒqe))-XPON(UVAL(Ⓒε₀/Ⓒqe))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡε₀q*Ⓒε₀q);-2);1_F/(m*C))' "
                "  'Ⓡε₀' "
                "  5.526⁳⁷ F/(m·C) ]",
    // * qε₀ product - Calculation from measurement
    "qε₀",      "[ 'ROUND(CONVERT(Ⓒqe*Ⓒε₀;1_F*C/m);XPON(UVAL(Ⓡqε₀*Ⓒqe*Ⓒε₀))-XPON(UVAL(Ⓒqe*Ⓒε₀))-2)' "
                "  'CONVERT(ROUND(UBASE(Ⓡqε₀*Ⓒqε₀);-2);1_F*C/m)' "
                "  'Ⓡε₀' "
                "  1.419⁳⁻³⁰ F·C/m ]",
    // *Dielectric constant - Definition convention
    "εsi",      "[ 11.9 "
                "  0 "
                "  0 ]",
    // *SiO2 dielectric constant - Definition convention
    "εox",      "[ 3.9 "
                "  0 "
                "  0 ]",

    // ------------------------------------------------------------------------
    // *Ref intensity - Definition convention
    "I₀",       "[ 0.000000000001_W/m² "
                "  0_W/m² "
                "  0 ]",

    // ------------------------------------------------------------------------
    //  Comp
    // ------------------------------------------------------------------------
    "Computing",   nullptr,

    "No",                       "False",                // No value = false
    "Yes",                      "True",                 // Yes value = true
    "UnixEpoch",                "19700101_date",
    "SinclairZX81RAM",          "1_KiB",
    "PageSize",                 "4_KiB",
    "HelloWorld",               "\"Hello World\""
};
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


static symbol_p constant_label(symbol_r sym)
// ----------------------------------------------------------------------------
//   Compute numerical value of equation
// ----------------------------------------------------------------------------
{
    if (sym)
    {
        size_t   len    = 0;
        utf8     source = sym->value(&len);
        if (object_p obj = object::parse(source, len))
        {
            if (array_p a = obj->as<array>())
            {
                uint idx = 0;
                for (object_p disp : *a)
                {
                    if (idx == 0 || idx == 3)
                        obj = disp;
                    idx++;
                }
            }
            if (expression_p expr = obj->as<expression>())
            {
                settings::SaveNumericalResults snr(true);
                if (object_p evaluated = expr->evaluate())
                    obj = evaluated;
            }
            settings::SaveDisplayDigits sdd(4);
            if (symbol_p ssym = obj->as_symbol(false))
                return ssym;
        }
    }
    return sym;
}


const constant::config constant::constants =
// ----------------------------------------------------------------------------
//  Define the configuration for the constants
// ----------------------------------------------------------------------------
{
    .menu_help     = " Constants",
    .help          = " Constant",
    .prefix        = L'Ⓒ',
    .type          = ID_constant,
    .first_menu    = ID_ConstantsMenu00,
    .last_menu     = ID_ConstantsMenu99,
    .name          = ID_ConstantName,
    .value         = ID_ConstantValue,
    .command       = ID_object,
    .file          = "config/constants.csv",
    .library       = "library",
    .builtins      = basic_constants,
    .nbuiltins     = sizeof(basic_constants) / sizeof(*basic_constants),
    .error         = invalid_constant_error,
    .label         = constant_label,
    .show_builtins = show_builtin_constants,
    .stack_prefix  = false,
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
    unicode cp = utf8_codepoint(source);
    if (cp != cfg.prefix)
        return SKIP;
    parsed = utf8_next(source, parsed, max);
    size_t first = parsed;

    // Other characters must be alphabetic
    while (parsed < max && is_valid_in_constant_name(source + parsed))
        parsed = utf8_next(source, parsed, max);
    if (parsed <= first)
        return SKIP;

    size_t     len = parsed - first;
    constant_p cst = do_lookup(cfg, source + first, len, true);
    p.length       = parsed;
    p.out          = cst;
    return cst ? OK : ERROR;
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
    cstring   ctxt     = nullptr;
    size_t    clen     = 0;
    uint      idx      = 0;

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_g category = cfile.next(true))
        {
            while (symbol_p name = cfile.next(false))
            {
                ctxt = cstring(name->value(&clen));

                // Constant name comparison is case-sensitive
                if (len == clen && memcmp(txt, ctxt, len) == 0)
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
            ctxt = builtins[b];
            if (ctxt[len] == 0 && memcmp(ctxt, txt, len) == 0)
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
        while (symbol_g category = cfile.next(true))
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

    // Check in-file constants
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_g category = cfile.next(true))
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
    size_t len = 0;
    utf8 base = do_name(cfg, &len);
    snprintf(buf, sizeof(buf), "%.*s%s", int(len), base, cfg.help);
    return utf8(buf);
}





// ============================================================================
//
//   Build a constants menu
//
// ============================================================================

utf8 constant_menu::do_name(constant::config_r cfg, id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name associated with the type
// ----------------------------------------------------------------------------
{
    uint count = type - cfg.first_menu;
    unit_file cfile(cfg.file);

    // List all preceding entries
    if (cfile.valid())
        while (symbol_p mname = cfile.next(true))
            if (*mname->value() != '=')
                if (!count--)
                    return mname->value(&len);

    if (cfg.show_builtins())
    {
        size_t maxb     = cfg.nbuiltins;
        auto   builtins = cfg.builtins;
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b+1] || !*builtins[b+1])
            {
                if (!count--)
                {
                    len = strlen(builtins[b]);
                    return utf8(builtins[b]);
                }
            }
        }
    }

    return nullptr;
}


bool constant_menu::do_submenu(constant::config_r cfg, menu_info &mi) const
// ----------------------------------------------------------------------------
//   Load the menu from a file
// ----------------------------------------------------------------------------
{
    // Use the constants loaded from the constants file
    unit_file cfile(cfg.file);
    size_t    matching = 0;
    uint      position = 0;
    uint      count    = 0;
    id        type     = this->type();
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

    items_init(mi, count + matching, 2, 1);

    // Insert the built-in constants after the ones from the file
    uint skip     = mi.skip;
    uint planes   = 1 + !!cfg.value + !!cfg.command;
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
            if (plane == 1)
            {
                while (symbol_g mentry = cfile.next(false))
                {
                    uint posafter = cfile.position();
                    size_t mlen = 0;
                    utf8 mtxt = mentry->value(&mlen);
                    cfile.seek(position);
                    mentry = cfile.lookup(mtxt, mlen, false, false);
                    if (cfg.label)
                        mentry = cfg.label(mentry);
                    cfile.seek(posafter);
                    if (mentry)
                        items(mi, mentry, type);
                }
            }
            else
            {
                while (symbol_g mentry = cfile.next(false))
                    items(mi, mentry, type);
            }
        }
        for (uint i = 0; i < count; i++)
        {
            cstring   label = builtins[first + 2 * i + plane % 2];
            if (plane == 1 && cfg.label)
            {
                symbol_g mentry = symbol::make(label);
                mentry = cfg.label(mentry);
                items(mi, mentry, type);
            }
            else
            {
                items(mi, label, type);
            }
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
    size_t len = 0;
    utf8 base = do_name(cfg, cst->type(), len);
    snprintf(buf, sizeof(buf), "%.*s%s", int(len), base, cfg.menu_help);
    return utf8(buf);
}


bool constant::do_collection_menu(constant::config_r cfg, menu_info &mi)
// ----------------------------------------------------------------------------
//   Build the collection menu for the given config
// ----------------------------------------------------------------------------
{
    uint      infile   = 0;
    uint      count    = 0;
    uint      maxmenus = cfg.last_menu - cfg.first_menu;
    size_t    maxb     = cfg.nbuiltins;
    auto      builtins = cfg.builtins;
    unit_file cfile(cfg.file);

    // List all menu entries in the file (up to 100)
    if (cfile.valid())
        while (symbol_p mname = cfile.next(true))
            if (*mname->value() != '=')
                if (infile++ >= maxmenus)
                    break;

    // Count built-in constant menu titles
    if (!infile || cfg.show_builtins())
    {
        for (size_t b = 0; b < maxb; b += 2)
            if (!builtins[b+1] || !*builtins[b+1])
                count++;
        if (infile + count > maxmenus)
            count = maxmenus - infile;
    }

    menu::items_init(mi, infile + count);
    infile = 0;
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_p mname = cfile.next(true))
        {
            if (*mname->value() == '=')
                continue;
            if (infile >= maxmenus)
                break;
            menu::items(mi, mname, id(cfg.first_menu + infile++));
        }
    }
    if (!infile || cfg.show_builtins())
    {
        for (size_t b = 0; b < maxb; b += 2)
        {
            if (!builtins[b+1] || !*builtins[b+1])
            {
                if (infile >= maxmenus)
                    break;
                menu::items(mi, builtins[b], id(cfg.first_menu + infile++));
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
        while (symbol_g category = cfile.next(true))
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
//   Remove teh cached value from the runtime
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
    .name           = ID_ConstantName,
    .value          = ID_ConstantValue,
    .command        = ID_object,
    .file           = "config/constants.csv",
    .library        = "library",
    .builtins       = basic_constants,
    .nbuiltins      = sizeof(basic_constants) / sizeof(*basic_constants),
    .error          = invalid_constant_error,
    .label          = nullptr,
    .show_builtins  = show_builtin_constants,
    .stack_prefix  = true,
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
    .name           = ID_ConstantName,
    .value          = ID_ConstantValue,
    .command        = ID_object,
    .file           = "config/constants.csv",
    .library        = "library",
    .builtins       = basic_constants,
    .nbuiltins      = sizeof(basic_constants) / sizeof(*basic_constants),
    .error          = invalid_constant_error,
    .label          = nullptr,
    .show_builtins  = show_builtin_constants,
    .stack_prefix   = true,
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
