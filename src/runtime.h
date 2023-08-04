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
//        [Pointer to outermost directory in path]
//        [ ... intermediate directory pointers ...]
//        [Pointer to innermost directory in path]
//      Directories     Bottom of stack, start of global
//        [Local N]
//        [...]
//        [Local 0]
//      Locals
//        [undo N]
//        [...]
//        [undo 1, a list captured from the stack]
//      Undos
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

#include <cstdio>
#include <cstring>


struct object;                  // RPL object
struct directory;
typedef const object *object_p;
typedef const directory *directory_p;

RECORDER_DECLARE(runtime);
RECORDER_DECLARE(runtime_error);
RECORDER_DECLARE(errors);
RECORDER_DECLARE(gc);
RECORDER_DECLARE(editor);

// The one and only runtime
struct runtime;
extern runtime rt;



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

    object_p clone_global(object_p source);
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

    size_t edit();
    // ------------------------------------------------------------------------
    //   Append the scratch pad to the editor (at end)
    // ------------------------------------------------------------------------


    utf8 close_editor(bool convert = false);
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


    void remove(size_t offset, size_t len);
    // ------------------------------------------------------------------------
    //   Remove characers from the editor
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //   Object management
    //
    // ========================================================================

    size_t gc();
    // ------------------------------------------------------------------------
    //   Garbage collector (purge unused objects from memory to make space)
    // ------------------------------------------------------------------------


    void move(object_p to, object_p from, size_t sz, bool scratch=false);
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
        gcptr(byte *ptr = nullptr) : safe(ptr), next(rt.GCSafe)
        {
            rt.GCSafe = this;
        }
        gcptr(const gcptr &o): safe(o.safe), next(rt.GCSafe)
        {
            rt.GCSafe = this;
        }
        ~gcptr();

        operator byte  *() const                { return safe; }
        operator byte *&()                      { return safe; }
        byte *&Safe()                           { return safe; }
        operator bool()                         { return safe != nullptr; }
        operator int()                          = delete;
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

    void call(gcp<const object> callee);
    // ------------------------------------------------------------------------
    //   Push the current object on the RPL stack
    // ------------------------------------------------------------------------

    void ret();
    // ------------------------------------------------------------------------
    //   Return from an RPL call
    // ------------------------------------------------------------------------



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
        return Undos - Stack;
    }



    // ========================================================================
    //
    //   Local variables
    //
    // ========================================================================

    object_p local(uint index);
    // ------------------------------------------------------------------------
    //   Fetch local at given index
    // ------------------------------------------------------------------------

    bool local(uint index, object_p obj);
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

    directory *variables(uint depth) const
    // ------------------------------------------------------------------------
    //   Current directory for global variables
    // ------------------------------------------------------------------------
    {
        if (depth >= (uint) ((object_p *) Returns - Directories))
            return nullptr;
        return (directory *) Directories[depth];
    }

    directory *homedir() const
    // ------------------------------------------------------------------------
    //   Return the home directory
    // ------------------------------------------------------------------------
    {
        directory **home = (directory **) (Returns - 1);
        return *home;
    }

    size_t directories() const
    // ------------------------------------------------------------------------
    //   Return number of directories
    // ------------------------------------------------------------------------
    {
        size_t depth = (object_p *) Returns - Directories;
        return depth;
    }


    bool enter(directory_p dir);
    bool updir(size_t count = 1);



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
            record(errors, "Error [%s]", message);
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

    runtime &source(utf8 spos)
    // ------------------------------------------------------------------------
    //   Set the source location for the current error
    // ------------------------------------------------------------------------
    {
        ErrorSource = spos;
        return *this;
    }

    runtime &source(cstring spos)
    // ------------------------------------------------------------------------
    //   Set the source location for the current error
    // ------------------------------------------------------------------------
    {
        return source(utf8(spos));
    }

    utf8 source()
    // ------------------------------------------------------------------------
    //   Get the pointer to the problem
    // ------------------------------------------------------------------------
    {
        return ErrorSource;
    }

    runtime &command(utf8 cmd)
    // ------------------------------------------------------------------------
    //   Set the faulting command
    // ------------------------------------------------------------------------
    {
        ErrorCommand = cmd;
        return *this;
    }

    runtime &command(cstring cmd)
    // ------------------------------------------------------------------------
    //   Set the faulting command
    // ------------------------------------------------------------------------
    {
        return command(utf8(cmd));
    }

    utf8 command()
    // ------------------------------------------------------------------------
    //   Get the faulting command if there is one
    // ------------------------------------------------------------------------
    {
        return ErrorCommand;
    }


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

#define ERROR(name, msg)        runtime &name##_error();
#include "errors.tbl"


protected:
    utf8      Error;        // Error message if any
    utf8      ErrorSave;    // Last error message (for ERRM)
    utf8      ErrorSource;  // Source of the error if known
    utf8      ErrorCommand; // Source of the error if known
    object_p  Code;         // Currently executing code
    object_p  LowMem;       // Bottom of available memory
    object_p  Globals;      // End of global objects
    object_p  Temporaries;  // Temporaries (must be valid objects)
    size_t    Editing;      // Text editor (utf8 encoded)
    size_t    Scratch;      // Scratch pad (may be invalid objects)
    object_p *Stack;        // Top of user stack
    object_p *Undos;        // Start of Undos area, end of stack
    object_p *Locals;       // Start of locals, end of undos
    object_p *Directories;  // Start of directories, end of returns
    object_p *Returns;      // Start of return stack, end of locals
    object_p *HighMem;      // End of available memory

    // Pointers that are GC-adjusted
    static gcptr *GCSafe;
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

#define GCP(T)                                  \
    struct T;                                   \
    typedef const T *T##_p;                     \
    typedef gcp<T>   T##_g;                     \
    typedef gcm<T>   T##_m;                     \
    typedef const T##_g &T##_r;


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


template <typename Obj, typename ... Args>
const Obj *runtime::make(typename Obj::id type, const Args &... args)
// ----------------------------------------------------------------------------
//   Make a new temporary of the given size
// ----------------------------------------------------------------------------
{
    // Find required memory for this object
    size_t size = Obj::required_memory(type, args...);
    record(runtime,
           "Initializing object %p type %d %+s size %u",
           Temporaries, type, Obj::object::name(type), size);

    // Check if we have room (may cause garbage collection)
    if (available(size) < size)
        return nullptr;    // Failed to allocate
    Obj *result = (Obj *) Temporaries;
    Temporaries = (object *) ((byte *) Temporaries + size);

    // Move the editor up (available() checked we have room)
    move(Temporaries, (object_p) result, Editing + Scratch, true);

    // Initialize the object in place
    new(result) Obj(args..., type);

#ifdef SIMULATOR
    object_validate(type, (const object *) result, size);
#endif // SIMULATOR

    // Return initialized object
    return result;
}


template <typename Obj, typename ... Args>
const Obj *runtime::make(const Args &... args)
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
        if (size_t added = growth())
            rt.free(added);
    }
    void commit()
    {
        allocated = rt.allocated();
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


// User interface signal that a GC is in progress
extern void draw_gc();

#endif // RUNTIME_H
