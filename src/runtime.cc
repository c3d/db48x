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

#include "user_interface.h"
#include "object.h"
#include "variables.h"

#include <cstring>


// The one and only runtime
runtime rt(nullptr, 0);
runtime::gcptr *runtime::GCSafe = nullptr;

RECORDER(runtime,       16, "RPL runtime");
RECORDER(runtime_error, 16, "RPL runtime error (anomalous behaviors)");
RECORDER(editor,        16, "Text editor (command line)");
RECORDER(errors,        16, "Runtime errors)");
RECORDER(gc,           256, "Garbage collection events");
RECORDER(gc_errors,     16, "Garbage collection errors");
RECORDER(gc_details,   256, "Details about garbage collection (noisy)");


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
      ErrorCommand(nullptr),
      Code(nullptr),
      LowMem(),
      Globals(),
      Temporaries(),
      Editing(),
      Scratch(),
      Stack(),
      Undos(),
      Locals(),
      Directories(),
      Returns(),
      HighMem()
{
    if (mem)
        memory(mem, size);
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
    Directories = Returns - 1;                  // Make room for one path
    Locals = Directories;                       // No locals
    Undos = Locals;                             // No undos
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
}


void runtime::reset()
// ----------------------------------------------------------------------------
//   Reset the runtime to initial state
// ----------------------------------------------------------------------------
{
    memory((byte *) LowMem, (byte_p) HighMem - (byte_p) LowMem);
}




// ============================================================================
//
//    Temporaries
//
// ============================================================================

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

    for (obj = first; obj < last; obj = next)
    {
        object::id type = obj->type();
        if (type >= object::NUM_IDS)
            return false;
        next = obj->skip();
    }
    if (obj != last)
        return false;

    for (object_p *s = stack; s < stackEnd; s++)
        if (!*s || (*s)->type() >= object::NUM_IDS)
            return false;

    return true;
}


bool runtime::integrity_test()
// ----------------------------------------------------------------------------
//   Check all the objects in a given range
// ----------------------------------------------------------------------------
{
    return integrity_test(rt.Globals,rt.Temporaries,rt.Stack,rt.Returns);
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
                     rt.Globals, rt.Temporaries, rt.Stack, rt.Returns);
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

#endif // SIMULATOR


runtime::gcptr::~gcptr()
// ----------------------------------------------------------------------------
//   Destructor for a garbage-collected pointer
// ----------------------------------------------------------------------------
{
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


size_t runtime::gc()
// ----------------------------------------------------------------------------
//   Recycle unused temporaries
// ----------------------------------------------------------------------------
//   Temporaries can only be referenced from the stack
//   Objects in the global area are copied there, so they need no recycling
//   This algorithm is linear in number of objects and moves only live data
{
    size_t   recycled = 0;
    object_p first    = (object_p) Globals;
    object_p last     = Temporaries;
    object_p free     = first;
    object_p next;

    draw_gc();

    record(gc, "Garbage collection, available %u, range %p-%p",
           available(), first, last);
#ifdef SIMULATOR
    if (!integrity_test(first, last, Stack, Returns))
    {
        record(gc_errors, "Integrity test failed pre-collection");
        RECORDER_TRACE(gc) = 1;
        dump_object_list("Pre-collection failure",
                         first, last, Stack, Returns);
        integrity_test(first, last, Stack, Returns);
        recorder_dump();
    }
    if (RECORDER_TRACE(gc) > 1)
        dump_object_list("Pre-collection",
                         first, last, Stack, Returns);
#endif // SIMULATOR

    object_p *firstobjptr = Stack;
    object_p *lastobjptr = HighMem;
    size_t count = 0;

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
                ||  (ErrorCommand  >= start && ErrorCommand  < end)
                ||  (ui.command    >= start && ui.command    < end);
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
        if (count++ % 0x400 == 0)
            draw_gc();
    }

    // Move the command line and scratch buffer
    if (Editing + Scratch)
    {
        object_p edit = Temporaries;
        move(edit - recycled, edit, Editing + Scratch, true);
    }

    // Adjust Temporaries
    Temporaries -= recycled;


#ifdef SIMULATOR
    if (!integrity_test(Globals, Temporaries, Stack, Returns))
    {
        record(gc_errors, "Integrity test failed post-collection");
        RECORDER_TRACE(gc) = 2;
        dump_object_list("Post-collection failure",
                         first, last, Stack, Returns);
        recorder_dump();
    }
    if (RECORDER_TRACE(gc) > 1)
        dump_object_list("Post-collection",
                         (object_p) Globals, Temporaries,
                         Stack, Returns);
#endif // SIMULATOR

    record(gc, "Garbage collection done, purged %u, available %u",
           recycled, available());
    return recycled;
}


