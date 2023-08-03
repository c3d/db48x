// ****************************************************************************
//  conditionals.cc                                               DB48X project
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

#include "conditionals.h"

#include "integer.h"
#include "renderer.h"
#include "settings.h"
#include "user_interface.h"



// ============================================================================
//
//    If-Then
//
// ============================================================================

PARSE_BODY(IfThen)
// ----------------------------------------------------------------------------
//   Leverage the conditional loop parsing
// ----------------------------------------------------------------------------
{
    return loop::object_parser(p, "if", "then",
                               "end",  ID_IfThen,
                               "else", ID_IfThenElse,
                               "end",
                               false);
}


RENDER_BODY(IfThen)
// ----------------------------------------------------------------------------
//   Render if-then
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "if", "then", "end");
}


EVAL_BODY(IfThen)
// ----------------------------------------------------------------------------
//   Evaluate if-then
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_g body = cond->skip();
    result   r    = OK;

    // Evaluate the condition
    r = cond->evaluate();
    if (r != OK)
        return r;

    // Check if we should evaluate the body
    bool test = false;
    r = o->condition(test);
    if (r != OK || !test)
        return r;

    // Evaluate the body if needed
    r = body->evaluate();
    return r;
}


INSERT_BODY(IfThen)
// ----------------------------------------------------------------------------
//    Insert 'if-then' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("if  then  end"), ui.PROGRAM, 3);
}



// ============================================================================
//
//    If-Then-Else
//
// ============================================================================

SIZE_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//   Compute the size of an if-then-else
// ----------------------------------------------------------------------------
{
    object_p p = object_p(o->payload());
    p = p->skip()->skip()->skip();
    return ptrdiff(p, o);
}

PARSE_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//   Done by the 'if-then' case.
// ----------------------------------------------------------------------------
{
    return SKIP;
}


RENDER_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//   Render if-then-else
// ----------------------------------------------------------------------------
{
    // Source objects
    byte_p   p      = payload(o);

    // Isolate condition, true and false part
    object_g cond   = object_p(p);
    object_g ift    = cond->skip();
    object_g iff    = ift->skip();
    auto     format = Settings.command_fmt;

    // Write the header
    r.put('\n');
    r.put(format, utf8(o->type() == ID_IfErrThenElse ? "iferr" : "if"));

    // Render condition
    r.indent();
    cond->render(r);
    r.unindent();

    // Render 'if-true' part
    r.put(format, utf8("then"));
    r.indent();
    ift->render(r);
    r.unindent();

    // Render 'if-false' part
    r.put(format, utf8("else"));
    r.indent();
    iff->render(r);
    r.unindent();

    // Render the 'end'
    r.put(format, utf8("end"));

    return r.size();
}


EVAL_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//   Evaluate if-then-else
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_g ift  = cond->skip();
    object_g iff  = ift->skip();
    result   r    = OK;

    // Evaluate the condition
    r = cond->evaluate();
    if (r != OK)
        return r;

    // Check if we should evaluate the body
    bool test = false;
    r = o->condition(test);
    if (r != OK)
        return r;

    // Evaluate the body if needed
    if (test)
        r = ift->evaluate();
    else
        r = iff->evaluate();
    return r;
}


INSERT_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//    Insert 'if-then-else' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("if  then  else  end"), ui.PROGRAM, 3);
}



// ============================================================================
//
//    IfErr-Then
//
// ============================================================================

PARSE_BODY(IfErrThen)
// ----------------------------------------------------------------------------
//   Leverage the conditional loop parsing
// ----------------------------------------------------------------------------
{
    return loop::object_parser(p, "iferr", "then",
                               "end", ID_IfErrThen,
                               "else", ID_IfErrThenElse,
                               "end",
                               false);
}


RENDER_BODY(IfErrThen)
// ----------------------------------------------------------------------------
//   Render iferr-then
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "iferr", "then", "end");
}


EVAL_BODY(IfErrThen)
// ----------------------------------------------------------------------------
//   Evaluate iferr-then
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_g body = cond->skip();
    result   r    = OK;

    // Evaluate the condition
    r = cond->evaluate();
    if (r != OK || rt.error())
    {
        rt.clear_error();
        r = body->evaluate();
    }

    return r;
}


INSERT_BODY(IfErrThen)
// ----------------------------------------------------------------------------
//    Insert 'iferr-then' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("iferr  then  end"), ui.PROGRAM, 6);
}



