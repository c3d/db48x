// ****************************************************************************
//  sysmenu.cc                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Handles the DMCP application menus on the DM42
//
//     This piece of code is DM42-specific
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

#include "sysmenu.h"

#include "file.h"
#include "user_interface.h"
#include "program.h"
#include "main.h"
#include "object.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "types.h"
#include "util.h"
#include "variables.h"

#include <cstdio>
#include <dmcp.h>


// ============================================================================
//
//    Main application menu
//
// ============================================================================

const uint8_t application_menu_items[] =
// ----------------------------------------------------------------------------
//    Application menu items
// ----------------------------------------------------------------------------
{
    MI_DB48_SETTINGS,           // Application setting
    MI_DB48_ABOUT,              // About dialog

    MI_48STATE,                 // File operations on state
    MI_48STATUS,                // Status bar settings

    MI_MSC,                     // Activate USB disk
    MI_PGM_LOAD,                // Load program
    MI_LOAD_QSPI,               // Load QSPI
    MI_SYSTEM_ENTER,            // Enter system

    0
}; // Terminator


const smenu_t application_menu =
// ----------------------------------------------------------------------------
//   Application menu
// ----------------------------------------------------------------------------
{
    "Setup",  application_menu_items,   NULL, NULL
};


void about_dialog()
// ----------------------------------------------------------------------------
//   Display the About dialog
// ----------------------------------------------------------------------------
{
    lcd_clear_buf();
    lcd_writeClr(t24);

    // Header based on original system about
    lcd_for_calc(DISP_ABOUT);
    lcd_putsAt(t24,4,"");
    lcd_prevLn(t24);

    // Display the main text
    int h2 = lcd_lineHeight(t20)/2; // Extra spacing
    lcd_setXY(t20, t24->x, t24->y + h2);
    lcd_puts(t20, "DB48X v" PROGRAM_VERSION " (C) C. de Dinechin");
    t20->y += h2;
    lcd_puts(t20, "DMCP platform (C) SwissMicros GmbH");
    lcd_puts(t20, "Intel Decimal Floating Point Lib v2.0u1");
    lcd_puts(t20, "  (C) 2007-2018, Intel Corp.");

    t20->y = LCD_Y - lcd_lineHeight(t20);
    lcd_putsR(t20, "    Press EXIT key to continue...");

    lcd_refresh();

    wait_for_key_press();
}



// ============================================================================
//
//    Settings menu
//
// ============================================================================

const uint8_t settings_menu_items[] =
// ----------------------------------------------------------------------------
//    Settings menu items
// ----------------------------------------------------------------------------
{
    MI_SET_TIME,                // Standard set time menu
    MI_SET_DATE,                // Standard set date menu
    MI_BEEP_MUTE,               // Mute the beep
    MI_SLOW_AUTOREP,            // Slow auto-repeat
    0
}; // Terminator


const smenu_t settings_menu =
// ----------------------------------------------------------------------------
//   Settings menu
// ----------------------------------------------------------------------------
{
    "Settings",  settings_menu_items,  NULL, NULL
};



// ============================================================================
//
//    Status bar menu
//
// ============================================================================

const uint8_t status_bar_menu_items[] =
// ----------------------------------------------------------------------------
//    Menu items for "Status bar" meni
// ----------------------------------------------------------------------------
{
    MI_48STATUS_DAY_OF_WEEK,    // Display day of week
    MI_48STATUS_TIME,           // Display time
    MI_48STATUS_24H,            // Display time in 24h format
    MI_48STATUS_SECONDS,        // Display seconds
    MI_48STATUS_DATE,           // Display the date
    MI_48STATUS_DATE_SEPARATOR, // Select date separator
    MI_48STATUS_SHORT_MONTH,    // Short month
    MI_48STATUS_VOLTAGE,        // Display voltage
    0
}; // Terminator


const smenu_t status_bar_menu =
// ----------------------------------------------------------------------------
//   Status bar menu
// ----------------------------------------------------------------------------
{
    "Status bar",  status_bar_menu_items,  NULL, NULL
};



