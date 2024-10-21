// ****************************************************************************
//  user_interface.cc                                            DB48X project
// ****************************************************************************
//
//   File Description:
//
//     User interface for the calculator
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

#include "user_interface.h"

#include "arithmetic.h"
#include "blitter.h"
#include "characters.h"
#include "command.h"
#include "dmcp.h"
#include "expression.h"
#include "files.h"
#include "functions.h"
#include "graphics.h"
#include "grob.h"
#include "list.h"
#include "menu.h"
#include "precedence.h"
#include "program.h"
#include "runtime.h"
#include "settings.h"
#include "stack.h"
#include "symbol.h"
#include "sysmenu.h"
#include "target.h"
#include "utf8.h"
#include "util.h"

#ifdef SIMULATOR
#include "tests.h"
#endif // SIMULATOR

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <wctype.h>


// The primary user interface of the calculator
user_interface ui;

using std::max;
using std::min;

RECORDER(user_interface,16, "ui processing");
RECORDER(text_editor,   16, "Text editor");
RECORDER(menus,         16, "Menu operations");
RECORDER(help,          16, "On-line help");
RECORDER(help_search,   16, "On-line help topic search");
RECORDER(tests_ui,      16, "Test interaction with user interface");

#define NUM_TOPICS      (sizeof(topics) / sizeof(topics[0]))


user_interface::user_interface()
// ----------------------------------------------------------------------------
//   Initialize the user interface
// ----------------------------------------------------------------------------
    : evaluating(0),
      command(),
      help(-1u),
      line(0),
      topic(0),
      topics_history(0),
      topics(),
      image(nullptr),
      impos(0),
      cursor(0),
      select(~0U),
      searching(~0U),
      xoffset(0),
      mode(STACK),
      last(0),
      stack(LCD_H),
      cx(0),
      cy(0),
      edRows(0),
      edRow(0),
      edColumn(0),
      menuStack(),
      pageStack(),
      menuPage(),
      menuPages(),
      menuHeight(),
      busy(0),
      nextRefresh(~0U),
      dirty(),
      editing(),
      editingLevel(0),
      cmdIndex(0),
      clipboard(),
      shift(false),
      xshift(false),
      alpha(false),
      transalpha(false),
      lowercase(false),
      shift_drawn(false),
      xshift_drawn(false),
      alpha_drawn(false),
      lowerc_drawn(false),
      down(false),
      up(false),
      repeat(false),
      longpress(false),
      blink(false),
      follow(false),
      force(false),
      dirtyMenu(false),
      dirtyStack(false),
      dirtyCommand(false),
      dirtyHelp(false),
      autoComplete(false),
      adjustSeps(false),
      graphics(false),
      dbl_release(false),
      helpfile()
{
    for (uint p = 0; p < NUM_PLANES; p++)
    {
        for (uint k = 0; k < NUM_KEYS; k++)
            function[p][k] = nullptr;
        for (uint k = 0; k < NUM_SOFTKEYS; k++)
        {
            menu_label[p][k] = nullptr;
            menu_marker[p][k] = 0;
            menu_marker_align[p][k] = false;
        }
    }
}


static inline bool is_algebraic(user_interface::modes mode)
// ----------------------------------------------------------------------------
//   Return true for algebraic and parentheses mode
// ----------------------------------------------------------------------------
{
    return (mode == user_interface::ALGEBRAIC ||
            mode == user_interface::PARENTHESES);
}


void user_interface::insert(unicode c, modes m, bool autoclose)
// ----------------------------------------------------------------------------
//   Begin editing with a given character
// ----------------------------------------------------------------------------
{
    dirtyEditor = true;

    // If already editing, keep current mode
    if (rt.editing())
        m = mode;

    uint savec = cursor;
    insert(cursor, c);

    // Test delimiters
    unicode closing = 0;
    switch(c)
    {
    case '(':                   closing = ')';  m = PARENTHESES;        break;
    case '[':                   closing = ']';  m = MATRIX;             break;
    case '{':                   closing = '}';  m = PROGRAM;            break;
    case ':':  if (m != TEXT)   closing = ':';  m = DIRECT;             break;
    case '"':                   closing = '"';  m = TEXT;               break;
    case '\'':                  closing = '\''; m = ALGEBRAIC;          break;
    case L'«':                  closing = L'»'; m = PROGRAM;            break;
    case '_':                                   m = UNIT;               break;
    case '\n': edRows = 0;                    break; // Recompute rows
    }
    if (closing && autoclose)
    {
        byte *ed = rt.editor();
        if (mode == PROGRAM || mode == DIRECT ||
            (is_algebraic(mode) && c != '('))
            if (savec > 0 && ed[savec-1] != ' ' && ed[savec-1] != '=')
                insert(savec, ' ');
        size_t back = insert(cursor, closing);
        cursor -= back;
    }

    mode = m;
    adjustSeps = true;
}


object::result user_interface::insert(utf8 text, size_t len, modes m)
// ----------------------------------------------------------------------------
//   Enter the given text on the command line
// ----------------------------------------------------------------------------
{
    dirtyEditor = true;

    bool   editing     = rt.editing();
    byte  *ed          = rt.editor();
    bool   skip        = m == POSTFIX && is_algebraic(mode);
    bool   alpha_infix = m == INFIX && is_alpha(text);

    // Skip the x in postfix operators (x⁻¹, x², x³ or x!)
    if (skip)
    {
        text++;
        len--;
    }

    if (!editing)
    {
        cursor = 0;
        select = ~0U;
        dirtyStack = true;
    }
    else if (m == TEXT || mode == UNIT)
    {
    }
    else if ((!is_algebraic(mode) || !is_algebraic(m)) &&
             cursor > 0 && ed[cursor-1] != ' ')
    {
        if (!skip && (!is_algebraic(mode) || ((m != INFIX || alpha_infix) &&
                                              m != CONSTANT)))
        {
            gcutf8 s = text;
            insert(cursor, ' ');
            text = +s;
        }
    }

    uint   offset = 0;
    if (cstring ins = strchr(cstring(text), '\t'))
        offset = ins - cstring(text);

    uint   pos    = cursor;
    size_t added  = insert(cursor, text, len);

    if (m == TEXT || mode == UNIT)
    {
    }
    else if ((m == POSTFIX || (m == INFIX && !alpha_infix) || m == CONSTANT) &&
             is_algebraic(mode))
    {
        /* nothing */
    }
    else if (!is_algebraic(mode) || !is_algebraic(m) ||
             (m == INFIX && alpha_infix))
    {
        insert(cursor, ' ');
    }
    else if (m != INFIX)
    {
        if (insert(cursor, utf8("()"), 2) == 2)
            cursor--;
    }

    // Offset from beginning or end of inserted text
    if (offset)
    {
        cursor = pos + offset;
        remove(cursor, 1);
    }

    dirtyEditor = true;
    adjustSeps = true;
    update_mode();
    return added == len ? object::OK : object::ERROR;
}


object::result user_interface::insert(utf8 text, modes m)
// ----------------------------------------------------------------------------
//   Edit a null-terminated text
// ----------------------------------------------------------------------------
{
    return insert(text, strlen(cstring(text)), m);
}


bool user_interface::end_edit()
// ----------------------------------------------------------------------------
//   Clear the editor
// ----------------------------------------------------------------------------
{
    alpha       = false;
    lowercase   = false;
    shift       = false;
    xshift      = false;
    dirtyEditor = true;
    dirtyStack  = true;
    dirtyMenu   = true;

    searching   = ~0U;
    edRows      = 0;
    last        = 0;
    select      = ~0;

    clear_help();
    rt.clear_error();

    size_t edlen = rt.editing();
    if (edlen)
    {
        gcutf8  ed   = rt.editor();
        size_t  o    = 0;
        bool    text = false;
        unicode nspc = Settings.NumberSeparator();
        unicode hspc = Settings.BasedSeparator();

        draw_busy();

        // Save the command-line history (prior to removing spaces)
        text_g saved = text::make(ed, edlen);

        // Remove all additional decorative number spacing
        while (o < edlen)
        {
            unicode cp = utf8_codepoint(ed + o);
            if (cp == '"')
            {
                text = !text;
                o += 1;
            }
            else if (!text && (cp == nspc || cp == hspc))
            {
                size_t ulen = utf8_size(cp);
                ulen = remove(o, ulen);
                edlen -= ulen;
            }
            else
            {
                o += utf8_size(cp);
            }
        }

        text_g edstr = rt.close_editor();
        if (edstr)
        {
            gcutf8 editor = edstr->value();
            program_g cmds = program::parse(editor, edlen);
            if (cmds)
            {
                // We successfully parsed the line
                editor_save(saved, false);
                clear_editor();
                if (editingLevel)
                {
                    bool first = true;
                    for (auto obj : *cmds)
                    {
                        if (first)
                        {
                            rt.stack(editingLevel - 1, obj);
                            first = false;
                        }
                        else
                        {
                            rt.push(obj);
                            rt.rolld(editingLevel);
                        }
                    }
                    Stack.interactive = editingLevel;
                    editing = nullptr;
                    editingLevel = 0;
                    dirtyStack = true;
                }
                else
                {
                    editing = nullptr;
                    editingLevel = 0;
                    if (Settings.SaveStack())
                        rt.save();
                    save<bool> no_halt(program::halted, false);
                    cmds->run(Settings.SaveLastArguments());
                }
            }
            else
            {
                // Move cursor to error if there is one
                utf8   pos  = rt.source();
                utf8   ed   = editor;
                size_t slen = rt.source_length();
                if (pos >= editor && pos <= ed + edlen)
                    cursor = pos - ed;
                select = slen ? cursor + slen : ~0U;
                if (!rt.edit(ed, edlen))
                {
                    cursor = 0;
                    select = ~0U;
                }
                draw_idle();
                if (!rt.error())
                    rt.internal_error();
                return false;
            }
        }
        draw_idle();
    }

    return true;
}


void user_interface::clear_editor()
// ----------------------------------------------------------------------------
//   Clear the editor either after edit, or when pressing EXIT
// ----------------------------------------------------------------------------
{
    rt.clear();
    cursor      = 0;
    select      = ~0U;
    searching   = ~0U;
    xoffset     = 0;
    edRows      = 0;
    alpha       = false;
    shift       = false;
    xshift      = false;
    lowercase   = false;
    longpress   = false;
    repeat      = false;
    dirtyEditor = true;
    dirtyStack  = true;
    clear_help();
}


text_p user_interface::editor_save(bool rewinding)
// ----------------------------------------------------------------------------
//   Save current editor content for history
// ----------------------------------------------------------------------------
{
    if (rt.editing())
        if (text_g editor = rt.close_editor(false, false))
            return editor_save(editor, rewinding);
    return nullptr;
}


text_p user_interface::editor_save(text_r &editor, bool rewinding)
// ----------------------------------------------------------------------------
//   Save text as editor content for history
// ----------------------------------------------------------------------------
{
    bool found = false;
    uint base = rewinding ? cmdHistoryIndex : cmdIndex;
    for (uint h = 1; !found && h < HISTORY; h++)
    {
        uint i = (base + HISTORY - h) % HISTORY;
        if (history[i] && editor->is_same_as(history[i]))
        {
            std::swap(history[base], history[i]);
            found = true;
        }
    }
    if (!found)
        history[base] = editor;
    if (!rewinding)
    {
        cmdIndex = (cmdIndex + 1) % HISTORY;
        cmdHistoryIndex = cmdIndex;
    }
    return editor;
}


void user_interface::editor_history(bool back)
// ----------------------------------------------------------------------------
//   Restore editor buffer from history
// ----------------------------------------------------------------------------
{
    editor_save(true);
    for (uint h = 0; h < HISTORY; h++)
    {
        uint next = cmdHistoryIndex + (back ? 1 : HISTORY - 1);
        cmdHistoryIndex = next % HISTORY;
        if (history[cmdHistoryIndex])
        {
            size_t sz = 0;
            gcutf8 ed = history[cmdHistoryIndex]->value(&sz);
            rt.edit(+ed, sz);
            cursor = 0;
            select = ~0U;
            alpha = xshift = shift = false;
            edRows = 0;
            dirtyEditor = true;
            break;
        }
    }
    menu::static_object(menu::ID_EditMenu)->evaluate();
}


void user_interface::clear_help()
// ----------------------------------------------------------------------------
//   Clear help data
// ----------------------------------------------------------------------------
{
    command     = nullptr;
    help        = -1u;
    line        = 0;
    image       = nullptr;
    impos       = 0;
    topic       = 0;
    follow      = false;
    last        = 0;
    longpress   = false;
    repeat      = false;
    dirtyMenu   = true;
    dirtyHelp   = true;         // Need to redraw what is behind help
    dirtyEditor = true;
    dirtyStack  = true;
    helpfile.close();
}


void user_interface::clear_menu()
// ----------------------------------------------------------------------------
//   Clear the menu
// ----------------------------------------------------------------------------
{
    menu(nullptr, 0);
    menus(0, nullptr, nullptr);
}

bool user_interface::key(int key, bool repeating, bool talpha)
// ----------------------------------------------------------------------------
//   Process an input key
// ----------------------------------------------------------------------------
{
    int skey = key;

    if (handle_screen_capture(key))
        return true;

    longpress = key && repeating;
    record(user_interface,
           "Key %d shifts %d longpress", key, shift_plane(), longpress);
    repeat = false;

    if (rt.error())
    {
        if (key && Settings.NoNeedToClearErrors())
        {
            // Do not return true, handle the key as if there was no error
            // This is the way the HP48 and HP50 actually behave
            rt.clear_error();
            dirtyStack = true;
            dirtyEditor = true;
            if (key == KEY_EXIT || key == KEY_ENTER || key == KEY_BSP)
                return true;
        }
        else
        {
            if (key == KEY_EXIT || key == KEY_ENTER || key == KEY_BSP)
                rt.clear_error();
            else if (key == KEY_SHIFT)
                handle_shifts(key, talpha);
            else if (key)
                beep(2200, 75);
            dirtyStack = true;
            dirtyEditor = true;
            return true;
        }
    }

    // Handle keys
    bool result =
        handle_shifts(key, talpha)      ||
        handle_help(key)                ||
        handle_editing(key)             ||
        handle_alpha(key)               ||
        handle_digits(key)              ||
        handle_functions(key)           ||
        key == 0;

    if (rt.editing())
        update_mode();

    if (!skey && last != KEY_SHIFT)
    {
        shift = false;
        xshift = false;
        freezeHeader = false;
        freezeStack = false;
        freezeMenu = false;
        menu_refresh(menu::ID_Catalog);
    }

    if (!skey)
        command = nullptr;

    return result;
}


void user_interface::assign(int key, uint plane, object_p code)
// ----------------------------------------------------------------------------
//   Assign an object to a given key
// ----------------------------------------------------------------------------
{
    if (key >= 1 && key <= NUM_KEYS && plane <= NUM_PLANES)
        function[plane][key - 1] = code;
}


object_p user_interface::assigned(int key, uint plane)
// ----------------------------------------------------------------------------
//   Assign an object to a given key
// ----------------------------------------------------------------------------
{
    if (key >= 1 && key <= NUM_KEYS && plane <= NUM_PLANES)
        return function[plane][key - 1];
    return nullptr;
}


void user_interface::update_mode()
// ----------------------------------------------------------------------------
//   Scan the command line to check what the state is at the cursor
// ----------------------------------------------------------------------------
{
    utf8    ed    = rt.editor();
    utf8    last  = ed + cursor;
    uint    progs = 0;
    uint    lists = 0;
    uint    algs  = 0;
    uint    txts  = 0;
    uint    cmts  = 0;
    uint    vecs  = 0;
    uint    based = 0;
    uint    syms  = 0;
    uint    inum  = 0;
    uint    fnum  = 0;
    uint    hnum  = 0;
    uint    parn  = 0;
    uint    unit  = 0;
    unicode nspc  = Settings.NumberSeparator();
    unicode hspc  = Settings.BasedSeparator();
    unicode dmrk  = Settings.DecimalSeparator();
    unicode emrk  = Settings.ExponentSeparator();
    utf8    num   = nullptr;
    bool    inexp = false;

    mode = DIRECT;
    for (utf8 p = ed; p < last; p = utf8_next(p))
    {
        unicode code = utf8_codepoint(p);

        if (!txts && !cmts)
        {
            if (unit)
            {
                unit = code == byte(code) && isalnum(code);
                if (!unit)
                {
                    static utf8 valid = utf8("/÷×·^↑⁻¹²³");
                    for (utf8 p = valid; *p && !unit; p = utf8_next(p))
                        unit = code == utf8_codepoint(p);
                }
            }
            else if ((inum || fnum) && code == emrk)
            {
                inexp = true;
            }
            else if (inexp && (code == '-' || code == '+'))
            {
            }
            else if (code == nspc || code == hspc)
            {
                // Ignore all extra spacing in numbers
                if (!num)
                    num = p;
            }
            else if (based)
            {
                if  (code < '0'
                 || (code > '9' && code < 'A')
                 || (code > 'Z' && code < 'a')
                 || (code > 'z'))
                {
                    based = 0;
                }
                else
                {
                    if (!num)
                        num = p;
                    hnum++;
                }
                inexp = false;
            }
            else if (!syms && code >= '0' && code <= '9')
            {
                if (!num)
                    num = p;
                if (fnum)
                    fnum++;
                else
                    inum++;
                inexp = false;
            }
            else if (code == dmrk)
            {
                if (!num)
                    num = p;
                fnum = 1;
                inexp = false;
            }
            else if (code == '@')
            {
                cmts++;
            }
            else
            {
                // All other characters: reset numbering
                based = inum = fnum = hnum = unit = 0;
                num = nullptr;
                if (is_valid_as_name_initial(code))
                    syms = true;
                else if (syms && !is_valid_in_name(code))
                    syms = false;
                inexp = false;
            }

            switch(code)
            {
            case '\'':      algs = 1 - algs;                break;
            case '"':       txts = 1 - txts;                break;
            case '{':       lists++;                        break;
            case '}':       lists--;                        break;
            case '[':       vecs++;                         break;
            case ']':       vecs--;                         break;
            case '(':       parn++;                         break;
            case ')':       parn--;                         break;
            case L'«':      progs++;                        break;
            case L'»':      progs--;                        break;
            case '_':       unit = true;                    break;
            case '#':       based++;
                            hnum = inum = syms = 0;
                            num = nullptr;                  break;
            }
        }
        else if (txts && code == '"')
        {
            txts = 1 - txts;
        }
        else if (cmts && code == '\n')
        {
            cmts = 0;
        }
    }

    if (txts)
        mode = TEXT;
    else if (based)
        mode = BASED;
    else if (unit)
        mode = UNIT;
    else if (parn)
        mode = PARENTHESES;
    else if (algs)
        mode = vecs ? PARENTHESES : ALGEBRAIC;
    else if (vecs)
        mode = MATRIX;
    else if (lists || progs)
        mode = PROGRAM;
    else
        mode = DIRECT;

    if (adjustSeps)
    {
        if  ((inum || fnum || hnum) && num)
        {
            // We are editing some kind of number. Insert relevant spacing.
            size_t len = rt.editing();

            // First identify the number range and remove all extra spaces in it
            bool   isnum = true;
            size_t frpos = 0;
            size_t start = num - ed;
            size_t o     = start;

            while (o < len && isnum)
            {
                unicode code = utf8_codepoint(ed + o);

                // Remove all spacing in the range
                if (code == nspc || code == hspc)
                {
                    size_t rlen = utf8_size(code);
                    rlen = remove(o, rlen);
                    len -= rlen;
                    ed = rt.editor(); // Defensive coding (no move on remove)
                    continue;
                }

                isnum = ((code >= '0' && code <= '9')
                         || (hnum && ((code >= 'A' && code <= 'Z')
                                      || (code >= 'a' && code <= 'z')))
                         || code == '+'
                         || code == '-'
                         || code == '#'
                         || code == dmrk);
                if (code == dmrk)
                    frpos = o + 1;
                if (isnum)
                    o += utf8_size(code);
            }

            // Insert markers on the fractional part if necessary
            if (frpos)
            {
                byte   encoding[4];
                size_t ulen = utf8_encode(nspc, encoding);
                uint   sf   = Settings.FractionSpacing();
                size_t end  = o;

                o = frpos - 1;
                if (sf)
                {
                    frpos += sf;
                    while (frpos < end)
                    {
                        if (!insert(frpos, encoding, ulen))
                            break;
                        frpos += sf + ulen;
                        len += ulen;
                        end += ulen;
                    }
                }
            }

            // Then insert markers on the integral part
            byte encoding[4];
            uint sp = hnum ? Settings.BasedSpacing()
                           : Settings.MantissaSpacing();
            if (sp)
            {
                unicode spc  = hnum ? Settings.BasedSeparator()
                                    : Settings.NumberSeparator();
                size_t ulen = utf8_encode(spc, encoding);
                while (o > start + sp)
                {
                    o -= sp;
                    if (!insert(o, encoding, ulen))
                        break;
                }
            }
        }
        adjustSeps = false;
    }
}


