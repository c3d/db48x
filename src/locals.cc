// ****************************************************************************
//  locals.cc                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Block with local variables, e.g. `→ X Y « X Y - X Y + * »`
//
//     Local values reside above the stack. They are referenced by an index,
//     which makes them very efficient (there is no name lookup). Reading
//     or storing in a local variable is as efficient as accessing the stack.
//     This is much faster than global variables, which require a rather slow
//     linear name lookup and, when storing, moving the directory object.
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

#include "locals.h"

#include "expression.h"
#include "parser.h"
#include "program.h"
#include "renderer.h"
#include "symbol.h"
#include "utf8.h"

//#include <strings.h>


locals_stack *locals_stack::stack = nullptr;



// ============================================================================
//
//   Implementation of program with local variables
//
// ============================================================================

static inline bool is_program_separator(unicode cp)
// ----------------------------------------------------------------------------
//   Check if a given unicode character can begin a program object
// ----------------------------------------------------------------------------
{
    return cp == L'«'           // Program object
        || cp == '\''           // Equation
        || cp == '{';           // List
}


PARSE_BODY(locals)
// ----------------------------------------------------------------------------
//    Try to parse this as a block with locals
// ----------------------------------------------------------------------------
{
    gcutf8  s   = p.source;
    size_t  max = p.length;
    unicode cp  = utf8_codepoint(s);
    if (cp != L'→' && cp != L'▶')
        return SKIP;
    s = utf8_next(s);

    // Check that we have a space after that, could be →List otherwise
    cp = utf8_codepoint(s);
    if (!utf8_whitespace(cp))
        return SKIP;

    // Parse the names
    scribble scr;
    size_t   names  = 0;
    gcmbytes countp = rt.scratchpad();
    byte     encoding[4];

    while (utf8_more(p.source, s, max))
    {
        cp = utf8_codepoint(s);
        if (utf8_whitespace(cp))
        {
            s = utf8_next(s);
            continue;
        }
        if (is_program_separator(cp))
        {
            break;
        }
        if (!is_valid_as_name_initial(cp))
        {
            object_p cmd = symbol::make("Local variables block");
            rt.missing_variable_error().source(s).command(cmd);
            return ERROR;
        }

        // Allocate byte for name length
        gcmbytes lengthp = rt.scratchpad();
        size_t namelen = 0;
        while (is_valid_in_name(cp) && utf8_more(p.source, s, max))
        {
            size_t cplen = utf8_encode(cp, encoding);
            gcbytes namep = rt.allocate(cplen);
            if (!namep)
                return ERROR;
            memcpy(namep, encoding, cplen);
            namelen += cplen;
            s += cplen;
            cp = utf8_codepoint(s);
        }

        // Encode name
        size_t lsize = leb128size(namelen);
        gcbytes endp = rt.allocate(lsize);
        if (!endp)
            return ERROR;
        byte *lp = lengthp;
        memmove(lp + lsize, lp, namelen);
        leb128(lp, namelen);

        // Count names
        names++;
    }

    // If we did not get a program after the names, fail
    size_t decls = utf8(s) - utf8(p.source);
    if (!is_program_separator(cp) || p.length <= decls)
    {
        object_p cmd = static_object(ID_locals);
        rt.syntax_error().command(cmd).source(s);
        return ERROR;
    }

    // Encode number of names
    size_t csz  = leb128size(names);
    byte *end = rt.allocate(csz);
    if (!end)
        return ERROR;
    byte  *cntp = countp;
    size_t sz  = end - cntp;
    memmove(cntp + csz, cntp, sz);
    leb128(cntp, names);

    // Build the program with the context pointing to the names
    locals_stack frame((byte_p)countp);
    p.source += decls;
    p.length -= decls;

    object::result result = ERROR;
    switch(cp)
    {
    case L'«':  result = program   ::do_parse(p); break;
    case  '\'': result = expression::do_parse(p); break;
    case '{':   result = list      ::do_parse(p); break;
    default:                                    break;
    }
    if (result != OK)
        return result;

    // Copy the program to the scratchpad
    if (!rt.append(p.out))
        return ERROR;

    // Compute total number of bytes in payload and build object
    gcbytes scratch = scr.scratch();
    size_t alloc = scr.growth();
    p.out = rt.make<locals>(ID_locals, scratch, alloc);

    // Adjust size of parsed text for what we parsed before program
    p.source += -decls;
    p.length += decls;

    return OK;
}


