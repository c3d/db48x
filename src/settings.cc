// ****************************************************************************
//  settings.cc                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Representation of settings
//
//
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

#include "settings.h"

#include "arithmetic.h"
#include "command.h"
#include "font.h"
#include "functions.h"
#include "integer.h"
#include "menu.h"
#include "renderer.h"
#include "user_interface.h"

#include <cstdarg>
#include <cstdlib>
#include <random>
#include <stdexcept>

settings Settings;



// ============================================================================
//
//   Save the current settings to a renderer
//
// ============================================================================

void settings::save(renderer &out, bool show_defaults)
// ----------------------------------------------------------------------------
//   Save the current settings to the given renderer
// ----------------------------------------------------------------------------
{
    settings Defaults;

    // Save the current menu
    if (menu_p menu = ui.menu())
    {
        menu->render(out);
        out.put('\n');
    }

    // Save current computation precision
    if (precision != BID128_MAXDIGITS || show_defaults)
        out.put("%u Precision\n", precision);

    // Save current display setting
    switch(display_mode)
    {
    case NORMAL:
        if (displayed == settings::STD_DISPLAYED)
        {
            if (show_defaults)
                out.put("%STD\n");
        }
        else    out.printf("%u SIG\n", displayed); break;
    case FIX:   out.printf("%u FIX\n", displayed); break;
    case SCI:   out.printf("%u SCI\n", displayed); break;
    case ENG:   out.printf("%u ENG\n", displayed); break;
    }

    // Save Decimal separator
    if (decimal_mark == ',')
        out.put("DecimalComma\n");
    else if (show_defaults)
        out.put("DecimalDot\n");

    // Save preferred exponent display mode
    if (exponent_mark != L'⁳' || !fancy_exponent)
        out.put("ClassicExponent\n");
    else if (show_defaults)
        out.put("FancyExponent\n");

    // Save preferred expenent for switching to scientfiic mode
    if (standard_exp != Defaults.standard_exp || show_defaults)
        out.printf("%u StandardExponent\n", standard_exp);

    // Save current angle mode
    switch(angle_mode)
    {
    default:
    case DEGREES:       if (show_defaults)      out.put("DEG\n"); break;
    case RADIANS:                               out.put("RAD\n"); break;
    case GRADS:                                 out.put("GRAD\n"); break;
    case PI_RADIANS:                            out.put("PIRADIANS\n"); break;
    }

    // Save default base
    if( base != Defaults.base || show_defaults)
        out.printf("%u Base\n", base);

    // Save default word size
    if (wordsize != Defaults.wordsize || show_defaults)
        out.printf("%u WordSize\n", wordsize);

    // Save default command format
    switch(command_fmt)
    {
    default:
    case LONG_FORM:     if (show_defaults)      out.put("LongForm\n");    break;
    case LOWERCASE:                             out.put("lowercase\n");   break;
    case UPPERCASE:                             out.put("UPPERCASE\n");   break;
    case CAPITALIZED:                           out.put("Capitalized\n"); break;
    }

    // Check if we want to show 1.0 as 1 and not 1.
    if (!show_decimal)
        out.put("NoTrailingDecimal\n");
    else if (show_defaults)
        out.put("TrailingDecimal\n");

    // Font size
    if (result_sz != Defaults.result_sz || show_defaults)
        out.printf("%u ResultFontSize\n", result_sz);
    if (stack_sz != Defaults.stack_sz || show_defaults)
        out.printf("%u StackFontSize\n", result_sz);
    if (editor_sz != Defaults.editor_sz || show_defaults)
        out.printf("%u EditorFontSize\n", result_sz);
    if (editor_ml_sz != Defaults.editor_ml_sz || show_defaults)
        out.printf("%u EditorMultilineFontSize\n", result_sz);

    // Number spacing
    if (spacing_mantissa != Defaults.spacing_mantissa || show_defaults)
        out.printf("%u MantissaSpacing\n", spacing_mantissa);
    if (spacing_fraction != Defaults.spacing_fraction || show_defaults)
        out.printf("%u FractionSpacing\n", spacing_fraction);
    if (spacing_based != Defaults.spacing_based || show_defaults)
        out.printf("%u BasedSpacing\n", spacing_based);

    switch (space)
    {
    default:
    case SPACE_DEFAULT:
        if (show_defaults)
                                out.put("NumberSpaces\n");      break;
    case '.': case ',':         out.put("NumberDotOrComma\n");  break;
    case L'’':                  out.put("NumberTicks\n");       break;
    case '_':                   out.put("NumberUnderscore\n");  break;
    }

    switch (space_based)
    {
    default:
    case SPACE_DEFAULT:
        if (show_defaults)
                                out.put("BasedSpaces\n");       break;
    case '.': case ',':         out.put("BasedDotOrComma\n");   break;
    case L'’':                  out.put("BasedTicks\n");        break;
    case '_':                   out.put("BasedUnderscore\n");   break;
    }

    if (!auto_simplify)
        out.put("NoAutoSimplify\n");
    else if (show_defaults)
        out.put("AutoSimplify\n");

    if (maxbignum != Defaults.maxbignum || show_defaults)
        out.printf("%u MaxBigNumBits\n", maxbignum);
    if (maxrewrites != Defaults.maxrewrites || show_defaults)
        out.printf("%u MaxRewrites\n", maxbignum);
    if (fraciter != Defaults.fraciter || show_defaults)
        out.printf("%u ToFractionIterations\n", fraciter);
    if (fracprec != Defaults.fracprec || show_defaults)
        out.printf("%u ToFractionDigits\n", fracprec);
}


