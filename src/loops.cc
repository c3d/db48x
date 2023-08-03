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
#include "decimal-32.h"
#include "decimal-64.h"
#include "decimal128.h"
#include "integer.h"
#include "locals.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "user_interface.h"
#include "utf8.h"

#include <stdio.h>
#include <string.h>


RECORDER(loop, 16, "Loops");
RECORDER(loop_error, 16, "Errors processing loops");



SIZE_BODY(loop)
// ----------------------------------------------------------------------------
//   Compute size for a loop
// ----------------------------------------------------------------------------
{
    object_p p = object_p(o->payload());
    p = p->skip();
    return ptrdiff(p, o);
}


loop::loop(object_g body, symbol_g name, id type)
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


SIZE_BODY(conditional_loop)
// ----------------------------------------------------------------------------
//   Compute size for a conditional loop
// ----------------------------------------------------------------------------
{
    object_p p = object_p(o->payload());
    p = p->skip()->skip();
    return ptrdiff(p, o);
}


conditional_loop::conditional_loop(object_g first, object_g second, id type)
// ----------------------------------------------------------------------------
//   Constructor for conditional loops
// ----------------------------------------------------------------------------
    : loop(first, nullptr, type)
{
    size_t fsize = first->size();
    byte *p = (byte *) payload() + fsize;
    size_t ssize = second->size();
    memmove(p, byte_p(second), ssize);
}



object::result conditional_loop::condition(bool &value)
// ----------------------------------------------------------------------------
//   Check if the stack is a true condition
// ----------------------------------------------------------------------------
{
    if (object_p cond = rt.pop())
    {
        int truth = cond->as_truth(true);
        if (truth >= 0)
        {
            value = (bool) truth;
            return OK;
        }
    }
    return ERROR;
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
            size_t remaining = max - size_t(utf8(src) - utf8(p.source));
            if (len <= remaining
                && strncasecmp(cstring(utf8(src)), sep, len) == 0
                && (len >= remaining ||
                    command::is_separator(utf8(src) + len)))
            {
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
                if (len2 <= remaining
                    && strncasecmp(cstring(utf8(src)), close2, len2) == 0
                    && (len2 >= remaining ||
                        command::is_separator(utf8(src) + len2)))
                {
                    src += len;
                    found = true;
                    type = id2;
                    terminator = nullptr;
                    continue;
                }
            }

            // Parse an object
            size_t   done   = utf8(src) - utf8(p.source);
            size_t   length = max > done ? max - done : 0;
            object_g obj    = object::parse(src, length);
            if (!obj)
                return ERROR;

            // Copy the parsed object to the scratch pad (may GC)
            size_t objsize = obj->size();
            byte *objcopy = rt.allocate(objsize);
            if (!objcopy)
                return ERROR;
            memmove(objcopy, (byte *) obj, objsize);

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
    p.end         = parsed;
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
    auto     format = Settings.command_fmt;

    // Write the header, e.g. "DO"
    r.put('\n');
    r.put(format, utf8(open));

    // Render name if any
    if (name)
    {
        r.put(' ');
        r.put(name, namesz);
    }

    // Ident condition or first body
    r.indent();

    // Emit the first object (e.g. condition in do-until)
    first->render(r);

    // Emit the second object if there is one
    if (middle)
    {
        // Emit separator after condition
        r.unindent();
        r.put(format, utf8(middle));
        r.indent();
        second->render(r);
    }

    // Emit closing separator
    r.unindent();
    r.put(format, utf8(close));

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
    return ui.edit(utf8("do  until  end"), ui.PROGRAM, 3);
}


EVAL_BODY(DoUntil)
// ----------------------------------------------------------------------------
//   Evaluate a do..until..end loop
// ----------------------------------------------------------------------------
//   In this loop, the body comes first
{
    byte    *p    = (byte *) o->payload();
    object_g body = object_p(p);
    object_g cond = body->skip();
    result   r    = OK;

    while (!interrupted() && r == OK)
    {
        r = body->evaluate();
        if (r != OK)
            break;
        r = cond->evaluate();
        if (r != OK)
            break;
        bool test = false;
        r = o->condition(test);
        if (r != OK || test)
            break;
    }
    return r;
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
    return ui.edit(utf8("while  repeat  end"), ui.PROGRAM, 6);
}


EVAL_BODY(WhileRepeat)
// ----------------------------------------------------------------------------
//   Evaluate a while..repeat..end loop
// ----------------------------------------------------------------------------
//   In this loop, the condition comes first
{
    byte    *p    = (byte *) o->payload();
    object_g cond = object_p(p);
    object_g body = cond->skip();
    result   r    = OK;

    while (!interrupted() && r == OK)
    {
        r = cond->evaluate();
        if (r != OK)
            break;
        bool test = false;
        r = condition(test);
        if (r != OK || !test)
            break;
        r = body->evaluate();
    }
    return r;
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
    return ui.edit(utf8("start  next"), ui.PROGRAM, 6);
}


