// ****************************************************************************
//  main.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//      The DB48X main RPL loop
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
//
// This code is distantly derived from the SwissMicro SDKDemo calculator

#include "main.h"

#include "blitter.h"
#include "dmcp.h"
#include "expression.h"
#include "font.h"
#include "program.h"
#include "recorder.h"
#include "stack.h"
#include "sysmenu.h"
#include "target.h"
#include "user_interface.h"
#include "util.h"

#if SIMULATOR
#  include "tests.h"
#endif


#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using std::max;
using std::min;

// ============================================================================
//
// Those are put in the same file to guarantee initialization order
//
// ============================================================================

// Initialize the screen
surface Screen((pixword *) lcd_line_addr(0), LCD_W, LCD_H, LCD_SCANLINE);

// Pre-built patterns for shades of grey
const pattern pattern::black   = pattern(0, 0, 0);
const pattern pattern::gray10  = pattern(32, 32, 32);
const pattern pattern::gray25  = pattern(64, 64, 64);
const pattern pattern::gray50  = pattern(128, 128, 128);
const pattern pattern::gray75  = pattern(192, 192, 192);
const pattern pattern::gray90  = pattern(224, 224, 224);
const pattern pattern::white   = pattern(255, 255, 255);
const pattern pattern::invert  = pattern(~0ULL);

// Settings depend on patterns
settings Settings;

// Runtime must be initialized before ser interface, which contains GC pointers
runtime::gcptr *runtime::GCSafe;
runtime rt(nullptr, 0);
user_interface ui;

uint last_keystroke_time = 0;
int  last_key            = 0;

RECORDER(main,          16, "Main RPL thread");
RECORDER(main_error,    16, "Errors in the main RPL thread");
RECORDER(tests_rpl,    256, "Test request processing on RPL");
RECORDER(refresh,       16, "Refresh requests");


static byte *lcd_buffer = nullptr;
static uint  row_min    = ~0;
static uint  row_max    = 0;

void mark_dirty(uint row)
// ----------------------------------------------------------------------------
//   Mark a screen range as dirty
// ----------------------------------------------------------------------------
{
    if (row < LCD_H)
    {
#ifndef SIMULATOR
        if (Settings.DMCPDisplayRefresh())
        {
            bitblt24(0, 8, row, 0, BLT_XOR, BLT_NONE);
        }
        else if (!lcd_buffer[52 * row - 2])
        {
            lcd_buffer[52 * row - 2] = 1;
            lcd_buffer[52 * row] ^= 1;
            if (row_min > row)
                row_min = row;
            if (row_max < row)
                row_max = row;
        }
#endif // SIMULATOR
    }
}


void refresh_dirty()
// ----------------------------------------------------------------------------
//  Send an LCD refresh request for the area dirtied by drawing
// ----------------------------------------------------------------------------
{
    uint start = sys_current_ms();
#ifndef SIMULATOR
    if (ST(STAT_OFF))
        return;
    if (Settings.DMCPDisplayRefresh())
    {
        lcd_refresh();
    }
    else
    {
        for (uint row = 0; row < LCD_H; row++)
        {
            if (lcd_buffer[52 * row - 2])
            {
                lcd_buffer[52 * row - 1] = LCD_H - row;
                LCD_write_line(&lcd_buffer[52 * row - 2]);
                lcd_buffer[52 * row - 2] = 0;
            }
        }
    }
#else
    lcd_refresh();
#endif
    row_min = ~0;
    row_max = 0;
    program::refresh_time += sys_current_ms() - start;
}


void redraw_lcd(bool force)
// ----------------------------------------------------------------------------
//   Redraw the whole LCD
// ----------------------------------------------------------------------------
{
    uint now = sys_current_ms();

    record(main, "Begin redraw at %u", now);

    // Draw the various components handled by the user interface
    ui.draw_start(force);
    ui.draw_header();
    ui.draw_battery();
    ui.draw_annunciators();
    ui.draw_menus();
    if (!ui.draw_help())
    {
        ui.draw_editor();
        ui.draw_cursor(true, ui.cursor_position());
        ui.draw_stack();
        if (!ui.draw_stepping_object())
            ui.draw_command();

    }
    ui.draw_error();

    // Refresh the screen
    refresh_dirty();

    // Compute next refresh
    uint then = sys_current_ms();
    uint period = ui.draw_refresh();
    record(main,
           "Refresh at %u (%u later), period %u", then, then - now, period);

    // Refresh screen moving elements after the requested period
    sys_timer_start(TIMER1, period);

    program::display_time += sys_current_ms() - now;
}


