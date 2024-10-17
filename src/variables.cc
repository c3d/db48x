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

#include "bignum.h"
#include "command.h"
#include "constants.h"
#include "expression.h"
#include "files.h"
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
    cstring ref   = cstring(utf8(p.source));
    size_t  max   = p.length;
    cstring label = "directory";
    size_t  len   = strlen(label);
    bool    isdir = false;

    do
    {
        isdir = (len <= max
                 && strncasecmp(ref, label, len) == 0
                 && is_separator(utf8(ref + len)));
    } while (!isdir && len > 3 && (len = 3));

    if (isdir)
    {
        utf8 body = utf8(ref + len);
        max -= len;
        size_t ws = utf8_skip_whitespace(body, max);
        if (ws < max && body[ws] == '{')
        {
            if (object_g obj = object::parse(body, max))
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
                            // In directories, we accept Eq as a name,
                            // and with the right option, we may have
                            // numbered variables as well
                            if (obj->arity() != 0)
                            {
                                body = p.source + len + ws;
                                rt.malformed_directory_error().source(body);
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
                        body = p.source + len + ws;
                        rt.malformed_directory_error().source(body);
                        return SKIP;
                    }

                    // If we passed all these tests, build a directory
                    p.out    = rt.make<directory>(ID_directory, start, size);
                    p.length = max + len;
                    return p.out ? OK : ERROR;
                }
            }
        }
        else
        {
            // The word 'DIR' alone denotes an empty directory on HP calculators
            p.out = rt.make<directory>(ID_directory, rt.allocate(0), 0);
            p.length = ws + len;
            return p.out ? OK : ERROR;
        }
    }

    return SKIP;
}


bool directory::render_name(object_p name, object_p obj, void *arg)
// ----------------------------------------------------------------------------
//    Render an item in the directory
// ----------------------------------------------------------------------------
{
    renderer &r = *((renderer *) arg);
    r.wantCR();
    name->render(r);
    if (obj->is_algebraic())
    {
        r.wantSpace();
        obj->render(r);
    }
    else
    {
        r.indent();
        r.wantCR();
        obj->render(r);
        r.unindent();
    }
    r.wantCR();
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


object::result directory::enter() const
// ----------------------------------------------------------------------------
//   Enter directory when executing a directory
// ----------------------------------------------------------------------------
{
    if (rt.enter(this))
    {
        if (menu_p m = ui.menu())
            if (m->type() == ID_CustomMenu)
                run<VariablesMenu>();
        ui.menu_refresh(ID_VariablesMenu);
        return OK;
    }
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

    // If this is a quoted name, extract it
    if (object_p quoted = name->as_quoted(ID_object))
        name = quoted;

    // Deal with all special cases
    id nty = name->type();
    switch (nty)
    {
    case ID_local:
        // Deal with local variables
        return rt.local(local_p(+name)->index(), value);

    case ID_text:
    {
        // Deal with storing to file
        files_g disk = files::make("data");
        return disk->store(text_p(+name), value);
    }

    // Special names that are allowed as variable names
    case ID_StatsData:
    case ID_StatsParameters:
    case ID_Equation:
    case ID_PlotParameters:
    case ID_AlgebraConfiguration:
    case ID_AlgebraVariable:
    case ID_CustomMenu:
        break;

    case ID_symbol:
        break;

#define ID(n)
#define SETTING(Name, Low, High, Init)          \
    case ID_##Name:
#define FLAG(Enable, Disable)                   \
    case ID_##Enable:                           \
    case ID_##Disable:
#include "ids.tbl"
        return settings::store(nty, value);

    case ID_integer:
        if (Settings.NumberedVariables())
            break;
        // Fall-through
    default:
        rt.invalid_name_error();
        return false;
    }

    // Normal case
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
        rt.clone_global(evalue, es);

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
        size_t  dirsize   = leb128<size_t>(p);
        gcbytes body      = p;
        if (rt.available(requested) < requested)
            return false;               // Out of memory

        // Move memory from directory up
        object_p start = object_p(+body);
        if (Settings.StoreAtEnd())
            start += dirsize;
        rt.move_globals(start + requested, start);

        // Copy name and value at end of directory
        memmove((byte *) start, (byte *) name, ns);
        memmove((byte *) start + ns, (byte *) value, vs);

        // Compute new size of the directory
        delta = requested;
    }

    // Adjust all directory sizes
    adjust_sizes(thisdir, delta);

    // Refresh the variables menu
    ui.menu_refresh(ID_VariablesMenu);
    if (nty == ID_CustomMenu)
        ui.menu_refresh(nty);

    return true;
}


