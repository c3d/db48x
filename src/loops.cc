// ****************************************************************************
//  loops.cc                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of basic loops
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

#include "loops.h"

#include "command.h"
#include "compare.h"
#include "conditionals.h"
#include "decimal.h"
#include "integer.h"
#include "locals.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "types.h"
#include "user_interface.h"
#include "utf8.h"

#include <stdio.h>
#include <string.h>


RECORDER(loop, 16, "Loops");
RECORDER(loop_error, 16, "Errors processing loops");

// The payload(o) optimization won't work otherwise
COMPILE_TIME_ASSERT((object::ID_DoUntil < 128) == (object::ID_ForStep < 128));


SIZE_BODY(loop)
// ----------------------------------------------------------------------------
//   Compute size for a loop
// ----------------------------------------------------------------------------
{
    object_p p = object_p(o->payload());
    p = p->skip();
    return ptrdiff(p, o);
}


loop::loop(id type, object_g body, symbol_g name)
// ----------------------------------------------------------------------------
//   Constructor for loops
// ----------------------------------------------------------------------------
    : command(type)
{
    byte *p = (byte *) payload();
    if (name)
    {
        // Named loop like For Next: copy symbol, replace ID with 1 (# locals)
        size_t nsize = object_p(symbol_p(name))->size();
        memmove(p, symbol_p(name), nsize);
        p[0] = 1;
        p += nsize;
    }
    size_t bsize = body->size();
    memmove(p, byte_p(body), bsize);
}


object::result loop::evaluate_condition(id type, bool (runtime::*method)(bool))
// ----------------------------------------------------------------------------
//   Evaluate the stack condition and call runtime method
// ----------------------------------------------------------------------------
{
    if (object_p cond = rt.pop())
    {
        // Evaluate expressions in conditionals
        if (cond->is_program())
        {
            // This defers the execution of that conditional
            if (program::defer(type) &&
                program::defer(ID_need_conditional) &&
                program::run_program(cond) == OK)
                return OK;
        }
        else
        {
            int truth = cond->as_truth(true);
            if (truth >= 0)
                if ((rt.*method)(truth))
                    return OK;
        }
    }
    return ERROR;
}


SIZE_BODY(conditional_loop)
// ----------------------------------------------------------------------------
//   Compute size for a conditional loop
// ----------------------------------------------------------------------------
{
    object_p p = object_p(o->payload());
    p = p->skip()->skip();
    return ptrdiff(p, o);
}


conditional_loop::conditional_loop(id type, object_g first, object_g second)
// ----------------------------------------------------------------------------
//   Constructor for conditional loops
// ----------------------------------------------------------------------------
    : loop(type, first, nullptr)
{
    size_t fsize = first->size();
    byte *p = (byte *) payload() + fsize;
    size_t ssize = second->size();
    memmove(p, byte_p(second), ssize);
}


object::result loop::object_parser(parser  &p,
                                   cstring  open,
                                   cstring  middle,
                                   cstring  close2, id id2,
                                   cstring  close1, id id1,
                                   cstring  terminator,
                                   bool     loopvar)
