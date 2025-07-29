// ****************************************************************************
//  list.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of RPL lists
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

#include "list.h"

#include "algebraic.h"
#include "array.h"
#include "compare.h"
#include "constants.h"
#include "expression.h"
#include "grob.h"
#include "locals.h"
#include "parser.h"
#include "polynomial.h"
#include "precedence.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "symbol.h"
#include "unit.h"
#include "utf8.h"
#include "variables.h"

#include <stdio.h>
#include <stdlib.h>


RECORDER(list, 16, "Lists");
RECORDER(list_parse, 16, "List parsing");
RECORDER(list_error, 16, "Errors processing lists");


object::result list::list_parse(id      type,
                                parser &p,
                                unicode open,
                                unicode close)
// ----------------------------------------------------------------------------
//   Generic parser for sequences (list, program, etc)
// ----------------------------------------------------------------------------
//   We have to be careful here, because parsing sub-objects may allocate new
//   temporaries, which itself may cause garbage collection. So we need to keep
//   track of our current usage of the scratchpad to allow for recursive parsing
//   of complex objects, like { { A B { C D } } }
{
    // We have to be careful that we may have to GC to make room for list
    gcutf8   s           = p.source;
    size_t   max         = p.length;
    object_g infix       = nullptr;
    object_g prefix      = nullptr;
    object_g postfix     = nullptr;
    object_g obj         = nullptr;
    object_g xroot_arg   = nullptr;
    int      precedence  = p.precedence;
    int      lowest      = precedence;
    uint     arity       = 0;
    uint     arg         = 0;
    size_t   objcount    = 0;
    size_t   non_alg     = 0;
    size_t   non_alg_len = 0;
    bool     xroot       = false;
    bool     alist       = precedence && (open == '[' || open == '{');

    // Some commands such as IFTE, Sum, Integrate or Root are special in that
    // we need to defer argument evaluation, i.e. keep arguments as expressions
    // so that their 'evaluate' function receives the symbolic form
    id       special     = ID_object;

    // The `|` operator (where) is special when parsing parentheses
    bool     iswhere     = false;

    record(list, "Parse %lc%lc precedence %d length %u [%s]",
           open, close, precedence, max, utf8(s));

    // Check if we have the opening marker
    unicode cp  = 0;
    if (open)
    {
        cp = utf8_codepoint(s);
        if (cp != open)
            return SKIP;
        s = utf8_next(s);
        cp = 0;                 // Do not accept "'" as an empty equation
    }

    scribble scr;
    while (utf8_more(p.source, s, max))
    {
        cp = utf8_codepoint(s);
        if (cp == close)
        {
            s = +p.source + utf8_next(+p.source, s - +p.source, max);
            break;
        }
        if (precedence && (cp == '\'' || cp == ')' ||
                           (!alist && (cp == ';' || cp == '}' || cp == ']'))))
            break;
        if (utf8_whitespace(cp) || (cp == ';' && alist))
        {
            s = utf8_next(s);
            if (cp == ';')
                precedence = p.precedence;
            continue;
        }

        // Parse an object
        size_t done        = (byte *) s - (byte *) p.source;
        size_t length      = max > done ? max - done : 0;
        id     postfix_cmd = id(0);

        // For algebraic objects, check if we have or need parentheses
        if (precedence && length)
        {
            if (precedence > 0)
            {
                // Check if we see parentheses, or if we have `sin sin X`
                bool parenthese = (cp == '(' || arity > 1) && !infix;
                if (parenthese || infix || prefix || alist)
                {
                    int childp = infix      ? int(infix->precedence() + 1)
                               : parenthese ? int(LOWEST)
                               : alist      ? int(LOWEST)
                                            : int(SYMBOL);
                    if (prefix)
                    {
                        int pp = prefix->precedence();
                        if (pp && pp < FUNCTION)
                        {
                            childp = pp + 1;
                            parenthese = false;
                        }
                    }
                    if (infix && infix->type() == ID_Copy)
                    {
                        childp = int(SYMBOL);
                        special = ID_Copy;
                    }

                    if (infix && cp == '(' && infix->type() == ID_Where)
                    {
                        iswhere = true;
                        if (utf8_more(p.source, s, max))
                            s = utf8_next(s);
                    }
                    if (iswhere)
                    {
                        parenthese = false;
                        childp = int(LOWEST);
                    }

                    parser  child(p, s, childp);
                    unicode iopen  = parenthese ? '(' : 0;
                    unicode iclose = parenthese ? ')' : 0;
                    id ctype = alist || type == ID_unit ? ID_expression : type;

                    if (!infix && arity > 1)
                    {
                        if (arg)
                            iopen = 0;
                        arg++;
                        if (arg < arity)
                            iclose = ';';
                    }

                    record(list_parse, "%+s starting at offset %u '%s'",
                           parenthese ? "Parenthese" : "Child",
                           utf8(s) - utf8(p.source),
                           utf8(s));

                    auto rc = list_parse(ctype, child, iopen, iclose);
                    if (rc != OK)
                        return rc;
                    obj = child.out;
                    if (!obj)
                        return ERROR;

                    // A sum must have a name as first argument
                    if (arg == 1 && prefix)
                    {
                        id pty = prefix->type();
                        if (pty == ID_Sum || pty == ID_Product)
                        {
                            if (obj->as_quoted<symbol>() == nullptr)
                            {
                                rt.missing_variable_error()
                                    .source(+s+1, child.length-2);
                                return ERROR;
                            }
                        }
                    }

                    s = s + child.length;
                    record(list_parse,
                           "Child parsed as %t length %u",
                           object_p(obj), child.length);
                    precedence = -precedence; // Stay in postfix mode
                    cp = utf8_more(p.source, s, max) ? utf8_codepoint(s) : 0;
                    length = 0;

                    if (iswhere)
                    {
                        utf8 start = +s - child.length;
                        while (utf8_whitespace(cp) &&
                               utf8_more(p.source, s, max))
                        {
                            s = utf8_next(s);
                            cp = utf8_codepoint(s);
                        }
                        if (cp == ')')
                        {
                            iswhere = false;
                        }
                        else if (cp != ';')
                        {
                            rt.unterminated_error().source(start, +s-start);
                            return ERROR;
                        }
                        s = utf8_next(s);
                        cp = utf8_more(p.source,s,max) ? utf8_codepoint(s) : 0;
                    }
                }

                // Check to see if we have a sign
                else if (cp == '-' || cp == '+')
                {
                    if (cp == '+')
                    {
                        s = utf8_next(s);
                        continue;
                    }
                    obj = static_object(ID_neg);
                    length = 1;
                }

            }
            if (precedence < 0)
            {
                // Check special postfix notations
                switch(cp)
                {
                case L'²':
                    postfix_cmd = ID_sq;
                    break;
                case L'³':
                    postfix_cmd = ID_cubed;
                    break;
                case '!':
                    postfix_cmd = ID_fact;
                    break;
                case L'⁻':
                    if (utf8_codepoint(utf8_next(s)) == L'¹')
                        postfix_cmd = ID_inv;
                    break;
                default:
                    break;
                }
                if (postfix_cmd)
                {
                    utf8 cur = utf8(s);
                    (obj ? postfix : obj) = command::static_object(postfix_cmd);
                    length = postfix_cmd == ID_inv
                        ? utf8_next(utf8_next(cur)) - cur
                        : utf8_next(cur) - cur;
                    precedence = -precedence; // Stay in postfix mode
                }
            }
        }

        if (!obj && length)
        {
            obj = object::parse(s, length, precedence);
            record(list_parse,
                   "Item parsed as %t length %u arity %u",
                   object_p(obj), length, arity);
        }
        if (!obj)
            return ERROR;

        if (precedence && !postfix_cmd)
        {
            // We are parsing an equation
            if (precedence > 0)
            {
                // We just parsed an algebraic, e.g. 'sin', etc
                // stash it and require parentheses for arguments
                id type = obj->type();
                if (function::has_symbolic_arguments(type))
                    special = type;
                if (!is_extended_algebraic(type) && !special)
                {
                    if (objcount)
                    {
                        rt.prefix_expected_error().source(s, length);
                        return ERROR;
                    }
                    non_alg = +s - +p.source;
                    non_alg_len = length;
                }

                // TODO: A symbol could be a function, need to deal with that
                if (is_command(type))
                {
                    prefix = obj;
                    if (prefix)
                    {
                        arity = prefix->arity();
                        arg = 0;

                        // xroot is the one command that parses args backwards
                        if (type == ID_xroot)
                            xroot = true;
                    }
                    if (arity)
                    {
                        precedence = -SYMBOL;
                        obj = nullptr;
                    }
                    else
                    {
                        prefix = nullptr;
                    }
                }
            }
            else if (int objprec = obj->precedence())
            {
                // We just parsed an infix, e.g. +, -, etc
                // stash it, or exit loop if it has lower precedence
                if (objprec < lowest)
                    break;
                if (objprec < FUNCTIONAL)
                {
                    infix = obj;
                    precedence = -objprec;
                    obj = nullptr;
                }
                else if (!infix)
                {
                    // Implicit multiplication
                    infix = static_object(ID_multiply);
                }
            }
            else
            {
                precedence = -precedence;
            }
        }

        if (xroot && obj && !xroot_arg)
        {
            xroot_arg = obj;
            xroot = false;
            obj = nullptr;
            precedence = -precedence;
        }

        if (obj)
        {
            // Copy the parsed object to the scratch pad (may GC)
            do
            {
                record(list_parse, "Copying %t to scratchpad", object_p(obj));
                objcount++;

                size_t objsize = obj->size();
                bool   done    = special && obj->type() == special;

                // For expressions, copy only the payload unless we want it
                // to be preserved as a symbolic expression
                if (precedence && !alist)
                    if (!special ||
                        !function::is_symbolic_argument(special, arity-arg))
                        if (expression_p eq = obj->as<expression>())
                            obj = eq->objects(&objsize);

                if (!rt.append(obj, objsize))
                    return ERROR;

                if (xroot_arg)
                {
                    obj = xroot_arg;
                    xroot_arg = nullptr;
                    arg++;
                }
                else if (prefix)
                {
                    if (arity > 1 && arg < arity)
                    {
                        precedence = -precedence;
                        break;
                    }
                    obj = prefix;
                    prefix = nullptr;
                }
                else if (postfix)
                {
                    obj = postfix;
                    postfix = nullptr;
                }
                else
                {
                    obj = infix;
                    infix = nullptr;
                }
                if (done)
                    special = ID_object;
            } while (obj);

            if (iswhere)
            {
                infix = object::static_object(ID_Where);
                precedence = -WHERE;
            }
        }

        // Jump past what we parsed
        s = s + length;

        // For equations switch between infix and prefix
        precedence = -precedence;
    }

    record(list, "Exiting parser at %s infix=%t prefix=%p",
           utf8(s), object_p(infix), object_p(prefix));


    // If we still have a pending opcode here, syntax error (e.g. '1+`)
    if (infix || prefix)
    {
        object_g obj = prefix ? prefix : infix;
        rt.command(obj);
        if (objcount == 0 && scr.growth() == 0)
        {
            if (!rt.append(obj))
                return ERROR;
            objcount = 1;
        }
        else
        {
            rt.argument_expected_error();
            return ERROR;
        }
    }

    if (non_alg && objcount != 1)
    {
        rt.syntax_error().source(p.source + non_alg, non_alg_len);
        return ERROR;
    }

    // Check that we have a matching closing character
    if (close && cp != close)
    {
        if (cp != ';')
            record(list_error,
                   "Missing terminator, got %u (%c) not %u (%c) at %s",
                   cp, cp, close, close, utf8(s));
        rt.unterminated_error().source(p.source, +s - +p.source);
        return ERROR;
    }

    // Create the object
    gcbytes scratch = scr.scratch();
    size_t  alloc   = scr.growth();
    size_t  parsed  = utf8(s) - utf8(p.source);

    // Check for the case of an empty equation
    if (alloc == 0 && type == ID_expression)
    {
        record(list_error, "Empty equation");
        rt.syntax_error().source(p.source);
        return ERROR;
    }

    // Create the object
    p.length = parsed;
    p.out    = rt.make<list>(type, scratch, alloc);

    record(list_parse, "Parsed as %t length %u", object_p(p.out), parsed);

    // Return success
    return p.out ? OK : ERROR;
}


