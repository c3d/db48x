// ****************************************************************************
//  tests.cc                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Tests for the runtime
//
//     The tests are run by actually sending keystrokes and observing the
//     calculator's state
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

#include "tests.h"

#include "dmcp.h"
#include "equations.h"
#include "list.h"
#include "recorder.h"
#include "settings.h"
#include "sim-dmcp.h"
#include "stack.h"
#include "types.h"
#include "user_interface.h"

#include <regex.h>
#include <stdio.h>

extern bool run_tests;
volatile uint test_command = 0;

RECORDER(tests, 256, "Information about tests");
RECORDER_TWEAK_DEFINE(snapshots, 0, "Record snapshots for failing tests");
RECORDER_DECLARE(errors);

uint    tests::default_wait_time  = 1000;
uint    tests::key_delay_time     = 0;
uint    tests::refresh_delay_time = 20;
uint    tests::image_wait_time    = 500;
cstring tests::dump_on_fail       = nullptr;
bool    tests::running            = false;

#define TEST_CATEGORY(name, enabled, descr)                     \
    RECORDER_TWEAK_DEFINE(est_##name, enabled, "Test " descr);  \
    static inline bool check_##name(tests &t)                   \
    {                                                           \
        bool result = RECORDER_TWEAK(est_##name);               \
        if (!result)                                            \
            t.begin("Skipping " #name ": " descr, true);        \
        else                                                    \
            t.begin(#name ": " descr);                          \
        return result;                                          \
    }

#define TESTS(name, descr)      TEST_CATEGORY(name, true,  descr)
#define EXTRA(name, descr)      TEST_CATEGORY(name, false, descr)

#define BEGIN(name)                             \
    do                                          \
    {                                           \
        position(__FILE__, __LINE__);           \
        if (!check_##name(*this))               \
            return;                             \
    } while (0)

TESTS(defaults,         "Reset settings to defaults");
TESTS(demo_ui,          "Demo of DB48X user interface");
TESTS(demo_math,        "Demo of DB48X math capabilities");
TESTS(demo_pgm,         "Demo of DB48X programming");
TESTS(shifts,           "Shift logic");
TESTS(keyboard,         "Keyboard entry");
TESTS(types,            "Data types");
TESTS(editor,           "Editor operations");
TESTS(istack,           "Interactive stack operations");
TESTS(stack,            "Stack operations");
TESTS(arithmetic,       "Arithmetic operations");
TESTS(globals,          "Global variables");
TESTS(locals,           "Local variables");
TESTS(for_loops,        "For loops");
TESTS(conditionals,     "Conditionals");
TESTS(logical,          "Logical operations");
TESTS(styles,           "Commands display formats");
TESTS(iformat,          "Integer display formats");
TESTS(fformat,          "Fraction display formats");
TESTS(dformat,          "Decimal display formats");
TESTS(ifunctions,       "Integer functions");
TESTS(dfunctions,       "Decimal functions");
TESTS(float,            "Hardware-accelerated 7-digit (float)")
TESTS(double,           "Hardware-accelerated 16-digit (double)")
TESTS(highp,            "High-precision computations (60 digits)")
TESTS(trigoptim,        "Special trigonometry optimzations");
TESTS(trigunits,        "Trigonometric units");
TESTS(dfrac,            "Simple conversion to decimal and back");
TESTS(round,            "Rounding and truncating");
TESTS(ctypes,           "Complex types");
TESTS(carith,           "Complex arithmetic");
TESTS(cfunctions,       "Complex functions");
TESTS(autocplx,         "Automatic complex promotion");
TESTS(ranges,           "Range types");
TESTS(units,            "Units and conversions");
TESTS(lists,            "List operations");
TESTS(sorting,          "Sorting operations");
TESTS(text,             "Text operations");
TESTS(vectors,          "Vectors");
TESTS(matrices,         "Matrices");
TESTS(solver,           "Solver");
TESTS(cstlib,           "Built-in constants parsing");
TESTS(equations,        "Built-in equations");
TESTS(colnbeams,        "Columns and Beams equations in library");
TESTS(integrate,        "Numerical integration");
TESTS(syminteg,         "Numerical integration with symbolic primitive");
TESTS(simplify,         "Auto-simplification of expressions");
TESTS(rewrites,         "Equation rewrite engine");
TESTS(symbolic,         "Symbolic operations");
TESTS(derivative,       "Symbolic differentiation");
TESTS(primitive,        "Symbolic integration (primitive)");
TESTS(tagged,           "Tagged objects");
TESTS(catalog,          "Catalog of commands");
TESTS(cycle,            "Cycle command for quick conversions");
TESTS(rotate,           "Shift and rotate instructions");
TESTS(flags,            "User flags");
TESTS(explode,          "Extracting object structure");
TESTS(finance,          "Financial functions and solver");
TESTS(regressions,      "Regression checks");
TESTS(plotting,         "Plotting, graphing and charting");
TESTS(graphics,         "Graphic commands");
TESTS(offline,          "Off-line graphics");
TESTS(input,            "User input");
TESTS(help,             "On-line help");
TESTS(gstack,           "Graphic stack rendering")
TESTS(hms,              "HMS and DMS operations");
TESTS(date,             "Date operations");
TESTS(infinity,         "Infinity and undefined operations");
TESTS(overflow,         "Overflow and underflow");
TESTS(insert,           "Insertion of variables, units and constants");
TESTS(constants,        "Check the value of all built-in constants");
TESTS(characters,       "Character menu and catalog");
TESTS(statistics,       "Statistics");
TESTS(probabilities,    "Probabilities");
TESTS(sumprod,          "Sums and products");
TESTS(poly,             "Polynomials");
TESTS(quorem,           "Quotient and remainder");
TESTS(expr,             "Operations on expressions");
TESTS(random,           "Random number generation");
TESTS(library,          "Library entries");
TESTS(examples,         "On-line help examples");

EXTRA(plotfns,          "Plot all functions");
EXTRA(sysflags,         "Enable/disable every RPL flag");
EXTRA(settings,         "Recall and activate every RPL setting");
EXTRA(commands,         "Parse every single RPL command");


void tests::run(uint onlyCurrent)
// ----------------------------------------------------------------------------
//   Run all test categories
// ----------------------------------------------------------------------------
{
    save<bool> markRunning(running, true);
    rpl_command(START_TEST);

    tindex = sindex = cindex = count = 0;
    failures.clear();

    auto tracing           = RECORDER_TRACE(errors);
    RECORDER_TRACE(errors) = false;

    // Reset to known settings state
    reset_settings();
    if (onlyCurrent)
    {
        here().begin("Current");
        if (onlyCurrent & 1)
            range_types();

#if 0
        if (onlyCurrent & 2)
            demo_ui();
        if (onlyCurrent & 4)
            demo_math();
        if (onlyCurrent & 8)
            demo_pgm();
#endif
    }
    else
    {
        shift_logic();
        keyboard_entry();
        data_types();
        editor_operations();
        stack_operations();
        interactive_stack_operations();
        arithmetic();
        global_variables();
        local_variables();
        for_loops();
        conditionals();
        logical_operations();
        command_display_formats();
        integer_display_formats();
        fraction_display_formats();
        decimal_display_formats();
        integer_numerical_functions();
        decimal_numerical_functions();
        float_numerical_functions();
        double_numerical_functions();
        high_precision_numerical_functions();
        exact_trig_cases();
        trig_units();
        fraction_decimal_conversions();
        rounding_and_truncating();
        complex_types();
        complex_arithmetic();
        complex_functions();
        complex_promotion();
        range_types();
        units_and_conversions();
        list_functions();
        sorting_functions();
        vector_functions();
        matrix_functions();
        solver_testing();
        constants_parsing();
        eqnlib_parsing();
        eqnlib_columns_and_beams();
        numerical_integration();
        symbolic_numerical_integration();
        text_functions();
        auto_simplification();
        rewrite_engine();
        symbolic_operations();
        symbolic_differentiation();
        symbolic_integration();
        tagged_objects();
        catalog_test();
        cycle_test();
        shift_and_rotate();
        flags_functions();
        flags_by_name();
        settings_by_name();
        parsing_commands_by_name();
        plotting();
        plotting_all_functions();
        graphic_commands();
        user_input_commands();
        hms_dms_operations();
        date_operations();
        infinity_and_undefined();
        overflow_and_underflow();
        online_help();
        graphic_stack_rendering();
        insertion_of_variables_constants_and_units();
        constants_menu();
        character_menu();
        statistics();
        probabilities();
        sum_and_product();
        polynomials();
        quotient_and_remainder();
        expression_operations();
        random_number_generation();
        object_structure();
        financial_functions();
        library();
        check_help_examples();
        regression_checks();
        demo_ui();
        demo_math();
        demo_pgm();
    }
    summary();

    RECORDER_TRACE(errors) = tracing;

    if (run_tests)
        exit(failures.size() ? 1 : 0);
}


static double speedup = 1.0;

void tests::demo_setup()
// ----------------------------------------------------------------------------
//   Setup the environment used by demos
// ----------------------------------------------------------------------------
{
    static bool setup = false;
    if (setup)
        return;

    step("Setup")
        .test(CLEAR,
              LSHIFT, RUNSTOP,
              "1 3 START 0 0.5 RANDOM NEXT RGB FOREGROUND 3 DISP "
              "#0 FOREGROUND  ", ENTER, F, ALPHA, D, NOSHIFT, STO);

    setup = true;
    if (cstring env = getenv("DB48X_SPEEDUP"))
        speedup = atof(env);

#define DSU(n)        (uint(speedup * (n)))
#define W1            WAIT(DSU(100))
#define W2            WAIT(DSU(200))
#define W3            WAIT(DSU(300))
#define W4            WAIT(DSU(400))
#define W5            WAIT(DSU(500))
#define WLABEL        WAIT(DSU(750))
#define WSHOW         WAIT(DSU(750))
#define KDELAY(n)     KEY_DELAY(DSU(n))
}


void tests::demo_ui()
// ----------------------------------------------------------------------------
//   Run a 30 second demo of the user interface
// ----------------------------------------------------------------------------
{
    BEGIN(demo_ui);
    demo_setup();

    step("An RPL calculator with infinite stack")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                   An RPL calculator", RSHIFT, BSP,
              "                   with infinite stack", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              1, ENTER, 2, ENTER, 3, ENTER, 4, ENTER,
              5, ENTER, 6, ENTER, 7, ENTER, 8, ENTER, W3,
              KDELAY(75),
              DIV, MUL, SUB, ADD, DIV, MUL, SUB, WSHOW);

    step("Function keys")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                 6 function keys", RSHIFT, BSP,
              "            provide quick access to ", RSHIFT, BSP,
              "               up to 18 functions", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              LSHIFT, RUNSTOP,
              LSHIFT, O,
              F1, F2, F3, F4, F5, F6,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5, RSHIFT, F6,
              ENTER,
              WSHOW);

    step("Hyperlinked help")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                      On-line help", RSHIFT, BSP,
              "                   with hyperlinks", RSHIFT, BSP,
              "           activated with long-press", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              LONGPRESS, K, W5, DOWN, DOWN, DOWN, W5, F1, DOWN, DOWN, DOWN, W5);

    step("Library of equations and constants")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "             Equations and constants", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              LSHIFT, I, F2, F1, F2, MUL, WSHOW,
              LSHIFT, F1, LSHIFT, F2, WSHOW,
              LSHIFT, I, F3, F1, LSHIFT, F1, WSHOW,
              CLEAR,
              ID_EquationsMenu, F2, RSHIFT, F2, RSHIFT, F1, WSHOW,
              LSHIFT, F1, RSHIFT, F1, WSHOW);

    step("Graphing and plotting")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "            Graphing and plotting", RSHIFT, BSP,
              "                   with patterns", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(20),
              ID_ModesMenu, F2,
              KDELAY(0),
              F, 3, MUL, J,  3, ID_multiply, ALPHA, X, DOWN,
              NOSHIFT, ADD, 4, ENTER,
              F,
              K, "4.47", ID_multiply, ALPHA, X, NOSHIFT, DOWN, MUL,
              J, ALPHA, X, NOSHIFT, DOWN, MUL,
              L, "2.13", ID_multiply, ALPHA, X, ENTER,
              WSHOW,
              KDELAY(0),
              RSHIFT, O, LSHIFT, RUNSTOP,
              "3 LINEWIDTH "
              "0.9 0 0 RGB FOREGROUND ", NOSHIFT, F1,
              " 0 0 0.8 RGB FOREGROUND ", NOSHIFT, F2,
              " 0 0 0 RGB FOREGROUND ", ENTER,
              RUNSTOP, WSHOW, ENTER);

    step("Quick conversion")
        .test(CLEAR, RSHIFT, ENTER,
              "      Quick conversion (cycle) key", ENTER, "D", ENTER,
              WLABEL, ENTER,
              KDELAY(0), "2.335", ENTER,
              KDELAY(75), O, O, O,
              ID_ComplexMenu, 1, F1, 1, ENTER,
              KDELAY(125), O, O, WSHOW);

    step("Tool key")
        .test(CLEAR, EXIT, RSHIFT, ENTER,
              "      Tool key selects best menu", ENTER, "D", ENTER,
              WLABEL, ENTER,
              123, ENTER,
              123, ID_ComplexMenu, F1, 456, ENTER,
              RSHIFT, ENTER, "ABCD", ENTER,
              KDELAY(25), A, W5,
              BSP, A, W5,
              BSP, A, W5,
              BSP, A, W5);

    step("End of UI demo")
        .test(CLEAR, "#0 Foreground", ENTER);
}


void tests::demo_math()
// ----------------------------------------------------------------------------
//   Run a 30 second demo of the math capabilities
// ----------------------------------------------------------------------------
{
    BEGIN(demo_math);
    demo_setup();

    step("Integers, decimals and fractions")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "      Integer, decimal and fractions", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(50),
              2, ENTER, 3, DIV, 4, ENTER, 7, DIV, ADD,
              "2.", ENTER, 3, DIV, "4.", ENTER, 7, DIV, ADD, W2,
              LSHIFT, DOT, WSHOW, ENTER);
    step("Arbitrary precision")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                Arbitrary precision", RSHIFT, BSP,
              "       integer and decimal numbers", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              NOSHIFT, F, 80, ID_ProbabilitiesMenu, F3, W2, ENTER, RUNSTOP,
              LSHIFT, DOT, WSHOW, ENTER,
              ID_ModesMenu, F2,
              LSHIFT, O, 420, F5, 420, F6,
              1, LSHIFT, L, 4, MUL,
              LSHIFT, DOT, WSHOW, ENTER,
              KDELAY(0), 12, F5, 24, F6);
    step("Complex numbers")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                Complex numbers", RSHIFT, BSP,
              "             Polar and rectangular", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              ID_ModesMenu, F1,
              ID_ComplexMenu,
              2, F1, 3, ENTER, 4, F1, 5, W2, ADD,
              W2,
              2, F2, 30, ENTER, 3, F2, 40, MUL,
              WSHOW);
    step("Vectors and matrices")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                Vectors and matrix", RSHIFT, BSP,
              "             arithmetic and operations", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(5),
              LSHIFT, KEY9, "1 2 3", ENTER, W2,
              LSHIFT, KEY9,
              LSHIFT, KEY9, "1 2 3", NOSHIFT, DOWN,
              LSHIFT, KEY9, "4 5 6", NOSHIFT, DOWN,
              LSHIFT, KEY9, "7 8 9", NOSHIFT, ENTER, W2,
              KDELAY(25),
              B, W2, ENTER,
              RSHIFT, KEY9, LSHIFT, F1, W2,
              KDELAY(0),
              LSHIFT, M,
              LSHIFT, KEY9,
              LSHIFT, KEY9, "0 0 0", NOSHIFT, DOWN,
              LSHIFT, KEY9, "0 0 0", NOSHIFT, DOWN,
              LSHIFT, KEY9, "0 0 10", NOSHIFT,
              KDELAY(25), ENTER, ADD,
              B, WSHOW);
    step("Symbolic arithmetic")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                 Symbolic arithmetic", RSHIFT, BSP,
              "                    and expressions", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              "x", ENTER, 2, MUL, 3, ENTER, "y", ENTER, D, SUB,
              C, B, 1, SUB, ENTER,
              J, K, L, B, E, C,
              WSHOW);
    step("Based numbers")
        .test(CLEAR,
              RSHIFT, ENTER,
              "                 Based numbers", RSHIFT, BSP,
              "        in any base between 2 and 36", RSHIFT, BSP,
              "                 with any word size", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(15),
              ID_BasesMenu,
              F1, KEY1, KEY2, KEY3, A, B, C, ENTER,
              KDELAY(25),
              F1, C, D, E, ADD,
              W5,
              KEY2, F1, KEY1, KEY0, KEY0, KEY1, ENTER, W2,
              LSHIFT, F2, W1, LSHIFT, F3, W1, LSHIFT, F4, W1,
              3, LSHIFT, F1, WSHOW, LSHIFT, F5);
    step("DMS and HMS operations")
        .test(CLEAR,
              RSHIFT, ENTER,
              "        Degrees, minutes and seconds", RSHIFT, BSP,
              "        Hours, minutes and seconds,", RSHIFT, BSP,
              "           Dates and time operations", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              NOSHIFT, KEY1, DOT, KEY2, KEY3, DOT, KEY3, KEY6, ENTER, W1,
              NOSHIFT, KEY2, DOT, KEY4, KEY1, DOT, KEY5, KEY1, W1,
              ADD, W1,
              RSHIFT, KEY6, LSHIFT, F3, W1,
              RSHIFT, F4, "19681205", NOSHIFT, F1, SUB, WSHOW);

    step("End of math demo")
        .test(CLEAR, "#0 Foreground", ENTER);
}


void tests::demo_pgm()
// ----------------------------------------------------------------------------
//   Run a 30 second demo of the programming capabilities
// ----------------------------------------------------------------------------
{
    BEGIN(demo_pgm);
    demo_setup();

    step("Engineering units")
        .test(CLEAR,
              RSHIFT, ENTER,
              "                 Engineering units", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              KDELAY(25),
              LSHIFT, KEY5, F3, "3500.25", F2, LSHIFT, F1,
              LSHIFT, KEY5, F4,
              1000, F2, LSHIFT, F1, WSHOW, DIV, WSHOW,
              "1_EUR/km", RSHIFT, KEY5, F1, WSHOW);

    step("RPL programming")
        .test(CLEAR, EXIT,
              RSHIFT, ENTER,
              "                 RPL programming", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              LSHIFT, RUNSTOP,
              KEY2, MUL, KEY1, ADD, ENTER,
              F, "MyFn", NOSHIFT, G,
              H, 1, F1, W3, F1, W3, F1, W3);

    step("Program editing")
        .test(RSHIFT, ENTER,
              "                 Advanced editor", RSHIFT, BSP,
              "        with cut, copy, paste, search...", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              LSHIFT, F1, DOWN, DOWN, DOWN,
              RSHIFT, DOWN, F1, DOWN, DOWN, DOWN, DOWN, WSHOW,
              F5, F6, F6, F6,
              F4, NOSHIFT, KEY2, F4, W3, F4, W3,  F4, W3,
              ENTER, ENTER, W3,
              H, RSHIFT, F1, 24, F1, W3, F1, W3);

    step("Command-line history")
        .test(RSHIFT, ENTER,
              "             Command-line history", RSHIFT, BSP,
              "        Recalls last eight commands", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(25),
              RSHIFT, UP, WSHOW,
              F2, WSHOW,
              F2, WSHOW,
              F2, WSHOW,
              F2, WSHOW);

    step("Loops and conditions")
        .test(RSHIFT, ENTER,
              "                 Loops and conditions", ENTER, "D", ENTER,
              WLABEL, ENTER, KDELAY(10),
              LSHIFT, RUNSTOP,
              LSHIFT, F,
              "1 1000 ", F3, "i ", W3,
              "i ", NOSHIFT, J, ID_sq,
              " i ", NOSHIFT, K, ID_sq,
              " i 0.321", ID_multiply, K, ID_sq,
              RSHIFT, DOT, RSHIFT, F1, F6, F2, 3, F6, RSHIFT, F2,
              RSHIFT, RUNSTOP, DOWN,
              "i", NOSHIFT, J, 8, ID_multiply, ADD,
              "i 3.214", ID_multiply, NOSHIFT, K, 4, ID_multiply, ADD,
              RSHIFT, RUNSTOP, DOWN,
              "i 5.234", ID_multiply, NOSHIFT, J, 4, ID_multiply, ADD,
              "i 8.214", ID_multiply, NOSHIFT, K, 2, ID_multiply, ADD,
              RSHIFT, DOT, F1, ENTER, WSHOW,
              LENGTHY(2000), RUNSTOP, WSHOW, ENTER);

    step("End of programming demo")
        .test(CLEAR, "#0 Foreground", ENTER);
}


void tests::reset_settings()
// ----------------------------------------------------------------------------
//   Use settings that make the results predictable on screen
// ----------------------------------------------------------------------------
{
    // Reset to default test settings
    BEGIN(defaults);
    Settings = settings();

    // Check that we have actually reset the settings
    step("Select Modes menu")
        .test(ID_ModesMenu).noerror();
    step("Checking output modes")
        .test(ID_Modes, ENTER)
        .want("« ModesMenu »");

    // Check that we can change a setting
    step("Selecting FIX 3")
        .test(CLEAR, SHIFT, O, 3, F2, "1.23456", ENTER)
        .expect("1.235");
    step("Checking Modes for FIX")
        .test("Modes", ENTER)
        .want("« 3 FixedDisplay 3 DisplayDigits DisplayModesMenu »");
    step("Reseting with command")
        .test("ResetModes", ENTER)
        .noerror()
        .test("Modes", ENTER)
        .want("« DisplayModesMenu »");

    // Disable debugging on error, since we generate many errors intentionally
    step("Disable DebugOnError")
        .test(CLEAR, "KillOnError", ENTER).noerror();
}


void tests::shift_logic()
// ----------------------------------------------------------------------------
//   Test all keys and check we have the correct output
// ----------------------------------------------------------------------------
{
    BEGIN(shifts);

    step("Shift state must be cleared at start")
        .shift(false)
        .xshift(false)
        .alpha(false)
        .lower(false);

    step("Shift basic cycle")
        .test(SHIFT)
        .shift(true)
        .xshift(false)
        .alpha(false)
        .lower(false);
    step("Shift-Shift is Right Shift")
        .test(SHIFT)
        .shift(false)
        .xshift(true)
        .alpha(false)
        .lower(false);
    step("Third shift clears all shifts")
        .test(SHIFT)
        .shift(false)
        .xshift(false)
        .alpha(false)
        .lower(false);

    step("Shift second cycle")
        .test(SHIFT)
        .shift(true)
        .xshift(false)
        .alpha(false)
        .lower(false);
    step("Shift second cycle: Shift-Shift is Right Shift")
        .test(SHIFT)
        .shift(false)
        .xshift(true)
        .alpha(false)
        .lower(false);
    step("Shift second cycle: Third shift clears all shifts")
        .test(SHIFT)
        .shift(false)
        .xshift(false)
        .alpha(false)
        .lower(false);

    step("Long-press shift is Alpha")
        .test(SHIFT, false)
        .wait(600)
        .test(RELEASE)
        .shift(false)
        .xshift(false)
        .alpha(true);
    step("Long-press shift clears Alpha")
        .test(SHIFT, false)
        .wait(600)
        .test(RELEASE)
        .shift(false)
        .xshift(false)
        .alpha(false);

    step("Typing alpha")
        .test(LONGPRESS, SHIFT, A)
        .shift(false)
        .alpha(true)
        .lower(false)
        .editor("A");
    step("Selecting lowercase with Shift-ENTER")
        .test(SHIFT, ENTER)
        .alpha(true)
        .lower(true);
}


void tests::keyboard_entry()
// ----------------------------------------------------------------------------
//   Test all keys and check we have the correct output
// ----------------------------------------------------------------------------
{
    BEGIN(keyboard);

    step("Uppercase entry");
    cstring entry = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    test(CLEAR, entry).editor(entry);

    step("Lowercase entry");
    cstring lowercase = "abcdefghijklmnopqrstuvwxyz0123456789";
    test(CLEAR, lowercase).editor(lowercase);

    step("Special characters");
    cstring special = "X+-*/!? #_";
    test(CLEAR, special).editor(special);

    step("Separators");
    cstring seps = "\"Hello [A] (B) {C} 'Test' D\"";
    test(CLEAR, seps).editor(seps);

    step("Separators with auto-spacing");
    cstring seps2     = "{}()[]";
    cstring seps2auto = "{} () []";
    test(CLEAR, seps2).editor(seps2auto);

    step("Key repeat");
    test(CLEAR, LONGPRESS, SHIFT, LONGPRESS, A)
        .wait(1000)
        .test(RELEASE)
        .check(ui.cursor > 4);

    step("Space key during data entry inserts space")
        .test(CLEAR, SHIFT, RUNSTOP,
              KEY7, SPACE, ALPHA, A, SPACE, B,
              NOSHIFT, ADD, ADD)
        .editor("«7 A B + + »");
    step("Space key in immediate mode evaluates")
        .test(ENTER).want("« 7 A B + + »")
        .test(SPACE).expect("'7+(A+B)'");
    step("F key inserts equation")
        .test(CLEAR, F).editor("''")
        .test(KEY1).editor("'1'");
    step("Space key in expresion inserts = sign")
        .test(SPACE).editor("'1='")
        .test(KEY2).editor("'1=2'")
        .test(ADD).editor("'1=2+'")
        .test(KEY3).editor("'1=2+3'");
    step("F key in equation inserts parentheses")
        .test(MUL).editor("'1=2+3·'")
        .test(F).editor("'1=2+3·()'");
    step("Automatic insertion of parentheses after functions")
        .test(ID_exp).editor("'1=2+3·(exp())'")
        .test(KEY0).editor("'1=2+3·(exp(0))'");
    step("Space key in parentheses insert semi-colon")
        .test(SPACE).editor("'1=2+3·(exp(0;))'")
        .test(KEY7).editor("'1=2+3·(exp(0;7))'");

    step("STO key while entering equation (bug #390)")
        .test(CLEAR, EXIT, KEY1, KEY2, F,
              ALPHA, A, B, C, ID_Sto).noerror()
        .test(F, ALPHA, A, B, C, ENTER, SPACE).expect("12")
        .test("'ABC'", ENTER, ID_MemoryMenu, ID_Purge).noerror();

    step("Inserting a colon in text editor inserts tag delimiters")
        .test(CLEAR, ALPHA, KEY0).editor("::");
    step("Inserting a colon in text inserts a single colon")
        .test(CLEAR, RSHIFT, ENTER, KEY0).editor("\":\"");
}


void tests::data_types()
// ----------------------------------------------------------------------------
//   Check the basic data types
// ----------------------------------------------------------------------------
{
    BEGIN(types);

    step("Positive integer");
    test(CLEAR, "1", ENTER).type(ID_integer).expect("1");
    step("Negative integer");
    test(CLEAR, "1", CHS, ENTER).type(ID_neg_integer).expect("-1");

#if CONFIG_FIXED_BASED_OBJECTS
    step("Binary based integer");
    test(CLEAR, "#10010101b", ENTER)
        .type(ID_bin_integer)
        .expect("#1001 0101₂");
    test(CLEAR, "#101b", ENTER).type(ID_bin_integer).expect("#101₂");

    step("Decimal based integer");
    test(CLEAR, "#12345d", ENTER)
        .type(ID_dec_integer)
        .expect("#1 2345₁₀");
    test(CLEAR, "#123d", ENTER).type(ID_dec_integer).expect("#123₁₀");

    step("Octal based integer");
    test(CLEAR, "#12345o", ENTER)
        .type(ID_oct_integer)
        .expect("#1 2345₈");
    test(CLEAR, "#123o", ENTER).type(ID_oct_integer).expect("#123₈");

    step("Hexadecimal based integer");
    test(CLEAR, "#1234ABCDh", ENTER)
        .type(ID_hex_integer)
        .type(ID_hex_integer)
        .expect("#1234 ABCD₁₆");
    test(CLEAR, "#DEADBEEFh", ENTER)
        .type(ID_hex_integer)
        .expect("#DEAD BEEF₁₆");
#endif // CONFIG_FIXED_BASED_OBJECTS

    step("Arbitrary base input");
    test(CLEAR, "8#777", ENTER)
#if CONFIG_FIXED_BASED_OBJECTS
        .type(ID_oct_integer)
        .expect("#777₈")
#else // !CONFIG_FIXED_BASED_OBJECTS
        .type(ID_based_integer)
        .expect("#1FF₁₆")
#endif // CONFIG_FIXED_BASED_OBJECTS
        .noerror();
    test(CLEAR, "2#10000#ABCDE", ENTER)
#if CONFIG_FIXED_BASED_OBJECTS
        .type(ID_hex_integer)
#else // !CONFIG_FIXED_BASED_OBJECTS
        .type(ID_based_integer)
#endif // CONFIG_FIXED_BASED_OBJECTS
        .expect("#A BCDE₁₆");

    step("Do not parse #7D as a decimal (#371)")
        .test(CLEAR, "#7D", ENTER).expect("#7D₁₆");

    step("Decimal value")
        .test(CLEAR, "123.456", ENTER)
        .type(ID_decimal).expect("123.456");
    step("Decimal with trailing dot")
        .test(CLEAR, "123.", ENTER).type(ID_decimal).expect("123.");
    step("Negative decimal with leading zero")
        .test(CLEAR, "0.123", ENTER).type(ID_decimal).expect("0.123");
    step("Decimal with leading dot")
        .test(CLEAR, ".123", ENTER).type(ID_decimal).expect("0.123");
    step("Negative decimal")
        .test(CLEAR, "-0.123", ENTER)
        .type(ID_neg_decimal).expect("-0.123");
    step("Negative decimal with leading dot")
        .test(CLEAR, "-.123", ENTER)
        .type(ID_neg_decimal).expect("-0.123");
    step("Decimal with exponent")
        .test(CLEAR, "123E1", ENTER).type(ID_decimal).expect("1 230.");
    step("Decimal with negative exponent")
        .test(CLEAR, "123E-1", ENTER).type(ID_decimal).expect("12.3");
    step("Decimal with exponent")
        .test(CLEAR, "12.3E1", ENTER).type(ID_decimal).expect("123.");
    step("Decimal with negative exponent")
        .test(CLEAR, "12.3E-1", ENTER).type(ID_decimal).expect("1.23");

    step("Comma as decimal dot is accepted by default")
        .test(CLEAR, "0,123").editor("0,123").test(ENTER)
        .type(ID_decimal).expect("0.123");
    step("Selecting comma as decimal dot")
        .test(CLEAR, LSHIFT, O, LSHIFT, F6, F6);
    step("Comma as decimal dot is accepted after changing flag")
        .test(CLEAR, "0,123", ENTER).type(ID_decimal).expect("0,123");
    step("Dot as decimal dot is accepted after selecting comma separator")
        .test(CLEAR, "0,123", ENTER).type(ID_decimal).expect("0,123");
    step("Restoring dot as decimal separator")
        .test(F5, ENTER, BSP).type(ID_decimal).expect("0.123");
    step("Do not generate extra object when wrong decimal is used")
        .test(CLEAR, "{ 0,123 4.567 }", ENTER).expect("{ 0.123 4.567 }");
    step("Do not generate extra object when wrong decimal is used")
        .test(CLEAR, F6, "{ 0,123 4.567 }", ENTER).expect("{ 0,123 4,567 }")
        .test(F5).expect("{ 0.123 4.567 }");

    step("Symbols");
    cstring symbol = "ABC123Z";
    test(CLEAR, symbol, ENTER).type(ID_expression).expect("'ABC123Z'");

    step("Text");
    cstring string = "\"Hello World\"";
    test(CLEAR, string, ENTER).type(ID_text).expect(string);

    step("Text containing quotes")
        .test(CLEAR, RSHIFT, ENTER,
              SHIFT, SHIFT, ENTER, DOWN,
              ALPHA, H, LOWERCASE, E, L, L, O,
              SHIFT, SHIFT, ENTER, DOWN, ENTER)
        .type(ID_text).expect("\"\"\"Hello\"\"\"")
        .test("1 DISP", ENTER).image("quoted-text", 25500);

    step("List");
    cstring list = "{ A 1 3 }";
    test(CLEAR, list, ENTER).type(ID_list).expect(list);

    step("Program");
    cstring prgm = "« 1 + sin »";
    test(CLEAR, SHIFT, RUNSTOP, 1, ADD, "sin", ENTER)
        .type(ID_program)
        .want(prgm);

    step("Equation");
    cstring eqn = "'X+1'";
    test(CLEAR, XEQ, "X", ENTER, KEY1, ADD)
        .type(ID_expression)
        .expect(eqn);
    cstring eqn2 = "'sin(X+1)'";
    test(SIN)
        .type(ID_expression)
        .expect(eqn2);
    test(DOWN)
        .editor(eqn2);
    test(ENTER, 1, ADD).
        type(ID_expression).expect("'sin(X+1)+1'");

    step("Equation parsing and simplification");
    test(CLEAR, "'(((A))+(B))-(C+D)'", ENTER)
        .type(ID_expression)
        .expect("'A+B-(C+D)'");
    step("Equation fancy rendering");
    test(CLEAR, XEQ, "X", ENTER, INV,
         XEQ, "Y", ENTER, SHIFT, SQRT, XEQ, "Z", ENTER,
         "CUBED", ENTER, ADD, ADD)
        .type(ID_expression)
        .expect("'X⁻¹+(Y²+Z³)'");
    step("Equation fancy parsing from editor");
    test(DOWN, SPACE, SPACE, SPACE,
         RSHIFT, DOWN, SHIFT, F3, " 1 +", ENTER)
        .type(ID_expression).expect("'X⁻¹+(Y²+Z³)+1'");
    step("Parsing text in an algebraic expression")
        .test(CLEAR, "'SIZE(\"Hello\")'", ENTER)
        .expect("'Size \"Hello\"'")
        .test(ID_Run)
        .expect("5");
    step("Parsing arrays in an algebraic expression")
        .test(CLEAR, "'SIZE([1;2;3;4])'", ENTER)
        .expect("'Size [1;2;3;4]'")
        .test(ID_Run)
        .expect("{ 4 }");

    step("Fractions");
    test(CLEAR, "1/3", ENTER).type(ID_fraction).expect("¹/₃");
    test(CLEAR, "-80/60", ENTER).type(ID_neg_fraction).expect("-1 ¹/₃");
    test(CLEAR, "20/60", ENTER).type(ID_fraction).expect("¹/₃");

    step("Large integers");
    cstring b = "123456789012345678901234567890123456789012345678901234567890";
    cstring mb =
        "-123 456 789 012 345 678 901 234 567 890"
        " 123 456 789 012 345 678 901 234 567 890";
    test(CLEAR, b, ENTER).type(ID_bignum).expect(mb+1);
    test(DOWN, CHS, ENTER).type(ID_neg_bignum).expect(mb);
    test(CHS).type(ID_bignum).expect(mb + 1);
    test(DOWN, CHS, ENTER).type(ID_neg_bignum).expect(mb);

    step("Large fractions");
    cstring bf =
        "123456789012345678901234567890123456789012345678901234567890/"
        "123456789012345678901234567890123456789012345678901234567891";
    cstring mbf =
        "-¹²³ ⁴⁵⁶ ⁷⁸⁹ ⁰¹² ³⁴⁵ ⁶⁷⁸ ⁹⁰¹ ²³⁴ ⁵⁶⁷ ⁸⁹⁰ ¹²³ ⁴⁵⁶ ⁷⁸⁹ ⁰¹² ³⁴⁵ "
        "⁶⁷⁸ ⁹⁰¹ ²³⁴ ⁵⁶⁷ ⁸⁹⁰/"
        "₁₂₃ ₄₅₆ ₇₈₉ ₀₁₂ ₃₄₅ ₆₇₈ ₉₀₁ ₂₃₄ ₅₆₇ ₈₉₀ ₁₂₃ ₄₅₆ ₇₈₉ ₀₁₂ ₃₄₅ "
        "₆₇₈ ₉₀₁ ₂₃₄ ₅₆₇ ₈₉₁";
    test(CLEAR, bf, ENTER).type(ID_big_fraction).expect(mbf+1);
    test(DOWN, CHS, ENTER).type(ID_neg_big_fraction).expect(mbf);
    test(CHS).type(ID_big_fraction).expect(mbf+1);
    test(CHS).type(ID_neg_big_fraction).expect(mbf);
    test(DOWN, CHS, ENTER).type(ID_big_fraction).expect(mbf+1);

    step("Directory from command line")
        .test(CLEAR, "DIR { A 2 B 3 }", ENTER)
        .want("Directory { A 2 B 3 }");
    step("Empty directory on command line")
        .test(CLEAR, "DIR A 2 B 3", ENTER)
        .got("3", "'B'", "2", "'A'", "Directory {}");

    step("Graphic objects")
        .test(CLEAR,
              "GROB 9 15 "
              "E300140015001C001400E3008000C110AA00940090004100220014102800",
              ENTER)
        .type(ID_grob);

    clear();

    step ("Bytes command");
    test(CLEAR, "12", ENTER, "bytes", ENTER)
        .expect("2")
        .test(BSP)
        .match("#C..₁₆");
    test(CLEAR, "129", ENTER, "bytes", ENTER)
        .expect("3")
        .test(BSP)
        .match("#1 81..₁₆");

    step("Type command (direct mode)");
    test(CLEAR, "DetailedTypes", ENTER).noerror();
    test(CLEAR, "12 type", ENTER)
        .type(ID_neg_integer)
        .expect(~int(ID_integer));
    test(CLEAR, "'ABC*3' type", ENTER)
        .type(ID_neg_integer)
        .expect(~int(ID_expression));

    step("Type command (compatible mode)");
    test(CLEAR, "CompatibleTypes", ENTER).noerror();
    test(CLEAR, "12 type", ENTER)
        .type(ID_integer)
        .expect(28);
    test(CLEAR, "'ABC*3' type", ENTER)
        .type(ID_integer)
        .expect(9);

    step("TypeName command");
    test(CLEAR, "12 typename", ENTER)
        .type(ID_text)
        .expect("\"integer\"");
    test(CLEAR, "'ABC*3' typename", ENTER)
        .type(ID_text)
        .expect("\"expression\"");
}


void tests::editor_operations()
// ----------------------------------------------------------------------------
//   Check text editor operations
// ----------------------------------------------------------------------------
{
    BEGIN(editor);

    step("Edit an object")
        .test(CLEAR, "12", ENTER).expect("12")
        .test(DOWN).editor("12");
    step("Inserting text")
        .test("A").editor("A12");
    step("Moving cursor right")
        .test(DOWN, DOWN, "B").editor("A12B");
    step("Moving cursor left")
        .test(UP, UP, "C").editor("A1C2B");
    step("Entering command line")
        .test(ENTER).expect("'A1C2B'");
    step("Entering another entry")
        .test("1 2 3 4", ENTER).expect("4");
    step("Editor history")
        .test(RSHIFT, UP)
        .editor("1 2 3 4")
        .test(RSHIFT, UP)
        .editor("A1C2B");
    step("Editor menu")
        .test(RSHIFT, DOWN);
    step("Selection")
        .test(F1, DOWN, DOWN).editor("A1C2B");
    step("Cut")
        .test(F5).editor("C2B");
    step("Paste")
        .test(F6).editor("A1C2B")
        .test(DOWN, F6).editor("A1CA12B");
    step("Select backwards")
        .test(F1).editor("A1CA12B");
    step("Move cursor word left")
        .test(F2, "X").editor("XA1CA12B");
    step("Move cursor word right")
        .test(F3, "Y").editor("XA1CA12BY");
    step("Swap cursor and selection")
        .test(SHIFT, F1, RUNSTOP, "M").editor("XA1CA1 M2BY");
    step("Copy")
        .test(SHIFT, F5, F2, F6).editor("XA1CA12BY M2BY");
    step("Select to clear selection")
        .test(F1);
    step("Search")
        .test(F4, A, ENTER, N).editor("XAN1CA12BY M2BY");
    step("Search again")
        .test(F1, F4, B, Y, F4, ENTER, SHIFT, F1, Q).editor("XAN1CA12BY M2QBY");
    step("Replace")
        .test(SHIFT, F5, F1, F4, A, SHIFT, F4).editor("XBYN1CA12BY M2QBY");
    step("Second replace")
        .test(SHIFT, F4).editor("XBYN1CBY12BY M2QBY");
    step("Third replace")
        .test(SHIFT, F4).editor("XBYN1CBY12BY M2QBY");
    step("End of search, same editor")
        .test(ENTER).editor("XBYN1CBY12BY M2QBY");
    step("End of editing, empty editor")
        .test(ENTER).editor("");
    step("History")
        .test(RSHIFT, UP).editor("XBYN1CBY12BY M2QBY");
    step("History level 2")
        .test(RSHIFT, UP).editor("1 2 3 4");
    step("Exiting old history")
        .test(EXIT).editor("");
    step("Check 8-level history")
        .test("A", ENTER, "B", ENTER, "C", ENTER, "D", ENTER,
              "E", ENTER, "F", ENTER, "G", ENTER, "H", ENTER,
              RSHIFT, UP).editor("H")
        .test(RSHIFT, UP).editor("G")
        .test(RSHIFT, UP).editor("F")
        .test(RSHIFT, UP).editor("E")
        .test(RSHIFT, UP).editor("D")
        .test(RSHIFT, UP).editor("C")
        .test(RSHIFT, UP).editor("B")
        .test(RSHIFT, UP).editor("A")
        .test(RSHIFT, UP).editor("H");
    step("EXIT key still saves editor contents")
        .test(CLEAR, "ABCD").editor("ABCD")
        .test(EXIT).editor("").noerror()
        .test(RSHIFT, UP).editor("ABCD");
    step("End of editor")
        .test(CLEAR);

    step("Entering n-ary expressions")
        .test(CLEAR, "'Σ(i;1;10;i^3)'", ENTER).expect("'Σ(i;1;10;i↑3)'")
        .test(CLEAR, "'sum(i;1;10;i^3)'", ENTER).expect("'Σ(i;1;10;i↑3)'")
        .test(CLEAR, "'∏(j;a;b;2^j)'", ENTER).expect("'∏(j;a;b;2↑j)'")
        .test(CLEAR, "'product(j;a;b;2^j)'", ENTER).expect("'∏(j;a;b;2↑j)'")
        .test(CLEAR, "'xroot(x+1;5)'", ENTER).expect("'xroot(x+1;5)'");

    step("Order of xroot arguments")
        .test(CLEAR, "A B", ID_xroot)
        .expect("'xroot(B;A)'").image_noheader("xroot-order")
        .test(DOWN).editor("'xroot(B;A)'")
        .test(ENTER).image_noheader("xroot-order");

    step("Position of negation parsing xroot")
        .test(CLEAR, "'XROOT(A;-B)'", ENTER, EXIT)
        .expect("'xroot(A;-B)'").image_noheader("xroot-negation")
        .test(DOWN).editor("'xroot(A;-B)'")
        .test(ENTER, EXIT).image_noheader("xroot-negation");

    step("Position of negation parsing summand")
        .test(CLEAR, "'Σ(X;1;10;-X)'", ENTER)
        .expect("'Σ(X;1;10;-X)'")
        .test(EXIT).image_noheader("sum-negation")
        .test(DOWN).editor("'Σ(X;1;10;-X)'")
        .test(ENTER, EXIT).image_noheader("sum-negation");

    step("Position of negation parsing sum end")
        .test(CLEAR, "'Σ(X;1;-10;X)'", ENTER)
        .expect("'Σ(X;1;-10;X)'")
        .test(EXIT).image_noheader("sum-negation2")
        .test(DOWN).editor("'Σ(X;1;-10;X)'")
        .test(ENTER, EXIT).image_noheader("sum-negation2");

    step("Position of negation parsing sum start")
        .test(CLEAR, "'Σ(X;-1;10;X)'", ENTER)
        .expect("'Σ(X;-1;10;X)'")
        .test(EXIT).image_noheader("sum-negation3")
        .test(DOWN).editor("'Σ(X;-1;10;X)'")
        .test(ENTER, EXIT).image_noheader("sum-negation3");

    step("Variable in sum must be a variable")
        .test(CLEAR, "'Σ(-X;1;10;X)'", ENTER)
        .error("Expected variable name");

    step("Variable in product must be a variable")
        .test(CLEAR, "'∏(-X;1;10;X)'", ENTER)
        .error("Expected variable name");

    step("Error parsing n-ary expressions")
        .test(CLEAR, "'Σ(i;1)'", ENTER).error("Unterminated")
        .test(CLEAR, "'sum(i;1;10;i^3;42)'", ENTER).error("Unterminated")
        .test(CLEAR, "'xroot(x+1*5)'", ENTER).error("Unterminated")
        .test(CLEAR, "'xroot()'", ENTER).error("Unterminated")
        .test(CLEAR, "'xroot 42'", ENTER).error("Syntax error");

    step("User-defined function call")
        .test(CLEAR, "'F(1;2+3;4^5;G(x;y;z))'", ENTER)
        .expect("'F(1;2+3;4↑5;G(x;y;z))'");
    step("Evaluating user-defined function call")
        .test(RUNSTOP).expect("'F'")
        .test("DEPTH TOLIST", ENTER)
        .expect("{ 1 5 1 024 'x' 'y' 'z' 'G' 'F' }");
    step("Library function call")
        .test(CLEAR, "'ⓁSiDensity(273_K)'", ENTER)
        .expect("'SiDensity(273 K)'")
        .test(DOWN).editor("'ⓁSiDensity(273_K)'")
        .test(ENTER)
        .expect("'SiDensity(273 K)'")
        .test(RUNSTOP)
        .expect("799 498 575.637 (cm↑3)⁻¹");

    step("Implicit multiplication")
        .test(CLEAR, "'2X'", ENTER).expect("'2·X'");

    step("Graphical rendering of integrals - Simple expression")
        .test(CLEAR, "'integrate(A;B;sin(X);X)'", ENTER, EXIT)
        .image_noheader("integral");
    step("Graphical rendering of integrals - Additive expression")
        .test(CLEAR, "'integrate(A;B;1+sin(X);X)'", ENTER, EXIT)
        .image_noheader("integral-add");
    step("Graphical rendering of integrals - Divide expression")
        .test(CLEAR, "'integrate(A;B;1/sin(X);X)'", ENTER, EXIT)
        .image_noheader("integral-div");

    step("Enter X mod Y and checking it can be edited")
        .test(CLEAR, NOSHIFT, F, "X", ID_RealMenu, ID_mod, "Y")
        .editor("'X mod Y'")
        .test(ENTER)
        .expect("'X mod Y'")
        .test(DOWN)
        .editor("'X mod Y'")
        .test(ENTER);
    step("Enter X and Y and checking it can be edited")
        .test(CLEAR, NOSHIFT, F, "x", ID_BasesMenu, F2, "y")
        .editor("'x and y'")
        .test(ENTER)
        .expect("'x and y'")
        .test(DOWN)
        .editor("'x and y'")
        .test(ENTER);

    step("Insert if-then from menu")
        .test(CLEAR, LSHIFT, KEY3, LSHIFT, F2, LSHIFT, F1)
        .editor("if  then  end ");
    step("Insert if-then-else from menu")
        .test(CLEAR, LSHIFT, KEY3, LSHIFT, F2, LSHIFT, F2)
        .editor("if  then  else  end ");
    step("Insert iferr-then from menu")
        .test(CLEAR, LSHIFT, KEY3, LSHIFT, F2, LSHIFT, F3)
        .editor("iferr  then  end ");
    step("Insert iferr-then-else from menu")
        .test(CLEAR, LSHIFT, KEY3, LSHIFT, F2, LSHIFT, F4)
        .editor("iferr  then  else  end ");

    step("Check numbering separators after - (bug #1032)")
        .test(CLEAR, F, KEY1, KEY0, KEY0, KEY0, SUB, KEY1)
        .editor("'1 000-1'");
    step("Check numbering separators after - with exponent")
        .test(CLEAR, F, KEY1, KEY0, KEY0, KEY0, O, N, KEY1)
        .editor("'1 000⁳-1'");

    step("Editing unit in program (bug #1192)")
        .test(CLEAR, LSHIFT, RUNSTOP, "25.4", ENTER)
        .want("« 25.4 »")
        .test(DOWN, DOWN, "123", LSHIFT, KEY5, F4, F2, ENTER)
        .want("« 123 yd 25.4 »")
        .test(DOWN, DOWN, DOWN, DOWN, DOWN, F3, ENTER)
        .error("Syntax error")
        .test(DOWN, DOWN, RUNSTOP, KEY3, ENTER)
        .want("« 123 ft 3 yd 25.4 »");

    step("Using regular unit cycle for meters")
        .test(CLEAR, "1_m", NOSHIFT).editor("1_m")
        .test(EEX).editor("1_km")
        .test(EEX).editor("1_cm")
        .test(EEX).editor("1_mm")
        .test(EEX).editor("1_μm")
        .test(EEX).editor("1_Mm")
        .test(EEX).editor("1_Gm")
        .test(EEX).editor("1_Tm")
        .test(EEX).editor("1_pm")
        .test(EEX).editor("1_nm")
        .test(EEX).editor("1_m")
        .test(EEX).editor("1_km")
        .test(EEX).editor("1_cm")
        .test(EEX).editor("1_mm")
        .test(EEX).editor("1_μm")
        .test(EEX).editor("1_Mm")
        .test(EEX).editor("1_Gm")
        .test(EEX).editor("1_Tm")
        .test(EEX).editor("1_pm")
        .test(EEX).editor("1_nm");
    step("Setting units SI cycle that works well for Farad")
        .test(CLEAR, "\"pμn\" ", ID_UnitsSIPrefixCycle).noerror()
        .test(CLEAR, "1_F", NOSHIFT).editor("1_F")
        .test(EEX).editor("1_pF")
        .test(EEX).editor("1_μF")
        .test(EEX).editor("1_nF")
        .test(EEX).editor("1_F");
    step("Setting units SI cycle that works well for bytes")
        .test(CLEAR, "\"KMGTPE\" ", ID_UnitsSIPrefixCycle).noerror()
        .test(CLEAR, "1_B", NOSHIFT).editor("1_B")
        .test(EEX).editor("1_KB")
        .test(EEX).editor("1_MB")
        .test(EEX).editor("1_GB")
        .test(EEX).editor("1_TB")
        .test(EEX).editor("1_PB")
        .test(EEX).editor("1_EB")
        .test(EEX).editor("1_B");
    step("Setting units SI prefix to something wrong")
        .test(CLEAR, "123 ", ID_UnitsSIPrefixCycle)
        .error("Bad argument type");
    step("Purge SI units variable")
        .test(CLEAR, "'unitssiprefixcycle'", ID_Purge).noerror();
}


void tests::interactive_stack_operations()
// ----------------------------------------------------------------------------
//   Check interactive stack operations
// ----------------------------------------------------------------------------
{
    BEGIN(istack);

    step("Interactive stack")
        .test(CLEAR, EXIT, EXIT, EXIT,
              "111 222 333 444 555 666 'inv(sqrt((2+3*6)*X))' 888 999",
              ENTER,
              "X 2", ID_multiply, C, B, ENTER, UP)
        .image_noheader("istack-1");
    step("Interactive stack level 2")
        .test(UP)
        .image_noheader("istack-2");
    step("Interactive stack level scroll")
        .test(UP, UP, UP, UP, UP)
        .image_noheader("istack-3a");
    step("Interactive stack level reach end")
        .test(UP, UP, UP, UP, UP)
        .image_noheader("istack-3b");
    step("Interactive stack level down")
        .test(DOWN, DOWN)
        .image_noheader("istack-3c");
    step("Interactive stack level down scroll")
        .test(DOWN, DOWN, DOWN, DOWN, DOWN)
        .image_noheader("istack-3d");
    step("Interactive stack ->List")
        .test(LSHIFT, F5)
        .image_noheader("istack-4")
        .expect("{ 888 999 '(√(2·X))⁻¹' '(√(2·X))⁻¹' }");
    step("Interactive stack Pick")
        .test(UP, F5)
        .image_noheader("istack-5");
    step("Interactive stack Roll Down")
        .test(UP, UP, UP, UP, F4)
        .image_noheader("istack-6");
    step("Interactive stack Level")
        .test(RSHIFT, F6)
        .image_noheader("istack-7")
        .test(ENTER)
        .expect("6");
    step("Interactive stack jump to level 2")
        .test(UP, NOSHIFT, KEY2)
        .image_noheader("istack-7b");
    step("Interactive stack going up")
        .test(UP)
        .image_noheader("istack-8", 0, 1000);
    step("Interactive stack Show")
        .test(F2)
        .image_noheader("istack-9", 0, 2000);
    step("Interactive stack Show after EXIT")
        .test(EXIT)
        .image_noheader("istack-9b", 0, 1000);
    step("Interactive stack show with dot key")
        .test(NOSHIFT, UP, UP, NOSHIFT, DOT)
        .image_noheader("istack-9c", 0, 1000)
        .test(ENTER)
        .image_noheader("istack-9d", 0, 1000);
    step("Interactive stack Echo")
        .test(DOWN, F1, UP, F1, DOWN, F1)
        .image_noheader("istack-10", 0, 1000)
        .editor("666 555 666 ")
        .test(ENTER)
        .editor("666 555 666 ")
        .test(ENTER)
        .expect("666")
        .test(BSP)
        .expect("555");
    step("Interactive stack Echo without spaces")
        .test(UP, RSHIFT, F1, UP, RSHIFT, F1, DOWN, RSHIFT, F1)
        .image_noheader("istack-11", 0, 2000)
        .editor("555666555")
        .test(EXIT, EXIT);
    step("Interactive stack jump to level 5")
        .test(UP, NOSHIFT, KEY5)
        .image_noheader("istack-12", 0, 1000);
    step("Interactive stack jump to level 1")
        .test(NOSHIFT, KEY1)
        .image_noheader("istack-13", 0, 1000);
    step("Interactive stack jump to level 11")
        .test(NOSHIFT, KEY1)
        .image_noheader("istack-14", 0, 1000);
    step("Interactive stack jump to level 5")
        .test(NOSHIFT, KEY5)
        .image_noheader("istack-15", 0, 1000);
    step("Interactive stack evaluate level 5")
        .test(F3)
        .image_noheader("istack-16", 0, 1000);
    step("Interactive stack show level 5")
        .test(NOSHIFT, DOT)
        .image_noheader("istack-17", 0, 1000)
        .test(ENTER);
    step("Interactive stack info about 5")
        .test(LSHIFT, F6)
        .image_noheader("istack-18", 0, 1000)
        .test(ENTER);
    step("Interactive stack edit level 5")
        .test(F6)
        .image_noheader("istack-19", 0, 2000)
        .editor("'(√(20·X))⁻¹'");
    step("Interactive stack edit object that was at level 5")
        .test(UP, MUL, KEY3, ADD, KEY2)
        .editor("'(√(20·X))⁻¹·3+2'");
    step("Interactive stack end editing object level 5")
        .test(ENTER)
        .image_noheader("istack-20", 0, 1000)
        .test(ENTER, ADD)
        .expect("1 221");
    step("Interactive stack memory sort")
        .test(NOSHIFT, UP, NOSHIFT, KEY7, RSHIFT, F3)
        .image_noheader("istack-21", 0, 1000);
    step("Interactive stack revert")
        .test(RSHIFT, F4)
        .image_noheader("istack-22", 0, 1000);
    step("Interactive stack DropN")
        .test(KEY2, LSHIFT, F2)
        .image_noheader("istack-22b", 0, 1000);
    step("Interactive stack value sort")
        .test(NOSHIFT, KEY3, RSHIFT, F2)
        .image_noheader("istack-23", 0, 1000);
    step("Interactive stack revert")
        .test(RSHIFT, F4)
        .image_noheader("istack-24", 0, 1000);

    step("Interactive stack DupN and sort")
        .test(ENTER, CLEAR, "111 222 333 444", ENTER,
              UP, KEY3, LSHIFT, F1, KEY6, RSHIFT, F2, ENTER)
        .got("222", "222", "333", "333", "444", "444", "111");

    step("Interactive stack DupN and non-reverted sort")
        .test(ENTER, CLEAR, "123 456 789 ABC", ENTER,
              UP, KEY3, LSHIFT, F1, KEY6, RSHIFT, F3, ENTER)
        .got("789", "789", "456", "456", "'ABC'", "'ABC'", "123");

    step("Interactive stack DupN and reverted sort")
        .test(ENTER, CLEAR, "123 456 789 ABC", ENTER,
              UP, KEY3, LSHIFT, F1, KEY6, RSHIFT, F3, RSHIFT, F4, ENTER)
        .got("'ABC'", "'ABC'", "456", "456", "789", "789", "123");

    step("Interactive stack Keep")
        .test(ENTER, CLEAR, "123 456 789 ABC DEF GHI", ENTER,
              UP, UP, UP, LSHIFT, F3, ENTER)
        .got("'GHI'", "'DEF'", "'ABC'");

   step("Interactive stack Swap and Level")
        .test(ENTER, CLEAR, "123 456 789 ABC DEF GHI", ENTER,
              UP, UP, UP, RSHIFT, F5, RSHIFT, F6, ENTER)
       .got("3", "'GHI'", "'DEF'", "789", "'ABC'", "456", "123");
}


void tests::stack_operations()
// ----------------------------------------------------------------------------
//   Test stack operations
// ----------------------------------------------------------------------------
{
    BEGIN(stack);

    step("Multi-line stack rendering")
        .test(CLEAR, "[[1 2][3 4]]", ENTER)
        .noerror().expect("[[ 1 2 ]\n  [ 3 4 ]]")
        .test("SingleLineResult", ENTER)
        .noerror().expect("[[ 1 2 ][ 3 4 ]]")
        .test("MultiLineResult", ENTER)
        .noerror().expect("[[ 1 2 ]\n  [ 3 4 ]]");
    step("Multi-line stack rendering does not impact editing")
        .test(NOSHIFT, DOWN)
        .editor("[[ 1 2 ]\n  [ 3 4 ]]")
        .test(ENTER, "SingleLineResult", ENTER, DOWN)
        .editor("[[ 1 2 ]\n  [ 3 4 ]]")
        .test(ENTER, "MultiLineResult", ENTER, DOWN)
        .editor("[[ 1 2 ]\n  [ 3 4 ]]")
        .test(ENTER).noerror();

    step("Dup with ENTER")
        .test(CLEAR, "12", ENTER, ENTER, ADD).expect("24");
    step("Drop with Backspace")
        .test(CLEAR, "12 34", ENTER).noerror().expect("34")
        .test(BSP).noerror().expect("12")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("Dup in program")
        .test(CLEAR, "13 Dup +", ENTER).expect("26");
    step("Dup2 in program")
        .test(CLEAR, "13 25 Dup2 * + *", ENTER).expect("4 550");
    step("Over in program")
        .test(CLEAR, "13 25 Over / +", ENTER).expect("14 ¹²/₁₃");
    step("Rot in program")
        .test(CLEAR, "13 17 25 Rot / +", ENTER).expect("18 ¹²/₁₃");
    step("Nip in program")
        .test(CLEAR, "42 13 17 25 Nip / +", ENTER).expect("42 ¹³/₂₅");
    step("Pick3 in program")
        .test(CLEAR, "42 13 17 25 Pick3", ENTER).expect("13")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).expect("42")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("NDupN in program")
        .test(CLEAR, "13 17 25 42 3 NDUPN", ENTER)
        .got("3", "42", "42", "42", "25", "17", "13");
    step("NDupN with short stack")
        .test(CLEAR, "2 5 NDUPN", ENTER)
        .got("5", "2", "2", "2", "2", "2");
    step("DupDup in program")
        .test(CLEAR, "13 17 42 DUPDUP", ENTER).expect("42")
        .test(BSP).expect("42")
        .test(BSP).expect("42")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("Over in stack menu")
        .test(CLEAR, I, "13 25", F5, DIV, ADD).expect("14 ¹²/₁₃");
    step("Rot in stack menu")
        .test(CLEAR, "13 17 25", F3, DIV, ADD).expect("18 ¹²/₁₃");
    step("Depth in stack menu")
        .test(CLEAR, "13 17 25", RSHIFT, F3).expect("3");
    step("Pick in stack menu")
        .test(CLEAR, "13 17 25 2", LSHIFT, F5).expect("17");
    step("Roll in stack menu")
        .test(CLEAR, "13 17 25 42 21 372 3", F4).expect("42")
        .test(BSP).expect("372")
        .test(BSP).expect("21")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("RollDn in stack menu")
        .test(CLEAR, "13 17 25 42 21 372 4", LSHIFT, F4).expect("21")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("372")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DropN in stack menu")
        .test(CLEAR, "13 17 25 42 21 372 4", RSHIFT, F2).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DupN in stack menu")
        .test(CLEAR, "13 17 25 42 21 372 4", RSHIFT, F1).expect("372")
        .test(BSP).expect("21")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("372")
        .test(BSP).expect("21")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Drop2 in stack menu")
        .test(CLEAR, "13 17 25 42 21 372 4", LSHIFT, F2).expect("21")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Dup2 in stack menu")
        .test(CLEAR, "13 17 25 42", LSHIFT, F1).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Nip in stack menu")
        .test(CLEAR, "13 17 25 42", RSHIFT, F4).expect("42")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Pick3 in stack menu")
        .test(CLEAR, "13 17 25 42", RSHIFT, F5).expect("17")
        .test(BSP).expect("42")
        .test(BSP).expect("25")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("NDupN in stack menu")
        .test(CLEAR, "13 17 25 42 3", F6, LSHIFT, F1, F6)
        .got("3", "42", "42", "42", "25", "17", "13");
    step("NDupN with short stack")
        .test(CLEAR, "2 5", F6, LSHIFT, F1, F6)
        .got("5", "2", "2", "2", "2", "2");
    step("DupDup in stack menu")
        .test(CLEAR, "13 17 42", F6, LSHIFT, F2, F6).expect("42")
        .test(BSP).expect("42")
        .test(BSP).expect("42")
        .test(BSP).expect("17")
        .test(BSP).expect("13")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Simple stack commands from menu")
        .test(CLEAR, SHIFT, RUNSTOP,
              F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              F6,
              F1, F2, F3, F4, F5,
              LSHIFT, F1,
              F6, ENTER)
        .want("« Duplicate Drop Rot Roll Over "
              "Duplicate2 Drop2 UnRot RollDown Pick "
              "DuplicateN DropN Depth Nip Pick3 "
              "Swap LastArguments LastX Clear "
              "NDuplicateN »")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("LastArg")
        .test(CLEAR, "1 2", NOSHIFT, ADD).expect("3")
        .test(SHIFT, M).expect("2")
        .test(BSP).expect("1")
        .test(BSP).expect("3")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("Undo")
        .test(CLEAR, "1 2", NOSHIFT, ADD).expect("3")
        .test(RSHIFT, M).expect("2")
        .test(BSP).expect("1")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("LastX")
        .test(CLEAR, "1 2", NOSHIFT, ADD).expect("3")
        .test(NOSHIFT, I, F6, F3).expect("2")
        .test(BSP).expect("3")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("ClearStk")
        .test(CLEAR, "1 2 3 4", ENTER)
        .test(RSHIFT, F5).noerror()
        .test(BSP).error("Too few arguments");

    step("LastArg with Dup")
        .test(CLEAR, "1 2", ENTER, ENTER).expect("2")
        .test(LSHIFT, M).expect("2")
        .test(BSP).expect("2")
        .test(BSP).expect("2")
        .test(BSP).expect("1")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("LastArg with Drop")
        .test(CLEAR, "1 2", ENTER, BSP).expect("1")
        .test(LSHIFT, M).expect("2")
        .test(BSP).expect("1")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("LastArg with DupN")
        .test(CLEAR, "111 222 333 444 555", ENTER)
        .test("3", ID_StackMenu, ID_DupN,
              ID_LastArg, ID_Depth, ID_ObjectMenu, ID_ToList)
        .expect("{ 111 222 333 444 555 333 444 555 333 444 555 3 }");
    step("LastArg with DropN")
        .test(CLEAR, "111 222 333 444 555", ENTER)
        .test("3", ID_StackMenu, ID_DropN).expect("222")
        .test(ID_LastArg, ID_Depth, ID_ObjectMenu, ID_ToList)
        .expect("{ 111 222 333 444 555 3 }");
    step("LastArg with Pick")
        .test(CLEAR, "111 222 333 444 555", ENTER)
        .test("3", ID_StackMenu, ID_Pick).expect("333")
        .test(ID_LastArg, ID_Depth, ID_ListMenu, ID_ToList)
        .expect("{ 111 222 333 444 555 333 3 }");
}


void tests::arithmetic()
// ----------------------------------------------------------------------------
//   Tests for basic arithmetic operations
// ----------------------------------------------------------------------------
{
    BEGIN(arithmetic);

    step("Integer addition");
    test(CLEAR, 1, ENTER, 1, ADD).type(ID_integer).expect("2");
    test(1, ADD).type(ID_integer).expect("3");
    test(-1, ADD).type(ID_integer).expect("2");
    test(-1, ADD).type(ID_integer).expect("1");
    test(-1, ADD).type(ID_integer).expect("0");
    test(-1, ADD).type(ID_neg_integer).expect("-1");
    test(-1, ADD).type(ID_neg_integer).expect("-2");
    test(-1, ADD).type(ID_neg_integer).expect("-3");
    test(1, ADD).type(ID_neg_integer).expect("-2");
    test(1, ADD).type(ID_neg_integer).expect("-1");
    test(1, ADD).type(ID_integer).expect("0");

    step("Integer addition overflow");
    test(CLEAR, (1ULL << 63) - 2ULL, ENTER, 1, ADD)
        .type(ID_integer)
        .expect("9 223 372 036 854 775 807");
    test(CLEAR, (1ULL << 63) - 3ULL, CHS, ENTER, -2, ADD)
        .type(ID_neg_integer)
        .expect("-9 223 372 036 854 775 807");

    test(CLEAR, ~0ULL, ENTER, 1, ADD)
        .type(ID_bignum)
        .expect("18 446 744 073 709 551 616");
    test(CLEAR, ~0ULL, CHS, ENTER, -2, ADD)
        .type(ID_neg_bignum)
        .expect("-18 446 744 073 709 551 617");

    step("Adding ten small integers at random");
    srand48(sys_current_ms());
    Settings.MantissaSpacing(0);
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, ADD)
            .explain("Computing ", x, " + ", y, ", ")
            .expect(x + y);
    }
    Settings.MantissaSpacing(3);

    step("Integer subtraction");
    test(CLEAR, 1, ENTER, 1, SUB).type(ID_integer).expect("0");
    test(1, SUB).type(ID_neg_integer).expect("-1");
    test(-1, SUB).type(ID_integer).expect("0");
    test(-1, SUB).type(ID_integer).expect("1");
    test(-1, SUB).type(ID_integer).expect("2");
    test(1, SUB).type(ID_integer).expect("1");
    test(1, SUB).type(ID_integer).expect("0");
    test(3, SUB).type(ID_neg_integer).expect("-3");
    test(-1, SUB).type(ID_neg_integer).expect("-2");
    test(1, SUB).type(ID_neg_integer).expect("-3");
    test(-3, SUB).type(ID_integer).expect("0");

    step("Integer subtraction overflow");
    test(CLEAR, 0xFFFFFFFFFFFFFFFFull, CHS, ENTER, 1, SUB)
        .type(ID_neg_bignum)
        .expect("-18 446 744 073 709 551 616");
    test(CLEAR, -3, ENTER, 0xFFFFFFFFFFFFFFFFull, SUB)
        .type(ID_neg_bignum)
        .expect("-18 446 744 073 709 551 618");

    step("Subtracting ten small integers at random");
    Settings.MantissaSpacing(0);
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, SUB)
            .explain("Computing ", x, " - ", y, ", ")
            .expect(x - y);
    }
    Settings.MantissaSpacing(3);

    step("Integer multiplication");
    test(CLEAR, 3, ENTER, 7, MUL).type(ID_integer).expect("21");
    test(3, MUL).type(ID_integer).expect("63");
    test(-3, MUL).type(ID_neg_integer).expect("-189");
    test(2, MUL).type(ID_neg_integer).expect("-378");
    test(-7, MUL).type(ID_integer).expect("2 646");

    step("Multiplying ten small integers at random");
    Settings.MantissaSpacing(0);
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, MUL)
            .explain("Computing ", x, " * ", y, ", ")
            .expect(x * y);
    }
    Settings.MantissaSpacing(3);

    step("Integer division");
    test(CLEAR, 210, ENTER, 2, DIV).type(ID_integer).expect("105");
    test(5, DIV).type(ID_integer).expect("21");
    test(-3, DIV).type(ID_neg_integer).expect("-7");
    test(-7, DIV).type(ID_integer).expect("1");

    step("Dividing ten small integers at random");
    Settings.MantissaSpacing(0);
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0x3FFF) - 0x4000;
        large y = (lrand48() & 0x3FFF) - 0x4000;
        test(CLEAR, x * y, ENTER, y, DIV)
            .explain("Computing ", x * y, " / ", y, ", ")
            .expect(x);
    }
    Settings.MantissaSpacing(3);

    step("Division with fractional output");
    test(CLEAR, 1, ENTER, 3, DIV).expect("¹/₃");
    test(CLEAR, 2, ENTER, 5, DIV).expect("²/₅");

    step("Manual computation of 100!");
    test(CLEAR, 1, ENTER);
    for (uint i = 1; i <= 100; i++)
        test(i, MUL);
    expect( "93 326 215 443 944 152 681 699 238 856 266 700 490 715 968 264 "
            "381 621 468 592 963 895 217 599 993 229 915 608 941 463 976 156 "
            "518 286 253 697 920 827 223 758 251 185 210 916 864 000 000 000 "
            "000 000 000 000 000");
    step("Manual division by all factors of 100!");
    for (uint i = 1; i <= 100; i++)
        test(i * 997 % 101, DIV);
    expect(1);

    step("Manual computation of 997/100!");
    test(CLEAR, 997, ENTER);
    for (uint i = 1; i <= 100; i++)
        test(i * 997 % 101, DIV);
    expect("⁹⁹⁷/"
           "₉₃ ₃₂₆ ₂₁₅ ₄₄₃ ₉₄₄ ₁₅₂ ₆₈₁ ₆₉₉ ₂₃₈ ₈₅₆ ₂₆₆ ₇₀₀ ₄₉₀ ₇₁₅ ₉₆₈ "
           "₂₆₄ ₃₈₁ ₆₂₁ ₄₆₈ ₅₉₂ ₉₆₃ ₈₉₅ ₂₁₇ ₅₉₉ ₉₉₃ ₂₂₉ ₉₁₅ ₆₀₈ ₉₄₁ ₄₆₃ "
           "₉₇₆ ₁₅₆ ₅₁₈ ₂₈₆ ₂₅₃ ₆₉₇ ₉₂₀ ₈₂₇ ₂₂₃ ₇₅₈ ₂₅₁ ₁₈₅ ₂₁₀ ₉₁₆ ₈₆₄ "
           "₀₀₀ ₀₀₀ ₀₀₀ ₀₀₀ ₀₀₀ ₀₀₀ ₀₀₀ ₀₀₀");

    step("Computation of 2^256 (bug #460)")
        .test(CLEAR, 2, ENTER, 256, ID_pow)
        .expect("115 792 089 237 316 195 423 570 985 008 687 907 853 269 984 "
                "665 640 564 039 457 584 007 913 129 639 936");
    step("Sign of modulo and remainder");
    test(CLEAR, " 7  3 MOD", ENTER).expect(1);
    test(CLEAR, " 7 -3 MOD", ENTER).expect(1);
    test(CLEAR, "-7  3 MOD", ENTER).expect(2);
    test(CLEAR, "-7 -3 MOD", ENTER).expect(2);
    test(CLEAR, " 7  3 REM", ENTER).expect(1);
    test(CLEAR, " 7 -3 REM", ENTER).expect(1);
    test(CLEAR, "-7  3 REM", ENTER).expect(-1);
    test(CLEAR, "-7 -3 REM", ENTER).expect(-1);

    step("Fraction modulo and remainder");
    test(CLEAR, " 7/2  3 REM", ENTER).expect("¹/₂");
    test(CLEAR, " 7/2 -3 REM", ENTER).expect("¹/₂");
    test(CLEAR, "-7/2  3 REM", ENTER).expect("-¹/₂");
    test(CLEAR, "-7/2 -3 REM", ENTER).expect("-¹/₂");
    test(CLEAR, " 7/2  3 REM", ENTER).expect("¹/₂");
    test(CLEAR, " 7/2 -3 REM", ENTER).expect("¹/₂");
    test(CLEAR, "-7/2  3 REM", ENTER).expect("-¹/₂");
    test(CLEAR, "-7/2 -3 REM", ENTER).expect("-¹/₂");

    step("Modulo of negative value");
    test(CLEAR, "-360 360 MOD", ENTER).expect("0");
    test(CLEAR, "1/3 -1/3 MOD", ENTER).expect("0");
    test(CLEAR, "360 -360 MOD", ENTER).expect("0");
    test(CLEAR, "-1/3 1/3 MOD", ENTER).expect("0");

    step("Power");
    test(CLEAR, "2 3 ^", ENTER).expect("8");
    test(CLEAR, "-2 3 ^", ENTER).expect("-8");
    step("Negative power");
    test(CLEAR, "2 -3 ^", ENTER).expect("¹/₈");
    test(CLEAR, "-2 -3 ^", ENTER).expect("-¹/₈");

    step("Special case of 0^0")
        .test(CLEAR, "0 0 ^", ENTER).noerror().expect("1")
        .test(CLEAR,
              "ZeroPowerZeroIsUndefined", ENTER,
              "0 0 ^", ENTER).error("Undefined operation")
        .test(CLEAR,
              "ZeroPowerZeroIsOne", ENTER,
              "0 0 ^", ENTER).noerror().expect("1");

    step("xroot");
    test(CLEAR, "8 3 xroot", ENTER).expect("2.");
    test(CLEAR, "-8 3 xroot", ENTER).expect("-2.");
}


void tests::global_variables()
// ----------------------------------------------------------------------------
//   Tests for access to global variables
// ----------------------------------------------------------------------------
{
    BEGIN(globals);

    step("Store in global variable");
    test(CLEAR, 12345, ENTER).expect("12 345");
    test(XEQ, "A", ENTER).expect("'A'");
    test(STO).noerror();
    step("Recall global variable");
    test(CLEAR, 1, ENTER, XEQ, "A", ENTER).expect("'A'");
    test("RCL", ENTER).noerror().expect("12 345");

    step("Store with arithmetic")
        .test(CLEAR, "12 'A' STO+ A", ENTER).expect("12 357")
        .test(CLEAR, "13 'A' STO- A", ENTER).expect("12 344")
        .test(CLEAR, "5 'A' STO* A", ENTER).expect("61 720")
        .test(CLEAR, "2 'A' STO/ A", ENTER).expect("30 860");

    step("Recall with arithmetic")
        .test(CLEAR, "12 'A' RCL+", ENTER).expect("30 872")
        .test(CLEAR, "13 'A' RCL-", ENTER).expect("-30 847")
        .test(CLEAR, "2 'A' RCL*", ENTER).expect("61 720")
        .test(CLEAR, "2 'A' RCL/", ENTER).expect("¹/₁₅ ₄₃₀");

    step("Increment")
        .test(CLEAR, "'A' INCR", ENTER).expect("30 861")
        .test(CLEAR, "'A' Increment", ENTER).expect("30 862");

    step("Decrement")
        .test(CLEAR, "'A' DECR", ENTER).expect("30 861")
        .test(CLEAR, "'A' Decrement", ENTER).expect("30 860");

    step("Copy")
        .test(CLEAR, "42 'A' ▶", ENTER).expect("42")
        .test("A", ENTER).expect("42");
    step("Copy in algebraic")
        .test(CLEAR, "'A+A▶A' ", ENTER).expect("'A+A▶A'")
        .test(RUNSTOP).expect("84");
    step("Copy precedence of target symbol")
        .test("'A+A▶A+A'", ENTER).expect("'(A+A▶A)+A'")
        .test(RUNSTOP).expect("336")
        .test("A", ENTER).expect("168");
    step("Copy with external parentheses")
        .test("'(A+A▶A)+A'", ENTER).expect("'(A+A▶A)+A'")
        .test(RUNSTOP).expect("672")
        .test("A", ENTER).expect("336");
    step("Copy with internal parentheses")
        .test("'A+(A+1▶A)+A'", ENTER).expect("'A+(A+1▶A)+A'")
        .test(RUNSTOP).expect("1 010")
        .test("A", ENTER).expect("337");
    step("Check precedence for Copy")
        .test("'A+A▶A+A'", ENTER).expect("'(A+A▶A)+A'")
        .test(ID_ObjectMenu, ID_ToProgram)
        .want("« A A + 'A' ▶ A + »");
    step("Check that we can copy to a local variable")
        .test("5 2 3 → a b h « 'a·b+1▶h' EVAL 2 * h →V2 »", ENTER)
        .expect("[ 22 11 ]");

    step("Assignment with simple value")
        .test(CLEAR, "A=42", ENTER).got("A=42")
        .test(CLEAR, "A", ENTER).got("42");
    step("Assignment with evaluated value")
        .test(CLEAR, "A='42+3*5'", ENTER).expect("A='42+3·5'")
        .test("A", ENTER).expect("57")
        .test(BSP).expect("A='42+3·5'")
        .test(RUNSTOP).expect("A='42+3·5'");
    step("Assignment with error in evaluation")
        .test(CLEAR, "A='(42+3*5)/0'", ENTER).error("Divide by zero");
    step("Assignment with evaluated value and PushEvaluatedAssignment")
        .test(CLEAR, "PushEvaluatedAssignment", ENTER)
        .test("A='42+3*5'", ENTER).expect("A=57")
        .test("A", ENTER).expect("57")
        .test(BSP).expect("A=57")
        .test(RUNSTOP).expect("A=57")
        .test("'pushevaluatedassignment' purge", ENTER);
    step("Assignment with evaluated value and PushEvaluatedAssignment purged")
        .test(CLEAR, "A='42+3*5'", ENTER).expect("A='42+3·5'")
        .test("A", ENTER).expect("57")
        .test(BSP).expect("A='42+3·5'")
        .test(RUNSTOP).expect("A='42+3·5'");

    step("Clone")
        .test(CLEAR,
              "Mem Drop Mem "
              "{ 1 2 3 4 5 6 } 75 * 3 Get "
              "Mem Swap Clone Mem Nip "
              "→ A B C « A B - A C - »", ENTER)
        .expect("34")
        .test(BSP).expect("923");

    step("Memory menu")
        .test(CLEAR, ID_MemoryMenu, RSHIFT, RUNSTOP,
              F1, F2, F3, F4, F5,
              ENTER)
        .expect("{ Store Recall Purge CreateDirectory UpDirectory }")
        .test(RSHIFT, RUNSTOP,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              ENTER)
        .expect("{ AvailableMemory Variables HomeDirectory"
                " DirectoryPath GarbageCollect }")
        .test(RSHIFT, RUNSTOP,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("{ FreeMemory TypedVariables PurgeAll"
                " RuntimeStatistics GarbageCollectorStatistics }")
        .test(F6,
              RSHIFT, RUNSTOP,
              F1, F2, F3, F4, F5,
              ENTER)
        .expect("{ Store Store+ Store- Store× Store÷ }")
        .test(RSHIFT, RUNSTOP,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              ENTER)
        .expect("{ Recall Recall+ Recall- Recall× Recall÷ }")
        .test(RSHIFT, RUNSTOP,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("{ ▶ Clone Increment Decrement CurrentDirectory }")
        .test(F6,
              RSHIFT, RUNSTOP,
              F1, F2, F3, F4, F5,
              ENTER)
        .expect("{ GarbageCollectorStatistics RuntimeStatistics"
                " AvailableMemory SystemMemory Bytes }")
        .test(RSHIFT, RUNSTOP,
              LSHIFT, F1, LSHIFT, F2, ENTER)
        .expect("{ GCStatsClearAfterRead RunStatsClearAfterRead }");

    step("Store in long-name global variable");
    test(CLEAR, "\"Hello World\"", ENTER, XEQ, "SomeLongVariable", ENTER, STO)
        .noerror();
    step("Recall global variable");
    test(CLEAR, XEQ, "SomeLongVariable", ENTER, "recall", ENTER)
        .noerror()
        .expect("\"Hello World\"");

    step("Recall non-existent variable");
    test(CLEAR, XEQ, "DOESNOTEXIST", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();

    step("Store and recall invalid variable object");
    test(CLEAR, 5678, ENTER, 1234, ENTER,
         "STO", ENTER).error("Invalid name").clear();
    test(CLEAR, 1234, ENTER,
         "RCL", ENTER).error("Invalid name").clear();

    step("Store and recall to EQ");
    test(CLEAR, "'X+Y' 'eq' STO", ENTER).noerror();
    test(CLEAR, "'EQ' RCL", ENTER).expect("'X+Y'");
    test(CLEAR, "'equation' RCL", ENTER).expect("'X+Y'");
    test(CLEAR, "'Equation' PURGE", ENTER).noerror();

    step("Store and recall to ΣData");
    test(CLEAR, "[1 2 3] 'ΣData' STO", ENTER).noerror();
    test(CLEAR, "'ΣDat' RCL", ENTER).expect("[ 1 2 3 ]");
    test(CLEAR, "'StatsData' RCL", ENTER).expect("[ 1 2 3 ]");
    test(CLEAR, "'ΣData' PURGE", ENTER).noerror();

    step("Store and recall to StatsParameters")
        .test(CLEAR, "{0} 'ΣParameters' STO", ENTER).noerror()
        .test("'ΣPar' RCL", ENTER).expect("{ 0 }")
        .test("'StatsParameters' RCL", ENTER).expect("{ 0 }")
        .test("'ΣPar' purge", ENTER).noerror()
        .test("'StatsParameters' RCL", ENTER).error("Undefined name");

    step("Store and recall to PlotParameters")
        .test(CLEAR, "{1} 'PPAR' STO", ENTER).noerror()
        .test("'PlotParameters' RCL", ENTER).expect("{ 1 }")
        .test("'ppar' RCL", ENTER).expect("{ 1 }")
        .test("'PPAR' PGALL", ENTER).noerror()
        .test("'PlotParameters' RCL", ENTER).error("Undefined name");

    step("Numbered store and recall should fail by default");
    test(CLEAR, 5678, ENTER, 1234, ENTER, "STO", ENTER).error("Invalid name");
    test(CLEAR, 1234, ENTER, "RCL", ENTER).error("Invalid name");
    test(CLEAR, 1234, ENTER, "Purge", ENTER).error("Invalid name");

    step("Enable NumberedVariables");
    test(CLEAR, "NumberedVariables", ENTER).noerror();
    test(CLEAR, 5678, ENTER, 1234, ENTER, "STO", ENTER).noerror();
    test(CLEAR, 1234, ENTER, "RCL", ENTER).noerror().expect("5 678");
    test(CLEAR, 1234, ENTER, "Purge", ENTER).noerror();

    step("Disable NumberedVariables");
    test(CLEAR, "NoNumberedVariables", ENTER).noerror();
    test(CLEAR, 5678, ENTER, 1234, ENTER, "STO", ENTER).error("Invalid name");
    test(CLEAR, 1234, ENTER, "RCL", ENTER).error("Invalid name");
    test(CLEAR, 1234, ENTER, "Purge", ENTER).error("Invalid name");

    step("Store program in global variable");
    test(CLEAR, "« 1 + »", ENTER, XEQ, "MyINCR", ENTER, STO).noerror();
    step("Evaluate global variable");
    test(CLEAR, "A MyINCR", ENTER).expect("58");

    step("Purge global variable");
    test(CLEAR, XEQ, "A", ENTER, "PURGE", ENTER).noerror();
    test(CLEAR,
         "{ MyINCR SomeLongVariable }", ENTER,
         "PURGE", ENTER).noerror();

    test(CLEAR, XEQ, "A", ENTER, "RCL", ENTER).error("Undefined name").clear();
    test(CLEAR, XEQ, "MyINCR", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();
    test(CLEAR, XEQ, "SomeLongVariable", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();

    step("Go to top-level")
        .test(CLEAR, "Home", ENTER).noerror();
    step("Clear 'DirTest'")
        .test(CLEAR, "'DirTest' pgdir", ENTER);
    step("Create directory")
        .test(CLEAR, "'DirTest' crdir", ENTER).noerror();
    step("Enter directory")
        .test(CLEAR, "DirTest", ENTER).noerror();
    step("Path function")
        .test(CLEAR, "PATH", ENTER).expect("{ HomeDirectory DirTest }");
    step("Vars command")
        .test(CLEAR, "VARS", ENTER).expect("{ }");
    step("Updir function")
        .test(CLEAR, "UpDir path", ENTER).expect("{ HomeDirectory }");
    step("Enter directory again")
        .test(CLEAR, "DirTest path", ENTER).expect("{ HomeDirectory DirTest }");
    step("Current directory content")
        .test(CLEAR, "CurrentDirectory", ENTER).want("Directory { }");
    step("Store in subdirectory")
        .test(CLEAR, "242 'Foo' STO", ENTER).noerror();
    step("Recall from subdirectory")
        .test(CLEAR, "Foo", ENTER).expect("242");
    step("Store another variable in subdirectory")
        .test(CLEAR, "\"Glop\" 'Baz' STO", ENTER).noerror();
    step("List variables in subdirectory")
        .test(CLEAR, "variables", ENTER).expect("{ Baz Foo }");
    step("List variables in subdirectory with the correct type")
        .test(CLEAR, "28 tvars", ENTER).expect("{ Foo }")
        .test(CLEAR, "2 tvars", ENTER).expect("{ Baz }");
    step("List variables in subdirectory with an incorrect type")
        .test(CLEAR, "0 tvars", ENTER).expect("{ }");
    step("List variables in subdirectory with multiple types")
        .test(CLEAR, "{ 0 2 } tvars", ENTER).expect("{ Baz }")
        .test(CLEAR, "{ 28 2 } tvars", ENTER).expect("{ Baz Foo }");
    step("List variables in subdirectory with DB48X types")
        .test(CLEAR, ~int(ID_integer)," tvars", ENTER)
        .expect("{ Foo }")
        .test(CLEAR, "{ ",
              ~int(ID_integer), " ",
              ~int(ID_array), " } tvars", ENTER)
        .expect("{ Foo }")
        .test(CLEAR, "{ ",
              ~int(ID_text), " ",
              ~int(ID_integer), " } tvars", ENTER)
        .expect("{ Baz Foo }")
        .test(CLEAR, "{ 28 2 } tvars", ENTER).expect("{ Baz Foo }");
    step("Recursive directory")
        .test(CLEAR, "'DirTest2' crdir", ENTER).noerror();
    step("Entering sub-subdirectory")
        .test(CLEAR, "DirTest2", ENTER).noerror();
    step("Path in sub-subdirectory")
        .test(CLEAR, "path", ENTER)
        .expect("{ HomeDirectory DirTest DirTest2 }");

    step("Check that we cannot purge a directory we are in")
        .test(CLEAR, "'DirTest' PurgeAll", ENTER)
        .error("Cannot purge active directory");

    step("Find variable from level above")
        .test(CLEAR, "Foo", ENTER).expect("242");
    step("Create local variable")
        .test(CLEAR, "\"Hello\" 'Foo' sto", ENTER).noerror();
    step("Local variable hides variable above")
        .test(CLEAR, "Foo", ENTER).expect("\"Hello\"");
    step("Updir shows shadowed variable again")
        .test(CLEAR, "Updir Foo", ENTER).expect("242");
    step("Two independent variables with the same name")
        .test(CLEAR, "DirTest2 Foo", ENTER).expect("\"Hello\"");
    step("Cleanup")
        .test(CLEAR, "'Foo' Purge", ENTER).noerror();

    step("Make sure elements are cloned when purging (#854)")
        .test(CLEAR, "{ 11 23 34 44 } 'X' Sto", ENTER).noerror()
        .test("X", ENTER).expect("{ 11 23 34 44 }")
        .test("X 1 GET", ENTER).expect("11")
        .test("X 2 GET", ENTER).expect("23")
        .test("X 3 GET", ENTER).expect("34")
        .test("X 4 GET", ENTER).expect("44")
        .test("'X' Purge", ENTER).expect("44")
        .test(NOSHIFT, BSP).expect("34")
        .test(NOSHIFT, BSP).expect("23")
        .test(NOSHIFT, BSP).expect("11")
        .test(NOSHIFT, BSP).expect("{ 11 23 34 44 }");

    step("Save to file as text")
        .test(CLEAR, "1.42 \"Hello.txt\"", NOSHIFT, G).noerror();
    step("Restore from file as text")
        .test(CLEAR, "\"Hello.txt\" RCL", ENTER).noerror().expect("\"1.42\"");
    step("Save to file as source")
        .test(CLEAR, "1.42 \"Hello.48s\"", NOSHIFT, G).noerror();
    step("Restore from file as source")
        .test(CLEAR, "\"Hello.48s\" RCL", ENTER).noerror().expect("1.42");
    step("Save to file as binary")
        .test(CLEAR, "1.42 \"Hello.48b\"", NOSHIFT, G).noerror();
    step("Restore from file as text")
        .test(CLEAR, "\"Hello.48b\" RCL", ENTER).noerror().expect("1.42");
    step("Save to file as BMP")
        .test(CLEAR, "'X' cbrt inv 1 + sqrt dup 1 + /", ENTER)
        .test("\"Hello.bmp\" STO", ENTER).noerror();
    step("Recall from file as BMP")
        .test(CLEAR, EXIT, "\"Hello.bmp\" RCL", ENTER).noerror()
        .image_noheader("rcl-bmp");

    step("Allowing command names in quotes")
        .test(CLEAR, "'bar'", ENTER)
        .expect("'BarPlot'");
    step("Editing command names")
        .test(DOWN).editor("'BarPlot'")
        .test(ENTER)
        .expect("'BarPlot'");
    step("Rejecting command names as variable names")
        .test(CLEAR, "124 'bar' STO", ENTER)
        .error("Invalid name");
}


void tests::local_variables()
// ----------------------------------------------------------------------------
//   Tests for access to local variables
// ----------------------------------------------------------------------------
{
    BEGIN(locals);

    step("Creating a local block");
    cstring source = "« → A B C « A B + A B - × B C + B C - × ÷ » »";
    test(CLEAR, source, ENTER).type(ID_program).want(source);
    test(XEQ, "LocTest", ENTER, STO).noerror();

    step("Calling a local block with numerical values");
    test(CLEAR, 1, ENTER, 2, ENTER, 3, ENTER, "LocTest", ENTER).expect("³/₅");

    step("Calling a local block with symbolic values");
    test(CLEAR,
         XEQ, "X", ENTER,
         XEQ, "Y", ENTER,
         XEQ, "Z", ENTER,
         "LocTest", ENTER)
        .expect("'(X+Y)·(X-Y)÷((Y+Z)·(Y-Z))'");

    step("Cleanup");
    test(CLEAR, XEQ, "LocTest", ENTER, "PurgeAll", ENTER).noerror();
}


void tests::for_loops()
// ----------------------------------------------------------------------------
//   Test simple for loops
// ----------------------------------------------------------------------------
{
    BEGIN(for_loops);

    step("Disable auto-simplification")
        .test(CLEAR, "noautosimplify", ENTER).noerror();

    step("Simple 1..10");
    cstring pgm  = "« 0 1 10 FOR i i SQ + NEXT »";
    cstring pgmo = "« 0 1 10 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP).noerror().type(ID_integer).expect(385);

    step("Algebraic 1..10");
    pgm  = "« 'X' 1 5 FOR i i SQ + NEXT »";
    pgmo = "« 'X' 1 5 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP).noerror().type(ID_expression).expect("'X+1+4+9+16+25'");

    step("Stepping by 2");
    pgm  = "« 0 1 10 FOR i i SQ + 2 STEP »";
    pgmo = "« 0 1 10 for i i x² + 2 step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP).noerror().type(ID_integer).expect(165);

    step("Stepping by i");
    pgm  = "« 'X' 1 100 FOR i i SQ + i step »";
    pgmo = "« 'X' 1 100 for i i x² + i step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP)
        .noerror()
        .type(ID_expression)
        .expect("'X+1+4+16+64+256+1 024+4 096'");

    step("Negative stepping");
    pgm  = "« 0 10 1 FOR i i SQ + -1 STEP »";
    pgmo = "« 0 10 1 for i i x² + -1 step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP).noerror().type(ID_integer).expect(385);

    step("Negative stepping algebraic");
    pgm  = "« 'X' 10 1 FOR i i SQ + -1 step »";
    pgmo = "« 'X' 10 1 for i i x² + -1 step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP)
        .noerror()
        .type(ID_expression)
        .expect("'X+100+81+64+49+36+25+16+9+4+1'");

    step("Fractional");
    pgm  = "« 'X' 0.1 0.9 FOR i i SQ + 0.1 step »";
    pgmo = "« 'X' 0.1 0.9 for i i x² + 0.1 step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP)
        .noerror()
        .type(ID_expression)
        .expect("'X+0.01+0.04+0.09+0.16+0.25+0.36+0.49+0.64+0.81'");

    step("Fractional down");
    pgm  = "« 'X' 0.9 0.1 FOR i i SQ + -0.1 step »";
    pgmo = "« 'X' 0.9 0.1 for i i x² + -0.1 step »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP)
        .noerror()
        .type(ID_expression)
        .expect("'X+0.81+0.64+0.49+0.36+0.25+0.16+0.09+0.04+0.01'");

    step("Execute at least once");
    pgm  = "« 'X' 10 1 FOR i i SQ + NEXT »";
    pgmo = "« 'X' 10 1 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerror().type(ID_program).want(pgmo);
    test(RUNSTOP).noerror().type(ID_expression).expect("'X+100'");

    step("Update variable inside the loop")
        .test(CLEAR,
              "1 10 FOR i "
              " i "
              " IF i 3 > THEN 'Exiting' 100 'i' STO END "
              "NEXT", ENTER)
        .got("'Exiting'", "4", "3", "2", "1");

    step("For loop on a list")
        .test(CLEAR, "{ 1 3 5 \"ABC\" } for i i 2 * 1 + next", ENTER)
        .expect("\"ABCABC1\"")
        .test(BSP).expect("11")
        .test(BSP).expect("7")
        .test(BSP).expect("3")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("For loop on an array")
        .test(CLEAR, "[ 1 3 5 \"ABC\" ] for i i 2 * 1 + next", ENTER)
        .expect("\"ABCABC1\"")
        .test(BSP).expect("11")
        .test(BSP).expect("7")
        .test(BSP).expect("3")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("Nested for loop on lists")
        .test(CLEAR,
              "{ A B C} for i { D E F G } for j i j * next next "
              "12 →List", ENTER)
        .expect("{ 'A·D' 'A·E' 'A·F' 'A·G' "
                "'B·D' 'B·E' 'B·F' 'B·G' "
                "'C·D' 'C·E' 'C·F' 'C·G' }");

    step("Restore auto-simplification")
        .test(CLEAR, "'noautosimplify' purge", ENTER).noerror();

}


void tests::conditionals()
// ----------------------------------------------------------------------------
//   Test conditionals
// ----------------------------------------------------------------------------
{
    BEGIN(conditionals);

    step("If-Then (true)");
    test(CLEAR, "PASS if 0 0 > then FAIL end", ENTER)
        .expect("'PASS'");
    step("If-Then (false)");
    test(CLEAR, "FAIL if 1 0 > then PASS end", ENTER)
        .expect("'PASS'");
    step("If-Then-Else (true)");
    test(CLEAR, "if 1 0 > then PASS else FAIL end", ENTER)
        .expect("'PASS'");
    step("If-Then-Else (false)");
    test(CLEAR, "if 1 0 = then FAIL else PASS end", ENTER)
        .expect("'PASS'");

    step("IFT command (true)");
    test(CLEAR, "FAIL true PASS IFT", ENTER)
        .expect("'PASS'");
    step("IFT command (false)");
    test(CLEAR, "PASS 0 FAIL IFT", ENTER)
        .expect("'PASS'");
    step("IFTE command (true)");
    test(CLEAR, "true PASS FAIL IFTE", ENTER)
        .expect("'PASS'");
    step("IFTE command (false)");
    test(CLEAR, "0 FAIL PASS IFTE", ENTER)
        .expect("'PASS'");

    step("IFT command (true evaluation)");
    test(CLEAR, "FAIL '1+1' 'PASS+0' IFT", ENTER)
        .expect("'PASS'");
    step("IFT command (false no evaluation)");
    test(CLEAR, "PASS '1-1' 'ln(0)' IFT", ENTER)
        .expect("'PASS'");
    step("IFTE command (true evaluation)");
    test(CLEAR, "'1+1' 'PASS+0' 'ln(0)' IFTE", ENTER)
        .expect("'PASS'");
    step("IFTE command (false evaluation)");
    test(CLEAR, "'1-1' 'ln(0)' 'PASS+0' IFTE", ENTER)
        .expect("'PASS'");

    step("IFTE expression, true case)");
    test(CLEAR, "'IFTE(1+1;PASS+0;ln(0))'", ENTER, RUNSTOP)
        .expect("'PASS'");
    step("IFTE expression (false case)");
    test(CLEAR, "'IFTE(1-1;ln(0);PASS+0)'", ENTER, RUNSTOP)
        .expect("'PASS'");

    step("Clear DebugOnError for IfErr tests")
        .test(CLEAR, "DebugOnError", ENTER).noerror();
    step("IfErr-Then (true)");
    test(CLEAR, "FAIL iferr 1 0 / drop then PASS end", ENTER)
        .expect("'PASS'");
    step("IfErr-Then (false)");
    test(CLEAR, "PASS iferr 1 0 + drop then FAIL end", ENTER)
        .expect("'PASS'");
    step("IfErr-Then-Else (true)");
    test(CLEAR, "iferr 1 0 / drop then PASS ELSE FAIL end", ENTER)
        .expect("'PASS'");
    step("IfErr-Then-Else (false)");
    test(CLEAR, "IFERR 1 0 + drop then FAIL ELSE PASS end", ENTER)
        .expect("'PASS'");

    step("IfErr reading error message");
    test(CLEAR, "iferr 1 0 / drop then errm end", ENTER)
        .expect("\"Divide by zero\"");
    step("IfErr reading error number");
    test(CLEAR, "iferr 1 0 / drop then errn end", ENTER)
        .type(ID_based_integer)
        .expect("#B₁₆");        // May change if you update errors.tbl

    step("DoErr with built-in message");
    test(CLEAR, "3 DoErr", ENTER)
        .error("Too few arguments");
    step("DoErr with custom message");
    test(CLEAR, "\"You lose!\" doerr \"You lose worse!\"", ENTER)
        .error("You lose!");
    step("errm for custom error message")
        .test(CLEARERR).noerror()
        .test("errm", ENTER)
        .expect("\"You lose!\"");
    step("errn for custom error message");
    test("errn", ENTER)
        .expect("#7 0000₁₆");

    step("Getting message after iferr");
    test(CLEAR, "« FAILA iferr 1 0 / then FAILB end errm »",
         ENTER, RUNSTOP)
        .expect("\"Divide by zero\"");

    step("err0 clearing message");
    test(CLEAR, "« FAILA iferr 1 0 / then FAILB end err0 errm errn »",
         ENTER, RUNSTOP)
        .expect("#0₁₆")
        .test(BSP)
        .expect("\"\"");

    // Same thing with menus
    step("Menu if-Then (true)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "PASS", LSHIFT, F1, "0 0", F3, RSHIFT, DOWN, F3, " FAIL", ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("Menu if-Then (false)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "FAIL", LSHIFT, F1, "0 0", F2, RSHIFT, DOWN, F3, " PASS", ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("If-Then-Else (true)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         LSHIFT, F2, "1 0", F3,
         RSHIFT, DOWN, F3, " PASS",
         RSHIFT, DOWN, F3, " FAIL", ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("Menu If-Then-Else (false)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         LSHIFT, F2, "1 0", F2,
         RSHIFT, DOWN, F3, " FAIL",
         RSHIFT, DOWN, F3, " PASS", ENTER,
         RUNSTOP)
        .expect("'PASS'");

    step("Menu IFT command (true)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "FAIL 1 0", F3, " PASS", LSHIFT, F5, ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("Menu IFT command (false)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "PASS 0 FAIL", LSHIFT, F5, ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("Menu IFTE command (true)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "1 0", F2, " FAIL PASS", LSHIFT, F6, ENTER,
         RUNSTOP)
        .expect("'PASS'");
    step("Menu IFTE command (false)");
    test(CLEAR,
         LSHIFT, KEY3, LSHIFT, F2, LSHIFT, RUNSTOP,
         "0 FAIL PASS", LSHIFT, F6, ENTER,
         RUNSTOP)
        .expect("'PASS'");

    // Make sure we enforce conditionals when evaluating conditions
    step("Conditionals forward progress")
        .test(CLEAR, "if FAIL then else end", ENTER)
        .error("Bad argument type");
    step("Conditionals forward progress with program")
        .test(CLEAR, "if 0 PASS FAIL IFTE then else end", ENTER)
        .error("Bad argument type");
    step("Conditionals forward progress with true condition")
        .test(CLEAR, "if 0 1 2 IFTE then PASS else FAIL end", ENTER)
        .expect("'PASS'");
    step("Conditionals forward progress with false condition")
        .test(CLEAR, "if 0 1 0 IFTE then FAIL else PASS end", ENTER)
        .expect("'PASS'");

    step("Restore the KillOnError setting for testing")
        .test(CLEAR, "KillOnError Kill", ENTER);
}


void tests::logical_operations()
// ----------------------------------------------------------------------------
//   Perform logical operations on small and big integers
// ----------------------------------------------------------------------------
{
    BEGIN(logical);

#if CONFIG_FIXED_BASED_OBJECTS
    step("Binary number");
    cstring binary  = "#10001b";
    cstring binaryf = "#1 0001₂";
    test(CLEAR, binary, ENTER).type(ID_bin_integer).expect(binaryf);

    step("Octal number");
    cstring octal  = "#1777o";
    cstring octalf = "#1777₈";
    test(CLEAR, octal, ENTER).type(ID_oct_integer).expect(octalf);

    step("Decimal based number");
    cstring decimal  = "#12345d";
    cstring decimalf = "#1 2345₁₀";
    test(CLEAR, decimal, ENTER).type(ID_dec_integer).expect(decimalf);

    step("Hexadecimal number");
    cstring hexa  = "#135AFh";
    cstring hexaf = "#1 35AF₁₆";
    test(CLEAR, hexa, ENTER).type(ID_hex_integer).expect(hexaf);
#endif // CONFIG_FIXED_BASED_OBJECTS

    step("Based number (default base)");
    cstring based  = "#1234A";
    cstring basedf = "#1 234A₁₆";
    test(CLEAR, based, ENTER).type(ID_based_integer).expect(basedf);

    step("Based number (arbitrary base)");
    cstring abased  = "17#1234AG";
    cstring abasedf = "#18 75A4₁₆";
    test(CLEAR, abased, ENTER).type(ID_based_integer).expect(abasedf);

    step("Display in arbitrary base");
    test("17 base", ENTER).expect("#12 34AG₁₇");
    test("3 base", ENTER).expect("#10 0001 0221 2122₃");
    test("36 base", ENTER).expect("#YCV8₃₆");
    test("16 base", ENTER).expect("#18 75A4₁₆");

    step("Range for bases");
    test("1 base", ENTER).error("Argument outside domain");
    test(CLEAR, "37 base", ENTER).error("Argument outside domain");
    test(CLEAR, "0.3 base", ENTER).error("Argument outside domain");
    test(CLEAR);

    step("Default word size");
    test("RCWS", ENTER).expect("64");
    step("Set word size to 16");
    test(CLEAR, "16 STWS", ENTER).noerror();

    step("Binary not");
    test(CLEAR, "#12 not", ENTER).expect("#FFED₁₆");
    test("not", ENTER).expect("#12₁₆");

    step("Binary neg");
    test(CLEAR, "#12 neg", ENTER).expect("#FFEE₁₆");
    test("neg", ENTER).expect("#12₁₆");

    step("Binary or");
    test(CLEAR, "#123 #A23 or", ENTER).expect("#B23₁₆");

    step("Binary xor");
    test(CLEAR, "#12 #A23 xor", ENTER).expect("#A31₁₆");

    step("Binary and");
    test(CLEAR, "#72 #A23 and", ENTER).expect("#22₁₆");

    step("Binary nand");
    test(CLEAR, "#72 #A23 nand", ENTER).expect("#FFDD₁₆");

    step("Binary nor");
    test(CLEAR, "#72 #A23 nor", ENTER).expect("#F58C₁₆");

    step("Binary implies");
    test(CLEAR, "#72 #A23 implies", ENTER).expect("#FFAF₁₆");

    step("Binary excludes");
    test(CLEAR, "#72 #A23 excludes", ENTER).expect("#50₁₆");

    step("Set word size to 32");
    test(CLEAR, "32 STWS", ENTER).noerror();
    test(CLEAR, "#12 not", ENTER).expect("#FFFF FFED₁₆");
    test("not", ENTER).expect("#12₁₆");

    step("Set word size to 30");
    test(CLEAR, "30 STWS", ENTER).noerror();
    test(CLEAR, "#142 not", ENTER).expect("#3FFF FEBD₁₆");
    test("not", ENTER).expect("#142₁₆");
    test(CLEAR, "#142 neg", ENTER).expect("#3FFF FEBE₁₆");
    test("neg", ENTER).expect("#142₁₆");
    test("#3 #5 -", ENTER).expect("#3FFF FFFE₁₆");

    step("Set word size to 48");
    test(CLEAR, "48 STWS", ENTER).noerror();
    test(CLEAR, "#233 not", ENTER).expect("#FFFF FFFF FDCC₁₆");
    test("not", ENTER).expect("#233₁₆");
    test(CLEAR, "#233 neg", ENTER).expect("#FFFF FFFF FDCD₁₆");
    test("neg", ENTER).expect("#233₁₆");
    test("#8 #15 -", ENTER).expect("#FFFF FFFF FFF3₁₆");

    step("Set word size to 64");
    test(CLEAR, "64 STWS", ENTER).noerror();
    test(CLEAR, "#64123 not", ENTER).expect("#FFFF FFFF FFF9 BEDC₁₆");
    test("not", ENTER).expect("#6 4123₁₆");
    test(CLEAR, "#64123 neg", ENTER).expect("#FFFF FFFF FFF9 BEDD₁₆");
    test("neg", ENTER).expect("#6 4123₁₆");
    test("#8 #21 -", ENTER).expect("#FFFF FFFF FFFF FFE7₁₆");

    step("Set word size to 128");
    test(CLEAR, "128 STWS", ENTER).noerror();
    test(CLEAR, "#12 not", ENTER)
        .expect("#FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFED₁₆");
    test("dup not", ENTER).expect("#12₁₆");
    test("xor not", ENTER).expect("#0₁₆");
    test(CLEAR, "#12 neg", ENTER)
        .expect("#FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEE₁₆");
    test("neg", ENTER)
        .expect("#12₁₆");
    test("#7A02 #21445 -", ENTER)
        .expect("#FFFF FFFF FFFF FFFF FFFF FFFF FFFE 65BD₁₆");

    step("Set word size to 623");
    test(CLEAR, "623 STWS", ENTER).noerror();
    test(CLEAR, "#12 not", ENTER)
        .expect("#7FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFED₁₆");
    test("dup not", ENTER).expect("#12₁₆");
    test("xor not", ENTER).expect("#0₁₆");
    test("#7A03 #21447 -", ENTER)
        .expect("#7FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFE 65BC₁₆");
    test(CLEAR, "#12 neg", ENTER)
        .expect("#7FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF "
                "FFFF FFFF FFEE₁₆");
    test("#12", ID_add).expect("#0₁₆");

    step("Check that arithmetic truncates to small word size (#624)")
        .test("15 STWS", ENTER).noerror()
        .test("#0 #4 -", ENTER).expect("#7FFC₁₆")
        .test("#321 *", ENTER).expect("#737C₁₆")
        .test("#27 /", ENTER).expect("#2F6₁₆")
        .test("13 STWS", ENTER).noerror()
        .test("#0 #6 -", ENTER).expect("#1FFA₁₆")
        .test("#321 *", ENTER).expect("#D3A₁₆")
        .test("#27 /", ENTER).expect("#56₁₆");

    step("Reset word size to default")
        .test(CLEAR, "64 WordSize", ENTER).noerror();

    step("Check that we promote to binary")
        .test(CLEAR, "100 #45 AND", ENTER).expect("#44₁₆")
        .test(CLEAR, "#45 100 AND", ENTER).expect("#44₁₆");
    step("Check that deal with logical")
        .test(CLEAR, "100. #45 AND", ENTER).expect("True")
        .test(CLEAR, "#45 100. AND", ENTER).expect("True");


    step("Use large word size")
        .test(CLEAR, "1024 STWS", ENTER).noerror();

    step("First bit set (integer)")
        .test(CLEAR, "16#147800 FirstBitSet", ENTER)
        .expect("11")
        .test(CLEAR, "2 22 ^ FirstBitSet", ENTER)
        .expect("22");
    step("First bit set (zero)")
        .test(CLEAR, "16#0 FirstBitSet", ENTER)
        .expect("-1");
    step("First bit set (bignum)")
        .test(CLEAR, "16#147800 75 SLC FirstBitSet", ENTER)
        .expect("86")
        .test(CLEAR, "2 103 ^ FirstBitSet", ENTER)
        .expect("103");;
    step("Last bit set (integer)")
        .test(CLEAR, "16#147800 LastBitSet", ENTER)
        .expect("20")
        .test(CLEAR, "2 22 ^ LastBitSet", ENTER)
        .expect("22");
    step("Last bit set (zero)")
        .test(CLEAR, "16#0 LastBitSet", ENTER)
        .expect("-1");
    step("Last bit set (bignum)")
        .test(CLEAR, "16#147800 75 SLC LastBitSet", ENTER)
        .expect("95")
        .test(CLEAR, "2 102 ^ LastBitSet", ENTER)
        .expect("102");
;
    step("Count bits set (integer)")
        .test(CLEAR, "16#147800 CountBits", ENTER)
        .expect("6");
    step("Count bits set (bignum)")
        .test(CLEAR, "16#147800 75 SLC CountBits", ENTER)
        .expect("6");

    step("Restore word size")
        .test(CLEAR, "'WordSize' PURGE", ENTER).noerror();

    step("Logical with symbolic values")
        .test(CLEAR, "'X' 'Y' AND", ENTER)
        .expect("'X and Y'")
        .test(CLEAR, "2 'Y' OR", ENTER)
        .expect("'2 or Y'")
        .test(CLEAR, "'X' 2 XOR", ENTER)
        .expect("'X xor 2'")
        .test(CLEAR, "'X' NOT", ENTER)
        .expect("'not X'");

    step("Logical with integers")
        .test(CLEAR, "42 7 XOR", ENTER)
        .expect("45")
        .test(CLEAR, "10#42 10#7 XOR", ENTER)
        .expect("#45₁₀")
        .test(CLEAR, "TruthLogicForIntegers", ENTER)
        .test(CLEAR, "42 7 XOR", ENTER)
        .expect("False")
        .test(CLEAR, "10#42 10#7 XOR", ENTER)
        .expect("#45₁₀")
        .test(CLEAR, "'TruthLogicForIntegers' Purge", ENTER).noerror();

    step("Convert True and False to decimal")
        .test(CLEAR, "True",  ENTER, ID_ToDecimal).expect("True")
        .test(CLEAR, "False", ENTER, ID_ToDecimal).expect("False");
}


void tests::command_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for commands
// ----------------------------------------------------------------------------
{
    BEGIN(styles);

    step("Commands");
    // There is a trap in this command line
    cstring prgm =
        "«"
        "  1 1.0 "
        "+ - * / ^ sqrt sq inv neg "
        "sin cos tan asin acos atan "
        "LowerCase PurgeAll Precision "
        "start step next start step for i next for i step "
        "while repeat end do until end » ";

    test(CLEAR, prgm, ENTER).noerror();
    step("Lower case");
    test("lowercase", ENTER)
        .want("« 1 1. + - * / ^ √ sq inv neg sin cos tan asin acos atan "
              "lowercase pgall prec "
              "start  step next start  step for i  next for i  step "
              "while  repeat  end do  until  end »");

    step("Upper case");
    test("UPPERCASE", ENTER)
        .want("« 1 1. + - * / ^ √ SQ INV NEG SIN COS TAN ASIN ACOS ATAN "
              "LOWERCASE PGALL PREC "
              "START  STEP next START  STEP FOR i  NEXT FOR i  STEP "
              "WHILE  REPEAT  END DO  UNTIL  END »");

    step("Capitalized");
    test("Capitalized", ENTER)
        .want("« 1 1. + - * / ^ √ Sq Inv Neg Sin Cos Tan Asin Acos Atan "
              "LowerCase PgAll Prec "
              "Start  Step next Start  Step For i  Next For i  Step "
              "While  Repeat  End Do  Until  End »");

    step("Long form");
    test("LongForm", ENTER)
        .want("« 1 1. + - × ÷ ↑ √ x² x⁻¹ Negate sin cos tan sin⁻¹ cos⁻¹ tan⁻¹ "
              "LowerCaseCommands PurgeAll Precision "
              "start  step next start  step for i  next for i  step "
              "while  repeat  end do  until  end »");
}


void tests::integer_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for integer values
// ----------------------------------------------------------------------------
{
    BEGIN(iformat);

    step("Reset settings to defaults");
    test(CLEAR)
        .test("3 MantissaSpacing", ENTER)       .noerror()
        .test("5 FractionSpacing", ENTER)       .noerror()
        .test("4 BasedSpacing", ENTER)          .noerror()
        .test("NumberSpaces", ENTER)            .noerror()
        .test("BasedSpaces", ENTER)             .noerror();

    step("Default integer rendering");
    test(CLEAR, 1, ENTER)
        .type(ID_integer)
        .expect("1");
    test(CLEAR, 12, ENTER)
        .type(ID_integer)
        .expect("12");
    test(CLEAR, 123, ENTER)
        .type(ID_integer)
        .expect("123");
    test(CLEAR, 1234, ENTER)
        .type(ID_integer)
        .expect("1 234");
    test(CLEAR, 12345, ENTER)
        .type(ID_integer)
        .expect("12 345");
    test(CLEAR, 123456789, ENTER)
        .type(ID_integer)
        .expect("123 456 789");

    step("No spacing");
    test("0 MantissaSpacing", ENTER)
        .expect("123456789");

    step("Four spacing");
    test("4 MantissaSpacing", ENTER)
        .expect("1 2345 6789");

    step("Five spacing");
    test("5 MantissaSpacing", ENTER)
        .expect("1234 56789");

    step("Three spacing");
    test("3 MantissaSpacing 5 FractionSpacing", ENTER)
        .expect("123 456 789");

    step("Comma spacing");
    test("NumberDotOrComma", ENTER)
        .expect("123,456,789");

    step("Dot spacing");
    test("DecimalComma", ENTER)
        .expect("123.456.789");

    step("Ticks spacing");
    test("DecimalDot", ENTER)
        .expect("123,456,789");
    test("NumberTicks", ENTER)
        .expect("123’456’789");

    step("Underscore spacing");
    test("NumberUnderscore", ENTER)
        .expect("123_456_789");

    step("Space spacing");
    test("NumberSpaces", ENTER)
        .expect("123 456 789");

    step("Big integer rendering");
    test(CLEAR, "123456789012345678901234567890", ENTER)
        .type(ID_bignum)
        .expect("123 456 789 012 345 678 901 234 567 890");

    step("Entering numbers with spacing");
    test(CLEAR, "FancyExponent", ENTER).noerror();

    test(CLEAR, "1").editor("1");
    test(CHS).editor("-1");
    test(CHS).editor("1");
    test("2").editor("12");
    test("3").editor("123");
    test("4").editor("1 234");
    test("5").editor("12 345");
    test(CHS).editor("-12 345");
    test(EEX).editor("-12 345⁳");
    test("34").editor("-12 345⁳34");
    test(CHS).editor("-12 345⁳-34");
    test(" ").editor("-12 345⁳-34 ");
    test("12345.45678901234").editor("-12 345⁳-34 12 345.45678 90123 4");
    test(ENTER).noerror();

    step("Based number rendering");
    test(CLEAR, "#1234ABCDEFh", ENTER)
#if CONFIG_FIXED_BASED_OBJECTS
        .type(ID_hex_integer)
#endif // CONFIG_FIXED_BASED_OBJECTS
        .expect("#12 34AB CDEF₁₆");

    step("Two spacing");
    test("2 BasedSpacing", ENTER)
        .expect("#12 34 AB CD EF₁₆");

    step("Three spacing");
    test("3 BasedSpacing", ENTER)
        .expect("#1 234 ABC DEF₁₆");

    step("Four spacing");
    test("4 BasedSpacing", ENTER)
        .expect("#12 34AB CDEF₁₆");

    step("Comma spacing");
    test("BasedDotOrComma", ENTER)
        .expect("#12,34AB,CDEF₁₆");

    step("Dot spacing");
    test("DecimalComma", ENTER)
        .expect("#12.34AB.CDEF₁₆");

    step("Ticks spacing");
    test("DecimalDot", ENTER)
        .expect("#12,34AB,CDEF₁₆");
    test("BasedTicks", ENTER)
        .expect("#12’34AB’CDEF₁₆");

    step("Underscore spacing");
    test("BasedUnderscore", ENTER)
        .expect("#12_34AB_CDEF₁₆");

    step("Space spacing");
    test("BasedSpaces", ENTER)
        .expect("#12 34AB CDEF₁₆");
}


void tests::fraction_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for fraction values
// ----------------------------------------------------------------------------
{
    BEGIN(fformat);

    step("Default format for small fractions (1/3)")
        .test(CLEAR, 1, ENTER, 3, DIV)
        .type(ID_fraction).expect("¹/₃");
    step("Big fraction format")
        .test("BigFractions", ENTER).expect("1/3");
    step("Mixed big fraction")
        .test("MixedFractions", ENTER).expect("1/3");
    step("Small fractions")
        .test("SmallFractions", ENTER).expect("¹/₃");
    step("Improper fractions")
        .test("ImproperFractions", ENTER).expect("¹/₃");

    step("Default format for medium fractions (355/113)")
        .test(CLEAR, 355, ENTER, 113, DIV)
        .type(ID_fraction).expect("³⁵⁵/₁₁₃");
    step("Big fraction format")
        .test("BigFractions", ENTER).expect("355/113");
    step("Mixed big fraction")
        .test("MixedFractions", ENTER).expect("3 16/113");
    step("Small fractions")
        .test("SmallFractions", ENTER).expect("3 ¹⁶/₁₁₃");
    step("Improper fractions")
        .test("ImproperFractions", ENTER).expect("³⁵⁵/₁₁₃");

    step("Default format for large fractions (1000000000/99999999)")
        .test(CLEAR, 1000000000, ENTER, 99999999, DIV)
        .type(ID_fraction).expect("¹ ⁰⁰⁰ ⁰⁰⁰ ⁰⁰⁰/₉₉ ₉₉₉ ₉₉₉");
    step("Big fraction format")
        .test("BigFractions", ENTER).expect("1 000 000 000/99 999 999");
    step("Mixed big fraction")
        .test("MixedFractions", ENTER).expect("10 10/99 999 999");
    step("Small fractions")
        .test("SmallFractions", ENTER).expect("10 ¹⁰/₉₉ ₉₉₉ ₉₉₉");
    step("Improper fractions")
        .test("ImproperFractions", ENTER).expect("¹ ⁰⁰⁰ ⁰⁰⁰ ⁰⁰⁰/₉₉ ₉₉₉ ₉₉₉");
    step("Back to mixed fractions")
        .test("MixedFractions", ENTER).expect("10 ¹⁰/₉₉ ₉₉₉ ₉₉₉");
}


void tests::decimal_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for decimal values
// ----------------------------------------------------------------------------
{
    BEGIN(dformat);

    step("Standard mode");
    test(CLEAR, "STD", ENTER).noerror();

    step("Small number");
    test(CLEAR, "1.03", ENTER)
        .type(ID_decimal)
        .expect("1.03");

    step("Zero with dot is an error")
        .test(CLEAR, ".", ENTER).error("Syntax error").test(EXIT);
    step("Zero as 0. is accepted")
        .test(CLEAR, "0.").editor("0.")
        .test(ENTER).type(ID_decimal).expect("0.");

    // Regression test for bug #726
    step("Showing 0.2");
    test(CLEAR, "0.2", ENTER).type(ID_decimal).expect("0.2");
    step("Showing 0.2 with NoTrailingDecimal (bug #726)");
    test("NoTrailingDecimal", ENTER).type(ID_decimal).expect("0.2");
    step("Showing 0.2 with TrailingDecimal (bug #726)");
    test("TrailingDecimal", ENTER).type(ID_decimal).expect("0.2");

    step("Negative");
    test(CLEAR, "0.3", CHS, ENTER)
        .type(ID_neg_decimal)
        .expect("-0.3");

    step("Scientific entry");
    test(CLEAR, "1", EEX, "2", ENTER)
        .type(ID_decimal)
        .expect("100.");

    step("Scientific entry with negative exponent");
    test(CLEAR, "1", EEX, "2", CHS, ENTER)
        .type(ID_decimal)
        .expect("0.01");

    step("Negative entry with negative exponent");
    test(CLEAR, "1", CHS, EEX, "2", CHS, ENTER)
        .type(ID_neg_decimal)
        .expect("-0.01");

    step("Non-scientific display");
    test(CLEAR, "0.245", ENTER)
        .type(ID_decimal)
        .expect("0.245");
    test(CLEAR, "0.0003", CHS, ENTER)
        .type(ID_neg_decimal)
        .expect("-0.0003");
    test(CLEAR, "123.456", ENTER)
        .type(ID_decimal)
        .expect("123.456");

    step("Formerly selection of decimal64");
    test(CLEAR, "1.2345678", ENTER)
        .type(ID_decimal)
        .expect("1.23456 78");

    step("Formerly selection of decimal64 based on exponent");
    test(CLEAR, "1.23", EEX, 100, ENTER)
        .type(ID_decimal)
        .expect("1.23⁳¹⁰⁰");

    step("Formerly selection of decimal128");
    test(CLEAR, "1.2345678901234567890123", ENTER)
        .type(ID_decimal)
        .expect("1.23456 78901 2");
    step("Selection of decimal128 based on exponent");
    test(CLEAR, "1.23", EEX, 400, ENTER)
        .type(ID_decimal)
        .expect("1.23⁳⁴⁰⁰");

    step("Automatic switching to scientific display");
    test(CLEAR, "1000000000000.", ENTER)
        .expect("1.⁳¹²");
    test(CLEAR, "0.00000000000025", ENTER)
        .expect("2.5⁳⁻¹³");

    step("FIX 4 mode");
    test(CLEAR, "4 FIX", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.0100")
        .test(CHS).expect("-1.0100");
    test(CLEAR, "1.0123", ENTER).expect("1.0123");
    test(CLEAR, "10.12345", ENTER).expect("10.1235");
    test(CLEAR, "101.29995", ENTER).expect("101.3000");
    test(CLEAR, "1999.99999", ENTER).expect("2 000.0000");
    test(CLEAR, "19999999999999.", ENTER).expect("2.0000⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("2.0000⁳⁻¹¹")
        .test(CHS).expect("-2.0000⁳⁻¹¹");

    step("FIX 24 mode");
    test(CLEAR, "24 FIX", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01000 00000 00000 00000 0000");
    test(CLEAR, "1.0123 ln", ENTER)
        .expect("0.01222 49696 22568 97092 2453");

    step("SCI 3 mode");
    test(CLEAR, "3 Sci", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.010⁳⁰")
        .test(CHS).expect("-1.010⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.012⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("1.012⁳¹");
    test(CLEAR, "101.2543", ENTER).expect("1.013⁳²");
    test(CLEAR, "1999.999", ENTER).expect("2.000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("2.000⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("2.000⁳⁻¹¹")
        .test(CHS).expect("-2.000⁳⁻¹¹");

    step("ENG 3 mode");
    test(CLEAR, "3 eng", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.010⁳⁰")
        .test(CHS).expect("-1.010⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.012⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("10.12⁳⁰");
    test(CLEAR, "101.2543", ENTER).expect("101.3⁳⁰");
    test(CLEAR, "1999.999", ENTER).expect("2.000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("20.00⁳¹²");
    test(CLEAR, "0.00000000001999999", ENTER).expect("20.00⁳⁻¹²")
        .test(CHS).expect("-20.00⁳⁻¹²");

    step("SIG 3 mode");
    test(CLEAR, "3 sig", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01")
        .test(CHS).expect("-1.01");
    test(CLEAR, "1.0123", ENTER).expect("1.01");
    test(CLEAR, "10.12345", ENTER).expect("10.1");
    test(CLEAR, "101.2543", ENTER).expect("101.");
    test(CLEAR, "1999.999", ENTER).expect("2 000.");
    test(CLEAR, "19999999999999.", ENTER).expect("2.⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("2.⁳⁻¹¹")
        .test(CHS).expect("-2.⁳⁻¹¹");

    step("SCI 5 mode");
    test(CLEAR, "5 Sci", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01000⁳⁰")
        .test(CHS).expect("-1.01000⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.01230⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("1.01235⁳¹");
    test(CLEAR, "101.2543", ENTER).expect("1.01254⁳²");
    test(CLEAR, "1999.999", ENTER).expect("2.00000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("2.00000⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("2.00000⁳⁻¹¹")
        .test(CHS).expect("-2.00000⁳⁻¹¹");

    step("ENG 5 mode");
    test(CLEAR, "5 eng", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01000⁳⁰")
        .test(CHS).expect("-1.01000⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.01230⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("10.1235⁳⁰");
    test(CLEAR, "101.2543", ENTER).expect("101.254⁳⁰");
    test(CLEAR, "1999.999", ENTER).expect("2.00000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("20.0000⁳¹²");
    test(CLEAR, "0.00000000001999999", ENTER).expect("20.0000⁳⁻¹²")
        .test(CHS).expect("-20.0000⁳⁻¹²");

    step("SIG 5 mode");
    test(CLEAR, "5 sig", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01")
        .test(CHS).expect("-1.01");
    test(CLEAR, "1.0123", ENTER).expect("1.0123");
    test(CLEAR, "10.12345", ENTER).expect("10.123");
    test(CLEAR, "101.2543", ENTER).expect("101.25");
    test(CLEAR, "1999.999", ENTER).expect("2 000.");
    test(CLEAR, "19999999999999.", ENTER).expect("2.⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("2.⁳⁻¹¹")
        .test(CHS).expect("-2.⁳⁻¹¹");

    step("SCI 13 mode");
    test(CLEAR, "13 Sci", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01000 00000 000⁳⁰")
        .test(CHS).expect("-1.01000 00000 000⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.01230 00000 000⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("1.01234 50000 000⁳¹");
    test(CLEAR, "101.2543", ENTER).expect("1.01254 30000 000⁳²");
    test(CLEAR, "1999.999", ENTER).expect("1.99999 90000 000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("1.99999 99999 999⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("1.99999 90000 000⁳⁻¹¹")
        .test(CHS).expect("-1.99999 90000 000⁳⁻¹¹");

    step("ENG 13 mode");
    test(CLEAR, "13 eng", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01000 00000 000⁳⁰")
        .test(CHS).expect("-1.01000 00000 000⁳⁰");
    test(CLEAR, "1.0123", ENTER).expect("1.01230 00000 000⁳⁰");
    test(CLEAR, "10.12345", ENTER).expect("10.12345 00000 00⁳⁰");
    test(CLEAR, "101.2543", ENTER).expect("101.25430 00000 0⁳⁰");
    test(CLEAR, "1999.999", ENTER).expect("1.99999 90000 000⁳³");
    test(CLEAR, "19999999999999.", ENTER).expect("19.99999 99999 99⁳¹²");
    test(CLEAR, "0.00000000001999999", ENTER).expect("19.99999 00000 00⁳⁻¹²")
        .test(CHS).expect("-19.99999 00000 00⁳⁻¹²");

    step("SIG 13 mode");
    test(CLEAR, "13 sig", ENTER).noerror();
    test(CLEAR, "1.01", ENTER).expect("1.01")
        .test(CHS).expect("-1.01");
    test(CLEAR, "1.0123", ENTER).expect("1.0123");
    test(CLEAR, "10.12345", ENTER).expect("10.12345");
    test(CLEAR, "101.2543", ENTER).expect("101.2543");
    test(CLEAR, "1999.999", ENTER).expect("1 999.999");
    test(CLEAR, "19999999999999.", ENTER).expect("2.⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("1.99999 9⁳⁻¹¹")
        .test(CHS).expect("-1.99999 9⁳⁻¹¹");

    step("FIX 4 in HP48-compatible mode")
        .test(CLEAR, "4", LSHIFT, O, F2, "0", LSHIFT, F5).noerror()
        .test("0.635", ENTER).expect("0.6350")
        .test("10", DIV).expect("0.0635")
        .test("10", DIV).expect("0.0064")
        .test("10", DIV).expect("0.0006")
        .test("10", DIV).expect("0.0001")
        .test("10", DIV).expect("6.3500⁳⁻⁶")
        .test("10", DIV).expect("6.3500⁳⁻⁷");

    step("FIX 4 showing 2 significant digits")
        .test(CLEAR, "2", LSHIFT, F5).noerror()
        .test("0.635", ENTER).expect("0.6350")
        .test("10", DIV).expect("0.0635")
        .test("10", DIV).expect("0.0064")
        .test("10", DIV).expect("6.3500⁳⁻⁴")
        .test("10", DIV).expect("6.3500⁳⁻⁵")
        .test("10", DIV).expect("6.3500⁳⁻⁶")
        .test("10", DIV).expect("6.3500⁳⁻⁷");

    step("FIX 4 showing 12 significant digits")
        .test(CLEAR, "12", LSHIFT, F5).noerror()
        .test("0.635", ENTER).expect("0.6350")
        .test("10", DIV).expect("0.0635")
        .test("10", DIV).expect("6.3500⁳⁻³")
        .test("10", DIV).expect("6.3500⁳⁻⁴")
        .test("10", DIV).expect("6.3500⁳⁻⁵")
        .test("10", DIV).expect("6.3500⁳⁻⁶")
        .test("10", DIV).expect("6.3500⁳⁻⁷");

    step("FIX 4 in old HP style (showing 0.0000)")
        .test(CLEAR, "-1", LSHIFT, F5).noerror()
        .test("0.635", ENTER).expect("0.6350")
        .test("10", DIV).expect("0.0635")
        .test("10", DIV).expect("0.0064")
        .test("10", DIV).expect("0.0006")
        .test("10", DIV).expect("0.0001")
        .test("10", DIV).expect("0.0000")
        .test("10", DIV).expect("0.0000");

    step("FIX 4 with no trailing decimal and 500 (#1236)")
        .test(CLEAR, "NoTrailingDecimals 500.", ENTER).expect("500.0000")
        .test(CLEAR, "TrailingDecimals 500.", ENTER).expect("500.0000");

    step("Reset defaults");
    test(CLEAR, LSHIFT, O, F1, KEY3, LSHIFT, F5).noerror();

    step("Test display of 5000.");
    test(CLEAR, "5000.", ENTER)        .expect("5 000.");
    test(CLEAR, "50000.", ENTER)       .expect("50 000.");
    test(CLEAR, "500000.", ENTER)      .expect("500 000.");
    test(CLEAR, "5000000.", ENTER)     .expect("5 000 000.");

    step("Test display of very large exponents")
        .test(CLEAR, "1E1234567890123456", ENTER,
              ID_DisplayModesMenu, ID_SeparatorModesMenu)
        .expect("1.⁳¹²³⁴⁵⁶⁷⁸⁹⁰¹²³⁴⁵⁶")
        .test(ID_ClassicExponent)
        .expect("1.E1234567890123456")
        .test(ID_FancyExponent);

    step("Display integers as decimal")
        .test(CLEAR, "123", ENTER)
        .expect("123")
        .test(ID_DisplayModesMenu, ID_SeparatorModesMenu, ID_ShowAsDecimal)
        .expect("123.")
        .test("2 FIX", ENTER)
        .expect("123.00")
        .test(ID_ShowAsDecimal)
        .expect("123");;

    step("Display fractions as decimal")
        .test(CLEAR, "5/2", ENTER)
        .expect("2 ¹/₂")
        .test(ID_DisplayModesMenu, ID_SeparatorModesMenu, ID_ShowAsDecimal)
        .expect("2.50")
        .test("2 FIX", ENTER)
        .expect("2.50")
        .test(ID_ShowAsDecimal).
        expect("2 ¹/₂");
}


void tests::integer_numerical_functions()
// ----------------------------------------------------------------------------
//   Test integer numerical functions
// ----------------------------------------------------------------------------
{
    BEGIN(ifunctions);

    step("neg")
        .test(CLEAR, "3 neg", ENTER).expect("-3")
        .test("negate", ENTER).expect("3");
    step("inv")
        .test(CLEAR, "3 inv", ENTER).expect("¹/₃")
        .test("inv", ENTER).expect("3")
        .test(CLEAR, "-3 inv", ENTER).expect("-¹/₃")
        .test("inv", ENTER).expect("-3");
    step("sq (square)")
        .test(CLEAR, "-3 sq", ENTER).expect("9")
        .test("sq", ENTER).expect("81");
    step("cubed")
        .test(CLEAR, "3 cubed", ENTER).expect("27")
        .test("cubed", ENTER).expect("19 683")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19 683");
    step("abs")
        .test(CLEAR, "-3 abs", ENTER).expect("3")
        .test("abs", ENTER, 1, ADD).expect("4");
    step("norm").test("-5 norm", ENTER).expect("5");
}


void tests::decimal_numerical_functions()
// ----------------------------------------------------------------------------
//   Test decimal numerical functions
// ----------------------------------------------------------------------------
{
    BEGIN(dfunctions);

    step("Select 34-digit precision to match Intel Decimal 128");
    test(CLEAR,
         "34 PRECISION 64 SIG RAD "
         "0 MantissaSpacing 0 FractionSpacing", ENTER).noerror();

    step("Addition")
        .test(CLEAR, "1.23 2.34", ID_add).expect("3.57")
        .test(CLEAR, "1.23 -2.34", ID_add).expect("-1.11")
        .test(CLEAR, "-1.23 2.34", ID_add).expect("1.11")
        .test(CLEAR, "-1.23 -2.34", ID_add).expect("-3.57")
        .test(CLEAR, "1.234 SIN 2.34", ID_add).expect("3.2838182093746337048617510061568276")
        .test(CLEAR, "1.23 COS -2.34", ID_add).expect("-2.0057622728754974017604527545023355")
        .test(CLEAR, "-1.23 TAN 2.34", ID_add).expect("-0.479815734268151974808881834909673")
        .test(CLEAR, "-1.23 TANH -2.34", ID_add).expect("-3.1825793256589295428907208915016509");
    step("Subtraction")
        .test(CLEAR, "1.23 2.34", ID_subtract).expect("-1.11")
        .test(CLEAR, "1.23 -2.34", ID_subtract).expect("3.57")
        .test(CLEAR, "-1.23 2.34", ID_subtract).expect("-3.57")
        .test(CLEAR, "-1.23 -2.34", ID_subtract).expect("1.11")
        .test(CLEAR, "1.234 SIN 2.34", ID_subtract).expect("-1.3961817906253662951382489938431724")
        .test(CLEAR, "1.23 COS -2.34", ID_subtract).expect("2.6742377271245025982395472454976645")
        .test(CLEAR, "-1.23 TAN 2.34", ID_subtract).expect("-5.159815734268151974808881834909673")
        .test(CLEAR, "-1.23 TANH -2.34", ID_subtract).expect("1.4974206743410704571092791084983491");
    step("Multiplication")
        .test(CLEAR, "1.23 2.34", ID_multiply).expect("2.8782")
        .test(CLEAR, "1.23 -2.34", ID_multiply).expect("-2.8782")
        .test(CLEAR, "-1.23 2.34", ID_multiply).expect("-2.8782")
        .test(CLEAR, "-1.23 -2.34", ID_multiply).expect("2.8782")
        .test(CLEAR, "1.234 SIN 2.34", ID_multiply).expect("2.20853460993664286937649735440697658")
        .test(CLEAR, "1.23 COS -2.34", ID_multiply).expect("-0.78211628147133607988054055446453493")
        .test(CLEAR, "-1.23 TAN 2.34", ID_multiply).expect("-6.5983688181874756210527834936886348")
        .test(CLEAR, "-1.23 TANH -2.34", ID_multiply).expect("1.97163562204189513036428688611386311");
    step("Division")
        .test(CLEAR, "1.23 2.34", ID_divide).expect("0.525641025641025641025641025641025641")
        .test(CLEAR, "1.23 -2.34", ID_divide).expect("-0.525641025641025641025641025641025641")
        .test(CLEAR, "-1.23 2.34", ID_divide).expect("-0.525641025641025641025641025641025641")
        .test(CLEAR, "-1.23 -2.34", ID_divide).expect("0.525641025641025641025641025641025641")
        .test(CLEAR, "1.234 SIN 2.34", ID_divide).expect("0.403341115117364831137500429981550256")
        .test(CLEAR, "1.23 COS -2.34", ID_divide).expect("-0.142836635523291708649379164742591666")
        .test(CLEAR, "-1.23 TAN 2.34", ID_divide).expect("-1.20504945908895383538841104055968931")
        .test(CLEAR, "-1.23 TANH -2.34", ID_divide).expect("0.360076634896978437132786705769936282");
    step("Power")
        .test(CLEAR, "1.23 2.34", ID_pow).expect("1.623222151685370761702177674374041")
        .test(CLEAR, "1.23 -2.34", ID_pow).expect("0.6160586207881113580350956467249859")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.9008215014432917465348578887507")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 -23", ID_pow).expect("-0.00855425981747830324192990196438470826")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "1.234 SIN 2.34", ID_pow).expect("0.8734513971114369515506870445407018")
        .test(CLEAR, "1.23 COS -2.34", ID_pow).expect("12.99302283398205639426875012788037")
        .test(CLEAR,
              "-1.23 TAN 23", ID_pow,
              "-22650447100.367345363211380882677399527350835552", ID_subtract)
        .expect("-6.05⁳⁻²³")
        .test(CLEAR, "-1.23 TAN 2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 TAN -23", ID_pow).expect("-4.41492388900253532657391833311441433⁳⁻¹¹")
        .test(CLEAR, "-1.23 TANH -2.34", ID_pow).error("Argument outside domain");

    step("Square root of 2")
        .test(CLEAR, "2 sqrt", ENTER)
        .expect("1.414213562373095048801688724209698");
    step("Square root of 3")
        .test(CLEAR, "3 sqrt", ENTER)
        .expect("1.732050807568877293527446341505872");
    step("Square root of 4")
        .test(CLEAR, "4 sqrt", ENTER)
        .expect("2.");
    step("Cube root of 2")
        .test(CLEAR, "2 cbrt", ENTER)
        .expect("1.259921049894873164767210607278228");
    step("Cube root of 3")
        .test(CLEAR, "3 cbrt", ENTER)
        .expect("1.44224957030740838232163831078011");
    step("Cube root of 27")
        .test(CLEAR, "27 cbrt", ENTER)
        .expect("3.");

    step("neg")
        .test(CLEAR, "3.21 neg", ENTER).expect("-3.21")
        .test("negate", ENTER).expect("3.21");
    step("inv")
        .test(CLEAR, "3.21 inv", ENTER)
        .expect("0.311526479750778816199376947040498442")
        .test("inv", ENTER).expect("3.21");
    step("sq (square)")
        .test(CLEAR, "-3.21 sq", ENTER).expect("10.3041")
        .test("sq", ENTER).expect("106.17447681");
    step("cubed")
        .test(CLEAR, "3.21 cubed", ENTER).expect("33.076161")
        .test("cubed", ENTER).expect("36186.392678065901161281")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19683");
    step("abs")
        .test(CLEAR, "-3.21 abs", ENTER).expect("3.21")
        .test("abs", ENTER, 1, ADD).expect("4.21");

    step("Setting radians mode");
    test(CLEAR, "RAD", ENTER).noerror();

#define TFNA(name, arg)                                         \
    step(#name).test(CLEAR, #arg " " #name, ENTER)
#define TFN(name)  TFNA(name, 0.321)

    TFN(sqrt).expect("0.566568618968611779925473404696769");
    TFN(sin).expect("0.3155156385927271113065931111434637");
    TFN(cos).expect("0.9489203769565830175439451328269255");
    TFN(tan).expect("0.332499592436471875108708730102738");
    TFN(asin).expect("0.3267851765314954632691997645195983 r");
    TFN(acos).expect("1.244011150263401155962121927120153 r");
    TFN(atan).expect("0.310609792813889917606700051446836 r");
    TFN(sinh).expect("0.3265411649518063570122065638857343");
    TFN(cosh).expect("1.051964415941947538435224143605678");
    TFN(tanh).expect("0.3104108466058860214850502093830959");
    TFN(asinh).expect("0.3157282658293796179108945471020638");
    TFNA(acosh, 1.321).expect("0.7812302051962526147422171616034349");
    TFN(atanh).expect("0.3327615884818145958017641705087511");
    TFN(ln1p).expect("0.278389025540188266771628342111551");
    TFN(lnp1).expect("0.278389025540188266771628342111551");
    TFN(expm1).expect("0.3785055808937538954474307074914123");
    TFN(ln).expect("-1.13631415585212118735433031010729");
    TFN(log10).expect("-0.493494967595127921870430857283449");
    TFN(exp).expect("1.378505580893753895447430707491412");
    TFN(exp10).expect("2.094112455850892670519881985846254");
    TFN(exp2).expect("1.249196125653376700521466782085807");
    TFN(erf).expect("0.3501442208200238235516032450502391");
    TFN(erfc).expect("0.6498557791799761764483967549497609");
    TFN(tgamma).expect("2.786634540845472367950764212781773");
    TFN(lgamma).expect("1.024834609957313198691092753834887");
    TFN(gamma).expect("2.786634540845472367950764212781773");
    TFN(cbrt).expect("0.684702127757224161840927732646815");
    TFN(norm).expect("0.321");
#undef TFN
#undef TFNA

    step("pow")
        .test(CLEAR, "3.21 1.23 pow", ENTER)
        .expect("4.197601340269557031334155704388712")
        .test(CLEAR, "1.23 2.31", ID_pow)
        .expect("1.613172490755543844341414892337986");

    step("hypot")
        .test(CLEAR, "3.21 1.23 hypot", ENTER)
        .expect("3.437586362551492319961655732945235");

    step("atan2 pos / pos quadrant")
        .test(CLEAR, "3.21 1.23 atan2", ENTER)
        .expect("1.204875625152809234008669105495307 r");
    step("atan2 pos / neg quadrant")
        .test(CLEAR, "3.21 -1.23 atan2", ENTER)
        .expect("1.936717028436984004453974277784196 r");
    step("atan2 neg / pos quadrant")
        .test(CLEAR, "-3.21 1.23 atan2", ENTER)
        .expect("-1.204875625152809234008669105495307 r");
    step("atan2 neg / neg quadrant")
        .test(CLEAR, "-3.21 -1.23 atan2", ENTER)
        .expect("-1.936717028436984004453974277784196 r");

    step("ln for very small value")
        .test(CLEAR, "1E-100 LN", ENTER)
        .expect("-230.2585092994045684017991454684364");

    step("Restore default 24-digit precision");
    test(CLEAR,
         "STD DEG { PRECISION MantissaSpacing FractionSpacing } PURGE", ENTER)
        .noerror();

    step("→Frac should work for integers")
        .test(CLEAR, "0 →Frac", ENTER).noerror().expect("0")
        .test(CLEAR, "1 →Frac", ENTER).noerror().expect("1")
        .test(CLEAR, "-123 →Frac", ENTER).noerror().expect("-123");

    step("Assignment with functions")
        .test(CLEAR, "x=3 TAN", ENTER).noerror().expect("0.05240 77792 83")
        .test(CLEAR, "'x' PURGE", ENTER);

    step("xpon function")
        .test(ID_PartsMenu)
        .test(CLEAR, "12.345E37", ID_xpon).expect("38");
    step("mant function")
        .test(CLEAR, "12.345E37", ID_mant).expect("1.2345");
    step("SigDig function")
        .test(CLEAR, "0", ID_SigDig).expect("0")
        .test(CLEAR, "1", ID_SigDig).expect("1")
        .test(CLEAR, "12", ID_SigDig).expect("2")
        .test(CLEAR, "1.23", ID_SigDig).expect("3")
        .test(CLEAR, "1.234", ID_SigDig).expect("4")
        .test(CLEAR, "1.2345", ID_SigDig).expect("5")
        .test(CLEAR, "123456", ID_SigDig).expect("6")
        .test(CLEAR, "123.4567", ID_SigDig).expect("7");
    step("xpon function with unit")
        .test(CLEAR, "12.345E37_cm XPON", ENTER).expect("38");
    step("mant function with unit")
        .test(CLEAR, "12.345E37_cm MANT", ENTER).expect("1.2345");
    step("SigDig function")
        .test(CLEAR, "12.345E7_nm SIGDIG", ENTER).expect("5");
}


void tests::float_numerical_functions()
// ----------------------------------------------------------------------------
//   Test hardware-accelerated numerical functions
// ----------------------------------------------------------------------------
{
    BEGIN(float);

    step("Direct data entry of float value")
        .test(CLEAR, "1.2F", ENTER).noerror().expect("1.20000 00476 8F");
    step("Select float acceleration")
        .test(CLEAR, "7 PRECISION 10 SIG HardFP", ENTER).noerror();
    step("Data entry is in decimal")
        .test(CLEAR, "1.2", ENTER).noerror().expect("1.2");
    step("Computation results in inexact binary representation")
        .test(CLEAR, "1.2 2 * 2 /", ENTER).noerror().expect("1.20000 0048F");
    step("Binary representation does not align with decimal")
        .test(CLEAR, "1.2F", ENTER).noerror().expect("1.20000 0048F");
    step("Select 6-digit precision and Radians for output stability")
        .test("6 SIG RAD", ENTER)
        .noerror();

    step("Addition")
        .test(CLEAR, "1.23 2.34", ID_add).expect("3.57F")
        .test(CLEAR, "1.23 -2.34", ID_add).expect("-1.11F")
        .test(CLEAR, "-1.23 2.34", ID_add).expect("1.11F")
        .test(CLEAR, "-1.23 -2.34", ID_add).expect("-3.57F")
        .test(CLEAR, "1.234 SIN 2.34", ID_add).expect("3.28382F")
        .test(CLEAR, "1.23 COS -2.34", ID_add).expect("-2.00576F")
        .test(CLEAR, "-1.23 TAN 2.34", ID_add).expect("-0.47981 6F")
        .test(CLEAR, "-1.23 TANH -2.34", ID_add).expect("-3.18258F");
    step("Subtraction")
        .test(CLEAR, "1.23 2.34", ID_subtract).expect("-1.11F")
        .test(CLEAR, "1.23 -2.34", ID_subtract).expect("3.57F")
        .test(CLEAR, "-1.23 2.34", ID_subtract).expect("-3.57F")
        .test(CLEAR, "-1.23 -2.34", ID_subtract).expect("1.11F")
        .test(CLEAR, "1.234 SIN 2.34", ID_subtract).expect("-1.39618F")
        .test(CLEAR, "1.23 COS -2.34", ID_subtract).expect("2.67424F")
        .test(CLEAR, "-1.23 TAN 2.34", ID_subtract).expect("-5.15982F")
        .test(CLEAR, "-1.23 TANH -2.34", ID_subtract).expect("1.49742F");
    step("Multiplication")
        .test(CLEAR, "1.23 2.34", ID_multiply).expect("2.8782F")
        .test(CLEAR, "1.23 -2.34", ID_multiply).expect("-2.8782F")
        .test(CLEAR, "-1.23 2.34", ID_multiply).expect("-2.8782F")
        .test(CLEAR, "-1.23 -2.34", ID_multiply).expect("2.8782F")
        .test(CLEAR, "1.234 SIN 2.34", ID_multiply).expect("2.20853F")
        .test(CLEAR, "1.23 COS -2.34", ID_multiply).expect("-0.78211 6F")
        .test(CLEAR, "-1.23 TAN 2.34", ID_multiply).expect("-6.59837F")
        .test(CLEAR, "-1.23 TANH -2.34", ID_multiply).expect("1.97164F");
    step("Division")
        .test(CLEAR, "1.23 2.34", ID_divide).expect("0.52564 1F")
        .test(CLEAR, "1.23 -2.34", ID_divide).expect("-0.52564 1F")
        .test(CLEAR, "-1.23 2.34", ID_divide).expect("-0.52564 1F")
        .test(CLEAR, "-1.23 -2.34", ID_divide).expect("0.52564 1F")
        .test(CLEAR, "1.234 SIN 2.34", ID_divide).expect("0.40334 1F")
        .test(CLEAR, "1.23 COS -2.34", ID_divide).expect("-0.14283 7F")
        .test(CLEAR, "-1.23 TAN 2.34", ID_divide).expect("-1.20505F")
        .test(CLEAR, "-1.23 TANH -2.34", ID_divide).expect("0.36007 7F");
    step("Power")
        .test(CLEAR, "1.23 2.34", ID_pow).expect("1.62322F")
        .test(CLEAR, "1.23 -2.34", ID_pow).expect("0.61605 9F")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.901F")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.901F")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "1.234 SIN 2.34", ID_pow).expect("0.87345 1F")
        .test(CLEAR, "1.23 COS -2.34", ID_pow).expect("12.993F")
        .test(CLEAR, "-1.23 TAN 23", ID_pow).expect("-2.26505⁳¹⁰F")
        .test(CLEAR, "-1.23 TAN 2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 TAN -23", ID_pow).expect("-4.41492⁳⁻¹¹F")
        .test(CLEAR, "-1.23 TANH -2.34", ID_pow).error("Argument outside domain");

    step("Square root of 2")
        .test(CLEAR, "2 sqrt", ENTER)
        .expect("1.41421F");
    step("Square root of 3")
        .test(CLEAR, "3 sqrt", ENTER)
        .expect("1.73205F");
    step("Square root of 4")
        .test(CLEAR, "4 sqrt", ENTER)
        .expect("2.F");
    step("Cube root of 2")
        .test(CLEAR, "2 cbrt", ENTER)
        .expect("1.25992F");
    step("Cube root of 3")
        .test(CLEAR, "3 cbrt", ENTER)
        .expect("1.44225F");
    step("Cube root of 27")
        .test(CLEAR, "27 cbrt", ENTER)
        .expect("3.F");

    step("neg")
        .test(CLEAR, "3.21 neg", ENTER).expect("-3.21")
        .test("negate", ENTER).expect("3.21");
    step("inv")
        .test(CLEAR, "3.21 inv", ENTER)
        .expect("0.31152 6")
        .test("inv", ENTER).expect("3.21");
    step("sq (square)")
        .test(CLEAR, "-3.21 sq", ENTER).expect("10.3041F")
        .test("sq", ENTER).expect("106.174F");
    step("cubed")
        .test(CLEAR, "3.21 cubed", ENTER).expect("33.0762F")
        .test("cubed", ENTER).expect("36 186.4F")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19 683");
    step("abs")
        .test(CLEAR, "-3.21 abs", ENTER).expect("3.21")
        .test("abs", ENTER, 1, ADD).expect("4.21F");

    step("Setting radians mode");
    test(CLEAR, "RAD", ENTER).noerror();

#define TFNA(name, arg)         step(#name).test(CLEAR, #arg " " #name, ENTER)
#define TFN(name)               TFNA(name, 0.321)

    TFN(sqrt).expect("0.56656 9F");
    TFN(sin).expect("0.31551 6F");
    TFN(cos).expect("0.94892F");
    TFN(tan).expect("0.3325F");
    TFN(asin).expect("0.32678 5F r");
    TFN(acos).expect("1.24401F r");
    TFN(atan).expect("0.31061F r");
    TFN(sinh).expect("0.32654 1F");
    TFN(cosh).expect("1.05196F");
    TFN(tanh).expect("0.31041 1F");
    TFN(asinh).expect("0.31572 8F");
    TFNA(acosh, 1.321).expect("0.78123F");
    TFN(atanh).expect("0.33276 2F");
    TFN(ln1p).expect("0.27838 9F");
    TFN(lnp1).expect("0.27838 9F");
    TFN(expm1).expect("0.37850 6F");
    TFN(ln).expect("-1.13631F");
    TFN(log10).expect("-0.49349 5F");
    TFN(exp).expect("1.37851F");
    TFN(exp10).expect("2.09411F");
    TFN(exp2).expect("1.2492F");
    TFN(erf).expect("0.35014 4F");
    TFN(erfc).expect("0.64985 6F");
    TFN(tgamma).expect("2.78663F");
    TFN(lgamma).expect("1.02483F");
    TFN(gamma).expect("2.78663F");
    TFN(cbrt).expect("0.68470 2F");
    TFN(norm).expect("0.321");
#undef TFN
#undef TFNA

    step("pow")
        .test(CLEAR, "3.21 1.23 pow", ENTER)
        .expect("4.1976F")
        .test(CLEAR, "1.23 2.31", ID_pow)
        .expect("1.61317F");

    step("hypot")
        .test(CLEAR, "3.21 1.23 hypot", ENTER)
        .expect("3.43759F");

    step("atan2 pos / pos quadrant")
        .test(CLEAR, "3.21 1.23 atan2", ENTER)
        .expect("1.20488F r");
    step("atan2 pos / neg quadrant")
        .test(CLEAR, "3.21 -1.23 atan2", ENTER)
        .expect("1.93672F r");
    step("atan2 neg / pos quadrant")
        .test(CLEAR, "-3.21 1.23 atan2", ENTER)
        .expect("-1.20488F r");
    step("atan2 neg / neg quadrant")
        .test(CLEAR, "-3.21 -1.23 atan2", ENTER)
        .expect("-1.93672F r");

    step("Check integer rounding in hardware FP mode (#1309)")
        .test(CLEAR, "{ 3 3 } RANM", ENTER)
        .type(ID_array);

    step("Check hwfp overflow (#1317)")
        .test(CLEAR, "1E80", ID_sq)
        .expect("∞");
    step("Check hwfp overflow (#1317)")
        .test(CLEAR, "OverflowError", ENTER).noerror()
        .test("1E80", ID_sq)
        .error("Numerical overflow")
        .test(CLEAR, "'OverflowError' Purge", ENTER).noerror();

    step("Check rounding to fraction(#1481)")
        .test(CLEAR, DIRECT("10.25F ToFraction"), ENTER)
        .expect("10 ¹/₄");

    step("Restore default 24-digit precision");
    test(CLEAR, "24 PRECISION 12 SIG SoftFP", ENTER).noerror();
}


void tests::double_numerical_functions()
// ----------------------------------------------------------------------------
//   Test hardware-accelerated numerical functions
// ----------------------------------------------------------------------------
{
    BEGIN(double);

    step("Direct data entry of double value")
        .test(CLEAR, "1.2D", ENTER).noerror().expect("1.2D");
    step("Select double-precision witih hardware acceleration")
        .test(CLEAR, "16 PRECISION 24 SIG HardFP", ENTER).noerror();
    step("Binary representation does not align with decimal")
        .test(CLEAR, "1.2 2 * 2 /", ENTER).noerror()
        .expect("1.19999 99999 99999 96D");
    step("Select 15-digit precision for output stability")
        .test("15 SIG RAD", ENTER).noerror();

    step("Addition")
        .test(CLEAR, "1.23 2.34", ID_add).expect("3.57D")
        .test(CLEAR, "1.23 -2.34", ID_add).expect("-1.11D")
        .test(CLEAR, "-1.23 2.34", ID_add).expect("1.11D")
        .test(CLEAR, "-1.23 -2.34", ID_add).expect("-3.57D")
        .test(CLEAR, "1.234 SIN 2.34", ID_add).expect("3.28381 82093 7463D")
        .test(CLEAR, "1.23 COS -2.34", ID_add).expect("-2.00576 22728 755D")
        .test(CLEAR, "-1.23 TAN 2.34", ID_add).expect("-0.47981 57342 68152D")
        .test(CLEAR, "-1.23 TANH -2.34", ID_add).expect("-3.18257 93256 5893D");
    step("Subtraction")
        .test(CLEAR, "1.23 2.34", ID_subtract).expect("-1.11D")
        .test(CLEAR, "1.23 -2.34", ID_subtract).expect("3.57D")
        .test(CLEAR, "-1.23 2.34", ID_subtract).expect("-3.57D")
        .test(CLEAR, "-1.23 -2.34", ID_subtract).expect("1.11D")
        .test(CLEAR, "1.234 SIN 2.34", ID_subtract).expect("-1.39618 17906 2537D")
        .test(CLEAR, "1.23 COS -2.34", ID_subtract).expect("2.67423 77271 245D")
        .test(CLEAR, "-1.23 TAN 2.34", ID_subtract).expect("-5.15981 57342 6815D")
        .test(CLEAR, "-1.23 TANH -2.34", ID_subtract).expect("1.49742 06743 4107D");
    step("Multiplication")
        .test(CLEAR, "1.23 2.34", ID_multiply).expect("2.8782D")
        .test(CLEAR, "1.23 -2.34", ID_multiply).expect("-2.8782D")
        .test(CLEAR, "-1.23 2.34", ID_multiply).expect("-2.8782D")
        .test(CLEAR, "-1.23 -2.34", ID_multiply).expect("2.8782D")
        .test(CLEAR, "1.234 SIN 2.34", ID_multiply).expect("2.20853 46099 3664D")
        .test(CLEAR, "1.23 COS -2.34", ID_multiply).expect("-0.78211 62814 71336D")
        .test(CLEAR, "-1.23 TAN 2.34", ID_multiply).expect("-6.59836 88181 8747D")
        .test(CLEAR, "-1.23 TANH -2.34", ID_multiply).expect("1.97163 56220 419D");
    step("Division")
        .test(CLEAR, "1.23 2.34", ID_divide).expect("0.52564 10256 41026D")
        .test(CLEAR, "1.23 -2.34", ID_divide).expect("-0.52564 10256 41026D")
        .test(CLEAR, "-1.23 2.34", ID_divide).expect("-0.52564 10256 41026D")
        .test(CLEAR, "-1.23 -2.34", ID_divide).expect("0.52564 10256 41026D")
        .test(CLEAR, "1.234 SIN 2.34", ID_divide).expect("0.40334 11151 17365D")
        .test(CLEAR, "1.23 COS -2.34", ID_divide).expect("-0.14283 66355 23292D")
        .test(CLEAR, "-1.23 TAN 2.34", ID_divide).expect("-1.20504 94590 8895D")
        .test(CLEAR, "-1.23 TANH -2.34", ID_divide).expect("0.36007 66348 96978D");
    step("Power")
        .test(CLEAR, "1.23 2.34", ID_pow).expect("1.62322 21516 8537D")
        .test(CLEAR, "1.23 -2.34", ID_pow).expect("0.61605 86207 88111D")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.90082 15014 43D")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.90082 15014 43D")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "1.234 SIN 2.34", ID_pow).expect("0.87345 13971 11437D")
        .test(CLEAR, "1.23 COS -2.34", ID_pow).expect("12.99302 28339 821D")
        .test(CLEAR, "-1.23 TAN 23", ID_pow).expect("-2.26504 47100 3673⁳¹⁰D")
        .test(CLEAR, "-1.23 TAN 2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 TAN -23", ID_pow).expect("-4.41492 38890 0254⁳⁻¹¹D")
        .test(CLEAR, "-1.23 TANH -2.34", ID_pow).error("Argument outside domain");

    step("Square root of 2")
        .test(CLEAR, "2 sqrt", ENTER)
        .expect("1.41421 35623 731D");
    step("Square root of 3")
        .test(CLEAR, "3 sqrt", ENTER)
        .expect("1.73205 08075 6888D");
    step("Square root of 4")
        .test(CLEAR, "4 sqrt", ENTER)
        .expect("2.D");
    step("Cube root of 2")
        .test(CLEAR, "2 cbrt", ENTER)
        .expect("1.25992 10498 9487D");
    step("Cube root of 3")
        .test(CLEAR, "3 cbrt", ENTER)
        .expect("1.44224 95703 0741D");
    step("Cube root of 27")
        .test(CLEAR, "27 cbrt", ENTER)
        .expect("3.D");

    step("neg")
        .test(CLEAR, "3.21 neg", ENTER).expect("-3.21")
        .test("negate", ENTER).expect("3.21");
    step("inv")
        .test(CLEAR, "3.21 inv", ENTER)
        .expect("0.31152 64797 50779")
        .test("inv", ENTER).expect("3.21");
    step("sq (square)")
        .test(CLEAR, "-3.21 sq", ENTER).expect("10.3041D")
        .test("sq", ENTER).expect("106.17447 681D");
    step("cubed")
        .test(CLEAR, "3.21 cubed", ENTER).expect("33.07616 1D")
        .test("cubed", ENTER).expect("36 186.39267 80659D")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19 683");
    step("abs")
        .test(CLEAR, "-3.21 abs", ENTER).expect("3.21")
        .test("abs", ENTER, 1, ADD).expect("4.21D");

    step("Setting radians mode");
    test(CLEAR, "RAD", ENTER).noerror();

#define TFNA(name, arg)         step(#name).test(CLEAR, #arg " " #name, ENTER)
#define TFN(name)               TFNA(name, 0.321)

    TFN(sqrt).expect("0.56656 86189 68612D");
    TFN(sin).expect("0.31551 56385 92727D");
    TFN(cos).expect("0.94892 03769 56583D");
    TFN(tan).expect("0.33249 95924 36472D");
    TFN(asin).expect("0.32678 51765 31495D r");
    TFN(acos).expect("1.24401 11502 634D r");
    TFN(atan).expect("0.31060 97928 1389D r");
    TFN(sinh).expect("0.32654 11649 51806D");
    TFN(cosh).expect("1.05196 44159 4195D");
    TFN(tanh).expect("0.31041 08466 05886D");
    TFN(asinh).expect("0.31572 82658 2938D");
    TFNA(acosh, 1.321).expect("0.78123 02051 96253D");
    TFN(atanh).expect("0.33276 15884 81815D");
    TFN(ln1p).expect("0.27838 90255 40188D");
    TFN(lnp1).expect("0.27838 90255 40188D");
    TFN(expm1).expect("0.37850 55808 93754D");
    TFN(ln).expect("-1.13631 41558 5212D");
    TFN(log10).expect("-0.49349 49675 95128D");
    TFN(exp).expect("1.37850 55808 9375D");
    TFN(exp10).expect("2.09411 24558 5089D");
    TFN(exp2).expect("1.24919 61256 5338D");
    TFN(erf).expect("0.35014 42208 20024D");
    TFN(erfc).expect("0.64985 57791 79976D");
    TFN(tgamma).expect("2.78663 45408 4547D");
    TFN(lgamma).expect("1.02483 46099 5731D");
    TFN(gamma).expect("2.78663 45408 4547D");
    TFN(cbrt).expect("0.68470 21277 57224D");
    TFN(norm).expect("0.321");
#undef TFN
#undef TFNA

    step("pow")
        .test(CLEAR, "3.21 1.23 pow", ENTER)
        .expect("4.19760 13402 6956D")
        .test(CLEAR, "1.23 2.31", ID_pow)
        .expect("1.61317 24907 5554D");

    step("hypot")
        .test(CLEAR, "3.21 1.23 hypot", ENTER)
        .expect("3.43758 63625 5149D");

    step("atan2 pos / pos quadrant")
        .test(CLEAR, "3.21 1.23 atan2", ENTER)
        .expect("1.20487 56251 5281D r");
    step("atan2 pos / neg quadrant")
        .test(CLEAR, "3.21 -1.23 atan2", ENTER)
        .expect("1.93671 70284 3698D r");
    step("atan2 neg / pos quadrant")
        .test(CLEAR, "-3.21 1.23 atan2", ENTER)
        .expect("-1.20487 56251 5281D r");
    step("atan2 neg / neg quadrant")
        .test(CLEAR, "-3.21 -1.23 atan2", ENTER)
        .expect("-1.93671 70284 3698D r");

    step("Check integer rounding in hardware FP mode (#1309)")
        .test(CLEAR, "{ 3 3 } RANM", ENTER)
        .type(ID_array);

    step("Check hwfp overflow (#1317)")
        .test(CLEAR, "1E256", ID_sq)
        .expect("∞");
    step("Check hwfp overflow (#1317)")
        .test(CLEAR, "OverflowError", ENTER).noerror()
        .test("1E256", ID_sq)
        .error("Numerical overflow")
        .test(CLEAR, "'OverflowError' Purge", ENTER).noerror();

    step("Check rounding to fraction(#1481)")
        .test(CLEAR, DIRECT("10.25D ToFraction"), ENTER)
        .expect("10 ¹/₄");

    step("Restore default 24-digit precision");
    test(CLEAR, "24 PRECISION 12 SIG SoftFP", ENTER).noerror();
}


void tests::high_precision_numerical_functions()
// ----------------------------------------------------------------------------
//   Test high-precision numerical functions
// ----------------------------------------------------------------------------
{
    BEGIN(highp);

    step("Select 120-digit precision")
        .test(CLEAR, "120 PRECISION 119 SIG", ENTER).noerror();
    step("Setting radians mode")
        .test(CLEAR, "RAD", ENTER).noerror();

    step("Addition")
        .test(CLEAR, "1.23 2.34", ID_add).expect("3.57")
        .test(CLEAR, "1.23 -2.34", ID_add).expect("-1.11")
        .test(CLEAR, "-1.23 2.34", ID_add).expect("1.11")
        .test(CLEAR, "-1.23 -2.34", ID_add).expect("-3.57")
        .test(CLEAR, "1.234 SIN 2.34", ID_add).expect("3.28381 82093 74633 70486 17510 06156 82758 95172 14272 07657 60747 22091 17818 71399 90696 80994 83012 59886 50556 27858 44350 79955 18738 766")
        .test(CLEAR, "1.23 COS -2.34", ID_add).expect("-2.00576 22728 75497 40176 04527 54502 33554 62422 20360 95512 16741 09716 34981 87666 27553 75383 23279 23951 11502 06776 89604 78156 26344 97")
        .test(CLEAR, "-1.23 TAN 2.34", ID_add).expect("-0.47981 57342 68151 97480 88818 34909 67267 63017 29576 63870 87847 72873 08737 86224 89502 16556 77388 45242 02685 46713 25008 91512 90180 8082")
        .test(CLEAR, "-1.23 TANH -2.34", ID_add).expect("-3.18257 93256 58929 54289 07208 91501 65091 42132 21054 06082 52654 90143 67515 93012 41309 88423 04706 28583 94673 60063 58625 76729 87437 237");
    step("Subtraction")
        .test(CLEAR, "1.23 2.34", ID_subtract).expect("-1.11")
        .test(CLEAR, "1.23 -2.34", ID_subtract).expect("3.57")
        .test(CLEAR, "-1.23 2.34", ID_subtract).expect("-3.57")
        .test(CLEAR, "-1.23 -2.34", ID_subtract).expect("1.11")
        .test(CLEAR, "1.234 SIN 2.34", ID_subtract).expect("-1.39618 17906 25366 29513 82489 93843 17241 04827 85727 92342 39252 77908 82181 28600 09303 19005 16987 40113 49443 72141 55649 20044 81261 234")
        .test(CLEAR, "1.23 COS -2.34", ID_subtract).expect("2.67423 77271 24502 59823 95472 45497 66445 37577 79639 04487 83258 90283 65018 12333 72446 24616 76720 76048 88497 93223 10395 21843 73655 03")
        .test(CLEAR, "-1.23 TAN 2.34", ID_subtract).expect("-5.15981 57342 68151 97480 88818 34909 67267 63017 29576 63870 87847 72873 08737 86224 89502 16556 77388 45242 02685 46713 25008 91512 90180 808")
        .test(CLEAR, "-1.23 TANH -2.34", ID_subtract).expect("1.49742 06743 41070 45710 92791 08498 34908 57867 78945 93917 47345 09856 32484 06987 58690 11576 95293 71416 05326 39936 41374 23270 12562 763");
    step("Multiplication")
        .test(CLEAR, "1.23 2.34", ID_multiply).expect("2.8782")
        .test(CLEAR, "1.23 -2.34", ID_multiply).expect("-2.8782")
        .test(CLEAR, "-1.23 2.34", ID_multiply).expect("-2.8782")
        .test(CLEAR, "-1.23 -2.34", ID_multiply).expect("2.8782")
        .test(CLEAR, "1.234 SIN 2.34", ID_multiply).expect("2.20853 46099 36642 86937 64973 54406 97655 94702 81396 65918 80148 49693 35695 79075 78230 53527 90249 48134 42301 69188 75780 87095 13848 713")
        .test(CLEAR, "1.23 COS -2.34", ID_multiply).expect("-0.78211 62814 71336 07988 05405 54464 53482 17932 04355 36501 52825 83263 74142 40860 91524 21603 23526 57954 39085 16142 06324 81114 34352 7696")
        .test(CLEAR, "-1.23 TAN 2.34", ID_multiply).expect("-6.59836 88181 87475 62105 27834 93688 63406 25460 47209 33457 85563 68523 02446 59766 25435 06742 85088 97866 34283 99309 00520 86140 19023 091")
        .test(CLEAR, "-1.23 TANH -2.34", ID_multiply).expect("1.97163 56220 41895 13036 42868 86113 86313 92589 37266 50233 11212 46936 19987 27649 04665 12909 93012 70886 43536 22548 79184 29547 90603 134");
    step("Division")
        .test(CLEAR, "1.23 2.34", ID_divide).expect("0.52564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 2564")
        .test(CLEAR, "1.23 -2.34", ID_divide).expect("-0.52564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 2564")
        .test(CLEAR, "-1.23 2.34", ID_divide).expect("-0.52564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 2564")
        .test(CLEAR, "-1.23 -2.34", ID_divide).expect("0.52564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 25641 02564 10256 41025 64102 56410 2564")
        .test(CLEAR, "1.234 SIN 2.34", ID_divide).expect("0.40334 11151 17364 83113 75004 29981 55025 19304 33449 60537 43909 06876 57187 48461 49870 43160 18381 45250 64340 28999 33483 24767 17409 7291")
        .test(CLEAR, "1.23 COS -2.34", ID_divide).expect("-0.14283 66355 23291 70864 93791 64742 59164 69050 34033 77986 25324 31745 14965 00997 31814 63511 43897 76089 26708 51804 74527 87112 70792 7478")
        .test(CLEAR, "-1.23 TAN 2.34", ID_divide).expect("-1.20504 94590 88953 83538 84110 40559 68917 79067 22041 29859 34977 66185 08007 63343 97223 14767 85208 74035 05421 13980 02140 56202 09478 978")
        .test(CLEAR, "-1.23 TANH -2.34", ID_divide).expect("0.36007 66348 96978 43713 27867 05769 93628 81253 08142 76103 64382 43651 14323 04706 15944 39497 02865 93411 94304 95753 66934 08858 92067 1952");
    step("Power")
        .test(CLEAR, "1.23 2.34", ID_pow).expect("1.62322 21516 85370 76170 21776 74374 04103 27090 58024 62880 50736 29360 27592 07917 75146 99083 57726 38100 05735 87359 05132 61280 29729 274")
        .test(CLEAR, "1.23 -2.34", ID_pow).expect("0.61605 86207 88111 35803 50956 46724 98591 90279 99659 77958 49978 01436 78988 97209 72893 73693 48233 61309 17629 97957 78283 38559 84827 6568")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.90082 15014 43291 74653 48578 88750 68007 69541 15726 7")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 23", ID_pow).expect("-116.90082 15014 43291 74653 48578 88750 68007 69541 15726 7")
        .test(CLEAR, "-1.23 -2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "1.234 SIN 2.34", ID_pow).expect("0.87345 13971 11436 95155 06870 44540 70174 27291 82925 84673 60872 62775 48945 10990 94126 48813 44383 61846 88450 45997 75145 12827 34289 0572")
        .test(CLEAR, "1.23 COS -2.34", ID_pow).expect("12.99302 28339 82056 39426 87501 27880 37045 92536 16587 57403 56215 08880 50350 81194 61226 34205 49843 15463 66527 28429 54768 38033 10733 25")
        .test(CLEAR, "-1.23 TAN 23", ID_pow).expect("-2.26504 47100 36734 53632 11380 88267 73995 83095 30275 90565 69960 79911 60281 89036 12608 17378 72500 95112 47589 25610 99723 61528 46412 707⁳¹⁰")
        .test(CLEAR, "-1.23 TAN 2.34", ID_pow).error("Argument outside domain")
        .test(CLEAR, "-1.23 TAN -23", ID_pow).expect("-4.41492 38890 02535 32657 39183 33114 42610 79161 90457 07890 27869 50941 95017 26203 95996 17209 38898 89303 26193 59642 46151 77992 62440 536⁳⁻¹¹")
        .test(CLEAR, "-1.23 TANH -2.34", ID_pow).error("Argument outside domain");

    step("Square root of 2")
        .test(CLEAR, "2 sqrt", ENTER)
        .expect("1.41421 35623 73095 04880 16887 24209 69807 85696 71875 37694 80731 76679 73799 07324 78462 10703 88503 87534 32764 15727 35013 84623 09122 97");
    step("Square root of 3")
        .test(CLEAR, "3 sqrt", ENTER)
        .expect("1.73205 08075 68877 29352 74463 41505 87236 69428 05253 81038 06280 55806 97945 19330 16908 80003 70811 46186 75724 85756 75626 14141 54067 03");
    step("Square root of 4")
        .test(CLEAR, "4 sqrt", ENTER)
        .expect("2.");
    step("Cube root of 2")
        .test(CLEAR, "2 cbrt", ENTER)
        .expect("1.25992 10498 94873 16476 72106 07278 22835 05702 51464 70150 79800 81975 11215 52996 76513 95948 37293 96562 43625 50941 54310 25603 56156 653");
    step("Cube root of 3")
        .test(CLEAR, "3 cbrt", ENTER)
        .expect("1.44224 95703 07408 38232 16383 10780 10958 83918 69253 49935 05775 46416 19454 16875 96829 99733 98547 55479 70564 52566 86835 08085 44895 5");
    step("Cube root of 27")
        .test(CLEAR, "27 cbrt", ENTER)
        .expect("3.");

    step("neg")
        .test(CLEAR, "3.21 neg", ENTER).expect("-3.21")
        .test("negate", ENTER).expect("3.21");
    step("inv")
        .test(CLEAR, "3.21 inv", ENTER)
        .expect("0.31152 64797 50778 81619 93769 47040 49844 23676 01246 10591 90031 15264 79750 77881 61993 76947 04049 84423 67601 24610 59190 03115 26479 7508")
        .test("inv", ENTER).expect("3.21");
    step("sq (square)")
        .test(CLEAR, "-3.21 sq", ENTER).expect("10.3041")
        .test("sq", ENTER).expect("106.17447 681");
    step("cubed")
        .test(CLEAR, "3.21 cubed", ENTER).expect("33.07616 1")
        .test("cubed", ENTER).expect("36 186.39267 80659 01161 281")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19 683");
    step("abs")
        .test(CLEAR, "-3.21 abs", ENTER).expect("3.21")
        .test("abs", ENTER, 1, ADD).expect("4.21");

    uint dur = 500;
#define TFNA(name, arg)                                                 \
    step(#name).test(CLEAR, #arg " " #name, LENGTHY(dur), ENTER)
#define TFN(name)               TFNA(name, 0.321)

    TFN(sqrt).expect("0.56656 86189 68611 77992 54734 04696 76902 95391 98874 84029 02431 74015 07100 23314 25810 89388 23378 74831 09026 25322 95207 15522 13334 6095");
    TFN(sin).expect("0.31551 56385 92727 11130 65931 11143 46372 42059 02807 32616 09042 60788 57395 26113 45495 81136 00515 46916 99088 86060 26856 68677 57717 8763");
    TFN(cos).expect("0.94892 03769 56583 01754 39451 32826 92551 54763 03148 22817 38878 87425 10454 37289 66657 74827 69303 30686 52796 72622 06704 70515 68539 2244");
    TFN(tan).expect("0.33249 95924 36471 87510 87087 30102 73796 83946 23980 80503 83311 21021 12491 95974 29552 25917 15859 83641 11747 44032 23741 83994 87487 0302");
    TFN(asin).expect("0.32678 51765 31495 46326 91997 64519 59826 36182 58080 21574 39673 71903 92028 08817 69439 28409 80968 96776 17859 18932 19725 72513 95078 4427 r");
    TFN(acos).expect("1.24401 11502 63401 15596 21219 27120 15317 84803 26619 47180 89431 15568 37587 30264 33703 82040 12171 20636 49246 66407 71348 31811 71333 091 r");
    TFN(atan).expect("0.31060 97928 13889 91760 67000 51446 83602 81125 07025 77281 14539 44776 64690 76612 68860 40731 31597 84656 31883 84021 79831 76697 34106 3622 r");
    TFN(sinh).expect("0.32654 11649 51806 35701 22065 63885 73434 59869 32810 98627 21625 46131 20539 70600 10083 27315 63713 66136 47461 26495 76415 60697 57676 2939");
    TFN(cosh).expect("1.05196 44159 41947 53843 52241 43605 67798 60702 39830 04737 76342 59201 97569 28172 48173 45468 64605 47110 19220 77704 23747 11369 53013 732");
    TFN(tanh).expect("0.31041 08466 05886 02148 50502 09383 09588 97683 04936 20954 99090 64143 22194 05034 27301 10326 36239 07947 98201 74192 58627 58374 14428 5904");
    TFN(asinh).expect("0.31572 82658 29379 61791 08945 47102 06380 00526 27320 40054 59952 39850 65785 93616 95975 70753 88242 69995 19084 50283 99306 71224 23629 0976");
    TFNA(acosh, 1.321).expect("0.78123 02051 96252 61474 22171 61603 43488 77028 85612 70883 33986 53192 83139 13864 10921 83081 88302 58903 47353 53634 04169 89742 02815 2852");
    TFN(atanh).expect("0.33276 15884 81814 59580 17641 70508 75106 43974 10006 34850 01665 72697 61781 57932 14419 67812 59706 77324 50200 63307 05966 90651 74209 3097");
    TFN(ln1p).expect("0.27838 90255 40188 26677 16283 42111 55094 94375 15179 05132 39494 81036 05142 66257 54337 55520 43633 04277 35736 38433 06042 83576 22139 6359");
    TFN(lnp1).expect("0.27838 90255 40188 26677 16283 42111 55094 94375 15179 05132 39494 81036 05142 66257 54337 55520 43633 04277 35736 38433 06042 83576 22139 6359");
    TFN(expm1).expect("0.37850 55808 93753 89544 74307 07491 41233 20571 72641 03364 97968 05333 18108 98772 58256 72784 28319 13246 66682 04200 00162 72067 10690 0258");
    TFN(ln).expect("-1.13631 41558 52121 18735 43303 10107 28991 65926 67631 93216 19228 05172 65001 85061 66283 45581 72770 57156 95345 21563 26917 04911 30388 598");
    TFN(log10).expect("-0.49349 49675 95127 92187 04308 57283 44904 46730 54244 17528 47831 88472 35123 39989 07607 74010 64305 99151 74781 24152 01829 22941 99221 5485");
    TFN(exp).expect("1.37850 55808 93753 89544 74307 07491 41233 20571 72641 03364 97968 05333 18108 98772 58256 72784 28319 13246 66682 04200 00162 72067 10690 026");
    TFN(exp10).expect("2.09411 24558 50892 67051 98819 85846 25435 50121 44808 82328 80597 04327 54118 26943 97658 88916 82284 18499 99928 85620 51265 40190 16492 158");
    TFN(exp2).expect("1.24919 61256 53376 70052 14667 82085 80659 83711 96789 11078 50872 03968 89639 54927 57400 23696 00219 70718 47302 80643 90803 89872 28867 485");
    TFN(erf).expect("0.35014 42208 20023 82355 16032 45050 23912 83120 71924 29072 35684 90423 15676 68631 26483 67740 59618 93127 36786 06239 23468 00013 58887 219");
    TFN(erfc).expect("0.64985 57791 79976 17644 83967 54949 76087 16879 28075 70927 64315 09576 84323 31368 73516 32259 40381 06872 63213 93760 76531 99986 41112 781");
    TFN(tgamma).expect("2.78663 45408 45472 36795 07642 12781 77275 03497 82995 16602 55760 07828 51424 44941 90542 89306 12905 33223 77665 62678 93736 32221 42288 144", 2000);
    TFN(lgamma).expect("1.02483 46099 57313 19869 10927 53834 88666 18028 66769 43209 08437 87004 46327 04911 25770 09539 00530 12325 23947 42518 21539 88411 28272 448", 2000);
    TFN(gamma).expect("2.78663 45408 45472 36795 07642 12781 77275 03497 82995 16602 55760 07828 51424 44941 90542 89306 12905 33223 77665 62678 93736 32221 42288 144", 2000);
    TFN(cbrt).expect("0.68470 21277 57224 16184 09277 32646 81496 28057 14749 53139 45950 35873 52977 73009 35191 71304 84396 28932 73625 07589 02266 77954 73690 2353");
    TFN(norm).expect("0.321");
#undef TFN
#undef TFNA

    step("pow")
        .test(CLEAR, "3.21 1.23 pow", ENTER)
        .expect("4.19760 13402 69557 03133 41557 04388 71185 62403 13482 15741 54975 76397 39514 93831 64438 34447 96787 36431 56648 68643 95471 93476 15863 236")
        .test(CLEAR, "1.23 2.31", ID_pow)
        .expect("1.61317 24907 55543 84434 14148 92337 98559 17006 64245 18957 27180 28125 67872 74870 17458 75459 57723 53996 95111 93456 40634 86700 09601 019");

    step("hypot")
        .test(CLEAR, "3.21 1.23 hypot", ENTER)
        .expect("3.43758 63625 51492 31996 16557 32945 23541 88726 55087 78271 21507 69382 98782 20308 03280 97137 37583 47164 32055 25578 11148 26146 57350 441");

    step("atan2 pos / pos quadrant")
        .test(CLEAR, "3.21 1.23 atan2", ENTER)
        .expect("1.20487 56251 52809 23400 86691 05495 30674 32743 54426 68497 01001 78719 37086 47165 61508 05592 53255 02332 28917 23139 67613 92267 03142 769 r");
    step("atan2 pos / neg quadrant")
        .test(CLEAR, "3.21 -1.23 atan2", ENTER)
        .expect("1.93671 70284 36984 00445 39742 77784 19614 09228 14972 69013 57207 96225 22144 30998 44778 15307 33025 32493 05294 47540 14534 16384 29680 297 r");
    step("atan2 neg / pos quadrant")
        .test(CLEAR, "-3.21 1.23 atan2", ENTER)
        .expect("-1.20487 56251 52809 23400 86691 05495 30674 32743 54426 68497 01001 78719 37086 47165 61508 05592 53255 02332 28917 23139 67613 92267 03142 769 r");
    step("atan2 neg / neg quadrant")
        .test(CLEAR, "-3.21 -1.23 atan2", ENTER)
        .expect("-1.93671 70284 36984 00445 39742 77784 19614 09228 14972 69013 57207 96225 22144 30998 44778 15307 33025 32493 05294 47540 14534 16384 29680 297 r");

    step("Restore default 24-digit precision");
    test(CLEAR, "24 PRECISION 12 SIG", ENTER).noerror();
}


void tests::exact_trig_cases()
// ----------------------------------------------------------------------------
//   Special trig cases that are handled accurately for polar representation
// ----------------------------------------------------------------------------
{
    BEGIN(trigoptim);

    cstring unit_names[] = { "Grads", "Degrees", "PiRadians" };
    int circle[] = { 400, 360, 2 };

    step("Switch to big fractions")
        .test("BigFractions", ENTER).noerror();

    for (uint unit = 0; unit < 3; unit++)
    {
        step(unit_names[unit]);
        test(CLEAR, unit_names[unit], ENTER).noerror();

        int base = ((lrand48() & 0xFF) - 0x80) * 360;
        char buf[80];
        snprintf(buf, sizeof(buf),
                 "Selecting base %d degrees for %s angles",
                 base, unit_names[unit]);
        step(buf);
        test(CLEAR, base, ENTER, 360, " mod", ENTER).expect("0");
        test(CLEAR, base, ENTER, circle[unit], MUL, 360, DIV,
             circle[unit], " mod", ENTER).expect("0");

        step("sin(0) = 0")
            .test(base + 0, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("0");
        step("cos(0) = 1")
            .test(base + 0, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("1");
        step("tan(0) = 0")
            .test(base + 0, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("0");

        step("sin(30) = 1/2")
            .test(base + 30, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("1/2");
        step("tan(45) = 1")
            .test(base + 45, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("1");
        step("cos(60) = 1/2")
            .test(base + 60, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("1/2");

        step("sin(90) = 1")
            .test(base + 90, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("1");
        step("cos(90) = 0")
            .test(base + 90, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("0");

        step("cos(120) = -1/2")
            .test(base + 120, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("-1/2");
        step("tan(135) = -1")
            .test(base + 135, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("-1");
        step("sin(150) = 1/2")
            .test(base + 150, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("1/2");

        step("sin(180) = 0")
            .test(base + 180, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("0");
        step("cos(180) = -1")
            .test(base + 180, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("-1");
        step("tan(180) = 0")
            .test(base + 180, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("0");

        step("sin(210) = -1/2")
            .test(base + 210, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("-1/2");
        step("tan(225) = 1")
            .test(base + 225, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("1");
        step("cos(240) = -1/2")
            .test(base + 240, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("-1/2");

        step("sin(270) = -1")
            .test(base + 270, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("-1");
        step("cos(270) = 0")
            .test(base + 270, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("0");

        step("cos(300) = 1/2")
            .test(base + 300, ENTER, circle[unit], MUL, 360, DIV, COS)
            .expect("1/2");
        step("tan(315) = -1")
            .test(base + 315, ENTER, circle[unit], MUL, 360, DIV, TAN)
            .expect("-1");
        step("sin(330) = -1/2")
            .test(base + 330, ENTER, circle[unit], MUL, 360, DIV, SIN)
            .expect("-1/2");

        step("atan2(0,2) = 0 degrees")
            .test("0 2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * 0 / 2);
        step("atan2(2,2) = 45 degrees")
            .test("2 2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * 1 / 2);
        step("atan2(2,0) = 90 degrees")
            .test("2 0 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * 2 / 2);
        step("atan2(2,-2) = 135 degrees")
            .test("2 -2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * 3 / 2);
        step("atan2(0,-2) = 180 degrees")
            .test("0 -2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * 4 / 2);
        step("atan2(-2,-2) = -135 degrees")
            .test("-2 -2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * -3 / 2);
        step("atan2(0,-2) = 90 degrees")
            .test("-2 0 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * -2 / 2);
        step("atan2(2,-2) = 135 degrees")
            .test("-2 2 ATAN2", ENTER)
            .type(ID_unit)
            .test("UVAL 4 *", ENTER)
            .expect(circle[unit] * -1 / 2);
        step("asin(0) = 0")
            .test("0 asin", ENTER)
            .type(ID_unit)
            .test("UVAL", ENTER)
            .expect("0.");
        step("asin(1) = 90")
            .test("1 asin", ENTER)
            .type(ID_unit)
            .test("UVAL 4 * ", circle[unit], " -", ENTER)
            .expect("0.");
        step("asin(-1) = -90")
            .test("-1 asin", ENTER)
            .type(ID_unit)
            .test("UVAL -4 * ", circle[unit], " -", ENTER)
            .expect("0.");
        step("acos(0) = 90")
            .test("0 acos", ENTER)
            .type(ID_unit)
            .test("UVAL 4 * ", circle[unit], " -", ENTER)
            .expect("0.");
        step("acos(1) = 0")
            .test("1 acos", ENTER)
            .type(ID_unit)
            .test("UVAL", ENTER)
            .expect("0.");
        step("acos(-1) = 180 degrees")
            .test("-1 acos", ENTER)
            .type(ID_unit)
            .test("UVAL 2 * ", circle[unit], " - ToDecimal", ENTER)
            .expect("0.");
    }

    step("Conversion from non-standard units")
        .test(CLEAR, "1/8_turn COS", ENTER)
        .expect("0.70710 67811 87");

    step("Cleaning up")
        .test(CLEAR, "SmallFractions DEG", ENTER).noerror();
}


void tests::fraction_decimal_conversions()
// ----------------------------------------------------------------------------
//   Exercise the conversion from decimal to fraction and back
// ----------------------------------------------------------------------------
{
    cstring cases[] =
    {
        // Easy exact cases (decimal)
        "1/2",          "0.5",
        "1/4",          "0.25",
        "5/4",          "1.25",
        "-5/4",         "-1.25",

        // More tricky fractions
        "1/3",          "0.33333 33333 33",
        "-1/7",         "-0.14285 71428 57",
        "22/7",         "3.14285 71428 6",
        "37/213",       "0.17370 89201 88",
    };

    BEGIN(dfrac);

    step("Selecting big mixed fraction mode")
        .test(CLEAR, "BigFractions ImproperFractions", ENTER).noerror();

    for (uint c = 0; c < sizeof(cases) / sizeof(*cases); c += 2)
    {
        step(cases[c]);
        test(CLEAR, cases[c], ENTER).expect(cases[c]);
        test("→Num", ENTER).expect(cases[c+1]);
        test("→Q", ENTER).expect(cases[c]);
    }

    step("Alternate spellings");
    test(CLEAR, "1/4 →Decimal", ENTER).expect("0.25");
    test(CLEAR, "1/5 ToDecimal", ENTER).expect("0.2");
    test(CLEAR, "0.25 →Frac", ENTER).expect("1/4");
    test(CLEAR, "0.2 ToFraction", ENTER).expect("1/5");

    step("Integer conversions");
    test(CLEAR, "3. R→I", ENTER).expect("3");
    test(CLEAR, "-3. R→I", ENTER).expect("-3");
    test(CLEAR, "3.2 R→I", ENTER).error("Bad argument value");

    step("Complex numbers");
    test(CLEAR, "1-2ⅈ 4", ENTER, DIV).expect("1/4-1/2ⅈ");
    test("→Num", ENTER).expect("0.25-0.5ⅈ");
    test("→Q", ENTER).expect("1/4-1/2ⅈ");

    step("Vectors");
    test(CLEAR, "[1-2ⅈ 3] 4", ENTER, DIV).expect("[ 1/4-1/2ⅈ 3/4 ]");
    test("→Num", ENTER).expect("[ 0.25-0.5ⅈ 0.75 ]");
    test("→Q", ENTER).expect("[ 1/4-1/2ⅈ 3/4 ]");

    step("Expressions");
    test(CLEAR, "355 113 /",
         LSHIFT, I, F2, F1, "-", ENTER) .expect("'355/113-π'");
    test("→Num", ENTER).expect("0.00000 02667 64");

    step("Restoring small fraction mode")
        .test(CLEAR, "SmallFractions MixedFractions", ENTER).noerror();
}


void tests::trig_units()
// ----------------------------------------------------------------------------
//   Check trigonometric units
// ----------------------------------------------------------------------------
{
    BEGIN(trigunits);

    step("Select degrees mode")
        .test(CLEAR, ID_ModesMenu, F1).noerror();
    step("Disable trig units mode")
        .test("NoAngleUnits", ENTER).noerror();
    step("Check that arc-sin produces numerical value")

        .test(CLEAR, "0.2", LSHIFT, J)
        .noerror()
        .type(ID_decimal)
        .expect("11.53695 90328");
    step("Check that arc-sin numerical value depends on angle mode")
        .test(CLEAR, ID_ModesMenu, F2)
        .test("0.2", LSHIFT, J)
        .noerror()
        .type(ID_decimal)
        .expect("0.20135 79207 9");

    step("Enable trig units mode")
        .test("SetAngleUnits", ENTER).noerror();
    step("Select degrees mode")
        .test(CLEAR, ID_ModesMenu, F1).noerror();
    step("Check that arc-sin produces unit value with degrees")

        .test("0.2", LSHIFT, J)
        .noerror()
        .type(ID_unit)
        .expect("11.53695 90328 °");
    step("Check that arc-sin produces radians unit")
        .test(F2)
        .test("0.2", LSHIFT, J)
        .noerror()
        .type(ID_unit)
        .expect("0.20135 79207 9 r");
    step("Check that arc-sin produces pi-radians unit")
        .test(F3)
        .test("0.2", LSHIFT, J)
        .noerror()
        .type(ID_unit)
        .expect("0.06409 42168 49 πr");
    step("Check that arc-sin produces grads unit")
        .test(LSHIFT, F1)
        .test("0.2", LSHIFT, J)
        .noerror()
        .type(ID_unit)
        .expect("12.81884 33698 grad");

    step("Check that grad value is respected in degrees")
        .test(F1, J)
        .expect("0.2")
        .test(BSP);
    step("Check that pi-radians value is respected in grads")
        .test(SHIFT, F1, J)
        .expect("0.2")
        .test(BSP);
    step("Check that radians value is respected in degrees")
        .test(F1, J)
        .expect("0.2")
        .test(BSP);
    step("Check that degrees value is respected in degrees")
        .test(F1, J)
        .expect("0.2")
        .test(BSP);

    step ("Numerical conversion from degrees to radians")
        .test(CLEAR, "1.2 R→D", ENTER).noerror().expect("68.75493 54157");
    step ("Symbolic conversion from degrees to radians")
        .test(CLEAR, "'X' R→D", ENTER).noerror().expect("'57.29577 95131·X'");
    step ("Numerical conversion from radians to degrees")
        .test(CLEAR, "1.2 D→R", ENTER).noerror().expect("0.02094 39510 24");
    step ("Symbolic conversion from radians to degrees")
        .test(CLEAR, "'X' D→R", ENTER).noerror().expect("'0.01745 32925 2·X'");

    step("Select degrees mode")
        .test(CLEAR, ID_ModesMenu, LSHIFT, F2, F1).noerror();
    step("Numerical conversion to degrees in degrees mode")
        .test("1.2", LSHIFT, F1).expect("1.2 °");
    step("Numerical conversion to radians in degrees mode")
        .test("1.2", LSHIFT, F2).expect("0.02094 39510 24 r");
    step("Numerical conversion to grad in degrees mode")
        .test("1.2", LSHIFT, F3).expect("1.33333 33333 3 grad");
    step("Numerical conversion to pi-radians in degrees mode")
        .test("1.2", LSHIFT, F4).expect("0.00666 66666 67 πr");

    step("Select radians mode")
        .test(CLEAR, ID_ModesMenu, LSHIFT, F2, F2).noerror();
    step("Numerical conversion to degrees in radians mode")
        .test("1.2", LSHIFT, F1).expect("68.75493 54157 °");
    step("Numerical conversion to radians in radians mode")
        .test("1.2", LSHIFT, F2).expect("1.2 r");
    step("Numerical conversion to grad in radians mode")
        .test("1.2", LSHIFT, F3).expect("76.39437 26841 grad");
    step("Numerical conversion to pi-radians in radians mode")
        .test("1.2", LSHIFT, F4).expect("0.38197 18634 21 πr");

    step("Select grads mode")
        .test(CLEAR, ID_ModesMenu, LSHIFT, F2, F3).noerror();
    step("Numerical conversion to degrees in grads mode")
        .test("1.2", LSHIFT, F1).expect("1.08 °");
    step("Numerical conversion to radians in grads mode")
        .test("1.2", LSHIFT, F2).expect("0.01884 95559 22 r");
    step("Numerical conversion to grad in grads mode")
        .test("1.2", LSHIFT, F3).expect("1.2 grad");
    step("Numerical conversion to pi-radians in grads mode")
        .test("1.2", LSHIFT, F4).expect("0.006 πr");

    step("Select pi-radians mode")
        .test(CLEAR, ID_ModesMenu, LSHIFT, F2, F4).noerror();
    step("Numerical conversion to degrees in pi-radians mode")
        .test("1.2", LSHIFT, F1).expect("216. °");
    step("Numerical conversion to radians in pi-radians mode")
        .test("1.2", LSHIFT, F2).expect("3.76991 11843 1 r");
    step("Numerical conversion to grad in pi-radians mode")
        .test("1.2", LSHIFT, F3).expect("240. grad");
    step("Numerical conversion to pi-radians in pi-radians mode")
        .test("1.2", LSHIFT, F4).expect("1.2 πr");

    step("Selecting degrees")
        .test(CLEAR, ID_ModesMenu, LSHIFT, F2, F1).noerror();
    step("Creating a degrees value")
        .test(CLEAR, "1/2", LSHIFT, F1).expect("¹/₂ °");
    step("Converting to grad")
        .test(LSHIFT, F3).expect("⁵/₉ grad");
    step("Converting to pi-radians")
        .test(LSHIFT, F4).expect("¹/₃₆₀ πr");
    step("Converting to degrees")
        .test(LSHIFT, F1).expect("¹/₂ °");
    step("Converting to radians")
        .test(LSHIFT, F2).expect("0.00872 66462 6 r");
    step("Converting to degrees")
        .test(LSHIFT, F1).expect("0.5 °");
}


void tests::rounding_and_truncating()
// ----------------------------------------------------------------------------
//   Test rounding and truncating
// ----------------------------------------------------------------------------
{
    BEGIN(round);

    step("Rounding to decimal places")
        .test(CLEAR, "1.234567890", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("1.")
        .test(BSP, ENTER, "1", F5).expect("1.2")
        .test(BSP, ENTER, "2", F5).expect("1.23")
        .test(BSP, ENTER, "3", F5).expect("1.235")
        .test(BSP, ENTER, "4", F5).expect("1.2346")
        .test(BSP, ENTER, "5", F5).expect("1.23457")
        .test(BSP, ENTER, "6", F5).expect("1.23456 8")
        .test(BSP, ENTER, "7", F5).expect("1.23456 79")
        .test(BSP, ENTER, "8", F5).expect("1.23456 789")
        .test(BSP, ENTER, "9", F5).expect("1.23456 789")
        .test(BSP, ENTER, "10", F5).expect("1.23456 789");
    step("Rounding to decimal places with exponent")
        .test(CLEAR, "1.234567890E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("0.")
        .test(BSP, ENTER, "1", F5).expect("0.")
        .test(BSP, ENTER, "2", F5).expect("0.")
        .test(BSP, ENTER, "3", F5).expect("0.001")
        .test(BSP, ENTER, "4", F5).expect("0.0012")
        .test(BSP, ENTER, "5", F5).expect("0.00123")
        .test(BSP, ENTER, "6", F5).expect("0.00123 5")
        .test(BSP, ENTER, "7", F5).expect("0.00123 46")
        .test(BSP, ENTER, "8", F5).expect("0.00123 457")
        .test(BSP, ENTER, "9", F5).expect("0.00123 4568")
        .test(BSP, ENTER, "10", F5).expect("0.00123 45679");

    step("Rounding negative number to decimal places")
        .test(CLEAR, "-9.876543210", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("-10.")
        .test(BSP, ENTER, "1", F5).expect("-9.9")
        .test(BSP, ENTER, "2", F5).expect("-9.88")
        .test(BSP, ENTER, "3", F5).expect("-9.877")
        .test(BSP, ENTER, "4", F5).expect("-9.8765")
        .test(BSP, ENTER, "5", F5).expect("-9.87654")
        .test(BSP, ENTER, "6", F5).expect("-9.87654 3")
        .test(BSP, ENTER, "7", F5).expect("-9.87654 32")
        .test(BSP, ENTER, "8", F5).expect("-9.87654 321")
        .test(BSP, ENTER, "9", F5).expect("-9.87654 321")
        .test(BSP, ENTER, "10", F5).expect("-9.87654 321");
    step("Rounding negative number to decimal places with exponent")
        .test(CLEAR, "-9.876543210E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("0.")
        .test(BSP, ENTER, "1", F5).expect("0.")
        .test(BSP, ENTER, "2", F5).expect("-0.01")
        .test(BSP, ENTER, "3", F5).expect("-0.01")
        .test(BSP, ENTER, "4", F5).expect("-0.0099")
        .test(BSP, ENTER, "5", F5).expect("-0.00988")
        .test(BSP, ENTER, "6", F5).expect("-0.00987 7")
        .test(BSP, ENTER, "7", F5).expect("-0.00987 65")
        .test(BSP, ENTER, "8", F5).expect("-0.00987 654")
        .test(BSP, ENTER, "9", F5).expect("-0.00987 6543")
        .test(BSP, ENTER, "10", F5).expect("-0.00987 65432");

    step("Rounding to significant digits")
        .test(CLEAR, "1.234567890", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "1", CHS, F5).expect("1.")
        .test(BSP, ENTER, "2", CHS, F5).expect("1.2")
        .test(BSP, ENTER, "3", CHS, F5).expect("1.23")
        .test(BSP, ENTER, "4", CHS, F5).expect("1.235")
        .test(BSP, ENTER, "5", CHS, F5).expect("1.2346")
        .test(BSP, ENTER, "6", CHS, F5).expect("1.23457")
        .test(BSP, ENTER, "7", CHS, F5).expect("1.23456 8")
        .test(BSP, ENTER, "8", CHS, F5).expect("1.23456 79")
        .test(BSP, ENTER, "9", CHS, F5).expect("1.23456 789")
        .test(BSP, ENTER, "10", CHS, F5).expect("1.23456 789");
    step("Rounding to decimal places with exponent")
        .test(CLEAR, "1.234567890E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "1", CHS, F5).expect("0.001")
        .test(BSP, ENTER, "2", CHS, F5).expect("0.0012")
        .test(BSP, ENTER, "3", CHS, F5).expect("0.00123")
        .test(BSP, ENTER, "4", CHS, F5).expect("0.00123 5")
        .test(BSP, ENTER, "5", CHS, F5).expect("0.00123 46")
        .test(BSP, ENTER, "6", CHS, F5).expect("0.00123 457")
        .test(BSP, ENTER, "7", CHS, F5).expect("0.00123 4568")
        .test(BSP, ENTER, "8", CHS, F5).expect("0.00123 45679")
        .test(BSP, ENTER, "9", CHS, F5).expect("0.00123 45678 9")
        .test(BSP, ENTER, "10", CHS, F5).expect("0.00123 45678 9");

    step("Rounding negative number to decimal places")
        .test(CLEAR, "-9.876543210", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("-10.")
        .test(BSP, ENTER, "1", F5).expect("-9.9")
        .test(BSP, ENTER, "2", F5).expect("-9.88")
        .test(BSP, ENTER, "3", F5).expect("-9.877")
        .test(BSP, ENTER, "4", F5).expect("-9.8765")
        .test(BSP, ENTER, "5", F5).expect("-9.87654")
        .test(BSP, ENTER, "6", F5).expect("-9.87654 3")
        .test(BSP, ENTER, "7", F5).expect("-9.87654 32")
        .test(BSP, ENTER, "8", F5).expect("-9.87654 321")
        .test(BSP, ENTER, "9", F5).expect("-9.87654 321")
        .test(BSP, ENTER, "10", F5).expect("-9.87654 321");
    step("Rounding negative number to decimal places with exponent")
        .test(CLEAR, "-9.876543210E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", F5).expect("0.")
        .test(BSP, ENTER, "1", F5).expect("0.")
        .test(BSP, ENTER, "2", F5).expect("-0.01")
        .test(BSP, ENTER, "3", F5).expect("-0.01")
        .test(BSP, ENTER, "4", F5).expect("-0.0099")
        .test(BSP, ENTER, "5", F5).expect("-0.00988")
        .test(BSP, ENTER, "6", F5).expect("-0.00987 7")
        .test(BSP, ENTER, "7", F5).expect("-0.00987 65")
        .test(BSP, ENTER, "8", F5).expect("-0.00987 654")
        .test(BSP, ENTER, "9", F5).expect("-0.00987 6543")
        .test(BSP, ENTER, "10", F5).expect("-0.00987 65432");

    step("Truncating to decimal places")
        .test(CLEAR, "1.234567890", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("1.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("1.2")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("1.23")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("1.234")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("1.2345")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("1.23456")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("1.23456 7")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("1.23456 78")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("1.23456 789")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("1.23456 789")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("1.23456 789");
    step("Truncating to decimal places with exponent")
        .test(CLEAR, "1.234567890E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("0.001")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("0.0012")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("0.00123")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("0.00123 4")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("0.00123 45")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("0.00123 456")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("0.00123 4567")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("0.00123 45678");

    step("Truncating negative number to decimal places")
        .test(CLEAR, "-9.876543210", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("-9.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("-9.8")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("-9.87")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("-9.876")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("-9.8765")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("-9.87654")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("-9.87654 3")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("-9.87654 32")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("-9.87654 321")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("-9.87654 321")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("-9.87654 321");
    step("Truncating negative number to decimal places with exponent")
        .test(CLEAR, "-9.876543210E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("-0.")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("-0.009")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("-0.0098")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("-0.00987")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("-0.00987 6")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("-0.00987 65")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("-0.00987 654")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("-0.00987 6543")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("-0.00987 65432");

    step("Truncating to significant digits")
        .test(CLEAR, "1.234567890", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "1", CHS, LSHIFT, F1).expect("1.")
        .test(BSP, ENTER, "2", CHS, LSHIFT, F1).expect("1.2")
        .test(BSP, ENTER, "3", CHS, LSHIFT, F1).expect("1.23")
        .test(BSP, ENTER, "4", CHS, LSHIFT, F1).expect("1.234")
        .test(BSP, ENTER, "5", CHS, LSHIFT, F1).expect("1.2345")
        .test(BSP, ENTER, "6", CHS, LSHIFT, F1).expect("1.23456")
        .test(BSP, ENTER, "7", CHS, LSHIFT, F1).expect("1.23456 7")
        .test(BSP, ENTER, "8", CHS, LSHIFT, F1).expect("1.23456 78")
        .test(BSP, ENTER, "9", CHS, LSHIFT, F1).expect("1.23456 789")
        .test(BSP, ENTER, "10", CHS, LSHIFT, F1).expect("1.23456 789");
    step("Truncating to decimal places with exponent")
        .test(CLEAR, "1.234567890E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "1", CHS, LSHIFT, F1).expect("0.001")
        .test(BSP, ENTER, "2", CHS, LSHIFT, F1).expect("0.0012")
        .test(BSP, ENTER, "3", CHS, LSHIFT, F1).expect("0.00123")
        .test(BSP, ENTER, "4", CHS, LSHIFT, F1).expect("0.00123 4")
        .test(BSP, ENTER, "5", CHS, LSHIFT, F1).expect("0.00123 45")
        .test(BSP, ENTER, "6", CHS, LSHIFT, F1).expect("0.00123 456")
        .test(BSP, ENTER, "7", CHS, LSHIFT, F1).expect("0.00123 4567")
        .test(BSP, ENTER, "8", CHS, LSHIFT, F1).expect("0.00123 45678")
        .test(BSP, ENTER, "9", CHS, LSHIFT, F1).expect("0.00123 45678 9")
        .test(BSP, ENTER, "10", CHS, LSHIFT, F1).expect("0.00123 45678 9");

    step("Truncating negative number to decimal places")
        .test(CLEAR, "-9.876543210", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("-9.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("-9.8")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("-9.87")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("-9.876")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("-9.8765")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("-9.87654")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("-9.87654 3")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("-9.87654 32")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("-9.87654 321")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("-9.87654 321")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("-9.87654 321");
    step("Truncating negative number to decimal places with exponent")
        .test(CLEAR, "-9.876543210E-3", ENTER, RSHIFT, E, ENTER)
        .test(BSP, ENTER, "0", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "1", LSHIFT, F1).expect("0.")
        .test(BSP, ENTER, "2", LSHIFT, F1).expect("-0.")
        .test(BSP, ENTER, "3", LSHIFT, F1).expect("-0.009")
        .test(BSP, ENTER, "4", LSHIFT, F1).expect("-0.0098")
        .test(BSP, ENTER, "5", LSHIFT, F1).expect("-0.00987")
        .test(BSP, ENTER, "6", LSHIFT, F1).expect("-0.00987 6")
        .test(BSP, ENTER, "7", LSHIFT, F1).expect("-0.00987 65")
        .test(BSP, ENTER, "8", LSHIFT, F1).expect("-0.00987 654")
        .test(BSP, ENTER, "9", LSHIFT, F1).expect("-0.00987 6543")
        .test(BSP, ENTER, "10", LSHIFT, F1).expect("-0.00987 65432");
}


void tests::complex_types()
// ----------------------------------------------------------------------------
//   Complex data typess
// ----------------------------------------------------------------------------
{
    BEGIN(ctypes);

    step("Select degrees for the angle");
    test(CLEAR, "DEG", ENTER).noerror();

    step("Integer rectangular form");
    test(CLEAR, "0ⅈ0", ENTER)
        .type(ID_rectangular).expect("0+0ⅈ");
    test(CLEAR, "1ⅈ2", ENTER)
        .type(ID_rectangular).expect("1+2ⅈ");
    test(CLEAR, "3+ⅈ4", ENTER)
        .type(ID_rectangular).expect("3+4ⅈ")
        .test(DOWN, ENTER)
        .type(ID_rectangular).expect("3+4ⅈ");
    test("ComplexIBeforeImaginary", ENTER)
        .type(ID_rectangular).expect("3+ⅈ4");
    test("ComplexIAfterImaginary", ENTER)
        .type(ID_rectangular).expect("3+4ⅈ");

    step("Behaviour of CHS on command-line");
    test(CLEAR, "4+ⅈ5", CHS, ENTER)
        .type(ID_rectangular).expect("4-5ⅈ");
    test(CLEAR, "5", CHS, "ⅈ6", CHS, ENTER)
        .type(ID_rectangular).expect("-5-6ⅈ");
    test(CLEAR, "6+7ⅈ", ENTER)
        .type(ID_rectangular).expect("6+7ⅈ");
    test(CLEAR, "7-8ⅈ", ENTER)
        .type(ID_rectangular).expect("7-8ⅈ");

    step("Integer polar form");
    test(CLEAR, "0∡0", ENTER)
        .type(ID_polar).expect("0∡0°")
        .test(DOWN, ENTER)
        .type(ID_polar).expect("0∡0°");
    test(CLEAR, "1∡90", ENTER)
        .type(ID_polar).expect("1∡90°")
        .test(DOWN, ENTER)
        .type(ID_polar).expect("1∡90°");
    test(CLEAR, "1∡-90", ENTER)
        .type(ID_polar).expect("1∡-90°")
        .test(DOWN, ENTER)
        .type(ID_polar).expect("1∡-90°");
    test(CLEAR, "-1∡0", ENTER)
        .type(ID_polar).expect("1∡180°");

    step("Decimal rectangular form");
    test(CLEAR, "0.1ⅈ2.3", ENTER)
        .type(ID_rectangular).expect("0.1+2.3ⅈ");
    test(CLEAR, "0.1ⅈ2.3", CHS, ENTER)
        .type(ID_rectangular).expect("0.1-2.3ⅈ");

    step("Decimal polar form");
    test(CLEAR, "0.1∡2.3", ENTER)
        .type(ID_polar).expect("0.1∡2.3°");
    test(CLEAR, "0.1∡2.3", CHS, ENTER)
        .type(ID_polar).expect("0.1∡-2.3°");

    step("Symbolic rectangular form");
    test(CLEAR, "aⅈb", ENTER)
        .type(ID_rectangular).expect("a+bⅈ");
    test(CLEAR, "c+dⅈ", ENTER)
        .type(ID_rectangular).expect("c+dⅈ");

    step("Symbolic polar form");
    test(CLEAR, "a∡b", ENTER)
        .type(ID_polar).expect("a∡b");
    test(CLEAR, "c∡d", ENTER)
        .type(ID_polar).expect("c∡d");

    step("Polar angle conversions")
        .test(CLEAR, "1∡90", ENTER).expect("1∡90°")
        .test("GRAD", ENTER).expect("1∡100ℊ")
        .test("PiRadians", ENTER).expect("1∡¹/₂ℼ")
        .test("RAD", ENTER).expect("1∡1.57079 63267 9ʳ");

    step("Angle mode conversions during polar entry")
        .test(CLEAR, "GRAD", ENTER).noerror()
        .test("1∡90°", ENTER).expect("1∡100ℊ")
        .test("1", ID_ComplexMenu, F2, ID_ModesMenu, "90", F1, ENTER)
        .expect("1∡100ℊ");

    step("Convert real to rectangular");
    test(CLEAR, "1 2", ID_ComplexMenu, F3)
        .type(ID_rectangular)
        .expect("1+2ⅈ");
    test(CLEAR, "1.2 3.4", ID_ComplexMenu, F3)
        .type(ID_rectangular)
        .expect("1.2+3.4ⅈ");

    step("Convert rectangular to real");
    test(CLEAR, "1ⅈ2", ID_ComplexMenu, F4)
        .type(ID_tag)
        .expect("im:2")
        .test(NOSHIFT, BSP)
        .type(ID_tag)
        .expect("re:1");
    test(CLEAR, "1.2ⅈ3.4", ID_ComplexMenu, F4)
        .type(ID_tag)
        .expect("im:3.4")
        .test(NOSHIFT, BSP)
        .type(ID_tag)
        .expect("re:1.2");

    step("Convert real to rectangular and back (strip tags)");
    test(CLEAR, "1 2", ID_ComplexMenu, F3)
        .type(ID_rectangular)
        .expect("1+2ⅈ")
        .test(F4).expect("im:2")
        .test(F3).expect("1+2ⅈ");
    test(CLEAR, "1.2 3.4", ID_ComplexMenu, F3)
        .type(ID_rectangular)
        .expect("1.2+3.4ⅈ")
        .test(F4).expect("im:3.4")
        .test(F3).expect("1.2+3.4ⅈ");

    step("Convert real to polar");
    test(CLEAR, ID_ModesMenu, F1).noerror();
    test(CLEAR, "1 2", ID_ComplexMenu, RSHIFT, F2)
        .type(ID_polar)
        .expect("1∡2°");
    test(CLEAR, "1.2 3.4", ID_ComplexMenu, RSHIFT, F2)
        .type(ID_polar)
        .expect("1.2∡3.4°");

    step("Convert polar to real");
    test(CLEAR, "1∡2", ID_ComplexMenu, RSHIFT, F3)
        .type(ID_tag)
        .expect("arg:2 °")
        .test(NOSHIFT, BSP)
        .type(ID_tag)
        .expect("mod:1");
    test(CLEAR, "1.2∡3.4", ID_ComplexMenu, RSHIFT, F3)
        .type(ID_tag)
        .expect("arg:3.4 °")
        .test(NOSHIFT, BSP)
        .type(ID_tag)
        .expect("mod:1.2");

    step("Convert real to polar and back (add units, strip tags)");
    test(CLEAR, ID_ModesMenu, F1).noerror();
    test(CLEAR, "1 2", ID_ComplexMenu, RSHIFT, F2)
        .type(ID_polar)
        .expect("1∡2°")
        .test(RSHIFT, F3).expect("arg:2 °")
        .test("RAD", ENTER).expect("arg:2 °")
        .test(RSHIFT, F2).expect("1∡0.03490 65850 4ʳ")
        .test("DEG", ENTER).expect("1∡2°");
    test(CLEAR, "1.2 3.4", ID_ComplexMenu, RSHIFT, F2)
        .type(ID_polar)
        .expect("1.2∡3.4°")
        .test(RSHIFT, F3).expect("arg:3.4 °")
        .test("RAD", ENTER).expect("arg:3.4 °")
        .test(RSHIFT, F2).expect("1.2∡0.05934 11945 68ʳ")
        .test("DEG", ENTER).expect("1.2∡3.4°");

    step("Short rectangular forms for i")
        .test(CLEAR, "ⅈ", ENTER)
        .type(ID_rectangular).expect("0+1ⅈ");
    step("Short rectangular forms for 3.5i")
        .test(CLEAR, "3.5ⅈ", ENTER)
        .type(ID_rectangular).expect("0+3.5ⅈ");
    step("Short rectangular forms for i12.05")
        .test(CLEAR, "ⅈ12.05", ENTER)
        .type(ID_rectangular).expect("0+12.05ⅈ");

    step("Syntax error for empty phase")
        .test(CLEAR, "1∡", ENTER)
        .error("Syntax error");
}


void tests::complex_arithmetic()
// ----------------------------------------------------------------------------
//   Complex arithmetic operations
// ----------------------------------------------------------------------------
{
    BEGIN(carith);

    step("Use degrees");
    test("DEG", ENTER).noerror();

    step("Addition");
    test(CLEAR, "1ⅈ2", ENTER, "3+ⅈ4", ENTER, ADD)
        .type(ID_rectangular).expect("4+6ⅈ");
    step("Subtraction");
    test("1-2ⅈ", SUB)
        .type(ID_rectangular).expect("3+8ⅈ");
    step("Multiplication");
    test("7+8ⅈ", MUL)
        .type(ID_rectangular).expect("-43+80ⅈ");
    step("Division");
    test("7+8ⅈ", DIV)
        .type(ID_rectangular).expect("3+8ⅈ");
    test("2+3ⅈ", DIV)
        .type(ID_rectangular).expect("2 ⁴/₁₃+⁷/₁₃ⅈ");
    test("2+3ⅈ", MUL)
        .type(ID_rectangular).expect("3+8ⅈ");
    step("Power");
    test("5", ID_pow)
        .type(ID_rectangular).expect("44 403-10 072ⅈ");

    step("Symbolic addition");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", ADD)
        .expect("'a+c'+'b+d'ⅈ");
    step("Symbolic subtraction");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", SUB)
        .expect("'a-c'+'b-d'ⅈ");
    step("Symbolic multiplication");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", MUL)
        .expect("'a·c-b·d'+'a·d+b·c'ⅈ");
    step("Symbolic division");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", DIV)
        .expect("'(a·c+b·d)÷(c²+d²)'+'(b·c-a·d)÷(c²+d²)'ⅈ");

    step("Addition in aligned polar form");
    test(CLEAR, "1∡2", ENTER, "3∡2", ENTER, ADD)
        .expect("4∡2°");
    step("Subtraction in aligned polar form");
    test("1∡2", SUB)
        .expect("3∡2°");
    test("5∡2", SUB)
        .expect("2∡-178°");
    step("Addition in polar form");
    test(CLEAR, "1∡2", ENTER, "3∡4", ENTER, ADD)
        .expect("3.99208 29778+0.24416 89179 35ⅈ");
    step("Subtraction");
    test("1∡2", SUB)
        .expect("2.99269 21507 8+0.20926 94212 32ⅈ");
    step("Multiplication");
    test("7∡8", MUL)
        .expect("21.∡12.°");
    step("Division");
    test("7∡8", DIV)
        .expect("3.∡4.°");
    test("2∡3", DIV)
        .expect("1.5∡1.°");
    test("2∡3", MUL)
        .expect("3.∡4.°");
    step("Power");
    test("5", ID_pow)
        .expect("243.∡20.°");

    step("Symbolic addition aligned");
    test(CLEAR, "a∡b", ENTER, "c∡b", ENTER, ADD)
        .expect("'a+c'∡b");
    step("Symbolic addition");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, ADD)
        .expect("'a·cos b+c·cos d'+'a·sin b+c·sin d'ⅈ");
    step("Symbolic substraction aligned");
    test(CLEAR, "a∡b", ENTER, "c∡b", ENTER, SUB)
        .expect("'a-c'∡b");
    step("Symbolic subtraction");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, SUB)
        .expect("'a·cos b-c·cos d'+'a·sin b-c·sin d'ⅈ");
    step("Symbolic multiplication");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, MUL)
        .expect("'a·c'∡'b+d'");
    step("Symbolic division");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, DIV)
        .expect("'a÷c'∡'b-d'");

    step("Precedence of complex numbers during rendering");
    test(CLEAR, "'2+3ⅈ' '3∡4' *", ENTER)
        .expect("'(2+3ⅈ)·(3∡4°)'");
    test(CLEAR, "'2+3ⅈ' '3∡4' +", ENTER)
        .expect("'(2+3ⅈ)+(3∡4°)'");
    test(CLEAR, "'2+3ⅈ' '3∡4' -", ENTER)
        .expect("'(2+3ⅈ)-(3∡4°)'");

    step("Do not promote symbols to complex");
    test(CLEAR, "2+3ⅈ 'A' +", ENTER)
        .expect("'(2+3ⅈ)+A'");

    step("Complex expression involving constants")
        .test(CLEAR, LSHIFT, I, F2, F2, F3, F1, MUL, ID_pow)
        .expect("'e↑(ⅈ·π)'")
        .test(LSHIFT, KEY1)
        .expect("-1.");
}


void tests::complex_functions()
// ----------------------------------------------------------------------------
//   Complex functions
// ----------------------------------------------------------------------------
{
    BEGIN(cfunctions);

    step("Select 34-digit precision to match Intel Decimal 128");
    test(CLEAR, "34 PRECISION 20 SIG", ENTER).noerror();

    step("Using radians");
    test(CLEAR, "RAD", ENTER).noerror();

    step("Square root (optimized negative case)");
    test(CLEAR, "-1ⅈ0", ENTER, SQRT).expect("0+1.ⅈ");
    test(CLEAR, "-4ⅈ0", ENTER, SQRT).expect("0+2.ⅈ");

    step("Square root (optimized positive case)");
    test(CLEAR, "1ⅈ0", ENTER, SQRT).expect("1.+0ⅈ");
    test(CLEAR, "4ⅈ0", ENTER, SQRT).expect("2.+0ⅈ");

    step("Square root (disable optimization for symbols)");
    test(CLEAR, "aⅈ0", ENTER, SQRT).expect("'√((a⊿0+a)÷2)'+'√((a⊿0-a)÷2)'ⅈ");

    step("Square");
    test(CLEAR, "1+2ⅈ", ENTER, SHIFT, SQRT).expect("-3+4ⅈ");

    step("Square root");
    test(SQRT).expect("1.+2.ⅈ");

    step("Negate");
    test(CLEAR, "1+2ⅈ", ENTER, CHS).expect("-1-2ⅈ");
    test(CHS).expect("1+2ⅈ");

    step("Invert");
    test(CLEAR, "3+7ⅈ", ENTER, INV).expect("³/₅₈-⁷/₅₈ⅈ");
    test("58", MUL).expect("3-7ⅈ");
    test(INV).expect("³/₅₈+⁷/₅₈ⅈ");

    step("Symbolic sqrt");
    test(CLEAR, "aⅈb", ENTER, SQRT)
        .expect("'√((a⊿b+a)÷2)'+'sign (√((a⊿b-a)÷2))·√((a⊿b-a)÷2)'ⅈ");

    step("Symbolic sqrt in polar form");
    test(CLEAR, "a∡b", ENTER, SQRT)
        .expect("'√ a'∡'b÷2'");

    step("Cubed");
    test(CLEAR, "3+7ⅈ", ENTER, "cubed", ENTER)
        .expect("-414-154ⅈ");
    step("Cube root");
    test("cbrt", ENTER)
        .expect("7.61577 31058 63908 2857∡-0.92849 05618 83382 29639ʳ");

    step("Logarithm");
    test(CLEAR, "12+14ⅈ", ID_ln)
        .expect("2.91447 28088 05103 5368+0.86217 00546 67226 34884ⅈ");
    step("Exponential");
    test(ID_exp)
        .expect("18.43908 89145 85774 62∡0.86217 00546 67226 34884ʳ");

    step("Power");
    test(CLEAR, "3+7ⅈ", ENTER, "2-3ⅈ", ID_pow)
        .expect("1 916.30979 15541 96293 8∡2.52432 98723 79583 8639ʳ");

    step("Sine");
    test(CLEAR, "4+2ⅈ", ID_sin)
        .expect("-2.84723 90868 48827 8827-2.37067 41693 52001 6145ⅈ");

    step("Cosine");
    test(CLEAR, "3+11ⅈ", ID_cos)
        .expect("-29 637.47552 74860 62145-4 224.71967 95347 02126ⅈ");

    step("Tangent");
    test(CLEAR, "2+1ⅈ", ID_tan)
        .expect("-0.24345 82011 85725 2527+1.16673 62572 40919 8818ⅈ");

    step("Arc sine");
    test(CLEAR, "3+5ⅈ", ID_asin)
        .expect("0.53399 90695 94168 61164+2.45983 15216 23434 5129ⅈ");

    step("Arc cosine");
    test(CLEAR, "7+11ⅈ", ID_acos)
        .expect("1.00539 67973 35154 2326-3.26167 13063 80062 6275ⅈ");

    step("Arc tangent");
    test(CLEAR, "9.+2ⅈ", ID_atan)
        .expect("1.46524 96601 83523 3458+0.02327 26057 66502 98838ⅈ");

    step("Hyperbolic sine");
    test(CLEAR, "4+2ⅈ", ID_HyperbolicMenu, ID_sinh)
        .expect("-11.35661 27112 18172 906+24.83130 58489 46379 372ⅈ");

    step("Hyperbolic cosine");
    test(CLEAR, "3+11ⅈ", ID_HyperbolicMenu, ID_cosh)
        .expect("0.04455 64314 39089 01653-10.01777 68178 59741 201ⅈ");

    step("Hyperbolic tangent");
    test(CLEAR, "2+8ⅈ", ID_HyperbolicMenu, ID_tanh)
        .expect("1.03564 79469 63237 6354-0.01092 58843 35752 53196ⅈ");

    step("Hyperbolic arc sine");
    test(CLEAR, "3+5ⅈ", ID_HyperbolicMenu, ID_asinh)
        .expect("2.45291 37425 02811 7695+1.02382 17465 11782 9101ⅈ");

    step("Hyperbolic arc cosine");
    test(CLEAR, "7+11ⅈ", ID_HyperbolicMenu, ID_acosh)
        .expect("3.26167 13063 80062 6275+1.00539 67973 35154 2326ⅈ");

    step("Hyperbolic arc tangent");
    test(CLEAR, "9.+2ⅈ", ID_HyperbolicMenu, ID_atanh)
        .expect("0.10622 07984 91316 49131+1.54700 47751 56404 9213ⅈ");

    step("Real to complex");
    test(CLEAR, "1 2", ID_ComplexMenu, ID_RealToRectangular)
        .type(ID_rectangular).expect("1+2ⅈ");
    step("Symbolic real to complex");
    test(CLEAR, "a b", ID_ComplexMenu, ID_RealToRectangular)
        .type(ID_rectangular).expect("'a'+'b'ⅈ");

    step("Complex to real");
    test(CLEAR, "1+2ⅈ",  ID_ComplexMenu, ID_RectangularToReal)
        .expect("im:2").test(BSP).expect("re:1");
    step("Symbolic complex to real");
    test(CLEAR, "a+bⅈ", ID_ComplexMenu, ID_RectangularToReal)
        .expect("im:b").test(BSP).expect("re:a");

    step("Re function");
    test(CLEAR, "33+22ⅈ", ID_re).expect("33");
    step("Symbolic Re function");
    test(CLEAR, "a+bⅈ", ID_re).expect("a");
    step("Re function on integers");
    test(CLEAR, "31", ID_re).expect("31");
    step("Re function on decimal");
    test(CLEAR, "31.234 Re", ENTER).expect("31.234");

    step("Im function");
    test(CLEAR, "33+22ⅈ Im", ENTER).expect("22");
    step("Symbolic Im function");
    test(CLEAR, "a+bⅈ", ID_im).expect("b");
    step("Im function on integers");
    test(CLEAR, "31 Im", ENTER).expect("0");
    step("Im function on decimal");
    test(CLEAR, "31.234 Im", ENTER).expect("0");

    step("Complex modulus");
    test(CLEAR, "3+4ⅈ abs", ENTER).expect("5.");
    step("Symbolic complex modulus");
    test(CLEAR, "a+bⅈ abs", ENTER).expect("'a⊿b'");
    step("Norm alias");
    test(CLEAR, "3+4ⅈ norm", ENTER).expect("5.");
    test(CLEAR, "a+bⅈ norm", ENTER).expect("'a⊿b'");
    step("Modulus alias");
    test(CLEAR, "3+4ⅈ modulus", ENTER).expect("5.");
    test(CLEAR, "a+bⅈ modulus", ENTER).expect("'a⊿b'");

    step("Complex conjugate");
    test(CLEAR, "3+4ⅈ conj", ENTER).expect("3-4ⅈ");
    step("Symbolic complex conjugate");
    test(CLEAR, "a+bⅈ conj", ENTER).expect("a+'-b'ⅈ");
    step("Complex conjugate on integers");
    test(CLEAR, "31 conj", ENTER).expect("31");
    step("Complex conjugate on decimals");
    test(CLEAR, "31.234 conj", ENTER).expect("31.234");

    step("Complex argument");
    test(CLEAR, "1+1ⅈ arg", ENTER).expect("0.78539 81633 97448 30962 r");
    step("Symbolic complex argument");
    test(CLEAR, "a+bⅈ arg", ENTER).expect("'b∠a'");
    step("Complex argument on integers");
    test(CLEAR, "31 arg", ENTER).expect("0 r");
    step("Complex argument on decimals");
    test(CLEAR, "31.234 arg", ENTER).expect("0 r");
    step("Complex argument on negative integers");
    test(CLEAR, "-31 arg", ENTER).expect("3.14159 26535 89793 2385 r");
    step("Complex argument on negative decimals");
    test(CLEAR, "-31.234 arg", ENTER).expect("3.14159 26535 89793 2385 r");

    step("Complex argument in degrees");
    test(CLEAR, "DEG", ENTER);
    test(CLEAR, "1+1ⅈ arg", ENTER).expect("45 °");
    step("Symbolic complex argument in degrees");
    test(CLEAR, "a+bⅈ arg", ENTER).expect("'b∠a'");
    step("Complex argument on integers in degrees");
    test(CLEAR, "31 arg", ENTER).expect("0 °");
    step("Complex argument on decimals in degrees");
    test(CLEAR, "31.234 arg", ENTER).expect("0 °");
    test(CLEAR, "-31 arg", ENTER).expect("180 °");
    step("Complex argument on decimals in degrees");
    test(CLEAR, "-31.234 arg", ENTER).expect("180 °");
    test(CLEAR, "RAD", ENTER);

    step("Restore default 24-digit precision");
    test(CLEAR, "24 PRECISION 12 SIG", ENTER).noerror();
}


void tests::complex_promotion()
// ----------------------------------------------------------------------------
//   Complex promotion for real input, e.g. sqrt(-1)
// ----------------------------------------------------------------------------
{
    BEGIN(autocplx);

    step("Using degrees");
    test(CLEAR, "DEG", ENTER).noerror();

    step("Disable complex mode")
        .test(CLEAR, "RealResults", ENTER).noerror()
        .test("-103 FS?", ENTER).expect("False");

    step("sqrt(-1) fails in real mode")
        .test(CLEAR, "-1 sqrt", ENTER).error("Argument outside domain");
    step("asin(-2) fails in real mode")
        .test(CLEAR, "-2 asin", ENTER).error("Argument outside domain");
    step("acos(-2) fails in real mode")
        .test(CLEAR, "-2 acos", ENTER).error("Argument outside domain");
    step("asin(-2) fails in real mode")
        .test(CLEAR, "-2 asin", ENTER).error("Argument outside domain");
    step("asin(-2) fails in real mode")
        .test(CLEAR, "-2 asin", ENTER).error("Argument outside domain");
    step("atanh(-2) fails in real mode")
        .test(CLEAR, "-2 atanh", ENTER).error("Argument outside domain");
    step("log(-2) fails in real mode")
        .test(CLEAR, "-2 log", ENTER).error("Argument outside domain");

    step("Enable complex mode")
        .test(CLEAR, "ComplexResults", ENTER).noerror()
        .test("-103 FS?", ENTER).expect("True");

    step("sqrt(-1) succeeds in complex mode")
        .test(CLEAR, "-1 sqrt", ENTER)
        .expect("0+1.ⅈ");
    step("asin(-2) succeeds in complex mode")
        .test(CLEAR, "-2 asin", ENTER)
        .expect("-1.57079 63267 9+1.31695 78969 2ⅈ °");
    step("acos(-2) succeeds in complex mode")
        .test(CLEAR, "-2 acos", ENTER)
        .expect("3.14159 26535 9-1.31695 78969 2ⅈ °");
    step("asin(-2) succeeds in complex mode")
        .test(CLEAR, "-2 asin", ENTER)
        .expect("-1.57079 63267 9+1.31695 78969 2ⅈ °");
    step("asin(-2) succeeds in complex mode")
        .test(CLEAR, "-2 asin", ENTER)
        .expect("-1.57079 63267 9+1.31695 78969 2ⅈ °");
    step("atanh(-2) succeeds in complex mode")
        .test(CLEAR, "-2 atanh", ENTER)
        .expect("-0.54930 61443 34+1.57079 63267 9ⅈ");
    step("log(-2) succeeds in complex mode")
        .test(CLEAR, "-2 ln", ENTER)
        .expect("0.69314 71805 6+3.14159 26535 9ⅈ");

    step("Restore complex mode")
        .test(CLEAR, "'ComplexResults' purge", ENTER).noerror()
        .test("-103 FS?", ENTER).expect("False");
}


void tests::range_types()
// ----------------------------------------------------------------------------
//   Ranges (intervals, delta and percentage) data typess
// ----------------------------------------------------------------------------
{
    BEGIN(ranges);

    step("Interval form")
        .test(CLEAR, "1…3", ENTER).type(ID_range).expect("1…3");
    step("Delta form")
        .test(CLEAR, "1±3", ENTER).type(ID_drange).expect("1±3");
    step("Percentage form")
        .test(CLEAR, "1±300%", ENTER).type(ID_prange).expect("1±300%");
    step("Uncertain (sigma at end)")
        .test(CLEAR, "1±3σ", ENTER).type(ID_uncertain).expect("1±3σ");
    step("Uncertain (sigma at beginning)")
        .test(CLEAR, "1±σ3", ENTER).type(ID_uncertain).expect("1±3σ");
    step("Uncertain (sigma in the middle)")
        .test(CLEAR, "1σ3", ENTER).type(ID_uncertain).expect("1±3σ");

    step("Cycle")
        .test(CLEAR, "1…3", ENTER).expect("1…3")
        .test(EEX).expect("2±1")
        .test(EEX).expect("2±50%")
        .test(EEX).expect("1…3")
        .test(EEX).expect("2±1")
        .test(EEX).expect("2±50%");

    step("Add intervals")
        .test(CLEAR, "1…3 2…5", NOSHIFT, ADD).expect("3…8");
    step("Subtract intervals")
        .test(CLEAR, "1…3 2…5", NOSHIFT, SUB).expect("-4…1");
    step("Multiply intervals")
        .test(CLEAR, "1…3 2…5", NOSHIFT, MUL).expect("2…15");
    step("Divide intervals")
        .test(CLEAR, "1…3 2…5", NOSHIFT, DIV).expect("¹/₅…1 ¹/₂");
    step("Power intervals")
        .test(CLEAR, "1…3 2…5", NOSHIFT, ID_pow).expect("1.…243.");
    step("Invert intervals")
        .test(CLEAR, "1…3", NOSHIFT, ID_inv).expect("¹/₃…1");
    step("Negate intervals")
        .test(CLEAR, "1…3", ENTER, ID_neg).expect("-3…-1");

    step("Add intervals with promotion")
        .test(CLEAR, "1…3 5", NOSHIFT, ADD).expect("6…8");
    step("Subtract intervals")
        .test(CLEAR, "1…3 5", NOSHIFT, SUB).expect("-4…-2");
    step("Multiply intervals")
        .test(CLEAR, "1…3 5", NOSHIFT, MUL).expect("5…15");
    step("Divide intervals")
        .test(CLEAR, "1…3 5", NOSHIFT, DIV).expect("¹/₅…³/₅");
    step("Power intervals")
        .test(CLEAR, "1…3 5", NOSHIFT, ID_pow).expect("1…243");

    step("Add intervals with promotion")
        .test(CLEAR, "5 1…3", NOSHIFT, ADD).expect("6…8");
    step("Subtract intervals")
        .test(CLEAR, "5 1…3", NOSHIFT, SUB).expect("2…4");
    step("Multiply intervals")
        .test(CLEAR, "5 1…3", NOSHIFT, MUL).expect("5…15");
    step("Divide intervals")
        .test(CLEAR, "5 1…3", NOSHIFT, DIV).expect("1 ²/₃…5");
    step("Power intervals")
        .test(CLEAR, "5 1…3", NOSHIFT, ID_pow).expect("5.…125.");

#define TFNA(name, arg)                                         \
    step(#name " (interval)").test(CLEAR, arg " " #name, ENTER)
#define TFN(name)  TFNA(name, "1…3")

    TFN(sqrt).expect("1.…1.73205 08075 7");
    TFN(sin).expect("0.01745 24064 37…0.05233 59562 43");
    TFNA(sin, "-45…45").expect("-0.70710 67811 87…0.70710 67811 87");
    TFNA(sin, "-45…90").expect("-0.70710 67811 87…1");
    TFNA(sin, "-45…120").expect("-0.70710 67811 87…1");
    TFNA(sin, "-45…180").expect("-0.70710 67811 87…1");
    TFNA(sin, "-45…270").expect("-1…1");
    TFNA(sin, " 45…270").expect("-1…1");
    TFNA(sin, "135…270").expect("-1…0.70710 67811 87");
    TFNA(sin, "150…190").expect("-0.17364 81776 67…¹/₂");
    TFNA(sin, "170…210").expect("-¹/₂…0.17364 81776 67");
    TFNA(sin, "240…280").expect("-1…-0.86602 54037 84");
    TFNA(sin, "260…300").expect("-1…-0.86602 54037 84");
    TFNA(sin, "240…360").expect("-1…0");
    TFNA(sin, "-45…360").expect("-1…1");
    TFNA(sin, "-45…480").expect("-1…1");
    TFNA(sin, "120…480").expect("-1…1");
    TFN(cos).expect("0.99862 95347 55…0.99984 76951 56");
    TFNA(cos, "-45…45").expect("0.70710 67811 87…1");
    TFNA(cos, "-45…90").expect("0…1");
    TFNA(cos, "-45…120").expect("-¹/₂…1");
    TFNA(cos, "-45…180").expect("-1…1");
    TFNA(cos, "-45…270").expect("-1…1");
    TFNA(cos, " 45…270").expect("-1…0.70710 67811 87");
    TFNA(cos, "135…270").expect("-1…0");
    TFNA(cos, "150…190").expect("-1…-0.86602 54037 84");
    TFNA(cos, "170…210").expect("-1…-0.86602 54037 84");
    TFNA(cos, "240…280").expect("-¹/₂…0.17364 81776 67");
    TFNA(cos, "260…300").expect("-0.17364 81776 67…¹/₂");
    TFNA(cos, "240…360").expect("-¹/₂…1");
    TFNA(cos, "-45…360").expect("-1…1");
    TFNA(cos, "-45…480").expect("-1…1");
    TFNA(cos, "120…480").expect("-1…1");
    TFN(tan).expect("0.01745 50649 28…0.05240 77792 83");
    TFNA(tan, "-45…45").expect("-1…1");
    TFNA(tan, "-45…90").expect("−∞…∞");
    TFNA(tan, "-45…120").expect("−∞…∞");
    TFNA(tan, "-45…180").expect("−∞…∞");
    TFNA(tan, "-45…270").expect("−∞…∞");
    TFNA(tan, " 45…270").expect("−∞…∞");
    TFNA(tan, "135…270").expect("−∞…∞");
    TFNA(tan, "150…190").expect("-0.57735 02691 9…0.17632 69807 08");
    TFNA(tan, "170…210").expect("-0.17632 69807 08…0.57735 02691 9");
    TFNA(tan, "240…280").expect("−∞…∞");
    TFNA(tan, "260…300").expect("−∞…∞");
    TFNA(tan, "240…360").expect("−∞…∞");
    TFNA(tan, "-45…360").expect("−∞…∞");
    TFNA(tan, "-45…480").expect("−∞…∞");
    TFNA(tan, "120…480").expect("−∞…∞");
    TFNA(asin, "0.25…0.5").expect("14.47751 21859 °…30. °");
    TFNA(acos, "0.25…0.5").expect("60. °…75.52248 78141 °");
    TFN(atan).expect("45. °…71.56505 11771 °");
    TFN(sinh).expect("1.17520 11936 4…10.01787 49274");
    TFN(cosh).expect("1.54308 06348 2…10.06766 19958");
    TFN(tanh).expect("0.76159 41559 56…0.99505 47536 87");
    TFN(asinh).expect("0.88137 35870 2…1.81844 64592 3");
    TFNA(acosh, "1.321…1.325").expect("0.78123 02051 96…0.78584 80192 36");
    TFNA(atanh, "0.321…0.325").expect("0.33276 15884 82…0.33722 75237 74");
    TFN(ln1p).expect("0.69314 71805 6…1.38629 43611 2");
    TFN(lnp1).expect("0.69314 71805 6…1.38629 43611 2");
    TFN(expm1).expect("1.71828 18284 6…19.08553 69232");
    TFN(ln).expect("0.…1.09861 22886 7");
    TFN(log10).expect("0.…0.47712 12547 2");
    TFN(exp).expect("2.71828 18284 6…20.08553 69232");
    TFN(exp10).expect("10.…1 000.");
    TFN(exp2).expect("2.…8.");
    TFN(erf).expect("0.84270 07929 5…0.99997 79095 03");
    TFN(erfc).expect("0.00002 20904 97…0.15729 92070 5");
    TFN(tgamma).expect("1.…2.");
    TFN(lgamma).expect("0.…0.69314 71805 6");
    TFN(gamma).expect("1.…2.");
    TFN(cbrt).expect("1.…1.44224 95703 1");
    TFN(norm).expect("1…3");
#undef TFN
#undef TFNA

    step("Add delta ranges")
        .test(CLEAR, "1±3 2±5", NOSHIFT, ADD).expect("3±8");
    step("Subtract delta ranges")
        .test(CLEAR, "1±3 2±5", NOSHIFT, SUB).expect("-1±8");
    step("Multiply delta ranges")
        .test(CLEAR, "1±3 2±5", NOSHIFT, MUL).expect("7±21");
    step("Divide delta ranges")
        .test(CLEAR, "10±3 20±5", NOSHIFT, DIV).expect("⁴³/₇₅±²²/₇₅")
        .test(CLEAR, "1±3 2±5", NOSHIFT, DIV).expect("−∞…∞");
    step("Power delta ranges")
        .test(CLEAR, "2±1 5±2", NOSHIFT, ID_pow).expect("1 094.±1 093.");
    step("Invert delta ranges")
        .test(CLEAR, "10±3", NOSHIFT, ID_inv).expect("¹⁰/₉₁±³/₉₁")
        .test(CLEAR, "1±3", NOSHIFT, ID_inv).expect("−∞…∞");
    step("Negate delta ranges")
        .test(CLEAR, "1±3", ENTER, ID_neg).expect("-1±3");

    step("Add delta ranges with promotion")
        .test(CLEAR, "1±3 5", NOSHIFT, ADD).expect("6±3");
    step("Subtract delta ranges")
        .test(CLEAR, "1±3 5", NOSHIFT, SUB).expect("-4±3");
    step("Multiply delta ranges")
        .test(CLEAR, "1±3 5", NOSHIFT, MUL).expect("5±15");
    step("Divide delta ranges")
        .test(CLEAR, "1±3 5", NOSHIFT, DIV).expect("¹/₅±³/₅");
    step("Power delta ranges")
        .test(CLEAR, "1±3 5", NOSHIFT, ID_pow).expect("256±768");

    step("Add delta ranges with promotion")
        .test(CLEAR, "5 1±3", NOSHIFT, ADD).expect("6±3");
    step("Subtract delta ranges")
        .test(CLEAR, "5 1±3", NOSHIFT, SUB).expect("4±3");
    step("Multiply delta ranges")
        .test(CLEAR, "5 1±3", NOSHIFT, MUL).expect("5±15");
    step("Divide delta ranges")
        .test(CLEAR, "5 10±3", NOSHIFT, DIV).expect("⁵⁰/₉₁±¹⁵/₉₁")
        .test(CLEAR, "5 1±3", NOSHIFT, DIV).expect("−∞…∞");
    step("Power delta ranges")
        .test(CLEAR, "5 1±3", NOSHIFT, ID_pow).expect("312.52±312.48");

#define TFNA(name, arg)                                 \
    step(#name " (delta range)").test(CLEAR, arg " " #name, ENTER)
#define TFN(name)  TFNA(name, "1±3")

    TFNA(sqrt, "3±1").expect("1.70710 67811 9±0.29289 32188 13");
    TFN(sin).expect("0.01742 84885 21±0.05232 79852 23");
    TFN(cos).expect("0.99878 20251 3±0.00121 79748 7");
    TFN(tan).expect("0.01750 30212 26±0.05242 37907 18");
    TFNA(tan, "0±45").expect("0±1");
    TFNA(tan, "30±60").expect("−∞…∞");
    TFNA(asin, "0.25±0.1").expect("14.55712 08367 °±5.93019 42780 2 °");
    TFNA(acos, "0.25±0.1").expect("75.44287 91633 °±5.93019 42780 2 °");
    TFN(atan).expect("6.26440 38545 8 °±69.69935 26775 °");
    TFN(sinh).expect("11.83152 83946±15.45838 88025");
    TFN(cosh).expect("14.15411 6418±13.15411 6418");
    TFN(tanh).expect("0.01765 08598 32±0.98167 84399 07");
    TFN(asinh).expect("0.32553 85360 41±1.76917 40112 2");
    TFNA(acosh, "1.321±0.025").expect("0.78058 71062 93±0.02898 78937 9");
    TFNA(atanh, "0.321±0.025").expect("0.33301 11698 75±0.02788 14094 98");
    TFNA(ln1p, "3±1").expect("1.35402 51005 5±0.25541 28118 83");
    TFNA(lnp1, "3±1").expect("1.35402 51005 5±0.25541 28118 83");
    TFN(expm1).expect("26.36674 26582±27.23140 7375");
    TFNA(ln, "3±1").expect("1.03972 07708 4±0.34657 35902 8");
    TFNA(log10, "3±1").expect("0.45154 49934 96±0.15051 49978 32");
    TFN(exp).expect("27.36674 26582±27.23140 7375");
    TFN(exp10).expect("5 000.005±4 999.995");
    TFN(exp2).expect("8.125±7.875");
    TFN(erf).expect("0.00233 88597 82±0.99766 11248 01");
    TFN(erfc).expect("0.99766 11402 18±0.99766 11248 01");
    TFNA(tgamma, "1.321±0.025").expect("0.89482 58872 58±-0.00326 05508 06");
    TFNA(lgamma, "1.321±0.025").expect("-0.11113 27576 28±-0.00364 37985 12");
    TFNA(gamma, "1.321±0.025").expect("0.89482 58872 58±-0.00326 05508 06");
    TFN(cbrt).expect("0.16374 00010 37±1.42366 10509 3");
    TFN(norm).expect("2±2");
#undef TFN
#undef TFNA

    step("Add percent ranges")
        .test(CLEAR, "1±3% 2±5%", NOSHIFT, ADD).expect("3±4 ¹/₃%");
    step("Subtract percent ranges")
        .test(CLEAR, "1±3% 2±5%", NOSHIFT, SUB).expect("-1±13%");
    step("Multiply percent ranges")
        .test(CLEAR, "1±3% 2±5%", NOSHIFT, MUL).expect("2 ³/₁ ₀₀₀±7 ¹ ⁹⁷⁹/₂ ₀₀₃%");
    step("Divide percent ranges")
        .test(CLEAR, "1±3% 2±5%", NOSHIFT, DIV).expect("² ⁰⁰³/₃ ₉₉₀±7 ¹ ⁹⁷⁹/₂ ₀₀₃%");
    step("Power percent ranges")
        .test(CLEAR, "1±3% 2±5%", NOSHIFT, ID_pow).expect("1.00103 94929 8±6.29356 18446 3%");
    step("Invert percent ranges")
        .test(CLEAR, "1±3%", NOSHIFT, ID_inv).expect("1 ⁹/₉ ₉₉₁±3%");
    step("Negate percent ranges")
        .test(CLEAR, "1±3%", ENTER, ID_neg).expect("-1±3%");

    step("Add percent ranges with promotion")
        .test(CLEAR, "1±3% 5", NOSHIFT, ADD).expect("6±¹/₂%");
    step("Subtract percent ranges")
        .test(CLEAR, "1±3% 5", NOSHIFT, SUB).expect("-4±³/₄%");
    step("Multiply percent ranges")
        .test(CLEAR, "1±3% 5", NOSHIFT, MUL).expect("5±3%");
    step("Divide percent ranges")
        .test(CLEAR, "1±3% 5", NOSHIFT, DIV).expect("¹/₅±3%");
    step("Power percent ranges")
        .test(CLEAR, "1±3% 5", NOSHIFT, ID_pow).expect("1 ¹⁸⁰ ⁰⁸¹/₂₀ ₀₀₀ ₀₀₀±14 ⁹⁰ ⁰⁹⁴ ⁵⁷³/₁₀₀ ₉₀₀ ₄₀₅%");

    step("Add percent ranges with promotion")
        .test(CLEAR, "5 1±3%", NOSHIFT, ADD).expect("6±¹/₂%");
    step("Subtract percent ranges")
        .test(CLEAR, "5 1±3%", NOSHIFT, SUB).expect("4±³/₄%");
    step("Multiply percent ranges")
        .test(CLEAR, "5 1±3%", NOSHIFT, MUL).expect("5±3%");
    step("Divide percent ranges")
        .test(CLEAR, "5 1±3%", NOSHIFT, DIV).expect("5 ⁴⁵/₉ ₉₉₁±3%");
    step("Power percent ranges")
        .test(CLEAR, "5 1±3%", NOSHIFT, ID_pow).expect("5.00582 92857 2±4.82456 52123 7%");

#define TFNA(name, arg)                                 \
    step(#name " (percent range)").test(CLEAR, arg " " #name, ENTER)
#define TFN(name)  TFNA(name, "1±3%")

    TFN(sqrt).expect("0.99988 74683 44±1.50033 76519 6%");
    TFN(sin).expect("0.01745 24040 45±2.99969 56505 2%");
    TFN(cos).expect("0.99984 75580 99±0.00091 39451 46%");
    TFN(tan).expect("0.01745 50697 15±3.00060 87730 3%");
    TFNA(tan, "0±4500%").expect("0±100%");
    TFNA(tan, "30±200%").expect("−∞…∞");
    TFNA(asin, "0.25±0.1%").expect("14.47751 26791 °±0.10218 40366%");
    TFNA(acos, "0.25±0.1%").expect("75.52248 73209 °±0.01958 84793 78%");
    TFN(atan).expect("44.98710 84505 °±1.91069 30918 4%");
    TFN(sinh).expect("1.17573 00738 5±3.93792 45500 1%");
    TFN(cosh).expect("1.54377 50731 8±2.28409 72797 9%");
    TFN(tanh).expect("0.76130 63134 13±1.65531 61613 5%");
    TFN(asinh).expect("0.88121 44969 49±2.40735 92360 4%");
    TFNA(acosh, "1.321±0.025%").expect("0.78123 00931 78±0.04897 49317 72%");
    TFNA(atanh, "0.321±0.025%").expect("0.33276 15910 51±0.02688 68087 85%");
    TFN(ln1p).expect("0.69303 46679 02±2.16455 62403 6%");
    TFN(lnp1).expect("0.69303 46679 02±2.16455 62403 6%");
    TFN(expm1).expect("1.71950 51470 3±4.74326 51081 9%");
    TFN(ln).expect("-0.00045 02026 22±6 665.66639 654%");
    TFN(log10).expect("-0.00019 55205 14±6 665.66639 654%");
    TFN(exp).expect("2.71950 51470 3±2.99910 03238 8%");
    TFN(exp10).expect("10.02386 80302±6.89678 89453 7%");
    TFN(exp2).expect("2.00043 24232 9±2.07914 18713 2%");
    TFN(erf).expect("0.84232 72522 45±1.47887 40570 9%");
    TFN(erfc).expect("0.15767 27477 55±7.90051 50773 4%");
    TFN(tgamma).expect("1.00089 09463 2±1.73255 59622 8%");
    TFN(lgamma).expect("0.00074 04396 24±2 340.13590 506%");
    TFN(gamma).expect("1.00089 09463 2±1.73255 59622 8%");
    TFN(cbrt).expect("0.99989 99666 5±1.00026 68000 8%");
    TFN(norm).expect("1±3%");
#undef TFN
#undef TFNA

    step("Exploding range objects")
        .test(CLEAR, "1…3", ID_ObjectMenu, ID_Explode)
        .got("3", "1");
    step("Exploding delta range objects")
        .test(CLEAR, "1±3", ID_ObjectMenu, ID_Explode)
        .got("4", "-2");
    step("Exploding percent range objects")
        .test(CLEAR, "1±200%", ID_ObjectMenu, ID_Explode)
        .got("3", "-1");
}


void tests::units_and_conversions()
// ----------------------------------------------------------------------------
//   Unit types and data conversions
// ----------------------------------------------------------------------------
{
    BEGIN(units);

    step("Entering unit from command-line")
        .test(CLEAR, "1_kg", ENTER)
        .type(ID_unit)
        .expect("1 kg");
    step("Unit symbol from unit menu")
        .test(CLEAR, SHIFT, KEY5, KEY1, F1, LOWERCASE, M, S, ENTER)
        .type(ID_unit)
        .expect("1 ms");
    step("Unit symbol division from unit menu")
        .test(CLEAR, SHIFT, KEY5, KEY1, F1, LOWERCASE, M, SHIFT, DIV, S, ENTER)
        .type(ID_unit)
        .expect("1 m/s");
    step("Unit symbol multiplication from unit menu")
        .test(CLEAR, SHIFT, KEY5, KEY1, F1, LOWERCASE, M, SHIFT, MUL, S, ENTER)
        .type(ID_unit)
        .expect("1 m·s");
    step("Insert unit with soft key")
        .test(CLEAR, SHIFT, KEY5, KEY1, F2, F1)
        .editor("1_in")
        .test(ENTER)
        .type(ID_unit)
        .expect("1 in");
    step("Convert integer unit with soft key")
        .test(SHIFT, F2)
        .type(ID_unit)
        .expect("25 ²/₅ mm");
    step("Convert decimal unit with soft key")
        .test(CLEAR, KEY2, DOT, F1, ENTER, SHIFT, F2)
        .type(ID_unit)
        .expect("50.8 mm");
    step("Do not apply simplifications for unit conversions")
        .test(CLEAR, KEY1, DOT, F1, ENTER, SHIFT, F2)
        .type(ID_unit)
        .expect("25.4 mm");
    step("Multiply by unit using softkey")
        .test(CLEAR, SHIFT, KEY5, KEY1, F2, F1, F2)
        .editor("1_in·mm")
        .test(ENTER)
        .type(ID_unit)
        .expect("1 in·mm");
    step("Divide by unit using softkey")
        .test(CLEAR, SHIFT, KEY5, KEY1, F2, F1, RSHIFT, F2)
        .editor("1_in/(mm)")
        .test(ENTER)
        .type(ID_unit)
        .expect("1 in/mm");
    step("Conversion across compound units")
        .test(CLEAR, SHIFT, KEY5, KEY1, F2, F3)
        .editor("1_km/h")
        .test(ENTER)
        .type(ID_unit).expect("1 km/h")
        .test(SHIFT, F4).type(ID_unit).expect("¹⁵ ⁶²⁵/₂₅ ₁₄₆ mph")
        .test(SHIFT, F3).type(ID_unit).expect("1 km/h");
    step("Conversion to base units")
        .test(ENTER, RSHIFT, KEY5, F2)
        .type(ID_unit).expect("⁵/₁₈ m/s");
    step("Extract value from unit object")
        .test(ENTER, F3)
        .expect("⁵/₁₈");
    step("Split unit object")
        .test(BSP, ID_ObjectMenu, ID_Explode).expect("'m÷s'")
        .test(BSP).expect("⁵/₁₈");
    step("Convert operation")
        .test(CLEAR, KEY1, SHIFT, KEY5, F2, F3)
        .editor("1_km/h")
        .test(ENTER)
        .type(ID_unit).expect("1 km/h")
        .test(KEY1, F1, SHIFT, KEY5, SHIFT, F1, RSHIFT, F2)
        .editor("1_in/(min)")
        .test(ENTER)
        .type(ID_unit).expect("1 in/min")
        .test(RSHIFT, KEY5, F1) // Convert
        .type(ID_unit).expect("656 ⁶⁴/₃₈₁ in/min");
    step("Convert to unit")
        .test(CLEAR, KEY3, KEY7, ENTER).expect("37")
        .test(LSHIFT, KEY5, KEY4, KEY2, F2, F3).editor("42_km/h")
        .test(ENTER).expect("42 km/h")
        .test(RSHIFT, KEY5, F5).expect("37 km/h");
    step("Factoring out a unit")
        .test(CLEAR, KEY3, SHIFT, KEY5, SHIFT, F6, F2, ENTER).expect("3 kW")
        .test(KEY1, SHIFT, KEY5, SHIFT, F4, F1, ENTER).expect("1 N")
        .test(RSHIFT, KEY5, F4).expect("3 000 N·m/s");
    step("Orders of magnitude")
        .test(CLEAR, KEY3, SHIFT, KEY5, SHIFT, F6, F2, ENTER).expect("3 kW")
        .test(RSHIFT, KEY5, SHIFT, F2).expect("300 000 cW")
        .test(LSHIFT, F3).expect("3 kW")
        .test(LSHIFT, F4).expect("³/₁ ₀₀₀ MW");
    step("Unit simplification (same unit)")
        .test(CLEAR, KEY3, SHIFT, KEY5, SHIFT, F6, F2, ENTER).expect("3 kW")
        .test(SHIFT, KEY5, SHIFT, F4, F1).expect("3 kW·N")
        .test(SHIFT, KEY5, SHIFT, F6, RSHIFT, F2, ENTER).expect("3 N");
    step("Arithmetic on units")
        .test(CLEAR, KEY3, KEY7, SHIFT, KEY5, F2, F4, ENTER).expect("37 mph")
        .test(SHIFT, KEY5, KEY4, KEY2, F2, F3, ENTER).expect("42 km/h")
        .test(ADD).expect("101 ⁸ ⁵²⁷/₁₅ ₆₂₅ km/h");
    step("Arithmetic on units (decimal)")
        .test(CLEAR, KEY3, KEY7, DOT, SHIFT, KEY5, F2, F4).editor("37._mph")
        .test(ENTER).expect("37. mph")
        .test(SHIFT, KEY5, KEY4, KEY2, F2, F3, ENTER).expect("42 km/h")
        .test(ADD).expect("101.54572 8 km/h");
    step("Unit parsing on command line")
        .test(CLEAR, "12_km/s^2", ENTER).expect("12 km/s↑2");
    step("Parsing degrees as a unit")
        .test(CLEAR, "DEG", ENTER).noerror()
        .test("1∡90", ENTER).expect("1∡90°")
        .test(DOWN).editor("1∡90°")
        .test(DOWN, DOWN, BSP, DOWN, DOWN, "_").editor("190_°")
        .test(ENTER).expect("190 °");

    step("Auto-simplification for unit addition")
        .test(CLEAR, "1_s", ENTER, "0", ID_add)
        .expect("1 s");
    step("No auto-simplification for unit subtraction")
        .test(CLEAR, "1_s", ENTER, ENTER, SUB)
        .noerror()
        .expect("0 s");
    step("No auto-simplification for unit multiplication")
        .test(CLEAR, "1_s", ENTER, "1", ID_multiply)
        .noerror()
        .expect("1 s");
    step("No auto-simplification for unit division")
        .test(CLEAR, "1_s", ENTER, "1", ID_divide)
        .noerror()
        .expect("1 s");

    step("Sqrt for units")
        .test(CLEAR, "12_km/h", ENTER, ID_sq).expect("144 km↑2/h↑2")
        .test(C).expect("12. km/h");
    step("Cube root for units")
        .test(CLEAR, "12_km/h", ENTER, "3", ID_pow).expect("1 728 km↑3/h↑3")
        .test("CBRT", ENTER).expect("12. km/h");
    step("xroot for units")
        .test(CLEAR, "12_km/h", ENTER, "3", ID_pow).expect("1 728 km↑3/h↑3")
        .test("3", ID_xroot).expect("12. km/h");

    step("Invalid unit exponent")
        .test(CLEAR, "1_km^s", ENTER).error("Invalid unit expression");
    step("Invalid unit expression for 1_km/(s+N)")
        .test(CLEAR, "1_km/",
              ALPHA, SHIFT, F,
              LOWERCASE, S, ID_add, ALPHA, N,
              ENTER).error("Invalid unit expression")
        .test(CLEAR);
    step("Invalid unit expression for 1_km(s+N)")
        .test(CLEAR, "1_km/",
              ALPHA, SHIFT, F, UP, BSP, DOWN,
              LOWERCASE, S, ID_add, ALPHA, N,
              ENTER).error("Invalid unit expression")
        .test(CLEAR);

    step("Stop parsing units at end of unit expression")
        .test(CLEAR, "'360_°-30_°'", ENTER)
        .expect("'360 °-30 °'")
        .test(RUNSTOP)
        .expect("330 °");

    step("Convert units from Cycle section (#1227)")
        .test(CLEAR, "2_lb 1_kg CONVERT", ENTER)
        .expect("⁴⁵ ³⁵⁹ ²³⁷/₅₀ ₀₀₀ ₀₀₀ kg")
        .test(CLEAR,
              LSHIFT, KEY5,     // Units menu
              LSHIFT, F3,       // Select Mass
              KEY2, F6, F1,     // Enter 2_lb
              LSHIFT, F6,       // Previous page
              LSHIFT, F1)       // Convert to kg
        .expect("⁴⁵ ³⁵⁹ ²³⁷/₅₀ ₀₀₀ ₀₀₀ kg");
    step("Convert units from units configuration file (#1227)")
        .test(CLEAR, "2_EUR 1_GBP CONVERT", ENTER)
        .expect("1.72580 64516 1 GBP")
        .test(CLEAR,
              LSHIFT, KEY5,     // Units menu
              F3,               // Select Mass
              KEY2, F2,         // Enter 2_lb
              LSHIFT, F1)       // Convert to USD
        .expect("2.14 USD");

    step("Temperature conversions forward, simple case")
        .test(CLEAR, "100_°C 1_K CONVERT", ENTER)
        .expect("373.15 K");
    step("Temperature conversions backward, simple case")
        .test(CLEAR, "100_K 1_°C", ID_UnitsConversionsMenu, ID_Convert)
        .expect("-173.15 °C");
    step("Temperature conversions forward, combined case")
        .test(CLEAR, "30_°C 1_°F", ID_UnitsConversionsMenu, ID_Convert)
        .expect("86. °F");
    step("Temperature conversions with scaling operations on units")
        .test(CLEAR, "100_°C/s 1_K/min", ID_UnitsConversionsMenu, ID_Convert)
        .error("Inconsistent units");
    step("Ubase on non-scaling units")
        .test(CLEAR, "100_°C UBASE", ENTER)
        .expect("373.15 K");
    step("Ubase on non-scaling units with scaling request")
        .test(CLEAR, "100_°C/s", ID_UnitsConversionsMenu, ID_UBase)
        .error("Inconsistent units");
    step("Ubase in arithmetic expression (#1321)")
        .test(CLEAR, "'UBASE(100_km)'", ENTER)
        .expect("'BaseUnits 100 km'")
        .test(ID_Run)
        .expect("100 000 m");
    step("Ubase on numerical values (#1322)")
        .test(CLEAR, "'ubase(1)'", ENTER)
        .expect("'BaseUnits 1'")
        .test(ID_Run)
        .expect ("1")
        .test(CLEAR, "1 ubase", ENTER)
        .expect("1");

    step("Convert when evaluation is needed")
        .test(CLEAR,
              "α=0.00729735256434 Urα=1.6E-10 "
              "μ0='CONVERT(4*Ⓒπ*α*Ⓒℏ/(Ⓒqe^2*Ⓒc);1_H/m)' "
              "Urμ0='Urα' "
              "Usμ0='ROUND(Urα*μ0;-2)'", ENTER)
        .got("Usμ0='Round(Urα·μ0;-2)'",
             "Urμ0='Urα'",
             "μ0='Convert(4·π·α·ℏ÷(qe↑2·c);1 H/m)'",
             "Urα=1.6⁳⁻¹⁰",
             "α=0.00729 73525 64")
        .test(CLEAR, "Usμ0", ENTER)
        .expect("2.⁳⁻¹⁶ H/m");

    step("Convert arguments to add")
        .test(CLEAR,
              DIRECT("'1_m/s+(1_A)÷((8.5⁳28_(m↑3)⁻¹)·Ⓒqe·Ⓒπ·(0.01_cm↑2))'"),
              ENTER)
        .expect("'1 m/s+1 A÷(8.5⁳²⁸ (m↑3)⁻¹·qe·π·0.01 cm↑2)'")
        .test(ID_Run)
        .expect("0.00010 00023 37 A·m↑3/(C·cm↑2)");
    step("Convert arguments to sub")
        .test(CLEAR,
              DIRECT("'1_m/s-(1_A)÷((8.5⁳28_(m↑3)⁻¹)·Ⓒqe·Ⓒπ·(0.01_cm↑2))'"),
              ENTER)
        .expect("'1 m/s-1 A÷(8.5⁳²⁸ (m↑3)⁻¹·qe·π·0.01 cm↑2)'")
        .test(ID_Run)
        .expect("0.00009 99976 63 A·m↑3/(C·cm↑2)");

    step("Convert dimensionless argument to add")
        .test(CLEAR, "'1_km/in+3'", ENTER, ID_Run)
        .expect("39 373 ¹⁰/₁₂₇");
    step("Convert dimensionless argument to add")
        .test(CLEAR, "'3+1_km/in'", ENTER, ID_Run)
        .expect("39 373 ¹⁰/₁₂₇");
    step("Convert dimensionless argument to sub")
        .test(CLEAR, "'1_km/in-3'", ENTER, ID_Run)
        .expect("39 367 ¹⁰/₁₂₇");
    step("Convert dimensionless argument to sub")
        .test(CLEAR, "'3-1_km/in'", ENTER, ID_Run)
        .expect("-39 367 ¹⁰/₁₂₇");

    step("UVAL for unit value")
        .test(CLEAR, "1_km", ID_UnitsConversionsMenu, ID_UVal)
        .expect("1")
        .test(CLEAR, "1.23_m/s", ID_UVal)
        .expect("1.23")
        .test(CLEAR, "'UVAL(4.35_km)'", ENTER, ID_Run)
        .expect("4.35");
    step("UVAL for number")
        .test(CLEAR, "123", ID_UVal)
        .expect("123")
        .test(CLEAR, "-1.23", ID_UVal)
        .expect("-1.23")
        .test(CLEAR, "'UVAL(3/4)'", ENTER, ID_Run)
        .expect("³/₄");
}


void tests::list_functions()
// ----------------------------------------------------------------------------
//   Some operations on lists
// ----------------------------------------------------------------------------
{
    BEGIN(lists);

    step("Integer index")
        .test(CLEAR, "{ A B C }", ENTER, "2 GET", ENTER)
        .expect("B");
    step("Real index")
        .test(CLEAR, "{ A B C }", ENTER, "2.3 GET", ENTER)
        .expect("B");
    step("Computed index")
        .test(CLEAR, "{ A B C }", ENTER, "'1.3+1' GET", ENTER)
        .expect("B");
    step("Computed index with variable")
        .test(CLEAR, "{ A B C } 1.3 'Z' STO", ENTER, "'Z+1' GET", ENTER)
        .expect("B")
        .test(CLEAR, "'Z' PURGE", ENTER).noerror();
    step("Bad index type")
        .test(CLEAR, "{ A B C }", ENTER, "\"A\" GET", ENTER)
        .error("Bad argument type");
    step("Out-of-range index")
        .test(CLEAR, "{ A B C }", ENTER, "5 GET", ENTER)
        .error("Index out of range");
    step("Empty list index")
        .test(CLEAR, "{ A B C }", ENTER, "{} GET", ENTER)
        .expect("{ A B C }");
    step("Single element list index")
        .test(CLEAR, "{ A B C }", ENTER, "{2} GET", ENTER)
        .expect("B");
    step("List index nested")
        .test(CLEAR, "{ A {D E F} C }", ENTER, "{2 3} GET", ENTER)
        .expect("F");
    step("List index, too many items")
        .test(CLEAR, "{ A B C }", ENTER, "{2 3} GET", ENTER)
        .error("Bad argument type");
    step("Character from array")
        .test(CLEAR, "\"Hello World\"", ENTER, "2 GET", ENTER)
        .expect("\"e\"");
    step("Deep nesting");
    test(CLEAR, "{ A { D E { 1 2 \"Hello World\" } F } 2 3 }", ENTER,
         "{ 2 3 3 5 } GET", ENTER)
        .expect("\"o\"");

    step("Incrementing integer index")
        .test(CLEAR,
              "{ A B C }", ENTER, "2 ")
        .test("GETI", ENTER).expect("B").test(BSP)
        .test("GETI", ENTER).expect("C").test(BSP)
        .test("GETI", ENTER).expect("A").test(BSP);

    step("Incrementing decimal index")
        .test(CLEAR,
              "{ A B C }", ENTER, "2.3 ")
        .test("GETI", ENTER).expect("B").test(BSP)
        .test("GETI", ENTER).expect("C").test(BSP)
        .test("GETI", ENTER).expect("A").test(BSP);
    step("Bad index type for GETI")
        .test(CLEAR, "{ A B C }", ENTER, "\"A\" GETI", ENTER)
        .error("Bad argument type");
    step("Out-of-range index for GETI")
        .test(CLEAR, "{ A B C }", ENTER, "5 GETI", ENTER)
        .error("Index out of range");
    step("Empty list index for GETI")
        .test(CLEAR, "{ A B C }", ENTER, "{} GETI", ENTER)
        .error("Bad argument value");
    step("Single element list index for GETI")
        .test(CLEAR, "{ A B C }", ENTER, "{2} ")
        .test("GETI", ENTER).expect("B").test(BSP).expect("{ 3 }")
        .test("GETI", ENTER).expect("C").test(BSP).expect("{ 1 }")
        .test("GETI", ENTER).expect("A").test(BSP).expect("{ 2 }");
    step("List index nested for GETI")
        .test(CLEAR, "{ A {D E F} C }", ENTER, "{2 3} ")
        .test("GETI", ENTER).expect("F").test(BSP).expect("{ 3 1 }")
        .test("GETI", ENTER).error("Bad argument type");
    step("List index, too many items for GETI")
        .test(CLEAR, "{ A B C }", ENTER, "{2 3} GETI", ENTER)
        .error("Bad argument type");
    step("Character from array using GETI")
        .test(CLEAR, "\"Hello\"", ENTER, "2 ")
        .test("GETI", ENTER).expect("\"e\"").test(BSP).expect("3")
        .test("GETI", ENTER).expect("\"l\"").test(BSP).expect("4")
        .test("GETI", ENTER).expect("\"l\"").test(BSP).expect("5")
        .test("GETI", ENTER).expect("\"o\"").test(BSP).expect("1")
        .test("GETI", ENTER).expect("\"H\"").test(BSP).expect("2")
        .test("GETI", ENTER).expect("\"e\"").test(BSP).expect("3");
    step("Deep nesting for GETI");
    test(CLEAR, "{ A { D E { 1 2 \"Hello World\" } F } 2 3 }", ENTER,
         "{ 2 3 3 5 } GETI", ENTER)
        .expect("\"o\"").test(BSP).expect("{ 2 3 3 6 }");

    step("Array indexing");
    test(CLEAR, "[ A [ D E [ 1 2 \"Hello World\" ] F ] 2 3 ]", ENTER,
         "[ 2 3 3 5 ] GET", ENTER)
        .expect("\"o\"");

    step("Variable access with GET")
        .test(CLEAR, "{ 11 22 33 44 } 'L' STO", ENTER).noerror()
        .test("'L' 1 GET", ENTER).expect("11")
        .test("'L' 3 GET", ENTER).expect("33");
    step("Variable access with GETI")
        .test(CLEAR, "'L' 1 GETI", ENTER).expect("11").test(BSP).expect("2")
        .test(ID_ListMenu, F6)
        .test(F3).expect("22").test(BSP).expect("3")
        .test(F3).expect("33").test(BSP).expect("4")
        .test(F3).expect("44").test(BSP).expect("1")
        .test(F3).expect("11").test(BSP).expect("2");

    step("Putting in a list")
        .test(CLEAR, "{ 11 22 33 } 1 55 PUT", ENTER).expect("{ 55 22 33 }")
        .test(ENTER, "2", F1).expect("22")
        .test("3", MUL).expect("66")
        .test("2", NOSHIFT, M, F2).expect("{ 55 66 33 }");
    step("Putting in a list with evaluation")
        .test(CLEAR, "{ 11 22 33 } '3-2' 55 PUT", ENTER).expect("{ 55 22 33 }")
        .test(ENTER, "2", F1).expect("22")
        .test("3", MUL).expect("66")
        .test("'4-2'", NOSHIFT, M, F2).expect("{ 55 66 33 }");
    step("Putting out of range")
        .test(CLEAR, "{ 11 22 33 } 4 55 PUT", ENTER)
        .error("Index out of range");
    step("Incremental put (PUTI)")
        .test(CLEAR, "{ 11 22 33 } 3 55 PUTI", ENTER).expect("1")
        .test(NOSHIFT, M).expect("{ 11 22 55 }")
        .test(NOSHIFT, M, "88", F4).expect("2")
        .test(NOSHIFT, M).expect("{ 88 22 55 }")
        .test(EXIT);
    step("Index error when putting out of range with PUTI")
        .test(CLEAR, "{ 11 22 33 } 5 55 PUTI", ENTER)
        .error("Index out of range");

    step("Concatenation of lists");
    test(CLEAR, "{ A B C D } { F G H I } +", ENTER)
        .expect("{ A B C D F G H I }");
    step("Concatenation of item to list");
    test(CLEAR, "{ A B C D } 2.3 +", ENTER)
        .expect("{ A B C D 2.3 }");
    test(CLEAR, "2.5 { A B C D } +", ENTER)
        .expect("{ 2.5 A B C D }");

    step("Concatenation of list and text");
    test(CLEAR, "{ } \"Hello\" +", ENTER)
        .expect("{ \"Hello\" }");

    step("Repetition of a list");
    test(CLEAR, "{ A B C D } 3 *", ENTER)
        .expect("{ A B C D A B C D A B C D }");
    test(CLEAR, "3 { A B C D } *", ENTER)
        .expect("{ A B C D A B C D A B C D }");

    step("Applying a function to a  list");
    test(CLEAR, "{ A B C } sin", ENTER)
        .expect("{ 'sin A' 'sin B' 'sin C' }");

    step("List sum in program")
        .test(CLEAR, "{ 5 8 2 } ΣList", ENTER).expect("15")
        .test(CLEAR, "{ A B C 1 } ΣList", ENTER).expect("'A+B+C+1'");
    step("List product in program")
        .test(CLEAR, "{ 5 8 2 } ∏List", ENTER).expect("80")
        .test(CLEAR, "{ A B C 1 } ΣList", ENTER).expect("'A+B+C+1'");
    step("List differences in program")
        .test(CLEAR, "{ 4 20 1 17 60 91 } ∆List", ENTER)
        .expect("{ 16 -19 16 43 31 }")
        .test(CLEAR, "{ A B C 1 2 3 } ∆List", ENTER)
        .expect("{ 'B-A' 'C-B' '1-C' 1 1 }");

    step("List sum in menu")
        .test(CLEAR, ID_ListMenu)
        .test(CLEAR, "{ 5 8 2 }", LSHIFT, F3).expect("15")
        .test(CLEAR, "{ A B C 1 }", LSHIFT, F3).expect("'A+B+C+1'");
    step("List product in program")
        .test(CLEAR, "{ 5 8 2 }", LSHIFT, F4).expect("80")
        .test(CLEAR, "{ A B C 1 }", LSHIFT, F4).expect("'A·B·C'");
    step("List differences in program")
        .test(CLEAR, "{ 4 20 1 17 60 91 }", LSHIFT, F5)
        .expect("{ 16 -19 16 43 31 }")
        .test(CLEAR, "{ A B C 1 2 3 }",LSHIFT, F5)
        .expect("{ 'B-A' 'C-B' '1-C' 1 1 }");

    step("DoList with explicit size in program")
        .test(CLEAR, "{ A B 3 } { D 5 6 } { E 8 F } 3 « + * » DOLIST", ENTER)
        .expect("{ 'A·(D+E)' 'B·13' '3·(6+F)' }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoList with explicit size from menu")
        .test(CLEAR, "{ 1 2 3 } { 4 5 6 } { 7 8 9 } 3 « + * »",
              ID_ProbabilitiesMenu, ID_ListMenu, F6, LSHIFT, F1)
        .expect("{ 11 26 45 }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoList with implicit size in program")
        .test(CLEAR, "{ 3 A B } { 5 D 6 } { 8 F E } « + » DOLIST", ENTER)
        .expect("{ 13 'D+F' '6+E' }")
        .test(BSP)
        .expect("{ 3 A B }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoList with implicit size from menu")
        .test(CLEAR, "{ 1 2 3 } { 4 5 6 } { 7 8 9 } « * »",
              ID_ProbabilitiesMenu, ID_ListMenu, F6, LSHIFT, F1)
        .expect("{ 28 40 54 }")
        .test(BSP).expect("{ 1 2 3 }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoList with bad arguments")
        .test(CLEAR, "{ A B 3 } { D 5 6 } { E 8 F }  « + »  DUP DOLIST", ENTER)
        .error("Bad argument type");

    step("DoSubs with explicit size in program")
        .test(CLEAR, "{ A B 3 D 5 6 E 8 F } 3 « + * » DOSUBS", ENTER)
        .expect("{ 'A·(B+3)' 'B·(3+D)' '3·(D+5)' 'D·11' '5·(6+E)' "
                "'6·(E+8)' 'E·(8+F)' }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with explicit size from menu")
        .test(CLEAR, "{ 1 2 3 4 5 6 7 8 9 } 3 « + * »",
              ID_ProbabilitiesMenu, ID_ListMenu, F6, LSHIFT, F2)
        .expect("{ 5 14 27 44 65 90 119 }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with implicit size in program")
        .test(CLEAR, "{ 3 A B 5 D 6 8 F E } « + » DOSUBS", ENTER)
        .expect("{ '3+A' 'A+B' 'B+5' '5+D' 'D+6' 14 '8+F' 'F+E' }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with implicit size in program from HP50G ARM")
        .test(CLEAR, "{ A B C D E } « - » DOSUBS", ENTER)
        .expect("{ 'A-B' 'B-C' 'C-D' 'D-E' }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with explicit size in program from HP50G ARM")
        .test(CLEAR, "{ A B C } 2 « DUP * * » DOSUBS", ENTER)
        .expect("{ 'A·B²' 'B·C²' }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with explicit size in program from HP50G ARM")
        .test(CLEAR,
              "{ 1 2 3 4 5 } "
              "« → a b "
              "« CASE "
              "  'NSUB=1' THEN a END "
              "  'NSUB=ENDSUB' THEN b END "
              "  'a+b' EVAL END » » DOSUBS", ENTER)
        .expect("{ 1 5 7 5 }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with implicit size from menu")
        .test(CLEAR, "{ 1 2 A D 5 6 B 8 9 } « * »",
              ID_ProbabilitiesMenu, ID_ListMenu, F6, LSHIFT, F2)
        .expect("{ 2 '2·A' 'A·D' 'D·5' 30 '6·B' 'B·8' 72 }")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");
    step("DoSubs with bad arguments")
        .test(CLEAR, "{ A B 3 D 5 6 E 8 F }  « + »  DUP DOSUBS", ENTER)
        .error("Bad argument type");

    step("Extract with integer range")
        .test(CLEAR, "{ A B C D E } 2 4", ID_ListMenu, ID_Extract)
        .expect("{ B C D }");
    step("Extract with negative integer range")
        .test(CLEAR, "{ A B C D E } -2 4", ID_ListMenu, ID_Extract)
        .expect("{ A B C D }");
    step("Extract with range outside of size")
        .test(CLEAR, "{ A B C D E } 1 6", ID_ListMenu, ID_Extract)
        .expect("{ A B C D E }");
    step("Extract with one-element range")
        .test(CLEAR, "{ A B C D E } 1 1", ID_ListMenu, ID_Extract)
        .expect("{ A }");
    step("Extract with empty range")
        .test(CLEAR, "{ A B C D E } 2 1", ID_ListMenu, ID_Extract)
        .expect("{ }");
    step("Extract with one-level list range")
        .test(CLEAR, "{ A B C D E } {3} {5}", ID_ListMenu, ID_Extract)
        .expect("{ C D E }");
    step("Extract with two level unexpected list range")
        .test(CLEAR, "{ A B C D E } { 3 4 } { 5 5 }", ID_ListMenu, ID_Extract)
        .error("Invalid dimension");
    step("Extract with two level list range")
        .test(CLEAR, "{ {A B} {C D} {E F} {G} } { 1 2 } { 4 3 }",
              ID_ListMenu, ID_Extract)
        .want("{ { B } { D } { F } { } }");
    step("Extract with unexpected three level list range")
        .test(CLEAR, "{ {A B} {C D} {E F} {G} } { 1 2 3 } { 3 3 5 }",
              ID_ListMenu, ID_Extract)
        .error("Invalid dimension");
    step("Extract with inconsistent range size")
        .test(CLEAR, "{ A B C D E } { 1 6 3 } { 2 4 }",
              ID_ListMenu, ID_Extract)
        .error("Invalid dimension");

    step("Extract array with integer range")
        .test(CLEAR, "[ A B C D E ] 2 4", ID_ListMenu, ID_Extract)
        .expect("[ B C D ]");
    step("Extract array with negative integer range")
        .test(CLEAR, "[ A B C D E ] -2 4", ID_ListMenu, ID_Extract)
        .expect("[ A B C D ]");
    step("Extract array with range outside of size")
        .test(CLEAR, "[ A B C D E ] 1 6", ID_ListMenu, ID_Extract)
        .expect("[ A B C D E ]");
    step("Extract array with one-element range")
        .test(CLEAR, "[ A B C D E ] 1 1", ID_ListMenu, ID_Extract)
        .expect("[ A ]");
    step("Extract array with empty range")
        .test(CLEAR, "[ A B C D E ] 2 1", ID_ListMenu, ID_Extract)
        .expect("[ ]");
    step("Extract array with one-level list range")
        .test(CLEAR, "[ A B C D E ] [3] [5]", ID_ListMenu, ID_Extract)
        .expect("[ C D E ]");
    step("Extract array with two level unexpected list range")
        .test(CLEAR, "[ A B C D E ] [ 3 4 ] [ 5 5 ]", ID_ListMenu, ID_Extract)
        .error("Invalid dimension");
    step("Extract array with two level list range")
        .test(CLEAR, "[ [A B] [C D] [E F] [G] ] [ 1 2 ] [ 4 3 ]",
              ID_ListMenu, ID_Extract)
        .want("[[ B ] [ D ] [ F ] [ ]]");
    step("Extract array with unexpected three level list range")
        .test(CLEAR, "[ [A B] [C D] [E F] [G] ] [ 1 2 3 ] [ 3 3 5 ]",
              ID_ListMenu, ID_Extract)
        .error("Invalid dimension");
    step("Extract array with inconsistent range size")
        .test(CLEAR, "[ A B C D E ] [ 1 6 3 ] [ 2 4 ]",
              ID_ListMenu, ID_Extract)
        .error("Invalid dimension");
}


void tests::sorting_functions()
// ----------------------------------------------------------------------------
//   Sorting operations
// ----------------------------------------------------------------------------
{
    BEGIN(sorting);

    step("Value sort (SORT)")
        .test(CLEAR, "{ 7 2.5 3 9.2 \"DEF\" 8.4 \"ABC\" } SORT", ENTER)
        .expect("{ 2.5 3 7 8.4 9.2 \"ABC\" \"DEF\" }");
    step("Reverse list (REVLIST)")
         .test("revlist", ENTER)
         .expect("{ \"DEF\" \"ABC\" 9.2 8.4 7 3 2.5 }");
    step("Memory sort (QUICKSORT)")
        .test("QUICKSORT", ENTER)
        .expect("{ 2.5 8.4 9.2 3 7 \"ABC\" \"DEF\" }");
    step("Reverse memory sort (ReverseQuickSort)")
        .test("reverseQuickSort", ENTER)
        .expect("{ \"DEF\" \"ABC\" 7 3 9.2 8.4 2.5 }");
    step("Reverse sort (ReverseSort)")
        .test("ReverseSort", ENTER)
        .expect("{ \"DEF\" \"ABC\" 9.2 8.4 7 3 2.5 }");
    step("Min function (integer)")
        .test(CLEAR, "1 2 MIN", ENTER).expect("1");
    step("Max function (integer)")
        .test(CLEAR, "1 2 MAX", ENTER).expect("2");
    step("Min function (decimal)")
        .test(CLEAR, "1.23 4.56 MIN", ENTER).expect("1.23");
    step("Max function (decimal)")
        .test(CLEAR, "1.23 4.56 MAX", ENTER).expect("4.56");
    step("Min function (fraction)")
        .test(CLEAR, "1/23 4/56 MIN", ENTER).expect("¹/₂₃");
    step("Max function (fraction)")
        .test(CLEAR, "1/23 4/56 MAX", ENTER).expect("¹/₁₄");
    step("Min function (mixed numbers)")
        .test(CLEAR, "1/23 4.56 MIN", ENTER).expect("¹/₂₃");
    step("Max function (mixed numbers)")
        .test(CLEAR, "1/23 4.56 MAX", ENTER).expect("4.56");
    step("Min function (text)")
        .test(CLEAR, "\"ABC\" \"DEF\" MIN", ENTER).expect("\"ABC\"");
    step("Max function (text)")
        .test(CLEAR, "\"ABC\" \"DEF\" MAX", ENTER).expect("\"DEF\"");
    step("Min function (mixed types)")
        .test(CLEAR, "1 \"DEF\" MAX", ENTER).error("Bad argument type");
    step("Min function (symbolic types)")
        .test(CLEAR, "1 X MIN", ENTER).expect("'Min(1;X)'")
        .test(CLEAR, "X 1 MIN", ENTER).expect("'Min(X;1)'")
        .test(CLEAR, "X Y MIN", ENTER).expect("'Min(X;Y)'");
    step("Max function (symbolic types)")
        .test(CLEAR, "1 X MAX", ENTER).expect("'Max(1;X)'")
        .test(CLEAR, "X 1 MAX", ENTER).expect("'Max(X;1)'")
        .test(CLEAR, "X Y MAX", ENTER).expect("'Max(X;Y)'");
    step("Min with arrays")
        .test(CLEAR, "[1 2 3] [3 2 1] Min", ENTER).expect("[ 1 2 1 ]");
    step("Max with arrays")
        .test(CLEAR, "[1 2 3] [3 2 1] Max", ENTER).expect("[ 3 2 3 ]");
    step("Min with array and scalar")
        .test(CLEAR, "[1 2 3] 2 Min", ENTER).expect("[ 1 2 2 ]")
        .test(CLEAR, "2 [1 2 3] Min", ENTER).expect("[ 1 2 2 ]");
    step("Max with array and scalar")
        .test(CLEAR, "[1 2 3] 2 Max", ENTER).expect("[ 2 2 3 ]")
        .test(CLEAR, "2 [1 2 3] Max", ENTER).expect("[ 2 2 3 ]");
    step("Min with array and symbolic scalar")
        .test(CLEAR, "[1 2 3] X Min", ENTER).expect("'Min([1;2;3];X)'")
        .test(CLEAR, "X [1 2 3] Min", ENTER).expect("'Min(X;[1;2;3])'");
    step("Max with array and symbolic scalar")
        .test(CLEAR, "[1 2 3] X Max", ENTER).expect("'Max([1;2;3];X)'")
        .test(CLEAR, "X [1 2 3] Max", ENTER).expect("'Max(X;[1;2;3])'");
    step("Max with arrays")
        .test(CLEAR, "[1 2 3] [3 2 1] Max", ENTER).expect("[ 3 2 3 ]");
    step("Min function (symbolic types)")
        .test(CLEAR, "1 \"DEF\" MAX", ENTER).error("Bad argument type");
    step("Max function (symbolic types)")
        .test(CLEAR, "1 \"DEF\" MAX", ENTER).error("Bad argument type");
}


void tests::text_functions()
// ----------------------------------------------------------------------------
//   Some operations on text
// ----------------------------------------------------------------------------
{
    BEGIN(text);

    step("Concatenation of text");
    test(CLEAR, "\"Hello \" \"World\" +", ENTER)
        .expect("\"Hello World\"");
    step("Concatenation of text and object");
    test(CLEAR, "\"Hello \" 2.3 +", ENTER)
        .expect("\"Hello 2.3\"");
    step("Concatenation of object and text");
    test(CLEAR, "2.3 \"Hello \" +", ENTER)
        .expect("\"2.3Hello \"");

    step("Repeating text");
    test(CLEAR, "\"AbC\" 3 *", ENTER)
        .expect("\"AbCAbCAbC\"");
    test(CLEAR, "3 \"AbC\" *", ENTER)
        .expect("\"AbCAbCAbC\"");

    step("Character generation with CHR")
        .test(CLEAR, "64 CHR", ENTER).type(ID_text).expect("\"@\"");
    step("Codepoint generation with NUM")
        .test(CLEAR,"\"a\" NUM", ENTER).type(ID_integer).expect(97);
    step("Codepoint generation with NUM, multiple characters")
        .test(CLEAR,"\"ba\" NUM", ENTER).type(ID_integer).expect(98);

    step("Convert object to text")
        .test(CLEAR, RSHIFT, KEY4, "1.42", F1)
        .type(ID_text).expect("\"1.42\"");
    step("Convert object from text")
        .test(CLEAR, RSHIFT, KEY4, "\"1.42 2.43 +\"", F2)
        .type(ID_decimal).expect("3.85");
    step("Size of single object")
        .test(CLEAR, "3.85", F3)
        .type(ID_integer).expect("1");
    step("Length of null text")
        .test(ENTER, RSHIFT, ENTER, ENTER, F3)
        .type(ID_integer).expect("0");
    step("Length of text")
        .test(CLEAR, RSHIFT, KEY4, "\"1.42 2.43 +\"", F3)
        .type(ID_integer).expect("11")
        .test(SHIFT, M, ADD, ENTER, ADD, F3)
        .type(ID_integer).expect("26");

    step("Conversion of text to code")
        .test(CLEAR, RSHIFT, ENTER, "Hello", NOSHIFT, RSHIFT, KEY4, SHIFT, F1)
        .type(ID_list).expect("{ 72 101 108 108 111 }");
    step("Conversion of code to text")
        .test(CLEAR, RSHIFT, RUNSTOP,
              232, SPACE, 233, SPACE, 234, SPACE, 235, SPACE,
              960, SPACE, 8730, SPACE, 8747, ENTER,
              RSHIFT, KEY4, SHIFT, F2)
        .type(ID_text).expect("\"èéêëπ√∫\"");

    step("Head of text")
        .test(CLEAR, "\"Hello\" HEAD", ENTER)
        .expect("\"H\"")
        .test(CLEAR, "\"À demain\" HEAD", ENTER)
            .expect("\"À\"");
    step("Tail of text")
        .test(CLEAR, "\"Hello\" TAIL", ENTER)
        .expect("\"ello\"")
        .test(CLEAR, "\"À demain\" TAIL", ENTER)
        .expect("\" demain\"");

    step("Extract text range")
        .test(CLEAR, "\"Hello World\" 3 5 EXTRACT", ENTER)
        .expect("\"llo\"");
    step("Extract one-character text range")
        .test(CLEAR, "\"Hello World\" 3 3", ID_TextMenu, ID_Extract)
        .expect("\"l\"");
    step("Extract empty text range")
        .test(CLEAR, "\"Hello World\" 3 2", ID_TextMenu, ID_Extract)
        .expect("\"\"");
    step("Extract text range starting at 0")
        .test(CLEAR, "\"Hello World\" 3 2", ID_TextMenu, ID_Extract)
        .expect("\"\"");
    step("Extract text range with negative value")
        .test(CLEAR, "\"Hello World\" -3 2", ID_TextMenu, ID_Extract)
        .expect("\"He\"");
    step("Extract text range with end out of bounds")
        .test(CLEAR, "\"Hello World\" 1 55", ID_TextMenu, ID_Extract)
        .expect("\"Hello World\"");
    step("Extract text range with range out of bounds")
        .test(CLEAR, "\"Hello World\" 53 55 EXTRACT", ENTER)
        .expect("\"\"");

    step("Ensure we can parse integer numbers with separators in them")
        .test(CLEAR, "100000", ENTER).expect("100 000")
        .test(RSHIFT, ENTER, NOSHIFT, ENTER).expect("\"\"")
        .test(ID_add).expect("\"100 000\"")
        .test(NOSHIFT, A, F2).expect("100 000");
    step("Ensure we can parse decimal numbers with separators in them")
        .test(CLEAR, "100000.123456123456", ENTER).expect("100 000.12345 6")
        .test(RSHIFT, ENTER, NOSHIFT, ENTER).expect("\"\"")
        .test(ID_add).expect("\"100 000.12345 6\"")
        .test(NOSHIFT, A, F2).expect("100 000.12345 6");
    step("Ensure we can parse base numbers with separators in them")
        .test(CLEAR, "16#ABCD1234", ENTER).expect("#ABCD 1234₁₆")
        .test(RSHIFT, ENTER, NOSHIFT, ENTER).expect("\"\"")
        .test(ID_add).expect("\"#ABCD 1234₁₆\"")
        .test(NOSHIFT, A, F2).expect("#ABCD 1234₁₆");
    step("Compatible based numbers")
        .test(CLEAR, ID_CompatibleBasedNumbers).noerror()
        .test(CLEAR, "16#ABCD1234", ENTER).expect("#ABCD 1234h")
        .test(RSHIFT, ENTER, NOSHIFT, ENTER).expect("\"\"")
        .test(ID_add).expect("\"#ABCD 1234h\"")
        .test(NOSHIFT, A, F2).expect("#ABCD 1234h")
        .test(ID_ModernBasedNumbers).expect("#ABCD 1234₁₆");
}


void tests::vector_functions()
// ----------------------------------------------------------------------------
//   Test operations on vectors
// ----------------------------------------------------------------------------
{
    BEGIN(vectors);

    step("Data entry in numeric form");
    test(CLEAR, "[  1  2  3  ]", ENTER)
        .type(ID_array).expect("[ 1 2 3 ]");
    test(CLEAR, "[  1.5  2.300  3.02  ]", ENTER)
        .type(ID_array).expect("[ 1.5 2.3 3.02 ]");

    step("Symbolic vector");
    test(CLEAR, "[a b c]", ENTER)
        .expect("[ a b c ]");

    step("Non-homogneous data types");
    test(CLEAR, "[  \"ABC\"  'X' 3/2  ]", ENTER)
        .type(ID_array).expect("[ \"ABC\" 'X' 1 ¹/₂ ]");

    step("Addition");
    test(CLEAR, "[1 2 3][4 5 6] +", ENTER)
        .expect("[ 5 7 9 ]");
    test(CLEAR, "[a b c][d e f] +", ENTER)
        .expect("[ 'a+d' 'b+e' 'c+f' ]");

    step("Subtraction");
    test(CLEAR, "[1 2 3 4][4 5 2 1] -", ENTER)
        .expect("[ -3 -3 1 3 ]");
    test(CLEAR, "[a b c][d e f] -", ENTER)
        .expect("[ 'a-d' 'b-e' 'c-f' ]");

    step("Multiplication (extension)");
    test(CLEAR, "[1 2  3 4 6][4 5 2 1 3] *", ENTER)
        .expect("[ 4 10 6 4 18 ]");
    test(CLEAR, "[a b c][d e f] *", ENTER)
        .expect("[ 'a·d' 'b·e' 'c·f' ]");

    step("Division (extension)");
    test(CLEAR, "[1 2  3 4 6][4 5 2 1 3] /", ENTER)
        .expect("[ ¹/₄ ²/₅ 1 ¹/₂ 4 2 ]");
    test(CLEAR, "[a b c][d e f] /", ENTER)
        .expect("[ 'd⁻¹·a' 'e⁻¹·b' 'f⁻¹·c' ]");

    step("Power (extension)");
    test(CLEAR, "[1 2  3 4 6][4 5 2 1 3] ^", ENTER)
        .want("[[ 1 1 1 1 1 ]"
              " [ 16 32 4 2 8 ]"
              " [ 81 243 9 3 27 ]"
              " [ 256 1 024 16 4 64 ]"
              " [ 1 296 7 776 36 6 216 ]]");
    test(CLEAR, "[a b c][d e f] ^", ENTER)
        .want("[[ 'a↑d' 'a↑e' 'a↑f' ]"
              " [ 'b↑d' 'b↑e' 'b↑f' ]"
              " [ 'c↑d' 'c↑e' 'c↑f' ]]");

    step("Addition of constant (extension)");
    test(CLEAR, "[1 2 3] 3 +", ENTER)
        .expect("[ 4 5 6 ]");
    test(CLEAR, "[a b c] x +", ENTER)
        .expect("[ 'a+x' 'b+x' 'c+x' ]");

    step("Subtraction of constant (extension)");
    test(CLEAR, "[1 2 3 4] 3 -", ENTER)
        .expect("[ -2 -1 0 1 ]");
    test(CLEAR, "[a b c] x -", ENTER)
        .expect("[ 'a-x' 'b-x' 'c-x' ]");
    test(CLEAR, "x [a b c] -", ENTER)
        .expect("[ 'x-a' 'x-b' 'x-c' ]");

    step("Multiplication by constant (extension)");
    test(CLEAR, "[a b c] x *", ENTER)
        .expect("[ 'a·x' 'b·x' 'c·x' ]");
    test(CLEAR, "x [a b c] *", ENTER)
        .expect("[ 'x·a' 'x·b' 'x·c' ]");

    step("Division by constant (extension)");
    test(CLEAR, "[a b c] x /", ENTER)
        .expect("[ 'a÷x' 'b÷x' 'c÷x' ]");
    test(CLEAR, "x [a b c] /", ENTER)
        .expect("[ 'x÷a' 'x÷b' 'x÷c' ]");

    step("Power by constant (extension)");
    test(CLEAR, "[a b c] x ^", ENTER)
        .expect("[ 'a↑x' 'b↑x' 'c↑x' ]");
    test(CLEAR, "x [a b c] ^", ENTER)
        .expect("[ 'x↑a' 'x↑b' 'x↑c' ]");
    test(CLEAR, "[a b c] 2 ^", ENTER)
        .expect("[ 'a²' 'b²' 'c²' ]");
    test(CLEAR, "2 [a b c] ^", ENTER)
        .expect("[ '2↑a' '2↑b' '2↑c' ]");

    step("Invalid dimension for binary operations");
    test(CLEAR, "[1 2 3][1 2] +", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[1 2 3][1 2] -", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[1 2 3][1 2] *", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[1 2 3][1 2] /", ENTER)
        .error("Invalid dimension");

    step("Component-wise inversion of a vector");
    test(CLEAR, "[1 2 3] INV", ENTER)
        .expect("[ 1 ¹/₂ ¹/₃ ]");

    step("Fröbenius norm");
    test(CLEAR, "[1 2 3] ABS", ENTER)
        .expect("3.74165 73867 7");
    test(CLEAR, "[1 2 3] NORM", ENTER)
        .expect("3.74165 73867 7");

    step("Component-wise application of functions");
    test(CLEAR, "[a b c] SIN", ENTER)
        .expect("[ 'sin a' 'sin b' 'sin c' ]");

    step("3D vector rectangular → rectangular")
        .test(CLEAR, "[1 2 3]", ENTER, NOSHIFT, A, F4)
        .expect("[ 1 2 3 ]");
    step("3D vector rectangular → polar")
        .test(F5)
        .expect("[ 2.23606 79775 63.43494 88229 ° 3 ]");
    step("3D vector polar → polar")
        .test(F5)
        .expect("[ 2.23606 79775 63.43494 88229 ° 3 ]");
    step("3D vector polar → spherical")
        .test(F6)
        .expect("[ 3.74165 73867 7 63.43494 88229 ° 36.69922 52005 ° ]");
    step("3D vector spherical → spherical")
        .test(F6)
        .expect("[ 3.74165 73867 7 63.43494 88229 ° 36.69922 52005 ° ]");
    step("3D vector spherical → polar")
        .test(F5)
        .expect("[ 2.23606 79775 63.43494 88229 ° 3. ]");
    step("3D vector polar → rectangular")
        .test(F4)
        .expect("[ 1. 2. 3. ]");
    step("3D vector rectangular → cylindrical")
        .test(LSHIFT, F4)
        .expect("[ 2.23606 79775 63.43494 88229 ° 3. ]");
    step("3D vector cylindrical → rectangular")
        .test(F4)
        .expect("[ 1. 2. 3. ]");
    step("3D vector rectangular → spherical")
        .test(F6)
        .expect("[ 3.74165 73867 7 63.43494 88229 ° 36.69922 52005 ° ]");
    step("3D vector spherical → rectangular")
        .test(F4)
        .expect("[ 1. 2. 3. ]");

    step("3D vector conversion error - Symbolic")
        .test(CLEAR, "[x y z]", ENTER, NOSHIFT, A, F4)
        .error("Bad argument type")
        .test(CLEAR, "[x y z]", ENTER, NOSHIFT, A, LSHIFT, F4)
        .error("Bad argument type")
        .test(CLEAR, "[x y z]", ENTER, NOSHIFT, A, F5)
        .error("Bad argument type")
        .test(CLEAR, "[x y z]", ENTER, NOSHIFT, A, F6)
        .error("Bad argument type");

    step("3D vector with non-angle units (#1276)")
        .test(CLEAR, "[ 1_m 2_s 3_g ]", ENTER, A, LSHIFT, F3)
        .got("3 g", "2 s", "1 m")
        .test(CLEAR, "[ 1_m 2_s 3_g ] Vector→", ENTER)
        .got("3 g", "2 s", "1 m");

    step("2D vector rectangular → rectangular")
        .test(CLEAR, "[ 1 2 ]", ENTER, NOSHIFT, A)
        .test(F4)
        .expect("[ 1 2 ]");
    step("2D vector rectangular → polar")
        .test(F5)
        .expect("[ 2.23606 79775 63.43494 88229 ° ]");
    step("2D vector polar → polar")
        .test(F5)
        .expect("[ 2.23606 79775 63.43494 88229 ° ]");
    step("2D vector polar → spherical")
        .test(F6)
        .error("Bad argument type");
    step("2D vector polar → rectangular")
        .test(F4)
        .expect("[ 1. 2. ]");
    step("2D vector rectangular → cylindrical")
        .test(LSHIFT, F4)
        .error("Bad argument type");
    step("2D vector rectangular → spherical")
        .test(F6)
        .error("Bad argument type");

   step("2D vector conversion error - Symbolic")
       .test(CLEAR, "[x y]", ENTER, NOSHIFT, A, F4)
       .error("Bad argument type")
       .test(CLEAR, "[x y]", ENTER, NOSHIFT, A, LSHIFT, F4)
       .error("Bad argument type")
       .test(CLEAR, "[x y]", ENTER, NOSHIFT, A, F5)
       .error("Bad argument type")
       .test(CLEAR, "[x y]", ENTER, NOSHIFT, A, F6)
       .error("Bad argument type");

    step("2D vector with non-angle units (#1276)")
        .test(CLEAR, "[ 1_m 2_s ]", ENTER, A, LSHIFT, F3)
        .got("2 s", "1 m")
        .test(CLEAR, "[ 1_m 2_s ] V→", ENTER)
        .got("2 s", "1 m");

   step("Vector conversion only works on 2D or 3D vectors")
       .test(CLEAR, "[ 1 2 3 4 ]", ENTER, NOSHIFT, A, F4)
       .error("Bad argument type");

   step("3D addition after polar conversion")
       .test(CLEAR, "[ 1 2 3 ]", ENTER, NOSHIFT, A, F5)
       .expect("[ 2.23606 79775 63.43494 88229 ° 3 ]")
       .test("[ 4 5 6 ]", ID_add)
       .expect("[ 5. 7. 9 ]");
   step("3D addition after cylindrical conversion")
       .test(CLEAR, "[ 1 2 3 ]", ENTER, NOSHIFT, A, F6)
       .expect("[ 3.74165 73867 7 63.43494 88229 ° 36.69922 52005 ° ]")
       .test("[ 4 5 6 ]", ID_add)
       .expect("[ 5. 7. 9. ]");

   step("3D addition with polar conversion")
       .test(CLEAR, "[ 1 2 3 ]", ENTER, NOSHIFT, A, F4)
       .expect("[ 1 2 3 ]")
       .test("[ 4 5 6 ]", F5, ID_add)
       .expect("[ 8.60232 52670 4 54.46232 2208 ° 9 ]");
   step("3D addition with cylindrical conversion")
       .test(CLEAR, "[ 1 2 3 ]", ENTER, NOSHIFT, A, F4)
       .expect("[ 1 2 3 ]")
       .test("[ 4 5 6 ]", F6, ID_add)
       .expect("[ 12.44989 9598 54.46232 2208 ° 43.70578 41445 ° ]");
   step("3D addition polar + cylindrical")
       .test(CLEAR, "[ 1 2 3 ]", ENTER, NOSHIFT, A, F5)
       .expect("[ 2.23606 79775 63.43494 88229 ° 3 ]")
       .test("[ 4 5 6 ]", F6, ID_add)
       .expect("[ 12.44989 9598 54.46232 2208 ° 43.70578 41445 ° ]");
   step("3D addition with angle as last coordinate (#1455)")
       .test(CLEAR, "[ 3 2 1.7_° ] [2 1 2.5_° ] -", ENTER)
       .expect("[ 1 1 -0.8 ° ]");

   step("2D addition after polar conversion")
       .test(CLEAR, "[ 1 2 ]", ENTER, NOSHIFT, A, F5)
       .expect("[ 2.23606 79775 63.43494 88229 ° ]")
       .test("[ 4 5 ]", ID_add)
       .expect("[ 5. 7. ]");
   step("2D addition with polar conversion")
       .test(CLEAR, "[ 1 2 ]", ENTER, NOSHIFT, A, F4)
       .expect("[ 1 2 ]")
       .test("[ 4 5 ]", F5, ID_add)
       .expect("[ 8.60232 52670 4 54.46232 2208 ° ]");

   step("Extracting from 1D vector (#1310)")
       .test(CLEAR, "[ 1_m ]", ENTER, A, LSHIFT, F3)
       .got("1 m");
   step("Extracting from 4D vector (#1310)")
       .test(CLEAR, "[ 1_m 2_s 3_km 42 ] V→", ENTER)
       .got("42", "3 km", "2 s", "1 m");
   step("Extracting vector from matrix")
       .test(CLEAR, "[ [ 1_m 2_s 3_km 42 ] ] V→", ENTER)
        .error("Bad argument type");

   step("Error in in vector operation")
       .test(CLEAR, "[1][0]", ID_divide)
       .error("Divide by zero");

}


void tests::matrix_functions()
// ----------------------------------------------------------------------------
//   Test operations on vectors
// ----------------------------------------------------------------------------
{
    BEGIN(matrices);

    step("Data entry in numeric form");
    test(CLEAR, "[  [1  2  3][4 5 6]  ]", ENTER)
        .type(ID_array).want("[[ 1 2 3 ] [ 4 5 6 ]]");

    step("Non-rectangular matrices");
    test(CLEAR, "[  [ 1.5  2.300 ] [ 3.02 ]]", ENTER)
        .type(ID_array).want("[[ 1.5 2.3 ] [ 3.02 ]]")
        .test(ENTER, ID_add)
        .want("[[ 3. 4.6 ] [ 6.04 ]]");

    step("Symbolic matrix");
    test(CLEAR, "[[a b] [c d]]", ENTER)
        .want("[[ a b ] [ c d ]]");

    step("Non-homogneous data types");
    test(CLEAR, "[  [ \"ABC\"  'X' ] 3/2  [ 4 [5] [6 7]]]", ENTER)
        .type(ID_array)
        .want("[[ \"ABC\" 'X' ] 1 ¹/₂ [ 4 [ 5 ] [ 6 7 ] ] ]");

    step("Insert single row in vector")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 0 ROW+"), ENTER)
        .error("Bad argument value")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 1 ROW+"), ENTER)
        .want("[ 4 1 2 3 ]")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 2 ROW+"), ENTER)
        .want("[ 1 4 2 3 ]")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 3 ROW+"), ENTER)
        .want("[ 1 2 4 3 ]")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 4 ROW+"), ENTER)
        .want("[ 1 2 3 4 ]")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 5 ROW+"), ENTER)
        .error("Bad argument value");

    step("Insert multiple rows in vector")
        .test(CLEAR, DIRECT("[ 1 2 3 ] [ 4 5 ] 2 ROW+"), ENTER)
        .want("[ 1 4 5 2 3 ]");

    step("Insert single column in vector")
        .test(CLEAR, DIRECT("[ 1 2 3 ] 4 2 COL+"), ENTER)
        .want("[ 1 4 2 3 ]");

    step("Insert multiple column in vector")
        .test(CLEAR, DIRECT("[ 1 2 3 ] [ 4 5 ] 2 COL+"), ENTER)
        .want("[ 1 4 5 2 3 ]");

    step("Insert rows in list vector")
        .test(CLEAR, DIRECT("{ 1 2 3 } 4 1 ROW+"), ENTER)
        .error("Bad argument type")
        .test(CLEAR, DIRECT("LaxArrayResizing { 1 2 3 } 4 1 ROW+"), ENTER)
        .want("{ 4 1 2 3 }")
        .test(CLEAR, DIRECT("'LaxArrayResizing' PURGE "
                            "{ 1 2 3 } 4 1 ROW+"), ENTER)
        .error("Bad argument type");

    step("Addition");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] +", ENTER)
        .want("[[ 6 8 ] [ 10 12 ]]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] +", ENTER)
        .want("[[ 'a+e' 'b+f' ] [ 'c+g' 'd+h' ]]");

    step("Addition (non-square), #1343");
    test(CLEAR, "[[1 2]] [[7 8]] +", ENTER)
        .want("[[ 8 10 ]]");
    test(CLEAR, "[[a b c]] [[g h i]] -", ENTER)
        .want("[[ 'a-g' 'b-h' 'c-i' ]]");
    test(CLEAR, "[[a] [b] [c]] [[g][ h] [i]] +", ENTER)
        .want("[[ 'a+g' ] [ 'b+h' ] [ 'c+i' ]]");

    step("Subtraction");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] -", ENTER)
        .want("[[ -4 -4 ] [ -4 -4 ]]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] -", ENTER)
        .want("[[ 'a-e' 'b-f' ] [ 'c-g' 'd-h' ]]");

    step("Multiplication (square)");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] *", ENTER)
        .want("[[ 19 22 ] [ 43 50 ]]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] *", ENTER)
        .want("[[ 'a·e+b·g' 'a·f+b·h' ] [ 'c·e+d·g' 'c·f+d·h' ]]");

    step("Multiplication (non-square)");
    test(CLEAR, "[[1 2 3] [4 5 6]] [[5 6][7 8][9 10]] *", ENTER)
        .want("[[ 46 52 ] [ 109 124 ]]");
    test(CLEAR, "[[a b c d][e f g h]] [[x][y][z][t]] *", ENTER)
        .want("[[ 'a·x+b·y+c·z+d·t' ] [ 'e·x+f·y+g·z+h·t' ]]");
    test(CLEAR, "[[a b c d][e f g h]] [x y z t] *", ENTER)
        .want("[ 'a·x+b·y+c·z+d·t' 'e·x+f·y+g·z+h·t' ]");

    step("Division");
    test(CLEAR,
         "[[5 12 1968][17 2 1969][30 3 1993]] "
         "[[16 5 1995][21 5 1999][28 5 2009]] /", ENTER)
        .want("[[ 3 ¹/₁₁ -4 ⁸/₁₁ -3 ¹⁰/₁₁ ]"
              " [ 335 ⁷/₁₀ -1 342 ⁷/₁₀ -1 643 ³/₁₀ ]"
              " [ -¹⁹/₂₂ 3 ⁹/₂₂ 5 ³/₂₂ ]]");
    step("Division (symbolic)");
    test(CLEAR, "[[a b][c d]][[e f][g h]] /", ENTER)
        .want("[[ '(e⁻¹-f÷e·((-g)÷(e·h-g·f)))·a+(-(f÷e·e÷(e·h-g·f)))·c' "
              "'(e⁻¹-f÷e·((-g)÷(e·h-g·f)))·b+(-(f÷e·e÷(e·h-g·f)))·d' ] "
              "[ '(-g)÷(e·h-g·f)·a+e÷(e·h-g·f)·c' "
              "'(-g)÷(e·h-g·f)·b+e÷(e·h-g·f)·d' ]]");

    step("Power")
        .test(CLEAR,
              "[[2 3][5 6]] "
              "[[11 12][14 15]] ^", ENTER)
        .want("[[[[ 2 048 4 096 ]"
              "   [ 16 384 32 768 ]]"
              "  [[ 177 147 531 441 ]"
              "   [ 4 782 969 14 348 907 ]]]"
              " [[[ 48 828 125 244 140 625 ]"
              "   [ 6 103 515 625 30 517 578 125 ]]"
              "  [[ 362 797 056 2 176 782 336 ]"
              "   [ 78 364 164 096 470 184 984 576 ]]]]");
    step("Power (symbolic)")
        .test(CLEAR, "[[a b][c d]][[e f][g h]] ^", ENTER)
        .want("[[[[ 'a↑e' 'a↑f' ]"
              "   [ 'a↑g' 'a↑h' ]]"
              "  [[ 'b↑e' 'b↑f' ]"
              "   [ 'b↑g' 'b↑h' ]]]"
              " [[[ 'c↑e' 'c↑f' ]"
              "   [ 'c↑g' 'c↑h' ]]"
              "  [[ 'd↑e' 'd↑f' ]"
              "   [ 'd↑g' 'd↑h' ]]]]");

    step("Addition of constant (extension)");
    test(CLEAR, "[[1 2] [3 4]] 3 +", ENTER)
        .want("[[ 4 5 ] [ 6 7 ]]");
    test(CLEAR, "[[a b] [c d]] x +", ENTER)
        .want("[[ 'a+x' 'b+x' ] [ 'c+x' 'd+x' ]]");

    step("Subtraction of constant (extension)");
    test(CLEAR, "[[1 2] [3 4]] 3 -", ENTER)
        .want("[[ -2 -1 ] [ 0 1 ]]");
    test(CLEAR, "[[a b] [c d]] x -", ENTER)
        .want("[[ 'a-x' 'b-x' ] [ 'c-x' 'd-x' ]]");

    step("Multiplication by constant (extension)");
    test(CLEAR, "[[a b] [c d]] x *", ENTER)
        .want("[[ 'a·x' 'b·x' ] [ 'c·x' 'd·x' ]]");
    test(CLEAR, "x [[a b] [c d]] *", ENTER)
        .want("[[ 'x·a' 'x·b' ] [ 'x·c' 'x·d' ]]");

    step("Division by constant (extension)");
    test(CLEAR, "[[a b] [c d]] x /", ENTER)
        .want("[[ 'a÷x' 'b÷x' ] [ 'c÷x' 'd÷x' ]]");
    test(CLEAR, "x [[a b] [c d]] /", ENTER)
        .want("[[ 'x÷a' 'x÷b' ] [ 'x÷c' 'x÷d' ]]");

    step("Power by constant (extension)");
    test(CLEAR, "[[a b] [c d]] x ^", ENTER)
        .want("[[ 'a↑x' 'b↑x' ] [ 'c↑x' 'd↑x' ]]");
    test(CLEAR, "[[a b] [c d]] 2 ^", ENTER)
        .want("[[ 'a²+b·c' 'a·b+b·d' ]"
              " [ 'c·a+d·c' 'c·b+d²' ]]");
    test(CLEAR, "[[a b] [c d]] 2.0 ^", ENTER)
        .want("[[ 'a↑2.' 'b↑2.' ] [ 'c↑2.' 'd↑2.' ]]");
    test(CLEAR, "x [[a b] [c d]] ^", ENTER)
        .want("[[ 'x↑a' 'x↑b' ] [ 'x↑c' 'x↑d' ]]");
    test(CLEAR, "2 [[a b] [c d]] ^", ENTER)
        .want("[[ '2↑a' '2↑b' ] [ '2↑c' '2↑d' ]]");
    test(CLEAR, "2.0 [[a b] [c d]] ^", ENTER)
        .want("[[ '2.↑a' '2.↑b' ] [ '2.↑c' '2.↑d' ]]");

    step("Invalid dimension for addition (second is larger)")
        .test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] +", ENTER)
        .error("Invalid dimension");
    step("Invalid dimension for addition (second is smaller)")
        .test(CLEAR, "[[1 2] [3 4] [5 6]][[1 2][3 4]] +", ENTER)
        .error("Invalid dimension");
    step("Invalid dimension for subtraction (second is larger)")
        .test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] -", ENTER)
        .error("Invalid dimension");
    step("Invalid dimension for subtraction (second is smaller)")
        .test(CLEAR, "[[1 2] [3 4] [5 6]][[1 2][3 4]] -", ENTER)
        .error("Invalid dimension");
    step("Element-wise addition (extension)")
        .test(CLEAR, "[[1 2] [3 4]][1 2] +", ENTER)
        .want("[[ 2 3 ] [ 5 6 ]]");
    step("Element-wise subtraction (extension)")
        .test(CLEAR, "[[1 2] [3 4]][1 2] -", ENTER)
        .want("[[ 0 1 ] [ 1 2 ]]");
    step("Element-wise multiplication (extension)")
        .test(CLEAR, "[[1 2] [3 4]] 4 *", ENTER)
        .want("[[ 4 8 ] [ 12 16 ]]");
    step("Matrix-vector multiplication")
        .test(CLEAR, "[[1 2] [3 4]][4 5] *", ENTER)
        .want("[ 14 32 ]");
    step("Element-wise division (extension)")
        .test(CLEAR, "[[1 2] [3 4]][2 3] /", ENTER)
        .want("[[ ¹/₂ 1 ] [ 1 1 ¹/₃ ]]");

    step("Non-rectangular addition (extension)")
        .test(CLEAR, "[[1][2 3]][[4][5 6]] +", ENTER)
        .want("[[ 5 ] [ 7 9 ]]");
    step("Non-rectangular subtraction (extension)")
        .test(CLEAR, "[[1][2 3]][4 [5 6]] -", ENTER)
        .want("[[ -3 ] [ -3 -3 ]]");
    step("Matrix of matrices")
        .test(CLEAR, "[[[ 1 2 ][3 4]][[ 1 2 ][3 4]]]", ENTER,
              ENTER, 3, ID_multiply, ID_multiply)
        .want("[[[ 12 24 ] [ 54 72 ]] [[ 12 24 ] [ 54 72 ]]]");

    step("Min and max")
        .test(CLEAR, "[[1 2][3 4][5 6]] [[0 3][4 6][2 1]] MIN", ENTER)
        .want("[[ 0 2 ] [ 3 4 ] [ 2 1 ]]")
        .test(CLEAR, "[[1 2][3 4][5 6]] [[0 3][4 6][2 1]] MAX", ENTER)
        .want("[[ 1 3 ] [ 4 6 ] [ 5 6 ]]");

    step("Inversion of a definite matrix");
    test(CLEAR, "[[1 2 3][4 5 6][7 8 19]] INV", ENTER)
        .want("[[ -1 ¹⁷/₃₀ ⁷/₁₅ ¹/₁₀ ]"
              " [ 1 ²/₁₅ ¹/₁₅ -¹/₅ ]"
              " [ ¹/₁₀ -¹/₅ ¹/₁₀ ]]");
    test(CLEAR, "[[a b][c d]] INV", ENTER)
        .want("[[ 'a⁻¹-b÷a·((-c)÷(a·d-c·b))' "
              "'-(b÷a·a÷(a·d-c·b))' ] "
              "[ '(-c)÷(a·d-c·b)' 'a÷(a·d-c·b)' ]]");

    step("Invert with zero determinant");       // HP48 gets this one wrong
    test(CLEAR, "[[1 2 3][4 5 6][7 8 9]] INV", ENTER)
        .error("Divide by zero");

    step("Determinant");                        // HP48 gets this one wrong
    test(CLEAR, "[[1 2 3][4 5 6][7 8 9]] DET", ENTER)
        .want("0");
    test(CLEAR, "[[1 2 3][4 5 6][7 8 19]] DET", ENTER)
        .want("-30");

    step("Froebenius norm");
    test(CLEAR, "[[1 2] [3 4]] ABS", ENTER)
        .want("5.47722 55750 5");
    test(CLEAR, "[[1 2] [3 4]] NORM", ENTER)
        .want("5.47722 55750 5");

    step("Component-wise application of functions");
    test(CLEAR, "[[a b] [c d]] SIN", ENTER)
        .want("[[ 'sin a' 'sin b' ] [ 'sin c' 'sin d' ]]");

    step("Dot product (numerical)")
        .test(CLEAR, "[2 3 4 5 6] [7 8 9 10 11] DOT", ENTER)
        .expect("190");
    step("Dot product (symbolic)")
        .test(CLEAR, "[a b c d] [e f g h] DOT", ENTER)
        .expect("'a·e+b·f+c·g+d·h'");
    step("Dot product (type error)")
        .test(CLEAR, "2 3 DOT", ENTER)
        .error("Bad argument type");
    step("Dot product (dimension error)")
        .test(CLEAR, "[1 2 3] [4 5] DOT", ENTER)
        .error("Invalid dimension");

    step("Cross product (numerical)")
        .test(CLEAR, "[2 3 4] [7 8 9] CROSS", ENTER)
        .expect("[ -5 10 -5 ]");
    step("Cross product (symbolic)")
        .test(CLEAR, "[a b c] [e f g] CROSS", ENTER)
        .expect("[ 'b·g-c·f' 'c·e-a·g' 'a·f-b·e' ]");
    step("Cross product (zero-extend)")
        .test(CLEAR, "[a b] [e f g] CROSS", ENTER)
        .expect("[ 'b·g' '-(a·g)' 'a·f-b·e' ]")
        .test(CLEAR, "[a b c] [e f] CROSS", ENTER)
        .expect("[ '-(c·f)' 'c·e' 'a·f-b·e' ]")
        .test(CLEAR, "[a b] [e f] CROSS", ENTER)
        .expect("[ 0 0 'a·f-b·e' ]");
    step("Cross product (type error)")
        .test(CLEAR, "2 3 CROSS", ENTER)
        .error("Bad argument type");
    step("Cross product (dimension error)")
        .test(CLEAR, "[1 2 3 4] [4 5] CROSS", ENTER)
        .error("Invalid dimension");

    step("Array→ and →Array on vectors")
        .test(CLEAR, "[1 2 3 4]", ENTER, RSHIFT, KEY9)
        .test(LSHIFT, F4).expect("{ 4 }")
        .test(LSHIFT, F3).expect("[ 1 2 3 4 ]")
        .test(CLEAR, "[[1 2 3][4 5 6]]", ENTER, RSHIFT, KEY9)
        .test(LSHIFT, F4).expect("{ 2 3 }")
        .test(LSHIFT, F3).want("[[ 1 2 3 ] [ 4 5 6 ]]");

    step("Constant vector")
        .test(CLEAR, "3 10 CON", ENTER)
        .expect("[ 10 10 10 ]");
    step("Constant complex vector")
        .test(CLEAR, "2 1ⅈ2", RSHIFT, KEY9, F3)
        .expect("[ 1+2ⅈ 1+2ⅈ ]");
    step("Constant text vector")
        .test(CLEAR, "1 \"ABC\"", RSHIFT, KEY9, F3)
        .expect("[ \"ABC\" ]");
    step("Constant vector from list size")
        .test(CLEAR, "{ 4 } 42", RSHIFT, KEY9, F3)
        .expect("[ 42 42 42 42 ]");

    step("Constant matrix")
        .test(CLEAR, "{ 3 2 } 10 CON", ENTER)
        .want("[[ 10 10 ] [ 10 10 ] [ 10 10 ]]");
    step("Constant matrix")
        .test(CLEAR, "{ 3 2 5 } 10 CON", ENTER)
        .error("Invalid dimension");
    step("Constant vector from vector")
        .test(CLEAR, "[ 1 2 3] 2ⅈ3", RSHIFT, KEY9, F3)
        .expect("[ 2+3ⅈ 2+3ⅈ 2+3ⅈ ]");
    step("Constant matrix from matrix")
        .test(CLEAR, "[[1 2]] \"ABC\"", RSHIFT, KEY9, F3)
        .want("[[ \"ABC\" \"ABC\" ]]");
    step("Constant vector in name")
        .test(CLEAR, "{ 4 } 24", RSHIFT, KEY9, F3)
        .expect("[ 24 24 24 24 ]")
        .test("'MyVec'", NOSHIFT, G)
        .noerror()
        .test("'MyVec' 1.5 CON", ENTER)
        .noerror()
        .test("MyVec", ENTER)
        .expect("[ 1.5 1.5 1.5 1.5 ]")
        .test("'MyVec'", ENTER, NOSHIFT, BSP, F2);

    step("Idenitity matrix")
        .test(CLEAR, "3 IDN", ENTER)
        .want("[[ 1 0 0 ] [ 0 1 0 ] [ 0 0 1 ]]");
    step("Identity from list")
        .test(CLEAR, "{ 2 }", RSHIFT, KEY9, F2)
        .want("[[ 1 0 ] [ 0 1 ]]");
    step("Identity from 2-list")
        .test(CLEAR, "{ 2 2 }", RSHIFT, KEY9, F2)
        .want("[[ 1 0 ] [ 0 1 ]]");
    step("Identity from 2-list")
        .test(CLEAR, "{ 2 3 }", RSHIFT, KEY9, F2)
        .error("Invalid dimension");
    step("Identity from vector")
        .test(CLEAR, "[ 1  2 3 ]", RSHIFT, KEY9, F2)
        .want("[[ 1 0 0 ] [ 0 1 0 ] [ 0 0 1 ]]");
    step("Identity matrix in name")
        .test(CLEAR, "{ 4 }", RSHIFT, KEY9, F2)
        .test(KEY3, MUL, KEY2, ADD)
        .want("[[ 5 2 2 2 ] [ 2 5 2 2 ] [ 2 2 5 2 ] [ 2 2 2 5 ]]")
        .test("'MyIdn'", NOSHIFT, G)
        .noerror()
        .test("'MyIdn' IDN", ENTER)
        .noerror()
        .test("MyIdn", ENTER)
        .want("[[ 1 0 0 0 ] [ 0 1 0 0 ] [ 0 0 1 0 ] [ 0 0 0 1 ]]")
        .test("'MyIdn'", ENTER, NOSHIFT, BSP, F2);

    step("Divide fails")
        .test(CLEAR, "[[1 1][1 1]]", ENTER, ENTER, DIV)
        .error("Divide by zero");

    step("Do not leave garbage on the stack (#1363)")
        .test(CLEAR, "[[1 2 3][4 5 6]][[1 2][3 4][5 6]] DOT", ENTER)
        .error("Invalid dimension")
        .test(EXIT)
        .want("[[ 1 2 ] [ 3 4 ] [ 5 6 ]]")
        .test(BSP)
        .want("[[ 1 2 3 ] [ 4 5 6 ]]")
        .test(BSP).noerror()
        .test(BSP).error("Too few arguments");

    step("Tagged array operations")
        .test(CLEAR, ":A:[1 2] :B:[3 4] +", ENTER)
        .want("[ 4 6 ]");

    step("Transpose")
        .test(CLEAR, "[[1 2 3+2ⅈ][4 5-2ⅈ 6]]", ID_ToolsMenu, ID_Transpose)
        .want("[[ 1 4 ] [ 2 5-2ⅈ ] [ 3+2ⅈ 6 ]]");
    step("Transpose and conjugate")
        .test(CLEAR, "[[1 2 3+2ⅈ][4 5-2ⅈ 6]]", ID_ToolsMenu, ID_TransConjugate)
        .want("[[ 1 4 ] [ 2 5+2ⅈ ] [ 3-2ⅈ 6 ]]");

    step("Row norm for vector")
        .test(CLEAR, "[1 2 3 4]", ID_MatrixMenu, ID_RowNorm)
        .expect("4");
    step("Row norm for vector")
        .test(CLEAR, "[[1 2] [3 4]]", ID_MatrixMenu, ID_RowNorm)
        .expect("7");
    step("Column norm for vector")
        .test(CLEAR, "[1 2 3 4]", ID_MatrixMenu, ID_ColumnNorm)
        .expect("10");
    step("Column norm for vector")
        .test(CLEAR, "[[1 2] [3 4]]", ID_MatrixMenu, ID_ColumnNorm)
        .expect("6");
}


void tests::solver_testing()
// ----------------------------------------------------------------------------
//   Test that the solver works as expected
// ----------------------------------------------------------------------------
{
    BEGIN(solver);

    step("Enter directory for solving")
        .test(CLEAR, "'SLVTST' CRDIR SLVTST", ENTER);

    step("Select purely numerical solver")
        .test(CLEAR, "NumericalSolver", ENTER).noerror();

    step("Solver with expression")
        .test(CLEAR, "'X+3' 'X' 0 ROOT", ENTER)
        .noerror().expect("X=-3.");
    step("Solver with arithmetic syntax")
        .test(CLEAR, "'ROOT(X+3;X;0)'", ENTER)
        .expect("'Root(X+3;X;0)'")
        .test(RUNSTOP)
        .expect("X=-3.")
        .test("X", ENTER)
        .expect("-3.")
        .test("'X' purge", ENTER)
        .noerror();
    step("Solver with equation")
        .test(CLEAR, "'sq(x)=3' 'X' 0 ROOT", ENTER)
        .noerror().expect("X=1.73205 08075 7")
        .test("X", ENTER)
        .expect("1.73205 08075 7")
        .test("'X'", ENTER, LSHIFT, BSP, F2)
        .noerror();
    step("Solver without solution")
        .test(CLEAR, "'sq(x)+3=0' 'X' 1 ROOT", ENTER)
        .error("No solution?")
        .test(CLEAR, "X", ENTER)
        .expect("-2.19049 50614 3⁳⁻¹³")
        .test("'X'", ENTER, LSHIFT, BSP, F2)
        .noerror();
    step("Solver with slow slope")
        .test(CLEAR, "'tan(x)=224' 'x' 1 ROOT", ENTER)
        .expect("x=89.74421 69693");
    step("Solver with slow slope 2")
        .test(CLEAR, "'tan(x)=224' 'x' 0 ROOT", ENTER)
        .expect("x=89.74421 69693");


    step("Solving menu")
        .test(CLEAR, "'A²+B²=C²'", ENTER)
        .test(LSHIFT, KEY7, LSHIFT, F1, F6)
        .test("3", NOSHIFT, F2, "4", NOSHIFT, F3, LSHIFT, F4)
        .expect("C=5.");
    step("Evaluate equation case Left=Right")
        .test(F1)
        .expect("'25=25.+2.11075 8519⁳⁻¹²'");

    step("Verify that we display the equation after entering value")
        .test(CLEAR, "42", F4)
        .image_noheader("solver-eqdisplay");
    step("Evaluate equation case Left=Right")
        .test("4", F4, F1)
        .expect("'25=16+9'");
    step("Evaluate equation case Left=Right")
        .test("7", F4, F1)
        .expect("'25=49-24'");

    step("Solving with units")
        .test("30_cm", NOSHIFT, F2, ".4_m", NOSHIFT, F3, "100_in", NOSHIFT, F4)
        .test(LSHIFT, F4)
        .expect("C=19.68503 93701 in")
        .test(LSHIFT, KEY5, F4, LSHIFT, F1)
        .test(LSHIFT, A, LSHIFT, A)
        .expect("0.5 m");

    step("Solving with large values (#1179)")
        .test(CLEAR, "DEG '1E45*sin(x)-0.5E45' 'x' 2 ROOT", ENTER)
        .expect("x=30.");
    step("Solving equation containing a zero side (#1179)")
        .test(CLEAR, "'-3*expm1(-x)-x=0' 'x' 2 ROOT", ENTER)
        .expect("x=2.82143 93721 2");
    step("Solving Antoine's equation (#1495)")
        .test(CLEAR, DIRECT("'log10(P)=6.90565-1211.033/(98+220.73)' "
                            "'P' 1000 ROOT"), ENTER)
        .expect("P=1 276.71035 463");

    step("Select algebraically-assisted solver")
        .test(CLEAR, "SymbolicSolver", ENTER).noerror();

    step("Solver with expression")
        .test(CLEAR, "'X+3' 'X' 0 ROOT", ENTER)
        .noerror().expect("X=-3");
    step("Solver with arithmetic syntax")
        .test(CLEAR, "'ROOT(X+3;X;0)'", ENTER)
        .expect("'Root(X+3;X;0)'")
        .test(RUNSTOP)
        .expect("X=-3")
        .test("X", ENTER)
        .expect("-3")
        .test("'X' purge", ENTER)
        .noerror();
    step("Solver with equation")
        .test(CLEAR, "'sq(x)=3' 'X' 0 ROOT", ENTER)
        .noerror().expect("X=1.73205 08075 7")
        .test("X", ENTER)
        .expect("1.73205 08075 7")
        .test("'X'", ENTER, LSHIFT, BSP, F2)
        .noerror();
    step("Solver without solution")
        .test(CLEAR, "'sq(x)+3=0' 'X' 1 ROOT", ENTER)
        .error("Argument outside domain")
        .test(CLEAR, "X", ENTER)
        .expect("'X'")
        .test("'X'", ENTER, LSHIFT, BSP, F2)
        .noerror();
    step("Solver with slow slope")
        .test(CLEAR, "'tan(x)=224' 'x' 1 ROOT", ENTER)
        .expect("x=89.74421 69693 °");
    step("Solver with slow slope 2")
        .test(CLEAR, "'tan(x)=224' 'x' 0 ROOT", ENTER)
        .expect("x=89.74421 69693 °");


    step("Solving menu")
        .test(CLEAR, "{ A B C } PURGE", ENTER).noerror()
        .test(CLEAR, "'A²+B²=C²'", ENTER)
        .test(LSHIFT, KEY7, LSHIFT, F1, F6)
        .test("3", NOSHIFT, F2, "4", NOSHIFT, F3, LSHIFT, F4)
        .expect("C=5.");
    step("Evaluate equation case Left=Right")
        .test(F1)
        .expect("'25=25.'");

    step("Verify that we display the equation after entering value")
        .test(CLEAR, "42", F4)
        .image_noheader("solver-eqdisplay");
    step("Evaluate equation case Left=Right")
        .test("4", F4, F1)
        .expect("'25=16+9'");
    step("Evaluate equation case Left=Right")
        .test("7", F4, F1)
        .expect("'25=49-24'");

    step("Solving with units")
        .test("30_cm", NOSHIFT, F2, ".4_m", NOSHIFT, F3, "100_in", NOSHIFT, F4)
        .test(LSHIFT, F4)
        .expect("C=19.68503 93701 in")
        .test(LSHIFT, KEY5, F4, LSHIFT, F1)
        .test(LSHIFT, A, LSHIFT, A)
        .expect("0.5 m");

    step("Solving with large values (#1179)")
        .test(CLEAR, "DEG '1E45*sin(x)-0.5E45' 'x' 2 ROOT", ENTER)
        .expect("x=30. °");
    step("Solving equation containing a zero side (#1179)")
        .test(CLEAR, "'-3*expm1(-x)-x=0' 'x' 2 ROOT", ENTER)
        .expect("x=2.82143 93721 2");
    step("Solving Antoine's equation (#1495)")
        .test(CLEAR, DIRECT("'log10(P)=6.90565-1211.033/(98+220.73)' "
                            "'P' 1000 ROOT"), ENTER)
        .expect("P=1 276.71035 463");

    step("Jacobian solver, linear case")
        .test(CLEAR, "{ '3*X=2*Y-3' '2*X=3*Y-5' }"
              "{ X Y } { 0 0 } ROOT", ENTER)
        .expect("{ X=0.2 Y=1.8 }");
    step("Jacobian solver, linear case with extra true equation")
        .test(CLEAR, "{ '3*X=2*Y-3' '2*X=3*Y-5' '4*X-6*Y+10=0' }"
              "{ X Y } DUP PURGE { 0 0 } ROOT", ENTER)
        .expect("{ X=0.2 Y=1.8 }");
    step("Jacobian solver, linear case with extra false equation")
        .test(CLEAR, "{ '3*X=2*Y-3' '2*X=3*Y-5' '4*X-6*Y=10' }"
              "{ X Y } DUP PURGE { 0 0 } ROOT", ENTER)
        .error("Unable to solve for all variables");
    step("Jacobian solver, two circles")
        .test(CLEAR, "{ 'X^2+Y^2=1' '(X-1)^2+Y^2=1' }"
              "{ X Y } { 0 0 } ROOT", ENTER)
        .expect("{ X=0.5 Y=0.86602 54037 84 }");

    step("Solving when the variable is initialized with a constant")
        .test(CLEAR, DIRECT("m=Ⓒme "
                            "'MSlv(ⒺRelativityMassEnergy;[E];[1 eV])' "
                            "Eval Pick3 StEq SolvingMenu"), ENTER,
              LSHIFT, F3)
        .expect("9.10938 37139⁳⁻³¹ kg");
    step("Solving with constant initializer, second case (#1418)")
        .test(CLEAR, DIRECT(
                  "θ=40_°  p=1e-23_kg*m/s m=Ⓒme n=2 "
                  "'ROOT(ⒺDe Broglie Wave;[λ;K;v;d];[1_nm;1_eV;1_m/s;1_nm])'"),
              ENTER, ID_Run,
              DIRECT("ⒺDe Broglie Wave STEQ SolvingMenu NextEQ"), ENTER,
              LSHIFT, F3)
        .expect("m=9.10938 37139⁳⁻³¹ kg");

    step("Exit: Clear variables")
        .test(CLEAR, "UPDIR 'SLVTST' PURGE", ENTER);
}


void tests::constants_parsing()
// ----------------------------------------------------------------------------
//   Test that we can parse every single builtin constant
// ----------------------------------------------------------------------------
{
    BEGIN(cstlib);

    size_t nbuiltins = constant::constants.nbuiltins;
    const cstring *cst = constant::constants.builtins;

    for (size_t i = 0; i < nbuiltins; i += 2)
    {
        if (cst[i+1])
        {
            istep(cst[i]);
            test(CLEAR, DIRECT(cst[i+1]), ENTER).noerror();
            test(DIRECT("if dup typename \"array\" = "
                        "then →Num else Run end"), ENTER).noerror();
        }
        else
        {
            begin(cst[i], true);
        }
        if (!ok)
        {
            test(cst[i+1]);
            break;
        }
    }
}


void tests::eqnlib_parsing()
// ----------------------------------------------------------------------------
//   Test that we can parse every single builtin equation
// ----------------------------------------------------------------------------
{
    BEGIN(equations);

    size_t nbuiltins = equation::equations.nbuiltins;
    const cstring *eq = equation::equations.builtins;

    for (size_t i = 0; i < nbuiltins; i += 2)
    {
        if (eq[i+1])
        {
            istep(eq[i]);
            test(CLEAR, DIRECT(eq[i+1]), ENTER).noerror();
        }
        else
        {
            begin(eq[i], true);
        }
        if (!ok)
        {
            test(eq[i+1]);
            break;
        }
    }
}


void tests::eqnlib_columns_and_beams()
// ----------------------------------------------------------------------------
//   Test that the solver works as expected
// ----------------------------------------------------------------------------
{
    BEGIN(colnbeams);

    step("Enter directory for solving")
        .test(CLEAR, "'SLVTST' CRDIR SLVTST", ENTER);

    step("Select CurrentEquationsVariables")
        .test(CLEAR, "CurrentEquationVariables", ENTER);

    step("Solving Elastic Buckling")
        .test(CLEAR, ID_EquationsMenu, F2, RSHIFT, F1)
        .test("53.0967", NOSHIFT, F3)
        .expect("A=53.0967 cm↑2")
        .test("199947961.502", NOSHIFT, F4)
        .expect("E=199 947 961.502 kPa")
        .test(".7", NOSHIFT, F5, F6)
        .expect("K=0.7")
        .test("7.3152", NOSHIFT, F1)
        .expect("L=7.3152 m")
        .test("4.1148", NOSHIFT, F2, F6)
        .expect("r=4.1148 cm")
        .test(LSHIFT, F2).expect("Pcr=676.60192 6324 kN");
    step("Solving Elastic Buckling second equation")
        .test(CLEAR, LSHIFT, F1, LSHIFT, F4)
        .expect("I=8 990 109.72813 mm↑4")
        .test(NOSHIFT, F1)
        .expect("'676.60192 6324 kN"
                "=6.76601 92632 4⁳¹⁴ kPa·mm↑4/m↑2"
                "+0.00000 0005 kPa·mm↑4/m↑2'");
    step("Solving Elastic Buckling third equation")
        .test(CLEAR, LSHIFT, F1, LSHIFT, F2)
        .expect("σcr=127 428.24437 8 kPa")
        .test(NOSHIFT, F1)
        .expect("'127 428.24437 8 kPa=12.74282 44378 kN/cm↑2'");
    step("Solving Elastic Buckling fourth equation")
        .test(CLEAR, LSHIFT, F1, LSHIFT, F4)
        .expect("r=4.1148 cm")
        .test(NOSHIFT, F1)
        .expect("'16.93157 904 cm↑2=169 315.7904 mm↑4/cm↑2+5.⁳⁻¹⁸ mm↑4/cm↑2'");

    step("Solving Eccentric Columns")
        .test(CLEAR, ID_EquationsMenu, F2, RSHIFT, F2)
        .test("1.1806", NOSHIFT, F3)
        .expect("ε=1.1806 cm")
        .test("187.9351", NOSHIFT, F4)
        .expect("A=187.9351 cm↑2")
        .test("15.24", NOSHIFT, F5, F6)
        .expect("c=15.24 cm")
        .test("206842718.795", NOSHIFT, F1)
        .expect("E=206 842 718.795 kPa")
        .test("1", NOSHIFT, F2)
        .expect("K=1")
        .test("6.6542", NOSHIFT, F3)
        .expect("L=6.6542 m")
        .test("1908.2571", NOSHIFT, F4)
        .expect("P=1 908.2571 kN")
        .test("8.4836", NOSHIFT, F5)
        .expect("r=8.4836 cm")
        .test(F6, LSHIFT, F2)
        .expect("σmax=140 853.09700 6 kPa");
    step("Solving Eccentric Column second equation")
        .test(CLEAR, LSHIFT, F1, LSHIFT, F3)
        .expect("I=135 259 652.161 mm↑4");

    step("Solving Simple Deflection")
        .test(CLEAR, ID_EquationsMenu, F2, RSHIFT, F3)
        .test("10_ft", NOSHIFT, F2)
        .expect("a=10 ft")
        .test("17_ft", NOSHIFT, F3)
        .expect("c=17 ft")
        .test("29000000_psi", NOSHIFT, F4)
        .expect("E=29 000 000 psi")
        .test("40_in^4", NOSHIFT, F5, F6)
        .expect("I=40 in↑4")
        .test("20_ft", NOSHIFT, F1)
        .expect("L=20 ft")
        .test("3687.81_ft*lbf", NOSHIFT, F2)
        .expect("M=3 687.81 ft·lbf")
        .test("674.427_lbf", NOSHIFT, F3)
        .expect("P=674.427 lbf")
        .test("102.783_lbf/ft", NOSHIFT, F4)
        .expect("w=102.783 lbf/ft")
        .test("9_ft", NOSHIFT, F5)
        .expect("x=9 ft")
        .test(F6, LSHIFT, F1)
        .expect("y=-1.52523 29401 2 cm")
        .test("1_in", F1)
        .expect("y=1 in")
        .test(LSHIFT, F1)
        .expect("y=-0.60048 54094 96 in");

    step("Solving Simple Slope")
        .test(CLEAR, ID_EquationsMenu, F2, RSHIFT, F4)
        .test("10_ft", NOSHIFT, F3)
        .expect("a=10 ft")
        .test("17_ft", NOSHIFT, F4)
        .expect("c=17 ft")
        .test("29000000_psi", NOSHIFT, F5, F6)
        .expect("E=29 000 000 psi")
        .test("40_in^4", NOSHIFT, F1)
        .expect("I=40 in↑4")
        .test("20_ft", NOSHIFT, F2)
        .expect("L=20 ft")
        .test("3687.81_ft*lbf", NOSHIFT, F3)
        .expect("M=3 687.81 ft·lbf")
        .test("674.427_lbf", NOSHIFT, F4)
        .expect("P=674.427 lbf")
        .test("102.783_lbf/ft", NOSHIFT, F5, F6)
        .expect("w=102.783 lbf/ft")
        .test("9_ft", NOSHIFT, F1)
        .expect("x=9 ft")
        .test(F6, LSHIFT, F2)
        .expect("θ=-0.08763 17825 27 °");

    step("Solving Simple Moment")
        .test(CLEAR, ID_EquationsMenu, F2, RSHIFT, F5)
        .test("20_ft", NOSHIFT, F5)
        .expect("L=20 ft")
        .test("10_ft", NOSHIFT, F3)
        .expect("a=10 ft")
        .test("674.427_lbf", NOSHIFT, F6, F2)
        .expect("P=674.427 lbf")
        .test("17_ft", NOSHIFT, F6, F4)
        .expect("c=17 ft")
        .test("3687.81_ft*lbf", NOSHIFT, F6, F1)
        .expect("M=3 687.81 ft·lbf")
        .test("102.783_lbf/ft", NOSHIFT, F3)
        .expect("w=102.783 lbf/ft")
        .test("9_ft", NOSHIFT, F4)
        .expect("x=9 ft")
        .test(F6, LSHIFT, F2)
        .expect("Mx=13 262.87487 72 N·m")
        .test("1_ft*lbf", NOSHIFT, F2)
        .expect("Mx=1 ft·lbf")
        .test(LSHIFT, F2)
        .expect("Mx=9 782.1945 ft·lbf");

    step("Solving Simple Shear")
        .test(CLEAR, EXIT, ID_EquationsMenu, F2, F6, RSHIFT, F1)
        .test("20_ft", NOSHIFT, F3)
        .expect("L=20 ft")
        .test("10_ft", NOSHIFT, F2)
        .expect("a=10 ft")
        .test("674.427_lbf", NOSHIFT, F5)
        .expect("P=674.427 lbf")
        .test("3687.81_ft*lbf", NOSHIFT, F4)
        .expect("M=3 687.81 ft·lbf")
        .test("102.783_lbf/ft", NOSHIFT, F6, F2)
        .expect("w=102.783 lbf/ft")
        .test("9_ft", NOSHIFT, F3)
        .expect("x=9 ft")
        .test(LSHIFT, F1)
        .expect("V=2 777.41174 969 N")
        .test("1_lbf", F1)
        .expect("V=1 lbf")
        .test(LSHIFT, F1)
        .expect("V=624.387 lbf");

    step("Solving Cantilever Deflection")
        .test(CLEAR, EXIT, ID_EquationsMenu, F2, F6, RSHIFT, F2)
        .test("10_ft", NOSHIFT, F6, F1)
        .expect("L=10 ft")
        .test("29000000_psi", NOSHIFT, F6, F6, F4)
        .expect("E=29 000 000 psi")
        .test("15_in^4", NOSHIFT, F5)
        .expect("I=15 in↑4")
        .test("500_lbf", NOSHIFT, F6, F3)
        .expect("P=500 lbf")
        .test("800_ft*lbf", NOSHIFT, F2)
        .expect("M=800 ft·lbf")
        .test("3_ft", NOSHIFT, F6, F6, F2)
        .expect("a=3 ft")
        .test("6_ft", NOSHIFT, F3)
        .expect("c=6 ft")
        .test("100_lbf/ft", NOSHIFT, F6, F4)
        .expect("w=100 lbf/ft")
        .test("8_ft", NOSHIFT, F5)
        .expect("x=8 ft")
        .test(F6, LSHIFT, F1)
        .expect("y=-0.33163 03448 28 in")
        .test("1_lbf", F1)
        .error("Inconsistent units")
        .test(CLEAR, "1_cm", F1)
        .expect("y=1 cm")
        .test(LSHIFT, F1)
        .expect("y=-0.84234 10758 62 cm");

    step("Solving Cantilever Slope")
        .test(CLEAR, EXIT, ID_EquationsMenu, F2, F6, RSHIFT, F3)
        .test("10_ft", NOSHIFT, F6, F2)
        .expect("L=10 ft")
        .test("29000000_psi", LSHIFT, F6, F5)
        .expect("E=29 000 000 psi")
        .test("15_in^4", NOSHIFT, F6, F1)
        .expect("I=15 in↑4")
        .test("500_lbf", NOSHIFT, F4)
        .expect("P=500 lbf")
        .test("800_ft*lbf", NOSHIFT, F3)
        .expect("M=800 ft·lbf")
        .test("3_ft", LSHIFT, F6, F3)
        .expect("a=3 ft")
        .test("6_ft", NOSHIFT, F4)
        .expect("c=6 ft")
        .test("100_lbf/ft", NOSHIFT, F6, F5)
        .expect("w=100 lbf/ft")
        .test("8_ft", NOSHIFT, F6, F1)
        .expect("x=8 ft")
        .test(F6, LSHIFT, F2)
        .expect("θ=-0.26522 01876 49 °");

    step("Solving Cantilever Moment")
        .test(CLEAR, EXIT, ID_EquationsMenu, F2, F6, RSHIFT, F4)
        .test("10_ft", NOSHIFT, F5)
        .expect("L=10 ft")
        .test("500_lbf", NOSHIFT, F6, F2)
        .expect("P=500 lbf")
        .test("800_ft*lbf", NOSHIFT, F1)
        .expect("M=800 ft·lbf")
        .test("3_ft", LSHIFT, F6, F3)
        .expect("a=3 ft")
        .test("6_ft", NOSHIFT, F4)
        .expect("c=6 ft")
        .test("100_lbf/ft", NOSHIFT, F6, F3)
        .expect("w=100 lbf/ft")
        .test("8_ft", NOSHIFT, F4)
        .expect("x=8 ft")
        .test(F6, LSHIFT, F2)
        .expect("Mx=-200 ft·lbf");

    step("Solving Cantilever Shear")
        .test(CLEAR, EXIT, ID_EquationsMenu, F2, F6, RSHIFT, F5)
        .test("10_ft", NOSHIFT, F3)
        .expect("L=10 ft")
        .test("500_lbf", NOSHIFT, F4)
        .expect("P=500 lbf")
        .test("3_ft", NOSHIFT, F2)
        .expect("a=3 ft")
        .test("8_ft", NOSHIFT, F6, F2)
        .expect("x=8 ft")
        .test("100_lbf/ft", NOSHIFT, F1)
        .expect("w=100 lbf/ft")
        .test(F6, LSHIFT, F5)
        .expect("V=200 lbf");

    step("Exit: Clear variables")
        .test(CLEAR,
              "UPDIR "
              "'SLVTST' PURGE "
              "'CurrentEquationVariables' PURGE", ENTER);
}


void tests::numerical_integration()
// ----------------------------------------------------------------------------
//   Test that the numerical integration function works as expected
// ----------------------------------------------------------------------------
{
    BEGIN(integrate);

    step("Disable symbolic integration")
        .test(CLEAR, DIRECT("NumericalIntegration"), ENTER);
    step("Integrate with expression")
        .test(CLEAR, "1 2 '1/X' 'X' INTEGRATE", ENTER)
        .noerror().expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("1.55318 8⁳⁻¹⁸");
    step("Integration through menu")
        .test(CLEAR, 2, ENTER).expect("2")
        .test(3, ENTER).expect("3")
        .test("'sq(Z)+Z'", ENTER).expect("'Z²+Z'")
        .test(F, ALPHA, Z, ENTER).expect("'Z'")
        .test(ID_IntegrationMenu, ID_Integrate).expect("8.83333 33333 3", 350);
    step("Integration with decimals")
        .test(CLEAR, "2.", ENTER).expect("2.")
        .test("3.", ENTER).expect("3.")
        .test("'sq(Z)+Z'", ENTER).expect("'Z²+Z'")
        .test(F, ALPHA, Z, ENTER).expect("'Z'")
        .test(ID_IntegrationMenu, ID_Integrate).expect("8.83333 33333 3", 350);

    step("Integrate with low precision")
        .test(CLEAR, "18 IntegrationImprecision", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 63")
        .test(KEY2, ID_ln, ID_subtract).expect("3.25558 45962 2⁳⁻¹²");
    step("Integrate with high precision")
        .test(CLEAR, "1 IntegrationImprecision  24 Sig", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .error("Numerical precision lost");
    step("Integrate with limited loops")
        .test(CLEAR, "15 IntegrationImprecision", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 59937 67585 3282")
        .test(KEY2, ID_ln, ID_subtract).expect("-7.63356 395⁳⁻¹⁵")
        .test("5 IntegrationIterations", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .error("Numerical precision lost");
    step("Integrate with restored settings")
        .test(CLEAR,
              "{ IntegrationImprecision IntegrationIterations } Purge Std",
              ENTER).noerror()
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("1.55318 8⁳⁻¹⁸");

    step("Integrate with display-induced imprecision")
        .test(CLEAR, "3 FIX", ENTER).noerror()
        .test("1 2 '1/X' 'X' ∫", ENTER).expect("6.931⁳⁻¹")
        .test(ID_DisplayModesMenu, ID_Std).expect("0.69314 56549 79")
        .test(KEY2, ID_ln, ID_subtract).expect("-0.00000 15255 81");


    step("Integration with error on low bound")
        .test(CLEAR, "0 1 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .expect("0.01745 29971 57");
    step("Integration with error on high bound")
        .test(CLEAR, "1 0 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .expect("-0.01745 29971 57");
    step("Integration with error on difference")
        .test(CLEAR, "1_m 1_h 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .error("Inconsistent units");

    step("Integrate with symbols")
        .test(CLEAR, "A B '1/X' 'X' ∫", ENTER)
        .expect("'∫(A;B;1÷X;X)'")
        .test(DOWN)
        .editor("'∫(A;B;1÷X;X)'")
        .test(ENTER)
        .expect("'∫(A;B;1÷X;X)'");
    step("Integrate with one symbol")
        .test(CLEAR, "1 B '1/X' 'X' ∫", ENTER)
        .expect("'∫(1;B;1÷X;X)'")
        .test(DOWN)
        .editor("'∫(1;B;1÷X;X)'")
        .test(ENTER)
        .expect("'∫(1;B;1÷X;X)'");
    step("Integrate with second symbol")
        .test(CLEAR, "A 1 '1/X' 'X' ∫", ENTER)
        .expect("'∫(A;1;1÷X;X)'")
        .test(DOWN)
        .editor("'∫(A;1;1÷X;X)'")
        .test(ENTER)
        .expect("'∫(A;1;1÷X;X)'");

    step("Check evaluation with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "-3 SF", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("22.14069 26328");
    step("Check inference variable with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "-3 SF 3 'X' STO", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("22.14069 26328");
    step("Check evaluation without NumericalResults flag clear")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("'∫(0;π;exp X;X)'")
        .test(ID_ToDecimal)
        .expect("22.14069 26328");
    step("Check inference variable with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "3 'X' STO", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("'∫(0;π;exp X;X)'")
        .test(ID_ToDecimal)
        .expect("22.14069 26328");
    step("Cleanup & restore symbolic integration")
        .test(CLEAR, DIRECT("{ X NumericalIntegration } Purge"), ENTER);
}


void tests::symbolic_numerical_integration()
// ----------------------------------------------------------------------------
//   Test symbolic-assisted numerical integration
// ----------------------------------------------------------------------------
{
    BEGIN(syminteg);

    step("Enable symbolic integration")
        .test(CLEAR, DIRECT("SymbolicIntegration"), ENTER);
    step("Integrate with expression")
        .test(CLEAR, "1 2 '1/X' 'X' INTEGRATE", ENTER)
        .noerror().expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("0");
    step("Integration through menu")
        .test(CLEAR, 2, ENTER).expect("2")
        .test(3, ENTER).expect("3")
        .test("'sq(Z)+Z'", ENTER).expect("'Z²+Z'")
        .test(F, ALPHA, Z, ENTER).expect("'Z'")
        .test(ID_IntegrationMenu, ID_Integrate).expect("8 ⁵/₆", 350);
    step("Integration with decimals")
        .test(CLEAR, "2.", ENTER).expect("2.")
        .test("3.", ENTER).expect("3.")
        .test("'sq(Z)+Z'", ENTER).expect("'Z²+Z'")
        .test(F, ALPHA, Z, ENTER).expect("'Z'")
        .test(ID_IntegrationMenu, ID_Integrate).expect("8.83333 33333 3", 350);

    step("Integrate with low precision")
        .test(CLEAR, "18 IntegrationImprecision", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("0");
    step("Integrate with high precision")
        .test(CLEAR, "1 IntegrationImprecision  24 Sig", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .expect("0.69314 71805 59945 30941 7232");
    step("Integrate with limited loops")
        .test(CLEAR, "15 IntegrationImprecision", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 59945 30941 7232")
        .test(KEY2, ID_ln, ID_subtract).expect("0")
        .test("5 IntegrationIterations", ENTER)
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .expect("0.69314 71805 59945 30941 7232");
    step("Integrate with restored settings")
        .test(CLEAR,
              "{ IntegrationImprecision IntegrationIterations } Purge Std",
              ENTER).noerror()
        .test("1 2 '1/X' 'X' ∫", ENTER)
        .noerror().expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("0");

    step("Integrate with display-induced imprecision")
        .test(CLEAR, "3 FIX", ENTER).noerror()
        .test("1 2 '1/X' 'X' ∫", ENTER).expect("6.931⁳⁻¹")
        .test(ID_DisplayModesMenu, ID_Std).expect("0.69314 71805 6")
        .test(KEY2, ID_ln, ID_subtract).expect("0");


    step("Integration with error on low bound")
        .test(CLEAR, "0 1 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .expect("0.01745 29971 57");
    step("Integration with error on high bound")
        .test(CLEAR, "1 0 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .expect("-0.01745 29971 57");
    step("Integration with error on difference")
        .test(CLEAR, "1_m 1_h 'sin(x)/x' 'x'", ENTER)
        .test(ID_IntegrationMenu, ID_Integrate)
        .error("Inconsistent units");

    step("Integrate with symbols")
        .test(CLEAR, "A B '1/X' 'X' ∫", ENTER)
        .expect("'ln (abs B)-ln (abs A)'")
        .test(DOWN)
        .editor("'ln (abs B)-ln (abs A)'")
        .test(ENTER)
        .expect("'ln (abs B)-ln (abs A)'");
    step("Integrate with one symbol")
        .test(CLEAR, "1 B '1/X' 'X' ∫", ENTER)
        .expect("'ln (abs B)'")
        .test(DOWN)
        .editor("'ln (abs B)'")
        .test(ENTER)
        .expect("'ln (abs B)'");
    step("Integrate with second symbol")
        .test(CLEAR, "A 1 '1/X' 'X' ∫", ENTER)
        .expect("'-ln (abs A)'")
        .test(DOWN)
        .editor("'-ln (abs A)'")
        .test(ENTER)
        .expect("'-ln (abs A)'");

    step("Check evaluation with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "-3 SF", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("22.14069 26328");
    step("Check inference variable with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "-3 SF 3 'X' STO", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("22.14069 26328");
    step("Check evaluation without NumericalResults flag clear")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("'exp π-1.'")
        .test(ID_ToDecimal)
        .expect("22.14069 26328");
    step("Check inference variable with NumericalResults flag set")
        .test(CLEAR, "-3 CF", ENTER,
              "0 Ⓒπ 'EXP(X)' 'X'", ENTER,
              "3 'X' STO", ENTER,
              ID_IntegrationMenu, ID_Integrate)
        .expect("'exp π-1.'")
        .test(ID_ToDecimal)
        .expect("22.14069 26328");
    step("Cleanup & restore symbolic integration")
        .test(CLEAR, DIRECT("{ X NumericalIntegration } PURGE"), ENTER);
}


void tests::auto_simplification()
// ----------------------------------------------------------------------------
//   Check auto-simplification rules for arithmetic
// ----------------------------------------------------------------------------
{
    BEGIN(simplify);

    step("Enable auto simplification");
    test(CLEAR, "AutoSimplify", ENTER).noerror();

    step("Limit number of iterations in polynomials (bug #1047)")
        .test(CLEAR, "X 3", ID_pow, KEY4, DIV, "X", ID_subtract, KEY1, ADD)
        .expect("'X↑3÷4-X+1'");

    step("X + 0 = X");
    test(CLEAR, "X 0 +", ENTER).expect("'X'");

    step("0 + X = X");
    test(CLEAR, "0 X +", ENTER).expect("'X'");

    step("X - 0 = X");
    test(CLEAR, "X 0 -", ENTER).expect("'X'");

    step("0 - X = -X");
    test(CLEAR, "0 X -", ENTER).expect("'-X'");

    step("X - X = 0");
    test(CLEAR, "X X -", ENTER).expect("0");

    step("0 * X = 0");
    test(CLEAR, "0 X *", ENTER).expect("0");

    step("X * 0 = 0");
    test(CLEAR, "X 0 *", ENTER).expect("0");

    step("1 * X = X");
    test(CLEAR, "1 X *", ENTER).expect("'X'");

    step("X * 1 = X");
    test(CLEAR, "X 1 *", ENTER).expect("'X'");

    step("X * X = sq(X)");
    test(CLEAR, "X sin 1 * X 0 + sin *", ENTER).expect("'(sin X)²'");

    step("0 / X = -");
    test(CLEAR, "0 X /", ENTER).expect("0");

    step("X / 1 = X");
    test(CLEAR, "X 1 /", ENTER).expect("'X'");

    step("1 / X = inv(X)");
    test(CLEAR, "1 X sin /", ENTER).expect("'(sin X)⁻¹'");

    step("X / X = 1");
    test(CLEAR, "X cos 1 * X 0 + cos /", ENTER).expect("1");

    step("1.0 == 1");
    test(CLEAR, "1.0000 X * ", ENTER).expect("'X'");

    step("0.0 == 0 (but preserves types)");
    test(CLEAR, "0.0000 X * ", ENTER).expect("0.");

    step("i*i == -1");
    test(CLEAR, "ⅈ", ENTER, ENTER, MUL).expect("-1");

    step("i*i == -1 (symbolic constant)");
    test(CLEAR, LSHIFT, I, F2, F3, ENTER, MUL).expect("-1");

    step("Simplification of rectangular real-only results");
    test(CLEAR, "0ⅈ3 0ⅈ5", ENTER, MUL).expect("-15");
    test(CLEAR, "0ⅈ3 0-ⅈ5", ENTER, MUL).expect("15");

    step("Simplification of polar real-only results");
    test(CLEAR, "2∡90 3∡90", ENTER, MUL).expect("-6");
    test(CLEAR, "2∡90 3∡-90", ENTER, MUL).expect("6");

    step("Applies when building a matrix");
    test(CLEAR, "[[3 0 2][2 0 -2][ 0 1 1 ]] [x y z] *", ENTER)
        .expect("[ '3·x+2·z' '2·x+-2·z' 'y+z' ]");

    step("Does not reduce matrices");
    test(CLEAR, "[a b c] 0 *", ENTER).expect("[ 0 0 0 ]");

    step("Does not apply to text");
    test(CLEAR, "\"Hello\" 0 +", ENTER)
        .expect("\"Hello0\"");

    step("Does not apply to lists");
    test(CLEAR, "{ 1 2 3 } 0 +", ENTER)
        .expect("{ 1 2 3 0 }");

    step("Do not apply to infinities")
        .test(CLEAR, "Ⓒ∞ Ⓒ∞ +", ENTER)
        .expect("'∞+∞'")
        .test(CLEAR, "0 Ⓒ∞ +", ENTER)
        .expect("'0+∞'")
        .test(CLEAR, "Ⓒ∞ 0 +", ENTER)
        .expect("'∞+0'")
        .test(CLEAR, "Ⓒ∞ Ⓒ∞ -", ENTER)
        .expect("'∞-∞'")
        .test(CLEAR, "Ⓒ∞ Ⓒ∞ *", ENTER)
        .expect("'∞·∞'")
        .test(CLEAR, "Ⓒ∞ Ⓒ∞ /", ENTER)
        .expect("'∞÷∞'")
        .test(CLEAR, "Ⓒ∞ Ⓒ∞ NEG /", ENTER)
        .expect("'∞÷(-∞)'");

    step("Fold constants: additions")
        .test(CLEAR, "'1+X+2'", ENTER).expect("'1+X+2'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+3'");
    step("Fold constants: subtractions")
        .test(CLEAR, "'2+X-1'", ENTER).expect("'2+X-1'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+1'");
    step("Fold constants: multiplications")
        .test(CLEAR, "'2*X*3'", ENTER).expect("'2·X·3'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'6·X'");
    step("Fold constants: divisions")
        .test(CLEAR, "'4*X/2'", ENTER).expect("'4·X÷2'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'2·X'");
    step("Fold constants: power")
        .test(CLEAR, "'X*2^3'", ENTER).expect("'X·2↑3'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'8·X'");
    step("Fold constants: complicated expression")
        .test(CLEAR, "'X*2^3+3+2*X-1'", ENTER).expect("'X·2↑3+3+2·X-1'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'10·X+2'");

    step("Zero elimination")
        .test(CLEAR, "'X-0+Y+0'", ENTER).expect("'X-0+Y+0'")
        .test(RUNSTOP).expect("'X+Y'");
    step("Adding to self")
        .test(CLEAR, "'(X+X)+(X+X)'", ENTER).expect("'X+X+(X+X)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'4·X'");
    step("Subtracting to self")
        .test(CLEAR, "'(X+X)-(X*2)'", ENTER).expect("'X+X-X·2'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'0'");
    step("Reordering terms")
        .test(CLEAR, "'(4+X+3)+2*3'", ENTER).expect("'4+X+3+2·3'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+13'");
    step("Double-negation")
        .test(CLEAR, "'-(-(-((4+X+3)+2*3)))'", ENTER).expect("'-(-(-(4+X+3+2·3)))'")
        .test(RUNSTOP).expect("'-(X+13)'");
    step("Cancelled invert")
        .test(CLEAR, "'inv(inv(X))'", ENTER).expect("'(X⁻¹)⁻¹'")
        .test(RUNSTOP).expect("'X'");
    step("Cancelled division")
        .test(CLEAR, "'7/(7/X)'", ENTER).expect("'7÷(7÷X)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X'");
    step("Reversed division")
        .test(CLEAR, "'7/(3/X)'", ENTER).expect("'7÷(3÷X)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'⁷/₃·X'");
    step("Factoring terms")
        .test(CLEAR, "'3*X+2*X-X+3*X+X'", ENTER).expect("'3·X+2·X-X+3·X+X'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'8·X'");
    step("Auto-squaring and auto-cubing")
        .test(CLEAR, "'A*A*A+B*B'", ENTER).expect("'A·A·A+B·B'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'A³+B²'");
    step("Auto-powers")
        .test(CLEAR, "'Z*(Z*Z*Z)*(Z*Z)*Z'", ENTER).expect("'Z·(Z·Z·Z)·(Z·Z)·Z'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'Z↑7'");
    step("Auto-powers from square and cubed")
        .test(CLEAR, "'sq A * cubed B'", ENTER).expect("'A²·B³'")
        .test(RUNSTOP).expect("'A²·B³'");
    step("Divide by self is one")
        .test(CLEAR, "'sin(X)/sin(0+X)'", ENTER).expect("'sin X÷sin(0+X)'")
        .test(RUNSTOP).expect("1");
    step("Power simplification")
        .test(CLEAR, "'X^3*X+Y*Y^5'", ENTER).expect("'X↑3·X+Y·Y↑5'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X↑4+Y↑6'");
    step("Power one and power zero")
        .test(CLEAR, "'X^0+Y^1'", ENTER).expect("'X↑0+Y↑1'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'Y+1'");

    step("sin simplification")
        .test(CLEAR, "'1+sin(asin(X))+asin(sin(Y))'", ENTER)
        .expect("'1+sin (sin⁻¹ X)+sin⁻¹ (sin Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+sin⁻¹ (sin Y)+1'");
    step("cos simplification")
        .test(CLEAR, "'1+cos(acos(X))+acos(cos(Y))'", ENTER)
        .expect("'1+cos (cos⁻¹ X)+cos⁻¹ (cos Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+cos⁻¹ (cos Y)+1'");
    step("tan simplification")
        .test(CLEAR, "'1+tan(atan(X))+atan(tan(Y))'", ENTER)
        .expect("'1+tan (tan⁻¹ X)+tan⁻¹ (tan Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+tan⁻¹ (tan Y)+1'");

    step("sinh simplification")
        .test(CLEAR, "'1+sinh(asinh(X))+asinh(sinh(Y))'", ENTER)
        .expect("'1+sinh (sinh⁻¹ X)+sinh⁻¹ (sinh Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+Y+1'");
    step("cosh simplification")
        .test(CLEAR, "'1+cosh(acosh(X))+acosh(cosh(Y))'", ENTER)
        .expect("'1+cosh (cosh⁻¹ X)+cosh⁻¹ (cosh Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+Y+1'");
    step("tanh simplification")
        .test(CLEAR, "'1+tanh(atanh(X))+atanh(tanh(Y))'", ENTER)
        .expect("'1+tanh (tanh⁻¹ X)+tanh⁻¹ (tanh Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'X+Y+1'");

    step("abs simplification")
        .test(CLEAR, "'1+abs(abs(X))+abs(-Y)'", ENTER)
        .expect("'1+abs (abs X)+abs(-Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'abs X+abs Y+1'");

    step("sqrt simplification")
        .test(CLEAR, "'1+abs(abs(X))+abs(-Y)'", ENTER)
        .expect("'1+abs (abs X)+abs(-Y)'")
        .test(ID_SymbolicMenu, ID_Simplify).expect("'abs X+abs Y+1'");

    step("Disable auto simplification");
    test(CLEAR, "NoAutoSimplify", ENTER).noerror();

    step("When disabled, get the complicated expression");
    test(CLEAR, "[[3 0 2][2 0 -2][ 0 1 1 ]] [x y z] *", ENTER)
        .expect("[ '3·x+0·y+2·z' '2·x+0·y+-2·z' '0·x+1·y+1·z' ]");

    step("Re-enable auto simplification");
    test(CLEAR, "AutoSimplify", ENTER).noerror();
}


void tests::rewrite_engine()
// ----------------------------------------------------------------------------
//   Equation rewrite engine
// ----------------------------------------------------------------------------
{
    BEGIN(rewrites);

    step("Single replacement");
    test(CLEAR, "'A+B' { 'X+Y' 'Y-sin X' } ↓match", ENTER)
        .expect("1")
        .test(BSP)
        .expect("'B-sin A'");

    step("In-depth replacement");
    test(CLEAR, " 'A*(B+C)' { 'X+Y' 'Y-sin X' } ↓Match", ENTER)
        .expect("1")
        .test(BSP)
        .expect("'A·(C-sin B)'");

    step("Variable matching");
    test(CLEAR, "'A*(B+C)' { 'X+X' 'X-sin X' }", RSHIFT, KEY7, F6, F1)
        .expect("0")
        .test(BSP)
        .expect("'A·(B+C)'");
    test(CLEAR, "'A*(B+(B))' { 'X+X' 'X-sin X' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'A·(B-sin B)'");

    step("Constant folding");
    test(CLEAR, "'A+B+0' { 'X+0' 'X' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'A+B'");

    step("Clearing final flag")
        .test(CLEAR, "-100 CF", ENTER).noerror();
    step("Single substitutions (down)");
    test(CLEAR, "'A+B+C' { 'X+Y' 'Y-X' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'C-(A+B)'");
    step("Multiple substitutions (up)");
    test(CLEAR, "'A+B+C' { 'X+Y' 'Y-X' }", RSHIFT, KEY7, F6, F2)
        .expect("1")
        .test(BSP)
        .expect("'B-A+C'");
    step("Setting final flag")
        .test(CLEAR, "-100 SF", ENTER).noerror();
    step("Multiple substitutions (down repeat)");
    test(CLEAR, "'A+B+C' { 'X+Y' 'Y-X' }", RSHIFT, KEY7, F6, F1)
        .expect("2")
        .test(BSP)
        .expect("'C-(B-A)'");
    step("Multiple substitutions (up repeat)");
    test(CLEAR, "'A+B+C' { 'X+Y' 'Y-X' }", RSHIFT, KEY7, F6, F1)
        .expect("2")
        .test(BSP)
        .expect("'C-(B-A)'");
    step("Setting step by step flag")
        .test(CLEAR, "StepByStepAlgebraResults", ENTER).noerror();

    step("Deep substitution");
    test(CLEAR, "'tan(A-B)+3' { 'X-Y' '-Y+X' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'tan(-B+A)+3'");
    step("Deep substitution with multiple changes (down single)");
    test(CLEAR, "StepByStepAlgebraResults", ENTER,
         "'5+tan(A-B)+(3-sin(C+D-A))' { 'X-Y' '-Y+X' }",
         RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'5+tan(A-B)+(-sin(C+D-A)+3)'");
    step("Deep substitution with multiple changes (up single)");
    test(CLEAR, "StepByStepAlgebraResults", ENTER,
         "'5+tan(A-B)+(3-sin(C+D-A))' { 'X-Y' '-Y+X' }",
         RSHIFT, KEY7, F6, F2)
        .expect("1")
        .test(BSP)
        .expect("'5+tan(-B+A)+(3-sin(C+D-A))'");
    step("Deep substitution with multiple changes (down multiple)");
    test(CLEAR, "FinalAlgebraResults", ENTER,
         "'5+tan(A-B)+(3-sin(C+D-A))' { 'X-Y' '-Y+X' }",
         RSHIFT, KEY7, F6, F1)
        .expect("3")
        .test(BSP)
        .expect("'5+tan(-B+A)+(-sin(-A+(C+D))+3)'");
    step("Deep substitution with multiple changes (up multiple)");
    test(CLEAR, "FinalAlgebraResults", ENTER,
         "'5+tan(A-B)+(3-sin(C+D-A))' { 'X-Y' '-Y+X' }",
         RSHIFT, KEY7, F6, F2)
        .expect("3")
        .test(BSP)
        .expect("'5+tan(-B+A)+(-sin(-A+(C+D))+3)'");

    step("Matching integers");
    test(CLEAR, "'(A+B)^3' { 'X^K' 'X*X^(K-1)' }", RSHIFT, KEY7, F6, F1)
        .expect("3")
        .test(BSP)
        .expect("'(A+B)·((A+B)·((A+B)·(A+B)↑(1-1)))'");

    step("Matching sorted integers (success)");
    test(CLEAR, "'3+5' { 'i+j' '21*(j-i)' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'42'");
    step("Matching sorted integers (failing)");
    test(CLEAR, "'5+3' { 'i+j' '21*(j-i)' }", RSHIFT, KEY7, F6, F1)
        .expect("0")
        .test(BSP)
        .expect("'5+3'");

    step("Matching unique terms");
    test(CLEAR, "'(A+B+A)' { 'X+U+X' '2*X+U' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP)
        .expect("'2·A+B'");
    test(CLEAR, "'(A+A+A)' { 'X+U+X' '2*X+U' }", RSHIFT, KEY7, F6, F1)
        .expect("0")
        .test(BSP)
        .expect("'A+A+A'");

    step("Clearing flag -100")
        .test(CLEAR, "-100 CF", ENTER);
    step("Matching down")
        .test(CLEAR, "'A+B+C' { 'X+Y' 'Y-(-X)' }", RSHIFT, KEY7, F6, F1)
        .expect("1")
        .test(BSP).expect("'C-(-(A+B))'");
    step("Matching up")
        .test(CLEAR, "'A+B+C' { 'X+Y' 'Y-(-X)' }", RSHIFT, KEY7, F6, F2)
        .expect("1")
        .test(BSP)
        .expect("'B-(-A)+C'");

    step("Returning to default")
        .test(CLEAR, "'FinalAlgebraResults' Purge", ENTER).noerror();
    step("Matching with conditions")
        .test(CLEAR,
              "'cos(2*A)+cos(3*B)+sin(4*C)' "
              "{ 'K*Y' '(K-1)*Y+Y' 'K>2' } "
              "↓match", ENTER)
        .expect("3")
        .test(BSP).expect("'cos(2·A)+cos((3-1)·B+B)+sin((3-1)·C+C+C)'");

    step("Setting ExplicitWildcards to match with &Wildcard")
        .test(CLEAR,
              "ExplicitWildcards", ENTER).noerror();
    step("Matching names no longer works")
        .test("'cos(2*A)+cos(3*B)+sin(4*C)' "
              "{ 'N*Y' '(N-1)*Y+Y' } "
              "↓match", ENTER)
        .expect("0")
        .test(BSP).expect("'cos(2·A)+cos(3·B)+sin(4·C)'");
    step("Matching explicit wildcards now works")
        .test("'cos(2*A)+cos(3*B)+sin(4*C)' "
              "{ '&K*&Y' '(&K-1)*&Y+&Y' } "
              "↓match", ENTER)
        .expect("9")
        .test(BSP).expect("'cos((1-1)·A+A+A)+cos((1-1)·B+B+B+B)+sin((1-1)·C+C+C+C+C)'");
    step("Restoring default for wildcards")
        .test(CLEAR, "'ExplicitWildcards' Purge", ENTER).noerror();
}


void tests::symbolic_operations()
// ----------------------------------------------------------------------------
//   Equation rewrite engine
// ----------------------------------------------------------------------------
{
    BEGIN(symbolic);

    step("Simple arithmetic - Symbol and constant")
        .test(CLEAR, "'A' 3 +", ENTER).expect("'A+3'")
        .test(CLEAR, "'A' 3 -", ENTER).expect("'A-3'")
        .test(CLEAR, "'A' 3 *", ENTER).expect("'A·3'")
        .test(CLEAR, "'A' 3 /", ENTER).expect("'A÷3'")
        .test(CLEAR, "'A' 3 ↑", ENTER).expect("'A↑3'");
    step("Simple arithmetic - Constant and symbol")
        .test(CLEAR, "3 'A' +", ENTER).expect("'3+A'")
        .test(CLEAR, "3 'A' -", ENTER).expect("'3-A'")
        .test(CLEAR, "3 'A' *", ENTER).expect("'3·A'")
        .test(CLEAR, "3 'A' /", ENTER).expect("'3÷A'")
        .test(CLEAR, "3 'A' ↑", ENTER).expect("'3↑A'");
    step("Simple arithmetic - Symbol and symbol")
        .test(CLEAR, "'A' 'B' +", ENTER).expect("'A+B'")
        .test(CLEAR, "'A' 'B' -", ENTER).expect("'A-B'")
        .test(CLEAR, "'A' 'B' *", ENTER).expect("'A·B'")
        .test(CLEAR, "'A' 'B' /", ENTER).expect("'A÷B'")
        .test(CLEAR, "'A' 'B' ↑", ENTER).expect("'A↑B'");
   step("Simple functions")
       .test(CLEAR, "'A'", ENTER, J).expect("'sin A'")
       .test(K, L).expect("'tan (cos (sin A))'");

    step("Simple arithmetic on equations")
        .test(CLEAR, "'A=B' 3 +", ENTER).expect("'A+3=B+3'")
        .test(CLEAR, "'A=B' 3 -", ENTER).expect("'A-3=B-3'")
        .test(CLEAR, "'A=B' 3 *", ENTER).expect("'A·3=B·3'")
        .test(CLEAR, "'A=B' 3 /", ENTER).expect("'A÷3=B÷3'")
        .test(CLEAR, "'A=B' 3 ↑", ENTER).expect("'A↑3=B↑3'");
    step("Simple arithmetic - Constant and symbol")
        .test(CLEAR, "3 'A=B' +", ENTER).expect("'3+A=3+B'")
        .test(CLEAR, "3 'A=B' -", ENTER).expect("'3-A=3-B'")
        .test(CLEAR, "3 'A=B' *", ENTER).expect("'3·A=3·B'")
        .test(CLEAR, "3 'A=B' /", ENTER).expect("'3÷A=3÷B'")
        .test(CLEAR, "3 'A=B' ↑", ENTER).expect("'3↑A=3↑B'");
    step("Simple arithmetic - Symbol and symbol")
        .test(CLEAR, "'A=B' 'C=D' +", ENTER).expect("'A+C=B+D'")
        .test(CLEAR, "'A=B' 'C=D' -", ENTER).expect("'A-C=B-D'")
        .test(CLEAR, "'A=B' 'C=D' *", ENTER).expect("'A·C=B·D'")
        .test(CLEAR, "'A=B' 'C=D' /", ENTER).expect("'A÷C=B÷D'")
        .test(CLEAR, "'A=B' 'C=D' ↑", ENTER).expect("'A↑C=B↑D'");
   step("Simple functions")
       .test(CLEAR, "'A=B'", ENTER, J).expect("'sin A=sin B'")
       .test(K, L).expect("'tan (cos (sin A))=tan (cos (sin B))'");

    step("Single add, right");
    test(CLEAR, "'(A+B)*C' expand ", ENTER)
        .expect("'A·C+B·C'");
    step("Single add, left");
    test(CLEAR, "'2*(A+B)' expand ", ENTER)
        .expect("'2·A+2·B'");

    step("Multiple adds");
    test(CLEAR, "'3*(A+B+C)' expand ", ENTER)
        .expect("'3·C+(3·A+3·B)'");

    step("Single sub, right");
    test(CLEAR, "'(A-B)*C' expand ", ENTER)
        .expect("'A·C-B·C'");
    step("Single sub, left");
    test(CLEAR, "'2*(A-B)' expand ", ENTER)
        .expect("'2·A-2·B'");

    step("Multiple subs");
    test(CLEAR, "'3*(A-B-C)' expand ", ENTER)
        .expect("'3·A-3·B-3·C'");

    step("Expand and collect a power");
    test(CLEAR, "'(A+B)^3' expand ", ENTER)
        .expect("'A·(A·A)+A·(A·B)+(A·(A·B)+A·(B·B))+(B·(A·A)+B·(A·B)+(B·(A·B)+B·(B·B)))'");
    test("collect ", ENTER)
        .expect("'(A+B)↑3'");
    // .expect("'(A+B)³'");

    step("Apply function call for user-defined function")
        .test(CLEAR, "{ 1 2 3 } 'F' APPLY", ENTER)
        .expect("'F(1;2;3)'");

    step("Apply function call for user-defined function and array")
        .test(CLEAR, "[ A B C D ] 'F' APPLY", ENTER)
        .expect("'F(A;B;C;D)'");

    step("Apply function call for algebraic function")
        .test(CLEAR, "{ 'x+y' } 'sin' APPLY", ENTER)
        .expect("'sin x+y'");

    step("Apply function call: incorrect arg count")
        .test(CLEAR, "{ x y } 'sin' APPLY", ENTER)
        .error("Wrong argument count");

    step("Apply function call: incorrect type")
        .test(CLEAR, "{ x y } 'drop' APPLY", ENTER)
        .error("Bad argument type");

    step("Apply function call: incorrect type")
        .test(CLEAR, "2 'F' APPLY", ENTER)
        .error("Bad argument type");

    step("Substitution with simple polynomial")
        .test(CLEAR, "'X^2+3*X+7' 'X=Z+1' SUBST", ENTER)
        .expect("'(Z+1)↑2+3·(Z+1)+7'")
        .test("'Z=sin(A+B)' SUBST", ENTER)
        .expect("'(sin(A+B)+1)↑2+3·(sin(A+B)+1)+7'");
    step("Substitution with numerical value")
        .test(CLEAR, "42 'X=Z+1' SUBST", ENTER)
        .expect("42");
    step("Type error on value to substitute")
        .test(CLEAR, "\"ABC\" 'X=Z+1' SUBST", ENTER)
        .error("Bad argument type");
    step("Bad argument value for substitution")
        .test(CLEAR, "'X^2+3*X+7' 'Z-1=Z+1' SUBST", ENTER)
        .error("Bad argument value");

    step("WHERE command with simple polynomial")
        .test(CLEAR, "'X^2+3*X+7' 'X=Z+1' WHERE", ENTER)
        .expect("'(Z+1)↑2+3·(Z+1)+7'")
        .test("{ 'Z=sin(A+B)' 'A=42' } WHERE", ENTER)
        .expect("'(sin(42+B)+1)↑2+3·(sin(42+B)+1)+7'");
    step("Substitution with numerical value")
        .test(CLEAR, "42 'X=Z+1' WHERE", ENTER)
        .expect("42");
    step("Type error on value to substitute in WHERE")
        .test(CLEAR, "\"ABC\" 'X=Z+1' WHERE", ENTER)
        .error("Bad argument type");
    step("Bad argument value for substitution in WHERE")
        .test(CLEAR, "'X^2+3*X+7' 'Z-1=Z+1' WHERE", ENTER)
        .error("Bad argument value");
    step("| operator")
        .test(CLEAR, "'X^2+3*X+7|X=Z+1'", ENTER)
        .expect("'X↑2+3·X+7|X=Z+1'")
        .test(RUNSTOP)
        .expect("'(Z+1)↑2+3·(Z+1)+7'");
    step("Chained | operator")
        .test("'X^2+3*X+7|X=Z+1|Z=sin(A+B)|A=42'", ENTER)
        .expect("'X↑2+3·X+7|X=Z+1|Z=sin(A+B)|A=42'")
        .test(RUNSTOP)
        .expect("'(sin(42+B)+1)↑2+3·(sin(42+B)+1)+7'");
    step("Chained | operator with HP syntax")
        .test("'X^2+3*X+7|(X=Z+1;Z=sin(A+B);A=42)'", ENTER)
        .expect("'X↑2+3·X+7|X=Z+1|Z=sin(A+B)|A=42'")
        .test(RUNSTOP)
        .expect("'(sin(42+B)+1)↑2+3·(sin(42+B)+1)+7'");
    step("Where operator on library equations")
        .test("'ⒺRelativity Mass Energy|m=(1_g)'", ENTER)
        .expect("'Relativity Mass Energy:{E=m·c²}|m=1 g'")
        .test(RUNSTOP)
        .expect("{ 'E=¹/₁ ₀₀₀ kg·c²' }");
    step("Where operator in non-algebraic form with list")
        .test(CLEAR, "'x^y' { x 2 y 3 } |", ENTER)
        .expect("'2↑3'");
    step("Where operator with lists and names as replacement")
        .test(CLEAR, DIRECT("'(A-2+sin(6*C))^J' {A V J 9} |"), ENTER)
        .expect("'(V-2+sin(6·C))↑9'");

    step("Isolate a single variable, simple case")
        .test(CLEAR, "'A+1=sin(X+B)+C' 'X' ISOL", ENTER)
        .expect("'X=sin⁻¹(A-C+1)+2·i1·π-B'")
        .test(RSHIFT, KEY7, F6);
    step("Isolate an expression with implicit =0")
        .test(CLEAR, "'A+X*B-C' 'X'", NOSHIFT, F3)
        .expect("'X=(C-A)÷B'");
    step("Isolate an expression that is already isolated")
        .test(CLEAR, "'X=B+C' 'X'", NOSHIFT, F3)
        .expect("'X=B+C'");
    step("Isolated variable grouping")
        .test(CLEAR, "'X=B-X' 'X'", NOSHIFT, F3)
        .expect("'X=B÷2'");
    step("Isolation failure")
        .test(CLEAR, "'X=sin X+1' 'X'", NOSHIFT, F3)
        .expect("'X-sin X=1'")
        .test("X", NOSHIFT, F3)
        .error("Unable to isolate");
    step("Isolate a single variable, addition")
        .test(CLEAR, "'A=X+B' X", NOSHIFT, F3).expect("'X=A-B'")
        .test(CLEAR, "'A=B+X' X", NOSHIFT, F3).expect("'X=A-B'");
    step("Isolate a single variable, subtraction")
        .test(CLEAR, "'A=X-B' X", NOSHIFT, F3).expect("'X=A+B'")
        .test(CLEAR, "'A=B-X' X", NOSHIFT, F3).expect("'X=B-A'");
    step("Isolate a single variable, multiplication")
        .test(CLEAR, "'A=X*B' X", NOSHIFT, F3).expect("'X=A÷B'")
        .test(CLEAR, "'A=B*X' X", NOSHIFT, F3).expect("'X=A÷B'");
    step("Isolate a single variable, division")
        .test(CLEAR, "'A=X/B' X", NOSHIFT, F3).expect("'X=A·B'")
        .test(CLEAR, "'A=B/X' X", NOSHIFT, F3).expect("'X=B÷A'");
    step("Isolate a single variable, power")
        .test(CLEAR, "'A=X^B' X", NOSHIFT, F3).expect("'X=A↑B⁻¹+exp(i1·π·ⅈ÷B)'")
        .test(CLEAR, "'A=B^X' X", NOSHIFT, F3).expect("'X=ln A÷ln B'");
    step("Isolate sin")
        .test(CLEAR, "'sin X=A' X", NOSHIFT, F3)
        .expect("'X=sin⁻¹ A+2·i1·π'");
    step("Isolate cos")
        .test(CLEAR, "'cos X=A' X", NOSHIFT, F3)
        .expect("'X=cos⁻¹ A+2·i1·π'");
    step("Isolate tan")
        .test(CLEAR, "'tan X=A' X", NOSHIFT, F3)
        .expect("'X=tan⁻¹ A+i1·π'");
    step("Isolate asin")
        .test(CLEAR, "'A=asin X' X", NOSHIFT, F3)
        .expect("'X=sin A'");
    step("Isolate acos")
        .test(CLEAR, "'A=acos X' X", NOSHIFT, F3)
        .expect("'X=cos A'");
    step("Isolate atan")
        .test(CLEAR, "'A=atan X' X", NOSHIFT, F3)
        .expect("'X=tan A'");
    step("Isolate sinh")
        .test(CLEAR, "'sinh X=A' X", NOSHIFT, F3)
        .expect("'X=sinh⁻¹ A+2·i1·π·ⅈ'");
    step("Isolate cosh")
        .test(CLEAR, "'cosh X=A' X", NOSHIFT, F3)
        .expect("'X=cosh⁻¹ A+2·i1·π·ⅈ'");
    step("Isolate tanh")
        .test(CLEAR, "'tanh X=A' X", NOSHIFT, F3)
        .expect("'X=tanh⁻¹ A+i1·π·ⅈ'");
    step("Isolate asinh")
        .test(CLEAR, "'A=asinh X' X", NOSHIFT, F3)
        .expect("'X=sinh A'");
    step("Isolate acosh")
        .test(CLEAR, "'A=acosh X' X", NOSHIFT, F3)
        .expect("'X=cosh A'");
    step("Isolate atanh")
        .test(CLEAR, "'A=atanh X' X", NOSHIFT, F3)
        .expect("'X=tanh A'");
    step("Isolate log")
        .test(CLEAR, "'A=ln X' X", NOSHIFT, F3)
        .expect("'X=exp A'");
    step("Isolate exp")
        .test(CLEAR, "'A=exp X' X", NOSHIFT, F3)
        .expect("'X=ln A+2·i1·π·ⅈ'");
    step("Isolate log2")
        .test(CLEAR, "'A=log2 X' X", NOSHIFT, F3)
        .expect("'X=exp2 A'");
    step("Isolate exp2")
        .test(CLEAR, "'A=exp2 X' X", NOSHIFT, F3)
        .expect("'X=log2 A+2·i1·π·ⅈ÷ln 2'");
    step("Isolate log10")
        .test(CLEAR, "'A=log10 X' X", NOSHIFT, F3)
        .expect("'X=exp10 A'");
    step("Isolate exp10")
        .test(CLEAR, "'A=exp10 X' X", NOSHIFT, F3)
        .expect("'X=log10 A+2·i1·π·ⅈ÷ln 10'");
    step("Isolate ln1p")
        .test(CLEAR, "'A=ln1p X' X", NOSHIFT, F3)
        .expect("'X=expm1 A'");
    step("Isolate expm1")
        .test(CLEAR, "'A=expm1 X' X", NOSHIFT, F3)
        .expect("'X=ln1p A+2·i1·π·ⅈ'");
    step("Isolate sq")
        .test(CLEAR, "'A=sq X' X", NOSHIFT, F3)
        .expect("'X=s1·√ A'");
    step("Isolate sqrt")
        .test(CLEAR, "'A=sqrt X' X", NOSHIFT, F3)
        .expect("'X=A²'");
    step("Isolate cubed")
        .test(CLEAR, "'A=cubed X' X", NOSHIFT, F3)
        .expect("'X=∛ A+exp(i1·π·ⅈ÷3)'");
    step("Isolate cbrt")
        .test(CLEAR, "'A=cbrt X' X", NOSHIFT, F3)
        .expect("'X=A³'");
}


void tests::symbolic_differentiation()
// ----------------------------------------------------------------------------
//   Symbolic differentiation
// ----------------------------------------------------------------------------
{
    BEGIN(derivative);

    step("Derivative of constant")
        .test(CLEAR, ID_IntegrationMenu, "42 'X'", ID_Derivative).expect("'0'");
    step("Derivative of a variable")
        .test(CLEAR, "'X' 'X'", ID_Derivative).expect("'1'");
    step("Derivative of a different variable")
        .test(CLEAR, "'A' 'X'", ID_Derivative).expect("'0'");
    step("Derivative of a product by a constant")
        .test(CLEAR, "'A*X' 'X'", ID_Derivative).expect("'A'");
    step("Derivative of a polynomial")
        .test(CLEAR, "'A*X+B*X^2-C*sq(X)+D*X^5+42' 'X'", ID_Derivative)
        .expect("'A+2·B·X+5·D·X↑4-2·C·X'");
    step("Derivative of ratio")
        .test(CLEAR, "'A*X/(B*X+1)' 'X'", ID_Derivative)
        .expect("'(A·(B·X+1)-A·X·B)÷(B·X+1)²'");
    step("Derivative of power by a numerical constant")
        .test(CLEAR, "'X^(2.5+3.2)' 'X'", ID_Derivative)
        .expect("'5.7·X↑4.7'");
    step("Derivative of power by a non-numerical constant")
        .test(CLEAR, "'X^(A+2)' 'X'", ID_Derivative)
        .expect("'X↑(A+2)·(A+2)÷X'");
    step("Derivative of power of a numerical constant")
        .test(CLEAR, "'2^X' 'X'", ID_Derivative)
        .expect("'0.69314 71805 6·2↑X'");
    step("Derivative of power of a non-numerical constant")
        .test(CLEAR, "'A^X' 'X'", ID_Derivative)
        .expect("'A↑X·ln A'")
        .test(RUNSTOP)
        .expect("'A↑X·ln A'");
    step("Derivative of power")
        .test(CLEAR, "'(A*X+B)^(C*X+D)' 'X'", ID_Derivative)
        .expect("'(A·X+B)↑(C·X+D)·(C·ln(A·X+B)+A·(C·X+D)÷(A·X+B))'");
    step("Derivative of negation, inverse, abs and sign")
        .test(CLEAR, "'-(inv(X) + abs(X) - sign(X^2))' 'X'", ID_Derivative)
        .expect("'-((-1)÷X²+sign X)'");
    step("Derivative of sine, cosine, tangent")
        .test(CLEAR, "'sin(A*X^2)+cos(X*B)+tan(C*X^6)' 'X'", ID_Derivative)
        .expect("'2·A·X·cos(A·X²)+(-B)·sin(X·B)+6·C·X↑5÷(cos(C·X↑6))²'");
    step("Derivative of hyperbolic sine, cosine, tangent")
        .test(CLEAR, "'sinh(A*X^3)+cosh(B*X^5)+tanh(C*X^3)' 'X'",
              LENGTHY(3000), ID_Derivative)
        .expect("'3·A·X²·cosh(A·X³)+5·B·X↑4·sinh(B·X↑5)+3·C·X²÷(cosh(C·X³))²'");
    step("Derivative of arcsine, arccosine, arctangent")
        .test(CLEAR, "'asin(A*X^2)+acos(X*B)+atan(C*X^6)' 'X'",
              LENGTHY(3000), ID_Derivative)
        .expect("'2·A·X÷√(1-(A·X²)²)+(-B)÷√(1-(X·B)²)+6·C·X↑5÷((C·X↑6)²+1)'");
    step("Derivative of inverse hyperbolic sine, cosine, tangent")
        .test(CLEAR, "'asinh(A*X)+acosh(X*B)+atanh(C+X)' 'X'",
              LENGTHY(3000), ID_Derivative)
        .expect("'A÷√((A·X)²+1)+B÷√((X·B)²-1)+(1-(C+X)²)⁻¹'");

    step("Derivative of log and exp")
        .test(CLEAR, "'ln(A*X+B)+exp(X*C-D)' 'X'", ID_Derivative)
        .expect("'A÷(A·X+B)+C·exp(X·C-D)'");
    step("Derivative of log2 and exp2")
        .test(CLEAR, "'log2(A*X+B)+exp2(X*C-D)' 'X'", ID_Derivative)
        .expect("'A÷(ln 2·(A·X+B))+ln 2·C·exp2(X·C-D)'");
    step("Derivative of log10 and exp10")
        .test(CLEAR, "'log10(A*X+B)+exp10(X*C-D)' 'X'", ID_Derivative)
        .expect("'A÷(ln 10·(A·X+B))+ln 10·C·exp10(X·C-D)'");

    step("Derivative of lnp1 and expm1")
        .test(CLEAR, "'ln1p(A*X+B)+expm1(X*C-D)' 'X'", ID_Derivative)
        .expect("'A÷(A·X+B+1)+C·exp(X·C-D)'");

    step("Derivative of square and cube")
        .test(CLEAR, "'sq(A*X+B)+cubed(X*C-D)' 'X'", ID_Derivative)
        .expect("'2·(A·X+B)·A+3·(X·C-D)²·C'");
    step("Derivative of square root and cube root")
        .test(CLEAR, "'sqrt(A*X+B)+cbrt(X*C-D)' 'X'", ID_Derivative)
        .expect("'A÷(2·√(A·X+B))+C÷(3·(∛(X·C-D))²)'");

    step("Derivative of single-variable user-defined function")
        .test(CLEAR, "'F(A*X+B)' 'X'", ID_Derivative)
        .expect("'A·F′(A·X+B)'");
    step("Derivative of nested single-variable user-defined function")
        .test(CLEAR, "'F(G(A*X+B))' 'X'", ID_Derivative)
        .expect("'A·G′(A·X+B)·F′(G(A·X+B))'");

    step("Derivative of multi-variable user-defined function")
        .test(CLEAR, "'F(A*X+B;C*X+D;E*X-G)' 'X'", ID_Derivative)
        .expect("'A·F′₁(A·X+B;C·X+D;E·X-G)"
                "+C·F′₂(A·X+B;C·X+D;E·X-G)"
                "+E·F′₃(A·X+B;C·X+D;E·X-G)'");

    step("Derivative of unknown form")
        .test(CLEAR, "'IP(X)' 'X'", ID_Derivative)
        .error("Unknown derivative");

    step("Derivative of unknown form in algebraic form")
        .test(CLEAR, "'∂x(→Num(x))'", ENTER, ID_Run)
        .error("Unknown derivative");

    step("Derivative of function with angle (#1491)")
        .test(CLEAR, DIRECT("'sin((0.5_r/s)·x)' 'x' ∂"), ENTER)
        .expect("'0.5 r/s·cos(0.5 r/s·x)'");
}


void tests::symbolic_integration()
// ----------------------------------------------------------------------------
//   Symbolic integration
// ----------------------------------------------------------------------------
{
    BEGIN(primitive);

    step("Primitive of constant")
        .test(CLEAR, ID_IntegrationMenu, "42 'X'", ID_Primitive)
        .expect("'42·X'");
    step("Primitive of a variable")
        .test(CLEAR, "1 'X'", ID_Primitive).expect("'X'");
    step("Primitive of a different variable")
        .test(CLEAR, "'A' 'X'", ID_Primitive).expect("'A·X'");
    step("Primitive of a product by a constant")
        .test(CLEAR, "'A*X' 'X'", ID_Primitive).expect("'A÷2·X²'");
    step("Primitive of a polynomial")
        .test(CLEAR, "'A*X+B*X^2-C*sq(X)+D*X^5+42' 'X'", ID_Primitive)
        .expect("'A÷2·X²+B÷3·X³+D÷6·X↑6+42·X-C÷3·X³'");
    step("Primitive of ratio")
        .test(CLEAR, "'A*X/(B*X+1)' 'X'", ID_Primitive)
        .expect("'A÷B²·(B·X-ln (abs(B·X+1))+1)'");
    step("Primitive of ratio of linear functions")
        .test(CLEAR, "'(A*X+B)/(C*X+D)' 'X'", ID_Primitive)
        .expect("'B÷C·ln (abs(C·X+D))+A÷C²·(C·X+D-D·ln (abs(C·X+D)))'");
    step("Primitive of power by a numerical constant")
        .test(CLEAR, "'X^(2.5+3.2)' 'X'", ID_Primitive)
        .expect("'X↑6.7÷6.7'");
    step("Primitive of power by a non-numerical constant")
        .test(CLEAR, "'X^(A+2)' 'X'", ID_Primitive)
        .expect("'X↑(A+3)÷(A+3)'");
    step("Primitive of power of a numerical constant")
        .test(CLEAR, "'2^X' 'X'", ID_Primitive)
        .expect("'2↑X÷0.69314 71805 6'");
    step("Primitive of power of a non-numerical constant")
        .test(CLEAR, "'A^X' 'X'", ID_Primitive)
        .expect("'A↑X÷ln A'")
        .test(RUNSTOP)
        .expect("'A↑X÷ln A'");
    step("Primitive of power")
        .test(CLEAR, "'(A*X+B)^(C*X+D)' 'X'", ID_Primitive)
        .error("Unknown primitive");
    step("Primitive of negation, inverse and sign")
        .test(CLEAR, "'-(inv(A*X+B) - sign(3-2*X))' 'X'", ID_Primitive)
        .expect("'-(ln (abs(A·X+B))÷A-abs(3-2·X)÷2)'");
    step("Primitive of sine, cosine, tangent")
        .test(CLEAR, "'sin(A*X+3)+cos(X*B-5)+tan(Z-C*X)' 'X'",
              LENGTHY(10000), ID_Primitive)
        .expect("'(-cos(A·X+3))÷A+sin(X·B-5)÷B+(-ln (cos(Z-C·X)))÷C'");
    step("Primitive of hyperbolic sine, cosine, tangent")
        .test(CLEAR, "'sinh(A*X-3)+cosh(B*X+5*A)+tanh(C*(X-A))' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'cosh(A·X-3)÷A+sinh(B·X+5·A)÷B+ln (cosh(C·(X-A)))÷C'");
    step("Primitive of arcsine, arccosine, arctangent")
        .test(CLEAR, "'asin(A*X+B)+acos(X*B+A*(X+1))+atan(C*(X-6))' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'((A·X+B)·sin⁻¹(A·X+B)+√(1-(A·X+B)²))÷A+((X·B+A·(X+1))·cos⁻¹(X·B+A·(X+1))-√(1-(X·B+A·(X+1))²))÷(B+A)+(C·(X-6)·tan⁻¹(C·(X-6))-ln((C·(X-6))²+1)÷2)÷C'");
    step("Primitive of inverse hyperbolic sine, cosine, tangent")
        .test(CLEAR, "'asinh(1-2*X)+acosh(1+3*X)+atanh(4*X-1)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'((1-2·X)·sinh⁻¹(1-2·X)-√((1-2·X)²+1))÷2+((3·X+1)·cosh⁻¹(3·X+1)-√((3·X+1)²-1))÷3+((4·X-1)·tan⁻¹(4·X-1)-ln(1-(4·X-1)²)÷2)÷4'");

    step("Primitive of log and exp")
        .test(CLEAR, "'ln(A*X+B)+exp(X*C-D)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'((A·X+B)·ln(A·X+B)-(A·X+B))÷A+exp(X·C-D)÷C'");
    step("Primitive of log2 and exp2")
        .test(CLEAR, "'log2(A*X+B)+exp2(X*C-D)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'((A·X+B)·log2(A·X+B)-(A·X+B)÷ln 2)÷A+exp2(X·C-D)÷(0.69314 71805 6·C)'");
    step("Primitive of log10 and exp10")
        .test(CLEAR, "'log10(A*X+B)+exp10(X*C-D)' 'X'",
              LENGTHY(20000),  ID_Primitive)
        .expect("'((A·X+B)·log10(A·X+B)-(A·X+B)÷ln 10)÷A+exp10(X·C-D)÷(2.30258 50929 9·C)'");

    step("Primitive of lnp1 and expm1")
        .test(CLEAR, "'ln1p(A*X+B)+expm1(X*C-D)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'((A·X+B-1)·ln1p(A·X+B)-(A·X+B-1))÷A+(expm1(X·C-D)-(X·C-D)+1)÷C'");

    step("Primitive of square and cube")
        .test(CLEAR, "'sq(A*X+B)+cubed(X*C-D)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'(A·X+B)³÷(3·A)+(X·C-D)↑4÷(4·C)'");
    step("Primitive of square root and cube root")
        .test(CLEAR, "'sqrt(A*X+B)+cbrt(X*C-D)' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'²/₃·A⁻¹·(√(A·X+B))³+³/₄·C⁻¹·∛(X·C-D)↑4'");

    step("Primitive of 1/(cos(x)*sin(x))")
        .test(CLEAR, "'inv(cos(3*X+2)*sin(3*X+2))' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'ln (tan(3·X+2))÷3'");
    step("Primitive of 1/(cosh(x)*sinh(x))")
        .test(CLEAR, "'inv(cosh(3*X+2)*sinh(3*X+2))' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'ln (tan(3·X+2))÷3'");
    step("Primitive of 1/(cosh(x)*sinh(x))")
        .test(CLEAR, "'inv(cosh(3*X+2)*sinh(3*X+2))' 'X'",
              LENGTHY(20000), ID_Primitive)
        .expect("'ln (tan(3·X+2))÷3'");

    step("Primitive of unknown form")
        .test(CLEAR, "'IP(X)' 'X'", ID_Primitive)
        .error("Unknown primitive");
    step("Primitive of unknown form in algebraic form")
        .test(CLEAR, "'∫x(→Num(x))'", ENTER, ID_Run)
        .error("Unknown primitive");

    step("Evaluate values matching integer constants in pattenrs")
        .test(CLEAR, DIRECT("'4/3·Ⓒπ·x³' 'x' ∂"), ENTER)
        .expect("'4·π·x²'");
    step("Evaluate value matching integer constants - Check with division")
        .test(CLEAR, DIRECT("'A/B·Ⓒπ·x³' 'x' ∂"), ENTER)
        .expect("'3·A÷B·π·x²'");
}


void tests::tagged_objects()
// ----------------------------------------------------------------------------
//   Some very basic testing of tagged objects
// ----------------------------------------------------------------------------
{
    BEGIN(tagged);

    step("Parsing tagged integer");
    test(CLEAR, ":ABC:123", ENTER)
        .type(ID_tag)
        .expect("ABC:123");
    step("Parsing tagged fraction");
    test(CLEAR, ":Label:123/456", ENTER)
        .type(ID_tag)
        .expect("Label:⁴¹/₁₅₂");
    step("Parsing nested label");
    test(CLEAR, ":Nested::Label:123.456", ENTER)
        .type(ID_tag)
        .expect("Nested:Label:123.456");

    step("Arithmetic");
    test(CLEAR, ":First:1 :Second:2 +", ENTER)
        .expect("3");
    test(CLEAR, "5 :Second:2 -", ENTER)
        .expect("3");
    test(CLEAR, ":First:3/2 2 *", ENTER)
        .expect("3");

    step("Functions");
    test(CLEAR, ":First:1 ABS", ENTER)
        .expect("1");
    test(CLEAR, ":First:0 SIN", ENTER)
        .expect("0");

    step("ToTag");
    test(CLEAR, "125 \"Hello\" ToTag", ENTER)
        .expect("Hello:125");
    test(CLEAR, "125 127 ToTag", ENTER)
        .type(ID_tag)
        .expect("127:125");

    step("FromTag");
    test(CLEAR, ":Hello:123 FromTag", ENTER)
        .type(ID_text)
        .expect("\"Hello\"")
        .test("Drop", ENTER)
        .expect("123");

    step("DeleteTag");
    test(CLEAR, ":Hello:123 DeleteTag", ENTER)
        .expect("123");

    step("Tagged unit")
        .test(CLEAR, ":ABC:1_kg", ENTER)
        .expect("ABC:1 kg");
    step("Tagged unit (without space)")
        .test(CLEAR, ALPHA, KEY0, A, B, C, NOSHIFT, DOWN,
              KEY1, ID_UnitsMenu, F1,
              LOWERCASE, K, G, ENTER)
        .expect("ABC:1 kg");
    step("Tagged complex (without space)")
        .test(CLEAR, ALPHA, KEY0,
              A, B, C, NOSHIFT, DOWN,
              KEY1, ID_ComplexMenu, F1, KEY2, KEY3, ENTER)
        .expect("ABC:1+23ⅈ")
        .test(ID_ObjectMenu, ID_FromTag) // TAG->
        .got("\"ABC\"", "1+23ⅈ");
}


void tests::catalog_test()
// ----------------------------------------------------------------------------
//   Test the catalog features
// ----------------------------------------------------------------------------
{
    BEGIN(catalog);

    step("Entering commands through the catalog")
        .test(CLEAR, RSHIFT, RUNSTOP).editor("{}")
        .test(ALPHA, A).editor("{A}")
        .test(ADD).editor("{A}")
        .test(F1).editor("{ abs }");
    step("Finding functions from inside")
        .test(B).editor("{ abs B}")
        .test(F1).editor("{ abs Background }");
    step("Finding functions with middle characters")
        .test(B, U).editor("{ abs Background BU}")
        .test(F1).editor("{ abs Background BusyIndicatorRefresh }");
    step("Catalog with nothing entered")
        .test(F6, F3).editor("{ abs Background BusyIndicatorRefresh cosh⁻¹ }");

    step("Test the default menu")
        .test(CLEAR, EXIT, A, RSHIFT, RUNSTOP).editor("{}")
        .test(F1).editor("{ Help }");
    step("Test catalog as a menu")
        .test(SHIFT, ADD, F1).editor("{ Help x! }")
        .test(ENTER).expect("{ Help x! }");
}


void tests::cycle_test()
// ----------------------------------------------------------------------------
//   Test the Cycle feature
// ----------------------------------------------------------------------------
{
    BEGIN(cycle);

    step("Using the EEX key to enter powers of 10")
        .test(CLEAR, KEY1, O, KEY3, KEY2).editor("1⁳32")
        .test(ENTER).expect("1.⁳³²");
    step("Convert decimal to integer")
        .test(O).expect("100 000 000 000 000 000 000 000 000 000 000");
    step("Convert integer to decimal")
        .test(ENTER, KEY2, KEY0, KEY0, DIV, SUB)
        .test(O).expect("9.95⁳³¹");
    step("Convert decimal to fraction")
        .test(CLEAR, KEY1, DOT, KEY2, ENTER).expect("1.2")
        .test(O).expect("1 ¹/₅");
    step("Convert fraction to decimal")
        .test(B).expect("⁵/₆")
        .test(O).expect("0.83333 33333 33");
    step("Convert decimal to fraction with rounding")
        .test(O).expect("⁵/₆");
    step("Convert decimal to fraction with multiple digits")
        .test(CLEAR, "1.325", ENTER, O).expect("1 ¹³/₄₀");
    step("Convert rectangular to polar")
        .test(CLEAR, "DEG", ENTER,
              "10", SHIFT, G, F1, "10", ENTER).expect("10+10ⅈ")
        .test(O).expect("14.14213 56237∡45°");
    step("Convert polar to rectangular")
        .test(O).expect("10.+10.ⅈ");
    step("Convert based integer bases")
        .test(CLEAR, "#123", ENTER).expect("#123₁₆")
        .test(O).expect("#123₁₆")
        .test(O).expect("#291₁₀")
        .test(O).expect("#443₈")
        .test(O).expect("#1 0010 0011₂")
        .test(O).expect("#123₁₆")
        .test(O).expect("#123₁₆");
    step("Convert list to array")
        .test(CLEAR, "{ 1 2 3 }", ENTER).expect("{ 1 2 3 }")
        .test(O).expect("[ 1 2 3 ]");
    step("Convert array to program")
        .test(O).want("« 1 2 3 »");
    step("Convert program to list")
        .test(O).expect("{ 1 2 3 }");
    step("Tags are preserved, cycle applies to tagged value")
        .test(CLEAR, ":ABC:1.25", ENTER).expect("ABC:1.25")
        .test(O).expect("ABC:1 ¹/₄")
        .test(O).expect("ABC:1.25");
    step("Cycle unit orders of magnitude up (as fractions)")
        .test(CLEAR, "1_kN", ENTER).expect("1 kN")
        .test(O).expect("¹/₁ ₀₀₀ MN")
        .test(O).expect("¹/₁ ₀₀₀ ₀₀₀ GN");
    step("Cycle unit orders of magnitude down (as decimal)")
        .test(O).expect("0.00000 1 GN")
        .test(O).expect("0.001 MN")
        .test(O).expect("1. kN")
        .test(O).expect("10. hN")
        .test(O).expect("100. daN")
        .test(O).expect("1 000. N")
        .test(O).expect("10 000. dN")
        .test(O).expect("100 000. cN")
        .test(O).expect("1 000 000. mN");
    step("Cycle unit orders of magnitude up (as integers)")
        .test(O).expect("1 000 000 mN")
        .test(O).expect("100 000 cN")
        .test(O).expect("10 000 dN")
        .test(O).expect("1 000 N")
        .test(O).expect("100 daN")
        .test(O).expect("10 hN")
        .test(O).expect("1 kN");
    step("Cycle unit orders of magnitude up (as fractions)")
        .test(O).expect("¹/₁ ₀₀₀ MN")
        .test(O).expect("¹/₁ ₀₀₀ ₀₀₀ GN");
    step("Cycle unit orders of magnitude up (back to decimal)")
        .test(O).expect("0.00000 1 GN")
        .test(O).expect("0.001 MN")
        .test(O).expect("1. kN");

    step("Cycle angle units")
        .test(CLEAR, "1.2.3", ENTER).expect("1°02′03″");
    step("Cycle from DMS to fractional pi-radians")
        .test(O).expect("¹ ²⁴¹/₂₁₆ ₀₀₀ πr");
    step("Cycle from fractional pi-radians to fractional degrees")
        .test(O).expect("1 ⁴¹/₁ ₂₀₀ °");
    step("Cycle from fractional degrees to fractional grad")
        .test(O).expect("1 ¹⁶¹/₁ ₀₈₀ grad");
    step("Cycle from fractional grad to decimal radians")
        .test(O).expect("0.01804 96133 48 r");
    step("Cycle from decimal radians to decimal grad")
        .test(O).expect("1.14907 40740 7 grad");
    step("Cycle from decimal grad to decimal degrees")
        .test(O).expect("1.03416 66666 7 °");
    step("Cycle from decimal degrees to decimal pi-radians")
        .test(O).expect("0.00574 53703 7 πr");
    step("Cycle to decimal DMS")
        .test(O).expect("1°02′03″");
    step("Cycle back to fractional DMS")
        .test(O).expect("1°02′03″");
    step("Check that DMS produced the original pi-radians fraction")
        .test(O).expect("¹ ²⁴¹/₂₁₆ ₀₀₀ πr");
    step("Check that DMS produced the original degrees fraction")
        .test(O).expect("1 ⁴¹/₁ ₂₀₀ °");
}


void tests::shift_and_rotate()
// ----------------------------------------------------------------------------
//    Test shift and rotate instructions
// ----------------------------------------------------------------------------
{
    BEGIN(rotate);

    step("Default word size should be 64")
        .test(CLEAR, "RCWS", ENTER).noerror().expect("64");

    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#2474₁₆")
        .test(F1).expect("#48E8₁₆")
        .test(F1).expect("#91D0₁₆")
        .test(F1).expect("#1 23A0₁₆")
        .test(F1).expect("#2 4740₁₆")
        .test(F1).expect("#4 8E80₁₆")
        .test(F1).expect("#9 1D00₁₆")
        .test(F1).expect("#12 3A00₁₆");
    step("Shift right")
        .test(F2).expect("#9 1D00₁₆")
        .test(F2).expect("#4 8E80₁₆")
        .test(F2).expect("#2 4740₁₆")
        .test(F2).expect("#1 23A0₁₆")
        .test(F2).expect("#91D0₁₆")
        .test(F2).expect("#48E8₁₆")
        .test(F2).expect("#2474₁₆")
        .test(F2).expect("#123A₁₆")
        .test(F2).expect("#91D₁₆")
        .test(F2).expect("#48E₁₆")
        .test(F2).expect("#247₁₆")
        .test(F2).expect("#123₁₆");
    step("Rotate left")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆")
        .test(F4).expect("#918₁₆")
        .test(F4).expect("#1230₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#3000 0000 0000 0012₁₆")
        .test(LSHIFT, F4).expect("#1230₁₆")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#2460 0000 0000 0000₁₆")
        .test(F4).expect("#48C0 0000 0000 0000₁₆")
        .test(F4).expect("#9180 0000 0000 0000₁₆")
        .test(F4).expect("#2300 0000 0000 0001₁₆")
        .test(F4).expect("#4600 0000 0000 0002₁₆")
        .test(F4).expect("#8C00 0000 0000 0004₁₆");
    step("Rotate right")
        .test(F5).expect("#4600 0000 0000 0002₁₆")
        .test(F5).expect("#2300 0000 0000 0001₁₆")
        .test(F5).expect("#9180 0000 0000 0000₁₆")
        .test(F5).expect("#48C0 0000 0000 0000₁₆")
        .test(F5).expect("#2460 0000 0000 0000₁₆")
        .test(F5).expect("#1230 0000 0000 0000₁₆")
        .test(F5).expect("#918 0000 0000 0000₁₆")
        .test(F5).expect("#48C 0000 0000 0000₁₆")
        .test(F5).expect("#246 0000 0000 0000₁₆")
        .test(F5).expect("#123 0000 0000 0000₁₆")
        .test(F5).expect("#91 8000 0000 0000₁₆")
        .test(F5).expect("#48 C000 0000 0000₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#48C0 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#48 C000 0000₁₆")
        .test(LSHIFT, F5).expect("#48C0 0000₁₆")
        .test(LSHIFT, F5).expect("#48 C000₁₆")
        .test(LSHIFT, F5).expect("#48C0₁₆")
        .test(LSHIFT, F5).expect("#C000 0000 0000 0048₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#FFC0 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF C000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FFC0 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FFFF C000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FFFF FFC0 0000₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#FFFF FFFF FFE0 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFF0 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFF8 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFFC 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFFE 0000₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#FFFF FFFF FE00 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFF FFFE 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFF FE00 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFE 0000 0000 0000₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#FF FE00 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FFFE 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FF FE00 0000₁₆")
        .test(LSHIFT, F2).expect("#FFFE 0000₁₆");

    step("32-bit test")
        .test(CLEAR, "32 STWS", ENTER, EXIT).noerror();
    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#2474₁₆")
        .test(F1).expect("#48E8₁₆")
        .test(F1).expect("#91D0₁₆")
        .test(F1).expect("#1 23A0₁₆")
        .test(F1).expect("#2 4740₁₆")
        .test(F1).expect("#4 8E80₁₆")
        .test(F1).expect("#9 1D00₁₆")
        .test(F1).expect("#12 3A00₁₆");
    step("Shift right")
        .test(F2).expect("#9 1D00₁₆")
        .test(F2).expect("#4 8E80₁₆")
        .test(F2).expect("#2 4740₁₆")
        .test(F2).expect("#1 23A0₁₆")
        .test(F2).expect("#91D0₁₆")
        .test(F2).expect("#48E8₁₆")
        .test(F2).expect("#2474₁₆")
        .test(F2).expect("#123A₁₆")
        .test(F2).expect("#91D₁₆")
        .test(F2).expect("#48E₁₆")
        .test(F2).expect("#247₁₆")
        .test(F2).expect("#123₁₆");
    step("Rotate left")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆")
        .test(F4).expect("#918₁₆")
        .test(F4).expect("#1230₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#3000 0012₁₆")
        .test(LSHIFT, F4).expect("#1230₁₆")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#3000 0012₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#6000 0024₁₆")
        .test(F4).expect("#C000 0048₁₆")
        .test(F4).expect("#8000 0091₁₆")
        .test(F4).expect("#123₁₆")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆");
    step("Rotate right")
        .test(F5).expect("#246₁₆")
        .test(F5).expect("#123₁₆")
        .test(F5).expect("#8000 0091₁₆")
        .test(F5).expect("#C000 0048₁₆")
        .test(F5).expect("#6000 0024₁₆")
        .test(F5).expect("#3000 0012₁₆")
        .test(F5).expect("#1800 0009₁₆")
        .test(F5).expect("#8C00 0004₁₆")
        .test(F5).expect("#4600 0002₁₆")
        .test(F5).expect("#2300 0001₁₆")
        .test(F5).expect("#9180 0000₁₆")
        .test(F5).expect("#48C0 0000₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#48 C000₁₆")
        .test(LSHIFT, F5).expect("#48C0₁₆")
        .test(LSHIFT, F5).expect("#C000 0048₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#FFC0 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF C000₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#FFFF E000₁₆")
        .test(F3).expect("#FFFF F000₁₆")
        .test(F3).expect("#FFFF F800₁₆")
        .test(F3).expect("#FFFF FC00₁₆")
        .test(F3).expect("#FFFF FE00₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#FFFE 0000₁₆")
        .test(LSHIFT, F1).expect("#FE00 0000₁₆")
        .test(LSHIFT, F1).expect("#0₁₆")
        .test(LSHIFT, M).expect("#FE00 0000₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#FE 0000₁₆")
        .test(LSHIFT, F2).expect("#FE00₁₆")
        .test(LSHIFT, F2).expect("#FE₁₆")
        .test(LSHIFT, F2).expect("#0₁₆");

    step("128-bit test")
        .test(CLEAR, "128 STWS", ENTER, EXIT).noerror();
    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#2474₁₆")
        .test(F1).expect("#48E8₁₆")
        .test(F1).expect("#91D0₁₆")
        .test(F1).expect("#1 23A0₁₆")
        .test(F1).expect("#2 4740₁₆")
        .test(F1).expect("#4 8E80₁₆")
        .test(F1).expect("#9 1D00₁₆")
        .test(F1).expect("#12 3A00₁₆");
    step("Shift right")
        .test(F2).expect("#9 1D00₁₆")
        .test(F2).expect("#4 8E80₁₆")
        .test(F2).expect("#2 4740₁₆")
        .test(F2).expect("#1 23A0₁₆")
        .test(F2).expect("#91D0₁₆")
        .test(F2).expect("#48E8₁₆")
        .test(F2).expect("#2474₁₆")
        .test(F2).expect("#123A₁₆")
        .test(F2).expect("#91D₁₆")
        .test(F2).expect("#48E₁₆")
        .test(F2).expect("#247₁₆")
        .test(F2).expect("#123₁₆");
    step("Rotate left")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆")
        .test(F4).expect("#918₁₆")
        .test(F4).expect("#1230₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#3000 0000 0000 0000 0000 0000 0000 0012₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#6000 0000 0000 0000 0000 0000 0000 0024₁₆")
        .test(F4).expect("#C000 0000 0000 0000 0000 0000 0000 0048₁₆")
        .test(F4).expect("#8000 0000 0000 0000 0000 0000 0000 0091₁₆")
        .test(F4).expect("#123₁₆")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆");
    step("Rotate right")
        .test(F5).expect("#246₁₆")
        .test(F5).expect("#123₁₆")
        .test(F5).expect("#8000 0000 0000 0000 0000 0000 0000 0091₁₆")
        .test(F5).expect("#C000 0000 0000 0000 0000 0000 0000 0048₁₆")
        .test(F5).expect("#6000 0000 0000 0000 0000 0000 0000 0024₁₆")
        .test(F5).expect("#3000 0000 0000 0000 0000 0000 0000 0012₁₆")
        .test(F5).expect("#1800 0000 0000 0000 0000 0000 0000 0009₁₆")
        .test(F5).expect("#8C00 0000 0000 0000 0000 0000 0000 0004₁₆")
        .test(F5).expect("#4600 0000 0000 0000 0000 0000 0000 0002₁₆")
        .test(F5).expect("#2300 0000 0000 0000 0000 0000 0000 0001₁₆")
        .test(F5).expect("#9180 0000 0000 0000 0000 0000 0000 0000₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#91 8000 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000 0000₁₆")
        .test(LSHIFT, F5).expect("#9180 0000₁₆")
        .test(LSHIFT, F5).expect("#91 8000₁₆")
        .test(LSHIFT, F5).expect("#9180₁₆")
        .test(LSHIFT, F5).expect("#8000 0000 0000 0000 0000 0000 0000 0091₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#FF80 0000 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF 8000 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FF80 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FFFF 8000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FFFF FFFF FF80 0000 0000 0000 0000 0000₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#FFFF FFFF FFC0 0000 0000 0000 0000 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFE0 0000 0000 0000 0000 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFF0 0000 0000 0000 0000 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFF8 0000 0000 0000 0000 0000₁₆")
        .test(F3).expect("#FFFF FFFF FFFC 0000 0000 0000 0000 0000₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#FFFF FFFF FC00 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFF FFFC 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFF FC00 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FFFC 0000 0000 0000 0000 0000 0000 0000₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#FF FC00 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FFFC 0000 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FF FC00 0000 0000 0000 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FFFC 0000 0000 0000 0000 0000₁₆");

    step("16-bit test")
        .test(CLEAR, "16 STWS", ENTER, EXIT).noerror();
    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#2474₁₆")
        .test(F1).expect("#48E8₁₆")
        .test(F1).expect("#91D0₁₆")
        .test(F1).expect("#23A0₁₆")
        .test(F1).expect("#4740₁₆")
        .test(F1).expect("#8E80₁₆")
        .test(F1).expect("#1D00₁₆")
        .test(F1).expect("#3A00₁₆");
    step("Shift right")
        .test(F2).expect("#1D00₁₆")
        .test(F2).expect("#E80₁₆")
        .test(F2).expect("#740₁₆")
        .test(F2).expect("#3A0₁₆")
        .test(F2).expect("#1D0₁₆");
    step("Rotate left")
        .test(F4).expect("#3A0₁₆")
        .test(F4).expect("#740₁₆")
        .test(F4).expect("#E80₁₆")
        .test(F4).expect("#1D00₁₆")
        .test(F4).expect("#3A00₁₆")
        .test(F4).expect("#7400₁₆")
        .test(F4).expect("#E800₁₆")
        .test(F4).expect("#D001₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#1D0₁₆")
        .test(LSHIFT, F4).expect("#D001₁₆")
        .test(LSHIFT, F4).expect("#1D0₁₆")
        .test(LSHIFT, F4).expect("#D001₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#A003₁₆")
        .test(F4).expect("#4007₁₆")
        .test(F4).expect("#800E₁₆")
        .test(F4).expect("#1D₁₆");
    step("Rotate right")
        .test(F5).expect("#800E₁₆")
        .test(F5).expect("#4007₁₆")
        .test(F5).expect("#A003₁₆")
        .test(F5).expect("#D001₁₆")
        .test(F5).expect("#E800₁₆")
        .test(F5).expect("#7400₁₆")
        .test(F5).expect("#3A00₁₆")
        .test(F5).expect("#1D00₁₆")
        .test(F5).expect("#E80₁₆")
        .test(F5).expect("#740₁₆")
        .test(F5).expect("#3A0₁₆")
        .test(F5).expect("#1D0₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#D001₁₆")
        .test(LSHIFT, F5).expect("#1D0₁₆")
        .test(LSHIFT, F5).expect("#D001₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#FFD0₁₆")
        .test(LSHIFT, F3).expect("#FFFF₁₆")
        .test(LSHIFT, F3).expect("#FFFF₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#FF00₁₆")
        .test(LSHIFT, F1).expect("#0₁₆")
        .test(LSHIFT, M).expect("#FF00₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#FF80₁₆")
        .test(F3).expect("#FFC0₁₆")
        .test(F3).expect("#FFE0₁₆")
        .test(F3).expect("#FFF0₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#FF₁₆")
        .test(LSHIFT, F2).expect("#0₁₆");

    step("13-bit test")
        .test(CLEAR, "13 STWS", ENTER, EXIT).noerror();
    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#474₁₆")
        .test(F1).expect("#8E8₁₆")
        .test(F1).expect("#11D0₁₆")
        .test(F1).expect("#3A0₁₆")
        .test(F1).expect("#740₁₆")
        .test(F1).expect("#E80₁₆")
        .test(F1).expect("#1D00₁₆");
    step("Shift right")
        .test(F2).expect("#E80₁₆")
        .test(F2).expect("#740₁₆")
        .test(F2).expect("#3A0₁₆")
        .test(F2).expect("#1D0₁₆")
        .test(F2).expect("#E8₁₆");
    step("Rotate left")
        .test(F4).expect("#1D0₁₆")
        .test(F4).expect("#3A0₁₆")
        .test(F4).expect("#740₁₆")
        .test(F4).expect("#E80₁₆")
        .test(F4).expect("#1D00₁₆")
        .test(F4).expect("#1A01₁₆")
        .test(F4).expect("#1403₁₆")
        .test(F4).expect("#807₁₆")
        .test(F4).expect("#100E₁₆")
        .test(F4).expect("#1D₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#1D00₁₆")
        .test(LSHIFT, F4).expect("#E8₁₆")
        .test(LSHIFT, F4).expect("#807₁₆")
        .test(LSHIFT, F4).expect("#740₁₆")
        .test(LSHIFT, F4).expect("#3A₁₆")
        .test(LSHIFT, F4).expect("#1A01₁₆")
        .test(LSHIFT, F4).expect("#1D0₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#3A0₁₆")
        .test(F4).expect("#740₁₆")
        .test(F4).expect("#E80₁₆")
        .test(F4).expect("#1D00₁₆")
        .test(F4).expect("#1A01₁₆")
        .test(F4).expect("#1403₁₆");
    step("Rotate right")
        .test(F5).expect("#1A01₁₆")
        .test(F5).expect("#1D00₁₆")
        .test(F5).expect("#E80₁₆")
        .test(F5).expect("#740₁₆")
        .test(F5).expect("#3A0₁₆")
        .test(F5).expect("#1D0₁₆")
        .test(F5).expect("#E8₁₆")
        .test(F5).expect("#74₁₆")
        .test(F5).expect("#3A₁₆")
        .test(F5).expect("#1D₁₆")
        .test(F5).expect("#100E₁₆")
        .test(F5).expect("#807₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#E8₁₆")
        .test(LSHIFT, F5).expect("#1D00₁₆")
        .test(LSHIFT, F5).expect("#1D₁₆")
        .test(LSHIFT, F5).expect("#3A0₁₆")
        .test(LSHIFT, F5).expect("#1403₁₆")
        .test(LSHIFT, F5).expect("#74₁₆")
        .test(LSHIFT, F5).expect("#E80₁₆")
        .test(LSHIFT, F5).expect("#100E₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#1807₁₆")
        .test(F3).expect("#1C03₁₆")
        .test(F3).expect("#1E01₁₆")
        .test(F3).expect("#1F00₁₆")
        .test(F3).expect("#1F80₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#1FFF₁₆")
        .test(LSHIFT, F3).expect("#1FFF₁₆")
        .test(LSHIFT, F3).expect("#1FFF₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#1F00₁₆")
        .test(LSHIFT, F1).expect("#0₁₆")
        .test(RSHIFT, M).expect("#1F00₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#1F₁₆")
        .test(LSHIFT, F2).expect("#0₁₆")
        .test(LSHIFT, F2).expect("#0₁₆")
        .test(RSHIFT, M).expect("#0₁₆");

    step("72-bit test")
        .test(CLEAR, "72 STWS", ENTER, EXIT).noerror();
    step("Shift left")
        .test(CLEAR, "#123A", ID_BasesMenu, F6)
        .test(F1).expect("#2474₁₆")
        .test(F1).expect("#48E8₁₆")
        .test(F1).expect("#91D0₁₆")
        .test(F1).expect("#1 23A0₁₆")
        .test(F1).expect("#2 4740₁₆")
        .test(F1).expect("#4 8E80₁₆")
        .test(F1).expect("#9 1D00₁₆")
        .test(F1).expect("#12 3A00₁₆");
    step("Shift right")
        .test(F2).expect("#9 1D00₁₆")
        .test(F2).expect("#4 8E80₁₆")
        .test(F2).expect("#2 4740₁₆")
        .test(F2).expect("#1 23A0₁₆")
        .test(F2).expect("#91D0₁₆")
        .test(F2).expect("#48E8₁₆")
        .test(F2).expect("#2474₁₆")
        .test(F2).expect("#123A₁₆")
        .test(F2).expect("#91D₁₆")
        .test(F2).expect("#48E₁₆")
        .test(F2).expect("#247₁₆")
        .test(F2).expect("#123₁₆");
    step("Rotate left")
        .test(F4).expect("#246₁₆")
        .test(F4).expect("#48C₁₆")
        .test(F4).expect("#918₁₆")
        .test(F4).expect("#1230₁₆");
    step("Rotate byte left")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#30 0000 0000 0000 0012₁₆")
        .test(LSHIFT, F4).expect("#1230₁₆")
        .test(LSHIFT, F4).expect("#12 3000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#1230 0000 0000 0000₁₆")
        .test(LSHIFT, F4).expect("#12 3000 0000 0000 0000₁₆");
    step("Rotate left with bit rotating")
        .test(F4).expect("#24 6000 0000 0000 0000₁₆")
        .test(F4).expect("#48 C000 0000 0000 0000₁₆")
        .test(F4).expect("#91 8000 0000 0000 0000₁₆")
        .test(F4).expect("#23 0000 0000 0000 0001₁₆")
        .test(F4).expect("#46 0000 0000 0000 0002₁₆")
        .test(F4).expect("#8C 0000 0000 0000 0004₁₆");
    step("Rotate right")
        .test(F5).expect("#46 0000 0000 0000 0002₁₆")
        .test(F5).expect("#23 0000 0000 0000 0001₁₆")
        .test(F5).expect("#91 8000 0000 0000 0000₁₆")
        .test(F5).expect("#48 C000 0000 0000 0000₁₆")
        .test(F5).expect("#24 6000 0000 0000 0000₁₆")
        .test(F5).expect("#12 3000 0000 0000 0000₁₆")
        .test(F5).expect("#9 1800 0000 0000 0000₁₆")
        .test(F5).expect("#4 8C00 0000 0000 0000₁₆")
        .test(F5).expect("#2 4600 0000 0000 0000₁₆")
        .test(F5).expect("#1 2300 0000 0000 0000₁₆")
        .test(F5).expect("#9180 0000 0000 0000₁₆")
        .test(F5).expect("#48C0 0000 0000 0000₁₆");
    step("Rotate right byte")
        .test(LSHIFT, F5).expect("#48 C000 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#48C0 0000 0000₁₆")
        .test(LSHIFT, F5).expect("#48 C000 0000₁₆")
        .test(LSHIFT, F5).expect("#48C0 0000₁₆")
        .test(LSHIFT, F5).expect("#48 C000₁₆")
        .test(LSHIFT, F5).expect("#48C0₁₆")
        .test(LSHIFT, F5).expect("#C0 0000 0000 0000 0048₁₆");
    step("Arithmetic shift right byte")
        .test(LSHIFT, F3).expect("#FF C000 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FF FFC0 0000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FF FFFF C000 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FF FFFF FFC0 0000 0000₁₆")
        .test(LSHIFT, F3).expect("#FF FFFF FFFF C000 0000₁₆");
    step("Arithmetic shift right")
        .test(F3).expect("#FF FFFF FFFF E000 0000₁₆")
        .test(F3).expect("#FF FFFF FFFF F000 0000₁₆")
        .test(F3).expect("#FF FFFF FFFF F800 0000₁₆")
        .test(F3).expect("#FF FFFF FFFF FC00 0000₁₆")
        .test(F3).expect("#FF FFFF FFFF FE00 0000₁₆");
    step("Shift left byte")
        .test(LSHIFT, F1).expect("#FF FFFF FFFE 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FF FFFF FE00 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FF FFFE 0000 0000 0000₁₆")
        .test(LSHIFT, F1).expect("#FF FE00 0000 0000 0000₁₆");
    step("Shift right byte")
        .test(LSHIFT, F2).expect("#FFFE 0000 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FF FE00 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FFFE 0000 0000₁₆")
        .test(LSHIFT, F2).expect("#FF FE00 0000₁₆");

}


void tests::flags_functions()
// ----------------------------------------------------------------------------
//    Check the user flag functions
// ----------------------------------------------------------------------------
{
    BEGIN(flags);

    const uint nflags = 11;

    step("Check that flags are initially clear");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FS?", ENTER).noerror().expect("False");

    step("Setting random flags");
    large fset = lrand48() & ((1<<nflags)-1);
    for (uint f = 0; f < 13; f++)
        test(CLEAR,
             (f * 23) % 128, (fset & (1<<f)) ? " SF" : " CF", ENTER).noerror();

    step("Getting flags value")
        .test(CLEAR, LSHIFT, KEY6, LSHIFT, F1).noerror()
        .type(ID_based_bignum);
    step("Clearing flag values from menu")
        .test("#0", LSHIFT, F2).noerror();
    step("Check that flags are initially clear");
    for (uint f = 0; f < nflags; f++)
        if (fset & (1 << f))
            test((f * 23) % 128, LSHIFT, F5).expect("False").test(BSP);
        else
            test((f * 23) % 128, LSHIFT, F6).expect("True").test(BSP);
    step("Restore values of flags from binary")
        .test(LSHIFT, KEY6, LSHIFT, F2).noerror();

    step("Check that flags were set as expected");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FS?", ENTER)
            .expect((fset & (1<<f)) ? "True" : "False");
    step("Check that flags were clear as expected");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER)
            .expect((fset & (1<<f)) ? "False" : "True");
    step("Check that flags were set and set them");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FS?C", ENTER)
            .expect((fset & (1<<f)) ? "True" : "False");
    step("Check that flags were set them");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER).expect("True");

    step("Setting random flags (inverse pattern) using menu")
        .test(CLEAR, LSHIFT, KEY6);
    for (uint f = 0; f < 13; f++)
        test(CLEAR,
             (f * 23) % 128, (fset & (1<<f)) ? F2 : F1).noerror();
    step("Check that flags were clear and clear them");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, F6, ENTER)
            .expect((fset & (1<<f)) ? "True" : "False");
    step("Check that flags were all clear");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER).expect("True");
    step("Clear flags with menus");
    for (uint f = 0; f < 13; f++)
        test(CLEAR, (f * 23) % 128, F2).noerror();
    step("Check that flags are still all clear");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER).expect("True");

    step("Flipping the bits to revert to original pattern using menu")
        .test(CLEAR, LSHIFT, KEY6);
    for (uint f = 0; f < 13; f++)
        if (fset & (1<<f))
            test(CLEAR, (f * 23) % 128, LSHIFT, F4).noerror();
    step("Check that required flags were flipped using FC?");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER)
            .expect((fset & (1<<f)) ? "False" : "True");
    step("Check that required flags were flipped using FS?C");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FS?C", ENTER)
            .expect((fset & (1<<f)) ? "True" : "False");

    step("Check that flags are all clear at end");
    for (uint f = 0; f < nflags; f++)
        test(CLEAR, (f * 23) % 128, " FC?", ENTER).expect("True");
}


void tests::flags_by_name()
// ----------------------------------------------------------------------------
//   Set and clear all flags by name
// ----------------------------------------------------------------------------
{
    BEGIN(sysflags);

#define ID(id)
#define FLAG(Enable, Disable)                                           \
    step("Clearing flag " #Disable " (default)")                        \
        .test(CLEAR, DIRECT(#Disable), ENTER).noerror()                 \
        .test(DIRECT("'" #Enable "' RCL"), ENTER).expect("False")       \
        .test(DIRECT("'" #Disable "' RCL"), ENTER).expect("True");      \
    step("Setting flag " #Enable)                                       \
        .test(CLEAR, DIRECT(#Enable), ENTER).noerror()                  \
        .test(DIRECT("'" #Enable "' RCL"), ENTER).expect("True")        \
        .test(DIRECT("'" #Disable "' RCL"), ENTER).expect("False");     \
    step("Purging flag " #Enable " (return to default)")                \
        .test(CLEAR, DIRECT("'" #Disable "' PURGE"), ENTER).noerror()   \
        .test(DIRECT("'" #Enable "' RCL"), ENTER).expect("False")       \
        .test(DIRECT("'" #Disable "' RCL"), ENTER).expect("True");      \
    step("Purging flag " #Disable " (return to default)")               \
        .test(CLEAR, DIRECT("'" #Enable "' PURGE"), ENTER).noerror()    \
        .test(DIRECT("'" #Enable "' RCL"), ENTER).expect("False")       \
        .test(DIRECT("'" #Disable "' RCL"), ENTER).expect("True");
#define SETTING(Name, Low, High, Init)                                  \
    step("Purging " #Name " to revert it to default " #Init)            \
        .test(CLEAR, DIRECT("'" #Name "' PURGE"), ENTER).noerror();
#include "ids.tbl"

    step("Clear DebugOnError for testing")
        .test(CLEAR, "KillOnError", ENTER);
}


void tests::settings_by_name()
// ----------------------------------------------------------------------------
//   Set and clear all settings by name
// ----------------------------------------------------------------------------
{
    BEGIN(settings);

#define ID(id)
#define FLAG(Enable, Disable)
#define SETTING(Name, Low, High, Init)                  \
    step("Getting " #Name " current value")             \
        .test(DIRECT("'" #Name "' RCL"), ENTER)         \
        .noerror();                                     \
    step("Setting " #Name " to its current value")      \
        .test(DIRECT("" #Name ""), ENTER)               \
        .noerror();
#include "ids.tbl"
}


void tests::parsing_commands_by_name()
// ----------------------------------------------------------------------------
//   Set and clear all settings by name
// ----------------------------------------------------------------------------
{
    BEGIN(commands);

#define ALIAS(ty, name)                                                 \
    if (object::is_command(object::ID_##ty))                            \
    {                                                                   \
        if (name)                                                       \
        {                                                               \
            step("Parsing " #name " for " #ty);                         \
            test(CLEAR,                                                 \
                 DIRECT("{ " + std::string(name) + " } 1 GET"),         \
                 ENTER)                                                 \
                .type(ID_##ty);                                         \
            test(CLEAR,                                                 \
                 DIRECT("\"{ \" " #name " + \" }\" + Str→ 1 GET"),      \
                 ENTER)                                                 \
                .type(ID_##ty);                                         \
        }                                                               \
    }
#define ID(ty)                  ALIAS(ty, #ty)
#define NAMED(ty, name)         ALIAS(ty, name) ALIAS(ty, #ty)
#include "ids.tbl"
}


void tests::hms_dms_operations()
// ----------------------------------------------------------------------------
//   Test HMS and DMS operations
// ----------------------------------------------------------------------------
{
    BEGIN(hms);

    step("Conversion should not round incorrectly (#1480)")
        .test(CLEAR, DIRECT("10.3033 FromHMS ToHMS"), ENTER)
        .expect("10:30:33")
        .test(CLEAR, DIRECT("10.3033 FromDMS ToDMS"), ENTER)
        .expect("10°30′33″");
    step("Conversion should work OK in symbolic mode")
        .test(CLEAR, DIRECT("NumericalResults 10.2555 FromHMS ToHMS"), ENTER)
        .expect("10:25:55");
    step("Conversion should use proper rounding")
        .test(CLEAR, DIRECT("SymbolicResults 10.2555 FromHMS ToDecimal "
                            "1_hms ToUnit"), ENTER)
        .expect("10:25:55");

    step("HMS data type")
        .test(CLEAR, "1.5_hms", ENTER).expect("1:30:00");
    step("DMS data type")
        .test(CLEAR, "1.7550_dms", ENTER).expect("1°45′18″");
    step("Creating DMS using fractions menu")
        .test(CLEAR, "1.2345", ID_FractionsMenu)
        .test(ID_FromDMS).expect("1 ¹⁹/₄₈")
        .test(ID_ToDMS).expect("1°23′45″");
    step("Creating DMS by adding zero")
        .test(CLEAR, "1.4241 0", ID_FractionsMenu)
        .test(ID_DMSAdd).expect("1°42′41″");
    step("Creating DMS by subtracting one")
        .test(CLEAR, "1.4241 1", ID_FractionsMenu)
        .test(ID_DMSSub).expect("0°42′41″");
    step("HMS addition")
        .test(CLEAR, "1.4241 1.2333 HMS+", ENTER).expect("3:06:14");
    step("DMS addition")
        .test(CLEAR, "1.4241 1.2333 DMS+", ENTER).expect("3°06′14″");
    step("DMS addition through menu")
        .test(CLEAR, "1.4241 1.2333", ID_FractionsMenu, ID_DMSAdd).expect("3°06′14″");
    step("HMS subtraction")
        .test(CLEAR, "1.4241 1.2333 HMS-", ENTER).expect("0:19:08");
    step("DMS subtraction")
        .test(CLEAR, "1.4241 1.2333 DMS-", ENTER).expect("0°19′08″");
    step("DMS subtraction through menu")
        .test(CLEAR, "1.4241 1.2333", ID_FractionsMenu, ID_DMSSub).expect("0°19′08″");
    step("DMS multiplication")
        .test(CLEAR, "1.2345", ID_FractionsMenu)
        .test(ID_FromDMS).expect("1 ¹⁹/₄₈")
        .test(ID_ToDMS).expect("1°23′45″")
        .test(2, MUL).expect("2°47′30″");
    step("DMS division")
        .test(2, DIV).expect("1°23′45″")
        .test(3, DIV).expect("0°27′55″")
        .test(5, DIV).expect("0°05′35″")
        .test(12, DIV).expect("0°00′27″¹¹/₁₂");

    step("Entering integral DMS using two dots")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(DOT).editor("1°_dms")
        .test(ENTER).expect("1°00′00″");
    step("Entering DMS degree/minutes values using two dots")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(ENTER).expect("1°02′00″");
    step("Entering DMS degree/minutes/seconds values using two dots")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(3).editor("1°2′3_dms")
        .test(ENTER).expect("1°02′03″");
    step("Entering degrees/minutes/seconds using three dots")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2,DOT).editor("1°2′_dms")
        .test(35,DOT).editor("1°2′35″_dms")
        .test(ENTER).expect("1°02′35″");
    step("Entering degrees/minutes/seconds/fraction using four dots")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2,DOT).editor("1°2′_dms")
        .test(35,DOT).editor("1°2′35″_dms")
        .test(42,DOT).editor("1°2′35″42/_dms")
        .test(100).editor("1°2′35″42/100_dms")
        .test(ENTER).expect("1°02′35″²¹/₅₀");
    step("Entering integral HMS using two dots and _hms")
        .test(CLEAR, ID_TimeMenu)
        .test(1, DOT).editor("1.")
        .test(DOT).editor("1°_dms")
        .test(F1).editor("1°_hms")
        .test(ENTER).expect("1:00:00");
    step("Entering HMS hr/min values using two dots and _hms")
        .test(CLEAR, ID_TimeMenu)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(F1).editor("1°2′_hms")
        .test(ENTER).expect("1:02:00");
    step("Entering HMS hr/min/sec values using two dots and _hms")
        .test(CLEAR, ID_TimeMenu)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(3).editor("1°2′3_dms")
        .test(F1).editor("1°2′3_hms")
        .test(ENTER).expect("1:02:03");
    step("Entering HMS hr/min using two dots and early _hms")
        .test(CLEAR, ID_TimeMenu)
        .test(1, DOT).editor("1.")
        .test(F1).editor("1._hms")
        .test(2, DOT).editor("1°2′_hms")
        .test(ENTER).expect("1:02:00");
    step("Entering HMS hr/min/sec using two dots and early _hms")
        .test(CLEAR, ID_TimeMenu)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(F1).editor("1°2′_hms")
        .test(3).editor("1°2′3_hms")
        .test(ENTER).expect("1:02:03");
    step("Error when no fraction is given")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2,DOT).editor("1°2′_dms")
        .test(35,DOT).editor("1°2′35″_dms")
        .test(42).editor("1°2′35″42_dms")
        .test(ENTER).error("Syntax error");
    step("Cancelling DMS with third dot")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(DOT).editor("1°_dms")
        .test(DOT).editor("1.")
        .test(ENTER).expect("1.");
    step("DMS disabled in text")
        .test(CLEAR, RSHIFT, ENTER,"1", NOSHIFT, DOT).editor("\"1.\"")
        .test(DOT).editor("\"1..\"")
        .test(DOT).editor("\"1...\"")
        .test(ENTER).expect("\"1...\"");
    step("Invalid DMS value should display correctly")
        .test(CLEAR, "ABC_dms", ENTER).expect("'ABC'");
    step("Invalid HMS value should display correctly")
        .test(CLEAR, "ABC_hms", ENTER).expect("'ABC'");
    step("Invalid date value should display correctly")
        .test(CLEAR, "ABC_date", ENTER).expect("'ABC'");
    step("Inserting zeros automatically")
        .test(CLEAR, DOT, KEY3, DOT, ENTER)
        .expect("0°03′00″");
    step("Inserting zeros automatically")
        .test(CLEAR, KEY3, DOT, DOT, ENTER)
        .expect("3°00′00″");
    step("Inserting zeros automatically")
        .test(CLEAR, DOT, DOT, KEY3, ENTER)
        .expect("0°00′03″");

    step("Entering HMS using cycle key")
        .test(CLEAR)
        .test(1, DOT).editor("1.")
        .test(2, DOT).editor("1°2′_dms")
        .test(3).editor("1°2′3_dms")
        .test(DOWN, EEX).editor("1°2′3_hms")
        .test(EEX).editor("1°2′3_dms")
        .test(DOWN, EEX).editor("1°2′3_hms")
        .test(ENTER).expect("1:02:03");

    step("Converting DMS to HMS")
        .test(CLEAR)
        .test(1, DOT, 2, DOT, 3, ENTER).expect("1°02′03″")
        .test(ID_FractionsMenu, LSHIFT, F5).expect("1:02:03")
        .test(F3).expect("1°02′03″")
        .test(F3).noerror().expect("1°02′03″")
        .test(LSHIFT, F5).noerror().expect("1:02:03")
        .test(LSHIFT, F5).noerror().expect("1:02:03");

    step("Converting time to DMS")
        .test(CLEAR, "1.25_h ToDMS", ENTER)
        .error("Inconsistent units");
    step("Converting time to HMS")
        .test(CLEAR, "1.25_h ToHMS", ENTER)
        .expect("1:15:00");

    step("Converting angles to DMS")
        .test(CLEAR, "0.5", LSHIFT, J, A, RSHIFT, F1).expect("30°00′00″");
}


void tests::date_operations()
// ----------------------------------------------------------------------------
//   Test date-related operations
// ----------------------------------------------------------------------------
{
    BEGIN(date);

    step("Displaying a date")
        .test(CLEAR, "19681205_date", ENTER)
        .expect("Thu 5/Dec/1968");
    step("Displaying a date with a time")
        .test(CLEAR, "19690217.035501_date", ENTER)
        .expect("Mon 17/Feb/1969, 3:55:01");
    step("Displaying a date with a fractional time")
        .test(CLEAR, "19690217.03550197_date", ENTER)
        .expect("Mon 17/Feb/1969, 3:55:01.97");
    step("Displaying invalid date and time")
        .test(CLEAR, "999999999.99999999_date", ENTER)
        .expect("Sat 99/99/99999, 99:99:99.99");

    step("Difference between two dates using DDays")
        .test(CLEAR, "20230908", ENTER)
        .expect("20 230 908")
        .test("19681205", ENTER)
        .expect("19 681 205")
        .test("DDays", ENTER)
        .expect("20 000 d");
    step("Difference between two dates using DDays (units)")
        .test(CLEAR, "19681205_date", ENTER)
        .expect("Thu 5/Dec/1968")
        .test("20230908_date", ENTER)
        .expect("Fri 8/Sep/2023")
        .test("DDays", ENTER)
        .expect("-20 000 d");
    step("Difference between two dates using sub")
        .test(CLEAR, "19681205_date", ENTER)
        .expect("Thu 5/Dec/1968")
        .test("20230908_date", ENTER)
        .expect("Fri 8/Sep/2023")
        .test(SUB)
        .expect("-20 000 d");
    step("Adding days to a date (before)")
        .test("20240217_date", ENTER, ID_add)
        .expect("Fri 16/May/1969");
    step("Adding days to a date (after)")
        .test(CLEAR, "20240217_date", ENTER)
        .expect("Sat 17/Feb/2024")
        .test("42", ID_add)
        .expect("Sat 30/Mar/2024");
    step("Subtracting days to a date")
        .test("116", ID_subtract)
        .expect("Tue 5/Dec/2023");
    step("Subtracting days to a date (with day unit)")
        .test("112_d", ID_subtract)
        .expect("Tue 15/Aug/2023");
    step("Adding days to a date (with time unit)")
        .test("112_h", ID_add)
        .expect("Sat 19/Aug/2023, 16:00:00");
    step("Adding days to a date (with HMS value)")
        .test("5/2_hms", ID_add)
        .expect("Sat 19/Aug/2023, 18:30:00");

    step("Runing TEVAL to time something")
        .test(CLEAR, LSHIFT, RUNSTOP,
              "0 1 10 FOR i i + 0.01 WAIT NEXT", ENTER,
              "TEVAL", LENGTHY(1500), ENTER).noerror()
        .match("duration:[1-3]?[0-9][0-9] ms");
}


void tests::online_help()
// ----------------------------------------------------------------------------
//   Check the online help system
// ----------------------------------------------------------------------------
{
    BEGIN(help);

    step("Main menu shows help as F1")
        .test(CLEAR, EXIT, A, LENGTHY(100), F1).noerror()
        .image_noheader("help");
    step("Exiting help with EXIT")
        .test(EXIT).noerror()
        .image_noheader("help-exit");
    step("Help with keyboard shortcut")
        .test(CLEAR, RSHIFT, ADD).noerror()
        .image_noheader("help");
    step("Following link with ENTER")
        .test(ENTER).noerror()
        .image_noheader("help-topic");
    step("Help with command line")
        .test(CLEAR, "help", ENTER).noerror()
        .image_noheader("help");
    step("History across invokations")
        .test(NOSHIFT, BSP).noerror()
        .image_noheader("help-topic");
    step("Help topic - Integers")
        .test(CLEAR, EXIT, "123", RSHIFT, ADD).noerror()
        .image_noheader("help-integers");
    step("Help topic - Decimal")
        .test(CLEAR, EXIT, "123.5", RSHIFT, ADD).noerror()
        .image_noheader("help-decimal");
    step("Help topic - topic")
        .test(CLEAR, EXIT, "\"authors\"",
              NOSHIFT, RSHIFT, ADD, DOWN, DOWN, DOWN, DOWN)
        .noerror()
        .image_noheader("help-authors");
    step("Returning to main screen with F1")
        .test(F1).noerror()
        .image_noheader("help");
    step("Page up and down with F2 and F3")
        .test(F3).noerror()
        .image_noheader("help-page2")
        .test(F3).noerror()
        .image_noheader("help-page3")
        .test(F2).noerror()
        .image_noheader("help-page4")
        .test(F3).noerror()
        .image_noheader("help-page5");
    step("Follow link with ENTER")
        .test(ENTER).noerror()
        .image_noheader("help-help");
    step("Back to previous topic with BSP")
        .test(BSP).noerror()
        .image_noheader("help-page6");
    step("Next link with F5")
        .test(F3, UP, F4, F5, ENTER).noerror()
        .image_noheader("help-keyboard");
    step("Back with F6")
        .test(F6).noerror()
        .image_noheader("help-page7");
    step("Previous topic with F4")
        .test(UP, F4).noerror()
        .image_noheader("help-page8");
    step("Select topic with ENTER")
        .test(LENGTHY(200), ENTER).noerror()
        .image_noheader("help-design");
    step("Loading a URL")
        .test(F1, F3, ENTER).noerror()
        .image_noheader("help-url")
        .test(EXIT)
        .image_noheader("help-after-url");
    step("Exit to normal command line")
        .test(EXIT, CLEAR, EXIT).noerror();
    step("Invoke help about SIN command with long press")
        .test(LONGPRESS, J)
        .image_noheader("help-sin");
    step("Invoke help about COS command with long press")
        .test(EXIT, LONGPRESS, K)
        .image_noheader("help-cos");
    step("Invoke help about DEG menu command with long press")
        .test(EXIT, SHIFT, N, LONGPRESS, F1)
        .image_noheader("help-degrees");
    step("Exit and cleanup")
        .test(EXIT, CLEAR, EXIT);

    step("Help about integers")
        .test(CLEAR, "123", ENTER, ID_Help)
        .image_noheader("help-integers")
        .test(EXIT, CLEAR, EXIT);
    step("Help about decimal numbers")
        .test(CLEAR, "123.456", ENTER, ID_Help)
        .image_noheader("help-decimal")
        .test(EXIT, CLEAR, EXIT);
    step("Help about based integers")
        .test(CLEAR, "16#123", ENTER, ID_Help)
        .image_noheader("help-based")
        .test(EXIT, CLEAR, EXIT);
    step("Help about texts")
        .test(CLEAR, "\"Text\"", ENTER, ID_Help)
        .image_noheader("help-text")
        .test(EXIT, CLEAR, EXIT);
    step("Help about lists")
        .test(CLEAR, "{ 1 2 3 }", ENTER, ID_Help)
        .image_noheader("help-lists")
        .test(EXIT, CLEAR, EXIT);
    step("Help about vectors")
        .test(CLEAR, "[ 1 2 3 ]", ENTER, ID_Help)
        .image_noheader("help-vectors")
        .test(EXIT, CLEAR, EXIT);
    step("Help about programs")
        .test(CLEAR, "« 1 2 3 »", ENTER, ID_Help)
        .image_noheader("help-programs")
        .test(EXIT, CLEAR, EXIT);
    step("Help about unit objects")
        .test(CLEAR, "« 1 2 3 »", ENTER, ID_Help)
        .image_noheader("help-units")
        .test(EXIT, CLEAR, EXIT);

    step("Enter example from help file into command line")
        .test(CLEAR, "\"ToUnit\" HELP", ENTER,
              DOWN, DOWN, DOWN, DOWN, DOWN)
        .image_noheader("help-example")
        .test(ENTER)
        .editor("@ Will be 3000_km\n3000 2_km →Unit")
        .test(ENTER)
        .expect("3 000 km");
}


void tests::infinity_and_undefined()
// ----------------------------------------------------------------------------
//   Check infinity and undefined operations
// ----------------------------------------------------------------------------
{
    BEGIN(infinity);

    step("Divide by zero error (integer)")
        .test(CLEAR, "1 0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (decimal)")
        .test(CLEAR, "1.0 0.0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (bignum)")
        .test(CLEAR, "2 100 ^ 0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (fractions)")
        .test(CLEAR, "1/3 0", ENTER, ID_divide)
        .error("Divide by zero");

    step("Setting infinity flag")
        .test(CLEAR, "'InfinityValue' FS?", ENTER).expect("False")
        .test("-22 SF", ENTER).noerror()
        .test("'InfinityValue' FS?", ENTER).expect("True");

    step("Clear infinite result flag")
        .test("-26 CF", ENTER).noerror()
        .test("'InfiniteResultIndicator' FS?", ENTER).expect("False");

    step("Divide by zero as symbolic infinity (integer)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1 0", ENTER, ID_divide)
        .expect("∞")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (decimal)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1.0 0.0", ENTER, ID_divide)
        .expect("∞")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (bignum)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("2 100 ^ 0", ENTER, ID_divide)
        .expect("∞")
        .test("'InfiniteResultIndicator' FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (fractions)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1/3 0", ENTER, ID_divide)
        .expect("∞")
        .test("'InfiniteResultIndicator' FS?C", ENTER).expect("True");

    step("Divide by zero as symbolic infinity (negative integer)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1 0", ENTER, ID_divide)
        .expect("−∞")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (decimal)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1.0 0.0", ENTER, ID_divide)
        .expect("−∞")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (bignum)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("2 100 ^ NEG 0", ENTER, ID_divide)
        .expect("−∞")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as symbolic infinity (fractions)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1/3 0", ENTER, ID_divide)
        .expect("−∞")
        .test("-26 FS?C", ENTER).expect("True");

    step("Setting numerical constants flag")
        .test(CLEAR, "'NumericalConstants' FS?", ENTER).expect("False")
        .test("-2 SF", ENTER).noerror()
        .test("'NumericalConstants' FS?", ENTER).expect("True");

    step("Divide by zero as numeric infinity (integer)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (decimal)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1.0 0.0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (bignum)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("2 100 ^ 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (fractions)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("1/3 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");

    step("Divide by zero as numeric infinity (negative integer)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (decimal)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1.0 0.0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (bignum)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("2 100 ^ NEG 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");
    step("Divide by zero as numeric infinity (fractions)")
        .test(CLEAR, "-26 FS?", ENTER).expect("False")
        .test("-1/3 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹")
        .test("-26 FS?C", ENTER).expect("True");

    step("Clearing numerical constants flag")
        .test(CLEAR, "'NumericalConstants' FS?", ENTER).expect("True")
        .test("-2 CF", ENTER).noerror()
        .test("'NumericalConstants' FS?", ENTER).expect("False");
    step("Setting numerical results flag")
        .test(CLEAR, "'NumericalResults' FS?", ENTER).expect("False")
        .test("-3 SF", ENTER).noerror()
        .test("'NumericalResults' FS?", ENTER).expect("True");

    step("Divide by zero as numeric infinity (integer)")
        .test(CLEAR, "1 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (decimal)")
        .test(CLEAR, "1.0 0.0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (bignum)")
        .test(CLEAR, "2 100 ^ 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (fractions)")
        .test(CLEAR, "1/3 0", ENTER, ID_divide)
        .expect("9.99999⁳⁹⁹⁹⁹⁹⁹");

    step("Divide by zero as numeric infinity (negative integer)")
        .test(CLEAR, "-1 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (decimal)")
        .test(CLEAR, "-1.0 0.0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (bignum)")
        .test(CLEAR, "2 100 ^ NEG 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Divide by zero as numeric infinity (fractions)")
        .test(CLEAR, "-1/3 0", ENTER, ID_divide)
        .expect("-9.99999⁳⁹⁹⁹⁹⁹⁹");

    step("Clear numerical results flag")
        .test(CLEAR, "'NumericalResults' FS?", ENTER).expect("True")
        .test("-3 CF", ENTER).noerror()
        .test("'NumericalResults' FS?", ENTER).expect("False");

    step("Divide by zero as symbolic infinity (integer)")
        .test(CLEAR, "1 0", ENTER, ID_divide)
        .expect("∞");
    step("Divide by zero as symbolic infinity (decimal)")
        .test(CLEAR, "1.0 0.0", ENTER, ID_divide)
        .expect("∞");
    step("Divide by zero as symbolic infinity (bignum)")
        .test(CLEAR, "2 100 ^ 0", ENTER, ID_divide)
        .expect("∞");
    step("Divide by zero as symbolic infinity (fractions)")
        .test(CLEAR, "1/3 0", ENTER, ID_divide)
        .expect("∞");

    step("Clear infinity value flag")
        .test(CLEAR, "'InfinityError' FS?", ENTER).expect("False")
        .test("-22 CF", ENTER).noerror()
        .test("'InfinityError' FS?", ENTER).expect("True");

    step("Divide by zero error (integer)")
        .test(CLEAR, "1 0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (decimal)")
        .test(CLEAR, "1.0 0.0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (bignum)")
        .test(CLEAR, "2 100 ^ 0", ENTER, ID_divide)
        .error("Divide by zero");
    step("Divide by zero error (fractions)")
        .test(CLEAR, "1/3 0", ENTER, ID_divide)
        .error("Divide by zero");

    test(CLEAR);
}


void tests::overflow_and_underflow()
// ----------------------------------------------------------------------------
//   Test overflow and underflow
// ----------------------------------------------------------------------------
{
    BEGIN(overflow);

    step("Set maximum exponent to 499")
        .test(CLEAR, "499 MaximumDecimalExponent", ENTER).noerror()
        .test("'MaximumDecimalExponent' RCL", ENTER).expect("#1F3₁₆");

    step("Check that undeflow error is not set by default")
        .test("'UnderflowError' FS?", ENTER).expect("False")
        .test("'UnderflowValue' FS?", ENTER).expect("True");

    step("Clear overflow and underflow indicators")
        .test("-23 CF -24 CF -25 CF -26 CF", ENTER).noerror();
    step("Check negative underflow indicator is clear")
        .test("'NegativeUnderflowIndicator' FS?", ENTER).expect("False");
    step("Check positive underflow indicator is clear")
        .test("'PositiveUnderflowIndicator' FS?", ENTER).expect("False");
    step("Check overflow indicator is clear")
        .test("'OverflowIndicator' FS?", ENTER).expect("False");
    step("Check infinite result indicator is clear")
        .test("'InfiniteResultindicator' FS?", ENTER).expect("False");

    step("Test numerical overflow as infinity for multiply")
        .test(CLEAR)
        .test("1E499 10 *", ENTER).expect("∞")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("True");
    step("Test numerical overflow as infinity for exponential")
        .test(CLEAR)
        .test("1280 exp", ENTER).expect("∞")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("True");
    step("Test numerical overflow as infinity")
        .test(CLEAR)
        .test("1E499 10 *", ENTER).expect("∞")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("True");
    step("Test positive numerical underflow as zero")
        .test(CLEAR)
        .test("1E-499 10 /", ENTER).expect("0.")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("True")
        .test("-25 FS?C", ENTER).expect("False");
    step("Test negative numerical underflow as zero")
        .test(CLEAR)
        .test("-1E-499 10 /", ENTER).expect("-0.")
        .test("-23 FS?C", ENTER).expect("True")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");

    step("Set underflow as error")
        .test(CLEAR, "-20 SF", ENTER).noerror()
        .test("'UnderflowError' FS?", ENTER).expect("True");

    step("Test numerical overflow as infinity")
        .test(CLEAR)
        .test("1E499 10 *", ENTER).expect("∞")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("True");
    step("Test positive numerical underflow as error")
        .test(CLEAR)
        .test("1E-499 10 /", ENTER).error("Positive numerical underflow")
        .test(CLEARERR).expect("10")
        .test(BSP).expect("1.⁳⁻⁴⁹⁹")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");
    step("Test negative numerical underflow as error")
        .test(CLEAR)
        .test("-1E-499 10 /", ENTER).error("Negative numerical underflow")
        .test(CLEARERR).expect("10")
        .test(BSP).expect("-1.⁳⁻⁴⁹⁹")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");

    step("Set overflow as error")
        .test(CLEAR, "-21 SF", ENTER).noerror()
        .test("'OverflowError' FS?", ENTER).expect("True");

    step("Test numerical overflow as infinity")
        .test(CLEAR)
        .test("1E499 10 *", ENTER).error("Numerical overflow")
        .test(CLEARERR).expect("10")
        .test(BSP).expect("1.⁳⁴⁹⁹")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");
    step("Test positive numerical underflow as error")
        .test(CLEAR)
        .test("1E-499 10 /", ENTER).error("Positive numerical underflow")
        .test(CLEARERR).expect("10")
        .test(BSP).expect("1.⁳⁻⁴⁹⁹")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");
    step("Test negative numerical underflow as error")
        .test(CLEAR)
        .test("-1E-499 10 /", ENTER).error("Negative numerical underflow")
        .test(CLEARERR).expect("10")
        .test(BSP).expect("-1.⁳⁻⁴⁹⁹")
        .test("-23 FS?C", ENTER).expect("False")
        .test("-24 FS?C", ENTER).expect("False")
        .test("-25 FS?C", ENTER).expect("False");

    step("Reset modes")
        .test(CLEAR, "ResetModes KillOnError", ENTER)
        .test("'MaximumDecimalExponent' RCL", ENTER)
        .expect("#1000 0000 0000 0000₁₆");
}


void tests::graphic_stack_rendering()
// ----------------------------------------------------------------------------
//   Check the rendering of expressions in graphic mode
// ----------------------------------------------------------------------------
{
    BEGIN(gstack);

    step("Draw expression")
        .test(CLEAR, EXIT, EXIT)
        .test("1 'X' +", ENTER, ID_inv, ID_sqrt, ID_ln,
              "3 X 3", ID_pow, ID_multiply, ID_add)
        .test(ALPHA, X, NOSHIFT, ID_sin, ID_cos, ID_tan, ID_add)
        .image_noheader("expression");

    step("Two levels of stack")
        .test(CLEAR, EXIT, EXIT)
        .test("1 'X' +", ENTER, ID_inv, ID_sqrt, ID_ln,
              "3 X 3", ID_pow, ID_multiply, ID_add)
        .test(ALPHA, X, NOSHIFT, ID_sin, ID_cos, ID_tan)
        .image_noheader("two-levels");

    step("Automatic reduction of size")
        .test(CLEAR, EXIT, EXIT)
        .test("1 'X' +", ENTER, ID_inv, ID_sqrt, ID_ln,
              "3 X 3", ID_pow, ID_multiply, ID_add)
        .test(ALPHA, X, NOSHIFT, ID_sin, ID_cos, ID_tan, ID_add,
              ID_sqrt, ID_inv, ID_sqrt, ID_inv)
        .image_noheader("reduced");

    step("Constants")
        .test(CLEAR, LSHIFT, I, F2, F1, F2, F3)
        .image_noheader("constants", 2);

    step("Vector")
        .test(CLEAR, LSHIFT, KEY9, "1 2 3", ENTER, EXIT)
        .image_noheader("vector-vertical");
    step("Vector horizontal rendering")
        .test("HorizontalVectors", ENTER)
        .image_noheader("vector-horizontal");
    step("Vector vertical rendering")
        .test("VerticalVectors", ENTER)
        .image_noheader("vector-vertical");

    step("Matrix")
        .test(CLEAR, LSHIFT, KEY9,
              LSHIFT, KEY9, "1 2 3 4", DOWN,
              LSHIFT, KEY9, "4 5 6 7", DOWN,
              LSHIFT, KEY9, "8 9 10 11", DOWN,
              LSHIFT, KEY9, "12 13 14 18", ENTER, EXIT)
        .image_noheader("matrix");
    step("Matrix with smaller size")
        .test(13, DIV, ENTER, MUL)
        .image_noheader("matrix-smaller");

    step("Lists")
        .test(CLEAR, RSHIFT, SPACE, "1 2 \"ABC\"", ENTER, EXIT)
        .image_noheader("list-horizontal");
    step("List vertical")
        .test("VerticalLists", ENTER)
        .test(CLEAR, RSHIFT, SPACE, "1 2 \"ABC\"", ENTER, EXIT)
        .image_noheader("list-vertical");
    step("List horizontal")
        .test("HorizontalLists", ENTER)
        .test(CLEAR, RSHIFT, SPACE, "1 2 \"ABC\"", ENTER, EXIT)
        .image_noheader("list-horizontal");

    step("Power")
        .test(CLEAR, "'2^x'", ENTER)
        .image_noheader("power-xgraph")
        .test(CLEAR, "'(x-1)^(n+3)'", ENTER)
        .image_noheader("power-expr-xgraph");
    step("Exponentials")
        .test(CLEAR, "'exp(y+1)'", ENTER)
        .image_noheader("exp-xgraph")
        .test(CLEAR, "'alog(1/(x+1))'", ENTER)
        .image_noheader("alog-xgraph")
        .test(CLEAR, "'exp2(3/(x-1))'", ENTER)
        .image_noheader("alog2-xgraph");

    step("Square root")
        .test(CLEAR, "'sqrt(1/(1+x))+1'", ENTER)
        .image_noheader("sqrt-xgraph");
    step("Cube root")
        .test(CLEAR, "'cbrt(1/(1+x))+1'", ENTER)
        .image_noheader("cbrt-xgraph");
    step("N-th root")
        .test(CLEAR, "'1/(1+x)' 'n-1'", ID_xroot, 1, ID_add)
        .image_noheader("xroot-xgraph");

    step("Combination and permutations")
        .test(CLEAR, "X Y COMB N M 1 + PERM + 3 +", ENTER)
        .image_noheader("comb-perm-xgraph");

    step("Sum")
        .test(CLEAR, "I 1 10 N + 'I+1' Σ 3 +", ENTER)
        .image_noheader("sum-xgraph");
    step("Product")
        .test(CLEAR, "2 J 1.2 10.2 K * 'J+4' ∏ *", ENTER)
        .image_noheader("product-xgraph");

    step("Graphics stack rendering settings")
        .test(CLEAR, "'sqrt(x)' 1/2 3/4", ENTER, EXIT,
              ID_UserInterfaceModesMenu)
        .image_noheader("graph-stack-and-result-both")
        .test(ID_GraphicStackDisplay)
        .image_noheader("graph-stack-and-result-result-only")
        .test(ID_GraphicResultDisplay)
        .image_noheader("graph-stack-and-result-none")
        .test(ID_GraphicStackDisplay)
        .image_noheader("graph-stack-and-stack-only")
        .test(ID_GraphicResultDisplay)
        .image_noheader("graph-stack-and-result-both");
}


void tests::insertion_of_variables_constants_and_units()
// ----------------------------------------------------------------------------
//   Check that we correctly insert constant and variables in programs
// ----------------------------------------------------------------------------
{
    BEGIN(insert);

    step("Select constant menu")
        .test(CLEAR, LSHIFT, I, F2).image_menus("constants-menu", 1);
    step("Insert pi")
        .test(CLEAR, F1).expect("π");
    step("Insert e")
        .test(CLEAR, F2).expect("e");
    step("Insert i")
        .test(CLEAR, F3).expect("ⅈ");
    step("Insert Infinity")
        .test(CLEAR, F4).expect("∞");
    step("Insert Undefined")
        .test(CLEAR, F5).expect("?");
    step("Insert j")
        .test(CLEAR, F6, F1).expect("ⅉ");
    step("Insert rad")
        .test(CLEAR, F2).expect("rad");
    step("Insert two pi")
        .test(CLEAR, F3).expect("twoπ");
    step("Insert angl")
        .test(CLEAR, F4, F6).expect("angl");

    step("Insert pi value")
        .test(CLEAR, LSHIFT, F1).expect("3.14159 26535 9");
    step("Insert e value")
        .test(CLEAR, LSHIFT, F2).expect("2.71828 18284 6");
    step("Insert i value")
        .test(CLEAR, LSHIFT, F3).expect("0+1ⅈ");
    step("Insert infinity value")
        .test(CLEAR, LSHIFT, F4).expect("9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Insert undefined value")
        .test(CLEAR, LSHIFT, F5).expect("Undefined");
    step("Insert j value")
        .test(CLEAR, F6, LSHIFT, F1).expect("0+1ⅈ");
    step("Insert rad value")
        .test(CLEAR, LSHIFT, F2).expect("1 r");
    step("Insert two pi value")
        .test(CLEAR, LSHIFT, F3).expect("6.28318 53071 8 r");
    step("Insert angl value")
        .test(CLEAR, LSHIFT, F4, F6).expect("180 °");

    step("Begin program")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»");
    step("Insert pi")
        .test(F1).editor("« Ⓒπ »");
    step("Insert e")
        .test(F2).editor("« Ⓒπ  Ⓒe »");
    step("Insert i")
        .test(F3).editor("« Ⓒπ  Ⓒe  Ⓒⅈ »");
    step("Insert infinity")
        .test(F4).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞ »");
    step("Insert undefined")
        .test(F5).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ? »");

    step("Insert pi value")
        .test(LSHIFT, F1).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ?  "
                                 "3.14159 26535 89793 23846 264 »");
    step("Insert e value")
        .test(LSHIFT, F2).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ?  "
                                 "3.14159 26535 89793 23846 264  "
                                 "2.71828 18284 59045 23536 029 »");
    step("Insert i value")
        .test(LSHIFT, F3).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ?  "
                                 "3.14159 26535 89793 23846 264  "
                                 "2.71828 18284 59045 23536 029  "
                                 "0+ⅈ1 »");
    step("Insert infinity value")
        .test(LSHIFT, F4).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ?  "
                                 "3.14159 26535 89793 23846 264  "
                                 "2.71828 18284 59045 23536 029  "
                                 "0+ⅈ1  "
                                 "9.99999⁳999999 »");
    step("Insert undefined value")
        .test(LSHIFT, F5).editor("« Ⓒπ  Ⓒe  Ⓒⅈ  Ⓒ∞  Ⓒ?  "
                                 "3.14159 26535 89793 23846 264  "
                                 "2.71828 18284 59045 23536 029  "
                                 "0+ⅈ1  "
                                 "9.99999⁳999999  "
                                 "Undefined »");

    step("Test that constants parse")
        .test(ENTER)
        .want("« π e ⅈ ∞ ? "
              "3.14159 26535 9 2.71828 18284 6 0+1ⅈ 9.99999⁳⁹⁹⁹⁹⁹⁹ "
              "Undefined »", 300);

    step("Select library menu")
        .test(CLEAR, RSHIFT, H).noerror();
    step("Select secrets menu")
        .test(F1).noerror();
    step("Insert Dedicace")
        .test(CLEAR, LSHIFT, RUNSTOP, F1, ENTER).want("« Dedicace »");
    cstring ded = "\"À tous ceux qui se souviennent de Maubert électronique\"";
    step("Evaluate stack Dedicace")
        .test(RUNSTOP).expect(ded);
    step("Evaluate Dedicace")
        .test(CLEAR, F1).expect(ded);
    step("Evaluate Dedicace directly")
        .test(LSHIFT, F1).expect(ded);

    step("Begin program")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»");
    step("Insert Dedicace")
        .test(F1).editor("« ⓁDedicace »");
    step("Insert LibraryHelp")
        .test(F2).editor("« ⓁDedicace  ⓁLibraryHelp »");

    step("Test that xlibs parse")
        .test(ENTER)
        .want("« Dedicace LibraryHelp »");
    step("Test that xlib can be edited")
        .test(DOWN)
        .editor("«\n\tⓁDedicace ⓁLibraryHelp\n»");
    step("Test that xlib edit can be entered")
        .test(ENTER)
        .want("« Dedicace LibraryHelp »");
    step("Test that xlib name can be edited")
        .test(DOWN)
        .editor("«\n\tⓁDedicace ⓁLibraryHelp\n»")
        .test(DOWN, DOWN, DOWN, DOWN, "ee")
        .editor("«\n\tⓁeeDedicace ⓁLibraryHelp\n»");
    step("Test that bad xlib name is detected")
        .test(ENTER)
        .error("Invalid or unknown library entry")
        .test(EXIT);

    step("Programmatic constant lookup (symbol)")
        .test(CLEAR, "c CONST", ENTER)
        .expect("299 792 458 m/s");
    step("Programmatic equation lookup (symbol)")
        .test(CLEAR, "RelativityMassEnergy LIBEQ", ENTER)
        .expect("'E=m·c↑2'");
    step("Programmatic library lookup (symbol)")
        .test(CLEAR, "Dedicace XLIB", ENTER)
        .expect("\"À tous ceux qui se souviennent de Maubert électronique\"");
    step("Programmatic constant lookup (text)")
        .test(CLEAR, "\"NA\" CONST", ENTER)
        .expect("6.02214 076⁳²³ mol⁻¹");
    step("Programmatic equation lookup (text)")
        .test(CLEAR, "\"IdealGas\" LIBEQ", ENTER)
        .expect("'P·V=n·R·T'");
    step("Programmatic library lookup (text)")
        .test(CLEAR, "\"LibraryHelp\" XLIB", ENTER)
        .expect("\"To modify the library, edit the config/library.csv file\"");
    step("Programmatic constant lookup (error)")
        .test(CLEAR, "NotExistent CONST", ENTER)
        .error("Invalid or unknown constant");
    step("Programmatic equation lookup (error)")
        .test(CLEAR, "\"StrangeGas\" LIBEQ", ENTER)
        .error("Not an equation or program");
    step("Programmatic library lookup (error)")
        .test(CLEAR, "\"Glop\" XLIB", ENTER)
        .error("Invalid or unknown library entry");

    step("Select units menu")
        .test(CLEAR, LSHIFT, KEY5, F4).image_menus("units-menu", 3);
    step("Select meter")
        .test(CLEAR, KEY1, F1).editor("1_m").test(ENTER).expect("1 m");
    step("Convert to yards")
        .test(LSHIFT, F2).expect("1 ¹⁰⁷/₁ ₁₄₃ yd");
    step("Select yards")
        .test(CLEAR, KEY1, F2).editor("1_yd").test(ENTER).expect("1 yd");
    step("Convert to feet")
        .test(LSHIFT, F3).expect("3 ft");
    step("Select feet")
        .test(CLEAR, KEY1, F3).editor("1_ft").test(ENTER).expect("1 ft");
    step("Convert to meters")
        .test(LSHIFT, F1).expect("³⁸¹/₁ ₂₅₀ m");

    step("Enter 27_m in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", NOSHIFT, F1).editor("«27_m»")
        .test(ENTER).want("« 27 m »")
        .test(RUNSTOP).expect("27 m");
    step("Enter 27_yd in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", NOSHIFT, F2).editor("«27_yd»")
        .test(ENTER).want("« 27 yd »")
        .test(RUNSTOP).expect("27 yd");
    step("Enter 27_ft in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", NOSHIFT, F3).editor("«27_ft»")
        .test(ENTER).want("« 27 ft »")
        .test(RUNSTOP).expect("27 ft");

    step("Enter A with unit _m in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("A", NOSHIFT, F1).editor("«A 1_m * »")
        .test(ENTER).want("« A 1 m × »")
        .test(RUNSTOP).expect("'A'");

    step("Enter 27_m⁻¹ in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", RSHIFT, F1).editor("«27_(m)⁻¹»")
        .test(ENTER).want("« 27 m⁻¹ »")
        .test(RUNSTOP).expect("27 m⁻¹");
    step("Enter 27_yd⁻¹ in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", RSHIFT, F2).editor("«27_(yd)⁻¹»")
        .test(ENTER).want("« 27 yd⁻¹ »")
        .test(RUNSTOP).expect("27 yd⁻¹");
    step("Enter 27_ft⁻¹ in program and evaluate it")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test("27", RSHIFT, F3).editor("«27_(ft)⁻¹»")
        .test(ENTER).want("« 27 ft⁻¹ »")
        .test(RUNSTOP).expect("27 ft⁻¹");

    step("Select variables menu")
        .test(CLEAR, NOSHIFT, H).noerror();
    step("Create variables named Foo and Baz")
        .test(CLEAR, "1968 'Foo'", NOSHIFT, G).noerror()
        .test("42", NOSHIFT, F, "Baz", NOSHIFT, G).noerror();

    step("Check we can read the variables back")
        .test(CLEAR, F1).expect("42")
        .test(CLEAR, F2).expect("1 968");

    step("Insert evaluation code")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test(F1).editor("« Baz »")
        .test(F2).editor("« Baz  Foo »");
    step("Check position of insertion point")
        .test("ABC").editor("« Baz  Foo ABC»");
    step("Check we can parse resulting program")
        .test(ENTER).want("« Baz Foo ABC »");
    step("Check evaluation of program")
        .test(RUNSTOP).expect("'ABC'")
        .test(BSP).expect("1 968")
        .test(BSP).expect("42")
        .test(BSP, BSP).error("Too few arguments");

    step("Insert recall code")
        .test(CLEAR, LSHIFT, RUNSTOP).editor("«»")
        .test(LSHIFT, F1).editor("« 'Baz' Recall »")
        .test(LSHIFT, F2).editor("« 'Baz' Recall  'Foo' Recall »");
    step("Insert store code")
        .test(RSHIFT, F1).editor("« 'Baz' Recall  'Foo' Recall  "
                                 "'Baz' Store »")
        .test(RSHIFT, F2).editor("« 'Baz' Recall  'Foo' Recall  "
                                 "'Baz' Store  'Foo' Store »");
    step("Check that it parses")
        .test(ENTER).want("« 'Baz' Recall 'Foo' Recall "
                          "'Baz' Store 'Foo' Store »");
    step("Check evaluation")
        .test(RUNSTOP)
        .test(CLEAR, F1).expect("1 968")
        .test(CLEAR, F2).expect("42");
    step("Cleanup")
        .test(CLEAR, "'Foo' Purge 'Baz' Purge", ENTER);
}


void tests::constants_menu()
// ----------------------------------------------------------------------------
//   Check that all the constants can be inserted in the program
// ----------------------------------------------------------------------------
{
    BEGIN(constants);

    step("Insert constant from command line")
        .test(CLEAR, "Ⓒc", ENTER)
        .expect("c")
        .test(ID_ToDecimal)
        .expect("299 792 458 m/s");
    step("Insert constant from constants menu")
        .test(CLEAR, ID_ConstantsMenu, RSHIFT, F1, "c", ENTER)
        .expect("c")
        .test(ID_ToDecimal)
        .expect("299 792 458 m/s");

    step("Check that numerical constants are adjusted with precision")
        .test(CLEAR,
              ID_DisplayModesMenu, 60, ID_Sig,
              NOSHIFT, F, ID_Precision, ENTER, ID_Purge,
              ID_ConstantsMenu, F2, LSHIFT, F1)
        .expect("3.14159 26535 89793 23846 264")
        .test(LSHIFT, F2)
        .expect("2.71828 18284 59045 23536 029")
        .test(CLEAR,
              ID_DisplayModesMenu, 60, ID_Precision,
              ID_ConstantsMenu, F2, LSHIFT, F1)
        .expect("3.14159 26535 89793 23846 26433 83279 50288 41971 69399 37510 58209 7494")
        .test(LSHIFT, F2)
        .expect("2.71828 18284 59045 23536 02874 71352 66249 77572 47093 69995 95749 6697")
        .test(CLEAR, ID_DisplayModesMenu, ID_Std,
              NOSHIFT, F, ID_Precision, ENTER, ID_Purge);

    step("Insert constant standard uncertainty from command line")
        .test(CLEAR, "Ⓢc", ENTER)
        .expect("Ⓢc")
        .test(ID_ToDecimal)
        .expect("0 m/s");
    step("Insert constant standard uncertainty from constants menu")
        .test(CLEAR, ID_ConstantsMenu, RSHIFT, F3, "c", ENTER)
        .expect("Ⓢc")
        .test(ID_ToDecimal)
        .expect("0 m/s");

    step("Insert constant relative uncertainty from command line")
        .test(CLEAR, "Ⓡc", ENTER)
        .expect("Ⓡc")
        .test(ID_ToDecimal)
        .expect("0");
    step("Insert constant relative uncertainty from constants menu")
        .test(CLEAR, ID_ConstantsMenu, RSHIFT, F5, "c", ENTER)
        .expect("Ⓡc")
        .test(ID_ToDecimal)
        .expect("0");

    step("Standard rounding from command line")
        .test(CLEAR, "5.36248084521_g 0.11_g StdRnd", ENTER)
        .expect("5.36 g");
    step("Standard rounding from menu")
        .test(CLEAR, "5.36248084521_g 0.11_g", ENTER,
              ID_PartsMenu, ID_StandardRound)
        .expect("5.36 g");
    step("Standard rounding with unit conversion")
        .test(CLEAR, "5.36248084521_kg 0.11_g", ENTER,
              ID_PartsMenu, ID_StandardRound)
        .expect("5.36248 kg");
    step("Standard rounding with invalid units")
        .test(CLEAR, "5.36248084521_kg 0.11",
              ID_PartsMenu, ID_StandardRound)
        .expect("5.36 kg");
    step("Standard rounding with invalid units")
        .test(CLEAR, "5.36248084521_kg 0.11_cm", ENTER,
              ID_PartsMenu, ID_StandardRound)
        .error("Inconsistent units");

    step("Relative rounding from command line")
        .test(CLEAR, "5.36248084521_g 0.101 RelRnd", ENTER)
        .expect("5.36 g");
    step("Relative rounding from menu")
        .test(CLEAR, "5.36248084521_g 0.101", ENTER,
              ID_PartsMenu, ID_RelativeRound)
        .expect("5.36 g");
    step("Relative rounding with unit conversion")
        .test(CLEAR, "5.36248084521_kg 0.101", ENTER,
              ID_PartsMenu, ID_RelativeRound)
        .expect("5.36 kg");
    step("Relative rounding with invalid units")
        .test(CLEAR, "5.36248084521_kg 0.11_g", ENTER,
              ID_PartsMenu, ID_RelativeRound)
        .error("Bad argument type");

    step("Precision rounding from command line")
        .test(CLEAR, "5.36248084521_g 0.101_g PrcRnd", ENTER)
        .expect("5.362 g");
    step("Precision rounding from menu")
        .test(CLEAR, "5.36248084521_g 0.101_g", ENTER,
              ID_PartsMenu, ID_PrecisionRound)
        .expect("5.362 g");
    step("Precision rounding with unit conversion")
        .test(CLEAR, "5.36248084521_kg 0.101_g", ENTER,
              ID_PartsMenu, ID_PrecisionRound)
        .expect("5.36248 1 kg");
    step("Precision rounding with invalid units")
        .test(CLEAR, "5.36248084521_kg 0.11_cm", ENTER,
              ID_PartsMenu, ID_StandardRound)
        .error("Inconsistent units");

    step("Use Const command from command line")
        .test(CLEAR, "'c' CONST", ENTER, ID_ToDecimal)
        .expect("299 792 458. m/s");

    step("Dates constants menu")
        .test(CLEAR, LSHIFT, I, F1);
    step("Bastille day")
        .test(CLEAR, NOSHIFT, F1).expect("BastilleDay")
        .test(LSHIFT, F1).expect("Tue 14/Jul/1789");
    step("Martin Luther King's day")
        .test(CLEAR, NOSHIFT, F2).expect("MartinLutherKingDeath")
        .test(LSHIFT, F2).expect("Thu 4/Apr/1968");
    step("Independence Day")
        .test(CLEAR, NOSHIFT, F3).expect("IndependenceDay")
        .test(LSHIFT, F3).expect("Thu 4/Jul/1776");

    // ------------------------------------------------------------------------
    step("Mathematics constants menu")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, F2);
    step("Pi")
        .test(CLEAR, NOSHIFT, F1).expect("π")
        .test(LSHIFT, F1).expect("3.14159 26535 9");
    step("e")
        .test(CLEAR, NOSHIFT, F2).expect("e")
        .test(LSHIFT, F2).expect("2.71828 18284 6");
    step("i")
        .test(CLEAR, NOSHIFT, F3).expect("ⅈ")
        .test(LSHIFT, F3).expect("0+1ⅈ");
    step("Undefined")
        .test(CLEAR, NOSHIFT, F4).expect("∞")
        .test(LSHIFT, F4).expect("9.99999⁳⁹⁹⁹⁹⁹⁹");
    step("Infinity")
        .test(CLEAR, NOSHIFT, F5).expect("?")
        .test(LSHIFT, F5).expect("Undefined");
    step("j")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("ⅉ")
        .test(LSHIFT, F1).expect("0+1ⅈ");
    step("rad")
        .test(CLEAR, NOSHIFT, F2).expect("rad")
        .test(LSHIFT, F2).expect("1 r");
    step("twoπ")
        .test(CLEAR, NOSHIFT, F3).expect("twoπ")
        .test(LSHIFT, F3).expect("6.28318 53071 8 r");
    step("anglπ")
        .test(CLEAR, NOSHIFT, F4).expect("angl")
        .test(LSHIFT, F4).expect("180 °");

    step("Chemistry constants")
        .test(CLEAR, LSHIFT, I, F3);
    step("Avogadro constant")
        .test(CLEAR, NOSHIFT, F1).expect("NA")
        .test(LSHIFT, F1).expect("6.02214 076⁳²³ mol⁻¹");
    step("Boltzmann constant")
        .test(CLEAR, NOSHIFT, F2).expect("k")
        .test(LSHIFT, F2).expect("1.38064 9⁳⁻²³ J/K");
    step("Molar volume")
        .test(CLEAR, NOSHIFT, F3).expect("Vm")
        .test(LSHIFT, F3).expect("0.02241 39695 45 m↑3/mol");
    step("Universal Gas constant")
        .test(CLEAR, NOSHIFT, F4).expect("R")
        .test(LSHIFT, F4).expect("8.31446 26181 5 J/(mol·K)");
    step("Stefan-Boltzmann constant")
        .test(CLEAR, NOSHIFT, F5).expect("σ")
        .test(LSHIFT, F5).expect("0.00000 00567 04 W/(m↑2·K↑4)");
    step("Standard temperature")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("StdT")
        .test(LSHIFT, F1).expect("273.15 K");
    step("Standard pressure")
        .test(CLEAR, NOSHIFT, F2).expect("StdP")
        .test(LSHIFT, F2).expect("101.325 kPa");
    step("Mass unit")
        .test(CLEAR, NOSHIFT, F3).expect("Mu")
        .test(LSHIFT, F3).expect("1.00000 00010 5 g/mol");
    step("Carbon-12 mass")
        .test(CLEAR, NOSHIFT, F4).expect("MC12")
        .test(LSHIFT, F4).expect("12.00000 00126 g/mol");
    step("Loschmidt constant")
        .test(CLEAR, NOSHIFT, F5).expect("n0")
        .test(LSHIFT, F5).expect("2.68678 01118⁳²⁵ (m↑3)⁻¹");
    step("Sakur-Tetrode constant")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("SoR")
        .test(LSHIFT, F1).expect("-1.16487 05214 9");
    step("Dalton constant")
        .test(CLEAR, NOSHIFT, F2).expect("Da")
        .test(LSHIFT, F2).expect("1.66053 90689 2⁳⁻²⁷ kg");
    step("Boltzmann / electron mass ratio")
        .test(CLEAR, NOSHIFT, F3).expect("kq")
        .test(LSHIFT, F3).expect("0.00008 61733 33 J/(K·C)");

    // ------------------------------------------------------------------------
    step("Physics constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, F4);
    step("Imaginary unit")
        .test(CLEAR, NOSHIFT, F1).expect("ⅉ")
        .test(LSHIFT, F1).expect("0+1ⅈ");
    step("Speed of light")
        .test(CLEAR, NOSHIFT, F2).expect("c")
        .test(LSHIFT, F2).expect("299 792 458 m/s");
    step("Gravitational constant")
        .test(CLEAR, NOSHIFT, F3).expect("G")
        .test(LSHIFT, F3).expect("6.6743⁳⁻¹¹ m↑3/(s↑2·kg)");
    step("Earth gravity")
        .test(CLEAR, NOSHIFT, F4).expect("g")
        .test(LSHIFT, F4).expect("9.80665 m/s↑2");
    step("Acceleration of Earth gravity Earth")
        .test(CLEAR, NOSHIFT, F5).expect("Z₀")
        .test(LSHIFT, F5).expect("376.73031 3412 Ω");
    step("Vaccuum permittivity")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("ε₀")
        .test(LSHIFT, F1).expect("8.85418 78188⁳⁻¹² F/m");
    step("Vaccuum permeability")
        .test(CLEAR, NOSHIFT, F2).expect("μ₀")
        .test(LSHIFT, F2).expect("0.00000 12566 37 H/m");
    step("Coulomb constant")
        .test(CLEAR, NOSHIFT, F3).expect("ke")
        .test(LSHIFT, F3).expect("8.98755 17862⁳⁹ N·m↑2/C↑2");

    // ------------------------------------------------------------------------
    step("Mass constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, F5);
    step("Electron mass")
        .test(CLEAR, NOSHIFT, F1).expect("me")
        .test(LSHIFT, F1).expect("9.10938 37139⁳⁻³¹ kg");
    step("Neutron mass")
        .test(CLEAR, NOSHIFT, F2).expect("mn")
        .test(LSHIFT, F2).expect("1.67492 75005 6⁳⁻²⁷ kg");
    step("Proton mass")
        .test(CLEAR, NOSHIFT, F3).expect("mp")
        .test(LSHIFT, F3).expect("1.67262 19259 5⁳⁻²⁷ kg");
    step("Hydrogen mass")
        .test(CLEAR, NOSHIFT, F4).expect("mH")
        .test(LSHIFT, F4).expect("1.00782 50322 3 u");
    step("Mass unit")
        .test(CLEAR, NOSHIFT, F5).expect("u")
        .test(LSHIFT, F5).expect("1.66053 90689 2⁳⁻²⁷ kg");
    step("Deuterium mass")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("mD")
        .test(LSHIFT, F1).expect("2.01410 17781 2 u");
    step("Tritium mass")
        .test(CLEAR, NOSHIFT, F2).expect("mT")
        .test(LSHIFT, F2).expect("3.01604 92779 u");
    step("Helium mass")
        .test(CLEAR, NOSHIFT, F3).expect("mHe")
        .test(LSHIFT, F3).expect("4.00260 32541 3 u");
    step("Muon mass")
        .test(CLEAR, NOSHIFT, F4).expect("mμ")
        .test(LSHIFT, F4).expect("0.11342 89257 u");
    step("Tau mass")
        .test(CLEAR, NOSHIFT, F5).expect("mτ")
        .test(LSHIFT, F5).expect("1.90754 u");
    step("Proton / electron mass ratio")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("mpme")
        .test(LSHIFT, F1).expect("1 836.15267 343");
    step("Electron relative atomic mass")
        .test(CLEAR, NOSHIFT, F2).expect("Are")
        .test(LSHIFT, F2).expect("0.00054 85799 09");


    // ------------------------------------------------------------------------
    step("Size constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F1);
    step("Classical electron radius")
        .test(CLEAR, NOSHIFT, F1).expect("re")
        .test(LSHIFT, F1).expect("2.81794 03204 6 fm");
    step("Proton charge radius")
        .test(CLEAR, NOSHIFT, F2).expect("rp")
        .test(LSHIFT, F2).expect("8.4075");
    step("Bohr radius")
        .test(CLEAR, NOSHIFT, F3).expect("a0")
        .test(LSHIFT, F3).expect("0.05291 77210 54 nm");
    step("Thomson cross-section")
        .test(CLEAR, NOSHIFT, F4).expect("σe")
        .test(LSHIFT, F4).expect("6.65245 87051⁳⁻²⁹ m↑2");

    // ------------------------------------------------------------------------
    step("Scattering constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F2);
    step("Electron Compton wavelength")
        .test(CLEAR, NOSHIFT, F1).expect("λc")
        .test(LSHIFT, F1).expect("0.00242 63102 35 nm");
    step("Proton Compton wavelength")
        .test(CLEAR, NOSHIFT, F2).expect("λcp")
        .test(LSHIFT, F2).expect("0.00000 13214 1 nm");
    step("Neutron Compton wavelength")
        .test(CLEAR, NOSHIFT, F3).expect("λcn")
        .test(LSHIFT, F3).expect("0.00000 13195 91 nm");
    step("Muon Compton wavelength")
        .test(CLEAR, NOSHIFT, F4).expect("λcμ")
        .test(LSHIFT, F4).expect("0.00001 17344 41 nm");
    step("Tau Compton wavelength")
        .test(CLEAR, NOSHIFT, F5).expect("λcτ")
        .test(LSHIFT, F5).expect("0.00000 06977 7 nm");


    // ------------------------------------------------------------------------
    step("Quantum constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F3);
    step("Planck")
        .test(CLEAR, NOSHIFT, F1).expect("h")
        .test(LSHIFT, F1).expect("6.62607 015⁳⁻³⁴ J·s");
    step("Dirac")
        .test(CLEAR, NOSHIFT, F2).expect("ℏ")
        .test(LSHIFT, F2).expect("1.05457 18176 5⁳⁻³⁴ J·s");
    step("fine structure constant")
        .test(CLEAR, NOSHIFT, F3).expect("α")
        .test(LSHIFT, F3).expect("0.00729 73525 64");
    step("Cs hyperfine transition")
        .test(CLEAR, NOSHIFT, F4).expect("ΔfCs")
        .test(LSHIFT, F4).expect("9 192 631 770 Hz");
    step("Weak mixing angle")
        .test(CLEAR, NOSHIFT, F5).expect("θw")
        .test(LSHIFT, F5).expect("28.183 °");
    step("Planck length")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("Lpl")
        .test(LSHIFT, F1).expect("1.61625 5⁳⁻³⁵ m");
    step("Planck time")
        .test(CLEAR, NOSHIFT, F2).expect("Tpl")
        .test(LSHIFT, F2).expect("5.39124 6⁳⁻⁴⁴ s");
    step("Planck mass")
        .test(CLEAR, NOSHIFT, F3).expect("Mpl")
        .test(LSHIFT, F3).expect("0.00000 00217 64 kg");
    step("Planck energy")
        .test(CLEAR, NOSHIFT, F4).expect("Epl")
        .test(LSHIFT, F4).expect("1.22089⁳¹⁹ GeV");
    step("Planck temperature")
        .test(CLEAR, NOSHIFT, F5).expect("T°pl")
        .test(LSHIFT, F5).expect("1.41678 4⁳³² K");

    step("Hartree energy")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("Eh")
        .test(LSHIFT, F1).expect("4.35974 47222 1⁳⁻¹⁸ J");


    // ------------------------------------------------------------------------
    step("Magnetism constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F4);
    step("Bohr magneton")
        .test(CLEAR, NOSHIFT, F1).expect("μB")
        .test(LSHIFT, F1).expect("9.27401 00657⁳⁻²⁴ J/T");
    step("Nuclear magneton")
        .test(CLEAR, NOSHIFT, F2).expect("μN")
        .test(LSHIFT, F2).expect("5.05078 37393⁳⁻²⁷ J/T");
    step("Electron gyromagnetic ratio")
        .test(CLEAR, NOSHIFT, F3).expect("γe")
        .test(LSHIFT, F3).expect("1.76085 96278 3⁳¹¹ (s·T)⁻¹");
    step("Proton gyromagnetic ratio")
        .test(CLEAR, NOSHIFT, F4).expect("γp")
        .test(LSHIFT, F4).expect("267 522 187.08 (s·T)⁻¹");
    step("Neutron gyromagnetic ratio")
        .test(CLEAR, NOSHIFT, F5).expect("γn")
        .test(LSHIFT, F5).expect("183 247 175. (s·T)⁻¹");
    step("Rydberg")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("R∞")
        .test(LSHIFT, F1).expect("10 973 731.5682 m⁻¹");
    step("von Klitzing constant")
        .test(CLEAR, NOSHIFT, F2).expect("Rk")
        .test(LSHIFT, F2).expect("25 812.80745 93 Ω");
    step("Faraday")
        .test(CLEAR, NOSHIFT, F3).expect("F")
        .test(LSHIFT, F3).expect("96 485.33212 33 C/mol");
    step("Conductance quantum")
        .test(CLEAR, NOSHIFT, F4).expect("G0")
        .test(LSHIFT, F4).expect("0.00007 74809 17 S");
    step("Fermi reduced coupling constant")
        .test(CLEAR, NOSHIFT, F5).expect("G0F")
        .test(LSHIFT, F5).expect("0.00001 16637 87 (GeV↑2)⁻¹");

    step("First radiation constant")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("c1")
        .test(LSHIFT, F1).expect("3.74177 18521 9⁳⁻¹⁶ W·m↑2");
    step("Second radiation constant")
        .test(CLEAR, NOSHIFT, F2).expect("c2")
        .test(LSHIFT, F2).expect("0.01438 77687 75 m·K");
    step("Wien's constant")
        .test(CLEAR, NOSHIFT, F3).expect("c3")
        .test(LSHIFT, F3).expect("2.89777 19551 9 mm·K");
    step("Wien's frequency constant")
        .test(CLEAR, NOSHIFT, F4).expect("c3f")
        .test(LSHIFT, F4).expect("0.05878 92575 76 THz/K");
    step("Magnetic flux quantum")
        .test(CLEAR, NOSHIFT, F5).expect("ø")
        .test(LSHIFT, F5).expect("2.06783 38484 6⁳⁻¹⁵ Wb");
    step("Josephson constant")
        .test(NOSHIFT, F6)
        .test(CLEAR, NOSHIFT, F1).expect("KJ")
        .test(LSHIFT, F1).expect("4.83597 84841 7⁳¹⁴ Hz/V");

    step("Quantum of circulation")
        .test(CLEAR, NOSHIFT, F2).expect("Kc")
        .test(LSHIFT, F2).expect("0.00036 36947 55 m↑2/s");

    // ------------------------------------------------------------------------
    step("Material constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F5);
    step(" ε₀q ratio")
        .test(CLEAR, NOSHIFT, F1).expect("ε₀q")
        .test(LSHIFT, F1).expect("55 263 493.618 F/(m·C)");
    step(" qε₀ product")
        .test(CLEAR, NOSHIFT, F2).expect("qε₀")
        .test(LSHIFT, F2).expect("1.41859 72836 3⁳⁻³⁰ F·C/m");
    step("Dielectric constant")
        .test(CLEAR, NOSHIFT, F3).expect("εsi")
        .test(LSHIFT, F3).expect("11.9");
    step("SiO2 dielectric constant")
        .test(CLEAR, NOSHIFT, F4).expect("εox")
        .test(LSHIFT, F4).expect("3.9");
    step("Sound reference intensity")
        .test(CLEAR, NOSHIFT, F5).expect("I₀")
        .test(LSHIFT, F5).expect("1.⁳⁻¹² W/m↑2");


    // ------------------------------------------------------------------------
    step("Computing constants")
    // ------------------------------------------------------------------------
        .test(CLEAR, LSHIFT, I, LSHIFT, F6);
    step("No constant")
        .test(CLEAR, NOSHIFT, F1).expect("No")
        .test(LSHIFT, F1).expect("False");
    step("Yes constant")
        .test(CLEAR, NOSHIFT, F2).expect("Yes")
        .test(LSHIFT, F2).expect("True");
    step("Unix epoch constant")
        .test(CLEAR, NOSHIFT, F3).expect("UnixEpoch")
        .test(LSHIFT, F3).expect("Thu 1/Jan/1970");
    step("Sinclair ZX81 RAM size")
        .test(CLEAR, NOSHIFT, F4).expect("SinclairZX81RAM")
        .test(LSHIFT, F4).expect("1 KiB");
    step("Page size")
        .test(CLEAR, NOSHIFT, F5).expect("PageSize")
        .test(LSHIFT, F5).expect("4 KiB");
    step("Hello World constant")
        .test(NOSHIFT, F6).expect("HelloWorld")
        .test(LSHIFT, F6).expect("\"Hello World\"");
}


void tests::character_menu()
// ----------------------------------------------------------------------------
//   Character menu and character catalog
// ----------------------------------------------------------------------------
{
    BEGIN(characters);

    step("Character menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .image_menus("char-menu", 3);

    step("RPL menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F2, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5, F6,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5, RSHIFT, F6,
              ENTER)
        .expect("\"→⇄Σ∏∆_⁳°′″ⒸⒺⓁ|?\"");
    step("Arith menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F3, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5, F6,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5, RSHIFT, F6,
              ENTER)
        .expect("\"+-*/×÷<=>≤≠≥·%^↑\\±\"");
    step("Math menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F4, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"Σ∏∆∂∫πℼ′″°ⅈⅉℂℚℝ\"");
    step("French menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F1, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"àéèêôÀÉÈÊÔëîïûü\"");
    step("Punct menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F5, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5, F6,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5, RSHIFT, F6,
              ENTER)
        .expect("\".,;:!?#$%&'\"\"¡¿`´~\\\"");
    step("Delim menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(F6, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5, F6,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              ENTER)
        .expect("\"()[]{}«»'\"\"¦§¨­¯\"");

    step("Arrows menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(LSHIFT, F2, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"←↑→↓↔↕⇄⇆↨⌂▲▼◀▬▶\"");
    step("Blocks menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(LSHIFT, F3, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"┌┬┐─├┼┤│└┴┘▬╒╤╕\"");
    step("Bullets menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(LSHIFT, F4, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              ENTER)
        .expect("\"·∙►▶→□▪▫▬○●◊◘◙\"");
    step("Currency menu")
        .test(CLEAR, ID_CharactersMenu).noerror()
        .test(LSHIFT, F5, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT,F3,
              LSHIFT, F4, LSHIFT, F5, LSHIFT, F6,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3,
              ENTER)
        .expect("\"$€¢£¤₣₤₧₫₭₹₺₽ƒ\"");
    step("Greek menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(LSHIFT, F1, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"αβγδεΑΒΓΔΕάΆϵΈέ\"");
    step("Europe menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(LSHIFT, F6, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5,
              LSHIFT, F1,
              LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"ÀÁÂÃÄàáâãäÅÆÇåæ\"");

    step("Cyrillic menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F1, RSHIFT, ENTER)
        .test(NOSHIFT,
              F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              ENTER)
        .expect("\"АБВГДабвгд\"");
    step("Picto menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F2, RSHIFT, ENTER)
        .test(NOSHIFT,
              F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"⌂№℡™⚙☺☻☼♀♂♠♣♥♦◊\"");
    step("Music menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F3, RSHIFT, ENTER)
        .test(NOSHIFT, F1, F2, F3, F4, F5, F6, ENTER)
        .expect("\"♩♪♫♭♮♯\"");
    step("Num-like menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F4, RSHIFT, ENTER)
        .test(NOSHIFT,
              F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"⁰¹²³⁴₀₁₂₃₄ⅠⅡⅢⅣⅤ\"");
    step("Ltr-like menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F5, RSHIFT, ENTER)
        .test(NOSHIFT,
              F1, F2, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\"$&@¢©¥ℂ℅ℊℎℏℓ№ℚℝ\"");
    step("All menu")
        .test(CLEAR, ID_CharactersMenu)
        .noerror()
        .test(RSHIFT, F6, RSHIFT, ENTER)
        .test(NOSHIFT,
              F1, F2, F3, F3, F4, F5,
              LSHIFT, F1, LSHIFT, F2, LSHIFT, F3, LSHIFT, F4, LSHIFT, F5,
              RSHIFT, F1, RSHIFT, F2, RSHIFT, F3, RSHIFT, F4, RSHIFT, F5,
              ENTER)
        .expect("\" !\"\"#$%&'()*+,-.\"");

    step("Catalog")
        .test(CLEAR, RSHIFT, ENTER, ADD, A, F4)
        .editor("\"À\"")
        .test(F2)
        .editor("\"A\"")
        .test(LSHIFT, F3)
        .editor("\"a\"");
    step("Générons un peu de français")
        .test(CLEAR, RSHIFT, ENTER, ADD,
              "Ge", F5, "ne", F5, "rons un peu de franc", F4, "ais")
        .editor("\"Générons un peu de français\"")
        .test(ENTER)
        .expect("\"Générons un peu de français\"");
}


void tests::statistics()
// ----------------------------------------------------------------------------
//   Statistics commands
// ----------------------------------------------------------------------------
{
    BEGIN(statistics);

    step("Clear statistics")
        .test(CLEAR, ID_StatisticsMenu, ID_ClearData).noerror()
        .test(CLEAR, ID_DataSize).expect("0")
        .test(CLEAR, ID_RecallData).expect("[ ]");

    step("1-variable add data");
    for (uint i = 0; i < 10; i++)
        test(i * i + 3 * i + 21, ID_AddData);
    step("1-variable size")
        .test(CLEAR, ID_DataSize).expect("10");
    step("1-variable total")
        .test(CLEAR, ID_DataTotal).expect("630");
    step("1-variable average")
        .test(CLEAR, ID_Average).expect("63");
    step("1-variable minimum")
        .test(CLEAR, ID_MinData).expect("21");
    step("1-variable maximum")
        .test(CLEAR, ID_MaxData).expect("129");
    step("1-variable median")
        .test(CLEAR, ID_Median).expect("55");
    step("1-variable standard deviation")
        .test(CLEAR, ID_StandardDeviation).expect("37.13040 08417");
    step("1-variable LinearRegression")
        .test(CLEAR, ID_LinearRegression)
        .error("Invalid ΣParameters").clear_error();
    step("1-variable RclΣ")
        .test(CLEAR, "RclΣ", ENTER)
        .want("[[ 21 ] [ 25 ] [ 31 ] [ 39 ] [ 49 ]"
              " [ 61 ] [ 75 ] [ 91 ] [ 109 ] [ 129 ]]")
        .clear();
    char buffer[80];
    step("1-variable remove data");
    for (uint j = 0; j < 10; j++)
    {
        uint i = 9-j;
        snprintf(buffer, sizeof(buffer), "[ %u ]", i * i + 3 * i + 21);
        test(CLEAR, ID_RemoveData).expect(buffer);
    }
    test(CLEAR, ID_RemoveData).error("Invalid ΣData").clear_error();
    step("2-variables StoΣ")
        .test(CLEAR, "[1 2 3 4] StoΣ", ENTER).noerror()
        .test(CLEAR, "AVG", ENTER).expect("2 ¹/₂");

    step("Clear statistics for 2-variable tests")
        .test(CLEAR, ID_StatisticsMenu, ID_ClearData).noerror()
        .test(CLEAR, ID_DataSize).expect("0")
        .test(CLEAR, ID_RecallData).expect("[ ]");

    step("2-variables add data");
    for (uint i = 1; i <= 10; i++)
    {
        snprintf(buffer, sizeof(buffer),
                 "[ %u %u %u %u ] Σ+", i, 2*i+3, 2*i*i*i, 3<<i);
        test(CLEAR, DIRECT(cstring(buffer)), ENTER);
    }
    step("2-variables size")
        .test(CLEAR, ID_DataSize).expect("10");
    step("2-variables total")
        .test(CLEAR, ID_DataTotal).expect("[ 55 140 6 050 6 138 ]");
    step("2-variables average")
        .test(CLEAR, ID_Average).expect("[ 5 ¹/₂ 14 605 613 ⁴/₅ ]");
    step("2-variables minimum")
        .test(CLEAR, ID_MinData).expect("[ 1 5 2 6 ]");
    step("2-variables maximum")
        .test(CLEAR, ID_MaxData).expect("[ 10 23 2 000 3 072 ]");
    step("2-variables median")
        .test(CLEAR, ID_Median).expect("[ 5 ¹/₂ 14 341 144 ]");
    step("2-variables standard deviation")
        .test(CLEAR, ID_StandardDeviation)
        .expect("[ 3.02765 03541 6.05530 07081 9 687.45666 5301 989.69106 2908 ]");
    step("2-variables RclΣ")
        .test(CLEAR, "RclΣ", ENTER)
        .want("[[ 1 5 2 6 ]"
              " [ 2 7 16 12 ]"
              " [ 3 9 54 24 ]"
              " [ 4 11 128 48 ]"
              " [ 5 13 250 96 ]"
              " [ 6 15 432 192 ]"
              " [ 7 17 686 384 ]"
              " [ 8 19 1 024 768 ]"
              " [ 9 21 1 458 1 536 ]"
              " [ 10 23 2 000 3 072 ]]")
        .clear();
    step("2-variable LinearRegression")
        .test(CLEAR, ID_LinearRegression)
        .got("Slope:2", "Intercept:3");
    step("2-variable ΣLine")
        .test(CLEAR, ID_RegressionFormula)
        .expect("'2·x+3'");
    step("PredX")
        .test(CLEAR, "3", ID_PredictX)
        .expect("0")
        .test(CLEAR, "7", ID_PredictX)
        .expect("2");
    step("PredY")
        .test(CLEAR, "4", ID_PredictY)
        .expect("11")
        .test(CLEAR, "6", ID_PredictY)
        .expect("15");
    step("Column 3: Power law")
        .test(CLEAR, "3", ID_DependentColumn)
        .test(CLEAR, ID_BestFit, ID_RegressionFormula)
        .expect("'2.·x↑3.'");
    step("PredX")
        .test(CLEAR, "54", ID_PredictX)
        .expect("3.")
        .test(CLEAR, "16", ID_PredictX)
        .expect("2.");
    step("PredY")
        .test(CLEAR, "4", ID_PredictY)
        .expect("128.")
        .test(CLEAR, "6", ID_PredictY)
        .expect("432.");
    step("Column 4: Exponential law")
        .test(CLEAR, "4", ID_DependentColumn)
        .test(CLEAR, ID_BestFit, ID_RegressionFormula)
        .expect("'3.·exp(0.69314 71805 6·x)'");
    step("PredX")
        .test(CLEAR, "3", ID_PredictX)
        .expect("5.77078 01635 6⁳⁻²³")
        .test(CLEAR, "24", ID_PredictX)
        .expect("3.");
    step("PredY")
        .test(CLEAR, "4", ID_PredictY)
        .expect("48.")
        .test(CLEAR, "6", ID_PredictY)
        .expect("192.");
    step("Column 4 and 1 reversed: Log law")
        .test(CLEAR, "4", ID_IndependentColumn)
        .test(CLEAR, "1", ID_DependentColumn)
        .test(CLEAR, ID_BestFit, ID_RegressionFormula)
        .expect("'1.44269 50408 9·ln x+-1.58496 25007 2'");
    step("PredX")
        .test(CLEAR, "3", ID_PredictX)
        .expect("24.")
        .test(CLEAR, "7", ID_PredictX)
        .expect("384.");
    step("PredY")
        .test(CLEAR, "768", ID_PredictY)
        .expect("8.")
        .test(CLEAR, "6", ID_PredictY)
        .expect("1.");
    step("2-variables remove data");
    test("0 MantissaSpacing", ENTER); // sprintf does not add spacing ;-)
    for (uint j = 0; j < 10; j++)
    {
        uint i = 10-j;
        snprintf(buffer, sizeof(buffer),
                 "[ %u %u %u %u ]", i, 2*i+3, 2*i*i*i, 3<<i);
        test(CLEAR, ID_RemoveData).want(buffer);
    }
    test(CLEAR, ID_RemoveData).error("Invalid ΣData").clear_error();
    test("'MantissaSpacing' PURGE", ENTER);
    step("2-variables StoΣ")
        .test("[1 2 3 4] StoΣ", ENTER).noerror()
        .test(CLEAR, "AVG", ENTER).expect("2 ¹/₂");
}


void tests::probabilities()
// ----------------------------------------------------------------------------
//   Probabilities functions and probabilities menu
// ----------------------------------------------------------------------------
{
    BEGIN(probabilities);

    step("Factorial in program")
        .test(CLEAR, "37 FACT", ENTER)
        .expect("13 763 753 091 226 345 046 315 979 581 580 902 400 000 000");
    step("Factorial in program with x! spelling")
        .test(CLEAR, "37 x!", ENTER)
        .expect("13 763 753 091 226 345 046 315 979 581 580 902 400 000 000");
    step("Factorial in program using Gamma")
        .test(CLEAR, "37.2 FACT", ENTER)
        .expect("2.84300 02599 5⁳⁴³");
    step("Combinations in program, returning zero")
        .test(CLEAR, "37 42 COMB", ENTER)
        .expect("0");
    step("Combinations in program")
        .test(CLEAR, "42 37 COMB", ENTER)
        .expect("850 668");
    step("Permutations in program, returning zero")
        .test(CLEAR, "37 42 PERM", ENTER)
        .expect("0");
    step("Permutations in program")
        .test(CLEAR, "42 37 PERM", ENTER)
        .expect("11 708 384 314 607 332 487 859 521 718 704 263 082 803 "
                "200 000 000");
    step("Combination with decimal non-fractional input")
        .test(CLEAR, "42. 37. COMB", ENTER)
        .expect("850 668.");
    step("Combination with decimal fractional input")
        .test(CLEAR, "0.5 2. COMB", ENTER)
        .expect("-0.125");
    step("Combination with very large decimal values")
        .test(CLEAR, "9999. 555. COMB", ENTER)
        .expect("2.24470 86227 3⁳⁹²⁹");
    step("Combination with hardware floating-point")
        .test(CLEAR, "HFP 16 Precision", ENTER, "9999. 555. COMB", ENTER)
        .error("Argument outside domain");
    step("Combination with hardware floating-point and symbolic input")
        .test(CLEAR, "NumericalResults", ENTER, "9999. 555. COMB", ENTER)
        .error("Argument outside domain");
    step("Restore software floating point and symbolc results")
        .test(CLEAR, "SFP 24 Precision SymbolicResults", ENTER).noerror();
    step("Permutations in program with decimal input")
        .test(CLEAR, "42. 37. PERM", ENTER)
        .expect("1.17083 84314 6⁳⁴⁹");
    step("Permutations in program")
        .test(CLEAR, "6. 4. PERM", ENTER)
        .expect("360.");

    step("Factorial in menu")
        .test(CLEAR, LSHIFT, W)
        .test(CLEAR, "37", NOSHIFT, F3)
        .expect("13 763 753 091 226 345 046 315 979 581 580 902 400 000 000");
    step("Factorial in menu using Gamma")
        .test(CLEAR, "37.2", NOSHIFT, F3)
        .expect("2.84300 02599 5⁳⁴³");
    step("Combinations in menu, returning zero")
        .test(CLEAR, "37 42", NOSHIFT, F1)
        .expect("0");
    step("Combinations in menu")
        .test(CLEAR, "42 37", NOSHIFT, F1)
        .expect("850 668");
    step("Permutations in menu, returning zero")
        .test(CLEAR, "37 42", NOSHIFT, F2)
        .expect("0");
    step("Permutations in menu")
        .test(CLEAR, "42 37", NOSHIFT, F2)
        .expect(
            "11 708 384 314 607 332 487 859 521 718 704 263 082 803 200 000 00"
            "0");

    step("Symbolic combinations")
        .test(CLEAR, "n m", NOSHIFT, F1)
        .expect("'Combinations(n;m)'")
        .test(CLEAR, "n 1", NOSHIFT, F1)
        .expect("'Combinations(n;1)'")
        .test(CLEAR, "1 z", NOSHIFT, F1)
        .expect("'Combinations(1;z)'");
    step("Symbolic permutations")
        .test(CLEAR, "n m", NOSHIFT, F2)
        .expect("'Permutations(n;m)'")
        .test(CLEAR, "n 1", NOSHIFT, F2)
        .expect("'Permutations(n;1)'")
        .test(CLEAR, "1 z", NOSHIFT, F2)
        .expect("'Permutations(1;z)'");
}


void tests::sum_and_product()
// ----------------------------------------------------------------------------
//   Sum and product operations
// ----------------------------------------------------------------------------
{
    BEGIN(sumprod);

    step("Sum of integers")
        .test(CLEAR, "I 1 10 'I^3' Σ", ENTER)
        .expect("3 025");
    step("Product of integers")
        .test(CLEAR, "I 1 10 'I^3' ∏", ENTER)
        .expect("47 784 725 839 872 000 000");
    step("Sum of decimal")
        .test(CLEAR, "I 1.2 10.2 'I^3' Σ", ENTER)
        .expect("3 262.68");
    step("Product of decimal")
        .test(CLEAR, "I 1.2 10.2 'I^3' ∏", ENTER)
        .expect("2.54564 43577 3⁳²⁰");
    step("Sum of fraction")
        .test(CLEAR, "I 1/3 10/3 'I^3' Σ", ENTER)
        .expect("52 ⁴/₂₇");
    step("Product of fraction")
        .test(CLEAR, "I 1/3 10/3 'I^3' ∏", ENTER)
        .expect("41 ¹⁶² ⁹¹⁹/₅₃₁ ₄₄₁");

    step("Symbolic sum of integers")
        .test(CLEAR, "I 1 10 '(A+I)^3' Σ", ENTER)
        .expect("'(A+1)↑3+(A+2)↑3+(A+3)↑3+(A+4)↑3+(A+5)↑3"
                "+(A+6)↑3+(A+7)↑3+(A+8)↑3+(A+9)↑3+(A+10)↑3'");
    step("Symbolic product of integers")
        .test(CLEAR, "I 1 10 '(A+I)^3' ∏", ENTER)
        .expect("'(A+1)↑3·(A+2)↑3·(A+3)↑3·(A+4)↑3·(A+5)↑3"
                "·(A+6)↑3·(A+7)↑3·(A+8)↑3·(A+9)↑3·(A+10)↑3'");
    step("Symbolic sum of decimal")
        .test(CLEAR, "I 1.2 10.2 '(A+I)^3' Σ", ENTER)
        .expect("'(A+1.2)↑3+(A+2.2)↑3+(A+3.2)↑3+(A+4.2)↑3+(A+5.2)↑3"
                "+(A+6.2)↑3+(A+7.2)↑3+(A+8.2)↑3+(A+9.2)↑3+(A+10.2)↑3'");
    step("Symbolic product of decimal")
        .test(CLEAR, "I 1.2 10.2 '(A+I)^3' ∏", ENTER)
        .expect("'(A+1.2)↑3·(A+2.2)↑3·(A+3.2)↑3·(A+4.2)↑3·(A+5.2)↑3"
                "·(A+6.2)↑3·(A+7.2)↑3·(A+8.2)↑3·(A+9.2)↑3·(A+10.2)↑3'");
    step("Symbolic sum of fraction")
        .test(CLEAR, "I 1/3 10/3 '(A+I)^3' Σ", ENTER)
        .expect("'(A+¹/₃)↑3+(A+⁴/₃)↑3+(A+⁷/₃)↑3+(A+¹⁰/₃)↑3'");
    step("Symbolic product of fraction")
        .test(CLEAR, "I 1/3 10/3 '(A+I)^3' ∏", ENTER)
        .expect("'(A+¹/₃)↑3·(A+⁴/₃)↑3·(A+⁷/₃)↑3·(A+¹⁰/₃)↑3'");

    step("Empty sum").test(CLEAR, "I 10 1 'I^3' Σ", ENTER).expect("0");
    step("Empty product").test(CLEAR, "I 10 1 'I^3' ∏", ENTER).expect("1");

    step("Symbolic sum expression")
        .test(CLEAR, "I 1 N '(A+I)^3' Σ", ENTER)
        .expect("'Σ(I;1;N;(A+I)↑3)'")
        .test(CLEAR, "I N 1 '(A+I)^3' Σ", ENTER)
        .expect("'Σ(I;N;1;(A+I)↑3)'");
    step("Symbolic product expression")
        .test(CLEAR, "I 1 N '(A+I)^3' ∏", ENTER)
        .expect("'∏(I;1;N;(A+I)↑3)'")
        .test(CLEAR, "I N 1 '(A+I)^3' ∏", ENTER)
        .expect("'∏(I;N;1;(A+I)↑3)'");

    step("Parsing x! in a sum #1285")
        .test(CLEAR, "'Σ(x;0;5;(2^x)/(2*x!))'", ENTER)
        .expect("'Σ(x;0;5;2↑x÷(2·x!))'")
        .test(RUNSTOP)
        .expect("3 ¹⁹/₃₀");
}


void tests::polynomials()
// ----------------------------------------------------------------------------
//   Operations on polynomials
// ----------------------------------------------------------------------------
{
    BEGIN(poly);

    step("Display polynomial prefix on the stack")
        .test(CLEAR, "PrefixPolynomialRender", ENTER)
        .noerror();

    step("Create polynomial from an expression")
        .test(CLEAR, "'X-Y' →Poly", ENTER)
        .expect("ⓅX-Y");
    step("Create polynomial from menu")
        .test(CLEAR, "'X-Y'", ENTER, ID_ToolsMenu, F4)
        .expect("ⓅX-Y");
    step("Create polynomial using self-insert")
        .test(CLEAR, ID_PolynomialsMenu, F1, "X-Y", ENTER)
        .expect("ⓅX-Y");
    step("Reordering of polynomials")
        .test(F1, "2*X*X*X+Y*(Y+1)*3+Z*Z*Z", ENTER)
        .expect("Ⓟ2·X↑3+3·Y↑2+3·Y+Z↑3");
    step("Adding polynomials").test(ADD).expect("ⓅX+2·Y+2·X↑3+3·Y↑2+Z↑3");
    step("Cancelling out terms")
        .test(F1, KEY2, ID_multiply, "Y", NOSHIFT, ENTER, ID_subtract)
        .expect("ⓅX+2·X↑3+3·Y↑2+Z↑3");
    step("Adding an expression to a polynomial")
        .test("'Y-X'", ENTER, ID_add)
        .expect("Ⓟ2·X↑3+3·Y↑2+Z↑3+Y");
    step("Adding a polynomial to an expression")
        .test("'Y-X'", ENTER, ID_Swap, ID_add)
        .expect("Ⓟ2·Y-X+2·X↑3+3·Y↑2+Z↑3");
    step("Multiplying a polynomial by an expression")
        .test("'Y-X'", ENTER, ID_Swap, ID_multiply)
        .expect("Ⓟ2·Y↑2+2·X↑3·Y+3·Y↑3+Y·Z↑3-3·X·Y+X↑2-2·X↑4-3·X·Y↑2-X·Z↑3");
    step("Adding/subtracting expressions to cancel out terms")
        .test("'X*Y*X*X*2'", ID_subtract)
        .expect("Ⓟ2·Y↑2+3·Y↑3+Y·Z↑3-3·X·Y+X↑2-2·X↑4-3·X·Y↑2-X·Z↑3");
    step("... step 2")
        .test("'Y*Y*3*Y'", ID_subtract)
        .expect("Ⓟ2·Y↑2+Y·Z↑3-3·X·Y+X↑2-2·X↑4-3·X·Y↑2-X·Z↑3");
    step("... step 3")
        .test("'Y*3*X*Y'", ID_add)
        .expect("Ⓟ2·Y↑2+Y·Z↑3-3·X·Y+X↑2-2·X↑4-X·Z↑3");
    step("... step 4")
        .test("'Z^3*(-X-Y)'", ENTER, ID_subtract)
        .expect("Ⓟ2·Y↑2+2·Y·Z↑3-3·X·Y+X↑2-2·X↑4");
    step("... step 5")
        .test("'(Y+Y)*(Y+Z*sq(Z))'", ENTER, ID_subtract)
        .expect("Ⓟ-3·X·Y+X↑2-2·X↑4");
    step("... step 6")
        .test("'X'", ENTER, ID_sq, ENTER, ID_sq, ENTER, ID_add, ID_subtract, ID_subtract)
        .expect("Ⓟ-3·X·Y");
    step("... step 7").test("'Y*X'", ENTER, ID_add).expect("Ⓟ-2·X·Y");
    step("... step 8").test("'X*Y'", ENTER, ID_add).expect("Ⓟ-X·Y");
    step("Special case where resulting polynomial is empty")
        .test("'X*Y'", ENTER, ID_add)
        .expect("Ⓟ0");
    step("Adding back one fo the original variables")
        .test("'X'", ENTER, ID_add)
        .expect("ⓅX");
    step("Multiplication of simple polynomials")
        .test(NOSHIFT, F1, "X-Y", ENTER, NOSHIFT, F1, "X+Y", ENTER, ID_multiply)
        .expect("ⓅX↑2-Y↑2");
    step("Polynomial euclidean division")
        .test(NOSHIFT, F1, "X-Y", ENTER, ID_divide)
        .expect("ⓅX+Y");
    step("Polynomial exponentiation")
        .test("3", ID_pow)
        .expect("ⓅX↑3+3·X↑2·Y+3·X·Y↑2+Y↑3");
    step("Polynomial division with remainder")
        .test(NOSHIFT, F1, "X-Y", ENTER, ID_divide)
        .expect("ⓅX↑2+4·X·Y+7·Y↑2");
    step("Polynomial remainder").test(LSHIFT, M, "rem", ENTER).expect("Ⓟ8·Y↑3");
    step("Checking result")
        .test(ID_Swap, F1, "X-Y", ENTER, ID_multiply, ID_add)
        .expect("ⓅY↑3+X↑3+3·X↑2·Y+3·X·Y↑2");

    step("Polynomial negation")
        .test(CLEAR, "'X-2*Y'", ENTER, ID_ToolsMenu, F4)
        .expect("ⓅX-2·Y")
        .test(ID_neg)
        .expect("Ⓟ-X+2·Y");

    step("Polynomial with large exponent")
        .test(CLEAR, "'X^999999999999'", ENTER, ID_ToolsMenu, F4)
        .expect("ⓅX↑999999999999");

    step("Restore default rendering for polynomials")
        .test(CLEAR, "'PrefixPolynomialRender' purge", ENTER)
        .noerror();
}


void tests::quotient_and_remainder()
// ----------------------------------------------------------------------------
//   DIV2 operation, computing simultaneous quotient and remainder
// ----------------------------------------------------------------------------
{
    BEGIN(quorem);

    step("Integer values")
        .test(CLEAR, 355, ENTER, 113, " DIV2", ENTER)
        .expect("R:16")
        .test(BSP)
        .expect("Q:3");
    step("Big integer values")
        .test(CLEAR, "2", ENTER, "70", ID_pow, 313, " IDIV2", ENTER)
        .expect("R:11")
        .test(BSP)
        .expect("Q:3 771 858 213 154 668 701");
    step("Fractions")
        .test(CLEAR, "2/3", ENTER, "4/55", RSHIFT, W, F3)
        .expect("R:²/₁₆₅")
        .test(BSP)
        .expect("Q:9");
    step("Decimal")
        .test(CLEAR, "2.3", ENTER, "0.32", RSHIFT, W, F3)
        .expect("R:0.06")
        .test(BSP)
        .expect("Q:7.");
    step("Polynomials")
        .test(CLEAR, "'X^2+X+1'", ENTER, "'2*(X+2)'", ID_PolynomialsMenu, F6)
        .expect("R:3")
        .test(BSP)
        .expect("Q:¹/₂·X-¹/₂");
    step("Polynomials with polynomial remainder")
        .test(CLEAR, "'(X^2+X+1)^3'", ENTER,
              "'2*(X+2)^2'", ID_PolynomialsMenu, F6)
        .expect("R:-81·X-135")
        .test(BSP)
        .expect("Q:¹/₂·X↑4-¹/₂·X↑3+3·X↑2-6 ¹/₂·X+17");
    step("Polynomials with zero remainder")
        .test(CLEAR, "'(X^2+X+1)^3'", ENTER, "'(1+X^2+X)^2'", ID_PolynomialsMenu, F6)
        .expect("R:0")
        .test(BSP)
        .expect("Q:X↑2+X+1");
}


void tests::expression_operations()
// ----------------------------------------------------------------------------
//   Operations on expressions
// ----------------------------------------------------------------------------
{
    BEGIN(expr);

    step("List variables in expression with LNAME")
        .test(CLEAR, "'ABC+A+X+Foo(Z;B;A)'", ENTER)
        .expect("'ABC+A+X+Foo(Z;B;A)'")
        .test("LNAME", ENTER)
        .got("[ ABC Foo A B X Z ]", "'ABC+A+X+Foo(Z;B;A)'");
    step("... with hidden local in sum")
        .test(CLEAR, "'ABC+A+X+SUM(Z;B;A;A+B*Z)'", ENTER)
        .expect("'ABC+A+X+Σ(Z;B;A;A+B·Z)'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+X+Σ(Z;B;A;A+B·Z)'");
    step("... with local in sum and same name before")
        .test(CLEAR, "'ABC+A+X+SUM(X;B;A;A+B*X)'", ENTER)
        .expect("'ABC+A+X+Σ(X;B;A;A+B·X)'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+X+Σ(X;B;A;A+B·X)'");
    step("... with local in sum and same name after")
        .test(CLEAR, "'ABC+A+SUM(X;B;A;A+B*X)+X'", ENTER)
        .expect("'ABC+A+Σ(X;B;A;A+B·X)+X'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+Σ(X;B;A;A+B·X)+X'");
    step("... with hidden local in product")
        .test(CLEAR, "'ABC+A+X+PRODUCT(Z;B;A;A+B*Z)'", ENTER)
        .expect("'ABC+A+X+∏(Z;B;A;A+B·Z)'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+X+∏(Z;B;A;A+B·Z)'");
     step("... with local in sum and same name before")
        .test(CLEAR, "'ABC+A+X+PRODUCT(X;B;A;A+B*X)'", ENTER)
        .expect("'ABC+A+X+∏(X;B;A;A+B·X)'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+X+∏(X;B;A;A+B·X)'");
    step("... with local in sum and same name after")
        .test(CLEAR, "'ABC+A+PRODUCT(X;B;A;A+B*X)+X'", ENTER)
        .expect("'ABC+A+∏(X;B;A;A+B·X)+X'")
        .test("LNAME", ENTER)
        .got("[ ABC A B X ]", "'ABC+A+∏(X;B;A;A+B·X)+X'");

    step("List variables in integral")
        .test(CLEAR, "'ABC+∫(A;B;X+Y;X)'", ENTER)
        .expect("'ABC+∫(A;B;X+Y;X)'")
        .test("LNAME", ENTER)
        .got("[ ABC A B Y ]", "'ABC+∫(A;B;X+Y;X)'");

    step("List variables in root")
        .test(CLEAR, "'ABC+ROOT(X+A*Y;Z;X)'", ENTER)
        .expect("'ABC+Root(X+A·Y;Z;X)'")
        .test("LNAME", ENTER)
        .got("[ ABC A Y Z ]", "'ABC+Root(X+A·Y;Z;X)'");

    step("List variables in program")
        .test(CLEAR, LSHIFT, RUNSTOP, "A BD + C *", ENTER)
        .want("« A BD + C × »")
        .test("LNAME", ENTER)
        .expect("[ BD A C ]")
        .test(BSP)
        .want("« A BD + C × »");
    step("List variables in polynomial")
        .test(CLEAR, "'2*X+Y'", ENTER, NOSHIFT, A, F4)
        .expect("2·X+Y")
        .test("LNAME", ENTER)
        .expect("[ X Y ]")
        .test(BSP)
        .expect("2·X+Y");

    step("List variables in expression with XVARS")
        .test(CLEAR, "'ABC+A+X+Foo(Z;B;A)'", ENTER)
        .expect("'ABC+A+X+Foo(Z;B;A)'")
        .test("XVARS", ENTER)
        .expect("{ ABC Foo A B X Z }")
        .test(BSP, BSP)
        .error("Too few arguments");
    step("List variables in program")
        .test(CLEAR, LSHIFT, RUNSTOP, "A BD + C *", ENTER)
        .want("« A BD + C × »")
        .test("XVARS", ENTER)
        .expect("{ BD A C }")
        .test(BSP, BSP)
        .error("Too few arguments");
    step("List variables in polynomial")
        .test(CLEAR, "'2*X+Y'", ENTER, NOSHIFT, A, F4)
        .expect("2·X+Y")
        .test("XVARS", ENTER)
        .expect("{ X Y }")
        .test(BSP, BSP)
        .error("Too few arguments");

    step("Check special parsing for expressions")
        .test(CLEAR, "'sin⁻¹ x'", ENTER)
        .expect("'sin⁻¹ x'")
        .test(CLEAR, "'cos⁻¹ x'", ENTER)
        .expect("'cos⁻¹ x'")
        .test(CLEAR, "'tan⁻¹ x'", ENTER)
        .expect("'tan⁻¹ x'")
        .test(CLEAR, "'sin⁻¹(x)'", ENTER)
        .expect("'sin⁻¹ x'")
        .test(CLEAR, "'cos⁻¹(x)'", ENTER)
        .expect("'cos⁻¹ x'")
        .test(CLEAR, "'tan⁻¹(x)'", ENTER)
        .expect("'tan⁻¹ x'");
}


void tests::random_number_generation()
// ----------------------------------------------------------------------------
//   Test the generation of random numbers
// ----------------------------------------------------------------------------
{
    BEGIN(random);

    step("Set a known seed 17").test(CLEAR, "17 RandomSeed", ENTER).noerror();

    step("Clear statistics data").test(CLEAR, ID_StatisticsMenu, ID_ClearData);

    step("Generate 1000 random numbers")
        .test(CLEAR, "1 1000 START RAND Σ+ NEXT", ENTER)
        .noerror();

    step("Check statistics total")
        .test(CLEAR, ID_StatisticsMenu, ID_DataTotal)
        .expect("504.95829 7562");
    step("Check statistics mean").test(ID_Average).expect("0.50495 82975 62");
    step("Check statistics min and max")
        .test(ID_MinData).expect("0.00167 92327 04")
        .test(ID_MaxData).expect("0.99918 57116 48");

    step("Set a known seed 42.42")
        .test(CLEAR, "42.42", ID_RandomSeed)
        .noerror();

    step("Clear statistics data to try again")
        .test(CLEAR, ID_StatisticsMenu, ID_ClearData);

    step("Generate 1000 random numbers")
        .test(CLEAR, "1 1000 START RAND Σ+ NEXT", ENTER)
        .noerror();

    step("Check statistics total")
        .test(CLEAR, ID_StatisticsMenu, ID_DataTotal)
        .expect("480.84282 6204");
    step("Check statistics mean").test(ID_Average).expect("0.48084 28262 04");
    step("Check statistics min and max")
        .test(ID_MinData).expect("0.00003 26239 04")
        .test(ID_MaxData).expect("0.99965 89406 4");

    step("Set a known seed 123.456")
        .test(CLEAR, "123.456 RandomSeed", ENTER)
        .noerror();

    step("Clear statistics data to try again")
        .test(CLEAR, ID_StatisticsMenu, ID_ClearData);

    step("Generate 1000 integer random numbers")
        .test(CLEAR, "1 1000 START -1000 1000 RANDOM Σ+ NEXT", ENTER)
        .noerror();

    step("Check statistics total")
        .test(CLEAR, ID_StatisticsMenu, ID_DataTotal).expect("3 388");
    step("Check statistics mean").test(ID_Average).expect("3 ⁹⁷/₂₅₀");
    step("Check statistics min and max")
        .test(ID_MinData).expect("-1 000")
        .test(ID_MaxData).expect("998");

    step("Random graphing")
        .test(CLEAR,
              "5121968 RDZ "
              "0 2500 start "
              "{} 0 399 random R→B + 0 239 random R→B + pixon "
              "next",
              LENGTHY(2500),
              ENTER)
        .image("random-graph");
}


void tests::object_structure()
// ----------------------------------------------------------------------------
//   Extracting structure from an object
// ----------------------------------------------------------------------------
{
    BEGIN(explode);

    step("Obj→ on rectangular complex value")
        .test(CLEAR, "1ⅈ2", ENTER, ID_ObjectMenu, ID_Explode)
        .got("2", "1");
    step("Obj→ on polar complex value")
        .test(CLEAR, "1∡90", ENTER, ID_ObjectMenu, ID_Explode)
        .got("¹/₂", "1");
    step("Obj→ on unit objects")
        .test(CLEAR, "123_km/h", ENTER, ID_ObjectMenu, ID_Explode)
        .got("'km÷h'", "123");
    step("Obj→ on program")
        .test(CLEAR, LSHIFT, RUNSTOP, "A B + 5 *", ENTER, ID_ObjectMenu, ID_Explode)
        .got("5", "×", "5", "+", "B", "A");
    step("Obj→ on expression")
        .test(CLEAR, "'5*(A+B)'", ENTER, ID_ObjectMenu, ID_Explode)
        .got("5", "×", "+", "B", "A", "5");
    step("Obj→ on list")
        .test(CLEAR, "{ A B + 5 * }", ENTER, ID_ObjectMenu, ID_Explode)
        .got("5", "×", "5", "+", "B", "A");
    step("Obj→ on user-defined function call")
        .test(CLEAR, "'F(A+B;C*D;E-F)'", ENTER, ID_ObjectMenu, ID_Explode)
        .expect("1")
        .test(BSP)
        .expect("'F(A+B;C·D;E-F)'")
        .test(F4)
        .got("[ F 'A+B' 'C·D' 'E-F' ]");
    step("Obj→ on vector")
        .test(CLEAR, "[a b c d]", ENTER, ID_ObjectMenu, ID_Explode)
        .got("{ 4 }", "d", "c", "b", "a");
    step("Obj→ on matrix")
        .test(CLEAR, "[[a b][c d]]", ENTER, ID_ObjectMenu, ID_Explode)
        .got("{ 2 2 }", "d", "c", "b", "a");
    step("Obj→ on polynomial")
        .test(CLEAR, "'X-Y+3*(X+Y^2)' →Poly", ENTER)
        .expect("4·X-Y+3·Y↑2")
        .test(ID_ObjectMenu, ID_Explode)
        .expect("'4·X+-1·Y+3·Y²'")
        .test(F4)
        .got("12", "+", "×", "x²", "Y", "3", "+", "×", "Y", "-1", "×", "X", "4");
    step("Obj→ on text")
        .test(CLEAR, "\"1 2 + 3 *\"", ENTER, ID_ObjectMenu, ID_Explode)
        .got("9");
    step("Obj→ on fractions")
        .test(CLEAR, "1/2", ENTER, ID_ObjectMenu, ID_Explode)
        .got("2", "1");
    step("Obj→ on tags")
        .test(CLEAR, ":abc:1.5", ENTER, ID_ObjectMenu, ID_Explode)
        .got("\"abc\"", "1.5");
}


void tests::financial_functions()
// ----------------------------------------------------------------------------
//   Test financial functions
// ----------------------------------------------------------------------------
{
    BEGIN(finance);

    step("Show TVM Menu")
        .test(CLEAR, "TVM", ENTER)
        .image_noheader("tvm-menu");

    step("Solve for present value")
        .test(CLEAR, "-200", F2).expect("Pmt=-200")
        .test(LSHIFT, F5).expect("PV=2 361.45");
    step("Solve for payment")
        .test(CLEAR, "24", F6).expect("n=24")
        .test(LSHIFT, F2).expect("Pmt=-101.5");
    step("Solve for interest rate")
        .test(CLEAR, "120", F6).expect("n=120")
        .test("20000", F5).expect("PV=20 000")
        .test("-200", F2).expect("Pmt=-200")
        .test(LSHIFT, F1).expect("I%Yr=3.74");
    step("Amortization (payment at end)")
        .test(CLEAR, "0 AMORT", ENTER)
        .got("Balance:20 000", "Interest:0", "Principal:0")
        .test(CLEAR, "1 AMORT", ENTER)
        .got("Balance:19 862.28", "Interest:-62.28", "Principal:-137.72")
        .test(CLEAR, "119 AMORT", ENTER)
        .got("Balance:199.38", "Interest:-3 999.38", "Principal:-19 800.62")
        .test(CLEAR, "120 AMORT", ENTER)
        .got("Balance:0.", "Interest:-4 000.", "Principal:-20 000.");
    step("Amortization table (payment at end)")
        .test(CLEAR, "5 AMORTTABLE", ENTER)
        .want("[[ -62.28 -137.72 19 862.28 ]"
              " [ -61.85 -138.15 19 724.14 ]"
              " [ -61.42 -138.58 19 585.56 ]"
              " [ -60.99 -139.01 19 446.56 ]"
              " [ -60.56 -139.44 19 307.12 ]]");
    step("Amortization table with first (payment at end)")
        .test(CLEAR, "{ 5 } AMORTTABLE", ENTER)
        .want("[[ -307.12 -692.88 19 307.12 ]]");
    step("Amortization table with first and count (payment at end)")
        .test(CLEAR, "{ 5 2 } AMORTTABLE", ENTER)
        .want("[[ -307.12 -692.88 19 307.12 ]"
              " [ -60.13 -139.87 19 167.24 ]]");
    step("Amortization table with first, count and step (payment at end)")
        .test(CLEAR, "{ 5 7 2 } AMORTTABLE", ENTER)
        .want("[[ -367.24 -832.76 19 167.24 ]"
              " [ -118.94 -281.06 18 886.19 ]"
              " [ -117.19 -282.81 18 603.38 ]]");
    step("TVM equation (payment at end)")
        .test(CLEAR, "TVMEquation", ENTER)
        .expect("'100·PYr÷I%Yr·Pmt·(1-(1+I%Yr÷(100·PYr))↑(-n))"
                "+FV·(1+I%Yr÷(100·PYr))↑(-n)+PV'");

    step("Switching to payment at beginning")
        .test(CLEAR, "TVMBEG", ENTER).noerror();
    step("Solve for present value")
        .test(CLEAR, "-200", F2).expect("Pmt=-200")
        .test(LSHIFT, F5).expect("PV=20 062.28");
    step("Solve for payment")
        .test(CLEAR, "24", F6).expect("n=24")
        .test(LSHIFT, F2).expect("Pmt=-866.16");
    step("Solve for interest rate")
        .test(CLEAR, "120", F6).expect("n=120")
        .test("20000", F5).expect("PV=20 000")
        .test("-250", F2).expect("Pmt=-250")
        .test(LSHIFT, F1).expect("I%Yr=8.86");
    step("Amortization (payment at beginning)")
        .test(CLEAR, "0 AMORT", ENTER)
        .got("Balance:20 000", "Interest:0", "Principal:0")
        .test(CLEAR, "1 AMORT", ENTER)
        .got("Balance:19 750", "Interest:0", "Principal:-250")
        .test(CLEAR, "119 AMORT", ENTER)
        .got("Balance:248.17", "Interest:-9 998.17", "Principal:-19 751.83")
        .test(CLEAR, "120 AMORT", ENTER)
        .got("Balance:0.", "Interest:-10 000.", "Principal:-20 000.");
    step("Amortization table (payment at beginning)")
        .test(CLEAR, "5 AMORTTABLE", ENTER)
        .want("[[ -147.68 -102.32 19 897.68 ]"
              " [ -146.92 -103.08 19 794.6 ]"
              " [ -146.16 -103.84 19 690.76 ]"
              " [ -145.39 -104.61 19 586.16 ]"
              " [ -144.62 -105.38 19 480.78 ]]");
    step("Amortization table with first (payment at beginning)")
        .test(CLEAR, "{ 5 } AMORTTABLE", ENTER)
        .want("[[ -730.78 -519.22 19 480.78 ]]");
    step("Amortization table with first and count (payment at beginning)")
        .test(CLEAR, "{ 5 2 } AMORTTABLE", ENTER)
        .want("[[ -730.78 -519.22 19 480.78 ]"
              " [ -143.84 -106.16 19 374.62 ]]");
    step("Amortization table with first, count and step (payment at beginning)")
        .test(CLEAR, "{ 5 7 2 } AMORTTABLE", ENTER)
        .want("[[ -874.62 -625.38 19 374.62 ]"
              " [ -285.33 -214.67 19 159.95 ]"
              " [ -282.15 -217.85 18 942.1 ]]");
    step("TVM equation (payment at beginning)")
        .test(CLEAR, "TVMEquation", ENTER)
        .expect("'(1+I%Yr÷(100·PYr))·(100·PYr)÷I%Yr·Pmt·"
                "(1-(1+I%Yr÷(100·PYr))↑(-n))+FV·(1+I%Yr÷(100·PYr))↑(-n)+PV'");

    step("Clear payment settings")
        .test(CLEAR, "'TVMBEG' PURGE", ENTER).noerror();

    step("TVM equation (restored)")
        .test(CLEAR, "TVMEquation", ENTER)
        .expect("'100·PYr÷I%Yr·Pmt·(1-(1+I%Yr÷(100·PYr))↑(-n))"
                "+FV·(1+I%Yr÷(100·PYr))↑(-n)+PV'");
    step("Cleanup")
        .test(CLEAR, "{ PYr n I%Yr Pmt FV PV } PURGE", ENTER).noerror();
}


void tests::library()
// ----------------------------------------------------------------------------
//   Check the content of the content library.
// ----------------------------------------------------------------------------
{
    BEGIN(library);

    step("Clear attached libraries")
        .test(CLEAR, ID_FilesMenu, ID_Libs, ID_Detach, ID_Libs)
        .expect("{ }");

    step("Secrets: Dedicace")
        .test(CLEAR, RSHIFT, H, F1, F1)
        .expect("\"À tous ceux qui se souviennent de Maubert électronique\"");
    step("Secrets: Library help")
        .test(CLEAR, F2)
        .expect("\"To modify the library, edit the config/library.csv file\"");

    step("Check that libraries were attached")
        .test(CLEAR, ID_FilesMenu, ID_Libs)
        .expect("{ Dedicace LibraryHelp }");
    step("Detach library by number")
        .test(CLEAR, "0", ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ LibraryHelp }");

    step("Physics: Relativistic and classical kinetic energy")
        .test(CLEAR, RSHIFT, H, F2, F1)
        .image("lib-kinetic", 2000);

    step("Check that libraries were attached")
        .test(CLEAR, ID_FilesMenu, ID_Libs)
        .expect("{ LibraryHelp KineticEnergy }");
    step("Detach library by library entry")
        .test(CLEAR, "'ⓁLibraryHelp'",
              ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ KineticEnergy }");
    step("Attach library by library entry")
        .test(CLEAR, "'ⓁLibraryHelp'",
              ID_FilesMenu, ID_Attach, ID_Libs)
        .expect("{ LibraryHelp KineticEnergy }");
    step("Attach library by library ID")
        .test(CLEAR, "{ 0 5 }",
              ID_FilesMenu, ID_Attach, ID_Libs)
        .expect("{ Dedicace LibraryHelp KineticEnergy CollatzBenchmark }");
    step("Detach library by library name")
        .test(CLEAR, "\"LibraryHelp\"",
              ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ Dedicace KineticEnergy CollatzBenchmark }");
    step("Attach library by library name")
        .test(CLEAR, "{ \"LibraryHelp\" \"KineticEnergy\" }",
              ID_FilesMenu, ID_Attach, ID_Libs)
        .expect("{ Dedicace LibraryHelp KineticEnergy CollatzBenchmark }");

    step("Math: Collatz conjecture benchmark")
        .test(CLEAR, RSHIFT, H, F3, LENGTHY(5000), F1, ENTER, SWAP)
        .expect("1")
        .test(BSP)
        .match("duration:[1-9].*ms");
    step("Math: Collatz conjecture")
        .test(CLEAR, "15", LENGTHY(500), F2, ENTER, ENTER)
        .expect("1");
    step("Math: Count primes").test(CLEAR, "227", F3).expect("49");
    step("Math: Triangle equations")
        .test(CLEAR, F4)
        .image_noheader("lib-triangle");

    step("Check attached libraries")
        .test(CLEAR, ID_FilesMenu, ID_Libs)
        .expect("{ "
                "Dedicace LibraryHelp KineticEnergy "
                "CollatzBenchmark CollatzConjecture CountPrimes "
                "TriangleEquations "
                "}");

    step("Detach libraries")
        .test(CLEAR, "{ ⓁTriangleEquations ⓁCollatzConjecture }",
              ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ Dedicace LibraryHelp KineticEnergy CollatzBenchmark "
                "CountPrimes }");

    step("Detach libraries by number")
        .test(CLEAR, "{ 0 { 1 2 }}",
              ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ CollatzBenchmark CountPrimes }");

    step("Detach libraries by name")
        .test(CLEAR, "{ \"CollatzBenchmark\" { CountPrimes }}",
              ID_FilesMenu, ID_Detach, ID_Libs)
        .expect("{ }");


}


void tests::check_help_examples()
// ----------------------------------------------------------------------------
//   Check the help examples
// ----------------------------------------------------------------------------
{
    BEGIN(examples);

    step("Creating and entering ExamplesTest directory")
        .test(CLEAR, "'ExamplesTest' CRDIR", ENTER)
        .noerror()
        .test("ExamplesTest", ENTER)
        .noerror();
    step("Set higher significant digits")
        .test(CLEAR, "11 MinimumSignificantDigits", ENTER).noerror();
    step("Set higher rendering limit for text")
        .test(CLEAR, "2048 TextRenderingSizeLimit", ENTER).noerror();
    step("Purge plot parameters")
        .test(CLEAR, "'PPAR' PGALL", ENTER).noerror();

    step("Opening help file").test(CLEAR);
    FILE *f = fopen(HELPFILE_NAME, "r");
    if (!f)
    {
        fail();
        return;
    }
    noerror();

    uint        opencheck  = 0;
    uint        closecheck = 0;
    uint        expcheck   = 0;
    uint        failcheck  = 0;
    uint        imgcheck   = 0;
    cstring     open       = "\n```rpl\n";
    cstring     close      = "\n```\n";
    cstring     expecting  = "@ Expecting ";
    cstring     failing    = "@ Failing ";
    cstring     image      = "@ Image ";
    bool        hadcr      = false;
    bool        testing    = false;
    bool        inref      = false;
    bool        inimg      = false;
    uint        line       = 1;
    uint        tidx       = 0;
    bool        intopic    = false;
    bool        skiptest   = false;
    char        topic[80];
    std::string ubuf;
    std::string ref;

    while (true)
    {
        int ci = fgetc(f);
        if (ci == EOF)
            break;
        byte c = ci;
        ASSERT(tidx < sizeof(topic));

        if (c == '#' && (hadcr || intopic))
            intopic = true;
        else if (intopic)
            topic[tidx++] = c;
        else if (inref)
            ref += c;

        hadcr = c == '\n';
        if (hadcr)
        {
            line++;
            if (intopic)
            {
                ASSERT(tidx > 0);
                topic[tidx - 1] = 0;
                tidx            = 0;
                intopic         = false;
            }
            if (inref)
            {
                inref = false;
                ref.pop_back();
            }
        }

        if (testing && c != '`')
            ubuf += c;

        if (c == open[opencheck])
        {
            opencheck++;
            if (!open[opencheck])
            {
                opencheck = 0;
                position(HELPFILE_NAME, line);
                istep(topic).itest(CLEAR);
                testing = true;
            }
        }
        else
        {
            opencheck = (c == open[0]);
        }

        if (c == close[closecheck])
        {
            closecheck++;
            if (!close[closecheck])
            {
                closecheck = 0;
                if (testing)
                {
                    bool keep =
                        ubuf.find("@ Keep") != ubuf.npos ||
                        ubuf.find("@ Save") != ubuf.npos;
                    itest(CLEAR, EXIT, DIRECT(ubuf));
                    ubuf.clear();

                    size_t nfailures = failures.size();
                    testing          = false;
                    itest(LENGTHY(20000), ENTER).noerror();
                    if (rt.depth() && Stack.type() == object::ID_expression)
                        itest(LENGTHY(20000), RUNSTOP).noerror();
                    if (!ref.empty())
                    {
                        if (inimg)
                        {
                            std::string imgname = ref+"-test-"+(topic+1);
                            image_noheader(imgname.c_str());
                            inimg = false;
                        }
                        else
                        {
                            want(ref.c_str());
                        }
                    }
                    bool fails = failures.size() > nfailures;
                    if (fails || skiptest)
                    {
                        std::string grep = "grep -inr '^##*";
                        grep += topic;
                        grep += "$' doc";
                        passfail(!skiptest ? 0 : fails ? 1 : -1);
                        system(grep.c_str());
                        if (skiptest && fails)
                            ok = -1;
                    }
                    ref      = "";
                    skiptest = false;
                    if (!keep)
                        itest(CLEARERR, CLEAR, EXIT,
                              DIRECT("variables "
                                     "{ Foreground Background LineWidth } + "
                                     "purge"), ENTER);
                }
            }
        }
        else
        {
            closecheck = (c == close[0]);
        }

        if (c == expecting[expcheck])
        {
            expcheck++;
            if (!expecting[expcheck])
            {
                expcheck = 0;
                inref    = true;
                ref      = "";
            }
        }
        else
        {
            expcheck = (c == expecting[0]);
        }

        if (c == failing[failcheck])
        {
            failcheck++;
            if (!failing[failcheck])
            {
                failcheck = 0;
                skiptest  = true;
                inref     = true;
                ref       = "";
            }
        }
        else
        {
            failcheck = (c == failing[0]);
        }

        if (c == image[imgcheck])
        {
            imgcheck++;
            if (!image[imgcheck])
            {
                imgcheck = 0;
                inimg    = true;
                inref    = true;
                ref      = "";
            }
        }
        else
        {
            imgcheck = (c == image[0]);
        }
    }
    fclose(f);

    step("Exiting ExamplesTest directory and purging it")
        .test(CLEAR, "UPDIR", ENTER)
        .noerror()
        .test("'ExamplesTest' PURGE")
        .noerror();
    step("Restore MinimumSignificantDigits")
        .test(CLEAR, "'MinimumSignificantDigits' PURGE", ENTER);
    step("Restore TextRenderingSizeLimit")
        .test(CLEAR, "'TextRenderingSizeLimit' PURGE", ENTER);
}


void tests::regression_checks()
// ----------------------------------------------------------------------------
//   Checks for specific regressions
// ----------------------------------------------------------------------------
{
    BEGIN(regressions);

    Settings = settings();

    step("Bug 1445: sqrt for perfect squares")
        .test(CLEAR, "25 sqrt 5 -", ENTER).expect("0.")
        .test(CLEAR, "36 sqrt 6 -", ENTER).expect("0.")
        .test(CLEAR, "49 sqrt 7 -", ENTER).expect("0.");
    step("Bug 1442: STOVX and RCLVX")
        .test(CLEAR, "'Z' STOVX", ENTER).noerror()
        .test(CLEAR, "RCLVX", ENTER).expect("Z")
        .test(CLEAR, "'Ⓓ' RCL", ENTER).want("Directory { ⓧ Z }")
        .test(CLEAR, "'AbCd' STOVX", ENTER).noerror()
        .test(CLEAR, "RCLVX", ENTER).expect("AbCd")
        .test(CLEAR, "'Ⓓ' RCL", ENTER).want("Directory { ⓧ AbCd }");

    step("Bug 1439: PPar premature range checking")
        .test(CLEAR, "20 30 XRange", ENTER)
        .noerror()              // Bug was "Invalid Plot Data"
        .test("'PPAR' PGALL", ENTER);
    step("Bug 1440: Conversion of integer to decimal may lose precision")
        .test("987654321 SQ ToDecimal 987654321 2 ^ ToDecimal -", ENTER)
        .expect("0");

    step("Bug 1429: SigDig on integer value with trailing zeros")
        .test(CLEAR, "70000 SIGDIG", ENTER)
        .expect("1")
        .test(CLEAR, "710000 SIGDIG", ENTER)
        .expect("2")
        .test(CLEAR, "0 SIGDIG", ENTER)
        .expect("0")
        .test(CLEAR, "70000 TODECIMAL BYTES", ENTER)
        .expect("5");
    step("Bug 116: Rounding of gamma(7) and gamma(8)");
    test(CLEAR, "7 gamma", ENTER).expect("720.");
    test(CLEAR, "8 gamma", ENTER).expect("5 040.");

    step("Bug 168: pi no longer parses correctly");
    test(CLEAR, LSHIFT, I, F2, F1).expect("π");
    test(DOWN).editor("Ⓒπ");
    test(ENTER).expect("π");

    step("Bug 207: parsing of cos(X+pi)");
    test(CLEAR, "'COS(X+π)'", ENTER).expect("'cos(X+π)'");

    step("Bug 238: Parsing of power");
    test(CLEAR, "'X↑3'", ENTER).expect("'X↑3'");
    test(CLEAR, "'X·X↑(N-1)'", ENTER).expect("'X·X↑(N-1)'");

    step("Bug 253: Complex cos outside domain");
    test(CLEAR, "0+30000.ⅈ sin", ENTER)
        .expect("3.41528 61889 6⁳¹³⁰²⁸∡90°");
    test(CLEAR, "0+30000.ⅈ cos", ENTER)
        .expect("3.41528 61889 6⁳¹³⁰²⁸∡0°");
    test(CLEAR, "0+30000.ⅈ tan", ENTER).expect("1∡90°");

    step("Bug 272: Type error on logical operations")
        .test(CLEAR, "{ x } #2134AF AND", ENTER).error("Bad argument type")
        .test(CLEAR, "'x' #2134AF AND", ENTER).expect("'x and #21 34AF₁₆'");

    step("Bug 277: 1+i should have positive arg");
    test(CLEAR, "1+1ⅈ arg", ENTER).expect("45 °");
    test(CLEAR, "1-1ⅈ arg", ENTER).expect("-45 °");
    test(CLEAR, "1 1 atan2", ENTER).expect("45 °");
    test(CLEAR, "1+1ⅈ ToPolar", ENTER).match("1.414.*∡45°");

    step("Bug 287: arg of negative number");
    test(CLEAR, "-35 arg", ENTER).expect("180 °");

    step("Bug 288: Abusive simplification of multiplication by -1");
    test(CLEAR, "-1 3 *", ENTER).expect("-3");

    step("Bug 279: 0/0 should error out");
    test(CLEAR, "0 0 /", ENTER).error("Divide by zero");

    step("Bug 695: Putting program separators in names");
    test(CLEAR,
         LSHIFT,
         RUNSTOP, // «»
         ALPHA_RS,
         G, // «→»
         N, // «→N»
         SHIFT,
         RUNSTOP, // «→N «»»
         UP,
         BSP,
         DOWN,
         DOWN,
         UP, // «→N«»»
         N,  // «→N«N»»
         ENTER)
        .noerror()
        .type(ID_program)
        .test(RUNSTOP)
        .noerror()
        .type(ID_program)
        .want("« N »")
        .test(BSP)
        .noerror()
        .type(ID_expression)
        .expect("'→N'");

    step("Bug 822: Fraction iteration")
        .test(CLEAR, ID_FractionsMenu, 100, RSHIFT, F3, 20, RSHIFT, F4)
        .test("1968.1205", F6)
        .expect("1 968 ²⁴¹/₂ ₀₀₀")
        .test("1968.0512", F6)
        .expect("1 968 ³²/₆₂₅")
        .test(ID_ModesMenu, RSHIFT, F4); // Reset modes

    step("Bug 906: mod and rem should have spaces during editing")
        .test(CLEAR, "X Y mod", ENTER)
        .expect("'X mod Y'")
        .test(NOSHIFT, DOWN, ENTER)
        .expect("'X mod Y'")
        .test(CLEAR, "X Y rem", ENTER)
        .expect("'X rem Y'")
        .test(NOSHIFT, DOWN, ENTER)
        .expect("'X rem Y'");

    step("Bug 917: Editor works when exiting and search is active")
        .test(CLEAR, "123", ENTER, DOWN)
        .editor("123")
        .test(NOSHIFT, A, EXIT)
        .expect("123")
        .test(KEY1)
        .editor("1")
        .test(ENTER)
        .expect("1");

    step("Bug 961: Type for decimal values")
        .test(CLEAR, "123.4 TYPE", ENTER)
        .expect("0");

    step("Bug 1110: Test computation of c from epsilon0 and mu0")
        .test(CLEAR, ID_ConstantsMenu, F4, F6, F1, F2,
              ID_multiply, ID_sqrt, ID_inv, ID_ToDecimal)
        .expect("299 792 458. m/(F↑(¹/₂)·H↑(¹/₂))");

    step("Checking parsing of unary -")
        .test(CLEAR, "'-X'", ENTER).expect("'-X'")
        .test(CLEAR, "'-X^2'", ENTER).expect("'-(X↑2)'")
        .test(CLEAR, "'-(X)^2'", ENTER).expect("'-(X↑2)'")
        .test(CLEAR, "'-(((X)))^2'", ENTER).expect("'-(X↑2)'")
        .test(CLEAR, "'-(((X))^2)'", ENTER).expect("'-(X↑2)'")
        .test(CLEAR, "'-X-Y'", ENTER).expect("'-X-Y'")
        .test(CLEAR, "'-X*3-Y'", ENTER).expect("'-(X·3)-Y'")
        .test(CLEAR, "'-X^2*3-Y'", ENTER).expect("'-(X↑2·3)-Y'")
        .test(CLEAR, "'-X²'", ENTER).expect("'-X²'")
        .test(CLEAR, "'-X²-Y'", ENTER).expect("'-X²-Y'")
        .test(CLEAR, "'-X²-3*-Y'", ENTER).expect("'-X²-3·(-Y)'");
}


void tests::plotting()
// ----------------------------------------------------------------------------
//   Test the plotting functions
// ----------------------------------------------------------------------------
{
    BEGIN(plotting);

    step("Select radians");
    test(CLEAR, "RAD", ENTER).noerror();

    step("Function plot: Sine wave");
    test(CLEAR, "'3*sin(x)' FunctionPlot", LENGTHY(200), ENTER)
        .noerror()
        .image("plot-sine");
    step("Function plot: Sine wave without axes");
    test(CLEAR, "NoPlotAxes '3*sin(x)' FunctionPlot", LENGTHY(200), ENTER)
        .noerror()
        .image("plot-sine-noaxes");
    step("Function plot: Sine wave not connected no axes");
    test(CLEAR, "NoCurveFilling '3*sin(x)' FunctionPlot", LENGTHY(200), ENTER)
        .noerror()
        .image("plot-sine-noaxes-nofill");
    step("Function plot: Sine wave with axes no fill");
    test(CLEAR, "-29 CF '3*sin(x)' FunctionPlot", LENGTHY(200), ENTER)
        .noerror()
        .image("plot-sine-nofill");
    step("Function plot: Sine wave defaults");
    test(CLEAR, "-31 CF '3*sin(x)' FunctionPlot", LENGTHY(200), ENTER)
        .noerror()
        .image("plot-sine");

    step("Function plot: Equation");
    test(CLEAR,
         ALPHA, X, ENTER, ENTER, ID_sin, 3, ID_multiply,
         ID_Swap, 21, ID_multiply, ID_cos, 2, ID_multiply, ID_add, ENTER,
         ID_PlotMenu, LENGTHY(200), ID_Function)
        .noerror()
        .image("plot-eq");
    step("Function plot: Program");
    test(CLEAR,
         LSHIFT, RUNSTOP,
         ID_StackMenu, ID_Dup, ID_tan, ID_Swap,
         41, ID_multiply, ID_sin, ID_multiply, ENTER,
         ID_PlotMenu, LENGTHY(200), ID_Function)
        .noerror()
        .image("plot-pgm");
    step("Function plot: Disable curve filling");
    test(CLEAR,
         RSHIFT, UP, ENTER,
         ID_NoCurveFilling,
         ID_PlotMenu, LENGTHY(200), ID_Function)
        .noerror()
        .image("plot-nofill");
    step("Check that LastArgs gives us the previous plot")
        .test(CLEAR, LSHIFT, M)
        .want("« Duplicate tan Swap 41 × sin × »");
    step("Function plot: Disable curve filling with flag -31");
    test("-31 CF", ENTER, RSHIFT, O, LENGTHY(200), F1)
        .noerror()
        .image("plot-pgm");

    step("Polar plot: Program");
    test(CLEAR,
         LSHIFT, RUNSTOP,
         61, ID_multiply,
         ID_tan, ID_sq,
         2, ID_add,
         ENTER,
         ID_PlotMenu, LENGTHY(200), ID_Polar)
        .noerror()
        .image("polar-pgm");
    step("Polar plot: Program, no fill");
    test(CLEAR,
         ID_NoCurveFilling,
         SHIFT, RUNSTOP,
         61, ID_multiply,
         ID_tan, ID_sq, 2, ID_add, ENTER,
         ID_PlotMenu, LENGTHY(200), ID_Polar)
        .noerror()
        .image("polar-pgm-nofill");
    step("Polar plot: Program, curve filling");
    test(CLEAR,
         ID_CurveFilling,
         LSHIFT, RUNSTOP,
         61, ID_multiply,
         ID_tan, ID_sq, 2, ID_add, ENTER,
         ID_PlotMenu, LENGTHY(200), ID_Polar)
        .noerror()
        .image("polar-pgm");
    step("Polar plot: Equation");
    test(CLEAR,
         F, J, 611, MUL, ALPHA, X, NOSHIFT, DOWN,
         MUL, K, 271, MUL, ALPHA, X, NOSHIFT, DOWN,
         ADD, KEY2, DOT, KEY5, ENTER,
         RSHIFT, O, ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-eq");
    step("Polar plot: Zoom in X and Y");
    test(EXIT, "0.5 XSCALE 0.5 YSCALE", ENTER)
        .noerror()
        .test(ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-zoomxy");
    step("Polar plot: Zoom out Y");
    test(EXIT, "2 YSCALE", ENTER)
        .noerror()
        .test(ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-zoomy");
    step("Polar plot: Zoom out X");
    test(EXIT, "2 XSCALE", ENTER)
        .noerror()
        .test(ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-zoomx");
    step("Saving plot parameters")
        .test("PPAR", ENTER, NOSHIFT, M);
    step("Polar plot: Select min point with PMIN");
    test(EXIT, "-3-4ⅈ PMIN", ENTER)
        .noerror()
        .test(ENTER, RSHIFT, O, LENGTHY(200), F2)
        .noerror()
        .image("polar-pmin");

    step("Polar plot: Select max point with PMAX");
    test(EXIT, "5+6ⅈ pmax", ENTER)
        .noerror()
        .test(ENTER, RSHIFT, O, LENGTHY(200), F2)
        .noerror()
        .image("polar-pmax");
    step("Polar plot: Select X range with XRNG");
    test(EXIT, "-6 7 xrng", ENTER)
        .noerror()
        .test(ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-xrng");
    step("Polar plot: Select Y range with YRNG");
    test(EXIT, "-3 2.5 yrng", ENTER)
        .noerror()
        .test(ENTER, LENGTHY(200), F2)
        .noerror()
        .image("polar-yrng");
    step("Restoring plot parameters")
        .test(ID_Swap, "'PPAR'", ID_Sto);

    step("Parametric plot: Program");
    test(CLEAR,
         SHIFT,
         RUNSTOP,
         "'9.5*sin(31.27*X)' eval '5.5*cos(42.42*X)' eval RealToComplex",
         ENTER,
         ENTER,
         LENGTHY(200),
         F3)
        .noerror()
        .image("pplot-pgm");
    step("Parametric plot: Degrees");
    test("DEG 2 LINEWIDTH", ENTER, LENGTHY(200), F3)
        .noerror()
        .image("pplot-deg");
    step("Parametric plot: Equation");
    test(CLEAR,
         "3 LINEWIDTH 0.25 GRAY FOREGROUND "
         "'exp((0.17ⅈ5.27)*x+(1.5ⅈ8))' ParametricPlot",
         LENGTHY(200),
         ENTER)
        .noerror()
        .image("pplot-eq");

    step("Bar plot");
    test(CLEAR,
         "[[ 1 -1 ][2 -2][3 -3][4 -4][5 -6][7 -8][9 -10]]",
         LENGTHY(200), ENTER,
         33, MUL, K, 2, MUL, RSHIFT,
         O,
         LENGTHY(200),
         F5)
        .noerror()
        .image("barplot");

    step("Scatter plot");
    test(CLEAR,
         "[[ -5 -5][ -3 0][ -5 5][ 0 3][ 5 5][ 3 0][ 5 -5][ 0 -3][-5 -5]]",
         ENTER,
         "4 LineWidth ScatterPlot",
         LENGTHY(200),
         ENTER)
        .noerror()
        .image("scatterplot");

    step("Reset drawing parameters");
    test(CLEAR, DIRECT("1 LineWidth 0 GRAY Foreground 'PPAR' PGALL"), ENTER)
        .noerror();
}


void tests::plotting_all_functions()
// ----------------------------------------------------------------------------
//   Plot all real functions
// ----------------------------------------------------------------------------
{
    BEGIN(plotfns);

    step("Select radians").test(CLEAR, SHIFT, N, F2).noerror();

    step("Select 24-digit precision").test(CLEAR, SHIFT, O, 24, F6).noerror();

    step("Purge the `PlotParameters` variable")
        .test(CLEAR, "'PPAR' pgall", ENTER)
        .noerror();

    step("Select plotting menu").test(CLEAR, RSHIFT, O).noerror();

    uint dur = 1500;

#define FUNCTION(name)       \
    step("Plotting " #name); \
    test(CLEAR, "'" #name "(x)'", LENGTHY(dur), F1).image("fnplot-" #name, dur)

    FUNCTION(sqrt);
    FUNCTION(cbrt);

    FUNCTION(sin);
    FUNCTION(cos);
    FUNCTION(tan);
    FUNCTION(asin);
    FUNCTION(acos);
    FUNCTION(atan);

    step("Select degrees");
    test(CLEAR, SHIFT, N, F1).noerror();

    step("Reselect plotting menu");
    test(CLEAR, RSHIFT, O).noerror();

    FUNCTION(sinh);
    FUNCTION(cosh);
    FUNCTION(tanh);
    FUNCTION(asinh);
    FUNCTION(acosh);
    FUNCTION(atanh);

    FUNCTION(ln1p);
    FUNCTION(expm1);
    FUNCTION(ln);
    FUNCTION(log10);
    FUNCTION(log2);
    FUNCTION(exp);
    FUNCTION(exp10);
    FUNCTION(exp2);
    FUNCTION(erf);
    FUNCTION(erfc);
    FUNCTION(tgamma);
    FUNCTION(lgamma);


    FUNCTION(abs);
    FUNCTION(sign);
    FUNCTION(IntPart);
    FUNCTION(FracPart);
    FUNCTION(ceil);
    FUNCTION(floor);
    FUNCTION(inv);
    FUNCTION(neg);
    FUNCTION(sq);
    FUNCTION(cubed);
    FUNCTION(fact);

    FUNCTION(re);
    FUNCTION(im);
    FUNCTION(arg);
    FUNCTION(conj);

    FUNCTION(ToDecimal);
    FUNCTION(ToFraction);
}


void tests::graphic_commands()
// ----------------------------------------------------------------------------
//   Graphic commands
// ----------------------------------------------------------------------------
{
    BEGIN(graphics);

    step("Cleanup environment")
        .test(DIRECT("'PPAR' PGALL {} CLIP"), ENTER);

    step("Extract graphic element")
        .test(CLEAR, "123 0", ID_ObjectMenu, ID_ToGrob, EXIT)
        .image_noheader("num-grob")
        .test("{ 10#5 10#3 } { 10#40 10#240 }",
              ID_GraphicsMenu, F6, F6, ID_Extract, EXIT)
        .image_noheader("num-grob-extracted");
    step("Extract graphic element")
        .test(CLEAR, "123 0", ID_ObjectMenu, ID_ToGrob, EXIT)
        .image_noheader("num-grob")
        .test("{ 10#15 10#13 } { 10#70 10#24 }",
              ID_GraphicsMenu, F6, F6, ID_Extract, EXIT)
        .image_noheader("num-grob-extracted2");

    step("Send to LCD")
        .test("→LCD", ENTER)
        .image("tolcd");

    step("Clear LCD")
        .test(CLEAR, "ClearLCD", ENTER)
        .noerror()
        .image("cllcd")
        .test(ENTER)
        .test(CLEAR, "CLLCD 1 1 DISP CLLCD", ENTER)
        .noerror()
        .image("cllcd");

    step("Draw graphic objects")
        .test(CLEAR, DIRECT(
              "13 LineWidth { 0 0 } 5 Circle 1 LineWidth "
              "GROB 9 15 "
              "E300140015001C001400E3008000C110AA00940090004100220014102800 "
              "2 25 for i "
              "PICT OVER "
              "2.321 ⅈ * i * exp 4.44 0.08 i * + * Swap "
              "GXor "
              "PICT OVER "
              "1.123 ⅈ * i * exp 4.33 0.08 i * + * Swap "
              "GAnd "
              "PICT OVER "
              "4.12 ⅈ * i * exp 4.22 0.08 i * + * Swap "
              "GOr "
              "next"),
              ENTER)
        .noerror()
        .image("walkman")
        .test(EXIT);

    step("Fetch from LCD")
        .test(CLEAR, DIRECT(
              "13 LineWidth { 0 0 } 5 Circle 1 LineWidth "
              "PICT { 10#125 10#44 }"
              "GROB 9 15 "
              "E300140015001C001400E3008000C110AA00940090004100220014102800 "
              "GXOR LCD→"), ENTER)
        .image("lcdsource")
        .test(EXIT, ENTER)
#ifdef CONFIG_COLOR
        .type(ID_pixmap)
        .expect("Large pixmap (192006 bytes)")
#else // CONFIG_COLOR
        .type(ID_bitmap)
        .expect("Large bitmap (12006 bytes)")
#endif // CONFIG_COLOR
        .test("{ 10#20 10#40 } { 10#380 10#123 } SUB", ENTER)
        .image_noheader("fromlcd")
        .test(EXIT);

    step("Displaying text, compatibility mode");
    test(CLEAR,
         DIRECT("\"Hello World\" 1 DISP "
                "\"Compatibility mode\" 2 DISP"),
         ENTER)
        .noerror()
        .image("text-compat")
        .test(ENTER);

    step("Displaying text, fractional row");
    test(CLEAR,
         DIRECT("\"Gutentag\" 1.5 DrawText "
                "\"Fractional row\" 3.8 DrawText"),
         ENTER)
        .noerror()
        .image("text-frac")
        .test(ENTER);

    step("Displaying text, pixel row");
    test(CLEAR,
         DIRECT("\"Bonjour tout le monde\" #5d DISP "
                "\"Pixel row mode\" #125d DISP"),
         ENTER)
        .noerror()
        .image("text-pixrow")
        .test(ENTER);

    step("Displaying text, x-y coordinates");
    test(CLEAR, DIRECT("\"Hello\" { 0 0 } DISP "), ENTER)
        .noerror()
        .image("text-xy")
        .test(ENTER);

    step("Displaying text, x-y pixel coordinates");
    test(CLEAR, DIRECT("\"Hello\" { #20d #20d } DISP"), ENTER)
        .noerror()
        .image("text-pixxy")
        .test(ENTER);

    step("Displaying text, font ID");
    test(CLEAR,
         DIRECT("\"Hello\" { 0 1 2 } DISP \"World\" { 0 -1 3 } DISP"),
         ENTER)
        .noerror()
        .image("text-font")
        .test(ENTER);

    step("Displaying text, erase and invert");
    test(CLEAR, DIRECT("\"Inverted\" { 0 0 3 true true } DISP"), ENTER)
        .noerror()
        .image("text-invert")
        .test(ENTER);

    step("Displaying text, background and foreground");
    test(CLEAR,
         DIRECT("1 Gray Background cllcd "
                "0.25 Gray Foreground 0.75 Gray Background "
                "\"Grayed\" { 0 0 } Disp"),
         ENTER)
        .noerror()
        .image("text-gray")
        .test(ENTER);

    step("Displaying text, restore background and foreground");
    test(CLEAR,
         DIRECT("0 Gray Foreground 1 Gray Background "
                "\"Grayed\" { 0 0 } Disp"),
         ENTER)
        .noerror()
        .image("text-normal")
        .test(ENTER);

    step("Displaying text, type check");
    test(CLEAR, "\"Bad\" \"Hello\" DISP", ENTER).error("Bad argument type");

    step("Displaying styled text");
    test(CLEAR,
         DIRECT("0 10 for i"
                "  \"Hello\" { }"
                "  i 135 * 321 mod 25 + R→B +"
                "  i  51 * 200 mod  3 + R→B +"
                "  i DISPXY "
                "next"),
         ENTER)
        .noerror()
        .image("text-dispxy");

    step("Lines");
    test(CLEAR,
         DIRECT("3 50 for i ⅈ i * exp i 2 + ⅈ * exp 5 * Line next"), ENTER)
        .noerror()
        .image("lines")
        .test(ENTER);

    step("Line width");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ #000 } #0 i 20 * + + "
         "{ #400 } #0 i 20 * + + "
         "i LineWidth Line "
         "next "
         "1 LineWidth"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("line-width")
        .test(ENTER);

    step("Line width, grayed");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ #000 } #0 i 20 * + + "
         "{ #400 } #0 i 20 * + + "
         "i 12 / gray foreground "
         "i LineWidth Line "
         "next "
         "1 LineWidth 0 Gray Foreground"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("line-width-gray")
        .test(ENTER);

    step("Circles");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ 0 0 } i Circle "
         "{ 0 1 } i 0.25 * Circle "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles")
        .test(ENTER);

    step("Circles, complex coordinates");
    test(CLEAR,
         DIRECT("2 150 for i "
                "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.4 0.003 i * + Circle "
                "next"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles-complex")
        .test(ENTER);

    step("Circles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0053 * gray Foreground "
         "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.1 0.008 i * +  Circle "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles-fill")
        .test(ENTER);

    step("Ellipses");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         " Ellipse "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("ellipses")
        .test(ENTER);

    step("Ellipses, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         " Ellipse "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("ellipses-fill")
        .test(ENTER);

    step("Rectangles");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         " Rect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rectangles")
        .test(ENTER);

    step("Rectangles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         " Rect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rectangle-fill")
        .test(ENTER);

    step("Rounded rectangles");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         "0.8 RRect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rounded-rectangle")
        .test(ENTER);

    step("Rounded rectangles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         "0.8 RRect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rounded-rectangle-fill")
        .test(ENTER);

    step("RGB colors")
        .test(CLEAR, DIRECT(
              "0 LINEWIDTH "
              "0 1 for r"
              "  0 1 for g"
              "   0 1 for b"
              "     r g b RGB Foreground"
              "     r g 2 + * 4 * g 27 * b 360 * + R→P 0.2 Circle"
              "   0.1 step"
              " 0.1 step "
              "0.1 step "
              "'LineWidth' PURGE "
              "1 0 0 RGB FOREGROUND \"Red\" 1 DISP "
              "0 1 0 RGB FOREGROUND \"Green\" 2 DISP "
              "0 0 1 RGB FOREGROUND \"Blue\" 3 DISP "),
              LENGTHY(5000), ENTER).noerror()
        .image("rgb-colors")
        .test(ENTER);

    step("Clipping");
    test(CLEAR, DIRECT(
         "0 LineWidth CLLCD { 120 135 353 175 } Clip "
         "2 150 for i "
         "i 0.0053 * gray Foreground "
         "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.1 0.008 i * +  Circle "
         "next "
         "{} Clip"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("clip-circles")
        .test(ENTER);

    step("Cleanup");
    test(CLEAR, DIRECT(
         "1 LineWidth 0 Gray Foreground 1 Gray Background "
         "{ -1 -1 } { 3 2 } rect"),
         ENTER)
        .noerror()
        .image("cleanup");

    step("PixOn")
        .test(CLEAR, DIRECT(
              "0 "
              "0 5000 for i"
              " 0.005 i * i 1.5 * R→P pixon "
              " 0.005 i * i 1.5 * R→P pix? 1 - neg + "
              "next"),
              LENGTHY(15000),
              ENTER)
        .image("pixon")
        .test(ENTER)
        .expect("5 001");
    step("PixOff")
        .test(CLEAR, DIRECT(
              "0 LINEWIDTH { #0 #0 } { 10#400 10#240 } rect 3 LINEWIDTH "
              "0 "
              "0 5000 for i"
              " 0.002 i * i 1.5 * R→P pixoff "
              " 0.002 i * i 1.5 * R→P pixelcolor + + + "
              "next "
              "1 LINEWIDTH"),
              LENGTHY(15000),
              ENTER)
        .image("pixoff")
        .test(ENTER)
        .expect("12 429");

    step("PixTest")
        .test(CLEAR, DIRECT(
              "CLLCD "
              "0 399 for i "
              "{ } 10#0 i + + 10#100 + "
              "if i 997.42 * sin 0 > then pixon else pixoff end "
              "next "
              "0 "
              "0 399 for i "
              "{ } 10#0 i + + 10#100 + "
              "pix? i 997.42 * sin 0 > 0 1 IFTE - 1 + +  "
              "next"),
              LENGTHY(15000),
              ENTER)
        .image("pixtest")
        .test(ENTER)
        .expect("400");

    step("Convert to graph")
        .test(CLEAR, DIRECT("'X+Y' cbrt inv 1 + sqrt dup 1 + 2 * /"),
              ENTER, EXIT)
        .image_noheader("eq-xgraph")
        .test("0 →Grob", ENTER)
        .image_noheader("eq-graph")
        .test("0 →Grob", ENTER)
        .image_noheader("eq-graph");

    step("Pattern in graph conversion")
        .test(CLEAR, DIRECT("0.85 GRAY FOREGROUND 0.15 GRAY BACKGROUND"),
              ENTER)
        .noerror()
        .test(CLEAR, DIRECT("'X+Y' cbrt inv 1 + sqrt dup 1 + 2 * /"),
              ENTER, EXIT)
        .image_noheader("pat-eq-xgraph")
        .test("2 →Grob", ENTER)
        .image_noheader("pat-eq-graph")
        .test("4 →Grob", ENTER)
        .image_noheader("pat-eq-graph");

    step("Reset pattern")
        .test(CLEAR, "0 GRAY FOREGROUND 1 GRAY BACKGROUND", ENTER)
        .noerror();

    step("GraphicAppend")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 4", ID_ToGrob,
              "DEFGH 2", ID_ToGrob,
              F6, ID_GraphicAppend, EXIT)
        .image_noheader("graph-append");
    step("GraphicStack")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 2", ID_ToGrob,
              "DEFGH 4", ID_ToGrob,
              F6, ID_GraphicStack, EXIT)
        .image_noheader("graph-stack");
    step("GraphicSubscript")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 0", ID_ToGrob,
              "DEFGH 1", ID_ToGrob,
              F6, ID_GraphicSubscript, EXIT)
        .image_noheader("graph-subscript");
    step("GraphicExponent")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 4", ID_ToGrob,
              "DEFGH 3", ID_ToGrob,
              F6, ID_GraphicExponent, EXIT)
        .image_noheader("graph-exponent");
    step("GraphicRatio")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 3", ID_ToGrob,
              "DEFGH 0", ID_ToGrob,
              F6, ID_GraphicRatio, EXIT)
        .image_noheader("graph-ratio");

    step("GraphicRoot")
        .test(CLEAR, ID_GraphicsMenu, F6, F6,
              "ABC 0", ID_ToGrob,
              F6, ID_GraphicRoot, EXIT)
        .image_noheader("graph-root");
    step("GraphicParentheses")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 2.1", ID_ToGrob,
              F6, ID_GraphicParentheses, EXIT)
        .image_noheader("graph-paren");
    step("GraphicNorm")
        .test(CLEAR, ID_GraphicsMenu, F6,
              "ABC 3.5", ID_ToGrob,
              F6, ID_GraphicNorm, EXIT)
        .image_noheader("graph-norm");

    step("GraphicSum")
        .test(CLEAR, ID_GraphicsMenu, F6, F6,
              "123", ID_GraphicSum, EXIT)
        .image_noheader("graph-sum");
    step("GraphicProduct")
        .test(CLEAR, ID_GraphicsMenu, F6, F6,
              "123", ID_GraphicProduct, EXIT)
        .image_noheader("graph-product");
    step("GraphicIntegral")
        .test(CLEAR, ID_GraphicsMenu, F6, F6,
              "123", ID_GraphicIntegral,EXIT)
        .image_noheader("graph-integral");

    step("BlankGraphic")
        .test(CLEAR, DIRECT("63 27 Blank "
                            "0.2 0.4 0.7 RGB BACKGROUND 24 32 BlankGraphic "
                            "'Background' PURGE"), ENTER)
        .image_noheader("blank-graphic");
    step("BlankBitmap")
        .test(CLEAR, DIRECT("63 27 BlankBitmap "
                            "0.2 0.4 0.7 RGB BACKGROUND 24 32 BlankBitmap "
                            "'Background' PURGE"), ENTER)
        .type(ID_bitmap)
        .image_noheader("blank-bitmap");
    step("BlankGrob")
        .test(CLEAR, DIRECT("63 27 BlankGrob "
                            "0.2 0.4 0.7 RGB BACKGROUND 24 32 BlankGrob "
                            "'Background' PURGE"), ENTER)
        .type(ID_grob)
        .image_noheader("blank-bitmap");
#if CONFIG_COLOR
    step("BlankPixmap")
        .test(CLEAR, DIRECT("63 27 BlankPixmap "
                            "0.2 0.4 0.7 RGB BACKGROUND 24 32 BlankPixmap "
                            "'Background' PURGE"), ENTER)
        .type(ID_pixmap)
        .image_noheader("blank-pixmap");
#endif // CONFIG_COLOR
}


void tests::offline_graphics()
// ----------------------------------------------------------------------------
//   Off-line graphics, i.e. graphics that are stored in a variable
// ----------------------------------------------------------------------------
{
    BEGIN(offline);

    step("Create 500x300 bitmap and store it in PICT")
        .test(CLEAR, "500 300 BLANKBITMAP PICT STO", ENTER).noerror();

    step("Send to LCD")
        .test("35 42 BLANK →LCD", ENTER)
        .image("tolcd-offline")
        .test(KEY6)
        .image("tolcd-offline2")
        .test(KEY6)
        .image("tolcd-offline3")
        .test(KEY2)
        .image("tolcd-offline4")
        .test(KEY4)
        .image("tolcd-offline5")
        .test(KEY8)
        .image("tolcd-offline6")
        .test(EXIT);

    step("Clear LCD")
        .test(CLEAR, "ClearLCD", ENTER)
        .noerror()
        .image("cllcd")
        .test(EXIT)
        .test(CLEAR, "CLLCD 1 1 DISP CLLCD", ENTER)
        .noerror()
        .image("cllcd-offline")
        .test(EXIT);

    step("Draw graphic objects")
        .test(CLEAR, DIRECT(
              "13 LineWidth { 0 0 } 5 Circle 1 LineWidth "
              "GROB 9 15 "
              "E300140015001C001400E3008000C110AA00940090004100220014102800 "
              "2 25 for i "
              "PICT OVER "
              "2.321 ⅈ * i * exp 4.44 0.08 i * + * Swap "
              "GXor "
              "PICT OVER "
              "1.123 ⅈ * i * exp 4.33 0.08 i * + * Swap "
              "GAnd "
              "PICT OVER "
              "4.12 ⅈ * i * exp 4.22 0.08 i * + * Swap "
              "GOr "
              "next"),
              ENTER)
        .noerror()
        .image("walkman-offline")
        .test(EXIT);

    step("Displaying text, compatibility mode");
    test(CLEAR,
         DIRECT("\"Hello World\" 1 DISP "
                "\"Compatibility mode\" 2 DISP"),
         ENTER)
        .noerror()
        .image("text-compat-offline")
        .test(EXIT);

    step("Displaying text, fractional row");
    test(CLEAR,
         DIRECT("\"Gutentag\" 1.5 DrawText "
                "\"Fractional row\" 3.8 DrawText"),
         ENTER)
        .noerror()
        .image("text-frac-offline")
        .test(EXIT);

    step("Displaying text, pixel row");
    test(CLEAR,
         DIRECT("\"Bonjour tout le monde\" #5d DISP "
                "\"Pixel row mode\" #125d DISP"),
         ENTER)
        .noerror()
        .image("text-pixrow-offline")
        .test(EXIT);

    step("Displaying text, x-y coordinates");
    test(CLEAR, DIRECT("\"Hello\" { 0 0 } DISP "), ENTER)
        .noerror()
        .image("text-xy-offline")
        .test(EXIT);

    step("Displaying text, x-y pixel coordinates");
    test(CLEAR, DIRECT("\"Hello\" { #20d #20d } DISP"), ENTER)
        .noerror()
        .image("text-pixxy-offline")
        .test(EXIT);

    step("Displaying text, font ID");
    test(CLEAR,
         DIRECT("\"Hello\" { 0 1 2 } DISP \"World\" { 0 -1 3 } DISP"),
         ENTER)
        .noerror()
        .image("text-font-offline")
        .test(EXIT);

    step("Displaying text, erase and invert");
    test(CLEAR, DIRECT("\"Inverted\" { 0 0 3 true true } DISP"), ENTER)
        .noerror()
        .image("text-invert-offline")
        .test(EXIT);

    step("Displaying text, background and foreground");
    test(CLEAR,
         DIRECT("1 Gray Background cllcd "
                "0.25 Gray Foreground 0.75 Gray Background "
                "\"Grayed\" { 0 0 } Disp"),
         ENTER)
        .noerror()
        .image("text-gray-offline")
        .test(EXIT);

    step("Displaying text, restore background and foreground");
    test(CLEAR,
         DIRECT("0 Gray Foreground 1 Gray Background "
                "\"Grayed\" { 0 0 } Disp"),
         ENTER)
        .noerror()
        .image("text-normal-offline")
        .test(EXIT);

    step("Displaying text, type check");
    test(CLEAR, "\"Bad\" \"Hello\" DISP", ENTER).error("Bad argument type");

    step("Displaying styled text");
    test(CLEAR,
         DIRECT("0 10 for i"
                "  \"Hello\" { }"
                "  i 135 * 321 mod 25 + R→B +"
                "  i  51 * 200 mod  3 + R→B +"
                "  i DISPXY "
                "next"),
         ENTER)
        .noerror()
        .image("text-dispxy-offline")
        .test(EXIT);

    step("Lines");
    test(CLEAR,
         DIRECT("3 50 for i ⅈ i * exp i 2 + ⅈ * exp 5 * Line next"), ENTER)
        .noerror()
        .image("lines-offline")
        .test(EXIT);

    step("Line width");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ #000 } #0 i 20 * + + "
         "{ #400 } #0 i 20 * + + "
         "i LineWidth Line "
         "next "
         "1 LineWidth"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("line-width-offline")
        .test(EXIT);

    step("Line width, grayed");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ #000 } #0 i 20 * + + "
         "{ #400 } #0 i 20 * + + "
         "i 12 / gray foreground "
         "i LineWidth Line "
         "next "
         "1 LineWidth 0 Gray Foreground"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("line-width-gray-offline")
        .test(EXIT);

    step("Circles");
    test(CLEAR, DIRECT(
         "1 11 for i "
         "{ 0 0 } i Circle "
         "{ 0 1 } i 0.25 * Circle "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles-offline")
        .test(EXIT);

    step("Circles, complex coordinates");
    test(CLEAR,
         DIRECT("2 150 for i "
                "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.4 0.003 i * + Circle "
                "next"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles-complex-offline")
        .test(EXIT);

    step("Circles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0053 * gray Foreground "
         "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.1 0.008 i * +  Circle "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("circles-fill-offline")
        .test(EXIT);

    step("Ellipses");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         " Ellipse "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("ellipses-offline")
        .test(EXIT);

    step("Ellipses, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         " Ellipse "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("ellipses-fill-offline")
        .test(EXIT);

    step("Rectangles");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         " Rect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rectangles-offline")
        .test(EXIT);

    step("Rectangles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         " Rect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rectangle-fill-offline")
        .test(EXIT);

    step("Rounded rectangles");
    test(CLEAR, DIRECT(
         "0 gray foreground 1 LineWidth "
         "2 150 for i "
         "i 0.12 * ⅈ * exp 0.05 i * 0.75 + * "
         "i 0.17 * ⅈ * exp 0.05 i * 0.75 + * "
         "0.8 RRect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rounded-rectangle-offline")
        .test(EXIT);

    step("Rounded rectangles, fill and patterns");
    test(CLEAR, DIRECT(
         "0 LineWidth "
         "2 150 for i "
         "i 0.0047 * gray Foreground "
         "0.23 ⅈ * exp 5.75 0.01 i * - * "
         "1.27 ⅈ * exp 5.45 0.01 i * - * neg "
         "0.8 RRect "
         "next "),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("rounded-rectangle-fill-offline")
        .test(EXIT);

    step("RGB colors")
        .test(CLEAR, DIRECT(
              "0 LINEWIDTH "
              "0 1 for r"
              "  0 1 for g"
              "   0 1 for b"
              "     r g b RGB Foreground"
              "     r g 2 + * 4 * g 27 * b 360 * + R→P 0.2 Circle"
              "   0.1 step"
              " 0.1 step "
              "0.1 step "
              "'LineWidth' PURGE "
              "1 0 0 RGB FOREGROUND \"Red\" 1 DISP "
              "0 1 0 RGB FOREGROUND \"Green\" 2 DISP "
              "0 0 1 RGB FOREGROUND \"Blue\" 3 DISP "),
              LENGTHY(5000), ENTER).noerror()
        .image("rgb-colors-offline")
        .test(EXIT);

    step("Clipping");
    test(CLEAR, DIRECT(
         "0 LineWidth CLLCD { 120 135 353 175 } Clip "
         "2 150 for i "
         "i 0.0053 * gray Foreground "
         "ⅈ i 0.12 * * exp 0.75 0.05 i * + * 0.1 0.008 i * +  Circle "
         "next "
         "{} Clip"),
         LENGTHY(5000),
         ENTER)
        .noerror()
        .image("clip-circles-offline")
        .test(EXIT);

    step("Cleanup");
    test(CLEAR, DIRECT(
         "1 LineWidth 0 Gray Foreground 1 Gray Background "
         "{ -1 -1 } { 3 2 } rect"),
         ENTER)
        .noerror()
        .image("cleanup-offline")
        .test(EXIT);

    step("PixOn")
        .test(CLEAR, DIRECT(
              "0 "
              "0 500 for i"
              " 0.005 i * i 1.5 * R→P pixon "
              " 0.005 i * i 1.5 * R→P pix? 1 - neg + "
              "next"),
              LENGTHY(15000), ENTER)
        .test(KEY6)
        .image("pixon-offline")
        .test(EXIT)
        .expect("501");
    step("PixOff")
        .test(CLEAR, DIRECT(
              "0 LINEWIDTH { #0 #0 } { 10#400 10#240 } rect 3 LINEWIDTH "
              "0 "
              "0 500 for i"
              " 0.002 i * i 1.5 * R→P pixoff "
              " 0.002 i * i 1.5 * R→P pixelcolor + + + "
              "next "
              "1 LINEWIDTH"),
              LENGTHY(15000), ENTER)
        .test(KEY6)
        .image("pixoff-offline")
        .test(EXIT)
        .expect("1 503");

    step("PixTest")
        .test(CLEAR, DIRECT(
              "CLLCD "
              "0 399 for i "
              "{ } 10#0 i + + 10#100 + "
              "if i 997.42 * sin 0 > then pixon else pixoff end "
              "next "
              "0 "
              "0 399 for i "
              "{ } 10#0 i + + 10#100 + "
              "pix? i 997.42 * sin 0 > 0 1 IFTE - 1 + +  "
              "next"),
              LENGTHY(15000),
              ENTER)
        .image("pixtest-offline")
        .test(EXIT)
        .expect("400");

    step("Function plot")
        .test(CLEAR,
              DIRECT("CLLCD 'sin(1000*x)*(sq(x)/12)' FunctionPlot"), ENTER)
        .image("function-offline")
        .test(EXIT);
    step("Expliti DRAX")
        .test(CLEAR, DIRECT("DRAX"), ENTER)
        .image("function+drax-offline")
        .test(EXIT);
    step("Second function superimposed")
        .test(CLEAR, DIRECT("0.8 0.4 0.2 RGB FOREGROUND 3 LINEWIDTH"
                            "'sin(150*x)' FUNCTIONPLOT"), ENTER)
        .image("two-functions-offline")
        .test(EXIT, DIRECT("{ FOREGROUND LINEWIDTH } PURGE"), ENTER).noerror();
}


void tests::user_input_commands()
// ----------------------------------------------------------------------------
//   User input commands (PROMPT, INPUT)
// ----------------------------------------------------------------------------
{
    BEGIN(input);

    step("Prompt with single-line display")
        .test(CLEAR, EXIT,
              "\"Enter value\" PROMPT 1 +",
              ENTER)
        .image("prompt-display", 2000)
        .test("123")
        .image("prompt-entry", 2000)
        .test(ENTER)
        .expect("123")
        .test(RUNSTOP)
        .expect("124");
    step("Prompt with 2 lines display")
        .test(CLEAR, EXIT, "123 456 \"Enter value\nNow!\" PROMPT 1 +", ENTER)
        .image("prompt2-display", 2000)
        .test("123")
        .image("prompt2-entry", 2000)
        .test(ENTER)
        .expect("123")
        .test(RUNSTOP)
        .got("124", "456", "123");
    step("Multiple prompts")
        .test(CLEAR, EXIT,
              "\"Enter first value\" PROMPT "
              "\"Enter second value\" PROMPT +", ENTER)
        .image("prompts-display1", 2000)
        .test("123", ENTER)
        .expect("123")
        .test(RUNSTOP, "456")
        .image("prompts-display2", 2000)
        .test(ENTER, RUNSTOP)
        .got("579");

    step("Input command with text")
        .test(CLEAR, EXIT, "\"Enter value\" \"Data\" INPUT 4 +", ENTER)
        .image("input-display", 2000)
        .test("123")
        .image("input-display-123", 2000)
        .test(ENTER)
        .got("\"Data1234\"");

    step("Input command with list")
        .test(CLEAR, EXIT, "\"Enter value\" { \"Data\" } INPUT 4 +", ENTER)
        .image("input-list-display", 2000)
        .test("123")
        .image("input-list-display-123", 2000)
        .test(ENTER)
        .got("\"Data1234\"");

    step("Input command with list and position")
        .test(CLEAR, EXIT, "\"Enter value\" { \"Data\" 3 } INPUT 4 +", ENTER)
        .image("input-pos2-display", 2000)
        .test("123")
        .image("input-pos2-display-123", 2000)
        .test(ENTER)
        .got("\"Da123ta4\"");

    step("Input command for text")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 text } INPUT 4 +", ENTER)
        .image("input-pos2-display", 20000)
        .test("123")
        .image("input-pos2-display-123", 20000)
        .test(ENTER)
        .got("\"Da123ta4\"");

    step("Input command for alpha")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 alpha } INPUT 4 +", ENTER)
        .image("input-pos2-display", 2000)
        .test("123")
        .image("input-pos2-display-123", 2000)
        .test(ENTER)
        .got("\"Da123ta4\"");

    step("Input command text")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 α } INPUT 4 +", ENTER)
        .image_nomenus("input-pos2-alpha-display", 3, 2000)
        .test("123")
        .image_nomenus("input-pos2-alpha-display-123", 3, 2000)
        .test(ENTER)
        .got("\"Da123ta4\"");

    step("Input command for alg")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 alg } INPUT 4 +", ENTER)
        .image_nomenus("input-pos2-alg-display", 2000)
        .test("123")
        .image_nomenus("input-pos2-alg-display-123", 2000)
        .test(ENTER)
        .got("\"Da123ta4\"");

    step("Input command for algebraic")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 algebraic } INPUT", ENTER)
        .test("123", ENTER)
        .type(ID_symbol)
        .got("Da123ta");

    step("Input command for expression")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"Data\" 3 expression } INPUT", ENTER)
        .test("123", ENTER)
        .type(ID_expression)
        .got("'Da123ta'");

    step("Input command with for algebraic number")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"\" 3 algebraic } INPUT", ENTER)
        .test("123+").editor("123+")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, ".4").editor("123.4")
        .test(ENTER).type(ID_decimal).got("123.4");

    step("Input command for expression")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"\" 3 expression } INPUT", ENTER)
        .test("123+").editor("123+")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, ".4").editor("123.4")
        .test(ENTER).type(ID_expression).got("'123.4'");

    step("Input command for arithmetic expression")
        .test(CLEAR, EXIT,
              "\"Enter value\" { \"\" 3 expression } INPUT", ENTER)
        .test("123+X").editor("123+X")
        .test(ENTER).type(ID_expression).got("'123+X'");

    step("Input command for single object with text")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"\"\"He\"\"\" 4 object } INPUT", ENTER)
        .test("llo\"\"", BSP, " ").editor("\"Hello\" \"")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, LSHIFT, BSP).editor("\"Hello\"")
        .test(ENTER).type(ID_text).got("\"Hello\"");

    step("Input command for single object with list")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"{ Hello } World\" 0 object } INPUT", ENTER)
        .editor("{ Hello } World")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, BSP, BSP, BSP, BSP).editor("{ Hello } ")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP).editor("{ Hello }")
        .test(ENTER).type(ID_list).got("{ Hello }");

    step("Input command for multiple objects with text")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"\"\"He\"\"\" 4 objects } INPUT", ENTER)
        .test("llo\"\"", BSP, " ").editor("\"Hello\" \"")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, LSHIFT, BSP, " World").editor("\"Hello\" World")
        .test(ENTER).type(ID_text).got("\"\"\"Hello\"\" World\"");

    step("Input command for multiple object with list")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"{ Hello } World\" 0 objects } INPUT", ENTER)
        .editor("{ Hello } World")
        .test(ENTER).type(ID_text).got("\"{ Hello } World\"");

    step("Input command for program with text")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"\"\"He\"\"\" 4 program } INPUT", ENTER)
        .test("llo\"\"", BSP, " ").editor("\"Hello\" \"")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, LSHIFT, BSP, " World").editor("\"Hello\" World")
        .test(ENTER).type(ID_program).want("« \"Hello\" World »");

    step("Input command for program with list")
        .test(CLEAR, EXIT,
              "\"Enter object\" { \"{ Hello } World\" 0 program } INPUT", ENTER)
        .editor("{ Hello } World")
        .test(ENTER).type(ID_program).want("« { Hello } World »");

    step("Input command for number with integer")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123 0 n } INPUT", ENTER)
        .editor("123")
        .test("45+", ENTER).error("Invalid input")
        .test(BSP, BSP).editor("12345")
        .test(ENTER).type(ID_integer).got("12 345");

    step("Input command for number with decimal")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123.45 0 n } INPUT", ENTER)
        .editor("123.45")
        .test("45+", ENTER).error("Invalid input")
        .test(BSP, BSP).editor("123.4545")
        .test(ENTER).type(ID_decimal).got("123.4545");

    step("Input command for integer with decimal then integer")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123.45 0 i } INPUT", ENTER)
        .editor("123.45")
        .test("45+", ENTER).error("Invalid input")
        .test(BSP, BSP).editor("123.4545")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, BSP, BSP, BSP, BSP).editor("123")
        .test(ENTER).type(ID_integer).got("123");

    step("Input command for integer with negative integer")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123 1 i } INPUT", ENTER)
        .editor("123")
        .test("-", ENTER).type(ID_neg_integer).got("-123");

    step("Input command for real with decimal")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123.45 0 r } INPUT", ENTER)
        .editor("123.45")
        .test("45+", ENTER).error("Invalid input")
        .test(BSP, BSP).editor("123.4545")
        .test(ENTER).type(ID_decimal).got("123.4545");

    step("Input command for positive with negative then positive")
        .test(CLEAR, EXIT,
              "\"Enter object\" { 123 1 positive } INPUT", ENTER)
        .editor("123")
        .test("-").editor("-123")
        .test(ENTER).error("Invalid input")
        .test(BSP, BSP, ENTER).type(ID_integer).got("123");

    step("Input command with user-defined validation")
        .test(CLEAR, EXIT,
              "\"Enter 42\" { 123 0 « if \"42\" = then 55 end » } INPUT", ENTER)
        .editor("123")
        .test(ENTER).error("Invalid input")
        .test(BSP, "42", UP, UP, BSP, BSP, BSP, ENTER)
        .type(ID_integer).got("55");
}



// ============================================================================
//
//   Sequencing tests
//
// ============================================================================

tests &tests::passfail(int ok)
// ----------------------------------------------------------------------------
//   Print a pass/fail message
// ----------------------------------------------------------------------------
{
#define GREEN  "\033[32m"
#define ORANGE "\033[43;90m"
#define RED    "\033[41;97m"
#define RESET  "\033[39;49;99;27m"
    fprintf(stderr,
            "%s\n",
            ok < 0   ? ORANGE "[TODO]" RESET
            : ok > 0 ? GREEN "[PASS]" RESET
                     : RED "[FAIL]" RESET);
#undef GREEN
#undef RED
#undef RESET
    return *this;
}

tests &tests::begin(cstring name, bool disabled)
// ----------------------------------------------------------------------------
//   Beginning of a test
// ----------------------------------------------------------------------------
{
    if (sindex)
    {
        if (ok >= 0)
            passfail(ok);
        if (ok <= 0)
            show(failures.back());
        if (ok < 0)
            failures.pop_back();
    }

    tstart = sys_current_ms();
    tname  = name;
    tindex++;
#undef BLACK
#define BLACK  "\033[40;97m"
#define GREY   "\033[100;37m"
#define CLREOL "\033[K"
#define RESET  "\033[39;49;27m"
    if (disabled)
        fprintf(stderr, GREY "%3u: %-75s" CLREOL RESET "\n", tindex, name);
    else
        fprintf(stderr, BLACK "%3u: %-75s" CLREOL RESET "\n", tindex, name);
#undef BLACK
#undef CLREOL
#undef RESET
    sindex      = 0;
    ok          = true;
    explanation = "";

    // Start with a clean state
    clear();

    return *this;
}


tests &tests::istep(cstring name)
// ----------------------------------------------------------------------------
//  Beginning of a step
// ----------------------------------------------------------------------------
{
    record(tests, "Step %+s, catching up", name);
    sname = name;
    if (sindex++)
    {
        if (ok >= 0)
            passfail(ok);
        if (ok <= 0)
            show(failures.back());
        if (ok < 0)
            failures.pop_back();
    }
    uint    spent = sys_current_ms() - tstart;
    cstring blk   = "                                                        ";
    size_t  off   = utf8_length(utf8(sname.c_str()));
    cstring pad   = blk + (off < 56 ? off : 56);
    std::string slabel = sname;
    if (off >= 56)
    {
        utf8 s = utf8(sname.c_str());
        utf8 b = s;
        for (uint i = 0; i < 56; i++)
            s = utf8_next(s);
        slabel.erase(slabel.begin() + (s - b), slabel.end());
    }
    fprintf(stderr,
            "|%3u: %03u %3u.%u: %s%s",
            tindex,
            sindex,
            spent / 1000,
            spent / 100 % 10,
            slabel.c_str(),
            pad);
    cindex = 0;
    count++;
    ok          = true;
    explanation = "";

    return *this;
}


tests &tests::position(cstring sourceFile, uint sourceLine)
// ----------------------------------------------------------------------------
//  Record the position of the current test step
// ----------------------------------------------------------------------------
{
    file = sourceFile;
    line = sourceLine;
    return *this;
}


tests &tests::check(bool valid)
// ----------------------------------------------------------------------------
//   Record if a test fails
// ----------------------------------------------------------------------------
{
    cindex++;
    if (!valid)
        fail();
    return *this;
}


tests &tests::fail()
// ----------------------------------------------------------------------------
//   Report that a test failed
// ----------------------------------------------------------------------------
{
    failures.push_back(
        failure(file, line, tname, sname, explanation, tindex, sindex, cindex));
    if (RECORDER_TWEAK(snapshots))
        image_match(sname.c_str(), 0, 0, LCD_W, LCD_H, true);
    if (dump_on_fail)
        recorder_dump_for(dump_on_fail);
    ok = false;
    return *this;
}


tests &tests::summary()
// ----------------------------------------------------------------------------
//   Summarize the test results
// ----------------------------------------------------------------------------
{
    if (sindex)
        if (ok >= 0)
            passfail(ok);

    if (failures.size())
    {
        fprintf(stderr, "Summary of %zu failures:\n", failures.size());
        std::string last;
        uint        line = 0;
        for (auto f : failures)
            show(f, last, line);
    }
    fprintf(stderr, "Ran %u tests, %zu failures\n", count, failures.size());
    return *this;
}


tests &tests::show(tests::failure &f)
// ----------------------------------------------------------------------------
//   Show a single failure
// ----------------------------------------------------------------------------
{
    std::string last;
    uint        line = 0;
    return show(f, last, line);
}


tests &tests::show(tests::failure &f, std::string &last, uint &line)
// ----------------------------------------------------------------------------
//   Show an individual failure
// ----------------------------------------------------------------------------
{
    if (f.test != last || f.line != line)
    {
        fprintf(stderr,
                "%s:%d:  Test #%u: %s\n",
                f.file,
                f.line,
                f.tindex,
                f.test.c_str());
        last = f.test;
    }
    fprintf(stderr,
            "%s:%d: %3u:%03u.%03u: %s\n",
            f.file,
            f.line,
            f.tindex,
            f.sindex,
            f.cindex,
            f.step.c_str());
    fprintf(stderr, "%s\n", f.explanation.c_str());
    return *this;
}


// ============================================================================
//
//   Utilities to build the tests
//
// ============================================================================

tests &tests::rpl_command(uint command, uint extrawait)
// ----------------------------------------------------------------------------
//   Send a command to the RPL thread and wait for it to be picked up
// ----------------------------------------------------------------------------
{
    record(tests, "RPL command %u, current is %u", command, test_command);
    if (test_command)
    {
        explain("Piling up RPL command ",
                command,
                " while command ",
                test_command,
                " is running");
        fail();
    }

    // Write the command for the RPL thread
    record(tests, "Sending RPL command %u", command);
    test_command   = command;

    // Wait for the RPL thread to have processed it
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (test_command == command && sys_current_ms() - start < wait_time)
        sys_delay(key_delay_time);

    if (test_command)
    {
        explain("RPL command ",
                command,
                " was not processed, "
                "got ",
                test_command,
                " after waiting ",
                wait_time,
                " ms");
        fail();
    }
    return *this;
}


tests &tests::keysync(uint extrawait)
// ----------------------------------------------------------------------------
//   Wait for keys to sync with the RPL thread
// ----------------------------------------------------------------------------
{
    // Wait for the RPL thread to process the keys
    record(tests, "Need to send KEYSYNC with last_key=%d", last_key);
    rpl_command(KEYSYNC, extrawait);
    return *this;
}


tests &tests::clear(uint extrawait)
// ----------------------------------------------------------------------------
//   Make sure we are in a clean state
// ----------------------------------------------------------------------------
{
    flush();
    nokeys(extrawait);
    rpl_command(CLEAR, extrawait);
    noerror(extrawait);
    return *this;
}


tests &tests::clear_error(uint extrawait)
// ----------------------------------------------------------------------------
//   Clear errors in a way that does not depend on error settings
// ----------------------------------------------------------------------------
//   Two settings can impact how we clear errors:
//   - The NeedToClearErrors setting impacts which key is actually needed
//   - Having a beep may delay how long it takes for screen refresh to show up
//   So for that reason, we send a special key to
{
    flush();
    nokeys(extrawait);
    rpl_command(CLEARERR, extrawait);
    noerror(extrawait);
    return *this;
}


tests &tests::ready(uint extrawait)
// ----------------------------------------------------------------------------
//   Check if the calculator is ready and we can look at it
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    refreshed(extrawait);
    return *this;
}


tests &tests::screen_refreshed(uint extrawait)
// ----------------------------------------------------------------------------
//    Wait until the screen was updated by the calculator
// ----------------------------------------------------------------------------
{
    record(tests,
           "Screen refreshed count=%u ui=%u",
           refresh_count,
           ui_refresh_count());
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;

    // Wait for a screen redraw
    record(tests, "Waiting for screen update");
    while (sys_current_ms() - start < wait_time &&
           ui_refresh_count() == refresh_count)
        sys_delay(refresh_delay_time);
    if (ui_refresh_count() == refresh_count)
    {
        explain("No screen refresh");
        fail();
    }
    record(tests,
           "Done checking if screen refreshed count=%u ui=%u",
           refresh_count,
           ui_refresh_count());
    return *this;
}


tests &tests::refreshed(uint extrawait)
// ----------------------------------------------------------------------------
//    Wait until the screen and stack were updated by the calculator
// ----------------------------------------------------------------------------
{
    // Wait for a stack update
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    int  key       = 0;
    bool found     = false;
    bool updated   = false;
    record(tests, "Waiting for key %d in stack at %u", last_key, start);
    while (sys_current_ms() - start < wait_time)
    {
        screen_refreshed(extrawait);

        uint available = Stack.available();
        record(tests, "Stack available = %u", available);
        if (!available)
        {
            sys_delay(refresh_delay_time);
        }
        else if (available > 1)
        {
            record(tests, "Consume extra %u stack", available);
            Stack.consume();
            updated = true;
        }
        else
        {
            key = Stack.key();
            if (key == last_key)
            {
                found = true;
                record(tests, "Consume expected stack %d", key);
                break;
            }
            else
            {
                record(tests, "Wrong key %d, expected %d", key, last_key);
                Stack.consume();
                updated = true;
            }
        }
    }
    if (!found)
    {
        if (updated)
            explain("Stack was updated but for wrong key ",
                    key,
                    " != ",
                    last_key);
        else
            explain("Stack was not updated in expected delay");
        fail();
    }

    record(tests,
           "Refreshed, key %d, needs=%u update=%u available=%u",
           Stack.key(),
           refresh_count,
           ui_refresh_count(),
           Stack.available());

    return *this;
}


tests &tests::itest(id cmd)
// ----------------------------------------------------------------------------
//   Find a key sequence that can send the given command
// ----------------------------------------------------------------------------
//   Three strategies are tried in turn to send a command:
//   1. Scan the current menus to find the command in a menu, if so use fkeys
//   2. Scan the current keymap to see if the command is anywhere
//   3. Type the name of the command
//   We start with the menu in order to be able to enter DUP from the menu,
//   because otherwise the keymap uses the ENTER key irrespective of current
//   keyboard entry state (i.e. even while entering a program)
{
    // Pass 1: Look into the current menu to see if we find it
    for (uint p = 0; p < ui.NUM_PLANES; p++)
        for (uint f = 0; f < ui.NUM_SOFTKEYS; f++)
            if (object_p asn = ui.function[p][f])
                if (id(asn->type()) == cmd)
                    return shifts(p&1, p&2, false, false)
                        .itest(tests::key(F1 + f));

    // Pass 2: Try to find it in the user interface keymap
    if (list_p keymap = ui.keymap)
    {
        uint shplane = 0;
        uint key     = 0;
        for (object_p plobj : *keymap)
        {
            if (list_p plane = plobj->as<list>())
            {
                key = 0;
                for (object_p kobj : *plane)
                {
                    key++;
                    if (id(kobj->type()) == cmd)
                    {
                        // Found in the keymap: just use it
                        shifts((shplane % 3) & 1,
                               (shplane % 3) & 2,
                               shplane / 3,
                               shplane / 3 > 1);
                        return itest(tests::key(key));
                    }
                }
            }
            shplane++;
        }
    }

    // No solution found: simply type the name
    return itest(cstring(object::name(object::id(cmd))), ENTER);
}


tests &tests::itest(tests::key k, bool release)
// ----------------------------------------------------------------------------
//   Type a given key directly
// ----------------------------------------------------------------------------
{
    extern int key_remaining();

    // Check for special key sequences
    switch (k)
    {
    case NOSHIFT:
    case LSHIFT:
    case RSHIFT:
    case ALPHA:
    case ALPHA_LS:
    case ALPHA_RS:
    case LOWERCASE:
    case LOWER_LS:
    case LOWER_RS:
        return shifts((k - NOSHIFT) & 1,
                      (k - NOSHIFT) & 2,
                      (k - NOSHIFT) & 4,
                      (k - NOSHIFT) & 8);

    case CLEAR: return clear();
    case CLEARERR: return clear_error();
    case NOKEYS: return nokeys();
    case REFRESH: return refreshed();
    case LONGPRESS:
        longpress = true; // Next key will be a long press
        return *this;

    default: break;
    }


    // Wait for the RPL thread to process the keys (to be revisited on DM42)
    while (!key_empty())
        sys_delay(key_delay_time);

    uint lcd_updates = ui_refresh_count();
    record(tests,
           "Push key %d update %u->%u last %d",
           k,
           refresh_count,
           lcd_updates,
           last_key);
    refresh_count = lcd_updates;
    Stack.catch_up();
    last_key = k;

    key_push(k);
    if (longpress)
    {
        sys_delay(600);
        longpress = false;
        release   = false;
    }
    sys_delay(key_delay_time);

    if (release && k != RELEASE)
    {
        while (!key_remaining())
            sys_delay(key_delay_time);
        record(tests,
               "Release key %d update %u->%u last %d",
               k,
               refresh_count,
               lcd_updates,
               last_key);
        refresh_count = lcd_updates;
        Stack.catch_up();
        last_key = -k;
        record(tests, "Releasing key (sending key 0)");
        key_push(RELEASE);
        keysync();
    }

    return *this;
}


tests &tests::itest(unsigned int value)
// ----------------------------------------------------------------------------
//    Test a numerical value
// ----------------------------------------------------------------------------
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%u", value);
    return itest(NOSHIFT, cstring(buffer));
}


tests &tests::itest(int value)
// ----------------------------------------------------------------------------
//   Test a signed numerical value
// ----------------------------------------------------------------------------
{
    if (value < 0)
        return itest(uint(-value), CHS);
    else
        return itest(uint(value));
}


tests &tests::itest(unsigned long value)
// ----------------------------------------------------------------------------
//    Test a numerical value
// ----------------------------------------------------------------------------
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%lu", value);
    return itest(NOSHIFT, cstring(buffer));
}


tests &tests::itest(long long value)
// ----------------------------------------------------------------------------
//   Test a signed numerical value
// ----------------------------------------------------------------------------
{
    if (value < 0)
        return itest((unsigned long long) -value, CHS);
    else
        return itest((unsigned long long) value);
}


tests &tests::itest(unsigned long long value)
// ----------------------------------------------------------------------------
//    Test a numerical value
// ----------------------------------------------------------------------------
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%llu", value);
    return itest(NOSHIFT, cstring(buffer));
}


tests &tests::itest(long value)
// ----------------------------------------------------------------------------
//   Test a signed numerical value
// ----------------------------------------------------------------------------
{
    if (value < 0)
        return itest((unsigned long long) (-value), CHS);
    else
        return itest((unsigned long long) value);
}


tests &tests::itest(char c)
// ----------------------------------------------------------------------------
//   Type the character on the calculator's keyboard
// ----------------------------------------------------------------------------
{
    const char buf[] = { c, 0 };
    return itest(buf);
}


void tests::end(unicode closer)
// ----------------------------------------------------------------------------
//   Add a terminator to the list of closing characters
// ----------------------------------------------------------------------------
{
    terminators.push_back(closer);
}


bool tests::had(unicode closer)
// ----------------------------------------------------------------------------
//   Check that we have the expected terminator, if so just skip right
// ----------------------------------------------------------------------------
{
    if (terminators.size() && terminators.back() == closer)
    {
        itest(NOSHIFT, DOWN);
        terminators.pop_back();
        return true;
    }
    return false;
}


void tests::flush()
// ----------------------------------------------------------------------------
//  Remove the terminator we did not use
// ----------------------------------------------------------------------------
{
    if (size_t sz = terminators.size())
    {
        terminators.clear();
        for (size_t i = 0; i < sz; i++)
            itest(LSHIFT, BSP);
    }
}


tests &tests::itest(cstring txt)
// ----------------------------------------------------------------------------
//   Type the string on the calculator's keyboard
// ----------------------------------------------------------------------------
{
    utf8 u = utf8(txt);
    shift(false);
    xshift(false);

    while (*u)
    {
        unicode c = utf8_codepoint(u);
        u         = utf8_next(u);

        nokeys();

        bool alpha  = ui.alpha;
        bool lower  = ui.lowercase;
        bool shift  = false;
        bool xshift = false;
        key  k      = RELEASE;
        key  fn     = RELEASE;
        bool bsp    = false;

        switch(c)
        {
        case 'A': k = A;            alpha = true; lower = false; break;
        case 'B': k = B;            alpha = true; lower = false; break;
        case 'C': k = C;            alpha = true; lower = false; break;
        case 'D': k = D;            alpha = true; lower = false; break;
        case 'E': k = E;            alpha = true; lower = false; break;
        case 'F': k = F;            alpha = true; lower = false; break;
        case 'G': k = G;            alpha = true; lower = false; break;
        case 'H': k = H;            alpha = true; lower = false; break;
        case 'I': k = I;            alpha = true; lower = false; break;
        case 'J': k = J;            alpha = true; lower = false; break;
        case 'K': k = K;            alpha = true; lower = false; break;
        case 'L': k = L;            alpha = true; lower = false; break;
        case 'M': k = M;            alpha = true; lower = false; break;
        case 'N': k = N;            alpha = true; lower = false; break;
        case 'O': k = O;            alpha = true; lower = false; break;
        case 'P': k = P;            alpha = true; lower = false; break;
        case 'Q': k = Q;            alpha = true; lower = false; break;
        case 'R': k = R;            alpha = true; lower = false; break;
        case 'S': k = S;            alpha = true; lower = false; break;
        case 'T': k = T;            alpha = true; lower = false; break;
        case 'U': k = U;            alpha = true; lower = false; break;
        case 'V': k = V;            alpha = true; lower = false; break;
        case 'W': k = W;            alpha = true; lower = false; break;
        case 'X': k = X;            alpha = true; lower = false; break;
        case 'Y': k = Y;            alpha = true; lower = false; break;
        case 'Z': k = Z;            alpha = true; lower = false; break;

        case 'a': k = A;            alpha = true; lower = true;  break;
        case 'b': k = B;            alpha = true; lower = true;  break;
        case 'c': k = C;            alpha = true; lower = true;  break;
        case 'd': k = D;            alpha = true; lower = true;  break;
        case 'e': k = E;            alpha = true; lower = true;  break;
        case 'f': k = F;            alpha = true; lower = true;  break;
        case 'g': k = G;            alpha = true; lower = true;  break;
        case 'h': k = H;            alpha = true; lower = true;  break;
        case 'i': k = I;            alpha = true; lower = true;  break;
        case 'j': k = J;            alpha = true; lower = true;  break;
        case 'k': k = K;            alpha = true; lower = true;  break;
        case 'l': k = L;            alpha = true; lower = true;  break;
        case 'm': k = M;            alpha = true; lower = true;  break;
        case 'n': k = N;            alpha = true; lower = true;  break;
        case 'o': k = O;            alpha = true; lower = true;  break;
        case 'p': k = P;            alpha = true; lower = true;  break;
        case 'q': k = Q;            alpha = true; lower = true;  break;
        case 'r': k = R;            alpha = true; lower = true;  break;
        case 's': k = S;            alpha = true; lower = true;  break;
        case 't': k = T;            alpha = true; lower = true;  break;
        case 'u': k = U;            alpha = true; lower = true;  break;
        case 'v': k = V;            alpha = true; lower = true;  break;
        case 'w': k = W;            alpha = true; lower = true;  break;
        case 'x': k = X;            alpha = true; lower = true;  break;
        case 'y': k = Y;            alpha = true; lower = true;  break;
        case 'z': k = Z;            alpha = true; lower = true;  break;

        case '0': k = KEY0;         shift = alpha; break;
        case '1': k = KEY1;         shift = alpha; break;
        case '2': k = KEY2;         shift = alpha; break;
        case '3': k = KEY3;         shift = alpha; break;
        case '4': k = KEY4;         shift = alpha; break;
        case '5': k = KEY5;         shift = alpha; break;
        case '6': k = KEY6;         shift = alpha; break;
        case '7': k = KEY7;         shift = alpha; break;
        case '8': k = KEY8;         shift = alpha; break;
        case '9': k = KEY9;         shift = alpha; break;
        case L'⁳': k = EEX;                        break;

        case '+': k = ADD;          alpha = true;  shift = true; break;
        case '-': k = SUB;          alpha = true;  shift = true; break;
        case '*': k = MUL;          alpha = true; xshift = true; break;
        case '/': k = DIV;          alpha = true; xshift = true; break;
        case '.': k = DOT;          shift = alpha; break;
        case ',': k = DOT;          alpha = true;  break;
        case ' ': k = RUNSTOP;      alpha = true;  break;
        case '?': k = KEY7;         alpha = true; xshift = true;  break;
        case '!': k = ADD;          alpha = true; xshift = true;  break;
        case '_': k = SUB;          alpha = true;  break;
        case '%': k = RCL;          alpha = true;  shift = true;  break;
        case ':': if (had(':')) continue; end(':');
                  k = KEY0;         alpha = true;                 break;
        case ';': k = KEY0;         alpha = true; xshift = true;  break;
        case '<': k = SIN;          alpha = true;  shift = true;  break;
        case '=': k = COS;          alpha = true;  shift = true;  break;
        case '>': k = TAN;          alpha = true;  shift = true;  break;
        case '^': k = INV;          alpha = true;  shift = true;  break;
        case '(': end(')');
                  k = XEQ;          alpha = true;  shift = true;  break;
        case ')': if (had(')')) continue;
                  k = XEQ;          alpha = true;  shift = true;  bsp = true; break;
        case '[': end(']');
                  k = KEY9;         xshift = alpha; shift = !alpha; break;
        case ']': if (had(']')) continue;
                  k = KEY9;         xshift = alpha; shift = !alpha;  bsp = true; break;
        case '{': end('}');
                  k = RUNSTOP;      xshift = true; break;
        case '}': if (had('}')) continue;
                  k = RUNSTOP;      xshift = true;  bsp = true; break;
        case '"': if (had('"')) continue; end('"');
                  k = ENTER;        xshift = true; break;
        case '\'':if (had('\'')) continue; end('\'');
                  k = XEQ;          xshift = alpha; break;
        case '&': k = KEY1;         alpha = true; xshift = true; break;
        case '@': k = KEY2;         alpha = true; xshift = true; break;
        case '$': k = KEY3;         alpha = true; xshift = true; break;
        case '#': k = KEY4;         alpha = true; xshift = true; break;
        case '|': k = KEY6;         alpha = true; xshift = true; break;
        case '\\': k = ADD;         alpha = true; xshift = true; break;
        case '\n': k = BSP;         alpha = true; xshift = true; break;
        case L'«': end(L'»');
                   k = RUNSTOP;     shift = true; break;
        case L'»': if (had(L'»')) continue;
                   k = RUNSTOP;     shift = true; bsp = true; break;
        case L'▶': k = STO;         alpha = true;  shift = true; break;
        case L'→': k = STO;         alpha = true; xshift = true; break;
        case L'←': k = H;           alpha = true; xshift = true; break;
        case L'·': k = MUL;         alpha = true;  shift = true; break;
        case L'×': k = MUL;         alpha = true;  shift = true; break;
        case L'÷': k = DIV;         alpha = true;  shift = true; break;
        case L'↑': k = C;           alpha = true; xshift = true; break;
        case L'↓': k = I;           alpha = true; xshift = true; break;
        case L'ⅈ': k = G; fn = F1;  alpha = false; shift = true; break;
        case L'∡': k = G; fn = F2;  alpha = false; shift = true; break;
        case L'σ': k = E;           alpha = true;  shift = true; break;
        case L'θ': k = E;           alpha = true; xshift = true; break;
        case L'π': k = I;           alpha = true;  shift = true; break;
        case L'Σ': k = A;           alpha = true;  shift = true; break;
        case L'∏': k = A;           alpha = true; xshift = true; break;
        case L'∆': k = B;           alpha = true; xshift = true; break;
        case L'∂': k = D;           alpha = true;  shift = true; break;
        case L'≤': k = J;           alpha = true; xshift = true; break;
        case L'≠': k = K;           alpha = true; xshift = true; break;
        case L'≥': k = L;           alpha = true; xshift = true; break;
        case L'√': k = C;           alpha = true;  shift = true; break;
        case L'∫': k = KEY8;        alpha = true; xshift = true; break;
        case L'…': k = SUB;         alpha = true; xshift = true; break;
        case L'±': k = N;           alpha = true;  shift = true; break;

            // Special characters that require the characters menu
#define NEXT        itest(ID_ToolsMenu); k = RESERVED2; break

        case L'à': itest(ID_CharactersMenu, F1, F1); NEXT;
        case L'À': itest(ID_CharactersMenu, F1, LSHIFT, F1); NEXT;

        case L'ℂ': itest(ID_CharactersMenu, F4, RSHIFT, F3); NEXT;
        case L'ℚ': itest(ID_CharactersMenu, F4, RSHIFT, F4); NEXT;
        case L'ℝ': itest(ID_CharactersMenu, F4, RSHIFT, F5); NEXT;
        case L'⁻': itest(ID_CharactersMenu, RSHIFT, F4, F6, F6, RSHIFT, F3); NEXT;
        case L'⁰': itest(ID_CharactersMenu, RSHIFT, F4, F1); NEXT;
        case L'¹': itest(ID_CharactersMenu, RSHIFT, F4, F2); NEXT;
        case L'²': itest(ID_CharactersMenu, RSHIFT, F4, F3); NEXT;
        case L'³': itest(ID_CharactersMenu, RSHIFT, F4, F4); NEXT;
        case L'⁴': itest(ID_CharactersMenu, RSHIFT, F4, F5); NEXT;
        case L'⁵': itest(ID_CharactersMenu, RSHIFT, F4, F6, F1); NEXT;
        case L'⁶': itest(ID_CharactersMenu, RSHIFT, F4, F6, F2); NEXT;
        case L'⁷': itest(ID_CharactersMenu, RSHIFT, F4, F6, F3); NEXT;
        case L'⁸': itest(ID_CharactersMenu, RSHIFT, F4, F6, F4); NEXT;
        case L'⁹': itest(ID_CharactersMenu, RSHIFT, F4, F6, F5); NEXT;
        case L'₀': itest(ID_CharactersMenu, RSHIFT, F4, LSHIFT, F1); NEXT;
        case L'₁': itest(ID_CharactersMenu, RSHIFT, F4, LSHIFT, F2); NEXT;
        case L'₂': itest(ID_CharactersMenu, RSHIFT, F4, LSHIFT, F3); NEXT;
        case L'₃': itest(ID_CharactersMenu, RSHIFT, F4, LSHIFT, F4); NEXT;
        case L'₄': itest(ID_CharactersMenu, RSHIFT, F4, LSHIFT, F5); NEXT;
        case L'₅': itest(ID_CharactersMenu, RSHIFT, F4, F6, LSHIFT, F1); NEXT;
        case L'₆': itest(ID_CharactersMenu, RSHIFT, F4, F6, LSHIFT, F2); NEXT;
        case L'₇': itest(ID_CharactersMenu, RSHIFT, F4, F6, LSHIFT, F3); NEXT;
        case L'₈': itest(ID_CharactersMenu, RSHIFT, F4, F6, LSHIFT, F4); NEXT;
        case L'₉': itest(ID_CharactersMenu, RSHIFT, F4, F6, LSHIFT, F5); NEXT;
        case L'∛': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, LSHIFT, F2); NEXT;
        case L'∜': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, LSHIFT, F3); NEXT;
        case L'⊿': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, F6, F5); NEXT;
        case L'∠': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, F6, F3); NEXT;
        case L'Ⓒ': itest(ID_CharactersMenu, F2, RSHIFT, F1); NEXT;
        case L'Ⓔ': itest(ID_CharactersMenu, F2, RSHIFT, F2); NEXT;
        case L'Ⓛ': itest(ID_CharactersMenu, F2, RSHIFT, F3); NEXT;
        case L'Ⓓ': itest(ID_CharactersMenu, F2, F6, F6, F1); NEXT;
        case L'ⓧ': itest(ID_CharactersMenu, F2, F6, F6, F2); NEXT;
        case L'Ⓡ': itest(ID_CharactersMenu, F2, F6, F6, F3); NEXT;
        case L'Ⓢ': itest(ID_CharactersMenu, F2, F6, F6, F4); NEXT;
        case L'°': itest(ID_CharactersMenu, F2, F6, SHIFT, F3); NEXT;
        case L'⨯': itest(ID_CharactersMenu, F4, LSHIFT, F6, LSHIFT, F1); NEXT;
        case L'⋅': itest(ID_CharactersMenu, F4, LSHIFT, F6, LSHIFT, F2); NEXT;
        case L'α': itest(ID_CharactersMenu, LSHIFT, F1, F1); NEXT;
        case L'β': itest(ID_CharactersMenu, LSHIFT, F1, F2); NEXT;
        case L'γ': itest(ID_CharactersMenu, LSHIFT, F1, F3); NEXT;
        case L'δ': itest(ID_CharactersMenu, LSHIFT, F1, F4); NEXT;
        case L'ε': itest(ID_CharactersMenu, LSHIFT, F1, F5); NEXT;
        case L'ϵ': itest(ID_CharactersMenu, LSHIFT, F1, RSHIFT, F3); NEXT;
        case L'ζ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F1); NEXT;
        case L'η': itest(ID_CharactersMenu, LSHIFT, F1, F6, F2); NEXT;
        case L'ι': itest(ID_CharactersMenu, LSHIFT, F1, F6, F4); NEXT;
        case L'κ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F5); NEXT;
        case L'λ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F1); NEXT;
        case L'μ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F2); NEXT;
        case L'ν': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F3); NEXT;
        case L'ξ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F4); NEXT;
        case L'ο': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F5); NEXT;
        case L'ρ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F2); NEXT;
        case L'τ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F4); NEXT;
        case L'υ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F5); NEXT;
        case L'φ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, F1); NEXT;
        case L'χ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, F2); NEXT;
        case L'ψ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, F3); NEXT;
        case L'ω': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, F4); NEXT;

        case L'Α': itest(ID_CharactersMenu, LSHIFT, F1, LSHIFT, F1); NEXT;
        case L'Β': itest(ID_CharactersMenu, LSHIFT, F1, LSHIFT, F2); NEXT;
        case L'Γ': itest(ID_CharactersMenu, LSHIFT, F1, LSHIFT, F3); NEXT;
        case L'Δ': itest(ID_CharactersMenu, LSHIFT, F1, LSHIFT, F4); NEXT;
        case L'Ε': itest(ID_CharactersMenu, LSHIFT, F1, LSHIFT, F5); NEXT;
        case L'Ζ': itest(ID_CharactersMenu, LSHIFT, F1, F6, LSHIFT, F1); NEXT;
        case L'Η': itest(ID_CharactersMenu, LSHIFT, F1, F6, LSHIFT, F2); NEXT;
        case L'Θ': itest(ID_CharactersMenu, LSHIFT, F1, F6, LSHIFT, F3); NEXT;
        case L'Ι': itest(ID_CharactersMenu, LSHIFT, F1, F6, LSHIFT, F4); NEXT;
        case L'Κ': itest(ID_CharactersMenu, LSHIFT, F1, F6, LSHIFT, F5); NEXT;
        case L'Λ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, LSHIFT, F1); NEXT;
        case L'Μ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, LSHIFT, F2); NEXT;
        case L'Ν': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, LSHIFT, F3); NEXT;
        case L'Ξ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, LSHIFT, F4); NEXT;
        case L'Ο': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, LSHIFT, F5); NEXT;
        case L'Π': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, LSHIFT, F1); NEXT;
        case L'Ρ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, LSHIFT, F2); NEXT;
        case L'Τ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, LSHIFT, F4); NEXT;
        case L'Υ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, LSHIFT, F5); NEXT;
        case L'Φ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, LSHIFT, F1); NEXT;
        case L'Χ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, LSHIFT, F2); NEXT;
        case L'Ψ': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, LSHIFT, F3); NEXT;
        case L'Ω': itest(ID_CharactersMenu, LSHIFT, F1, F6, F6, F6, F6, LSHIFT, F4); NEXT;
        case L'∞': itest(ID_CharactersMenu, F4, F6, F6, RSHIFT, F5); NEXT;
        case L'ℏ': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, LSHIFT, F5); NEXT;
        case L' ': itest(ID_CharactersMenu, F6, RSHIFT, F4); NEXT;
        case L'’': itest(ID_CharactersMenu, F6, RSHIFT, F5); NEXT;
        case L'█': itest(ID_CharactersMenu, LSHIFT, F3, F6, F5); NEXT;
        case L'▓': itest(ID_CharactersMenu, LSHIFT, F3, F6, F6, F6, LSHIFT, F2); NEXT;
        case L'⊕': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, F6, LSHIFT, F3); NEXT;
        case L'⊖': itest(ID_CharactersMenu, F4, F6, F6, F6, F6, F6, LSHIFT, F4); NEXT;
        case '\t': itest(ID_CharactersMenu, F6, RSHIFT, F6); NEXT;

        case L' ': continue; // Number space: just ignore
#undef NEXT
        }

        if (k == RESERVED2)
            continue;

        if (shift)
            xshift = false;
        else if (xshift)
            shift = false;

        if (k == RELEASE)
        {
            fprintf(stderr, "Cannot translate '%lc' (%d)\n", wchar_t(c), c);
        }
        else
        {
            // Reach the required shift state
            shifts(shift, xshift, alpha, lower);

            // Send the key
            itest(k);

            // If we have a pair, like (), check if we need bsp or del
            if (bsp)
                itest(BSP, DOWN);

            // If we have a follow-up key, use that
            if (fn != RELEASE)
                itest(fn);
        }
    }
    return *this;
}


tests &tests::shifts(bool lshift, bool rshift, bool alpha, bool lowercase)
// ----------------------------------------------------------------------------
//   Reach the desired shift state from the current state
// ----------------------------------------------------------------------------
{
    // Must wait for the calculator to process our keys for valid state
    nokeys();

    // Check that we have no error here
    data_entry_noerror();

    // Check invalid input: can only have one shift
    if (lshift && rshift)
        lshift = false;

    // If not alpha, disable lowercase
    if (!alpha)
        lowercase = false;

    // First change lowercase state as necessary, since this messes up shift
    while (lowercase != ui.lowercase || alpha != ui.alpha)
    {
        data_entry_noerror();
        while (!ui.shift)
            itest(SHIFT, NOKEYS);
        itest(ENTER, NOKEYS);
    }

    while (rshift != ui.xshift)
        itest(SHIFT, NOKEYS);

    while (lshift != ui.shift)
        itest(SHIFT, NOKEYS);

    return *this;
}


tests &tests::itest(tests::WAIT delay)
// ----------------------------------------------------------------------------
//   Wait for a given delay
// ----------------------------------------------------------------------------
{
    sys_delay(delay.delay);
    return *this;
}


tests &tests::itest(tests::DIRECT direct)
// ----------------------------------------------------------------------------
//   Insert some text directly into the editor
// ----------------------------------------------------------------------------
{
    nokeys(2000);
    ui.insert(utf8(direct.text.c_str()), direct.text.size(), ui.TEXT);
    return *this;
}



// ============================================================================
//
//    Test validation
//
// ============================================================================

tests &tests::nokeys(uint extrawait)
// ----------------------------------------------------------------------------
//   Check until the key buffer is empty, indicates that calculator is done
// ----------------------------------------------------------------------------
{
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time && !key_empty())
        sys_delay(refresh_delay_time);
    if (!key_empty())
    {
        explain("Unable to get an empty keyboard buffer");
        fail();
        clear_error();
    }
    return *this;
}


tests &tests::data_entry_noerror(uint extrawait)
// ----------------------------------------------------------------------------
//  During data entry, check that no error message pops up
// ----------------------------------------------------------------------------
{
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time && rt.error())
        sys_delay(refresh_delay_time);

    // Check that we are not displaying an error message
    if (rt.error())
    {
        explain("Unexpected error message [", rt.error(), "] "
                "during data entry, cleared");
        fail();
        clear_error();
    }
    return *this;
}


tests &tests::wait(uint ms)
// ----------------------------------------------------------------------------
//   Force a delay after the calculator was ready
// ----------------------------------------------------------------------------
{
    record(tests, "Waiting %u ms", ms);
    sys_delay(ms);
    return *this;
}


tests &tests::want(cstring ref, uint extrawait)
// ----------------------------------------------------------------------------
//   We want something that looks like this (ignore spacing)
// ----------------------------------------------------------------------------
{
    record(tests, "Expect [%+s] ignoring spacing", ref);
    ready(extrawait);
    cindex++;

    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        if (rt.error())
        {
            explain("Expected output [", ref, "], "
                    "got error [", rt.error(), "] instead");
            return fail();
        }

        if (cstring out = cstring(Stack.recorded()))
        {
            record(tests, "Comparing [%s] to [%+s] ignoring spaces", out, ref);
            cstring iout = out;
            cstring iref = ref;
            while (true)
            {
                if (*out == 0 && *ref == 0)
                    return *this; // Successful match

                if (isspace(*ref))
                {
                    while (*ref && isspace(*ref))
                        ref++;
                    if (!isspace(*out))
                        break;
                    while (*out && isspace(*out))
                        out++;
                }
                else
                {
                    if (*out != *ref)
                        break;
                    out++;
                    ref++;
                }
            }

            if (strcmp(ref, cstring(out)) == 0)
                return *this;
            explain("Expected output matching [", iref, "], ");
            explain("                     got [", iout, "] instead, ");
            explain("                         [", ref, "] differs from ");
            explain("                         [", out, "]");
            return fail();
        }
        sys_delay(refresh_delay_time);
    }
    record(tests, "No output");
    explain("Expected output [", ref, "] but got no stack change");
    return fail();
}


tests &tests::expect(cstring output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output at first level of stack matches the string
// ----------------------------------------------------------------------------
{
    record(tests, "Expecting [%+s]", output);
    ready(extrawait);
    cindex++;
    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        if (rt.error())
        {
            explain("Expected output [", output, "], "
                    "got error [", rt.error(), "] instead");
            return fail();
        }
        if (utf8 out = Stack.recorded())
        {
            record(tests,
                   "Comparing [%s] to [%+s] %+s",
                   out,
                   output,
                   strcmp(output, cstring(out)) == 0 ? "OK" : "FAIL");
            if (strcmp(output, cstring(out)) == 0)
                return *this;
            explain("Expected output [", output, "], ");
            explain("            got [", cstring(out), "] instead");
            return fail();
        }
        sys_delay(refresh_delay_time);
    }
    record(tests, "No output");
    explain("Expected output [", output, "] but got no stack change");
    return fail();
}


tests &tests::expect(int output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%d", output);
    return expect(num, extrawait);
}


tests &tests::expect(unsigned int output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%u", output);
    return expect(num, extrawait);
}


tests &tests::expect(long output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%ld", output);
    return expect(num, extrawait);
}


tests &tests::expect(unsigned long output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%lu", output);
    return expect(num, extrawait);
}


tests &tests::expect(long long output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%lld", output);
    return expect(num, extrawait);
}


tests &tests::expect(unsigned long long output, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%llu", output);
    return expect(num, extrawait);
}


tests &tests::match(cstring restr, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output at first level of stack matches the string
// ----------------------------------------------------------------------------
{
    ready(extrawait);
    cindex++;

    uint start     = sys_current_ms();
    uint wait_time = default_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        if (rt.error())
        {
            explain("Expected output matching [",
                    restr,
                    "], "
                    "got error [",
                    rt.error(),
                    "] instead");
            return fail();
        }

        if (utf8 out = Stack.recorded())
        {
            regex_t    re;
            regmatch_t rm;

            regcomp(&re, restr, REG_EXTENDED | REG_ICASE);
            bool ok = regexec(&re, cstring(out), 1, &rm, 0) == 0 &&
                      rm.rm_so == 0 && out[rm.rm_eo] == 0;
            regfree(&re);
            if (ok)
                return *this;
            explain("Expected output matching [", restr, "], "
                    "got [", out, "]");
            return fail();
        }
        sys_delay(refresh_delay_time);
    }
    explain("Expected output matching [", restr, "] but stack not updated");
    return fail();
}


tests &tests::image(cstring file, int x, int y, int w, int h, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the output in the screen matches what is in the file
// ----------------------------------------------------------------------------
{
    record(tests, "Image check for file %+s w=%d h=%d", file, w, h);
    nokeys(extrawait);
    screen_refreshed(extrawait);
    cindex++;

    // If it is not good, keep it on screen a bit longer
    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        if (image_match(file, x, y, w, h, false))
            return *this;
        record(tests,
               "Retry image check for file %+s after %u/%u",
               file,
               sys_current_ms() - start,
               wait_time);
        sys_delay(refresh_delay_time);
    }

    explain("Expected screen to match [", file, "]");
    image_match(file, x, y, w, h, true);
    return fail();
}


tests &tests::image(cstring name, uint extrawait)
// ----------------------------------------------------------------------------
//   Image, waiting some extra time
// ----------------------------------------------------------------------------
{
    return image(name, 0, 0, LCD_W, LCD_H, extrawait);
}


tests &tests::image_noheader(cstring name, uint ignoremenus, uint xtrawait)
// ----------------------------------------------------------------------------
//   Image, skipping the header area
// ----------------------------------------------------------------------------
{
    const int header_h = 23;
    const int menu_h   = 25 * ignoremenus;
    return image(name, 0, header_h, LCD_W, LCD_H - header_h - menu_h, xtrawait);
}


tests &tests::image_nomenus(cstring name, uint ignoremenus, uint xtrawait)
// ----------------------------------------------------------------------------
//   Image, skipping the menus area
// ----------------------------------------------------------------------------
{
    const int header_h = 0;
    const int menu_h   = 25 * ignoremenus;
    return image(name, 0, header_h, LCD_W, LCD_H - header_h - menu_h, xtrawait);
}


tests &tests::image_menus(cstring name, uint menus, uint extrawait)
// ----------------------------------------------------------------------------
//   Image, skipping the header area
// ----------------------------------------------------------------------------
{
    const int menu_h = 25 * menus;
    return image(name, 0, LCD_H - menu_h, LCD_W, menu_h, extrawait);
}


tests &tests::type(id ty, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the top of stack matches the type
// ----------------------------------------------------------------------------
{
    ready(extrawait);
    cindex++;

    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    object::id oty = object::id(ty);
    while (sys_current_ms() - start < wait_time)
    {
        if (rt.error())
        {
            explain("Expected type [", object::name(object::id(ty)), "], "
                    "got error [", rt.error(), "] instead");
            return fail();
        }

        if (Stack.recorded())
        {
            object::id tty = Stack.type();
            if (id(tty) == ty)
                return *this;
            explain("Expected type ", object::name(oty), " (", int(oty), ")"
                    " but got ", object::name(tty), " (", int(tty), ")");
            return fail();
        }
        sys_delay(refresh_delay_time);
    }
    explain("Expected type ", object::name(oty), " (", int(ty), ")"
            " but stack not updated");
    return fail();
}


tests &tests::shift(bool s, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the shift state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(ui.shift == s, "Expected shift ", s, ", got ", ui.shift);
}


tests &tests::xshift(bool x, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the right shift state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(ui.xshift == x, "Expected xshift ", x, " got ", ui.xshift);
}


tests &tests::alpha(bool a, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the alpha state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(ui.alpha == a, "Expected alpha ", a, " got ", ui.alpha);
}


tests &tests::lower(bool l, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the lowercase state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(ui.lowercase == l, "Expected alpha ", l, " got ", ui.alpha);
}


tests &tests::editing(uint extrawait)
// ----------------------------------------------------------------------------
//   Check that we are editing, without checking the length
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(rt.editing(),
                 "Expected to be editing, got length ",
                 rt.editing());
}


tests &tests::editing(size_t length, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the editor has exactly the expected length
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(rt.editing() == length,
                 "Expected editing length to be ", length,
                 " got ", rt.editing());
}


tests &tests::editor(cstring text, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the editor contents matches the text
// ----------------------------------------------------------------------------
{
    byte_p ed = nullptr;
    size_t sz = 0;

    nokeys(extrawait);

    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        if (rt.error())
        {
            explain("Expected editor [", text, "], "
                    "got error [",
                    rt.error(),
                    "] instead");
            return fail();
        }

        ed = rt.editor();
        sz = rt.editing();
        if (ed && sz == strlen(text) && memcmp(ed, text, sz) == 0)
            return *this;

        sys_delay(refresh_delay_time);
    }

    if (!ed)
        explain("Expected editor to contain [",
                text,
                "], "
                "but it's empty");
    if (sz != strlen(text))
        explain("Expected ",
                strlen(text),
                " characters in editor"
                " [",
                text,
                "], "
                "but got ",
                sz,
                " characters "
                " [",
                std::string(cstring(ed), sz),
                "]");
    if (memcmp(ed, text, sz))
        explain("Expected editor to contain [",
                text,
                "], "
                "but it contains [",
                std::string(cstring(ed), sz),
                "]");

    fail();
    return *this;
}


tests &tests::cursor(size_t csr, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the cursor is at expected position
// ----------------------------------------------------------------------------
{
    nokeys(extrawait);
    return check(ui.cursor == csr,
                 "Expected cursor to be at position ",
                 csr,
                 " but it's at position ",
                 ui.cursor);
}


tests &tests::error(cstring msg, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the error message matches expectations
// ----------------------------------------------------------------------------
{
    utf8 err = nullptr;
    nokeys(extrawait);

    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        err = rt.error();
        if (!msg == !err && (!msg || strcmp(cstring(err), msg) == 0))
            return *this;
        sys_delay(refresh_delay_time);
    }
    if (!msg && err)
        explain("Expected no error, got [", err, "]").itest(CLEAR);
    if (msg && !err)
        explain("Expected error message [", msg, "], got none");
    if (msg && err && strcmp(cstring(err), msg) != 0)
        explain("Expected error message [",
                msg,
                "], "
                "got [",
                err,
                "]");
    fail();
    return *this;
}


tests &tests::command(cstring ref, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the command result matches expectations
// ----------------------------------------------------------------------------
{
    utf8 cmd = nullptr;
    nokeys(extrawait);

    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        size_t sz = 0;
        if (object_p cmdo = rt.command())
            if (text_p cmdt = cmdo->as_text())
                cmd = cmdt->value(&sz);
        if (!ref == !cmd && (!ref || strcmp(ref, cstring(cmd)) == 0))
            return *this;

        sys_delay(refresh_delay_time);
    }


    if (!ref && cmd)
        explain("Expected no command, got [", cmd, "]");
    if (ref && !cmd)
        explain("Expected command [", ref, "], got none");
    if (ref && cmd && strcmp(ref, cstring(cmd)) != 0)
        explain("Expected command [", ref, "], got [", cmd, "]");

    fail();
    return *this;
}


tests &tests::source(cstring ref, uint extrawait)
// ----------------------------------------------------------------------------
//   Check that the source indicated in the editor matches expectations
// ----------------------------------------------------------------------------
{
    utf8 src = nullptr;
    nokeys(extrawait);

    uint start     = sys_current_ms();
    uint wait_time = image_wait_time + extrawait;
    while (sys_current_ms() - start < wait_time)
    {
        utf8 src = rt.source();
        if (!src == !ref && (!ref || strcmp(ref, cstring(src)) == 0))
            return *this;
        sys_delay(refresh_delay_time);
    }

    if (!ref && src)
        explain("Expected no source, got [", src, "]");
    if (ref && !src)
        explain("Expected source [", ref, "], got none");
    if (ref && src && strcmp(ref, cstring(src)) != 0)
        explain("Expected source [", ref, "], "
                "got [", src, "]");

    fail();
    return *this;
}