bool directory::update(object_p name, object_p value)
// ----------------------------------------------------------------------------
//   Update an existing value
// ----------------------------------------------------------------------------
{
    // Strip quote if any
    if (object_p quoted = name->as_quoted(ID_object))
        name = quoted;

    // Check if this exists somewhere, if so update it with new value
    directory *dir = nullptr;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
        if (dir->recall(name))
            return dir->store(name, value);
    return false;
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
        if (+dir== thisdir.Safe())
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
    byte_p   p     = payload();
    size_t   size  = leb128<size_t>(p);
    size_t   rsize = ref->size();
    symbol_p rsym  = ref->as<symbol>();

    while (size)
    {
        object_p name = (object_p) p;
        size_t ns = name->size();
        if (name == ref)          // Optimization when name is from directory
            return name;
        if (ns == rsize)
        {
            if (rsym)
            {
                // Regular symbols: case insensitive comparison
                if (symbol_p nsym = name->as<symbol>())
                    if (rsym->is_same_as(nsym))
                        return name;
            }
            else
            {
                // Special symbols, e.g. ΣData
                if (memcmp(cstring(name), cstring(ref), rsize) == 0)
                    return name;
            }
        }

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


object_p directory::recall_all(object_p name, bool report_missing)
// ----------------------------------------------------------------------------
//   If the referenced object exists in directory, return associated value
// ----------------------------------------------------------------------------
{
    // Strip quote if any
    if (object_p quoted = name->as_quoted(ID_object))
        name = quoted;

    // Deal with all special cases
    id nty = name->type();
    switch (nty)
    {
    case ID_local:
        // Deal with local variables
        return rt.local(local_p(name)->index());

    case ID_text:
    {
        // Deal with storing to file
        files_g disk = files::make("data");
        return disk->recall(text_p(name));
    }

    case ID_constant:
        return constant_p(name)->value();

    // Special names that are allowed as variable names
    case ID_StatsData:
    case ID_StatsParameters:
    case ID_Equation:
    case ID_PlotParameters:
    case ID_AlgebraConfiguration:
    case ID_AlgebraVariable:
    case ID_CustomMenu:
        break;

    case ID_symbol:
    {
        // Check independent / dependent values for plotting
        symbol_p s = symbol_p(name);
        if (expression::independent && s->is_same_as(*expression::independent))
            return expression::independent_value
                ? *expression::independent_value : nullptr;
        if (expression::dependent && s->is_same_as(*expression::dependent))
            return expression::dependent_value
                ? *expression::dependent_value : nullptr;
        break;
    }

#define ID(n)
#define SETTING(Name, Low, High, Init)          \
    case ID_##Name:
#define FLAG(Enable, Disable)                   \
    case ID_##Enable:                           \
    case ID_##Disable:
#include "ids.tbl"
        return settings::recall(nty);

    case ID_integer:
        if (Settings.NumberedVariables())
            break;
        // Fall-through
    default:
        rt.invalid_name_error();
        return nullptr;
    }

    directory *dir = nullptr;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
        if (object_p value = dir->recall(name))
            return value;
    if (report_missing)
        rt.undefined_name_error();
    return nullptr;
}


bool directory::store_here(object_p name, object_p value)
// ----------------------------------------------------------------------------
//  Store a variable in the current directory
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return false;
    }
    return dir->store(name, value);
}


