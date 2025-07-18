// ****************************************************************************
//  command.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Shared code for all RPL commands
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

#include "arithmetic.h"
#include "bignum.h"
#include "custom.h"
#include "decimal.h"
#include "dmcp.h"
#include "equations.h"
#include "fraction.h"
#include "integer.h"
#include "parser.h"
#include "polynomial.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "stack.h"
#include "symbol.h"
#include "sysmenu.h"
#include "tag.h"
#include "target.h"
#include "tests.h"
#include "unit.h"
#include "user_interface.h"
#include "utf8.h"
#include "util.h"
#include "version.h"

#ifdef SIMULATOR
#include "sim-dmcp.h"
#endif // SIMULATOR

#include <ctype.h>
#include <stdio.h>
#include <string.h>


RECORDER(command,         16, "RPL Commands");
RECORDER(command_error,   16, "Errors processing a command");
RECORDER(command_sorting, 16, "Sorting commands");


static inline bool at_end(utf8 name, size_t max, utf8 cmd, size_t len, bool eq)
// ----------------------------------------------------------------------------
//   Check if we are at end of a command
// ----------------------------------------------------------------------------
{
    return len >= max
        || (eq && (!is_valid_as_name_initial(cmd) ||
                   ((name[len] < '0' || name[len] > '9') &&
                    !is_valid_in_function_name(name + len))))
        || (is_separator(name + len) && (*cmd != '=' || name[len] != '='));
}


object::id command::lookup(utf8 name, size_t &maxlen, bool eq)
// ----------------------------------------------------------------------------
//   Lookup a command and return its ID
// ----------------------------------------------------------------------------
{
    id     type  = id(0);
    size_t len   = maxlen;

    if (!sorted_ids)
    {
        if (!initialize_sorted_ids())
        {
            static bool shown = false;
            if (!shown)
            {
                shown = true;
                ui.draw_message("Warning: Could not initialize sorted IDs",
                                "Catalog and object parsing will be slower");
            }
        }
    }

    // Binary search if we have sorted IDs
    if (sorted_ids)
    {
        size_t low  = 0;
        size_t high = sorted_ids_count;

        // Compute the maximum possible size for the command
        size_t max = maxlen;
        if (is_valid_as_name_initial(name) ||
            (!eq && utf8_codepoint(name) == L'↑')) // e.g. ↑Match
        {
            for (max = utf8_size(name, max);
                 max < maxlen && !is_separator(name + max);
                 max += utf8_size(name + max, maxlen - max))
                /* nop */;
        }
        else
        {
            // '=' is special because there is == (two consecutive separators)
            if (name[0] == '=')
            {
                maxlen = (max > 1 && name[1] == '=') ? 2 : 1;
                return  maxlen == 2 ? ID_TestSame : ID_TestEQ;
            }
            else
            {
                // All non-names cmds are 1 character
                max = utf8_size(name, maxlen);
            }
        }

        while (true)
        {
            uint mid = (low + high) / 2;
            uint16_t spidx = sorted_ids[mid];
            auto    &s     = spellings[spidx];
            id       type  = s.type;
            int      cmp   = -1;

            if (is_command(type))
            {
                if (utf8 cmd = utf8(s.name))
                {
                    // When parsing an equation skip x³ command spelling,
                    // since we parse x³ as cubed(x).
                    bool xsq = (eq && (type == ID_sq  || type == ID_cubed ||
                                       type == ID_inv || type == ID_fact) &&
                                *cmd == 'x');

                    // No function name like `min` while parsing units
                    bool uskip = unit::mode && is_valid_as_name_initial(cmd);

                    len = strlen(cstring(cmd));
                    if (len <= max)
                    {
                        cmp = strncasecmp(cstring(cmd), cstring(name), len);
                        if (cmp == 0 && at_end(name, max, cmd, len, eq))
                        {
                            if (uskip || xsq)
                                return id(0);
                            maxlen = len;
                            return type;
                        }
                        if (cmp == 0)
                            cmp = -1; // Logically equivalent to cmd ending in 0
                    }
                    else
                    {
                        cmp = strncasecmp(cstring(cmd), cstring(name), max);
                        if (cmp == 0)
                            cmp = 1; // Logically equivalent to name ending in 0
                    }
                }
            }

            if (mid == low)
                return id(0);
            if (cmp < 0)
                low = mid;
            else
                high = mid;
        }
    }

    id found = id(0);
    for (size_t i = 0; i < spelling_count; i++)
    {
        if (!is_command(spellings[i].type))
            continue;
        if (utf8 cmd = utf8(spellings[i].name))
        {
            if (type != spellings[i].type)
            {
                type = spellings[i].type;

                // When parsing an equation, parse x³ as cubed(x)
                if (eq && (type == ID_sq  || type == ID_cubed ||
                           type == ID_inv || type == ID_fact))
                    continue;
            }

            // No function names like `min` while parsing units
            if (unit::mode && is_valid_as_name_initial(cmd))
                continue;

            len = strlen(cstring(cmd));
            if (len <= maxlen
                && strncasecmp(cstring(name), cstring(cmd), len) == 0
                && at_end(name, maxlen, cmd, len, eq))
            {
                found = type;
                break;
            }
        }
    }
    if (found)
        maxlen  = len;
    return found;
}



