// ****************************************************************************
//  util.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Basic utilities
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

#include "util.h"

#include "dmcp.h"
#include "program.h"
#include "settings.h"
#include "target.h"

#include <algorithm>


void invert_screen()
// ----------------------------------------------------------------------------
//   Invert the screen and refresh it
// ----------------------------------------------------------------------------
{
    Screen.invert();
    lcd_refresh_lines(0, LCD_H);
}


bool exit_key_pressed()
// ----------------------------------------------------------------------------
//   Check if exit key is pressed
// ----------------------------------------------------------------------------
{
    save<bool> nohalt(program::halted, false);
    return program::interrupted();
}


void beep(uint frequency, uint duration)
// ----------------------------------------------------------------------------
//   Emit a short beep
// ----------------------------------------------------------------------------
{
    bool beeping = Settings.BeepOn();
    bool flash = Settings.SilentBeepOn();

    if (beeping)
        start_buzzer_freq(frequency * 1000);
    if (flash)
        invert_screen();
    while (duration > 20 && !exit_key_pressed())
    {
        sys_delay(20);
        duration -= 20;
    }
    if (duration && duration <= 20)
        sys_delay(duration);
    if (beeping)
        stop_buzzer();
    if (flash)
        invert_screen();
}


void click(uint frequency)
// ----------------------------------------------------------------------------
//   A very short beep
// ----------------------------------------------------------------------------
{
    bool silent = Settings.SilentBeepOn();
    Settings.SilentBeepOn(false);
    beep(frequency, 10);
    Settings.SilentBeepOn(silent);
}


bool screenshot()
// ----------------------------------------------------------------------------
//  Take a screenshot
// ----------------------------------------------------------------------------
{
    click(4400);

    // Make screenshot - allow to report errors
    if (create_screenshot(1) == 2)
    {
        // Was error just wait for confirmation
        wait_for_key_press();
        return false;
    }

    // End click
    click(8000);

    return true;
}


void assertion_failed(const char *msg)
// ----------------------------------------------------------------------------
//   Function to make it easier to put a breakpoint somewhere
// ----------------------------------------------------------------------------
{
    record(assert_error, "Assertion failed: %s", msg);
}


char *render_u64(char *buffer, ularge value)
// ----------------------------------------------------------------------------
//   Render an unsigned long-long value in the buffer, returns end address
// ----------------------------------------------------------------------------
//   This is made necessary due to lack of support for %llu in nano-libc
{
    char *ptr = buffer;
    do
    {
        *ptr++ = '0' + value % 10;
        value /= 10;
    } while (value);
    char *last = ptr;
    char *top = buffer;
    while (ptr > top)
        std::swap(*top++, *--ptr);
    return last;
}


char *render_i64(char *buffer, large value)
// ----------------------------------------------------------------------------
//   Render an signed long-long value in the buffer, returns address
// ----------------------------------------------------------------------------
//   This is made necessary due to lack of support for %llu in nano-libc
{
    if (value < 0)
    {
        *buffer = '-';
        return render_u64(buffer+1, ularge(-value));
    }
    return render_u64(buffer, ularge(value));
}