bool user_interface::at_end_of_number(bool want_polar)
// ----------------------------------------------------------------------------
//   Check if we are at the end of a number in the editor
// ----------------------------------------------------------------------------
{
    size_t len       = rt.editing();
    utf8   ed        = rt.editor();
    utf8   last      = ed + len;
    utf8   curs      = ed + cursor;
    uint   lastnum   = ~0U;
    bool   quoted    = false;
    bool   numok     = true;
    bool   hadexp    = false;
    bool   inexp     = false;
    bool   had_polar = false;

    for (utf8 p = ed; p < last; p = utf8_next(p))
    {
        unicode code = utf8_codepoint(p);

        // Avoid text
        if (code == '"')
        {
            quoted = !quoted;
            continue;
        }
        if (quoted)
            continue;

        if (code >= '0' && code <= '9')
        {
            hadexp = false;
            if (numok)
                lastnum = p - ed;
            continue;
        }
        if (code == '+' || code == '-')
        {
            if (hadexp)
            {
                hadexp = false;
            }
            else if (~lastnum)       // 12+3: no longer have a number,
            {
                if (p >= curs)
                    break;
                lastnum = ~0U;
                numok = true;
                had_polar = false;
            }
            continue;
        }

        // Check characters accepted inside a number
        if (~lastnum)
        {
            // An exponent must be followed by numbers
            if (code == L'⁳' || code == 'E' || code == 'e')
            {
                hadexp = true;
                inexp = true;
                continue;
            }

            // A decimal separator
            if (code == '.'  || code == ',')
            {
                if (inexp)
                {
                    if (p >= curs)
                        break;
                    lastnum = ~0U;
                    numok = false;
                    inexp = hadexp = false;
                }
                else
                {
                    lastnum = p - ed;
                }
                continue;
            }
            if (code == settings::SPACE_DEFAULT || code == L'’' || code == '_')
                continue;
        }

        // If we had a space, keep position of last number, accept numbers
        if (isspace(code) || is_separator(code) ||
            code == complex::I_MARK || code == complex::ANGLE_MARK)
        {
            numok = true;
            inexp = false;
            had_polar = code == complex::ANGLE_MARK;
            if (p >= curs && !(want_polar && had_polar))
                break;
            continue;
        }

        // Any other character means we no longer have a number
        if (p < curs)
        {
            lastnum = ~0U;
            numok = false;
            hadexp = false;
            inexp = false;
        }
        else
        {
            // Past cursor: we are done searching
            break;
        }
    }

    // If lastnum was not found, say we have no number
    if (~lastnum == 0 || lastnum + 1 < cursor || (want_polar && !had_polar))
        return false;

    // Move cursor here
    cursor_position(lastnum + 1);
    select = ~0U;
    return true;
}


unicode user_interface::character_left_of_cursor()
// ----------------------------------------------------------------------------
//    Return the unicode character at left of cursor
// ----------------------------------------------------------------------------
{
    size_t edlen = rt.editing();
    utf8   ed    = rt.editor();
    if (!ed || edlen == 0)
        return 0;

    uint    ppos = utf8_previous(ed, cursor);
    utf8    prev = ed + ppos;
    unicode code = utf8_codepoint(prev);
    return code;
}


bool user_interface::replace_character_left_of_cursor(unicode code)
// ----------------------------------------------------------------------------
//   Replace a character code
// ----------------------------------------------------------------------------
{
    byte buf[4];
    size_t sz = utf8_encode(code, buf);
    return replace_character_left_of_cursor(buf, sz);
}


bool user_interface::replace_character_left_of_cursor(symbol_p sym)
// ----------------------------------------------------------------------------
//    Replace the character left of cursor with teh symbol
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8   txt = sym->value(&len);
    return replace_character_left_of_cursor(txt, len);
}


bool user_interface::replace_character_left_of_cursor(utf8 text, size_t len)
// ----------------------------------------------------------------------------
//   Replace the character left of cursor with the new text
// ----------------------------------------------------------------------------
{
    size_t edlen = rt.editing();
    utf8   ed    = rt.editor();
    if (ed && edlen)
    {
        uint ppos = utf8_previous(ed, cursor);
        if (ppos != cursor)
            remove(ppos, cursor - ppos);
    }
    insert(text, len, TEXT);
    return true;
}


void user_interface::menu(menu_p menu, uint page)
// ----------------------------------------------------------------------------
//   Set menu and page
// ----------------------------------------------------------------------------
{
    menu::id mid = menu ? menu->type() : menu::ID_object;

    record(menus, "Selecting menu %t page %u", menu, page);

    if (mid != *menuStack)
    {
        pageStack[0] = menuPage;
        memmove(menuStack+1, menuStack, sizeof(menuStack) - sizeof(*menuStack));
        memmove(pageStack+1, pageStack, sizeof(pageStack) - sizeof(*pageStack));
        if (menu)
        {
            menuStack[0] = mid;
            pageStack[0] = page;
            menu->update(page);
        }
        else
        {
            menuStack[0] = menu::ID_object;
        }
        menuPage = page;
        dirtyMenu = true;
    }

    for (uint i = 0; i < HISTORY; i++)
        record(menus, "  History %u %+s.%u",
               i, menu::name(menuStack[i]), pageStack[i]);
}


menu_p user_interface::menu()
// ----------------------------------------------------------------------------
//   Return the current menu
// ----------------------------------------------------------------------------
{
    return menuStack[0] ? menu_p(menu::static_object(menuStack[0])) : nullptr;
}


void user_interface::menu_pop()
// ----------------------------------------------------------------------------
//   Pop last menu in menu history
// ----------------------------------------------------------------------------
{
    id current = menuStack[0];
    uint cpage = pageStack[0];

    record(menus, "Popping menu %+s", menu::name(current));

    memmove(menuStack, menuStack + 1, sizeof(menuStack) - sizeof(*menuStack));
    memmove(pageStack, pageStack + 1, sizeof(pageStack) - sizeof(*pageStack));
    menuStack[HISTORY-1] = menu::ID_object;
    pageStack[HISTORY-1] = 0;
    for (uint i = 1; i < HISTORY; i++)
    {
        if (menuStack[i] == menu::ID_object)
        {
            menuStack[i] = current;
            pageStack[i] = cpage;
            break;
        }
    }
    menuPage = 0;
    if (menu::id mty = menuStack[0])
    {
        menu_p m = menu_p(menu::static_object(mty));
        menuPage = pageStack[0];
        m->update(menuPage);
    }
    else
    {
        menus(0, nullptr, nullptr);
    }
    dirtyMenu = true;

    for (uint i = 0; i < HISTORY; i++)
        record(menus, "  History %u %+s.%u",
               i, menu::name(menuStack[i]), pageStack[i]);
}


uint user_interface::page()
// ----------------------------------------------------------------------------
//   Return the currently displayed page
// ----------------------------------------------------------------------------
{
    return menuPage;
}


void user_interface::page(uint p)
// ----------------------------------------------------------------------------
//   Set the menu page to display
// ----------------------------------------------------------------------------
{
    menuPage = (p + menuPages) % menuPages;
    if (menu_p m = menu())
        m->update(menuPage);
    dirtyMenu = true;
}


uint user_interface::pages()
// ----------------------------------------------------------------------------
//   Return number of menu pages
// ----------------------------------------------------------------------------
{
    return menuPages;
}


void user_interface::pages(uint p)
// ----------------------------------------------------------------------------
//   Return number of menu pages
// ----------------------------------------------------------------------------
{
    menuPages = p ? p : 1;
}


void user_interface::menus(uint count, cstring labels[], object_p function[])
// ----------------------------------------------------------------------------
//   Assign all menus at once
// ----------------------------------------------------------------------------
{
    for (uint m = 0; m < NUM_MENUS; m++)
    {
        if (m < count)
            menu(m, labels[m], function[m]);
        else
            menu(m, cstring(nullptr), nullptr);
    }
    autoComplete = false;
}


void user_interface::menu(uint menu_id, cstring label, object_p fn)
// ----------------------------------------------------------------------------
//   Assign one menu item
// ----------------------------------------------------------------------------
{
    if (menu_id < NUM_MENUS)
    {
        int softkey_id       = menu_id % NUM_SOFTKEYS;
        int key              = KEY_F1 + softkey_id;
        int plane            = menu_id / NUM_SOFTKEYS;
        function[plane][key-1] = fn;
        menu_label[plane][softkey_id] = label;
        menu_marker[plane][softkey_id] = 0;
        menu_marker_align[plane][softkey_id] = false;
        dirtyMenu = true;       // Redraw menu
    }
}


void user_interface::menu(uint id, symbol_p label, object_p fn)
// ----------------------------------------------------------------------------
//   The drawing of menus recognizes symbols
// ----------------------------------------------------------------------------
{
    menu(id, (cstring) label, fn);
}


bool user_interface::menu_refresh()
// ----------------------------------------------------------------------------
//   Udpate current menu
// ----------------------------------------------------------------------------
{
    if (menuStack[0])
    {
        menu_p m = menu_p(menu::static_object(menuStack[0]));
        return m->update(menuPage) == object::OK;
    }
    return false;
}


bool user_interface::menu_refresh(object::id menu)
// ----------------------------------------------------------------------------
//   Request a refresh of a menu
// ----------------------------------------------------------------------------
{
    if (menuStack[0] == menu ||
        (menu == object::ID_VariablesMenu &&
         menuStack[0] == object::ID_SolvingMenu))
        return menu_refresh();
    return false;
}

void user_interface::marker(uint menu_id, unicode mark, bool alignLeft)
// ----------------------------------------------------------------------------
//   Record that we have a menu marker for this menu
// ----------------------------------------------------------------------------
{
    if (menu_id < NUM_MENUS)
    {
        int softkey_id       = menu_id % NUM_SOFTKEYS;
        int plane            = menu_id / NUM_SOFTKEYS;
        menu_marker[plane][softkey_id] = mark;
        menu_marker_align[plane][softkey_id] = alignLeft;
        dirtyMenu = true;
    }
}


symbol_p user_interface::label(uint menu_id)
// ----------------------------------------------------------------------------
//   Return the label for a given menu ID
// ----------------------------------------------------------------------------
{
    cstring lbl = label_text(menu_id);
    if (lbl && (*lbl == object::ID_symbol || *lbl == object::ID_text))
        return (symbol_p) lbl;
    return nullptr;
}


cstring user_interface::label_text(uint menu_id)
// ----------------------------------------------------------------------------
//   Return the label for a given menu ID
// ----------------------------------------------------------------------------
{
    int     softkey_id = menu_id % NUM_SOFTKEYS;
    int     plane      = menu_id / NUM_SOFTKEYS;
    cstring lbl        = menu_label[plane][softkey_id];
    return lbl;
}


uint user_interface::menu_planes()
// ----------------------------------------------------------------------------
//   Count menu planes
// ----------------------------------------------------------------------------
{
    int planes = 3;
    if (showing_help())
    {
        planes = 1;
    }
    else if (Stack.interactive)
    {
        planes = 3;
    }
    else if (Settings.HideEmptyMenu())
    {
        while (planes > 0)
        {
            bool found = false;
            for (uint sk = 0; !found && sk < NUM_SOFTKEYS; sk++)
                found = menu_label[planes-1][sk] != 0;
            if (found)
                break;
            planes--;
        }
    }
    return planes;
}


bool user_interface::freeze(uint flags)
// ----------------------------------------------------------------------------
//   Freeze the given areas
// ----------------------------------------------------------------------------
{
    if (flags & 1)
        freezeHeader = true;
    if (flags & 2)
        freezeStack = true;
    if (flags & 4)
        freezeMenu = true;
    graphics = true;
    return true;
}


void user_interface::draw_start(bool forceRedraw, uint refresh)
// ----------------------------------------------------------------------------
//   Start a drawing cycle
// ----------------------------------------------------------------------------
{
    dirty = rect();
    force = forceRedraw;
    nextRefresh = refresh;
    graphics = false;
}


void user_interface::draw_refresh(uint delay)
// ----------------------------------------------------------------------------
//   Indicates that a component expects a refresh in the given delay
// ----------------------------------------------------------------------------
{
    if (nextRefresh > delay)
        nextRefresh = delay;
}


void user_interface::draw_dirty(coord x1, coord y1, coord x2, coord y2)
// ----------------------------------------------------------------------------
//   Indicates that a component dirtied a given area of the screen
// ----------------------------------------------------------------------------
{
    draw_dirty(rect(min(x1,x2), min(y1,y2), max(x1,x2), max(y1,y2)));
}


void user_interface::draw_dirty(const rect &r)
// ----------------------------------------------------------------------------
//   Indicates that a component dirtied a given area of the screen
// ----------------------------------------------------------------------------
{
    if (dirty.empty())
        dirty = r;
    else
        dirty |= r;
}


bool user_interface::draw_graphics(bool erase)
// ----------------------------------------------------------------------------
//   Start graphics mode
// ----------------------------------------------------------------------------
{
    if (!graphics || erase)
    {
        draw_start(false);
        graphics = true;
        Screen.fill(pattern(Settings.Background()));
        draw_dirty(0, 0, LCD_W-1, LCD_H-1);
        return true;
    }
    return false;
}


bool user_interface::draw_menus()
// ----------------------------------------------------------------------------
//   Draw the softkey menus
// ----------------------------------------------------------------------------
{
    if (freezeMenu)
        return false;

    static int  lastp   = 0;
    static uint lastt   = 0;
    static uint animate = 0;
    uint        time    = sys_current_ms();
    int         shplane = shift_plane();
    uint        period  = usb_powered() ? 200 : 850;

    bool animating = animate && (time - lastt > period);
    bool redraw = dirtyMenu || shplane != lastp || animating;
    if (!force && !redraw)
        return false;

    if (force || dirtyMenu || shplane != lastp)
    {
        animate = 0;
        animating = false;
    }

    font_p font   = MenuFont;
    bool   square = Settings.SquareMenus();
    int    mh     = font->height() + 5 - square;
    int    mw     = (LCD_W - 10) / 6;
    int    sp     = (LCD_W - 5) - 6 * mw;
    rect   clip   = Screen.clip();
    bool   help   = showing_help();

    if (period > time - last)
        period = time - last;

    static unsigned menuShift = 0;
    menuShift++;

    int  planes        = menu_planes();
    id   menuStyle     = Settings.MenuAppearance();
    bool single        = menuStyle == object::ID_SingleRowMenus;
    bool flat          = menuStyle == object::ID_FlatMenus;
    int  visiblePlanes = single ? 1 : planes;
    uint newMenuHeight = visiblePlanes * mh;
    if (newMenuHeight != menuHeight)
    {
        menuHeight = newMenuHeight;
        dirtyStack = true;
        dirtyEditor = true;
    }

    if (flat)
    {
        object_p prevo = command::static_object(command::ID_MenuPreviousPage);
        object_p nexto = command::static_object(command::ID_MenuNextPage);
        object_p what = function[0][KEY_F6-1];
        bool prev = what == prevo;
        bool next = what == nexto;
        if (prev || next)
        {
            if (shplane != prev)
            {
                if (shplane)
                {
                    function[0][KEY_F6-1] = prevo;
                    menu_label[0][NUM_SOFTKEYS-1] = "◀︎";
                }
                else
                {
                    function[0][KEY_F6-1] = nexto;
                    menu_label[0][NUM_SOFTKEYS-1] = "▶";
                }
            }
        }
        shplane = 0;
    }

    settings::SaveTabWidth stw(0);
    for (int plane = 0; plane < planes; plane++)
    {
        cstring *labels = menu_label[plane];
        if (help)
        {
            static cstring helpMenu[] =
            {
                "Home", "Page▲", "Page▼", "Link▲", "Link▼", "← Topic"
            };
            labels = helpMenu;
        }
        else if (Stack.interactive)
        {
            static cstring stackMenu[] =
            {
                "Echo",    "Show",    "Eval",     "Roll↓",  "Pick",  "Edit",
                "DupN",    "DropN",   "Keep",     "Roll↑",  "→List", "Info",
                "EchoNSp", "ValSort", "MemSort",  "Revert", "Swap",  "Level"
            };
            labels = stackMenu + 6 * plane;
        }

        if (single)
            if (plane != shplane)
                continue;

        int my = LCD_H - (plane * !single + 1) * mh + square;
        if (force || dirtyMenu)
        {
            pattern mbg = Settings.StackBackground();
            Screen.fill(0, my, LCD_W-1, my+mh-1, mbg);
        }
        for (int m = 0; m < NUM_SOFTKEYS; m++)
        {
            uint animask = (1<<(m + plane * NUM_SOFTKEYS));
            if (animating && (~animate & animask))
                continue;

            coord x = (2 * m + 1) * mw / 2 + (m * sp) / 5 + 2;
            size mcw = mw;
            rect mrect(x - mw/2-1, my, x + mw/2, my+mh-1);
            if (animating)
                draw_dirty(mrect);

            bool alt = planes > 1 && plane != shplane;
            pattern color = alt
                ? Settings.RoundMenuBackground()
                : Settings.RoundMenuForeground();

            if (square)
            {
                mrect.x2++;
                mrect.y2++;
                color = Settings.SquareMenuForeground();
                pattern border = alt
                    ? Settings.SkippedMenuBackground()
                    : Settings.SelectedMenuForeground();
                Screen.fill(mrect, border);
                mrect.inset(1, 1);
                Screen.fill(mrect, pattern(Settings.SquareMenuBackground()));
                if (!alt)
                {
                    rect trect(x - mw/2-1, my, x + mw/2, my+1);
                    Screen.fill(trect, color);
                    trect.offset(0, mh-2);
                    Screen.fill(trect, color);
                }
            }
            else
            {
                pattern clr = Settings.MenuBackground();
                pattern bg  = Settings.RoundMenuBackground();
                pattern fg  = Settings.RoundMenuForeground();
                Screen.fill(mrect, clr);
                mrect.inset(3,  1);
                Screen.fill(mrect, bg);
                mrect.inset(-1, 1);
                Screen.fill(mrect, bg);
                mrect.inset(-1, 1);
                Screen.fill(mrect, bg);
                mrect.inset(2, 0);
                if (alt)
                    Screen.fill(mrect, fg);
                mrect.inset(0, -1);
            }


            utf8 label = utf8(labels[m]);
            if (label)
            {
                unicode marker = 0;
                coord   mkw    = 0;
                coord   mkx    = 0;

                size_t len = 0;
                if (*label == object::ID_symbol || *label == object::ID_text)
                {
                    COMPILE_TIME_ASSERT(object::ID_symbol < ' ');
                    COMPILE_TIME_ASSERT(object::ID_text < ' ');

                    // If we are given a symbol, use its length
                    label++;
                    len = leb128<size_t>(label);
                }
                else
                {
                    // Regular C string
                    len = strlen(cstring(label));
                }

                // Check if we have a marker from VariablesMenu
                rect trect = mrect;
                if (!help && !Stack.interactive)
                {
                    if (unicode mark = menu_marker[plane][m])
                    {
                        if (mark == 1)
                        {
                            mark = settings::MARK;
                        }
                        if (mark == L'░')
                        {
                            color = Settings.UnimplementedForeground();
                        }
                        else
                        {
                            bool alignLeft = menu_marker_align[plane][m];
                            marker         = mark;
                            mkw            = (marker == '/'
                                              ? 0
                                              : font->width(marker));
                            mkx            = alignLeft ? x - mw / 2 + 2
                                                       : x + mw / 2 - mkw - 2;
                            mcw -= mkw;
                            if (alignLeft)
                                trect.x1 += mkw;
                            else if (marker != L'◥')
                                trect.x2 -= mkw;
                        }
                    }
                }

                Screen.clip(trect);
                size tw = font->width(label, len);
                if (marker == '/')
                    tw += font->width(utf8("⁻¹"));
                if (tw + 2 >= mcw)
                {
                    animate |= animask;
                    x = mrect.x1 - menuShift % (tw - mcw + 5);
                }
                else
                {
                    x = (mrect.x1 + mrect.x2 - tw) / 2;
                }
                coord ty = mrect.y1 - 1;
                x = Screen.text(x, ty, label, len, font, color);
                if (marker)
                {
                    Screen.clip(mrect);
                    bool dossier = marker == L'◥';
                    if (dossier)
                    {
                        pattern fldcol = Settings.FolderCornerForeground();
                        if (alt || square)
                            Screen.glyph(mkx+3, ty-3, marker, font, color);
                        trect.inset(-2,-2);
                        Screen.clip(trect);
                        Screen.glyph(mkx+4, ty-4, marker, font, fldcol);
                    }
                    else if (marker == '/')
                    {
                        Screen.text(x, ty,
                                    utf8("⁻¹"), sizeof("⁻¹")-1, font, color);
                    }
                    else
                    {
                        Screen.glyph(mkx, ty, marker, font, color);
                    }
                }
                Screen.clip(clip);
            }
        }
    }
    if (square && shplane < visiblePlanes)
    {
        int my = LCD_H - (shplane * !single + 1) * mh;
        pattern sel = Settings.SelectedMenuForeground();
        Screen.fill(0, my, LCD_W-1, my, sel);
    }

    if (animate)
        draw_refresh(period);
    if (!animating)
        draw_dirty(0, LCD_H - 1 - menuHeight, LCD_W, LCD_H-1);

    lastp = shplane;
    lastt = time;
    dirtyMenu = false;

    return true;
}


