// ****************************************************************************
//  unit.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Unit objects represent objects such as 1_km/s.
//
//    The representation is an equation where the outermost operator is _
//    which is different from the way the HP48 does it, but simplify
//    many other operations
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

#include "unit.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "compare.h"
#include "datetime.h"
#include "expression.h"
#include "file.h"
#include "functions.h"
#include "grob.h"
#include "integer.h"
#include "parser.h"
#include "renderer.h"
#include "settings.h"
#include "tag.h"
#include "user_interface.h"
#include "variables.h"


RECORDER(units,         16, "Unit objects");
RECORDER(units_error,   16, "Error on unit objects");


PARSE_BODY(unit)
// ----------------------------------------------------------------------------
//    Try to parse this as an unit
// ----------------------------------------------------------------------------
{
    // Check if we have a real part
    algebraic_g uval = algebraic_p(+p.out);
    if (!uval)
        return ERROR;

    size_t max = p.length;
    if (!max)
        return SKIP;

    // First character must be compatible with a unit
    size_t  offs  = 0;
    unicode cp    = p.separator;
    bool    umark = cp == '_' || cp == settings::SPACE_UNIT;
    if (!umark)
        return SKIP;
    offs = utf8_next(p.source, offs, max);
    size_t   usz  = max - offs;
    object_p uobj = parse_uexpr(p.source + offs, usz);
    if (!uobj)
        return ERROR;
    algebraic_g uexpr = uobj->as_algebraic();
    if (!uexpr)
        return SKIP;
    offs += usz;

    p.out    = unit::simple(uval, uexpr);
    p.length = offs;
    return p.out ? OK : ERROR;
}


algebraic_p unit::parse_uexpr(gcutf8 source, size_t &plen)
// ----------------------------------------------------------------------------
//  Parse a uexpr as an expression without quotes
// ----------------------------------------------------------------------------
{
    save<bool> save(unit::mode, true);
    uint       parens = 0;
    size_t     len    = plen;
    for (size_t offs = 0; offs < len; offs = utf8_next(source, offs))
    {
        unicode cp = utf8_codepoint(+source + offs);
        if (cp == '(')
            parens++;
        else if (cp == ')')
            parens--;
        if (is_separator(cp))
        {
            if (cp != '(' && (cp != ')' || parens + 1 == 0))
            {
                len = offs;
                break;
            }
        }
    }
    parser p(source, len, MULTIPLICATIVE);
    object::result result = list::list_parse(ID_expression, p, 0, 0);
    if (result == object::OK)
    {
        if (algebraic_p alg = p.out->as_algebraic())
        {
            plen = p.length;
            if (!alg->as_quoted<unit>())
                return alg;
            rt.syntax_error().source(source, p.length);
        }
    }
    return nullptr;
}


unit_p unit::make(algebraic_g v, algebraic_g u, id ty)
// ----------------------------------------------------------------------------
//   Build a unit object from its components
// ----------------------------------------------------------------------------
{
    if (!v || !u)
        return nullptr;

    save<bool> save(unit::mode, true);
    bool more = true;
    while (more)
    {
        more = false;
        unit::mode = false;
        while (unit_g uu = unit::get(u))
        {
            v = uu->value() * v;
            u = uu->uexpr();
            more = true;
        }
        unit::mode = true;
        while (unit_g vu = unit::get(v))
        {
            u = vu->uexpr() * u;
            v = vu->value();
            more = true;
        }
    }
    if (expression_p eq = u->as<expression>())
        u = eq->simplify_products();
    if (!u || !v)
        return nullptr;
    return rt.make<unit>(ty, v, u);
}


algebraic_p unit::simple(algebraic_g v, algebraic_g u, id ty)
// ----------------------------------------------------------------------------
//   Build a unit object from its components, simplify if it ends up numeric
// ----------------------------------------------------------------------------
{
    unit_g uobj = make(v, u, ty);
    if (uobj)
    {
        algebraic_g uexpr = uobj->uexpr();
        if (expression_p eq = uexpr->as<expression>())
            if (object_p q = eq->quoted())
                if (q->is_real())
                    uexpr = algebraic_p(q);
        if (uexpr->is_real())
        {
            algebraic_g uval = uobj->value();
            if (!uexpr->is_one())
                uval = uval * uexpr;
            return uval;
        }
    }
    return uobj;
}


RENDER_BODY(unit)
// ----------------------------------------------------------------------------
//   Do not emit quotes around unit objects
// ----------------------------------------------------------------------------
{
    algebraic_g value = o->value();
    algebraic_g uexpr = o->uexpr();
    size_t      sz    = 0;
    bool        ed    = r.editing();
    if (symbol_p sym = uexpr->as_quoted<symbol>())
    {
        if (value->is_real())
        {
            if (sym->matches("dms"))
                sz = render_dms(r, value, "°", "′", "″");
            else if (sym->matches("hms"))
                sz = ed ? render_dms(r, value, "°", "′", "″")
                    : render_dms(r, value, ":", ":", "");
            else if (sym->matches("date") && !ed)
                sz = render_date(r, value);
            if (sz && !ed)
                return sz;
        }
    }
    bool hide = !ed && value->as_quoted<symbol>();
    if (sz)
    {
        if (!hide)
            r.put('_');
    }
    else
    {
        value->render(r);
        if (!hide)
            r.put(ed ? unicode('_') : unicode(settings::SPACE_UNIT));
    }

    if (!hide)
    {
        save<bool> m(mode, true);
        if (expression_p ueq = uexpr->as<expression>())
            ueq->render(r, false);
        else
            uexpr->render(r);
    }

    return r.size();
}


GRAPH_BODY(unit)
// ----------------------------------------------------------------------------
//   Render units as expressions
// ----------------------------------------------------------------------------
{
    algebraic_g value = o->value();
    algebraic_g uexpr = o->uexpr();
    if (symbol_p sym = uexpr->as_quoted<symbol>())
    {
        if (value->is_real())
        {
            if (sym->matches("dms") ||
                sym->matches("hms") ||
                sym->matches("date"))
            {
                save<bool> sstk(g.stack, true); // Do not render as _dms
                return object::do_graph(o, g);
            }
        }
    }

    bool hide = value->as_quoted<symbol>();
    grob_g result = value->graph(g);
    if (!hide)
    {
        save<bool> sumode(unit::mode, true);
        coord  vr    = g.voffset;
        grob_g ugrob = uexpr->graph(g);
        coord  vu    = g.voffset;
        save<bool> sstk(g.stack, true); // Do not render as _dms
        result = expression::infix(g, vr, result, 0, " ", vu, ugrob);
    }

    return result;
}


EVAL_BODY(unit)
// ----------------------------------------------------------------------------
//   Evaluate the value, and if in unit mode, evaluate the uexpr as well
// ----------------------------------------------------------------------------
{
    algebraic_g value = o->value();
    algebraic_g uexpr = o->uexpr();
    bool skip = value->as_quoted<symbol>();
    {
        save<bool> umode(unit::mode, false);
        value = value->evaluate();
    }
    if (!value)
        return ERROR;
    if (!skip)
    {
        if (unit::mode)
        {
            algebraic_g svu = uexpr;
            uexpr = uexpr->evaluate();
            if (!uexpr)
                return ERROR;
            if (unit_g u = uexpr->as<unit>())
            {
                if (expression_p oe = u->value()->as<expression>())
                {
                    symbol_g name = svu->as<symbol>();
                    if (!name)
                    {
                        // Cannot convert °F/s into °C/min, sorry
                        rt.inconsistent_units_error();
                        return ERROR;
                    }
                    save<symbol_g *> si(expression::independent, &name);
                    object_g         xvalue = +value;
                    save<object_g *> sv(expression::independent_value, &xvalue);
                    save<bool>       sumode(unit::mode, false);
                    uexpr = oe->evaluate();
                    if (!uexpr)
                        return ERROR;
                    uexpr = unit_p(unit::simple(uexpr, u->uexpr()));
                    return uexpr && rt.push(+uexpr) ? OK : ERROR;
                }
            }

            settings::SaveAutoSimplify sas(true);
            while (unit_g u = unit::get(uexpr))
            {
                algebraic_g scale = u->value();
                uexpr = u->uexpr();
                value = scale * value;
            }
        }
        value = unit::simple(value, uexpr);
    }
    return rt.push(+value) ? OK : ERROR;
}