size_t directory::purge(object_p name)
// ----------------------------------------------------------------------------
//    Purge a name (and associated value) from the directory
// ----------------------------------------------------------------------------
{
    directory_g thisdir = this;

    // Deal with all special cases
    id nty = name->type();
    switch (nty)
    {
    case ID_local:
        // Deal with local variables
        rt.type_error();
        return 0;

    case ID_text:
    {
        // Deal with storing to file
        files_g disk = files::make("data");
        return disk->purge(text_p(name));
    }

    // Special names that are allowed as variable names
    case ID_StatsData:
    case ID_StatsParameters:
    case ID_Equation:
    case ID_PlotParameters:
    case ID_AlgebraConfiguration:
    case ID_AlgebraVariable:
    case ID_CustomMenu:
        break;

    case ID_symbol:
        break;

#define ID(n)
#define SETTING(Name, Low, High, Init)          \
    case ID_##Name:
#define FLAG(Enable, Disable)                   \
    case ID_##Enable:                           \
    case ID_##Disable:
#include "ids.tbl"
        return settings::purge(nty);

    case ID_list:
    case ID_array:
    {
        size_t size = 0;
        for (object_p obj : *list_p(name))
            size += purge(obj);
        return size;
    }

    case ID_integer:
        if (Settings.NumberedVariables())
            break;
        // Fall-through
    default:
        rt.invalid_name_error();
        return 0;
    }

    name = lookup(name);
    if (name)
    {
        size_t   ns     = name->size();
        object_p value  = name + ns;
        if (rt.is_active_directory(value))
        {
            rt.purge_active_directory_error();
            return 0;
        }
        size_t   vs     = value->size();
        size_t   purged = ns + vs;
        object_p header = (object_p) payload();
        object_p body   = header;
        size_t   old    = leb128<size_t>(body); // Old size of directory

        rt.clone_global(value, vs);
        rt.move_globals(name, name + purged);

        if (old < purged)
        {
            record(directory_error,
                   "Purging %u bytes in %u bytes directory", purged, old);
            purged = old;
        }

        adjust_sizes(thisdir, -int(purged));

        // Adjust variables menu
        ui.menu_refresh(ID_VariablesMenu);

        return purged;
    }

    // If nothing purged, return 0
    return 0;
}


