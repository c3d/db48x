// ****************************************************************************
//  runtime.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of the RPL runtime
//
//     See memory layout in header file
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

#include "runtime.h"

#include "arithmetic.h"
#include "compare.h"
#include "constants.h"
#include "expression.h"
#include "integer.h"
#include "object.h"
#include "program.h"
#include "user_interface.h"
#include "variables.h"

#include <cstring>



RECORDER(runtime,       16, "RPL runtime");
RECORDER(runtime_error, 16, "RPL runtime error (anomalous behaviors)");
RECORDER(editor,        16, "Text editor (command line)");
RECORDER(errors,        16, "Runtime errors)");
RECORDER(gc,           256, "Garbage collection events");
RECORDER(gc_errors,     16, "Garbage collection errors");
RECORDER(gc_details,   256, "Details about garbage collection (noisy)");
RECORDER(cache,        256, "Cached values for the stack");


// ============================================================================
//
//   Runtime invariants check
//
// ============================================================================

#ifdef SIMULATOR
void runtime_invariants::check_invariants()
// ----------------------------------------------------------------------------
//  Check runtime invariants on entry and return from runtime code
// ----------------------------------------------------------------------------
{
    ASSERT(rt.Returns <= rt.HighMem);
    ASSERT(rt.CallStack <= rt.Returns);
    ASSERT(rt.XLibs <= rt.Constants);
    ASSERT(rt.Constants <= rt.CallStack);
    ASSERT(rt.Directories <= rt.XLibs);
    ASSERT(rt.Locals <= rt.Directories);
    ASSERT(rt.Undo <= rt.Locals);
    ASSERT(rt.Args <= rt.Undo);
    ASSERT(rt.Stack <= rt.Args);
};
#endif // SIMULATOR



// ============================================================================
//
//   Initialization
//
// ============================================================================

runtime::runtime(byte *mem, size_t size)
// ----------------------------------------------------------------------------
//   Runtime constructor
// ----------------------------------------------------------------------------
    : Error(nullptr),
      ErrorSave(nullptr),
      ErrorSource(nullptr),
      ErrorSrcLen(0),
      ErrorCommand(nullptr),
      LowMem(),
      Globals(),
      Temporaries(),
      Editing(),
      Scratch(),
      Stack(),
      Args(),
      Undo(),
      Locals(),
      Directories(),
      XLibs(),
      Constants(),
      CallStack(),
      Returns(),
      HighMem(),
      Cache(),
      CacheIndex(),
      GCCycles(),
      GCPurged(),
      GCDuration(),
      GCLPurged(),
      GCLDuration(),
      GCCleared(),
      GCUnclear(),
      SaveArgs(false)
{
    if (mem)
        memory(mem, size);
    runtime_invariants check;
}


void runtime::memory(byte *memory, size_t size)
// ----------------------------------------------------------------------------
//   Assign the given memory range to the runtime
// ----------------------------------------------------------------------------
{
    LowMem = (object_p) memory;
    HighMem = (object_p *) (memory + size);

    // Stuff at top of memory
    Returns = HighMem;                          // No return stack
    CallStack = Returns;                        // Reserve space for call stack
    Constants = CallStack;                      // No Constants loaded
    XLibs = Constants;                          // No XLibs loaded
    Directories = XLibs - 1;                    // Make room for one path
    Locals = Directories;                       // No locals
    Args = Locals;                              // No args
    Undo = Locals;                              // No undo stack
    Stack = Locals;                             // Empty stack

    // Stuff at bottom of memory
    Globals = LowMem;
    directory_p home = new((void *) Globals) directory();   // Home directory
    *Directories = (object_p) home;             // Current search path
    Globals = home->skip();                     // Globals after home
    Temporaries = Globals;                      // Area for temporaries
    Editing = 0;                                // No editor
    Scratch = 0;                                // No scratchpad

    record(runtime, "Memory %p-%p size %u (%uK)",
           LowMem, HighMem, size, size>>10);
    runtime_invariants check;
}


void runtime::reset()
// ----------------------------------------------------------------------------
//   Reset the runtime to initial state
// ----------------------------------------------------------------------------
{
    memory((byte *) LowMem, (byte_p) HighMem - (byte_p) LowMem);
    runtime_invariants check;
}




// ============================================================================
//
//    Temporaries
//
// ============================================================================

#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

size_t runtime::available()
// ----------------------------------------------------------------------------
//   Return the size available for temporaries
// ----------------------------------------------------------------------------
{
    size_t aboveTemps = Editing + Scratch + redzone;
    return (byte *) Stack - (byte *) Temporaries - aboveTemps;
}


size_t runtime::available(size_t size)
// ----------------------------------------------------------------------------
//   Check if we have enough for the given size
// ----------------------------------------------------------------------------
{
    if (available() < size)
    {
        gc();
        size_t avail = available();
        if (avail < size)
            out_of_memory_error();
        return avail;
    }
    return size;
}



// ============================================================================
//
//   Garbage collection
//
// ============================================================================

#ifdef SIMULATOR
bool runtime::integrity_test(object_p first,
                             object_p last,
                             object_p *stack,
                             object_p *stackEnd)
// ----------------------------------------------------------------------------
//   Check all the objects in a given range
// ----------------------------------------------------------------------------
{
    object_p next, obj;
    uint count = 0;

    for (obj = first; obj < last; obj = next)
    {
        object::id type = obj->type();
        if (type >= object::NUM_IDS)
        {
            record(runtime_error, "Object at %p (%u) has type %u (max %u)",
                   obj, count, type, object::NUM_IDS);
            object::object_error(type, obj);
            return false;
        }
        next = obj->skip();
        count++;
    }
    if (obj != last)
    {
        record(runtime_error, "Reach past last object, %p vs %p, count=%u",
               obj, last, count);
        return false;
    }
    for (object_p *s = stack; s < stackEnd; s++)
    {
        if (!*s || (*s)->type() >= object::NUM_IDS)
        {
            record(runtime_error, "Stack at %p (%u) has type %u (max %u)",
                   *s, s - stack, (*s) ? (*s)->type() : object::ID_object,
                   object::NUM_IDS);
            if (*s)
                object::object_error((*s)->type(), *s);
            return false;
        }
    }
    return true;
}


bool runtime::integrity_test()
// ----------------------------------------------------------------------------
//   Check all the objects in a given range
// ----------------------------------------------------------------------------
{
    return integrity_test(rt.Globals,rt.Temporaries,rt.Stack,rt.XLibs);
}


void runtime::dump_object_list(cstring  message,
                               object_p first,
                               object_p last,
                               object_p *stack,
                               object_p *stackEnd)
