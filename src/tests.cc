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
#include "recorder.h"
#include "settings.h"
#include "stack.h"
#include "user_interface.h"

#include <regex.h>
#include <stdio.h>


extern volatile int lcd_needsupdate;
volatile uint keysync_sent = 0;
volatile uint keysync_done = 0;

RECORDER_DECLARE(errors);

uint wait_time  = 200;
uint delay_time = 2;

void tests::run(bool onlyCurrent)
// ----------------------------------------------------------------------------
//   Run all test categories
// ----------------------------------------------------------------------------
{
    tindex = sindex = cindex = count = 0;
    failures.clear();

    auto tracing           = RECORDER_TRACE(errors);
    RECORDER_TRACE(errors) = false;

    // Reset to known settings stateg
    reset_settings(onlyCurrent);

    current();
    if (!onlyCurrent)
    {
        shift_logic();
        keyboard_entry();
        data_types();
        arithmetic();
        global_variables();
        local_variables();
        for_loops();
        command_display_formats();
        integer_display_formats();
        decimal_display_formats();
        integer_numerical_functions();
        decimal_numerical_functions();
        complex_types();
        complex_arithmetic();
        complex_functions();
        list_functions();
        text_functions();
        rewrite_engine();
        expand_collect_simplify();
        regression_checks();
    }
    summary();

    RECORDER_TRACE(errors) = tracing;
}


void tests::current()
// ----------------------------------------------------------------------------
//   Test the current thing (this is a temporary test)
// ----------------------------------------------------------------------------
{

    step("Type command");
    test(CLEAR, "12 type", ENTER)
        .type(object::ID_integer)
        .expect(object::ID_integer);
    test(CLEAR, "'ABC*3' type", ENTER)
        .type(object::ID_integer)
        .expect(object::ID_equation);

    step("TypeName command");
    test(CLEAR, "12 typename", ENTER)
        .type(object::ID_text)
        .expect("\"integer\"");
    test(CLEAR, "'ABC*3' typename", ENTER)
        .type(object::ID_text)
        .expect("\"equation\"");

#if 0
    step("Testing sign of modulo for bignum");
#define ZEROS "00000000000000000000"
    test(CLEAR, " 7" ZEROS "  3" ZEROS " MOD", ENTER).expect("1" ZEROS);
    test(CLEAR, " 7" ZEROS " -3" ZEROS " MOD", ENTER).expect("1" ZEROS);
    test(CLEAR, "-7" ZEROS "  3" ZEROS " MOD", ENTER).expect("2" ZEROS);
    test(CLEAR, "-7" ZEROS " -3" ZEROS " MOD", ENTER).expect("2" ZEROS);
    test(CLEAR, " 7" ZEROS "  3" ZEROS " REM", ENTER).expect("1" ZEROS);
    test(CLEAR, " 7" ZEROS " -3" ZEROS " REM", ENTER).expect("1" ZEROS);
    test(CLEAR, "-7" ZEROS "  3" ZEROS " REM", ENTER).expect("-1" ZEROS);
    test(CLEAR, "-7" ZEROS " -3" ZEROS " REM", ENTER).expect("-1" ZEROS);
#endif
}


void tests::reset_settings(bool fast)
// ----------------------------------------------------------------------------
//   Use settings that make the results predictable on screen
// ----------------------------------------------------------------------------
{
    // Reset to default test settings
    Settings = settings();

    // Do it the fast way if we only run current tests
    if (fast)
    {
        begin("Fast-track settings reset");
        return;
    }

    // Otherwise exercise settings routines
    begin("Reset settings");
    step("Numerical settings").test("StandardDisplay", ENTER).noerr();
    step("Switching to degrees").test("Degrees", ENTER).noerr();
    step("Using long form for commands").test("LongForm", ENTER).noerr();
    step("Using dot as fractional mark").test("DecimalDot", ENTER).noerr();
    step("Setting trailing decimal").test("TrailingDecimal", ENTER).noerr();
    step("Using default 34-digit precision")
        .test("34 Precision", ENTER)
        .noerr();
    step("Using fancy unicode exponent")
        .test("FancyExponent", ENTER)
        .noerr();
    step("Using 64-bit word size").test("64 StoreWordSize", ENTER).noerr();
    step("Disable spacing")
        .test("3 NumberSpacing", ENTER)         .noerr()
        .test("3 MantissaSpacing", ENTER)       .noerr()
        .test("5 FractionSpacing", ENTER)       .noerr()
        .test("4 BasedSpacing", ENTER)          .noerr();
    step("Select Modes menu")
        .test("ModesMenu", ENTER)               .noerr();
    step("Checking output modes")
        .test("Modes", ENTER)
        .expect("« ModesMenu »");

}


void tests::shift_logic()
// ----------------------------------------------------------------------------
//   Test all keys and check we have the correct output
// ----------------------------------------------------------------------------
{
    begin("Shift logic");
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
    begin("Keyboard logic");

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
    cstring seps = "\"Hello [A] (B) {C} 'Test' D";
    test(CLEAR, seps).editor(seps).wait(500);

    step("Separators with auto-spacing");
    cstring seps2     = "{}()[]";
    cstring seps2auto = "{ } ( ) []";
    test(CLEAR, seps2).editor(seps2auto).wait(500);

    step("Key repeat");
    test(CLEAR, LONGPRESS, SHIFT, LONGPRESS, A)
        .wait(1000)
        .test(RELEASE)
        .check(ui.cursor > 4);
}


void tests::data_types()
// ----------------------------------------------------------------------------
//   Check the basic data types
// ----------------------------------------------------------------------------
{
    begin("Data types");

    step("Positive integer");
    test(CLEAR, "1", ENTER).type(object::ID_integer).expect("1");
    step("Negative integer");
    test(CLEAR, "1", CHS, ENTER).type(object::ID_neg_integer).expect("-1");

#if CONFIG_FIXED_BASED_OBJECTS
    step("Binary based integer");
    test(CLEAR, "#10010101b", ENTER)
        .type(object::ID_bin_integer)
        .expect("#1001 0101₂");
    test(CLEAR, "#101B", ENTER).type(object::ID_bin_integer).expect("#101₂");

    step("Decimal based integer");
    test(CLEAR, "#12345d", ENTER)
        .type(object::ID_dec_integer)
        .expect("#1 2345₁₀");
    test(CLEAR, "#123D", ENTER).type(object::ID_dec_integer).expect("#123₁₀");

    step("Octal based integer");
    test(CLEAR, "#12345o", ENTER)
        .type(object::ID_oct_integer)
        .expect("#1 2345₈");
    test(CLEAR, "#123O", ENTER).type(object::ID_oct_integer).expect("#123₈");

    step("Hexadecimal based integer");
    test(CLEAR, "#1234ABCDH", ENTER)
        .type(object::ID_hex_integer)
        .type(object::ID_hex_integer)
        .expect("#1234 ABCD₁₆");
    test(CLEAR, "#DEADBEEFH", ENTER)
        .type(object::ID_hex_integer)
        .expect("#DEAD BEEF₁₆");
#endif // CONFIG_FIXED_BASED_OBJECTS

    step("Arbitrary base input");
    test(CLEAR, "8#777", ENTER).type(object::ID_based_integer).expect("#1FF₁₆");
    test(CLEAR, "2#10000#ABCDE", ENTER)
        .type(object::ID_based_integer)
        .expect("#A BCDE₁₆");

    step("Symbols");
    cstring symbol = "ABC123Z";
    test(CLEAR, symbol, ENTER).type(object::ID_equation).expect("'ABC123Z'");

    step("Text");
    cstring string = "\"Hello World\"";
    test(CLEAR, string, ENTER).type(object::ID_text).expect(string);

    step("List");
    cstring list = "{ A 1 3 }";
    test(CLEAR, list, ENTER).type(object::ID_list).expect(list);

    step("Program");
    cstring prgm = "« 1 + sin »";
    test(CLEAR, SHIFT, RUNSTOP, 1, ADD, "sin", ENTER)
        .type(object::ID_program)
        .expect(prgm);

    step("Equation");
    cstring eqn = "'X+1'";
    test(CLEAR, XEQ, "X", ENTER, KEY1, ADD)
        .type(object::ID_equation)
        .expect(eqn);
    cstring eqn2 = "'sin(X+1)'";
    test(SIN)
        .type(object::ID_equation)
        .expect(eqn2);
    test(DOWN)
        .editor(eqn2);
    test(ENTER, 1, ADD).
        type(object::ID_equation).expect("'sin(X+1)+1'");

    step("Equation parsing and simplification");
    test(CLEAR, "'(((A))+(B))-(C+D)'", ENTER)
        .type(object::ID_equation)
        .expect("'A+B-(C+D)'");
    step("Equation fancy rendering");
    test(CLEAR, XEQ, "X", ENTER, INV,
         XEQ, "Y", ENTER, SHIFT, SQRT, XEQ, "Z", ENTER,
         "CUBED", ENTER, ADD, ADD, WAIT(100))
        .type(object::ID_equation)
        .expect("'X⁻¹+(Y²+Z³)'");
    step("Equation fancy parsing from editor");
    test(DOWN, "   ", SHIFT, SHIFT, DOWN, " 1 +", ENTER)
        .type(object::ID_equation).expect("'X⁻¹+(Y²+Z³)+1'");

    step("Fractions");
    test(CLEAR, "1/3", ENTER).type(object::ID_fraction).expect("1/3");
    test(CLEAR, "-80/60", ENTER).type(object::ID_neg_fraction).expect("-4/3");
    test(CLEAR, "20/60", ENTER).type(object::ID_fraction).expect("1/3");

    step("Large integers");
    cstring b = "123456789012345678901234567890123456789012345678901234567890";
    cstring mb =
        "-123 456 789 012 345 678 901 234 567 890"
        " 123 456 789 012 345 678 901 234 567 890";
    test(CLEAR, b, ENTER).type(object::ID_bignum).expect(mb+1);
    test(DOWN, CHS, ENTER).type(object::ID_neg_bignum).expect(mb);
    test(CHS).type(object::ID_bignum).expect(mb + 1);
    test(DOWN, CHS, ENTER).type(object::ID_neg_bignum).expect(mb);

    step("Large fractions");
    cstring bf =
        "123456789012345678901234567890123456789012345678901234567890/"
        "123456789012345678901234567890123456789012345678901234567891";
    cstring mbf =
        "-123 456 789 012 345 678 901 234 567 890 123 456 789"
        " 012 345 678 901 234 567 890/"
        "123 456 789 012 345 678 901 234 567 890 123 456 789"
        " 012 345 678 901 234 567 891";
    test(CLEAR, bf, ENTER).type(object::ID_big_fraction).expect(mbf+1);
    test(DOWN, CHS, ENTER).type(object::ID_neg_big_fraction).expect(mbf);
    test(CHS).type(object::ID_big_fraction).expect(mbf+1);
    test(CHS).type(object::ID_neg_big_fraction).expect(mbf);
    test(DOWN, CHS, ENTER).type(object::ID_big_fraction).expect(mbf+1);

    clear();

    step ("Bytes command");
    test(CLEAR, "12", ENTER, "bytes", ENTER)
        .expect("2")
        .test(BSP)
        .match("#C....");
    test(CLEAR, "129", ENTER, "bytes", ENTER)
        .expect("3")
        .test(BSP)
        .match("#1 81....");

    step("Type command");
    test(CLEAR, "12 type", ENTER)
        .type(object::ID_integer)
        .expect(object::ID_integer);
    test(CLEAR, "'ABC*3' type", ENTER)
        .type(object::ID_integer)
        .expect(object::ID_equation);

    step("TypeName command");
    test(CLEAR, "12 typename", ENTER)
        .type(object::ID_text)
        .expect("\"integer\"");
    test(CLEAR, "'ABC*3' typename", ENTER)
        .type(object::ID_text)
        .expect("\"equation\"");
}