// ----------------------------------------------------------------------------
//   Generic parser for loops
// ----------------------------------------------------------------------------
//   Like for programs, we have to be careful here, because parsing sub-objects
//   may allocate new temporaries, which itself may cause garbage collection.
{
    // We have to be careful that we may have to GC to make room for loop
    gcutf8   src  = p.source;
    size_t   max  = p.length;
    object_g obj1 = nullptr;
    object_g obj2 = nullptr;
    object_g obj3 = nullptr;    // Case of 'else'
    symbol_g name = nullptr;
    id       type = id1;

    // Loop over the two or three separators we got
    while (open || middle || close1 || close2 || terminator)
    {
        cstring  sep   = open   ? open
                       : middle ? middle
                       : close1 ? close1
                       : close2 ? close2
                                : terminator;
        size_t   len   = strlen(sep);
        bool     found = sep == nullptr;
        scribble scr;

        // Scan the body of the loop
        while (!found && utf8_more(p.source, src, max))
        {
            // Skip spaces
            unicode cp = utf8_codepoint(src);
            if (utf8_whitespace(cp))
            {
                src = utf8_next(src);
                continue;
            }

            // Check if we have the separator
            size_t   done   = utf8(src) - utf8(p.source);
            size_t   length = max > done ? max - done : 0;
            if (len <= length
                && strncasecmp(cstring(utf8(src)), sep, len) == 0
                && (len >= length ||
                    is_separator(utf8(src) + len)))
            {
                if (loopvar && sep != open)
                {
                    rt.missing_variable_error().source(src);
                    return ERROR;
                }
                src += len;
                found = true;
                continue;
            }

            // If we get there looking for the opening separator, mismatch
            if (sep == open)
                return SKIP;

            // Check if we have the alternate form ('step' vs. 'next')
            if (sep == close1 && close2)
            {
                size_t len2 = strlen(close2);
                if (len2 <= length
                    && strncasecmp(cstring(utf8(src)), close2, len2) == 0
                    && (len2 >= length ||
                        is_separator(utf8(src) + len2)))
                {
                    if (loopvar && sep != open)
                    {
                        rt.missing_variable_error().source(src);
                        return ERROR;
                    }
                    src += len;
                    found = true;
                    type = id2;
                    terminator = nullptr;
                    continue;
                }
            }

            // Parse an object
            done   = utf8(src) - utf8(p.source);
            length = max > done ? max - done : 0;
            object_g obj    = object::parse(src, length);
            if (!obj)
                return ERROR;

            // Copy the parsed object to the scratch pad (may GC)
            byte *objcopy = rt.append(obj);
            if (!objcopy)
                return ERROR;

            // Check if we have a loop variable name
            if (loopvar && sep != open)
            {
                if (obj->type() != ID_symbol)
                {
                    rt.missing_variable_error().source(src);
                    return ERROR;
                }

                // Here, we create a locals stack that has:
                // - 1 (number of names)
                // - Length of name
                // - Name characters
                // That's the same structure as the symbol, except that
                // we replace the type ID from ID_symbol to number of names 1
                objcopy[0] = 1;
                loopvar = false;

                // This is now the local names for the following block
                locals_stack *stack = locals_stack::current();
                stack->names(byte_p(objcopy));

                // Remember that to create the ForNext object
                name = symbol_p(object_p(obj));
            }

            // Jump past what we parsed
            src = utf8(src) + length;
        }

        if (!found)
        {
            // If we did not find the terminator, we reached end of text
            rt.unterminated_error().source(p.source);
            return ERROR;
        }
        else if (sep == open)
        {
            // We just matched the first word, no object created here
            open = nullptr;
            continue;
        }

        // Create the program object for condition or body
        size_t   namesz  = name ? object_p(symbol_p(name))->size() : 0;
        gcbytes  scratch = scr.scratch() + namesz;
        size_t   alloc   = scr.growth() - namesz;
        object_p prog    = rt.make<program>(ID_block, scratch, alloc);
        if (sep == middle)
        {
            obj1 = prog;
            middle = nullptr;
        }
        else if (sep == close1 || sep == close2)
        {
            obj2 = prog;
            close1 = close2 = nullptr;
        }
        else
        {
            obj3 = prog;
            terminator = nullptr;
        }
    }

    size_t parsed = utf8(src) - utf8(p.source);
    p.length      = parsed;
    p.out         =
          name
        ? rt.make<ForNext>(type, obj2, name)
        : obj3
        ? rt.make<IfThenElse>(type, obj1, obj2, obj3)
        : obj1
        ? rt.make<conditional_loop>(type, obj1, obj2)
        : rt.make<loop>(type, obj2, nullptr);

    return OK;
}


intptr_t loop::object_renderer(renderer &r,
                               cstring   open,
                               cstring   middle,
                               cstring   close,
                               bool      loopvar) const