PARSE_BODY(command)
// ----------------------------------------------------------------------------
//    Try to parse this as a command, using either short or long name
// ----------------------------------------------------------------------------
{
    bool    eq     = p.precedence;
    utf8    name   = p.source;
    size_t  len    = p.length;
    id      found  = lookup(name, len, eq);
    record(command,
           "Parsing [%s] found %u %+s %+s len %u",
           name, found, command::name(found), command::fancy(found), len);
    if (!found)
        return SKIP;

    // Record output - Dynamically generate ID for use in programs
    p.length = len;
    p.out = rt.make<command>(found);

    return OK;
}


RENDER_BODY(command)
// ----------------------------------------------------------------------------
//   Render the command into the given string buffer
// ----------------------------------------------------------------------------
{
    id ty = o->type();
    if (ty < NUM_IDS)
    {
        id format = Settings.CommandDisplayMode();

        // Ensure that we display + as `+` irrespective of mode
        utf8 fname = object::name(ty);

        while (unit::mode)
        {
            if (ty == ID_divide)
                r.put('/');
            else if (ty == ID_multiply)
                r.put(unicode(L'·'));
            else
                break;
            return r.size();
        }
        if (r.expression())
        {
            if (ty == ID_multiply &&
                format == ID_LongForm &&
                Settings.UseDotForMultiplication())
                fname = utf8("·");
            else if (ty == ID_Derivative ||
                     ty == ID_Primitive  ||
                     ty == ID_Where)
                fname = object::alias(ty, 0);
        }
        else
        {
            if (ty == ID_dot || ty == ID_cross)
                fname = object::alias(ty, 1);
        }
        r.put(format, fname);
    }

    record(command, "Render %u as [%s]", ty, (cstring) r.text());
    return r.size();
}


uint32_t command::uint32_arg(uint level)
// ----------------------------------------------------------------------------
//   Get an unsigned value from the stack
// ----------------------------------------------------------------------------
{
    if (object_p d = rt.stack(level))
        return d->as_uint32(0, true);
    return 0;
}


int32_t command::int32_arg(uint level)
// ----------------------------------------------------------------------------
//   Get a signed value from the stack
// ----------------------------------------------------------------------------
{
    if (object_p d = rt.stack(level))
        return d->as_int32(0, true);
    return 0;
}



// ============================================================================
//
//   Sorted IDs for the catalog
//
// ============================================================================

uint16_t *command::sorted_ids       = nullptr;
size_t    command::sorted_ids_count = 0;


#ifdef DEOPTIMIZE_CATALOG
// This is necessary on the DM32, where otherwise we access memory too
// fast and end up with bad data in the sorted array
#  pragma GCC push_options
#  pragma GCC optimize("-O2")
#endif // DEOPTIMIZE_CATALOG

static int sort_ids(const void *left, const void *right)
// ----------------------------------------------------------------------------
//   Sort the IDs alphabetically based on their fancy name
// ----------------------------------------------------------------------------
{
    uint16_t l = *((uint16_t *) left);
    uint16_t r = *((uint16_t *) right);
    if (!object::spellings[l].name || !object::spellings[r].name)
        return !!object::spellings[l].name - !!object::spellings[r].name;
    return strcasecmp(object::spellings[l].name, object::spellings[r].name);
}