COMMAND_BODY(Modes)
// ----------------------------------------------------------------------------
//   Return a program that restores the current modes
// ----------------------------------------------------------------------------
{
    renderer modes;
    modes.put("«");
    Settings.save(modes);
    modes.put("»");

    size_t size = modes.size();
    gcutf8 code = modes.text();
    if (object_g program = object::parse(code, size))
        if (rt.push(program))
            return OK;
    return ERROR;
}


font_p settings::font(font_id size)
// ----------------------------------------------------------------------------
//   Return a font based on a font size
// ----------------------------------------------------------------------------
{
    switch (size)
    {
    case EDITOR:        return EditorFont;
    default:
    case STACK:         return StackFont;
    case HELP:          return HelpFont;

    case LIB17:         return LibMonoFont10x17;
    case LIB18:         return LibMonoFont11x18;
    case LIB20:         return LibMonoFont12x20;
    case LIB22:         return LibMonoFont14x22;
    case LIB25:         return LibMonoFont17x25;
    case LIB28:         return LibMonoFont17x28;

    case SKR18:         return SkrMono13x18;
    case SKR24:         return SkrMono18x24;

    case FREE42:        return Free42Font;
    }
}


font_p settings::cursor_font(font_id size)
// ----------------------------------------------------------------------------
//   Return a cursor font based on a font size
// ----------------------------------------------------------------------------
{
    switch (size)
    {
    case EDITOR:        return StackFont;
    default:
    case STACK:         return LibMonoFont14x22;
    case HELP:          return HelpFont;

    case LIB17:         return LibMonoFont10x17;
    case LIB18:         return LibMonoFont10x17;
    case LIB20:         return LibMonoFont11x18;
    case LIB22:         return LibMonoFont12x20;
    case LIB25:         return LibMonoFont14x22;
    case LIB28:         return LibMonoFont17x25;

    case SKR18:
    case SKR24:         return SkrMono13x18;

    case FREE42:        return Free42Font;
    }
}




// ============================================================================
//
//    Commands to manipulate the settings
//
// ============================================================================

static inline bool IsStd()
// ----------------------------------------------------------------------------
//   Check if the current settings is Std
// ----------------------------------------------------------------------------
{
    return Settings.display_mode == settings::NORMAL
        && Settings.displayed == settings::STD_DISPLAYED;
}


SETTINGS_COMMAND_BODY(Std, IsStd())
// ----------------------------------------------------------------------------
//   Switch to standard display mode
// ----------------------------------------------------------------------------
{
    Settings.displayed = settings::STD_DISPLAYED;
    Settings.display_mode = settings::NORMAL;
    return OK;
}


SETTINGS_COMMAND_LABEL(Std)
// ----------------------------------------------------------------------------
//   Return the label for Std
// ----------------------------------------------------------------------------
{
    return "Std";
}


static uint integer_arg(uint min, uint max, bool base = false)
// ----------------------------------------------------------------------------
//   Get an integer argument from the stack
// ----------------------------------------------------------------------------
{
    if (object_p arg = rt.top())
    {
        if (integer_p argint = arg->as<integer>())
        {
            uint value = argint->value<uint>();
            if (value < min || value > max)
            {
                if (value < min)        value = min;
                if (value > max)        value = max;
                if (base)
                    rt.invalid_base_error();
                else
                    rt.domain_error();
            }
            rt.pop();
            return value;
        }
        else
        {
            rt.type_error();
        }
    }
    return min;
}