HELP_BODY(unit)
// ----------------------------------------------------------------------------
//   Help topic for units
// ----------------------------------------------------------------------------
{
    return utf8("Units");
}



// ============================================================================
//
//   Unit lookup
//
// ============================================================================

// Evaluating a uexpr
bool unit::mode = false;

// Factoring out a uexpr (limit simplifications)
bool unit::factoring = false;

// Skip date conversions in arithmetic
bool unit::nodates = false;


static const cstring basic_units[] =
// ----------------------------------------------------------------------------
//   List of basic units
// ----------------------------------------------------------------------------
//   The value of these units is taken from Wikipedia.
//   In many cases, e.g. parsec or au, it does not match the HP48
//
//   Units ending with 'US' are the US Survey funny set of units
//   See https://www.northamptonma.gov/740/US-Survey-Foot-vs-Meter and
//   https://www.nist.gov/pml/us-surveyfoot/revised-unit-conversion-factors
//   for details about this insanity.
//   The bottom line is that on January 1, 2023, all US units changed
//   to align to the "metric foot". So all units below have two variants,
//   a US (U.S. Survey, pre 2023) and non US variant. Yadi Yada.
//   The HP48 had a single ftUS unit, which was imprecise, because it did
//   not have fractions to represent it precisely. This unit is the only
//   one kept here. Otherwise, you can use the US unit, e.g. using
//   1_cable*US will give you the U.S. Survey version of the cable.
//
//   clang-format off
{
    // ------------------------------------------------------------------------
    // Length menu
    // ------------------------------------------------------------------------
    "Length",   nullptr,

    // Human scale
    "m",        "1_m",                  // meter, based for SI lengths
    "yd",       "9144/10000_m",         // yard
    "ft",       "3048/10000_m",         // foot
    "ftUS",     "1200/3937_m",          // US survey foot
    "US",       "1_ftUS/ft",            // Conversion factor

    // Small stuff
    "cm",       "=",                    // Centimeter
    "mm",       "=",                    // Millimeter
    "in",       "254/10000_m",          // inch
    "mil",      "254/10000000_m",       // A thousands of a inch (min is taken)
    "μ",        "1_μm",                 // A micron can be written as μ

    // Short travel distance
    "km",       "=",                    // Kilometer
    "mi",       "5280_ft",              // Mile
    "nmi",      "1852_m",               // Nautical mile
    "miUS",     "1_mi*US",              // Mile (US Survey)
    "fur",      "660_ft",               // Furlong

    // US Survey
    "ch",       "66_ft",                // Chain
    "rd",       "1/4_ch",               // Rod, pole, perch
    "cable",    "720_ft",               // Cable's length (US navy)
    "fath",     "6_ft",                 // Fathom
    "league",   "3_mi",                 // League

    // Astronomy
    "Mpc",      "=",                    // Megaparsec
    "pc",       "30856775814913673_m",  // Parsec
    "au",       "149597870700_m",       // Astronomical unit
    "lyr",      "31557600_ls",          // Light year
    "ls",       "299792458_m",          // Light-second

    // US Survey, convert between pre-2023 and post-2023
    "mi",       "=",                    // New mile
    "miUS",     "=",                    // Old mile
    "ft",       "=",                    // New foot
    "ftUS",     "=",                    // Old foot
    "US",       "=",                    // Conversion factor

    // Nautical
    "nmi",      "=",                    // Nautical mile
    "cable",    "=",                    // Cable length
    "li",       "1/100_ch",             // Link
    "acable",   "18532/100_m",          // Cable's length (Imperial/Admiralty)
    "icable",   "1852/10_m",            // Cable's length ("International")

    // Microscopic
    "Å",        "100_pm",               // Angstroem is 100pm, 1E-10m
    "fermi",    "1_fm",                 // fermi is another name for femtometer
    "μm",       "=",                    // Micron
    "nm",       "=",                    // Nanometer

    // Long-name aliases
    "chain",    "1_ch",                 // Chain
    "fathom",   "1_fath",               // Fathom
    "furlong",  "1_fur",                // Furlong
    "link",     "1_li",                 // Link
    "rod",      "1_rd",                 // Alternate spelling
    "pole",     "1_rd",                 // Pole
    "perch",    "1_rd",                 // Perch


    // ------------------------------------------------------------------------
    // Area menu
    // ------------------------------------------------------------------------
    "Area",     nullptr,

    // Human scale
    "m²",       "=",                    // Square meter
    "yd²",      "=",                    // Square yard
    "ft²",      "=",                    // Square foot
    "in²",      "=",                    // Square inch
    "cm²",      "=",                    // Square centimeter

    // Surveying
    "km²",      "=",                    // Square kilometer
    "mi²",      "=",                    // Square mile
    "ha",       "=",                    // Hectare
    "a",        "100_m²",               // Are
    "acre",     "1_ac",                 // Acre

    // US-Survey conversion
    "ac",       "10_ch²",               // Acre
    "acUS",     "10_ch²*US²",           // Acre (pre-2023)
    "mi²",      "=",                    // Square mile
    "miUS²",    "=",                    // Square mile (pre-2023)
    "US²",      "=",                    // Conversion factor

    // Microscopic stuff and aliases
    "b",        "100_fermi²",           // Barn, 1E-28 m^2
    "barn",     "1_b",                  // Barn, 1E-28 m^2
    "mm²",      "=",                    // Square millimeter
    "μm²",      "=",                    // Square micron
    "nm²",      "=",                    // Square nanometer

    // ------------------------------------------------------------------------
    // Volume menu
    // ------------------------------------------------------------------------
    "Volume",   nullptr,

    // Usual
    "m³",       "=",                    // Cubic meter
    "l",        "1_dm³",                // Liter
    "gal",      "231_in³",              // Gallon
    "cm³",      "=",                    // Cubic centimeter
    "mm³",      "=",                    // Cubic millimeter

    // Imperial units
    "gal",      "=",                    // Gallon
    "qt",       "1/4_gal",              // Quart
    "pt",       "1/8_gal",              // Pint
    "cup",      "1/16_gal",             // Cup
    "floz",     "1/32_qt",              // Fluid ounce

    // Human scale
    "m³",       "=",                    // Cubic meter
    "yd³",      "=",                    // Cubic yard
    "ft³",      "=",                    // Cubic foot
    "in³",      "=",                    // Cubic inch
    "cm³",      "=",                    // Cubic centimeter

    // More imperial units
    "gill",     "1/32_gal",             // Gill
    "drqt",     "67200625/1000000_in³",	// US dry quart
    "drgal",    "4_drqt",               // US dry gallon
    "bu",       "32_drqt",              // US dry bushel
    "pk",       "8_drqt",               // US dry peck

    // Other gallons, just because
    "galC",     "4546090_mm³",           // Canadian gallon
    "galUK",    "4546092_mm³",           // UK gallon
    "ptUK",     "1/2_galUK",             // UK pint
    "ozUK",     "1/40_galUK",            // UK fluid ounce
    "fbm",      "1_ft²*in",              // Board foot

    // Other funny volume units
    "tbsp",     "4_oz",                 // Tablespoon
    "tsp",      "1/3_tbsp",             // Teasppon
    "st",       "1_m³",                 // Stere (wood volume)
    "bbl",      "7056_in³",             // Barrel
    "crbl",     "5826_in³",             // Cranberry barrel

    "L",        "l",                    // Liter can be spelled uppercase

    // ------------------------------------------------------------------------
    // Time menu
    // ------------------------------------------------------------------------
    "Time",     nullptr,

    // Basic time units
    "s",        "1_s",                  // Second
    "min",      "60_s",                 // Minute
    "h",        "3600_s",               // Hour
    "d",        "86400_s",              // Day
    "yr",       "36524219/100000_d",    // Mean tropical year

    // Frequencies
    "Hz",       "1_s⁻¹",                // Hertz
    "kHz",      "=",                    // Kilohertz
    "MHz",      "=",                    // Megahertz
    "GHz",      "=",                    // Gigahertz
    "rpm",      "1_turn/min",           // Rotations per minute

    // Alias names for common time units
    "year",     "1_yr",                 // Year
    "day",      "1_d",                  // Day
    "hour",     "1_h",                  // Hour
    "minute",   "1_min",                // Minute
    "second",   "1_s",                  // Second

    "hms",      "1_h",                  // Hour/minutes/seconds to hours

    // ------------------------------------------------------------------------
    // Speed menu
    // ------------------------------------------------------------------------
    "Speed",     nullptr,

    // Standard speed
    "m/s",      "=",                    // Meter per second
    "km/h",     "=",                    // Kilometer per hour
    "ft/s",     "=",                    // Feet per second
    "mph",      "1_mi/h",               // Miles per hour
    "knot",     "1_nmi/h",              // 1 knot is 1 nautical mile per hour

    // Physics
    "c",        "299792458_m/s",        // Speed of light
    "ga",       "980665/100000_m/s^2",  // Standard freefall acceleration
    "G",        "1_ga",                 // Alternate spelling (1_G)
    "kph",      "1_km/h",               // US common spelling for km/h

    // ------------------------------------------------------------------------
    // Mass menu
    // ------------------------------------------------------------------------
    "Mass",     nullptr,

    // Metric units
    "kg",       "1_kg",                 // Kilogram (SI base unit)
    "g",        "1/1000_kg",            // Gram
    "t",        "1000_kg",              // Metric ton
    "ct",       "200_mg",               // Carat
    "mol",      "1_mol",                // Mole (quantity of matter)

    // Imperial units
    "lb",       "45359237/100000_g",    // Avoirdupois pound
    "oz",       "1/16_lb",              // Ounce
    "dr",       "1/256_lb",             // Drachm
    "stone",    "14_lb",                // Stone
    "grain",    "1/7000_lb",            // Grain (sometimes "gr")

    // UK/US conversions
    "qrUK",     "28_lb",                // Quarter (UK)
    "qrUS",     "25_lb",                // Quarter (US)
    "cwtUK",    "112_lb",               // Long hundredweight (UK)
    "cwtUS",    "100_lb",               // Short hundredweight (US)
    "gr",       "1_grain",              // Grain

    "tonUK",    "20_cwtUK",             // Long ton
    "tonUS",    "20_cwtUS",             // Short ton
    "ton",      "1_tonUS",              // Short ton
    "slug",     "1_lbf*s^2/ft",         // Slug (what?)
    "blob",     "12_slug",              // Blob (seriously????)

    // Troy weight system
    "dwt",      "24_grain",             // Pennyweight (Troy weight system)
    "ozt",      "20_dwt",               // Troy ounce
    "lbt",      "12_ozt",               // Troy pound
    "dram",     "1_dr",                 // Alternate spelling
    "drachm",   "1_dr",                 // Alternate spelling

    // Alternate spellings
    "mole",     "1_mol",                // Mole (quantity of matter)
    "carat",    "1_ct",                 // Carat
    "u",        "1.66053906892E-27_kg", // Unified atomic mass
    "Da",       "1.66053906892E-27_kg", // Dalton (Alternative spelling for u)
    "Avogadro", "6.02214076E23",        // Avogadro constant (# units in 1_mol)

    // ------------------------------------------------------------------------
    // Force menu
    // ------------------------------------------------------------------------
    "Force",    nullptr,

    "N",        "1_kg*m/s^2",           // Newton
    "dyn",      "1/100000_N",           // Dyne
    "kip",      "1000_lbf",             // Kilopound-force
    "lbf",      "44482216152605/10000000000000_N",    // Pound-force
    "gf",       "980665/100000000_N",   // Gram-force

    "pdl",      "138254954376/1000000000000_N",       // Poundal

    // ------------------------------------------------------------------------
    // Energy menu
    // ------------------------------------------------------------------------
    "Energy",   nullptr,

    "J",        "1_kg*m^2/s^2",         // Joule
    "erg",      "1/10000000_J",         // erg
    "Kcal",     "=",                    // Large calorie
    "cal",      "41868/10000_J",        // International calorie (1929, 1956)
    "Btu",      "1055.05585262_J",      // British thermal unit

    "calth",    "4184/1000_J",          // Thermochemical Calorie
    "cal4",     "4204/1000_J",          // 4°C calorie
    "cal15",    "41855/10000_J",        // 15°C calorie
    "cal20",    "4182/1000_J",          // 20°C calorie
    "calmean",  "4190/1000_J",          // 4°C calorie

    "therm",    "105506000_J",          // EEC therm
    "eV",       "1.60217733E-19_J",     // electron-Volt

    // ------------------------------------------------------------------------
    // Power menu
    // ------------------------------------------------------------------------
    "Power",    nullptr,

    "W",        "1_J/s",                // Watt
    "kW",       "=",                    // Kilowatt
    "MW",       "=",                    // Megawatt
    "GW",       "=",                    // Gigawatt
    "hp",       "745.699871582_W",      // Horsepower

    // ------------------------------------------------------------------------
    // Fluids menu
    // ------------------------------------------------------------------------
    "Fluids",    nullptr,

    "K",        "1_K",                  // Kelvin
    "°C",       "'(°C+273.15)'_K",      // Celsius
    "°R",       "5/9_K",                // Rankin
    "°F",       "'((°F+459.67)*5/9)'_K",// Fahrenheit
    "°D",       "'(373.15-2/3*°D)'_K",  // Delisle

    "Pa",       "1_N/m^2",              // Pascal
    "atm",      "101325_Pa",            // Atmosphere
    "bar",      "100000_Pa",            // bar
    "psi",      "6894.75729317_Pa",     // Pound per square inch
    "ksi",      "1000_psi",             // Kilopound per square inch

    "kPa",      "=",                    // Kilopascal
    "mmHg",     "1_torr",               // millimeter of mercury
    "inHg",     "1_in/mm*mmHg",         // inch of mercury
    "inH2O",    "249.0889_Pa",          // Inch of H2O
    "torr",     "1/760_atm",            // Torr = 1/760 standard atm

    "m³/yr",    "=",                    // Cubic meters per year
    "m³/d",     "=",                    // Cubic meters per day
    "m³/h",     "=",                    // Cubic meters per hour
    "l/min",    "=",                    // liters per minute
    "l/s",      "=",                    // liters per second

    "cm³/s",    "=",                    // Cubic centimeters per second
    "gal/yr",   "=",                    // Gallons per year
    "gal/d",    "=",                    // Gallons per day
    "gal/min",  "=",                    // Gallons per minute
    "gal/s",    "=",                    // Gallons per second

    "P",        "1/10_Pa*s",            // Poise
    "St",       "1_cm^2/s",             // Stokes

    // ------------------------------------------------------------------------
    // Electricity menu
    // ------------------------------------------------------------------------
    "Electricity",     nullptr,

    "A",        "1_A",                  // Ampere
    "V",        "1_kg*m^2/(A*s^3)",     // Volt
    "C",        "1_A*s",                // Coulomb
    "Ω",        "1_V/A",                // Ohm
    "F",        "1_C/V",                // Farad

    "Fdy",      "96487_A*s",            // Faraday
    "H",        "1_ohm*s",              // Henry
    "S",        "1_A/V",                // Siemens
    "T",        "1_V*s/m^2",            // Tesla
    "Wb",       "1_V*s",                // Weber

    "mho",      "1_S",                  // Ohm spelled backwards
    "ohm",      "1_Ω",                  // Ohm
    "Ω",        "1_Ω",                  // Robustness: Unicode u8486 vs u937

    // ------------------------------------------------------------------------
    // Angles menu
    // ------------------------------------------------------------------------
    "Angle",    nullptr,

    "turn",     "1_turn",               // Full turns
    "°",        "1/360_turn",           // Degree
    "grad",     "1/400_turn",           // Grad
    "r",        "'0.5/Ⓒπ'_turn",       // Radian
    "πr",       "1/2_turn",             // Pi radians

    "dms",      "1_°",                  // Degrees shown as DMS
    "arcmin",   "1/60_°",               // Arc minute
    "arcs",     "1/60_arcmin",          // Arc second
    "sr",       "1_sr",                 // Steradian
    "pir",      "1/2_turn",             // Pi radians

    // ------------------------------------------------------------------------
    // Light/radiation menu
    // ------------------------------------------------------------------------
    "Light",    nullptr,

    "cd",       "1_cd",                 // Candela
    "lm",       "1_cd*sr",              // Lumen
    "lx",       "1_lm/m^2",             // Lux
    "fc",       "1_lm/ft^2",            // Footcandle
    "flam",     "1_cd/ft^2*r/pir",      // Foot-Lambert

    "ph",       "10000_lx",             // Phot
    "sb",       "10000_cd/m^2",         // Stilb
    "lam",      "1_cd/cm^2*r/pir",      // Lambert
    "nit",      "1_cd/m^2",             // Nit
    "nt",       "1_cd/m^2",             // Nit

    "Gy",       "1_m^2/s^2",            // Gray
    "rad",      "1/100_m^2/s^2",        // rad
    "rem",      "1_rad",                // rem
    "Sv",       "1_Gy",                 // Sievert
    "Bq",       "1_Hz",                 // Becquerel (disintegrations/s)

    "Ci",       "37_GBq",               // Curie
    "R",        "258_µC/kg",            // Roentgen

    // ------------------------------------------------------------------------
    // Computing
    // ------------------------------------------------------------------------
    "Computing", nullptr,

    "bit",      "1_bit",                // Bit
    "byte",     "8_bit",                // Byte
    "bps",      "1_bit/s",              // bit per second
    "baud",     "1_bps/SR",             // baud
    "flops",    "1_flops",              // Floating point operation per second

    "KiB",      "=",                    // Usual sizes with powers of two
    "MiB",      "=",                    //
    "GiB",      "=",                    //
    "TiB",      "=",                    //
    "PiB",      "=",                    //

    "KB",       "=",                    // Usual sizes  with powers of 10
    "MB",       "=",                    //
    "GB",       "=",                    //
    "TB",       "=",                    //
    "PB",       "=",                    //

    "B",        "1_byte",               // Byte
    "Bd",       "1_baud",               // baud (standard unit)
    "mips",     "1_mips",               // Million instructions per second
    "SR",       "1",                    // Symbol rate (default is 1)
    "dB",       "1_dB",                  // decibel

    "kbit/s",   "=",
    "Mbit/s",   "=",
    "Gbit/s",   "=",
    "kbaud",    "=",
    "kbps",     "=",

    "Mflops",   "=",
    "Gflops",   "=",
    "Tflops",   "=",
    "Pflops",   "=",
    "Eflops",   "=",
};