static void redraw_periodics()
// ----------------------------------------------------------------------------
//   Redraw the elements that move
// ----------------------------------------------------------------------------
{
    uint now         = sys_current_ms();
    uint dawdle_time = now - last_keystroke_time;

    record(main, "Periodics %u", now);
    ui.draw_start(false);
    ui.draw_header();
    ui.draw_battery();
    if (program::animated())
    {
        ui.draw_cursor(false, ui.cursor_position());
        ui.draw_menus();
    }
    refresh_dirty();

    // Slow things down if inactive for long enough
    uint period = ui.draw_refresh();
    if (!program::animated())
    {
        // Adjust refresh time based on time since last interaction
        // After 10s, update at most every 3s
        // After 1 minute, update at most every 10s
        // After 3 minutes, update at most once per minute
        if (dawdle_time > 180000 && period < 60000)
            period = 60000;
        else if (dawdle_time > 60000 && period < 10000)
            period = 10000;
        else if (dawdle_time > 10000 && period < 3000)
            period = 3000;
    }

    uint then = sys_current_ms();
    record(main, "Dawdling for %u at %u after %u", period, then, then-now);

    // Refresh screen moving elements after 0.1s
    sys_timer_start(TIMER1, period);

    program::display_time += sys_current_ms() - now;
}


static void handle_key(int key, bool repeating, bool talpha)
// ----------------------------------------------------------------------------
//   Handle all user-interface keys
// ----------------------------------------------------------------------------
{
    sys_timer_disable(TIMER0);
    bool consumed = ui.key(key, repeating, talpha);
    if (!consumed)
        beep(1835, 125);

    // Key repeat timer
    if (ui.repeating())
        sys_timer_start(TIMER0, repeating ? 80 : 500);
}


void db48x_set_beep_mute(int val)
// ----------------------------------------------------------------------------
//   Set the beep flag (shared with firmware)
// ----------------------------------------------------------------------------
{
    Settings.BeepOff(val);
    Settings.SilentBeepOn(val);
}


int db48x_is_beep_mute()
// ----------------------------------------------------------------------------
//   Check the beep flag from our settings
// ----------------------------------------------------------------------------
{
    return Settings.BeepOff();
}


bool load_saved_keymap(cstring name)
// ----------------------------------------------------------------------------
//   Load the default system state file
// ----------------------------------------------------------------------------
{
    bool isdefault = false;
    char keymap_name[80] = { 0 };
    if (name)
    {
        file kcfg("config/keymap.cfg", file::WRITING);
        if (kcfg.valid())
            kcfg.write(name, strlen(name));
    }

    file kcfg("config/keymap.cfg", file::READING);
    if (kcfg.valid())
    {
        kcfg.read(keymap_name, sizeof(keymap_name)-1);
        for (size_t i = 0; i < sizeof(keymap_name); i++)
            if (keymap_name[i] == '\n')
                keymap_name[i] = 0;
    }
    else
    {
        strncpy(keymap_name, "config/db48x.48k", sizeof(keymap_name));
        isdefault = true;
    }

    // Load default keymap
    if (!ui.load_keymap(keymap_name))
    {
        // Fail silently if we try to load a default file
        if (isdefault)
            rt.clear_error();
        else
            rt.command(command::static_object(object::ID_KeyMap));
        return false;
    }
    return true;
}


extern uint memory_size;
void program_init()
// ----------------------------------------------------------------------------
//   Initialize the program
// ----------------------------------------------------------------------------
{
    // Setup application menu callbacks
    run_menu_item_app = menu_item_run;
    menu_line_str_app = menu_item_description;
    is_beep_mute = db48x_is_beep_mute;
    set_beep_mute = db48x_set_beep_mute;
    lcd_buffer = lcd_line_addr(0);

    // Setup default fonts
    font_defaults();

#ifndef SIMULATOR
    // Give as much as memory as possible to the runtime
    // Experimentally, this is the amount of memory we need to leave free
    size_t size = sys_free_mem() - 10 * 1024;
#else
    // Give 4K bytes to the runtime to stress-test the GC
    size_t size = 1024 * memory_size;
#endif
    byte *memory = (byte *) malloc(size);
    rt.memory(memory, size);

    // Check if we have a state file to load
    load_system_state();
    load_saved_keymap();
}


