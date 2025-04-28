// ****************************************************************************
//  text.cc                                                    DB48X project
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

#include "text.h"

#include "grob.h"
#include "integer.h"
#include "parser.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "utf8.h"

#include <stdio.h>


SIZE_BODY(text)
// ----------------------------------------------------------------------------
//   Compute the size of a text (and all objects with a size at beginning)
// ----------------------------------------------------------------------------
{
    byte_p p = o->payload();
    size_t sz = leb128<size_t>(p);
    p += sz;
    return ptrdiff(p, o);
}


PARSE_BODY(text)
// ----------------------------------------------------------------------------
//    Try to parse this as a text
// ----------------------------------------------------------------------------
//    For simplicity, this deals with all kinds of texts
{
    utf8 source = p.source;
    utf8 s      = source;
    if (*s++ != '"')
        return SKIP;

    utf8   end    = source + p.length;
    size_t quotes = 0;
    bool   ok     = false;
    while (s < end)
    {
        if (*s++ == '"')
        {
            if (s >= end || *s != '"')
            {
                ok = true;
                break;
            }
            s++;
            quotes++;
        }
    }

    if (!ok)
    {
        rt.unterminated_error().source(p.source);
        return ERROR;
    }

    size_t parsed = s - source;
    size_t slen   = parsed - 2;
    gcutf8 txt    = source + 1;
    p.length      = parsed;
    p.out         = rt.make<text>(ID_text, txt, slen, quotes);

    return p.out ? OK : ERROR;
}


RENDER_BODY(text)
// ----------------------------------------------------------------------------
//   Render the text into the given text buffer
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    gcutf8 txt = o->value(&len);
    size_t off = 0;
    r.put('"');
    while (off < len)
    {
        unicode c = utf8_codepoint(txt + off);
        if (c == '"')
            r.put('"');
        r.put(c);
        off += utf8_size(c);
    }
    r.put('"');
    return r.size();
}


text_g operator+(text_r x, text_r y)
// ----------------------------------------------------------------------------
//   Concatenate two texts or lists
// ----------------------------------------------------------------------------
{
    if (!x)
        return y;
    if (!y)
        return x;
    object::id type = x->type();
    size_t sx = 0, sy = 0;
    utf8 tx = x->value(&sx);
    utf8 ty = y->value(&sy);
    text_g concat = rt.make<text>(type, tx, sx + sy);
    if (concat)
    {
        utf8 tc = concat->value();
        memcpy((byte *) tc + sx, (byte *) ty, sy);
    }
    return concat;
}


text_g operator*(text_r xr, uint y)
// ----------------------------------------------------------------------------
//    Repeat the text a given number of times
// ----------------------------------------------------------------------------
{
    text_g result = rt.make<text>(xr->type(), xr->value(), 0);
    text_g x = xr;
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


size_t text::utf8_characters() const
// ----------------------------------------------------------------------------
//   Count number of utf8 characters (for the `Size` command
// ----------------------------------------------------------------------------
{
    byte_p p     = payload(this);
    size_t len   = leb128<size_t>(p);
    utf8   start = utf8(p);
    utf8   end   = start + len;
    size_t count = 0;
    for (utf8 s = start; s < end; s = utf8_next(s))
        count++;
    return count;
}


static cstring conversions[] =
// ----------------------------------------------------------------------------
//   Conversion from standard ASCII to HP-48 characters
// ----------------------------------------------------------------------------
{
    "<<", "«",
    ">>", "»",
    "->", "→"
};


text_p text::import() const
// ----------------------------------------------------------------------------
//    Convert text containing sequences such as -> or <<
// ----------------------------------------------------------------------------
{
    text_p   result = this;
    size_t   sz     = 0;
    gcutf8   txt    = value(&sz);
    size_t   rcount = sizeof(conversions) / sizeof(conversions[0]);
    gcmbytes replace;
    scribble scr;

    for (size_t o = 0; o < sz; o++)
    {
        bool replaced = false;
        for (uint r = 0; r < rcount && !replaced; r += 2)
        {
            size_t olen = strlen(conversions[r]);
            if (!strncmp(conversions[r], cstring(+txt) + o, olen))
            {
                size_t rlen = strlen(conversions[r+1]);
                if (!replace)
                {
                    replace = rt.allocate(o);
                    if (!replace)
                        return result;
                    memmove((byte *) +replace, +txt, o);
                }
                byte *cp = rt.allocate(rlen);
                if (!cp)
                    return result;
                memcpy(cp, conversions[r+1], rlen);
                replaced = true;
                o += olen-1;
            }
        }

        if (!replaced && replace)
        {
            byte *cp = rt.allocate(1);
            if (!cp)
                return result;
            *cp = utf8(txt)[o];
        }
    }

    if (replace)
        if (text_p ok = make(+replace, scr.growth()))
            result = ok;

    return result;
}


object_p text::compile() const
// ----------------------------------------------------------------------------
//   Compile and run the text as if on the command line
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8   txt = value(&len);
    size_t sz  = len;
    object_p result = object::parse(txt, len);
    if (len != sz)
        result = nullptr;
    return result;
}


