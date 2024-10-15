// ****************************************************************************
//  object.cc                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Runtime support for objects
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

#include "object.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "array.h"
#include "bignum.h"
#include "catalog.h"
#include "characters.h"
#include "comment.h"
#include "compare.h"
#include "complex.h"
#include "conditionals.h"
#include "constants.h"
#include "datetime.h"
#include "decimal.h"
#include "equations.h"
#include "expression.h"
#include "font.h"
#include "fraction.h"
#include "functions.h"
#include "graphics.h"
#include "grob.h"
#include "hwfp.h"
#include "integer.h"
#include "integrate.h"
#include "library.h"
#include "list.h"
#include "locals.h"
#include "logical.h"
#include "loops.h"
#include "menu.h"
#include "parser.h"
#include "plot.h"
#include "polynomial.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "solve.h"
#include "stack-cmds.h"
#include "stats.h"
#include "symbol.h"
#include "tag.h"
#include "text.h"
#include "unit.h"
#include "user_interface.h"
#include "variables.h"

#include <stdio.h>


RECORDER(object,         16, "Operations on objects");
RECORDER(parse,          16, "Parsing objects");
RECORDER(parse_attempts,256, "Attempts parsing an object");
RECORDER(render,         16, "Rendering objects");
RECORDER(eval,           16, "Evaluating objects");
RECORDER(run,            16, "Running commands on objects");
RECORDER(object_errors,  16, "Runtime errors on objects");
RECORDER(assert_error,   16, "Assertion failures");


const object::spelling object::spellings[] =
// ----------------------------------------------------------------------------
//   Table of all the possible spellings for a given type
// ----------------------------------------------------------------------------
{
#define ALIAS(ty, name)         { ID_##ty, name },
#define ID(ty)                  ALIAS(ty, #ty)
#define NAMED(ty, name)         ALIAS(ty, name) ALIAS(ty, #ty)
#include "ids.tbl"
};

const size_t object::spelling_count  = sizeof(spellings) / sizeof(*spellings);


utf8 object::alias(id t, uint index)
// ----------------------------------------------------------------------------
//   Return the name of the object at given index
// ----------------------------------------------------------------------------
{
    for (size_t i = 0; i < spelling_count; i++)
        if (t == spellings[i].type)
            if (cstring name = spellings[i].name)
                if (index-- == 0)
                    return utf8(name);
    return nullptr;
}


utf8 object::fancy(id t)
// ----------------------------------------------------------------------------
//   Return the fancy name for the given index
// ----------------------------------------------------------------------------
{
    return alias(t, 0);
}


utf8 object::name(id t)
// ----------------------------------------------------------------------------
//   Return the name for a given ID with current style
// ------------------------------------------------------------------------
{
    bool compat = Settings.CommandDisplayMode() != ID_LongForm;
    cstring result = nullptr;
    for (size_t i = 0; i < spelling_count; i++)
    {
        if (t == spellings[i].type)
        {
            if (cstring name = spellings[i].name)
            {
                result = name;
                if (!compat)
                    break;
            }
        }
        else if (result)
        {
            break;
        }
    }
    return utf8(result);
}



const object::dispatch object::handler[NUM_IDS] =
// ----------------------------------------------------------------------------
//   Table of handlers for each object type
// ----------------------------------------------------------------------------
{
#define ID(id)          NAMED(id,#id)
#define CMD(id)         ID(id)
#define NAMED(id, label)                                     \
    [ID_##id] = {                                            \
        .size         = (size_fn) id::do_size,               \
        .parse        = (parse_fn) id::do_parse,             \
        .help         = (help_fn) id::do_help,               \
        .evaluate     = (evaluate_fn) id::do_evaluate,       \
        .render       = (render_fn) id::do_render,           \
        .graph        = (graph_fn) id::do_graph,             \
        .insert       = (insert_fn) id::do_insert,           \
        .menu         = (menu_fn) id::do_menu,               \
        .menu_marker  = (menu_marker_fn) id::do_menu_marker, \
        .arity        = id::ARITY,                           \
        .precedence   = id::PRECEDENCE,                      \
    },
#include "ids.tbl"
};


static inline unicode tolow(unicode cp)
// ----------------------------------------------------------------------------
//  A cheap version of tolower for ASCII letters
// ----------------------------------------------------------------------------
{
    return cp | 0x20;
}


object_p object::parse(utf8    source,
                       size_t &size,
                       int     precedence,
                       unicode separator)
