#ifndef RUNTIME_H
#define RUNTIME_H
// ****************************************************************************
//  runtime.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     The basic RPL runtime
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
//   Layout in memory is as follows
//
//      HighMem         End of usable memory
//        [Pointer to return address N]
//        [... intermediate return addresses ...]
//        [Pointer to return address 0]
//      Returns
//        [... Returns reserve]
//      CallStack
//        [Pointers to constants or 0 if not loaded]
//      Constants
//        [Pointers to xlibs or 0 if not loaded]
//      XLibs
//        [Pointer to outermost directory in path]
//        [ ... intermediate directory pointers ...]
//        [Pointer to innermost directory in path]
//      Directories     Bottom of stack, start of global
//        [Local N]
//        [...]
//        [Local 0]
//      Locals
//        [Last stack from command-line evaluation]
//      Undo
//        [Arguments to last command]
//      Args
//        [User stack]
//      Stack        Top of stack
//        .
//        .
//        .
//        [Free, may be temporarily written prior to being put in scratch]
//        .
//        .
//        .
//      Scratch         Binary scratch pad (to assemble objects like lists)
//        [Scratchpad allocated area]
//      Editor          The text editor
//        [Text editor contents]
//      Temporaries     Temporaries, allocated up
//        [Previously allocated temporary objects, can be garbage collected]
//      Globals         End of global named RPL objects
//        [Top-level directory of global objects]
//      LowMem          Bottom of memory
//
//   When allocating a temporary, we move 'Temporaries' up
//   When allocating stuff on the stack, we move Stack down
//   Everything above Stack is word-aligned
//   Everything below Temporaries is byte-aligned
//   Stack elements point to temporaries, globals or robjects (read-only)
//   Everything above Stack is pointers to garbage-collected RPL objects

#include "recorder.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

#if SIMULATOR
#include <mutex>
#endif


struct object;                  // RPL object
struct directory;               // Directory (storing global variables)
struct symbol;                  // Symbols (references to a directory)
struct text;
struct algebraic;
typedef const object *object_p;
typedef const directory *directory_p;
typedef const text *text_p;
typedef const algebraic *algebraic_p;

RECORDER_DECLARE(runtime);
RECORDER_DECLARE(runtime_error);
RECORDER_DECLARE(errors);
RECORDER_DECLARE(gc);
RECORDER_DECLARE(editor);

// The one and only runtime
struct runtime;
extern runtime rt;


// ============================================================================
//
//   Runtime invariants check
//
// ============================================================================

struct runtime_invariants
// ----------------------------------------------------------------------------
//  Check runtime invariants on entry and return from runtime code
// ----------------------------------------------------------------------------
{
#ifdef SIMULATOR
    runtime_invariants()
    {
        check_invariants();
    }
    ~runtime_invariants()
    {
        check_invariants();
    }
    void check_invariants();
#else // !SIMULATOR
    runtime_invariants()        {}
    ~runtime_invariants()       {}
#endif // SIMULATOR
};