void tests::arithmetic()
// ----------------------------------------------------------------------------
//   Tests for basic arithmetic operations
// ----------------------------------------------------------------------------
{
    begin("Arithmetic");

    step("Integer addition");
    test(CLEAR, 1, ENTER, 1, ADD).type(object::ID_integer).expect("2");
    test(1, ADD).type(object::ID_integer).expect("3");
    test(-1, ADD).type(object::ID_integer).expect("2");
    test(-1, ADD).type(object::ID_integer).expect("1");
    test(-1, ADD).type(object::ID_integer).expect("0");
    test(-1, ADD).type(object::ID_neg_integer).expect("-1");
    test(-1, ADD).type(object::ID_neg_integer).expect("-2");
    test(-1, ADD).type(object::ID_neg_integer).expect("-3");
    test(1, ADD).type(object::ID_neg_integer).expect("-2");
    test(1, ADD).type(object::ID_neg_integer).expect("-1");
    test(1, ADD).type(object::ID_integer).expect("0");

    step("Integer addition overflow");
    test(CLEAR, (1ULL << 63) - 2ULL, ENTER, 1, ADD)
        .type(object::ID_integer)
        .expect("9 223 372 036 854 775 807");
    test(CLEAR, (1ULL << 63) - 3ULL, CHS, ENTER, -2, ADD)
        .type(object::ID_neg_integer)
        .expect("-9 223 372 036 854 775 807");

    test(CLEAR, ~0ULL, ENTER, 1, ADD)
        .type(object::ID_bignum)
        .expect("18 446 744 073 709 551 616");
    test(CLEAR, ~0ULL, CHS, ENTER, -2, ADD)
        .type(object::ID_neg_bignum)
        .expect("-18 446 744 073 709 551 617");

    step("Adding ten small integers at random");
    srand48(sys_current_ms());
    Settings.spacing_mantissa = 0;
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, ADD)
            .explain("Computing ", x, " + ", y, ", ")
            .expect(x + y);
    }
    Settings.spacing_mantissa = 3;

    step("Integer subtraction");
    test(CLEAR, 1, ENTER, 1, SUB).type(object::ID_integer).expect("0");
    test(1, SUB).type(object::ID_neg_integer).expect("-1");
    test(-1, SUB).type(object::ID_integer).expect("0");
    test(-1, SUB).type(object::ID_integer).expect("1");
    test(-1, SUB).type(object::ID_integer).expect("2");
    test(1, SUB).type(object::ID_integer).expect("1");
    test(1, SUB).type(object::ID_integer).expect("0");
    test(3, SUB).type(object::ID_neg_integer).expect("-3");
    test(-1, SUB).type(object::ID_neg_integer).expect("-2");
    test(1, SUB).type(object::ID_neg_integer).expect("-3");
    test(-3, SUB).type(object::ID_integer).expect("0");

    step("Integer subtraction overflow");
    test(CLEAR, 0xFFFFFFFFFFFFFFFFull, CHS, ENTER, 1, SUB)
        .type(object::ID_neg_bignum)
        .expect("-18 446 744 073 709 551 616");
    test(CLEAR, -3, ENTER, 0xFFFFFFFFFFFFFFFFull, SUB)
        .type(object::ID_neg_bignum)
        .expect("-18 446 744 073 709 551 618");

    step("Subtracting ten small integers at random");
    Settings.spacing_mantissa = 0;
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, SUB)
            .explain("Computing ", x, " - ", y, ", ")
            .expect(x - y);
    }
    Settings.spacing_mantissa = 3;

    step("Integer multiplication");
    test(CLEAR, 3, ENTER, 7, MUL).type(object::ID_integer).expect("21");
    test(3, MUL).type(object::ID_integer).expect("63");
    test(-3, MUL).type(object::ID_neg_integer).expect("-189");
    test(2, MUL).type(object::ID_neg_integer).expect("-378");
    test(-7, MUL).type(object::ID_integer).expect("2 646");

    step("Multiplying ten small integers at random");
    Settings.spacing_mantissa = 0;
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0xFFFFFF) - 0x800000;
        large y = (lrand48() & 0xFFFFFF) - 0x800000;
        test(CLEAR, x, ENTER, y, MUL)
            .explain("Computing ", x, " * ", y, ", ")
            .expect(x * y);
    }
    Settings.spacing_mantissa = 3;

    step("Integer division");
    test(CLEAR, 210, ENTER, 2, DIV).type(object::ID_integer).expect("105");
    test(5, DIV).type(object::ID_integer).expect("21");
    test(-3, DIV).type(object::ID_neg_integer).expect("-7");
    test(-7, DIV).type(object::ID_integer).expect("1");

    step("Dividing ten small integers at random");
    Settings.spacing_mantissa = 0;
    for (int i = 0; i < 10; i++)
    {
        large x = (lrand48() & 0x3FFF) - 0x4000;
        large y = (lrand48() & 0x3FFF) - 0x4000;
        test(CLEAR, x * y, ENTER, y, DIV)
            .explain("Computing ", x * y, " / ", y, ", ")
            .expect(x);
    }
    Settings.spacing_mantissa = 3;

    step("Division with fractional output");
    test(CLEAR, 1, ENTER, 3, DIV).expect("1/3");
    test(CLEAR, 2, ENTER, 5, DIV).expect("2/5");

    step("Manual computation of 100!");
    test(CLEAR, 1, ENTER);
    for (uint i = 1; i <= 100; i++)
        test(i, MUL, NOKEYS, WAIT(20));
    expect( "93 326 215 443 944 152 681 699 238 856 266 700 490 715 968 264 "
           "381 621 468 592 963 895 217 599 993 229 915 608 941 463 976 156 "
           "518 286 253 697 920 827 223 758 251 185 210 916 864 000 000 000 "
           "000 000 000 000 000");
    step("Manual division by all factors of 100!");
    for (uint i = 1; i <= 100; i++)
        test(i * 997 % 101, DIV, NOKEYS, WAIT(20));
    expect(1);

    step("Manual computation of 997/100!");
    test(CLEAR, 997, ENTER);
    for (uint i = 1; i <= 100; i++)
        test(i * 997 % 101, DIV, NOKEYS, WAIT(20));
    expect("997/"
           "93 326 215 443 944 152 681 699 238 856 266 700 490 715 968 264 "
           "381 621 468 592 963 895 217 599 993 229 915 608 941 463 976 156 "
           "518 286 253 697 920 827 223 758 251 185 210 916 864 000 000 000 "
           "000 000 000 000 000");

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
    test(CLEAR, " 7/2  3 REM", ENTER).expect("1/2");
    test(CLEAR, " 7/2 -3 REM", ENTER).expect("1/2");
    test(CLEAR, "-7/2  3 REM", ENTER).expect("-1/2");
    test(CLEAR, "-7/2 -3 REM", ENTER).expect("-1/2");
    test(CLEAR, " 7/2  3 REM", ENTER).expect("1/2");
    test(CLEAR, " 7/2 -3 REM", ENTER).expect("1/2");
    test(CLEAR, "-7/2  3 REM", ENTER).expect("-1/2");
    test(CLEAR, "-7/2 -3 REM", ENTER).expect("-1/2");

    step("Modulo of negative value");
    test(CLEAR, "-360 360 MOD", ENTER).expect("0");
    test(CLEAR, "1/3 -1/3 MOD", ENTER).expect("0");
    test(CLEAR, "360 -360 MOD", ENTER).expect("0");
    test(CLEAR, "-1/3 1/3 MOD", ENTER).expect("0");

    step("Power");
    test(CLEAR, "2 3 ^", ENTER).expect("8");
    test(CLEAR, "-2 3 ^", ENTER).expect("-8");
    step("Negative power");
    test(CLEAR, "2 -3 ^", ENTER).expect("1/8");
    test(CLEAR, "-2 -3 ^", ENTER).expect("-1/8");
}