size_t directory::purge_all(object_p name)
// ----------------------------------------------------------------------------
//   Purge objects from
// ----------------------------------------------------------------------------
{
    size_t     result = 0;
    directory *dir    = nullptr;
    for (uint depth = 0; (dir = rt.variables(depth)); depth++)
        result += dir->purge(name);
    return result;
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
        object_p name = object_p(p);
        size_t   ns   = name->size();
        p += ns;
        object_p value = object_p(p);
        size_t   vs    = value->size();
        p += vs;

        // Defensive coding against malformed directorys
        if (ns + vs > size)
        {
            record(directory_error,
                   "Malformed directory during enumeration "
                   "(ns=%u vs=%u size=%u)",
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


bool directory::find(uint index, object_p &nref, object_p &vref) const
// ----------------------------------------------------------------------------
//   Return the name of the n-th element in directory
// ----------------------------------------------------------------------------
{
    object_p p     = object_p(payload());
    size_t   size  = leb128<size_t>(p);
    object_p name  = nullptr;
    object_p value = nullptr;

    index++;
    while (index && size)
    {
        name = p;
        size_t   ns   = name->size();
        p += ns;
        value = p;
        size_t   vs    = value->size();
        p += vs;

        // Defensive coding against malformed directorys
        if (ns + vs > size)
        {
            record(directory_error,
                   "Malformed directory searching name (ns=%u vs=%u size=%u)",
                   ns, vs, size);
            return false;     // Malformed directory, quick exit
        }

        size -= (ns + vs);
        index--;
    }
    if (index)
        name = value = nullptr;
    nref = name;
    vref = value;
    return index == 0;
}


object_p directory::name(uint index) const
// ----------------------------------------------------------------------------
//   Return name at given index
// ----------------------------------------------------------------------------
{
    object_p name, value;
    if (find(index, name, value))
        return name;
    return nullptr;
}


object_p directory::value(uint index) const
// ----------------------------------------------------------------------------
//   Return name at given index
// ----------------------------------------------------------------------------
{
    object_p name, value;
    if (find(index, name, value))
        return value;
    return nullptr;
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
    // Check that we have two objects in the stack
    object_p name = rt.stack(0);
    object_p value = rt.stack(1);
    if (name && value && directory::store_here(name, value))
    {
        rt.drop(2);
        return OK;
    }

    // Otherwise, return an error
    return ERROR;
}


COMMAND_BODY(Rcl)
// ----------------------------------------------------------------------------
//   Recall a global variable from current directory
// ----------------------------------------------------------------------------
{
    object_p name = rt.stack(0);
    if (!name)
        return ERROR;

    // Lookup all directorys, starting with innermost one
    if (object_p value = directory::recall_all(name, true))
        return rt.top(value) ? OK : ERROR;

    // Otherwise, return an error
    return ERROR;
}



COMMAND_BODY(Copy)
// ----------------------------------------------------------------------------
//   Arithmetic copy operation
// ----------------------------------------------------------------------------
{
    if (object_p nobj = rt.top())
    {
        if (symbol_p name = nobj->as_quoted<symbol>())
        {
            if (object_g value = rt.stack(1))
                if (directory::store_here(name, value))
                    if (rt.drop() && rt.top(value))
                        return OK;
        }
        else
        {
            rt.invalid_name_error();
        }
    }
    return ERROR;
}


static object::result store_op(object::id op)
// ----------------------------------------------------------------------------
//   Store with a given operation
// ----------------------------------------------------------------------------
{
    object_g name = rt.stack(0);
    object_g value = rt.stack(1);
    if (!name || !value)
        return object::ERROR;
    object_g existing = directory::recall_all(name, true);
    if (!existing)
        return object::ERROR;
    rt.stack(1, existing);
    rt.stack(0, value);
    object_p cmd = object::static_object(op);
    if (object::result res = cmd->evaluate())
        return res;
    value = rt.pop();
    if (value && directory::store_here(name, value))
        return object::OK;
    return object::ERROR;
}


COMMAND_BODY(StoreAdd)          { return store_op(ID_add); }
COMMAND_BODY(StoreSub)          { return store_op(ID_sub); }
COMMAND_BODY(StoreMul)          { return store_op(ID_mul); }
COMMAND_BODY(StoreDiv)          { return store_op(ID_div); }


static object::result store_op(object::id op, object_p cstval)
// ----------------------------------------------------------------------------
//   Store with a given operation
// ----------------------------------------------------------------------------
{
    object_g name = rt.stack(0);
    object_g value = cstval;
    if (!name || !value)
        return object::ERROR;
    object_g existing = directory::recall_all(name, true);
    if (!existing)
        return object::ERROR;
    rt.stack(0, existing);
    rt.push(value);
    object_p cmd = object::static_object(op);
    if (object::result res = cmd->evaluate())
        return res;
    value = rt.top();
    if (value && directory::store_here(name, value))
        return object::OK;
    return object::ERROR;
}


COMMAND_BODY(Increment)
// ----------------------------------------------------------------------------
//   Increment the given variable
// ----------------------------------------------------------------------------
{
    return store_op(ID_add, integer::make(1));
}


COMMAND_BODY(Decrement)
// ----------------------------------------------------------------------------
//   Decrement the given variable
// ----------------------------------------------------------------------------
{
    return store_op(ID_sub, integer::make(1));
}


static object::result recall_op(object::id op)
// ----------------------------------------------------------------------------
//   Store with a given operation
// ----------------------------------------------------------------------------
{
    object_g name = rt.stack(0);
    if (!name)
        return object::ERROR;
    object_g existing = directory::recall_all(name, true);
    if (!existing || !rt.top(existing))
        return object::ERROR;
    object_p cmd = object::static_object(op);
    return cmd->evaluate();
}


COMMAND_BODY(RecallAdd)         { return recall_op(ID_add); }
COMMAND_BODY(RecallSub)         { return recall_op(ID_sub); }
COMMAND_BODY(RecallMul)         { return recall_op(ID_mul); }
COMMAND_BODY(RecallDiv)         { return recall_op(ID_div); }


COMMAND_BODY(Purge)
// ----------------------------------------------------------------------------
//   Purge a global variable from current directory
// ----------------------------------------------------------------------------
{
    object_p name = rt.stack(0);
    if (!name)
        return ERROR;
    if (object_p quoted = name->as_quoted(ID_object))
        name = quoted;

    // Purge the object (HP48 doesn't error out if name does not exist)
    if (directory *dir = rt.variables(0))
        dir->purge(name);
    rt.drop();
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
    if (rt.push(result))
        return OK;
    return  ERROR;
}


COMMAND_BODY(FreeMemory)
// ----------------------------------------------------------------------------
//   Return amount of free memory (available without garbage collection)
// ----------------------------------------------------------------------------
{
    size_t available = rt.available();
    integer_p result = rt.make<integer>(ID_integer, available);
    if (rt.push(result))
        return OK;
    return ERROR;
}


COMMAND_BODY(SystemMemory)
// ----------------------------------------------------------------------------
//   Return the amount of memory that is seen as free by the system
// ----------------------------------------------------------------------------
{
    size_t mem = sys_free_mem();
    integer_p result = rt.make<integer>(ID_integer, mem);
    if (rt.push(result))
        return OK;
    return ERROR;
}


COMMAND_BODY(home)
// ----------------------------------------------------------------------------
//   Return the home directory
// ----------------------------------------------------------------------------
{
    rt.updir(~0U);
    ui.menu_refresh(ID_VariablesMenu);
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


static bool path_callback(object_p name, object_p obj, void *arg)
// ----------------------------------------------------------------------------
//   Find the directory in enclosing directory
// ----------------------------------------------------------------------------
{
    if (obj == object_p(arg))
        if (rt.append(name))
            return true;
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
    ui.menu_refresh(ID_VariablesMenu);
    return OK;
}


COMMAND_BODY(pgdir)
// ----------------------------------------------------------------------------
//   Really the same as 'purge'
// ----------------------------------------------------------------------------
{
    return Purge::evaluate();
}


static bool vars_enumerator(object_p name, object_p value, void *arg)
// ----------------------------------------------------------------------------
//   Callback to add the variable name to the scratchpad
// ----------------------------------------------------------------------------
{
    object_p types = object_p(arg);
    if (types)
    {
        int tval = value->type_value();
        settings::SaveCompatibleTypes sct(!Settings.CompatibleTypes());
        int atval = value->type_value();
        bool found = false;
        if (list_p ltypes = types->as<list>())
        {
            for (object_p li : *ltypes)
            {
                int itval = li->as_int32(0, true);
                found = itval == tval || itval == atval;
                if (found)
                    break;
            }
        }
        else
        {
            int itval = types->as_int32(0, true);
            found = itval == tval || itval == atval;
        }
        if (!found || rt.error())
            return false;
    }

    object_g gname = name;
    size_t sz = name->size();
    byte *p = rt.allocate(sz);
    if (!p)
        return false;
    memmove(p, byte_p(+gname), sz);
    return true;
}


static list_p variables(object_p types)
// ----------------------------------------------------------------------------
//  List all the variables matching the given type in the current directory
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (!dir)
    {
        rt.no_directory_error();
        return nullptr;
    }
    scribble scr;
    dir->enumerate(vars_enumerator, (void *) types);
    list_p list = list::make (scr.scratch(), scr.growth());
    return list;
}


COMMAND_BODY(Vars)
// ----------------------------------------------------------------------------
//   List variables in current directory
// ----------------------------------------------------------------------------
{
    if (list_p list = variables(nullptr))
        if (rt.push(list))
            return OK;
    return ERROR;
}


COMMAND_BODY(TVars)
// ----------------------------------------------------------------------------
//   List variables in current directory matching the type
// ----------------------------------------------------------------------------
{
    if (object_p types = rt.top())
        if (list_p list = variables(types))
            if (rt.top(list))
                return OK;
    return ERROR;
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
    items_init(mi, nitems, 3, 1);
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


static bool evaluate_variable(object_p name, object_p value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
{
    symbol_p disp = name->as<symbol>();
    if (!disp)
        disp = name->as_symbol(true);
    menu::info &mi = *((menu::info *) arg);
    if (value->type() == object::ID_directory)
        mi.marker = L'◥';
    menu::items(mi, disp, menu::ID_VariablesMenuExecute);

    return true;
}


static bool recall_variable(object_p name, object_p UNUSED value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
//   For a name X, we create an object « 'Name' RCL »
{
    symbol_p disp = name->as<symbol>();
    if (!disp)
        disp = name->as_symbol(true);
    menu::info &mi = *((menu::info *) arg);
    menu::items(mi, disp, menu::ID_VariablesMenuRecall);
    return true;
}


static bool store_variable(object_p name, object_p UNUSED value, void *arg)
// ----------------------------------------------------------------------------
//   Add a variable to evaluate in the menu
// ----------------------------------------------------------------------------
{
    symbol_p disp = name->as<symbol>();
    if (!disp)
        disp = name->as_symbol(true);
    menu::info &mi = *((menu::info *) arg);
    menu::items(mi, disp, menu::ID_VariablesMenuStore);
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
}


COMMAND_BODY(VariablesMenuExecute)
// ----------------------------------------------------------------------------
//   Recall a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        if (directory *dir = rt.variables(0))
        {
            uint     index = key - KEY_F1 + 5 * ui.page();
            object_p name, value;
            if (dir->find(index, name, value))
            {
                if (symbol_p sym = name->as_symbol(true))
                {
                    size_t sz = 0;
                    utf8 help = sym->value(&sz);
                    ui.draw_user_command(help, sz);
                }
                return program::run(value);
            }
        }
    }

    return ERROR;
}


INSERT_BODY(VariablesMenuExecute)
// ----------------------------------------------------------------------------
//   Insert the name of a variable
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " ", " ", false);
}


COMMAND_BODY(VariablesMenuRecall)
// ----------------------------------------------------------------------------
//   Recall a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        if (directory *dir = rt.variables(0))
        {
            uint index = key - KEY_F1 + 5 * ui.page();
            if (object_p value = dir->value(index))
                if (rt.push(value))
                    return OK;
        }
    }

    return ERROR;
}