// ----------------------------------------------------------------------------
//  Try parsing the object as a top-level temporary
// ----------------------------------------------------------------------------
//  If precedence is set, then we are parsing inside an equation
//  + if precedence > 0, then we are parsing an object of that precedence
//  + if precedence < 0, then we are parsing an infix at that precedence
{
    record(parse, ">Parsing [%s] precedence %d, %u IDs to try",
           source, precedence, NUM_IDS);

    // Skip spaces and newlines
    size_t skipped = utf8_skip_whitespace(source, size);
    if (skipped >= size)
        return nullptr;
    size -= skipped;
    source += skipped;

    // Try parsing with the various handlers
    size_t  length = size;
    result  r      = SKIP;
    unicode cp     = utf8_codepoint(source);
    parser  p(source, size, precedence, separator);

retry:
    switch (cp)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case '+': case '-':
    case '#':                   // Numbers
        r = integer::do_parse(p);
        if (r == OK)
            break;
        if (cp != '#')
        {
    case '.': case ',':
            if (r == SKIP || r == WARN)
                r = hwfp_base::do_parse(p);
            if (r == SKIP)
                r = decimal::do_parse(p);
            if (r == OK)
                rt.clear_error();
        }
        if (r == SKIP && (cp == '+' || cp == '-'))
            r = command::do_parse(p);
        break;
    case '@':                   // Comments, processed above
        r = comment::do_parse(p);
        if (r == OK && p.out)
            break;
        if (r == COMMENTED)
        {
            size_t commented = p.length;
            if (commented >= length)
                return nullptr;
            length -= commented;
            p.length = length;
            p.source += commented;
            skipped += commented;

            size_t spaces = utf8_skip_whitespace(p.source, length);
            if (spaces >= p.length)
                return nullptr;
            skipped += spaces;
            p.source += spaces;
            cp = utf8_codepoint(p.source);
        }
        goto retry;
    case '\'':                  // Expressions
        r = expression::do_parse(p);
        break;
    case '"':                   // Text
        r = text::do_parse(p);
        break;
    case '{':                   // List
        r = list::do_parse(p);
        break;
    case '[':                   // Arrays
        r = array::do_parse(p);
        break;
    case L'«':                  // Programs
        r = program::do_parse(p);
        break;
    case '(':
        r = rectangular::do_parse(p);
        if (r == SKIP && p.precedence)
            r = expression::do_parse(p);
        break;
    case complex::I_MARK:       // Complex numbers in rectangular form
        r = rectangular::do_parse(p);
        break;
    case complex::ANGLE_MARK:   // Polar complex numbers
        r = polar::do_parse(p);
        break;
    case L'Ⓒ':                  // Constants
        r = constant::do_parse(p);
        break;
    case L'Ⓔ':                  // Equations
        r = equation::do_parse(p);
        break;
    case L'Ⓛ':                  // Library items
        r = xlib::do_parse(p);
        break;
    case L'Ⓟ':                  // Polynomials
        r = polynomial::do_parse(p);
        break;
    case ':':                   // Tagged objects
        r = tag::do_parse(p);
        break;
    case L'∂':
        r = Derivative::do_parse(p);
        break;
    case L'∫':
        r = Primitive::do_parse(p);
        break;

    default:
        // Symbols and commands
        r = command::do_parse(p);
        if (r == SKIP && is_valid_as_name_initial(cp))
        {
            if (r == SKIP && (cp == L'→' || cp == L'▶'))
                r = locals::do_parse(p);
            if (r == SKIP)
            {
                switch(tolow(cp))
                {
                case 'c':
                    r = CaseStatement::do_parse(p);
                    break;
                case 'd':
                    r = DoUntil::do_parse(p);
                    if (r == SKIP)
                        r = directory::do_parse(p);
                    break;
                case 'f':
                    r = ForNext::do_parse(p);
                    break;
                case 'i':
                    r = IfThen::do_parse(p);
                    if (r == SKIP)
                        r = IfErrThen::do_parse(p);
                    break;
                case 'w':
                    r = WhileRepeat::do_parse(p);
                    break;
                case 's':
                    r = StartNext::do_parse(p);
                    break;
                case 'g':
                case 'b':
                    r = grob::do_parse(p);
                    break;
                default:
                    break;
                }
            }
            if (r == SKIP)
                r = local::do_parse(p);
            if (r == SKIP)
                r = symbol::do_parse(p);
        }
        break;
    } // Switch statement


    // Compute output size
    record(parse, "<Done parsing [%s], end is at %d", utf8(p.source), p.length);
    size = p.length + skipped;

    // Check if there is a second part to the object
    if (r == OK && p.out && !p.separator && p.length < length &&
        cp != complex::I_MARK && cp != complex::ANGLE_MARK)
    {
        result r2 = SKIP;
        cp = utf8_codepoint(p.source + p.length);

        bool maybe_rect  =
            (precedence < ADDITIVE && (cp == '+' || cp == '-')) ||
            (precedence < POWER && cp == complex::I_MARK);
        bool maybe_polar = cp == complex::ANGLE_MARK;
        bool maybe_unit  = cp == '_' || cp == settings::SPACE_UNIT;
        bool maybe_fcall = p.precedence && (cp == '(' || utf8_whitespace(cp));

        if (maybe_rect || maybe_polar || maybe_unit || maybe_fcall)
        {
            if (p.out->is_algebraic())
            {
                size_t parsed = p.length;
                length -= parsed;
                p.length = length;
                p.source += parsed;
                p.separator = cp;

                if (maybe_rect)
                    r2 = rectangular::do_parse(p);
                else if (maybe_polar)
                    r2 = polar::do_parse(p);
                else if (maybe_unit)
                    r2 = unit::do_parse(p);
                else if (maybe_fcall)
                    r2 = funcall::do_parse(p);

                // Check if we found the second part
                if (r2 == OK)
                    size += p.length;
                else if (r2 != SKIP)
                    r = r2;
            }
        }
    }

    if (r == SKIP || r == WARN)
    {
        if (!rt.error())
            rt.syntax_error();
        if (!rt.source())
            rt.source(p.source, size);
    }

    return r == OK ? p.out : nullptr;
}


bool object::defer() const
// ----------------------------------------------------------------------------
//   Defer evaluation of the given object after next one
// ----------------------------------------------------------------------------
{
    return rt.run_push(this, skip());
}


