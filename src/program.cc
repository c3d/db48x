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
#include "tests.h"
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
//   The 'save_last_args' indicates if we save `LastArgs` at this level
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
    return object_p(obj)->evaluate();
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
    object_g obj;
    while ((obj = rt.run_next(depth)))
    {
        if (interrupted())
        {
            obj->defer();
            if (!halted)
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


static uint last_interrupted = 0;
static uint count_interrupted = 0;

bool program::interrupted()
// ----------------------------------------------------------------------------
//   Return true if the current program must be interrupted
// ----------------------------------------------------------------------------
{
    if (count_interrupted++ < 32)
        return halted;

    count_interrupted = 0;
    reset_auto_off();
    uint now = sys_current_ms();
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

bool program::running = false;
bool program::halted = false;
uint program::stepping = 0;


COMMAND_BODY(Halt)
// ----------------------------------------------------------------------------
//   Set the 'halted' flag to interrupt the program
// ----------------------------------------------------------------------------
{
    program::halted = true;
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