bool command::initialize_sorted_ids()
// ----------------------------------------------------------------------------
//   Sort IDs alphabetically
// ----------------------------------------------------------------------------
{
    // Count number of items to put in the list
    uint count = 0;
    for (uint i = 0; i < object::spelling_count; i++)
        if (object::id ty = object::spellings[i].type)
            if (object::is_command(ty))
                if (object::spellings[i].name)
                    count++;

    sorted_ids = (uint16_t *) realloc(sorted_ids, count * sizeof(uint16_t));
    if (sorted_ids)
    {
        uint cmd = 0;
        for (uint i = 0; i < object::spelling_count; i++)
            if (object::id ty = object::spellings[i].type)
                if (object::is_command(ty))
                    if (object::spellings[i].name)
                        sorted_ids[cmd++] = i;
        qsort(sorted_ids, count, sizeof(sorted_ids[0]), sort_ids);

        // Make sure we have unique commands in the catalog
        cstring spelling = nullptr;
        cmd = 0;
        for (uint i = 0; i < count; i++)
        {
            uint16_t j = sorted_ids[i];
            auto &s = object::spellings[j];

            if (object::is_command(s.type))
            {
                if (cstring sp = s.name)
                {
                    if (!spelling ||
                        (spelling != sp && strcasecmp(sp, spelling) != 0))
                    {
                        sorted_ids[cmd++] = sorted_ids[i];
                        spelling = sp;
                    }
                    else if (cmd)
                    {
                        uint c = sorted_ids[cmd - 1];
                        auto &last = object::spellings[c];
                        if (s.type != last.type)
                        {
                            record(command_error,
                                   "Types %u and %u have same spelling "
                                   "%+s and %+s",
                                   s.type, last.type, spelling, sp);
                        }
                    }
                }
            }
            else
            {
                // Do not remove this code
                // It seems useless, but without it, the catalog is
                // badly broken on DM42. Apparently, the loop is a bit
                // too fast, and we end up adding a varying, but too small,
                // number of commands to the array
                debug_printf(5, "Not a command for %u, type %u[%s]",
                             i, s.type, object::name(s.type));
                debug_wait(-1);
            }
        }
        sorted_ids_count = cmd;

#if SIMULATOR
        if (RECORDER_TRACE(command_sorting))
        {
            if (RECORDER_TRACE(command_sorting) > 1)
            {
                for (size_t i = 0; i < sorted_ids_count; i++)
                {
                    auto s = spellings[sorted_ids[i]];
                    record(command_sorting, "%u: %u %s spelled %s",
                           i, s.type, object::name(s.type), s.name);
                }
            }
            else
            {
                for (int pass = 0; pass < 3; pass++)
                {
                    cstring types[3] = { "DMNONE", "DM42", "DM32" };
                    printf("<!--- %s --->\n", types[pass]);
                    bool shown[object::NUM_IDS] = { false };
                    for (size_t i = 0; i < sorted_ids_count; i++)
                    {
                        auto s = spellings[sorted_ids[i]];
                        if (!shown[s.type])
                        {
                            size_t curlen = strlen(cstring(s.name));
                            size_t shortest = curlen;
                            for (size_t j = 0; j < sorted_ids_count; j++)
                            {
                                auto a = spellings[sorted_ids[j]];
                                if (a.type == s.type)
                                {
                                    size_t len = strlen(cstring(a.name));
                                    if (len < shortest)
                                        shortest = len;
                                }
                            }
                            if (shortest == curlen)
                            {
                                int count = 0;
                                shown[s.type] = true;
                                printf("* `%s`", s.name);
                                for (size_t j = 0;
                                     !pass && j < sorted_ids_count;
                                     j++)
                                {
                                    auto a = spellings[sorted_ids[j]];
                                    if (a.type == s.type && j != i)
                                    {
                                        count++;
                                        if (count == 1)
                                            printf(" (");
                                        else
                                            printf(", ");
                                        printf("`%s`", a.name);
                                    }
                                }
                                if (count)
                                    printf(")");
                                printf("\n");
                            }
                        }
                    }
                    printf("<!--- !%s --->\n", types[pass]);
                }
            }
        }
#endif // SIMULATOR
    }
    return sorted_ids;
}

#ifdef DEOPTIMIZE_CATALOG
#pragma GCC pop_options
#endif // DEOPTIMIZE_CATALOG



// ============================================================================
//
//   Command implementations
//
// ============================================================================

COMMAND_BODY(Eval)
// ----------------------------------------------------------------------------
//   Evaluate an object
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.pop())
        return program::run(x);
    return ERROR;
}


COMMAND_BODY(Run)
// ----------------------------------------------------------------------------
//   Resume execution if halted, otherwise evaluate
// ----------------------------------------------------------------------------
{
    if (rt.run_stepping())
        return program::run_loop(0);
    if (object_p x = rt.pop())
        return program::run(x);
    return OK;
}


COMMAND_BODY(ToText)
// ----------------------------------------------------------------------------
//   Convert an object to text
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.top())
        if (object_g txt = obj->as_text())
            if (rt.top(txt))
                return OK;
    return ERROR;
}


COMMAND_BODY(Compile)
// ----------------------------------------------------------------------------
//   Interpret the object as a command line and evaluate it
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (text_p tobj = obj->as<text>())
        {
            if (rt.drop() && tobj->compile_and_run())
                return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


COMMAND_BODY(Explode)
// ----------------------------------------------------------------------------
//  Implement the Obj→ command
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    id       oty = obj->type();
    switch (oty)
    {
    case ID_rectangular:
    case ID_polar:
    case ID_unit:
    {
        complex_p cplx = complex_p(obj);
        if (rt.top(cplx->x()) && rt.push(+cplx->y()))
            return OK;
        break;
    }
    case ID_range:
    case ID_prange:
    case ID_drange:
    case ID_uncertain:
    {
        // Like the above, but with 2-byte IDs so need range::x() and range::y()
        range_p r = range_p(obj);
        if (rt.top(r->x()) && rt.push(+r->y()))
            return OK;
        break;
    }


    case ID_program:
    case ID_expression:
    case ID_list:
        if (rt.drop())
        {
            if (list_p(obj)->expand())
                return OK;
            rt.push(obj);
        }
        break;
    case ID_funcall:
        if (list_p lst = funcall_p(obj)->args())
        {
            rt.top(lst);
            return OK;
        }
        break;
    case ID_array:
        if (rt.drop())
        {
            if (array_p(obj)->expand())
                return OK;
            rt.dimension_error();
            rt.push(obj);
        }
        break;
    case ID_polynomial:
        if (algebraic_p alg = polynomial_p(obj)->as_expression())
            if (rt.top(alg))
                return OK;
        break;
    case ID_text:
        if (rt.drop())
        {
            size_t depth = rt.depth();
            if (text_p(obj)->compile_and_run())
                return OK;
            // Try to undo the damage - Won't always work
            if (rt.depth() > depth)
                rt.drop(rt.depth() - depth);
            rt.push(obj);
        }
        break;
    case ID_fraction:
    case ID_neg_fraction:
    case ID_big_fraction:
    case ID_neg_big_fraction:
    {
        fraction_p frac = fraction_p(obj);
        bignum_g num = frac->numerator();
        bignum_g den = frac->denominator();
        if (num && den && rt.top(num) && rt.push(+den))
            return OK;
        break;
    }
    case ID_tag:
    {
        tag_p tobj = tag_p(obj);
        if (rt.top(tobj->tagged_object()) && rt.push(tobj->label()))
            return OK;
        break;
    }
    default:
        rt.type_error();
        break;
    }
    return ERROR;
}