object::result loop::counted(object_g body, bool stepping, bool named)
// ----------------------------------------------------------------------------
//   Evaluate a counted loop
// ----------------------------------------------------------------------------
{
    object::result r      = object::OK;
    object_p       finish = rt.stack(0);
    object_p       start  = rt.stack(1);

    // If stack is empty, exit
    if (!start || !finish)
        return ERROR;
    rt.drop(2);

    // Negative integer or real value needed
    algebraic_p astep   = nullptr;
    bool        skip    = false;

    // Check that we have integers
    integer_p ifinish = finish->as<integer>();
    integer_p istart = start->as<integer>();
    if (istart && ifinish)
    {
        // We have an integer-only loop, go the fast route
        ularge incr = 1;
        ularge cnt  = istart->value<ularge>();
        ularge last = ifinish->value<ularge>();

        while (!interrupted() && r == OK)
        {
            // For named loops, store that in the local variable 0
            if (named)
            {
                integer_g ival = integer::make(cnt);
                rt.local(0, integer_p(ival));
            }

            r = body->evaluate();
            if (r != OK)
                break;

            // Check if we have a 'step' variant
            if (stepping)
            {
                object_p step = rt.pop();
                if (!step)
                    return ERROR;

                // Check if the type forces us to exit the integer loop
                id ty = step->type();
                if (ty == ID_integer)
                {
                    // Normal case
                    integer_p istep = integer_p(step);
                    incr            = istep->value<ularge>();
                }
                else if (is_real(ty))
                {
                    // Switch to slower "algebraic" loop
                    algebraic_g stp = algebraic_p(step);
                    algebraic_g sta = integer::make(cnt);
                    algebraic_g fin = integer::make(last);

                    // Skip first execution since we did it here
                    skip = true;

                    // No GC beyond this point
                    astep = stp;
                    start = sta;
                    finish = fin;
                    break;
                }
                else
                {
                    rt.type_error();
                    return ERROR;
                }
            }
            cnt += incr;
            if (cnt > last)
                break;

        }

        if (!astep)
            return r;
    }
    else if (start->is_real() && finish->is_real())
    {
        // Need to be a bit careful with GC here
        object_g    sta = start;
        object_g    fin = finish;
        algebraic_g stp = integer::make(1);
        if (!stp)
            return ERROR;

        // No GC beyond this point
        astep = stp;
        start = sta;
        finish = fin;
    }
    else
    {
        // Bad input, need to error out
        rt.type_error();
        return ERROR;
    }

    // Case where we need a slower loop
    if (astep)
    {
        // Now we need GC-safe pointers
        algebraic_g cnt       = algebraic_p(start);
        algebraic_g last      = algebraic_p(finish);
        algebraic_g zero      = integer::make(0);
        algebraic_g step      = astep;
        if (!zero)
            return ERROR;

        while (!interrupted() && r == OK)
        {
            if (skip)
            {
                skip = false;
            }
            else
            {
                // For named loops, store that in the local variable 0
                if (named)
                    rt.local(0, cnt);

                r = body->evaluate();
                if (r != OK)
                    break;

                // Check if we have a 'step' variant
                if (stepping)
                {
                    step = algebraic_g(algebraic_p(rt.pop()));
                    if (!step)
                        return ERROR;
                }
            }

            // Increment and check end of loop
            cnt = cnt + step;
            bool countdown = stepping && (step < zero)->as_truth() == 1;
            algebraic_g test = countdown ? cnt < last : cnt > last;
            if (test->as_truth())
                break;

         }
    }


    return r;
}


EVAL_BODY(StartNext)
// ----------------------------------------------------------------------------
//   Evaluate a for..next loop
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_p body = object_p(p);
    return counted(body, false);
}


// ============================================================================
//
//   START...STEP loop
//
// ============================================================================

PARSE_BODY(StartStep)
// ----------------------------------------------------------------------------
//  Parser for start-step loops
// ----------------------------------------------------------------------------
{
    // This is dealt with in StartNext
    return SKIP;
}


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
    return ui.edit(utf8("start  step"), ui.PROGRAM, 6);
}


EVAL_BODY(StartStep)
// ----------------------------------------------------------------------------
//   Evaluate a for..step loop
// ----------------------------------------------------------------------------
{
    byte    *p    = (byte *) o->payload();
    object_p body = object_p(p);
    return counted(body, true);
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
    locals_stack locals(o->payload());
    return o->object_renderer(r, "for", nullptr, "next", true);
}


INSERT_BODY(ForNext)
// ----------------------------------------------------------------------------
//   Insert a for-next loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("for  next"), ui.PROGRAM, 4);
}


object::result ForNext::counted(object_p o, bool stepping)
// ----------------------------------------------------------------------------
//   Evaluate a `for` counted loop
// ----------------------------------------------------------------------------
{
    byte *p = (byte *) o->payload();

    // For debugging or conversion to text, ensure we track names
    locals_stack stack(p);

    // Skip name
    if (p[0] != 1)
        record(loop_error, "Evaluating for-next loop with %u locals", p[0]);
    p += 1;
    size_t namesz = leb128<size_t>(p);
    p += namesz;

    // Get start value as local
    object_p start = rt.stack(1);
    if (!start)
        return ERROR;
    if (!rt.push(start))
        return ERROR;

    // Evaluate loop itself with one local created during loop
    object_p body = object_p(p);
    rt.locals(1);
    result r = loop::counted(body, stepping, true);
    rt.unlocals(1);
    return r;

}


EVAL_BODY(ForNext)
// ----------------------------------------------------------------------------
//   Evaluate a for..next loop
// ----------------------------------------------------------------------------
{
    return counted(o, false);
}



// ============================================================================
//
//   FOR...STEP loop
//
// ============================================================================

PARSE_BODY(ForStep)
// ----------------------------------------------------------------------------
//  Parser for for-step loops
// ----------------------------------------------------------------------------
{
    return SKIP;                                // Handled in ForNext
}


RENDER_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Renderer for for-step loop
// ----------------------------------------------------------------------------
{
    locals_stack locals(o->payload());
    return o->object_renderer(r, "for", nullptr, "step", true);
}


INSERT_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Insert a for-step loop in the editor
// ----------------------------------------------------------------------------
{
    return ui.edit(utf8("for  step"), ui.PROGRAM, 4);
}


EVAL_BODY(ForStep)
// ----------------------------------------------------------------------------
//   Evaluate a for..step loop
// ----------------------------------------------------------------------------
{
    return counted(o, true);
}