bool object::defer(id type)
// ----------------------------------------------------------------------------
//   Defer evaluation of a given opcode
// ----------------------------------------------------------------------------
{
    if (object_p obj = command::static_object(type))
        return rt.run_push(obj, obj->skip());
    return false;
}


size_t object::render(char *output, size_t length) const
// ----------------------------------------------------------------------------
//   Render the object in a text buffer
// ----------------------------------------------------------------------------
{
    record(render, "Rendering %+s %p into %p", name(), this, output);
    renderer r(output, length);
    return render(r);
}


size_t object::edit() const
// ----------------------------------------------------------------------------
//   Render an object into the scratchpad, then move it into editor
// ----------------------------------------------------------------------------
{
    utf8 tname = name();     // Object may be GC'd during render
    record(render, "Rendering %+s %p into editor", tname, this);
    renderer r;
    size_t size = render(r);
    record(render, "Rendered %+s as size %u [%s]", tname, size, r.text());
    if (size)
    {
        rt.edit();
        r.clear();
    }
    return size;
}


text_p object::as_text(bool edit, bool equation) const
// ----------------------------------------------------------------------------
//   Render an object into a text
// ----------------------------------------------------------------------------
{
    if (type() == ID_text && !equation)
        return text_p(this);

    record(render, "Rendering %+s %p into text", name(), this);
    renderer r(equation, edit);
    size_t size = render(r);
    record(render, "Rendered %+s as size %u [%s]", name(), size, r.text());
    if (!size)
        return nullptr;
    id type = equation ? ID_symbol : ID_text;
    gcutf8 txt = r.text();
    text_g result = rt.make<text>(type, txt, size);
    return result;
}


uint32_t object::as_uint32(uint32_t def, bool err) const
// ----------------------------------------------------------------------------
//   Return the given object as an uint32
// ----------------------------------------------------------------------------
//   def is the default value if no valid value comes from object
//   err indicates if we error out in that case
{
    id ty = type();
    switch(ty)
    {
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_integer:
    case ID_dec_integer:
    case ID_oct_integer:
    case ID_bin_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_integer:
        return integer_p(this)->value<uint32_t>();
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_bignum:
    case ID_dec_bignum:
    case ID_oct_bignum:
    case ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_bignum:
    case ID_bignum:
        return bignum_p(this)->value<uint32_t>();
    case ID_neg_integer:
    case ID_neg_decimal:
    case ID_neg_bignum:
    case ID_neg_fraction:
    case ID_neg_big_fraction:
        if (err)
            rt.value_error();
        return def;
    case ID_hwfloat:
        return hwfloat_p(this)->as_unsigned();
    case ID_hwdouble:
        return hwdouble_p(this)->as_unsigned();
    case ID_decimal:
        return decimal_p(this)->as_unsigned();

    case ID_fraction:
        return fraction_p(this)->as_unsigned();
    case ID_big_fraction:
        return big_fraction_p(this)->as_unsigned();

    default:
        if (err)
            rt.type_error();
        return def;
    }
}


int32_t object::as_int32 (int32_t  def, bool err)  const
// ----------------------------------------------------------------------------
//   Return the given object as an int32
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_integer:
    case ID_dec_integer:
    case ID_oct_integer:
    case ID_bin_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_integer:
        return integer_p(this)->value<uint32_t>();
    case ID_neg_integer:
        return  -integer_p(this)->value<uint32_t>();
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_bignum:
    case ID_dec_bignum:
    case ID_oct_bignum:
    case ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_bignum:
    case ID_bignum:
        return bignum_p(this)->value<uint32_t>();
    case ID_neg_bignum:
        return -bignum_p(this)->value<uint32_t>();

    case ID_hwfloat:
        return hwfloat_p(this)->as_int32();
    case ID_hwdouble:
        return hwdouble_p(this)->as_int32();
    case ID_decimal:
    case ID_neg_decimal:
        return decimal_p(this)->as_int32();

    case ID_fraction:
        return fraction_p(this)->as_unsigned();
    case ID_neg_fraction:
        return -fraction_p(this)->as_unsigned();
    case ID_big_fraction:
        return big_fraction_p(this)->as_unsigned();
    case ID_neg_big_fraction:
        return -big_fraction_p(this)->as_unsigned();

    default:
        if (err)
            rt.type_error();
        return def;
    }
}


uint64_t object::as_uint64(uint64_t def, bool err) const
// ----------------------------------------------------------------------------
//   Return the given object as an uint64
// ----------------------------------------------------------------------------
//   def is the default value if no valid value comes from object
//   err indicates if we error out in that case
{
    id ty = type();
    switch(ty)
    {
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_integer:
    case ID_dec_integer:
    case ID_oct_integer:
    case ID_bin_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_integer:
        return integer_p(this)->value<uint64_t>();
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_bignum:
    case ID_dec_bignum:
    case ID_oct_bignum:
    case ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_bignum:
    case ID_bignum:
        return bignum_p(this)->value<uint64_t>();
    case ID_neg_integer:
    case ID_neg_decimal:
    case ID_neg_bignum:
    case ID_neg_fraction:
    case ID_neg_big_fraction:
        if (err)
            rt.value_error();
        return def;
    case ID_hwfloat:
        return hwfloat_p(this)->as_unsigned();
    case ID_hwdouble:
        return hwdouble_p(this)->as_unsigned();
    case ID_decimal:
        return decimal_p(this)->as_unsigned();

    case ID_fraction:
        return fraction_p(this)->as_unsigned();
    case ID_big_fraction:
        return big_fraction_p(this)->as_unsigned();

    default:
        if (err)
            rt.type_error();
        return def;
    }
}


