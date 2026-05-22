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
#include <fstream>
#include <iostream>
#include <iterator>


// RPL commands piled up for the RPL thread to pick up
sim_commands rplcmds;

static void transliterate(std::string &s)
// ----------------------------------------------------------------------------
//   Transliterate RPL special characters from source
// ----------------------------------------------------------------------------
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++)
    {
        cstring repl = nullptr;
        if (i + 1 < s.size())
        {
            if (s[i] == '<' && s[i + 1] == '<')
                repl = "«";
            else if (s[i] == '>' && s[i + 1] == '>')
                repl = "»";
            else if (s[i] == '-' && s[i + 1] == '>')
                repl = "→";
        }
        if (repl)
            ++i, out += repl;
        else
            out += s[i];
    }
    s.swap(out);
}


static bool read_stream(std::istream &input, std::string &out, cstring name)
// ----------------------------------------------------------------------------
//   Read commands from a file or stdin stream
// ----------------------------------------------------------------------------
{
    std::string data((std::istreambuf_iterator<char>(input)),
                     std::istreambuf_iterator<char>());
    if (input.bad())
    {
        fprintf(stderr, "Cannot read commands from %s\n", name);
        return false;
    }
    data.swap(out);
    return true;
}


bool sim_commands::queue(cstring arg, bool print, bool file)
// ----------------------------------------------------------------------------
//   Queue a command line; arg "-" reads all of stdin (one command per line)
// ----------------------------------------------------------------------------
{
    if (!arg || !*arg)
    {
        fprintf(stderr, "Missing argument");
        return false;
    }

    std::string command;
    if (file)
    {
        if (strcmp(arg, "-"))
        {
            std::ifstream f(arg);
            if (!read_stream(f, command, arg))
                return false;
        }
        else
        {
            if (!read_stream(std::cin, command, "standard input"))
                return false;
        }
    }
    else
    {
        command = arg;
    }

    transliterate(command);
    commands.emplace_back(command);
    if (print)
        commands.emplace_back();
    return true;
}


bool sim_commands::run(const std::string &cmd)
// ----------------------------------------------------------------------------
//   Parse and evaluate a command line like interactive entry
// ----------------------------------------------------------------------------
{
    program_g cmds = program::parse(utf8(cmd.c_str()), cmd.length());
    if (!cmds)
    {
        fprintf(stderr, "Cannot parse command line: %s\n", cmd.c_str());
        return false;
    }

    save<bool> no_halt(program::halted, false);
    return cmds->run(false) == object::OK;
}


void sim_commands::print_stack()
// ----------------------------------------------------------------------------
//   Print all stack levels, bottom first (optional level prefixes)
// ----------------------------------------------------------------------------
{
    FILE  *out   = headless ? stdout : stderr;
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

        if (print_levels)
            fprintf(out, "%u:", level + 1);
        if (len)
            fwrite(text, 1, len, out);
        fputc('\n', out);
    }

    if (utf8 err = rt.error())
        fprintf(stderr, "%s\n", cstring(err));

    fflush(out);
}


void sim_commands::process_commands()
// ----------------------------------------------------------------------------
//   Run queued -e or -E command lines once at simulator startup
// ----------------------------------------------------------------------------
{
    for (const std::string &cmd : commands)
    {
        if (cmd.empty())
            print_stack();
        else
            run(cmd);
    }
    commands.clear();
    if (headless)
        key_push(tests::EXIT_PGM);
}
