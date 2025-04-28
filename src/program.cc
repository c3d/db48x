// ****************************************************************************
//  program.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of RPL programs and blocks
//
//     Programs are lists with a special way to execute
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
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

#include "program.h"

#include "dmcp.h"
#include "parser.h"
#include "settings.h"
#include "sysmenu.h"
#include "tag.h"
#include "tests.h"
#include "unit.h"
#include "util.h"
#include "variables.h"


RECORDER(program, 16, "Program evaluation");



// ============================================================================
//
//    Program
//
// ============================================================================

EVAL_BODY(program)
// ----------------------------------------------------------------------------
//   Normal evaluation from a program places it on stack
// ----------------------------------------------------------------------------
{
    if (running)
        return rt.push(o) ? OK : ERROR;
    return o->run_program();
}


PARSE_BODY(program)
// ----------------------------------------------------------------------------
//    Try to parse this as a program
// ----------------------------------------------------------------------------
{
    return list_parse(ID_program, p, L'«', L'»');
}


RENDER_BODY(program)
// ----------------------------------------------------------------------------
//   Render the program into the given program buffer
// ----------------------------------------------------------------------------
{
    return o->list_render(r, L'«', L'»');
}


HELP_BODY(program)
// ----------------------------------------------------------------------------
//   Help topic for programs
// ----------------------------------------------------------------------------
{
    return utf8("Programs");
}


program_p program::parse(utf8 source, size_t size)
// ----------------------------------------------------------------------------
//   Parse a program without delimiters (e.g. command line)
// ----------------------------------------------------------------------------
{
    record(program, ">Parsing command line [%s]", source);
    parser p(source, size);
    result r = list_parse(ID_program, p, 0, 0);
    record(program, "<Command line [%s], end at %u, result %p",
           utf8(p.source), p.length, object_p(p.out));
    if (r != OK)
        return nullptr;
    object_p  obj  = p.out;
    if (!obj)
        return nullptr;
    program_p prog = obj->as<program>();
    return prog;
}


#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

object::result program::run(bool synchronous) const
// ----------------------------------------------------------------------------
//   Execute a program
// ----------------------------------------------------------------------------
{
    size_t   depth     = rt.call_depth();
    bool     outer     = depth == 0 && !running;
    object_p first     = objects();
    object_p end       = skip();

    record(program, "Run %p (%p-%p) %+s",
           this, first, end, outer ? "outer" : "inner");

    if (!rt.run_push(first, end))
        return ERROR;
    if (outer || synchronous)
        return run_loop(depth);

    return OK;
}


object::result program::run(object_p obj, bool sync)
// ----------------------------------------------------------------------------
//    Run a program as top-level
// ----------------------------------------------------------------------------
{
    if (program_p prog = obj->as_program())
        return prog->run(sync);
    if (directory_p dir = obj->as<directory>())
        return dir->enter();
    return obj->evaluate();
}


object::result program::run(algebraic_p obj, bool sync)
// ----------------------------------------------------------------------------
//    Run a program as top-level as an algebraic
// ----------------------------------------------------------------------------
{
    if (program_p prog = obj->as_program())
    {
        settings::PrepareForProgramEvaluation nosave;
        return prog->run(sync);
    }
    if (directory_p dir = obj->as<directory>())
        return dir->enter();
    size_t depth = rt.call_depth();
    result ok    = object_p(obj)->evaluate();
    if (sync && ok == OK)
        ok = run_loop(depth);
    return ok;
}

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42


object::result program::run_loop(size_t depth)
// ----------------------------------------------------------------------------
//   Continue executing a program
// ----------------------------------------------------------------------------
//   The 'save_last_args' indicates if we save `LastArgs` at this level
{
    result   result    = OK;
    bool     outer     = depth == 0 && !running;
    bool     last_args = outer
        ? Settings.SaveLastArguments()
        : Settings.ProgramLastArguments();

    save<bool> save_running(running, true);
    object_g   obj;

    while ((obj = rt.run_next(depth)))
    {
        if (interrupted())
        {
            obj->defer();
            if (ui.in_input())
            {
                halted = false;
            }
            else if (!halted)
            {
                result = ERROR;
                rt.interrupted_error().command(obj);
            }
            break;
        }
        if (last_args)
            rt.need_save();
        result = obj->evaluate();

        if (result != OK)
        {
            if (Settings.DebugOnError())
            {
                obj->defer();
                static_object(ID_DebugMenu)->evaluate();
            }
            else
            {
                while(rt.run_next(depth));
            }
            break;
        }

        if (stepping)
        {
            ui.draw_busy(L'›', Settings.SteppingIconForeground());
            halted = --stepping == 0;
        }
    }

    return result;
}