struct si_prefix
// ----------------------------------------------------------------------------
//   Representation of a SI prefix
// ----------------------------------------------------------------------------
{
    cstring     prefix;
    int         exponent;
};


static const si_prefix si_prefixes[] =
// ----------------------------------------------------------------------------
//  List of standard SI prefixes
// ----------------------------------------------------------------------------
{
    { "",       0 },                    // No prefix
    { "da",     1 },                    // deca (the only one with 2 letters)
    { "d",     -1 },                    // deci
    { "c",     -2 },                    // centi
    { "h",      2 },                    // hecto
    { "m",     -3 },                    // milli
    { "k",      3 },                    // kilo
    { "K",      3 },                    // kilo (computer-science)
    { "µ",     -6 },                    // micro (0xB5)
    { "μ",     -6 },                    // micro (0x3BC)
    { "u",     -6 },                    // micro (for quick transalpha entry)
    { "M",      6 },                    // mega
    { "n",     -9 },                    // nano
    { "G",      9 },                    // giga
    { "p",    -12 },                    // pico
    { "T",     12 },                    // tera
    { "f",    -15 },                    // femto
    { "P",     15 },                    // peta
    { "a",    -18 },                    // atto
    { "E",     18 },                    // exa
    { "z",    -21 },                    // zepto
    { "Z",     21 },                    // zetta
    { "y",    -24 },                    // yocto
    { "Y",     24 },                    // yotta
    { "r",    -27 },                    // ronna
    { "R",     27 },                    // ronto
    { "q",    -30 },                    // quetta
    { "Q",     30 },                    // quecto
};
//   clang-format on