int64_t object::as_int64 (int64_t  def, bool err)  const
// ----------------------------------------------------------------------------
//   Return the given object as an int64
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_integer:
    case ID_dec_integer:
    case ID_oct_integer:
    case ID_bin_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_integer:
        return integer_p(this)->value<uint64_t>();
    case ID_neg_integer:
        return  -integer_p(this)->value<uint64_t>();
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_bignum:
    case ID_dec_bignum:
    case ID_oct_bignum:
    case ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_bignum:
    case ID_bignum:
        return bignum_p(this)->value<uint64_t>();
    case ID_neg_bignum:
        return -bignum_p(this)->value<uint64_t>();

    case ID_hwfloat:
        return hwfloat_p(this)->as_integer();
    case ID_hwdouble:
        return hwdouble_p(this)->as_integer();
    case ID_decimal:
    case ID_neg_decimal:
        return decimal_p(this)->as_integer();

    case ID_fraction:
        return fraction_p(this)->as_unsigned();
    case ID_neg_fraction:
        return -fraction_p(this)->as_unsigned();
    case ID_big_fraction:
        return big_fraction_p(this)->as_unsigned();
    case ID_neg_big_fraction:
        return -big_fraction_p(this)->as_unsigned();

    default:
        if (err)
            rt.type_error();
        return def;
    }
}


object_p object::at(size_t index, bool err) const
// ----------------------------------------------------------------------------
//   Return item at given index, works for list, array or text
// ----------------------------------------------------------------------------
{
    id ty = type();
    object_p result = nullptr;
    switch(ty)
    {
    case ID_list:
    case ID_array:
        result = list_p(this)->at(index); break;
    case ID_text:
        result = text_p(this)->at(index); break;
    default:
        if (err)
            rt.type_error();
    }
    // Check if we index beyond what is in the object
    if (err && !result && !rt.error())
        rt.index_error();
    return result;
}


object_p object::at(object_p index) const
// ----------------------------------------------------------------------------
//  Index an object, either from a list or a numerical value
// ----------------------------------------------------------------------------
{
    if (list_p idxlist = index->as_array_or_list())
    {
        object_p result = this;
        for (object_p idxobj : *idxlist)
        {
            result = result->at(idxobj);
            if (!result)
                return nullptr;
        }
        return result;
    }

    size_t idx = index->as_uint32(1, true);
    if (!idx)
        rt.index_error();
    if (rt.error())
        return nullptr;
    return at(idx - 1);
}


object_p object::at(object_p index, object_p value) const
// ----------------------------------------------------------------------------
//  Replace an object at given index with the value
// ----------------------------------------------------------------------------
//  Consider a list L and an index
//    {
//       { 1 2 3 }
//       4
//       {
//          5
//          { 6 7 }
//       }
//    }
//    { 3 3 } "ABC" PUT
//
//    This turns into:
//    L 3 L 3 GET 3 "ABC" PUT PUT
//
{
    object_g ref  = this;
    object_g head = index;
    list_g   tail = nullptr;
    object_g item = value;

    if (list_p idxlist = index->as<list>())
    {
        head = idxlist->head();
        tail = idxlist->tail();
    }
    size_t idx = head->as_uint32(1, true);
    if (!idx)
        rt.index_error();
    if (rt.error())
        return nullptr;
    idx--;

    id ty = ref->type();
    if (is_array_or_list(ty))
    {
        object_g first = ref->at(idx);
        if (!first)
            return nullptr;

        // Check if we need to recurse
        if (tail && tail->length())
            item = first->at(tail, value);

        // For a list, copy bytes before, value bytes, and bytes after
        size_t   size  = 0;
        object_g items = list_p(+ref)->objects(&size);
        size_t   fsize = first->size();
        object_g next  = +first + fsize;
        size_t   hsize = +first - +items;
        size_t   tsize = size - (+next - +items);
        list_g   head  = rt.make<list>(ty, byte_p(+items), hsize);
        list_g   mid   = rt.make<list>(ty, byte_p(+item), item->size());
        list_g   tail  = rt.make<list>(ty, byte_p(+next), tsize);
        return head + mid + tail;
    }

    if (ty == ID_text)
    {
        if (tail && tail->length())
        {
            rt.dimension_error();
            return nullptr;
        }

        // For text, replace the indexed character with the value as text
        text_g tval = value->as_text();
        size_t size  = 0;
        gcutf8 chars = text_p(this)->value(&size);
        size_t o = 0;
        for (o = 0; idx && o < size; o = utf8_next(chars, o))
            idx--;
        if (idx)
        {
            rt.index_error();
            return nullptr;
        }
        size_t n = utf8_next(chars, o);
        text_g head = text::make(chars, o);
        text_g tail = text::make(chars + n, size - n);
        return head + tval + tail;
    }

    rt.type_error();
    return nullptr;
}