static const size header_width = 248;


bool user_interface::draw_header()
// ----------------------------------------------------------------------------
//   Draw the header with the state name
// ----------------------------------------------------------------------------
{
    if (freezeHeader)
        return false;

    static uint day = 0, month = 0, year = 0;
    static uint hour = 0, minute = 0, second = 0;
    static uint dow = 0;
    bool changed = force;

    if (!changed || !day)
    {
        dt_t dt;
        tm_t tm;
        rtc_wakeup_delay();
        rtc_read(&tm, &dt);

        if (day != dt.day || month != dt.month || year != dt.year)
        {
            day = dt.day;
            month = dt.month;
            year = dt.year;
            changed = true;
        }
        if (hour != tm.hour || minute != tm.min || second != tm.sec)
        {
            hour = tm.hour;
            minute = tm.min;
            second = tm.sec;
            changed = true;
        }
        if (dow != tm.dow)
        {
            dow = tm.dow;
            changed = true;
        }
    }

    if (changed)
    {
        const coord hdr_right = header_width - 1;
        const coord hdr_bottom = HeaderFont->height() + 1;
        rect clip = Screen.clip();
        rect header = rect(0, 0, hdr_right, hdr_bottom);
        Screen.clip(header);
        Screen.fill(header, pattern(Settings.HeaderBackground()));

        coord  x  = 1;

        // Read the real-time clock
        if (Settings.ShowDate())
        {
            renderer r;
            char mname[4];
            if (Settings.ShowMonthName())
                snprintf(mname, 4, "%s", get_month_shortcut(month));
            else
                snprintf(mname, 4, "%d", month);
            char ytext[6];
            if (Settings.TwoDigitYear())
                snprintf(ytext, 6, "%02d", year % 100);
            else
                snprintf(ytext, 6, "%d", year);

            if (Settings.ShowDayOfWeek())
                r.printf("%s ", get_wday_shortcut(dow));

            char sep   = Settings.DateSeparator();
            uint index = 2 * Settings.YearFirst() + Settings.MonthBeforeDay();
            switch(index)
            {
            case 0: r.printf("%d%c%s%c%s ", day, sep, mname, sep, ytext); break;
            case 1: r.printf("%s%c%d%c%s ", mname, sep, day, sep, ytext); break;
            case 2: r.printf("%s%c%d%c%s ", ytext, sep, day, sep, mname); break;
            case 3: r.printf("%s%c%s%c%d ", ytext, sep, mname, sep, day); break;
            }
            pattern datecol = Settings.DateForeground();
            x = Screen.text(x, 0, r.text(), r.size(), HeaderFont, datecol);
        }
        if (Settings.ShowTime())
        {
            renderer r;
            r.printf("%d", Settings.Time24H() ? hour : hour % 12);
            r.printf(":%02d", minute);
            if (Settings.ShowSeconds())
                r.printf(":%02d", second);
            if (Settings.Time12H())
                r.printf("%c", hour < 12 ? 'A' : 'P');
            r.printf(" ");
            pattern timecol = Settings.TimeForeground();
            x = Screen.text(x, 0, r.text(), r.size(), HeaderFont, timecol);
        }

        renderer r;
        r.printf("%s", state_name());

        pattern namecol = Settings.StateNameForeground();
        x = Screen.text(x, 0, r.text(), r.size(), HeaderFont, namecol);
        Screen.clip(clip);
        draw_dirty(header);

        if (x > coord(header_width))
            x = header_width;
        busy_left = x;
    }
    if (Settings.ShowTime() || Settings.ShowDate())
        draw_refresh(Settings.ShowSeconds() ? 1000 : 1000 * (60 - second));
    return changed;
}


static const uint ann_width   = 15;
static const uint ann_height  = 12;
static const uint alpha_width = 30;

bool user_interface::draw_battery()
// ----------------------------------------------------------------------------
//    Draw the battery information
// ----------------------------------------------------------------------------
{
    if (freezeHeader)
        return false;

    static uint last       = 0;
    uint        time       = sys_current_ms();

    size        h          = HeaderFont->height() + 1;
    coord       ann_y      = (h - 1 - ann_height) / 2;

    // Print battery voltage
    static int  vdd = 3000;
    static bool low = false;
    static bool usb = false;

    uint delay = time - last;
    uint refresh = Settings.BatteryRefresh();
    if (delay > refresh)
    {
        vdd  = (int) read_power_voltage();
        low  = get_lowbat_state();
        usb  = usb_powered();
    }
    else if (!force)
    {
        delay = refresh - delay;
        if (delay < 20)
            delay = 20;
        draw_refresh(delay);
        return false;
    }

    // Experimentally, battery voltage below 2.6V cause calculator flakiness
    const int vmax = BATTERY_VMAX;
    const int vmin = BATTERY_VMIN;
    const int vhalf = (BATTERY_VMAX + BATTERY_VMIN) / 2;

    pattern   vpat  = usb          ? Settings.ChargingForeground()
                    : low          ? Settings.LowBatteryForeground()
                    : vdd <= vhalf ? Settings.HalfBatteryForeground()
                                   : Settings.BatteryLevelForeground();
    pattern   bg    = Settings.HeaderBackground();
    coord     x     = LCD_W - 1;

    if (Settings.ShowVoltage())
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d.%03dV", vdd / 1000, vdd % 1000);
        pattern vcol = Settings.VoltageForeground();
        if (vcol.bits == Settings.HeaderBackground())
            vcol = vpat;
        size w = HeaderFont->width(utf8(buffer));
        x -= w;

        rect bgr(x-4, 0, LCD_W-1, h);
        Screen.fill(bgr, bg);
        Screen.text(x, 0, utf8(buffer), HeaderFont, vcol);

        x -= 4;
    }

    size bat_width = 25;
    size bat_tipw = 3;

    x -= bat_width;

    rect  bat_bgr(x, 0, x + bat_width, h);
    Screen.fill(bat_bgr, bg);

    rect  bat_body(x + bat_tipw, ann_y,
                   x + bat_width - 1, ann_y + ann_height);
    pattern bfg = Settings.BatteryForeground();
    pattern bbg = Settings.BatteryBackground();

    rect bat_tip(x, ann_y + 3, x + 4, ann_y + ann_height - 3);
    Screen.fill(bat_tip, bfg);

    Screen.fill(bat_body, bfg);
    bat_body.inset(1,1);
    Screen.fill(bat_body, bbg);
    bat_body.inset(1,1);

    size batw = bat_body.width();
    size w = (vdd - vmin) * batw / (vmax - vmin);
    if (w > batw)
        w = batw;
    else if (w < 1)
        w = 1;
    bat_body.x1 = bat_body.x2 - w;
    Screen.fill(bat_body, vpat);

    if (!usb)
    {
        bat_body.x2 += 1;
        while (bat_body.x2 > x + 8)
        {
            bat_body.x2 -= 4;
            bat_body.x1 = bat_body.x2;
            Screen.fill(bat_body, bbg);
        }
    }

    battery_left = x;
    draw_dirty(x, 0, LCD_W-1, h);
    draw_refresh(refresh);
    last = time;

    // Power off if battery power is really low
    if (vdd < BATTERY_VOFF)
        power_off();

    return true;
}


static const byte ann_right[] =
// ----------------------------------------------------------------------------
//   Right-shift annunciator
// ----------------------------------------------------------------------------
{
    0xfe, 0x3f, 0xff, 0x7f, 0x9f, 0x7f,
    0xcf, 0x7f, 0xe7, 0x7f, 0x03, 0x78,
    0x03, 0x70, 0xe7, 0x73, 0xcf, 0x73,
    0x9f, 0x73, 0xff, 0x73, 0xfe, 0x33
};


static const byte ann_left[] =
// ----------------------------------------------------------------------------
//   Left-shift annunciator
// ----------------------------------------------------------------------------
{
    0xfe, 0x3f, 0xff, 0x7f, 0xff, 0x7c,
    0xff, 0x79, 0xff, 0x73, 0x0f, 0x60,
    0x07, 0x60, 0xe7, 0x73, 0xe7, 0x79,
    0xe7, 0x7c, 0xe7, 0x7f, 0xe6, 0x3f
};


bool user_interface::draw_annunciators()
// ----------------------------------------------------------------------------
//    Draw the annunciators for Shift, Alpha, etc
// ----------------------------------------------------------------------------
{
    if (freezeHeader)
        return false;

    bool adraw = force || alpha != alpha_drawn || lowercase != lowerc_drawn;
    bool sdraw = force || shift != shift_drawn || xshift != xshift_drawn;

    if (!adraw && !sdraw)
        return false;

    pattern bg      = Settings.HeaderBackground();
    size    h       = HeaderFont->height() + 1;
    size    alpha_w = alpha_width;
    coord   alpha_x = battery_left - alpha_w;
    coord   ann_x   = alpha_x - ann_width;

    if (!adraw && busy_right > alpha_x)
        adraw = true;
    if (!sdraw && busy_right > ann_x)
        sdraw = true;

    busy_right = battery_left - 1;
    if (adraw)
    {
        rect r = rect(alpha_x, 0, battery_left - 1, h);
        Screen.fill(r, bg);

        if (alpha)
        {
            utf8 label = utf8(lowercase ? "abc" : "ABC");
            pattern apat = lowercase
                ? Settings.LowerAlphaForeground()
                : Settings.AlphaForeground();
            Screen.text(alpha_x + 1, 0, label, HeaderFont, apat);
        }
        alpha_drawn = alpha;
        lowerc_drawn = lowercase;
    }
    if (alpha)
        busy_right = alpha_x - 1;

    if (sdraw)
    {
        coord       ann_y  = (h - ann_height) / 2;
        rect        ann(ann_x, 0, alpha_x - 1, h);
        Screen.fill(ann, bg);
        const byte *source = xshift ? ann_right : shift ? ann_left : nullptr;
        if (source)
        {
            pixword      *sw = (pixword *) source;
            grob::surface s(sw, ann_width, ann_height, 16);
            pattern       fg = shift
                ? Settings.LeftShiftForeground()
                : Settings.RightShiftForeground();
            pattern       bg = shift
                ? Settings.LeftShiftBackground()
                : Settings.RightShiftBackground();
            Screen.draw(s, ann_x, ann_y, fg);
            Screen.draw_background(s, ann_x, ann_y, bg);
        }
        shift_drawn = shift;
        xshift_drawn = xshift;
    }
    if (shift || xshift)
        busy_right = ann_x - 1;

    rect dirty(busy_right, 0, battery_left, h);
    draw_dirty(dirty);
    return true;
}


rect user_interface::draw_busy_background()
// ----------------------------------------------------------------------------
//   Draw the background behind the busy cursor and annunciators
// ----------------------------------------------------------------------------
{
    if (freezeHeader)
        return false;

    size h  = HeaderFont->height() + 1;
    pattern bg = Settings.HeaderBackground();
    rect busy(busy_left, 0, busy_right, h);
    Screen.fill(busy, bg);
    return busy;
}


bool user_interface::draw_busy()
// ----------------------------------------------------------------------------
//   Draw the default busy cursor
// ----------------------------------------------------------------------------
{
    return draw_busy(L'▶', Settings.RunningIconForeground());
}


bool user_interface::draw_busy(unicode glyph, pattern color)
// ----------------------------------------------------------------------------
//    Draw the busy flying cursor
// ----------------------------------------------------------------------------
{
    if (graphics || freezeHeader)
        return false;

    rect busy = draw_busy_background();
    if (glyph)
    {
        rect clip = Screen.clip();
        Screen.clip(busy);
        size  w = HeaderFont->width('M');
        coord x = busy.x1 + sys_current_ms() / 16 % (busy.width() - w);
        coord y = busy.y1;
        Screen.glyph(x, y, glyph, HeaderFont, color);
        Screen.clip(clip);
    }
    draw_dirty(busy);
    refresh_dirty();
    return true;
}


bool user_interface::draw_idle()
// ----------------------------------------------------------------------------
//   Clear busy indicator
// ----------------------------------------------------------------------------
{
    if (freezeHeader)
        return false;
    if (graphics)
    {
        record(tests_ui, "Waiting for key");
        graphics = false;
        wait_for_key_press();
        record(tests_ui, "Redraw LCD");
        redraw_lcd(true);
    }
    draw_busy(0, pattern::black);
    alpha_drawn = !alpha_drawn;
    shift_drawn = !shift;
    xshift_drawn = !xshift;
    draw_annunciators();
    refresh_dirty();
    return true;
}


bool user_interface::draw_editor()
// ----------------------------------------------------------------------------
//   Draw the editor
// ----------------------------------------------------------------------------
{
    if ((!force && !dirtyEditor) || freezeHeader)
        return false;

    record(text_editor, "Redrawing %+s %+s curs=%d, offset=%d cx=%d",
           dirtyEditor ? "dirty" : "clean",
           force ? "forced" : "lazy",
           cursor, xoffset, cx);

    // Get the editor area
    utf8   ed   = rt.editor();
    size_t len  = rt.editing();
    utf8   last = ed + len;
    dirtyEditor = false;

    if (!len)
    {
        // Editor is not open, compute stack bottom
        int ns = LCD_H - menuHeight;
        if (stack != ns)
        {
            stack = ns;
            dirtyStack = true;
        }
        return false;
    }

    // Select font
    font_p font = Settings.editor_font(false);

    // Count rows and colums
    int  rows   = 1;            // Number of rows in editor
    int  cwidth = 0;            // Column width
    int  edrow  = 0;            // Row number of line being edited
    int  cursx  = 0;            // Cursor X position
    bool found  = false;

    byte *wed = (byte *) ed;
    wed[len] = 0;               // Ensure utf8_next does not go into the woods

    // Count rows to check if we need to switch to stack font
reposition:
    if (!edRows)
    {
        for (utf8 p = ed; p < last; p = utf8_next(p))
            if (*p == '\n')
                rows++;
        edRows = rows;

        font = Settings.editor_font(rows > 2);

        rows = 1;
        for (utf8 p = ed; p < last; p = utf8_next(p))
        {
            if (p - ed == (int) cursor)
            {
                edrow = rows - 1;
                cursx = cwidth;
                found = true;
            }

            if (*p == '\n')
            {
                rows++;
                cwidth = 0;
            }
            else
            {
                unicode cp  = utf8_codepoint(p);
                cwidth     += font->width(cp);
            }
        }
        if (!found)
        {
            edrow = rows - 1;
            cursx = cwidth;
        }

        edRow = edrow;

        record(text_editor, "Computed: row %d/%d cursx %d (%d+%d=%d)",
               edrow, rows, cursx, cx, xoffset, cx+xoffset);
    }
    else
    {
        rows  = edRows;
        edrow = edRow;
        cursx = cx + xoffset;
        font = Settings.editor_font(rows > 2);

        record(text_editor, "Cached: row %d/%d cursx %d (%d+%d)",
               edrow, rows, cursx, cx, xoffset, cx+xoffset);
    }

    // Check if we want to move the cursor up or down
    if (up || down)
    {
        int   r    = 0;
        coord c    = 0;
        int   tgt  = edrow - (up && edrow > 0) + down;
        bool  done = up && edrow == 0;
        bool  repo = false;

        record(text_editor,
               "Moving %+s%+s edrow=%d target=%d curs=%d cursx=%d edcx=%d",
               up ? "up" : "", down ? "down" : "",
               edrow, tgt, cursor, cursx, edColumn);

        if (done)
        {
            cursor = 0;
            repo = true;
        }

        for (utf8 p   = ed; p < last && !done; p = utf8_next(p))
        {
            if (*p == '\n')
            {
                r++;
                if (r > tgt)
                {
                    cursor = p - ed;
                    edrow  = tgt;
                    done   = true;
                }
            }
            else if (r == tgt)
            {
                unicode cp = utf8_codepoint(p);
                c += font->width(cp);
                if (c > edColumn)
                {
                    cursor = p - ed;
                    edrow = r;
                    done = true;
                }
            }
        }
        if (!done && down)
        {
            cursor = len;
            edrow = rows - 1;
            repo = true;
        }
        record(text_editor, "Moved %+s%+s row=%d curs=%d",
               up ? "up" : "", down ? "down" : "",
               edrow, cursor);

        up   = false;
        down = false;
        edRow = edrow;
        if (repo)
        {
            edRows = 0;
            goto reposition;
        }
    }
    else
    {
        edColumn = cursx;
    }

    // Draw the area that fits on the screen
    int   lineHeight      = font->height();
    int   errorHeight     = rt.error() ? LCD_H / 3 + 10 : 0;
    int   bottom          = LCD_H-1 - menuHeight;
    int   top             = (Stack.interactive
                             ? bottom - lineHeight
                             : HeaderFont->height() + errorHeight + 2);
    int   availableHeight = bottom - top;
    int   fullRows        = availableHeight / lineHeight;
    int   clippedRows     = (availableHeight + lineHeight - 1) / lineHeight;
    utf8  display         = ed;
    coord y               = bottom - rows * lineHeight;

    blitter::rect clip = Screen.clip();
    Screen.clip(0, top, LCD_W, bottom);
    record(text_editor, "Clip between %d and %d", top, bottom);
    if (rows > fullRows)
    {
        // Skip rows to show the cursor
        int half = fullRows / 2;
        int skip = edrow < half         ? 0
                 : edrow >= rows - half ? rows - fullRows
                                        : edrow - half;
        record(text_editor,
               "Available %d, ed %d, displaying %d, skipping %d",
               fullRows,
               edrow,
               clippedRows,
               skip);

        for (int r = 0; r < skip; r++)
        {
            do
                display = utf8_next(display);
            while (*display != '\n');
        }
        if (skip)
            display = utf8_next(display);
        record(text_editor, "Truncated from %d to %d, text=%s",
               rows, clippedRows, display);
        rows = clippedRows;
        y = top;
    }


    // Draw the editor rows
    int  hskip = 180;
    size cursw = font->width('M');
    if (xoffset > cursx)
        xoffset = (cursx > hskip) ? cursx - hskip : 0;
    else if (coord(xoffset + LCD_W - cursw) < cursx)
        xoffset = cursx - LCD_W + cursw + hskip;

    coord x = -xoffset;
    int   r = 0;

    if (y < top)
        y = top;
    if (stack != y - 1)
    {
        stack      = y - 1;
        dirtyStack = true;
    }
    rect edbck(0, stack, LCD_W, bottom);
    Screen.fill(edbck, Settings.EditorBackground());
    draw_dirty(edbck);

    while (r < rows && display <= last)
    {
        bool atCursor = display == ed + cursor;
        if (atCursor)
        {
            cx = x;
            cy = y;
        }
        if (display >= last)
            break;

        unicode c   = utf8_codepoint(display);
        uint    pos = display - ed;
        bool    sel = ~select && int((pos - cursor) ^ (pos - select)) < 0;
        display     = utf8_next(display);
        if (c == '\n')
        {
            if (sel && x >= 0 && x < LCD_W)
                Screen.fill(x, y, LCD_W, y + lineHeight - 1,
                            Settings.SelectionBackground());
            y += lineHeight;
            x  = -xoffset;
            r++;
            continue;
        }
        int cw = font->width(c);
        if (x + cw >= 0 && x < LCD_W)
        {
            pattern fg = sel ? (~searching ? Settings.SearchForeground()
                                           : Settings.SelectionForeground())
                             : Settings.EditorForeground();
            pattern bg = sel ? (~searching ? Settings.SearchBackground()
                                           : Settings.SelectionBackground())
                             : Settings.EditorBackground();
            x          = Screen.glyph(x, y, c, font, fg, bg);
        }
        else
        {
            x += cw;
        }
    }
    if (cursor >= len)
    {
        cx = x;
        cy = y;
    }

    Screen.clip(clip);

    return true;
}