void tests::global_variables()
// ----------------------------------------------------------------------------
//   Tests for access to global variables
// ----------------------------------------------------------------------------
{
    begin("Global variables");

    step("Store in global variable");
    test(CLEAR, 12345, ENTER).expect("12 345");
    test(XEQ, "A", ENTER).expect("'A'");
    test(STO).noerr();
    step("Recall global variable");
    test(CLEAR, 1, ENTER, XEQ, "A", ENTER).expect("'A'");
    test("RCL", ENTER).noerr().expect("12 345");

    step("Store in long-name global variable");
    test(CLEAR, "\"Hello World\"", ENTER, XEQ, "SomeLongVariable", ENTER, STO)
        .noerr();
    step("Recall global variable");
    test(CLEAR, XEQ, "SomeLongVariable", ENTER, "recall", ENTER)
        .noerr()
        .expect("\"Hello World\"");

    step("Recall non-existent variable");
    test(CLEAR, XEQ, "DOESNOTEXIST", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();
    step("Recall invalid variable object");
    test(1234, ENTER, "RCL", ENTER).error("Invalid name").clear();

    step("Store program in global variable");
    test(CLEAR, "« 1 + »", ENTER, XEQ, "INCR", ENTER, STO).noerr();
    step("Evaluate global variable");
    test(CLEAR, "A INCR", ENTER).expect("12 346");

    step("Purge global variable");
    test(CLEAR, XEQ, "A", ENTER, "PURGE", ENTER).noerr();
    test(CLEAR, XEQ, "INCR", ENTER, "PURGE", ENTER).noerr();
    test(CLEAR, XEQ, "SomeLongVariable", ENTER, "PURGE", ENTER).noerr();

    test(CLEAR, XEQ, "A", ENTER, "RCL", ENTER).error("Undefined name").clear();
    test(CLEAR, XEQ, "INCR", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();
    test(CLEAR, XEQ, "SomeLongVariable", ENTER, "RCL", ENTER)
        .error("Undefined name")
        .clear();
}


void tests::local_variables()
// ----------------------------------------------------------------------------
//   Tests for access to local variables
// ----------------------------------------------------------------------------
{
    begin("Local variables");

    step("Creating a local block");
    cstring source = "« → A B C « A B + A B - × B C + B C - × ÷ » »";
    test(CLEAR, source, ENTER).type(object::ID_program).expect(source);
    test(XEQ, "LocTest", ENTER, STO).noerr();

    step("Calling a local block with numerical values");
    test(CLEAR, 1, ENTER, 2, ENTER, 3, ENTER, "LocTest", ENTER).expect("3/5");

    step("Calling a local block with symbolic values");
    test(CLEAR,
         XEQ, "X", ENTER,
         XEQ, "Y", ENTER,
         XEQ, "Z", ENTER,
         "LocTest", ENTER)
        .expect("'(X+Y)×(X-Y)÷((Y+Z)×(Y-Z))'");

    step("Cleanup");
    test(CLEAR, XEQ, "LocTest", ENTER, "PurgeAll", ENTER).noerr();
}


void tests::for_loops()
// ----------------------------------------------------------------------------
//   Test simple for loops
// ----------------------------------------------------------------------------
{
    begin("For loops");

    step("Simple 1..10");
    cstring pgm  = "« 0 1 10 FOR i i SQ + NEXT »";
    cstring pgmo = "« 0 1 10 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP).noerr().type(object::ID_integer).expect(385);

    step("Algebraic 1..10");
    pgm  = "« 'X' 1 5 FOR i i SQ + NEXT »";
    pgmo = "« 'X' 1 5 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP).noerr().type(object::ID_equation).expect("'X+1+4+9+16+25'");

    step("Stepping by 2");
    pgm  = "« 0 1 10 FOR i i SQ + 2 STEP »";
    pgmo = "« 0 1 10 for i i x² + 2 step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP).noerr().type(object::ID_integer).expect(165);

    step("Stepping by i");
    pgm  = "« 'X' 1 100 FOR i i SQ + i step »";
    pgmo = "« 'X' 1 100 for i i x² + i step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP)
        .noerr()
        .type(object::ID_equation)
        .expect("'X+1+4+16+64+256+1 024+4 096'");

    step("Negative stepping");
    pgm  = "« 0 10 1 FOR i i SQ + -1 STEP »";
    pgmo = "« 0 10 1 for i i x² + -1 step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP).noerr().type(object::ID_integer).expect(385);

    step("Negative stepping algebraic");
    pgm  = "« 'X' 10 1 FOR i i SQ + -1 step »";
    pgmo = "« 'X' 10 1 for i i x² + -1 step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP)
        .noerr()
        .type(object::ID_equation)
        .expect("'X+100+81+64+49+36+25+16+9+4+1'");

    step("Fractional");
    pgm  = "« 'X' 0.1 0.9 FOR i i SQ + 0.1 step »";
    pgmo = "« 'X' 0.1 0.9 for i i x² + 0.1 step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP)
        .noerr()
        .type(object::ID_equation)
        .expect("'X+0.01+0.04+0.09+0.16+0.25+0.36+0.49+0.64+0.81'");

    step("Fractional down");
    pgm  = "« 'X' 0.9 0.1 FOR i i SQ + -0.1 step »";
    pgmo = "« 'X' 0.9 0.1 for i i x² + -0.1 step »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP)
        .noerr()
        .type(object::ID_equation)
        .expect("'X+0.81+0.64+0.49+0.36+0.25+0.16+0.09+0.04+0.01'");

    step("Execute at least once");
    pgm  = "« 'X' 10 1 FOR i i SQ + NEXT »";
    pgmo = "« 'X' 10 1 for i i x² + next »";
    test(CLEAR, pgm, ENTER).noerr().type(object::ID_program).expect(pgmo);
    test(RUNSTOP).noerr().type(object::ID_equation).expect("'X+100'");
}


void tests::logical_operations()
// ----------------------------------------------------------------------------
//   Perform logical operations on small and big integers
// ----------------------------------------------------------------------------
{
    begin("Logical operations");

#if CONFIG_FIXED_BASED_OBJECTS
    step("Binary number");
    cstring binary  = "#10001b";
    cstring binaryf = "#1 0001₂";
    test(CLEAR, binary, ENTER).type(object::ID_bin_integer).expect(binaryf);

    step("Octal number");
    cstring octal  = "#1777o";
    cstring octalf = "#1777₈";
    test(CLEAR, octal, ENTER).type(object::ID_oct_integer).expect(octalf);

    step("Decimal number");
    cstring decimal  = "#12345d";
    cstring decimalf = "#1 2345₁₀";
    test(CLEAR, decimal, ENTER).type(object::ID_dec_integer).expect(decimalf);

    step("Hexadecimal number");
    cstring hexa  = "#135AFh";
    cstring hexaf = "#1 35AF₁₆";
    test(CLEAR, hexa, ENTER).type(object::ID_hex_integer).expect(hexaf);
#endif // CONFIG_FIXED_BASED_OBJECTS

    step("Based number (default base)");
    cstring based  = "#1234A";
    cstring basedf = "#1 234A₁₆";
    test(CLEAR, based, ENTER).type(object::ID_based_integer).expect(basedf);

    step("Based number (arbitrary base)");
    cstring abased  = "17#1234AG";
    cstring abasedf = "#18 75A4₁₆";
    test(CLEAR, abased, ENTER).type(object::ID_based_integer).expect(abasedf);

    step("Display in arbitrary base");
    test("17 base", ENTER).expect("#12 34AG₁₇");
    test("3 base", ENTER).expect("#10 0001 0221 2122₃");
    test("36 base", ENTER).expect("#YCV8₃₆");
    test("16 base", ENTER).expect("#18 75A4₁₆");

    step("Range for bases");
    test("1 base", ENTER).error("Invalid numeric base");
    test(CLEAR, "37 base", ENTER).error("Invalid numeric base");
    test(CLEAR, "0.3 base", ENTER).error("Bad argument type");
    test(CLEAR);

    step("Default word size");
    test("WordSize", ENTER).expect("64");
    step("Set word size to 16");
    test(CLEAR, "16 STWS", ENTER).noerr();

    step("Binary not");
    test(CLEAR, "#12 not", ENTER).expect("#FFED₁₆");
    test("not", ENTER).expect("#12₁₆");

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
    test(CLEAR, "32 STWS", ENTER).noerr();
    test(CLEAR, "#12 not", ENTER).expect("#FFFF FFED₁₆");
    test("not", ENTER).expect("#12₁₆");

    step("Set word size to 30");
    test(CLEAR, "30 STWS", ENTER).noerr();
    test(CLEAR, "#142 not", ENTER).expect("#3FFF FEBD₁₆");
    test("not", ENTER).expect("#142₁₆");

    step("Set word size to 48");
    test(CLEAR, "48 STWS", ENTER).noerr();
    test(CLEAR, "#233 not", ENTER).expect("#FFFF FFFF FDCC₁₆");
    test("not", ENTER).expect("#233₁₆");

    step("Set word size to 64");
    test(CLEAR, "64 STWS", ENTER).noerr();
    test(CLEAR, "#64123 not", ENTER).expect("#FFFF FFFF FFF9 BEDC₁₆");
    test("not", ENTER).expect("#6 4123₁₆");

    step("Set word size to 128");
    test(CLEAR, "128 STWS", ENTER).noerr();
    test(CLEAR, "#12 not", ENTER).expect("#FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFED₁₆");
    test("dup not", ENTER).expect("#12₁₆");
    test("xor not", ENTER).expect("#0₁₆");
}


void tests::command_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for commands
// ----------------------------------------------------------------------------
{
    begin("Commands display formats");

    step("Commands");
    // There is a trap in this command line
    cstring prgm =
        "«"
        "  1 1.0"
        "+ - * / ^ "
        "sin cos tan asin acos atan "
        "LowerCase PurgeAll Precision "
        "start step next start step for i next for i step "
        "while repeat end do until end » ";

    test(CLEAR, prgm, ENTER).noerr();
    step("Lower case");
    test("lowercase", ENTER)
        .expect("« 1 1. + - × ÷ ↑ sin cos tan asin acos atan lowercase "
                "purgeall precision start step next start step for i next "
                "for i step while repeat end do until end »");

    step("Upper case");
    test("UPPERCASE", ENTER)
        .expect("« 1 1. + - × ÷ ↑ SIN COS TAN ASIN ACOS ATAN LOWERCASE "
                "PURGEALL PRECISION START STEP next START STEP FOR i NEXT "
                "FOR i STEP WHILE REPEAT END DO UNTIL END »");

    step("Capitalized");
    test("Capitalized", ENTER)
        .expect("« 1 1. + - × ÷ ↑ Sin Cos Tan Asin Acos Atan Lowercase "
                "Purgeall Precision Start Step next Start Step For i Next "
                "For i Step While Repeat End Do Until End »");

    step("Long form");
    test("LongForm", ENTER)
        .expect("« 1 1. + - × ÷ ↑ sin cos tan sin⁻¹ cos⁻¹ tan⁻¹ LowerCase "
                "PurgeAll Precision start step next start step for i next "
                "for i step while repeat end do until end »");
}


void tests::integer_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for integer values
// ----------------------------------------------------------------------------
{
    begin("Integer display formats");

    step("Reset settings to defaults");
    test(CLEAR)
        .test("3 NumberSpacing", ENTER)         .noerr()
        .test("3 MantissaSpacing", ENTER)       .noerr()
        .test("5 FractionSpacing", ENTER)       .noerr()
        .test("4 BasedSpacing", ENTER)          .noerr()
        .test("NumberSpaces", ENTER)            .noerr()
        .test("BasedSpaces", ENTER)             .noerr();

    step("Default integer rendering");
    test(CLEAR, 1, ENTER)
        .type(object::ID_integer)
        .expect("1");
    test(CLEAR, 12, ENTER)
        .type(object::ID_integer)
        .expect("12");
    test(CLEAR, 123, ENTER)
        .type(object::ID_integer)
        .expect("123");
    test(CLEAR, 1234, ENTER)
        .type(object::ID_integer)
        .expect("1 234");
    test(CLEAR, 12345, ENTER)
        .type(object::ID_integer)
        .expect("12 345");
    test(CLEAR, 123456789, ENTER)
        .type(object::ID_integer)
        .expect("123 456 789");

    step("No spacing");
    test("0 MantissaSpacing", ENTER)
        .expect("123456789");

    step("Four spacing");
    test("4 NumberSpacing", ENTER)
        .expect("1 2345 6789");

    step("Five spacing");
    test("5 NumberSpacing", ENTER)
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
        .type(object::ID_bignum)
        .expect("123 456 789 012 345 678 901 234 567 890");

    step("Entering numbers with spacing");
    test(CLEAR, "FancyExponent", ENTER).noerr();

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
    test(ENTER).noerr();

    step("Based number rendering");
    test(CLEAR, "#1234ABCDEFH", ENTER)
#if CONFIG_FIXED_BASED_OBJECTS
        .type(object::ID_hex_integer)
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


void tests::decimal_display_formats()
// ----------------------------------------------------------------------------
//   Check the various display formats for decimal values
// ----------------------------------------------------------------------------
{
    begin("Decimal display formats");

    step("Standard mode");
    test(CLEAR, "STD", ENTER).noerr();

    step("Small number");
    test(CLEAR, "1.03", ENTER)
        .type(object::ID_decimal32)
        .expect("1.03");

    step("Zero");
    test(CLEAR, ".", ENTER)
        .type(object::ID_decimal32)
        .expect("0.");

    step("Negative");
    test(CLEAR, "0.3", CHS, ENTER)
        .type(object::ID_decimal32)
        .expect("-0.3");

    step("Scientific entry");
    test(CLEAR, "1", EEX, "2", ENTER)
        .type(object::ID_decimal32)
        .expect("100.");

    step("Scientific entry with negative exponent");
    test(CLEAR, "1", EEX, "2", CHS, ENTER)
        .type(object::ID_decimal32)
        .expect("0.01");

    step("Negative entry with negative exponent");
    test(CLEAR, "1", CHS, EEX, "2", CHS, ENTER)
        .type(object::ID_decimal32)
        .expect("-0.01");

    step("Non-scientific display");
    test(CLEAR, "0.245", ENTER)
        .type(object::ID_decimal32)
        .expect("0.245");
    test(CLEAR, "0.0003", CHS, ENTER)
        .type(object::ID_decimal32)
        .expect("-0.0003");
    test(CLEAR, "123.456", ENTER)
        .type(object::ID_decimal32)
        .expect("123.456");

    step("Selection of decimal64");
    test(CLEAR, "1.2345678", ENTER)
        .type(object::ID_decimal64)
        .expect("1.23456 78");

    step("Selection of decimal64 based on exponent");
    test(CLEAR, "1.23", EEX, 100, ENTER)
        .type(object::ID_decimal64)
        .expect("1.23⁳¹⁰⁰");

    step("Selection of decimal128");
    test(CLEAR, "1.2345678901234567890123", ENTER)
        .type(object::ID_decimal128)
        .expect("1.23456 78901 23456 789");
    step("Selection of decimal128 based on exponent");
    test(CLEAR, "1.23", EEX, 400, ENTER)
        .type(object::ID_decimal128)
        .expect("1.23⁳⁴⁰⁰");

    step("Automatic switching to scientific display");
    test(CLEAR, "1000000000000.", ENTER)
        .expect("1.⁳¹²");
    test(CLEAR, "0.00000000000025", ENTER)
        .expect("2.5⁳⁻¹³");

    step("FIX 4 mode");
    test(CLEAR, "4 FIX", ENTER).noerr();
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
    test(CLEAR, "24 FIX", ENTER).noerr();
    test(CLEAR, "1.01", ENTER).expect("1.01000 00000 00000 00000 0000");
    test(CLEAR, "1.0123 log", ENTER)
        .expect("1.22249 69622 56897 09224 5327⁳⁻²");

    step("SCI 3 mode");
    test(CLEAR, "3 Sci", ENTER).noerr();
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
    test(CLEAR, "3 eng", ENTER).noerr();
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
    test(CLEAR, "3 sig", ENTER).noerr();
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
    test(CLEAR, "5 Sci", ENTER).noerr();
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
    test(CLEAR, "5 eng", ENTER).noerr();
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
    test(CLEAR, "5 sig", ENTER).noerr();
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
    test(CLEAR, "13 Sci", ENTER).noerr();
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
    test(CLEAR, "13 eng", ENTER).noerr();
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
    test(CLEAR, "13 sig", ENTER).noerr();
    test(CLEAR, "1.01", ENTER).expect("1.01")
        .test(CHS).expect("-1.01");
    test(CLEAR, "1.0123", ENTER).expect("1.0123");
    test(CLEAR, "10.12345", ENTER).expect("10.12345");
    test(CLEAR, "101.2543", ENTER).expect("101.2543");
    test(CLEAR, "1999.999", ENTER).expect("1 999.999");
    test(CLEAR, "19999999999999.", ENTER).expect("2.⁳¹³");
    test(CLEAR, "0.00000000001999999", ENTER).expect("1.99999 9⁳⁻¹¹")
        .test(CHS).expect("-1.99999 9⁳⁻¹¹");

    step("Reset defaults");
    test(CLEAR, "Std", ENTER).noerr();

    step("Test display of 5000.");
    test(CLEAR, "5000.", ENTER)        .expect("5 000.");
    test(CLEAR, "50000.", ENTER)       .expect("50 000.");
    test(CLEAR, "500000.", ENTER)      .expect("500 000.");
    test(CLEAR, "5000000.", ENTER)     .expect("5 000 000.");
}


void tests::integer_numerical_functions()
// ----------------------------------------------------------------------------
//   Test integer numerical functions
// ----------------------------------------------------------------------------
{
    begin("Integer functions");

    step("neg")
        .test(CLEAR, "3 neg", ENTER).expect("-3")
        .test("negate", ENTER).expect("3");
    step("inv")
        .test(CLEAR, "3 inv", ENTER).expect("1/3")
        .test("inv", ENTER).expect("3")
        .test(CLEAR, "-3 inv", ENTER).expect("-1/3")
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
    begin("Decimal functions");

    step("neg")
        .test(CLEAR, "3.21 neg", ENTER).expect("-3.21")
        .test("negate", ENTER).expect("3.21");
    step("inv")
        .test(CLEAR, "3.21 inv", ENTER).expect("3.11526 47975 07788 162⁳⁻¹")
        .test("inv", ENTER).expect("3.21");
    step("sq (square)")
        .test(CLEAR, "-3.21 sq", ENTER).expect("10.3041")
        .test("sq", ENTER).expect("106.17447 681");
    step("cubed")
        .test(CLEAR, "3.21 cubed", ENTER).expect("33.07616 1")
        .test("cubed", ENTER).expect("36 186.39267 80659 01161")
        .test(CLEAR, "-3 cubed", ENTER).expect("-27")
        .test("cubed", ENTER).expect("-19 683");
    step("abs")
        .test(CLEAR, "-3.21 abs", ENTER).expect("3.21")
        .test("abs", ENTER, 1, ADD).expect("4.21");

    step("Setting radians mode");
    test(CLEAR, "RAD", ENTER).noerr();

#define TFNA(name, arg, result)                                           \
    step(#name).test(CLEAR, #arg " " #name, ENTER).expect(result);
#define TFN(name, result)  TFNA(name, 0.321, result)

    TFN(sqrt, "5.66568 61896 86117 7993⁳⁻¹");
    TFN(sin, "3.15515 63859 27271 1131⁳⁻¹");
    TFN(cos, "9.48920 37695 65830 1754⁳⁻¹");
    TFN(tan, "3.32499 59243 64718 7511⁳⁻¹");
    TFN(asin, "3.26785 17653 14954 6327⁳⁻¹");
    TFN(acos, "1.24401 11502 63401 156");
    TFN(atan, "3.10609 79281 38899 1761⁳⁻¹");
    TFN(sinh, "3.26541 16495 18063 5701⁳⁻¹");
    TFN(cosh, "1.05196 44159 41947 5384");
    TFN(tanh, "3.10410 84660 58860 2149⁳⁻¹");
    TFN(asinh, "3.15728 26582 93796 1791⁳⁻¹");
    TFNA(acosh, 1.321, "7.81230 20519 62526 1474⁳⁻¹");
    TFN(atanh, "3.32761 58848 18145 958⁳⁻¹");
    TFN(log1p, "2.78389 02554 01882 6677⁳⁻¹");
    TFN(lnp1, "2.78389 02554 01882 6677⁳⁻¹");
    TFN(expm1, "3.78505 58089 37538 9545⁳⁻¹");
    TFN(log, "-1.13631 41558 52121 1874");
    TFN(log10, "-4.93494 96759 51279 2187⁳⁻¹");
    TFN(exp, "1.37850 55808 93753 8954");
    TFN(exp10, "2.09411 24558 50892 6705");
    TFN(exp2, "1.24919 61256 53376 7005");
    TFN(erf, "3.50144 22082 00238 2355⁳⁻¹");
    TFN(erfc, "6.49855 77917 99761 7645⁳⁻¹");
    TFN(tgamma, "2.78663 45408 45472 368");
    TFN(lgamma, "1.02483 46099 57313 1987");
    TFN(gamma, "2.78663 45408 45472 368");
    TFN(cbrt, "6.84702 12775 72241 6184⁳⁻¹");
    TFN(norm, "0.321");
#undef TFN

    step("pow")
        ,test(CLEAR, "3.21 1.23 pow", ENTER)
        .expect("4.19760 13402 69557 0313")
        .test(CLEAR, "1.23 2.31").shifts(true,false,false,false).test(B)
        .expect("1.61317 24907 55543 8443");

    step("hypot")
        .test(CLEAR, "3.21 1.23 hypot", ENTER)
        .expect("3.43758 63625 51492 32");
}


void tests::exact_trig_cases()
// ----------------------------------------------------------------------------
//   Special trig cases that are handled accurately for polar representation
// ----------------------------------------------------------------------------
{
    begin("Special trigonometry cases");

    cstring unit_names[] = { "Grads", "Degrees", "PiRadians" };
    int circle[] = { 400, 360, 2 };

    for (uint unit = 0; unit < 3; unit++)
    {
        step(unit_names[unit]);
        test(CLEAR, unit_names[unit], ENTER).noerr();

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
    }

    test(CLEAR, "DEG", ENTER).noerr();
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
        "1/3",          "3.33333 33333 33333 3333⁳⁻¹",
        "-1/7",         "-1.42857 14285 71428 5714⁳⁻¹",
        "22/7",         "3.14285 71428 57142 8571",
        "37/213",       "1.73708 92018 77934 2723⁳⁻¹"
    };

    begin("Simple conversion to decimal and back");
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

    step("Complex numbers");
    test(CLEAR, "1-2ⅈ 4", ENTER, DIV).expect("1/4-1/2ⅈ");
    test("→Num", ENTER).expect("0.25-0.5ⅈ");
    test("→Q", ENTER).expect("1/4-1/2ⅈ");

    step("Vectors");
    test(CLEAR, "[1-2ⅈ 3] 4", ENTER, DIV).expect("[ 1/4-1/2ⅈ 3/4 ]");
    test("→Num", ENTER).expect("[ 0.25-0.5ⅈ 0.75 ]");
    test("→Q", ENTER).expect("[ 1/4-1/2ⅈ 3/4 ]");
}


void tests::complex_types()
// ----------------------------------------------------------------------------
//   Complex data types
// ----------------------------------------------------------------------------
{
    begin("Complex types");
    step("Select degrees for the angle");
    test(CLEAR, "DEG", ENTER).noerr();

    step("Integer rectangular form");
    test(CLEAR, "0ⅈ0", ENTER)
        .type(object::ID_rectangular).expect("0+0ⅈ");
    test(CLEAR, "1ⅈ2", ENTER)
        .type(object::ID_rectangular).expect("1+2ⅈ");
    test(CLEAR, "3+ⅈ4", ENTER)
        .type(object::ID_rectangular).expect("3+4ⅈ");

    step("Behaviour of CHS on command-line");
    test(CLEAR, "4+ⅈ5", CHS, ENTER)
        .type(object::ID_rectangular).expect("4-5ⅈ");
    test(CLEAR, "5", CHS, "ⅈ6", CHS, ENTER)
        .type(object::ID_rectangular).expect("-5-6ⅈ");
    test(CLEAR, "6+7ⅈ", ENTER)
        .type(object::ID_rectangular).expect("6+7ⅈ");
    test(CLEAR, "7-8ⅈ", ENTER)
        .type(object::ID_rectangular).expect("7-8ⅈ");

    step("Integer polar form");
    test(CLEAR, "0∡0", ENTER)
        .type(object::ID_polar).expect("0∡0°");
    test(CLEAR, "1∡90", ENTER)
        .type(object::ID_polar).expect("1∡90°");
    test(CLEAR, "1∡-90", ENTER)
        .type(object::ID_polar).expect("1∡-90°");
    test(CLEAR, "-1∡0", ENTER)
        .type(object::ID_polar).expect("1∡180°");

    step("Decimal rectangular form");
    test(CLEAR, "0.1ⅈ2.3", ENTER)
        .type(object::ID_rectangular).expect("0.1+2.3ⅈ");
    test(CLEAR, "0.1ⅈ2.3", CHS, ENTER)
        .type(object::ID_rectangular).expect("0.1-2.3ⅈ");

    step("Decimal polar form");
    test(CLEAR, "0.1∡2.3", ENTER)
        .type(object::ID_polar).expect("0.1∡2.3°");
    test(CLEAR, "0.1∡2.3", CHS, ENTER)
        .type(object::ID_polar).expect("0.1∡-2.3°");

    step("Symbolic rectangular form");
    test(CLEAR, "aⅈb", ENTER)
        .type(object::ID_rectangular).expect("a+bⅈ");
    test(CLEAR, "c+dⅈ", ENTER)
        .type(object::ID_rectangular).expect("c+dⅈ");

    step("Symbolic polar form");
    test(CLEAR, "a∡b", ENTER)
        .type(object::ID_polar).expect("a∡b");
    test(CLEAR, "c∡d", ENTER)
        .type(object::ID_polar).expect("c∡d");

    step("Polar angle conversions");
    test(CLEAR, "1∡90", ENTER).expect("1∡90°");
    test("GRAD", ENTER).expect("1∡100ℊ");
    test("PiRadians", ENTER).expect("1∡1/2π");
    test("RAD", ENTER).expect("1∡1.57079 63267 94896 6192ℼ");
}


void tests::complex_arithmetic()
// ----------------------------------------------------------------------------
//   Complex arithmetic operations
// ----------------------------------------------------------------------------
{
    begin("Complex arithmetic");

    step("Use degrees");
    test("DEG", ENTER).noerr();

    step("Addition");
    test(CLEAR, "1ⅈ2", ENTER, "3+ⅈ4", ENTER, ADD)
        .type(object::ID_rectangular).expect("4+6ⅈ");
    step("Subtraction");
    test("1-2ⅈ", SUB)
        .type(object::ID_rectangular).expect("3+8ⅈ");
    step("Multiplication");
    test("7+8ⅈ", MUL)
        .type(object::ID_rectangular).expect("-43+80ⅈ");
    step("Division");
    test("7+8ⅈ", DIV)
        .type(object::ID_rectangular).expect("3+8ⅈ");
    test("2+3ⅈ", DIV)
        .type(object::ID_rectangular).expect("30/13+7/13ⅈ");
    test("2+3ⅈ", MUL)
        .type(object::ID_rectangular).expect("3+8ⅈ");
    step("Power");
    test("5", SHIFT, B)
        .type(object::ID_rectangular).expect("44 403-10 072ⅈ");

    step("Symbolic addition");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", ADD)
        .expect("'a+c'+'b+d'ⅈ");
    step("Symbolic subtraction");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", SUB)
        .expect("'a-c'+'b-d'ⅈ");
    step("Symbolic multiplication");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", MUL)
        .expect("'a×c-b×d'+'a×d+b×c'ⅈ");
    step("Symbolic division");
    test(CLEAR, "a+bⅈ", ENTER, "c+dⅈ", DIV)
        .expect("'(a×c+b×d)÷(c²+d²)'+'(b×c-a×d)÷(c²+d²)'ⅈ");

    step("Addition in polar form");
    test(CLEAR, "1∡2", ENTER, "3∡4", ENTER, ADD)
        .expect("3.99208 29777 98568 4728+2.44168 91793 48768 7397⁳⁻¹ⅈ");
    step("Subtraction");
    test("1∡2", SUB)
        .expect("2.99269 21507 79472 7428+2.09269 42123 23759 0233⁳⁻¹ⅈ");
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
    test("5", SHIFT, B)
        .expect("243.∡20.°");

    step("Symbolic addition");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, ADD)
        .expect("'a×cos b+c×cos d'+'a×sin b+c×sin d'ⅈ");
    step("Symbolic subtraction");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, SUB)
        .expect("'a×cos b-c×cos d'+'a×sin b-c×sin d'ⅈ");
    step("Symbolic multiplication");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, MUL)
        .expect("'a×c'∡'b+d'");
    step("Symbolic division");
    test(CLEAR, "a∡b", ENTER, "c∡d", ENTER, DIV)
        .expect("'a÷c'∡'b-d'");

    step("Precedence of complex numbers during rendering");
    test(CLEAR, "'2+3ⅈ' '3∡4' *", ENTER)
        .expect("'(2+3ⅈ)×(3∡4°)'");
    test(CLEAR, "'2+3ⅈ' '3∡4' +", ENTER)
        .expect("'(2+3ⅈ)+(3∡4°)'");
    test(CLEAR, "'2+3ⅈ' '3∡4' -", ENTER)
        .expect("'(2+3ⅈ)-(3∡4°)'");

    step("Do not promote symbols to complex");
    test(CLEAR, "2+3ⅈ 'A' +", ENTER)
        .expect("'(2+3ⅈ)+A'");
}


