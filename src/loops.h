#ifndef LOOPS_H
#define LOOPS_H
// ****************************************************************************
//  loops.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Implement basic loops
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
//
// Payload format:
//
//   Loops have the same format:
//   - ID for the type
//   - Total length
//   - Condition object, typically a program
//   - Body object, typically a program, which is executed repeatedly


#include "command.h"
#include "program.h"
#include "symbol.h"


struct loop : command
// ----------------------------------------------------------------------------
//    Loop structures
// ----------------------------------------------------------------------------
{
    loop(object_g body, symbol_g name, id type);
    result condition(bool &value) const;

    static size_t required_memory(id i, object_g body, symbol_g name)
    {
        return leb128size(i)
            + (name ? object_p(symbol_p(name))->size() : 0)
            + body->size();
    }

    static bool interrupted()   { return program::interrupted(); }

protected:
    // Shared code for parsing and rendering, taking delimiters as input
    static result object_parser(parser UNUSED &p,
                                cstring open,
                                cstring middle,
                                cstring close2, id id2,
                                cstring close1, id id1,
                                cstring terminator,
                                bool    loopvar);
    static result object_parser(parser UNUSED &p,
                                cstring op,
                                cstring mid,
                                cstring cl1, id id1,
                                cstring cl2, id id2,
                                bool    loopvar)
    {
        // Warning: close1/close2 intentionally swapped here
        return object_parser(p, op, mid, cl1, id1, cl2, id2, nullptr, loopvar);
    }
    intptr_t object_renderer(renderer &r,
                             cstring open, cstring middle, cstring close,
                             bool loopvar = false) const;
    static result counted(object_g body, bool stepping, bool named = false);

public:
    SIZE_DECL(loop);
};


struct conditional_loop : loop
// ----------------------------------------------------------------------------
//    Loop structures
// ----------------------------------------------------------------------------
{
    conditional_loop(object_g condition, object_g body, id type);
    static result condition(bool &value);

    static size_t required_memory(id i, object_g condition, object_g body)
    {
        return leb128size(i) + condition->size() + body->size();
    }

protected:
    // Shared code for parsing and rendering, taking delimiters as input
    static result counted(object_g body, bool stepping);

public:
    SIZE_DECL(conditional_loop);
};


struct DoUntil : conditional_loop
// ----------------------------------------------------------------------------
//   do...until...end loop
// ----------------------------------------------------------------------------
{
    DoUntil(object_g condition, object_g body, id type)
        : conditional_loop(condition, body, type) {}

public:
    OBJECT_DECL(DoUntil);
    PARSE_DECL(DoUntil);
    EVAL_DECL(DoUntil);
    RENDER_DECL(DoUntil);
    INSERT_DECL(DoUntil);
};


struct WhileRepeat : conditional_loop
// ----------------------------------------------------------------------------
//   while...repeat...end loop
// ----------------------------------------------------------------------------
{
    WhileRepeat(object_g condition, object_g body, id type)
        : conditional_loop(condition, body, type) {}

public:
    OBJECT_DECL(WhileRepeat);
    PARSE_DECL(WhileRepeat);
    EVAL_DECL(WhileRepeat);
    RENDER_DECL(WhileRepeat);
    INSERT_DECL(WhileRepeat);
};


struct StartNext : loop
// ----------------------------------------------------------------------------
//   start..next loop
// ----------------------------------------------------------------------------
{
    StartNext(object_g body, id type): loop(body, nullptr, type) {}
    StartNext(object_g body, symbol_g n, id type): loop(body, n, type) {}

public:
    OBJECT_DECL(StartNext);
    PARSE_DECL(StartNext);
    EVAL_DECL(StartNext);
    RENDER_DECL(StartNext);
    INSERT_DECL(StartNext);
};


struct StartStep : StartNext
// ----------------------------------------------------------------------------
//   start..step loop
// ----------------------------------------------------------------------------
{
    StartStep(object_g body, id type): StartNext(body, type) {}

public:
    OBJECT_DECL(StartStep);
    PARSE_DECL(StartStep);
    EVAL_DECL(StartStep);
    RENDER_DECL(StartStep);
    INSERT_DECL(StartStep);
};


struct ForNext : StartNext
// ----------------------------------------------------------------------------
//   for..next loop
// ----------------------------------------------------------------------------
{
    ForNext(object_g body, symbol_g name, id type)
        : StartNext(body, name, type) {}

    static result counted(object_p o, bool stepping);

public:
    OBJECT_DECL(ForNext);
    PARSE_DECL(ForNext);
    SIZE_DECL(ForNext);
    EVAL_DECL(ForNext);
    RENDER_DECL(ForNext);
    INSERT_DECL(ForNext);
};


struct ForStep : ForNext
// ----------------------------------------------------------------------------
//   for..step loop
// ----------------------------------------------------------------------------
{
    ForStep(object_g body, symbol_g name, id type)
        : ForNext(body, name, type) {}

public:
    OBJECT_DECL(ForStep);
    PARSE_DECL(ForStep);
    EVAL_DECL(ForStep);
    RENDER_DECL(ForStep);
    INSERT_DECL(ForStep);
};

#endif // LOOPS_H
