// ****************************************************************************
//  tag.cc                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Tag type
//
//     The tag type is used to tag objects
//     It otherwise evaluates and behaves like the tagged object
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

#include "tag.h"

#include "expression.h"
#include "grob.h"
#include "parser.h"
#include "renderer.h"


SIZE_BODY(tag)
// ----------------------------------------------------------------------------
//   Compute the size of a tag object
// ----------------------------------------------------------------------------
{
    byte_p p = o->payload();
    size_t sz = leb128<size_t>(p);
    p += sz;
    sz = object_p(p)->size();
    p += sz;
    return ptrdiff(p, o);
}


HELP_BODY(tag)
// ----------------------------------------------------------------------------
//   Help topic for tagged objects
// ----------------------------------------------------------------------------
{
    return utf8("Tagged objects");
}


PARSE_BODY(tag)
// ----------------------------------------------------------------------------
//    Try to parse this as a tag, i.e. :LABEL:Object
// ----------------------------------------------------------------------------
{
    utf8 source = p.source;
    utf8 s      = source;
    if (*s++ != ':')
        return SKIP;

    utf8 end = source + p.length;
    while (s < end && *s != ':')
        s++;

    if (*s != ':')
    {
        rt.unterminated_error().source(p.source);
        return ERROR;
    }
    s++;

    size_t parsed = s - source;
    size_t llen   = parsed - 2;
    gcutf8 lbl    = source + 1;

    size_t remaining = p.length - parsed;
    object_g obj = object::parse(s, remaining);
    if (!obj)
    {
        rt.unterminated_error();
        return ERROR;
    }

    p.length = parsed + remaining;
    p.out    = rt.make<tag>(ID_tag, lbl, llen, obj);

    return p.out ? OK : ERROR;
}


RENDER_BODY(tag)
// ----------------------------------------------------------------------------
//   Render the tag into the given text buffer
// ----------------------------------------------------------------------------
//   When rendering on the stack, we render as "LABEL:object"
//   Otherwise, we render as ":LABEL:object"
{
    size_t  llen = 0;
    utf8    ltxt = o->label_value(&llen);
    if (!r.stack())
        r.put(':');
    r.put(ltxt, llen);
    r.put(':');

    object_p obj = o->tagged_object();
    obj->render(r);

    return r.size();
}


GRAPH_BODY(tag)
// ----------------------------------------------------------------------------
//   Render tag in graphical form
// ----------------------------------------------------------------------------
{
    size_t   llen = 0;
    utf8     ltxt = o->label_value(&llen);
    object_p obj  = o->tagged_object();
    grob_g   go   = obj->graph(g);
    coord    vo   = g.voffset;
    symbol_g label = symbol::make(ltxt, llen);
    if (!label)
        return nullptr;
    auto fid = g.font;
    g.reduce_font();
    grob_g gl = object::do_graph(label, g);
    gl = expression::infix(g, 0, gl, 0, ":", vo, go);
    g.font = fid;
    return gl;
}


COMMAND_BODY(DTag)
// ----------------------------------------------------------------------------
//   Remove the tag from an object
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (tag_p tobj = obj->as<tag>())
        {
            do
            {
                obj = tobj->tagged_object();
            } while ((tobj = obj->as<tag>()));
            if (!rt.top(obj))
                return ERROR;
        }
        return OK;
    }
    return ERROR;
}


COMMAND_BODY(ToTag)
// ----------------------------------------------------------------------------
//   Build a tag object from a name and object
// ----------------------------------------------------------------------------
{
    if (object_g x = rt.stack(0))
    {
        if (object_g y = rt.stack(1))
        {
            while (tag_p tagged = x->as<tag>())
                x = tagged->tagged_object();

            if (text_g label = x->as_text())
            {
                size_t lsz = 0;
                utf8 ltxt = label->value(&lsz);
                tag_g tagged = tag::make(ltxt, lsz, y);
                if (tagged && rt.drop() && rt.top(tagged))
                    return OK;
            }
        }
    }
    return ERROR;
}


COMMAND_BODY(FromTag)
// ----------------------------------------------------------------------------
//   Expand a tagged object into its value and tag
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.top())
    {
        if (tag_g tagged = x->as<tag>())
        {
            if (rt.top(tagged->tagged_object()))
            {
                size_t lsz = 0;
                utf8 ltxt = tagged->label_value(&lsz);
                text_p label = text::make(ltxt, lsz);
                if (label && rt.push(label))
                    return OK;
            }
        }
    }

    return ERROR;
}