void tests::complex_functions()
// ----------------------------------------------------------------------------
//   Complex functions
// ----------------------------------------------------------------------------
{
    begin("Complex functions");

    step("Using radians");
    test(CLEAR, "RAD", ENTER).noerr();

    step("Square root");
    test(CLEAR, "-1ⅈ0", ENTER, SQRT)
        .expect("0.+1.ⅈ");

    step("Square and square root");
    test(CLEAR, "1+2ⅈ", ENTER, SHIFT, SQRT)
        .expect("-3+4ⅈ");
    test(SQRT)
        .expect("1.+2.ⅈ");

    step("Negate");
    test(CLEAR, "1+2ⅈ", ENTER, CHS)
        .expect("-1-2ⅈ");
    test(CHS)
        .expect("1+2ⅈ");

    step("Invert");
    test(CLEAR, "3+7ⅈ", ENTER, INV)
        .expect("3/58-7/58ⅈ");
    test("58", MUL)
        .expect("3-7ⅈ");
    test(INV)
        .expect("3/58+7/58ⅈ");

    step("Symbolic sqrt");
    test(CLEAR, "aⅈb", ENTER, SQRT)
        .expect("'√((a⊿b+a)÷2)'+'sign (√((a⊿b-a)÷2))×√((a⊿b-a)÷2)'ⅈ");
    step("Symbolic sqrt in polar form");
    test(CLEAR, "a∡b", ENTER, SQRT)
        .expect("'√ a'∡'b÷2'");

    step("Cubed");
    test(CLEAR, "3+7ⅈ", ENTER, "cubed", ENTER)
        .expect("-414-154ⅈ");
    step("Cube root");
    test("cbrt", ENTER)
        .expect("7.61577 31058 63908 2857∡-9.28490 56188 33822 9639⁳⁻¹ℼ");

    step("Logarithm");
    test(CLEAR, "12+14ⅈ", ENTER, LN)
        .expect("2.91447 28088 05103 5368+8.62170 05466 72263 4884⁳⁻¹ⅈ");
    step("Exponential");
    test("exp", ENTER)
        .expect("18.43908 89145 85774 62∡8.62170 05466 72263 4884⁳⁻¹ℼ");

    step("Power");
    test(CLEAR, "3+7ⅈ", ENTER, "2-3ⅈ", ENTER, SHIFT, B)
        .expect("1 916.30979 15541 96293 8∡2.52432 98723 79583 8639ℼ");

    step("Sine");
    test(CLEAR, "4+2ⅈ", ENTER, SIN)
        .expect("-2.84723 90868 48827 8827-2.37067 41693 52001 6145ⅈ");

    step("Cosine");
    test(CLEAR, "3+11ⅈ", ENTER, COS)
        .expect("-29 637.47552 74860 62145-4 224.71967 95347 02126ⅈ");

    step("Tangent");
    test(CLEAR, "2+8ⅈ", ENTER, TAN)
        .expect("1.39772 11770 40026 1373⁳⁻⁴-1.00030 51824 41239 0233ⅈ");

    step("Arc sine");
    test(CLEAR, "3+5ⅈ", ENTER, SHIFT, SIN)
        .expect("5.33999 06959 41686 1164⁳⁻¹+2.45983 15216 23434 5129ⅈ");

    step("Arc cosine");
    test(CLEAR, "7+11ⅈ", ENTER, SHIFT, COS)
        .expect("1.00539 67973 35154 2326-3.26167 13063 80062 6275ⅈ");

    step("Arc tangent");
    test(CLEAR, "9.+2ⅈ", ENTER, SHIFT, TAN)
        .expect("1.35459 24390 09627 6993+2.32726 05766 50298 8381⁳⁻²ⅈ");

    step("Hyperbolic sine");
    test(CLEAR, "4+2ⅈ", ENTER, "SINH", ENTER)
        .expect("-11.35661 27112 18172 906+24.83130 58489 46379 372ⅈ");

    step("Hyperbolic cosine");
    test(CLEAR, "3+11ⅈ", ENTER, "COSH", ENTER)
        .expect("4.43360 88910 78241 4161⁳⁻²-10.06756 33986 40475 46ⅈ");

    step("Hyperbolic tangent");
    test(CLEAR, "2+8ⅈ", ENTER, "TANH", ENTER)
        .expect("1.03564 79469 63237 6354-1.09258 84335 75253 1964⁳⁻²ⅈ");

    step("Hyperbolic arc sine");
    test(CLEAR, "3+5ⅈ", ENTER, SHIFT, "ASINH", ENTER)
        .expect("2.45291 37425 02811 7695+1.02382 17465 11782 9101ⅈ");

    step("Hyperbolic arc cosine");
    test(CLEAR, "7+11ⅈ", ENTER, SHIFT, "ACOSH", ENTER)
        .expect("3.26167 13063 80062 6275+1.00539 67973 35154 2326ⅈ");

    step("Hyperbolic arc tangent");
    test(CLEAR, "9.+2ⅈ", ENTER, SHIFT, "ATANH", ENTER)
        .expect("1.06220 79849 13164 9131⁳⁻¹+1.54700 47751 56404 9213ⅈ");

    step("Real to complex");
    test(CLEAR, "1 2 R→C", ENTER)
        .type(object::ID_rectangular).expect("1+2ⅈ");
    test(CLEAR, "a b R→C", ENTER)
        .type(object::ID_rectangular).expect("'a'+'b'ⅈ");

    step("Complex to real");
    test(CLEAR, "1+2ⅈ C→R", ENTER)
        .expect("2").test(BSP).expect("1");
    test(CLEAR, "a+bⅈ C→R", ENTER)
        .expect("b").test(BSP).expect("a");

    step("Re function");
    test(CLEAR, "33+22ⅈ Re", ENTER).expect("33");
    test(CLEAR, "a+bⅈ Re", ENTER).expect("a");
    test(CLEAR, "31 Re", ENTER).expect("31");
    test(CLEAR, "31.234 Re", ENTER).expect("31.234");

    step("Im function");
    test(CLEAR, "33+22ⅈ Im", ENTER).expect("22");
    test(CLEAR, "a+bⅈ Im", ENTER).expect("b");
    test(CLEAR, "31 Im", ENTER).expect("0");
    test(CLEAR, "31.234 Im", ENTER).expect("0");

    step("Complex modulus");
    test(CLEAR, "3+4ⅈ abs", ENTER).expect("5.");
    test(CLEAR, "a+bⅈ abs", ENTER).expect("'a⊿b'");
    test(CLEAR, "3+4ⅈ norm", ENTER).expect("5.");
    test(CLEAR, "a+bⅈ norm", ENTER).expect("'a⊿b'");
    test(CLEAR, "3+4ⅈ modulus", ENTER).expect("5.");
    test(CLEAR, "a+bⅈ modulus", ENTER).expect("'a⊿b'");

    step("Complex argument");
    test(CLEAR, "1+1ⅈ arg", ENTER).expect("7.85398 16339 74483 0962⁳⁻¹");
    test(CLEAR, "a+bⅈ arg", ENTER).expect("'b∠a'");
    test(CLEAR, "31 arg", ENTER).expect("0");
    test(CLEAR, "31.234 arg", ENTER).expect("0");

    step("Complex conjugate");
    test(CLEAR, "3+4ⅈ conj", ENTER).expect("3-4ⅈ");
    test(CLEAR, "a+bⅈ conj", ENTER).expect("a+'-b'ⅈ");
    test(CLEAR, "31 conj", ENTER).expect("31");
    test(CLEAR, "31.234 conj", ENTER).expect("31.234");
}