intptr_t list::list_render(renderer &r, unicode open, unicode close) const
// ----------------------------------------------------------------------------
//   Render the list into the given buffer
// ----------------------------------------------------------------------------
{
    // Check if we need an indent in the body
    bool   unnest      = false;
    bool   first       = true;
    list_g list        = this;
    id     lty         = type();
    bool   need_indent = lty == ID_program;
    bool   expr        = r.expression();
    settings::SaveShowEquationBody sseb(false);

    if (!expr)
    {
        for (object_p obj : *list)
        {
            id oty = obj->type();
            switch (oty)
            {
            case ID_array:
                if (first && lty == oty)
                    unnest = true;
                // fallthrough
            case ID_list:
            case ID_program:
            case ID_locals:
            case ID_comment:
            case ID_IfThen:
            case ID_IfThenElse:
            case ID_DoUntil:
            case ID_WhileRepeat:
            case ID_StartStep:
            case ID_ForNext:
            case ID_ForStep:
            case ID_IfErrThen:
            case ID_IfErrThenElse:
                need_indent = true;
                break;
            default:
                break;
            }
            if (need_indent)
                break;
            first = false;
        }
    }

    // Write the header, e.g. "{ "
    bool crpgm = need_indent && !unnest && !expr;
    if (open)
    {
        r.put(open);
        if (!unnest && !expr)
        {
            r.indent();
            r.wantSpace();
        }
    }
    if (crpgm)
        r.wantCR();

    // Loop on all objects inside the list
    for (object_g obj : *list)
    {
        id oty = obj->type();
        if (oty == ID_program || oty == ID_list || oty == ID_array)
        {
            if (open == 1 || !unnest)
                r.wantCR();
            if (unnest && open == 1)
                r.put("   ");
        }
        if (open == 1 && expr)
            r.put(';');
        open = 1;

        // Render the object in what remains (may GC)
        if (oty != ID_array && !expr)
            r.wantSpace();
        if ((lty == ID_program || lty == ID_block) &&
            r.editing() &&
            Settings.VerticalProgramRendering())
            r.wantCR();
        obj->render(r);
        if (!unnest && oty != ID_array && !expr)
            r.wantSpace();
    }

    // Add final space and closing separator
    if (close)
    {
        if (!unnest && !expr)
        {
            r.unindent();
            if (lty != ID_array && !expr)
                r.wantSpace();
        }
        if (crpgm)
            r.wantCR();
        r.put(close);
        if (!unnest && lty != ID_array && !expr)
            r.wantSpace();
    }
    if (crpgm)
        r.wantCR();

    return r.size();
}


list_p list::append(list_p a) const
// ----------------------------------------------------------------------------
//   Append the second list to this one
// ----------------------------------------------------------------------------
{
    text_g x = text_p(this);
    text_g y = text_p(a);
    return list_p(+(x + y));
}


list_p list::append(object_p o) const
// ----------------------------------------------------------------------------
//   Append object to list
// ----------------------------------------------------------------------------
{
    text_g x = text_p(this);
    text_g y = text::make(byte_p(o), o->size());
    return list_p(+(x + y));
}


list_p list::remove(size_t first, size_t len) const
// ----------------------------------------------------------------------------
//   Remove items in the given range
// ----------------------------------------------------------------------------
{
    scribble scr;
    size_t   idx = 0;
    id       ty  = type();
    for (object_p copy : *this)
    {
        if (idx < first || idx >= first + len)
            if (!rt.append(copy))
                return nullptr;
        idx++;
    }
    return list::make(ty, scr.scratch(), scr.growth());
}


list_p list::insert(object_p whato, size_t pos) const
// ----------------------------------------------------------------------------
//   Insert `what` in the list at position `pos` (0-based)
// ----------------------------------------------------------------------------
{
    scribble scr;
    size_t   idx = 0;
    id       ty  = type();
    object_g what = whato;
    for (object_g copy : *this)
    {
        if (idx == pos)
            if (!rt.append(what))
                return nullptr;
        if (!rt.append(copy))
            return nullptr;
        idx++;
    }
    if (idx == pos)
        if (!rt.append(what))
            return nullptr;
    return list::make(ty, scr.scratch(), scr.growth());
}


