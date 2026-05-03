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
#  include "sim-eval.h"
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
surface Screen((pixword *) lcd_line_addr(0), LCD_W, LCD_H, LCD_SCANLINE, LCD_W);

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


void mark_dirty(int x1, int y1, int x2, int y2)
// ----------------------------------------------------------------------------
//   Mark a screen rectangle as dirty
// ----------------------------------------------------------------------------
{
    (void) (x1 + x2);
    if (y1 > y2)
        std::swap(y1, y2);
    if (y1 < 0)
        y1 = 0;
    else if (y1 >= LCD_H)
        y1 = LCD_H - 1;
    if (y2 < 0)
        y2 = 0;
    else if (y2 >= LCD_H)
        y2 = LCD_H - 1;

    record(refresh, "Refreshing rows %d..%d", y1, y2);
    for (uint y = uint(y1); y <= uint(y2); y++)
        mark_dirty(y);
    record(refresh, "Refreshed  rows %d..%d", y1, y2);
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
        for (uint row = row_min; row <= row_max; row++)
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


void set_timer(uint timerid, uint period)
// ----------------------------------------------------------------------------
//   Conditionally set a timer based on period
// ----------------------------------------------------------------------------
{
    if (period >= 1000)
    {
        sys_timer_disable(timerid);
        if (period >= 60000)
            CLR_ST(STAT_CLK_WKUP_SECONDS);
        else
            SET_ST(STAT_CLK_WKUP_SECONDS);
    }
    else
    {
        sys_timer_start(timerid, period);
    }
}


void redraw_lcd(bool force)
// ----------------------------------------------------------------------------
//   Redraw the whole LCD
// ----------------------------------------------------------------------------
{
    uint start = sys_current_ms();

    record(main, "Begin redraw at %u", start);

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
    ui.refresh();

    // Compute next refresh
    uint end = sys_current_ms();
    uint period = ui.draw_refresh();
    record(main,
           "Refresh at %u (%u later), period %u", end, end - start, period);

    // Refresh screen moving elements after the requested period
    set_timer(TIMER1, period);
    program::display_time += end - start;
}


static void redraw_periodics()
// ----------------------------------------------------------------------------
//   Redraw the elements that move
// ----------------------------------------------------------------------------
{
    uint start       = program::read_time();
    uint dawdle_time = start - last_keystroke_time;

    record(main, "Periodics %u", start);
    ui.draw_start(false);
    ui.draw_header();
    ui.draw_battery();
    if (program::animated())
    {
        ui.draw_cursor(false, ui.cursor_position());
        ui.draw_menus();
    }
    ui.refresh();

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

    uint end = program::read_time();
    record(main, "Dawdling for %u at %u after %u", period, end, end-start);

    // Refresh screen moving elements after 0.1s
    set_timer(TIMER1, period);

    program::display_time += end - start;
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


uint slowdown(uint random)
// ----------------------------------------------------------------------------
//   Artificial slow-down for QSPI acceess
// ----------------------------------------------------------------------------
//   The reason for slowing down is that the QSPI occasionally feeds bad bytes
//   if read too fast. See also issue #12, issue #304, #1532, and
//   commit 63eb8efabd1 introducing DEOPTIMIZE_CATALOG.
{
    char buffer[8];
    for (uint i = 0; i < sizeof(buffer); i++)
        buffer[i] = random++;
    return buffer[0] ^ buffer[sizeof(buffer)-1];
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

    // Enable wakeup each minute (for clock update)
    SET_ST(STAT_CLK_WKUP_ENABLE);
}


void power_check(bool running, bool showimage)
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
            uint tin = sys_current_ms();
            if (last_awake)
                program::active_time += tin - last_awake;
            sys_sleep();
            uint tout = sys_current_ms();
            last_awake = tout;
            program::sleeping_time += tout - tin;
            program::run_cycles++;
        }
        if (ST(STAT_PGM_END) || ST(STAT_SUSPENDED))
        {
            // Wakeup in off state or going to sleep
            if (!ST(STAT_SUSPENDED))
            {
                bool lowbat = !program::on_usb && program::low_battery();
                if (lowbat)
                {
                    ui.draw_message("Switched off due to low power",
                                    "Connect to USB to avoid losing memory",
                                    "Replace the battery as soon as possible");
                }
                else if (ui.showing_graphics())
                {
                    // Preserve (most of) the graphics being shown
                    for (uint i = 0; i < 4; i++)
                    {
                        coord x = (i & 1) ? 10 : LCD_W - 11;
                        coord y = (i & 2) ? 10 : LCD_H - 11;
                        Screen.circle(x, y, 12, 0, pattern::black);
                        Screen.circle(x, y,  8, 0, pattern::white);
                        Screen.circle(x, y,  4, 0, pattern::black);
                    }
                    lcd_refresh_wait();
                }
                else if (running)
                {
                    ui.draw_message("Switched off to conserve battery",
                                    "Press the ON/EXIT key to resume");
                }
                else if (showimage)
                {
                    draw_power_off_image(0);
                }
                else
                {
                    lcd_refresh_wait();
                }

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

        else if (ST(STAT_CLK_WKUP_FLAG))
        {
            // Clock wakeup (once per second or per minute)
            CLR_ST(STAT_CLK_WKUP_FLAG);
            if (running)
                break;
            if (!ui.showing_graphics())
                redraw_periodics();
        }
        else if (ST(STAT_POWER_CHANGE))
        {
            // Power state change (to/from USB)
            CLR_ST(STAT_POWER_CHANGE);
            sys_timer_disable(TIMER0);
            sys_timer_disable(TIMER1);
            // Force reload battery with correct value at next clock refresh.
            ui.draw_battery(true);
            program::last_interrupted -= Settings.BatteryRefresh() - 1000;
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

        // Redraw the LCD content
        program::read_battery();
        if (ui.showing_graphics())
        {
            ui.show_graphics(false);
            ui.draw_graphics(false);
        }
        else
        {
            redraw_lcd(true);
        }
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
        static const char __attribute__((section(".flash"))) msg[] =
            "Incompatible " PROGRAM_NAME
            " build ID\n"
            "Please reload program and QSPI\n"
            "from the same build";
        msg_box(t24, msg, true);
        lcd_refresh();
        wait_for_key_press();
        return;
    }
#endif

    // Initialization
    program_init();
#if SIMULATOR && !WASM
    process_sim_eval_commands();
#endif // SIMULATOR && !WASM
    redraw_lcd(true);
    last_keystroke_time = program::read_time();

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

        // Check if we are displaying a graphic image - If so wait for key
        bool graphics = ui.showing_graphics();
        if (graphics && hadKey)
        {
            if (key > 0)
            {
                record(tests_rpl, "Clearing graphics from key %d", key);
                ui.show_graphics(false);
                redraw_lcd(true);
            }
        }

        // Fetch the key (<0: no key event, >0: key pressed, 0: key released)
        record(main, "Testing key %d (%+s)", key, hadKey ? "had" : "nope");
        if (key >= 0 && hadKey)
        {
#if SIMULATOR && !WASM
            if (process_test_key(key))
                graphics = false;
#endif // SIMULATOR && !WASM

            if (!graphics)
            {
                record(main, "Handle key %d last %d", key, last_key);
                handle_key(key, repeating, transalpha);
                record(main, "Did key %d last %d", key, last_key);

                // Redraw the LCD unless there is some type-ahead
                if (key_empty() && !ui.showing_graphics())
                    redraw_lcd(false);
            }

            // Record the last keystroke
            last_keystroke_time = program::read_time();
            record(main, "Last keystroke time %u", last_keystroke_time);
        }
        else
        {
            // Blink the cursor
            if (!graphics && sys_timer_timeout(TIMER1))
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
bool            noisy_tests           = false;
bool            no_beep               = false;
// test_command and tests::running are defined in tests.cc (same as Qt sim).

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
bool process_test_key(int key)
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
    return key >= tests::TEST_KEYS;
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
        if (ui.showing_graphics())
        {
            ui.show_graphics(false);
            redraw_lcd(true);
        }
    }
    else if (test_command == tests::KEYSYNC)
    {
        record(tests_rpl, "Key sync requested");
    }
    else if (test_command == tests::START_TEST)
    {
        program::read_battery();
    }
    else if (test_command == tests::EVAL_LINE)
    {
        record(tests_rpl, "Evaluating command line [%s]", sim_eval_pending_line());
        sim_eval_run(sim_eval_pending_line());
    }
    if (!ui.showing_graphics())
        redraw_lcd(true);
    record(tests_rpl, "Done redrawing LCD after command %u, last=%d",
           test_command, last_key);
    test_command = 0;
}
#endif // SIMULATOR
