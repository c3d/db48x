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
#include "renderer.h"
#include "target.h"
#include "types.h"

struct renderer;

RECORDER_DECLARE(settings);

#define DB48X_MAXDIGITS    9999
#define DB48X_MAXEXPONENT  (1LL<<60)


struct settings
// ----------------------------------------------------------------------------
//    Internal representation of settings
// ----------------------------------------------------------------------------
{
public:
    using id = object::id;

#ifdef CONFIG_COLOR
#define BLACK(r, g, b)  (pattern(r,g,b).bits)
#define WHITE(r, g, b)  (pattern(r,g,b).bits)
#define GRAY25(r,g, b)  (pattern(r,g,b).bits)
#define GRAY50(r,g, b)  (pattern(r,g,b).bits)
#define GRAY75(r,g, b)  (pattern(r,g,b).bits)
#define GRAY90(r,g, b)  (pattern(r,g,b).bits)
#else // !CONFIG_COLOR
#define BLACK(r, g, b)   ularge(0)
#define WHITE(r, g, b)  ~ularge(0)
#define GRAY25(r,g, b)  (ularge(pattern::gray25.bits))
#define GRAY50(r,g, b)  (ularge(pattern::gray50.bits))
#define GRAY75(r,g, b)  (ularge(pattern::gray75.bits))
#define GRAY90(r,g, b)  (ularge(pattern::gray90.bits))
#endif // CONFIG_COLOR


#define ID(id)
#define FLAG(Enable, Disable)
#define SETTING(Name, Low, High, Init)                typeof(Init) Name##_bits;
#define SETTING_BITS(Name,Type,Bits,Low,High,Init)
#include "ids.tbl"

    // Define the packed bits settings
#define ID(id)
#define FLAG(Enable, Disable)
#define SETTING(Name, Low, High, Init)
#define SETTING_BITS(Name,Type,Bits,Low,High,Init)    uint Name##_bits : Bits;
#include "ids.tbl"

    // Define the flags
#define ID(id)
#define FLAG(Enable, Disable)                         bool Enable##_bit : 1;
#define SETTING(Name, Low, High, Init)
#define SETTING_BITS(Name,Type,Bits,Low,High,Init)
#include "ids.tbl"

    bool reserved : 1;

public:
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
        SPACE_UNIT              = SPACE_6_PER_EM,
        CONSTANT_MARKER         = L'Ⓒ',
        EQUATION_MARKER         = L'Ⓔ',
        XLIB_MARKER             = L'Ⓛ',

        MARK                    = L'●', // L'■'
        CLEAR_MARK              = L'○',
        COMPLEX_I               = L'ⅈ',
        DEGREES_SYMBOL          = L'°',
        RADIANS_SYMBOL          = L'ʳ', // ʳʳ'
        GRAD_SYMBOL             = L'ℊ',
        PI_RADIANS_SYMBOL       = L'ℼ',
    };

    enum font_id
    // ------------------------------------------------------------------------
    //  Selection of font size for the stack
    // ------------------------------------------------------------------------
    {
        HELP, REDUCED, STACK, EDITOR,
        LIB28, LIB25, LIB22, LIB20, LIB18, LIB17,
        SKR24, SKR18,
        FREE42,
        FIRST_FONT = HELP,
        LAST_FONT = FREE42,
        NUM_FONTS
    };


    static font_id smaller_font(font_id f)
    // ------------------------------------------------------------------------
    //   Find the next smaller font
    // ------------------------------------------------------------------------
    {
        switch (f)
        {
        default:
        case HELP:
        case REDUCED:   return HELP;
        case STACK:     return REDUCED;
        case EDITOR:    return STACK;
        case LIB17:
        case LIB18:     return LIB17;
        case LIB20:     return LIB18;
        case LIB22:     return LIB20;
        case LIB25:     return LIB22;
        case LIB28:     return LIB25;
        case SKR18:
        case SKR24:     return SKR18;
        case FREE42:    return FREE42;
        }
    }

#define ID(i)   static const object::id ID_##i = object::ID_##i;
#include "ids.tbl"


    settings();

    // Hash value, used to check if settings changed
    uint hash() const;

    // Accessor functions