void tests::list_functions()
// ----------------------------------------------------------------------------
//   Some operations on lists
// ----------------------------------------------------------------------------
{
    begin("List operations");
    step("Integer index");
    test(CLEAR, "{ A B C }", ENTER, "2 GET", ENTER)
        .expect("B");
    step("Real index");
    test(CLEAR, "{ A B C }", ENTER, "2.3 GET", ENTER)
        .expect("B");
    step("Bad index type");
    test(CLEAR, "{ A B C }", ENTER, "\"A\" GET", ENTER)
        .error("Bad argument type");
    step("Out-of-range index");
    test(CLEAR, "{ A B C }", ENTER, "5 GET", ENTER)
        .error("Index out of range");
    step("Empty list index");
    test(CLEAR, "{ A B C }", ENTER, "{} GET", ENTER)
        .expect("{ A B C }");
    step("Single element list index");
    test(CLEAR, "{ A B C }", ENTER, "{2} GET", ENTER)
        .expect("B");
    step("List index nested");
    test(CLEAR, "{ A {D E F} C }", ENTER, "{2 3} GET", ENTER)
        .expect("F");
    step("List index, too many items");
    test(CLEAR, "{ A B C }", ENTER, "{2 3} GET", ENTER)
        .error("Bad argument type");
    step("Character from array");
    test(CLEAR, "\"Hello World\"", ENTER, "2 GET", ENTER)
        .expect("\"e\"");
    step("Deep nesting");
    test(CLEAR, "{ A { D E { 1 2 \"Hello World\" } F } 2 3 }", ENTER,\
         "{ 2 3 3 5 } GET", ENTER)
        .expect("\"o\"");

    step("Array indexing");
    test(CLEAR, "[ A [ D E [ 1 2 \"Hello World\" ] F ] 2 3 ]", ENTER,\
         "[ 2 3 3 5 ] GET", ENTER)
        .expect("\"o\"");

    step("Concatenation of lists");
    test(CLEAR, "{ A B C D } { F G H I } +", ENTER)
        .expect("{ A B C D F G H I }");
    step("Concatenation of item to list");
    test(CLEAR, "{ A B C D } 2.3 +", ENTER)
        .expect("{ A B C D 2.3 }");
    test(CLEAR, "2.5 { A B C D } +", ENTER)
        .expect("{ 2.5 A B C D }");

    step("Repetition of a list");
    test(CLEAR, "{ A B C D } 3 *", ENTER)
        .expect("{ A B C D A B C D A B C D }");
    test(CLEAR, "3 { A B C D } *", ENTER)
        .expect("{ A B C D A B C D A B C D }");

    step("Applying a function to a  list");
    test(CLEAR, "{ A B C } sin", ENTER)
        .expect("{ 'sin A' 'sin B' 'sin C' }");
}


