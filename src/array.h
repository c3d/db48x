#ifndef ARRAY_H
#define ARRAY_H
// ****************************************************************************
//  array.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of arrays (vectors, matrices and maybe tensors)
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
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

#include "arithmetic.h"
#include "list.h"
#include "runtime.h"


GCP(array);

struct array : list
// ----------------------------------------------------------------------------
//   An array is a list with [ and ] as delimiters
// ----------------------------------------------------------------------------
{
    array(id type, gcbytes bytes, size_t len): list(type, bytes, len) {}

    static array_g map(algebraic_fn fn, array_r x)
    {
        return x->map(fn);
    }

    array_p map(algebraic_fn fn) const
    {
        return array_p(list::map(fn));
    }

    array_p map(arithmetic_fn fn, algebraic_r y) const
    {
        return array_p(list::map(fn, y));
    }

    array_p map(algebraic_r x, arithmetic_fn fn) const
    {
        return array_p(list::map(x, fn));
    }

    // Append data
    array_p append(array_p a) const     { return array_p(list::append(a)); }
    array_p append(object_p o) const    { return array_p(list::append(o)); }
    static array_p wrap(object_p o);

    // Check if vector or matrix, and push all elements on stack
    bool is_matrix(size_t *rows, size_t *columns,
                   bool push = true, bool striptags = true) const;
    bool is_vector(size_t *size,
                   bool push = true, bool striptags = true) const;
    id   is_2Dor3D(bool push = true) const;
    bool is_vector() const { return is_vector(nullptr, false); }
    bool is_matrix() const { return is_matrix(nullptr, nullptr, false); }
    list_p dimensions(bool expand = false) const;
    bool expand() const;
    static bool size_from_stack(size_t *rows, size_t *columns, uint level=0);
    static bool size_from_object(size_t *rows, size_t *columns, object_r obj);
    static array_p from_stack(size_t rows, size_t columns);
    typedef object_p (*item_fn)(size_t rows, size_t columns,
                                size_t row, size_t column,
                                void *data);
    static array_p build(size_t rows, size_t columns,
                         item_fn items, void *data = nullptr);

    // Compute the result at row r column c from stack-exploded input
    typedef algebraic_p (*vector_fn)(size_t c, size_t cx, size_t cy);
    typedef algebraic_p (*matrix_fn)(size_t r, size_t c,
                                     size_t rx, size_t cx,
                                     size_t ry, size_t cy);
    typedef bool (*dimension_fn)(size_t rx, size_t cx, size_t ry, size_t cy,
                                 size_t *rr, size_t *cr);
    static array_p      do_matrix(array_r x, array_r y,
                                  dimension_fn d, vector_fn v, matrix_fn m);

    algebraic_p         determinant() const;
    algebraic_p         norm_square() const;
    algebraic_p         norm() const;
    array_p             invert() const;

    array_p             to_rectangular() const;
    array_p             to_polar() const;
    array_p             to_cylindrical() const;
    array_p             to_spherical() const;

    static bool         is_non_rectangular(id type)
    {
        return (type == ID_ToPolar       ||
                type == ID_ToCylindrical ||
                type == ID_ToSpherical);
    }

public:
    OBJECT_DECL(array);
    PARSE_DECL(array);
    RENDER_DECL(array);
    GRAPH_DECL(array);
    HELP_DECL(array);
};


array_p operator-(array_r x);
array_p operator+(array_r x, array_r y);
array_p operator-(array_r x, array_r y);
array_p operator*(array_r x, array_r y);
array_p operator/(array_r x, array_r y);

COMMAND_DECLARE(det, 1);
COMMAND_DECLARE_SPECIAL(dot,   command, 2, PREC_DECL(MULTIPLICATIVE); );
COMMAND_DECLARE_SPECIAL(cross, command, 2, PREC_DECL(MULTIPLICATIVE); );
COMMAND_DECLARE(ToArray, ~2);
COMMAND_DECLARE(FromArray, 1);
COMMAND_DECLARE(ConstantArray, 2);
COMMAND_DECLARE(IdentityMatrix, 1);
COMMAND_DECLARE(RandomMatrix, 1);

COMMAND_DECLARE(ToCylindrical, 1);
COMMAND_DECLARE(ToSpherical, 1);
COMMAND_DECLARE(To2DVector, 2);
COMMAND_DECLARE(To3DVector, 3);
COMMAND_DECLARE(FromVector, 1);

#endif // ARRAY_H