void runtime::move(object_p to, object_p from, size_t size, bool scratch)
// ----------------------------------------------------------------------------
//   Move objects in memory to a new location, adjusting pointers
// ----------------------------------------------------------------------------
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
    object_p last = scratch ? (object_p) Stack : from + size;
    record(gc_details, "Adjustment range is %p-%p, %+s",
           from, last, scratch ? "scratch" : "no scratch");
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
    object_p *lastobjptr = Returns;
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
    if (ErrorCommand >= start && ErrorCommand < end)
        ErrorCommand += delta;
    if (ui.command >= start && ui.command < end)
        ui.command += delta;
}


void runtime::move_globals(object_p to, object_p from)
// ----------------------------------------------------------------------------
//    Move data in the globals area
// ----------------------------------------------------------------------------
//    In that case, we need to move everything up to the scratchpad
{
    object_p last = (object_p) scratchpad() + allocated();
    object_p first = to < from ? to : from;
    size_t moving = last - first;
    move(to, from, moving);

    // Adjust Globals and Temporaries (for Temporaries, must be <=, not <)
    int delta = to - from;
    if (Globals >= first && Globals < last)             // Storing global var
        Globals += delta;
    if (Temporaries >= first && Temporaries <= last)    // Probably always
        Temporaries += delta;
}



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


void runtime::remove(size_t offset, size_t len)
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
}