unit_p unit::lookup(symbol_p namep, int *prefix_info)
// ----------------------------------------------------------------------------
//   Lookup a built-in unit
// ----------------------------------------------------------------------------
{
    symbol_g  name = namep;
    size_t    len  = 0;
    gcutf8    gtxt = namep->value(&len);
    uint      maxs = sizeof(si_prefixes) / sizeof(si_prefixes[0]);
    unit_file ufile;

    record(units, "Lookup %t", name);
    for (uint si = 0; si < maxs; si++)
    {
        utf8    ntxt   = gtxt;
        cstring prefix = si_prefixes[si].prefix;
        size_t  plen   = strlen(prefix);
        if (memcmp(prefix, ntxt, plen) != 0)
            continue;

        int    e       = si_prefixes[si].exponent;
        size_t maxu    = sizeof(basic_units) / sizeof(basic_units[0]);
        size_t maxkibi = 1 + (e > 0 && e % 3 == 0 &&
                              ntxt[plen] == 'i' && len > plen+1);
        for (uint kibi = 0; kibi < maxkibi; kibi++)
        {
            size_t  rlen = len - plen - kibi;
            gcutf8  txt  = +gtxt + plen + kibi;
            cstring utxt = nullptr;
            cstring udef = nullptr;
            size_t  ulen = 0;

            // Check in-file units
            if (ufile.valid())
            {
                bool first = true;
                while (symbol_p def = ufile.lookup(txt, rlen, false, first))
                {
                    first = false;
                    utf8 fdef = def->value(&ulen);

                    // If definition begins with '=', only show unit in menus
                    if (*fdef != '=')
                    {
                        udef = cstring(fdef);
                        utxt = cstring(+txt);
                        break;
                    }
                }
            }

            // Check built-in units
            for (size_t u = 0; !udef && u < maxu; u += 2)
            {
                utxt = basic_units[u];
                if (memcmp(utxt, +txt, rlen) == 0 && utxt[rlen] == 0)
                {
                    udef = basic_units[u + 1];
                    if (udef)
                        ulen  = strlen(udef);
                }
            }

            // If we found a definition, use that unless it begins with '='
            if (udef)
            {
                if (object_p obj = object::parse(utf8(udef), ulen))
                {
                    if (unit_g u = unit::get(obj))
                    {
                        // Record prefix info if we need it
                        if (prefix_info)
                            *prefix_info = kibi ? -si : si;

                        // Apply multipliers
                        if (e)
                        {
                            // Convert SI exp into value, e.g cm-> 1/100
                            // If kibi mode, use powers of 2
                            algebraic_g exp   = integer::make(e);
                            algebraic_g scale = integer::make(10);
                            if (kibi)
                            {
                                scale = integer::make(3);
                                exp = exp / scale;
                                scale = integer::make(1024);
                            }
                            scale = pow(scale, exp);
                            exp = +u;
                            scale = scale * exp;
                            if (scale)
                                if (unit_p us = unit::get(scale))
                                    u = us;
                        }

                        // Check if we have a terminal unit
                        algebraic_g uexpr = u->uexpr();
                        if (symbol_g sym = uexpr->as_quoted<symbol>())
                        {
                            size_t slen = 0;
                            utf8   stxt = sym->value(&slen);
                            if (slen == rlen && memcmp(stxt, utxt, slen) == 0)
                                return u;
                        }

                        // Check if we must evaluate, e.g. 1_min -> seconds
                        ufile.close();

                        settings::SaveAutoSimplify sas(false);
                        settings::SaveNumericalConstants snc(true);
                        save<symbol_g *> si(expression::independent, &name);
                        save<object_g *> sv(expression::independent_value,
                                            nullptr);
                        uexpr = u->evaluate();
                        if (!uexpr || uexpr->type() != ID_unit)
                        {
                            rt.inconsistent_units_error();
                            return nullptr;
                        }
                        u = unit_p(+uexpr);
                        return u;
                    }
                }
            }
        }
    }
    return nullptr;
}



// ============================================================================
//
//   Unit conversion
//
// ============================================================================

