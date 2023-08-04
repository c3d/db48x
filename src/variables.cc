// ****************************************************************************
//  variables.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Implementation of variables
//
//      Global variables are stored in mutable directory objects that occupy
//      a reserved area of the runtime, and can grow/shrinnk as you store
//      or purge global variables
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

#include "variables.h"

#include "command.h"
#include "integer.h"
#include "list.h"
#include "locals.h"
#include "parser.h"
#include "renderer.h"


RECORDER(directory,       16, "Directories");
RECORDER(directory_error, 16, "Errors from directories");


PARSE_BODY(directory)
// ----------------------------------------------------------------------------
//    Try to parse this as a directory
// ----------------------------------------------------------------------------
//    A directory has the following structure:
//        Directory { Name1 Value1 Name2 Value2 ... }
{
    cstring ref = cstring(utf8(p.source));
    size_t maxlen = p.length;
    cstring label = "directory";
    size_t len = strlen(label);
    if (len <= maxlen
        && strncasecmp(ref, label, len) == 0
        && command::is_separator(utf8(ref + len)))
    {
        gcutf8 body = utf8(ref + len);
        maxlen -= len;
        if (object_g obj = object::parse(body, maxlen))
        {
            if (obj->type() == ID_list)
            {
                // Check that we only have names in there
                uint    count  = 0;
                gcbytes bytes  = obj->payload();
                byte_p  ptr    = bytes;
                size_t  size   = leb128<size_t>(ptr);
                gcbytes start  = ptr;
                size_t  offset = 0;

                // Loop on all objects inside the list
                while (offset < size)
                {
                    object_p obj = object_p(byte_p(start) + offset);
                    size_t   objsize = obj->size();

                    if ((count & 1) == 0)
                    {
                        if (obj->type() != ID_symbol)
                        {
                            rt.error("Invalid name in directory").source(body);
                            return ERROR;
                        }
                    }
                    count++;

                    // Loop on next object
                    offset += objsize;
                }

                // We should have an even number of items here
                if (count & 1)
                {
                    rt.malformed_directory_error().source(body);
                    return SKIP;
                }

                // If we passed all these tests, build a directory
                p.out = rt.make<directory>(ID_directory, start, size);
                p.end = maxlen + len;
                return OK;
            }
        }
    }

    return SKIP;
}


bool directory::render_name(symbol_p name, object_p obj, void *arg)
// ----------------------------------------------------------------------------
//    Render an item in the directory
// ----------------------------------------------------------------------------
{
    renderer &r = *((renderer *) arg);
    name->render(r);
    r.indent();
    obj->render(r);
    r.unindent();
    return true;
}


RENDER_BODY(directory)
// ----------------------------------------------------------------------------
//   Render the directory into the given directory buffer
// ----------------------------------------------------------------------------
{
    r.put("Directory {");
    r.indent();
    o->enumerate(directory::render_name, &r);
    r.unindent();
    r.put("}");
    return r.size();
}


EXEC_BODY(directory)
// ----------------------------------------------------------------------------
//   Enter directory when executing a directory
// ----------------------------------------------------------------------------
{
    if (rt.enter(o))
        return OK;
    return ERROR;
}