bool object::next_index(object_p *indexp) const
// ----------------------------------------------------------------------------
//  Find the next index on this object, returns true if we wrap
// ----------------------------------------------------------------------------
{
    bool wrap = false;
    object_g index = *indexp;
    if (list_g idxlist = index->as<list>())
    {
        object_g obj     = this;
        object_g idxhead = idxlist->head();
        if (!idxhead)
        {
            // Bad argument value error, like on the HP50
            rt.value_error();
            return false;
        }

        list_p   idxtail = idxlist->tail();
        if (idxtail && idxtail->length())
        {
            object_g itobj   = idxtail;
            object_g child   = obj->at(+idxhead);
            if (child->next_index(&+itobj))
                wrap = obj->next_index(&+idxhead);
            idxlist = list::make(idxhead);
            idxlist = idxlist + list_g(list_p(+itobj));
            *indexp = +idxlist;
            return wrap;
        }
        wrap = obj->next_index(&+idxhead);
        idxlist = list::make(idxhead);
        *indexp = +idxlist;
        return wrap;
    }

    size_t idx = index->as_uint32(1, true);
    if (!idx)
        rt.index_error();
    if (rt.error())
        return false;
    wrap = at(idx, false) == nullptr;
    idx = wrap ? 1 : idx + 1;
    *indexp = integer::make(idx);
    return wrap;
}


void object::object_error(id type, object_p ptr)
// ----------------------------------------------------------------------------
//    Report an error in an object
// ----------------------------------------------------------------------------
{
    uintptr_t debug[2];
    byte *d = (byte *) debug;
    byte *s = (byte *) ptr;
    for (uint i = 0; i < sizeof(debug); i++)
        d[i] = s[i];
    record(object_errors,
           "Invalid type %d for %p  Data %16llX %16llX",
           type, ptr, debug[0], debug[1]);
}



// ============================================================================
//
//   Default implementations for the object protocol
//
// ============================================================================

PARSE_BODY(object)
// ----------------------------------------------------------------------------
//   By default, cannot parse an object
// ----------------------------------------------------------------------------
{
    return SKIP;
}


HELP_BODY(object)
// ----------------------------------------------------------------------------
//   Default help topic for an object is the fancy name
// ----------------------------------------------------------------------------
{
    return o->fancy();
}


EVAL_BODY(object)
// ----------------------------------------------------------------------------
//   Show an error if we attempt to evaluate an object
// ----------------------------------------------------------------------------
{
    return rt.push(o) ? OK : ERROR;
}


SIZE_BODY(object)
// ----------------------------------------------------------------------------
//   The default size is just the ID
// ----------------------------------------------------------------------------
{
    return ptrdiff(o->payload(), o);
}


RENDER_BODY(object)
// ----------------------------------------------------------------------------
//  The default for rendering is to print a pointer
// ----------------------------------------------------------------------------
{
    r.printf("Internal:%s[%p]", name(o->type()), o);
    return r.size();
}


grob_p object::as_grob() const
// ----------------------------------------------------------------------------
//   Return object as a graphic object
// ----------------------------------------------------------------------------
{
    grapher g;
    return graph(g);
}


static inline coord flatten_text(grob::surface &s, coord x, coord y,
                                 utf8 start, utf8 end,
                                 font_p font,
                                 grob::pattern fg, grob::pattern bg)
// ----------------------------------------------------------------------------
//   Flatten a text
// ----------------------------------------------------------------------------
{
    for (utf8 wp = start; wp < end; wp = utf8_next(wp))
    {
        unicode cp = utf8_codepoint(wp);
        if (cp == '\t' || cp == '\n')
            cp = ' ';
        x = s.glyph(x, y, cp, font, fg, bg);
    }
    return x;
}