utf8 runtime::close_editor(bool convert)
// ----------------------------------------------------------------------------
//   Close the editor and encapsulate its content into a string
// ----------------------------------------------------------------------------
//   This will move the editor below the temporaries, encapsulated as
//   a string. After that, it is safe to allocate temporaries without
//   overwriting the editor
{
    // Compute the extra size we need for a string header
    size_t hdrsize = leb128size(object::ID_text) + leb128size(Editing + 1);
    if (available(hdrsize+1) < hdrsize+1)
        return nullptr;

    // Move the editor data above that header
    char *ed = (char *) Temporaries;
    char *str = ed + hdrsize;
    memmove(str, ed, Editing);

    // Null-terminate that string for safe use by C code
    str[Editing] = 0;
    record(editor, "Closing editor size %u at %p [%s]", Editing, ed, str);

    // Write the string header
    text_p obj = text_p(ed);
    ed = leb128(ed, object::ID_text);
    ed = leb128(ed, Editing + 1);

    // Move Temporaries past that newly created string
    Temporaries = (object_p) str + Editing + 1;

    // We are no longer editing
    Editing = 0;

    // Import special characters
    if (convert)
    {
        text_p imported = obj->import();
        if (imported != obj)
            str = (char *) imported->value();
    }

    // Return a pointer to a valid C string safely wrapped in a RPL string
    return utf8(str);
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


size_t runtime::edit()
// ----------------------------------------------------------------------------
//   Append the scratchpad to the editor (at end of buffer)
// ----------------------------------------------------------------------------
{
    record(editor, "Editing scratch pad size %u, editor was %u",
           Scratch, Editing);
    Editing += Scratch;
    Scratch = 0;

    record(editor, "Editor size now %u", Editing);
    return Editing;
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
    byte *ptr = allocate(sz);
    if (ptr)
        memcpy(ptr, bytes.Safe(), sz);
    return ptr;
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
    move(Temporaries, result, Editing + Scratch, true);
    memmove((void *) result, source, size);
    return result;
}


object_p runtime::clone_global(object_p global)
// ----------------------------------------------------------------------------
//   Check if any entry in the stack points to a given global, if so clone it
// ----------------------------------------------------------------------------
{
    object_p cloned = nullptr;
    object_p *begin = Stack;
    object_p *end = Returns;
    for (object_p *s = begin; s < end; s++)
    {
        if (*s == global)
        {
            if (!cloned)
                cloned = clone(global);
            *s = cloned;
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
    ASSERT(obj && "Pushing a NULL object");

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
    if (Stack >= Undos)
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
    ASSERT(obj && "Putting a NULL object on top of stack");

    if (Stack >= Undos)
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
    if (Stack >= Undos)
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
//   Local variables
//
// ============================================================================

object_p runtime::local(uint index)
// ----------------------------------------------------------------------------
//   Fetch local at given index
// ----------------------------------------------------------------------------
{
    size_t count = Directories - Locals;
    if (index >= count)
    {
        invalid_local_error();
        return nullptr;
    }
    return Locals[index];
}


bool runtime::local(uint index, object_p obj)
// ----------------------------------------------------------------------------
//   Set a local in the local stack
// ----------------------------------------------------------------------------
{
    size_t count = Directories - Locals;
    if (index >= count)
    {
        invalid_local_error();
        return false;
    }
    Locals[index] = obj;
    return true;
}


bool runtime::locals(size_t count)
// ----------------------------------------------------------------------------
//   Allocate the given number of locals from stack
// ----------------------------------------------------------------------------
{
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
    Undos -= count;
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
    // Sanity check on what we remove
    if (count > size_t(Directories - Locals))
    {
        invalid_local_error();
        return false;
    }

    // Move pointers up
    object_p *oldp = Locals;
    Stack += count;
    Undos += count;
    Locals += count;
    object_p *newp = Locals;
    size_t moving = Locals - Stack;
    for (size_t i = 0; i < moving; i++)
        *(--newp) = *(--oldp);

    return true;
}



// ============================================================================
//
//   Directories
//
// ============================================================================

bool runtime::enter(directory_p dir)
// ----------------------------------------------------------------------------
//   Enter a given directory
// ----------------------------------------------------------------------------
{
    size_t sz = sizeof(dir);
    if (available(sz) < sz)
        return false;

    // Move pointers down
    Stack--;
    Undos--;
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
    size_t depth = (object_p *) Returns - Directories;
    if (count >= depth - 1)
        count = depth - 1;
    if (!count)
        return false;

    // Move pointers up
    object_p *oldp = Directories;
    Stack += count;
    Undos += count;
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
//   Return stack
//
// ============================================================================

void runtime::call(object_g callee)
// ------------------------------------------------------------------------
//   Push the current object on the RPL stack
// ------------------------------------------------------------------------
{
    if (available(sizeof(callee)) < sizeof(callee))
    {
        recursion_error();
        return;
    }
    Stack--;
    Locals--;
    for (object_p *s = Locals; s < Stack; s++)
        s[0] = s[1];
    *(--Returns) = Code;
    Code = callee;
}


void runtime::ret()
// ----------------------------------------------------------------------------
//   Return from an RPL call
// ----------------------------------------------------------------------------
{
    if ((byte *) Returns >= (byte *) HighMem)
    {
        return_without_caller_error();
        return;
    }
    Code = *Returns++;
    Stack++;
    Locals++;
    for (object_p *s = Stack; s > Stack; s--)
        s[0] = s[-1];
}



// ============================================================================
//
//   Generation of the error functions
//
// ============================================================================

#define ERROR(name, msg)                        \
runtime &runtime::name##_error()                \
{                                               \
    return error(msg);                          \
}
#include "errors.tbl"
