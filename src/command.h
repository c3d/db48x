#ifndef COMMAND_H
#define COMMAND_H
// ****************************************************************************
//  command.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Description of an RPL command
//
//     All RPL commands take input on the stack and emit results on the stack
//     There are facilities for type checking the stack inputs
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
//
//     Unlike traditional RPL, commands are case-insensitive, i.e. you can
//     use either "DUP" or "dup". There is a setting to display them as upper
//     or lowercase. The reason is that on the DM42, lowercases look good.
//
//     Additionally, many commands also have a long form. There is also an
//     option to prefer displaying as long form. This does not impact encoding,
//     and when typing programs, you can always use the short form

#include "object.h"
#include "runtime.h"

RECORDER_DECLARE(command);

struct command : object
// ----------------------------------------------------------------------------
//   Shared logic for all commands
// ----------------------------------------------------------------------------
{
    command(id i): object(i) {}

    template<typename Obj>
    const Obj *arg(uint level = 0, Obj *def = nullptr)
    // ------------------------------------------------------------------------
    //   Return the arg at a given level on the stack, or default value
    // ------------------------------------------------------------------------
    {
        const Obj *obj = rt.stack(level);
        if (obj && obj->type() == Obj::static_id)
            return (Obj *) obj;
        return def;
    }

    // Get the top of the stack as an integer
    static uint32_t uint32_arg(uint level = 0);
    static int32_t  int32_arg (uint level = 0);

    // Execute a command
    static result   evaluate()    { return OK; }

    // Find the commadn object ID associated with a given spelling
    static id       lookup(utf8 name, size_t &len, bool eq=false);

public:
    PARSE_DECL(command);
    RENDER_DECL(command);

public:
    // Sorting command IDs for faster lookup, used in the catalog and parsing
    static uint16_t *sorted_ids;
    static size_t    sorted_ids_count;

    static bool      initialize_sorted_ids();
};