RENDER_BODY(locals)
// ----------------------------------------------------------------------------
//   Render the program into the given program buffer
// ----------------------------------------------------------------------------
{
    // Skip object size
    gcbytes p       = o->payload();
    size_t  objsize = leb128<size_t>(+p);
    (void) objsize;

    // Create a local frame for rendering local names
    locals_stack frame(p);

    // Emit header
    r.wantCR();
    r.put("→ ");

    // Loop on names
    size_t names = leb128<size_t>(+p);
    for (size_t n = 0; n < names; n++)
    {
        size_t len = leb128<size_t>(+p);
        r.put(+p, len);
        r.put(n + 1 < names ? ' ' : '\n');
        p += len;
    }

    // Render object (which should be a program, an equation or a list)
    object_p obj = object_p(+p);
    return obj->render(r);
}


EVAL_BODY(locals)
// ----------------------------------------------------------------------------
//   Evaluate a program with locals (executes the code)
// ----------------------------------------------------------------------------
{
    object_g p   = object_p(o->payload());
    size_t   len = leb128<size_t>(+p);
    object_g end = p + len;

    // Copy local values from stack
    size_t names   = leb128<size_t>(+p);
    if (!rt.locals(names))
        return ERROR;
    if (!rt.run_push_data(nullptr, object_p(names)))
    {
        rt.unlocals(names);
        return ERROR;
    }

    // Skip names to get to program
    for (uint n = 0; n < names; n++)
    {
        size_t nlen;
        p = leb128(nlen, +p);
        p += nlen;
    }

    // Defer execution of body to the caller
    program_p prog = p->as_program();
    if (!prog)
    {
        rt.malformed_local_program_error();
        return ERROR;
    }
    if (!rt.run_push(prog->objects(), end))
        return ERROR;
    return OK;
}



// ============================================================================
//
//  Implementation of local name
//
// ============================================================================

SIZE_BODY(local)
// ----------------------------------------------------------------------------
//  Compute size for a local object
// ----------------------------------------------------------------------------
{
    byte_p p = o->payload();
    return ptrdiff(p, o) + leb128size(p);
}


PARSE_BODY(local)
// ----------------------------------------------------------------------------
//    Check if we have local names, and check if there is a match
// ----------------------------------------------------------------------------
{
    utf8 source = p.source;
    utf8 s      = source;

    // First character must be alphabetic
    unicode cp = utf8_codepoint(s);
    if (!is_valid_as_name_initial(cp))
        return SKIP;

    // Check what is acceptable in a name
    while (is_valid_in_name(s))
        s = utf8_next(s);
    size_t len = s - source;

    // Check all the locals currently in effect
    size_t index = 0;
    for (locals_stack *f = locals_stack::current(); f; f = f->enclosing())
    {
        // Need to null-check here because we create null locals parsing 'for'
        if (gcbytes names = f->names())
        {
            // Check if name is found in local frame
            size_t count = leb128<size_t>(+names);
            for (size_t n = 0; n < count; n++)
            {
                size_t nlen = leb128<size_t>(+names);
                if (nlen == len && symbol::compare(+names, source, nlen) == 0)
                {
                    // Found a local name, return it
                    gcutf8 text = source;
                    p.length    = len;
                    p.out       = rt.make<local>(ID_local, index);
                    return OK;
                }
                names += nlen;
                index++;
            }
        }
    }

    // Not found in locals, treat as a global name
    return SKIP;
}


RENDER_BODY(local)
// ----------------------------------------------------------------------------
//   Render a local name
// ----------------------------------------------------------------------------
{
    gcbytes p = o->payload();
    uint index = leb128<uint>(+p);

    for (locals_stack *f = locals_stack::current(); f; f = f->enclosing())
    {
        gcbytes names = f->names();

        // Check if name is found in local frame
        size_t count = leb128<size_t>(+names);
        if (index >= count)
        {
            // Name is beyond current frame, skip to next one
            index -= count;
            continue;
        }

        // Skip earlier names in index
        for (size_t n = 0; n < index; n++)
        {
            size_t len = leb128<size_t>(+names);
            names += len;
        }

        // Emit name and exit
        size_t len = leb128<size_t>(+names);
        r.put(+names, len);
        return r.size();
    }

    // We have not found the name, render bogus name
    r.printf("LocalVariable%u", index);
    return r.size();
}


EVAL_BODY(local)
// ----------------------------------------------------------------------------
//   Evaluate a local by fetching it from locals area and evaluating it
// ----------------------------------------------------------------------------
{
    if (object_p obj = o->recall())
        return program::run_program(obj);
    return ERROR;
}
