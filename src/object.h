#ifndef OBJECT_H
#define OBJECT_H
// ****************************************************************************
//  object.h                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     The basic RPL object
//
//     An RPL object is a bag of bytes densely encoded using LEB128
//
//     It is important that the base object be empty, sizeof(object) == 1
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
// Object encoding:
//
//    RPL objects are encoded using sequence of LEB128 values
//    An LEB128 value is a variable-length encoding with 7 bits per byte,
//    the last byte having its high bit clear. This, values 0-127 are coded
//    as 0-127, values up to 16384 are coded on two bytes, and so on.
//
//    All objects begin with an "identifier" (the type is called id in the code)
//    which uniquely defines the type of the object. Identifiers are defined in
//    the source file called ids.tbl.
//
//    For commands, the object type is all there is to the object. Therefore,
//    most RPL commands consume only one byte in memory. For other objects,
//    there is a "payload" following that identifier. The format of the paylaod
//    is described in the source file for the corresponding object type, but a
//    general guiding principle is that the payload must make it easy to skip
//    the object in memory, notably during garbage collection.
//
// Object handler:
//
//    The type of the object is an index in an object-handler table, so they act
//    either as commands (performing an action when evaluated) or as data types
//    (putting themselves on the runtime stack when evaluated).
//
//    Handlers are function pointers in a table that (except for `parse()`)
//    take an object as input and process it. The most important function is
//    `evaluate()`, which will evaluate an object like the `Evaluate` command.
//    An important variant is `execute()`, which executes programs. That
//    function does not call itself recursively. If it needs to call another
//    it calls the runtime's `call()` function, so that we manage the RPL call
//    using RPL garbage-collected memory. This is important to allow for a deep
//    evaluation of RPL objects. Without that approach, recursive evaluation
//    on the C++ call stack would limit us to about 40 levels of RPL calls.
//
//    The handler is not exactly equivalent to the user command.
//    It may present an internal interface that is more convenient for C code.
//    This approach makes it possible to pass other IDs to an object, for
//    example the "add" operator can delegate the addition of complex numbers
//    to the complex handler by calling the complex handler with 'add'.
//
// Rationale:
//
//    The reason for this design is that the DM42 is very memory-starved
//    (~70K available to DMCP programs), so the focus is on a format for objects
//    that is extremely compact.
//
//    Notably, for size encoding, with only 70K available, the chances of a size
//    exceeding 2 bytes (16384) are exceedingly rare.
//
//    We can also use the lowest opcodes for the most frequently used features,
//    ensuring that 128 of them can be encoded on one byte only. Similarly, all
//    constants less than 128 can be encoded in two bytes only (one for the
//    opcode, one for the value), and all constants less than 16384 are encoded
//    on three bytes.
//
//    Similarly, the design of RPL calls for a garbage collector.
//    The format of objects defined above ensures that all objects are moveable.
//    The garbage collector can therefore be "compacting", moving all live
//    objects at the beginning of memory. This in turns means that each garbage
//    collection cycle gives us a large amount of contiguous memory, but more
//    importantly, that the allocation of new objects is extremely simple, and
//    therefore quite fast.
//
//    The downside is that we can't really use the C++ built-in dyanmic dispatch
//    mechanism (virtual functions), as having a pointer to a vtable would
//    increase the size of the object too much.


#include "leb128.h"
#include "precedence.h"
#include "recorder.h"
#include "types.h"

RECORDER_DECLARE(object);
RECORDER_DECLARE(parse);
RECORDER_DECLARE(parse_attempts);
RECORDER_DECLARE(render);
RECORDER_DECLARE(eval);
RECORDER_DECLARE(run);
RECORDER_DECLARE(object_errors);

struct algebraic;
struct menu_info;
struct object;
struct parser;
struct program;
struct renderer;
struct grapher;
struct runtime;
struct symbol;
struct text;
struct grob;
struct list;
struct user_interface;