// Macro to defined a simple command handler for derived classes
#define COMMAND_DECLARE_SPECIAL(derived, base, nargs, special)          \
struct derived : base                                                   \
{                                                                       \
    derived(id i = ID_##derived) : base(i) { }                          \
                                                                        \
    OBJECT_DECL(derived);                                               \
    ARITY_DECL(nargs >= 0 ? nargs : ~nargs);                            \
    EVAL_DECL(derived)                                                  \
    {                                                                   \
        record(command, "Evaluating " #derived " command %t", o);       \
        rt.command(o);                                                  \
        if (nargs >= 0 && !rt.args(nargs))                              \
            return ERROR;                                               \
        return evaluate();                                              \
    }                                                                   \
    special                                                             \
    static result evaluate();                                           \
}

#define COMMAND_DECLARE(derived, nargs)               \
    COMMAND_DECLARE_SPECIAL(derived, command, nargs, )

#define COMMAND_DECLARE_INSERT(derived, nargs)          \
    COMMAND_DECLARE_SPECIAL(derived, command, nargs,    \
                            INSERT_DECL(derived);)

#define COMMAND_DECLARE_INSERT_HELP(derived, nargs)     \
    COMMAND_DECLARE_SPECIAL(derived, command, nargs,    \
                            INSERT_DECL(derived);       \
                            HELP_DECL(derived);)

#define COMMAND_BODY(derived)                   \
    object::result derived::evaluate()

#define COMMAND(derived, nargs)                         \
    COMMAND_DECLARE(derived, nargs);                    \
    inline COMMAND_BODY(derived)



// ============================================================================
//
//    Some basic commands
//
// ============================================================================

struct Unimplemented : command
// ----------------------------------------------------------------------------
//   Used for unimplemented commands, e.g. in menus
// ----------------------------------------------------------------------------
{
    Unimplemented(id i = ID_Unimplemented) : command(i) { }

    OBJECT_DECL(Unimplemented);
    EVAL_DECL(Unimplemented);
    MARKER_DECL(Unimplemented);
};

// Various global commands
COMMAND_DECLARE(Eval,1);                // Evaluate an object
COMMAND_DECLARE(Run, 0);                // Resume execution or evaluate
COMMAND_DECLARE(Compile,1);             // Compile and evalaute a text
COMMAND_DECLARE(Explode,1);             // Explode an object (aka Obj→)
COMMAND_DECLARE(ToText,1);              // Convert an object to text
COMMAND_DECLARE(ToProgram,1);           // Convert expression to program
COMMAND_DECLARE(SelfInsert,-1);         // Enter menu label in the editor
COMMAND_DECLARE(ReplaceChar,-1);        // Replace editor character with label
COMMAND_DECLARE(Ticks,0);               // Return number of ticks
COMMAND_DECLARE(Wait,1);                // Wait a given amount of time
COMMAND_DECLARE(Bytes,1);               // Return bytes for object
COMMAND_DECLARE(Type,1);                // Return the type of the object
COMMAND_DECLARE(TypeName,1);            // Return the type name of the object
COMMAND_DECLARE(Off,-1);                // Switch the calculator off
COMMAND_DECLARE(OffWithImage,-1);       // ... and show off-images
COMMAND_DECLARE(SaveState, -1);         // Save state to disk
COMMAND_DECLARE(BatteryVoltage, 0);     // Return battery voltage
COMMAND_DECLARE(PowerVoltage, 0);       // Return power voltage
COMMAND_DECLARE(USBPowered, 0);         // Return true if on battery
COMMAND_DECLARE(DMCPLowBattery, 0);     // Return DMCP low-battery indicator
COMMAND_DECLARE(LowBattery, 0);         // Return true if battery is low
COMMAND_DECLARE(SystemSetup,-1);        // Select the system menu
COMMAND_DECLARE(ScreenCapture,-1);      // Snapshot screen state to a file
COMMAND_DECLARE(Beep,2);                // Emit a sound (if enabled)
COMMAND_DECLARE(Version,0);             // Return a version string
COMMAND_DECLARE(Help,-1);               // Activate online help
COMMAND_DECLARE(LastArg,-1);            // Return last arguments
COMMAND_DECLARE(LastX,-1);              // Return last X argument
COMMAND_DECLARE(Undo,-1);               // Revert to the Undo stack
COMMAND_DECLARE(Cycle,1);               // Cycle among representations
COMMAND_DECLARE(BinaryToReal,1);        // Convert binary to real
COMMAND_DECLARE(RealToBinary,1);        // Convert real to binary

COMMAND_DECLARE(EditorSelect,-1);       // Select from current cursor position
COMMAND_DECLARE(EditorWordLeft,-1);     // Move cursor one word left
COMMAND_DECLARE(EditorWordRight,-1);    // Move cursor one word right
COMMAND_DECLARE(EditorBegin,-1);        // Move cursor to beginning of buffer
COMMAND_DECLARE(EditorEnd,-1);          // Move cursor to end of buffer
COMMAND_DECLARE(EditorCut,-1);          // Cut current selection
COMMAND_DECLARE(EditorCopy,-1);         // Copy current selection
COMMAND_DECLARE(EditorPaste,-1);        // Paste to cursor position
COMMAND_DECLARE(EditorSearch,-1);       // Begin search
COMMAND_DECLARE(EditorReplace,-1);      // Replace search with cursor
COMMAND_DECLARE(EditorClear,-1);        // Clear editor
COMMAND_DECLARE(EditorFlip,-1);         // Flip cursor and selection
COMMAND_DECLARE(EditorHistory,-1);      // Find last entry in editor history
COMMAND_DECLARE(EditorHistoryBack,-1);  // Find previous entry in editor history

COMMAND_DECLARE(Edit,-1);               // Edit top level object
COMMAND_DECLARE(StackEditor,-1);        // Enter interactive stack

COMMAND_DECLARE(UILeftShift,-1);        // Set left shift
COMMAND_DECLARE(UIRightShift,-1);       // Set right shift
COMMAND_DECLARE(UINoShift,-1);          // Set no shift
COMMAND_DECLARE(UIAlpha,-1);            // Set alpha
COMMAND_DECLARE(UILowercase,-1);        // Set lowercase alpha
COMMAND_DECLARE(UINoAlpha,-1);          // Set no alpha
COMMAND_DECLARE(UIAlgebraic,-1);        // Algebraic key (ticks and parens)
COMMAND_DECLARE(UIEnter,-1);            // Enter current line (end of editing)
COMMAND_DECLARE(UIExit,-1);             // Exit current editing session
COMMAND_DECLARE(UIDecimal,-1);          // Insert decimal dot for current mode
COMMAND_DECLARE(UIText,-1);             // Insert text in the editor
COMMAND_DECLARE(UILeft,-1);             // Actions related to left key
COMMAND_DECLARE(UIRight,-1);            // Actions related to right key
COMMAND_DECLARE(UIUp,-1);               // Actions related to up key
COMMAND_DECLARE(UIDown,-1);             // Actions related to down key
COMMAND_DECLARE(UIBackspace,-1);        // Actions related to backspace
COMMAND_DECLARE(UIDelete,-1);           // Action related to delete

#endif // COMMAND_H