void tests::text_functions()
// ----------------------------------------------------------------------------
//   Some operations on text
// ----------------------------------------------------------------------------
{
    begin("Text operations");
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
}


void tests::vector_functions()
// ----------------------------------------------------------------------------
//   Test operations on vectors
// ----------------------------------------------------------------------------
{
    begin("Vectors");

    step("Data entry in numeric form");
    test(CLEAR, "[  1  2  3  ]", ENTER)
        .type(object::ID_array).expect("[ 1 2 3 ]");
    test(CLEAR, "[  1.5  2.300  3.02  ]", ENTER)
        .type(object::ID_array).expect("[ 1.5 2.3 3.02 ]");

    step("Symbolic vector");
    test(CLEAR, "[a b c]", ENTER)
        .expect("[ a b c ]");

    step("Non-homogneous data types");
    test(CLEAR, "[  \"ABC\"  'X' 3/2  ]", ENTER)
        .type(object::ID_array).expect("[ \"ABC\" 'X' 3/2 ]");

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
        .expect("[ 'a×d' 'b×e' 'c×f' ]");

    step("Division (extension)");
    test(CLEAR, "[1 2  3 4 6][4 5 2 1 3] /", ENTER)
        .expect("[ 1/4 2/5 3/2 4 2 ]");
    test(CLEAR, "[a b c][d e f] /", ENTER)
        .expect("[ 'a÷d' 'b÷e' 'c÷f' ]");

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
        .expect("[ 'a×x' 'b×x' 'c×x' ]");
    test(CLEAR, "x [a b c] *", ENTER)
        .expect("[ 'x×a' 'x×b' 'x×c' ]");

    step("Division by constant (extension)");
    test(CLEAR, "[a b c] x /", ENTER)
        .expect("[ 'a÷x' 'b÷x' 'c÷x' ]");
    test(CLEAR, "x [a b c] /", ENTER)
        .expect("[ 'x÷a' 'x÷b' 'x÷c' ]");

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
        .expect("[ 1 1/2 1/3 ]");

    step("Froebenius norm");
    test(CLEAR, "[1 2 3] ABS", ENTER)
        .expect("3.74165 73867 73941 3856");
    test(CLEAR, "[1 2 3] NORM", ENTER)
        .expect("3.74165 73867 73941 3856");

    step("Component-wise application of functions");
    test(CLEAR, "[a b c] SIN", ENTER)
        .expect("[ 'sin a' 'sin b' 'sin c' ]");
}


void tests::matrix_functions()
// ----------------------------------------------------------------------------
//   Test operations on vectors
// ----------------------------------------------------------------------------
{
    begin("Matrices");

    step("Data entry in numeric form");
    test(CLEAR, "[  [1  2  3][4 5 6]  ]", ENTER)
        .type(object::ID_array).expect("[ [ 1 2 3 ] [ 4 5 6 ] ]");

    step("Non-rectangular matrices");
    test(CLEAR, "[  [ 1.5  2.300 ] [ 3.02 ] ]", ENTER)
        .type(object::ID_array).expect("[ [ 1.5 2.3 ] [ 3.02 ] ]");

    step("Symbolic matrix");
    test(CLEAR, "[ [a b] [c d] ]", ENTER)
        .expect("[ [ a b ] [ c d ] ]");

    step("Non-homogneous data types");
    test(CLEAR, "[  [ \"ABC\"  'X' ] 3/2  [ 4 [5] [6 7] ] ]", ENTER)
        .type(object::ID_array)
        .expect("[ [ \"ABC\" 'X' ] 3/2 [ 4 [ 5 ] [ 6 7 ] ] ]");

    step("Addition");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] +", ENTER)
        .expect("[ [ 6 8 ] [ 10 12 ] ]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] +", ENTER)
        .expect("[ [ 'a+e' 'b+f' ] [ 'c+g' 'd+h' ] ]");

    step("Subtraction");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] -", ENTER)
        .expect("[ [ -4 -4 ] [ -4 -4 ] ]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] -", ENTER)
        .expect("[ [ 'a-e' 'b-f' ] [ 'c-g' 'd-h' ] ]");

    step("Multiplication (square)");
    test(CLEAR, "[[1 2] [3 4]] [[5 6][7 8]] *", ENTER)
        .expect("[ [ 19 22 ] [ 43 50 ] ]");
    test(CLEAR, "[[a b][c d]] [[e f][g h]] *", ENTER)
        .expect("[ [ 'a×e+b×g' 'a×f+b×h' ] [ 'c×e+d×g' 'c×f+d×h' ] ]");

    step("Multiplication (non-square)");
    test(CLEAR, "[[1 2 3] [4 5 6]] [[5 6][7 8][9 10]] *", ENTER)
        .expect("[ [ 46 52 ] [ 109 124 ] ]");
    test(CLEAR, "[[a b c d][e f g h]] [[x][y][z][t]] *", ENTER)
        .expect("[ [ 'a×x+b×y+c×z+d×t' ] [ 'e×x+f×y+g×z+h×t' ] ]");
    test(CLEAR, "[[a b c d][e f g h]] [x y z t] *", ENTER)
        .expect("[ 'a×x+b×y+c×z+d×t' 'e×x+f×y+g×z+h×t' ]");

    step("Division");
    test(CLEAR,
         "[[5 12 1968][17 2 1969][30 3 1993]] "
         "[[16 5 1995][21 5 1999][28 5 2009]] /", ENTER)
        .expect("[ [ 34/11 -52/11 -43/11 ] [ 3 357/10 -13 427/10 -16 433/10 ] [ -19/22 75/22 113/22 ] ]");
    test(CLEAR, "[[a b][c d]][[e f][g h]] /", ENTER)
        .expect("[ [ '(1÷e-f÷e×((e×0-g×1)÷(e×h-g×f)))×a+(0÷e-f÷e×((e×1-g×0)÷(e×h-g×f)))×c' '(1÷e-f÷e×((e×0-g×1)÷(e×h-g×f)))×b+(0÷e-f÷e×((e×1-g×0)÷(e×h-g×f)))×d' ] [ '(e×0-g×1)÷(e×h-g×f)×a+(e×1-g×0)÷(e×h-g×f)×c' '(e×0-g×1)÷(e×h-g×f)×b+(e×1-g×0)÷(e×h-g×f)×d' ] ]");

    step("Addition of constant (extension)");
    test(CLEAR, "[[1 2] [3 4]] 3 +", ENTER)
        .expect("[ [ 4 5 ] [ 6 7 ] ]");
    test(CLEAR, "[[a b] [c d]] x +", ENTER)
        .expect("[ [ 'a+x' 'b+x' ] [ 'c+x' 'd+x' ] ]");

    step("Subtraction of constant (extension)");
    test(CLEAR, "[[1 2] [3 4]] 3 -", ENTER)
        .expect("[ [ -2 -1 ] [ 0 1 ] ]");
    test(CLEAR, "[[a b] [c d]] x -", ENTER)
        .expect("[ [ 'a-x' 'b-x' ] [ 'c-x' 'd-x' ] ]");

    step("Multiplication by constant (extension)");
    test(CLEAR, "[[a b] [c d]] x *", ENTER)
        .expect("[ [ 'a×x' 'b×x' ] [ 'c×x' 'd×x' ] ]");
    test(CLEAR, "x [[a b] [c d]] *", ENTER)
        .expect("[ [ 'x×a' 'x×b' ] [ 'x×c' 'x×d' ] ]");

    step("Division by constant (extension)");
    test(CLEAR, "[[a b] [c d]] x /", ENTER)
        .expect("[ [ 'a÷x' 'b÷x' ] [ 'c÷x' 'd÷x' ] ]");
    test(CLEAR, "x [[a b] [c d]] /", ENTER)
        .expect("[ [ 'x÷a' 'x÷b' ] [ 'x÷c' 'x÷d' ] ]");

    step("Invalid dimension for binary operations");
    test(CLEAR, "[[1 2] [3 4]][1 2] +", ENTER)
        .error("Bad argument type");
    test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] +", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][1 2] +", ENTER)
        .error("Bad argument type");
    test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] -", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][1 2] +", ENTER)
        .error("Bad argument type");
    test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] -", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][1 2] +", ENTER)
        .error("Bad argument type");
    test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] *", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][1 2 3] *", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][[1 2][3 4][5 6]] /", ENTER)
        .error("Invalid dimension");
    test(CLEAR, "[[1 2] [3 4]][1 2] /", ENTER)
        .error("Bad argument type");

    step("Inversion of a definite matrix");
    test(CLEAR, "[[1 2 3][4 5 6][7 8 19]] INV", ENTER)
        .expect("[ [ -47/30 7/15 1/10 ] [ 17/15 1/15 -1/5 ] [ 1/10 -1/5 1/10 ] ]");
    test(CLEAR, "[[a b][c d]] INV", ENTER)
        .expect("[ [ '1÷a-b÷a×((a×0-c×1)÷(a×d-c×b))' '0÷a-b÷a×((a×1-c×0)÷(a×d-c×b))' ] [ '(a×0-c×1)÷(a×d-c×b)' '(a×1-c×0)÷(a×d-c×b)' ] ]");

    step("Invert with zero determinant");       // HP48 gets this one wrong
    test(CLEAR, "[[1 2 3][4 5 6][7 8 9]] INV", ENTER)
        .error("Divide by zero");

    step("Determinant");                        // HP48 gets this one wrong
    test(CLEAR, "[[1 2 3][4 5 6][7 8 9]] DET", ENTER)
        .expect("0");
    test(CLEAR, "[[1 2 3][4 5 6][7 8 19]] DET", ENTER)
        .expect("-30");

    step("Froebenius norm");
    test(CLEAR, "[[1 2] [3 4]] ABS", ENTER)
        .expect("5.47722 55750 51661 1346");
    test(CLEAR, "[[1 2] [3 4]] NORM", ENTER)
        .expect("5.47722 55750 51661 1346");

    step("Component-wise application of functions");
    test(CLEAR, "[[a b] [c d]] SIN", ENTER)
        .expect("[ [ 'sin a' 'sin b' ] [ 'sin c' 'sin d' ] ]");
}


void tests::auto_simplification()
// ----------------------------------------------------------------------------
//   Check auto-simplification rules for arithmetic
// ----------------------------------------------------------------------------
{
    begin("Auto-simplification of expressions");

    step("Enable auto simplification");
    test(CLEAR, "AutoSimplify", ENTER).noerr();

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

    step("Applies when building a matrix");
    test(CLEAR, "[[3 0 2][2 0 -2][ 0 1 1 ]] [x y z] *", ENTER)
        .expect("[ '3×x+2×z' '2×x+-2×z' 'y+z' ]");

    step("Does not reduce matrices");
    test(CLEAR, "[a b c] 0 *", ENTER).expect("[ 0 0 0 ]");

    step("Does not apply to text");
    test(CLEAR, "\"Hello\" 0 +", ENTER)
        .expect("\"Hello0\"");

    step("Does not apply to lists");
    test(CLEAR, "{ 1 2 3 } 0 +", ENTER)
        .expect("{ 1 2 3 0 }");

    step("Disable auto simplification");
    test(CLEAR, "NoAutoSimplify", ENTER).noerr();

    step("When disabled, get the complicated expression");
    test(CLEAR, "[[3 0 2][2 0 -2][ 0 1 1 ]] [x y z] *", ENTER)
        .expect("[ '3×x+0×y+2×z' '2×x+0×y+-2×z' '0×x+1×y+1×z' ]");

    step("Re-enable auto simplification");
    test(CLEAR, "AutoSimplify", ENTER).noerr();
}


