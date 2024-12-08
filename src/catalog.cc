// ****************************************************************************
//  catalog.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Auto-completion for commands (Catalog)
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

#include "catalog.h"

#include "characters.h"
#include "runtime.h"
#include "user_interface.h"

#include <stdlib.h>
#include <string.h>


RECORDER(catalog_error, 16, "Errors building the catalog");


MENU_BODY(Catalog)
// ----------------------------------------------------------------------------
//   Process the MENU command for Catalog
// ----------------------------------------------------------------------------
{
    if (ui.editing_mode() == ui.TEXT)
    {
        // Character catalog
        character_menu::build_at_cursor(mi);
    }
    else
    {
        // Command catalog
        uint  nitems = count_commands();
        items_init(mi, nitems);
        list_commands(mi);
        if (mi.page >= mi.pages)
            mi.page = 0;
    }
    ui.menu_auto_complete();
    return OK;
}


static uint matches(utf8 start, size_t size, utf8 name)
// ----------------------------------------------------------------------------
//   Check if what was typed matches the name
// ----------------------------------------------------------------------------
{
    size_t len   = strlen(cstring(name));
    uint found = 0;
    for (uint o = 0; !found && o + size <= len; o++)
    {
        found = o+1;
        for (uint i = 0; found && i < size; i++)
            if (tolower(start[i]) != tolower(name[i + o]))
                found = 0;
    }
    return found;
}



uint Catalog::count_commands()
// ----------------------------------------------------------------------------
//    Count the commands to display in the catalog
// ----------------------------------------------------------------------------
{
    utf8   start  = 0;
    size_t size   = 0;
    bool   filter = ui.current_word(start, size);
    uint   count  = 0;

    for (size_t i = 0; i < spelling_count; i++)
    {
        object::id ty = object::spellings[i].type;
        if (!object::is_command(ty))
            continue;

        if (cstring name = spellings[i].name)
            if (!filter || matches(start, size, utf8(name)))
                count++;
    }

    return count;
}


void Catalog::list_commands(info &mi)
// ----------------------------------------------------------------------------
//   Fill the menu with all possible spellings of the command
// ----------------------------------------------------------------------------
{
    utf8   start  = nullptr;
    size_t size   = 0;
    bool   filter = ui.current_word(start, size);

    if (!sorted_ids)
        initialize_sorted_ids();

    if (sorted_ids)
    {
        for (uint pass = 0; pass < uint(filter) + 1; pass++)
        {
            for (size_t i = 0; i < sorted_ids_count; i++)
            {
                uint16_t j = sorted_ids[i];
                auto &s = object::spellings[j];
                id ty  = s.type;
                if (cstring name = s.name)
                {
                    uint ok = !filter;
                    if (!ok)
                    {
                        ok = matches(start, size, utf8(name));
                        if (ok)
                            ok = pass ? ok != 1 : ok == 1;
                    }
                    if (ok)
                        menu::items(mi, name, command::static_object(ty));
                }
            }
        }
    }
    else
    {
        // Fallback if we did not have enough memory for sorted_ids
        for (size_t i = 0; i < spelling_count; i++)
        {
            object::id ty = object::spellings[i].type;
            if (object::is_command(ty))
                if (cstring name = spellings[i].name)
                    if (!filter || matches(start, size, utf8(name)))
                        menu::items(mi, name, command::static_object(ty));
        }
    }
}