static object::result set_display_mode(settings::display mode)
// ----------------------------------------------------------------------------
//   Set a mode with a given number of digits
// ----------------------------------------------------------------------------
{
    uint mindigits = mode == settings::NORMAL ? 1 : 0;
    uint disp = integer_arg(mindigits, BID128_MAXDIGITS);
    if (!rt.error())
    {
        Settings.displayed = disp;
        Settings.display_mode = mode;
        return object::OK;
    }
    return object::ERROR;
}


static cstring get_display_mode(settings::display mode, cstring label)
// ----------------------------------------------------------------------------
//   Compute the label for display mode
// ----------------------------------------------------------------------------
{
    if (Settings.display_mode == mode)
    {
        // We can share the buffer here since only one mode is active
        static char buffer[8];
        snprintf(buffer, sizeof(buffer), "%s %u", label, Settings.displayed);
        return buffer;
    }
    return label;
}


SETTINGS_COMMAND_BODY(Fix, Settings.display_mode == settings::FIX)
// ----------------------------------------------------------------------------
//   Switch to fixed display mode
// ----------------------------------------------------------------------------
{
    return set_display_mode(settings::FIX);
}


SETTINGS_COMMAND_LABEL(Fix)
// ----------------------------------------------------------------------------
//   Return the label for Fix mode
// ----------------------------------------------------------------------------
{
    return get_display_mode(settings::FIX, "Fix");
}


SETTINGS_COMMAND_BODY(Sci, Settings.display_mode == settings::SCI)
// ----------------------------------------------------------------------------
//   Switch to scientific display mode
// ----------------------------------------------------------------------------
{
    return set_display_mode(settings::SCI);
}


SETTINGS_COMMAND_LABEL(Sci)
// ----------------------------------------------------------------------------
//   Return the label for Fix mode
// ----------------------------------------------------------------------------
{
    return get_display_mode(settings::SCI, "Sci");
}


SETTINGS_COMMAND_BODY(Eng, Settings.display_mode == settings::ENG)
// ----------------------------------------------------------------------------
//   Switch to engineering display mode
// ----------------------------------------------------------------------------
{
    return set_display_mode(settings::ENG);
}


SETTINGS_COMMAND_LABEL(Eng)
// ----------------------------------------------------------------------------
//   Return the label for Eng mode
// ----------------------------------------------------------------------------
{
    return get_display_mode(settings::ENG, "Eng");
}


SETTINGS_COMMAND_BODY(Sig,
                      Settings.display_mode == settings::NORMAL
                      && Settings.displayed != settings::STD_DISPLAYED
                     )
// ----------------------------------------------------------------------------
//   Switch to significant display mode
// ----------------------------------------------------------------------------
{
    return set_display_mode(settings::NORMAL);
}


SETTINGS_COMMAND_LABEL(Sig)
// ----------------------------------------------------------------------------
//   Return the label for Eng mode
// ----------------------------------------------------------------------------
{
    return get_display_mode(settings::NORMAL, "Sig");
}


SETTINGS_COMMAND_NOLABEL(Deg,
                         Settings.angle_mode == settings::DEGREES)
// ----------------------------------------------------------------------------
//   Switch to degrees
// ----------------------------------------------------------------------------
{
    Settings.angle_mode = settings::DEGREES;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Rad,
                         Settings.angle_mode == settings::RADIANS)
// ----------------------------------------------------------------------------
//   Switch to radians
// ----------------------------------------------------------------------------
{
    Settings.angle_mode = settings::RADIANS;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Grad,
                         Settings.angle_mode == settings::GRADS)