// ----------------------------------------------------------------------------
//   Dump all objects in a given range
// ----------------------------------------------------------------------------
{
    uint count = 0;
    size_t sz = 0;
    object_p next;

    record(gc, "%+s object list", message);
    for (object_p obj = first; obj < last; obj = next)
    {
        object::id i = obj->type();
        if (i >= object::NUM_IDS)
        {
            record(gc_errors, " %p: corrupt object ID type %u", obj, i);
            break;
        }

        next = obj->skip();
        record(gc, " %p+%llu: %+s (%d)",
               obj, next - obj, object::name(i), i);
        sz += next - obj;
        count++;
    }
    record(gc, "%+s stack", message);
    for (object_p *s = stack; s < stackEnd; s++)
        record(gc, " %u: %p (%+s)",
               s - stack, *s,
               *s ? object::name((*s)->type()) : utf8("null"));
    record(gc, "%+s: %u objects using %u bytes", message, count, sz);
}


void runtime::dump_object_list(cstring  message)
// ----------------------------------------------------------------------------
//   Dump object list for the runtime
// ----------------------------------------------------------------------------
{
    dump_object_list(message,
                     rt.Globals, rt.Temporaries, rt.Stack, rt.Args);
}


void runtime::object_validate(unsigned      typeID,
                              const object *object,
                              size_t        size)
// ----------------------------------------------------------------------------
//   Check if an object we created is valid
// ----------------------------------------------------------------------------
{
    object::id type = (object::id) typeID;
    if (object->size() != size)
        object::object_error(type, object);
}




void runtime::dump_gc_pointers()
// ----------------------------------------------------------------------------
//   Helper function to dump all the GC pointers
// ----------------------------------------------------------------------------
{
    uint i = 0;
    for (gcptr *p = GCSafe; p; p = p->next)
        printf("%4u: %p: %p\n", ++i, p, p->safe);
}
#endif // SIMULATOR


runtime::gcptr::~gcptr()
// ----------------------------------------------------------------------------
//   Destructor for a garbage-collected pointer
// ----------------------------------------------------------------------------
{
    lock it;
    gcptr *last = nullptr;
    if (this == rt.GCSafe)
    {
        rt.GCSafe = next;
        return;
    }

    for (gcptr *gc = rt.GCSafe; gc; gc = gc->next)
    {
        if (gc == this)
        {
            last->next = gc->next;
            return;
        }
        last = gc;
    }
}


object_p runtime::cached(bool level0, object_p key)
// ----------------------------------------------------------------------------
//   Check if there is a value associated to this object
// ----------------------------------------------------------------------------
{
    size_t max = sizeof(Cache[level0]) / sizeof(Cache[level0][0]);
    for (uint i = 0; i < max; i += 2)
    {
        uint j = (CacheIndex - i) % max;
        if (Cache[level0][j] == key)
        {
            record(cache, "Got %p for %p at %u.%u",
                   Cache[level0][j+1], key, level0, j);
            return Cache[level0][j+1];
        }
    }
    record(cache, "Did not find %p", key);
    return nullptr;
}


bool runtime::cache(bool level0, object_p key, object_p value)
// ----------------------------------------------------------------------------
//   Cache the value
// ----------------------------------------------------------------------------
{
    size_t max = sizeof(Cache[level0]) / sizeof(Cache[level0][0]);
    for (uint i = 0; i < max; i += 2)
    {
        uint j = (CacheIndex - i) % max;
        if (Cache[level0][j] == key)
        {
            record(cache, "Replace %p with %p for %p at %u.%u",
                   Cache[level0][j+1], value, key, level0, j);
            Cache[level0][j+1] = value;
            return true;
        }
    }
    CacheIndex = (CacheIndex + 2) % max;
    record(cache, "Set  %p for %p at %u.%u, erasing %p",
           value, key, level0, CacheIndex, Cache[level0][CacheIndex+1]);
    Cache[level0][CacheIndex] = key;
    Cache[level0][CacheIndex + 1] = value;
    return false;
}


void runtime::uncache(object_p start, size_t sz)
// ----------------------------------------------------------------------------
//   Drop the whole cache
// ----------------------------------------------------------------------------
{
    object_p end = start + sz;
    record(cache, "Clear cache %p-%p sz %u", start, end, sz);
    for (uint l = 0; l < 2; l++)
    {
        const uint max = sizeof(Cache[0]) / sizeof(Cache[0][0]);
        for (uint i = 0; i < max; i++)
        {
            object_p &ptr = Cache[l][i];
            if (ptr >= start && ptr < end)
                ptr = nullptr;
        }
    }
}


size_t runtime::gc()
// ----------------------------------------------------------------------------
//   Recycle unused temporaries
// ----------------------------------------------------------------------------
//   Temporaries can only be referenced from the stack
//   Objects in the global area are copied there, so they need no recycling
//   This algorithm is linear in number of objects and moves only live data
{
    lock     it;
    uint     now      = sys_current_ms();
    size_t   recycled = 0;
    object_p first    = (object_p) Globals;
    object_p last     = Temporaries;
    object_p free     = first;
    object_p next;

    ui.draw_busy(L'●', Settings.GCIconForeground());

    record(gc, "Garbage collection, available %u, range %p-%p",
           available(), first, last);
#ifdef SIMULATOR
    if (!integrity_test(first, last, Stack, XLibs))
    {
        record(gc_errors, "Integrity test failed pre-collection");
        RECORDER_TRACE(gc) = 1;
        dump_object_list("Pre-collection failure",
                         first, last, Stack, XLibs);
        integrity_test(first, last, Stack, XLibs);
        recorder_dump();
    }
    if (RECORDER_TRACE(gc) > 1)
        dump_object_list("Pre-collection",
                         first, last, Stack, XLibs);
#endif // SIMULATOR

    object_p *firstobjptr = Stack;
    object_p *lastobjptr = HighMem;

    for (object_p obj = first; obj < last; obj = next)
    {
        bool found = false;
        next = obj->skip();
        record(gc_details, "Scanning object %p (ends at %p)", obj, next);
        for (object_p *s = firstobjptr; s < lastobjptr && !found; s++)
        {
            found = *s >= obj && *s < next;
            if (found)
                record(gc_details, "Found %p at stack level %u",
                       obj, s - firstobjptr);
        }
        if (!found)
        {
            for (gcptr *p = GCSafe; p && !found; p = p->next)
            {
                found = p->safe >= (byte *) obj && p->safe <= (byte *) next;
                if (found)
                    record(gc_details, "Found %p in GC-safe pointer %p (%p)",
                           obj, p->safe, p);
            }
        }
        if (!found)
        {
            // Check if some of the error information was user-supplied
            utf8 start = utf8(obj);
            utf8 end = utf8(next);
            found = (Error         >= start && Error         < end)
                ||  (ErrorSave     >= start && ErrorSave     < end)
                ||  (ErrorSource   >= start && ErrorSource   < end)
                ||  (ErrorCommand  >= obj   && ErrorCommand  < next)
                ||  (ui.command    >= start && ui.command    < end)
                ||  (ui.keymap     >= obj   && ui.keymap     < next);
            if (!found)
            {
                utf8 *label = (utf8 *) &ui.menuLabel[0][0];
                for (uint l = 0; !found && l < ui.NUM_MENUS; l++)
                    found = label[l] >= start && label[l] < end;

                object_p *functions = &ui.function[0][0];
                const uint max = sizeof(ui.function)/sizeof(ui.function[0][0]);
                for (uint k = 0; !found && k < max; k++)
                    found = functions[k] >= obj && functions[k] < next;

                if (!found)
                {
                    object_p vi = object_p(ui.validate_input);
                    found = (vi >= obj && vi < next);
                }
            }
        }

        if (found)
        {
            // Move object to free space
            record(gc_details, "Moving %p-%p to %p", obj, next, free);
            move(free, obj, next - obj);
            free += next - obj;
        }
        else
        {
            recycled += next - obj;
            record(gc_details, "Recycling %p size %u total %u",
                   obj, next - obj, recycled);
        }
    }

    // Move the command line and scratch buffer
    if (Editing + Scratch)
    {
        object_p edit = Temporaries;
        move(edit - recycled, edit, Editing + Scratch, 1, true);
    }

    // Adjust Temporaries
    Temporaries -= recycled;


#ifdef SIMULATOR
    if (!integrity_test(Globals, Temporaries, Stack, XLibs))
    {
        record(gc_errors, "Integrity test failed post-collection");
        RECORDER_TRACE(gc) = 2;
        dump_object_list("Post-collection failure",
                         first, last, Stack, XLibs);
        recorder_dump();
    }
    if (RECORDER_TRACE(gc) > 1)
        dump_object_list("Post-collection",
                         (object_p) Globals, Temporaries,
                         Stack, XLibs);
#endif // SIMULATOR

    record(gc, "Garbage collection done, purged %u, available %u",
           recycled, available());

    ui.draw_busy();

    // Clear the cache
    uncache();

    // Update statistics
    uint duration = sys_current_ms() - now;
    GCCycles += 1;
    GCLPurged = recycled;
    GCLDuration = duration;
    GCPurged += recycled;
    GCDuration += duration;

    return recycled;
}