COMMAND_BODY(ReplaceChar)
// ----------------------------------------------------------------------------
//   Find the label associated to the menu and enter it in the editor
// ----------------------------------------------------------------------------
{
    if (int key = ui.evaluating_function_key())
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (symbol_p sym = ui.label(menu_idx))
            ui.replace_character_left_of_cursor(sym);
        else if (cstring lbl = ui.label_text(menu_idx))
            ui.replace_character_left_of_cursor(utf8(lbl), strlen(lbl));
    }
    return OK;
}


COMMAND_BODY(SelfInsert)
// ----------------------------------------------------------------------------
//   Find the label associated to the menu and enter it in the editor
// ----------------------------------------------------------------------------
{
    if (int key = ui.evaluating_function_key())
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (symbol_p sym = ui.label(menu_idx))
        {
            size_t len = 0;
            utf8 txt = sym->value(&len);
            ui.insert(txt, len, ui.TEXT);
        }
        else if (cstring lbl = ui.label_text(menu_idx))
        {
            ui.insert(utf8(lbl), ui.TEXT);
        }
    }
    return OK;
}


EVAL_BODY(Unimplemented)
// ----------------------------------------------------------------------------
//   Display an unimplemented error
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating_function_key();
    rt.command(o);
    if (key)
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (cstring lbl = ui.label_text(menu_idx))
            rt.command(symbol::make(lbl));
    }
    rt.unimplemented_error();
    return ERROR;
}


MARKER_BODY(Unimplemented)
// ----------------------------------------------------------------------------
//   We mark unimplemented features with a little gray mark
// ----------------------------------------------------------------------------
{
    return L'░';
}


COMMAND_BODY(Ticks)
// ----------------------------------------------------------------------------
//   Return number of ticks
// ----------------------------------------------------------------------------
{
    uint ticks = program::read_time();
    if (integer_p ti = rt.make<integer>(ID_integer, ticks))
        if (rt.push(ti))
            return OK;
    return ERROR;
}


COMMAND_BODY(Wait)
// ----------------------------------------------------------------------------
//   Wait the specified amount of seconds
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (algebraic_g wtime = obj->as_algebraic())
        {
            rt.drop();
            algebraic_g scale = integer::make(1000);
            wtime = wtime * scale;
            if (wtime)
            {
                bool     negative = wtime->is_negative();
                uint32_t msec     = negative ? 0 : wtime->as_uint32(1000, true);
                uint32_t end      = program::read_time() + msec;
                bool     infinite = msec == 0 || negative;
                int      key      = 0;

                if (negative)
                    ui.draw_menus();
                while (!key)
                {
                    // Sleep in chunks of one minute
                    int remains = infinite
                        ? 60000
                        : int(end - program::read_time());
                    if (remains <= 0)
                        break;
                    set_timer(TIMER1, remains);

                    // Do not switch off if on USB power
                    if (program::on_usb)
                        reset_auto_off();

                    // Honor auto-off while waiting, do not erase drawn image
                    power_check(true);

                    if (!key_empty())
                    {
#if SIMULATOR
                        if (key_tail() != KEY_EXIT)
                        {
                            key = key_pop();
                            record(tests_rpl,
                                   "Wait cmd processing "
                                   "key %d, last=%d, command=%u",
                                   key, last_key, test_command);
                            process_test_key(key);
                        }
#else // SIMULATOR
                        key = key_pop();
#endif // SIMULATOR
                    }
                    if (key == KEY_EXIT)
                    {
                        program::halted = true;
                        program::stepping = 0;
                    }
                    if (key == KEY_SHIFT || key == 0)
                    {
                        ui.key(key, false, false);
                        ui.draw_start(false);
                        ui.draw_annunciators();
                        ui.draw_battery();
                        refresh_dirty();
                        key = 0;
                    }
                }
                sys_timer_disable(TIMER0);
                sys_timer_disable(TIMER1);
                if (infinite)
                {
                    uint sp = ui.shift_plane();
                    uint ap = ui.alpha_plane();
                    bool ls = sp==1;
                    bool rs = sp==2;
                    bool al = ap & 1;
                    bool lc = ap & 2;
                    bool tr = ap & 4;
                    key = platform_keyid(key, ls, rs, al, lc, tr);
                    ui.clear_shift();
                    ui.draw_start(false);
                    ui.draw_annunciators();
                    ui.draw_battery();
                    refresh_dirty();
                    if (object_p keyid = compatible_key_id(key))
                        if (rt.push(keyid))
                            return OK;
                    return ERROR;
                }
                return OK;
            }
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}



COMMAND_BODY(Bytes)
// ----------------------------------------------------------------------------
//   Return the bytes and a binary represenetation of the object
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        size_t size = top->size();
        size_t maxsize = (Settings.WordSize() + 7) / 8;
        size_t hashsize = size > maxsize ? maxsize : size;
        gcbytes bytes = byte_p(top);