bool directory::store(object_g name, object_g value)
// ----------------------------------------------------------------------------
//    Store an object in the directory
// ----------------------------------------------------------------------------
//    Note that the directory itself should never move because of GC
//    That's because it normally should reside in the globals area
{
    size_t      vs      = value->size();        // Size of value
    int         delta   = 0;                    // Change in directory size
    directory_g thisdir = this;                 // Can move because of GC

    if (object_g existing = lookup(name))
    {
        // Replace an existing entry
        object_g evalue = existing->skip();
        size_t es = evalue->size();
        if (vs > es)
        {
            size_t requested = vs - es;
            if (rt.available(requested) < requested)
                return false;           // Out of memory
        }

        // Clone any value in the stack that points to the existing value
        rt.clone_global(evalue);

        // Move memory above storage if necessary
        if (vs != es)
            rt.move_globals((object_p) evalue + vs, (object_p) evalue + es);

        // Copy new value into storage location
        memmove((byte *) evalue, (byte *) value, vs);

        // Compute change in size for directories
        delta = vs - es;
    }
    else
    {
        // New entry, need to make room for name and value
        size_t  ns        = name->size();
        size_t  vs        = value->size();
        size_t  requested = vs + ns;
        byte_p  p         = payload();
        size_t  old       = leb128<size_t>(p);
        gcbytes body      = p;
        if (rt.available(requested) < requested)
            return false;               // Out of memory

        // Move memory above end of directory
        object_p end = object_p(body.Safe() + old);
        rt.move_globals(end + requested, end);

        // Copy name and value at end of directory
        memmove((byte *) end, (byte *) name, ns);
        memmove((byte *) end + ns, (byte *) value, vs);

        // Compute new size of the directory
        delta = requested;
    }

    // Adjust all directory sizes
    adjust_sizes(thisdir, delta);
    return true;
}


void directory::adjust_sizes(directory_r thisdir, int delta)
// ----------------------------------------------------------------------------
//   Ajust the size for this directory and all enclosing ones
// ----------------------------------------------------------------------------
{
    // Resize directories up the chain
    uint depth = 0;
    bool found = false;
    while (directory_g dir = rt.variables(depth++))
    {
        // Start modifying only if we find this directory in path
        if (dir.Safe() == thisdir.Safe())
            found = true;
        if (found)
        {
            byte_p p = dir->payload();
            object_p hdr = object_p(p);
            size_t dirlen = leb128<size_t>(p);
            size_t newdirlen = dirlen + delta;
            size_t szbefore  = leb128size(dirlen);
            size_t szafter = leb128size(newdirlen);
            if (szbefore != szafter)
            {
                rt.move_globals(hdr + szafter, hdr + szbefore);
                delta += szafter - szbefore;
            }
            leb128(hdr, newdirlen);
        }
    }
}


object_p directory::lookup(object_p ref) const
// ----------------------------------------------------------------------------
//   Find if the name exists in the directory, if so return pointer to it
// ----------------------------------------------------------------------------
{
    byte_p p = payload();
    size_t size = leb128<size_t>(p);
    size_t rsize = ref->size();

    while (size)
    {
        object_p name = (object_p) p;
        size_t ns = name->size();
        if (name == ref)          // Optimization when name is from directory
            return name;
        if (ns == rsize && memcmp(name, ref, rsize) == 0)
            return name;

        p += ns;
        object_p value = (object_p) p;
        size_t vs = value->size();
        p += vs;

        // Defensive coding against malformed directorys
        if (ns + vs > size)
        {
            record(directory_error,
                   "Lookup malformed directory (ns=%u vs=%u size=%u)",
                   ns, vs, size);
            return nullptr;     // Malformed directory, quick exit
        }

        size -= (ns + vs);
    }

    return nullptr;
}


object_p directory::recall(object_p ref) const
// ----------------------------------------------------------------------------
//   If the referenced object exists in directory, return associated value
// ----------------------------------------------------------------------------
{
    if (object_p found = lookup(ref))
        // The value follows the name
        return found->skip();
    return nullptr;
}


size_t directory::purge(object_p ref)
// ----------------------------------------------------------------------------
//    Purge a name (and associated value) from the directory
// ----------------------------------------------------------------------------
{
    directory_g thisdir = this;

    if (object_g name = lookup(ref))
    {
        size_t   ns     = name->size();
        object_p value  = name + ns;
        size_t   vs     = value->size();
        size_t   purged = ns + vs;
        object_p header = (object_p) payload();
        object_p body   = header;
        size_t   old    = leb128<size_t>(body); // Old size of directory

        rt.move_globals(name, name + purged);

        if (old < purged)
        {
            record(directory_error,
                   "Purging %u bytes in %u bytes directory", purged, old);
            purged = old;
        }

        adjust_sizes(thisdir, -int(purged));
        return purged;
    }

    // If nothing purged, return 0
    return 0;
}