void runtime::move(object_p to, object_p from,
                   size_t size, size_t overscan, bool scratch)
// ----------------------------------------------------------------------------
//   Move objects in memory to a new location, adjusting pointers
// ----------------------------------------------------------------------------
//   This is called from various places that need to move memory.
//   - During garbage collection, when we move an object to its new location.
//     In that case, we don't want to move a pointer that is outside of object.
//   - When writing a global variable and moving everything above it.
//     In that case, we need to move everything up to the end of temporaries.
//   - When building temporary objects in the scratchpad
//     In that case, the object is not yet referenced by the stack, but we
//     may have gcp that are just above temporaries, so overscan is 1
//   The scratch flag indicates that we move the scratch area. In that case,
//   we don't need to adjust stack or function pointers, only gc-safe pointers.
//   Furthermore, scratch pointers may (temporarily) be above the scratch area.
//   See list parser for an example.
{
    int delta = to - from;
    if (!delta)
        return;

    // Move the object in memory
    memmove((byte *) to, (byte *) from, size);

    // Adjust the protected pointers
    object_p last = from + size + overscan;
    record(gc_details, "Move %p to %p size %u, %+s",
           from, to, size, scratch ? "scratch" : "no scratch");
    for (gcptr *p = GCSafe; p; p = p->next)
    {
        if (p->safe >= (byte *) from && p->safe < (byte *) last)
        {
            record(gc_details, "Adjusting GC-safe %p from %p to %p",
                   p, p->safe, p->safe + delta);
            p->safe += delta;
        }
    }

    // No need to walk the stack pointers and function pointers
    if (scratch)
        return;

    // Adjust the stack pointers
    object_p *firstobjptr = Stack;
    object_p *lastobjptr = HighMem;
    for (object_p *s = firstobjptr; s < lastobjptr; s++)
    {
        if (*s >= from && *s < last)
        {
            record(gc_details, "Adjusting stack level %u from %p to %p",
                   s - firstobjptr, *s, *s + delta);
            *s += delta;
        }
    }

    // Adjust error messages
    utf8 start = utf8(from);
    utf8 end   = utf8(last);
    if (Error >= start && Error < end)
        Error += delta;
    if (ErrorSave >= start && ErrorSave < end)
        ErrorSave += delta;
    if (ErrorSource >= start && ErrorSource < end)
        ErrorSource += delta;
    if (ErrorCommand >= from && ErrorCommand < last)
        ErrorCommand += delta;
    if (ui.command >= start && ui.command < end)
        ui.command += delta;
    if (object_p vi = object_p(ui.validate_input))
        if (vi >= from && vi < last)
            ui.validate_input = (typeof(ui.validate_input))(vi + delta);

    // Adjust menu labels
    utf8 *label = (utf8 *) &ui.menuLabel[0][0];
    for (uint l = 0; l < ui.NUM_MENUS; l++)
        if (label[l] >= start && label[l] < end)
            label[l] += delta;

    // Adjust keymap
    if (ui.keymap >= from && ui.keymap < last)
        ui.keymap = list_p(object_p(ui.keymap) + delta);

    // Adjust functions
    object_p *functions = &ui.function[0][0];
    const uint max = sizeof(ui.function) / sizeof(ui.function[0][0]);
    for (uint k = 0; k < max; k++)
        if (functions[k] >= from && functions[k] < last)
            functions[k] += delta;
}


void runtime::move_globals(object_p to, object_p from)
// ----------------------------------------------------------------------------
//    Move data in the globals area
// ----------------------------------------------------------------------------
//    In that case, we need to move everything up to the scratchpad
{
    // We overscan by 1 to deal with gcp that point to end of objects
    object_p last = (object_p) scratchpad() + allocated();
    object_p first = to < from ? to : from;
    size_t moving = last - from;
    move(to, from, moving, 1);

    // Adjust Globals and Temporaries (for Temporaries, must be <=, not <)
    int delta = to - from;
    if (Globals >= first && Globals < last)             // Storing global var
        Globals += delta;
    Temporaries += delta;

    // Remove all cached entries, they may be covered by what we moved
    uncache(from, moving);
}

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42




// ============================================================================
//
//    Editor
//
// ============================================================================

