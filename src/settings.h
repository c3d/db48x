#ifndef SETTINGS_H
#define SETTINGS_H
// ****************************************************************************
//  settings.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     List of system-wide settings
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

#include "command.h"
#include "menu.h"

#include <types.h>


struct renderer;

struct settings
// ----------------------------------------------------------------------------
//    Internal representation of settings
// ----------------------------------------------------------------------------
{
    enum { STD_DISPLAYED = 20 };

    enum
    {
        // Try hard to make source code unreadable
        SPACE_3_PER_EM          = L' ',
        SPACE_4_PER_EM          = L' ',
        SPACE_6_PER_EM          = L' ',
        SPACE_THIN              = L' ',
        SPACE_MEDIUM_MATH       = L' ',

        SPACE_DEFAULT           = SPACE_MEDIUM_MATH,

        MARK                    = L'●', // L'■'
        COMPLEX_I               = L'𝒊',
        DEGREES_SYMBOL          = L'°',
        RADIANS_SYMBOL          = L'ℼ',
        GRAD_SYMBOL             = L'ℊ',
        PI_RADIANS_SYMBOL       = L'π',
    };

    settings()
        : precision(BID128_MAXDIGITS),
          display_mode(NORMAL),
          displayed(STD_DISPLAYED),
          spacing_mantissa(3),
          spacing_fraction(5),
          spacing_based(4),
          decimal_mark('.'),
          exponent_mark(L'⁳'),
          space(SPACE_DEFAULT),
          space_based(SPACE_DEFAULT),
          standard_exp(9),
          angle_mode(DEGREES),
          base(16),
          wordsize(64),
          maxbignum(1024),
          maxrewrites(100),
          fraciter(10),
          fracprec(12),
          command_fmt(LONG_FORM),
          show_decimal(true),
          fancy_exponent(true),
          auto_simplify(true),
          show_time(true),
          show_24h(true),
          show_seconds(true),
          show_date(DMY),
          show_dow(true),
          show_month(true),
          show_voltage(true),
          date_separator('/'),
          result_sz(STACK),
          stack_sz(STACK),
          editor_sz(EDITOR),
          editor_ml_sz(STACK)
    {}

    enum angles
    // ------------------------------------------------------------------------
    //   The base used for angles
    // ------------------------------------------------------------------------
    {
        DEGREES,
        RADIANS,
        GRADS,
        PI_RADIANS,
        NUM_ANGLES,
    };

    enum display
    // ------------------------------------------------------------------------
    //   The display mode for numbers
    // ------------------------------------------------------------------------
    {
        NORMAL, FIX, SCI, ENG
    };

    enum commands
    // ------------------------------------------------------------------------
    //   Display of commands
    // ------------------------------------------------------------------------
    {
        LOWERCASE,              // Display the short name in lowercase
        UPPERCASE,              // Display the short name in uppercase
        CAPITALIZED,            // Display the short name capitalized
        LONG_FORM,              // Display the long form
    };

    enum dmy_ord
    // ------------------------------------------------------------------------
    //   Order for date, month and year
    // ------------------------------------------------------------------------
    {
        NO_DATE, DMY, MDY, YMD
    };

    enum font_id
    // ------------------------------------------------------------------------
    //  Selection of font size for the stack
    // ------------------------------------------------------------------------
    {
        EDITOR, STACK, HELP,
        LIB28, LIB25, LIB22, LIB20, LIB18, LIB17,
        SKR24, SKR18,
        FREE42,
        NUM_FONTS
    };

    static font_p font(font_id sz);
    static font_p cursor_font(font_id sz);
    font_p result_font()        { return font(result_sz); }
    font_p stack_font()         { return font(stack_sz); }
    font_p editor_font(bool ml) { return font(ml ? editor_ml_sz : editor_sz); }
    font_p cursor_font(bool ml) { return cursor_font(ml ? editor_ml_sz : editor_sz); }

    void save(renderer &out, bool show_defaults = false);


public:
    uint16_t precision;         // Internal precision for numbers
    display  display_mode;      // Display mode
    uint8_t  displayed;         // Number of displayed digits
    uint8_t  spacing_mantissa;  // Spacing for the mantissa (0 = none)
    uint8_t  spacing_fraction;  // Spacing for the fraction (0 = none)
    uint8_t  spacing_based;     // Spacing for based numbers
    char     decimal_mark;      // Character used for decimal separator
    unicode  exponent_mark;     // The character used to represent exponents
    unicode  space;             // Space to use for normal numbers
    unicode  space_based;       // Space to use for based numbers
    uint16_t standard_exp;      // Maximum exponent before switching to sci
    angles   angle_mode;        // Angle mode ( degrees, radians or grads)
    uint8_t  base;              // The default base for #numbers
    size_t   wordsize;          // Wordsize for binary numbers (in bits)
    size_t   maxbignum;         // Maximum size for a bignum (in bits)
    uint16_t maxrewrites;       // Maximum number of rewrites
    uint16_t fraciter;          // Number of iterations for ->Q
    uint16_t fracprec;          // Number of digits for ->Q
    commands command_fmt;       // How we prefer to display commands
    bool     show_decimal   :1; // Show decimal dot for integral real numbers
    bool     fancy_exponent :1; // Show exponent with fancy superscripts
    bool     auto_simplify  :1; // Automatically simplify symbolic results
    bool     show_time      :1; // Show time in status bar
    bool     show_24h       :1; // Show 24-hours clock
    bool     show_seconds   :1; // Show seconds in status bar
    dmy_ord  show_date      :2; // Order for date, month and year
    bool     show_dow       :1; // Show day of week in status bar
    bool     show_month     :1; // Show month name in status bar
    bool     show_voltage   :1; // Show battery voltage in status bar
    char     date_separator;    // Date separator
    font_id  result_sz;         // Size for stack top
    font_id  stack_sz;          // Size for other stack levels
    font_id  editor_sz;         // Size for normal editor
    font_id  editor_ml_sz;      // Size for editor in multi-line mode
};