size_t directory::enumerate(enumeration_fn callback, void *arg) const
// ----------------------------------------------------------------------------
//   Process all the variables in turn, return number of true values
// ----------------------------------------------------------------------------
{
    gcbytes base  = payload();
    byte_p  p     = base;
    size_t  size  = leb128<size_t>(p);
    size_t  count = 0;

    while (size)
    {
        symbol_p name = (symbol_p) p;
        size_t   ns   = name->size();
        p += ns;
        object_p value = (object_p) p;
        size_t   vs    = value->size();
        p += vs;

        // Defensive coding against malformed directorys
        if (ns + vs > size)
        {
            record(directory_error,
                   "Malformed directory during enumeration (ns=%u vs=%u size=%u)",
                   ns, vs, size);
            return 0;     // Malformed directory, quick exit
        }

        // Stash in a gcp: the callback may cause garbage collection
        base = p;
        if (!callback || callback(name, value, arg))
            count++;

        size -= (ns + vs);
        p = base;
    }

    return count;
}



// ============================================================================
//
//    Variable-related commands
//
// ============================================================================

COMMAND_BODY(Sto)
// ----------------------------------------------------------------------------
//   Store a global variable into current directory
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return ERROR;
    }

    // Check that we have two objects in the stack
    object_p x = rt.stack(0);
    object_p y = rt.stack(1);
    if (x && y)
    {
        if (local_p loc = x->as_quoted<local>())
        {
            size_t index = loc->index();
            if (rt.local(index, y))
                return OK;
            return ERROR;
        }
        symbol_p name = x->as_quoted<symbol>();
        if (!name)
        {
            rt.invalid_name_error();
            return ERROR;
        }

        if (dir->store(name, y))
        {
            rt.drop();
            rt.drop();
            return OK;
        }
    }

    // Otherwise, return an error
    return ERROR;
}


COMMAND_BODY(Rcl)
// ----------------------------------------------------------------------------
//   Recall a global variable from current directory
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(0);
    if (!x)
        return ERROR;
    if (local_p loc = x->as_quoted<local>())
    {
        size_t index = loc->index();
        if (object_g obj = rt.local(index))
            if (rt.push(obj))
                return OK;
        return ERROR;
    }
    symbol_p name = x->as_quoted<symbol>();
    if (!name)
    {
        rt.invalid_name_error();
        return ERROR;
    }

    // Lookup all directorys, starting with innermost one
    directory *dir = nullptr;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
    {
        if (object_p value = dir->recall(name))
        {
            if (rt.top(value))
                return OK;
            return ERROR;       // Out of memory, cannot happen?

        }
    }

    // Otherwise, return an error
    rt.undefined_name_error();
    return ERROR;
}


COMMAND_BODY(Purge)
// ----------------------------------------------------------------------------
//   Purge a global variable from current directory
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(0);
    if (!x)
        return ERROR;
    symbol_g name = x->as_quoted<symbol>();
    if (!name)
    {
        rt.invalid_name_error();
        return ERROR;
    }
    rt.pop();

    // Lookup all directorys, starting with innermost one
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return ERROR;
    }

    // Purge the object (HP48 doesn't error out if name does not exist)
    dir->purge(name);
    return OK;
}


COMMAND_BODY(PurgeAll)
// ----------------------------------------------------------------------------
//   Purge a global variable from current directory and enclosing directories
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(0);
    if (!x)
        return ERROR;
    symbol_g name = x->as_quoted<symbol>();
    if (!name)
    {
        rt.invalid_name_error();
        return ERROR;
    }
    rt.pop();

    // Lookup all directorys, starting with innermost one, and purge there
    directory *dir = nullptr;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
        dir->purge(name);

    return OK;
}


COMMAND_BODY(Mem)
// ----------------------------------------------------------------------------
//    Return amount of available memory
// ----------------------------------------------------------------------------
//    The HP48 manual specifies that mem performs garbage collection
{
    rt.gc();
    run<FreeMemory>();
    return OK;
}


