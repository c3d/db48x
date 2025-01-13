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

#include "expression.h"
#include "integer.h"
#include "parser.h"
#include "program.h"
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
    object_p body = cond->skip();
    if (rt.run_conditionals(body, nullptr)      &&
        defer(ID_conditional)                   &&
        program::run_program(cond) == OK)
        return OK;
    return ERROR;
}


INSERT_BODY(IfThen)
// ----------------------------------------------------------------------------
//    Insert 'if-then' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("if \t then  end"), ui.PROGRAM);
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
    byte_p   p      = o->payload();

    // Isolate condition, true and false part
    object_g cond   = object_p(p);
    object_g ift    = cond->skip();
    object_g iff    = ift->skip();
    auto     format = Settings.CommandDisplayMode();

    // Write the header
    r.wantCR();
    r.put(format, utf8(o->type() == ID_IfErrThenElse ? "iferr" : "if"));
    r.wantCR();

    // Render condition
    r.indent();
    cond->render(r);
    r.unindent();

    // Render 'if-true' part
    r.wantCR();
    r.put(format, utf8("then"));
    r.wantCR();
    r.indent();
    ift->render(r);
    r.unindent();

    // Render 'if-false' part
    r.wantCR();
    r.put(format, utf8("else"));
    r.wantCR();
    r.indent();
    iff->render(r);
    r.unindent();

    // Render the 'end'
    r.wantCR();
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

    if (rt.run_conditionals(ift, iff)   &&
        defer(ID_conditional)           &&
        program::run_program(cond) == OK)
        return OK;
    return ERROR;
}


INSERT_BODY(IfThenElse)
// ----------------------------------------------------------------------------
//    Insert 'if-then-else' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("if \t then  else  end"), ui.PROGRAM);
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
    settings::SaveDebugOnError sdoe(false);

    // Evaluate the condition
    r = program::run(+cond);
    if (r != OK || rt.error())
    {
        rt.clear_error();
        r = program::run(+body);
    }

    return r;
}


INSERT_BODY(IfErrThen)
// ----------------------------------------------------------------------------
//    Insert 'iferr-then' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("iferr \t then  end"), ui.PROGRAM);
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
    byte     *p    = (byte *) o->payload();
    object_g  cond = object_p(p);
    object_g  ift  = cond->skip();
    object_g  iff  = ift->skip();
    result    r    = OK;
    settings::SaveDebugOnError sdoe(false);

    // Evaluate the condition
    r = program::run(+cond);
    if (r != OK || rt.error())
    {
        rt.clear_error();
        r = program::run(+ift);
    }
    else
    {
        r = program::run(+iff);
    }
    return r;
}


INSERT_BODY(IfErrThenElse)
// ----------------------------------------------------------------------------
//    Insert 'iferr-then-else' command in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("iferr \t then  else  end"), ui.PROGRAM);
}



// ============================================================================
//
//   Case statement
//
// ============================================================================

static inline bool match(cstring s, cstring sep, size_t len, size_t remaining)
// ----------------------------------------------------------------------------
//   Check if we match a given input
// ----------------------------------------------------------------------------
{
    return (len <= remaining &&
            strncasecmp(s, sep, len) == 0 &&
            (len >= remaining || is_separator(utf8(s) + len)));
}


