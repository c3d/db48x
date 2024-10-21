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
//   Parsing the constant from teh constant file
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
    algebraic_g value = o->value();
    return rt.push(+value) ? OK : ERROR;
}


HELP_BODY(constant)
// ----------------------------------------------------------------------------
//   Help topic for constants
// ----------------------------------------------------------------------------
{
    return o->do_instance_help(constant::constants);
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
    return constant::do_collection_menu(constant::constants, mi);
}


utf8 constant_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name for a menu entry
// ----------------------------------------------------------------------------
{
    return do_name(constant::constants, type, len);
}


COMMAND_BODY(ConstantName)
// ----------------------------------------------------------------------------
//   Put the name of a constant on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = constant::do_key(constant::constants, key))
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
    return ui.insert_softkey(key, " Ⓒ", " ", false);
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
            if (object_p value = cst->value())
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
//
// ============================================================================

static const cstring basic_constants[] =
// ----------------------------------------------------------------------------
//   List of basic constants
// ----------------------------------------------------------------------------
//   clang-format off
{
    // ------------------------------------------------------------------------
    // MATH CONSTANTS MENU
    // ------------------------------------------------------------------------
    "Mathematics",   nullptr,

    "π",        "3.14159",              // Evaluated specially (decimal-pi.h)
    "e",        "2.71828",              // Evaluated specially (decimal-e.h)
    "ⅈ",        "0+ⅈ1",                 // Imaginary unit
    "∞",        "9.99999E999999",       // A small version of infinity
    "?",        "Undefined",            // Undefined result

    "ⅉ",        "0+ⅈ1",                 // Imaginary unit
    "rad",      "1_r",                  // One radian
    "twoπ",     "'2*Ⓒπ'_r",             // Two pi radian
    "angl",     "180_°",                // Half turn


    // ------------------------------------------------------------------------
    //   Chemistry
    // ------------------------------------------------------------------------

    "Chemistry",     nullptr,

    "NA",       "6.0221367E23_mol⁻¹",   // Avogradro's number
    "k",        "1.380658E-23_J/K",     // Boltzmann
    "Vm",       "22.4141_mol⁻¹",        // Molar volume
    "R",        "8.31451_J/(mol*K)",    // Universal gas constant
    "StdT",     "273.15_K",             // Standard temperature
    "StdP",     "101.325_kPa",          // Standard temperature
    "σ",        "5.67051E-8_W/(m^2*K^4)", // Stefan-Boltzmann

    // ------------------------------------------------------------------------
    //   Physics
    // ------------------------------------------------------------------------

    "Physics",     nullptr,

    "ⅉ",        "0+ⅈ1",                 // Imaginary unit in physics
    "c",        "299792458_m/s",        // Speed of light
    "ε0",       "8.85418781761E-12_F/m",// Vaccuum permittivity
    "μ0",       "1.25663706144E-6_H/m", // Vaccuum permeability
    "g",        "9.80665_m/s²",         // Acceleration of Earth gravity
    "G",        "6.67259E-11_m^3/(s^2*kg)",// Gravitation constant
    "h",        "6.6260755E-34_J*s",    // Planck
    "ℏ",        "1.05457266E-34_J*s",   // Dirac
    "qe",       "1.602176634E-19_C",    // Electronic charge
    "me",       "9.1093837139E-31_kg",  // Electron mass
    "mn",       "1.674927471E-27_kg",   // Neutron mass
    "mp",       "1.67262192595E-27_kg", // Proton mass
    "mH",       "1.007825_u",           // Hydrogen mass
    "u",        "1.66053906892E-27_kg", // Mass unit
    "Da",       "1.66053906892E-27_kg", // Mass unit (Dalton)
    "qme",      "175881962000_C/kg",    // q/me ratio
    "mpme",     "1836.152701",          // mp/me ratio
    "α",        "0.00729735308",        // fine structure
    "ø",        "2.06783461E-15_Wb",    // Magnetic flux quantum
    "F",        "96485.309_C/mol",      // Faraday
    "R∞",       "10973731.534_m⁻¹",     // Rydberg
    "a0",       "0.0529177249_nm",      // Bohr radius
    "μB",       "9.2740154E-24_J/T",    // Bohr magneton
    "μN",       "5.0507837393E-27_J/T",    // Nuclear magneton
    "λ0",       "1239.8425_nm",         // Photon wavelength
    "f0",       "2.4179883E14_Hz",      // Photon frequency
    "λc",       "0.00242631058_nm",     // Compton wavelength
    "c3",       "0.002897756_m*K",      // Wien's
    "kq",       "0.00008617386_J/(K*C)",// k/q
    "ε0q",      "55263469.6_F/(m*C)",   // ε0/q
    "qε0",      "1.4185978E-30_F*C/m",  // q*ε0
    "εsi",      "11.9",                 // Dielectric constant
    "εox",      "3.9",                  // SiO2 dielectric constant
    "I0",       "0.000000000001_W/m^2", // Ref intensity

    // ------------------------------------------------------------------------
    //  Computing
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


const constant::config constant::constants =
// ----------------------------------------------------------------------------
//  Define the configuration for the constants
// ----------------------------------------------------------------------------
{
    .menu_help      = " Constants",
    .help           = " Constant",
    .prefix         = L'Ⓒ',
    .type           = ID_constant,
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
    .show_builtins  = show_builtin_constants
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
    if (file *saving = r.file_save())
    {
        // Saving: can't have two files open at once on DMCP
        file_closer_while_writing fc(*saving);
        txt = cst->do_name(cfg, &len);
    }
    else
    {
        txt = cst->do_name(cfg, &len);
    }
    if (r.editing())
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
        // Need to close the configuration file before we parse the constants
        file_closer fc(cfile);

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
                    {
                        file_closer fc(cfile);
                        mentry = cfg.label(mentry);
                    }
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
                file_closer fc(cfile);
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
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        size_t   len = 0;
        utf8     txt = nullptr;
        symbol_p sym = ui.label(key - KEY_F1);
        if (sym)
        {
            txt = sym->value(&len);
        }
        else if (cstring label = ui.label_text(key - KEY_F1))
        {
            txt = utf8(label);
            len = strlen(label);
        }

        if (txt)
            return do_lookup(cfg, txt, len, true);
    }
    return nullptr;
}


object::result constant::lookup_command(config_r cfg, bool numerical)
// ----------------------------------------------------------------------------
//   Process a command that looks up in the given config (e.g. CONST)
// ----------------------------------------------------------------------------
{
    object_p name = rt.top();
    id ty = name->type();
    if (symbol_p sym = name->as_quoted<symbol>())
    {
        name = sym;
        ty = name->type();
    }
    if (ty != ID_symbol && ty != ID_text)
    {
        rt.type_error();
        return ERROR;
    }

    size_t      len    = 0;
    utf8        txt    = text_p(name)->value(&len);
    if (constant_p cst = constant::do_lookup(cfg, txt, len, false))
    {
        if (object_p value = cst->do_value(cfg))
        {
            if (numerical)
            {
                if (expression_p expr = value->as<expression>())
                {
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