void tests::rewrite_engine()
// ----------------------------------------------------------------------------
//   Equation rewrite engine
// ----------------------------------------------------------------------------
{
    begin("Equation rewrite engine");

    step("Single replacement");
    test(CLEAR, "'A+B' 'X+Y' 'Y-sin X' rewrite", ENTER)
        .expect("'B-sin A'");

    step("In-depth replacement");
    test(CLEAR, " 'A*(B+C)' 'X+Y' 'Y-sin X' rewrite", ENTER)
        .expect("'A×(C-sin B)'");

    step("Variable matching");
    test(CLEAR, "'A*(B+C)' 'X+X' 'X-sin X' rewrite", ENTER)
        .expect("'A×(B+C)'");
    test(CLEAR, "'A*(B+(B))' 'X+X' 'X-sin X' rewrite", ENTER)
        .expect("'A×(B-sin B)'");

    step("Constant folding");
    test(CLEAR, "'A+B+0' 'X+0' 'X' rewrite", ENTER)
        .expect("'A+B'");
    step("Multiple substitutions");
    test(CLEAR, "'A+B+C' 'X+Y' 'Y-X' rewrite", ENTER)
        .expect("'C-(B-A)'");

    step("Deep substitution");
    test(CLEAR, "'tan(A-B)+3' 'A-B' '-B+A' rewrite", ENTER)
        .expect("'tan(-B+A)+3'");
    step("Deep substitution with multiple changes");
    test(CLEAR, "'5+tan(A-B)+(3-sin(C+D-A))' 'A-B' '-B+A' rewrite", ENTER)
        .expect("'5+tan(-B+A)+(-sin(-A+(C+D))+3)'");

    step("Matching integers");
    test(CLEAR, "'(A+B)^3' 'X^N' 'X*X^(N-1)' rewrite", ENTER)
        .expect("'(A+B)×((A+B)×((A+B)×(A+B)↑0))'");

    step("Matching unique terms");
    test(CLEAR, "'(A+B+A)' 'X+U+X' '2*X+U' rewrite", ENTER)
        .expect("'2×A+B'");
    test(CLEAR, "'(A+A+A)' 'X+U+X' '2*X+U' rewrite", ENTER)
        .expect("'A+A+A'");
}


void tests::expand_collect_simplify()
// ----------------------------------------------------------------------------
//   Equation rewrite engine
// ----------------------------------------------------------------------------
{
    begin("Expand");

    step("Single add, right");
    test(CLEAR, "'(A+B)*C' expand ", ENTER)
        .expect("'A×C+B×C'");
    step("Single add, left");
    test(CLEAR, "'2*(A+B)' expand ", ENTER)
        .expect("'2×A+2×B'");

    step("Multiple adds");
    test(CLEAR, "'3*(A+B+C)' expand ", ENTER)
        .expect("'3×A+3×B+3×C'");

    step("Single sub, right");
    test(CLEAR, "'(A-B)*C' expand ", ENTER)
        .expect("'A×C-B×C'");
    step("Single sub, left");
    test(CLEAR, "'2*(A-B)' expand ", ENTER)
        .expect("'2×A-2×B'");

    step("Multiple subs");
    test(CLEAR, "'3*(A-B-C)' expand ", ENTER)
        .expect("'3×A-3×B-3×C'");

    step("Expand and collect a power");
    test(CLEAR, "'(A+B)^3' expand ", ENTER)
        .expect("'A×A×A+A×B×A+A×A×B+A×B×B+B×A×A+B×B×A+B×A×B+B×B×B'");
    test("collect ", ENTER)
        .expect("'2×(B↑2×A)+(2×(A↑2×B)+A↑3+B↑2×A+A↑2×B)+B↑3'");
    // .expect("'(A+B)³'");


}


void tests::regression_checks()
// ----------------------------------------------------------------------------
//   Checks for specific regressions
// ----------------------------------------------------------------------------
{
    Settings = settings();

    begin("Regression checks");
    step("Bug 116: Rounding of gamma(7) and gamma(8)");
    test(CLEAR, "7 gamma", ENTER).expect("720.");
    test(CLEAR, "8 gamma", ENTER).expect("5 040.");

    step("Bug 168: pi no longer parses correctly");
    test(CLEAR, "pi", ENTER).expect("π");
    test(DOWN).editor("π");
    test(ENTER).expect("π");

    step("Bug 207: parsing of cos(X+pi)");
    test(CLEAR, "'COS(X+π)'", ENTER).expect("'cos(X+π)'");

    step("Bug 238: Parsing of power");
    test(CLEAR, "'X↑3'", ENTER).expect("'X↑3'");
    test(CLEAR, "'X×X↑(N-1)'", ENTER).expect("'X×X↑(N-1)'");

    step("Bug 253: Complex cos outside domain");
    test(CLEAR, "0+30000.ⅈ sin", ENTER).error("Argument outside domain");
    test(CLEAR, "0+30000.ⅈ cos", ENTER).error("Argument outside domain");
    test(CLEAR, "0+30000.ⅈ tan", ENTER).error("Argument outside domain");

    step("Bug 272: Type error on logical operations");
    test(CLEAR, "'x' #2134AF AND", ENTER).error("Bad argument type");
}


// ============================================================================
//
//   Sequencing tests
//
// ============================================================================

static void passfail(bool ok)
// ----------------------------------------------------------------------------
//   Print a pass/fail message
// ----------------------------------------------------------------------------
{
#define GREEN "\033[32m"
#define RED   "\033[41;97m"
#define RESET "\033[39;49;99;27m"
    fprintf(stderr, "%s\n", ok ? GREEN "[PASS]" RESET : RED "[FAIL]" RESET);
#undef GREEN
#undef RED
#undef RESET
}

tests &tests::begin(cstring name)
// ----------------------------------------------------------------------------
//   Beginning of a test
// ----------------------------------------------------------------------------
{
    if (sindex)
    {
        passfail(ok);
        if (!ok)
            show(failures.back());
    }

    tname = name;
    tindex++;
    fprintf(stderr, "%3u: %s\n", tindex, tname);
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
        passfail(ok);
        if (!ok)
            show(failures.back());
    }
    fprintf(stderr, "%3u:  %03u: %-60s", tindex, sindex, sname);
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
    ok = false;
    return *this;
}


tests &tests::summary()
// ----------------------------------------------------------------------------
//   Summarize the test results
// ----------------------------------------------------------------------------
{
    if (sindex)
        passfail(ok);

    if (failures.size())
    {
        fprintf(stderr, "Summary of %zu failures:\n", failures.size());
        cstring last = nullptr;
        uint    line = 0;
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
    cstring last = nullptr;
    uint    line = 0;
    return show(f, last, line);
}


tests &tests::show(tests::failure &f, cstring &last, uint &line)
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
                f.test);
        last = f.test;
    }
    fprintf(stderr,
            "%s:%d: %3u:%03u.%03u: %s\n",
            f.file,
            f.line,
            f.tindex,
            f.sindex,
            f.cindex,
            f.step);
    fprintf(stderr, "%s\n", f.explanation.c_str());
    return *this;
}


// ============================================================================
//
//   Utilities to build the tests
//
// ============================================================================

tests &tests::itest(tests::key k, bool release)
// ----------------------------------------------------------------------------
//   Type a given key directly
// ----------------------------------------------------------------------------
{
    extern int key_remaining();

    // Check for special key sequences
    switch (k)
    {
    case ALPHA: return shifts(false, false, true, false);

    case LOWERCASE: return shifts(false, false, true, true);

    case LONGPRESS:
        longpress = true; // Next key will be a long press
        return *this;

    case CLEAR: return clear();

    case NOKEYS: return nokeys();

    case REFRESH: return refreshed();

    default: break;
    }


    // Wait for the RPL thread to process the keys (to be revisited on DM42)
    while (!key_empty())
        sys_delay(delay_time);

    record(tests,
           "Push key %d update %u->%u last %d",
           k, lcd_update, lcd_needsupdate, last_key);
    lcd_update = lcd_needsupdate;
    Stack.catch_up();
    last_key = k;

    key_push(k);
    if (longpress)
    {
        sys_delay(600);
        longpress = false;
        release   = false;
    }
    sys_delay(delay_time);

    if (release && k != RELEASE)
    {
        while (!key_remaining())
            sys_delay(delay_time);
        record(tests,
               "Release key %d update %u->%u last %d",
               k, lcd_update, lcd_needsupdate, last_key);
        lcd_update = lcd_needsupdate;
        Stack.catch_up();
        last_key = -k;
        key_push(RELEASE);

        // Wait for the RPL thread to process the keys
        keysync_sent++;
        record(tests, "Key sync sent %u done %u", keysync_sent, keysync_done);
        key_push(KEYSYNC);
        while (keysync_done != keysync_sent)
            sys_delay(delay_time);
        record(tests, "Key sync done %u sent %u", keysync_done, keysync_sent);
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
    itest(cstring(buffer));
    return shifts(false, false, false, false);
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
    itest(cstring(buffer));
    return shifts(false, false, false, false);
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
    itest(cstring(buffer));
    return shifts(false, false, false, false);
}


tests &tests::itest(long value)
// ----------------------------------------------------------------------------
//   Test a signed numerical value
// ----------------------------------------------------------------------------
{
    if (value < 0)
        return itest(-value, CHS);
    else
        return itest(value);
}


tests &tests::itest(char c)
// ----------------------------------------------------------------------------
//   Type the character on the calculator's keyboard
// ----------------------------------------------------------------------------
{
    const char buf[] = { c, 0 };
    return itest(buf);
}


tests &tests::itest(cstring txt)
// ----------------------------------------------------------------------------
//   Type the string on the calculator's keyboard
// ----------------------------------------------------------------------------
{
    utf8 u = utf8(txt);

    while (*u)
    {
        unicode c = utf8_codepoint(u);
        u         = utf8_next(u);

        nokeys();

        bool alpha  = ui.alpha;
        bool shift  = false;
        bool xshift = false;
        bool lower  = ui.lowercase;
        key  k      = RELEASE;
        key  fn     = RELEASE;
        bool del    = false;
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
        case '+': k = ADD;          alpha = true;  shift = true; break;
        case '-': k = SUB;          alpha = true;  shift = true; break;
        case '*': k = MUL;          alpha = true; xshift = true; break;
        case '/': k = DIV;          alpha = true; xshift = true; break;
        case '.': k = DOT;          shift = alpha; break;
        case ',': k = DOT;          shift = !alpha; break;
        case ' ': k = RUNSTOP;      alpha = true;  break;
        case '?': k = KEY7;         alpha = true; xshift = true; break;
        case '!': k = ADD;          alpha = true; xshift = true; break;
        case '_': k = SUB;          alpha = true;  break;
        case '%': k = RCL;          alpha = true;  shift = true; break;
        case ':': k = KEY0;         alpha = true;  del   = true; break;
        case ';': k = KEY0;         alpha = true; xshift = true;  break;
        case '<': k = SIN;          alpha = true;  shift = true;  break;
        case '=': k = COS;          alpha = true;  shift = true;  break;
        case '>': k = TAN;          alpha = true;  shift = true;  break;
        case '^': k = INV;          alpha = true;  shift = true;  break;
        case '(': k = XEQ;          alpha = true;  shift = true;  del = true; break;
        case ')': k = XEQ;          alpha = true;  shift = true;  bsp = true; break;
        case '[': k = KEY9;         alpha = false; shift = true;  del = true; break;
        case ']': k = KEY9;         alpha = false; shift = true;  bsp = true; break;
        case '{': k = RUNSTOP;      alpha = true; xshift = true;  del = true; break;
        case '}': k = RUNSTOP;      alpha = true; xshift = true;  bsp = true; break;
        case '"': k = ENTER;        alpha = true; xshift = true;  bsp = true; break;
        case '\'': k = XEQ;         alpha = true; xshift = true;  bsp = true; break;
        case '&': k = KEY1;         alpha = true; xshift = true; break;
        case '@': k = KEY2;         alpha = true; xshift = true; break;
        case '$': k = KEY3;         alpha = true; xshift = true; break;
        case '#': k = KEY4;         alpha = true; xshift = true; break;
        case '\\': k = ADD;         alpha = true; xshift = true; break;
        case '\n': k = BSP;         alpha = true; xshift = true; break;
        case L'«': k = RUNSTOP;     alpha = false; shift = true; del = true; break;
        case L'»': k = RUNSTOP;     alpha = false; shift = true; bsp = true; break;
        case L'→': k = STO;         alpha = true; xshift = true; break;
        case L'×': k = MUL;         alpha = true;  shift = true; break;
        case L'÷': k = DIV;         alpha = true;  shift = true; break;
        case L'↑': k = C;           alpha = true; xshift = true; break;
        case L'ⅈ': k = G; fn = F1;  alpha = false;  shift = true; break;
        case L'∡': k = G; fn = F2;  alpha = false; shift = true; break;
        case L'ρ': k = E;           alpha = true;  shift = true; break;
        case L'θ': k = E;           alpha = true; xshift = true; break;
        case L'π': k = I; fn = F1;  alpha = false; shift = true; break;
        }

        if (shift)
            xshift = false;
        else if (xshift)
            shift = false;

        if (k == RELEASE)
        {
            fprintf(stderr, "Cannot translate '%c' (%d)\n", c, c);
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
            else if (del)
                itest(SHIFT, BSP);

            // If we have a follow-up key, use that
            if (fn != RELEASE)
                itest(fn);
        }
    }
    return *this;
}