// ============================================================================
//
//   State load/save
//
// ============================================================================

const uint8_t state_menu_items[] =
// ----------------------------------------------------------------------------
//    Program menu items
// ----------------------------------------------------------------------------
{
    MI_48STATE_LOAD,            // Load a 48 program from disk
    MI_48STATE_SAVE,            // Save a 48 program to disk
    MI_48STATE_CLEAN,           // Start with a fresh clean state
    MI_MSC,                     // Activate USB disk
    MI_DISK_INFO,               // Show disk information

    0
}; // Terminator


const smenu_t state_menu =
// ----------------------------------------------------------------------------
//   Program menu
// ----------------------------------------------------------------------------
{
    "State",  state_menu_items,  NULL, NULL
};


static bool state_save_variable(symbol_p name, object_p obj, void *renderer_ptr)
// ----------------------------------------------------------------------------
//   Emit Object 'Name' STO for each object in the top level directory
// ----------------------------------------------------------------------------
{
    renderer &r = *((renderer *) renderer_ptr);

    symbol_g  n = name;
    object_g  o = obj;

    o->render(r);
    r.put("\n'");
    n->render(r);
    r.put("' STO\n\n");
    return true;
}


static int state_save_callback(cstring fpath,
                                 cstring fname,
                                 void       *)
// ----------------------------------------------------------------------------
//   Callback when a file is selected
// ----------------------------------------------------------------------------
{
    // Display the name of the file being saved
    lcd_puts(t24,"Saving state...");
    lcd_puts(t24, fname);
    lcd_refresh();

    // Store the state file name so that we automatically reload it
    set_reset_state_file(fpath);

    // Open save file name
    file prog(fpath);
    if (!prog.valid())
    {
        disp_disk_info("State save failed");
        wait_for_key_press();
        return 1;
    }

    // Always render things to disk using default settings
    renderer render(&prog);
    settings saved = Settings;
    Settings = settings();
    Settings.fancy_exponent = false;
    Settings.standard_exp = 1;

    // Save global variables
    gcp<directory> home = rt.variables(0);
    home->enumerate(state_save_variable, &render);

    // Save the stack
    uint depth = rt.depth();
    while (depth > 0)
    {
        depth--;
        object_p obj = rt.stack(depth);
        obj->render(render);
        render.put('\n');
    }

    // Save current settings
    saved.save(render);

    // Restore the settings we had
    Settings = saved;

    return MRET_EXIT;
}


static int state_save()
// ----------------------------------------------------------------------------
//   Save a program to disk
// ------------------------------------------------------------1----------------
{
    // Check if we have enough power to write flash disk
    if (power_check_screen())
        return 0;

    bool display_new = true;
    bool overwrite_check = true;
    void *user_data = NULL;
    int ret = file_selection_screen("Save state",
                                    "/STATE", ".48S",
                                    state_save_callback,
                                    display_new, overwrite_check,
                                    user_data);
    return ret;
}


static bool danger_will_robinson(cstring header,
                                 cstring msg1,
                                 cstring msg2 = "",
                                 cstring msg3 = "",
                                 cstring msg4 = "",
                                 cstring msg5 = "",
                                 cstring msg6 = "",
                                 cstring msg7 = "")
// ----------------------------------------------------------------------------
//  Warn user about the possibility to lose calculator state
// ----------------------------------------------------------------------------
{
    lcd_writeClr(t24);
    lcd_clear_buf();
    lcd_putsR(t24, header);
    t24->ln_offs = 8;

    lcd_puts(t24, msg1);
    lcd_puts(t24, msg2);
    lcd_puts(t24, msg3);
    lcd_puts(t24, msg4);
    lcd_puts(t24, msg5);
    lcd_puts(t24, msg6);
    lcd_puts(t24, msg7);
    lcd_puts(t24, "Press [ENTER] to confirm.");
    lcd_refresh();

    wait_for_key_release(-1);

    while (true)
    {
        int key = runner_get_key(NULL);
        if (IS_EXIT_KEY(key) || is_menu_auto_off())
            return false;
        if ( key == KEY_ENTER )
            return true; // Proceed with reset
    }
}


