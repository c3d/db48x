// ****************************************************************************
//  sim-eval.cpp                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Evaluate RPL command lines passed to the simulator (-e/-E options)
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
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

#include "sim-eval.h"

#include "dmcp.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "tests.h"

#include <cstdio>
#include <cstring>

std::vector<std::string> sim_eval_commands;
std::vector<std::string> sim_eval_console_commands;
bool                     sim_eval_headless = false;
bool                     sim_eval_print_levels = false;
static std::string       sim_eval_pending;


void sim_eval_set_pending(cstring line)
// ----------------------------------------------------------------------------
//   Queue a command line for evaluation on the RPL thread
// ----------------------------------------------------------------------------
{
    sim_eval_pending = line ? line : "";
}


cstring sim_eval_pending_line()
// ----------------------------------------------------------------------------
//   Return the command line queued for evaluation on the RPL thread
// ----------------------------------------------------------------------------
{
    return sim_eval_pending.c_str();
}


bool sim_eval_run(cstring line)
// ----------------------------------------------------------------------------
//   Parse and evaluate a command line like interactive entry
// ----------------------------------------------------------------------------
{
    if (!line || !*line)
        return true;

    size_t    len  = strlen(line);
    program_g cmds = program::parse((utf8) line, len);
    if (!cmds)
    {
        fprintf(stderr, "Cannot parse command line: %s\n", line);
        return false;
    }

    save<bool> no_halt(program::halted, false);
    return cmds->run(false) == object::OK;
}


void sim_eval_print_stack()
// ----------------------------------------------------------------------------
//   Print all stack levels, bottom first (optional level prefixes)
// ----------------------------------------------------------------------------
{
    FILE  *out = sim_eval_headless ? stdout : stderr;
    uint   depth = rt.depth();
    bool   rml   = Settings.MultiLineResult();
    bool   sml   = Settings.MultiLineStack();

    for (uint n = depth; n > 0; n--)
    {
        uint     level = n - 1;
        object_p obj   = rt.stack(level);
        if (!obj)
            continue;

        bool     ml  = level ? sml : rml;
        renderer r(nullptr, ~0U, true, ml);
        size_t   len = obj->render(r);
        utf8     text = r.text();

        if (sim_eval_print_levels)
            fprintf(out, "%u:", level + 1);
        if (len)
            fwrite(text, 1, len, out);
        fputc('\n', out);
    }

    if (utf8 err = rt.error())
        fprintf(stderr, "%s\n", cstring(err));

    fflush(out);
}


void process_sim_eval_commands()
// ----------------------------------------------------------------------------
//   Run queued -e or -E command lines once at simulator startup
// ----------------------------------------------------------------------------
{
    static bool done = false;
    if (done)
        return;
    done = true;

    if (!sim_eval_console_commands.empty())
    {
        for (const std::string &cmd : sim_eval_console_commands)
        {
            sim_eval_run(cmd.c_str());
            sim_eval_print_stack();
        }
        sim_eval_console_commands.clear();
        if (sim_eval_headless)
            key_push(tests::EXIT_PGM);
    }

    for (const std::string &cmd : sim_eval_commands)
        sim_eval_run(cmd.c_str());
    sim_eval_commands.clear();
}
