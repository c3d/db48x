#ifndef UTIL_H
#define UTIL_H
// ****************************************************************************
//  util.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Basic utilities
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

#include "types.h"
#include <cstring>

void beep(uint frequency, uint duration);
void click(uint frequency = 4400);
bool screenshot();
bool exit_key_pressed();
void power_check(bool running, bool offimage = true);

inline cstring strend(cstring s)        { return s + strlen(s); }
inline char *  strend(char *s)          { return s + strlen(s); }
char *render_u64(char *buffer, ularge value);
char *render_i64(char *buffer, large value);

#endif // UTIL_H