bool text::compile_and_run() const
// ----------------------------------------------------------------------------
//   Compile and run the text as if on the command line
// ----------------------------------------------------------------------------
{
    size_t    len  = 0;
    utf8      txt  = value(&len);
    program_g cmds = program::parse(txt, len);
    if (cmds)
    {
        // We successfully parsed the line, execute it
        save<bool> no_halt(program::halted, false);
        return cmds->run(false) == OK;
    }
    return false;
}


static object::result to_unicode(bool (*body)(text_r tobj))
// ----------------------------------------------------------------------------
//   Convert a top-level text to a single character code or to a list
// ----------------------------------------------------------------------------
{
    if (text_g tobj = rt.top()->as<text>())
    {
        if (body(tobj))
            return object::OK;
    }
    else
    {
        rt.type_error();
    }
    return object::ERROR;
}


static bool to_unicode_char(text_r tobj)
// ----------------------------------------------------------------------------
//   Generate a single character on the stack
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8 first = tobj->value(&len);
    int code = len ? utf8_codepoint(first) : -1;
    integer_p icode = integer::make(code);
    return icode && rt.top(icode);
}


COMMAND_BODY(CharToUnicode)
// ----------------------------------------------------------------------------
//   Convert the first character in the string to an integer
// ----------------------------------------------------------------------------
{
    return to_unicode(to_unicode_char);
}


static bool to_unicode_list(text_r tobj)
// ----------------------------------------------------------------------------
//   Generate a single character on the stack
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    gcutf8 first = tobj->value(&len);
    list_g result = list::make(nullptr, 0);
    for (size_t o = 0; o < len; o = utf8_next(first, o, len))
    {
        unicode code = utf8_codepoint(first + o);
        integer_p icode = integer::make(code);
        if (!icode)
            return false;
        result = result->append(icode);
        if (!result)
            return false;
    }
    return result && rt.top(+result);
}


COMMAND_BODY(TextToUnicode)
// ----------------------------------------------------------------------------
//  Convert the text to a list of unicode code points
// ----------------------------------------------------------------------------
{
    return to_unicode(to_unicode_list);
}


static text_p unicode_to_text(object_p obj)
// ----------------------------------------------------------------------------
//   Convert an object to unicode
// ----------------------------------------------------------------------------
{
    int32_t code = obj->as_int32(-1, true);
    if (rt.error())
        return nullptr;

    byte buffer[4];
    size_t sz = code < 0 ? 0 : utf8_encode(unicode(code), buffer);
    text_p result = text::make(buffer, sz);
    return result;
}



COMMAND_BODY(UnicodeToText)
// ----------------------------------------------------------------------------
//   Convert a single integer to a one-character text, or a list to a text
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_g lobj = obj->as<list>())
    {
        text_g result = text::make("", 0);
        for (object_p iobj : *lobj)
        {
            text_g chr = unicode_to_text(iobj);
            if (!chr)
                return ERROR;
            result = result + chr;
        }
        if (result && rt.top(result))
            return OK;
    }
    else
    {
        text_p chr = unicode_to_text(obj);
        if (chr && rt.top(chr))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(Extract)
// ----------------------------------------------------------------------------
//   Extract a subtext from the main text (called "sub" on HP calculators)
// ----------------------------------------------------------------------------
{
    object_g value = rt.stack(2);
    object_g start = rt.stack(1);
    object_g end   = rt.stack(0);

    id ty = value->type();
    switch (ty)
    {
    case ID_text:
        value = text_p(+value)->extract(start, end);
        break;

    case ID_array:
    case ID_list:
        value = list_p(+value)->extract(start, end);
        break;

    case ID_grob:
#ifdef CONFIG_COLOR
    case ID_pixmap:
#endif
    case ID_bitmap:
    case ID_Pict:
        value = grob_p(+value)->extract(start, end);
        break;

    default:
        rt.type_error();
        return ERROR;
    }

    if (value && rt.drop(2) && rt.top(value))
        return OK;

    return ERROR;
}



text_p text::extract(object_r &first, object_r &last) const
// ----------------------------------------------------------------------------
//   Extract a subtext
// ----------------------------------------------------------------------------
{
    int fidx = first->as_int32(1, true);
    int lidx = last->as_int32(1, true);
    if (rt.error())
        return nullptr;
    if (--fidx < 0)
        fidx = 0;
    if (--lidx < 0)
        lidx = 0;
    return extract(fidx, lidx);
}


text_p text::extract(size_t first, size_t last) const
// ----------------------------------------------------------------------------
//   Extract a text from a given range
// ----------------------------------------------------------------------------
{
    iterator it(this, first);
    size_t fidx = it.index;
    while (first <= last)
    {
        first++;
        it++;
    }
    size_t len = it.index - fidx;
    gcutf8 txt = it.first + fidx;
    return text::make(txt, len);
}