void power_check(bool running)
// ----------------------------------------------------------------------------
//   Check power state, keep looping until it's safe to run
// ----------------------------------------------------------------------------
// Status flags:
// ST(STAT_PGM_END)   - Program should go to off state (set by auto off timer)
// ST(STAT_SUSPENDED) - Program signals it is ready for off
// ST(STAT_OFF)       - Program in off state (only [EXIT] key can wake it up)
// ST(STAT_RUNNING)   - OS doesn't sleep in this mode
{
    while (true)
    {
        // Already in off mode and suspended
        if ((ST(STAT_PGM_END) && ST(STAT_SUSPENDED)) ||
            // Go to sleep if no keys available
            (!ST(STAT_PGM_END) && key_empty()))
        {
            CLR_ST(STAT_RUNNING);
            static uint last_awake = 0;
            uint now = sys_current_ms();
            if (last_awake)
                program::active_time += now - last_awake;
            sys_sleep();
            uint then = sys_current_ms();
            last_awake = then;
            program::sleeping_time += then - now;
            program::run_cycles++;
        }
        if (ST(STAT_PGM_END) || ST(STAT_SUSPENDED))
        {
            // Wakeup in off state or going to sleep
            if (!ST(STAT_SUSPENDED))
            {
                bool lowbat = !program::on_usb && program::low_battery();
                if (lowbat)
                    ui.draw_message("Switched off due to low power",
                                    "Connect to USB to avoid losing memory",
                                    "Replace the battery as soon as possible");
                else if (running)
                    ui.draw_message("Switched off to conserve battery",
                                    "Press the ON/EXIT key to resume");
                else
                    draw_power_off_image(0);

                sys_critical_start();
                SET_ST(STAT_SUSPENDED);
                LCD_power_off(0);
                sys_timer_disable(TIMER0);
                sys_timer_disable(TIMER1);
                SET_ST(STAT_OFF);
                sys_critical_end();
            }
            // Already in OFF -> just continue to sleep above
        }

        // Check power change or wakeup
        else if (ST(STAT_CLK_WKUP_FLAG))
        {
            CLR_ST(STAT_CLK_WKUP_FLAG);
        }
        else if (ST(STAT_POWER_CHANGE))
        {
            CLR_ST(STAT_POWER_CHANGE);
            ui.draw_battery(true);
            refresh_dirty();
        }
        else
        {
            break;
        }
    }

    // Well, we are woken-up
    SET_ST(STAT_RUNNING);

    // Get up from OFF state
    if (ST(STAT_OFF))
    {
        LCD_power_on();

        // Ensure that RTC readings after power off will be OK
        rtc_wakeup_delay();

        CLR_ST(STAT_OFF);

        // Check if we need to redraw
        if (ui.showing_graphics())
            ui.draw_graphics(true);
        else
            redraw_lcd(true);
    }

    // We definitely reached active state, clear suspended flag
    CLR_ST(STAT_SUSPENDED);
}

#ifndef SIMULATOR
extern const uint prog_build_id;
extern const uint qspi_build_id;
#endif