GRAPH_BODY(object)
// ----------------------------------------------------------------------------
//  The default for rendering is to render the text using default font
// ----------------------------------------------------------------------------
{
    renderer r(nullptr, ~0U, g.stack, true, g.expression);
    using pixsize  = blitter::size;
    size_t  sz     = o->render(r);
    gcutf8  txt    = r.text();
    font_p  font   = Settings.font(g.font);
    pixsize fh     = font->height();
    pixsize width  = 0;
    pixsize height = fh;
    pixsize maxw   = g.maxw;
    pixsize maxh   = g.maxh;
    utf8    end    = txt + sz;
    pixsize rw     = 0;
    bool    flat   = false;

    if (g.graph && sz >= 2 && *txt == '"' && end[-1] == '"')
    {
        txt = +txt + 1;
        end = end-1;
        sz -= 2;
    }

    // Try to fit it with the original structure
    for (utf8 p = txt; p < end; p = utf8_next(p))
    {
        unicode c  = utf8_codepoint(p);
        pixsize cw = font->width(c);
        rw += cw;
        if (rw >= maxw)
        {
            flat = true;
            break;
        }
        if (c == '\n')
        {
            if (width < rw - cw)
                width = rw - cw;
            height += fh;
            rw = cw;
            if (height > maxh)
                break;
        }
    }

    // If it was too wide, try "flat" mode, flatten tabs and \n
    if (flat)
    {
        pixsize ww   = 0;
        utf8    word = nullptr;
        utf8    next = nullptr;
        rw           = 0;
        width        = 0;
        height       = fh;

        for (utf8 p = txt; p < end; p = next)
        {
            unicode c = utf8_codepoint(p);
            next = utf8_next(p);
            if (c == '\n' || c == '\t')
                c = ' ';
            bool    sp = is_unicode_space(c);
            pixsize cw = font->width(c);
            rw += cw;
            if (sp)
            {
                ww = 0;
                word = nullptr;
            }
            else
            {
                ww += cw;
                if (!word)
                    word = p;
            }
            if (!sp && rw > maxw)
            {
                if (word)
                {
                    rw -= ww;
                    next = word;
                    word = nullptr;
                }
                else if (cw > maxw)
                {
                    break;
                }
                else
                {
                    rw -= cw;
                    next = p;
                }

                if (width < rw)
                    width = rw;
                height += fh;
                if (height > maxh)
                    break;
                rw = 0;
            }
        }
    }

    if (width < rw)
        width = rw;

    grob_g  result = g.grob(width, height);
    if (!result)
        return result;
    grob::surface s = result->pixels();
    coord         x = 0;
    coord         y = 0;
    s.fill(g.background);

    // Reset end pointer in case grob allocation caused a GC
    end = txt + sz;

    if (flat)
    {
        // Flat mode - Cut at word boundaries
        utf8    word = nullptr;
        utf8    next = nullptr;
        utf8    row  = txt;
        rw           = 0;

        for (utf8 p = txt; p < end; p = next)
        {
            unicode c = utf8_codepoint(p);
            next = utf8_next(p);
            if (c == '\n' || c == '\t')
                c = ' ';
            bool    sp = is_unicode_space(c);
            pixsize cw = font->width(c);
            rw += cw;
            if (sp)
                word = nullptr;
            else if (!word)
                word = p;
            if (!sp && rw > width)
            {
                if (word)
                {
                    x = flatten_text(s, x, y, row, word,
                                     font, g.foreground, g.background);
                    next = word;
                    row = word;
                    word = nullptr;
                }
                else if (cw > width)
                {
                    break;
                }
                else
                {
                    x = flatten_text(s, x, y, row, p,
                                     font, g.foreground, g.background);
                    next = p;
                    row = p;
                }

                y += fh;
                if (y > coord(maxh))
                    break;
                x = 0;
                rw = 0;
            }
        }
        flatten_text(s, x, y, row, end, font, g.foreground, g.background);
    }
    else
    {
        // We can display it with the structure
        for (utf8 p = txt; p < end; p = utf8_next(p))
        {
            unicode c  = utf8_codepoint(p);
            pixsize cw = font->width(c);
            if (x + cw > width || c == '\n')
            {
                y += fh;
                if (y > coord(maxh))
                    break;
                x = 0;
            }
            x = s.glyph(x, y, c, font, g.foreground, g.background);
        }
    }

    return result;
}


grob_p object::graph() const
// ----------------------------------------------------------------------------
//  Render the object like for the `Show` command
// ----------------------------------------------------------------------------
{
    object_g obj = this;
    uint digits = Settings.DisplayDigits();
    if (obj->is_decimal())
        digits = decimal_p(+obj)->kigits() * 3;
    else if (obj->is_complex())
        digits = Settings.Precision();
    settings::SaveDisplayDigits sdd(digits);

    using size = grob::pixsize;
    grob_g  graph  = obj->is_graph() ? grob_p(+obj) : nullptr;
    size    width  = LCD_W;
    size    height = LCD_H;
    grapher g(width, height, settings::EDITOR,
              Settings.Foreground(), Settings.Background(), true);
    while (!graph)
    {
        graph = obj->graph(g);
        if (graph)
            break;

        if (g.reduce_font())
            continue;
        if (g.maxh < Settings.MaximumShowHeight())
        {
            g.maxh = Settings.MaximumShowHeight();
            g.font = settings::EDITOR;
            continue;
        }
        if (g.maxw < Settings.MaximumShowWidth())
        {
            g.maxw = Settings.MaximumShowWidth();
            g.font = settings::EDITOR;
            continue;
        }

        // Exhausted all options, give up
        break;
    }
    return graph;
}


INSERT_BODY(object)
// ----------------------------------------------------------------------------
//   Default insertion is as a program object
// ----------------------------------------------------------------------------
{
    return ui.edit(o->name(), ui.PROGRAM);
}


MENU_BODY(object)
// ----------------------------------------------------------------------------
//   No operation on menus by default
// ----------------------------------------------------------------------------
{
    return false;
}


MARKER_BODY(object)
// ----------------------------------------------------------------------------
//   No menu marker by default
// ----------------------------------------------------------------------------
{
    return 0;
}


object_p object::as_quoted(id ty) const
// ----------------------------------------------------------------------------
//   Check if something is a quoted value of the given type
// ----------------------------------------------------------------------------
//   This is typically used to quote symbols or locals (e.g. 'A')
{
    if (type() == ty)
        return this;
    if (expression_p eq = as<expression>())
        return eq->quoted(ty);
    return nullptr;
}


int object::as_truth(bool error) const
// ----------------------------------------------------------------------------
//   Get the logical value for an object, or -1 if invalid
// ----------------------------------------------------------------------------
{
    id ty = type();

    switch(ty)
    {
    case ID_True:
    case ID_False:
    case ID_integer:
    case ID_neg_integer:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_integer:
    case ID_oct_integer:
    case ID_dec_integer:
    case ID_hex_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_bignum:
    case ID_neg_bignum:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_bignum:
    case ID_oct_bignum:
    case ID_dec_bignum:
    case ID_hex_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_fraction:
    case ID_neg_fraction:
    case ID_big_fraction:
    case ID_neg_big_fraction:
    case ID_hwfloat:
    case ID_hwdouble:
    case ID_decimal:
    case ID_neg_decimal:
    case ID_polar:
    case ID_rectangular:
    case ID_unit:
        return !is_zero(error);

    default:
        if (error)
            rt.type_error();
    }
    return -1;
}