#if CONFIG_FIXED_BASED_OBJECTS
        // Force base 16 if we have that option
        const id type = ID_hex_bignum;
#else // !CONFIG_FIXED_BASED_OBJECTS
        const id type = ID_based_bignum;
#endif // CONFIG_FIXED_BASED_OBJECTS
        if (bignum_p bin = rt.make<bignum>(type, bytes, hashsize))
            if (rt.top(bin))
                if (rt.push(integer::make(size)))
                    return OK;
    }
    return ERROR;
}


COMMAND_BODY(Type)
// ----------------------------------------------------------------------------
//   Return the type of the top of stack as a numerical value
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        int tval = top->type_value();
        if (integer_p type = integer::make(tval))
            if (rt.top(type))
                return OK;
    }
    return ERROR;
}


COMMAND_BODY(TypeName)
// ----------------------------------------------------------------------------
//   Return the type of the top of stack as text
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
        if (text_p type = text::make(top->fancy()))
            if (rt.top(type))
                return OK;
    return ERROR;
}


COMMAND_BODY(Off)
// ----------------------------------------------------------------------------
//   Switch the calculator off
// ----------------------------------------------------------------------------
{
    power_off(false);
    return OK;
}


COMMAND_BODY(OffWithImage)
// ----------------------------------------------------------------------------
//   Switch the calculator off drawing the off image
// ----------------------------------------------------------------------------
{
    power_off(true);
    return OK;
}


COMMAND_BODY(SaveState)
// ----------------------------------------------------------------------------
//   Save the system state to disk
// ----------------------------------------------------------------------------
{
    save_system_state();
    return OK;
}


COMMAND_BODY(BatteryVoltage)
// ----------------------------------------------------------------------------
//   Return the current battery voltage, e.g. for custom headers
// ----------------------------------------------------------------------------
{
    int vdd = get_vbat();
    program::battery_voltage = vdd;
    if (algebraic_g value = integer::make(vdd))
        if (algebraic_g sym = +symbol::make("mV"))
            if (algebraic_p uvalue = unit::make(value, sym))
                if (rt.push(uvalue))
                    return OK;
    return ERROR;
}


COMMAND_BODY(PowerVoltage)
// ----------------------------------------------------------------------------
//   Return the current battery voltage, e.g. for custom headers
// ----------------------------------------------------------------------------
{
    int vdd = read_power_voltage();
    program::power_voltage = vdd;
    if (algebraic_g value = integer::make(vdd))
        if (algebraic_g sym = +symbol::make("mV"))
            if (algebraic_p uvalue = unit::make(value, sym))
                if (rt.push(uvalue))
                    return OK;
    return ERROR;
}


COMMAND_BODY(USBPowered)
// ----------------------------------------------------------------------------
//   Return True if the calculator is on USB power
// ----------------------------------------------------------------------------
{
    bool usb = usb_powered();
    program::on_usb = usb;
    if (object_p value = object::static_object(usb ? ID_True : ID_False))
        if (rt.push(value))
            return OK;
    return ERROR;
}


COMMAND_BODY(DMCPLowBattery)
// ----------------------------------------------------------------------------
//   Return True if the battery is low according to DMCP
// ----------------------------------------------------------------------------
//   This proved quite unreliable in practice, at least on DM42
//   Left in place for the purpose of experimentation by developers
{
    bool low = get_lowbat_state();
    if (object_p value = object::static_object(low ? ID_True : ID_False))
        if (rt.push(value))
            return OK;
    return ERROR;
}


COMMAND_BODY(LowBattery)
// ----------------------------------------------------------------------------
//   Return True if the battery is low
// ----------------------------------------------------------------------------
{
    uint vdd  = read_power_voltage();
    uint vmax = BATTERY_VMAX;
    uint vmin = Settings.MinimumBatteryVoltage();
    uint vlow = (vmax + 3 * vmin) / 4;
    bool low  = vdd < vlow;
    program::battery_voltage = vdd;
    if (object_p value = object::static_object(low ? ID_True : ID_False))
        if (rt.push(value))
            return OK;
    return ERROR;
}


COMMAND_BODY(SystemSetup)
// ----------------------------------------------------------------------------
//   Select the system menu
// ----------------------------------------------------------------------------
{
    system_setup();
    return OK;
}


COMMAND_BODY(ScreenCapture)
// ----------------------------------------------------------------------------
//   Snapshot the screen and save to a file
// ----------------------------------------------------------------------------
{
    if (screenshot())
        return OK;

    rt.screenshot_capture_error();
    return ERROR;
}