INSERT_BODY(VariablesMenuRecall)
// ----------------------------------------------------------------------------
//   Insert the name of a variable with `Recall` after it
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " '", "' Recall ", false);
}


COMMAND_BODY(VariablesMenuStore)
// ----------------------------------------------------------------------------
//   Store a variable from the VariablesMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        if (directory *dir = rt.variables(0))
        {
            uint index = key - KEY_F1 + 5 * ui.page();
            if (object_p name = dir->name(index))
                if (object_p value = rt.pop())
                    if (dir->store(name, value))
                        return OK;
        }
    }
    return ERROR;
}


INSERT_BODY(VariablesMenuStore)
// ----------------------------------------------------------------------------
//   Insert the name of a variable with `Store` after it
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " '", "' Store ", false);
}



// ============================================================================
//
//   Flag commands
//
// ============================================================================

static byte  *flags      = nullptr;
static size_t flags_size = 0;

static byte *init_flags()
// ----------------------------------------------------------------------------
//  Reinitialize the flags
// ----------------------------------------------------------------------------
{
    size_t maxflags = Settings.MaxFlags();
    maxflags = (maxflags + 7) / 8;
    if (!flags || flags_size != maxflags)
    {
        flags = (byte *) realloc(flags, maxflags);
        if (flags)
        {
            memset(flags + flags_size, 0, maxflags - flags_size);
            flags_size = maxflags;
        }
    }
    if (!flags)
        rt.out_of_memory_error();
    return flags;
}