COMMAND_BODY(GarbageCollect)
// ----------------------------------------------------------------------------
//   Run the garbage collector
// ----------------------------------------------------------------------------
{
    size_t saved = rt.gc();
    integer_p result = rt.make<integer>(ID_integer, saved);
    return rt.push(result) ? OK : ERROR;
}


COMMAND_BODY(FreeMemory)
// ----------------------------------------------------------------------------
//   Return amount of free memory (available without garbage collection)
// ----------------------------------------------------------------------------
{
    size_t available = rt.available();
    integer_p result = rt.make<integer>(ID_integer, available);
    return rt.push(result) ? OK : ERROR;
}


COMMAND_BODY(SystemMemory)
// ----------------------------------------------------------------------------
//   Return the amount of memory that is seen as free by the system
// ----------------------------------------------------------------------------
{
    size_t mem = sys_free_mem();
    integer_p result = rt.make<integer>(ID_integer, mem);
    return rt.push(result) ? OK : ERROR;
}


COMMAND_BODY(home)
// ----------------------------------------------------------------------------
//   Return the home directory
// ----------------------------------------------------------------------------
{
    rt.updir(~0U);
    return OK;
}


COMMAND_BODY(CurrentDirectory)
// ----------------------------------------------------------------------------
//   Return the current directory as an object
// ----------------------------------------------------------------------------
{
    directory_p dir = rt.variables(0);
    if (rt.push(dir))
        return OK;
    return ERROR;
}


static bool path_callback(symbol_p name, object_p obj, void *arg)
// ----------------------------------------------------------------------------
//   Find the directory in enclosing directory
// ----------------------------------------------------------------------------
{
    if (obj == object_p(arg))
    {
        rt.append(name->size(), byte_p(name));
        return true;
    }
    return false;
}


list_p directory::path(id type)
// ----------------------------------------------------------------------------
//   Return the current directory path as a list object of the given type
// ----------------------------------------------------------------------------
{
    scribble scr;

    size_t sz = leb128size(ID_home);
    byte *p = rt.allocate(sz);
    leb128(p, ID_home);

    uint depth = rt.directories();
    directory_p dir = rt.homedir();
    while (depth > 1)
    {
        depth--;
        directory_p next = rt.variables(depth-1);
        if (dir->enumerate(path_callback, (void *) next) != 1)
        {
            rt.directory_path_error();
            return nullptr;
        }
        dir = next;
    }

    list_p list = list::make(type, scr.scratch(), scr.growth());
    return list;
}


COMMAND_BODY(path)
// ----------------------------------------------------------------------------
//   Build a path with the list of paths
// ----------------------------------------------------------------------------
{
    if (list_p list = directory::path())
        if (rt.push(list))
            return OK;

    return ERROR;
}


COMMAND_BODY(crdir)
// ----------------------------------------------------------------------------
//   Create a directory
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return ERROR;
    }

    if (object_p obj = rt.pop())
    {
        symbol_p name = obj->as_quoted<symbol>();
        if (!name)
        {
            rt.invalid_name_error();
            return ERROR;
        }
        if (dir->recall(name))
        {
            rt.name_exists_error();
            return ERROR;
        }

        object_p newdir = rt.make<directory>();
        if (dir->store(name, newdir))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(updir)
// ----------------------------------------------------------------------------
//   Go up one directory
// ----------------------------------------------------------------------------
{
    rt.updir();
    return OK;
}


COMMAND_BODY(pgdir)
// ----------------------------------------------------------------------------
//   Really the same as 'purge'
// ----------------------------------------------------------------------------
{
    return Purge::evaluate();
}





// ============================================================================
//
//    Variables menu
//
// ============================================================================

MENU_BODY(VariablesMenu)
// ----------------------------------------------------------------------------
//   Process the MENU command for VariablesMenu
// ----------------------------------------------------------------------------
{
    uint  nitems = count_variables();
    items_init(mi, nitems, 1);
    list_variables(mi);
    return OK;
}


uint VariablesMenu::count_variables()
// ----------------------------------------------------------------------------
//    Count the variables in the current directory
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return 0;
    }
    return dir->count();
}