tests &tests::shifts(bool shift, bool xshift, bool alpha, bool lowercase)
// ----------------------------------------------------------------------------
//   Reach the desired shift state from the current state
// ----------------------------------------------------------------------------
{
    // Must wait for the calculator to process our keys for valid state
    nokeys();

    // Check that we are not displaying an error message
    if (rt.error())
    {
        explain("Unexpected error message [",
                rt.error(),
                "] "
                "during data entry, cleared");
        fail();
        rt.clear_error();
    }

    // Check invalid input: can only have one shift
    if (shift && xshift)
        shift = false;

    // If not alpha, disable lowercase
    if (!alpha)
        lowercase = false;

    // First change lowercase state as necessary, since this messes up shift
    while (lowercase != ui.lowercase || alpha != ui.alpha)
    {
        while (!ui.shift)
            itest(SHIFT, NOKEYS);
        itest(ENTER, NOKEYS);
    }

    while (xshift != ui.xshift)
        itest(SHIFT, NOKEYS);

    while (shift != ui.shift)
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


// ============================================================================
//
//    Test validation
//
// ============================================================================

tests &tests::clear()
// ----------------------------------------------------------------------------
//   Make sure we are in a clean state
// ----------------------------------------------------------------------------
{
    nokeys();
    key_push(CLEAR);
    while (!key_empty())
        sys_delay(delay_time);
    return *this;
}


tests &tests::ready()
// ----------------------------------------------------------------------------
//   Check if the calculator is ready and we can look at it
// ----------------------------------------------------------------------------
{
    nokeys();
    refreshed();
    return *this;
}


tests &tests::nokeys()
// ----------------------------------------------------------------------------
//   Check until the key buffer is empty, indicates that calculator is done
// ----------------------------------------------------------------------------
{
    while (!key_empty())
        sys_delay(delay_time);
    return *this;
}


tests &tests::refreshed()
// ----------------------------------------------------------------------------
//    Wait until the screen was updated by the calculator
// ----------------------------------------------------------------------------
{
    // Wait for a screen redraw
    record(tests, "Waiting for screen update");
    while (lcd_needsupdate == lcd_update)
        sys_delay(delay_time);

    // Wait for a stack update
    uint32_t start = sys_current_ms();
    record(tests, "Waiting for key %d in stack at %u", last_key, start);
    while (sys_current_ms() - start < wait_time)
    {
        if (!Stack.available())
        {
            sys_delay(delay_time);
        }
        else if (Stack.available() > 1)
        {
            record(tests, "Consume extra stack");
            Stack.consume();
        }
        else if (Stack.key() == last_key)
        {
            record(tests, "Consume extra stack");
            break;
        }
        else
        {
            record(tests, "Wrong key %d", Stack.key());
            Stack.consume();
        }
    }

    record(tests,
           "Refreshed, key %d, needs=%u update=%u available=%u",
           Stack.key(),
           lcd_needsupdate,
           lcd_update,
           Stack.available());

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


tests &tests::expect(cstring output)
// ----------------------------------------------------------------------------
//   Check that the output at first level of stack matches the string
// ----------------------------------------------------------------------------
{
    record(tests, "Expecting [%+s]", output);
    ready();
    cindex++;
    if (rt.error())
    {
        explain("Expected output [",
                output,
                "], got error [",
                rt.error(),
                "] instead");
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
        explain("Expected output [",
                output,
                "], "
                "got [",
                cstring(out),
                "] instead");
        return fail();
    }
    record(tests, "No output");
    explain("Expected output [", output, "] but got no stack change");
    return fail();
}


tests &tests::expect(int output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%d", output);
    return expect(num);
}


tests &tests::expect(unsigned int output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%u", output);
    return expect(num);
}


tests &tests::expect(long output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%ld", output);
    return expect(num);
}


tests &tests::expect(unsigned long output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%lu", output);
    return expect(num);
}


tests &tests::expect(long long output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%lld", output);
    return expect(num);
}


tests &tests::expect(unsigned long long output)
// ----------------------------------------------------------------------------
//   Check that the output matches an integer value
// ----------------------------------------------------------------------------
{
    char num[32];
    snprintf(num, sizeof(num), "%llu", output);
    return expect(num);
}


tests &tests::match(cstring restr)
// ----------------------------------------------------------------------------
//   Check that the output at first level of stack matches the string
// ----------------------------------------------------------------------------
{
    ready();
    cindex++;
    if (utf8 out = Stack.recorded())
    {
        regex_t    re;
        regmatch_t rm;

        regcomp(&re, restr, REG_EXTENDED | REG_ICASE);
        bool ok = regexec(&re, cstring(out), 1, &rm, 0) == 0 && rm.rm_so == 0 &&
                  out[rm.rm_eo] == 0;
        regfree(&re);
        if (ok)
            return *this;
        explain("Expected output matching [",
                restr,
                "], "
                "got [",
                out,
                "]");
        return fail();
    }
    explain("Expected output matching [", restr, "] but stack not updated");
    return fail();
}


tests &tests::type(object::id ty)
// ----------------------------------------------------------------------------
//   Check that the top of stack matches the type
// ----------------------------------------------------------------------------
{
    ready();
    cindex++;
    if (utf8 out = Stack.recorded())
    {
        object::id tty = Stack.type();
        if (tty == ty)
            return *this;
        explain("Expected type ",
                object::name(ty),
                " (",
                int(ty),
                ")"
                " but got ",
                object::name(tty),
                " (",
                int(tty),
                ")");
        return fail();
    }
    explain("Expected type ",
            object::name(ty),
            " (",
            int(ty),
            ")"
            " but stack not updated");
    return fail();
}


tests &tests::shift(bool s)
// ----------------------------------------------------------------------------
//   Check that the shift state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys();
    return check(ui.shift == s, "Expected shift ", s, ", got ", ui.shift);
}


tests &tests::xshift(bool x)
// ----------------------------------------------------------------------------
//   Check that the right shift state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys();
    return check(ui.xshift == x, "Expected xshift ", x, " got ", ui.xshift);
}


tests &tests::alpha(bool a)
// ----------------------------------------------------------------------------
//   Check that the alpha state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys();
    return check(ui.alpha == a, "Expected alpha ", a, " got ", ui.alpha);
}


tests &tests::lower(bool l)
// ----------------------------------------------------------------------------
//   Check that the lowercase state matches expectations
// ----------------------------------------------------------------------------
{
    nokeys();
    return check(ui.lowercase == l, "Expected alpha ", l, " got ", ui.alpha);
}


tests &tests::editing()
// ----------------------------------------------------------------------------
//   Check that we are editing, without checking the length
// ----------------------------------------------------------------------------
{
    ready();
    return check(rt.editing(),
                 "Expected to be editing, got length ",
                 rt.editing());
}


tests &tests::editing(size_t length)
// ----------------------------------------------------------------------------
//   Check that the editor has exactly the expected length
// ----------------------------------------------------------------------------
{
    ready();
    return check(rt.editing() == length,
                 "Expected editing length to be ",
                 length,
                 " got ",
                 rt.editing());
}


tests &tests::editor(cstring text)
// ----------------------------------------------------------------------------
//   Check that the editor contents matches the text
// ----------------------------------------------------------------------------
{
    ready();
    byte_p ed = rt.editor();
    size_t sz = rt.editing();

    if (!ed)
        return explain("Expected editor to contain [",
                       text,
                       "], "
                       "but it's empty")
            .fail();
    if (sz != strlen(text))
        return explain("Expected ",
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
                       "]")
            .fail();
    if (memcmp(ed, text, sz))
        return explain("Expected editor to contain [",
                       text,
                       "], "
                       "but it contains [",
                       std::string(cstring(ed), sz),
                       "]")
            .fail();

    return *this;
}


tests &tests::cursor(size_t csr)
// ----------------------------------------------------------------------------
//   Check that the cursor is at expected position
// ----------------------------------------------------------------------------
{
    ready();
    return check(ui.cursor == csr,
                 "Expected cursor to be at position ",
                 csr,
                 " but it's at position ",
                 ui.cursor);
}


tests &tests::error(cstring msg)
// ----------------------------------------------------------------------------
//   Check that the error message matches expectations
// ----------------------------------------------------------------------------
{
    ready();
    utf8 err = rt.error();

    if (!msg && err)
        return explain("Expected no error, got [", err, "]")
            .itest(CLEAR).fail();
    if (msg && !err)
        return explain("Expected error message [", msg, "], got none").fail();
    if (msg && err && strcmp(cstring(err), msg) != 0)
        return explain("Expected error message [", msg, "], "
                       "got [", err, "]")
            .fail();
    return *this;
}


tests &tests::command(cstring ref)
// ----------------------------------------------------------------------------
//   Check that the command result matches expectations
// ----------------------------------------------------------------------------
{
    ready();
    utf8 cmd = rt.command();

    if (!ref && cmd)
        return explain("Expected no command, got [", cmd, "]").fail();
    if (ref && !cmd)
        return explain("Expected command [", ref, "], got none").fail();
    if (ref && cmd && strcmp(ref, cstring(cmd)) != 0)
        return explain("Expected command [", ref, "], got [", cmd, "]").fail();

    return *this;
}


tests &tests::source(cstring ref)
// ----------------------------------------------------------------------------
//   Check that the source indicated in the editor matches expectations
// ----------------------------------------------------------------------------
{
    ready();
    utf8 src = rt.source();

    if (!ref && src)
        return explain("Expected no source, got [", src, "]").fail();
    if (ref && !src)
        return explain("Expected source [", ref, "], got none").fail();
    if (ref && src && strcmp(ref, cstring(src)) != 0)
        return explain("Expected source [", ref, "], " "got [", src, "]")
            .fail();

    return *this;
}