bool unit::convert(algebraic_g &x, bool error) const
// ----------------------------------------------------------------------------
//   Convert the object to the given unit
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;

    // If we already have a unit object, perform a conversion
    if (x->type() == ID_unit)
        return convert((unit_g &) x, error);

    // Otherwise, convert to a unity unit
    algebraic_g one = algebraic_p(integer::make(1));
    unit_g u = unit::make(x, one);
    if (!convert(u, error))
        return false;
    x = +u;
    return true;
}


bool unit::convert(unit_g &x, bool error) const
// ----------------------------------------------------------------------------
//   Convert a unit object to the current unit
// ----------------------------------------------------------------------------
{
    if (!x)
        return false;
    algebraic_g u   = uexpr();
    algebraic_g o   = x->uexpr();
    algebraic_g svu = u;

    // Check error case
    if (!u || !o)
        return false;

    // Common case where we have the exact same unit
    if (u->is_same_as(+o))
        return true;

    if (!unit::mode)
    {
        save<bool> sumode(unit::mode, true);

        // Evaluate the unit expression for this one
        u = u->evaluate();
        if (!u)
            return false;

        // Evaluate the unit expression for x
        o = o->evaluate();
        if (!o)
            return false;

        // Check the complicated cases involving expressions, e.g. °C to K
        if (unit_p ou = o->as<unit>())
        {
            if (expression_p oe = ou->value()->as<expression>())
            {
                algebraic_g      ounit = ou->uexpr();
                symbol_g         name  = x->uexpr()->as<symbol>();
                if (!name)
                {
                    // Cannot convert °F/s into °C/min, sorry
                    rt.inconsistent_units_error();
                    return false;
                }
                save<symbol_g *> si(expression::independent, &name);
                object_g         xvalue = x->value();
                save<object_g *> sv(expression::independent_value, &xvalue);
                save<bool>       sumode2(unit::mode, false);
                o = oe->evaluate();
                if (!o)
                    return false;
                x = unit_p(unit::simple(o, ounit));
                o = unit::simple(integer::make(1), ounit);
            }
        }
        // If the expression is in the destination
        if (unit_p uu = u->as<unit>())
        {
            if (expression_g ue = uu->value()->as<expression>())
            {
                // We have an expression like '(°C+273.15)_K'
                // Turn it into K=(°C+273.15), then isolate °C to get
                // °C=K-273.15, and run that the second half
                symbol_g tname = uu->uexpr()->as<symbol>(); // K
                if (!tname)
                {
                    rt.inconsistent_units_error();
                    return false;
                }
                algebraic_g tdef = +tname;
                algebraic_g udef = +ue;
                symbol_g xname = svu->as<symbol>();            // °C
                if (!xname)
                {
                    rt.inconsistent_units_error();
                    return false;
                }
                ue = expression::make(ID_TestEQ, tdef, udef);
                if (!ue)
                {
                    rt.inconsistent_units_error();
                    return false;
                }
                ue = ue->isolate(xname);

                expression_g tmpeq;
                if (ue && ue->split_equation(tmpeq, ue))
                {
                    save<symbol_g *> si(expression::independent, &tname);
                    object_g         xv = x->value();
                    save<object_g *> sv(expression::independent_value, &xv);
                    save<bool> sumode3(unit::mode, false);
                    udef = ue->evaluate();
                    x = unit_p(unit::simple(udef, +xname));
                    u = unit::simple(integer::make(1), +tname);
                }
            }
        }
        if (o)
        {
            // Two numerical values, simply compute conversion factor
            settings::SaveAutoSimplify sas(true);
            o = o / u;
        }

        // Check if this is a unit and if so, make sure the unit is 1
        while (unit_p cf = unit::get(o))
        {
            algebraic_g cfu = cf->uexpr();
            if (!cfu->is_real())
            {
                if (error)
                    rt.inconsistent_units_error();
                return false;
            }
            o = cf->value();
            if (!cfu->is_one(false))
                o = o * cfu;
        }

        if (!o->is_real())
        {
            if (error)
                rt.inconsistent_units_error();
            return false;
        }

        algebraic_g v = x->value();
        {
            settings::SaveAutoSimplify sas(false);
            v = v * o;
        }
        x = unit_p(unit::simple(v, svu)); // Wrong cast, but OK above
        return true;
    }

    // For now, the rest is not implemented
    return false;
}


bool unit::convert_to_linear(algebraic_g &value, algebraic_g &uexpr)
// ----------------------------------------------------------------------------
//   For units like °C, we need to convert to the baseline before computing
// ----------------------------------------------------------------------------
{
    if (symbol_g usym = uexpr->as<symbol>())
    {
        if (unit_g base = lookup(usym))
        {
            if (expression_p be = base->value()->as<expression>())
            {
                algebraic_g      bunit = base->uexpr();
                save<symbol_g *> si(expression::independent, &usym);
                object_g         xvalue = +value;
                save<object_g *> sv(expression::independent_value, &xvalue);
                save<bool>       sumode(unit::mode, false);
                if (algebraic_g cvt = be->evaluate())
                {
                    value = cvt;
                    uexpr = bunit;
                    return true;
                }
            }
        }
    }
    return false;
}


algebraic_p unit::convert_to_real() const
// ----------------------------------------------------------------------------
//   Cnvert a unit to real if possible
// ----------------------------------------------------------------------------
{
    algebraic_g u = this;
    error_save  ers;
    if (algebraic_g one = integer::make(1))
        if (unit_g unity = unit::make(one, one))
            if (unity->convert(u))
                return u;
    return nullptr;
}


unit_p unit::cycle() const
// ----------------------------------------------------------------------------
//   Cycle the unit SI prefix across the closest appropriate ones
// ----------------------------------------------------------------------------
{
    unit_g      u       = this; // GC may move this
    algebraic_g uexpr   = u->uexpr();

    if (symbol_p sym = uexpr->as_quoted<symbol>())
    {
        cstring dunit = nullptr;
        cstring funit = nullptr;
        bool tofrac = false;
        bool todec = false;

        if (sym->matches("dms"))
        {
            tofrac = true;
            dunit = "dms";
            funit = "πr";
        }
        else if (sym->matches("pir") || sym->matches("πr"))
        {
            dunit = "dms";
            funit = "°";
        }
        else if (sym->matches("°"))
        {
            dunit = "πr";
            funit = "grad";
        }
        else if (sym->matches("grad"))
        {
            dunit = "°";
            funit = "r";
        }
        else if (sym->matches("r"))
        {
            funit = "r";
            dunit = "grad";
            todec = true;
        }

        if (funit || dunit)
        {
            algebraic_g uval       = u->value();
            bool        isdec      = !uval->is_fractionable();
            cstring     tunit      = isdec ? dunit : funit;
            symbol_g    tuexpr     = symbol::make(tunit);
            unit_g      targetUnit = unit::make(integer::make(1), +tuexpr);
            if (targetUnit && targetUnit->convert(u))
            {
                if ((tofrac && isdec) || (todec && !isdec))
                {
                    uval = u->value();
                    if (isdec)
                    {
                        if (!arithmetic::to_fraction(uval))
                            return nullptr;
                    }
                    else
                    {
                        if (!arithmetic::decimal_promotion(uval))
                            return nullptr;
                    }
                    u = unit::make(uval, +tuexpr);
                }
                return u;
            }
            return nullptr;
        }
    }

    // Otherwise cycle through SI prefixes
    algebraic_g value   = u->value();
    int         max     = sizeof(si_prefixes) / sizeof(si_prefixes[0]);
    bool        decimal = value->is_decimal();
    bool        frac    = value->is_real() && !decimal;

    // Check if we can cycle through the prefixes
    if (symbol_g sym = uexpr->as_quoted<symbol>())
    {
        // Check if we can find it in a "=Cycle" section in unit file
        if (unit_g converted = custom_cycle(sym))
            return converted;

        int index = 0;
        if (lookup(sym, &index))
        {
            bool kibi = index < 0;
            if (kibi)
                index = -index;
            int         exp       = si_prefixes[index].exponent;
            cstring     opfx      = si_prefixes[index].prefix;
            size_t      olen      = strlen(opfx);
            int         candidate = -1;

            if (decimal)
            {
                // Try to see if we can go up in exponents
                int bexp = -1000;
                for (int i = 0; i < max; i++)
                {
                    int nexp = si_prefixes[i].exponent;
                    if (nexp < exp && nexp > bexp)
                    {
                        candidate = i;
                        bexp = nexp;
                    }
                }
            }
            else if (frac)
            {
                // Fraction: go down until we hit exponent mode
                int bexp = 1000;
                for (int i = 0; i < max; i++)
                {
                    int nexp = si_prefixes[i].exponent;
                    if (nexp > exp && nexp < bexp)
                    {
                        candidate = i;
                        bexp = nexp;
                    }
                }
            }

            if (candidate >= 0)
            {
                cstring  nprefix = si_prefixes[candidate].prefix;
                size_t   oulen   = 0;
                utf8     outxt   = sym->value(&oulen);
                scribble scr;
                renderer r;
                r.put(nprefix);
                r.put(outxt + olen, oulen - olen);
                oulen = r.size();
                algebraic_g nuexpr = parse_uexpr(r.text(), oulen);
                unit_g nunit = unit::make(integer::make(1), nuexpr);
                if (nunit->convert(u))
                {
                    uint16_t    stdxp = Settings.StandardExponent();
                    algebraic_g mag   = integer::make(stdxp);
                    algebraic_g range = integer::make(10);
                    algebraic_g nvalue = u->value();
                    range = pow(range, mag);
                    mag = abs::run(nvalue);

                    if (decimal)
                    {
                        algebraic_g test = mag >= range;
                        if (!test->as_truth(false))
                            if (arithmetic::to_decimal(nvalue))
                                return unit::make(nvalue, nuexpr);
                    }
                    else if (frac)
                    {
                        range = inv::run(range);
                        algebraic_g test = mag <= range;
                        if (!test->as_truth(false))
                            return unit::make(nvalue, nuexpr);
                    }
                }
            }
        }
    }

    // Check if we have a fraction or an integer, if so convert to decimal
    if (frac)
    {
        if (arithmetic::to_decimal(value, true))
            u = unit::make(value, uexpr);
    }
    else if (decimal)
    {
        if (arithmetic::to_fraction(value))
            u = unit::make(value, uexpr);
    }
    return u;
}