PARSE_BODY(CaseStatement)
// ----------------------------------------------------------------------------
//   Leverage the conditional loop parsing to process a case statement
// ----------------------------------------------------------------------------
{
    // We have to be careful that we may have to GC to make room for loop
    gcutf8   src      = p.source;
    size_t   max      = p.length;
    object_g obj1     = nullptr;
    object_g obj2     = nullptr;
    bool     had_then = false;
    bool     had_when = false;
    bool     had_end  = false;

    // Quick exit if we are not parsing a "case"
    if (!match(cstring(+src), "case", 4, max))
        return SKIP;
    src += size_t(4);

    // Outer scribble collects the various conditions
    scribble outer_scr;

    // Loop over the two or three separators we got
    while (!had_end)
    {
        while (!had_end)
        {
            if (!utf8_more(p.source, src, max))
            {
                rt.unterminated_error().source(src);
                return ERROR;
            }

            // Inner scribble collects the various code blocks
            scribble scr;

            // Scan the body of the loop
            while (utf8_more(p.source, src, max))
            {
                // Skip spaces
                unicode cp = utf8_codepoint(src);
                if (utf8_whitespace(cp))
                {
                    src = utf8_next(src);
                    continue;
                }

                // Check if we have 'end'
                size_t  remaining = max - size_t(utf8(src) - utf8(p.source));
                cstring s         = cstring(+src);
                if (match(s, "end", 3, remaining))
                {
                    src += size_t(3);
                    had_end = true;
                    break;
                }

                // Check if we have "then" or "when"
                if (!had_then)
                {
                    had_then = match(s, "then", 4, remaining);
                    if (had_then)
                    {
                        src += size_t(4);
                        break;
                    }
                }
                if (!had_when)
                {
                    had_when = match(s, "when", 4, remaining);
                    if (had_when)
                    {
                        src += size_t(4);
                        break;
                    }
                }

                // Parse an object
                size_t   done   = utf8(src) - utf8(p.source);
                size_t   length = max > done ? max - done : 0;
                object_g obj    = object::parse(src, length);
                if (!obj)
                    return ERROR;

                // Copy the parsed object to the scratch pad (may GC)
                if (!rt.append(obj))
                    return ERROR;

                // Jump past what we parsed
                src = utf8(src) + length;
            }

            // Create the program object for condition or body
            gcbytes  scratch = scr.scratch();
            size_t   alloc   = scr.growth();
            object_p prog    = rt.make<program>(ID_block, scratch, alloc);
            if (!had_end)
            {
                obj1 = prog;
            }
            else if (had_then || had_when)
            {
                id type = had_when ? ID_CaseWhen : ID_CaseThen;
                obj2 = prog;
                obj1 = rt.make<CaseThen>(type, obj1, obj2);
                had_then = had_when = false;
                had_end = false;
                break;
            }
            else
            {
                obj1 = prog;
                break;
            }
        } // Loop on conditions and blocks

        // Here, either had_end, and obj1 is the tail block, or adding a cond
        if (!had_end)
        {
            // Copy the parsed object to the scratch pad (may GC)
            if (!rt.append(obj1))
                return ERROR;
            obj1 = nullptr;
        }
    }

    size_t parsed = utf8(src) - utf8(p.source);
    if (!had_end)
    {
        // If we did not find the terminator, we reached end of text1
        rt.unterminated_error().source(p.source, parsed);
        return ERROR;
    }

    // Create the program object for the conditions
    if (!obj1)
        obj1 = rt.make<program>(ID_block, nullptr, 0);
    gcbytes  scratch = outer_scr.scratch();
    size_t   alloc   = outer_scr.growth();
    object_p prog    = rt.make<program>(ID_block, scratch, alloc);
    object_p cases   = rt.make<CaseStatement>(prog, obj1);
    p.length         = parsed;
    p.out            = cases;

    return OK;
}


static size_t render_case(renderer &r, cstring first, object_p o)
// ----------------------------------------------------------------------------
//   Render a then-end or when-end instruction
// ----------------------------------------------------------------------------
{
    // Source objects
    byte_p   p      = o->payload();
    object_g cond   = object_p(p);
    object_g body   = cond->skip();
    auto     format = Settings.CommandDisplayMode();

    cond->render(r);
    r.wantCR();
    r.put(format, utf8(first));
    r.wantCR();
    r.indent();
    body->render(r);
    r.unindent();
    r.wantCR();
    r.put(format, utf8("end"));
    r.wantCR();
    return r.size();
}


RENDER_BODY(CaseStatement)
// ----------------------------------------------------------------------------
//   Render case statement
// ----------------------------------------------------------------------------
{
    // Source objects
    byte_p   p      = o->payload();
    object_g conds  = object_p(p);
    object_g rest   = conds->skip();
    auto     format = Settings.CommandDisplayMode();

    r.wantCR();
    r.put(format, utf8("case"));
    r.wantCR();
    r.indent();
    conds->render(r);
    if (block_p block = block_p(+rest))
        if (block->length())
            block->render(r);
    r.unindent();
    r.wantCR();
    r.put(format, utf8("end"));
    r.wantCR();
    return r.size();

}


EVAL_BODY(CaseStatement)
// ----------------------------------------------------------------------------
//   Evaluate case statements
// ----------------------------------------------------------------------------
{
    byte    *p     = (byte *) o->payload();
    object_g conds = object_p(p);
    object_p rest  = conds->skip();
    if (defer(ID_case_end_conditional) && rest->defer() && conds->defer())
        return OK;
    return ERROR;
}


INSERT_BODY(CaseStatement)
// ----------------------------------------------------------------------------
//    Insert case statement in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("case \t end"), ui.PROGRAM);
}