// ----------------------------------------------------------------------------
//   Switch to grads
// ----------------------------------------------------------------------------
{
    Settings.angle_mode = settings::GRADS;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(PiRadians,
                         Settings.angle_mode == settings::PI_RADIANS)
// ----------------------------------------------------------------------------
//   Switch to grads
// ----------------------------------------------------------------------------
{
    Settings.angle_mode = settings::PI_RADIANS;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(LowerCase,
                         Settings.command_fmt == settings::LOWERCASE)
// ----------------------------------------------------------------------------
//   Switch to lowercase command display
// ----------------------------------------------------------------------------
{
    Settings.command_fmt = settings::LOWERCASE;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(UpperCase,
                         Settings.command_fmt == settings::UPPERCASE)
// ----------------------------------------------------------------------------
//  Switch to uppercase command display
// ----------------------------------------------------------------------------
{
    Settings.command_fmt = settings::UPPERCASE;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Capitalized,
                         Settings.command_fmt==settings::CAPITALIZED)
// ----------------------------------------------------------------------------
//  Switch to capitalized command display
// ----------------------------------------------------------------------------
{
    Settings.command_fmt = settings::CAPITALIZED;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(LongForm,
                         Settings.command_fmt==settings::LONG_FORM)
// ----------------------------------------------------------------------------
//   Switch to long-form command display
// ----------------------------------------------------------------------------
{
    Settings.command_fmt = settings::LONG_FORM;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(DecimalDot, Settings.decimal_mark == '.')
// ----------------------------------------------------------------------------
//  Switch to decimal dot
// ----------------------------------------------------------------------------
{
    Settings.decimal_mark = '.';
    if (Settings.space == '.')
        Settings.space = ',';
    if (Settings.space_based == '.')
        Settings.space_based = ',';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(DecimalComma, Settings.decimal_mark == ',')
// ----------------------------------------------------------------------------
//  Switch to decimal comma
// ----------------------------------------------------------------------------
{
    Settings.decimal_mark = ',';
    if (Settings.space == ',')
        Settings.space = '.';
    if (Settings.space_based == ',')
        Settings.space_based = '.';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(TrailingDecimal, Settings.show_decimal)
// ----------------------------------------------------------------------------
//  Indicate that we want a trailing decimal separator
// ----------------------------------------------------------------------------
{
    Settings.show_decimal = true;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(NoTrailingDecimal, !Settings.show_decimal)
// ----------------------------------------------------------------------------
//  Indicate that we don't want a traiing decimal separator
// ----------------------------------------------------------------------------
{
    Settings.show_decimal = false;
    return OK;
}


SETTINGS_COMMAND_BODY(Precision, 0)
// ----------------------------------------------------------------------------
//   Setting the precision
// ----------------------------------------------------------------------------
{
    uint prec = integer_arg(0, BID128_MAXDIGITS);
    if (!rt.error())
    {
        Settings.precision = prec;
        return object::OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(Precision)
// ----------------------------------------------------------------------------
//   Return the label for the current precision
// ----------------------------------------------------------------------------
{
    // We can share the buffer here since only one mode is active
    static char buffer[12];
    snprintf(buffer, sizeof(buffer), "Prec %u", Settings.precision);
    return buffer;
}


SETTINGS_COMMAND_BODY(StandardExponent, 0)
// ----------------------------------------------------------------------------
//   Setting the maximum exponent before switching to scientific mode
// ----------------------------------------------------------------------------
{
    uint exp = integer_arg(0, BID128_MAXDIGITS);
    if (!rt.error())
    {
        Settings.standard_exp = exp;
        return object::OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(StandardExponent)
// ----------------------------------------------------------------------------
//   Return the label for the current standard exponent
// ----------------------------------------------------------------------------
{
    // We can share the buffer here since only one mode is active
    static char buffer[12];
    snprintf(buffer, sizeof(buffer), "Exp %u", Settings.standard_exp);
    return buffer;
}


SETTINGS_COMMAND_NOLABEL(FancyExponent, Settings.fancy_exponent)
// ----------------------------------------------------------------------------
//   Setting the maximum exponent before switching to scientific mode
// ----------------------------------------------------------------------------
{
    Settings.fancy_exponent = true;
    Settings.exponent_mark = L'⁳';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(ClassicExponent, !Settings.fancy_exponent)
// ----------------------------------------------------------------------------
//   Setting the maximum exponent before switching to scientific mode
// ----------------------------------------------------------------------------
{
    Settings.fancy_exponent = false;
    Settings.exponent_mark = 'E';
    return OK;
}


SETTINGS_COMMAND_BODY(Base, 0)
// ----------------------------------------------------------------------------
//   Setting the maximum exponent before switching to scientific mode
// ----------------------------------------------------------------------------
{
    uint base = integer_arg(2, 36, true);
    if (!rt.error())
    {
        Settings.base = base;
        return object::OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(Base)
// ----------------------------------------------------------------------------
//   Return the label for the current base
// ----------------------------------------------------------------------------
{
    // We can share the buffer here since only one mode is active
    static char buffer[12];
    snprintf(buffer, sizeof(buffer), "Base %u", Settings.base);
    return buffer;
}


SETTINGS_COMMAND_NOLABEL(Bin, Settings.base == 2)
// ----------------------------------------------------------------------------
//   Select binary mode
// ----------------------------------------------------------------------------
{
    Settings.base = 2;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Oct, Settings.base == 8)
// ----------------------------------------------------------------------------
//   Select octal mode
// ----------------------------------------------------------------------------
{
    Settings.base = 8;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Dec, Settings.base == 10)
// ----------------------------------------------------------------------------
//   Select decimalmode
// ----------------------------------------------------------------------------
{
    Settings.base = 10;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(Hex, Settings.base == 16)
// ----------------------------------------------------------------------------
//   Select hexadecimal mode
// ----------------------------------------------------------------------------
{
    Settings.base = 16;
    return OK;
}


SETTINGS_COMMAND_BODY(stws, 0)
// ----------------------------------------------------------------------------
//   Setting the word size for binary computations
// ----------------------------------------------------------------------------
{
    uint ws = integer_arg(1, 16384);
    if (!rt.error())
    {
        Settings.wordsize = ws;
        return OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(stws)
// ----------------------------------------------------------------------------
//   Return the label for the current word size
// ----------------------------------------------------------------------------
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "WSz %u", Settings.wordsize);
    return buffer;
}


COMMAND_BODY(rcws)
// ----------------------------------------------------------------------------
//  Recall the current wordsize
// ----------------------------------------------------------------------------
{
    if (object_g ws = integer::make(Settings.wordsize))
        if (rt.push(ws))
            return OK;
    return ERROR;
}


#define FONT_SIZE_SETTING(id, field, label)             \
SETTINGS_COMMAND_BODY(id, 0)                            \
{                                                       \
    uint fs = integer_arg(0, settings::NUM_FONTS-1);    \
    if (!rt.error())                                    \
    {                                                   \
        Settings.field = settings::font_id(fs);         \
        return OK;                                      \
    }                                                   \
    return object::ERROR;                               \
}                                                       \
                                                        \
                                                        \
static char id##_buffer[16];                            \
SETTINGS_COMMAND_LABEL(id)                              \
{                                                       \
    snprintf(id##_buffer, sizeof(id##_buffer),          \
             label " %u", Settings.field);              \
    return id##_buffer;                                 \
}


FONT_SIZE_SETTING(ResultFontSize, result_sz, "Result")
FONT_SIZE_SETTING(StackFontSize, stack_sz, "Stack")
FONT_SIZE_SETTING(EditorFontSize, editor_sz, "Edit")
FONT_SIZE_SETTING(EditorMultilineFontSize, editor_ml_sz, "BigEdit")


#define SPACING_SIZE_SETTING(id, field, label)          \
SETTINGS_COMMAND_BODY(id, 0)                            \
{                                                       \
    uint fs = integer_arg(0, BID128_MAXDIGITS);         \
    if (!rt.error())                                    \
    {                                                   \
        Settings.field = settings::font_id(fs);         \
        return OK;                                      \
    }                                                   \
    return object::ERROR;                               \
}                                                       \
                                                        \
                                                        \
static char id##_buffer[16];                            \
SETTINGS_COMMAND_LABEL(id)                              \
{                                                       \
    snprintf(id##_buffer, sizeof(id##_buffer),          \
             label " %u", Settings.field);              \
    return id##_buffer;                                 \
}


COMMAND_BODY(NumberSpacing)
// ----------------------------------------------------------------------------
//  Set same spacing for both mantissa and fraction
// ----------------------------------------------------------------------------
{
    uint fs = integer_arg(0, BID128_MAXDIGITS);
    if (!rt.error())
    {
        Settings.spacing_mantissa = fs;
        Settings.spacing_fraction = fs;
        ui.menuNeedsRefresh();
        return OK;
    }
    return object::ERROR;
}

SPACING_SIZE_SETTING(MantissaSpacing, spacing_mantissa, "Mant")
SPACING_SIZE_SETTING(FractionSpacing, spacing_fraction, "Frac")
SPACING_SIZE_SETTING(BasedSpacing,    spacing_based,    "Based")


SETTINGS_COMMAND_NOLABEL(NumberSpaces,
                         Settings.space == settings::SPACE_DEFAULT)
// ----------------------------------------------------------------------------
//   Select a space as number separator
// ----------------------------------------------------------------------------
{
    Settings.space = settings::SPACE_DEFAULT;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(NumberDotOrComma,
                         Settings.space == '.' || Settings.space == ',')
// ----------------------------------------------------------------------------
//   Select a dot or comma as number separator
// ----------------------------------------------------------------------------
{
    Settings.space = Settings.decimal_mark == '.' ? ',' : '.';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(NumberTicks, Settings.space == L'’')
// ----------------------------------------------------------------------------
//   Select a tick as number separator
// ----------------------------------------------------------------------------
{
    Settings.space = L'’';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(NumberUnderscore, Settings.space == '_')
// ----------------------------------------------------------------------------
//   Select an underscore as number separator
// ----------------------------------------------------------------------------
{
    Settings.space = '_';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(BasedSpaces,
                         Settings.space_based == settings::SPACE_DEFAULT)
// ----------------------------------------------------------------------------
//   Select a space as based number separator
// ----------------------------------------------------------------------------
{
    Settings.space_based = settings::SPACE_DEFAULT;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(BasedDotOrComma,
                         Settings.space_based == '.'
                         || Settings.space_based == ',')
// ----------------------------------------------------------------------------
//   Select a dot or comma as based number separator
// ----------------------------------------------------------------------------
{
    Settings.space_based = Settings.decimal_mark == '.' ? ',' : '.';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(BasedTicks, Settings.space_based == L'’')
// ----------------------------------------------------------------------------
//   Select a tick as based number separator
// ----------------------------------------------------------------------------
{
    Settings.space_based = L'’';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(BasedUnderscore, Settings.space_based == '_')
// ----------------------------------------------------------------------------
//   Select an underscore as based number separator
// ----------------------------------------------------------------------------
{
    Settings.space_based = '_';
    return OK;
}


SETTINGS_COMMAND_NOLABEL(AutoSimplify, Settings.auto_simplify)
// ----------------------------------------------------------------------------
//   Enable automatic simplification of algebraic expressions
// ----------------------------------------------------------------------------
{
    Settings.auto_simplify = true;
    return OK;
}


SETTINGS_COMMAND_NOLABEL(NoAutoSimplify, !Settings.auto_simplify)
// ----------------------------------------------------------------------------
//   Disable automatic simplification of algebraic expressions
// ----------------------------------------------------------------------------
{
    Settings.auto_simplify = false;
    return OK;
}


SETTINGS_COMMAND_BODY(MaxBigNumBits, false)
// ----------------------------------------------------------------------------
//   Select maximum size for big numbers
// ----------------------------------------------------------------------------
{
    uint fs = integer_arg(0, 16384);
    if (!rt.error())
    {
        Settings.maxbignum = fs;
        return OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(MaxBigNumBits)
// ----------------------------------------------------------------------------
//   Return the label for big number size
// ----------------------------------------------------------------------------
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "BigNum %u", Settings.maxbignum);
    return buffer;
}


SETTINGS_COMMAND_BODY(MaxRewrites, false)
// ----------------------------------------------------------------------------
//   Select maximum number of equation rewrites in a single operation
// ----------------------------------------------------------------------------
{
    uint fs = integer_arg(1, ~0U);
    if (!rt.error())
    {
        Settings.maxrewrites = fs;
        return OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(MaxRewrites)
// ----------------------------------------------------------------------------
//   Return the label for maximum number of rewrites
// ----------------------------------------------------------------------------
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "BigRwr %u", Settings.maxrewrites);
    return buffer;
}


SETTINGS_COMMAND_BODY(ToFractionIterations, false)
// ----------------------------------------------------------------------------
//   Select maximum number of loops trying to compute fraction in ->Q
// ----------------------------------------------------------------------------
{
    uint fs = integer_arg(1, 30);
    if (!rt.error())
    {
        Settings.fraciter = fs;
        return OK;
    }
    return object::ERROR;
}

SETTINGS_COMMAND_LABEL(ToFractionIterations)
// ----------------------------------------------------------------------------
//   Return the label for ->Q iterations
// ----------------------------------------------------------------------------
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "Iter %u", Settings.fraciter);
    return buffer;
}


SETTINGS_COMMAND_BODY(ToFractionDigits, false)
// ----------------------------------------------------------------------------
//   Select maximum number of digits for the ->Q operation
// ----------------------------------------------------------------------------
{
    uint fs = integer_arg(1, BID128_MAXDIGITS - 3);
    if (!rt.error())
    {
        Settings.fracprec = fs;
        return OK;
    }
    return object::ERROR;
}


SETTINGS_COMMAND_LABEL(ToFractionDigits)
// ----------------------------------------------------------------------------
//   Return the label for maximum number of rewrites
// ----------------------------------------------------------------------------
{
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "Digits %u", Settings.fracprec);
    return buffer;
}