struct flag_conversion
// ----------------------------------------------------------------------------
//   Conversion between HP system flags and DB48X settings
// ----------------------------------------------------------------------------
// These are documented in section C-1 of the HP50G advanced reference manual
{
    int         index;
    object::id  setting;
};


static flag_conversion flag_conversions[] =
// ----------------------------------------------------------------------------
//   Conversion between HP and DB48X values
// ----------------------------------------------------------------------------
{
    {   -1,     object::ID_PrincipalSolution            },
    {   -2,     object::ID_NumericalConstants           },
    {   -3,     object::ID_NumericalResults             },
    {   -4,     object::ID_CarefulEvaluation            },
    {  -20,     object::ID_UnderflowError               },
    {  -21,     object::ID_OverflowError                },
    {  -22,     object::ID_InfinityValue                },
    {  -23,     object::ID_NegativeUnderflowIndicator   },
    {  -24,     object::ID_PositiveUnderflowIndicator   },
    {  -25,     object::ID_OverflowIndicator            },
    {  -26,     object::ID_InfiniteResultIndicator      },
    {  -29,     object::ID_NoPlotAxes                   },
    {  -29,     object::ID_NoPlotAxes                   },
    {  -31,     object::ID_NoCurveFilling               },
    {  -40,     object::ID_ShowTime                     },
    {  -41,     object::ID_Time24H                      },
    {  -42,     object::ID_DayBeforeMonth               },
    {  -51,     object::ID_DecimalComma                 },
    {  -52,     object::ID_MultiLineResult              },
    {  -55,     object::ID_NoLastArguments              },
    {  -56,     object::ID_BeepOff                      },
    {  -64,     object::ID_IndexWrapped                 },
    {  -65,     object::ID_MultiLineStack               },
    {  -97,     object::ID_VerticalLists                },
    {  -98,     object::ID_VerticalVectors              },
    { -100,     object::ID_FinalAlgebraResults          },
    { -103,     object::ID_ComplexResults               },
};