extern settings Settings;


// Macro to defined a simple command handler for derived classes
#define SETTINGS_COMMAND_DECLARE(derived)               \
    struct derived : command                            \
    {                                                   \
        derived(id i = ID_##derived) : command(i) {}    \
                                                        \
        OBJECT_DECL(derived);                           \
        EVAL_DECL(derived)                              \
        {                                               \
            rt.command(fancy(ID_##derived));            \
            ui.menuNeedsRefresh();                   \
            return evaluate();                          \
        }                                               \
        EXEC_DECL(derived)                              \
        {                                               \
            return do_evaluate(o);                      \
        }                                               \
        MARKER_DECL(derived);                           \
                                                        \
        static result  evaluate();                      \
        static cstring menu_label(menu::info &mi);      \
    }

#define SETTINGS_COMMAND_BODY(derived, mkr)     \
    MARKER_BODY(derived)                        \
    {                                           \
        return mkr ? settings::MARK : 0;        \
    }                                           \
    object::result derived::evaluate()

#define SETTINGS_COMMAND_NOLABEL(derived, mkr)          \
    MARKER_BODY(derived)                                \
    {                                                   \
        return mkr ? settings::MARK : 0;                \
    }                                                   \
    cstring derived::menu_label(menu::info UNUSED &mi)  \
    {                                                   \
        return #derived;                                \
    }                                                   \
    object::result derived::evaluate()

#define SETTINGS_COMMAND_LABEL(derived)                 \
    cstring derived::menu_label(menu::info UNUSED &mi)


COMMAND_DECLARE(Modes);

SETTINGS_COMMAND_DECLARE(Std);
SETTINGS_COMMAND_DECLARE(Fix);
SETTINGS_COMMAND_DECLARE(Sci);
SETTINGS_COMMAND_DECLARE(Eng);
SETTINGS_COMMAND_DECLARE(Sig);

SETTINGS_COMMAND_DECLARE(Deg);
SETTINGS_COMMAND_DECLARE(Rad);
SETTINGS_COMMAND_DECLARE(Grad);
SETTINGS_COMMAND_DECLARE(PiRadians);

SETTINGS_COMMAND_DECLARE(LowerCase);
SETTINGS_COMMAND_DECLARE(UpperCase);
SETTINGS_COMMAND_DECLARE(Capitalized);
SETTINGS_COMMAND_DECLARE(LongForm);

SETTINGS_COMMAND_DECLARE(DecimalDot);
SETTINGS_COMMAND_DECLARE(DecimalComma);
SETTINGS_COMMAND_DECLARE(NoTrailingDecimal);
SETTINGS_COMMAND_DECLARE(TrailingDecimal);
SETTINGS_COMMAND_DECLARE(Precision);
SETTINGS_COMMAND_DECLARE(StandardExponent);
SETTINGS_COMMAND_DECLARE(FancyExponent);
SETTINGS_COMMAND_DECLARE(ClassicExponent);

SETTINGS_COMMAND_DECLARE(Base);
SETTINGS_COMMAND_DECLARE(Bin);
SETTINGS_COMMAND_DECLARE(Oct);
SETTINGS_COMMAND_DECLARE(Dec);
SETTINGS_COMMAND_DECLARE(Hex);

SETTINGS_COMMAND_DECLARE(stws);
COMMAND_DECLARE(rcws);

SETTINGS_COMMAND_DECLARE(ResultFontSize);
SETTINGS_COMMAND_DECLARE(StackFontSize);
SETTINGS_COMMAND_DECLARE(EditorFontSize);
SETTINGS_COMMAND_DECLARE(EditorMultilineFontSize);

COMMAND_DECLARE(NumberSpacing);
SETTINGS_COMMAND_DECLARE(MantissaSpacing);
SETTINGS_COMMAND_DECLARE(FractionSpacing);
SETTINGS_COMMAND_DECLARE(BasedSpacing);

SETTINGS_COMMAND_DECLARE(NumberSpaces);
SETTINGS_COMMAND_DECLARE(NumberDotOrComma);
SETTINGS_COMMAND_DECLARE(NumberTicks);
SETTINGS_COMMAND_DECLARE(NumberUnderscore);
SETTINGS_COMMAND_DECLARE(BasedSpaces);
SETTINGS_COMMAND_DECLARE(BasedDotOrComma);
SETTINGS_COMMAND_DECLARE(BasedTicks);
SETTINGS_COMMAND_DECLARE(BasedUnderscore);

SETTINGS_COMMAND_DECLARE(AutoSimplify);
SETTINGS_COMMAND_DECLARE(NoAutoSimplify);

SETTINGS_COMMAND_DECLARE(MaxBigNumBits);
SETTINGS_COMMAND_DECLARE(MaxRewrites);

SETTINGS_COMMAND_DECLARE(ToFractionIterations);
SETTINGS_COMMAND_DECLARE(ToFractionDigits);

#endif // SETTINGS_H