// ============================================================================
//
//    IfErr-Then-Else
//
// ============================================================================

PARSE_BODY(IfErrThenElse)
// ----------------------------------------------------------------------------
//   Done by the 'iferr-then' case.
// ----------------------------------------------------------------------------
{
    return SKIP;
}


EVAL_BODY(IfErrThenElse)
// ----------------------------------------------------------------------------
//   Evaluate iferr-then-else
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_g ift  = cond->skip();
    object_g iff  = ift->skip();
    result   r    = OK;

    // Evaluate the condition
    r = cond->evaluate();
    if (r != OK || rt.error())
    {
        rt.clear_error();
        r = ift->evaluate();
    }
    else
    {
        r = iff->evaluate();
    }
    return r;
}


INSERT_BODY(IfErrThenElse)
// ----------------------------------------------------------------------------
//    Insert 'iferr-then-else' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("iferr  then  else  end"), ui.PROGRAM, 6);
}



// ============================================================================
//
//   IFT and IFTE commands
//
// ============================================================================

COMMAND_BODY(IFT)
// ----------------------------------------------------------------------------
//   Evaluate the 'IFT' command
// ----------------------------------------------------------------------------
{
    if (object_p toexec = rt.pop())
    {
        if (object_p condition = rt.pop())
        {
            int truth = condition->as_truth(true);
            if (truth == true)
                return toexec->execute();
            else if (truth == false)
                return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(IFTE)
// ----------------------------------------------------------------------------
//   Evaluate the 'IFT' command
// ----------------------------------------------------------------------------
{
    if (object_p iff = rt.pop())
    {
        if (object_p ift = rt.pop())
        {
            if (object_p condition = rt.pop())
            {
                int truth = condition->as_truth(true);
                if (truth == true)
                    return ift->execute();
                else if (truth == false)
                    return iff->execute();
            }
        }
    }
    return ERROR;
}



// ============================================================================
//
//   Error messages
//
// ============================================================================

COMMAND_BODY(errm)
// ----------------------------------------------------------------------------
//   Return the current error message
// ----------------------------------------------------------------------------
{
    if (utf8 msg = rt.error_message())
    {
        if (rt.push(text::make(msg)))
            return OK;
    }
    else
    {
        if (rt.push(text::make(utf8(""), 0)))
            return OK;
    }
    return ERROR;
}


static cstring messages[] =
// ----------------------------------------------------------------------------
//   List of built-in error messages
// ----------------------------------------------------------------------------
{
#define ERROR(name, msg)        msg,
#include "errors.tbl"
};


COMMAND_BODY(errn)
// ----------------------------------------------------------------------------
//   Return the current error message
// ----------------------------------------------------------------------------
{
    uint result = 0;
    utf8 error  = rt.error_message();

    if (error)
    {
        result = 0x70000;       // Value returned by HP48 for user errors
        for (uint i = 0; i < sizeof(messages) / sizeof(*messages); i++)
        {
            if (!strcmp(messages[i], cstring(error)))
            {
                result = i + 1;
                break;
            }
        }
    }
    if (rt.push(rt.make<based_integer>(result)))
        return OK;
    return ERROR;
}


COMMAND_BODY(err0)
// ----------------------------------------------------------------------------
//   Clear the error message
// ----------------------------------------------------------------------------
{
    rt.error(utf8(nullptr));          // Not clear_error, need to zero ErrorSave
    return OK;
}


COMMAND_BODY(doerr)
// ----------------------------------------------------------------------------
//   Generate an error message for the user
// ----------------------------------------------------------------------------
{
    rt.command(fancy(ID_doerr));
    rt.source(utf8(nullptr));
    if (object_p obj = rt.pop())
    {
        if (text_p tval = obj->as<text>())
        {
            // Need to null-terminate the text
            size_t size = 0;
            utf8   str  = tval->value(&size);
            text_g zt = text::make(str, size + 1);
            byte * payload = (byte *) zt->value();
            payload[size] = 0;
            rt.error(utf8(payload));
        }
        else
        {
            uint32_t ival = obj->as_uint32();
            if (ival || !rt.error())
            {
                if (!ival)
                    rt.interrupted_error();
                else if (ival - 1 < sizeof(messages) / sizeof(*messages))
                    rt.error(messages[ival-1]);
                else
                    rt.domain_error();
            }
        }
    }
    return ERROR;
}