static int state_load_callback(cstring path, cstring name, void *merge)
// ----------------------------------------------------------------------------
//   Callback when a file is selected for loading
// ----------------------------------------------------------------------------
{
    if (!merge)
    {
        // Check before erasing state
        if (!danger_will_robinson("Loading DB48X state",

                                  "You are about to erase the current",
                                  "calculator state to replace it with",
                                  "a new one",
                                  "",
                                  "WARNING: Current state will be lost"))
            return 0;

        // Clear the state
        rt.reset();

        set_reset_state_file(path);

    }

    // Display the name of the file being saved
    lcd_writeClr(t24);
    lcd_clear_buf();
    lcd_putsR(t24, merge ? "Merge state" : "Load state");
    lcd_puts(t24,"Loading state...");
    lcd_puts(t24, name);
    lcd_refresh();

    // Store the state file name
    file prog;
    prog.open(path);
    if (!prog.valid())
    {
        disp_disk_info("State load failed");
        wait_for_key_press();
        return 1;
    }

    // Loop on the input file and process it as if it was being typed
    uint bytes = 0;
    rt.clear();

    for (unicode c = prog.get(); c; c = prog.get())
    {
        byte buffer[4];
        size_t count = utf8_encode(c, buffer);
        rt.insert(bytes, buffer, count);
        bytes += count;
    }

    // End of file: execute the command we typed
    size_t edlen = rt.editing();
    if (edlen)
    {
        gcutf8 editor = rt.close_editor(true);
        if (editor)
        {
            char ds = Settings.decimal_mark;
            Settings.decimal_mark = '.';
            program_g cmds = program::parse(editor, edlen);
            Settings.decimal_mark = ds;
            if (cmds)
            {
                // We successfully parsed the line
                rt.clear();
                object::result exec = cmds->execute();
                if (exec != object::OK)
                {
                    lcd_print(t24, "Error loading file");
                    lcd_puts(t24, (cstring) rt.error());
                    lcd_print(t24, "executing %s", rt.command());
                    lcd_refresh();
                    wait_for_key_press();
                    return 1;
                }

                // Clone all objects on the stack so that we can purge
                // the command-line above.
                rt.clone_stack();
            }
            else
            {
                utf8 pos = rt.source();
                utf8 ed = editor;

                lcd_print(t24, "Error at byte %u", pos - ed);
                lcd_puts(t24, rt.error() ? (cstring) rt.error() : "");
                lcd_refresh();
                beep(3300, 100);
                wait_for_key_press();

                if (pos >= editor && pos <= ed + edlen)
                    ui.cursorPosition(pos - ed);
                if (!rt.edit(ed, edlen))
                    ui.cursorPosition(0);

                return 1;
            }
        }
        else
        {
            lcd_print(t24, "Out of memory");
            lcd_refresh();
            beep(3300, 100);
            wait_for_key_press();
            return 1;
        }
    }

    // Exit with success
    return MRET_EXIT;
}


static int state_load(bool merge)
// ----------------------------------------------------------------------------
//   Load a state from disk
// ----------------------------------------------------------------------------
{
    bool display_new = false;
    bool overwrite_check = false;
    void *user_data = (void *) merge;
    int ret = file_selection_screen(merge ? "Merge state" : "Load state",
                                    "/STATE", ".48S",
                                    state_load_callback,
                                    display_new, overwrite_check,
                                    user_data);
    return ret;
}


static int state_clear()
// ----------------------------------------------------------------------------
//   Reset calculator to factory state
// ----------------------------------------------------------------------------
{
    if (danger_will_robinson("Clear DB48X state",

                              "You are about to reset the DB48X",
                              "program to factory state.",
                              ""
                             "WARNING: Current state will be lost"))
    {
        // Reset statefile name for next load
        set_reset_state_file("");

        // Reset the system to force new statefile load
        set_reset_magic(NO_SPLASH_MAGIC);
        sys_reset();
    }


    return MRET_EXIT;
}