static bool evaluate_variable(symbol_p name, object_p UNUSED value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
{
    menu::info &mi = *((menu::info *) arg);
    menu::items(mi, name, menu::ID_VariablesMenuExecute);
    return true;
}


static bool recall_variable(symbol_p name, object_p UNUSED value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
//   For a name X, we create an object « 'Name' RCL »
{
    menu::info &mi = *((menu::info *) arg);
    menu::items(mi, name, menu::ID_VariablesMenuRecall);
    return true;
}


static bool store_variable(symbol_p name, object_p UNUSED value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
{
    menu::info &mi = *((menu::info *) arg);
    menu::items(mi, name, menu::ID_VariablesMenuStore);
    return true;
}



void VariablesMenu::list_variables(info &mi)
// ----------------------------------------------------------------------------
//   Fill the menu with variable names
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return;
    }

    uint skip = mi.skip;
    mi.plane  = 0;
    mi.planes = 1;
    dir->enumerate(evaluate_variable, &mi);
    mi.plane  = 1;
    mi.planes = 2;
    mi.skip   = skip;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    dir->enumerate(recall_variable, &mi);
    mi.plane  = 2;
    mi.planes = 3;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    mi.skip   = skip;
    dir->enumerate(store_variable, &mi);

    for (uint k = 0; k < ui.NUM_SOFTKEYS - (mi.pages > 1); k++)
    {
        ui.marker(k + 1 * ui.NUM_SOFTKEYS, L'▶', false);
        ui.marker(k + 2 * ui.NUM_SOFTKEYS, L'▶', true);
    }

    ui.menuNeedsRefresh();
}


static object::result insert_cmd(int key, cstring before, cstring after)
// ----------------------------------------------------------------------------
//   Insert the name associated with the key if editing
// ----------------------------------------------------------------------------
{
    if (symbol_p name = ui.label(key - KEY_F1))
    {
        uint     cursor = ui.cursorPosition();
        size_t   length = 0;
        utf8     text   = name->value(&length);

        cursor += rt.insert(cursor, utf8(before));
        cursor += rt.insert(cursor, text, length);
        cursor += rt.insert(cursor, utf8(after));

        ui.cursorPosition(cursor);

        return object::OK;
    }
    return object::ERROR;
}


COMMAND_BODY(VariablesMenuExecute)
// ----------------------------------------------------------------------------
//   Recall a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (rt.editing())
        return insert_cmd(key, "", " ");

    if (key >= KEY_F1 && key <= KEY_F6)
    {
        if (symbol_p name = ui.label(key - KEY_F1))
        {
            if (directory *dir = rt.variables(0))
            {
                if (object_p value = dir->recall(name))
                {
                    size_t sz = 0;
                    utf8 help = name->value(&sz);
                    ui.draw_user_command(help, sz);
                    return value->execute();
                }
            }
        }
    }

    return ERROR;
}


COMMAND_BODY(VariablesMenuRecall)
// ----------------------------------------------------------------------------
//   Recall a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (rt.editing())
        return insert_cmd(key, "'", "' Recall ");

    if (key >= KEY_F1 && key <= KEY_F6)
        if (symbol_p name = ui.label(key - KEY_F1))
            if (directory *dir = rt.variables(0))
                if (object_p value = dir->recall(name))
                    if (rt.push(value))
                        return OK;

    return ERROR;
}


COMMAND_BODY(VariablesMenuStore)
// ----------------------------------------------------------------------------
//   Store a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (rt.editing())
        return insert_cmd(key, "'", "' Store ");

    if (key >= KEY_F1 && key <= KEY_F6)
        if (symbol_p name = ui.label(key - KEY_F1))
            if (directory *dir = rt.variables(0))
                if (object_p value = rt.pop())
                    if (dir->store(name, value))
                        return OK;

    return ERROR;
}