uint program::count_interrupted = 0;
uint program::last_interrupted = 0;
uint program::last_power_check = 1U << 31;

bool program::interrupted()
// ----------------------------------------------------------------------------
//   Return true if the current program must be interrupted
// ----------------------------------------------------------------------------
{
    if (count_interrupted++ < 32)
        return halted;

    count_interrupted = 0;
    reset_auto_off();
    uint now = program::read_time();
    if (now - last_power_check >= Settings.BatteryRefresh())
    {
        if (low_battery())
            power_off(false);
    }
    if (now - last_interrupted >= Settings.BusyIndicatorRefresh())
    {
        ui.draw_busy();
        last_interrupted = now;
    }
    while (!key_empty())
    {
        int tail = key_tail();
        if (tail == KEY_EXIT)
        {
            halted = true;
            stepping = 0;
            return true;
        }
#if SIMULATOR
        int key = key_pop();
        extern int last_key;
        record(tests_rpl,
               "Program runner popped key %d, last=%d", key, last_key);
        process_test_key(key);
#else
        key_pop();
#endif
    }
    return halted;
}


bool program::battery_low = false;
uint program::ticks = 0;
uint program::battery_voltage = 999;
uint program::power_voltage = 999;

bool program::low_battery()
// ----------------------------------------------------------------------------
//    Return true if we are running low on battery
// ----------------------------------------------------------------------------
//    Experimentatlly get_lowbat_state() is not reliable
//    Offer an adjustable "low battery" threshold
{
    uint now = program::read_time();
    if (now - last_power_check >= Settings.BatteryRefresh())
        read_battery();
    uint vlow = Settings.MinimumBatteryVoltage();
    return !animated() &&
        (battery_low || battery_voltage < vlow || power_voltage < vlow);
}


uint program::read_battery()
// ----------------------------------------------------------------------------
//   Read battery information
// ----------------------------------------------------------------------------
{
    on_usb  = usb_powered();
    battery_low = get_lowbat_state();
    battery_voltage = get_vbat();
    power_voltage = read_power_voltage();
    last_power_check = program::read_time();
    return battery_voltage;
}


uint program::read_time()
// ----------------------------------------------------------------------------
//   Read real-time clock time
// ----------------------------------------------------------------------------
//   The sys_current_ms() function stops running when timers are off.
{
    dt_t date;
    tm_t time;
    rtc_read(&time, &date);
    int jd = julian_day(&date);
    ticks = ((time.sec +
             time.min * 60 +
             time.hour * 3600 +
              jd * 86400) * 100 + time.csec) * 10;
    return ticks;
}




// ============================================================================
//
//    Block
//
// ============================================================================

RENDER_BODY(block)
// ----------------------------------------------------------------------------
//   Render the program into the given program buffer
// ----------------------------------------------------------------------------
{
    return o->list_render(r, 0, 0);
}


EVAL_BODY(block)
// ----------------------------------------------------------------------------
//   Normal evaluation from a program places it on stack
// ----------------------------------------------------------------------------
{
    return o->run_program();
}



// ============================================================================
//
//   Debugging
//
// ============================================================================

bool   program::running       = false;
bool   program::halted        = false;
bool   program::on_usb        = true;
uint   program::stepping      = 0;


COMMAND_BODY(Halt)
// ----------------------------------------------------------------------------
//   Set the 'halted' flag to interrupt the program
// ----------------------------------------------------------------------------
{
    program::halted = true;
    program::stepping = 0;
    return OK;
}