list_p list::insert(list_p whato, size_t pos) const
// ----------------------------------------------------------------------------
//   Insert items in `what` in the list at position `pos` (0-based)
// ----------------------------------------------------------------------------
{
    scribble scr;
    size_t   idx = 0;
    id       ty  = type();
    list_g   what = whato;
    for (object_g copy : *this)
    {
        if (idx == pos)
            for (object_p inserted : *what)
                if (!rt.append(inserted))
                    return nullptr;
        if (!rt.append(copy))
            return nullptr;
        idx++;
    }
    if (idx == pos)
        for (object_p inserted : *what)
            if (!rt.append(inserted))
                return nullptr;
    return list::make(ty, scr.scratch(), scr.growth());
}


list_p list::swap(size_t first, size_t second) const
// ----------------------------------------------------------------------------
//   Swap elements at first and second position
// ----------------------------------------------------------------------------
{
    if (first == second)
        return this;
    if (first > second)
        std::swap(first, second);

    scribble scr;
    size_t   idx  = 0;
    id       ty   = type();
    object_g sobj = at(second);
    if (!sobj)
        return nullptr;

    for (object_g copy : *this)
    {
        if (idx == first)
        {
            if (!rt.append(sobj))
                return nullptr;
            sobj = copy;
            first = second;
        }
        else
        {
            if (!rt.append(copy))
                return nullptr;
        }
        idx++;
    }

    return list::make(ty, scr.scratch(), scr.growth());
}


bool list::expand_without_size(size_t *size) const
// ----------------------------------------------------------------------------
//   Expand items on the stack, but do not add the size
// ----------------------------------------------------------------------------
{
    size_t depth = rt.depth();
    for (object_p obj : *this)
    {
        if (!rt.push(obj))
        {
            rt.drop(rt.depth() - depth);
            return false;
        }
    }
    if (size)
        *size = rt.depth() - depth;
    return true;
}


bool list::expand() const
// ----------------------------------------------------------------------------
//   Expand items on the stack, adding the size
// ----------------------------------------------------------------------------
{
    size_t depth = rt.depth();
    if (expand_without_size())
        if (rt.push(integer::make(rt.depth() - depth)))
            return true;
    rt.drop(rt.depth() - depth);
    return false;
}


bool list::expand_deep(uint32_t which) const
// ----------------------------------------------------------------------------
//   Expand list content, expending inner expressions/programs/lists
// ----------------------------------------------------------------------------
{
    for (object_p obj : *this)
    {
        id ty = obj->type();
        if (uint(ty) < 32 && ((1U<<uint(ty)) & which))
        {
            if (!list_p(obj)->expand_deep(which))
                return false;
        }
        else if (!rt.push(obj))
        {
            return false;
        }
    }
    return true;
}


PARSE_BODY(list)
// ----------------------------------------------------------------------------
//    Try to parse this as an list
// ----------------------------------------------------------------------------
{
    return list_parse(ID_list, p, '{', '}');
}


EVAL_BODY(list)
// ----------------------------------------------------------------------------
//   Evaluate a list if `ListEvaluation` is set
// ----------------------------------------------------------------------------
{
    if (!program::running && o->type() == ID_list && Settings.ListAsProgram())
        return program_p(o)->run(false);
    return object::do_evaluate(o);
}


RENDER_BODY(list)
// ----------------------------------------------------------------------------
//   Render the list into the given list buffer
// ----------------------------------------------------------------------------
{
    return o->list_render(r, '{', '}');
}


using pixsize = grob::pixsize;


static pixsize row_height(size_t r, size_t rows, size_t cols)
// ----------------------------------------------------------------------------
//   Compute the height of a row (max of height of all grobs)
// ----------------------------------------------------------------------------
{
    size_t  nitems = rows * cols;
    pixsize rh     = 0;
    for (size_t c = 0; c < cols; c++)
    {
        size_t i       = r * cols + c;
        grob_p colitem = grob_p(rt.stack(nitems + ~i));
        ASSERT(colitem);
        pixsize h = colitem->height();
        if (rh < h)
            rh = h;
    }
    return rh;
}


static pixsize col_width(size_t c, size_t rows, size_t cols)
// ----------------------------------------------------------------------------
//   Compute the height of a row (max of height of all grobs)
// ----------------------------------------------------------------------------
{
    size_t  nitems = rows * cols;
    pixsize cw     = 0;
    for (size_t r = 0; r < rows; r++)
    {
        size_t  i       = r * cols + c;
        grob_p  colitem = grob_p(rt.stack(nitems + ~i));
        pixsize w       = colitem->width();
        if (cw < w)
            cw = w;
    }
    return cw;
}


grob_p list::graph(grapher &g, size_t rows, size_t cols, bool mat) const
// ----------------------------------------------------------------------------
//   Render a list or array exploded on stack
// ----------------------------------------------------------------------------
{
    list_g list = this;
    save<bool> sgraph(g.graph, false);

    // Convert all elements to graphical equivalent
    size_t nitems = rows * cols;
    for (size_t i = 0; i < nitems; i++)
    {
        object_p item = rt.stack(i);
        grob_g   grob = item->graph(g);
        if (!grob || !grob->is_monochrome())
        {
            // Ran into a problem with one rendering, e.g. out of memory
            // Fallback to rendering as text.
            record(list_error, "Problem graphing %zu in %zu x %zu",
                   i, rows, cols);
            rt.drop(nitems);
            return object::do_graph(list, g);
        }
        rt.stack(i, grob);
    }

    // Compute the width, starting with 8 pixels for borders
    pixsize bw  = mat ? 4 : 2;
    pixsize sw  = 2;
    pixsize gap = 12;
    pixsize gw  = 4 * bw + (cols - 1) * gap + 2 * sw;
    pixsize gh  = 0;

    // Compute the width as the sum of the max width of all columns
    for (size_t c = 0; c < cols; c++)
    {
        size_t cw = col_width(c, rows, cols);
        gw += cw;
        if (gw > g.maxw)
        {
            rt.drop(nitems);
            return nullptr;
        }
    }

    // Compute the height as the sum of the max height of all rows
    for (size_t r = 0; r < rows; r++)
    {
        size_t rh = row_height(r, rows, cols);
        gh += rh;
        if (gh > g.maxh)
        {
            rt.drop(nitems);
            return nullptr;
        }
    }

    // Create the resulting graph
    grob_g result = g.grob(gw, gh);
    if (!result)
    {
        rt.drop(nitems);
        return nullptr;
    }
    grob::surface rs = result->pixels();
    rs.fill(0, 0, gw, gh, g.background);

    // Position of items in enclosing border and adjust
    coord xl = 0;
    coord yt = 0;

    // Draw all items inside the matrix
    coord yi = yt;
    for (size_t r = 0; r < rows; r++)
    {
        pixsize rh = row_height(r, rows, cols);
        coord   xi = xl + 2 * bw + sw;
        for (size_t c = 0; c < cols; c++)
        {
            pixsize cw      = col_width(c, rows, cols);
            size_t  i       = r * cols + c;
            grob_p  colitem = grob_p(rt.stack(nitems + ~i));
            grob::surface is      = colitem->pixels();
            pixsize iw      = is.width();
            pixsize ih      = is.height();
            rs.copy(is, xi + (cw - iw)/2, yi + (rh - ih)/2);
            xi += cw + gap;
        }
        yi += rh;
    }

    rt.drop(nitems);
    return result;
}


GRAPH_BODY(list)
// ----------------------------------------------------------------------------
//  Render the list vertically if the option is given
// ----------------------------------------------------------------------------
{
    if (Settings.VerticalLists() && o->type() == ID_list)
    {
        size_t depth = rt.depth();
        list_g list = o;
        if (list->expand_without_size())
        {
            size_t rows = rt.depth() - depth;
            size_t cols = 1;
            grob_g result = list->graph(g, rows, cols, true);
            if (!result)
                return nullptr;
            grob::surface rs = result->pixels();
            pixsize       gw = rs.width();
            pixsize       gh = rs.height();

            // Dimensions of border and adjust
            coord   xl = 0;
            coord   xr = coord(gw) - 2;
            coord   yt = 0;
            coord   yb = coord(gh) - 4;
            pixsize bw = 2;

            // Add the borders
            for (coord y = 1; y < yb; y++)
            {
                coord d = y < yb/2 - 1 || y > yb/2 + 1;
                rs.fill(xl+d,    y, xl+d+bw,   y, g.foreground);
                rs.fill(xr-d-bw, y, xr-d,      y, g.foreground);
            }
            rs.fill(xl+bw,   yt, xl+2*bw, yt+1, g.foreground);
            rs.fill(xr-2*bw, yt, xr-bw,   yt+1, g.foreground);
            rs.fill(xl+bw,   yb, xl+2*bw, yb+1, g.foreground);
            rs.fill(xr-2*bw, yb, xr-bw,   yb+1, g.foreground);

            return result;
        }
    }

    return object::do_graph(o, g);
}