static object::result do_flag(bool read, bool test, bool write, bool set)
// ----------------------------------------------------------------------------
//   RPL command for changing flag
// ----------------------------------------------------------------------------
{
    object_p   arg     = rt.top();
    object::id aty     = arg->type();
    bool       value   = false;
    bool       builtin = false;
    bool       flip    = !read && !write;

    // Check built-in flags
    if (object_p quoted = arg->as_quoted(object::ID_object))
    {
        arg = quoted;
        aty = arg->type();
    }
    if (int32_t index = arg->as_int32(0, false))
    {
        if (index < 0)
        {
            if (index < -128)
            {
                rt.domain_error();
                return object::ERROR;
            }
            uint max = sizeof(flag_conversions) / sizeof(*flag_conversions);
            for (uint i = 0; i < max; i++)
            {
                if (flag_conversions[i].index == index)
                {
                    aty = flag_conversions[i].setting;
                    break;
                }
            }
        }
    }

    if (read && Settings.flag(aty, &value))
        builtin = true;
    if (write && Settings.flag(aty, set))
        builtin = true;
    if (builtin)
    {
        rt.drop();
        if (read)
        {
            if (!test)
                value = !value;
            object::id rty = value ? object::ID_True : object::ID_False;
            object_p value = command::static_object(rty);
            if (!rt.push(value))
                return object::ERROR;
        }
        return object::OK;
    }

    // Normal numeric flags
    int32_t index = arg->as_int32(0, true);
    if (rt.error())
        return object::ERROR;

    // System flags that were not recognized
    if (index < 0)
    {
        rt.unsupported_flag_error();
        return object::ERROR;
    }

    size_t maxflags = Settings.MaxFlags();
    if (size_t(index) > maxflags)
    {
        rt.index_error();
        return object::ERROR;
    }

    // Allocate memory for the flags if necessary
    if (!init_flags())
        return object::ERROR;
    byte *fp = flags + index / 8;
    byte bits = 1 << (index % 8);
    value = *fp & bits;
    if (flip)
    {
        set = !value;
        write = true;
    }
    if (write)
        *fp = (*fp & ~bits) | (set ? bits : 0);
    rt.drop();
    if (read)
    {
        if (!test)
            value = !value;
        object::id rty = value ? object::ID_True : object::ID_False;
        object_p value = command::static_object(rty);
        if (!rt.push(value))
            return object::ERROR;
    }
    return object::OK;
}


COMMAND_BODY(SetFlag)                   { return do_flag(false, false, true,  true);  }
COMMAND_BODY(ClearFlag)                 { return do_flag(false, false, true,  false); }
COMMAND_BODY(FlipFlag)                  { return do_flag(false, false, false, false); }
COMMAND_BODY(TestFlagSet)               { return do_flag(true,  true,  false, false); }
COMMAND_BODY(TestFlagClear)             { return do_flag(true,  false, false, false); }
COMMAND_BODY(TestFlagClearThenClear)    { return do_flag(true,  false, true,  false); }
COMMAND_BODY(TestFlagClearThenSet)      { return do_flag(true,  false, true,  true); }
COMMAND_BODY(TestFlagSetThenClear)      { return do_flag(true,  true,  true,  false); }
COMMAND_BODY(TestFlagSetThenSet)        { return do_flag(true,  true,  true,  true); }

COMMAND_BODY(FlagsToBinary)
// ----------------------------------------------------------------------------
//   Create a big integer from the flags
// ----------------------------------------------------------------------------
{
    size_t maxflags = Settings.MaxFlags();
    object::id type = ID_based_bignum;
    if (!init_flags())
        return object::ERROR;
    bignum_p binary = rt.make<bignum>(type, byte_p(flags), (maxflags+7)/8);
    if (binary && rt.push(binary))
        return OK;
    return ERROR;
}


COMMAND_BODY(BinaryToFlags)
// ----------------------------------------------------------------------------
//   Store a binary value into the flags
// ----------------------------------------------------------------------------
{
    if (rt.args(1))
    {
        object_p value = rt.top();
        if (value->is_integer())
        {
            bignum_g big;
            if (value->is_bignum())
                big = bignum_p(value);
            else
                big = rt.make<bignum>(integer_g(integer_p(value)));
            size_t sz = 0;
            byte_p data = big->value(&sz);
            size_t maxflags = Settings.MaxFlags();
            if (sz * 8 > maxflags)
                sz = (maxflags + 7) / 8;

            if (!init_flags())
                return object::ERROR;
            memcpy(flags, data, sz);
            if (sz < maxflags / 8)
                memset(flags + sz, 0, (maxflags + 7) / 8 - sz);
            if (rt.drop())
                return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}