COMMAND_BODY(Beep)
// ----------------------------------------------------------------------------
//   Emit a sound
// ----------------------------------------------------------------------------
{
    algebraic_g duration  = rt.stack(0)->as_real();
    if (!duration)
    {
        rt.type_error();
        return ERROR;
    }
    uint frequency = rt.stack(1)->as_uint32(4400, true);
    if (frequency < 1 || frequency > 18000)
    {
        rt.drop(2);
        return OK;
    }
    duration = duration * integer::make(1000);
    if (duration)
    {
        uint ms = duration->as_uint32(10, true);
        if (ms > 10000)
            ms = 10000;
        if (!rt.error())
        {
            rt.drop(2);
            beep(frequency, ms);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(Version)
// ----------------------------------------------------------------------------
//   Return a version string
// ----------------------------------------------------------------------------
{
    const utf8 version_text = (utf8)
        "DB48X " DB48X_VERSION "\n"
        "A modern implementation of\n"
        "Reverse Polish Lisp (RPL)\n"
        "and a tribute to\n"
        "Bill Hewlett and Dave Packard\n"
        "© 2025 Christophe de Dinechin";
    if (text_g version = text::make(version_text))
        if (rt.push(object_p(version)))
            return OK;
    return ERROR;
}


COMMAND_BODY(Help)
// ----------------------------------------------------------------------------
//   Bring contextual help
// ----------------------------------------------------------------------------
{
    utf8   topic  = utf8("Overview");
    size_t length = 0;

    if (rt.depth())
    {
        if (object_p top = rt.top())
        {
            if (text_p index = top->as<text>())
            {
                utf8 what = index->value(&length);
                if (length)
                    topic = what;
            }
            else if (symbol_p index = top->as<symbol>())
            {
                topic = index->value(&length);
            }
            else
            {
                topic = top->help();
            }
        }
    }

    ui.load_help(topic, length);
    return OK;
}


COMMAND_BODY(Cycle)
// ----------------------------------------------------------------------------
//  Cycle object across multiple representations
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id     cmd   = ID_object;
        id     type  = ID_object;
        id     ttype = top->type();
        switch(ttype)
        {
        case ID_hwfloat:
        case ID_hwdouble:
        case ID_decimal:
        case ID_neg_decimal:            cmd = ID_ToFraction; break;
        case ID_integer:
        case ID_bignum:
        case ID_neg_integer:
        case ID_neg_bignum:
        case ID_fraction:
        case ID_neg_fraction:
        case ID_big_fraction:
        case ID_neg_big_fraction:       cmd = ID_ToDecimal;     break;
        case ID_polar:                  cmd = ID_ToRectangular; break;
        case ID_rectangular:            cmd = ID_ToPolar;       break;
#if CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:          type = ID_hex_integer;  break;
        case ID_hex_integer:            type = ID_dec_integer;  break;
        case ID_dec_integer:            type = ID_oct_integer;  break;
        case ID_oct_integer:            type = ID_bin_integer;  break;
        case ID_bin_integer:            type = ID_based_integer;break;

        case ID_based_bignum:           type = ID_hex_bignum;   break;
        case ID_hex_bignum:             type = ID_dec_bignum;   break;
        case ID_dec_bignum:             type = ID_oct_bignum;   break;
        case ID_oct_bignum:             type = ID_bin_bignum;   break;
        case ID_bin_bignum:             type = ID_based_bignum; break;
#else // ! CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:
        case ID_based_bignum:
        {
            uint base = Settings.Base();
            switch(base)
            {
            default:
            case 2:     base = 16; break;
            case 8:     base = 2;  break;
            case 10:    base = 8;  break;
            case 16:    base = 10; break;
            }
            Settings.Base(base);
            ui.menu_refresh(ID_BasesMenu);
            return OK;
        }
#endif // CONFIG_FIXED_BASED_OBJECTS
        case ID_range:          type = ID_drange;       break;
        case ID_drange:         type = ID_prange;       break;
        case ID_prange:         type = ID_range;        break;
        case ID_uncertain:      break;

        case ID_constant:
        case ID_equation:
        case ID_xlib:
        case ID_symbol:
        case ID_text:
        case ID_expression:
            Settings.GraphicResultDisplay(!Settings.GraphicResultDisplay());
            if (!Settings.GraphicResultDisplay())
                Settings.GraphicStackDisplay(!Settings.GraphicStackDisplay());
            if (0)
                ui.draw_message("Stack rendering",
                                Settings.GraphicStackDisplay()
                                ? "Stack shown graphically"
                                : "Stack shown as text",
                                Settings.GraphicResultDisplay()
                                ? "Results shown graphically"
                                : "Results shown as text");
            break;
        case ID_list:                   type = ID_array;        break;
        case ID_array:                  type = ID_program;      break;
        case ID_program:                type = ID_list;         break;
        case ID_unit:
        {
            // Cycle prefix
            unit_p uobj = unit_p(top);
            uobj = uobj->cycle();
            if (uobj && rt.top(uobj))
                return OK;
            return ERROR;
        }
        case ID_tag:
        {
            // Cycle tagged object, keep tag
            tag_g tagged = tag_p(top);
            size_t len = 0;
            gcutf8 label = tagged->label_value(&len);
            if (object_g obj = tagged->tagged_object())
                if (rt.top(obj))
                    if (Cycle::evaluate() == OK)
                        if ((obj = rt.top()))
                            if ((tagged = tag::make(label, len, obj)))
                                if (rt.top(tagged))
                                        return OK;
            return ERROR;
        }

        default:
            rt.type_error();
            return ERROR;
        }

        // In-place retyping
        if (type != ID_object)
        {
            ASSERT(leb128size(type) == leb128size(top->type()));
            if (object_p clone = rt.clone(top))
            {
                byte *p = (byte *) clone;
                leb128(p, type);
                if (rt.top(clone))
                    return OK;
            }
            return ERROR;
        }

        // Evaluation of a command
        if (cmd != ID_object)
            return command::static_object(cmd)->evaluate();

        return OK;
    }
    return ERROR;
}


COMMAND_BODY(ToProgram)
// ----------------------------------------------------------------------------
//  Convert top level object to program
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        list_p expr = expression::get(top);
        if (!expr)
            expr = top->as<list>();
        if (expr)
        {
            if (object_p clone = rt.clone(expr))
            {
                byte *p = (byte *) clone;
                leb128(p, ID_program);
                top = clone;
            }
        }
        else
        {
            object_g arg = top;
            top = list::make(ID_program, arg);
        }
        if (top && rt.top(top))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(BinaryToReal)
// ----------------------------------------------------------------------------
//    Convert binary values to real (really integer)
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id type = top->type();
        id to = ID_object;

        switch (type)
        {
#if CONFIG_FIXED_BASED_OBJECTS
        case ID_hex_integer:
        case ID_dec_integer:
        case ID_oct_integer:
        case ID_bin_integer:
            to = ID_integer;
            break;
        case ID_hex_bignum:
        case ID_dec_bignum:
        case ID_oct_bignum:
        case ID_bin_bignum:
            to = ID_bignum;
            break;
#endif // CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:
            to = ID_integer;
            break;
        case ID_based_bignum:
            to = ID_bignum;
            break;
        default:
            rt.type_error();
            return ERROR;
        }

        ASSERT(leb128size(type) == leb128size(to));
        if (object_p clone = rt.clone(top))
        {
            byte *p = (byte *) clone;
            leb128(p, to);
            if (rt.top(clone))
                return OK;
        }
        return ERROR;

    }
    return ERROR;
}