unit_p unit::get(object_p obj)
// ----------------------------------------------------------------------------
//   Convert an object to a unit if possible at all
// ----------------------------------------------------------------------------
{
    if (!obj)
        return nullptr;
    obj = strip(obj);
    unit_p u = obj->as_quoted<unit>();
    return u;
}


unit_p unit::get_after_evaluation(object_p obj)
// ----------------------------------------------------------------------------
//   Convert an object to a unit if possible at all, evaluate if necessary
// ----------------------------------------------------------------------------
{
    unit_p u = get(obj);
    if (!u)
    {
        if (algebraic_g alg = obj->as_extended_algebraic())
            if (algebraic::to_decimal(alg, true))
                if (unit_p ue = alg->as_quoted<unit>())
                    return ue;

    }
    return u;
}


unit_p unit::custom_cycle(symbol_r sym) const
// ----------------------------------------------------------------------------
//   If there is an "=Cycle" section in units file, use that
// ----------------------------------------------------------------------------
{
    unit_file ufile;
    if (ufile.valid())
    {
        unit_g from = this;
        if (ufile.lookup(utf8("=Cycle"), sizeof("=Cycle")-1, true))
        {
            size_t sz = 0;
            utf8   txt = sym->value(&sz);
            if (symbol_g found = ufile.lookup(txt, sz, false, false))
            {
                ufile.close();          // Can't have 2 files open on DM42
                unit_g to = unit::make(integer::make(1), +found);
                if (to->convert(from))
                    return from;
            }
        }
    }
    return nullptr;
}


symbol_p unit_file::lookup(gcutf8 what, size_t len, bool menu, bool seek0)
// ----------------------------------------------------------------------------
//   Find the next row that begins with "what", return definition for it
// ----------------------------------------------------------------------------
//   The definition is set to nullptr if there is no second column
{
    uint     column   = 0;
    bool     quoted   = false;
    bool     found    = false;
    bool     hadeq    = false;
    bool     nodefs   = false;
    size_t   matching = 0;
    size_t   index    = 0;
    symbol_g def      = nullptr;
    scribble scr;

    def = nullptr;
    if (seek0)
        seek(0);
    while (valid())
    {
        byte c = getchar();
        if (!c)
            break;
        if (c == '"')
        {
            if (quoted && peek() == '"') // Treat double "" as a data quote
            {
                c = getchar();
                if (column == 1 && found)
                {
                    byte *buf = rt.allocate(1);
                    *buf = byte(c);
                }
            }
            else
            {
                quoted = !quoted;
            }
            if (quoted)
            {
                index = 0;
                if (!column)
                {
                    found = true;
                    matching = 0;
                }
            }
            else
            {
                bool endline = peek() == '\n';
                if (found)
                {
                    if (column == 0)
                    {
                        found = found && matching == len && (!menu || endline);
                        if (menu && found)
                            def = symbol::make(what, matching);
                    }
                    else if (column == 1 && !menu)
                    {
                        if (!nodefs)
                            def = symbol::make(scr.scratch(), scr.growth());
                        else
                            found = false;
                    }
                }
                if (column == 0 && endline)
                {
                    nodefs = hadeq;
                    hadeq = false;
                }
                scr.clear();
                column++;
            }
        }
        else if (c == '\n')
        {
            // We had a full record, exit if we found our entry
            if (found)
                break;
            column = 0;
        }
        else if (quoted)
        {
            if (column == 0)
            {
                found = found && matching < len && c == (+what)[matching++];
                if (!index && c == '=')
                    hadeq = true;
            }
            else if (column == 1 && found)
            {
                byte *buf = rt.allocate(1);
                *buf = byte(c);
            }
            index++;
        }
    }
    return def;
}


symbol_p unit_file::next(bool menu)
// ----------------------------------------------------------------------------
//   Find the next file entry if there is one
// ----------------------------------------------------------------------------
//   A menu is an entry where the definition is not present or emtpy
{
    uint     column   = 0;
    bool     quoted   = false;
    symbol_g sym      = nullptr;
    uint     pos      = position();
    scribble scr;

    while (valid())
    {
        char c = getchar();
        if (!c)
            break;

        if (c == '"')
        {
            quoted = !quoted;
            if (!quoted)
                column++;
        }
        else if (c == '\n')
        {
            // We had a full record, exit if we found our entry
            if (column)
            {
                if (menu == (column == 1))
                {
                    sym = symbol::make(scr.scratch(), scr.growth());
                    break;
                }
                if (column == 1 && !menu)
                {
                    seek(pos);
                    break;
                }
            }
            scr.clear();
            column = 0;
            pos = position();
        }
        else if (quoted)
        {
            if (column == 0)
            {
                byte *buf = rt.allocate(1);
                *buf = byte(c);
            }
        }
    }
    return sym;
}



// ============================================================================
//
//   Build a units menu
//
// ============================================================================

utf8 unit_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name associated with the type
// ----------------------------------------------------------------------------
{
    uint count = type - ID_UnitMenu00;
    unit_file ufile;

    // List all preceding entries
    if (ufile.valid())
        while (symbol_p mname = ufile.next(true))
            if (*mname->value() != '=')
                if (!count--)
                    return mname->value(&len);

    if (Settings.ShowBuiltinUnits())
    {
        size_t maxu = sizeof(basic_units) / sizeof(basic_units[0]);
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (!basic_units[u+1] || !*basic_units[u+1])
            {
                if (!count--)
                {
                    len = strlen(basic_units[u]);
                    return utf8(basic_units[u]);
                }
            }
        }
    }

    return nullptr;
}


