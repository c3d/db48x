#ifndef SIM_EVAL_H
#define SIM_EVAL_H
// ****************************************************************************
//  sim-eval.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Evaluate RPL command lines passed to the simulator (-e option)
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

extern std::vector<std::string> sim_eval_commands;

bool     sim_eval_run(cstring line);
void     process_sim_eval_commands();
void     sim_eval_set_pending(cstring line);
cstring  sim_eval_pending_line();

#endif // SIM_EVAL_H