HELP_BODY(list)
// ----------------------------------------------------------------------------
//   Help topic for lists
// ----------------------------------------------------------------------------
{
    return utf8("Lists");
}



// ============================================================================
//
//   Command implementation
//
// ============================================================================

list_p list::list_from_stack(uint depth, id ty)
// ----------------------------------------------------------------------------
//  Make a list from the stack as an object
// ----------------------------------------------------------------------------
{
    scribble scr;
    for (uint i = 0; i < depth; i++)
        if (object_g obj = rt.stack(depth + ~i))
            if (!rt.append(obj))
                return nullptr;

    if (list_p result = list::make(ty, scr.scratch(), scr.growth()))
    {
        rt.drop(depth);
        return result;
    }
    return nullptr;
}


object::result list::push_list_from_stack(uint depth, id ty)
// ----------------------------------------------------------------------------
//  Make a list on the stack
// ----------------------------------------------------------------------------
{
    if (object_g list = list_from_stack(depth, ty))
        if (rt.push(list))
            return object::OK;
    return object::ERROR;
}


COMMAND_BODY(ToList)
// ----------------------------------------------------------------------------
//   Convert elements to a list
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
    {
        if (!rt.args(depth + 1))
            return ERROR;

        if (rt.pop())
            return list::push_list_from_stack(depth);
    }
    return ERROR;
}