COMMAND_BODY(RealToBinary)
// ----------------------------------------------------------------------------
//    Convert real and integer values to binary
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
    retry:
        id   type = top->type();
        id   to   = ID_object;
        bool neg  = false;

        switch (type)
        {
        case ID_neg_integer:
            neg = true;
            [[fallthrough]];
        case ID_integer:
            to = ID_based_integer;
            break;
        case ID_neg_bignum:
            neg = true;
            [[fallthrough]];
        case ID_bignum:
            to = ID_based_bignum;
            break;
        case ID_hwfloat:
            top = hwfloat_p(top)->to_integer();
            goto retry;
        case ID_hwdouble:
            top = hwdouble_p(top)->to_integer();
            goto retry;
        case ID_neg_decimal:
            neg = true;
            [[fallthrough]];
        case ID_decimal:
            top = decimal_p(top)->to_integer();
            goto retry;
        default:
            rt.type_error();
            return ERROR;
        }

        ASSERT(leb128size(type) == leb128size(to));
        if (object_p clone = rt.clone(top))
        {
            byte *p = (byte *) clone;
            leb128(p, to);
            if (rt.top(clone))
            {
                if (neg)
                    return neg::evaluate();
                return OK;
            }
        }
        return ERROR;

    }
    return ERROR;
}



// ============================================================================
//
//   History and undo
//
// ============================================================================

COMMAND_BODY(LastArg)
// ----------------------------------------------------------------------------
//   Return the last arguments
// ----------------------------------------------------------------------------
{
    return rt.last() ? OK : ERROR;
}


COMMAND_BODY(LastX)
// ----------------------------------------------------------------------------
//   Return the last first argument
// ----------------------------------------------------------------------------
{
    return rt.last(0) ? OK : ERROR;
}


COMMAND_BODY(Undo)
// ----------------------------------------------------------------------------
//   Return the undo stack
// ----------------------------------------------------------------------------
{
    return rt.undo() ? OK : ERROR;
}



// ============================================================================
//
//   Editor commands
//
// ============================================================================

COMMAND_BODY(EditorSelect)
// ----------------------------------------------------------------------------
//   Select current cursor position
// ----------------------------------------------------------------------------
{
    return ui.editor_select() ? OK : ERROR;
}


COMMAND_BODY(EditorWordLeft)
// ----------------------------------------------------------------------------
//   Move cursor one word to the left
// ----------------------------------------------------------------------------
{
    return ui.editor_word_left() ? OK : ERROR;
}


COMMAND_BODY(EditorWordRight)
// ----------------------------------------------------------------------------
//   Move cursor one word to the right
// ----------------------------------------------------------------------------
{
    return ui.editor_word_right() ? OK : ERROR;
}


COMMAND_BODY(EditorBegin)
// ----------------------------------------------------------------------------
//   Move cursor to beginning of editor
// ----------------------------------------------------------------------------
{
    return ui.editor_begin() ? OK : ERROR;
}


COMMAND_BODY(EditorEnd)
// ----------------------------------------------------------------------------
//   Move cursor to end of editor
// ----------------------------------------------------------------------------
{
    return ui.editor_end() ? OK : ERROR;
}


COMMAND_BODY(EditorCut)
// ----------------------------------------------------------------------------
//   Cut selection
// ----------------------------------------------------------------------------
{
    return ui.editor_cut() ? OK : ERROR;
}


COMMAND_BODY(EditorCopy)
// ----------------------------------------------------------------------------
//   Copy selection
// ----------------------------------------------------------------------------
{
    return ui.editor_copy() ? OK : ERROR;
}


COMMAND_BODY(EditorPaste)
// ----------------------------------------------------------------------------
//   Paste selection
// ----------------------------------------------------------------------------
{
    return ui.editor_paste() ? OK : ERROR;
}


