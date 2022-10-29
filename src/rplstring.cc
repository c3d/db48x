// ****************************************************************************
//  rplstring.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of basic string operations
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

#include "rplstring.h"

#include "parser.h"
#include "renderer.h"
#include "runtime.h"

#include <stdio.h>


OBJECT_HANDLER_BODY(string)
// ----------------------------------------------------------------------------
//    Handle commands for strings
// ----------------------------------------------------------------------------
{
    switch(op)
    {
    case EVAL:
        // String values evaluate as self
        rt.push(obj);
        return OK;
    case SIZE:
        return size(obj, payload);
    case PARSE:
        return object_parser(OBJECT_PARSER_ARG(), rt);
    case RENDER:
        return obj->object_renderer(OBJECT_RENDERER_ARG(), rt);
    case HELP:
        return (intptr_t) "text";

    default:
        // Check if anyone else knows how to deal with it
        return DELEGATE(object);
    }

}


OBJECT_PARSER_BODY(string)
// ----------------------------------------------------------------------------
//    Try to parse this as an string
// ----------------------------------------------------------------------------
//    For simplicity, this deals with all kinds of strings
{
    utf8 source = p.source;
    utf8 s      = source;
    if (*s++ != '"')
        return SKIP;

    utf8 end = source + p.length;
    while (s < end && *s != '"')
        s++;

    if (*s != '"')
    {
        rt.error("Invalid string", s);
        return ERROR;
    }
    s++;

    size_t parsed = s - source;
    size_t slen   = parsed - 2;
    gcutf8 text   = source + 1;
    p.end         = parsed;
    p.out         = rt.make<string>(ID_string, text, slen);

    return OK;
}


OBJECT_RENDERER_BODY(string)
// ----------------------------------------------------------------------------
//   Render the string into the given string buffer
// ----------------------------------------------------------------------------
{
    size_t  len = 0;
    utf8 txt = text(&len);
    return snprintf(r.target, r.length, "\"%.*s\"", (int) len, txt);
}


string_g operator+(string_g x, string_g y)
// ----------------------------------------------------------------------------
//   Concatenate two strings
// ----------------------------------------------------------------------------
{
    runtime &rt = runtime::RT;
    size_t sx = 0, sy = 0;
    utf8 tx = x->text(&sx);
    utf8 ty = y->text(&sy);
    string_g concat = rt.make<string>(string::ID_string, tx, sx + sy);
    if (concat)
    {
        utf8 tc = concat->text();
        memcpy((byte *) tc + sx, (byte *) ty, sy);
    }
    return concat;
}


string_g operator*(string_g x, uint y)
// ----------------------------------------------------------------------------
//    Repeat the string a given number of times
// ----------------------------------------------------------------------------
{
    runtime &rt = runtime::RT;
    string_g result = rt.make<string>(string::ID_string, x->text(), 0);
    while (y)
    {
        if (y & 1)
            result = result + x;
        if (!result)
            break;
        y /= 2;
        if (y)
            x = x + x;
    }
    return result;
}