COMMAND_BODY(FromList)
// ----------------------------------------------------------------------------
//   Convert elements to a list
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_p li = obj->as<list>())
    {
        rt.drop();
        if (li->expand())
            return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(Size)
// ----------------------------------------------------------------------------
//   Return the size of an object
// ----------------------------------------------------------------------------
//   Behaves differently from standard RPL for integers, equations and units
//   where it returns 1 and not some weirdo internal count
{
    object_p obj    = rt.top();
    id       oty    = obj->type();
    size_t   size   = 1;

    switch (oty)
    {
    case ID_list:
        size = list_p(obj)->items(); break;
    case ID_array:
        if (object_p result = array_p(obj)->dimensions())
            if (rt.top(result))
                return OK;
        break;
    case ID_text:
        size = text_p(obj)->utf8_characters(); break;
    case ID_grob:
#if CONFIG_COLOR
    case ID_pixmap:
#endif // CONFIG_COLOR
    case ID_bitmap:
        if (grob_p gr = grob_p(obj))
        {
            grob::pixsize w = 0, h = 0;
            if (gr->pixels(&w, &h))
            {
                integer_g wo = rt.make<based_integer>(w);
                integer_g ho = rt.make<based_integer>(h);
                if (wo && ho && rt.top(+wo) && rt.push(+ho))
                    return OK;
            }
        }
        return ERROR;
    case ID_range:
    case ID_drange:
    case ID_prange:
        if (algebraic_g lo = range_p(obj)->x())
            if (algebraic_g hi = range_p(obj)->y())
                if (algebraic_g sz = hi - lo)
                    if (sz && rt.top(+sz))
                        return OK;
        return ERROR;
    case ID_uncertain:
        if (algebraic_g s = uncertain_p(obj)->stddev())
            if (s && rt.top(+s))
                return OK;
        return ERROR;
    default:
        break;
    }

    integer_p szo = integer::make(size);
    if (szo && rt.top(szo))
        return OK;
    return ERROR;
}


static object::result get(bool increment)
// ----------------------------------------------------------------------------
//   Get element from structure, incrementing index or not
// ----------------------------------------------------------------------------
{
    // Check we have an object at level 2
    if (object_p items = rt.stack(1))
    {
        if (symbol_p name = items->as_quoted<symbol>())
        {
            items = directory::recall_all(name, true);
            if (!items)
                return object::ERROR;
        }

        object_p item = items->at(rt.stack(0));
        if (!item)
        {
            if (!rt.error())
                rt.index_error();
        }
        else if (increment)
        {
            rt.push(item);
            object_g index = rt.stack(1);
            bool wrap = items->next_index(&+index);
            if (index)
            {
                rt.stack(1, index);
                Settings.IndexWrapped(wrap);
                return object::OK;
            }
        }
        else if (rt.pop() && rt.top(item))
        {
            return object::OK;
        }
    }
    return object::ERROR;
}


COMMAND_BODY(Get)
// ----------------------------------------------------------------------------
//   Get an element in a list
// ----------------------------------------------------------------------------
{
    return get(false);
}


COMMAND_BODY(GetI)
// ----------------------------------------------------------------------------
//   Get an element in a list and increment the index
// ----------------------------------------------------------------------------
{
    return get(true);
}


static object::result put(bool increment)
// ----------------------------------------------------------------------------
//   Put element in structure, incrementing index or not
// ----------------------------------------------------------------------------
{
    // Check that we have an object at level 2
    if (object_p items = rt.stack(2))
    {
        symbol_p name = items->as_quoted<symbol>();
        if (name)
        {
            items = directory::recall_all(name, true);
            if (!items)
                return object::ERROR;
        }

        if (object_g result = items->at(rt.stack(1), rt.top()))
        {
            if (increment)
            {
                object_g index = rt.stack(1);
                bool wrap = result->next_index(&+index);
                if (index)
                {
                    rt.stack(1, +index);
                    Settings.IndexWrapped(wrap);
                }
            }
            if (name)
            {
                name = rt.stack(2)->as_quoted<symbol>();
                if (directory::update(name, result))
                {
                    rt.drop(increment ? 1 : 3);
                    return object::OK;
                }
            }
            else
            {
                if (rt.drop(increment ? 1 : 2))
                    if (rt.stack(increment ? 1 : 0, result))
                        return object::OK;
            }
        }

        if(!rt.error())
            rt.index_error();
    }
    return object::ERROR;
}


COMMAND_BODY(Put)
// ----------------------------------------------------------------------------
//   Put an element in a list
// ----------------------------------------------------------------------------
{
    return put(false);
}


COMMAND_BODY(PutI)
// ----------------------------------------------------------------------------
//   Put an element in a list, incrementing the index
// ----------------------------------------------------------------------------
{
    return put(true);
}


COMMAND_BODY(Head)
// ----------------------------------------------------------------------------
//   Return first element in a list
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_p li = obj->as_array_or_list())
    {
        if (object_p hd = li->head())
        {
            if (rt.top(hd))
                return OK;
        }
        else
        {
            rt.dimension_error();
        }
    }
    else if (text_p tobj = obj->as<text>())
    {
        size_t len = 0;
        gcutf8 txt = tobj->value(&len);
        if (len)
            tobj = text::make(txt, utf8_size(txt, len));
        if (tobj && rt.top(tobj))
            return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(Tail)
// ----------------------------------------------------------------------------
//   Return all but first element in a list
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_p li = obj->as_array_or_list())
    {
        if (object_p tl = li->tail())
        {
            if (rt.top(tl))
                return OK;
        }
        else
        {
            // On HP50, HEAD returns an error, but TAIL returns an empty list
            // rt.dimension_error();
            return OK;
        }
    }
    else if (text_p tobj = obj->as<text>())
    {
        size_t len = 0;
        gcutf8 txt = tobj->value(&len);
        if (len)
        {
            size_t sz = utf8_size(txt, len);
            tobj = text::make(txt + sz, len > sz ? len - sz : 0);
        }
        if (tobj && rt.top(tobj))
            return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


static object::result map_reduce_filter(object_p (list::*cmd)(object_p) const)
// ----------------------------------------------------------------------------
//   Shared code for map, reduce and filter
// ----------------------------------------------------------------------------
{
    size_t   depth = rt.depth();
    object_p obj   = rt.stack(1);
    object_g prg   = rt.top();
    if (list_p li = obj->as_array_or_list())
    {
        object_p result = (li->*cmd)(prg);
        if (!result)
            goto error;
        if (rt.drop() && rt.top(result))
            return object::OK;
    }
    else
    {
        rt.type_error();
    }
error:
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return object::ERROR;
}


COMMAND_BODY(Map)
// ----------------------------------------------------------------------------
//   Apply unary function in level 1 to all elements in level 2
// ----------------------------------------------------------------------------
{
    return map_reduce_filter(&list::map_as_object);
}


COMMAND_BODY(Reduce)
// ----------------------------------------------------------------------------
//   Apply binary function in level 1 pairwise to combine elements in level 2
// ----------------------------------------------------------------------------
{
    return map_reduce_filter(&list::reduce);
}


COMMAND_BODY(Filter)
// ----------------------------------------------------------------------------
//   Filter the function in level 1 to all elements in level 2
// ----------------------------------------------------------------------------
{
    return map_reduce_filter(&list::filter_as_object);
}


static object::result list_reduce(object::id cmd)
// ----------------------------------------------------------------------------
//   Shared code for map, reduce and filter
// ----------------------------------------------------------------------------
{
    object_p   obj = rt.stack(0);
    if (list_p li = obj->as_array_or_list())
    {
        object_p result = li->reduce(command::static_object(cmd));
        if (result && rt.top(result))
            return object::OK;
    }
    else
    {
        rt.type_error();
    }
    return object::ERROR;
}


static object::result list_pair_map(object::id cmd)
// ----------------------------------------------------------------------------
//   Shared code for map, reduce and filter
// ----------------------------------------------------------------------------
{
    object_p   obj = rt.stack(0);
    if (list_p li = obj->as_array_or_list())
    {
        object_p cmdobj = command::static_object(cmd);
        object_p result = li->pair_map(cmdobj);
        if (result && rt.top(result))
            return object::OK;
    }
    else
    {
        rt.type_error();
    }
    return object::ERROR;
}


COMMAND_BODY(ListSum)
// ----------------------------------------------------------------------------
//   Return the sum of a list or array
// ----------------------------------------------------------------------------
{
    return list_reduce(ID_add);
}


COMMAND_BODY(ListProduct)
// ----------------------------------------------------------------------------
//   Return the product of a list or array
// ----------------------------------------------------------------------------
{
    return list_reduce(ID_multiply);
}


COMMAND_BODY(ListDifferences)
// ----------------------------------------------------------------------------
//   Return the product of a list or array
// ----------------------------------------------------------------------------
{
    return list_pair_map(ID_subtract);
}


object_p list::head() const
// ----------------------------------------------------------------------------
//   Return the first element in the list
// ----------------------------------------------------------------------------
{
    size_t   size  = 0;
    object_p first = objects(&size);
    if (!size)
        return nullptr;
    return first;
}


list_p list::tail() const
// ----------------------------------------------------------------------------
//   Return the tail elements of the list
// ----------------------------------------------------------------------------
{
    size_t   size  = 0;
    object_p first = objects(&size);
    if (!size)
        return nullptr;
    size_t osize = first->size();
    byte_p rest  = byte_p(first) + osize;
    return list::make(type(), rest, size - osize);
}


list_p list::map(object_p prgobj) const
// ----------------------------------------------------------------------------
//   Apply an RPL object (nominally a program) on all elements in the list
// ----------------------------------------------------------------------------
{
    id       ty    = type();
    object_g prg   = prgobj;
    size_t   depth = rt.depth();
    scribble scr;
    for (object_p obj : *this)
    {
        id oty = obj->type();
        if (is_array_or_list(oty))
        {
            list_g sub = list_p(obj)->map(prg);
            obj = +sub;
        }
        else
        {
            if (!rt.push(obj))
                goto error;
            if (program::run(prg, true) != OK)
                goto error;
            if (rt.depth() != depth + 1)
            {
                rt.misbehaving_program_error();
                goto error;
            }
            obj = rt.pop();
        }
        if (!obj)
            goto error;

        if (!rt.append(obj))
            goto error;
    }

    return list::make(ty, scr.scratch(), scr.growth());

error:
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return nullptr;

}


object_p list::reduce(object_p prgobj) const
// ----------------------------------------------------------------------------
//   Apply an RPL object (nominally a program) on pairs of list elements
// ----------------------------------------------------------------------------
{
    object_g prg    = prgobj;
    size_t   depth  = rt.depth();
    object_g result = nullptr;
    for (object_p obj : *this)
    {
        if (!rt.push(obj))
            goto error;
        if (!result)
        {
            result = obj;
        }
        else
        {
            if (program::run(prg, true) != OK)
                goto error;
            if (rt.depth() != depth + 1)
                rt.misbehaving_program_error();
            result = rt.top();
        }
        if (rt.error())
            goto error;
    }
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return result;

error:
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return nullptr;
}


list_p list::filter(object_p prgobj) const
// ----------------------------------------------------------------------------
//   Apply an RPL object (nominally a program) to filter elements in a list
// ----------------------------------------------------------------------------
{
    id       ty    = type();
    object_g prg   = prgobj;
    size_t   depth = rt.depth();
    scribble scr;
    for (object_g obj : *this)
    {
        id   oty  = obj->type();
        bool keep = false;
        if (is_array_or_list(oty))
        {
            object_g sub = list_p(+obj)->filter(prg);
            obj = +sub;
            keep = true;
        }
        else
        {
            if (!rt.push(obj))
                goto error;
            if (program::run(prg, true) != OK)
                goto error;
            if (rt.depth() != depth + 1)
            {
                rt.misbehaving_program_error();
                goto error;
            }
            object_p test = rt.pop();
            keep = test->as_truth(true);
            if (rt.error())
                goto error;
        }

        if (keep && !rt.append(obj))
            goto error;
    }

    return list::make(ty, scr.scratch(), scr.growth());

error:
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return nullptr;
}


list_p list::pair_map(object_p prgobj) const
// ----------------------------------------------------------------------------
//   Apply an RPL object (nominally a program) to combine successive elements
// ----------------------------------------------------------------------------
{
    id       ty    = type();
    object_g prg   = prgobj;
    size_t   depth = rt.depth();
    object_g prev;
    scribble scr;
    for (object_g obj : *this)
    {
        if (prev)
        {
            if (!rt.push(obj) || !rt.push(prev))
                goto error;
            if (program::run(prg, true) != OK)
                goto error;
            if (rt.depth() != depth + 1)
            {
                rt.misbehaving_program_error();
                goto error;
            }
            object_p item   = rt.pop();
            if (!rt.append(item))
                goto error;
        }
        prev = obj;
    }

    return list::make(ty, scr.scratch(), scr.growth());

error:
    if (rt.depth() > depth)
        rt.drop(rt.depth() - depth);
    return nullptr;
}


list_p list::map(algebraic_fn fn) const
// ----------------------------------------------------------------------------
//   Apply an algebraic function on all elements in the list
// ----------------------------------------------------------------------------
{
    id ty = type();
    scribble scr;
    for (object_p obj : *this)
    {
        id oty = obj->type();
        if (is_array_or_list(oty))
        {
            list_g sub = list_p(obj)->map(fn);
            obj = +sub;
        }
        else
        {
            algebraic_g a = obj->as_algebraic();
            if (!a)
            {
                rt.type_error();
                return nullptr;
            }

            a = fn(a);
            if (!a)
                return nullptr;
            obj = +a;
        }

        if (!rt.append(obj))
            return nullptr;
    }

    return list::make(ty, scr.scratch(), scr.growth());
}


list_p list::map(arithmetic_fn fn, algebraic_r y) const
// ----------------------------------------------------------------------------
//   Right-apply an arithmtic function on all elements in the list
// ----------------------------------------------------------------------------
{
    id ty = type();
    scribble scr;
    for (object_p obj : *this)
    {
        id oty = obj->type();
        if (is_array_or_list(oty))
        {
            list_g sub = list_p(obj)->map(fn, y);
            obj = +sub;
        }
        else
        {
            algebraic_g a = obj->as_algebraic();
            if (!a)
            {
                rt.type_error();
                return nullptr;
            }

            a = fn(a, y);
            if (!a)
                return nullptr;
            obj = +a;
        }

        if (!rt.append(obj))
            return nullptr;
    }

    return list::make(ty, scr.scratch(), scr.growth());
}


list_p list::map(algebraic_r x, arithmetic_fn fn) const
// ----------------------------------------------------------------------------
//   Left-apply an arithmtic function on all elements in the list
// ----------------------------------------------------------------------------
{
    id ty = type();
    scribble scr;
    for (object_p obj : *this)
    {
        id oty = obj->type();
        if (is_array_or_list(oty))
        {
            list_g sub = list_p(obj)->map(x, fn);
            obj = +sub;
            if (!obj)
                return nullptr;
        }
        else
        {
            algebraic_g a = obj->as_algebraic();
            if (!a)
            {
                rt.type_error();
                return nullptr;
            }

            a = fn(x, a);
            if (!a)
                return nullptr;
            obj = +a;
        }

        if (!rt.append(obj))
            return nullptr;
    }

    return list::make(ty, scr.scratch(), scr.growth());
}



// ============================================================================
//
//   Sorting
//
// ============================================================================

int memory_compare(object_p *xp, object_p *yp)
// ----------------------------------------------------------------------------
//   Compare using type IDs and memory content
// ----------------------------------------------------------------------------
{
    object_p x = *xp;
    object_p y = *yp;
    return x->compare_to(y);
}


int value_compare(object_p *xp, object_p *yp)
// ----------------------------------------------------------------------------
//   Sort items according to value
// ----------------------------------------------------------------------------
{
    object_p x = *xp;
    object_p y = *yp;
    object::id xty = x->type();
    object::id yty = y->type();
    if ((object::is_algebraic(xty) && object::is_algebraic(yty)) ||
        (xty == object::ID_array   && yty == object::ID_array) ||
        (xty == object::ID_list    && yty == object::ID_list))
    {
        algebraic_g xa     = algebraic_p(x);
        algebraic_g ya     = algebraic_p(y);
        int         result = 0;
        if (comparison::compare(&result, xa, ya))
            return result;
    }
    return x->compare_to(y);
}


static int value_compare_reverse(object_p *xp, object_p *yp)
// ----------------------------------------------------------------------------
//   Sort item according in decreasing value order
// ----------------------------------------------------------------------------
{
    return -value_compare(xp, yp);
}


static int memory_compare_reverse(object_p *xp, object_p *yp)
// ----------------------------------------------------------------------------
//   Sort item according in decreasing value order
// ----------------------------------------------------------------------------
{
    return -memory_compare(xp, yp);
}


static object::result do_sort(int (*compare)(object_p *x, object_p *y))
// ----------------------------------------------------------------------------
//   RPL command for a sort
// ----------------------------------------------------------------------------
{
    if  (object_p obj = rt.stack(0))
    {
        if (list_p items = obj->as_array_or_list())
        {
            items = items->sort(compare);
            return items && rt.top(+items) ? object::OK : object::ERROR;
        }
        else
        {
            rt.type_error();
        }
    }
    return object::ERROR;
}


COMMAND_BODY(Sort)
// ----------------------------------------------------------------------------
//   Sort contents of a list according to value
// ----------------------------------------------------------------------------
{
    return do_sort(value_compare);
}


COMMAND_BODY(QuickSort)
// ----------------------------------------------------------------------------
//   Sort contents of a list using memory comparisons
// ----------------------------------------------------------------------------
{
    return do_sort(memory_compare);
}


COMMAND_BODY(ReverseSort)
// ----------------------------------------------------------------------------
//   Sort contents of a list according to value
// ----------------------------------------------------------------------------
{
    return do_sort(value_compare_reverse);
}


COMMAND_BODY(ReverseQuickSort)
// ----------------------------------------------------------------------------
//   Sort contents of a list using memory comparisons
// ----------------------------------------------------------------------------
{
    return do_sort(memory_compare_reverse);
}


COMMAND_BODY(ReverseList)
// ----------------------------------------------------------------------------
//   Reverse a list
// ----------------------------------------------------------------------------
{
    return do_sort(nullptr);
}



// ============================================================================
//
//   Enumeration of names in an expression, a program or a list
//
// ============================================================================

bool list::names_enumerate(size_t depth, bool units, list_g &forbidden) const
// ----------------------------------------------------------------------------
//   Enumerate the names in the object
// ----------------------------------------------------------------------------
{
    save<list_g> sforbidden(forbidden, forbidden);

    switch(type())
    {
    case ID_expression:
    case ID_funcall:
    case ID_program:
    case ID_list:
    case ID_block:
    case ID_array:
    {
        list_g   lst = this;
        size_t   max = items();
        size_t   pos[max];
        size_t   local = 0;
        size_t   arg   = 0;
        size_t   nargs = 0;
        size_t   vars  = 0;
        object_g deferred[4];
        bool     clear = false;

        for (auto obj : *this)
            pos[vars++] = byte_p(obj) - byte_p(+lst);
        ASSERT(vars == max || "Incorrect count of objects in list");
        for (size_t i = 0; i < max; i++)
        {
            object_p obj = object_p(byte_p(+lst) + pos[max + ~i]);
            if (local)
            {
                --local;
                if (!local)
                {
                    symbol_g lsym = obj->as_quoted<symbol>();
                    if (!lsym)
                        return false;
                    if (forbidden)
                        forbidden = forbidden->append(+lsym);
                    else
                        forbidden = make(lsym);
                }
            }
            if (nargs)
            {
                if (arg < nargs)
                    deferred[arg++] = obj;
                if (arg < nargs)
                {
                    obj = nullptr;
                }
                else
                {
                    nargs = 0;
                    arg--;
                }
            }

            while (obj)
            {
                switch(obj->type())
                {
                case ID_symbol:
                    if (!names_insert(depth, symbol_p(obj), nullptr, forbidden))
                        return false;
                    break;
                case ID_expression:
                case ID_funcall:
                case ID_polynomial:
                    if (!expression_p(obj)->names_enumerate(depth,
                                                            units,
                                                            forbidden))
                        return false;
                    break;
                case ID_unit:
                    if (algebraic_p value = unit_p(obj)->value())
                        if (symbol_p sym = value->as_quoted<symbol>())
                            if (!names_insert(depth,
                                              sym,
                                              units ? unit_p(obj) : nullptr,
                                              forbidden))
                                return false;
                    break;
                case ID_Sum:
                case ID_Product:
                    local = 4;
                    nargs = obj->arity();
                    break;
                case ID_Isolate:
                case ID_Root:
                    local = 1;
                    nargs = obj->arity();
                    break;
                case ID_Integrate:
                    local = 1;
                    nargs = obj->arity();
                    break;
                case ID_Derivative:
                case ID_Primitive:
                    local = 1;
                    nargs = obj->arity();
                    break;
                default:
                    break;
                }
                if (arg && !nargs)
                {
                    obj = deferred[--arg];
                    if (!arg)
                        clear = true;
                }
                else
                {
                    obj = nullptr;
                }
            }
            if (clear)
                forbidden = sforbidden.saved;
        }
        break;
    }
    case ID_polynomial:
    {
        polynomial_g p    = polynomial_p(this);
        size_t       vars = p->variables();
        for (size_t v = 0; v < vars; v++)
            if (!names_insert(depth, p->variable(v), nullptr, forbidden))
                return false;
        break;
    }
    default:
        break;
    }
    return true;
}


bool list::names_insert(size_t   depth,
                        symbol_p sym,
                        unit_p   uobj,
                        list_g  &forbidden)
// ----------------------------------------------------------------------------
//   Insert a name according to LNAME sorting order
// ----------------------------------------------------------------------------
{
    if (+forbidden)
        for (object_p obj : *forbidden)
            if (symbol_p unwanted = obj->as<symbol>())
                if (unwanted->is_same_as(sym))
                    return true;

    size_t existing = rt.depth() - depth;
    size_t len      = 0;
    utf8   name     = sym->value(&len);
    int    cmp      = -1;
    size_t level;
    for (level = 0; level < existing; level++)
    {
        object_p oobj  = rt.stack(level);
        unit_p   ounit = unit::get(oobj);
        symbol_p other = ounit ? symbol_p(ounit->value()) : symbol_p(oobj);
        size_t   olen  = 0;
        utf8     oname = other->value(&olen);
        cmp = olen - len;
        if (!cmp)
        {
            cmp = symbol::compare(name, oname, olen);

            // If one iteration has no unit and we add it afterwards...
            if (cmp == 0 && !ounit && uobj)
                rt.stack(level, uobj);
        }
        if (cmp <= 0)
            break;
    }
    if (cmp)
    {
        symbol_g s = sym;
        unit_g   u = uobj;
        if (!rt.push(+sym))
            return false;
        for (size_t i = 0; i < level; i++)
            if (!rt.stack(i, rt.stack(i+1)))
                return false;
        if (!rt.stack(level, uobj ? object_p(+u) : object_p(+s)))
            return false;
    }
    return true;
}


list_p list::names(bool units, id type) const
// ----------------------------------------------------------------------------
//   Return the list of names in an expression
// ----------------------------------------------------------------------------
//   The sorting order defined for `LNAME` in HP50G manual is:
//   - Longer variables first
//   - For same size, alphabetic order
{
    size_t   depth = rt.depth();
    list_g   forbidden;
    if (names_enumerate(depth, units, forbidden))
    {
        // Copy the items to the list
        scribble scr;
        while (rt.depth() > depth)
        {
            object_g obj = rt.pop();
            if (!rt.append(obj))
                goto error;
        }

        return list::make(type, scr.scratch(), scr.growth());
    }

error:
    if (size_t now = rt.depth())
        if (now > depth)
            rt.drop(now - depth);
    return nullptr;
}


static list_p list_variables(object_p obj, object::id type, bool units)
// ----------------------------------------------------------------------------
//   List all variables in an expressions
// ----------------------------------------------------------------------------
{
    object::id objty = obj->type();

    if (objty == object::ID_expression ||
        objty == object::ID_funcall    ||
        objty == object::ID_polynomial ||
        objty == object::ID_program    ||
        objty == object::ID_block      ||
        objty == object::ID_list       ||
        objty == object::ID_array)
        return list_p(obj)->names(units, type);

    rt.type_error();
    return nullptr;
}


COMMAND_BODY(XVars)
// ----------------------------------------------------------------------------
//   List all variable names in an array, consume input
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (list_p result = list_variables(obj, ID_list, true))
            if (rt.top(result))
                return OK;
    return ERROR;
}