algebraic_p object::as_algebraic() const
// ----------------------------------------------------------------------------
//   Return the value as an algebraic if possible
// ----------------------------------------------------------------------------
{
    object_p untagged = tag::strip(this);
    if (untagged && untagged->is_algebraic())
        return algebraic_p(untagged);
    return nullptr;
}


bool object::is_zero(bool error) const
// ----------------------------------------------------------------------------
//   Check if an object is zero
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
    case ID_True:
        return false;
    case ID_False:
        return true;
    case ID_integer:
    case ID_neg_integer:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_integer:
    case ID_oct_integer:
    case ID_dec_integer:
    case ID_hex_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
        return integer_p(this)->is_zero();
    case ID_bignum:
    case ID_neg_bignum:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_bignum:
    case ID_oct_bignum:
    case ID_dec_bignum:
    case ID_hex_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
        return bignum_p(this)->is_zero();
    case ID_fraction:
    case ID_neg_fraction:
        return fraction_p(this)->is_zero();
    case ID_big_fraction:
    case ID_neg_big_fraction:
        return big_fraction_p(this)->numerator()->is_zero();
    case ID_hwfloat:
        return hwfloat_p(this)->is_zero();
    case ID_hwdouble:
        return hwdouble_p(this)->is_zero();
    case ID_decimal:
    case ID_neg_decimal:
        return decimal_p(this)->is_zero();
    case ID_polar:
        return polar_p(this)->is_zero();
    case ID_rectangular:
        return rectangular_p(this)->is_zero();
    case ID_unit:
        return unit_p(this)->value()->is_zero(error);

    default:
        if (error)
            rt.type_error();
    }
    return false;
}


bool object::is_one(bool error) const
// ----------------------------------------------------------------------------
//   Check if an object is zero
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
    case ID_integer:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_integer:
    case ID_oct_integer:
    case ID_dec_integer:
    case ID_hex_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
        return integer_p(this)->is_one();
    case ID_bignum:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_bignum:
    case ID_oct_bignum:
    case ID_dec_bignum:
    case ID_hex_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
        return bignum_p(this)->is_one();
    case ID_fraction:
        return fraction_p(this)->is_one();
    case ID_hwfloat:
        return hwfloat_p(this)->is_one();
    case ID_hwdouble:
        return hwdouble_p(this)->is_one();
    case ID_decimal:
    case ID_neg_decimal:
        return decimal_p(this)->is_one();
    case ID_polar:
        return polar_p(this)->is_one();
    case ID_rectangular:
        return rectangular_p(this)->is_one();
    case ID_neg_integer:
    case ID_neg_bignum:
    case ID_neg_fraction:
        return false;

    default:
        if (error)
            rt.type_error();
    }
    return false;
}


bool object::is_negative(bool error) const
// ----------------------------------------------------------------------------
//   Check if an object is negative
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
    case ID_integer:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_integer:
    case ID_oct_integer:
    case ID_dec_integer:
    case ID_hex_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_integer:
    case ID_bignum:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_bin_bignum:
    case ID_oct_bignum:
    case ID_dec_bignum:
    case ID_hex_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_fraction:
    case ID_big_fraction:
        return false;
    case ID_neg_integer:
    case ID_neg_bignum:
    case ID_neg_fraction:
    case ID_neg_big_fraction:
        return !fraction_p(this)->is_zero();
    case ID_hwfloat:
        return hwfloat_p(this)->is_negative();
    case ID_hwdouble:
        return hwdouble_p(this)->is_negative();
    case ID_decimal:
    case ID_neg_decimal:
        return decimal_p(this)->is_negative();
    case ID_unit:
        return unit_p(this)->value()->is_negative(error);

    default:
        if (error)
            rt.type_error();
    }
    return false;
}


int object::compare_to(object_p other) const
// ----------------------------------------------------------------------------
//   Bitwise comparison of two objects
// ----------------------------------------------------------------------------
{
    if (other == this)
        return 0;
    id ty = type();
    id oty = other->type();
    if (ty != oty)
        return ty > oty ? -1 : 1;
    size_t sz = size();
    size_t osz = other->size();
    size_t ssz = sz < osz ? sz : osz;
    if (int diff = memcmp(this, other, ssz))
        return diff;
    return sz < osz ? -1 : sz > osz ? 1 : 0;
}


object_p object::child(uint index) const
// ----------------------------------------------------------------------------
//    For a complex, list or array, return nth element
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch (ty)
    {
    case ID_rectangular:
        return index ? rectangular_p(this)->im() : rectangular_p(this)->re();
    case ID_polar:
        return index ? polar_p(this)->im() : polar_p(this)->re();

    case ID_list:
    case ID_array:
        if (object_p obj = list_p(this)->at(index))
            return obj;
        rt.value_error();
        break;
    default:
        rt.type_error();
        break;
    }
    return nullptr;
}


algebraic_p object::algebraic_child(uint index) const
// ----------------------------------------------------------------------------
//    For a complex, list or array, return nth element as algebraic
// ----------------------------------------------------------------------------
{
    if (object_p obj = child(index))
    {
        if (obj->is_algebraic())
            return algebraic_p(obj);
        else
            rt.type_error();
    }

    return nullptr;
}