size_t runtime::insert(size_t offset, utf8 data, size_t len)
// ----------------------------------------------------------------------------
//   Insert data in the editor, return size inserted
// ----------------------------------------------------------------------------
{
    record(editor,
           "Insert %u bytes at offset %u starting with %c, %u available",
           len, offset, data[0], available());
    if (offset <= Editing)
    {
        if (available(len) >= len)
        {
            size_t moved = Scratch + Editing - offset;
            byte_p edr = (byte_p) editor() + offset;
            move(object_p(edr + len), object_p(edr), moved);
            memcpy(editor() + offset, data, len);
            Editing += len;
            return len;
        }
    }
    else
    {
        record(runtime_error,
               "Invalid insert at %zu size=%zu len=%zu [%s]\n",
               offset, Editing, len, data);
    }
    return 0;
}


size_t runtime::remove(size_t offset, size_t len)
// ----------------------------------------------------------------------------
//   Remove characers from the editor
// ----------------------------------------------------------------------------
{
    record(editor, "Removing %u bytes at offset %u", len, offset);
    size_t end = offset + len;
    if (end > Editing)
        end = Editing;
    if (offset > end)
        offset = end;
    len = end - offset;
    size_t moving = Scratch + Editing - end;
    byte_p edr = (byte_p) editor() + offset;
    move(object_p(edr), object_p(edr + len), moving);
    Editing -= len;
    return len;
}


text_p runtime::close_editor(bool convert, bool trailing_zero)
// ----------------------------------------------------------------------------
//   Close the editor and encapsulate its content into a string
// ----------------------------------------------------------------------------
//   This will move the editor below the temporaries, encapsulated as
//   a string. After that, it is safe to allocate temporaries without
//   overwriting the editor
{
    // Compute the extra size we need for a string header
    size_t tzs = trailing_zero ? 1 : 0;
    size_t hdrsize = leb128size(object::ID_text) + leb128size(Editing + tzs);
    if (available(hdrsize+tzs) < hdrsize+tzs)
        return nullptr;

    // Move the editor data above that header
    char *ed = (char *) Temporaries;
    char *str = ed + hdrsize;
    memmove(str, ed, Editing);

    // Null-terminate that string for safe use by C code
    if (trailing_zero)
        str[Editing] = 0;
    record(editor, "Closing editor size %u at %p [%s]", Editing, ed, str);

    // Write the string header
    text_p obj = text_p(ed);
    ed = leb128(ed, object::ID_text);
    ed = leb128(ed, Editing + tzs);

    // Move Temporaries past that newly created string
    Temporaries = (object_p) str + Editing + tzs;

    // We are no longer editing
    Editing = 0;

    // Import special characters if necessary (importing text file)
    if (convert)
        obj = obj->import();

    // Return a pointer to a valid C string safely wrapped in a RPL string
    return obj;
}


size_t runtime::edit(utf8 buf, size_t len)
// ----------------------------------------------------------------------------
//   Open the editor with a known buffer
// ----------------------------------------------------------------------------
{
    gcutf8 buffer = buf;        // Need to keep track of GC movements

    if (available(len) < len)
    {
        record(editor, "Insufficent memory for %u bytes", len);
        out_of_memory_error();
        Editing = 0;
        return 0;
    }

    // Copy the scratchpad up (available() ensured we have room)
    if (Scratch)
        memmove((char *) Temporaries + len, Temporaries, Scratch);

    memcpy((byte *) Temporaries, (byte *) buffer, len);
    Editing = len;
    return len;
}


size_t runtime::edit(size_t offset, size_t len)
// ----------------------------------------------------------------------------
//   Append the scratchpad to the editor without any memory allocation
// ----------------------------------------------------------------------------
//   Start with: bbbbb|aaa|ssss|iiiiiiii|
//                    O   E    I        S
//   where O=offset, E=Editor, I=Scratch-Len, S=Scratch
//         b=text before, a=text after, s=scratch to keep, i=scratch to insert
//
//   End with:   bbbbb|iiiiiiii|aaa|ssss|
//                    O            E    S
//
//   First swapping pass:
//   Step 1:     bbbbb|iaa|ssss|iiiiiiia|
//                    O   E    I        S
//   Step 2:     bbbbb|iiaa|ssss|iiiiiiaa|
//                    O   E    I        S
//   ...
//   Step N:     bbbbb|iiiiiiii|ssss|aaa|
//                    O   E    I        S
//
//   Second swapping pass:
//   Step 1:     bbbbb|iiiiiiii|asss|aas|
//                    O   E    I        S
//   Step 2:     bbbbb|iiiiiiii|aass|ass|
//                    O   E    I        S
//   ...
//   Step N:     bbbbb|iiiiiiii|aaa|ssss|
//                    O   E    I        S
{
    if (len > Scratch || offset > Editing)
    {
        record(runtime_error, "Invalid edit %u > %u, offset %u into %u",
               len, Scratch, offset, Editing);
        return 0;
    }
    record(editor, "Editing %u scratch %u, offset %u in editor %u",
           len, Scratch, offset, Editing);
    if (Scratch > len || offset < Editing)
    {
        // Move data around in place, two passes swapping order of bytes
        byte *ed = editor();
        size_t end = Scratch + Editing;
        size_t swapping = (end - offset)/2;
        for (size_t idx = 0; idx < swapping; idx++)
            std::swap(ed[end + ~idx], ed[offset+idx]);
        swapping = len / 2;
        size_t tend = offset + len;
        for (size_t idx = 0; idx < swapping; idx++)
            std::swap(ed[tend + ~idx], ed[offset + idx]);
        swapping = (Editing - offset + Scratch - len) / 2;
        for (size_t idx = 0; idx < swapping; idx++)
            std::swap(ed[end + ~idx], ed[tend + idx]);
    }
    Editing += len;
    Scratch -= len;

    record(editor, "Editor size now %u", Editing);
    return len;
}


byte *runtime::allocate(size_t sz)
// ----------------------------------------------------------------------------
//   Allocate additional bytes at end of scratchpad
// ----------------------------------------------------------------------------
{
    if (available(sz) >= sz)
    {
        byte *scratch = editor() + Editing + Scratch;
        Scratch += sz;
        return scratch;
    }

    // Ran out of memory despite garbage collection
    return nullptr;
}


byte *runtime::append(size_t sz, gcbytes bytes)
// ----------------------------------------------------------------------------
//   Append some bytes at end of scratch pad
// ----------------------------------------------------------------------------
{
    if (!+bytes)
        return nullptr;
    byte *ptr = allocate(sz);
    if (ptr)
        memmove(ptr, +bytes, sz);
    return ptr;
}


byte *runtime::append(object_p obj)
// ----------------------------------------------------------------------------
//   Append an object at end of scratch pad
// ----------------------------------------------------------------------------
{
    return obj ? append(obj, obj->size()) : nullptr;
}


byte *runtime::append(object_p obj, size_t sz)
// ----------------------------------------------------------------------------
//   Append an object at end of scratch pad with a specific size
// ----------------------------------------------------------------------------
{
    return append(sz, byte_p(obj));
}