COMMAND_BODY(LName)
// ----------------------------------------------------------------------------
//   List variable names in expression as an array, leave expression in place
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (list_p result = list_variables(obj, ID_array, false))
            if (rt.push(result))
                return OK;
    return ERROR;
}



// ============================================================================
//
//   DoList and DoSubs commands
//
// ============================================================================

static size_t program_arity(object_p prg)
// ----------------------------------------------------------------------------
//   Compute the program arity for a given program
// ----------------------------------------------------------------------------
{
    if (!prg)
        return -1;
    object::id id = prg->type();
    if (id == object::ID_list || id == object::ID_program)
    {
        if (list_p(prg)->items() != 1)
        {
            rt.argument_count_error();
            return 0;
        }
        prg = list_p(prg)->at(0);
    }
    if (locals_p locs = prg->as<locals>())
        return locs->variables();

    return prg->arity();
}


COMMAND_BODY(DoList)
// ----------------------------------------------------------------------------
//   Apply commands to lists
// ----------------------------------------------------------------------------
{
    if (object_g prg = rt.stack(0))
    {
        if (object_p cntobj = rt.stack(1))
        {
            size_t count = 0;
            size_t base  = 0;
            if (cntobj->is_real())
            {
                count = cntobj->as_uint32(0, true);
                base = 2;
            }
            else if (cntobj->type() == ID_list)
            {
                base = 1;
                count = program_arity(prg);
            }
            else
            {
                rt.type_error();
            }
            if (rt.error())
                return ERROR;
            if (!rt.args(base + count))
                return ERROR;

            // Check consistency of list dimensions
            size_t length = 0;
            id lty = ID_list;
            for (size_t d = 0; d < count; d++)
            {
                object_p obj = rt.stack(base + d);
                if (!obj)
                    return ERROR;
                id ty = obj->type();
                if (!is_array_or_list(ty))
                {
                    rt.type_error();
                    return ERROR;
                }
                list_p lst = list_p(obj);
                if (!d)
                {
                    length = lst->items();
                    lty = ty;
                }
                else if (length != lst->items())
                {
                    rt.dimension_error();
                    return ERROR;
                }
                else if (ty != lty)
                {
                    rt.type_error();
                    return ERROR;
                }
            }

            // Run program on all the lists
            scribble scr;
            size_t depth = rt.depth();
            for (size_t i = 0; i < length; i++)
            {
                size_t offs = base + count - 1;
                for (size_t d = count; d --> 0; )
                {
                    list_p lst = list_p(rt.stack(offs));
                    if (!rt.push(lst->at(i)))
                        return ERROR;
                }

                program::run(prg, true);

                size_t after = rt.depth();
                if (after < depth)
                {
                    rt.missing_argument_error();
                    return ERROR;
                }

                size_t added = after - depth;
                for (size_t d = added; d --> 0; )
                {
                    object_g obj = rt.stack(d);
                    if (!rt.append(obj))
                        return ERROR;
                }
                rt.drop(added);
            }

            list_p result =  list::make(lty, scr.scratch(), scr.growth());
            if (rt.drop(base + count) && rt.push(result))
                return OK;
        }
    }

    return ERROR;
}


