#ifndef CONDITIONALS_H
#  define CONDITIONALS_H
// ****************************************************************************
//  conditionals.h                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Implement RPL conditionals (If-Then, If-Then-Else, IFT, IFTE)
//
//
//
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

#include "loops.h"


struct IfThen : conditional_loop
// ----------------------------------------------------------------------------
//   The 'if-then' command behaves mostly like a conditional loop
// ----------------------------------------------------------------------------
{
    IfThen(object_g condition, object_g body, id type = ID_IfThen)
        : conditional_loop(condition, body, type) { }

    OBJECT_DECL(IfThen);
    PARSE_DECL(IfThen);
    RENDER_DECL(IfThen);
    EVAL_DECL(IfThen);
    INSERT_DECL(IfThen);
};


struct IfThenElse : IfThen
// ----------------------------------------------------------------------------
//   The if-then-else command adds the `else` part
// ----------------------------------------------------------------------------
{
    IfThenElse(object_g cond, object_g ift, object_g iff, id t = ID_IfThenElse)
        : IfThen(cond, ift, t)
    {
        // Copy the additional object
        // Do NOT use payload(this) here:
        // ID_IfThenElse is 1 byte, ID_IfErrThenElse is 2 bytes
        object_p p = object_p(payload());
        object_p after = p->skip()->skip();
        byte *tgt = (byte *) after;
        size_t iffs = iff->size();
        memcpy(tgt, iff.Safe(), iffs);
    }

    static size_t required_memory(id i,
                                  object_g cond, object_g ift, object_g iff)
    {
        return leb128size(i) + cond->size() + ift->size() + iff->size();
    }

    OBJECT_DECL(IfThenElse);
    PARSE_DECL(IfThenElse);
    RENDER_DECL(IfThenElse);
    EVAL_DECL(IfThenElse);
    SIZE_DECL(IfThenElse);
    INSERT_DECL(IfThenElse);
};


struct IfErrThen : IfThen
// ----------------------------------------------------------------------------
//    iferr-then-end  statement
// ----------------------------------------------------------------------------
{
    IfErrThen(object_g condition, object_g body, id type = ID_IfErrThen)
        : IfThen(condition, body, type) { }

    OBJECT_DECL(IfErrThen);
    PARSE_DECL(IfErrThen);
    RENDER_DECL(IfErrThen);
    EVAL_DECL(IfErrThen);
    INSERT_DECL(IfErrThen);
};


struct IfErrThenElse : IfThenElse
// ----------------------------------------------------------------------------
//   The if-then-else command adds the `else` part
// ----------------------------------------------------------------------------
{
    IfErrThenElse(object_g cond, object_g ift, object_g iff,
                  id t = ID_IfErrThenElse)
        : IfThenElse(cond, ift, iff, t)
    { }

    OBJECT_DECL(IfErrThenElse);
    PARSE_DECL(IfErrThenElse);
    EVAL_DECL(IfErrThenElse);
    INSERT_DECL(IfErrThenElse);
};


// The stack-based forms
COMMAND_DECLARE(IFT);
COMMAND_DECLARE(IFTE);

// Saved error message
COMMAND_DECLARE(errm);
COMMAND_DECLARE(errn);
COMMAND_DECLARE(err0);
COMMAND_DECLARE(doerr);


#endif // CONDITIONALS_H