MENU_BODY(unit_menu)
// ----------------------------------------------------------------------------
//   Build a units menu
// ----------------------------------------------------------------------------
{
    // Use the units loaded from the units file
    unit_file ufile;
    size_t    matching = 0;
    size_t    maxu     = sizeof(basic_units) / sizeof(basic_units[0]);
    uint      position = 0;
    uint      count    = 0;
    size_t    first    = 0;
    size_t    last     = maxu;
    id        type     = o->type();
    id        menu     = ID_UnitMenu00;

    if (ufile.valid())
    {
        while (symbol_p mname = ufile.next(true))
        {
            if (*mname->value() == '=')
                continue;
            if (menu == type)
            {
                position = ufile.position();
                while (ufile.next(false))
                    matching++;
                menu = id(menu + 1);
                break;
            }
            menu = id(menu + 1);
        }
    }

     // Disable built-in units if we loaded a file
    if (!matching || Settings.ShowBuiltinUnits())
    {
        bool found = false;
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (!basic_units[u+1] || !*basic_units[u+1])
            {
                if (found)
                {
                    last = u;
                    break;
                }
                if (menu == type)
                {
                    found = true;
                    first = u + 2;
                }
                menu = id(menu + 1);
            }
        }
        if (found)
            count = (last - first) / 2;
    }

    items_init(mi, count + matching, 3, 1);

    // Insert the built-in units after the ones from the file
    uint skip = mi.skip;
    for (uint plane = 0; plane < 3; plane++)
    {
        static const id ids[3] =
        {
            ID_ApplyUnit, ID_ConvertToUnit, ID_ApplyInverseUnit
        };
        mi.plane  = plane;
        mi.planes = plane + 1;
        mi.index  = plane * ui.NUM_SOFTKEYS;
        mi.skip   = skip;
        id type = ids[plane];

        if (matching)
        {
            ufile.seek(position);
            while (symbol_g mentry = ufile.next(false))
                items(mi, mentry, type);
        }
        for (uint i = 0; i < count; i++)
            items(mi, basic_units[first + 2*i], type);
    }

    for (uint k = 0; k < ui.NUM_SOFTKEYS - (mi.pages > 1); k++)
    {
        ui.marker(k + 1 * ui.NUM_SOFTKEYS, L'→', true);
        ui.marker(k + 2 * ui.NUM_SOFTKEYS, '/', false);
    }

    return true;
}


MENU_BODY(UnitsMenu)
// ----------------------------------------------------------------------------
//   The units menu is dynamically populated
// ----------------------------------------------------------------------------
{
    uint      infile   = 0;
    uint      count    = 0;
    uint      maxmenus = ID_UnitMenu99 - ID_UnitMenu00;
    size_t    maxu     = sizeof(basic_units) / sizeof(basic_units[0]);
    unit_file ufile;

    // List all menu entries in the file (up to 100)
    if (ufile.valid())
        while (symbol_p mname = ufile.next(true))
            if (*mname->value() != '=')
                if (infile++ >= maxmenus)
                    break;

    // Count built-in unit menu titles
    if (!infile || Settings.ShowBuiltinUnits())
    {
        for (size_t u = 0; u < maxu; u += 2)
            if (!basic_units[u+1] || !*basic_units[u+1])
                count++;
        if (infile + count > maxmenus)
            count = maxmenus - infile;
    }

    items_init(mi, 1 + infile + count);
    items(mi, "_", ID_SelfInsert);

    infile = 0;
    if (ufile.valid())
    {
        ufile.seek(0);
        while (symbol_p mname = ufile.next(true))
        {
            if (*mname->value() == '=')
                continue;
            if (infile >= maxmenus)
                break;
            items(mi, mname, id(ID_UnitMenu00 + infile++));
        }
    }
    if (!infile || Settings.ShowBuiltinUnits())
    {
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (!basic_units[u+1] || !*basic_units[u+1])
            {
                if (infile >= maxmenus)
                    break;
                items(mi, basic_units[u], id(ID_UnitMenu00 + infile++));
            }
        }
    }

    return true;
}



// ============================================================================
//
//   Unit-related commands
//
// ============================================================================

COMMAND_BODY(Convert)
// ----------------------------------------------------------------------------
//   Convert level 2 into unit of level 1
// ----------------------------------------------------------------------------
{
    unit_p y = unit::get_after_evaluation(rt.stack(1));
    unit_p x = unit::get_after_evaluation(rt.stack(0));
    if (!y || !x)
    {
        rt.type_error();
        return ERROR;
    }
    algebraic_g r = y;
    if (!x->convert(r))
        return ERROR;
    if (!r || !rt.drop() || !rt.top(r))
        return ERROR;
    return OK;
}


FUNCTION_BODY(UBase)
// ----------------------------------------------------------------------------
//   Convert level 1 to the base SI units
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (unit_p u = unit::get_after_evaluation(x))
    {
        algebraic_g r = u;
        save<bool> ueval(unit::mode, true);
        r = r->evaluate();
        return r;
    }

    // No-op for numerical values
    id ty = x->type();
    if (is_real(ty) || is_complex(ty) || ty == ID_symbol)
        return x;

    if (expression_p expr = expression::get(x))
    {
        scribble scr;
        for (object_p lobj : *expr)
        {
            if (unit_p u = unit::get(lobj))
            {
                algebraic_g r = u;
                save<bool> ueval(unit::mode, true);
                r = r->evaluate();
                lobj = r;
            }
            if (!rt.append(lobj))
                return nullptr;

        }
        list_p list = list::make(object::ID_expression,
                                 scr.scratch(), scr.growth());
        return list;
    }
    if (!rt.error())
        rt.type_error();
    return nullptr;
}


COMMAND_BODY(UFact)
// ----------------------------------------------------------------------------
//   Factor level 1 unit out of level 2 unit
// ----------------------------------------------------------------------------
{
    unit_p x = unit::get_after_evaluation(rt.stack(0));
    unit_p y = unit::get_after_evaluation(rt.stack(1));
    if (!x || !y)
    {
        rt.type_error();
        return ERROR;
    }

    algebraic_g xa = x;
    algebraic_g ya = y;
    save<bool> ueval(unit::mode, true);
    algebraic_g r = xa * (ya / xa);
    if (r->is_same_as(ya))
    {
        algebraic_g d = xa->evaluate();
        ya = ya->evaluate();
        r = xa * (ya / d);
    }
    if (!r || !rt.drop() || !rt.top(r))
        return ERROR;
    return OK;
}


FUNCTION_BODY(UVal)
// ----------------------------------------------------------------------------
//   Extract value from unit object in level 1
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->is_symbolic())
        return symbolic(ID_UVal, x);
    if (unit_p u = unit::get(x))
        return u->value();
    if (x->is_algebraic_num())
        return x;
    rt.type_error();
    return nullptr;
}


COMMAND_BODY(ToUnit)
// ----------------------------------------------------------------------------
//   Combine a value and a unit object to build a new unit object
// ----------------------------------------------------------------------------
{
    object_p y = strip(rt.stack(1));
    unit_p x = unit::get_after_evaluation(rt.stack(0));
    if (!x || !y || !y->is_algebraic())
    {
        rt.type_error();
        return ERROR;
    }
    algebraic_g u = algebraic_p(y);
    algebraic_g result = unit::simple(u, x->uexpr());
    if (result && rt.pop() && rt.top(result))
        return OK;
    return ERROR;
}


static algebraic_p key_unit(uint key, bool uexpr)
// ----------------------------------------------------------------------------
//   Return a softkey label as a unit value
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
        {
            char buffer[32];
            if (len > sizeof(buffer) - 2)
            {
                rt.invalid_unit_error();
                return nullptr;
            }
            save<bool> umode(unit::mode, true);
            buffer[0] = '1';
            buffer[1] = '_';
            memcpy(buffer+2, txt, len);
            len += 2;
            if (object_p uobj = object::parse(utf8(buffer), len))
                if (unit_p u = unit::get(uobj))
                    return uexpr ? u->uexpr() : u;
        }
    }
    return nullptr;
}


COMMAND_BODY(ApplyUnit)
// ----------------------------------------------------------------------------
//   Apply a unit from a unit menu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (algebraic_g uname = key_unit(key, true))
        if (object_p value = strip(rt.top()))
            if (algebraic_g alg = value->as_algebraic())
                if (algebraic_g uobj = unit::simple(alg, uname))
                    if (rt.top(+uobj))
                        return OK;

    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(ApplyUnit)
// ----------------------------------------------------------------------------
//   Insert the application of the unit
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (ui.editing_mode() == ui.UNIT)
        return ui.insert_softkey(key,
                                 is_valid_in_name(ui.character_left_of_cursor())
                                 ? "·" : "", "", false);
    if (ui.at_end_of_number())
        return ui.insert_softkey(key, "_", "", false);
    return ui.insert_softkey(key, " 1_", " * ", false);
}