cstring state_name()
// ----------------------------------------------------------------------------
//    Return the state name as stored in the non-volatile memory
// ----------------------------------------------------------------------------
{
    cstring name = get_reset_state_file();
    if (name && *name && strstr(name, ".48S"))
    {
        cstring last = nullptr;
        for (cstring p = name; *p; p++)
        {
            if (*p == '/' || *p == '\\')
                name = p + 1;
            else if (*p == '.')
                last = p;
        }
        if (!last)
            last = name;

        static char buffer[16];
        char *end = buffer + sizeof(buffer);
        char *p = buffer;
        while (p < end && name < last && (*p++ = *name++))
            /* Copy */;
        *p = 0;
        return buffer;
    }

    return "DB48X";
}


bool load_state_file(cstring path)
// ----------------------------------------------------------------------------
//   Load the state file directly
// ----------------------------------------------------------------------------
{
    cstring name = path;
    for (cstring p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            name = p + 1;
    return state_load_callback(path, name, (void *) 1) == 0;
}


bool load_system_state()
// ----------------------------------------------------------------------------
//   Load the default system state file
// ----------------------------------------------------------------------------
{
    if (sys_disk_ok())
    {
        // Try to load the state file, but only if it has the right
        // extension. This is necessary, because get_reset_state_file() could
        // legitimately return a .f42 file if we just switched from DM42.
        char *state = get_reset_state_file();
        if (state && *state && strstr(state, ".48S"))
            return load_state_file(state);
    }
    return false;
}


bool save_state_file(cstring path)
// ----------------------------------------------------------------------------
//   Save the state file directly
// ----------------------------------------------------------------------------
{
    cstring name = path;
    for (cstring p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            name = p + 1;
    return state_save_callback(path, name, nullptr) == 0;
}


bool save_system_state()
// ----------------------------------------------------------------------------
//   Save the default system state file
// ----------------------------------------------------------------------------
{
    if (sys_disk_ok())
    {
        // Try to load the state file, but only if it has the right
        // extension. This is necessary, because get_reset_state_file() could
        // legitimately return a .f42 file if we just switched from DM42.
        char *state = get_reset_state_file();
        if (state && *state && strstr(state, ".48S"))
            return save_state_file(state);
        else
            return state_save() == 0;
    }
    return false;
}


static char next_date_sep(char sep)
// ----------------------------------------------------------------------------
//   Compute the next date separator
// ----------------------------------------------------------------------------
{
    switch(sep)
    {
    case '/':   return '.';
    case '.':   return '-';
    case '-':   return ' ';
    default:
    case ' ':   return '/';
    }
}


int menu_item_run(uint8_t menu_id)
// ----------------------------------------------------------------------------
//   Callback to run a menu item
// ----------------------------------------------------------------------------
{
    int ret = 0;

    switch (menu_id)
    {
    case MI_DB48_ABOUT:    about_dialog(); break;
    case MI_DB48_SETTINGS: ret = handle_menu(&settings_menu, MENU_ADD, 0); break;
    case MI_48STATE:       ret = handle_menu(&state_menu, MENU_ADD, 0); break;
    case MI_48STATE_LOAD:  ret = state_load(false); break;
    case MI_48STATE_MERGE: ret = state_load(true); break;
    case MI_48STATE_SAVE:  ret = state_save(); break;
    case MI_48STATE_CLEAN: ret = state_clear(); break;

    case MI_48STATUS:
        ret = handle_menu(&status_bar_menu, MENU_ADD, 0); break;
    case MI_48STATUS_DAY_OF_WEEK:
        Settings.show_dow = !Settings.show_dow; break;
    case MI_48STATUS_DATE:
        Settings.show_date = settings::dmy_ord((int(Settings.show_date) + 1) & 3); break;
    case MI_48STATUS_DATE_SEPARATOR:
        Settings.date_separator = next_date_sep(Settings.date_separator); break;
    case MI_48STATUS_SHORT_MONTH:
        Settings.show_month = !Settings.show_month;                     break;
    case MI_48STATUS_TIME:
        Settings.show_time = !Settings.show_time;                       break;
    case MI_48STATUS_SECONDS:
        Settings.show_seconds = !Settings.show_seconds;                 break;
    case MI_48STATUS_24H:
        Settings.show_24h = !Settings.show_24h;                         break;
    case MI_48STATUS_VOLTAGE:
        Settings.show_voltage = !Settings.show_voltage;                 break;
    default:
        ret = MRET_UNIMPL; break;
    }

    return ret;
}


static char *sep_str(char *s, cstring txt, char sep)
// ----------------------------------------------------------------------------
//   Build a separator string
// ----------------------------------------------------------------------------
{
    snprintf(s, 40, "[%c] %s", sep, txt);
    return s;
}


static char *flag_str(char *s, cstring txt, bool flag)
// ----------------------------------------------------------------------------
//   Build a flag string
// ----------------------------------------------------------------------------
{
    return sep_str(s, txt, flag ? 'X' : '_');
}


static char *dord_str(char *s, cstring txt, settings::dmy_ord flag)
// ----------------------------------------------------------------------------
//   Build a string for date order
// ----------------------------------------------------------------------------
{
    cstring order[] = { "___", "DMY", "MDY", "YMD" };
    snprintf(s, 40, "[%s] %s", order[flag], txt);
    return s;
}


cstring menu_item_description(uint8_t menu_id, char *s, const int UNUSED len)
// ----------------------------------------------------------------------------
//   Return the menu item description
// ----------------------------------------------------------------------------
{
    cstring ln = nullptr;

    switch (menu_id)
    {
    case MI_DB48_SETTINGS:              ln = "Settings >";              break;
    case MI_DB48_ABOUT:                 ln = "About >";                 break;

    case MI_48STATE:                    ln = "State >";                 break;
    case MI_48STATE_LOAD:               ln = "Load State";              break;
    case MI_48STATE_MERGE:              ln = "Merge State";             break;
    case MI_48STATE_SAVE:               ln = "Save State";              break;
    case MI_48STATE_CLEAN:              ln = "Clear state";             break;

    case MI_48STATUS:                   ln = "Status bar >";            break;
    case MI_48STATUS_DAY_OF_WEEK:
        ln = flag_str(s, "Day of week", Settings.show_dow);             break;
    case MI_48STATUS_DATE:
        ln = dord_str(s, "Date", Settings.show_date);                   break;
    case MI_48STATUS_DATE_SEPARATOR:
        ln = sep_str(s, "Date separator", Settings.date_separator);     break;
    case MI_48STATUS_SHORT_MONTH:
        ln = flag_str(s, "Month name", Settings.show_month);            break;
    case MI_48STATUS_TIME:
        ln = flag_str(s, "Time", Settings.show_time);                   break;
    case MI_48STATUS_SECONDS:
        ln = flag_str(s, "Show seconds", Settings.show_seconds);        break;
    case MI_48STATUS_24H:
        ln = flag_str(s, "Show 24h time", Settings.show_24h);           break;
    case MI_48STATUS_VOLTAGE:
        ln = flag_str(s, "Voltage", Settings.show_voltage);             break;

    default:                            ln = NULL;                      break;
    }

    return ln;
}


void power_off()
// ----------------------------------------------------------------------------
//   Power off the calculator
// ----------------------------------------------------------------------------
{
    SET_ST(STAT_PGM_END);
}


void system_setup()
// ----------------------------------------------------------------------------
//   Invoke the system setup
// ----------------------------------------------------------------------------
{
    SET_ST(STAT_MENU);
    int ret = handle_menu(&application_menu, MENU_RESET, 0);
    CLR_ST(STAT_MENU);
    if (ret != MRET_EXIT)
        wait_for_key_release(-1);
    redraw_lcd(true);
}
