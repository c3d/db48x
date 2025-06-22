#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
// ****************************************************************************
//  user_interface.h                                             DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Calculator user interface
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

#include "blitter.h"
#include "dmcp.h"
#include "file.h"
#include "object.h"
#include "runtime.h"
#include "target.h"
#include "text.h"
#include "types.h"

#include <string>
#include <vector>

GCP(menu);
GCP(grob);

struct user_interface
// ----------------------------------------------------------------------------
//    Calculator user_interface state
// ----------------------------------------------------------------------------
{
    user_interface();

    enum modes
    // ------------------------------------------------------------------------
    //   Current user_interface mode
    // ------------------------------------------------------------------------
    {
        STACK,                  // Showing the stack, not editing
        DIRECT,                 // Keys like 'sin' evaluate directly
        TEXT,                   // Alphanumeric entry, e.g. in strings
        PROGRAM,                // Keys like 'sin' show as 'sin' in the editor
        ALGEBRAIC,              // Keys like 'sin' show as 'sin()' in eqs
        PARENTHESES,            // Space inserts a semi-colon in eqs
        POSTFIX,                // Keys like '!' or 'x²' are postfix in eqs
        INFIX,                  // Keys like '+' are treated as infix in eqs
        CONSTANT,               // Entities like ⅈ or π have no parentheses
        MATRIX,                 // Matrix/vector mode
        BASED,                  // Based number: A-F map switch to alpha
        UNIT,                   // After a unit sign
    };

    enum
    // ------------------------------------------------------------------------
    //   Dimensioning constants
    // ------------------------------------------------------------------------
    {
        HISTORY         = 8,    // Number of menus and commands kept in history
        NUM_PLANES      = 3,    // NONE, Shift and "extended" shift
        NUM_KEYS        = 46,   // Including SCREENSHOT, SH_UP and SH_DN
        NUM_SOFTKEYS    = 6,    // Number of softkeys
        NUM_MENUS = NUM_PLANES * NUM_SOFTKEYS,
    };

    using result = object::result;
    using id     = object::id;

    using coord  = blitter::coord;
    using size   = blitter::size;
    using rect   = blitter::rect;

    bool        key(int key, bool repeating, bool transalpha);
    bool        repeating()     { return repeat; }
    object_p    assign(int keyid, object_p code);
    object_p    assigned(int keyid);
    void        toggle_user();

    void        update_mode();

    void        menu(menu_p menu, uint page = 0);
    menu_p      menu();
    void        menu_pop();
    uint        page();
    void        page(uint p);
    uint        pages();
    void        pages(uint p);
    uint        menu_planes();
    void        menus(uint count, cstring labels[], object_p function[]);
    void        menu(uint index, cstring label, object_p function);
    void        menu(uint index, symbol_p label, object_p function);
    void        marker(uint index, unicode mark, bool alignLeft = false);
    bool        menu_refresh(bool page0 = false);
    bool        menu_refresh(object::id menu, bool page0 = false);
    void        menu_auto_complete()    { autoComplete = true; }
    symbol_p    label(uint index);
    cstring     label_text(uint index);
    utf8        label_for_function_key(int key, size_t *sz = nullptr);
    utf8        label_for_function_key(size_t *sz = nullptr);

    bool        freeze(uint flags);

    void        draw_start(bool force, uint refresh = ~0U);
    void        draw_refresh(uint delay);
    void        draw_dirty(const rect &r);
    void        draw_dirty(coord x1, coord y1, coord x2, coord y2);
    uint        draw_refresh()          { return nextRefresh; }
    bool        draw_graphics(bool erase = false);

    bool        draw_header();                  // Left part of header
    bool        draw_battery(bool now=false);   // Rightmost part of header
    bool        draw_annunciators();            // Left of battery
    bool        draw_busy(unicode glyph, pattern col);
    rect        draw_busy_background();
    bool        draw_busy();
    bool        draw_idle();
    bool        draw_editor();
    bool        draw_stack();
    bool        draw_object(object_p obj, uint top, uint bottom);
    bool        draw_error();
    bool        draw_message(utf8 header, uint count, utf8 msg[]);
    bool        draw_message(cstring header, cstring = 0, cstring = 0);
    bool        draw_help();
    bool        draw_command();
    void        draw_user_command(utf8 cmd, size_t sz);
    bool        draw_stepping_object();
    void        dirty_all();

    bool        draw_menus();
    bool        draw_cursor(int show, uint ncursor);

    object_p    transient_object();
    bool        transient_object(object_p obj);

    modes       editing_mode()          { return mode; }
    void        editing_mode(modes m)   { mode = m; }
    void        stack_screen_top(int s) { stackTop = s; }
    int         stack_screen_top()      { return stackTop; }
    int         stack_screen_bottom()   { return stackBottom; }
    int         menu_screen_bottom()    { return menuHeight; }
    bool        showing_help()          { return help + 1 != 0; }
    bool        showing_graphics()      { return graphics; }
    uint        cursor_position()       { return cursor; }
    void        cursor_position(uint p) { cursor = p; dirtyEditor = true; edRows = 0; }
    bool        current_word(size_t &start, size_t &size);
    bool        current_word(utf8 &start, size_t &size);
    bool        at_end_of_number(bool want_polar=false);
    unicode     character_left_of_cursor();
    bool        replace_character_left_of_cursor(unicode code);
    bool        replace_character_left_of_cursor(symbol_p sym);
    bool        replace_character_left_of_cursor(utf8 text, size_t len);

    void        shift_plane(uint p) { shift = p & 1; xshift = p & 2; }
    uint        shift_plane()       { return xshift ? 2 : shift ? 1 : 0; }
    void        alpha_plane(uint p) { alpha = p != 0; lowercase = p == 2; }
    uint        alpha_plane()       { return alpha ? (lowercase ? 2 : 1) : 0; }
    void        clear_shift()       { xshift = shift = false; }
    void        clear_help();
    void        clear_menu();
    object_p    object_for_key(int key);
    int         evaluating_function_key() const;
    bool        end_edit();
    void        clear_editor();

    void        insert(unicode c, modes m, bool autoclose = true);
    result      insert(utf8 s, size_t len, modes m);
    result      insert(utf8 s, modes m);
    size_t      insert(size_t offs, utf8 data, size_t len);
    size_t      insert(size_t offs, unicode c);
    size_t      insert(size_t offs, byte c) { return insert(offs, &c, 1); }
    size_t      insert(size_t offs, char c) { return insert(offs, byte(c)); }
    result      insert_object(object_p obj, modes m);
    result      insert_object(object_p obj,
                              cstring bef="", cstring aft="",
                              bool midcursor = false);
    result      insert_softkey(int key,
                               cstring before, cstring after,
                               bool midcursor);
    size_t      remove(size_t offset, size_t len);
    size_t      replace(utf8 oldData, utf8 newData);

    bool        do_edit();
    bool        do_enter();
    bool        do_exit();
    bool        do_algebraic();
    bool        do_decimal_separator();
    bool        do_text();
    bool        do_left();
    bool        do_right();
    bool        do_up();
    bool        do_down();
    bool        do_delete(bool forward);

    text_p      editor_save(text_r ed, bool rewinding = false);
    text_p      editor_save(bool rewinding = false);
    bool        editor_history(bool back = false);
    bool        editor_select();
    bool        editor_word_left();
    bool        editor_word_right();
    bool        editor_begin();
    bool        editor_end();
    bool        editor_cut();
    bool        editor_copy();
    bool        editor_paste();
    bool        editor_search();
    bool        editor_replace();
    bool        editor_clear();
    bool        editor_selection_flip();
    size_t      adjust_cursor(size_t offset, size_t len);
    bool        in_input() const { return validate_input != nullptr; }
    void        input(bool (*fn)(gcutf8 &, size_t)) { validate_input = fn; }
    bool        check_input(gcutf8 &src, size_t len);

    void        load_help(utf8 topic, size_t len = 0);

    bool        load_keymap(cstring filename);

protected:
    bool        handle_screen_capture(int key);
    bool        handle_shifts(int &key, bool talpha);
    bool        handle_help(int &key);
    bool        handle_editing(int key);
    bool        handle_editing_command(object::id lower, object::id higher);
    bool        handle_alpha(int key);
    bool        handle_user(int key);
    bool        handle_functions(int key);
    bool        handle_functions(int key, object_p obj, bool user);
    bool        handle_digits(int key);
    bool        noHelpForKey(int key);
    bool        do_search(unicode with = 0, bool restart = false);


public:
    int      evaluating;        // Key being evaluated

protected:
    utf8     command;           // Command being executed
    uint     help;              // Offset of help being displayed in help file
    uint     line;              // Line offset in the help display
    uint     topic;             // Offset of topic being highlighted
    uint     topicsHistory;     // History depth
    uint     topics[8];         // Topics history
    grob_g   image;             // Image loaded in help file
    uint     impos;             // Position of image file
    uint     cursor;            // Cursor position in buffer
    uint     select;            // Cursor position for selection marker
    uint     searching;         // Searching start point
    coord    xoffset;           // Offset of the cursor
    modes    mode;              // Current editing mode
    int      last;              // Last key
    int      stackTop;          // Vertical top of the stack (bottom of header)
    int      stackBottom;       // Vertical bottom of the stack
    coord    cx, cy;            // Cursor position on screen
    uint     edRows;            // Editor rows
    int      edRow;             // Current editor row
    int      edColumn;          // Current editor column (in pixels)
    id       menuStack[HISTORY];// Current and past menus
    uint     pageStack[HISTORY];// Current and past menus pages
    uint     menuPage;          // Current menu page
    uint     menuPages;         // Number of menu pages
    uint     menuHeight;        // Height of the menu
    uint     busy;              // Busy counter
    coord    busyLeft;          // Left column for busy area in header
    coord    busyRight;         // Right column for busy area in header
    coord    batteryLeft;       // Left column for battery in header
    uint     time;              // Time at which we began drawing
    uint     nextRefresh;       // Time for next refresh
    int      lastShiftPlane;    // Last shift plane when drawing indicators
    uint     menuAnimate;       // Menu items to animate
    uint     menuDrawn;         // Last time the menu was drawn
    uint     cursorDrawn;       // Last time the cursor was drawn
    uint     customHeaderDrawn; // Last time the custom header was drawn
    uint     day, month, year;  // Date shown in header
    uint     dow;               // Day of week shown in header
    uint     hour, minute;      // Hour and minute shown
    uint     second;            // Second shown in header
    object_g editing;           // Object being edited if any
    uint     editingLevel;      // Stack level being edited
    uint     cmdIndex;          // Command index for next command to save
    uint     cmdHistoryIndex;   // Command index for next command history
    text_g   history[HISTORY];  // Command-line history
    text_g   clipboard;         // Clipboard for copy/paste operations
    bool     shift        : 1;  // Normal shift active
    bool     xshift       : 1;  // Extended shift active (simulate Right)
    bool     alpha        : 1;  // Alpha mode active
    bool     transalpha   : 1;  // Transitory alpha (up or down key)
    bool     lowercase    : 1;  // Lowercase
    bool     userOnce     : 1;  // User mode should be reset
    bool     shiftDrawn   : 1;  // Cache of drawn annunciators
    bool     xshiftDrawn  : 1;  // Cache
    bool     alphaDrawn   : 1;  // Cache
    bool     lowercDrawn  : 1;  // Cache
    bool     userDrawn    : 1;  // Cache
    bool     down         : 1;  // Move one line down
    bool     up           : 1;  // Move one line up
    bool     repeat       : 1;  // Repeat the key
    bool     longpress    : 1;  // We had a long press of the key
    bool     blink        : 1;  // Cursor blink indicator
    bool     follow       : 1;  // Follow a help topic
    bool     force        : 1;  // Force a redraw of everything
    bool     dirtyMenu    : 1;  // Menu label needs redraw
    bool     dirtyStack   : 1;  // Need to redraw the stack
    bool     dirtyCommand : 1;  // Need to redraw the command
    bool     dirtyEditor  : 1;  // Need to redraw the text editor
    bool     dirtyHelp    : 1;  // Need to redraw the help
    bool     autoComplete : 1;  // Menu is auto-complete
    bool     adjustSeps   : 1;  // Need to adjust separators
    bool     graphics     : 1;  // Displaying user-defined graphics screen
    bool     freezeHeader : 1;  // Freeze the header area
    bool     freezeStack  : 1;  // Freeze the stack area
    bool     freezeMenu   : 1;  // Freeze the menu area
    bool     doubleRelease: 1;  // Double release
    bool     batteryLow   : 1;  // Battery low indicator is shown

protected:
    // Key mappings
    list_p   keymap;
    object_p function[NUM_PLANES][NUM_SOFTKEYS];
    cstring  menuLabel[NUM_PLANES][NUM_SOFTKEYS];
    uint16_t menuMarker[NUM_PLANES][NUM_SOFTKEYS];
    bool     menuMarkerAlign[NUM_PLANES][NUM_SOFTKEYS];
    file     helpfile;
    bool     (*validate_input)(gcutf8 &src, size_t len);
    friend struct tests;
    friend struct runtime;
};


inline int user_interface::evaluating_function_key() const
// ----------------------------------------------------------------------------
//   Returns true if we are currently evaluating a function key
// ----------------------------------------------------------------------------
{
    return evaluating >= KEY_F1 && evaluating <= KEY_F6 ? evaluating : 0;
}


enum { TIMER0, TIMER1, TIMER2, TIMER3 };

extern user_interface ui;

#endif // INPUT_H