bool user_interface::draw_cursor(int show, uint ncursor)
// ----------------------------------------------------------------------------
//   Draw the cursor at the location
// ----------------------------------------------------------------------------
//   This function returns the cursor vertical position for screen refresh
{
    // Do not draw if not editing or if help is being displayed
    if (!rt.editing() || showing_help() || freezeStack)
        return false;

    static uint lastT = 0;
    uint time = sys_current_ms();
    const uint period = Settings.CursorBlinkRate();

    if (!force && !show && time - lastT < period)
    {
        draw_refresh(lastT + period - time);
        return false;
    }
    lastT = time;
    if (show)
        blink = show > 0;

    // Select editor font
    bool   ml         = edRows > 2;
    utf8   ed         = rt.editor();
    font_p edFont     = Settings.editor_font(ml);
    font_p cursorFont = Settings.cursor_font(ml);
    size_t len        = rt.editing();
    utf8   last       = ed + len;

    // Select cursor character
    unicode cursorChar = ~searching          ? 'S'
                       : mode == DIRECT      ? 'D'
                       : mode == TEXT        ? (lowercase ? 'L' : 'C')
                       : mode == PROGRAM     ? 'P'
                       : mode == ALGEBRAIC   ? 'A'
                       : mode == PARENTHESES ? 'E'
                       : mode == MATRIX      ? 'M'
                       : mode == BASED       ? 'B'
                       : mode == UNIT        ? 'U'
                                             : 'X';
    size    csrh       = cursorFont->height();
    coord   csrw       = cursorFont->width(cursorChar);
    size    ch         = edFont->height();

    coord   x          = cx;
    utf8    p          = ed + cursor;
    rect    clip       = Screen.clip();
    coord   ytop       = HeaderFont->height() + 2;
    coord   ybot       = LCD_H - menuHeight;

    Screen.clip(0, ytop, LCD_W, ybot);
    bool spaces = false;
    while (x <= cx + csrw + 1)
    {
        unicode cchar  = p < last ? utf8_codepoint(p) : ' ';
        if (cchar == '\n')
            spaces = true;
        if (spaces)
            cchar = ' ';

        size    cw  = edFont->width(cchar);
        bool    cur = x == cx && (!show || blink);

        // Write the character under the cursor
        uint    pos = p - ed;
        bool    sel = ~select && int((pos - ncursor) ^ (pos - select)) < 0;
        pattern fg  = sel ? (~searching ? Settings.SearchForeground()
                                        : Settings.SelectionForeground())
                          : Settings.EditorForeground();
        pattern bg  = sel ? (~searching ? Settings.SearchBackground()
                                        : Settings.SelectionBackground())
                    : cur ? Settings.CursorSelBackground()
                          : Settings.EditorBackground();
        x           = Screen.glyph(x, cy, cchar, edFont, fg, bg);
        draw_dirty(x, cy, x + cw - 1, cy + ch - 1);
        if (p < last)
            p = utf8_next(p);
    }

    if (blink)
    {
        coord csrx = cx;
        coord csry = cy + (ch - csrh)/2;
        Screen.invert(csrx, cy, csrx+1, cy + ch - 1);
        rect  r(csrx, csry - 1, csrx+csrw, csry + csrh);
        pattern border = alpha
            ? Settings.CursorAlphaBorder()
            : Settings.CursorBorder();
        pattern bg = alpha
            ? Settings.CursorAlphaBackground()
            : Settings.CursorBackground();
        pattern fg = alpha
            ? Settings.CursorAlphaForeground()
            : Settings.CursorForeground();
        Screen.fill(r, border);
        r.inset(1,1);
        Screen.fill(r, bg);
        Screen.glyph(csrx, csry, cursorChar, cursorFont, fg);
        draw_dirty(r);
    }

    blink = !blink;
    Screen.clip(clip);
    draw_refresh(period);
    return true;
}


bool user_interface::draw_command()
// ----------------------------------------------------------------------------
//   Draw the current command
// ----------------------------------------------------------------------------
{
    if (freezeStack)
        return false;

    if (force || dirtyCommand)
    {
        dirtyCommand = false;
        if (command && !rt.error())
        {
            font_p font = ReducedFont;
            size   w    = font->width(command);
            size   h    = font->height();
            coord  x    = 25;
            coord  y    = HeaderFont->height() + 6;

            pattern bg = Settings.CommandBackground();
            pattern fg = Settings.CommandForeground();
            Screen.fill(x - 2, y - 1, x + w + 2, y + h + 1, bg);
            Screen.text(x, y, command, font, fg);
            draw_dirty(x - 2, y - 1, x + w + 2, y + h + 1);
            return true;
        }
    }

    return false;
}


void user_interface::draw_user_command(utf8 cmd, size_t len)
// ----------------------------------------------------------------------------
//   Draw the current command
// ----------------------------------------------------------------------------
{
    if (freezeStack)
        return;

    font_p font = ReducedFont;
    size   w    = font->width(cmd, len);
    size   h    = font->height();
    coord  x    = 25;
    coord  y    = HeaderFont->height() + 6;

    // Erase normal command
    if (command)
    {
        size w = font->width(command);
        pattern bg = Settings.StackBackground();
        Screen.fill(x-2, y-1, x + w + 2, y + h + 1, bg);
    }

    // User-defined command, display in white
    pattern bg  = Settings.UserCommandBackground();
    pattern fg  = Settings.UserCommandForeground();
    pattern col = Settings.UserCommandBorder();
    rect    r(x - 2, y - 1, x + w + 2, y + h + 1);
    draw_dirty(r);
    Screen.fill(r, col);
    r.inset(1,1);
    Screen.fill(r, bg);
    Screen.text(x, y, cmd, len, font, fg);

    // Update screen
    refresh_dirty();
}


bool user_interface::draw_stepping_object()
// ----------------------------------------------------------------------------
//   Draw the next command to evaluate while stepping
// ----------------------------------------------------------------------------
{
    if (freezeStack)
        return false;

    if (object_g obj = rt.run_stepping())
    {
        renderer r(nullptr, 40);
        obj->render(r);
        draw_user_command(r.text(), r.size());
        draw_busy(L'♦', Settings.HaltedIconForeground());
        return true;
    }
    return false;
}


bool user_interface::draw_error()
// ----------------------------------------------------------------------------
//   Draw the error message if there is one
// ----------------------------------------------------------------------------
{
    if (utf8 err = rt.error())
    {
        const int border = 4;
        coord     top    = HeaderFont->height() + 10;
        coord     height = LCD_H / 3;
        coord     width  = LCD_W - 8;
        coord     x      = LCD_W / 2 - width / 2;
        coord     y      = top;

        rect clip = Screen.clip();
        rect r(x, y, x + width - 1, y + height - 1);
        draw_dirty(r);
        Screen.fill(r, Settings.ErrorBorder());
        r.inset(border);
        Screen.fill(r, Settings.ErrorBackground());
        r.inset(2);

        Screen.clip(r);
        pattern fg = Settings.ErrorForeground();
        if (text_p cmd = rt.command())
        {
            size_t sz = 0;
            utf8 cmdt = cmd->value(&sz);
            coord x = Screen.text(r.x1, r.y1, cmdt, sz, ErrorFont, fg);
            Screen.text(x, r.y1, utf8(" error:"), ErrorFont);
        }
        else
        {
            Screen.text(r.x1, r.y1, utf8("Error:"), ErrorFont, fg);
        }
        r.y1 += ErrorFont->height();
        Screen.text(r.x1, r.y1, err, ErrorFont, fg);
        Screen.clip(clip);

        refresh_dirty();
        if (uint freq = Settings.ErrorBeepFrequency())
            if (uint dur = Settings.ErrorBeepDuration())
                beep(freq, dur);
    }
    return true;
}


bool user_interface::draw_message(utf8 header, uint count, utf8 msgs[])
// ----------------------------------------------------------------------------
//   Draw an immediate message
// ----------------------------------------------------------------------------
{
    if (freezeStack)
        return false;

    font_p font   = LibMonoFont10x17;
    size   h      = font->height();
    size   ch     = h * 5 / 2 + h * count + 10;
    coord  top    = HeaderFont->height() + 10;
    size   height = ch < LCD_H / 3 ? LCD_H / 3 : ch;
    size   width  = LCD_W - 8;
    coord  x      = LCD_W / 2 - width / 2;
    coord  y      = top;
    rect   clip   = Screen.clip();
    rect   r(x, y, x + width - 1, y + height - 1);

    draw_dirty(r);
    Screen.fill(r, pattern::gray50);
    r.inset(1);
    Screen.fill(r, pattern::white);
    r.inset(1);
    Screen.fill(r, pattern::black);
    r.inset(2);
    Screen.fill(r, pattern::white);
    r.inset(2);

    Screen.clip(r);
    x = r.x1;
    y = r.y1;

    Screen.text(x+0, y, header, font);
    Screen.text(x+1, y, header, font);
    y += h * 3 / 2;

    for (uint i = 0; i < count; i++)
        if (msgs[i])
            Screen.text(x, y + i * h, msgs[i], font);

    Screen.clip(clip);
    refresh_dirty();

    return true;
}


bool user_interface::draw_message(cstring header, cstring msg1, cstring msg2)
// ----------------------------------------------------------------------------
//   Draw an immediate message in C string mode
// ----------------------------------------------------------------------------
{
    utf8 msgs[] = { utf8(msg1), utf8(msg2) };
    return draw_message(utf8(header), 2, msgs);
}


bool user_interface::draw_stack()
// ----------------------------------------------------------------------------
//   Redraw the stack if dirty
// ----------------------------------------------------------------------------
{
    if ((!force && !dirtyStack) || freezeStack)
        return false;
    draw_busy();
    uint top = HeaderFont->height() + 2;
    uint bottom = Stack.draw_stack();
    if (object_p transient = transient_object())
        draw_object(transient, top, bottom);
    draw_dirty(0, top, stack, LCD_H-1);
    draw_idle();
    dirtyStack = false;
    dirtyCommand = true;
    return true;
}


bool user_interface::draw_object(object_p objp, uint top, uint bottom)
// ----------------------------------------------------------------------------
//   Draw the current equation or other topical object if necessary
// ----------------------------------------------------------------------------
{
    auto fid = Settings.ResultFont();
    grapher g(LCD_W, bottom - top, fid,
              grob::pattern::black, grob::pattern::white,
              true);
    g.reduce_font();
    grob_g graph = nullptr;
    object_g obj = objp;
    do
    {
        graph = obj->graph(g);
    } while (!graph && !rt.error() &&
             Settings.AutoScaleStack() && g.reduce_font());

    if (graph)
    {
        grob::surface s  = graph->pixels();
        pattern       fg = Settings.ResultForeground();
        pattern       bg = Settings.ResultBackground();
        size          w  = s.width();
        size          h  = s.height();
        coord         x  = (LCD_W - w) / 2;
        coord         y  = top + 3;
        Screen.fill(x-1, y-1, x+w+1, y+h+1, pattern::gray50);
        Screen.draw(s, x, y, fg);
        Screen.draw_background(s, x, y, bg);
        draw_dirty(x-1, y-1, x+w, y+h);
    }
    return graph;
}


object_p user_interface::transient_object()
// ----------------------------------------------------------------------------
//   Return transient object to display if any
// ----------------------------------------------------------------------------
{
    object_p result = rt.editing() ? nullptr : editing;
    return result;
}


bool user_interface::transient_object(object_p obj)
// ----------------------------------------------------------------------------
//   Set transient object to draw on screen
// ----------------------------------------------------------------------------
{
    if (obj && !editing && !rt.editing())
    {
        editing = obj;
        dirtyStack = true;
        return true;
    }
    return false;
}


void user_interface::load_help(utf8 topic, size_t len)
// ----------------------------------------------------------------------------
//   Find the help message associated with the topic
// ----------------------------------------------------------------------------
{
    record(help, "Loading help topic %.*s", len, topic);

    if (!len)
        len = strlen(cstring(topic));
    command   = nullptr;
    follow    = false;
    dirtyHelp = true;
    dirtyMenu = true;

    if (!memcmp(topic, "http", 4))
    {
        static char buffer[50];
        snprintf(buffer, sizeof(buffer), "%.*s", int(len), topic);
        ui.draw_message("Local copy of the Internet not found",
                        "Please visit the following URL manually", buffer);
        wait_for_key_press();
        return;
    }

    // Check if this matches a command name. If so, we will search for
    // alternate spellings as well
    size_t     cmdlen = len;
    object::id cmd = command::lookup(topic, cmdlen);
    byte       ref[80];         // Length checked in the makefile
    size_t     refidx   = 0;
    if (cmdlen != len)
        cmd = object::id(0);
    record(help, "Length %u Command %u (%+s)", len, cmd, object::name(cmd));

    // Look for the topic in the file
    uint       level    = 0;
    bool       hadcr    = true;
    bool       matching = false;
    uint       topicpos = 0;
    bool       found    = false;
    uint       idxpos   = 0;

    // Check if the index exists. If so, scan it
    {
        file_closer hfc(helpfile);
        file index(HELPINDEX_NAME, false);
        if (index.valid())
        {
            for (char c = index.getchar(); !found && c; c = index.getchar())
            {
                if (c == '\n')
                {
                    uint filepos = atoi(cstring(ref));
                    byte_p p = ref;
                    byte_p end = ref + refidx;
                    while (p < end && *p++ != ':')
                        /* nop */;

                    level = 0;
                    while (p < end && *p++ == '#')
                        level++;
                    while (p < end && *p == ' ')
                        p++;

                    refidx = refidx - (p - ref);

                    // For regular topics, just to a string comparison
                    // We match markdown hyperlink style, i.e. case independent
                    // and matching '-' in the topic to ' ' in the text
                    found = refidx == len;
                    for (uint i = 0; found && i < len; i++)
                        found = (tolower(p[i]) == tolower(topic[i]) ||
                                 (p[i] == ' ' && topic[i] == '-'));

                    // If we do not match a direct comparison, check spellings
                    // We only do that for second and third level sections
                    if (!found && cmd && level >= 2)
                        found = command::lookup(p, refidx) == cmd;

                    if (found)
                    {
                        idxpos = filepos;
                        break;
                    }
                    refidx = 0;
                }
                else
                {
                    ref[refidx++] = c;
                }
            }

            // Not found in index, quit
            if (!found)
                goto notfound;
            found = false;
        }
    }


    // Need to have the help file open here
    if (!helpfile.valid())
    {
        helpfile.open(HELPFILE_NAME);
        if (!helpfile.valid())
        {
            help = -1u;
            line = 0;
            return;
        }
    }

    helpfile.seek(idxpos);
    for (char c = helpfile.getchar(); !found && c; c = helpfile.getchar())
    {
        // Reset topic after newline
        if (hadcr)
        {
            if (c == '#')
                topicpos = helpfile.position() - 1;
            refidx = level = 0;
            matching = false;
        }

        // Check if we start a line with # or ##, deduce topic level
        if (c == '#' && (hadcr || !refidx))
        {
            level += c == '#';
            matching = true;
            refidx = 0;
        }
        else if (matching)
        {
            if (c != '\n')
            {
                // Accumulate comparison topic
                if (refidx < sizeof(ref) - 1)
                    if (refidx || c != ' ')
                        ref[refidx++] = c;
            }
            else
            {
                ref[refidx] = 0;
                record(help_search, "Checking %u: %s", level, ref);

                // For regular topics, just to a string comparison
                // We match markdown hyperlink style, i.e. case independent
                // and matching '-' in the topic to ' ' in the text
                found = refidx == len;
                for (uint i = 0; found && i < len; i++)
                    found = (tolower(ref[i]) == tolower(topic[i]) ||
                             (ref[i] == ' ' && topic[i] == '-'));

                // If we do not match a direct comparison, check all spellings
                // We only do that for second and third level sections
                if (!found && cmd && level >= 2)
                    found = command::lookup(ref, refidx) == cmd;
            }
        }
        hadcr = c == '\n';
    }

    // Check if we found the topic
    if (found)
    {
        help = topicpos;
        line = 0;
        record(help, "Found topic %s at position %u level %u",
               topic, helpfile.position(), level);

        if (topics_history >= NUM_TOPICS)
        {
            // Overflow, keep the last topics
            for (uint i = 1; i < NUM_TOPICS; i++)
                topics[i - 1]   = topics[i];
            topics[topics_history - 1] = help;
        }
        else
        {
            // New topic, store it
            topics[topics_history++] = help;
        }
    }
    else
    {
    notfound:
        static char buffer[50];
        snprintf(buffer, sizeof(buffer), "No help for %.*s", int(len), topic);
        rt.command(object::static_object(object::ID_Help));
        rt.error(buffer);
    }
}