COMMAND_BODY(EditorSearch)
// ----------------------------------------------------------------------------
//   Search selection
// ----------------------------------------------------------------------------
{
    return ui.editor_search() ? OK : ERROR;
}


COMMAND_BODY(EditorReplace)
// ----------------------------------------------------------------------------
//   Replace selection
// ----------------------------------------------------------------------------
{
    return ui.editor_replace() ? OK : ERROR;
}


COMMAND_BODY(EditorClear)
// ----------------------------------------------------------------------------
//   Clear selection
// ----------------------------------------------------------------------------
{
    return ui.editor_clear() ? OK : ERROR;
}


COMMAND_BODY(EditorFlip)
// ----------------------------------------------------------------------------
//   Flip selection point and cursor
// ----------------------------------------------------------------------------
{
    return ui.editor_selection_flip() ? OK : ERROR;
}


COMMAND_BODY(EditorHistory)
// ----------------------------------------------------------------------------
//   Select the last history entry in the editor
// ----------------------------------------------------------------------------
{
    ui.editor_history();
    return OK;
}


COMMAND_BODY(EditorHistoryBack)
// ----------------------------------------------------------------------------
//   Select the previous history entry in the editor
// ----------------------------------------------------------------------------
{
    ui.editor_history(true);
    return OK;
}



// ============================================================================
//
//   User interface commands (from the keyboard map)
//
// ============================================================================

COMMAND_BODY(Edit)
// ----------------------------------------------------------------------------
//   Edit the top-level item
// ----------------------------------------------------------------------------
{
    return ui.do_edit() ? OK : ERROR;
}


COMMAND_BODY(StackEditor)
// ----------------------------------------------------------------------------
//   Enter the interactive stack
// ----------------------------------------------------------------------------
{
    if (!Stack.interactive)
    {
        Stack.interactive = 1;
        ui.dirty_all();
    }

    return OK;
}


COMMAND_BODY(UILeftShift)
// ----------------------------------------------------------------------------
//   Set left shift
// ----------------------------------------------------------------------------
{
    ui.shift_plane(1);
    return OK;
}


COMMAND_BODY(UIRightShift)
// ----------------------------------------------------------------------------
//   Set right shift
// ----------------------------------------------------------------------------
{
    ui.shift_plane(2);
    return OK;
}


COMMAND_BODY(UINoShift)
// ----------------------------------------------------------------------------
//   Clear shifts
// ----------------------------------------------------------------------------
{
    ui.shift_plane(0);
    return OK;
}


COMMAND_BODY(UIAlpha)
// ----------------------------------------------------------------------------
//   Set alpha mode
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(1);
    return OK;
}


COMMAND_BODY(UILowercase)
// ----------------------------------------------------------------------------
//   Set right shift
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(2);
    return OK;
}


COMMAND_BODY(UINoAlpha)
// ----------------------------------------------------------------------------
//   Clear shifts
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(0);
    return OK;
}


COMMAND_BODY(UIAlgebraic)
// ----------------------------------------------------------------------------
//   Magic key for algebraics
// ----------------------------------------------------------------------------
{
    return ui.do_algebraic() ? OK : ERROR;
}


COMMAND_BODY(UIEnter)
// ----------------------------------------------------------------------------
//   Enter current command line
// ----------------------------------------------------------------------------
{
    return ui.do_enter() ? OK : ERROR;
}


COMMAND_BODY(UIExit)
// ----------------------------------------------------------------------------
//   Clear command line
// ----------------------------------------------------------------------------
{
    return ui.do_exit() ? OK : ERROR;
}


COMMAND_BODY(UIDecimal)
// ----------------------------------------------------------------------------
//   Command for decimal separators
// ----------------------------------------------------------------------------
{
    return ui.do_decimal_separator() ? OK : ERROR;
}


COMMAND_BODY(UIText)
// ----------------------------------------------------------------------------
//   Command to insert text
// ----------------------------------------------------------------------------
{
    return ui.do_text() ? OK : ERROR;
}


COMMAND_BODY(UILeft)
// ----------------------------------------------------------------------------
//   Move cursor left or enter interactive stack
// ----------------------------------------------------------------------------
{
    return ui.do_left() ? OK : ERROR;
}


COMMAND_BODY(UIRight)
// ----------------------------------------------------------------------------
//   Move cursor right or start editing
// ----------------------------------------------------------------------------
{
    return ui.do_right() ? OK : ERROR;
}


COMMAND_BODY(UIUp)
// ----------------------------------------------------------------------------
//   Move cursor up or go back in history
// ----------------------------------------------------------------------------
{
    return ui.do_up() ? OK : ERROR;
}


COMMAND_BODY(UIDown)
// ----------------------------------------------------------------------------
//   Move cursor down or bring up editor menu
// ----------------------------------------------------------------------------
{
    return ui.do_down() ? OK : ERROR;
}


COMMAND_BODY(UIBackspace)
// ----------------------------------------------------------------------------
//   Delete what is left of cursor
// ----------------------------------------------------------------------------
{
    return ui.do_delete(false) ? OK : ERROR;
}


COMMAND_BODY(UIDelete)
// ----------------------------------------------------------------------------
//   Delete what is right of cursor
// ----------------------------------------------------------------------------
{
    return ui.do_delete(true) ? OK : ERROR;
}