COMMAND_BODY(Debug)
// ----------------------------------------------------------------------------
//   Take a program and evaluate it in halted mode
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (program_p prog = obj->as_program())
        {
            settings::SaveDebugOnError doe(true);
            rt.pop();
            program::halted = true;
            program::stepping = 0;
            prog->run_program();
            return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


COMMAND_BODY(SingleStep)
// ----------------------------------------------------------------------------
//   Single step an instruction
// ----------------------------------------------------------------------------
{
    if (!rt.run_stepping())
    {
        rt.no_debugged_program_error();
        return ERROR;
    }

    settings::SaveDebugOnError doe(true);
    program::stepping = 1;
    program::halted = false;
    return program::run_loop(0);
}


COMMAND_BODY(StepOver)
// ----------------------------------------------------------------------------
//   Step over the next instruction
// ----------------------------------------------------------------------------
{
    if (!rt.run_stepping())
    {
        rt.no_debugged_program_error();
        return ERROR;
    }

    if (object_p next = rt.run_next(0))
    {
        settings::SaveDebugOnError doe(true);
        size_t depth = rt.call_depth();
        save<bool> no_halt(program::halted, false);
        if (!next->defer())
            return ERROR;
        return program::run_loop(depth);
    }
    return OK;
}


COMMAND_BODY(StepOut)
// ----------------------------------------------------------------------------
//   Step over the next instruction
// ----------------------------------------------------------------------------
{
    if (!rt.run_stepping())
    {
        rt.no_debugged_program_error();
        return ERROR;
    }

    settings::SaveDebugOnError doe(true);
    size_t depth = rt.call_depth();
    save<bool> no_halt(program::halted, false);
    return program::run_loop(depth > 2 ? depth - 2 : 0);
}


COMMAND_BODY(MultipleSteps)
// ----------------------------------------------------------------------------
//   Step multiple instructions
// ----------------------------------------------------------------------------
{
    if (!rt.run_stepping())
    {
        rt.no_debugged_program_error();
        return ERROR;
    }

    if (object_p obj = rt.top())
    {
        if (uint steps = obj->as_uint32(0, true))
        {
            settings::SaveDebugOnError doe(true);
            rt.pop();
            program::stepping = steps;
            program::halted = false;
            return program::run_loop(0);
        }
    }
    return ERROR;
}


COMMAND_BODY(Continue)
// ----------------------------------------------------------------------------
//   Resume execution of the current program
// ----------------------------------------------------------------------------
{
    settings::SaveDebugOnError doe(true);
    program::halted = false;
    program::stepping = 0;
    return program::run_loop(0);
}


COMMAND_BODY(Kill)
// ----------------------------------------------------------------------------
//   Kill program execution
// ----------------------------------------------------------------------------
{
    // Flush the current program
    while (rt.run_next(0));
    program::halted = false;
    program::stepping = 0;
    return OK;
}




// ============================================================================
//
//   Program statistics
//
// ============================================================================

ularge program::run_cycles         = 0;
ularge program::active_time        = 0;
ularge program::sleeping_time      = 0;
ularge program::display_time       = 0;
ularge program::stack_display_time = 0;
ularge program::refresh_time       = 0;

COMMAND_BODY(RuntimeStatistics)
// ----------------------------------------------------------------------------
//   Return runtime statistics
// ----------------------------------------------------------------------------
{
    algebraic_g ms = +symbol::make("ms");
    tag_g running =
        tag::make("Running",
                  unit::make(integer::make(program::active_time), ms));
    tag_g sleeping =
        tag::make("Sleeping",
                  unit::make(integer::make(program::sleeping_time), ms));
    tag_g display =
        tag::make("Display",
                  unit::make(integer::make(program::display_time), ms));
    tag_g stack =
        tag::make("Stack",
                  unit::make(integer::make(program::stack_display_time), ms));
    tag_g refresh =
        tag::make("Refresh",
                  unit::make(integer::make(program::refresh_time), ms));
    tag_g runcycles = tag::make("Runs", integer::make(program::run_cycles));

    if (running && sleeping && runcycles)
    {
        scribble scr;
        if (rt.append(running)   &&
            rt.append(sleeping)  &&
            rt.append(display)   &&
            rt.append(stack)     &&
            rt.append(refresh)   &&
            rt.append(runcycles))
        {
            size_t sz = scr.growth();
            gcbytes data = scr.scratch();
            if (array_p a = rt.make<array>(ID_array, data, sz))
            {
                if (rt.push(a))
                {
                    if (Settings.RunStatsClearAfterRead())
                    {
                        program::active_time        = 0;
                        program::sleeping_time      = 0;
                        program::display_time       = 0;
                        program::stack_display_time = 0;
                        program::refresh_time       = 0;
                        program::run_cycles         = 0;
                    }
                    return OK;
                }
            }
        }
    }

    return  ERROR;
}