#define ID(id)
#define FLAG(Enable,Disable)                                            \
    bool Enable() const                 { return Enable##_bit; }        \
    bool Disable() const                { return !Enable##_bit; }       \
    void Enable(bool flag)              { Enable##_bit = flag; }        \
    void Disable(bool flag)             { Enable##_bit = !flag; }
#define SETTING(Name, Low, High, Init)                                  \
    typeof(Init) Name() const           { return Name##_bits;  }        \
    void Name(typeof(Init) value)       { Name##_bits = value; }
#define SETTING_BITS(Name, Type, Bits, Low, High, Init)                 \
    Type Name() const           { return (Type)(Low+Name##_bits);  }    \
    void Name(Type value)       { Name##_bits = value - Low; }
#include "ids.tbl"

    static font_p font(font_id sz);
    static font_p cursor_font(font_id sz);
    font_p result_font()        { return font(ResultFont()); }
    font_p stack_font()         { return font(StackFont()); }
    font_p editor_font(bool ml) { return font(ml ? MultilineEditorFont() : EditorFont()); }
    font_p cursor_font(bool ml) { return cursor_font(ml ? MultilineCursorFont() : CursorFont()); }
    font_p header_font()        { return font(HeaderFont()); }

    static unicode digit_separator(uint index);

    unicode NumberSeparator() const
    {
        return digit_separator(NumberSeparatorCommand() - object::ID_NumberSpaces);
    }
    unicode BasedSeparator() const
    {
        return digit_separator(BasedSeparatorCommand() - object::ID_BasedSpaces);
    }
    unicode DecimalSeparator() const
    {
        return DecimalComma() ? ',' : '.';
    }
    cstring DecimalSeparatorString() const
    {
        return DecimalComma() ? "," : ".";
    }
    unicode ExponentSeparator() const
    {
        return FancyExponent() ? L'⁳' : 'E';
    }

    char DateSeparator() const
    {
        uint index = DateSeparatorCommand() - object::ID_DateSlash;
        static char sep[4] = { '/', '-', '.', ' ' };
        return sep[index];
    }

    void NextDateSeparator()
    {
        DateSeparatorCommand_bits++;
    }

    void save(renderer &out, bool show_defaults = false);

    static bool     store(object::id name, object_p value);
    static object_p recall(object::id name);
    static bool     purge(object::id name);

    static bool     flag(object::id name, bool value);
    static bool     flag(object::id name, bool *value);

    static unicode  mark(bool flag) { return flag ? MARK : CLEAR_MARK; }


    // Utility classes to save the individual settings
#define ID(id)
#define FLAG(Enable,Disable)                    \
    SETTING(Enable,  false, true, false)        \
    SETTING(Disable, false, true, true)
#define SETTING_BITS(Name, Type, Bits, Low, High, Init) \
    struct Save##Name                                   \
    {                                                   \
        Save##Name(Type value);                         \
        ~Save##Name();                                  \
        Type saved;                                     \
    };
#define SETTING(Name, Low, High, Init)                  \
    SETTING_BITS(Name, typeof(Init), UNUSED, Low, High, Init)
#include "ids.tbl"

    struct PrepareForProgramEvaluation
    {
        SaveSaveLastArguments    saveLastArgs;
        SaveProgramLastArguments saveProgramLastArg;
        SaveSaveStack            saveLastStack;
        SaveDebugOnError         saveDebugOnError;
        ::save<uint>             saveStepping;
        ::save<bool>             saveHalted;

        PrepareForProgramEvaluation();
    };
    struct PrepareForSolveFunctionEvaluation : PrepareForProgramEvaluation
    {
        SaveNumericalResults     saveNumericalResults;

        PrepareForSolveFunctionEvaluation()
            : PrepareForProgramEvaluation(),
              saveNumericalResults(true)
        {}
    };
    struct PrepareForFunctionEvaluation : PrepareForSolveFunctionEvaluation
    {
        SaveSetAngleUnits     saveAngleUnits;

        PrepareForFunctionEvaluation()
            : PrepareForSolveFunctionEvaluation(),
              saveAngleUnits(false)
        {}
    };
};


extern settings Settings;

// Utility class to save the individual settings
#define ID(id)
#define FLAG(Enable,Disable)                    \
    SETTING(Enable,  false, true, false)        \
    SETTING(Disable, false, true, true)
#define SETTING_BITS(Name, Type, Bits, Low, High, Init)         \
    inline settings::Save##Name::Save##Name(Type value)         \
        : saved(Settings.Name())                                \
    {                                                           \
        Settings.Name(value);                                   \
    }                                                           \
    inline settings::Save##Name::~Save##Name()                  \
    {                                                           \
        Settings.Name(saved);                                   \
    }
#define SETTING(Name, Low, High, Init)                          \
    SETTING_BITS(Name, typeof(Init), UNUSED, Low, High, Init)
#include "ids.tbl"


template<typename T>
T setting_value(object_p obj, T init)
{
    return T(obj->as_uint32(init, true));
}

template <>
ularge setting_value<ularge>(object_p obj, ularge init);

template <>
int setting_value<int>(object_p obj, int init);

template <>
object::id setting_value<object::id>(object_p obj, object::id init);


struct setting : command
// ----------------------------------------------------------------------------
//   Shared code for settings
// ----------------------------------------------------------------------------
{
    setting(id i) : command(i) {}
    static result update(id ty)
    {
        rt.command(static_object(ty));
        ui.menu_refresh();
        return OK;
    }

    template<typename T>
    static bool validate(id type, T &valref, T low, T high)
    {
        rt.command(static_object(type));
        if (rt.args(1))
        {
            if (object_p obj = rt.top())
            {
                T val = setting_value(obj, T(valref));
                if (!rt.error())
                {
                    if (val >= low && val <= high)
                    {
                        valref = T(val);
                        rt.pop();
                        return true;
                    }
                    rt.domain_error();
                }
            }
        }
        return false;
    }

    static cstring label(id ty);
    static cstring printf(cstring format, ...);
    ARITY_DECL(1);
};


struct value_setting : setting
// ----------------------------------------------------------------------------
//   Use a setting value
// ----------------------------------------------------------------------------
{
    value_setting(id type): setting(type) {}
    EVAL_DECL(value_setting);
};

#define ID(i)

#define FLAG(Enable, Disable)                                           \
                                                                        \
struct Enable : setting                                                 \
{                                                                       \
    Enable(id i = ID_##Enable): setting(i) {}                           \
    OBJECT_DECL(Enable);                                                \
    EVAL_DECL(Enable)                                                   \
    {                                                                   \
        record(settings, "Evaluating " #Enable " enable %t", o);        \
        if (ui.evaluating_function_key())                               \
            Settings.Enable(!Settings.Enable());                        \
        else                                                            \
            Settings.Enable(true);                                      \
        return update(ID_##Enable);                                     \
    }                                                                   \
    MARKER_DECL(Enable)                                                 \
    {                                                                   \
        return Settings.mark(Settings.Enable());                        \
    }                                                                   \
};                                                                      \
                                                                        \
struct Disable : setting                                                \
{                                                                       \
    Disable(id i = ID_##Disable): setting(i) {}                         \
    OBJECT_DECL(Disable);                                               \
    EVAL_DECL(Disable)                                                  \
    {                                                                   \
        record(settings, "Evaluating " #Disable " disable %t", o);      \
        if (ui.evaluating_function_key())                               \
            Settings.Disable(!Settings.Disable());                      \
        else                                                            \
            Settings.Disable(true);                                     \
        return update(ID_##Disable);                                    \
    }                                                                   \
    MARKER_DECL(Disable)                                                \
    {                                                                   \
        return Settings.mark(Settings.Disable());                       \
    }                                                                   \
};


#define SETTING(Name, Low, High, Init)                                  \
                                                                        \
struct Name : setting                                                   \
{                                                                       \
    Name(id i = ID_##Name) : setting(i) {}                              \
                                                                        \
    OBJECT_DECL(Name);                                                  \
    EVAL_DECL(Name)                                                     \
    {                                                                   \
        record(settings, "Evaluating " #Name " setting %t", o);         \
        using type = typeof(Settings.Name());                           \
        type value = Settings.Name();                                   \
        if (!validate(ID_##Name, value, type(Low), type(High)))         \
            return ERROR;                                               \
        Settings.Name(value);                                           \
        update(ID_##Name);                                              \
        return OK;                                                      \
    }                                                                   \
};

#define SETTING_VALUE(Name, Alias, Base, Value)                 \
    struct Name : value_setting                                 \
    {                                                           \
        Name(id ty = ID_##Name) : value_setting(ty) {  }        \
        OBJECT_DECL(Name);                                      \
        MARKER_DECL(Name)                                       \
        {                                                       \
            return Settings.Base() == Value;                    \
        }                                                       \
    };

#include "ids.tbl"



COMMAND_DECLARE(Modes,0);
COMMAND_DECLARE(ResetModes,0);
COMMAND_DECLARE(RecallWordSize,0);

#endif // SETTINGS_H