struct style_description
// ----------------------------------------------------------------------------
//   A small struct recording style
// ----------------------------------------------------------------------------
{
    font_p  font;
    pattern color;
    pattern background;
    bool    bold      : 1;
    bool    italic    : 1;
    bool    underline : 1;
    bool    box       : 1;
};


enum style_name
// ----------------------------------------------------------------------------
//   Style index
// ----------------------------------------------------------------------------
{
    TITLE,
    SUBTITLE,
    NORMAL,
    BOLD,
    ITALIC,
    CODE,
    VERBATIM,
    KEY,
    TOPIC,
    HIGHLIGHTED_TOPIC,
    NUM_STYLES
};


bool user_interface::draw_help()
// ----------------------------------------------------------------------------
//    Draw the help content
// ----------------------------------------------------------------------------
{
restart:
    if ((!force && !dirtyHelp && !dirtyStack) || freezeStack)
        return false;
    dirtyHelp = false;

    if (!showing_help())
        return false;

    using p                                    = pattern;
    const style_description styles[NUM_STYLES] =
    // -------------------------------------------------------------------------
    //  Table of styles
    // -------------------------------------------------------------------------
    {
        { HelpTitleFont,    p::black,  p::white,  false, false, false, false },
        { HelpSubTitleFont, p::black,  p::gray50,  true, false, true,  false },
        { HelpFont,         p::black,  p::white,  false, false, false, false },
        { HelpBoldFont,     p::black,  p::white,   true, false, false, false },
        { HelpItalicFont,   p::black,  p::white,  false, true,  false, false },
        { HelpCodeFont,     p::black,  p::gray50, false, false, false, true  },
        { HelpCodeFont,     p::black,  p::gray90, false, false, false, false },
        { HelpFont,         p::white,  p::black,  false, false, false, false },
        { HelpFont,         p::black,  p::gray50, false, false, true,  false },
        { HelpFont,         p::white,  p::gray10, false, false, false, false },
    };


    // Compute the size for the help display
    coord      ytop   = HeaderFont->height() + 2;
    coord      ybot   = LCD_H - (MenuFont->height() + 5);
    coord      xleft  = 0;
    coord      xright = LCD_W - 1;
    style_name style  = NORMAL;
    bool       imdsp  = false;


    // Clear help area and add some decorative elements
    rect clip = Screen.clip();
    rect r(xleft, ytop, xright, ybot);
    draw_dirty(r);
    Screen.fill(r, pattern::gray50);
    r.inset(2);
    Screen.fill(r, pattern::black);
    r.inset(2);
    Screen.fill(r, pattern::white);

    // Clip drawing area in case some text does not really fit
    r.inset(1);
    Screen.clip(r);

    // Update drawing area
    ytop   = r.y1;
    ybot   = r.y2;
    xleft  = r.x1 + 2;
    xright = r.x2;


    // Select initial state
    font_p  font      = styles[style].font;
    coord   height    = font->height();
    coord   x         = xleft;
    coord   y         = ytop + 2 - line;
    unicode last      = '\n';
    uint    lastTopic = 0;
    uint    codeStart = 0;
    uint    shown     = 0;
    bool    hadTitle  = false;

    // Pun not indented
    helpfile.seek(help);

    // Display until end of help
    while (y < ybot)
    {
        byte       buffer[80];
        uint       widx    = 0;
        bool       emit    = false;
        bool       newline = false;
        bool       yellow  = false;
        bool       blue    = false;
        style_name restyle = style;

        if (!shown)
        {
            if (y >= ytop)
            {
                shown  = helpfile.position();
            }
            else if (last == '\n' && line > 0 && y < ytop - 2*LCD_H)
            {
                help = helpfile.position();
                line = ytop + 2 - y;
            }
        }

        while (!emit)
        {
            unicode ch   = helpfile.get();
            bool    skip = false;

            if (style == VERBATIM && ch != '`')
            {
                newline = ch == '\n';
                emit = ch == '\n' || ch == ' ';
            }
            else switch (ch)
            {
            case 0:
                emit = true;
                skip = true;
                newline = true;
                break;

            case ' ':
                if (style <= SUBTITLE)
                {
                    skip = last == '#';
                    break;
                }
                skip = last == ' ';
                emit = style != KEY && style != CODE;
                break;

            case '\n':
                if (last == '\n' || last == ' ' || style <= SUBTITLE)
                {
                    emit    = true;
                    skip    = true;
                    newline = last != '\n' || helpfile.peek() != '\n';
                    while (helpfile.peek() == '\n')
                        helpfile.get();
                    restyle = NORMAL;
                }
                else
                {
                    uint    off  = helpfile.position();
                    unicode nx   = helpfile.get();
                    unicode nnx  = helpfile.get();
                    if (nx      == '#' || (nx == '*' && nnx == ' '))
                    {
                        newline = true;
                        emit = true;
                    }
                    else
                    {
                        ch   = ' ';
                        emit = true;
                    }
                    helpfile.seek(off);
                }
                break;

            case '#':
                if (last == '#' || last == '\n')
                {
                    if (restyle == TITLE)
                        restyle = SUBTITLE;
                    else
                        restyle = TITLE;
                    skip        = true;
                    emit        = true;
                    newline     = restyle == TITLE && last != '\n';
                }
                break;

            case '!':
                // Process images
                // A valid image will look something like:
                // ![Left Shift](left-shift.bmp)
                // We can only load one image at a time
                if (last == '\n')
                {
                    unicode c = helpfile.get();
                    bool isimg = c == '[';
                    if (isimg)
                    {
                        while (c && c != ']' && c != '\n')
                            c = helpfile.get();
                        isimg = (c == ']');
                    }
                    if (isimg)
                    {
                        c = helpfile.get();
                        isimg = (c == '(');
                    }
                    text_g name;
                    if (isimg)
                    {
                        scribble scr;
                        while (isimg && c && c != ')' && c != '\n')
                        {
                            c = helpfile.get();
                            size_t sz = utf8_size(c);
                            if (byte *p = rt.allocate(sz))
                                utf8_encode(c, p);
                            else
                                isimg = false;
                        }
                        isimg = c == ')';
                        if (isimg)
                        {
                            byte *ext = scr.scratch() + scr.growth() - 4;
                            isimg =
                                ext[0] == 'b' &&
                                ext[1] == 'm' &&
                                ext[2] == 'p';
                        }
                        if (isimg)
                        {
                            name = text::make(scr.scratch(), scr.growth()-1);
                            isimg = name;
                        }
                    }
                    if (isimg)
                    {
                        uint offs = helpfile.position();
                        if (offs != impos)
                        {
                            helpfile.close();

                            if (files_g files = files::make("help"))
                                image = files->recall_grob(name);
                            impos = offs;

                            helpfile.open(HELPFILE_NAME);
                            helpfile.seek(offs);
                        }
                        imdsp = true;
                        emit = true;
                    }
                }
                skip = true;
                break;

            case '<':
                // Skip HTML tags
                if (last == '\n')
                {
                    unicode c = helpfile.get();
                    while (c != '\n' && c != unicode(EOF))
                        c = helpfile.get();
                }
                skip = true;
                break;

            case '*':
                if (last == '\n' && helpfile.peek() == ' ')
                {
                    restyle = NORMAL;
                    ch      = L'●'; // L'■'; // L'•';
                    xleft   = r.x1 + 2 + font->width(utf8("● "));
                    break;
                }
                // Fall-through
            case '_':
                if (style != CODE)
                {
                    //   **Hello** *World*
                    //   IB.....BN I.....N
                    if (last == ch)
                    {
                        if (style   == BOLD)
                            restyle  = NORMAL;
                        else
                            restyle  = BOLD;
                    }
                    else
                    {
                        style_name disp  = ch == '_' ? KEY : ITALIC;
                        if (style       == BOLD)
                            restyle = BOLD;
                        else if (style  == disp)
                            restyle      = NORMAL;
                        else
                            restyle      = disp;
                    }
                    skip = true;
                    emit = true;
                }
                break;

            case '`':
                if (last != '`' && helpfile.peek() != '`')
                {
                    restyle = style == CODE ? NORMAL : CODE;
                    skip    = true;
                    emit    = true;
                }
                else
                {
                    if (widx == 2 && buffer[0] == '`' && buffer[1] == '`')
                    {
                        bool wasCode = style == VERBATIM;
                        skip = true;
                        emit = true;
                        newline = true;
                        restyle = wasCode ? NORMAL : VERBATIM;

                        // Skip until end of line, e.g. ```rpl
                        widx = 0;
                        do
                        {
                            ch = helpfile.get();
                            buffer[widx] = ch;
                            if (widx < 3)
                                widx++;
                        } while (ch && ch != '\n');
                        bool rpl = (widx == 3 &&
                                    tolower(buffer[0]) == 'r' &&
                                    tolower(buffer[1]) == 'p' &&
                                    tolower(buffer[2]) == 'l' &&
                                    buffer[3] == '\n');
                        widx = 0;

                        if (rpl)
                            codeStart = helpfile.position();
                    }
                }
                break;

            case '[':
                if (style != CODE)
                {
                    if (helpfile.peek() != '!')
                    {
                        lastTopic      = helpfile.position();
                        if (topic < shown)
                            topic      = lastTopic;
                        if (lastTopic == topic)
                        {
                            restyle    = HIGHLIGHTED_TOPIC;
                            codeStart  = 0;
                        }
                        else
                        {
                            restyle    = TOPIC;
                        }
                        skip           = true;
                        emit           = true;
                    }
                    else
                    {
                        // Link to a picture, skip it
                        unicode c = helpfile.get();
                        while (c != '\n' && c != unicode(EOF))
                            c = helpfile.get();
                        skip = true;
                    }
                }
                break;
            case ']':
                if (style == TOPIC || style == HIGHLIGHTED_TOPIC)
                {
                    unicode n  = helpfile.get();
                    if (n != '(')
                    {
                        ch      = n;
                        restyle = NORMAL;
                        emit = true;
                        break;
                    }

                    static char  link[60];
                    char        *p = link;
                    while (n != ')')
                    {
                        n      = helpfile.get();
                        if (n != '#')
                            if (p < link + sizeof(link))
                                *p++ = n;
                    }
                    p[-1] = 0;
                    if (follow && style == HIGHLIGHTED_TOPIC)
                    {
                        if (topics_history)
                            topics[topics_history-1] = shown;
                        load_help(utf8(link));
                        Screen.clip(clip);
                        goto restart;
                    }
                    restyle = NORMAL;
                    emit    = true;
                    skip    = true;
                }
                break;
            case L'🟨':
                emit = true;
                yellow = true;
                break;
            case L'🟦':
                emit = true;
                blue = true;
                break;
            default:
                break;
            }

            if (!skip)
                widx += utf8_encode(ch, buffer+widx);
            if (widx         >= sizeof(buffer) / sizeof(buffer[0]) - 3)
                emit          = true;
            last              = ch;
        }

        // Select font and color based on style
        font              = styles[style].font;
        height            = font->height();

        // If we are rendering a command name, follow user preferences
        if (style == SUBTITLE || style == CODE)
        {
            size_t cmdlen = widx;
            if (object::id cmd = command::lookup(buffer, cmdlen))
            {
                if (cmdlen == widx)
                {
                    object_p cmdobj = object::static_object(cmd);
                    widx = cmdobj->render((char *) buffer, sizeof(buffer) - 1);

                    if (style == SUBTITLE)
                    {
                        font_p  af    = HelpFont;
                        coord ay    = y;
                        coord ah    = af->height();
                        int   count = 0;

                        cstring ref = cstring(buffer);
                        if (widx >= sizeof(buffer))
                            widx = sizeof(buffer)-1;
                        buffer[widx] = 0;

                        for (size_t i = 0; i < object::spelling_count; i++)
                            if (cmd == object::spellings[i].type)
                                if (cstring name = object::spellings[i].name)
                                    if (strcasecmp(name, ref))
                                        count++;
                        if (count * ah > height * 5 / 4)
                            ay -= count * ah - height * 5 / 4;

                        for (size_t i = 0; i < object::spelling_count; i++)
                        {
                            if (cmd == object::spellings[i].type)
                            {
                                if (cstring name = object::spellings[i].name)
                                {

                                    if (strcasecmp(name, ref))
                                    {
                                        size awidth = af->width(utf8(name));
                                        coord ax = xright - awidth - 1;
                                        Screen.text(ax, ay, utf8(name), af);
                                        ay += ah;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Compute width of word (or words in the case of titles)
        coord width = font->width(buffer, widx);
        size kwidth = 0;
        if (style == KEY)
        {
            kwidth = 2*font->width(' ');
            width += 2*kwidth;
        }

        if (style <= SUBTITLE)
        {
            // Center titles
            x  = (LCD_W - width) / 2;
            if (widx && !hadTitle)
                y += (4 - 3 * style) * height / 4;
            hadTitle = true;
        }
        else
        {
            // Go to new line if this does not fit
            coord right  = x + width;
            if (right >= xright - 1)
            {
                x = xleft;
                y += height;
            }
            if (widx && hadTitle)
            {
                y += 5 * height / 4;
                hadTitle = false;
            }
        }

        coord yf = y + height;
        bool draw = yf > ytop;

        pattern color     = styles[style].color;
        pattern bg        = styles[style].background;
        bool    bold      = styles[style].bold;
        bool    italic    = styles[style].italic;
        bool    underline = styles[style].underline;
        bool    box       = styles[style].box;

        // Draw a decoration
        coord xl = x;
        coord xr = x + width;

        if (restyle == style && x == xleft)
            if (style == VERBATIM)
                Screen.fill(xleft, y - 2, xright, y + height * 5 / 4 - 3, bg);

        if (underline)
        {
            if (draw)
            {
                xl -= 2;
                xr += 2;
                Screen.fill(xl, yf, xr, yf, bg);
                xl += 2;
                xr -= 2;
            }
        }
        else if (box)
        {
            if (draw)
            {
                xl += 1;
                xr += 8;
                Screen.fill(xl, yf, xr, yf, bg);
                Screen.fill(xl, y, xl, yf, bg);
                Screen.fill(xr, y, xr, yf, bg);
                Screen.fill(xl, y, xr, y, bg);
                xl -= 1;
                xr -= 8;
            }
            kwidth += 4;
        }
        else if (bg.bits != pattern::white.bits)
        {
            if (draw)
                Screen.fill(xl, y, xr, yf, bg);
        }

        // Draw next word
        for (int i = 0; i < 1 + 3 * italic; i++)
        {
            if (draw)
            {
                x = xl + kwidth;
                if (italic)
                {
                    coord yt  = y + (3-i) * height / 4;
                    coord yb  = y + (4-i) * height / 4;
                    x        += i;
                    rect itr(x, yt, xr + i, yb);
                    itr &= r;
                    Screen.clip(itr);
                }
                coord x0 = x;
                for (int b = 0; b <= bold; b++)
                    x = Screen.text(x0+b, y, buffer, widx, font, color);
            }
            else
            {
                x += bold + font->width(buffer, widx);
            }
            x += kwidth;
        }
        if (italic)
            if (draw)
                Screen.clip(r);

        // Check special case of yellow shift key
        if (yellow || blue)
        {
            if (draw)
            {
                const byte   *source = blue ? ann_right : ann_left;
                pixword      *sw     = (pixword *) source;
                grob::surface s(sw, ann_width, ann_height, 16);
                pattern       fg    = yellow ? Settings.LeftShiftForeground()
                                             : Settings.RightShiftForeground();
                pattern       bg    = yellow ? Settings.LeftShiftBackground()
                                             : Settings.RightShiftBackground();

                coord ann_x = x + 4;
                coord ann_y = y + (height - ann_height)/2;
                Screen.fill(x, y, x+ann_width+7, y+height, pattern::black);
                Screen.draw(s, ann_x, ann_y, fg);
                Screen.draw_background(s, ann_x, ann_y, bg);
            }
            yellow = blue = false;
            x += ann_width + 7 + font->width(' ');
        }

        // Check if we need to draw the image
        if (imdsp)
        {
            if (image)
            {
                grob::surface srcs = image->pixels();
                rect drect = srcs.area();
                coord yimg = y + drect.height();
                if (yimg > ytop)
                {
                    coord cx = (LCD_W - drect.width()) / 2;
                    Screen.copy(srcs, cx, y);
                }
                y += drect.height();
            }
            imdsp = false;
        }

        if (newline)
        {
            xleft  = r.x1 + 2;
            x = xleft;
            if (!hadTitle)
                y += height * 5 / 4;
        }
        if (style <= SUBTITLE)
            y += height / 2;

        // Select style for next round
        style = restyle;
    }

    if (helpfile.position() < topic)
        topic = lastTopic;

    Screen.clip(clip);

    if (follow && codeStart)
    {
        helpfile.seek(codeStart);
        uint codeEnd = 0;
        uint markers = 0;
        while (unicode c = helpfile.get())
        {
            if (c == '`')
            {
                if (!markers)
                    codeEnd = helpfile.position() - 2; // Remove last \n
                markers++;
                if (markers == 3)
                    break;
            }
            else
            {
                markers = 0;
            }
        }
        helpfile.seek(codeStart);
        while (helpfile.position() < codeEnd)
        {
            unicode c = helpfile.get();
            insert(c, TEXT, false);
        }
        clear_help();
        dirtyHelp = true;
    }
    follow = false;
    return true;
}


bool user_interface::noHelpForKey(int key)
// ----------------------------------------------------------------------------
//   Return true if key requires immediate action, no help displayed
// ----------------------------------------------------------------------------
{
    bool editing  = rt.editing();

    // Show help for Duplicate and Drop only if not editing
    if (key == KEY_ENTER || key == KEY_BSP)
        return editing;

    // No help in alpha mode
    if (alpha && key < KEY_F1)
        return true;
    if (transalpha)
        return true;

    if (editing)
    {
        // No help for ENTER or BSP key while editing
        if (key == KEY_ENTER || key == KEY_BSP ||
            key == KEY_UP || key == KEY_DOWN || key == KEY_RUN)
            return true;

        // No help for A-F keys in hexadecimal entry mode
        if (mode == BASED && (key >= KB_A && key <= KB_F))
            return true;
    }

    // No help for digits entry
    if (!shift && !xshift)
        if (key > KEY_ENTER && key < KEY_ADD &&
            key != KEY_SUB && key != KEY_MUL && key != KEY_DIV && key != KEY_RUN)
            return true;

    // Other cases are regular functions, we can display help
    return false;
}



bool user_interface::handle_screen_capture(int key)
// ----------------------------------------------------------------------------
//   Check if we need to do a screen capture
// ----------------------------------------------------------------------------
{
    if (key >= KEY_SCREENSHOT)
    {
        if (key == KEY_SCREENSHOT)
        {
            shift = xshift = alpha = longpress = repeat = false;
            last = 0;
            draw_annunciators();
            refresh_dirty();
            if (!screenshot())
                rt.screenshot_capture_error();
        }
        if (key == KEY_DOUBLE_RELEASE)
            dbl_release = true; // Ignore next key
        return true;
    }
    if (!key && dbl_release)
    {
        dbl_release = false;
        return true;
    }
    return false;
}


bool user_interface::handle_help(int &key)
// ----------------------------------------------------------------------------
//   Handle help keys when showing help
// ----------------------------------------------------------------------------
{
    if (!showing_help())
    {
        // Exit if we are editing or entering digits
        if (last == KEY_SHIFT || Stack.interactive)
            return false;

        // Check if we have a long press, if so load corresponding help
        if (key)
        {
            if (noHelpForKey(key))
                return false;

            record(help,
                   "Looking for help topic for key %d, long = %d shift=%d\n",
                   key, longpress, shift_plane());
            if (object_p obj = object_for_key(key))
            {
                record(help, "Looking for help topic for key %d\n", key);
                save<int> seval(evaluating, key);
                if (utf8 htopic = obj->help())
                {
                    record(help, "Help topic is %s\n", htopic);
                    if (!rt.editing())
                    {
                        command = htopic;
                        dirtyCommand = true;
                    }
                    if (longpress)
                    {
                        rt.command(command::static_object(object::ID_Help));
                        load_help(htopic);
                        if (rt.error())
                        {
                            key  = 0; // Do not execute a function if no help
                            last = 0;
                        }
                    }
                    else
                    {
                        repeat = true;
                    }
                    return true;
                }
            }
            key = 0;
        }
        else
        {
            if (!noHelpForKey(last))
                key = last;     // Time to evaluate
            last    = 0;
        }

        // Help keyboard movements only applies when help is shown
        return false;
    }

    // Help is being shown - Special keyboard mappings
    uint count = shift ? 8 : 1;
    size height = HelpFont->height();
    switch (key)
    {
    case KEY_F1:
        load_help(utf8("Overview"));
        break;
    case KEY_F2:
        count = 8;
        // Fallthrough
    case KEY_UP:
    case KEY_8:
    case KEY_SUB:
        if (line > count * height)
        {
            line -= count * height;
        }
        else
        {
            line = 0;
            count++;
            while(count--)
            {
                helpfile.seek(help);
                help = helpfile.rfind('\n');
                if (!help)
                    break;
            }
            if (help)
                help = helpfile.position();
        }
        repeat = true;
        dirtyHelp = true;
        break;

    case KEY_F3:
        count   = 8;
        // Fall through
    case KEY_DOWN:
    case KEY_2:
    case KEY_ADD:
        line   += count * height;
        repeat  = true;
        dirtyHelp = true;
        break;

    case KEY_F4:
    case KEY_9:
    case KEY_DIV:
        ++count;
        while(count--)
        {
            helpfile.seek(topic);
            topic = helpfile.rfind('[');
        }
        topic  = helpfile.position();
        repeat = true;
        dirtyHelp = true;
        break;
    case KEY_F5:
    case KEY_3:
    case KEY_MUL:
        helpfile.seek(topic);
        while (count--)
            helpfile.find('[');
        topic  = helpfile.position();
        repeat = true;
        dirtyHelp = true;
        break;

    case KEY_ENTER:
        follow = true;
        dirtyHelp = true;
        break;

    case KEY_F6:
    case KEY_BSP:
        if (topics_history)
        {
            --topics_history;
            if (topics_history)
            {
                help = topics[topics_history-1];
                line = 0;
                dirtyHelp = true;
                break;
            }
        }
        // Otherwise fall-through and exit
        [[fallthrough]];

    case KEY_EXIT:
        clear_help();
        dirtyHelp = true;
        break;
    }
    return true;
}


bool user_interface::handle_shifts(int &key, bool talpha)
// ----------------------------------------------------------------------------
//   Handle status changes in shift keys
// ----------------------------------------------------------------------------
{
    bool consumed = false;

    // Transient alpha management
    if (!transalpha)
    {
        // Not yet in trans alpha mode, check if we need to enable it
        if (talpha)
        {
            if (key == KEY_UP || key == KEY_DOWN)
            {
                // Let menu and normal keys go through
                if (xshift)
                    return false;

                // Delay processing of up or down until after delay
                if (longpress)
                {
                    repeat = true;
                    return false;
                }

                last = key;
                repeat = true;
                lowercase = key == KEY_DOWN;
                return true;
            }
            else if (key)
            {
                // A non-arrow key was pressed while arrows are down
                alpha = true;
                transalpha = true;
                last = 0;
                return false;
            }
            else
            {
                // Swallow the last key pressed (up or down)
                key = 0;
                last = 0;
                return true;
            }
        }
        else if (!key && (last == KEY_UP || last == KEY_DOWN))
        {
            if (!longpress)
                key = last;
            last = 0;
            return false;
        }
    }
    else
    {
        if (!talpha)
        {
            // We released the up/down key
            transalpha = false;
            alpha = false;
            lowercase = false;
            key = 0;
            last = 0;
            return true;
        }
        else if (key == KEY_UP || key == KEY_DOWN || key == 0)
        {
            // Ignore up/down or release in trans-alpha mode
            last = 0;
            return true;
        }

        // Other keys will be processed as alpha
    }


    if (key == KEY_SHIFT)
    {
        if (longpress)
        {
            alpha = !alpha;
            lowercase = false;
            xshift = 0;
            shift = 0;
        }
        else if (xshift)
        {
            xshift = false;
        }
        else
        {
            xshift = false;
#define SHM(d, x, s) ((d << 2) | (x << 1) | (s << 0))
#define SHD(d, x, s) (1 << SHM(d, x, s))
            // Double shift toggles xshift
            bool dshift = last == KEY_SHIFT;
            int  plane  = SHM(dshift, xshift, shift);
            const unsigned nextShift =
                SHD(0, 0, 0) | SHD(0, 1, 0) | SHD(1, 0, 0);
            const unsigned nextXShift =
                SHD(0, 0, 1) | SHD(0, 1, 0) | SHD(0, 1, 1) | SHD(1, 0, 1);
            shift  = (nextShift  & (1 << plane)) != 0;
            xshift  = (nextXShift & (1 << plane)) != 0;
            repeat = true;
        }
        consumed = true;
        menu_refresh(menu::ID_Catalog);
#undef SHM
#undef SHD
    }
    else if (shift && key == KEY_ENTER)
    {
        // Cycle ABC -> abc -> non alpha
        if (alpha)
        {
            if (lowercase)
                alpha = lowercase = false;
            else
                lowercase = true;
        }
        else
        {
            alpha     = true;
            lowercase = false;
        }
        consumed = true;
        shift = false;
        key = last = 0;
    }

    if (key)
        last = key;
    return consumed;
}


bool user_interface::handle_editing(int key)
// ----------------------------------------------------------------------------
//   Some keys always deal with editing
// ----------------------------------------------------------------------------
{
    bool   consumed = false;
    size_t isEditing  = rt.editing();

    if (uint interactive = Stack.interactive)
    {
        uint amount;
        uint digit = ~0U;
        switch (key)
        {
        case KEY_UP:
            interactive += shift ? 4 : xshift ? 100 : 1;
            if (interactive > rt.depth())
                interactive = rt.depth();
            Stack.interactive = interactive;
            dirtyStack = true;
            break;
        case KEY_DOWN:
            amount = shift ? 4 : xshift ? 100 : 1;
            if (interactive <= amount)
                interactive = 1;
            else
                interactive -= amount;
            Stack.interactive = interactive;
            dirtyStack = true;
            break;
        case KEY_EXIT:
            if (shift)          // Power off
                return false;
        case KEY_ENTER:
            Stack.interactive = 0;
            dirtyStack = true;
            dirtyMenu = true;
            last = 0;
            break;
        case KEY_BSP:
            rt.roll(interactive);
            rt.drop();
            if (interactive > rt.depth())
                Stack.interactive = rt.depth();
            dirtyStack = true;
            break;
        case KEY_F1:
            if (shift)
            {
                // DupN
                for (uint i = 0; i < interactive; i++)
                    if (object_p obj = rt.stack(interactive - 1))
                        if (!rt.push(obj))
                            break;
            }
            else
            {
        case KEY_RUN:
                if (object_p obj = rt.stack(interactive - 1))
                {
                    insert_object(obj);
                    if (!shift && !xshift)
                        insert(unicode(' '), PROGRAM, false);
                    edRows = 0;
                    dirtyEditor = true;
                }
            }
            dirtyStack = true;
            break;
        case KEY_F2:        // DropN
            if (xshift)
            {
                // Sort by value
                typedef int (*qsort_fn)(const void *, const void*);
                qsort(rt.stack_base(), interactive,
                      sizeof(object_p), qsort_fn(value_compare));
            }
            else if (shift)
            {
                rt.drop(interactive);
                Stack.interactive = 1;
            }
            else
            {
        case KEY_DOT:
                // Show
                if (object_g obj = rt.stack(interactive - 1))
                    show(obj);
            }
            dirtyStack = true;
            break;
        case KEY_F3:
            if (xshift)
            {
                // Sort by memory representation
                typedef int (*qsort_fn)(const void *, const void*);
                qsort(rt.stack_base(), interactive,
                      sizeof(object_p), qsort_fn(memory_compare));
            }
            else if (shift)
            {
                // Keep
                if (rt.depth() > interactive)
                {
                    size_t depth = rt.depth();
                    for (uint i = 0; i < interactive; i++)
                        rt.stack(depth + ~i, rt.stack(interactive + ~i));
                    rt.drop(depth - interactive);
                }
            }
            else
            {
                // Evaluate the given item and insert results in stack
                rt.roll(interactive);
                if (object_g obj = rt.pop())
                {
                    size_t odepth = rt.depth();
                    obj->evaluate();
                    size_t ndepth = rt.depth();
                    size_t diff = ndepth - odepth - 1;
                    while (ndepth > odepth)
                    {
                        rt.rolld(interactive + diff);
                        odepth++;
                    }
                }
            }
            dirtyStack = true;
            break;
        case KEY_F4:
            if (xshift)
            {
                // Revert
                for (uint i = 0; i < interactive / 2; i++)
                {
                    object_p a = rt.stack(i);
                    object_p b = rt.stack(interactive + ~i);
                    rt.stack(i, b);
                    rt.stack(interactive + ~i, a);
                }
            }
            else if (shift)
            {
                // Roll
                rt.roll(interactive);
            }
            else
            {
                // Rolld
                rt.rolld(interactive);
            }
            dirtyStack = true;
            break;
        case KEY_F5:
            if (xshift)
            {
                // Swap
                goto swap;
            }
            else if (shift)
            {
                // To List
                to_list(interactive);
                Stack.interactive = 1;
            }
            else
            {
                // Pick
                if (object_p obj = rt.stack(interactive - 1))
                    rt.push(obj);
            }
            dirtyStack = true;
            break;
        case KEY_F6:
            if (xshift)
            {
                // Put level on the stack
                if (integer_p depth = integer::make(interactive))
                    rt.push(depth);
            }
            else if (shift)
            {
                // Info
                if (object_g obj = rt.stack(interactive - 1))
                {
                    size_t size = obj->size();
                    size_t maxsize = (Settings.WordSize() + 7) / 8;
                    size_t hashsize = size > maxsize ? maxsize : size;
                    gcbytes bytes = byte_p(obj);
#if CONFIG_FIXED_BASED_OBJECTS
                    // Force base 16 if we have that option
                    const object::id type = object::ID_hex_bignum;
#else // !CONFIG_FIXED_BASED_OBJECTS
                    const object::id type = object::ID_based_bignum;
#endif // CONFIG_FIXED_BASED_OBJECTS
                    if (bignum_g bin = rt.make<bignum>(type, bytes, hashsize))
                    {
                        char sizebuf[20];
                        char valbuf[40];
                        snprintf(sizebuf, sizeof(sizebuf), "%zu bytes", size);
                        bin->render(valbuf, sizeof(valbuf));
                        draw_message("Object info", sizebuf, valbuf);
                        wait_for_key_press();
                    }
                }
            }
            else
            {
                // Edit
                if (object_p obj = rt.stack(interactive - 1))
                {
                    editingLevel = interactive;
                    editing = obj;
                    insert_object(obj);
                    edRows = 0;
                    dirtyEditor = true;
                    dirtyStack = true;
                    Stack.interactive = 0;
                    last = 0;
                }
            }
            dirtyStack = true;
            break;

        case KEY_SWAP:
            if (xshift)
            {
                // UNDO: Exit stack and restore stack
                Stack.interactive = 0;
                return false;
            }
            else if (shift)
            {
            }
            else
            {
            swap:
                // Swap
                if (interactive < rt.depth())
                {
                    object_p x = rt.stack(interactive - 1);
                    object_p y = rt.stack(interactive);
                    rt.stack(interactive,     x);
                    rt.stack(interactive - 1, y);
                    dirtyStack = true;
                }
            }
            break;

        case KEY_0:     digit = 0; break;
        case KEY_1:     digit = 1; break;
        case KEY_2:     digit = 2; break;
        case KEY_3:     digit = 3; break;
        case KEY_4:     digit = 4; break;
        case KEY_5:     digit = 5; break;
        case KEY_6:     digit = 6; break;
        case KEY_7:     digit = 7; break;
        case KEY_8:     digit = 8; break;
        case KEY_9:     digit = 9; break;

        default:
            break;
        }

        if (digit != ~0U)
        {
            interactive = 10 * interactive + digit;
            if (interactive > rt.depth() && digit)
                interactive = digit;
            if (interactive > rt.depth())
                interactive = rt.depth();
            Stack.interactive = interactive;
            dirtyStack = true;
        }

        // Ignore all other keys in interactive mode
        return true;
    }

    // Some editing keys that do not depend on data entry mode
    if (!alpha)
    {
        switch(key)
        {
        case KEY_XEQ:
            // XEQ is used to enter algebraic / equation objects
            if ((!isEditing  || mode != BASED) && !shift && !xshift)
            {
                bool is_eqn = isEditing && is_algebraic(mode);
                insert(is_eqn ? '(' : '\'', ALGEBRAIC);
                last = 0;
                return true;
            }
            break;
        case KEY_RUN:
            if (shift)
            {
                // Shift R/S = PRGM enters a program symbol
                if (isEditing && (mode == ALGEBRAIC || mode == PARENTHESES))
                    insert('=', ALGEBRAIC);
                else
                    insert(L'«', PROGRAM);
                last = 0;
                return true;
            }
            else if (xshift)
            {
                insert('{', PROGRAM);
                last = 0;
                return true;
            }
            else if (isEditing)
            {
                // Stick to space role while editing, do not EVAL, repeat
                if (mode == PARENTHESES)
                    insert(';', PARENTHESES);
                else if (mode == ALGEBRAIC)
                    insert('=', ALGEBRAIC);
                else
                    insert(' ', PROGRAM);
                repeat = true;
                return true;
            }
            break;

        case KEY_9:
            if (shift)
            {
                // Shift-9 enters a matrix
                insert('[', MATRIX);
                last = 0;
                return true;
            }
            break;
        }
    }

    // Transient alpha editor keys (bring up the editor if needed)
    switch(key)
    {
    case KEY_F1:
        return handle_editing_command(EditMenu::ID_EditorSelect,
                                      EditMenu::ID_EditorFlip);
    case KEY_F2:
        return handle_editing_command(EditMenu::ID_EditorWordLeft,
                                      EditMenu::ID_EditorBegin);
    case KEY_F3:
        return handle_editing_command(EditMenu::ID_EditorWordRight,
                                      EditMenu::ID_EditorEnd);
    case KEY_F4:
        return handle_editing_command(EditMenu::ID_EditorSearch,
                                      EditMenu::ID_EditorReplace);
    case KEY_F5:
        return handle_editing_command(EditMenu::ID_EditorCut,
                                      EditMenu::ID_EditorCopy);
    case KEY_F6:
        return handle_editing_command(EditMenu::ID_EditorPaste,
                                      EditMenu::ID_EditorClear);
    }

    if (isEditing)
    {
        record(user_interface, "Editing key %d", key);
        switch (key)
        {
        case KEY_BSP:
            if (xshift)
                return false;
            repeat = true;
            if (~searching)
            {
                utf8 ed = rt.editor();
                if (cursor > select)
                    cursor = utf8_previous(ed, cursor);
                else
                    select = utf8_previous(ed, select);
                if (cursor == select)
                    cursor = select = searching;
                else
                    do_search(0, true);
            }
            else
            {
                utf8 ed = rt.editor();

                if (~select && select != cursor)
                {
                    editor_clear();
                }
                else if (shift && cursor < isEditing)
                {
                    // Shift + Backspace = Delete to right of cursor
                    uint after = utf8_next(ed, cursor, isEditing);
                    unicode cp = utf8_codepoint(ed + cursor);
                    if (cp == '\n')
                        edRows = 0;
                    else if (cp == Settings.BasedSeparator() ||
                             cp == Settings.NumberSeparator())
                        after = utf8_next(ed, after, isEditing);
                    remove(cursor, after - cursor);
                }
                else if (!shift && cursor > 0)
                {
                    // Backspace = Erase on left of cursor
                    utf8 ed      = rt.editor();
                    uint before  = cursor;
                    cursor       = utf8_previous(ed, cursor);
                    unicode cp = utf8_codepoint(ed + cursor);
                    if (cp == '\n')
                        edRows = 0;
                    else if (cp == Settings.BasedSeparator() ||
                             cp == Settings.NumberSeparator())
                        cursor = utf8_previous(ed, cursor);
                    remove(cursor, before - cursor);
                }
                else
                {
                    // Limits of line: beep
                    repeat = false;
                    beep(4400, 50);
                }

                dirtyEditor = true;
                adjustSeps = true;
                menu_refresh(object::ID_Catalog);
            }

            // Do not stop editing if we delete last character
            if (!rt.editing())
                insert(' ', DIRECT);
            last = 0;
            return true;
        case KEY_ENTER:
        {
            // Finish editing and parse the result
            if (!shift && !xshift)
            {
                if (~searching)
                {
                    searching = ~0U;
                    dirtyEditor = true;
                    edRows = 0;
                }
                else
                {
                    end_edit();
                }
                return true;
            }
            return false;
        }
        case KEY_EXIT:
            // Clear error if there is one, else clear editor
            if (shift || xshift)
                return false;

            if (rt.error())
            {
                rt.clear_error();
                dirtyEditor = true;
                dirtyStack = true;
            }
            else
            {
                editor_save(false);
                clear_editor();
                if (editing)
                {
                    if (!editingLevel)
                        rt.push(editing);
                    editing = nullptr;
                    dirtyEditor = true;
                    dirtyStack = true;
                }
            }
            return true;

        case KEY_UP:
            repeat = true;
            if (shift)
            {
                up = true;
                dirtyEditor = true;
            }
            else if (xshift)
            {
                // Command-line history
                editor_history();
                return true;
            }
            else if (cursor > 0)
            {
                font_p edFont = Settings.editor_font(edRows > 2);
                utf8 ed = rt.editor();
                uint pcursor  = utf8_previous(ed, cursor);
                unicode cp = utf8_codepoint(ed + pcursor);
                if (cp != '\n')
                {
                    draw_cursor(-1, pcursor);
                    cursor = pcursor;
                    cx -= edFont->width(cp);
                    edColumn = cx;
                    draw_cursor(1, pcursor);
                    if (cx < 0)
                        dirtyEditor = true;
                }
                else
                {
                    cursor = pcursor;
                    edRows = 0;
                    dirtyEditor = true;
                }
            }
            else
            {
                repeat = false;
                beep(4000, 50);
            }
            return true;
        case KEY_DOWN:
            repeat = true;
            if (shift)
            {
                down = true;
                dirtyEditor = true;
            }
            else if (xshift)
            {
                return false;
            }
            else if (cursor < isEditing)
            {
                font_p edFont = Settings.editor_font(edRows > 2);
                utf8 ed = rt.editor();
                unicode cp = utf8_codepoint(ed + cursor);
                uint ncursor = utf8_next(ed, cursor, isEditing);
                if (cp != '\n')
                {
                    draw_cursor(-1, ncursor);
                    cursor = ncursor;
                    cx += edFont->width(cp);
                    edColumn = cx;
                    draw_cursor(1, ncursor);
                    if (cx >= LCD_W - edFont->width('M'))
                        dirtyEditor = true;
                }
                else
                {
                    cursor = ncursor;
                    edRows = 0;
                    dirtyEditor = true;
                }
            }
            else
            {
                repeat = false;
                beep(4800, 50);
            }
            return true;
        case 0:
            return false;
        }
    }
    else
    {
        switch(key)
        {
        case KEY_ENTER:
            if (xshift)
            {
                // Insert quotes and begin editing
                insert('\"', TEXT);
                alpha = true;
                return true;
            }
            break;
        case KEY_EXIT:
            if (shift || xshift)
                return false;
            alpha = false;
            if (Settings.ExitClearsMenu())
                clear_menu();
            return true;
        case KEY_DOWN:
            // Key down to edit last object on stack
            if (!shift && !xshift && !alpha)
            {
                if (rt.depth())
                {
                    if (object_p obj = rt.pop())
                    {
                        editing = obj;
                        editingLevel = 0;
                        insert_object(obj);
                        cursor = 0;
                        dirtyEditor = true;
                        return true;
                    }
                }
            }
            break;
        case KEY_UP:
            if (xshift)
            {
                editor_history();
                return true;
            }
            else if (!shift)
            {
                if (rt.args(rt.depth()))
                {
                    if (++Stack.interactive > rt.depth())
                        Stack.interactive = rt.depth();
                    dirtyStack = true;
                    dirtyMenu = true;
                }
                return true;
            }
            break;

        }
    }

    return consumed;
}


bool user_interface::handle_editing_command(object::id lo, object::id hi)
// ----------------------------------------------------------------------------
//   Handle F1-F6 in transient alpha mode
// ----------------------------------------------------------------------------
{
    if (!transalpha)
        return false;

    // All these commands are editing commands, edit if needed
    if (!rt.editing())
    {
        if (!rt.depth())
            return false;

        if (object_p obj = rt.pop())
        {
            editing = obj;
            editingLevel = 0;
            insert_object(obj);
            dirtyEditor = true;
        }
    }

    object_p cmd = command::static_object(lowercase ? lo : hi);
    cmd->evaluate();
    return true;
}


bool user_interface::handle_alpha(int key)
// ----------------------------------------------------------------------------
//    Handle alphabetic user_interface
// ----------------------------------------------------------------------------
{
    // Things that we never handle in alpha mode
    if (!key || (key >= KEY_F1 && key <= KEY_F6) || key == KEY_EXIT)
        return false;

    // Allow "alpha" mode for keys A-F in based number mode
    // xshift-ENTER inserts quotes, xshift-BSP inserts \n
    bool editing = rt.editing();
    bool hex = editing && !alpha && mode == BASED && key >= KB_A && key <= KB_F;
    bool special = xshift && (key == KEY_ENTER || (key == KEY_BSP && editing));
    if (!alpha && !hex && !special)
        return false;

    static const char upper[] =
        "ABCDEF"
        "GHIJKL"
        "_MNO_"
        "_PQRS"
        "_TUVW"
        "_XYZ_"
        "_:, ;";
    static const char lower[] =
        "abcdef"
        "ghijkl"
        "_mno_"
        "_pqrs"
        "_tuvw"
        "_xyz_"
        "_:, ;";

    static const unicode shifted[] =
    {
        L'Σ', '^', L'√', L'∂', L'ρ', '(',
        L'▶', '%', L'π', '<', '=', '>',
        '_', L'⇄', L'±', L'∡', '_',
        '_', '7', '8', '9', L'÷',
        '_', '4', '5', '6', L'×',
        '_', '1', '2', '3', '-',
        '_', '0', '.',  L'«', '+'
    };

    static const  unicode xshifted[] =
    {
        L'∏', L'∆', L'↑', L'μ', L'θ', '\'',
        L'→', L'←', L'↓', L'≤', L'≠', L'≥',
        '"',  '~', L'°', L'ε', '\n',
        '_',  '?', L'∫',   '[',  '/',
        '_',  '#',  L'∞', '|' , '*',
        '_',  '&',   '@', '$',  L'…',
        '_',  ';',  L'·', '{',  '!'
    };

    // Special case: + in alpha mode shows the catalog
    if (key == KEY_ADD && !shift && !xshift)
    {
        object_p cat = command::static_object(menu::ID_Catalog);
        cat->evaluate();
        return true;
    }

    key--;
    unicode c =
        hex       ? upper[key]    :
        xshift    ? xshifted[key] :
        shift     ? shifted[key]  :
        lowercase ? lower[key]    :
        upper[key];
    if (~searching)
    {
        if (!do_search(c))
            beep(2400, 100);
    }
    else
    {
        if (menu_p m = menu())
        {
            menu::id mid = m->type();
            if (mid >= menu::ID_CharactersMenu00 &&
                mid <= menu::ID_CharactersMenu99)
            {
                character_menu_p cm = character_menu_p(m);
                if (cm->transliterate(c))
                {
                    size_t edlen = rt.editing();
                    utf8   ed    = rt.editor();
                    if (ed && edlen)
                    {
                        uint ppos = utf8_previous(ed, cursor);
                        if (ppos != cursor)
                            remove(ppos, cursor - ppos);
                    }
                }
            }
        }
        insert(c, DIRECT);
        if (c == '"')
            alpha = true;
        repeat = true;
    }
    menu_refresh(object::ID_Catalog);
    return true;
}


bool user_interface::handle_digits(int key)
// ----------------------------------------------------------------------------
//    Handle alphabetic user_interface
// ----------------------------------------------------------------------------
{
    if (alpha || shift || xshift || !key)
        return false;

    static const char numbers[] =
        "______"
        "______"
        "__-__"
        "_789_"
        "_456_"
        "_123_"
        "_0.__";

    if (rt.editing())
    {
        if (key == KEY_CHS)
        {
            // Special case for change of sign
            byte   *ed          = rt.editor();
            byte   *p           = ed + cursor;
            utf8    found       = nullptr;
            unicode c           = utf8_codepoint(p);
            unicode dm          = Settings.DecimalSeparator();
            unicode ns          = Settings.NumberSeparator();
            unicode hs          = Settings.BasedSeparator();
            bool    had_complex = false;
            while (p > ed && !found)
            {
                p = (byte *) utf8_previous(p);
                c = utf8_codepoint(p);
                if (c == complex::I_MARK || c == complex::ANGLE_MARK)
                {
                    had_complex = true;
                    if (c == complex::ANGLE_MARK)
                    {
                        found = utf8_next(p);
                    }
                    else
                    {
                        found = p;
                        p = (byte *) utf8_previous(p);
                        c = utf8_codepoint(p);
                    }
                }
                else if ((c < '0' || c > '9') && c != dm && c != ns && c != hs)
                {
                    found = utf8_next(p);
                }
            }

            if (!found)
                found = ed;
            if (c == 'e' || c == 'E' || c == Settings.ExponentSeparator())
                c  = utf8_codepoint(p);

            if (had_complex)
            {
                if (c == '+' || c == '-')
                    *p = '+' + '-' - c;
                else
                    insert(found - ed, '-');
            }
            else if (c == '-')
            {
                remove(p - ed, 1);
            }
            else
            {
                insert(found - ed, '-');
            }
            last = 0;
            dirtyEditor = true;
            return true;
        }
        else if (key == KEY_E && !~searching)
        {
            if (mode == UNIT)
            {
                utf8 start = nullptr;
                size_t len = 0;
                if (current_word(start, len))
                {
                    byte *ed = rt.editor();
                    byte *st = (byte *) start;
                    unicode prefix = 0;
                    bool    ins    = false;
                    bool    del    = false;

                    switch(*start)
                    {
                    case 'k':   prefix = 'M';           break;
                    case 'M':   prefix = 'G';           break;
                    case 'G':   prefix = 'T';           break;
                    case 'T':   prefix = 'm';           break;
                    case 'm':   prefix = 'c';           break;
                    case 'c':                           del = true; break;
                    default:    prefix = 'k';           ins = true; break;
                    }
                    if (del && size_t(st - ed + 1) < rt.editing() &&
                        isalnum(start[1]))
                    {
                        remove(st - ed, 1);
                    }
                    else
                    {
                        if (!ins && (size_t(st - ed + 1) >= rt.editing() ||
                                     !isalnum(start[1])))
                        {
                            ins = true;
                            prefix = 'k';
                        }
                        if (ins)
                            insert(st - ed, prefix);
                        else
                            *st = prefix;
                    }
                }
            }
            else
            {
                byte   buf[4];
                size_t sz = utf8_encode(Settings.ExponentSeparator(), buf);
                insert(cursor, buf, sz);
            }
            last = 0;
            dirtyEditor = true;
            return true;

        }
    }
    if (key > KEY_CHS && key < KEY_F1)
    {
        unicode c = numbers[key - 1];
        if (~searching)
        {
            bool found = false;
            switch (key)
            {
            case KEY_ADD:       found = do_search('+'); break;
            case KEY_SUB:       found = do_search('-'); break;
            case KEY_MUL:       found = do_search('*')||do_search(L'×')
                                                      ||do_search(L'·'); break;
            case KEY_DIV:       found = do_search('/')||do_search(L'÷'); break;
            case KEY_DOT:       found = do_search('.')||do_search(L','); break;
            case KEY_E:         found = do_search('E')||do_search(L'⁳'); break;
            default:
                if (c == '_')
                    return false;
                found = do_search(c);
                break;
            }
            if (!found)
                beep(2400, 100);
            return true;
        }
        if (c == '_')
            return false;
        if (c == '.' && mode != TEXT)
        {
            // Check if we enter a DMS value
            byte   *ed    = rt.editor();
            byte   *p     = ed + cursor;
            utf8    found = nullptr;
            unicode dm    = Settings.DecimalSeparator();
            unicode ns    = Settings.NumberSeparator();
            unicode hs    = Settings.BasedSeparator();

            c = char(dm);
            while (p > ed && !found)
            {
                p = (byte *) utf8_previous(p);
                unicode cp = utf8_codepoint(p);
                if (cp == L'″')
                {
                    found = p;
                    c = '/';
                }
                else if (cp == L'′')
                {
                    found = p;
                    c = L'″';
                }
                else if (cp == L'°')
                {
                    found = p;
                    if (uint(found - ed) == cursor - utf8_size(cp))
                    {
                        remove(found - ed, utf8_size(cp));
                        c = dm;

                        size_t edlen = rt.editing();
                        ed = rt.editor();
                        if (cursor + 4 <= edlen &&
                            memcmp(ed + cursor, "_dms", 4) == 0)
                            remove(cursor, 4);
                    }
                    else
                    {
                        c = L'′';
                    }
                }
                else if (cp == dm)
                {
                    found = p;
                    remove (found - ed, utf8_size(cp));
                    if (uint(found - ed - 1) == cursor - utf8_size(cp))
                    {
                        c = L'°';
                    }
                    else
                    {
                        insert(found - ed, unicode(L'°'));
                        c = L'′';
                    }
                    size_t edlen = rt.editing();
                    ed = rt.editor();
                    if (cursor + 4 > edlen ||
                        memcmp(ed + cursor, "_dms", 4) != 0)
                    {
                        size_t add = insert(cursor, utf8("_dms"), 4);
                        cursor -= add;
                    }
                }
                else if ((cp < '0' || cp > '9') && cp != ns && cp != hs)
                {
                    break;
                }
            }
            if (found)
            {
                bool haddigit = p > ed;
                if (haddigit)
                {
                    utf8 pp = utf8_previous(p);
                    unicode cpp = utf8_codepoint(pp);
                    haddigit = cpp >= '0' && cpp <= '9';
                }
                if (!haddigit)
                    insert(found - ed, unicode('0'));
            }
        }
        insert(c, DIRECT);
        repeat = true;
        return true;
    }
    return false;
}



// ============================================================================
//
//   Tables with the default assignments
//
// ============================================================================

static const byte defaultUnshiftedCommand[2*user_interface::NUM_KEYS] =
// ----------------------------------------------------------------------------
//   RPL code for the commands assigned by default to each key
// ----------------------------------------------------------------------------
//   All the default-assigned commands fit in one or two bytes
{
#define OP2BYTES(key, id)                                              \
    [2*(key) - 2] = (id) < 0x80 ? (id) & 0x7F : ((id) & 0x7F) | 0x80,  \
    [2*(key) - 1] = (id) < 0x80 ?           0 : ((id) >> 7)

    OP2BYTES(KEY_SIGMA, menu::ID_ToolsMenu),
    OP2BYTES(KEY_INV,   function::ID_inv),
    OP2BYTES(KEY_SQRT,  function::ID_sqrt),
    OP2BYTES(KEY_LOG,   function::ID_exp),
    OP2BYTES(KEY_LN,    function::ID_log),
    OP2BYTES(KEY_XEQ,   0),
    OP2BYTES(KEY_STO,   command::ID_Sto),
    OP2BYTES(KEY_RCL,   command::ID_ToggleCustomMenu),
    OP2BYTES(KEY_RDN,   menu::ID_StackMenu),
    OP2BYTES(KEY_SIN,   function::ID_sin),
    OP2BYTES(KEY_COS,   function::ID_cos),
    OP2BYTES(KEY_TAN,   function::ID_tan),
    OP2BYTES(KEY_ENTER, function::ID_Dup),
    OP2BYTES(KEY_SWAP,  function::ID_Swap),
    OP2BYTES(KEY_CHS,   function::ID_neg),
    OP2BYTES(KEY_E,     function::ID_Cycle),
    OP2BYTES(KEY_BSP,   command::ID_Drop),
    OP2BYTES(KEY_UP,    0),
    OP2BYTES(KEY_7,     0),
    OP2BYTES(KEY_8,     0),
    OP2BYTES(KEY_9,     0),
    OP2BYTES(KEY_DIV,   arithmetic::ID_div),
    OP2BYTES(KEY_DOWN,  0),
    OP2BYTES(KEY_4,     0),
    OP2BYTES(KEY_5,     0),
    OP2BYTES(KEY_6,     0),
    OP2BYTES(KEY_MUL,   arithmetic::ID_mul),
    OP2BYTES(KEY_SHIFT, 0),
    OP2BYTES(KEY_1,     0),
    OP2BYTES(KEY_2,     0),
    OP2BYTES(KEY_3,     0),
    OP2BYTES(KEY_SUB,   command::ID_sub),
    OP2BYTES(KEY_EXIT,  0),
    OP2BYTES(KEY_0,     0),
    OP2BYTES(KEY_DOT,   0),
    OP2BYTES(KEY_RUN,   command::ID_Eval),
    OP2BYTES(KEY_ADD,   command::ID_add),

    OP2BYTES(KEY_F1,    0),
    OP2BYTES(KEY_F2,    0),
    OP2BYTES(KEY_F3,    0),
    OP2BYTES(KEY_F4,    0),
    OP2BYTES(KEY_F5,    0),
    OP2BYTES(KEY_F6,    0),

    OP2BYTES(KEY_SCREENSHOT, command::ID_ScreenCapture),
    OP2BYTES(KEY_SH_UP,  0),
    OP2BYTES(KEY_SH_DOWN, 0),
};


static const byte defaultShiftedCommand[2*user_interface::NUM_KEYS] =
// ----------------------------------------------------------------------------
//   RPL code for the commands assigned by default to shifted keys
// ----------------------------------------------------------------------------
//   All the default assigned commands fit in one or two bytes
{
    OP2BYTES(KEY_SIGMA, menu::ID_LastMenu),
    OP2BYTES(KEY_INV,   arithmetic::ID_pow),
    OP2BYTES(KEY_SQRT,  arithmetic::ID_sq),
    OP2BYTES(KEY_LOG,   function::ID_exp10),
    OP2BYTES(KEY_LN,    function::ID_log10),
    OP2BYTES(KEY_XEQ,   menu::ID_LoopsMenu),
    OP2BYTES(KEY_STO,   menu::ID_ComplexMenu),
    OP2BYTES(KEY_RCL,   menu::ID_FractionsMenu),
    OP2BYTES(KEY_RDN,   menu::ID_ConstantsMenu),
    OP2BYTES(KEY_SIN,   function::ID_asin),
    OP2BYTES(KEY_COS,   function::ID_acos),
    OP2BYTES(KEY_TAN,   function::ID_atan),
    OP2BYTES(KEY_ENTER, 0),     // Alpha
    OP2BYTES(KEY_SWAP,  menu::ID_LastArg),
    OP2BYTES(KEY_CHS,   menu::ID_ModesMenu),
    OP2BYTES(KEY_E,     menu::ID_DisplayModesMenu),
    OP2BYTES(KEY_BSP,   menu::ID_ClearThingsMenu),
    OP2BYTES(KEY_UP,    0),
    OP2BYTES(KEY_7,     menu::ID_SolverMenu),
    OP2BYTES(KEY_8,     menu::ID_IntegrationMenu),
    OP2BYTES(KEY_9,     0),     // Insert []
    OP2BYTES(KEY_DIV,   menu::ID_StatisticsMenu),
    OP2BYTES(KEY_DOWN,  0),
    OP2BYTES(KEY_4,     menu::ID_BasesMenu),
    OP2BYTES(KEY_5,     menu::ID_UnitsMenu),
    OP2BYTES(KEY_6,     menu::ID_FlagsMenu),
    OP2BYTES(KEY_MUL,   menu::ID_ProbabilitiesMenu),
    OP2BYTES(KEY_SHIFT, 0),
    OP2BYTES(KEY_1,     function::ID_ToDecimal),
    OP2BYTES(KEY_2,     0),
    OP2BYTES(KEY_3,     menu::ID_ProgramMenu),
    OP2BYTES(KEY_SUB,   menu::ID_ListMenu),
    OP2BYTES(KEY_EXIT,  command::ID_Off),
    OP2BYTES(KEY_0,     command::ID_SystemSetup),
    OP2BYTES(KEY_DOT,   command::ID_Show),
    OP2BYTES(KEY_RUN,   0),
    OP2BYTES(KEY_ADD,   menu::ID_Catalog),

    OP2BYTES(KEY_F1,    0),
    OP2BYTES(KEY_F2,    0),
    OP2BYTES(KEY_F3,    0),
    OP2BYTES(KEY_F4,    0),
    OP2BYTES(KEY_F5,    0),
    OP2BYTES(KEY_F6,    0),

    OP2BYTES(KEY_SCREENSHOT, command::ID_ScreenCapture),
    OP2BYTES(KEY_SH_UP, 0),
    OP2BYTES(KEY_SH_DOWN, 0),
};


static const byte defaultSecondShiftedCommand[2*user_interface::NUM_KEYS] =
// ----------------------------------------------------------------------------
//   RPL code for the commands assigned by default to long-shifted keys
// ----------------------------------------------------------------------------
//   All the default assigned commands fit in one or two bytes
{
    OP2BYTES(KEY_SIGMA, menu::ID_MainMenu),
    OP2BYTES(KEY_INV,   command::ID_xroot),
    OP2BYTES(KEY_SQRT,  menu::ID_PolynomialsMenu),
    OP2BYTES(KEY_LOG,   menu::ID_ExpLogMenu),
    OP2BYTES(KEY_LN,    menu::ID_PartsMenu),
    OP2BYTES(KEY_XEQ,   menu::ID_EquationsMenu),
    OP2BYTES(KEY_STO,   menu::ID_MemoryMenu),
    OP2BYTES(KEY_RCL,   menu::ID_Library),
    OP2BYTES(KEY_RDN,   menu::ID_MathMenu),
    OP2BYTES(KEY_SIN,   menu::ID_HyperbolicMenu),
    OP2BYTES(KEY_COS,   menu::ID_CircularMenu),
    OP2BYTES(KEY_TAN,   menu::ID_RealMenu),
    OP2BYTES(KEY_ENTER, 0),     // Text
    OP2BYTES(KEY_SWAP,  function::ID_Undo),
    OP2BYTES(KEY_CHS,   menu::ID_ObjectMenu),
    OP2BYTES(KEY_E,     menu::ID_PlotMenu),
    OP2BYTES(KEY_BSP,   function::ID_updir),
    OP2BYTES(KEY_UP,    0),
    OP2BYTES(KEY_7,     menu::ID_SymbolicMenu),
    OP2BYTES(KEY_8,     menu::ID_DifferentiationMenu),
    OP2BYTES(KEY_9,     menu::ID_MatrixMenu),
    OP2BYTES(KEY_DIV,   menu::ID_FinanceSolverMenu),
    OP2BYTES(KEY_DOWN,  menu::ID_EditMenu),
    OP2BYTES(KEY_4,     menu::ID_TextMenu),
    OP2BYTES(KEY_5,     menu::ID_UnitsConversionsMenu),
    OP2BYTES(KEY_6,     menu::ID_TimeMenu),
    OP2BYTES(KEY_MUL,   menu::ID_NumbersMenu),
    OP2BYTES(KEY_SHIFT, 0),
    OP2BYTES(KEY_1,     menu::ID_DebugMenu),
    OP2BYTES(KEY_2,     menu::ID_CharactersMenu),
    OP2BYTES(KEY_3,     menu::ID_TestsMenu),
    OP2BYTES(KEY_SUB,   menu::ID_IOMenu),
    OP2BYTES(KEY_EXIT,  command::ID_SaveState),
    OP2BYTES(KEY_0,     menu::ID_FilesMenu),
    OP2BYTES(KEY_DOT,   menu::ID_GraphicsMenu),
    OP2BYTES(KEY_RUN,   0),
    OP2BYTES(KEY_ADD,   command::ID_Help),

    OP2BYTES(KEY_F1,    0),
    OP2BYTES(KEY_F2,    0),
    OP2BYTES(KEY_F3,    0),
    OP2BYTES(KEY_F4,    0),
    OP2BYTES(KEY_F5,    0),
    OP2BYTES(KEY_F6,    0),

    OP2BYTES(KEY_SCREENSHOT, command::ID_ScreenCapture),
    OP2BYTES(KEY_SH_UP, 0),
    OP2BYTES(KEY_SH_DOWN, 0),
};


static const byte *const defaultCommand[user_interface::NUM_PLANES] =
// ----------------------------------------------------------------------------
//   Pointers to the default commands
// ----------------------------------------------------------------------------
{
    defaultUnshiftedCommand,
    defaultShiftedCommand,
    defaultSecondShiftedCommand,
};


object_p user_interface::object_for_key(int key)
// ----------------------------------------------------------------------------
//    Return the object for a given key
// ----------------------------------------------------------------------------
{
    uint plane = shift_plane();
    if (key >= KEY_F1 && key <= KEY_F6 && plane >= menu_planes())
        plane = 0;

    object_p obj = function[plane][key - 1];
    if (!obj)
    {
        const byte *ptr = defaultCommand[plane] + 2 * (key - 1);
        if (*ptr)
            obj = (object_p) ptr;
    }
    return obj;
}


bool user_interface::handle_functions(int key)
// ----------------------------------------------------------------------------
//   Check if we have one of the soft menu functions
// ----------------------------------------------------------------------------
{
    if (!key)
        return false;

    record(user_interface,
           "Handle function for key %d (plane %d) ", key, shift_plane());
    if (object_p obj = object_for_key(key))
    {
        save<int>  saveEvaluating(evaluating, key);
        object::id ty = obj->type();
        if (object::is_forced_entry(ty))
        {
            dirtyEditor = true;
            edRows = 0;
            return obj->insert() != object::ERROR;
        }

        bool imm     = object::is_immediate(ty);
        bool editing = rt.editing();
        if (editing && !imm)
        {
            if (key == KEY_ENTER || key == KEY_BSP)
                return false;

            if (autoComplete && key >= KEY_F1 && key <= KEY_F6)
            {
                size_t start = 0;
                size_t size  = 0;
                if (current_word(start, size))
                    remove(start, size);
            }

            if ((ty >= object::ID_Deg && ty <= object::ID_PiRadians) &&
                at_end_of_number(true))
            {
                unicode cp;
                switch(ty)
                {
                case object::ID_Deg:        cp = Settings.DEGREES_SYMBOL; break;
                case object::ID_Grad:       cp = Settings.GRAD_SYMBOL; break;
                case object::ID_PiRadians:  cp = Settings.PI_RADIANS_SYMBOL; break;
                default:
                case object::ID_Rad:        cp = Settings.RADIANS_SYMBOL; break;
                }
                insert(cp, TEXT, false);
                return true;
            }

            switch (mode)
            {
            case ALGEBRAIC:
            case PARENTHESES:
                if (ty == object::ID_Sto)
                {
                    if (!end_edit())
                        return false;
                    break;
                }
                [[fallthrough]];

            case PROGRAM:
            case MATRIX:
            unit_application:
                if (object::is_program_cmd(ty) || object::is_algebraic(ty))
                {
                    dirtyEditor = true;
                    edRows = 0;
                    return obj->insert() != object::ERROR;
                }
                break;

            case UNIT:
                if (ty == object::ID_mul ||
                    ty == object::ID_div ||
                    ty == object::ID_pow)
                    goto unit_application;
                [[fallthrough]];

            case DIRECT:
                if (ty == object::ID_ApplyUnit ||
                    ty == object::ID_ApplyInverseUnit)
                    goto unit_application;
                [[fallthrough]];

            default:
                // If we have the editor open, need to close it
                if (ty != object::ID_SelfInsert)
                {
                    if (!end_edit())
                        return false;
                    editing = false;
                }
                break;
            }

        }
        draw_busy();
        if (!imm && !editing)
        {
            if (Settings.SaveStack())
                rt.save();
            if (Settings.SaveLastArguments())
                rt.need_save();
        }
        save<bool> no_halt(program::halted, false);
        obj->evaluate();
        draw_idle();
        dirtyStack = true;
        if (!imm)
            alpha = false;
        xshift = false;
        shift = false;
        return true;
    }

    return false;
}


bool user_interface::current_word(size_t &start, size_t &size)
// ----------------------------------------------------------------------------
//   REturn position of word under the cursor if there is one
// ----------------------------------------------------------------------------
{
    utf8 sed = nullptr;
    bool result = current_word(sed, size);
    if (result)
        start = sed - rt.editor();
    return result;
}


bool user_interface::current_word(utf8 &start, size_t &size)
// ----------------------------------------------------------------------------
//   Find the word under the cursor in the editor, if there is one
// ----------------------------------------------------------------------------
{
    if (size_t sz = rt.editing())
    {
        byte *ed = rt.editor();
        uint  c  = cursor;
        c = utf8_previous(ed, c);
        while (c > 0 && !is_separator_or_digit(ed + c))
            c = utf8_previous(ed, c);
        if (is_separator_or_digit(ed + c))
            c = utf8_next(ed, c, sz);
        uint spos = c;
        while (c < sz && !is_separator(ed + c))
            c = utf8_next(ed, c, sz);
        uint end = c;
        if (end > spos)
        {
            start = ed + spos;
            size = end - spos;
            return true;
        }
    }
    return false;
}



// ============================================================================
//
//   Editor menu commands
//
// ============================================================================

bool user_interface::editor_select()
// ----------------------------------------------------------------------------
//   Set selection to current cursor position
// ----------------------------------------------------------------------------
{
    if (select == cursor)
        select = ~0U;
    else
        select = cursor;
    dirtyEditor = true;
    return true;
}


bool user_interface::editor_word_left()
// ----------------------------------------------------------------------------
//   Move cursor one word to the left
// ----------------------------------------------------------------------------
{
    if (rt.editing())
    {
        if (cursor != 0)
        {
            utf8 ed = rt.editor();

            // Skip whitespace
            while (cursor > 0)
            {
                unicode code = utf8_codepoint(ed + cursor);
                if (!isspace(code))
                    break;
                cursor = utf8_previous(ed, cursor);
            }

            // Skip word
            while (cursor > 0)
            {
                unicode code = utf8_codepoint(ed + cursor);
                if (isspace(code))
                    break;
                cursor = utf8_previous(ed, cursor);
            }

            edRows = 0;
            dirtyEditor = true;
        }
        else
        {
            editor_history();
        }
    }
    return true;
}


bool user_interface::editor_word_right()
// ----------------------------------------------------------------------------
//   Move cursor one word to the right
// ----------------------------------------------------------------------------
{
    if (size_t editing = rt.editing())
    {
        if (cursor < editing)
        {
            utf8 ed = rt.editor();

            // Skip whitespace
            while (cursor < editing)
            {
                unicode code = utf8_codepoint(ed + cursor);
                if (!isspace(code))
                    break;
                cursor = utf8_next(ed, cursor, editing);
            }

            // Skip word
            while (cursor < editing)
            {
                unicode code = utf8_codepoint(ed + cursor);
                if (isspace(code))
                    break;
                cursor = utf8_next(ed, cursor, editing);
            }

            edRows = 0;
            dirtyEditor = true;
        }
        else
        {
            editor_history(true);
            cursor = rt.editing();
        }
    }
    return true;
}

bool user_interface::editor_begin()
// ----------------------------------------------------------------------------
//   Move cursor to beginning of buffer
// ----------------------------------------------------------------------------
{
    cursor = 0;
    edRows = 0;
    dirtyEditor = true;
    return true;
}

bool user_interface::editor_end()
// ----------------------------------------------------------------------------
//   Move cursor one word to the right
// ----------------------------------------------------------------------------
{
    cursor = rt.editing();
    edRows = 0;
    dirtyEditor = true;
    return true;
}


bool user_interface::editor_cut()
// ----------------------------------------------------------------------------
//   Cut to clipboard (most recent history)
// ----------------------------------------------------------------------------
{
    editor_copy();
    editor_clear();
    return true;
}


bool user_interface::editor_copy()
// ----------------------------------------------------------------------------
//   Copy to clipboard
// ----------------------------------------------------------------------------
{
    if (~select && select != cursor)
    {
        uint start = cursor;
        uint end = select;
        if (start > end)
            std::swap(start, end);
        utf8 ed = rt.editor();
        clipboard = text::make(ed + start, end - start);
    }
    return true;
}


bool user_interface::editor_paste()
// ----------------------------------------------------------------------------
//   Paste from clipboard
// ----------------------------------------------------------------------------
{
    if (clipboard)
    {
        size_t len = 0;
        utf8 ed = clipboard->value(&len);
        insert(cursor, ed, len);
        edRows = 0;
        dirtyEditor = true;
    }
    return true;
}


bool user_interface::do_search(unicode with, bool restart)
// ----------------------------------------------------------------------------
//   Perform the actual search
// ----------------------------------------------------------------------------
{
    // Already search, find next position
    size_t max = rt.editing();
    utf8   ed  = rt.editor();
    if (!max || !ed)
        return false;
    if (!~select)
        select = cursor;

    bool   forward  = cursor >= select;
    size_t selected = forward ? cursor - select : select - cursor;
    if (selected > max)
    {
        selected = 0;
        select = cursor;
    }
    size_t found  = ~0;
    uint   ref    = forward ? select : cursor;
    uint   start  = restart ? searching : ref;
    uint   search = start;

    // Skip current location (search next) or not (incremental search)
    bool   skip   = with == 0;

    // Loop until we either find a new spot or we wrap around
    for (uint count = 0; !~found && count < max; count++)
    {
        if (skip)
        {
            // Move search forward or backward respecting unicode boundaries
            if (forward)
            {
                search = utf8_next(ed, search, max);
                if (search == max)
                    search = 0;
            }
            else
            {
                search = utf8_previous(ed, search);
                if (search == 0)
                    search = utf8_previous(ed, max);
            }
        }
        else
        {
            skip = true;
        }

        // Check if there is a match at the current location
        bool check  = true;
        uint last = search + selected;

        // Never match if past end of buffer
        if (last + (with != 0) > max)
            continue;

        // Otherwise, loop inside buffer
        for (uint s = search; check && s < last; s = utf8_next(ed, s, max))
        {
            unicode sc = utf8_codepoint(ed + s);
            unicode rc = utf8_codepoint(ed + ref + s - search);
            check = towlower(sc) == towlower(rc);
        }

        if (check && with)
        {
            unicode sc = utf8_codepoint(ed + last);
            check = towlower(sc) == towlower(with);
        }
        if (check)
        {
            found = search;
            break;
        }
    }

    if (~found)
    {
        if (with)
            selected += utf8_size(with);

        if (forward)
        {
            select = found;
            cursor = select + selected;
        }
        else
        {
            cursor = found;
            select = cursor + selected;
        }
        edRows = 0;
        dirtyEditor = true;
        return true;
    }
    return false;
}


bool user_interface::editor_search()
// ----------------------------------------------------------------------------
//   Start or repeat a search a search
// ----------------------------------------------------------------------------
{
    if (~select && cursor != select)
    {
        // Keep searching
        if (~searching == 0)
            searching = cursor > select ? select : cursor;
        if (!do_search())
            beep(2500, 100);
        edRows = 0;
        dirtyEditor = true;
    }
    else
    {
        // Start search
        searching = select = cursor;
        alpha = true;
        lowercase = false;
        shift = xshift = false;
    }
    return true;
}


bool user_interface::editor_replace()
// ----------------------------------------------------------------------------
//   Perform a search replacement
// ----------------------------------------------------------------------------
{
    bool result = true;
    if (~searching && ~select && cursor != select && clipboard)
    {
        uint start = cursor;
        uint end = select;
        if (start > end)
            std::swap(start, end);
        result = do_search();
        remove(start, end - start);

        size_t len = 0;
        utf8 ed = clipboard->value(&len);
        insert(start, ed, len);

        if (!result)
            select = ~0U;

        edRows = 0;
        dirtyEditor = true;
    }
    return result;
}


bool user_interface::editor_clear()
// ----------------------------------------------------------------------------
//   Paste from clipboard
// ----------------------------------------------------------------------------
{
    if (~select && select != cursor)
    {
        uint start = cursor;
        uint end = select;
        if (start > end)
            std::swap(start, end);
        remove(start, end - start);
        select = ~0U;
        edRows = 0;
        dirtyEditor = true;
    }
    return true;
}


bool user_interface::editor_selection_flip()
// ----------------------------------------------------------------------------
//   Flip cursor and selection point
// ----------------------------------------------------------------------------
{
    if (~select)
        std::swap(select, cursor);
    edRows = 0;
    dirtyEditor = true;
    return true;
}


size_t user_interface::adjust_cursor(size_t offset, size_t len)
// ----------------------------------------------------------------------------
//   Adjust cursor after inserting `len` characters
// ----------------------------------------------------------------------------
{
    if (~select && select >= offset)
        select += len;
    if (cursor >= offset)
        cursor += len;
    if (cursor > rt.editing())
        record(runtime_error, "Cursor %u > %u", cursor, rt.editing());
    return len;
}

size_t user_interface::insert(size_t offset, utf8 data, size_t len)
// ----------------------------------------------------------------------------
//   Insert data in the editor
// ----------------------------------------------------------------------------
{
    len = rt.insert(offset, data, len);
    return adjust_cursor(offset, len);
}


size_t user_interface::insert(size_t offset, unicode c)
// ----------------------------------------------------------------------------
//   Insert a Unicode glyph in the editor
// ----------------------------------------------------------------------------
{
    byte buffer[4];
    size_t sz = utf8_encode(c, buffer);
    return insert(offset, buffer, sz);
}


object::result user_interface::insert_softkey(int     key,
                                              cstring before,
                                              cstring after,
                                              bool    midcursor)
// ----------------------------------------------------------------------------
//   Insert the name associated with the key if editing
// ----------------------------------------------------------------------------
{
    if (cstring text = label_text(key - KEY_F1))
    {
        if (*text)
        {
            size_t length = 0;
            if (symbol_p name = label(key - KEY_F1))
            {
                text = (cstring) name->value(&length);
            }
            else
            {
                length = strlen(text);
            }

            insert(cursor, utf8(before), strlen(before));
            insert(cursor, utf8(text), length);
            uint mid = cursor_position();
            insert(cursor, utf8(after), strlen(after));

            if (midcursor)
                cursor_position(mid);

            return object::OK;
        }
    }

    return object::ERROR;
}


object::result user_interface::insert_object(object_p o,
                                             cstring before, cstring after,
                                             bool midcursor)
// ----------------------------------------------------------------------------
//   Insert the object in the editor
// ----------------------------------------------------------------------------
{
    object_g obj = o;

    size_t len = strlen(before);
    if (len && len != insert(cursor, utf8(before), len))
        return object::ERROR;

    renderer r;
    len = obj->render(r);
    if (len != rt.edit(cursor, len))
        return object::ERROR;

    r.clear();
    adjust_cursor(cursor, len);
    uint mid = cursor_position();

    len = strlen(after);
    if (len && len != insert(cursor, utf8(after), len))
        return object::ERROR;

    if (midcursor)
        cursor_position(mid);
    return object::OK;
}


object::result user_interface::insert_object(object_p obj, modes m)
// ----------------------------------------------------------------------------
//   Enter the given text on the command line
// ----------------------------------------------------------------------------
{
    bool    editing = rt.editing();
    utf8    ed      = rt.editor();
    bool    hadsp   = !cursor || ed[cursor - 1] == ' ';
    bool    algb    = is_algebraic(mode);
    bool    algi    = is_algebraic(m);
    cstring start   = algb ? "" : (editing && !hadsp) ? " " : "";
    cstring end     = algb ? (algi ? "()" : "") : editing ? " " : "";
    result  r       = insert_object(obj, start, end);
    if (algb && r == object::OK && cursor > 0)
        cursor--;
    dirtyEditor = true;
    adjustSeps  = true;
    update_mode();
    return r;
}


size_t user_interface::remove(size_t offset, size_t len)
// ----------------------------------------------------------------------------
//   Remove data from the editor
// ----------------------------------------------------------------------------
{
    len = rt.remove(offset, len);
    if (~select && select >= offset)
    {
        if (select >= offset + len)
            select -= len;
        else
            select = offset;
    }
    if (cursor >= offset)
    {
        if (cursor >= offset + len)
            cursor -= len;
        else
            cursor = offset;
    }
    return len;
}



// ============================================================================
//
//    Interface with DMCP
//
// ============================================================================

void ui_draw_message(const char *hdr)
// ----------------------------------------------------------------------------
//   Draw a message, e.g. file error
// ----------------------------------------------------------------------------
{
    ui.draw_message(hdr, (cstring) rt.error());
}



// ============================================================================
//
//   Debugging tool (printing on screen)
//
// ============================================================================

#include "font.h"
#include <cstdarg>

void debug_printf(int row, cstring format, ...)
// ----------------------------------------------------------------------------
//   Debug printf on the given row
// ----------------------------------------------------------------------------
{
    if (HelpFont)
    {
        char buffer[256];
        va_list va;
        va_start(va, format);
        vsnprintf(buffer, sizeof(buffer), format, va);
        va_end(va);
        size  h = HelpFont->height();
        coord y = row * h;
        Screen.text(0, y, utf8(buffer), HelpFont, pattern::white, pattern::black);
        ui.draw_dirty(0, y, LCD_W, y + h - 1);
        refresh_dirty();
    }
}


void debug_wait(int delay)
// ----------------------------------------------------------------------------
//   Wait for the given delay, or until key is pressed
// ----------------------------------------------------------------------------
{
    refresh_dirty();
    if (delay > 0)
        sys_delay(delay);
    else if (delay < 0)
        wait_for_key_press();
}