extern "C" void program_main()
// ----------------------------------------------------------------------------
//   DMCP main entry point and main loop
// ----------------------------------------------------------------------------
{
    int  key        = 0;
    bool transalpha = false;

#ifndef SIMULATOR
    if (prog_build_id != qspi_build_id)
    {
        msg_box(t24,
                "Incompatible " PROGRAM_NAME " build ID\n"
                "Please reload program and QSPI\n"
                "from the same build",
                true);
        lcd_refresh();
        wait_for_key_press();
        return;
    }
#endif

    // Initialization
    program_init();
    redraw_lcd(true);
    last_keystroke_time = sys_current_ms();

    // Main loop
    while (true)
    {
        // Check power state, and switch off if necessary
        power_check(false);

        // Key is ready -> clear auto off timer
        bool hadKey = false;

        if (!key_empty())
        {
            reset_auto_off();
            key    = key_pop();
            hadKey = true;
            record(main, "Got key %d", key);

#if !WASM
#if SIMULATOR
            // Process test-harness commands
            record(tests_rpl, "Processing key %d, last=%d, command=%u",
                   key, last_key, test_command);
            if (key == tests::EXIT_PGM || key == tests::SAVE_PGM)
            {
                cstring path = get_reset_state_file();
                printf("Exit: saving state to %s\n", path);
                if (path && *path)
                    save_state_file(path);
                if (key == tests::EXIT_PGM)
                    break;
            }
#else // Real hardware
#define read_key __sysfn_read_key
#endif // SIMULATOR
#endif // !WASM

            // Check transient alpha mode
            if (key == KEY_UP || key == KEY_DOWN)
            {
                transalpha = true;
            }
            else if (transalpha)
            {
                int k1, k2;
                int r = read_key(&k1, &k2);
                switch (r)
                {
                case 0:
                    transalpha = false;
                    break;
                case 1:
                    transalpha = k1 == KEY_UP || k1 == KEY_DOWN;
                    break;
                case 2:
                    transalpha = k1 == KEY_UP || k1 == KEY_DOWN
                        ||       k2 == KEY_UP || k2 == KEY_DOWN;
                    break;
                }
            }

        }
        bool repeating = key > 0
            && sys_timer_active(TIMER0)
            && sys_timer_timeout(TIMER0);
        if (repeating)
        {
            hadKey = true;
            record(main, "Repeating key %d", key);
        }

        // Fetch the key (<0: no key event, >0: key pressed, 0: key released)
        record(main, "Testing key %d (%+s)", key, hadKey ? "had" : "nope");
        if (key >= 0 && hadKey)
        {
#if SIMULATOR && !WASM
            process_test_key(key);
#endif // SIMULATOR && !WASM

            record(main, "Handle key %d last %d", key, last_key);
            handle_key(key, repeating, transalpha);
            record(main, "Did key %d last %d", key, last_key);

            // Redraw the LCD unless there is some type-ahead
            if (key_empty())
                redraw_lcd(false);

            // Record the last keystroke
            last_keystroke_time = sys_current_ms();
            record(main, "Last keystroke time %u", last_keystroke_time);
        }
        else
        {
            // Blink the cursor
            if (sys_timer_timeout(TIMER1))
                redraw_periodics();
            if (!key)
                sys_timer_disable(TIMER0);
        }
#if SIMULATOR && !WASM
        if (tests::running && test_command && key_empty())
            process_test_commands();
#endif // SIMULATOR && !WASM
    }
}


#if WASM
uint            memory_size           = 100;
volatile uint   test_command          = 0;
bool            noisy_tests           = false;
bool            no_beep               = false;
bool            tests::running        = false;

static void *rpl_thread(void *)
// ----------------------------------------------------------------------------
//   Run the RPL thread
// ----------------------------------------------------------------------------
{
    record(main, "Entering main thread");
    program_main();
    return nullptr;
}


int ui_init()
// ----------------------------------------------------------------------------
//   Initialization for the JavaScript version
// ----------------------------------------------------------------------------
{
    recorder_trace_set(".*error.*|.*warn.*");
    record(main, "ui_init invoked");
    pthread_t rpl;
    int rc = pthread_create(&rpl, nullptr, rpl_thread, nullptr);
    record(main, "pthread_create returned %d, %s", rc, strerror(rc));
    return 42;
}

#endif // WASM



#if SIMULATOR
void process_test_key(int key)
// ----------------------------------------------------------------------------
//   Process commands from the test harness
// ----------------------------------------------------------------------------
{
    record(tests_rpl, "Process test key %d, last was %d, command %u",
           key, last_key, test_command);
    if (key > 0)
        last_key = key;
    else if (last_key > 0)
        last_key = -last_key;
    record(tests_rpl, "Set last_key to %d for key %d", last_key, key);
}


void process_test_commands()
// ----------------------------------------------------------------------------
//   Process commands from the test harness
// ----------------------------------------------------------------------------
{
    record(tests_rpl, "Process test command %u with last key %d",
           test_command, last_key);

    if (test_command == tests::CLEARERR)
    {
        record(tests_rpl, "Clearing errors for tests");
        rt.clear_error();
    }
    else if (test_command == tests::CLEAR)
    {
        record(tests_rpl, "Clearing editor and stack for tests");
        rt.clear_error();
        ui.clear_editor();
        rt.drop(rt.depth());
        while (rt.run_next(0));
    }
    else if (test_command == tests::KEYSYNC)
    {
        record(tests_rpl, "Key sync requested");
    }
    if (!ui.showing_graphics())
        redraw_lcd(true);
    record(tests_rpl, "Done redrawing LCD after command %u, last=%d",
           test_command, last_key);
    test_command = 0;
}
#endif // SIMULATOR
