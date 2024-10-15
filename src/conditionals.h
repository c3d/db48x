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

#include "command.h"
#include "loops.h"


struct IfThen : conditional_loop
// ----------------------------------------------------------------------------
//   The 'if-then' command behaves mostly like a conditional loop
// ----------------------------------------------------------------------------
{
    IfThen(id type, object_g condition, object_g body)
        : conditional_loop(type, condition, body) { }

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
    IfThenElse(id type, object_g cond, object_g ift, object_g iff)
        : IfThen(type, cond, ift)
    {
        // Copy the additional object
        // Do NOT use payload(this) here:
        // ID_IfThenElse is 1 byte, ID_IfErrThenElse is 2 bytes
        object_p p = object_p(payload());
        object_p after = p->skip()->skip();
        byte *tgt = (byte *) after;
        size_t iffs = iff->size();
        memcpy(tgt, +iff, iffs);
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
    IfErrThen(id type, object_g condition, object_g body)
        : IfThen(type, condition, body) { }

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
    IfErrThenElse(id type, object_g cond, object_g ift, object_g iff)
        : IfThenElse(type, cond, ift, iff)
    { }

    OBJECT_DECL(IfErrThenElse);
    PARSE_DECL(IfErrThenElse);
    EVAL_DECL(IfErrThenElse);
    INSERT_DECL(IfErrThenElse);
};


struct CaseStatement : conditional_loop
// ----------------------------------------------------------------------------
//   CASE conditional statement
// ----------------------------------------------------------------------------
{
    CaseStatement(id type, object_g conditions, object_g rest)
        : conditional_loop(type, conditions, rest) {}

    OBJECT_DECL(CaseStatement);
    PARSE_DECL(CaseStatement);
    RENDER_DECL(CaseStatement);
    EVAL_DECL(CaseStatement);
    INSERT_DECL(CaseStatement);
};


struct CaseThen : conditional_loop
// ----------------------------------------------------------------------------
//   CASE conditional statement
// ----------------------------------------------------------------------------
{
    CaseThen(id type, object_g condition, object_g body)
        : conditional_loop(type, condition, body) {}

    OBJECT_DECL(CaseThen);
    RENDER_DECL(CaseThen);
    EVAL_DECL(CaseThen);
    INSERT_DECL(CaseThen);
};


struct CaseWhen : conditional_loop
// ----------------------------------------------------------------------------
//   CASE conditional statement
// ----------------------------------------------------------------------------
{
    CaseWhen(id type, object_g value, object_g body)
        : conditional_loop(type, value, body) {}

    OBJECT_DECL(CaseWhen);
    RENDER_DECL(CaseWhen);
    EVAL_DECL(CaseWhen);
    INSERT_DECL(CaseWhen);
};


struct case_then_conditional : conditional
// ----------------------------------------------------------------------------
//   A non-parseable object used to mark the 'then' in a 'case' statement
// ----------------------------------------------------------------------------
{
    case_then_conditional(id type): conditional(type) {}
    OBJECT_DECL(case_then_conditional);
    RENDER_DECL(case_then_conditional);
    EVAL_DECL(case_then_conditional);
};


struct case_when_conditional : conditional
// ----------------------------------------------------------------------------
//   A non-parseable object used to mark case-when statements
// ----------------------------------------------------------------------------
{
    case_when_conditional(id type): conditional(type) {}
    OBJECT_DECL(case_when_conditional);
    RENDER_DECL(case_when_conditional);
    EVAL_DECL(case_when_conditional);
};


struct case_end_conditional : conditional
// ----------------------------------------------------------------------------
//   A non-parseable object used to mark the end of a 'case' statement
// ----------------------------------------------------------------------------
{
    case_end_conditional(id type): conditional(type) {}
    OBJECT_DECL(case_end_conditional);
    RENDER_DECL(case_end_conditional);
    EVAL_DECL(case_end_conditional);
};


struct case_skip_conditional : conditional
// ----------------------------------------------------------------------------
//   A non-parseable object used to skip to the end of a case statement
// ----------------------------------------------------------------------------
{
    case_skip_conditional(id type): conditional(type) {}
    OBJECT_DECL(case_skip_conditional);
    RENDER_DECL(case_skip_conditional);
    EVAL_DECL(case_skip_conditional);
};


// The stack-based forms
COMMAND_DECLARE(IFT,2);
COMMAND_DECLARE_SPECIAL(IFTE,command,3,
                        static bool can_be_symbolic(uint a) { return a!=2; } );

// Saved error message
COMMAND_DECLARE(errm,0);
COMMAND_DECLARE(errn,0);
COMMAND_DECLARE(err0,0);
COMMAND_DECLARE(doerr,1);


#endif // CONDITIONALS_H