static size_t nsub = 0;
static size_t endsub = 0;


COMMAND_BODY(DoSubs)
// ----------------------------------------------------------------------------
//   Apply command to sublist
// ----------------------------------------------------------------------------
{
    if (object_g prg = rt.stack(0))
    {
        if (object_p cntobj = rt.stack(1))
        {
            size_t count = 0;
            size_t base  = 0;
            if (cntobj->is_real())
            {
                count = cntobj->as_uint32(0, true);
                base = 2;
            }
            else if (cntobj->type() == ID_list)
            {
                base = 1;
                count = program_arity(prg);
            }
            else
            {
                rt.type_error();
            }
            if (rt.error())
                return ERROR;
            if (!rt.args(base))
                return ERROR;

            // Check consistency of list dimensions
            object_p obj = rt.stack(base);
            if (!obj)
                return ERROR;
            id lty = obj->type();
            if (!is_array_or_list(lty))
                return ERROR;
            list_g lst = list_p(obj);
            size_t length = lst->items();
            if (count > length)
            {
                rt.dimension_error();
                return ERROR;
            }

            save<size_t> sns(nsub, 0);
            save<size_t> ses(endsub, length - count + 1);
            rt.drop(base + 1);

            // Run program on all subs in the list
            scribble scr;
            size_t depth = rt.depth();
            for (size_t i = 0; i < endsub; i++)
            {
                nsub = i + 1;
                for (size_t d = 0; d < count; d++)
                    if (!rt.push(lst->at(i + d)))
                        return ERROR;

                program::run(prg, true);

                size_t after = rt.depth();
                if (after < depth)
                {
                    rt.missing_argument_error();
                    return ERROR;
                }

                size_t added = after - depth;
                for (size_t d = added; d --> 0; )
                {
                    object_g obj = rt.stack(d);
                    if (!rt.append(obj))
                        return ERROR;
                }
                rt.drop(added);
            }

            list_p result =  list::make(lty, scr.scratch(), scr.growth());
            if (rt.push(result))
                return OK;
        }
    }

    return ERROR;
}


COMMAND_BODY(NSub)
// ----------------------------------------------------------------------------
//   Return current index in a DoSubs command
// ----------------------------------------------------------------------------
{
    if (!endsub)
        rt.undefined_local_name_error();
    else if (integer_p obj = integer::make(nsub))
        if (rt.push(obj))
            return OK;
    return ERROR;
}


COMMAND_BODY(EndSub)
// ----------------------------------------------------------------------------
//   Return last index in a DoSubs command
// ----------------------------------------------------------------------------
{
    if (!endsub)
        rt.undefined_local_name_error();
    else if (integer_p obj = integer::make(endsub))
        if (rt.push(obj))
            return OK;
    return ERROR;
}



// ============================================================================
//
//   List element substitution
//
// ============================================================================

algebraic_p list::where(algebraic_r expr, algebraic_r args)
// ----------------------------------------------------------------------------
//   Implementation of the "where" command
// ----------------------------------------------------------------------------
{
    return algebraic_p(substitute(+expr, +args));
}


