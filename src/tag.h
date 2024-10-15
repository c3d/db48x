#ifndef TAG_H
#define TAG_H
// ****************************************************************************
//  tag.h                                                         DB48X project
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
#include "object.h"
#include "runtime.h"
#include "text.h"

GCP(tag);

struct tag : object
// ----------------------------------------------------------------------------
//   A tag type is just used to display a label along an object
// ----------------------------------------------------------------------------
{
    tag(id type, gcutf8 label, size_t len, object_g obj) : object(type)
    {
        utf8 s = (utf8) label;
        byte *p = (byte *) payload(this);
        p = leb128(p, len);
        while (len--)
            *p++ = *s++;
        len = obj->size();
        memmove(p, +obj, len);
    }

    static size_t required_memory(id i, gcutf8 UNUSED label, size_t len, object_g obj)
    {
        return leb128size(i) + leb128size(len) + len + obj->size();
    }

    static tag_p make(gcutf8 str, size_t len, object_g obj)
    {
        if (!obj)
            return nullptr;
        return rt.make<tag>(str, len, obj);
    }

    static tag_p make(cstring str, object_p obj)
    {
        return make(utf8(str), strlen(str), obj);
    }

    static tag_p make(text_p text, object_p obj)
    {
        size_t sz = 0;
        utf8 name = text->value(&sz);
        return make(name, sz, obj);
    }

    size_t label_length() const
    {
        byte_p p = payload(this);
        return leb128<size_t>(p);
    }

    utf8 label_value(size_t *size = nullptr) const
    {
        byte_p p   = payload(this);
        size_t len = leb128<size_t>(p);
        if (size)
            *size = len;
        return (utf8) p;
    }

    text_p label() const
    {
        size_t length = 0;
        utf8 lbl = label_value(&length);
        return text::make(lbl, length);
    }

    object_p tagged_object() const
    {
        byte_p p = payload(this);
        size_t len = leb128<size_t>(p);
        p += len;
        return object_p(p);
    }

    static object_p strip(object_p obj)
    {
        if (obj)
            while (tag_p tobj = obj->as<tag>())
                obj = tobj->tagged_object();
        return obj;
    }

public:
    OBJECT_DECL(tag);
    PARSE_DECL(tag);
    SIZE_DECL(tag);
    HELP_DECL(tag);
    RENDER_DECL(tag);
    GRAPH_DECL(tag);
};

COMMAND_DECLARE(dtag,1);
COMMAND_DECLARE(ToTag,2);
COMMAND_DECLARE(FromTag,1);

#endif // TAG_H