COMMAND_BODY(ApplyInverseUnit)
// ----------------------------------------------------------------------------
//   Apply the invserse of a unit from a unit menu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (algebraic_g uname = key_unit(key, true))
        if (object_p value = strip(rt.top()))
            if (algebraic_g alg = value->as_algebraic())
                if (algebraic_g uobj = unit::simple(alg, inv::run(uname)))
                    if (rt.top(+uobj))
                        return OK;

    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(ApplyInverseUnit)
// ----------------------------------------------------------------------------
//   Insert the application of the inverse of a unit
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (ui.editing_mode() == ui.UNIT)
    {
        unicode c = ui.character_left_of_cursor();
        return ui.insert_softkey(key,
                                 c == '_'                  ? "1/("
                                 : (c == '/' || c == L'÷') ? "("
                                                           : "/(",
                                 ")",
                                 false);
    }
    if (ui.at_end_of_number())
        return ui.insert_softkey(key, "_(", ")⁻¹", false);
    return ui.insert_softkey(key, " 1_", " / ", false);
}


COMMAND_BODY(ConvertToUnit)
// ----------------------------------------------------------------------------
//   Apply conversion to a given menu unit
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (algebraic_g uname = key_unit(key, false))
        if (object_p value = strip(rt.top()))
            if (algebraic_g alg = value->as_algebraic())
                if (unit_g uobj = unit::get(uname))
                    if (uobj->convert(alg))
                        if (rt.top(+alg))
                            return OK;

    return ERROR;
}


INSERT_BODY(ConvertToUnit)
// ----------------------------------------------------------------------------
//   Insert a conversion to the given unit
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    ui.at_end_of_number();
    return ui.insert_softkey(key, " 1_", " Convert ", false);
}


static symbol_p unit_name(object_p obj)
// ----------------------------------------------------------------------------
//    If the object is a simple unit like `1_m`, return `m`
// ----------------------------------------------------------------------------
{
    if (obj)
    {
        if (unit_p uobj = unit::get(obj))
        {
            algebraic_p uexpr = uobj->uexpr();
            symbol_p name = uexpr->as<symbol>();
            if (!name)
                if (expression_p eq = uexpr->as<expression>())
                    if (symbol_p inner = eq->as_quoted<symbol>())
                        name = inner;
            return name;
        }
    }
    return nullptr;
}


COMMAND_BODY(ConvertToUnitPrefix)
// ----------------------------------------------------------------------------
//   Convert to a given unit prefix
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key < KEY_F1 || key > KEY_F6)
        return object::OK;

    // Read the prefix (e.g. "c") from the softkey label,
    uint index = key - KEY_F1 + ui.NUM_SOFTKEYS * ui.shift_plane();
    cstring prefix = ui.label_text(index);
    if (!prefix)
    {
        rt.undefined_operation_error();
        return ERROR;
    }

    // Read the stack value
    object_p value = strip(rt.top());
    if (!value)
        return ERROR;

    // Check if it is tagged
    if (tag_g tagged = value->as<tag>())
    {
        size_t len = 0;
        gcutf8 label = tagged->label_value(&len);
        rt.top(tagged->tagged_object());
        result rc = ConvertToUnitPrefix::evaluate();
        value = rt.top();
        if (rc != OK || !value)
        {
            rt.top(tagged);
            return rc;
        }
        tagged = tag::make(label, len, value);
        if (rt.top(tagged))
            return OK;
        return ERROR;
    }

    // This must be a unit type with a simple name
    unit_g   un  = unit::get(value);
    symbol_p sym = unit_name(un);
    if (!sym)
    {
        rt.type_error();
        return ERROR;
    }
    size_t syml = 0;
    gcutf8 symt = sym->value(&syml);

    // Lookup the name to get the underlying unit, e.g. 1_km -> 1000_m
    int    pfxi = 0;
    unit_p base = unit::lookup(sym, &pfxi);
    if (!base)
    {
        rt.inconsistent_units_error();
        return ERROR;
    }
    bool kibi = pfxi < 0;
    if (kibi)
        pfxi = -pfxi;
    const si_prefix *pfxp = &si_prefixes[pfxi];
    cstring          pfxt = pfxp->prefix;
    size_t           pfxl = strlen(pfxt) + kibi;

    // Find the prefix given in the label
    gcutf8 ptxt = utf8(prefix);
    size_t plen = strlen(prefix);
    if (cstring space = strchr(prefix, ' '))
    {
        size_t offset = space - prefix;
        if (plen > offset)
            plen = offset;
    }


    // Render 1_cm if the prefix is c
    renderer r;
    r.put("1_");
    r.put(ptxt, plen);
    r.put(symt + pfxl, syml - pfxl);

    plen = r.size();
    object_p scaled = object::parse(r.text(), plen);
    if (!scaled)
        return ERROR;
    unit_p target = unit::get(scaled);
    if (!target)
    {
        rt.inconsistent_units_error();
        return ERROR;
    }

    // Perform the conversion to the desired unit
    algebraic_g x = +un;
    if (!target->convert(x))
    {
        rt.inconsistent_units_error();
        return ERROR;
    }

    if (!rt.top(x))
        return ERROR;
    return OK;
}


static algebraic_p toAngleUnit(algebraic_r x, cstring angleUnit)
// ----------------------------------------------------------------------------
//   Convert the value x to the given angle unit
// ----------------------------------------------------------------------------
{
    unit_g uobj = unit::get(x);
    if (uobj)
    {
        object::id amode = object::ID_object;
        algebraic_g uexpr = uobj->uexpr();
        if (symbol_p sym = uexpr->as_quoted<symbol>())
        {
            if (sym->matches("dms") || sym->matches("°"))
                amode = object::ID_Deg;
            else if (sym->matches("r"))
                amode = object::ID_Rad;
            else if (sym->matches("pir") || sym->matches("πr"))
                amode = object::ID_PiRadians;
            else if (sym->matches("grad"))
                amode = object::ID_Grad;
        }
        if (!amode)
        {
            rt.inconsistent_units_error();
            return nullptr;
        }
    }
    else
    {
        if (!x->is_real())
        {
            rt.type_error();
            return nullptr;
        }

        cstring uname;
        switch(Settings.AngleMode())
        {
        case object::ID_Deg:        uname = "°";    break;
        case object::ID_Grad:       uname = "grad"; break;
        case object::ID_PiRadians:  uname = "πr";   break;
        default:
        case object::ID_Rad:        uname = "r";    break;
        }

        symbol_p uexpr = symbol::make(uname);
        uobj = unit::make(algebraic_p(+x), uexpr);
    }

    unit_g targetUnit = unit::make(integer::make(1), +symbol::make(angleUnit));
    if (targetUnit && targetUnit->convert(uobj))
        return uobj;
    return nullptr;
}


INSERT_BODY(ConvertToUnitPrefix)
// ----------------------------------------------------------------------------
//   This is not a programmable command, since we need to have the unit
// ----------------------------------------------------------------------------
{
    rt.command(command::static_object(ID_ConvertToUnitPrefix));
    rt.not_programmable_error();
    return ERROR;
}


FUNCTION_BODY(ToDegrees)
// ----------------------------------------------------------------------------
//   Convert to degrees unit
// ----------------------------------------------------------------------------
{
    return toAngleUnit(x, "°");
}


FUNCTION_BODY(ToRadians)
// ----------------------------------------------------------------------------
//   Convert to radians unit
// ----------------------------------------------------------------------------
{
    return toAngleUnit(x, "r");
}


FUNCTION_BODY(ToGrads)
// ----------------------------------------------------------------------------
//   Convert to grads unit
// ----------------------------------------------------------------------------
{
    return toAngleUnit(x, "grad");
}


FUNCTION_BODY(ToPiRadians)
// ----------------------------------------------------------------------------
//   Convert to pi-radians unit
// ----------------------------------------------------------------------------
{
    return toAngleUnit(x, "πr");
}


COMMAND_BODY(UnitsSIPrefixCycle)
// ----------------------------------------------------------------------------
//   Store the units cycle preference in the current directory
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (text_p pfx = obj->as<text>())
            if (object_p name = unit::si_prefixes_variable())
                if (directory::store_here(name, pfx))
                    if (rt.drop())
                        return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}