// ----------------------------------------------------------------------------
//   Render the loop into the given buffer
// ----------------------------------------------------------------------------
{
    // Source objects
    byte_p   p      = payload();

    // Find name
    gcbytes  name   = nullptr;
    size_t   namesz = 0;
    if (loopvar)
    {
        if (p[0] != 1)
            record(loop_error, "Got %d variables instead of 1", p[0]);
        p++;
        namesz = leb128<size_t>(p);
        name = p;
        p += namesz;
    }

    // Isolate condition and body
    object_g first  = object_p(p);
    object_g second = middle ? first->skip() : nullptr;
    auto     format = Settings.CommandDisplayMode();

    // Write the header, e.g. "DO"
    r.wantCR();
    r.put(format, utf8(open));

    // Render name if any
    if (name)
    {
        r.wantSpace();
        r.put(name, namesz);
    }

    // Ident condition or first body
    r.indent();
    r.wantCR();

    // Emit the first object (e.g. condition in do-until)
    first->render(r);
    r.wantCR();

    // Emit the second object if there is one
    if (middle)
    {
        // Emit separator after condition
        r.unindent();
        r.wantCR();
        r.put(format, utf8(middle));
        r.indent();
        r.wantCR();
        second->render(r);
        r.wantCR();
    }

    // Emit closing separator
    r.unindent();
    r.wantCR();
    r.put(format, utf8(close));
    r.wantCR();

    return r.size();
}



// ============================================================================
//
//   DO...UNTIL...END loop
//
// ============================================================================

PARSE_BODY(DoUntil)
// ----------------------------------------------------------------------------
//  Parser for do-unti loops
// ----------------------------------------------------------------------------
{
    return loop::object_parser(p, "do", "until",
                               "end", ID_DoUntil,
                               nullptr, ID_DoUntil,
                               false);
}


RENDER_BODY(DoUntil)
// ----------------------------------------------------------------------------
//   Renderer for do-until loop
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "do", "until", "end");
}


INSERT_BODY(DoUntil)
// ----------------------------------------------------------------------------
//   Insert a do-until loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("do \t until  end"), ui.PROGRAM);
}


EVAL_BODY(DoUntil)
// ----------------------------------------------------------------------------
//   Evaluate a do..until..end loop
// ----------------------------------------------------------------------------
//   In this loop, the body comes first
//   We evaluate it by pushing the following on the call stack:
//   - The loop object (which may be used for repetition)
//   - A 'conditional' object
//   - The condition (which will be evaluated after the body)
//   - The body (which will evaluate next)
//
// the condition, then the body for evaluation,
//   which causes the body to be ev
{
    byte    *p    = (byte *) payload(o);
    object_g body = object_p(p);
    object_g cond = body->skip();

    rt.command(o);

    // We loop until the condition becomes true
    if (rt.run_conditionals(nullptr, o) &&
        defer(ID_conditional)           &&
        cond->defer()                   &&
        body->defer())
        return OK;

    return ERROR;
}


// ============================================================================
//
//   WHILE...REPEAT...END loop
//
// ============================================================================

PARSE_BODY(WhileRepeat)
// ----------------------------------------------------------------------------
//  Parser for while loops
// ----------------------------------------------------------------------------
{
    return loop::object_parser(p, "while", "repeat",
                               "end", ID_WhileRepeat,
                               nullptr, ID_WhileRepeat,
                               false);
}


RENDER_BODY(WhileRepeat)
// ----------------------------------------------------------------------------
//   Renderer for while loop
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "while", "repeat", "end");
}


INSERT_BODY(WhileRepeat)
// ----------------------------------------------------------------------------
//   Insert a while loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("while \t repeat  end"), ui.PROGRAM);
}


EVAL_BODY(WhileRepeat)
// ----------------------------------------------------------------------------
//   Evaluate a while..repeat..end loop
// ----------------------------------------------------------------------------
//   In this loop, the condition comes first
{
    byte    *p    = (byte *) payload(o);
    object_g cond = object_p(p);
    object_g body = cond->skip();

    rt.command(o);

    // We loop while the condition is true
    if (rt.run_conditionals(o, body)            &&
        defer(ID_while_conditional)             &&
        cond->defer())
        return OK;

    return ERROR;
}



// ============================================================================
//
//   START...NEXT loop
//
// ============================================================================

PARSE_BODY(StartNext)
// ----------------------------------------------------------------------------
//  Parser for start-next loops
// ----------------------------------------------------------------------------
{
    return loop::object_parser(p,
                               "start", nullptr,
                               "next", ID_StartNext,
                               "step", ID_StartStep,
                               false);
}


RENDER_BODY(StartNext)
// ----------------------------------------------------------------------------
//   Renderer for start-next loop
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "start", nullptr, "next");
}


INSERT_BODY(StartNext)
// ----------------------------------------------------------------------------
//   Insert a start-next loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("start \t next"), ui.PROGRAM);
}


