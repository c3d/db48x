#ifndef CHARACTERS_H
#define CHARACTERS_H
// ****************************************************************************
//  characters.h                                                 DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Character loaded from a characters file
//
//     Characters are loaded from a `config/characters.csv` file.
//     This makes it possible to define them with arbitrary content
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "file.h"
#include "menu.h"
#include "text.h"

struct characters_file : file
// ----------------------------------------------------------------------------
//   Manage a unit file
// ----------------------------------------------------------------------------
{
    characters_file(cstring name = "config/characters.csv")
        : file(name, READING) {}
    ~characters_file() {}
    symbol_g    next();
};


struct character_menu : menu
// ----------------------------------------------------------------------------
//   A character menu is like a standard menu, but with characters
// ----------------------------------------------------------------------------
{
    character_menu(id type) : menu(type) { }

    static uint build_general_menu(menu_info &mi);
    static uint build_at_cursor(menu_info &mi);
    static uint build_for_code(menu_info &mi, unicode cp);
    static uint build_from_characters(menu_info &mi,
                                      utf8 chars, size_t len, size_t offset);

    symbol_p characters(size_t *matching) const;
    bool transliterate(unicode &input) const;

public:
    MENU_DECL(character_menu);
};
GCP(character_menu);


#define ID(i)
#define CHARACTER_MENU(CharacterMenu)           \
    struct CharacterMenu : character_menu {};
#include "ids.tbl"


#endif // CHARACTER_H