byte *runtime::append_expression(object_p obj)
// ----------------------------------------------------------------------------
//   Append an object at end of scratch pad, strip expressions
// ----------------------------------------------------------------------------
{
    if (!obj)
        return nullptr;
    size_t sz = obj->size();
    if (expression_p eq = obj->as<expression>())
        obj = eq->objects(&sz);
    return append(obj, sz);
}


object_p runtime::clone(object_p source)
// ----------------------------------------------------------------------------
//   Clone an object into the temporaries area
// ----------------------------------------------------------------------------
//   This is useful when storing into a global referenced from the stack
{
    size_t size = source->size();
    if (available(size) < size)
        return nullptr;
    object_p result = Temporaries;
    Temporaries = object_p((byte *) Temporaries + size);
    move(Temporaries, result, Editing + Scratch, 1, true);
    memmove((void *) result, source, size);
    return result;
}


object_p runtime::clone_global(object_p global, size_t sz)
// ----------------------------------------------------------------------------
//   Check if any entry in the stack points to a given global, if so clone it
// ----------------------------------------------------------------------------
//   We scan everywhere to see if an object is used. If so, we clone it
//   and adjust the pointer to the cloned value
//   We clone the object at most once, and adjust objects in a list or
//   program to preserve the original structure
{
    object_p  cloned = nullptr;
    object_p *begin  = Stack;
    object_p *end    = HighMem;
    uncache(global);
    for (object_p *s = begin; s < end; s++)
    {
        if (*s >= global && *s < global + sz)
        {
            if (!cloned)
                cloned = clone(global);
            *s = cloned + (*s - global);
        }
    }
    return cloned;
}


object_p runtime::clone_if_dynamic(object_p obj)
// ----------------------------------------------------------------------------
//   Clone object if it is in memory
// ----------------------------------------------------------------------------
//   This is useful to make a "small" copy of an object that currently lives in
//   a larger object, making it possible to free the larger object.
//   It will not clone a ROM-based object, e.g. the result of a
//   command::static_object call.
//   A use case is evaluating a menu:
//   - If you do it from the keyboard, we can keep the ROM object
//   - If you run from state load, this would force the whole command line to
//     stay in memory until you use another menu, which is wasteful, since the
//     command-line used to load the whole state may be quite large
{
    if (obj >= LowMem && obj <= object_p(HighMem))
        obj = clone(obj);
    return obj;
}


object_p runtime::clone_stack(uint level)
// ----------------------------------------------------------------------------
//    Clone a stack level if dynamic, but also try to reuse lower stack
// ----------------------------------------------------------------------------
//    This is done after we load the state with the following intent:
//    - Clone what is on the command-line so that we can purge it
//    - In the frequent case where the same object is on the stack multiple
//      times, chances are it is from a DUP or similar, so reunify the objects
{
    if (object_p obj = stack(level))
    {
        size_t size = obj->size();
        record(runtime,
               "Cloning stack level %u from %p size %u",
               level, obj, size);
        for (uint d = 0; d < level; d++)
        {
            if (object_p lower = stack(d))
            {
                if (lower->size() == size && memcmp(lower, obj, size) == 0)
                {
                    // Identical object, keep the lower one
                    stack(level, lower);
                    record(runtime, "  Level %u obj %p is a match", d, lower);
                    return lower;
                }
            }
        }

        if (object_p clone = clone_if_dynamic(obj))
        {
            stack(level, clone);
            record(runtime, "  cloned as %p", clone);
            return clone;
        }
    }
    return nullptr;
}


void runtime::clone_stack()
// ----------------------------------------------------------------------------
//   Clone all levels on the stack
// ----------------------------------------------------------------------------
{
    uint depth = this->depth();
    for (uint d = 0; d < depth; d++)
    {
        object_p ptr = clone_stack(d);
        record(runtime, "Cloned stack level %d as %p", d, ptr);
    }
}



// ============================================================================
//
//   RPL stack
//
// ============================================================================

bool runtime::push(object_g obj)
// ----------------------------------------------------------------------------
//   Push an object on top of RPL stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    ASSERT(obj && "Pushing a NULL object");
    ASSERT((obj->type() < object::NUM_IDS ||
            object::object_error(obj->type(), obj)) &&
           "Invalid type pushed");

    // This may cause garbage collection, hence the need to adjust
    if (available(sizeof(void *)) < sizeof(void *))
        return false;
    *(--Stack) = obj;
    return true;
}


object_p runtime::top()
// ----------------------------------------------------------------------------
//   Return the top of the runtime stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (Stack >= Args)
    {
        missing_argument_error();
        return nullptr;
    }
    return *Stack;
}


bool runtime::top(object_p obj)
// ----------------------------------------------------------------------------
//   Set the top of the runtime stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    ASSERT(obj && "Putting a NULL object on top of stack");

    if (Stack >= Args)
    {
        missing_argument_error();
        return false;
    }
    *Stack = obj;
    return true;
}


object_p runtime::pop()
// ----------------------------------------------------------------------------
//   Pop the top-level object from the stack, or return NULL
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (Stack >= Args)
    {
        missing_argument_error();
        return nullptr;
    }
    return *Stack++;
}


object_p runtime::stack(uint idx)
// ----------------------------------------------------------------------------
//    Get the object at a given position in the stack
// ----------------------------------------------------------------------------
{
    if (idx >= depth())
    {
        missing_argument_error();
        return nullptr;
    }
    return Stack[idx];
}


bool runtime::stack(uint idx, object_p obj)
// ----------------------------------------------------------------------------
//    Get the object at a given position in the stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (idx >= depth())
    {
        missing_argument_error();
        return false;
    }
    Stack[idx] = obj;
    return true;
}


bool runtime::roll(uint idx)
// ----------------------------------------------------------------------------
//    Move the object at a given position in the stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (idx)
    {
        idx--;
        if (idx >= depth())
        {
            missing_argument_error();
            return false;
        }
        object_p s = Stack[idx];
        memmove(Stack + 1, Stack, idx * sizeof(*Stack));
        *Stack = s;
    }
    return true;
}


bool runtime::rolld(uint idx)
// ----------------------------------------------------------------------------
//    Get the object at a given position in the stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (idx)
    {
        idx--;
        if (idx >= depth())
        {
            missing_argument_error();
            return false;
        }
        object_p s = *Stack;
        memmove(Stack, Stack + 1, idx * sizeof(*Stack));
        Stack[idx] = s;
    }
    return true;
}


bool runtime::drop(uint count)
// ----------------------------------------------------------------------------
//   Pop the top-level object from the stack, or return NULL
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (count > depth())
    {
        missing_argument_error();
        return false;
    }
    Stack += count;
    return true;
}



// ============================================================================
//
//   Args and undo management
//
// ============================================================================