static object::result counted_loop(object::id type, object_p o)
// ----------------------------------------------------------------------------
//   Place the correct loop on the run stack
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) object::payload(o);

    // Fetch loop initial and last steps
    object_g last  = rt.pop();
    object_g first = rt.pop();

    // Check if we need a local variable
    if (type >= object::ID_for_next_conditional)
    {
        // For debugging or conversion to text, ensure we track names
        locals_stack stack(p);

        // Skip name
        if (p[0] != 1)
            record(loop_error, "Evaluating for-next loop with %u locals", p[0]);
        p += 1;
        size_t namesz = leb128<size_t>(p);
        p += namesz;

        // Get start value as local
        if (!rt.push(first))
            return object::ERROR;
        rt.locals(1);

        // Pop local after execution
        if (!rt.run_push_data(nullptr, object_p(1)))
            return object::ERROR;
    }

    object_g body = object_p(p);
    if (body->defer() && rt.run_push_data(first, last) &&
        object::defer(type) && body->defer())
        return object::OK;

    return object::ERROR;
}


static object::result list_loop(object::id type, object_p o)
// ----------------------------------------------------------------------------
//   Place the correct loop on the run stack
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) object::payload(o);

    list_g range = list_p(rt.pop());

    // Check if we need a local variable
    if (type >= object::ID_for_next_list)
    {
        // For debugging or conversion to text, ensure we track names
        locals_stack stack(p);

        // Skip name
        if (p[0] != 1)
            record(loop_error, "Evaluating for-next loop with %u locals", p[0]);
        p += 1;
        size_t namesz = leb128<size_t>(p);
        p += namesz;

        // Get list as local
        if (!rt.push(+range))
            return object::ERROR;
        rt.locals(1);

        // Pop local after execution
        if (!rt.run_push_data(nullptr, object_p(1)))
            return object::ERROR;
    }

    size_t size = 0;
    object_p first = range->objects(&size);
    object_p last = first + size;
    object_g body = object_p(p);
    if (body->defer() && rt.run_push_data(first, last) &&
        object::defer(type))
        return object::OK;

    return object::ERROR;
}


EVAL_BODY(StartNext)
// ----------------------------------------------------------------------------
//   Evaluate a for..next loop
// ----------------------------------------------------------------------------
{
    rt.command(o);

    if (object_p arg = rt.top())
        if (id argty = arg->type())
            if (is_array_or_list(argty))
                return list_loop(ID_start_next_list, o);

    return counted_loop(ID_start_next_conditional, o);
}


// ============================================================================
//
//   START...STEP loop
//
// ============================================================================

RENDER_BODY(StartStep)
// ----------------------------------------------------------------------------
//   Renderer for start-step loop
// ----------------------------------------------------------------------------
{
    return o->object_renderer(r, "start", nullptr, "step");
}


INSERT_BODY(StartStep)
// ----------------------------------------------------------------------------
//   Insert a start-step loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("start \t step"), ui.PROGRAM);
}


EVAL_BODY(StartStep)
// ----------------------------------------------------------------------------
//   Evaluate a for..step loop
// ----------------------------------------------------------------------------
{
    rt.command(o);
    return counted_loop(ID_start_step_conditional, o);
}



// ============================================================================
//
//   FOR...NEXT loop
//
// ============================================================================

SIZE_BODY(ForNext)
// ----------------------------------------------------------------------------
//   The size of a for loop begins with the name table
// ----------------------------------------------------------------------------
{
    byte_p p = payload(o);
    if (p[0] != 1)
        record(loop_error, "Size got %d variables instead of 1", p[0]);
    p++;
    size_t sz = leb128<size_t>(p);
    p += sz;
    size_t osize = object_p(p)->size();
    p += osize;
    return ptrdiff(p, o);
}


PARSE_BODY(ForNext)
// ----------------------------------------------------------------------------
//  Parser for for-next loops
// ----------------------------------------------------------------------------
{
    locals_stack locals;
    return loop::object_parser(p,
                               "for", nullptr,
                               "next", ID_ForNext,
                               "step", ID_ForStep,
                               true);
}


RENDER_BODY(ForNext)
// ----------------------------------------------------------------------------
//   Renderer for for-next loop
// ----------------------------------------------------------------------------
{
    locals_stack locals(payload(o));
    return o->object_renderer(r, "for", nullptr, "next", true);
}