typedef const algebraic *algebraic_p;
typedef const object    *object_p;
typedef const program   *program_p;
typedef const symbol    *symbol_p;
typedef const text      *text_p;
typedef const grob      *grob_p;
typedef const list      *list_p;

struct object
// ----------------------------------------------------------------------------
//  The basic RPL object
// ----------------------------------------------------------------------------
{
    enum id : unsigned
    // ------------------------------------------------------------------------
    //  Object ID
    // ------------------------------------------------------------------------
    {
#define ID(i)   ID_##i,
#include "ids.tbl"
        NUM_IDS
    };

    object(id i)
    // ------------------------------------------------------------------------
    //  Write the id of the object
    // ------------------------------------------------------------------------
    {
        byte *ptr = (byte *) this;
        leb128(ptr, i);
    }
    ~object() {}


    // ========================================================================
    //
    //   Object protocol
    //
    // ========================================================================

    enum result
    // -------------------------------------------------------------------------
    //   Return values for parsing
    // -------------------------------------------------------------------------
    {
        OK,                     // Command ran successfully
        SKIP,                   // Command not for this handler, try next
        ERROR,                  // Error processing the command
        WARN,                   // Possible error (if no object succeeds)
        COMMENTED               // Code is commented out
    };


    typedef size_t      (*size_fn)(object_p o);
    typedef result      (*parse_fn)(parser &p);
    typedef utf8        (*help_fn)(object_p o);
    typedef result      (*evaluate_fn)(object_p o);
    typedef size_t      (*render_fn)(object_p o, renderer &r);
    typedef grob_p      (*graph_fn)(object_p o, grapher &g);
    typedef result      (*insert_fn)(object_p o);
    typedef bool        (*menu_fn)(object_p o, menu_info &m);
    typedef unicode     (*menu_marker_fn)(object_p o);

    struct dispatch
    // ------------------------------------------------------------------------
    //   Operations that can be run on an object
    // ------------------------------------------------------------------------
    {
        size_fn         size;           // Compute object size in bytes
        parse_fn        parse;          // Parse an object
        help_fn         help;           // Return help topic
        evaluate_fn     evaluate;       // Evaluate the object
        render_fn       render;         // Render the object as text
        graph_fn        graph;          // Render the object as a grob
        insert_fn       insert;         // Insert object in editor
        menu_fn         menu;           // Build menu entries
        menu_marker_fn  menu_marker;    // Show marker
        uint            arity;          // Number of input arguments
        uint            precedence;     // Precedence in equations
    };


    struct spelling
    // ------------------------------------------------------------------------
    //   One of the possible spellings for a commands
    // ------------------------------------------------------------------------
    {
        id              type;           // Type of the command
        cstring         name;           // Name for the command
    };
    static const spelling spellings[];
    static const size_t   spelling_count;



    // ========================================================================
    //
    //   Memory management
    //
    // ========================================================================

    static size_t required_memory(id i)
    // ------------------------------------------------------------------------
    //  Compute the amount of memory required for an object
    // ------------------------------------------------------------------------
    {
        return leb128size(i);
    }


#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif // DM42

    id type() const
    // ------------------------------------------------------------------------
    //   Return the type of the object
    // ------------------------------------------------------------------------
    {
        byte *ptr = (byte *) this;
        id ty = (id) leb128_u16(ptr);
        if (ty > NUM_IDS)
        {
            object_error(ty, this);
            ty = ID_object;
        }
        return ty;
    }


    static int type_value(id type);
    int type_value() const
    // ------------------------------------------------------------------------
    //   Return the type value as returned by `type` command
    // ------------------------------------------------------------------------
    {
        return type_value(type());
    }


    const dispatch &ops() const
    // ------------------------------------------------------------------------
    //   Return the handlers for the current object
    // ------------------------------------------------------------------------
    {
        return handler[type()];
    }


    size_t size() const
    // ------------------------------------------------------------------------
    //  Compute the size of the object by calling the handler with SIZE
    // ------------------------------------------------------------------------
    {
        return ops().size(this);
    }


    object_p skip() const
    // ------------------------------------------------------------------------
    //  Return the pointer to the next object in memory by skipping its size
    // ------------------------------------------------------------------------
    {
        return this + size();
    }


    template <typename Obj>
    static byte_p payload(const Obj *p)
    // ------------------------------------------------------------------------
    //  Return the object's payload, i.e. first byte after ID
    // ------------------------------------------------------------------------
    //  When we can, use the static type to know how many bytes to skip
    {
        return byte_p(p) + (Obj::static_id < 0x80 ? 1 : 2);
    }

    byte_p payload() const
    // ------------------------------------------------------------------------
    //  Return the object's payload, i.e. first byte after ID
    // ------------------------------------------------------------------------
    {
        return byte_p(leb128skip(this));
    }


    static void object_error(id type, const object *ptr);
    // ------------------------------------------------------------------------
    //   Report an error e.g. with with an object type
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //    High-level functions on objects
    //
    // ========================================================================

    result evaluate() const
    // ------------------------------------------------------------------------
    //  Evaluate an object by calling the handler
    // ------------------------------------------------------------------------
    {
        record(eval, "Evaluating %t", this);
        return ops().evaluate(this);
    }


    bool defer() const;
    static bool defer(id type);
    // ------------------------------------------------------------------------
    //   Deferred evaluation of an object
    // ------------------------------------------------------------------------


    size_t render(renderer &r) const
    // ------------------------------------------------------------------------
    //   Render the object into an existing renderer
    // ------------------------------------------------------------------------
    {
        record(render, "Rendering %p into %p", this, &r);
        return ops().render(this, r);
    }


    grob_p graph(grapher &g) const
    // ------------------------------------------------------------------------
    //   Render the object into an existing grapher
    // ------------------------------------------------------------------------
    {
        record(render, "Graphing %+s %p into %p", name(), this, &g);
        return ops().graph(this, g);
    }

    grob_p graph() const;
    // ------------------------------------------------------------------------
    //   Render like for the `Show` command
    // ------------------------------------------------------------------------

#ifdef DM42
#  pragma GCC pop_options
#endif


    size_t render(char *output, size_t length) const;
    // ------------------------------------------------------------------------
    //   Render the object into a static buffer
    // ------------------------------------------------------------------------


    size_t edit() const;
    // ------------------------------------------------------------------------
    //   Render the object into the scratchpad, then move into the editor
    // ------------------------------------------------------------------------


    text_p as_text(bool edit = true, bool eq = false) const;
    // ------------------------------------------------------------------------
    //   Return the object as text
    // ------------------------------------------------------------------------


    symbol_p as_symbol(bool editing) const
    // ------------------------------------------------------------------------
    //   Return the object as text
    // ------------------------------------------------------------------------
    {
        return symbol_p(as_text(editing, true));
    }


    program_p as_program() const
    // ------------------------------------------------------------------------
    //   Return the value as a program
    // ------------------------------------------------------------------------
    {
        return is_program() ? program_p(this) : nullptr;
    }


    algebraic_p as_real() const
    // ------------------------------------------------------------------------
    //   Check if something is a real number
    // ------------------------------------------------------------------------
    {
        return is_real() ? algebraic_p(this) : nullptr;
    }


    grob_p as_grob() const;
    // ------------------------------------------------------------------------
    //   Return the object as a pixel graphic object
    // ------------------------------------------------------------------------


    uint32_t as_uint32(uint32_t def, bool err) const;
    int32_t  as_int32 (int32_t  def, bool err)  const;
    uint64_t as_uint64(uint64_t def, bool err) const;
    int64_t  as_int64 (int64_t  def, bool err)  const;
    // ------------------------------------------------------------------------
    //   Return the object as an integer, possibly erroring out for bad type
    // ------------------------------------------------------------------------


    object_p at(size_t index, bool err = true) const;
    object_p at(object_p index) const;
    // ------------------------------------------------------------------------
    //   Extract a subobject at given index, works for list, array and text
    // ------------------------------------------------------------------------


    object_p at(object_p index, object_p value) const;
    // ------------------------------------------------------------------------
    //   Set a subobject at given index, works for list, array and text
    // ------------------------------------------------------------------------


    bool next_index(object_p *index) const;
    // ------------------------------------------------------------------------
    //   Find the next index for the current object, returns true if wraps
    // ------------------------------------------------------------------------


    result insert() const
    // ------------------------------------------------------------------------
    //   Insert in the editor at cursor position, with possible offset
    // ------------------------------------------------------------------------
    {
        return ops().insert(this);
    }


    static object_p parse(utf8    source,
                          size_t &size,
                          int     precedence = 0,
                          unicode separator  = 0);
    // ------------------------------------------------------------------------
    //  Try parsing the object as a top-level temporary
    // ------------------------------------------------------------------------
    //  If precedence != 0, parse as an equation object with that precedence


    utf8 help() const
    // ------------------------------------------------------------------------
    //   Return the help topic for the given object
    // ------------------------------------------------------------------------
    {
        return ops().help(this);
    }


    static cstring name(result r)
    // ------------------------------------------------------------------------
    //    Convenience function for the name of results
    // ------------------------------------------------------------------------
    {
        switch (r)
        {
        case OK:        return "OK";
        case SKIP:      return "SKIP";
        case ERROR:     return "ERROR";
        case WARN:      return "WARN";
        default:        return "<Unknown>";
        }
    }


    static utf8 alias(id i, uint index);
    // ------------------------------------------------------------------------
    //   Return the nth alias for a given ID
    // ------------------------------------------------------------------------


    static utf8 name(id i);
    // ------------------------------------------------------------------------
    //   Return the name for a given ID with current style
    // ------------------------------------------------------------------------


    static utf8 fancy(id i);
    // ------------------------------------------------------------------------
    //   Return the fancy name for a given ID
    // ------------------------------------------------------------------------


    static utf8 old_name(id i);
    // ------------------------------------------------------------------------
    //   Return the fancy name for a given ID
    // ------------------------------------------------------------------------


    utf8 name() const
    // ------------------------------------------------------------------------
    //   Return the name for the current object
    // ------------------------------------------------------------------------
    {
        return name(type());
    }


    utf8 fancy() const
    // ------------------------------------------------------------------------
    //   Return the fancy name for the current object
    // ------------------------------------------------------------------------
    {
        return fancy(type());
    }


    unicode marker() const
    // ------------------------------------------------------------------------
    //   Marker in menus
    // ------------------------------------------------------------------------
    {
        return ops().menu_marker(this);
    }



    // ========================================================================
    //
    //    Attributes of objects
    //
    // ========================================================================

#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O3")
#endif

    struct id_map
    // ------------------------------------------------------------------------
    //   Used to isolate the type range checking names
    // ------------------------------------------------------------------------
    {
        enum ids
        // --------------------------------------------------------------------
        //  Createa a local name matching the class name
        // --------------------------------------------------------------------
        {
#define ID(i)   i,
#include "ids.tbl"
        NUM_IDS
        };


        template <ids first, ids last>
        static INLINE bool in_range(id ty)
        // --------------------------------------------------------------------
        //   Check if a  given type is in the given range
        // --------------------------------------------------------------------
        {
            return ids(ty) >= first && ids(ty) <= last;
        }

        template <ids first, ids last, ids more, ids ...rest>
        static INLINE bool in_range(id ty)
        // --------------------------------------------------------------------
        //   Check if a  given type is in the given ranges
        // --------------------------------------------------------------------
        {
            return (ids(ty) >= first && ids(ty) <= last)
                || in_range<more, rest...>(ty);
        }

#define ID(x)
#define ID_RANGE(name, ...)                                             \
        static INLINE bool name(id ty)                                  \
        /* ------------------------------------------------------- */   \
        /*   Range-based type checking (faster than memory reads)  */   \
        /* ------------------------------------------------------- */   \
        {                                                               \
            return in_range<__VA_ARGS__>(ty);                           \
        }
#include "ids.tbl"

    };

#define ID(x)
#define ID_RANGE(name, ...)                                             \
    static INLINE bool name(id ty)                                      \
    /* ------------------------------------------------------- */       \
    /*   Range-based type checking (faster than memory reads)  */       \
    /* ------------------------------------------------------- */       \
    {                                                                   \
        return id_map::name(ty);                                        \
    }                                                                   \
                                                                        \
                                                                        \
    INLINE bool name() const                                            \
    /* ------------------------------------------------------- */       \
    /*   Range-based type checking (faster than memory reads)  */       \
    /* ------------------------------------------------------- */       \
    {                                                                   \
        return id_map::name(type());                                    \
    }
#include "ids.tbl"


    bool is_big() const;
    // ------------------------------------------------------------------------
    //   Check if any component is a bignum
    // ------------------------------------------------------------------------


    static bool is_fractionable(id ty)
    // -------------------------------------------------------------------------
    //   Check if a type is a fraction or a non-based integer
    // -------------------------------------------------------------------------
    {
        return is_fraction(ty) || (is_integer(ty) && is_real(ty));
    }


    bool is_fractionable() const
    // -------------------------------------------------------------------------
    //   Check if an object is a fraction or an integer
    // -------------------------------------------------------------------------
    {
        return is_fractionable(type());
    }


    static bool is_algebraic_number(id ty)
    // ------------------------------------------------------------------------
    //   Check if something is a number (real or complex)
    // ------------------------------------------------------------------------
   {
       return is_real(ty) || is_complex(ty) || is_unit(ty);
    }


    bool is_algebraic_number() const
    // ------------------------------------------------------------------------
    //   Check if something is a number (real or complex)
    // ------------------------------------------------------------------------
    {
        return is_algebraic_number(type());
    }


    static bool is_symbolic_arg(id ty)
    // ------------------------------------------------------------------------
    //    Check if a type denotes a symbolic argument (symbol, equation, number)
    // ------------------------------------------------------------------------
    {
        return is_symbolic(ty) || is_algebraic_number(ty);
    }


    bool is_symbolic_arg() const
    // ------------------------------------------------------------------------
    //   Check if an object is a symbolic argument
    // ------------------------------------------------------------------------
    {
        return is_symbolic_arg(type());
    }


    static bool is_algebraic(id ty)
    // ------------------------------------------------------------------------
    //    Check if a type denotes an algebraic value or function
    // ------------------------------------------------------------------------
    {
        return is_algebraic_fn(ty) || is_symbolic_arg(ty);
    }


    bool is_algebraic() const
    // ------------------------------------------------------------------------
    //   Check if an object is an algebraic function
    // ------------------------------------------------------------------------
    {
        return is_algebraic(type());
    }


    algebraic_p as_algebraic() const;
    // ------------------------------------------------------------------------
    //   Return an object as an algebraic if possible, or nullptr
    // ------------------------------------------------------------------------


    static bool is_algebraic_or_list(id ty)
    // ------------------------------------------------------------------------
    //    Check if a type denotes an algebraic value or function
    // ------------------------------------------------------------------------
    {
        return is_algebraic(ty) || is_array_or_list(ty);
    }


    bool is_algebraic_or_list() const
    // ------------------------------------------------------------------------
    //   Check if an object is an algebraic function
    // ------------------------------------------------------------------------
    {
        return is_algebraic_or_list(type());
    }


    algebraic_p as_algebraic_or_list() const
    // ------------------------------------------------------------------------
    //   Return an object as an algebraic if possible, or nullptr
    // ------------------------------------------------------------------------
    {
        if (is_algebraic_or_list())
            return algebraic_p(this);
        return nullptr;
    }


    static bool is_extended_algebraic(id ty)
    // ------------------------------------------------------------------------
    //    Extended algebraics include text and array values
    // ------------------------------------------------------------------------
    {
        return is_algebraic_or_list(ty) || ty == ID_text;
    }


    bool is_extended_algebraic() const
    // ------------------------------------------------------------------------
    //   Check if an object is an extended algebraic, including text or array
    // ------------------------------------------------------------------------
    {
        return is_extended_algebraic(type());
    }


    static bool is_array_or_list(id ty)
    // ------------------------------------------------------------------------
    //   Check if we have an array or list
    // ------------------------------------------------------------------------
    {
        return ty == ID_array || ty == ID_list;
    }


    bool is_array_or_list() const
    // ------------------------------------------------------------------------
    //   Check if we have an array or list
    // ------------------------------------------------------------------------
    {
        return is_array_or_list(type());
    }


    list_p as_array_or_list() const
    // ------------------------------------------------------------------------
    //   Convert to array or list if this is the type
    // ------------------------------------------------------------------------
    {
        if (is_array_or_list())
            return list_p(this);
        return nullptr;
    }


    algebraic_p as_extended_algebraic() const
    // ------------------------------------------------------------------------
    //   Return an object as an algebraic if possible, or nullptr
    // ------------------------------------------------------------------------
    {
        if (is_extended_algebraic())
            return algebraic_p(this);
        return nullptr;
    }


    uint arity() const
    // ------------------------------------------------------------------------
    //   Return the arity for arithmetic operators
    // ------------------------------------------------------------------------
    {
        return ops().arity;
    }


    uint precedence() const
    // ------------------------------------------------------------------------
    //   Return the arity for arithmetic operators
    // ------------------------------------------------------------------------
    {
        return ops().precedence;
    }


    template<typename Obj> const Obj *as() const
    // ------------------------------------------------------------------------
    //   Type-safe cast (note: only for exact type match)
    // ------------------------------------------------------------------------
    {
        if (type() == Obj::static_id)
            return (const Obj *) this;
        return nullptr;
    }


    template<typename Obj, typename Derived> const Obj *as() const
    // ------------------------------------------------------------------------
    //   Type-safe cast (note: only for exact type match)
    // ------------------------------------------------------------------------
    {
        id t = type();
        if (t >= Obj::static_id && t <= Derived::static_id)
            return (const Obj *) this;
        return nullptr;
    }

#ifdef DM42
#  pragma GCC pop_options
#endif


    object_p as_quoted(id ty = ID_symbol) const;
    template<typename T>
    const T *as_quoted() const { return (const T *) as_quoted(T::static_id); }
    // ------------------------------------------------------------------------
    //    Return object as a valid quoted name (e.g. 'ABC')
    // ------------------------------------------------------------------------


    int as_truth(bool error = true) const;
    // ------------------------------------------------------------------------
    //   Return 0 or 1 if this is a logical value, -1 and type error otherwise
    // ------------------------------------------------------------------------


    bool is_zero(bool error = true) const;
    // ------------------------------------------------------------------------
    //   Return true if this is zero, false otherwise. Can error if bad type
    // ------------------------------------------------------------------------


    bool is_one(bool error = true) const;
    // ------------------------------------------------------------------------
    //   Return true if this is one, false otherwise. Can error if bad type
    // ------------------------------------------------------------------------


    bool is_negative(bool error = true) const;
    // ------------------------------------------------------------------------
    //   Return true if this is negative, false otherwise, can error if bad
    // ------------------------------------------------------------------------



    int compare_to(object_p other) const;
    // ------------------------------------------------------------------------
    //   Compare two objects and return a signed comparison
    // ------------------------------------------------------------------------


    bool is_same_as(object_p other) const
    // ------------------------------------------------------------------------
    //   Compare two objects
    // ------------------------------------------------------------------------
    {
        return compare_to(other) == 0;
    }


    object_p child(uint index = 0) const;
    // ------------------------------------------------------------------------
    //   Return a child for a complex, list or array
    // ------------------------------------------------------------------------


    algebraic_p algebraic_child(uint index = 0) const;
    // ------------------------------------------------------------------------
    //   Return an algebraic child for a complex, list or array
    // ------------------------------------------------------------------------


    static object_p static_object(id i);
    // ------------------------------------------------------------------------
    //   Get a static pointer for the given object (typically for commands)
    // ------------------------------------------------------------------------



    // ========================================================================
    //
    //    Default implementations for object interface
    //
    // ========================================================================

#define OBJECT_DECL(D)  static const id static_id = ID_##D;


#define PARSE_DECL(D)   static result   do_parse(parser &p UNUSED)
#define HELP_DECL(D)    static utf8     do_help(const D *o UNUSED)
#define EVAL_DECL(D)                                                    \
    static const D *static_self()                                       \
    {                                                                   \
        return (const D *) static_object(static_id);                    \
    }                                                                   \
    static result   do_evaluate(const D *o UNUSED = static_self())
#define SIZE_DECL(D)    static size_t   do_size(const D *o UNUSED)
#define RENDER_DECL(D)  static size_t   do_render(const D *o UNUSED,renderer &r UNUSED)
#define GRAPH_DECL(D)   static grob_p   do_graph(const D *o UNUSED,grapher &g UNUSED)
#define INSERT_DECL(D)  static result   do_insert(const D *o UNUSED)
#define MENU_DECL(D)    static bool     do_menu(const D *o UNUSED, menu_info &mi UNUSED)
#define MARKER_DECL(D)  static unicode  do_menu_marker(const D *o UNUSED)
#define ARITY_DECL(A)   enum { ARITY = A }
#define PREC_DECL(P)    enum { PRECEDENCE = precedence::P }

    OBJECT_DECL(object);
    PARSE_DECL(object);
    HELP_DECL(object);
    EVAL_DECL(object);
    SIZE_DECL(object);
    RENDER_DECL(object);
    GRAPH_DECL(object);
    INSERT_DECL(object);
    MENU_DECL(object);
    MARKER_DECL(object);
    ARITY_DECL(0);
    PREC_DECL(NONE);

    template <typename T, typename U>
    static intptr_t ptrdiff(T *t, U *u)
    {
        return (byte *) t - (byte *) u;
    }

protected:
    static const dispatch   handler[NUM_IDS];

#if DEBUG
public:
    cstring debug() const;
#endif
};

#define PARSE_BODY(D)   object::result D::do_parse(parser &p UNUSED)
#define HELP_BODY(D)    utf8           D::do_help(const D *o UNUSED)
#define EVAL_BODY(D)    object::result D::do_evaluate(const D *o UNUSED)
#define SIZE_BODY(D)    size_t         D::do_size(const D *o UNUSED)
#define RENDER_BODY(D)  size_t         D::do_render(const D *o UNUSED, renderer &r UNUSED)
#define GRAPH_BODY(D)   grob_p         D::do_graph(const D *o UNUSED, grapher &g UNUSED)
#define INSERT_BODY(D)  object::result D::do_insert(const D *o UNUSED)
#define MENU_BODY(D)    bool           D::do_menu(const D *o UNUSED, menu_info &mi UNUSED)
#define MARKER_BODY(D)  unicode        D::do_menu_marker(const D *o UNUSED)

template <typename RPL>
object::result run()
// ----------------------------------------------------------------------------
//  Run a given RPL opcode directly
// ----------------------------------------------------------------------------
{
    const RPL *obj = (const RPL *) RPL::static_object(RPL::static_id);
    return RPL::do_evaluate(obj);
}

#endif // OBJECT_H