object_p list::substitute(object_p source, object_p args)
// ----------------------------------------------------------------------------
//   Implementation of the `where` command (input may not be algebraic)
// ----------------------------------------------------------------------------
{
    id patty = source->type();
    if (patty == ID_expression)
    {
        object_g ao = args;
        return expression_p(+source)->substitute(ao);
    }
    else if (is_real(patty) || is_complex(patty))
    {
        return source;
    }
    else if (is_array_or_list(patty))
    {
        algebraic_g aa = algebraic_p(args);
        return list_p(+source)->map(list::where, aa);
    }
    else if (patty == ID_equation || patty == ID_xlib || patty == ID_constant)
    {
        object_p value = constant_p(source)->value();
        return substitute(value, args);
    }
    return nullptr;
}


list_p list::substitute(symbol_r name, object_r replobj, size_t replsz) const
// ----------------------------------------------------------------------------
//  Substitute a single name with some other object
// ----------------------------------------------------------------------------
{
    scribble scr;
    id ltype = type();
    for (object_p obj : *this)
    {
        object_p tobj = obj;
        unit_p   uobj = obj->as<unit>();
        if (uobj)
            tobj = uobj->value();
        symbol_p oname = tobj->as<symbol>();
        if (oname && name->is_same_as(oname))
        {
            if (uobj)
            {
                // We map a name like (M_kg) with a value like 100_g
                // Check that we can convert units

                // In that case, the replacement needs to be algebraic
                algebraic_g arepl = replobj->as_algebraic();
                if (!arepl)
                {
                    rt.type_error();
                    return nullptr;
                }

                algebraic_g one = integer::make(1);
                unit_g fromu = unit::make(one, uobj->uexpr());
                if (!fromu->convert(arepl))
                {
                    rt.inconsistent_units_error();
                    return nullptr;
                }
                if (!arepl || !rt.append(arepl, arepl->size()))
                    return nullptr;
            }
            else if (!rt.append(replobj, replsz))
                return nullptr;
        }
        else
        {
            if (!rt.append(obj))
                return nullptr;
        }
    }
    list_p result = list::make(ltype, scr.scratch(), scr.growth());
    return result;
}


list_p list::substitute(symbol_r name, object_p replobj) const
// ----------------------------------------------------------------------------
//  Substitute a name with an object, which can be an expression
// ----------------------------------------------------------------------------
{
    if (expression_p expr = replobj->as<expression>())
    {
        size_t sz = 0;
        object_g obj = expr->objects(&sz);
        return substitute(name, obj, sz);
    }
    return substitute(name, replobj, replobj->size());
}


list_p list::substitute(expression_r assign) const
// ----------------------------------------------------------------------------
//   Perform substitution with an input that looks like A=B
// ----------------------------------------------------------------------------
{
    list_g       l = this;
    expression_g from, to;
    if (assign->split_equation(from, to))
        if (symbol_g name = from->as_quoted<symbol>())
            return l->substitute(name, (object_g) to);
    if (!rt.error())
        rt.value_error();
    return nullptr;
}


list_p list::substitute(list_r assignments) const
// ----------------------------------------------------------------------------
//   Substitute a list of assignments
// ----------------------------------------------------------------------------
{
    list_g result = this;
    symbol_g name;
    for (object_g obj : *assignments)
    {
        if (name)
        {
            result = result->substitute(name, obj);
            name = nullptr;
        }
        else if (symbol_p n = obj->as<symbol>())
        {
            name = n;
        }
        else
        {
            result = result->substitute(obj);
        }
    }
    return result;
}


list_p list::substitute(object_r repl) const
// ----------------------------------------------------------------------------
//   Check if we have an expression or a list as an argument
// ----------------------------------------------------------------------------
{
    id rty = repl->type();
    if (is_array_or_list(rty))
    {
        list_g rlist = list_p(+repl);
        return substitute(rlist);
    }
    if (rty == ID_expression)
    {
        expression_g expr = expression_p(+repl);
        return substitute(expr);
    }
    return this;
}


static list_p extract_sublist(list_r data, uint level, uint depth,
                              size_t fidx[], size_t lidx[])
// ----------------------------------------------------------------------------
//   Extract a rectangular sublist
// ----------------------------------------------------------------------------
{
    object::id ty = data->type();
    scribble   scr;
    for (size_t i = fidx[level]; i <= lidx[level]; i++)
    {
        object_p item = data->at(i);
        if (!item)
            break;
        if (level + 1 < depth)
        {
            if (list_g sublist = item->as_array_or_list())
            {
                item = extract_sublist(sublist, level+1, depth, fidx, lidx);
                if (!item)
                    return nullptr;
            }
            else
            {
                rt.dimension_error();
                return nullptr;
            }
        }
        rt.append(item);
    }

    list_p result = list::make(ty, scr.scratch(), scr.growth());
    return result;

}


list_p list::extract(object_r &first, object_r &last) const
// ----------------------------------------------------------------------------
//   Extract a sublist
// ----------------------------------------------------------------------------
{
    list_g   data = this;
    list_g   flst = first->as_array_or_list();
    list_g   llst = last->as_array_or_list();
    size_t   fnum = flst ? flst->items() : 1;
    size_t   lnum = llst ? llst->items() : 1;
    if (fnum != lnum)
    {
        rt.dimension_error();
        return nullptr;
    }

    size_t fidx[fnum];
    size_t lidx[fnum];
    for (size_t i = 0; i < fnum; i++)
    {
        int idx = (flst ? flst->at(i) : +first)->as_int32(1, true);
        if (--idx < 0)
            idx = 0;
        fidx[i] = idx;
        if (rt.error())
            return nullptr;
        idx = (llst ? llst->at(i) : +last)->as_int32(1, true);
        if (--idx < 0)
            idx = 0;
        lidx[i] = idx;
        if (rt.error())
            return nullptr;
    }

    return extract_sublist(data, 0, fnum, fidx, lidx);
}


symbol_p list::contains(symbol_p vsym) const
// ----------------------------------------------------------------------------
//   Check if the list contains a given symbol, if so return it
// ----------------------------------------------------------------------------
{
    for (object_p obj : *this)
        if (symbol_p known = obj->as_quoted<symbol>())
            if (known->is_same_as(vsym))
                return known;
    return nullptr;
}


object_p list::row(size_t index) const
// ----------------------------------------------------------------------------
//   Return the given row or item
// ----------------------------------------------------------------------------
{
    object_p result = nullptr;
    bool     vector = true;
    for (object_p obj : *this)
    {
        if (obj->is_array_or_list())
            vector = false;
        if (index-- == 0)
            result = obj;
    }
    return vector ? result : result->as_array_or_list();
}


object_p list::column(size_t index) const
// ----------------------------------------------------------------------------
//   Return the given column or item
// ----------------------------------------------------------------------------
{
    object_p result = nullptr;
    bool     vector = true;
    size_t   colidx = index;
    for (object_p obj : *this)
    {
        if (obj->is_array_or_list())
            vector = false;
        if (index-- == 0)
            result = obj;
    }
    // If processing a vector, return what we found
    if (vector)
        return result;

    // For more than one dimension, need a list at every level
    scribble scr;
    id ty = type();
    for (object_p obj : *this)
    {
        list_p row = obj->as_array_or_list();
        if (!row)
            return nullptr;
        object_p item = row->at(colidx);
        if (!item || !rt.append(item))
            return nullptr;
    }
    return list::make(ty, scr.scratch(), scr.growth());
}


// Get a sorted list
list_p list::sort() const
// ----------------------------------------------------------------------------
//   Return a sorted list based on value-compare
// ----------------------------------------------------------------------------
{
    return sort(value_compare);
}


list_p list::sort(int (*compare)(object_p *x, object_p *y)) const
// ----------------------------------------------------------------------------
//  Return a sorted list based on the given comparison function
// ----------------------------------------------------------------------------
{
    typedef int (*qsort_fn)(const void *, const void*);
    qsort_fn            cmp = qsort_fn(compare);
    id                  ty  = type();
    stack_depth_restore sdr;
    for (object_p item : *this)
        if (!rt.push(item))
            return nullptr;

    size_t count = sdr.count();
    if (cmp)
        qsort(rt.stack_base(), count, sizeof(object_p), cmp);

    scribble scr;
    for (size_t i = 0; i < count; i++)
        if (object_g obj = rt.stack(i))
            if (!rt.append(obj))
                return nullptr;
    rt.drop(count);
    return list::make(ty, scr.scratch(), scr.growth());
}