bool object::is_big() const
// ----------------------------------------------------------------------------
//   Return true if any component is a big num
// ----------------------------------------------------------------------------
{
    id ty = type();
    switch(ty)
    {
    case ID_bignum:
    case ID_neg_bignum:
    case ID_big_fraction:
    case ID_neg_big_fraction:
#if CONFIG_FIXED_BASED_OBJECTS
    case ID_hex_bignum:
    case ID_dec_bignum:
    case ID_oct_bignum:
    case ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case ID_based_bignum:
        return true;

    case ID_list:
    case ID_program:
    case ID_block:
    case ID_array:
    case ID_expression:
        for (object_p o : *(list_p(this)))
            if (o->is_big())
                return true;
        return false;

    case ID_rectangular:
    case ID_polar:
        return complex_p(this)->x()->is_big() || complex_p(this)->y()->is_big();
    case ID_unit:
        return unit_p(this)->value()->is_big();

    default:
        return false;
    }
}


object_p object::static_object(id i)
// ----------------------------------------------------------------------------
//   Return a pointer to a static object representing the command
// ----------------------------------------------------------------------------
{
    static byte cmds[] =
    {
#define ID(id)                                                \
    object::ID_##id < 0x80 ? (object::ID_##id & 0x7F) | 0x00  \
                           : (object::ID_##id & 0x7F) | 0x80, \
    object::ID_##id < 0x80 ? 0 : ((object::ID_##id) >> 7),
#include "ids.tbl"
    };

    if (i >= NUM_IDS)
        i = ID_object;

    return (object_p) (cmds + 2 * i);
}


int object::type_value(id ty)
// ----------------------------------------------------------------------------
//   Return the numerical type value as per the `TYPE` command
// ----------------------------------------------------------------------------
{
    if (Settings.CompatibleTypes())
    {
        int type = 0;
        switch (ty)
        {
        case object::ID_hwfloat:
        case object::ID_hwdouble:
        case object::ID_decimal:                type = 0; break; // Or 21
        case object::ID_rectangular:
        case object::ID_polar:                  type = 1; break;
        case object::ID_text:                   type = 2; break;
        // Treat as symbolic vector matrix on HP50G,
        // don't check inside to see if it's real (3) or complex (4) array
        case object::ID_array:                  type = 29; break;
        case object::ID_list:                   type = 5; break;
        case object::ID_symbol:                 type = 6; break;
        case object::ID_local:                  type = 7; break;
        case object::ID_block:
        case object::ID_locals:
        case object::ID_program:                type = 8; break;
        case object::ID_fraction:
        case object::ID_neg_fraction:
        case object::ID_big_fraction:
        case object::ID_neg_big_fraction:
        case object::ID_expression:             type = 9; break;

#ifdef CONFIG_FIXED_BASED_OBJECTS
        case object::ID_hex_integer:
        case object::ID_dec_integer:
        case object::ID_oct_integer:
        case object::ID_bin_integer:
        case object::ID_hex_bignum:
        case object::ID_dec_bignum:
        case object::ID_oct_bignum:
        case object::ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
        case object::ID_based_integer:
        case object::ID_based_bignum:           type = 10; break;
        case object::ID_grob:
        case object::ID_bitmap:                 type = 11; break;
        case object::ID_tag:                    type = 12; break;
        case object::ID_unit:                   type = 13; break;
        // No XLIB type 14 yet
        case object::ID_directory:              type = 15; break;
        // No Library type 16 yet
        // No Backup object type 17 yet
        case object::ID_integer:
        case object::ID_neg_integer:
        case object::ID_bignum:
        case object::ID_neg_bignum:             type = 28; break;
        case object::ID_dense_font:             type = 27; break;
        case object::ID_sparse_font:            type = 30; break;

        default:
            type = object::is_algebraic(object::id(type)) ? 18 : 19;
            break;
        }
        return type;
    }
    return ~int(ty);
}


#if DEBUG
cstring object::debug() const
// ----------------------------------------------------------------------------
//   Render an object from the debugger
// ----------------------------------------------------------------------------
{
    renderer r(false, true, true);
    render(r);
    r.put(char(0));
    return cstring(r.text());
}


cstring debug(object_p object)
// ----------------------------------------------------------------------------
//    Print an object pointer, for use in the debugger
// ----------------------------------------------------------------------------
{
    return object ? object->debug() : nullptr;
}


cstring debug(object_g object)
// ----------------------------------------------------------------------------
//   Same from an object_g
// ----------------------------------------------------------------------------
{
    return object ? object->debug() : nullptr;
}


cstring debug(object *object)
// ----------------------------------------------------------------------------
//   Same from an object *
// ----------------------------------------------------------------------------
{
    return object ? object->debug() : nullptr;
}


cstring debug(uint level)
// ----------------------------------------------------------------------------
//   Read a stack level
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.stack(level))
    {
        // We call both the object_g and object * variants so linker keeps them
        if (cstring result = obj->debug())
            return result;
        else if (object *op = (object *) object_p(obj))
            return debug(op);
    }
    return nullptr;
}


cstring debug()
// ----------------------------------------------------------------------------
//   Read top of stack
// ----------------------------------------------------------------------------
{
    return debug(0U);
}
#endif // SIMULATOR