bool runtime::args(uint count)
// ----------------------------------------------------------------------------
//   Add 'count' stack objects to the saved arguments
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t nstk = depth();
    if (count > nstk)
    {
        missing_argument_error();
        return false;
    }
    if (SaveArgs)
    {
        size_t nargs = args();
        if (count > nargs)
        {
            size_t sz = (count - nargs) * sizeof(object_p);
            if (available(sz) < sz)
                return false;
        }

        memmove(Stack + nargs - count, Stack, nstk * sizeof(object_p));
        Stack = Stack + nargs - count;
        Args = Args + nargs - count;
        memmove(Args, Stack, count * sizeof(object_p));
        SaveArgs = false;
    }
    return true;
}


bool runtime::last()
// ----------------------------------------------------------------------------
//   Push back the last arguments on the stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t nargs = args();
    size_t sz = nargs * sizeof(object_p);
    if (available(sz) < sz)
        return false;

    Stack -= nargs;
    memmove(Stack, Args, nargs * sizeof(object_p));
    return true;
}


bool runtime::last(uint index)
// ----------------------------------------------------------------------------
//   Push back the last argument on the stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t nargs = args();
    if (index >= nargs)
    {
        rt.missing_argument_error();
        return false;
    }
    size_t sz = sizeof(object_p);
    if (available(sz) < sz)
        return false;

    *--Stack = Args[index];
    return true;
}


bool runtime::save()
// ----------------------------------------------------------------------------
//   Save the stack in the undo area
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t scount = depth();
    size_t ucount = saved();
    if (scount > ucount)
    {
        size_t sz = (scount - ucount) * sizeof(object_p);
        if (available(sz) < sz)
            return false;
    }

    object_p *ns = Stack + ucount - scount;
    ASSERT(ns + (Undo - Stack) < HighMem);
    ASSERT(Stack + depth() < HighMem);
    memmove(ns, Stack, (Undo - Stack) * sizeof(object_p));
    Stack = ns;
    Args = Args + ucount - scount;
    Undo = Undo + ucount - scount;
    memmove(Undo, Stack, depth() * sizeof(object_p));

    return true;
}


bool runtime::undo()
// ----------------------------------------------------------------------------
//   Revert the stack to what it was before
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t ucount = saved();
    size_t scount = depth();
    if (ucount > scount)
    {
        size_t sz = (ucount - scount) * sizeof(object_p);
        if (available(sz) < sz)
            return false;
    }

    Stack = Stack + scount - ucount;
    memmove(Stack, Undo, ucount * sizeof(object_p));

    return true;
}


runtime &runtime::command(object_p cmd)
// ----------------------------------------------------------------------------
//   Set the command name and initialize the undo setup
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    ErrorCommand = cmd;
    return *this;
}



// ============================================================================
//
//   Local variables
//
// ============================================================================

object_p runtime::local(uint index)
// ----------------------------------------------------------------------------
//   Fetch local at given index
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t count = Directories - Locals;
    if (index >= count)
    {
        undefined_local_name_error();
        return nullptr;
    }
    return Locals[index];
}


object_p runtime::local(uint index, object_p obj)
// ----------------------------------------------------------------------------
//   Set a local in the local stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check
;
    size_t count = Directories - Locals;
    if (index >= count || !obj)
    {
        undefined_local_name_error();
        return nullptr;
    }
    Locals[index] = obj;
    return obj;
}


bool runtime::locals(size_t count)
// ----------------------------------------------------------------------------
//   Allocate the given number of locals from stack
// ----------------------------------------------------------------------------
{
    runtime_invariants check
;
    // We need that many arguments
    if (count > depth())
    {
        missing_argument_error();
        return false;
    }

    // Check if we have the memory
    size_t req = count * sizeof(void *);
    if (available(req) < req)
        return false;

    // Move pointers down
    Stack -= count;
    Args -= count;
    Undo -= count;
    Locals -= count;
    size_t moving = Locals - Stack;
    for (size_t i = 0; i < moving; i++)
        Stack[i] = Stack[i + count];

    // In `→ X Y « X Y - X Y +`, X is level 1 of the stack, Y is level 0
    for (size_t var = 0; var < count; var++)
        Locals[count - 1 - var] = *Stack++;

    return true;
}


bool runtime::unlocals(size_t count)
// ----------------------------------------------------------------------------
//    Free the given number of locals
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    if (count)
    {
        // Sanity check on what we remove
        if (count > size_t(Directories - Locals))
        {
            undefined_local_name_error();
            return false;
        }

        // Move pointers up
        object_p *oldp = Locals;
        Stack += count;
        Args += count;
        Undo += count;
        Locals += count;
        object_p *newp = Locals;
        size_t moving = Locals - Stack;
        for (size_t i = 0; i < moving; i++)
            *(--newp) = *(--oldp);
    }

    return true;
}



// ============================================================================
//
//   Directories
//
// ============================================================================

bool runtime::is_active_directory(object_p obj) const
// ----------------------------------------------------------------------------
//    Check if a global variable is referenced by the directories
// ----------------------------------------------------------------------------
{
    size_t depth = (object_p *) XLibs - Directories;
    for (size_t i = 0; i < depth; i++)
        if (obj == Directories[i])
            return true;
    return false;
}


bool runtime::enter(directory_p dir)
// ----------------------------------------------------------------------------
//   Enter a given directory
// ----------------------------------------------------------------------------
{
    runtime_invariants check;

    // Check if this is a directory up
    size_t depth = (object_p *) XLibs - Directories;
    for (size_t i = 0; i < depth; i++)
        if (dir == Directories[i])
            return !i || updir(i);

    size_t sz = sizeof(directory_p);
    if (available(sz) < sz)
        return false;

    // Move pointers down
    Stack--;
    Args--;
    Undo--;
    Locals--;
    Directories--;

    size_t moving = Directories - Stack;
    for (size_t i = 0; i < moving; i++)
        Stack[i] = Stack[i + 1];

    // Update directory
    *Directories = dir;

    return true;
}


bool runtime::updir(size_t count)
// ----------------------------------------------------------------------------
//   Move one directory up
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t depth = XLibs - Directories;
    if (count >= depth - 1)
        count = depth - 1;
    if (!count)
        return false;

    // Move pointers up
    object_p *oldp = Directories;
    Stack += count;
    Args += count;
    Undo += count;
    Locals += count;
    Directories += count;

    object_p *newp = Directories;
    size_t moving = Directories - Stack;
    for (size_t i = 0; i < moving; i++)
        *(--newp) = *(--oldp);

    return true;
}


// ============================================================================
//
//   XLibs and constants
//
// ============================================================================