RENDER_BODY(case_end_conditional)
// ----------------------------------------------------------------------------
//   A non-parseable object used to mark the end of the current 'case' stmt
// ----------------------------------------------------------------------------
{
    r.put("<case-end>");
    return r.size();
};


EVAL_BODY(case_end_conditional)
// ----------------------------------------------------------------------------
//   Reaching the end of a case statement
// ----------------------------------------------------------------------------
{
    return OK;
}


RENDER_BODY(case_skip_conditional)
// ----------------------------------------------------------------------------
//   A non-parseable object used to skip to the end of a case statement
// ----------------------------------------------------------------------------
{
    r.put("<case-skip>");
    return r.size();
};


EVAL_BODY(case_skip_conditional)
// ----------------------------------------------------------------------------
//   Skip to the end of a case statement
// ----------------------------------------------------------------------------
{
    while (object_p next = rt.run_next(0))
        if (next->type() == ID_case_end_conditional)
            break;
    return OK;
}


RENDER_BODY(CaseThen)
// ----------------------------------------------------------------------------
//   Render case statement
// ----------------------------------------------------------------------------
{
    return render_case(r, "then", o);
}


EVAL_BODY(CaseThen)
// ----------------------------------------------------------------------------
//   Evaluate case statements
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_p body = cond->skip();
    if (rt.run_conditionals(body, nullptr) &&
        defer(ID_case_then_conditional) &&
        cond->defer())
        return OK;
    return ERROR;
}


INSERT_BODY(CaseThen)
// ----------------------------------------------------------------------------
//    Insert case statement in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("then \t end"), ui.PROGRAM);
}


RENDER_BODY(case_then_conditional)
// ----------------------------------------------------------------------------
//   A non-parseable object used to test the 'then' in a case statement
// ----------------------------------------------------------------------------
{
    r.put("<case-then>");
    return r.size();
};


EVAL_BODY(case_then_conditional)
// ----------------------------------------------------------------------------
//   Check a condition in a 'case' statement. If successful, exit case
// ----------------------------------------------------------------------------
{
    return loop::evaluate_condition(ID_case_then_conditional,
                                    &runtime::run_select_case);
}


RENDER_BODY(CaseWhen)
// ----------------------------------------------------------------------------
//   Render case statement
// ----------------------------------------------------------------------------
{
    return render_case(r, "when", o);
}


EVAL_BODY(CaseWhen)
// ----------------------------------------------------------------------------
//   Evaluate case statements
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_p body = cond->skip();
    if (rt.run_conditionals(body, nullptr) &&
        defer(ID_case_when_conditional) &&
        cond->defer())
        return OK;
    return ERROR;
}


INSERT_BODY(CaseWhen)
// ----------------------------------------------------------------------------
//    Insert case statement in the editor
// ----------------------------------------------------------------------------
{
    return ui.insert(utf8("when \t end"), ui.PROGRAM);
}


RENDER_BODY(case_when_conditional)
// ----------------------------------------------------------------------------
//   A non-parseable object used to test the 'when' in a case statement
// ----------------------------------------------------------------------------
{
    r.put("<case-when>");
    return r.size();
};


EVAL_BODY(case_when_conditional)
// ----------------------------------------------------------------------------
//   Check a condition in a 'case' statement. If successful, exit case
// ----------------------------------------------------------------------------
{
    if (object_p value = rt.pop())
        if (object_p ref = rt.top())
            if (rt.run_select(value->is_same_as(ref)))
                return OK;
    return ERROR;
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
    if (object_p ift = rt.pop())
    {
        if (object_g condition = rt.pop())
        {
            if (expression_p expr = condition->as<expression>())
                condition = expr->evaluate();
            int cvalue = condition->as_truth(true);
            if (cvalue >= 0)
                return cvalue ? program::run(ift) : OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(IFTE)
// ----------------------------------------------------------------------------
//   Evaluate the 'IFTE' command
// ----------------------------------------------------------------------------
{
    if (object_p iff = rt.pop())
    {
        if (object_p ift = rt.pop())
        {
            if (object_g condition = rt.pop())
            {
                if (expression_p expr = condition->as<expression>())
                    condition = expr->evaluate();
                int cvalue = condition->as_truth(true);
                if (cvalue >= 0)
                    return program::run(cvalue ? ift : iff);
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


static const cstring messages[] =
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
            uint32_t ival = obj->as_uint32(0, true);
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