INSERT_BODY(ForNext)
// ----------------------------------------------------------------------------
//   Insert a for-next loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("for \t next"), ui.PROGRAM);
}


EVAL_BODY(ForNext)
// ----------------------------------------------------------------------------
//   Evaluate a for..next loop
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (object_p arg = rt.top())
        if (id argty = arg->type())
            if (is_array_or_list(argty))
                return list_loop(ID_for_next_list, o);
    return counted_loop(ID_for_next_conditional, o);
}



// ============================================================================
//
//   FOR...STEP loop
//
// ============================================================================

RENDER_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Renderer for for-step loop
// ----------------------------------------------------------------------------
{
    locals_stack locals(payload(o));
    return o->object_renderer(r, "for", nullptr, "step", true);
}


INSERT_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Insert a for-step loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("for \t step"), ui.PROGRAM);
}


EVAL_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Evaluate a for..step loop
// ----------------------------------------------------------------------------
{
    rt.command(o);
    return counted_loop(ID_for_step_conditional, o);
}



// ============================================================================
//
//   Conditional - Runtime selector for loops
//
// ============================================================================
//   In order to avoid C++ stack usage, we evaluate conditional loops by
//   pushing on the run stack the true and false loops, and selecting which
//   branch to evaluate by evaluating `conditional`. If the stack is true,
//   then it picks the first one pushed. Otherwise, it picks the second one.
//   One of the two paths can be empty to terminate evaluation.
//   It can also be the original loop to repeat

RENDER_BODY(conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<conditional>");
    return r.size();
}


EVAL_BODY(conditional)
// ----------------------------------------------------------------------------
//  Picks which branch to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    return loop::evaluate_condition(ID_conditional, &runtime::run_select);
}


RENDER_BODY(need_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<need-conditional>");
    return r.size();
}


EVAL_BODY(need_conditional)
// ----------------------------------------------------------------------------
//  Ensure that after evaluation of a program/expression, we get a boolean
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (object_p cond = rt.top())
    {
        int truth = cond->as_truth(true);
        if (truth >= 0)
        {
            rt.top(static_object(truth ? ID_True : ID_False));
            return OK;
        }
    }
    return ERROR;
}


RENDER_BODY(while_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<while-repeat>");
    return r.size();
}


EVAL_BODY(while_conditional)
// ----------------------------------------------------------------------------
//  Picks which branch of a while loop to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    return loop::evaluate_condition(ID_while_conditional,
                                    &runtime::run_select_while);
}


RENDER_BODY(start_next_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<start-next>");
    return r.size();
}


EVAL_BODY(start_next_conditional)
// ----------------------------------------------------------------------------
//  Picks which branch of a start next to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_start_step(false, false))
        return OK;
    return ERROR;
}


RENDER_BODY(start_step_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<start-step>");
    return r.size();
}


EVAL_BODY(start_step_conditional)
// ----------------------------------------------------------------------------
//  Picks which branch of a start step to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_start_step(false, true))
        return OK;
    return ERROR;
}


RENDER_BODY(for_next_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<for-next>");
    return r.size();
}


EVAL_BODY(for_next_conditional)
// ----------------------------------------------------------------------------
//  Picks which branch of a start next to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_start_step(true, false))
        return OK;
    return ERROR;
}


RENDER_BODY(for_step_conditional)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<for-step>");
    return r.size();
}


EVAL_BODY(for_step_conditional)
// ----------------------------------------------------------------------------
//  Picks which branch of a start next to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_start_step(true, true))
        return OK;
    return ERROR;
}


RENDER_BODY(start_next_list)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<start-next-list>");
    return r.size();
}


EVAL_BODY(start_next_list)
// ----------------------------------------------------------------------------
//  Picks which branch of a start step to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_list(false))
        return OK;
    return ERROR;
}


RENDER_BODY(for_next_list)
// ----------------------------------------------------------------------------
//   Display for debugging purpose
// ----------------------------------------------------------------------------
{
    r.put("<for-next-list>");
    return r.size();
}


EVAL_BODY(for_next_list)
// ----------------------------------------------------------------------------
//  Picks which branch of a start next to choose at runtime
// ----------------------------------------------------------------------------
{
    rt.command(o);
    if (rt.run_select_list(true))
        return OK;
    return ERROR;
}
