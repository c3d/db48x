#ifndef TEXT_H
#define TEXT_H
// ****************************************************************************
//  text.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//      The RPL text object type
//
//      Operations on text values
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
//
// Payload format:
//
//   The text object is a sequence of bytes containing:
//   - The type ID (one byte)
//   - The LEB128-encoded length of the text (one byte in most cases)
//   - The characters of the text, not null-terminated
//
//   On most texts, this format uses 3 bytes less than on the HP48.

#include "algebraic.h"
#include "runtime.h"
#include "utf8.h"


GCP(text);


struct text : algebraic
// ----------------------------------------------------------------------------
//    Represent text objects
// ----------------------------------------------------------------------------
//    We derive from 'algebraic' because many algebraic objects
//    derive from text (equation, symbol and local variables notably)
{
    text(id type, gcutf8 source, size_t len): algebraic(type)
    {
        utf8 s = (utf8) source;
        byte *p = (byte *) payload();
        p = leb128(p, len);
        while (len--)
            *p++ = *s++;
    }

    text(id type, gcutf8 source, size_t len, size_t quotes): algebraic(type)
    {
        utf8 s = (utf8) source;
        byte *p = (byte *) payload();
        p = leb128(p, len - quotes);
        while (len--)
        {
            *p = *s++;
            if (*p != '"' || *s != '"')
                p++;
        }
    }

    static size_t required_memory(id i, gcutf8 UNUSED str, size_t len)
    {
        return leb128size(i) + leb128size(len) + len;
    }

    static size_t required_memory(id i, gcutf8 UNUSED str,
                                  size_t len, size_t quotes)
    {
        len -= quotes;
        return leb128size(i) + leb128size(len) + len;
    }

    static text_p make(utf8 str, size_t len)
    {
        gcutf8 gcstr = str;
        return rt.make<text>(gcstr, len);
    }

    static text_p make(utf8 str)
    {
        return make(str, strlen(cstring(str)));
    }

    static text_p make(cstring str, size_t len)
    {
        return make(utf8(str), len);
    }

    static text_p make(cstring str)
    {
        return make(utf8(str), strlen(str));
    }

    size_t length() const
    {
        byte_p p = payload(this);
        return leb128<size_t>(p);
    }

    utf8 value(size_t *size = nullptr) const
    {
        byte_p p   = payload(this);
        size_t len = leb128<size_t>(p);
        if (size)
            *size = len;
        return (utf8) p;
    }

    size_t utf8_characters() const;
    text_p import() const;      // Import text containing << or >> or ->

    // Iterator, built in a way that is robust to garbage collection in loops
    struct iterator
    {
        typedef unicode value_type;
        typedef size_t difference_type;

        explicit iterator(text_p text, bool atend = false)
            : first(byte_p(text->value())),
              size(text->length()),
              index(atend ? size : 0) {}
        explicit iterator(text_p text, size_t skip)
            : first(byte_p(text->value())),
              size(text->length()),
              index(0)
        {
            while (skip && index < size)
            {
                operator++();
                skip--;
            }
        }

    public:
        iterator& operator++()
        {
            if (index < size)
            {
                utf8 p = +first + index;
                p = utf8_next(p);
                index = p - +first;
            }

            return *this;
        }
        iterator operator++(int)
        {
            iterator prev = *this;
            ++(*this);
            return prev;
        }
        bool operator==(const iterator &other) const
        {
            return index == other.index &&
                   +first == +other.first &&
                   size == other.size;
        }
        bool operator!=(const iterator &other) const
        {
            return !(*this == other);
        }
        value_type operator*() const
        {
            return index < size ? utf8_codepoint(+first + index) : 0;
        }

        text_g as_text() const
        {
            if (index < size)
            {
                utf8 p = +first+ index;
                utf8 n = utf8_next(p);
                return text::make(p, n - p);
            }
            return nullptr;
        }

    public:
        gcutf8 first;
        size_t size;
        size_t index;
    };
    iterator begin() const      { return iterator(this); }
    iterator end() const        { return iterator(this, true); }

    size_t items() const
    // ------------------------------------------------------------------------
    //   Return number of items in the text (codepoints)
    // ------------------------------------------------------------------------
    {
        size_t result = 0;
        for (unicode cp UNUSED : *this)
            result++;
        return result;
    }

    unicode operator[](size_t index) const
    // ------------------------------------------------------------------------
    //   Return the n-th element in the list
    // ------------------------------------------------------------------------
    {
        return *iterator(this, index);
    }

    text_g at(size_t index) const
    // ------------------------------------------------------------------------
    //   Return the n-th element in the list as a text
    // ------------------------------------------------------------------------
    {
        return iterator(this, index).as_text();
    }


    text_p extract(object_r first, object_r last) const;
    text_p extract(size_t first, size_t last) const;
    // ------------------------------------------------------------------------
    //   Extract a subtext
    // ------------------------------------------------------------------------


    object_p compile() const;
    bool compile_and_run() const;
    // ------------------------------------------------------------------------
    //   Compile and run the text
    // ------------------------------------------------------------------------

public:
    OBJECT_DECL(text);
    PARSE_DECL(text);
    SIZE_DECL(text);
    RENDER_DECL(text);
};

// Some operators on texts
text_g operator+(text_r x, text_r y);
text_g operator*(text_r x, uint y);

COMMAND_DECLARE(CharToUnicode,1);
COMMAND_DECLARE(TextToUnicode,1);
COMMAND_DECLARE(UnicodeToText,1);
COMMAND_DECLARE(Extract, 3);

#endif // TEXT_H