bool runtime::attach(size_t nentries)
// ----------------------------------------------------------------------------
//   Change the number of xlibs to the given number, then zero the new ones
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t existing = xlibs();
    size_t count    = nentries - existing;
    if (count)
    {
        if (nentries > existing)
        {
            size_t needed = (nentries - existing) * sizeof(object_p);
            if (available(needed) < needed)
                return false;
            for (object_p *ptr = Stack; ptr < Constants; ptr++)
                ptr[-count] = ptr[0];
            for (object_p *ptr = Constants - count; ptr < Constants; ptr++)
                *ptr = nullptr;
        }
        else if (nentries < existing)
        {
            for (object_p *ptr = Constants-1; ptr >= Stack; ptr--)
                ptr[0] = ptr[count];
        }
        Stack       -= count;
        Args        -= count;
        Undo        -= count;
        Locals      -= count;
        Directories -= count;
        XLibs       -= count;
    }
    return true;
}


bool runtime::constants(size_t nentries)
// ----------------------------------------------------------------------------
//   Change the number of constants to the given number, then zero the new ones
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t existing = constants();
    size_t count    = nentries - existing;
    if (count)
    {
        if (nentries > existing)
        {
            size_t needed = (nentries - existing) * sizeof(object_p);
            if (available(needed) < needed)
                return false;
            for (object_p *ptr = Stack; ptr < CallStack; ptr++)
                ptr[-count] = ptr[0];
            for (object_p *ptr = CallStack - count; ptr < CallStack; ptr++)
                *ptr = nullptr;
        }
        else if (nentries < existing)
        {
            for (object_p *ptr = CallStack-1; ptr >= Stack; ptr--)
                ptr[0] = ptr[count];
        }
        Stack       -= count;
        Args        -= count;
        Undo        -= count;
        Locals      -= count;
        Directories -= count;
        XLibs       -= count;
        Constants   -= count;
    }
    return true;
}



// ============================================================================
//
//   Return stack
//
// ============================================================================

#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

bool runtime::run_conditionals(object_p truecase, object_p falsecase, bool xeq)
// ----------------------------------------------------------------------------
//   Push the two conditionals
// ----------------------------------------------------------------------------
{
    object_g tc = truecase;
    object_g tce = truecase ? truecase->skip() : nullptr;
    object_g fc = falsecase;
    object_g fce = falsecase ? falsecase->skip() : nullptr;

    if (xeq)
    {
        // For IFT / IFTE, we want to execute programs, not put them on stack
        if (tc && tc->is_program())
            tc = program_p(+tc)->objects();
        if (fc && fc->is_program())
            fc = program_p(+fc)->objects();
    }

    return run_push(tc, tce) && run_push(fc, fce);
}


bool runtime::run_select(bool condition)
// ----------------------------------------------------------------------------
//   Select which condition path to pick
// ----------------------------------------------------------------------------
//   In this case, we have pushed the true condition and the false condition.
//   We only leave one depending on whether the condition is true or not
{
    if (Returns + 4 > HighMem)
    {
        record(runtime_error,
               "select (%+s) Returns=%p HighMem=%p",
               condition ? "true" : "false",
               Returns, HighMem);
        return false;
    }


    if (!condition)
    {
        Returns[3] = Returns[1];
        Returns[2] = Returns[0];
    }

    call_stack_drop(2);
    return true;
}


bool runtime::run_select_while(bool condition)
// ----------------------------------------------------------------------------
//   Select which condition of a while path to pick
// ----------------------------------------------------------------------------
//   In that case, we have pushed the loop and its body
//   If the condition is true, we leave loop and body
//   If the condition is false, we drop both
{
    if (Returns + 4 > HighMem)
    {
        record(runtime_error,
               "select_while (%+s) Returns=%p HighMem=%p",
               condition ? "true" : "false",
               Returns, HighMem);
        return false;
    }

    if (!condition)
        call_stack_drop(4);
    return true;
}


bool runtime::run_select_start_step(bool for_loop, bool has_step)
// ----------------------------------------------------------------------------
//   Select evaluation branches in a for loop
// ----------------------------------------------------------------------------
{
    if (Returns + 4 > HighMem)
    {
        record(runtime_error,
               "select_start_step (%+s %+s) Returns=%p HighMem=%p",
               for_loop ? "for" : "start",
               has_step ? "step" : "next",
               Returns, HighMem);
        return false;
    }

    bool down = false;
    algebraic_g step;
    object::id  ty = for_loop ? object::ID_ForStep : object::ID_StartStep;
    if (has_step)
    {
        object_p obj = rt.pop();
        if (!obj)
            return false;
        step = obj->as_algebraic();
        if (!step)
        {
            object_p cmd = command::static_object(ty);
            rt.command(cmd).type_error();
            return false;
        }
        down = step->is_negative();
    }
    else
    {
        step = integer::make(1);
        if (!step)
            return false;
    }

    // Increment and compare with last iteration
    if (for_loop)
        Returns[0] = rt.local(0);
    algebraic_g cur  = Returns[0] ? Returns[0]->as_algebraic() : nullptr;
    algebraic_g last = Returns[1] ? Returns[1]->as_algebraic() : nullptr;
    if (!cur || !last)
    {
        object_p cmd = command::static_object(ty);
        rt.command(cmd);
        return false;
    }
    cur = cur + step;
    last = down ? (cur < last) : (cur > last);
    if (cur)
        Returns[0] = cur;

    // Write the current value in the variable if it's a for loop
    if (for_loop)
        rt.local(0, cur);

    // Check the truth value
    int finished = last ? last->as_truth(true) : -1;
    if (finished < 0)
        return false;

    if (finished)
    {
        call_stack_drop(4);
    }
    else
    {
        object::id type = object::id(object::ID_start_next_conditional
                                     + 2*for_loop
                                     + has_step);
        return object::defer(type) && run_push_data(Returns[4], Returns[5]);
    }

    return true;
}


bool runtime::run_select_list(bool for_loop)
// ----------------------------------------------------------------------------
//   Select evaluation branches in a for loop evaluating lists
// ----------------------------------------------------------------------------
{
    if (Returns + 4 > HighMem)
    {
        record(runtime_error,
               "select_list (%+s %+s) Returns=%p HighMem=%p",
               for_loop ? "for" : "start",
               Returns, HighMem);
        return false;
    }

    // Increment and compare with last iteration
    object_p cur  = Returns[0];
    object_p last = Returns[1];
    if (!cur || !last)
    {
        object::id ty = for_loop?object::ID_ForStep:object::ID_StartStep;
        object_p cmd = command::static_object(ty);
        rt.command(cmd);
        return false;
    }
    // Check the truth value
    bool finished = +cur >= +last;
    if (finished)
    {
        call_stack_drop(4);
    }
    else
    {
        // Write the current value in the variable if it's a for loop
        if (for_loop)
            rt.local(0, cur);
        Returns[0] = cur->skip();

        object::id type =
            for_loop ? object::ID_for_next_list : object::ID_start_next_list;
        return object::defer(type) && run_push_data(Returns[4], Returns[5]);
    }

    return true;
}