struct runtime
// ----------------------------------------------------------------------------
//   The RPL runtime information
// ----------------------------------------------------------------------------
{
    runtime(byte *mem = nullptr, size_t size = 0);
    ~runtime() {}

    void memory(byte *memory, size_t size);
    // ------------------------------------------------------------------------
    //   Assign the given memory range to the runtime
    // ------------------------------------------------------------------------

    void reset();
    // ------------------------------------------------------------------------
    //   Reset to initial state
    // ------------------------------------------------------------------------

    // Amount of space we want to keep between stack top and temporaries
    const uint redzone = 2*sizeof(object_p);;

#if SIMULATOR
    struct lock : std::lock_guard<std::mutex>
    {
        lock() : std::lock_guard<std::mutex>(rt.mutex) {}
    };
    std::mutex mutex;
#else // !SIMULATOR
    struct lock
    {
        lock() {}
        ~lock() {}
    };
#endif // SIMULATOR


    // ========================================================================
    //
    //    Temporaries
    //
    // ========================================================================

    size_t available();
    // ------------------------------------------------------------------------
    //   Return the size available for temporaries
    // ------------------------------------------------------------------------

    size_t available(size_t size);
    // ------------------------------------------------------------------------
    //   Check if we have enough for the given size
    // ------------------------------------------------------------------------

    template <typename Obj, typename ... Args>
    const Obj *make(typename Obj::id type, const Args &... args);
    // ------------------------------------------------------------------------
    //   Make a new temporary of the given size
    // ------------------------------------------------------------------------

    template <typename Obj, typename ... Args>
    const Obj *make(const Args &... args);
    // ------------------------------------------------------------------------
    //   Make a new temporary of the given size
    // ------------------------------------------------------------------------

    object_p clone(object_p source);
    // ------------------------------------------------------------------------
    //   Clone an object into the temporaries area
    // ------------------------------------------------------------------------

    object_p clone_global(object_p source, size_t sz);
    // ------------------------------------------------------------------------
    //   Clone values in the stack that point to a global we will change
    // ------------------------------------------------------------------------

    object_p clone_if_dynamic(object_p source);
    // ------------------------------------------------------------------------
    //   Clone value if it is in RAM (i.e. not a command::static_object)
    // ------------------------------------------------------------------------

    template<typename T>
    T *clone_if_dynamic(T *source)
    // ------------------------------------------------------------------------
    //   Typed variant of the above
    // ------------------------------------------------------------------------
    {
        object_p obj = source;
        return (T *) clone_if_dynamic(obj);
    }

    object_p clone_stack(uint level);
    // -------------------------------------------------------------------------
    //    Clone a stack object if dynamic, but check if identical to lower stack
    // -------------------------------------------------------------------------

    void clone_stack();
    // -------------------------------------------------------------------------
    //    Clone all levels on the stack
    // -------------------------------------------------------------------------

    void need_save()
    // ------------------------------------------------------------------------
    //   Indicate that we need to save arguments
    // ------------------------------------------------------------------------
    {
        SaveArgs = true;
    }


    bool is_valid_object(object_p obj)
    // ------------------------------------------------------------------------
    //   Check if the object is valid
    // ------------------------------------------------------------------------
    {
        return !(intptr_t(obj) <= 0x1000 ||
                 (obj >= Temporaries && obj <= object_p(HighMem)));
    }


    // ========================================================================
    //
    //    Command-line editor (and buffer for renderer)
    //
    // ========================================================================

    byte *editor()
    // ------------------------------------------------------------------------
    //   Return the buffer for the editor
    // ------------------------------------------------------------------------
    //   This must be called each time a GC could have happened
    {
        byte *ed = (byte *) Temporaries;
        return ed;
    }


    size_t edit(utf8 buffer, size_t len);
    // ------------------------------------------------------------------------
    //   Open the editor with a known buffer
    // ------------------------------------------------------------------------

    size_t edit(size_t offset, size_t len);
    // ------------------------------------------------------------------------
    //   Append the scratch pad to the editor (at end)
    // ------------------------------------------------------------------------


    text_p close_editor(bool convert = false, bool trailing_zero = true);
    // ------------------------------------------------------------------------
    //   Close the editor and encapsulate its content in a temporary string
    // ------------------------------------------------------------------------


    size_t editing()
    // ------------------------------------------------------------------------
    //   Current size of the editing buffer
    // ------------------------------------------------------------------------
    {
        return Editing;
    }


    void clear()
    // ------------------------------------------------------------------------
    //   Clear the editor
    // ------------------------------------------------------------------------
    {
        Editing = 0;
    }


    size_t insert(size_t offset, utf8 data, size_t len);
    // ------------------------------------------------------------------------
    //   Insert data in the editor, return size inserted
    // ------------------------------------------------------------------------


    size_t insert(size_t offset, byte c)
    // ------------------------------------------------------------------------
    //   Insert a single character in the editor
    // ------------------------------------------------------------------------
    {
        return insert(offset, &c, 1);
    }


    size_t insert(size_t offset, utf8 data)
    // ------------------------------------------------------------------------
    //   Insert a null-terminated command name
    // ------------------------------------------------------------------------
    {
        return insert(offset, data, strlen(cstring(data)));
    }


    size_t remove(size_t offset, size_t len);
    // ------------------------------------------------------------------------
    //   Remove characers from the editor
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //   Cached association (for stack rendering acceleration)
    //
    // ========================================================================

    object_p cached(bool level0, object_p key);
    bool     cache(bool level0, object_p key, object_p value);
    void     uncache(object_p key, size_t sz);
    void     uncache(object_p key)      { uncache(key, 1); }
    void     uncache()                  { uncache(nullptr, ~0UL); }


    // ========================================================================
    //
    //   Object management
    //
    // ========================================================================

    size_t gc();
    // ------------------------------------------------------------------------
    //   Garbage collector (purge unused objects from memory to make space)
    // ------------------------------------------------------------------------


    void move(object_p to, object_p from,
              size_t sz, size_t overscan = 0, bool scratch=false);
    // ------------------------------------------------------------------------
    //    Like memmove, but update pointers to objects
    // ------------------------------------------------------------------------


    void move_globals(object_p to, object_p from);
    // ------------------------------------------------------------------------
    //    Move data in the globals area (move everything up to end of scratch)
    // ------------------------------------------------------------------------


    struct gcptr
    // ------------------------------------------------------------------------
    //   Protect a pointer against garbage collection
    // ------------------------------------------------------------------------
    {
        gcptr(byte *ptr = nullptr) : safe(ptr)
        {
            lock it;
            next = rt.GCSafe;
            rt.GCSafe = this;
        }
        gcptr(const gcptr &o): safe(o.safe)
        {
            lock it;
            next = rt.GCSafe;
            rt.GCSafe = this;
        }
        ~gcptr();

        operator byte  *() const                { return safe; }
        operator byte *&()                      { return safe; }
        byte *&Safe()                           { return safe; }
        byte *&operator+()                      { return safe; }
        byte *operator+() const                 { return safe; }
        operator bool() const                   { return safe != nullptr; }
        operator bool()                         { return safe != nullptr; }
        operator int() const                    = delete;
        gcptr &operator =(const gcptr &o)       { safe = o.safe; return *this; }
        gcptr &operator++()                     { safe++; return *this; }
        gcptr &operator+=(size_t sz)            { safe += sz; return *this; }
        friend gcptr operator+(const gcptr &left, size_t right)
        {
            gcptr result = left;
            result += right;
            return result;
        }

    private:
        byte  *safe;
        gcptr *next;

        friend struct runtime;
    };


    template<typename Obj>
    struct gcp : gcptr
    // ------------------------------------------------------------------------
    //   Protect a pointer against garbage collection
    // ------------------------------------------------------------------------
    {
        gcp(Obj *obj = nullptr): gcptr((byte *) obj) {}
        ~gcp() {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
        operator Obj *() const          { return (Obj *) safe; }
        operator Obj *&()               { return (Obj *&) safe; }
        const Obj *Safe() const         { return (const Obj *) safe; }
        const Obj *operator+() const    { return (const Obj *) safe; }
        const Obj *&operator+()         { return (const Obj * &) safe; }
        operator bool() const           { return safe != nullptr; }
        operator bool()                 { return safe != nullptr; }
        Obj *&Safe()                    { return (Obj *&) safe; }
        const Obj &operator *() const   { return *((Obj *) safe); }
        Obj &operator *()               { return *((Obj *) safe); }
        Obj *operator ->() const        { return (Obj *) safe; }
        gcp &operator++()               { safe += sizeof(Obj); return *this; }
        gcp &operator+=(size_t sz)      { safe += sz*sizeof(Obj);return *this; }
        friend gcp operator+(const gcp &left, size_t right)
        {
            gcp result = left;
            result += right;
            return result;
        }
#pragma GCC diagnostic pop
    };


#ifdef SIMULATOR
    static bool integrity_test(object_p first,
                               object_p last,
                               object_p *stack,
                               object_p *stackEnd);
    static bool integrity_test();
    static void dump_object_list(cstring  message,
                                 object_p first,
                                 object_p last,
                                 object_p *stack,
                                 object_p *stackEnd);
    static void dump_object_list(cstring  message);
    static void object_validate(unsigned typeID,
                                const object *obj,
                                size_t size);
    static void dump_gc_pointers();
#endif // SIMULATOR


    // ========================================================================
    //
    //   Scratchpad
    //
    // ========================================================================
    //   The scratchpad is a temporary area to store binary data
    //   It is used for example while building complex or composite objects

    byte *scratchpad()
    // ------------------------------------------------------------------------
    //   Return the buffer for the scratchpad
    // ------------------------------------------------------------------------
    //   This must be called each time a GC could have happened
    {
        byte *scratch = (byte *) Temporaries + Editing + Scratch;
        return scratch;
    }

    size_t allocated()
    // ------------------------------------------------------------------------
    //   Return the size of the temporary scratchpad
    // ------------------------------------------------------------------------
    {
        return Scratch;
    }

    byte *allocate(size_t sz);
    // ------------------------------------------------------------------------
    //   Allocate additional bytes at end of scratchpad
    // ------------------------------------------------------------------------


    byte *append(size_t sz, gcp<const byte> bytes);
    byte *append(object_p obj);
    byte *append(object_p obj, size_t sz);
    byte *append_expression(object_p obj);
    // ------------------------------------------------------------------------
    //   Append some bytes at end of scratch pad
    // ------------------------------------------------------------------------


    template <typename Int>
    byte *encode(Int value);
    // ------------------------------------------------------------------------
    //   Add an LEB128-encoded value to the scratchpad
    // ------------------------------------------------------------------------


    template <typename Int, typename ...Args>
    byte *encode(Int value, Args... args);
    // ------------------------------------------------------------------------
    //   Add an LEB128-encoded value to the scratchpad
    // ------------------------------------------------------------------------


    void free(size_t size)
    // ------------------------------------------------------------------------
    //   Free the whole scratchpad
    // ------------------------------------------------------------------------
    {
        if (Scratch >= size)
            Scratch -= size;
        else
            Scratch = 0;
    }

    object_p temporary()
    // ------------------------------------------------------------------------
    //   Make a temporary from the scratchpad
    // ------------------------------------------------------------------------
    {
        if (Editing == 0)
        {
            object_p result = Temporaries;
            Temporaries = (object_p) ((byte *) Temporaries + Scratch);
            Scratch = 0;
            return result;
        }
        return nullptr;
    }



    // ========================================================================
    //
    //   Return stack
    //
    // ========================================================================

#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

    enum { CALLS_BLOCK = 32 };

    bool run_push_data(object_p next, object_p end)
    // ------------------------------------------------------------------------
    //   Push an object to call on the RPL stack
    // ------------------------------------------------------------------------
    {
        runtime_invariants check;
        if (Returns <= CallStack)
            if (!call_stack_grow(next, end))
                return false;
        *(--Returns) = end;
        *(--Returns) = next;
        return true;
    }

    bool run_push(object_p next, object_p end)
    // ------------------------------------------------------------------------
    //   Push an object to call on the RPL stack
    // ------------------------------------------------------------------------
    {
        runtime_invariants check;
        if (next < end || !next)    // Can be nullptr for conditionals
        {
            end = object_p(byte_p(end) - 1);
            run_push_data(next, end);
        }
        return true;
    }

    inline object_p run_next(size_t depth)
    // ------------------------------------------------------------------------
    //   Pull the next object to execute from the RPL evaluation stack
    // ------------------------------------------------------------------------
    //   Getting proper inlining here is important for performance, but
    //   that requires the definition of object::skip()
#ifdef OBJECT_H
    {
        runtime_invariants check;
        object_p *high = HighMem - depth;
        while (Returns < high)
        {
            object_p next = Returns[0];
            object_p end  = Returns[1] + 1;
            if (next < end)
            {
                if (next)
                {
                    object_p nnext = next->skip();
                    Returns[0] = nnext;
                    if (nnext >= end)
                        // Note that call_stack_drop() cannot and MUST NOT GC
                        // so that the value of next cannot change
                        call_stack_drop(2);
                    return next;
                }
                unlocals(size_t(end) - 1);
            }

            call_stack_drop(2);
        }
        return nullptr;
    }
#else // !OBJECT_H
    // Don't have the definition of object::skip() - Simply mark as inline
    ;
#endif // OBJECT_H

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42

    object_p run_stepping()
    // ------------------------------------------------------------------------
    //   Return the next instruction for single-stepping
    // ------------------------------------------------------------------------
    {
        runtime_invariants check;
        if (Returns < HighMem)
            return Returns[0];
        return nullptr;
    }


    bool run_conditionals(object_p trueC, object_p falseC, bool xeq = false);
    // ------------------------------------------------------------------------
    //   Push true and false paths on the evaluation stack
    // ------------------------------------------------------------------------

    bool run_select(bool condition);
    // ------------------------------------------------------------------------
    //   Select true or false case from run_conditionals
    // ------------------------------------------------------------------------

    bool run_select_while(bool condition);
    // ------------------------------------------------------------------------
    //   Select true or false branch for while loops
    // ------------------------------------------------------------------------

    bool run_select_start_step(bool for_loop, bool has_step);
    // ------------------------------------------------------------------------
    //   Select the next branch in for-next, for-step, start-next or start-step
    // ------------------------------------------------------------------------

    bool run_select_list(bool for_loop);
    // ------------------------------------------------------------------------
    //   Select the next branch in for-next, for-step, start-next or start-step
    // ------------------------------------------------------------------------

    bool run_select_case(bool condition);
    // ------------------------------------------------------------------------
    //   Select true or false case for case statement
    // ------------------------------------------------------------------------


    bool call_stack_grow(object_p &next, object_p &end);
    void call_stack_drop();
    void call_stack_drop(uint n)
    // ------------------------------------------------------------------------
    //  Manage the call stack in blocks
    // ------------------------------------------------------------------------
    {
        Returns += n;
        if (Returns >= CallStack + CALLS_BLOCK)
            call_stack_drop();
    }


    size_t call_depth() const
    // ------------------------------------------------------------------------
    //   Return calldepth
    // ------------------------------------------------------------------------
    {
        runtime_invariants check;
        return HighMem - Returns;
    }



    // ========================================================================
    //
    //   Stack
    //
    // ========================================================================

    bool push(gcp<const object> obj);
    // ------------------------------------------------------------------------
    //   Push an object on top of RPL stack
    // ------------------------------------------------------------------------

    object_p top();
    // ------------------------------------------------------------------------
    //   Return the top of the runtime stack
    // ------------------------------------------------------------------------

    bool top(object_p obj);
    // ------------------------------------------------------------------------
    //   Set the top of the runtime stack
    // ------------------------------------------------------------------------

    object_p pop();
    // ------------------------------------------------------------------------
    //   Pop the top-level object from the stack, or return NULL
    // ------------------------------------------------------------------------

    object_p stack(uint idx);
    // ------------------------------------------------------------------------
    //    Get the object at a given position in the stack
    // ------------------------------------------------------------------------

    bool roll(uint idx);
    // ------------------------------------------------------------------------
    //    Get the object at a given position in the stack
    // ------------------------------------------------------------------------

    bool rolld(uint idx);
    // ------------------------------------------------------------------------
    //    Get the object at a given position in the stack
    // ------------------------------------------------------------------------

    bool stack(uint idx, object_p obj);
    // ------------------------------------------------------------------------
    //    Get the object at a given position in the stack
    // ------------------------------------------------------------------------

    bool drop(uint count = 1);
    // ------------------------------------------------------------------------
    //   Pop the top-level object from the stack, or return NULL
    // ------------------------------------------------------------------------

    uint depth()
    // ------------------------------------------------------------------------
    //   Return the stack depth
    // ------------------------------------------------------------------------
    {
        return Args - Stack;
    }

    object_p *stack_base()
    // ------------------------------------------------------------------------
    //   Return the base of the stack for sorting purpose
    // ------------------------------------------------------------------------
    {
        return Stack;
    }



    // ========================================================================
    //
    //   Last Args and Undo
    //
    // ========================================================================

    bool args(uint count);
    // ------------------------------------------------------------------------
    //   Indicate how many arguments we need to save in last args
    // ------------------------------------------------------------------------

    size_t args() const
    // ------------------------------------------------------------------------
    //   Return the number of args in the Args area
    // ------------------------------------------------------------------------
    {
        return Undo - Args;
    }

    bool last();
    // ------------------------------------------------------------------------
    //   Push back last arguments
    // ------------------------------------------------------------------------

    bool last(uint index);
    // ------------------------------------------------------------------------
    //   Push back last argument
    // ------------------------------------------------------------------------


    bool save();
    // ------------------------------------------------------------------------
    //  Save the state for undo
    // ------------------------------------------------------------------------

    size_t saved() const
    // ------------------------------------------------------------------------
    //   Return the size of the stack save area
    // ------------------------------------------------------------------------
    {
        return Locals - Undo;
    }

    bool undo();
    // ------------------------------------------------------------------------
    //   Undo and return earlier stack
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //   Local variables
    //
    // ========================================================================

    object_p local(uint index);
    // ------------------------------------------------------------------------
    //   Fetch local at given index
    // ------------------------------------------------------------------------

    object_p local(uint index, object_p obj);
    // ------------------------------------------------------------------------
    //   Set a local in the local stack
    // ------------------------------------------------------------------------

    bool locals(size_t count);
    // ------------------------------------------------------------------------
    //   Allocate the given number of locals from stack
    // ------------------------------------------------------------------------

    bool unlocals(size_t count);
    // ------------------------------------------------------------------------
    //    Free the number of locals
    // ------------------------------------------------------------------------

    size_t locals()
    // ------------------------------------------------------------------------
    //   Return the number of locals
    // ------------------------------------------------------------------------
    {
        return Directories - Locals;
    }



    // ========================================================================
    //
    //   Global directorys
    //
    // ========================================================================

    directory *variables(size_t depth) const
    // ------------------------------------------------------------------------
    //   Current directory for global variables
    // ------------------------------------------------------------------------
    {
        if (depth >= size_t(XLibs - Directories))
            return nullptr;
        return (directory *) Directories[depth];
    }

    directory *homedir() const
    // ------------------------------------------------------------------------
    //   Return the home directory
    // ------------------------------------------------------------------------
    {
        directory **home = (directory **) (XLibs - 1);
        return *home;
    }

    size_t directories() const
    // ------------------------------------------------------------------------
    //   Return number of directories
    // ------------------------------------------------------------------------
    {
        size_t depth = XLibs - Directories;
        return depth;
    }

    bool is_active_directory(object_p obj) const;
    bool enter(directory_p dir);
    bool updir(size_t count = 1);



    // ========================================================================
    //
    //   Library items
    //
    // ========================================================================

    object_p xlib(size_t id) const
    // ------------------------------------------------------------------------
    //   Return the given xlib
    // ------------------------------------------------------------------------
    {
        if (id >= size_t(Constants - XLibs))
            return nullptr;
        return XLibs[id];
    }

    bool xlib(size_t id, object_p value) const
    // ------------------------------------------------------------------------
    //   Set the given xlib
    // ------------------------------------------------------------------------
    {
        if (id >= size_t(Constants - XLibs))
            return false;
        XLibs[id] = value;
        return true;
    }

    size_t xlibs() const
    // ------------------------------------------------------------------------
    //   Return number of xlibs
    // ------------------------------------------------------------------------
    {
        size_t depth = size_t(Constants - XLibs);
        return depth;
    }

    bool attach(size_t nentries);
    // ------------------------------------------------------------------------
    //   Reserve and clear the given number of entries
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //   Constant items
    //
    // ========================================================================

    object_p constant(size_t id) const
    // ------------------------------------------------------------------------
    //   Return the given constant
    // ------------------------------------------------------------------------
    {
        if (id >= size_t(CallStack - Constants))
            return nullptr;
        return Constants[id];
    }

    bool constant(size_t id, object_p value) const
    // ------------------------------------------------------------------------
    //   Set the given constant
    // ------------------------------------------------------------------------
    {
        if (id >= size_t(CallStack - Constants))
            return false;
        Constants[id] = value;
        return true;
    }

    size_t constants() const
    // ------------------------------------------------------------------------
    //   Return number of constants
    // ------------------------------------------------------------------------
    {
        size_t depth = size_t(CallStack - Constants);
        return depth;
    }

    bool constants(size_t nentries);
    // ------------------------------------------------------------------------
    //   Reserve and clear the given number of entries
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //   Error handling
    //
    // ========================================================================

    runtime &error(utf8 message)
    // ------------------------------------------------------------------------
    //   Set the error message
    // ------------------------------------------------------------------------
    {
        if (message)
            record(errors, "Error [%+s]", message);
        else
            record(runtime, "Clearing error");
        Error = ErrorSave = message;
        return *this;
    }

    runtime &error(cstring message)
    // ------------------------------------------------------------------------
    //   Set the error message
    // ------------------------------------------------------------------------
    {
        return error(utf8(message));
    }

    utf8 error()
    // ------------------------------------------------------------------------
    //   Get the error message (as currently displayed)
    // ------------------------------------------------------------------------
    {
        return Error;
    }

    utf8 error_message()
    // ------------------------------------------------------------------------
    //   Get the error message (as saved for errm)
    // ------------------------------------------------------------------------
    {
        return ErrorSave;
    }

    runtime &source(utf8 spos, size_t len = 0)
    // ------------------------------------------------------------------------
    //   Set the source location for the current error
    // ------------------------------------------------------------------------
    {
        ErrorSource = spos;
        ErrorSrcLen = len;
        return *this;
    }

    runtime &source(cstring spos, size_t len = 0)
    // ------------------------------------------------------------------------
    //   Set the source location for the current error
    // ------------------------------------------------------------------------
    {
        return source(utf8(spos), len);
    }

    utf8 source()
    // ------------------------------------------------------------------------
    //   Get the pointer to the problem
    // ------------------------------------------------------------------------
    {
        return ErrorSource;
    }

    size_t source_length()
    // ------------------------------------------------------------------------
    //   Get the length of text for the problem
    // ------------------------------------------------------------------------
    {
        return ErrorSrcLen;
    }

    runtime &command(const object *cmd);
    // ------------------------------------------------------------------------
    //   Set the faulting command
    // ------------------------------------------------------------------------


    object_p command() const;
    // ------------------------------------------------------------------------
    //   Get the faulting command
    // ------------------------------------------------------------------------


    bool is_user_command(utf8 cmd)
    // ------------------------------------------------------------------------
    //   Check if the command is a user-defined command
    // ------------------------------------------------------------------------
    {
        return cmd >= utf8(LowMem) && cmd < utf8(HighMem);
    }

    void clear_error()
    // ------------------------------------------------------------------------
    //   Clear error state (but _does not_ clear ErrorSave intentionally)
    // ------------------------------------------------------------------------
    {
        Error = nullptr;
        ErrorSource = nullptr;
        ErrorCommand = nullptr;
    }



    // ========================================================================
    //
    //   Common errors
    //
    // ========================================================================

    algebraic_p zero_divide(bool negative) const;
    algebraic_p infinity(bool negative) const;
    algebraic_p numerical_overflow(bool negative) const;
    algebraic_p numerical_underflow(bool negative) const;
    algebraic_p undefined_result() const;
    // ------------------------------------------------------------------------
    //   Return the value for a divide by zero, overflow and underflow
    // ------------------------------------------------------------------------


#define ERROR(name, msg)        runtime &name##_error();
#include "errors.tbl"


protected:
    utf8      Error;        // Error message if any
    utf8      ErrorSave;    // Last error message (for ERRM)
    utf8      ErrorSource;  // Source of the error if known
    size_t    ErrorSrcLen;  // Length of error in source
    object_p  ErrorCommand; // Source of the error if known
    object_p  LowMem;       // Bottom of available memory
    object_p  Globals;      // End of global objects
    object_p  Temporaries;  // Temporaries (must be valid objects)
    size_t    Editing;      // Text editor (utf8 encoded)
    size_t    Scratch;      // Scratch pad (may be invalid objects)
    object_p *Stack;        // Top of user stack
    object_p *Args;         // Start of save area for last arguments
    object_p *Undo;         // Start of undo stack
    object_p *Locals;       // Start of locals
    object_p *Directories;  // Start of directories
    object_p *XLibs;        // Start of xlibs (which can be null)
    object_p *Constants;    // Start of constants (which can be null)
    object_p *CallStack;    // Start of call stack (rounded 16 entries)
    object_p *Returns;      // Start of return stack, end of locals
    object_p *HighMem;      // End of available memory
    object_p  Cache[2][32]; // 16 Key/Value pairs for stack acceleration
    uint      CacheIndex;   // Index of latest entry in cache
    size_t    GCCycles;     // Number of garbage collection cycles
    size_t    GCPurged;     // Number of bytes collected by the GC
    size_t    GCDuration;   // Total duration of GC execution
    size_t    GCLPurged;    // Number of bytes collected during last GC
    size_t    GCLDuration;  // Duration of last GC execution
    size_t    GCCleared;    // Cleaned automatically by `clearer`
    size_t    GCUnclear;    // Disable 'clearer' class
    bool      SaveArgs;     // Save arguents (LastArgs)

    // Pointers that are GC-adjusted
    static gcptr *GCSafe;

    friend struct GarbageCollectorStatistics;
    friend struct cleaner;
    friend struct runtime_invariants;
    friend void dump_gc_pointers();
};

template<typename T>
using gcp = runtime::gcp<const T>;
template<typename T>
using gcm = runtime::gcp<T>;

using gcstring  = gcp<char>;
using gcmstring = gcm<char>;
using gcbytes   = gcp<byte>;
using gcmbytes  = gcm<byte>;
using gcutf8    = gcp<byte>;
using gcmutf8   = gcm<byte>;

using object_g  = gcp<object>;
using object_r  = const object_g &;

#define GCP_EXISTING(T)                         \
    typedef const T *           T##_p;          \
    typedef gcp<T>              T##_g;          \
    typedef gcm<T>              T##_m;          \
    typedef const T##_g &       T##_r;

#define GCP(T)                                  \
    struct T;                                   \
    GCP_EXISTING(T)


// ============================================================================
//
//    Allocate objects
//
// ============================================================================

template <typename Obj, typename ...Args>
inline const Obj *make(Args &... args)
// ----------------------------------------------------------------------------
//    Create an object in the runtime
// ----------------------------------------------------------------------------
{
    return rt.make<Obj>(args...);
}


template <typename Obj>
inline void *operator new(size_t UNUSED size, Obj *where)
// ----------------------------------------------------------------------------
//    Placement new for objects
// ----------------------------------------------------------------------------
{
    return where;
}


template <typename Obj, typename ... ArgsT>
const Obj *runtime::make(typename Obj::id type, const
                         ArgsT &... args)
// ----------------------------------------------------------------------------
//   Make a new temporary of the given size
// ----------------------------------------------------------------------------
{
    // Find required memory for this object
    size_t size = Obj::required_memory(type, args...);
    record(runtime,
           "Initializing object %p type %d size %u", Temporaries, type, size);

    // Check if we have room (may cause garbage collection)
    if (available(size) < size)
        return nullptr;    // Failed to allocate
    Obj *result = (Obj *) Temporaries;
    Temporaries = (object *) ((byte *) Temporaries + size);

    // Move the editor up (available() checked we have room)
    move(Temporaries, (object_p) result, Editing + Scratch, 1, true);

    // Initialize the object in place (may GC and move result)
    gcbytes ptr = (byte *) result;
    new(result) Obj(type, args...);
    result = (Obj *) +ptr;

#ifdef SIMULATOR
    object_validate(type, (const object *) result, size);
#endif // SIMULATOR

    // Return initialized object
    return result;
}


template <typename Obj, typename ... ArgsT>
const Obj *runtime::make(const ArgsT &... args)
// ----------------------------------------------------------------------------
//   Make a new temporary of the given size
// ----------------------------------------------------------------------------
{
    // Find the required type for this object
    typename Obj::id type = Obj::static_id;
    return make<Obj>(type, args...);
}


struct scribble
// ----------------------------------------------------------------------------
//   Temporary area using the scratchpad
// ----------------------------------------------------------------------------
{
    scribble(): allocated(rt.allocated())
    {
    }
    ~scribble()
    {
        clear();
    }
    void commit()
    {
        allocated = rt.allocated();
    }
    void clear()
    {
        if (size_t added = growth())
            rt.free(added);
    }
    size_t growth()
    {
        return rt.allocated() - allocated;
    }
    byte *scratch()
    {
        return rt.scratchpad() - rt.allocated() + allocated;
    }

private:
    size_t  allocated;
};


struct stack_depth_restore
// ----------------------------------------------------------------------------
//   Restore the stack depth on exit
// ----------------------------------------------------------------------------
{
    stack_depth_restore(): depth(rt.depth()) {}
    ~stack_depth_restore()
    {
        size_t now = rt.depth();
        if (now > depth)
            rt.drop(now - depth);
    }
    size_t count() const        { return rt.depth() - depth; }
    size_t depth;
};


struct error_save
// ----------------------------------------------------------------------------
//   Save the current runtime error and clear it for internal evaluation
// ----------------------------------------------------------------------------
{
    error_save();
    ~error_save();
    gcutf8      errmsg;
    gcutf8      source;
    size_t      srclen;
    gcp<object> command;
};


struct cleaner
// ----------------------------------------------------------------------------
//   Reclaim temporaries created during a lengthy operation
// ----------------------------------------------------------------------------
{
    cleaner();
    object_p adjust(object_p temp);

    template<typename T>
    const T *operator()(const T* temp)
    {
        return (const T *) adjust(object_p(temp));
    }
    template<typename T>
    const T *operator()(const gcp<T> &temp)
    {
        return (const T *) adjust(object_p(+temp));
    }

    static void disable()       { rt.GCUnclear++; }

    object_p temporaries;
    size_t   gccycles;
};

#endif // RUNTIME_H
