#ifndef SIM_EVAL_H
#define SIM_EVAL_H
// ****************************************************************************
//  sim-eval.h                                                    DB48X project
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

#include "types.h"

#include <string>
#include <vector>

struct sim_commands
// ----------------------------------------------------------------------------
//  List of RPL commands to evaluate
// ----------------------------------------------------------------------------
{
    sim_commands() : commands(), headless(false), print_levels(false) {}
    ~sim_commands() {}

    bool queue(cstring arg, bool print, bool file);
    bool queue_snapshot(cstring arg);
    bool run(const std::string &cmd);
    void print_stack();
    void process_commands();

    enum opcode
    {
        EXECUTE,
        PRINT,
        SNAPSHOT
    };
    struct command
    {
        command(opcode op, cstring arg): op(op), arg(arg) {}
        command(opcode op, const std::string &arg): op(op), arg(arg) {}
        opcode      op;
        std::string arg;
    };

    std::vector<command>     commands;
    bool                     headless;
    bool                     print_levels;
};

extern sim_commands rplcmds;

#endif // SIM_EVAL_H