bool runtime::run_select_case(bool condition)
// ----------------------------------------------------------------------------
//   Select evaluation branches in a case statement
// ----------------------------------------------------------------------------
//   In that case, we have the true case at level 0, null at level 2
//   If the condition is true, we put an ID_case_skip_conditional in level 2
//
{
    runtime_invariants check;
    if (Returns + 4 > HighMem)
    {
        record(runtime_error,
               "select_case (%+s) Returns=%p HighMem=%p",
               condition ? "true" : "false",
               Returns, HighMem);
        return false;
    }

    if (condition)
    {
        object_p obj = command::static_object(object::ID_case_skip_conditional);
        ASSERT(Returns[0] == nullptr && Returns[1] + 1 == nullptr);
        Returns[0] = Returns[2];
        Returns[1] = Returns[3];
        Returns[2] = obj;
        Returns[3] = obj->skip() - 1;
    }
    else
    {
        call_stack_drop(4);
    }

    return true;
}


bool runtime::call_stack_grow(object_p &next, object_p &end)
// ----------------------------------------------------------------------------
//   Grow the call stack by a block
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    size_t   block = sizeof(object_p) * CALLS_BLOCK ;
    object_g nextg = next;
    object_g endg  = end;
    if (available(block) < block)
    {
        recursion_error();
        return false;
    }
    for (object_p *s = Stack; s < CallStack; s++)
        s[-CALLS_BLOCK] = s[0];
    Stack -= CALLS_BLOCK;
    Args -= CALLS_BLOCK;
    Undo -= CALLS_BLOCK;
    Locals -= CALLS_BLOCK;
    Directories -= CALLS_BLOCK;
    XLibs -= CALLS_BLOCK;
    Constants -= CALLS_BLOCK;
    CallStack -= CALLS_BLOCK;
    next = nextg;
    end = endg;
    return true;
}


void runtime::call_stack_drop()
// ----------------------------------------------------------------------------
//   Drop the outermost block
// ----------------------------------------------------------------------------
{
    runtime_invariants check;
    Stack += CALLS_BLOCK;
    Args += CALLS_BLOCK;
    Undo += CALLS_BLOCK;
    Locals += CALLS_BLOCK;
    Directories += CALLS_BLOCK;
    XLibs += CALLS_BLOCK;
    Constants += CALLS_BLOCK;
    CallStack += CALLS_BLOCK;
    for (object_p *s = CallStack-1; s >= Stack; s--)
        s[0] = s[-CALLS_BLOCK];
}

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42


// ============================================================================
//
//   Generation of the error functions
//
// ============================================================================

object_p runtime::command() const
// ----------------------------------------------------------------------------
//   Return the name associated with the command
// ----------------------------------------------------------------------------
{
    if (ErrorCommand)
        return ErrorCommand;
    return nullptr;
}


algebraic_p runtime::infinity(bool negative) const
// ----------------------------------------------------------------------------
//   Return a symbolic or numerical infinity
// ----------------------------------------------------------------------------
{
    algebraic_g infinity = constant::lookup(negative ? "−∞" : "∞");
    if (!infinity)
        return nullptr;
    if (Settings.NumericalConstants() || Settings.NumericalResults())
        infinity = constant_p(+infinity)->value();
    return infinity;
}


algebraic_p runtime::zero_divide(bool negative) const
// ----------------------------------------------------------------------------
//   Return a zero divide or an infinity constant
// ----------------------------------------------------------------------------
{
    if (Settings.InfinityError())
    {
        rt.zero_divide_error();
        return nullptr;
    }
    Settings.InfiniteResultIndicator(true);
    return infinity(negative);
}


algebraic_p runtime::numerical_overflow(bool negative) const
// ----------------------------------------------------------------------------
//   Return a numerical overflow result
// ----------------------------------------------------------------------------
{
    if (Settings.OverflowError())
    {
        rt.overflow_error();
        return nullptr;
    }
    Settings.OverflowIndicator(true);
    return infinity(negative);
}


algebraic_p runtime::numerical_underflow(bool negative) const
// ----------------------------------------------------------------------------
//   Return a numerical underflow result
// ----------------------------------------------------------------------------
{
    if (Settings.UnderflowError())
    {
        if (negative)
            rt.negative_underflow_error();
        else
            rt.positive_underflow_error();
        return nullptr;
    }
    if (negative)
        Settings.NegativeUnderflowIndicator(true);
    else
        Settings.PositiveUnderflowIndicator(true);
    return integer::make(0);
}


algebraic_p runtime::undefined_result() const
// ----------------------------------------------------------------------------
//   Return an undefined result
// ----------------------------------------------------------------------------
{
    if (Settings.UndefinedError())
    {
        rt.undefined_operation_error();
        return nullptr;
    }
    Settings.UndefinedResultIndicator(true);
    return constant::lookup("?");
}


#define ERROR(name, msg)                        \
runtime &runtime::name##_error()                \
{                                               \
    return error(msg);                          \
}
#include "errors.tbl"



error_save::error_save()
// ----------------------------------------------------------------------------
//  Save the current error state for local evaluation
// ----------------------------------------------------------------------------
    : errmsg(rt.error()),
      source(rt.source()),
      srclen(rt.source_length()),
      command(rt.command())
{
    rt.clear_error();
}


error_save::~error_save()
// ----------------------------------------------------------------------------
//  Restore saved error state
// ----------------------------------------------------------------------------
{
    rt.error(errmsg).source(source, srclen).command(+command);
}



// ============================================================================
//
//    Class that automatically cleans up temporaries
//
// ============================================================================

cleaner::cleaner()
// ----------------------------------------------------------------------------
//    Save the current temporaries and the latest number of GC Cycles
// ----------------------------------------------------------------------------
    : temporaries(rt.Temporaries), gccycles(rt.GCCycles + rt.GCUnclear)
{}


RECORDER(cleaner, 256, "Runtime temporary object cleaner");

object_p cleaner::adjust(object_p temp)
// ----------------------------------------------------------------------------
//   Check if we can cleanup temporaries
// ----------------------------------------------------------------------------
{
    if (temp && temp > temporaries &&
        gccycles == rt.GCCycles + rt.GCUnclear &&
        Settings.AutomaticTemporariesCleanup())
    {
        size_t sz = temp->size();
        record(cleaner,
               "Cleaning %u for %u bytes from %p to %p, T from %p to %p",
               temp - temporaries, sz, temp, temporaries,
               rt.Temporaries, temporaries + sz);
        rt.GCCleared += temp - temporaries;
        memmove((void *) temporaries, temp, sz);
        if (size_t scsz = rt.Editing + rt.Scratch)
            rt.move(temporaries + sz, rt.Temporaries, scsz, 1, 1);
        if (rt.command() >= temporaries)
            rt.command(nullptr);
        temp = temporaries;
        rt.Temporaries = temp + sz;
    }
    return temp;
}
