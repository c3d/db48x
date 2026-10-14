#ifndef SYMBOL_H
#define SYMBOL_H
// ****************************************************************************
//  symbol.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//      RPL names / symbols
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
//
// Payload format:
//
//   The symbol object is a sequence of bytes containing:
//   - The type ID (one byte)
//   - The LEB128-encoded length of the name (one byte in most cases)
//   - The characters of the name, not null-terminated
//
//   On most strings, this format uses 3 bytes less than on the HP48.
//   This representation allows arbitrary symbol names, including names with
//   weird UTF-8 symbols in them, such as ΣDATA or ∱√π²≄∞
//

#include "object.h"
#include "precedence.h"
#include "text.h"
#include "utf8.h"


GCP(symbol);

struct symbol : text
// ----------------------------------------------------------------------------
//    Represent symbol objects
// ----------------------------------------------------------------------------
{
    symbol(id type, gcutf8 source, size_t len): text(type, source, len)
    { }

    static symbol_g make(char c)
    {
        return rt.make<symbol>(ID_symbol, utf8(&c), 1);
    }

    static symbol_g make(cstring s)
    {
        return rt.make<symbol>(ID_symbol, utf8(s), strlen(s));
    }

    static symbol_g make(gcutf8 s, size_t len)
    {
        return rt.make<symbol>(ID_symbol, s, len);
    }

    object_p   recall(bool noerror = true) const;
    bool       store(object_g obj) const;
    bool       is_same_as(symbol_p other) const;

    bool       matches(cstring name) const
    {
        return matches(utf8(name), strlen(name));
    }
    bool       matches(utf8 name, size_t len) const;

    bool       starts_with(cstring name) const
    {
        return starts_with(utf8(name), strlen(name));
    }
    bool       starts_with(utf8 name, size_t len) const;

    bool       is_alpha() const;

    static int compare(utf8 x, utf8 y, size_t len);

    bool       found_in(object_p obj) const;
    object_p   library_lookup() const;

public:
    OBJECT_DECL(symbol);
    PARSE_DECL(symbol);
    EVAL_DECL(symbol);
    RENDER_DECL(symbol);
    GRAPH_DECL(symbol);
    PREC_DECL(SYMBOL);

    static bool unambiguous;
};

symbol_g operator+(symbol_r x, symbol_r y);


#endif // SYMBOL_H
