// ****************************************************************************
//  array.cc                                                      DB48X project
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

#include "array.h"

#include "arithmetic.h"
#include "compare.h"
#include "expression.h"
#include "functions.h"
#include "grob.h"
#include "stack-cmds.h"
#include "stats.h"
#include "tag.h"
#include "unit.h"
#include "variables.h"


RECORDER(matrix, 16, "Determinant computation");
RECORDER(matrix_error, 16, "Errors in matrix computations");



// ============================================================================
//
//    Array
//
// ============================================================================

PARSE_BODY(array)
// ----------------------------------------------------------------------------
//    Try to parse this as a program
// ----------------------------------------------------------------------------
{
    return list::list_parse(ID_array, p, '[', ']');
}


RENDER_BODY(array)
// ----------------------------------------------------------------------------
//   Render the program into the given program buffer
// ----------------------------------------------------------------------------
{
    return o->list_render(r, '[', ']');
}


HELP_BODY(array)
// ----------------------------------------------------------------------------
//   Help topic for arrays
// ----------------------------------------------------------------------------
{
    return utf8("Vectors and matrices");
}


GRAPH_BODY(array)
// ----------------------------------------------------------------------------
//   Render an array graphically
// ----------------------------------------------------------------------------
{
    using pixsize   = grob::pixsize;
    array_g a       = o;
    if (!a)
        return nullptr;

    size_t rows = 0;
    size_t cols = 0;
    bool   mat  = false;
    bool   vec  = false;
    if (a->is_matrix(&rows, &cols, true, false))
    {
        mat = rows > 0 && cols > 0;
    }
    else if (a->is_vector(&cols, true, false))
    {
        vec = true;
        rows = 1;
        if (Settings.VerticalVectors())
        {
            rows = cols;
            cols = 1;
        }
    }

    // Fallback if we don't have a proper matrix or vector, e.g. non-rect
    if (!mat && !vec)
        return object::do_graph(a, g);

    grob_g result = a->graph(g, rows, cols, mat);
    if (!result)
        return nullptr;
    grob::surface rs = result->pixels();
    pixsize gw = rs.width();
    pixsize gh = rs.height();

    // Dimensions of border and adjust
    coord   xl = 0;
    coord   xr = coord(gw) - 2;
    coord   yt = 0;
    coord   yb = coord(gh) - 4;
    pixsize bw = mat ? 4 : 2;


    // Add the borders
    for (coord y = 1; y < yb; y++)
    {
        rs.fill(xl,    y, xl+bw,   y, g.foreground);
        rs.fill(xr-bw, y, xr,      y, g.foreground);
    }
    rs.fill(xl,      yt, xl+2*bw, yt+1, g.foreground);
    rs.fill(xr-2*bw, yt, xr,      yt+1, g.foreground);
    rs.fill(xl,      yb, xl+2*bw, yb+1, g.foreground);
    rs.fill(xr-2*bw, yb, xr,      yb+1, g.foreground);

    return result;
}



// ============================================================================
//
//   Checking if a given array is a vector or a matrix
//
// ============================================================================
//
//   When they return true, these operations push all elements on the stack
//

bool array::is_vector(size_t *size, bool push, bool stripped) const
// ----------------------------------------------------------------------------
//   Check if this is a vector, and if so, push all elements on stack
// ----------------------------------------------------------------------------
{
    bool result = type() == ID_array;
    if (result)
    {
        size_t count = 0;
        for (object_p obj : *this)
        {
            object_p tagged = obj;
            obj = strip(obj);

            id oty = obj->type();
            if (is_array_or_list(oty))
                result = false;
            else if (!obj->is_algebraic())
                result = false;
            else if (push && !rt.push(stripped ? obj : tagged))
                result = false;
            if (!result)
                break;
            count++;
        }
        if (!result && push)
            rt.drop(count);
        else if (size)
            *size = count;
    }
    return result;
}


object::id array::is_2Dor3D(bool push) const
// ----------------------------------------------------------------------------
//   Check if this is a 2D or 3D vector, returns an ID of its structure
// ----------------------------------------------------------------------------
{
    id r = type() == ID_array ? ID_array : ID_object;
    if (r)
    {
        size_t count  = 0;
        uint   angles = 0;
        for (object_p obj : *this)
        {
            obj = strip(obj);

            id oty = obj->type();
            if (is_array_or_list(oty))
                r = ID_object;
            else if (!obj->is_algebraic())
                r = ID_object;
            else if (push && !rt.push(obj))
                r = ID_object;
            if (!r)
                break;

            algebraic_g x = algebraic_p(obj);
            if (x->is_symbolic())
                r = ID_object;
            else if (algebraic::adjust_angle(x))
                angles |= (1<<count);
            count++;
        }

        if (count != 2 && count != 3)
            r = ID_object;

        if (r)
        {
            r = (count == 2   ? (angles == 0 || angles == 3 ? ID_To2DVector
                                 : angles == 2              ? ID_ToPolar
                                                            : ID_object)
                 : count == 3 ? (angles == 0 || angles == 7 ? ID_To3DVector
                                 : angles == 2              ? ID_ToCylindrical
                                 : angles == 6              ? ID_ToSpherical
                                                            : ID_object)
                              : ID_object);
        }

        if (!r && push)
            rt.drop(count);
    }
    return r;
}


bool array::is_matrix(size_t *rows, size_t *cols, bool push, bool strip) const
// ----------------------------------------------------------------------------
//   Check if this is a vector, and if so, push all elements on stack
// ----------------------------------------------------------------------------
{
    bool result = type() == ID_array;
    if (result)
    {
        size_t depth = rt.depth();
        size_t r = 0;
        size_t c = 0;
        bool first = true;

        for (object_p robj : *this)
        {
            id oty = robj->type();
            result = oty == ID_array;
            if (result)
            {
                size_t rcol = 0;
                result = array_p(robj)->is_vector(&rcol, push, strip);
                if (result && first)
                    c = rcol;
                else if (rcol != c)
                    result = false;
                first = false;
            }
            if (!result)
                break;
            r++;
        }
        if (!result)
        {
            rt.drop(rt.depth() - depth);
        }
        else
        {
            if (rows)
                *rows = r;
            if (cols)
                *cols = c;
        }
    }
    return result;
}


bool array::is_matrix_or_vector(size_t *rows, size_t *cols,
                                bool push, bool strip) const
// ----------------------------------------------------------------------------
//   Return true if this is a matrix or vector
// ----------------------------------------------------------------------------
{
    return (is_matrix(rows, cols, push, strip) ||
            is_vector(rows, push, strip));
}


list_p array::dimensions(bool expand) const
// ----------------------------------------------------------------------------
//   Return the dimensions of the array
// ----------------------------------------------------------------------------
{
    size_t rows = 0, columns = 0;
    size_t depth = rt.depth();
    if (is_vector(&columns, expand))
    {
        integer_g cobj = integer::make(columns);
        if (cobj)
            return list::make(cobj);
    }
    else if (is_matrix(&rows, &columns, expand))
    {
        integer_g robj = integer::make(rows);
        integer_g cobj = integer::make(columns);
        if (robj && cobj)
            return list::make(robj, cobj);
    }
    rt.drop(rt.depth() - depth);
    return nullptr;
}


bool array::expand() const
// ----------------------------------------------------------------------------
//   Expand the array
// ----------------------------------------------------------------------------
{
    if (list_p dims = dimensions(true))
        if (rt.push(dims))
            return true;
    return false;
}


array_p array::wrap(object_p o)
// ----------------------------------------------------------------------------
//   Wrap object in a single-item arrray
// ----------------------------------------------------------------------------
{
    return array_p(list::make(ID_array, byte_p(o), o->size()));
}


bool array::size_from_stack(size_t *rows, size_t *columns, uint level)
// ----------------------------------------------------------------------------
//   Take the given level of the stack and interpret that as array size
// ----------------------------------------------------------------------------
{
    if (object_g dims = rt.stack(level))
        return size_from_object(rows, columns, dims);
    return false;
}


bool array::size_from_object(size_t *rows, size_t *columns, object_r dims)
// ----------------------------------------------------------------------------
//   Take the given object and interpret that as array size
// ----------------------------------------------------------------------------
{
    id ty = dims->type();
    if (is_array_or_list(ty))
    {
        object_g robj = list_p(+dims)->at(0);
        object_g cobj = list_p(+dims)->at(1);
        if (list_p(+dims)->at(2))
        {
            rt.dimension_error();
        }
        else
        {
            if (robj && rows)
                *rows = robj->as_uint32(0, true);
            if (cobj && columns)
                *columns = cobj->as_uint32(0, true);
            return !rt.error();
        }
    }
    else
    {
        if (rows)
            *rows = dims->as_uint32(0, true);
        if (columns)
            *columns = 0;
        return !rt.error();
    }
    return false;
}


static object_p item_from_stack(size_t rows, size_t columns,
                                size_t r, size_t c, void *)
// ----------------------------------------------------------------------------
//   Return an item from the stack
// ----------------------------------------------------------------------------
{
    if (!columns)
        columns = 1;
    size_t nitems = rows * columns;
    size_t idx = r * columns + c;
    if (object_p obj = rt.stack(nitems + ~idx))
        return obj;
    return nullptr;
}


static object_p transpose_from_stack(size_t rows, size_t columns,
                                     size_t r, size_t c, void *)
// ----------------------------------------------------------------------------
//   Return an item from the stack
// ----------------------------------------------------------------------------
{
    if (!rows)
        rows = 1;
    size_t nitems = rows * columns;
    size_t idx = c * rows + r;
    if (object_p obj = rt.stack(nitems + ~idx))
        return obj;
    return nullptr;
}


array_p array::from_stack(size_t rows, size_t columns, bool transpose)
// ----------------------------------------------------------------------------
//   Build an array from items on the stack
// ----------------------------------------------------------------------------
{
    array_p result = transpose
        ? build(columns, rows, transpose_from_stack)
        : build(rows, columns, item_from_stack);

    if (result)
        rt.drop(rows * (columns ? columns : 1));
    return result;
}


array_p array::build(size_t rows, size_t columns, item_fn items, void *data)
// ----------------------------------------------------------------------------
//   Build an a matrix or vector (columns == 0) from items in function
// ----------------------------------------------------------------------------
{
    scribble scr;
    if (rows)
    {
        if (columns)
        {
            list_g row = nullptr;
            for (size_t r = 0; r < rows; r++)
            {
                {
                    scribble srow;
                    for (size_t c = 0; c < columns; c++)
                    {
                        object_g it = items(rows, columns, r, c, data);
                        if (program::interrupted() || !rt.append(it))
                            return nullptr;
                    }
                    row = list::make(ID_array, srow.scratch(), srow.growth());
                }
                if (program::interrupted() || !rt.append(row))
                    return nullptr;
            }
        }
        else
        {
            for (size_t r = 0; r < rows; r++)
            {
                object_g it = items(rows, columns, r, 0, data);
                if (program::interrupted() || !rt.append(it))
                    return nullptr;
            }
        }
    }
    list_p result = list::make(ID_array, scr.scratch(), scr.growth());
    return array_p(result);
}



// ============================================================================
//
//    Additive operations
//
// ============================================================================

array_p operator-(array_r x)
// ----------------------------------------------------------------------------
//   Negate all elements in an array
// ----------------------------------------------------------------------------
{
    return array::map(neg::evaluate, x);
}



// ============================================================================
//
//    Determinant
//
// ============================================================================

algebraic_p array::determinant() const
// ----------------------------------------------------------------------------
//   Compute the determinant of a square matrix
// ----------------------------------------------------------------------------
{
    size_t cx, rx;
    size_t depth = rt.depth();
    algebraic_g det = this;
    if (is_matrix(&rx, &cx))
    {
        if (rx != cx)
        {
            rt.dimension_error();
            goto err;
        }

        size_t      n   = cx;
        size_t      pt  = n;    // n temporary elements to save diagonal
        size_t      px  = n * n + n;
        bool        neg = false;
        algebraic_g tot;

        // Make space for temporary elements
        for (size_t j = 0; j < n; j++)
            if (!rt.push(+det))
                goto err;
        det = nullptr;

#if SIMULATOR
        record(matrix, "Determinant of %ux%u matrix", n, n);
        for (uint j = 0; j < n; j++)
        {
            for (uint k = 0; k < n; k++)
            {
                size_t ixjk = j * n + k;
                object_p mjk = rt.stack(px + ~ixjk);
                record(matrix, "m[%u, %u] = %t", j, k, mjk);
            }
        }
#endif // SIMULATOR

        // Loop across the diagonal
        for (size_t i = 0; i < n; i++)
        {
            // Find the index of first non-zero element
            bool zero = true;
            size_t index;

            if (program::interrupted())
                goto err;

            record(matrix, " Row %u", i);
            for (index = i; zero && index < n; index++)
            {
                size_t ix = index * n + i;
                object_p xij = rt.stack(px + ~ix);
                if (!xij)
                    goto err;
                zero = xij->is_zero(false);
                record(matrix, "  Index %u xij=%t %+s",
                       index, xij, zero ? "zero" : "non-zero");
                if (!zero)
                    break;
            }

            // If only zeroes, determinant is zero
            if (zero)
            {
                record(matrix, "Determinant is zero");
                rt.drop(rt.depth() - depth);
                return integer::make(0);
            }

            // Check if need to swap diagonal element row and index rows
            record(matrix, " Row %u index %u", i, index);
            if (index != i)
            {
                record(matrix, " Swapping %u and %u", index, i);

                // Swap diagonal element row and index row
                for (size_t j = 0; j < n; j++)
                {
                    size_t ia = index * n + j;
                    size_t ib = i * n + j;
                    object_p a = rt.stack(px + ~ia);
                    object_p b = rt.stack(px + ~ib);
                    rt.stack(px + ~ia, b);
                    rt.stack(px + ~ib, a);
                }

#if SIMULATOR
                record(matrix, " After swapping %u and %u");
                for (uint j = 0; j < n; j++)
                {
                    for (uint k = 0; k < n; k++)
                    {

                        size_t ixjk = j * n + k;
                        object_p mjk = rt.stack(px + ~ixjk);
                        record(matrix, "  m[%u, %u] = %t", j, k, mjk);
                    }
                }
#endif // SIMULATOR
                if ((index - i) & 1)
                {
                    neg = !neg;
                    record(matrix, " Determinant is now %+s",
                           neg ? "negative" : "positive");
                }
            }

            // Store value for diagonal row elements
            record(matrix, " Saving row %u", i);
            for (size_t j = 0; j < n; j++)
            {
                size_t ixij = i * n + j;
                object_p matij = rt.stack(px + ~ixij);
                rt.stack(pt + ~j, matij);
                record(matrix, "  t[%u]=%t", j, matij);
            }

            // Traverse every row below the diagonal
            for (size_t j = i + 1; j < n; j++)
            {
                // Fetch value on diagonal and in next row
                size_t ixij = j * n + i;
                object_p a = rt.stack(pt + ~i);
                object_p b = rt.stack(px + ~ixij);
                if (!a || !b)
                    goto err;
                algebraic_g aa = a->as_algebraic();
                algebraic_g ba = b->as_algebraic();
                if (!aa || !ba)
                    goto err;

                record(matrix, "  m[%u,%u] a=%t", j, i, a);
                record(matrix, "  m[%u,%u] b=%t", j, i, b);

                // Traverse columns in this row
                for (size_t k = 0; k < n; k++)
                {
                    if (program::interrupted())
                        goto err;

                    size_t ixjk = j * n + k;
                    object_p mjk = rt.stack(px + ~ixjk);
                    object_p tk = rt.stack(pt + ~k);
                    if (!mjk || !tk)
                        goto err;
                    algebraic_g mjka = mjk->as_algebraic();
                    algebraic_g tka = tk->as_algebraic();
                    if (!mjka || !tka)
                        goto err;
                    mjka = aa * mjka - ba * tka;
                    record(matrix, "  m[%u,%u] is now %t", j, k, mjk);
                    rt.stack(px + ~ixjk, +mjka);
                }

                // Compute total
                tot = tot ? tot * aa : aa;
                record(matrix, " tot[%u]=%t", j, +tot);
            }

#if SIMULATOR
            record(matrix, " After diagonalization of row %u", i);
            for (uint j = 0; j < n; j++)
            {
                for (uint k = 0; k < n; k++)
                {

                    size_t ixjk = j * n + k;
                    object_p mjk = rt.stack(px + ~ixjk);
                    record(matrix, "m[%u, %u] = %t", j, k, mjk);
                }
            }
#endif // SIMULATOR
        }

        // Multiply diagonal elements to get determinant
        for (size_t i = 0; i < n; i++)
        {
            size_t ixii = i * n + i;
            object_p diag = rt.stack(px + ~ixii);
            if (!diag)
                goto err;
            algebraic_g diaga = diag->as_algebraic();
            if (!diaga)
                goto err;
            det = det ? det * diaga : diaga;
            record(matrix, "Diag %u det=%t", i, +det);
            if (!det)
                goto err;
        }

        // Return result
        rt.drop(rt.depth() - depth);
        det = det / tot;
        if (neg)
            det = -det;
        record(matrix, "Result det=%t", +det);
        return det;
    }
    else
    {
        rt.type_error();
    }
    return nullptr;

err:
    rt.drop(rt.depth() - depth);
    return nullptr;
}


#if SIMULATOR
static void dump_matrix_internal(size_t n, size_t pm, size_t pt)
// ----------------------------------------------------------------------------
//    Dump a matricx for debugging purpose
// ----------------------------------------------------------------------------
{
    for (uint j = 0; j < n; j++)
    {
        for (uint k = 0; k < n; k++)
        {
            size_t ojk = j * n + k;
            object_p mjk = rt.stack(pm + ~ojk);
            record(matrix, "    m[%u, %u] = %t", j, k, mjk);
        }
    }
    for (uint j = 0; j < n; j++)
    {
        for (uint k = 0; k < n; k++)
        {
            size_t ojk = j * n + k;
            object_p mjk = rt.stack(pt + ~ojk);
            record(matrix, "    i[%u, %u] = %t", j, k, mjk);
        }
    }
}
#define dump_matrix(...)                        \
do                                              \
{                                               \
    record(matrix, __VA_ARGS__);                \
    dump_matrix_internal(n, pm, pt);            \
} while (0)

#else

#define dump_matrix(msg, ...)

#endif // SIMULATOR


array_p array::invert() const
// ----------------------------------------------------------------------------
//   Compute the inverse of a square matrix
// ----------------------------------------------------------------------------
//
//   We start by creating an identity matrix of the right size
//
//      | .. ..  .. |  | 1          |
//   i  | .. ... .. |  |   1        |
//      | .. ... .. |  |     1      |
//   j  | .. ... .. |  | ..    ...  |
//      | .. ... .. |  |          1 |
//
//   Traverse every row below the diagonal
//   The objective is to have only zeroes on the left of the diagonal
//   and a one on the diagonal. At each step, we will consider a
//   row where the diagonal element a is non-zero:
//
//      | 1  z . .. |
//   i  | 0  a b .. |
//      | 0  ..  .. |
//   j  | 0  c d .. |
//      | 0  ..  .. |
//
//   We first perform r[j] = r[j] * a - r[i] * c, also applying it to
//   the temporary matrix. This gets us to:
//
//      | 1  z . .. |
//   i  | 0  a b .. |
//      | 0  ..  .. |
//   j  | 0  0 x .. |  (where x = ad - bc)
//      | 0  ..  .. |
//
//   When then do another transform r[i] = r[i] / a
//   This gets us to:
//
//      | 1  z . .. |
//   i  | 0  1 y .. |  y = b/a
//      | 0  ..  .. |
//   j  | 0  0 x .. |
//      | 0  ..  .. |
//
//   Finally, for j < i, we perform r[j] = r[j] - z * r[j]
//   This gets us to:
//
//   j  | 1  0 . .. |
//   i  | 0  1 0 .. |
//      | 0  ..  .. |
//      | 0  0 x .. |
//      | 0  ..  .. |
//
//   The last two steps are actually combined into a single one.
//   The same transforms are simulatenously applied to the identity matrix.
//
//   When the process is finished, the identity matrix has become the inverse
//
//   The matrices elements are all pushed to the stack, accessed backwards
//   - pm points to the end of the original matrix
//   - pt points to the end of the temporary area initialized with identity
//   Matrix elements are accessed as rt.stack(p + ~o) where o = r * cols + c
{
    size_t cx, rx;
    size_t depth = rt.depth();
    id     atype = type();

    if (is_matrix(&rx, &cx))
    {
        if (rx != cx)
        {
            rt.dimension_error();
            goto err;
        }

        cleaner     purge;
        size_t      n   = cx;
        size_t      pt  = n * n;    // n temporary elements to save diagonal
        size_t      pm  = 2 * pt;
        bool        neg = false;
        algebraic_g det;
        algebraic_g tot;
        algebraic_g one = integer::make(1);
        algebraic_g zero = integer::make(0);
        algebraic_g aa, ca, mika, mjka;

        // Create an identity matrix of the right size
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < n; j++)
                if (!rt.push(i == j ? +one: +zero))
                    goto err;

        dump_matrix("Inverse of %u x %u matrix", n, n);

        // Loop across the diagonal
        for (size_t i = 0; i < n; i++)
        {
            if (program::interrupted())
                goto err;

            // Find the index of first non-zero element
            bool   zero = true;
            size_t index;

            record(matrix, "Row %u", i);
            for (index = i; zero && index < n; index++)
            {
                size_t   ix  = index * n + i;
                object_p xij = rt.stack(pm + ~ix);
                if (!xij)
                    goto err;
                zero = xij->is_zero(false);
                record(matrix, "Index %u xij=%t %+s",
                       index, xij, zero ? "zero" : "non-zero");
                if (!zero)
                    break;
            }

            // If only zeroes, determinant is zero, matrix is not invertible
            if (zero)
            {
                record(matrix, "Cannot invert matrix with zero determinant");
                rt.drop(rt.depth() - depth);
                rt.zero_divide_error();
                return nullptr;
            }

            // Check if need to swap diagonal element row and index rows
            record(matrix, "Row %u index %u", i, index);
            if (index != i)
            {
                record(matrix, "Swapping %u and %u", index, i);

                // Swap diagonal element row and index row
                for (size_t j = 0; j < n; j++)
                {
                    // Swap rows in source or destination matrix
                    size_t oa = index * n + j;
                    size_t ob = i * n + j;
                    for (uint mat = 0; mat < 2; mat++)
                    {
                        if (program::interrupted())
                            goto err;

                        size_t p = mat ? pt : pm;
                        object_p a = rt.stack(p + ~oa);
                        object_p b = rt.stack(p + ~ob);
                        rt.stack(p + ~oa, b);
                        rt.stack(p + ~ob, a);
                    }

                }

                dump_matrix("After swapping %u and %u", index, i);

                if ((index - i) & 1)
                {
                    neg = !neg;
                    record(matrix, "Determinant is now %+s",
                           neg ? "negative" : "positive");
                }
            }

            // Fetch 'a', which we now know to be non-zero
            size_t   oii = i * n + i;
            object_p a   = rt.stack(pm + ~oii);
            record(matrix, "m[%u,%u]     a=%t", i, i, a);
            if (!a)
                goto err;
            aa = a->as_algebraic();
            if (!aa)
                goto err;

            // Loop below row i to compute r[j] = r[j] * a - r[i] * c
            record(matrix, "Zeroing sub-diagonals below %u", i);
            cleaner purge;
            for (size_t j = i + 1; j < n; j++)
            {
                // Fetch value on diagonal and in next row
                size_t oji = j * n + i;
                object_p c = rt.stack(pm + ~oji);
                record(matrix, "m[%u,%u]     c=%t", j, i, c);
                if (!c)
                    goto err;
                ca = c->as_algebraic();
                if (!ca)
                    goto err;

                // Traverse columns in r[j] for the two matrices
                for (uint mat = 0; mat < 2; mat++)
                {
                    size_t p = mat ? pt : pm;
                    record(matrix, "Matrix %+s row %u", mat ? "t" : "m", j);

                    // We don't bother computing r[j] below i, they are 0
                    for (size_t k = mat ? 0 : i; k < n; k++)
                    {
                        cleaner  purge;
                        size_t   ojk = j * n + k;
                        size_t   oik = i * n + k;
                        object_p mjk = rt.stack(p + ~ojk);
                        object_p mik = rt.stack(p + ~oik);
                        if (!mjk || !mik)
                            goto err;
                        mjka = mjk->as_algebraic();
                        mika = mik->as_algebraic();
                        mjka = aa * mjka - ca * mika;
                        if (!mjka)
                            goto err;
                        mjka = purge(mjka);
                        rt.stack(p + ~ojk, +mjka);
                        record(matrix, "%+s[%u,%u] is now %t",
                               mat ? "t" : "m", j, k, +mjka);
                    }
                    record(matrix, "Matrix %+s row %u done", mat?"t":"m", j);
                }
                dump_matrix("After zeroing sub-diagonal below row %u", j);
            }

            // Transform r[i] = r[i] / a
            record(matrix, "Make diagonal %u unity", i);
            for (uint mat = 0; mat < 2; mat++)
            {
                size_t p = mat ? pt : pm;
                for (size_t k = mat ? 0 : i + 1; k < n; k++)
                {
                    cleaner purge;
                    size_t   oik = i * n + k;
                    object_p mik = rt.stack(p + ~oik);
                    if (!mik)
                        goto err;
                    mika = mik->as_algebraic();
                    if (!mika)
                        goto err;
                    mika = mika / aa;
                    mika = purge(mika);
                    rt.stack(p + ~oik, +mika);
                }
            }
            dump_matrix("After making diagonal of row %u unity", i);

            // For j < i, transform r[j] = r[j] - z * r[i]
            for (uint j = 0; j < i; j++)
            {
                size_t oz = n * j + i;
                object_p z = rt.stack(pm + ~oz);
                if (!z)
                    goto err;
                ca = z->as_algebraic();
                if (!ca)
                    goto err;

                // This is only needed on the right matrix
                for (uint mat = 0; mat < 2; mat++)
                {
                    if (program::interrupted())
                        goto err;

                    size_t p = mat ? pt : pm;
                    for (size_t k = mat ? 0 : i; k < n; k++)
                    {
                        cleaner  purge;
                        size_t   oik = i * n + k;
                        size_t   ojk = j * n + k;
                        object_p mik = rt.stack(p + ~oik);
                        object_p mjk = rt.stack(p + ~ojk);
                        if (!mik || !mjk)
                            goto err;
                        mika = mik->as_algebraic();
                        mjka = mjk->as_algebraic();
                        if (!mika || !mjka)
                            goto err;
                        mjka = mjka  - ca * mika;
                        mjka = purge(mjka);
                        rt.stack(p + ~ojk, +mjka);
                    }
                }
            }
            dump_matrix("After creating zeros for row %u", i);
            record(matrix, "Row %u complete", i);
        }

        // Build result
        scribble sr;
        for (uint r = 0; r < n; r++)
        {
            object_p vec;
            {
                scribble sv;
                for (uint c = 0; c < n; c++)
                {
                    if (program::interrupted())
                        goto err;
                    size_t orc = r * n + c;
                    object_p mrc = rt.stack(pt + ~orc);
                    if (!rt.append(mrc))
                        goto err;
                }
                vec = list::make(atype, sv.scratch(), sv.growth());
            }
            if (!rt.append(vec))
                goto err;
        }

        // Return result
        rt.drop(rt.depth() - depth);
        object_p inv = list::make(atype, sr.scratch(), sr.growth());
        sr.clear();
        inv = purge(inv);
        record(matrix, "Result inv=%t", inv);
        return array_p(inv);
    }
    else if (atype == ID_array)
    {
        // Apply component-wise inversion
        return map(inv::evaluate);
    }
    else
    {
        rt.type_error();
    }
    return nullptr;

err:
    rt.drop(rt.depth() - depth);
    return nullptr;
}


algebraic_p array::norm_square() const
// ----------------------------------------------------------------------------
//   Compute the square of the norm of a matrix or vector
// ----------------------------------------------------------------------------
{
    algebraic_g sum;
    for (object_p obj : *this)
    {
        if (program::interrupted())
            return nullptr;
        id oty = obj->type();
        if (oty == ID_array)
        {
            algebraic_g enorm2 = array_p(obj)->norm_square();
            sum = sum ? sum + enorm2 : enorm2;
        }
        else if (algebraic_g elem = obj->as_algebraic())
        {
            elem = sq::run(elem);
            sum = sum ? sum + elem : elem;
        }
        else
        {
            rt.type_error();
            return nullptr;
        }
    }
    return sum;
}


algebraic_p array::norm() const
// ----------------------------------------------------------------------------
//   Compute the square of the norm of a matrix or vector
// ----------------------------------------------------------------------------
{
    algebraic_g sq = norm_square();
    return sqrt::run(sq);
}


COMMAND_BODY(det)
// ----------------------------------------------------------------------------
//   Implement the 'det' command
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (array_p arr = obj->as<array>())
        {
            if (algebraic_g det = arr->determinant())
                if (rt.top(det))
                    return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


algebraic_p array::dot(array_r &x, array_r &y)
// ----------------------------------------------------------------------------
//   Perform a dot product on the two arrays
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    array::iterator xi = x->begin();
    array::iterator yi = y->begin();
    size_t          count  = 0;
    uint            xangles = 0;
    uint            yangles = 0;
    algebraic_g     xa, ya, sum;

    cleaner         purge;
    while (object_p xo = *xi++)
    {
        if (program::interrupted())
            return nullptr;

        object_p yo = *yi++;
        if (!yo)
        {
            rt.dimension_error();
            return nullptr;
        }
        xo = object::strip(xo);
        yo = object::strip(yo);
        xa = xo->as_extended_algebraic();
        ya = yo->as_extended_algebraic();
        if (!xa || !ya)
        {
            rt.type_error();
            return nullptr;
        }

        // Check if we have a 2D or 3D polar, cylindrical or spherical vector
        if (count < 3)
        {
            if (xa->as<unit>())
                if (algebraic::adjust_angle(xa))
                    xangles |= (1 << count);
            if (ya->as<unit>())
                if (algebraic::adjust_angle(ya))
                    yangles |= (1 << count);
        }
        xa = xa * ya;
        sum = sum ? sum + xa : xa;
        if (!sum)
            return nullptr;
        count++;
    }
    sum = purge(sum);

    object_p yo = *yi++;
    if (yo)
    {
        rt.dimension_error();
        return nullptr;
    }

    // Check if we actually deal with polar or cylindrical or spherical vectors
    if (count == 2 && (xangles == 2 || yangles == 2))
    {
        array_g xx = xangles == 2 ? x->to_rectangular() : +x;
        array_g yx = yangles == 2 ? y->to_rectangular() : +y;
        xa = dot(xx, yx);
        return xa;
    }
    else if (count == 3)
    {
        bool xnonrect = (xangles == 2 || xangles == 6);
        bool ynonrect = (yangles == 2 || yangles == 6);
        if (xnonrect || ynonrect)
        {
            array_g xx = xnonrect ? x->to_rectangular() : +x;
            array_g yy = ynonrect ? y->to_rectangular() : +y;
            xa = dot(xx, yy);
            return xa;
        }
    }

    // Normal rectangular case
    return sum;
}


array_p array::cross(array_r &x, array_r &y)
// ----------------------------------------------------------------------------
//   Perform a cross product on the two arrays
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    array::iterator xi      = x->begin();
    array::iterator yi      = y->begin();
    size_t          count   = 0;
    size_t          vsize   = 0;
    uint            xangles = 0;
    uint            yangles = 0;
    algebraic_g     xa[3], ya[3];
    id              xty = x->type();

    cleaner         purge;
    while (count < 3)
    {
        if (program::interrupted())
            return nullptr;

        object_p xo = *xi++;
        object_p yo = *yi++;
        bool hadx = xo;
        bool hady = yo;
        if (!hadx)
            xo = integer::make(0);
        if (!hady)
            yo = integer::make(0);
        xo = object::strip(xo);
        yo = object::strip(yo);
        xa[count] = xo->as_extended_algebraic();
        ya[count] = yo->as_extended_algebraic();
        if (!xa[count] || !ya[count])
        {
            rt.type_error();
            return nullptr;
        }

        // Check if we have a 2D or 3D polar, cylindrical or spherical vector
        if (hadx)
            if (xa[count]->as<unit>())
                if (algebraic::adjust_angle(xa[count]))
                    xangles |= (1 << count);
        if (hady)
            if (ya[count]->as<unit>())
                if (algebraic::adjust_angle(ya[count]))
                    yangles |= (1 << count);
        count++;
        if (hadx || hady)
            vsize = count;
    }

    object_p xo = *xi++;
    object_p yo = *yi++;
    if (xo || yo)
    {
        rt.dimension_error();
        return nullptr;
    }

    // Check if we actually deal with polar or cylindrical or spherical vectors
    if (vsize == 2 && (xangles == 2 || yangles == 2))
    {
        array_g xx = xangles == 2 ? x->to_rectangular() : +x;
        array_g yy = yangles == 2 ? y->to_rectangular() : +y;
        xx = cross(xx, yy);
        return xx;
    }
    else if (vsize == 3)
    {
        bool xnonrect = (xangles == 2 || xangles == 6);
        bool ynonrect = (yangles == 2 || yangles == 6);
        if (xnonrect || ynonrect)
        {
            array_g xx = xnonrect ? x->to_rectangular() : +x;
            array_g yy = ynonrect ? y->to_rectangular() : +y;
            xx = cross(xx, yy);
            return xx;
        }
    }

    // Normal rectangular case
    algebraic_g r1 = xa[1] * ya[2] - xa[2] * ya[1];
    algebraic_g r2 = xa[2] * ya[0] - xa[0] * ya[2];
    algebraic_g r3 = xa[0] * ya[1] - xa[1] * ya[0];

    array_g result = array_p(list::make(xty, r1, r2, r3));
    result = purge(result);
    return result;
}


COMMAND_BODY(dot)
// ----------------------------------------------------------------------------
//   Implement a dot product
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.stack(1))
    {
        if (object_p y = rt.stack(0))
        {
            if (array_g xa = x->as<array>())
                if (array_g ya = y->as<array>())
                    if (algebraic_g r = array::dot(xa, ya))
                        if (rt.drop() && rt.top(+r))
                            return OK;
            if (algebraic_g xa = x->as_algebraic())
                if (algebraic_g ya = y->as_algebraic())
                    if (xa->is_symbolic() || ya->is_symbolic())
                        if (expression_g e = expression::make(ID_dot, xa, ya))
                            if (rt.drop() && rt.top(+e))
                                return OK;
            if (!rt.error())
                rt.type_error();
        }
    }
    return ERROR;
}


COMMAND_BODY(cross)
// ----------------------------------------------------------------------------
//   Implement a cross product
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.stack(1))
    {
        if (object_p y = rt.stack(0))
        {
            if (array_g xa = x->as<array>())
                if (array_g ya = y->as<array>())
                    if (algebraic_g r = array::cross(xa, ya))
                        if (rt.drop() && rt.top(r))
                            return OK;
            if (algebraic_g xa = x->as_algebraic())
                if (algebraic_g ya = y->as_algebraic())
                    if (xa->is_symbolic() || ya->is_symbolic())
                        if (expression_g e = expression::make(ID_cross, xa, ya))
                            if (rt.drop() && rt.top(+e))
                                return OK;
            if (!rt.error())
                rt.type_error();
        }
    }
    return ERROR;
}


algebraic_p array::one_norm(array_p ao, bool column)
// ----------------------------------------------------------------------------
//   Compute a row or column 1-norm
// ----------------------------------------------------------------------------
{
    size_t  cx, rx;
    array_g a = ao;
    algebraic_g norm, item, sum;
    if (a->is_matrix(&cx, &rx))
    {
        size_t nitems = cx * rx;
        if (column)
        {
            for (size_t c = 0; c < cx; c++)
            {
                for (size_t r = 0; r < rx; r++)
                {
                    size_t i = r * cx + c;
                    item = rt.stack(nitems + ~i)->as_algebraic();
                    item = abs::evaluate(item);
                    sum = sum ? sum + item : item;
                }
                if (!norm || compare(norm, sum) < 0)
                    norm = sum;
                sum = nullptr;
            }
        }
        else
        {
            for (size_t r = 0; r < rx; r++)
            {
                for (size_t c = 0; c < cx; c++)
                {
                    size_t i = r * cx + c;
                    item = rt.stack(nitems + ~i)->as_algebraic();
                    item = abs::evaluate(item);
                    sum = sum ? sum + item : item;
                }
                if (!norm || compare(norm, sum) < 0)
                    norm = sum;
                sum = nullptr;
            }
        }
        rt.drop(rx * cx);
        return norm;
    }
    else if (a->is_vector(&rx))
    {
        // For vectors, sum of the absolute values of vector elements
        for (size_t i = 0; i < rx; i++)
        {
            item = rt.stack(i)->as_algebraic();
            item = abs::evaluate(item);
            if (column)
                norm = norm ? norm + item : item;
            else if (!norm || compare(norm, item) < 0)
                norm = item;
        }
        rt.drop(rx);
        return norm;
    }

    rt.type_error();
    return nullptr;
}


object::result array::one_norm(bool column)
// ----------------------------------------------------------------------------
//   Compute a 1-norm (row or column)
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (array_p a = obj->as<array>())
    {
        if (algebraic_p norm = one_norm(a, column))
            if (rt.top(norm))
                return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(ColumnNorm)
// ----------------------------------------------------------------------------
//   Compute the column norm
// ----------------------------------------------------------------------------
{
    return array::one_norm(true);}


COMMAND_BODY(RowNorm)
// ----------------------------------------------------------------------------
//   Implement a cross product
// ----------------------------------------------------------------------------
{
    return array::one_norm(false);
}


COMMAND_BODY(FromArray)
// ----------------------------------------------------------------------------
//   Array to stack
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (array_p a = obj->as<array>())
    {
        rt.pop();
        if (a->expand())
            return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(ToArray)
// ----------------------------------------------------------------------------
//   Stack to array
// ----------------------------------------------------------------------------
{
    size_t rows = 0, columns = 0;
    if (array::size_from_stack(&rows, &columns))
    {
        size_t items = columns ? rows * columns : rows;
        if (rt.args(items+1))
        {
            rt.drop();
            if (array_p a = array::from_stack(rows, columns))
                if (rt.push(a))
                    return OK;
        }
    }

    return ERROR;
}


COMMAND_BODY(MatrixToRows)
// ----------------------------------------------------------------------------
//   Convert matrix to row vectors
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_g a = obj->as_array_or_list())
    {
        size_t rows  = 0;
        if (rt.pop())
        {
            for (object_p row : *a)
            {
                if (!rt.push(row))
                    goto error;
                rows++;
            }
            integer_p robj = integer::make(rows);
            if (robj && rt.push(+robj))
                return OK;
        }
    error:
        rt.drop(rows);
        rt.push(+a);
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(RowsToMatrix)
// ----------------------------------------------------------------------------
//   Transform individual rows to a matrix
// ----------------------------------------------------------------------------
{
    size_t rows = 0, columns = 0;
    if (array::size_from_stack(&rows, &columns))
    {
        if (columns)
        {
            rt.type_error();
            return ERROR;
        }

        scribble scr;
        for (size_t row = 0; row < rows; row++)
        {
            object_p obj = rt.stack(rows - row);
            if (!obj || !obj->is_extended_algebraic())
                goto type_error;
            if (!rt.append(obj))
                goto error;
        }
        list_p result = list::make(ID_array, scr.scratch(), scr.growth());
        if (result && rt.drop(rows) && rt.top(result))
            return OK;
    }
    else
    {
    type_error:
        rt.type_error();
    }
error:
    return ERROR;
}


COMMAND_BODY(MatrixToColumns)
// ----------------------------------------------------------------------------
//   Convert matrix to column vectors
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (list_g a = obj->as_array_or_list())
    {
        size_t rows  = 0, cols = 0;
        if (rt.pop())
        {
            id ty = a->type();
            for (object_p row : *a)
            {
                if (list_p l = row->as_array_or_list())
                    cols = std::max(cols, l->items());
                if (!rt.push(row))
                    goto error;
                rows++;
            }
            if (cols == 0)
            {
                // Vector case
                integer_g robj = integer::make(rows);
                if (robj && rt.push(+robj))
                    return OK;
            }
            for (size_t c = 0; c < cols; c++)
            {
                scribble scr;
                for (size_t r = 0; r < rows; r++)
                {
                    object_p row = rt.stack(c + rows + ~r);
                    if (list_p lrow = row->as_array_or_list())
                    {
                        if (object_p lcol = lrow->at(c))
                            row = lcol;
                        else
                            row = integer::make(0);
                    }
                    if (!row || !rt.append(row))
                    {
                        rows += c;
                        goto error;
                    }
                }
                list_p column = list::make(ty, scr.scratch(), scr.growth());
                if (!column || !rt.push(column))
                {
                    rows += c;
                    goto error;
                }
            }
            for (size_t c = 0; c < cols; c++)
            {
                object_p col = rt.stack(cols + ~c);
                rt.stack(cols + rows + ~c, col);
            }
            rt.drop(rows);
            integer_p cobj = integer::make(cols);
            if (cobj && rt.push(+cobj))
                return OK;
            rows = cols;
        }
    error:
        rt.drop(rows);
        rt.push(+a);
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(ColumnsToMatrix)
// ----------------------------------------------------------------------------
//   Transform individual columns to a matrix
// ----------------------------------------------------------------------------
{
    size_t rows = 0, columns = 0;
    if (array::size_from_stack(&columns, &rows))
    {
        if (rows)
        {
            rt.type_error();
            return ERROR;
        }

        rows = 0;
        for (size_t col = 0; col < columns; col++)
        {
            object_p obj = rt.stack(columns - col);
            if (list_p lobj = obj->as_array_or_list())
                rows = std::max(rows, lobj->items());
        }
        list_p result = nullptr;
        if (!rows)
        {
            rt.drop();
            result = array::from_stack(columns, 0);
            if (result && rt.push(result))
                return OK;
        }
        else
        {
            scribble scr;
            for (size_t row = 0; row < rows; row++)
            {
                list_g robj;
                {
                    scribble rscr;
                    for (size_t col = 0; col < columns; col++)
                    {
                        object_p obj = rt.stack(columns - col);
                        if (list_p lobj = obj->as_array_or_list())
                        {
                            if (object_p lrow = lobj->at(row))
                                obj = lrow;
                            else
                                obj = integer::make(0);
                        }
                        if (!obj || !rt.append(obj))
                        {
                            rt.drop(columns);
                            return ERROR;
                        }
                    }
                    robj = list::make(ID_array, rscr.scratch(), rscr.growth());
                }
                if (!robj || !rt.append(robj))
                {
                    rt.drop(columns);
                    return ERROR;
                }
            }
            result = list::make(ID_array, scr.scratch(), scr.growth());
            if (result && rt.drop(columns) && rt.top(result))
                return OK;
        }

    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


object::result array::add_row_or_column(bool columnist)
// ----------------------------------------------------------------------------
//   Shared code to add rows or columns (ROW+, COL+)
// ----------------------------------------------------------------------------
{
    // First argument must give the index position
    size_t row = 0, column = 0;
    if (!array::size_from_stack(&row, &column) || column)
    {
    type_error:
        rt.type_error();
        return ERROR;
    }
    row--;

    // Get outer and inner object
    object_g inner = rt.stack(1);
    object_p outer = rt.stack(2);

    // The outer always needs to be an array or vector
    list_g ol = outer->as_array_or_list();
    list_g il = inner->as_array_or_list();
    if (!ol)
        goto type_error;

    // The logic for reproducing HP50G's error is more complicated than
    // the rest of this functoin, which is weird
    if (Settings.StrictArrayResizing())
    {
        array_p oa = outer->as<array>();
        array_p ia = inner->as<array>();
        if (!oa)
            goto type_error;

        size_t orows = 0, ocols = 0;
        if (!oa->is_matrix_or_vector(&orows, &ocols, false))
            goto dimension_error;

        if (ocols)
        {
            size_t irows = 0, icols = 0;
            if (!ia)
                goto type_error;
            if (!ia->is_matrix_or_vector(&irows, &icols, false))
                goto dimension_error;
            if (columnist ? (orows != irows) : (ocols != icols))
                goto dimension_error;
            if (row > (columnist ? ocols : orows))
                goto value_error;
        }
        else if (row > orows)
        {
            goto value_error;
        }
        else
        {
            // Single value insert: works the same for rows or columns
            il = il ? ol->insert(il, row) : ol->insert(inner, row);
            if (il && rt.drop(2) && rt.top(il))
                return OK;
            return ERROR;
        }
    }

    // Insert row object
    if (columnist)
    {
        if (il)
        {
            list::iterator oi = ol->begin();
            list::iterator ii = il->begin();
            list::iterator oe = ol->end();
            list::iterator ie = il->end();
            id             ty = ol->type();
            scribble       scr;
            while (oi != oe && ii != ie)
            {
                object_p orow = *oi;
                object_p irow = *ii;
                list_p orl = orow->as_array_or_list();
                if (!orl)
                    goto dimension_error;
                list_p irl = irow->as_array_or_list();
                orl = irl ? orl->insert(irl, row) : orl->insert(irow, row);
                if (!orl || !rt.append(orl))
                    return ERROR;
                ++oi;
                ++ii;
            }
            il = list::make(ty, scr.scratch(), scr.growth());
        }
        else
        {
            il = ol->insert(inner, row);
        }
    }
    else
    {
        // Row case: simply build the outer array
        il = il ? ol->insert(il, row) : ol->insert(inner, row);
    }

    // Put result on stack
    if (il && rt.drop(2) && rt.top(il))
        return OK;

    // Return whatever error failed above
    return ERROR;
dimension_error:
    rt.dimension_error();
    return ERROR;
value_error:
    rt.value_error();
    return ERROR;
}


object::result array::delete_row_or_column(bool columnist)
// ----------------------------------------------------------------------------
//   Shared code to delete rows or columns (ROW-, COL-)
// ----------------------------------------------------------------------------
{
    // First argument must give the index position
    size_t row = 0, count = 1;
    if (!array::size_from_stack(&row, &count))
    {
    type_error:
        rt.type_error();
        return ERROR;
    }
    if (!count)
        count = 1;
    row--;

    // Get object we want to remove rows / columns from
    object_p outer = rt.stack(1);

    // The input always needs to be an array, list or vector
    list_g ol = outer->as_array_or_list();
    if (!ol)
        goto type_error;

    // Check if we want strict checking that we got rectangular arrays
    if (Settings.StrictArrayResizing())
    {
        array_p oa = outer->as<array>();
        if (!oa)
            goto type_error;

        size_t orows = 0, ocols = 0;
        if (!oa->is_matrix_or_vector(&orows, &ocols, false))
            goto dimension_error;

        if (ocols)
        {
            if (row > (columnist ? ocols : orows))
                goto value_error;
        }
        else if (row > orows)
        {
            goto value_error;
        }
        else
        {
            // Delete from a vector
            ol = ol->remove(row, count);
            if (ol && rt.drop() && rt.top(ol))
                return OK;
            return ERROR;
        }
    }

    // Remove row object
    if (columnist)
    {
        id       ty = ol->type();
        scribble scr;
        for (object_p orow : *ol)
        {
            list_p orl = orow->as_array_or_list();
            if (!orl)
                goto dimension_error;
            orl = orl->remove(row, count);
            if (!orl || !rt.append(orl))
                return ERROR;
        }
        ol = list::make(ty, scr.scratch(), scr.growth());
    }
    else
    {
        ol = ol->remove(row, count);
    }

    // Put result on stack
    if (ol && rt.drop() && rt.top(ol))
        return OK;

    // Return whatever error failed above
    return ERROR;
dimension_error:
    rt.dimension_error();
    return ERROR;
value_error:
    rt.value_error();
    return ERROR;
}


object::result array::swap_row_or_column(bool columnist)
// ----------------------------------------------------------------------------
//   Shared code to swap rows or columns (CSWP, RSWP)
// ----------------------------------------------------------------------------
{
    uint i = rt.stack(0)->as_uint32(1, true);
    uint j = rt.stack(1)->as_uint32(1, true);
    if (rt.error())
        return ERROR;
    list_p lst = rt.stack(2)->as_array_or_list();
    if (!lst)
    {
        rt.type_error();
        return ERROR;
    }
    i--;
    j--;
    if (columnist)
    {
        scribble scr;
        id       ty = lst->type();
        bool     vec = true;
        for (object_p row : *lst)
        {
            if (list_p rlst = row->as_array_or_list())
            {
                vec = false;
                row = rlst->swap(i, j);
                if (!row)
                {
                    if (!rt.error())
                        rt.value_error();
                    return ERROR;
                }
            }
            if (!rt.append(row))
                return ERROR;
        }
        if (vec)
            lst = lst->swap(i, j);
        else
            lst = list::make(ty, scr.scratch(), scr.growth());
    }
    else
    {
        lst = lst->swap(i, j);
    }
    if (lst && rt.drop(2) && rt.top(lst))
        return OK;
    if (!rt.error())
        rt.value_error();
    return ERROR;
}


COMMAND_BODY(AddRow)
// ----------------------------------------------------------------------------
//   Add a row in a matrix and returned combined matrix
// ----------------------------------------------------------------------------
{
    return array::add_row_or_column(false);
}


COMMAND_BODY(AddColumn)
// ----------------------------------------------------------------------------
//   Add a column in a matrix and returned combined matrix
// ----------------------------------------------------------------------------
{
    return array::add_row_or_column(true);
}


COMMAND_BODY(DeleteRow)
// ----------------------------------------------------------------------------
//   Add a row in a matrix and returned combined matrix and element
// ----------------------------------------------------------------------------
{
    return array::delete_row_or_column(false);
}


COMMAND_BODY(DeleteColumn)
// ----------------------------------------------------------------------------
//   Delete a column in a matrix and returned reduced matrix
// ----------------------------------------------------------------------------
{
    return array::delete_row_or_column(true);
}


COMMAND_BODY(RowSwap)
// ----------------------------------------------------------------------------
//   Swap two rows in an array
// ----------------------------------------------------------------------------
{
    return array::swap_row_or_column(false);
}


COMMAND_BODY(ColumnSwap)
// ----------------------------------------------------------------------------
//   Swap two columns in an array
// ----------------------------------------------------------------------------
{
    return array::swap_row_or_column(true);
}


static object_p item_from_constant(size_t, size_t,
                                   size_t, size_t, void *data)
// ----------------------------------------------------------------------------
//   Return an item from a constant
// ----------------------------------------------------------------------------
{
    object_g *ptr = (object_g *) data;
    return *ptr;
}


COMMAND_BODY(ConstantArray)
// ----------------------------------------------------------------------------
//   Build a constant array
// ----------------------------------------------------------------------------
{
    if (object_p value = rt.stack(0))
    {
        if (object_g dims = rt.stack(1))
        {
            symbol_g name = dims->as_quoted<symbol>();
            if (name)
            {
                dims = directory::recall_all(name, true);
                if (!dims)
                    return ERROR;
            }

            size_t  rows = 0, columns = 0;
            array_g da = dims->as<array>();
            bool is_array = da && (da->is_matrix(&rows, &columns, false) ||
                                   da->is_vector(&rows, false));
            if (is_array || array::size_from_object(&rows, &columns, dims))
            {
                if (array_g a = array::build(rows, columns,
                                             item_from_constant, &value))
                {
                    if (rt.drop(2))
                    {
                        if (name)
                        {
                            if (directory::store_here(name, a))
                                return OK;
                        }
                        else if (rt.push(+a))
                        {
                            return OK;
                        }
                    }
                }
            }
        }
    }
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


struct random_matrix_range
// ----------------------------------------------------------------------------
//    Random number range
// ----------------------------------------------------------------------------
{
    algebraic_g min;
    algebraic_g max;
};


static object_p item_from_random(size_t, size_t, size_t, size_t, void *rng)
// ----------------------------------------------------------------------------
//   Return a random value between -9 and 9
// ----------------------------------------------------------------------------
{
    random_matrix_range *range = (random_matrix_range *) rng;
    object_g value = random_number(range->min, range->max);
    return value;
}


COMMAND_BODY(RandomMatrix)
// ----------------------------------------------------------------------------
//   Build a random matrix
// ----------------------------------------------------------------------------
{
    if (object_g dims = rt.top())
    {
        symbol_g name = dims->as_quoted<symbol>();
        if (name)
        {
            dims = directory::recall_all(name, true);
            if (!dims)
                return ERROR;
        }

        size_t  rows = 0, columns = 0;
        array_g da = dims->as<array>();
        bool is_array = da && (da->is_matrix(&rows, &columns, false) ||
                               da->is_vector(&rows, false));
        if (is_array || array::size_from_object(&rows, &columns, dims))
        {
            random_matrix_range range =
            {
                .min = integer::make(-9),
                .max = integer::make(9)
            };

            if (array_g a = array::build(rows, columns,
                                         item_from_random, &range))
            {
                if (rt.drop())
                {
                    if (name)
                    {
                        if (directory::store_here(name, a))
                            return OK;
                    }
                    else if (rt.push(+a))
                    {
                        return OK;
                    }
                }
            }
        }
    }
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


static object_p item_from_identity(size_t, size_t,
                                   size_t r, size_t c, void *)
// ----------------------------------------------------------------------------
//   Return an item from a constant
// ----------------------------------------------------------------------------
{
    return integer::make(uint(r == c));
}


COMMAND_BODY(IdentityMatrix)
// ----------------------------------------------------------------------------
//   Build an identity matrix
// ----------------------------------------------------------------------------
{
    if (object_g dims = rt.top())
    {
        symbol_g name = dims->as_quoted<symbol>();
        if (name)
        {
            dims = directory::recall_all(name, true);
            if (!dims)
                return ERROR;
        }

        size_t  rows = 0, columns = 0;
        array_g da = dims->as<array>();
        if (da)
        {
            if (da->is_matrix(&rows, &columns, false))
            {
            }
            else if (da->is_vector(&rows, false))
            {
                columns = rows;
            }
            else
            {
                columns = 1;
            }
        }
        else if (array::size_from_object(&rows, &columns, dims))
        {
            if (!columns)
                columns = rows;
        }
        else
        {
            rt.type_error();
            return ERROR;
        }
        if (rows != columns)
        {
            rt.dimension_error();
            return ERROR;
        }
        if (array_g a = array::build(rows, columns, item_from_identity))
        {
            if (rt.drop())
            {
                if (name)
                {
                    if (directory::store_here(name, a))
                        return OK;
                }
                else if (rt.push(+a))
                {
                    return OK;
                }
                }
        }
    }
    if (!rt.error())
        rt.type_error();
    return ERROR;
}



// ============================================================================
//
//   Conversion between coordinate kinds
//
// ============================================================================

array_p array::to_rectangular() const
// ----------------------------------------------------------------------------
//   Convert 2D or 3D vector to rectangular form
// ----------------------------------------------------------------------------
{
    array_g v = this;
    if (id mode = v->is_2Dor3D(false))
    {
        switch (mode)
        {
        case ID_To2DVector:
        case ID_To3DVector:
            // Already in rectangular form
            return v;

        case ID_ToPolar:
        {
            algebraic_g r = algebraic_p(v->at(0));
            algebraic_g a = algebraic_p(v->at(1));
            algebraic_g x = r * cos::evaluate(a);
            algebraic_g y = r * sin::evaluate(a);
            return array_p(list::make(ID_array, x, y));
        }

        case ID_ToCylindrical:
        {
            algebraic_g r = algebraic_p(v->at(0));
            algebraic_g a = algebraic_p(v->at(1));
            algebraic_g h = algebraic_p(v->at(2));
            algebraic_g x = r * cos::evaluate(a);
            algebraic_g y = r * sin::evaluate(a);
            algebraic_g z = h;
            return array_p(list::make(ID_array, x, y, z));
        }

        case ID_ToSpherical:
        {
            algebraic_g r     = algebraic_p(v->at(0));
            algebraic_g theta = algebraic_p(v->at(1));
            algebraic_g phi   = algebraic_p(v->at(2));
            algebraic_g z     = r * cos::evaluate(phi);
            phi               = sin::evaluate(phi);
            algebraic_g x     = r * cos::evaluate(theta) * phi;
            algebraic_g y     = r * sin::evaluate(theta) * phi;
            return array_p(list::make(ID_array, x, y, z));
        }
        default:
            rt.type_error();
        }
    }
    return nullptr;
}


array_p array::to_polar() const
// ----------------------------------------------------------------------------
//   Convert 2D or 3D vector to rectangular form
// ----------------------------------------------------------------------------
{
    array_g v = this;
    if (id mode = v->is_2Dor3D(false))
    {
        switch (mode)
        {
        case ID_To2DVector:
        {
            algebraic_g x = algebraic_p(v->at(0));
            algebraic_g y = algebraic_p(v->at(1));
            algebraic_g r = hypot::evaluate(x, y);
            algebraic_g a = atan2::evaluate(y, x);
            return array_p(list::make(ID_array, r, a));
        }

        case ID_To3DVector:
        {
            // Convert to cylindrical
            algebraic_g x = algebraic_p(v->at(0));
            algebraic_g y = algebraic_p(v->at(1));
            algebraic_g z = algebraic_p(v->at(2));
            algebraic_g r = hypot::evaluate(x, y);
            algebraic_g a = atan2::evaluate(y, x);
            return array_p(list::make(ID_array, r, a, z));
        }

        case ID_ToCylindrical:
        case ID_ToPolar:
            return v;

        case ID_ToSpherical:
        {
            algebraic_g r     = algebraic_p(v->at(0));
            algebraic_g theta = algebraic_p(v->at(1));
            algebraic_g phi   = algebraic_p(v->at(2));
            algebraic_g z = r * cos::evaluate(phi);
            r = r * sin::evaluate(phi);
            return array_p(list::make(ID_array, r, theta, z));
        }
        default:
            rt.type_error();
        }
    }
    return nullptr;
}


array_p array::to_cylindrical() const
// ----------------------------------------------------------------------------
//   Convert 3D vector to cylindrical form
// ----------------------------------------------------------------------------
{
    array_g v = this;
    if (id mode = v->is_2Dor3D(false))
    {
        switch (mode)
        {
        case ID_ToCylindrical:
            return v;

        case ID_To3DVector:
        {
            // Convert to cylindrical
            algebraic_g x = algebraic_p(v->at(0));
            algebraic_g y = algebraic_p(v->at(1));
            algebraic_g z = algebraic_p(v->at(2));
            algebraic_g r = hypot::evaluate(x, y);
            algebraic_g a = atan2::evaluate(y, x);
            return array_p(list::make(ID_array, r, a, z));
        }

        case ID_ToSpherical:
        {
            algebraic_g r     = algebraic_p(v->at(0));
            algebraic_g theta = algebraic_p(v->at(1));
            algebraic_g phi   = algebraic_p(v->at(2));
            algebraic_g z = r * cos::evaluate(phi);
            r = r * sin::evaluate(phi);
            return array_p(list::make(ID_array, r, theta, z));
        }
        default:
            rt.type_error();
        }
    }
    return nullptr;
}


array_p array::to_spherical() const
// ----------------------------------------------------------------------------
//   Convert 3D vector to cylindrical form
// ----------------------------------------------------------------------------
{
    array_g v = this;
    if (id mode = v->is_2Dor3D(false))
    {
        switch (mode)
        {
        case ID_ToSpherical:
            return v;

        case ID_To3DVector:
        {
            // Convert rectangular to cylindrical
            algebraic_g x = algebraic_p(v->at(0));
            algebraic_g y = algebraic_p(v->at(1));
            algebraic_g z = algebraic_p(v->at(2));
            algebraic_g rho2 = x*x + y*y;
            algebraic_g r = sqrt::evaluate(rho2 + z*z);
            algebraic_g theta = acos::evaluate(z / r);
            algebraic_g phi = acos::evaluate(x / sqrt::evaluate(rho2));
            if (y->is_negative(false))
                phi = -phi;
            return array_p(list::make(ID_array, r, phi, theta));
        }

        case ID_ToCylindrical:
        {
            algebraic_g r = algebraic_p(v->at(0));
            algebraic_g theta = algebraic_p(v->at(1));
            algebraic_g z = algebraic_p(v->at(2));
            algebraic_g phi = atan2::evaluate(r, z);
            r = sqrt::evaluate(r*r + z*z);
            return array_p(list::make(ID_array, r, theta, phi));
        }
        default:
            rt.type_error();
        }
    }
    return nullptr;
}


COMMAND_BODY(ToCylindrical)
// ----------------------------------------------------------------------------
//   Convert vector to cylindrical coordinates
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (array_p v = obj->as<array>())
        {
            if (array_p c = v->to_cylindrical())
                if (rt.top(c))
                    return OK;
        }
        if (!rt.error())
            rt.type_error();
    }
    return ERROR;
}



COMMAND_BODY(ToSpherical)
// ----------------------------------------------------------------------------
//   Convert vector to spherical coordinates
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (array_p v = obj->as<array>())
        {
            if (array_p c = v->to_spherical())
                if (rt.top(c))
                    return OK;
        }
        if (!rt.error())
            rt.type_error();
    }
    return ERROR;
}


COMMAND_BODY(To2DVector)
// ----------------------------------------------------------------------------
//  Convert two coordinates to a 2D vector
// ----------------------------------------------------------------------------
{
    algebraic_g x = rt.stack(1)->as_algebraic();
    algebraic_g y = rt.stack(0)->as_algebraic();
    if (x && y)
    {
        if (array_p a = array_p(list::make(ID_array, x, y)))
            if (rt.drop() && rt.top(a))
                return OK;
    }
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


COMMAND_BODY(To3DVector)
// ----------------------------------------------------------------------------
//   Convert three coordinates ot a 3D vector
// ----------------------------------------------------------------------------
{
    algebraic_g x = rt.stack(2)->as_algebraic();
    algebraic_g y = rt.stack(1)->as_algebraic();
    algebraic_g z = rt.stack(0)->as_algebraic();
    if (x && y && z)
    {
        if (array_p a = array_p(list::make(ID_array, x, y, z)))
            if (rt.drop(2) && rt.top(a))
                return OK;
    }
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


COMMAND_BODY(FromVector)
// ----------------------------------------------------------------------------
//   Convert a vector to its stack representation
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.pop())
    {
        if (array_g v = obj->as<array>())
        {
            if (v->is_2Dor3D(true))
                return OK;
            if (v->is_vector(nullptr, true, false))
                return OK;
        }
        rt.push(obj);
        rt.type_error();
    }
    return ERROR;
}


array_p array::transpose() const
// ----------------------------------------------------------------------------
//   Transpose an array without conjugating elements
// ----------------------------------------------------------------------------
{
    array_g a    = this;
    size_t  rows = 0, columns = 0;
    if (a->is_matrix(&rows, &columns, true))
        return array::from_stack(rows, columns, true);
    return nullptr;
}


static object::result transpose(bool conjugate)
// ----------------------------------------------------------------------------
//   Transpose with or without conjugate
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.top())
    {
        if (array_p a = obj->as<array>())
        {
            a = a->transpose();
            if (a && rt.top(a))
                return conjugate ? conj::evaluate() : object::OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return object::ERROR;
}


COMMAND_BODY(Transpose)
// ----------------------------------------------------------------------------
//   Convert an array to its transposed version
// ----------------------------------------------------------------------------
{
    return transpose(false);
}


COMMAND_BODY(TransConjugate)
// ----------------------------------------------------------------------------
//   Convert an array to its transposed / conjugate version
// ----------------------------------------------------------------------------
{
    return transpose(true);
}



// ============================================================================
//
//    Arithmetic operations
//
// ============================================================================

array_p array::add_sub(array_r x, array_r y, bool sub)
// ----------------------------------------------------------------------------
//   Perform an addition or a subtraction of two vectors
// ----------------------------------------------------------------------------
{
    if (!x || !y)
        return nullptr;

    stack_depth_restore sdr;
    array::iterator xi = x->begin();
    array::iterator yi = y->begin();
    size_t          count  = 0;
    uint            xangles = 0;
    uint            yangles = 0;
    algebraic_g     xa, ya;
    id              xty = x->type();

    cleaner         purge;
    while (object_p xo = *xi++)
    {
        if (program::interrupted())
            return nullptr;

        object_p yo = *yi++;
        if (!yo)
        {
            rt.dimension_error();
            return nullptr;
        }
        xo = object::strip(xo);
        yo = object::strip(yo);
        xa = xo->as_extended_algebraic();
        ya = yo->as_extended_algebraic();
        if (!xa || !ya)
        {
            rt.type_error();
            return nullptr;
        }

        // Check if we have a 2D or 3D polar, cylindrical or spherical vector
        if (count < 3)
        {
            if (algebraic_g xu = xa->as<unit>())
                if (algebraic::adjust_angle(xu))
                    xangles |= (1 << count);
            if (algebraic_g yu = ya->as<unit>())
                if (algebraic::adjust_angle(yu))
                    yangles |= (1 << count);
        }
        count++;
    }

    object_p yo = *yi++;
    if (yo)
    {
        rt.dimension_error();
        return nullptr;
    }

    // Check if we actually deal with polar or cylindrical or spherical vectors
    if (count == 2 && (xangles == 2 || yangles == 2))
    {
        array_g xx = xangles == 2 ? x->to_rectangular() : +x;
        array_g yy = yangles == 2 ? y->to_rectangular() : +y;
        xx = add_sub(xx, yy, sub);
        if (yangles == 2)
            xx = xx->to_polar();
        return xx;
    }
    else if (count == 3)
    {
        bool xnonrect = (xangles == 2 || xangles == 6);
        bool ynonrect = (yangles == 2 || yangles == 6);
        if (xnonrect || ynonrect)
        {
            array_g xx = xnonrect ? x->to_rectangular() : +x;
            array_g yy = ynonrect ? y->to_rectangular() : +y;
            xx = add_sub(xx, yy, sub);
            if (yangles == 2)
                xx = xx->to_cylindrical();
            else if (yangles == 6)
                xx = xx->to_spherical();
            return xx;
        }
    }

    // Non-cylindrical / rectangular case
    xi = x->begin();
    yi = y->begin();
    for (uint i = 0; i < count; i++)
    {
        if (program::interrupted())
            return nullptr;
        xa = algebraic_p(object::strip(*xi++));
        ya = algebraic_p(object::strip(*yi++));
        xa = sub ? xa - ya : xa + ya;
        if (!xa || !rt.push(+xa))
            return nullptr;
    }

    // Normal rectangular case
    array_p result = array_p(list::list_from_stack(count, xty));
    result = purge(result);
    return result;
}


array_p array::mul(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Multiply two arrays
// ----------------------------------------------------------------------------
//  [[a b][c d]] * [x y]        = [a*x+b*y c*x+d*y]
//  [[a b][c d]] * [[e f][g h]] = [[a*e+b*g a*f+b*h][c*e+d*g c*f+d*h]]
//  [a*[e f] + b*[g h] c*[e f]+d*[g h]]
//  [[a*e a*f]+[b*g b*h] [c*e c*f]+[d*g d*h]]
//  [[a*e+b*g a*f+b*h][c*e+d*g c*f+d*h]]
//
{
    if (!x || !y)
        return nullptr;

    stack_depth_restore sdr;
    array::iterator xi = x->begin();
    array::iterator yi = y->begin();
    size_t          count  = 0;
    uint            xangles = 0;
    uint            yangles = 0;
    id              xty = x->type();
    algebraic_g     xa, ya;
    array_g         xrow;
    enum { UNKNOWN, MATRIX, VECTOR } matvec = UNKNOWN;

    cleaner         purge;
    while (object_p xo = *xi++)
    {
        if (program::interrupted())
            return nullptr;

        cleaner purge;
        xo = object::strip(xo);
        xa = xo->as_extended_algebraic();
        if (!xa)
        {
            rt.type_error();
            return nullptr;
        }

        xrow = xa->as<array>();
        if (xrow)
        {
            if (matvec == VECTOR)
            {
                rt.dimension_error();
                return nullptr;
            }

            // Need a dot product
            xa = array::dot(xrow, y);
            matvec = MATRIX;
        }
        else
        {
            object_p yo = *yi++;
            if (!yo || matvec == MATRIX)
            {
                rt.dimension_error();
                return nullptr;
            }
            yo = object::strip(yo);
            ya = yo->as_extended_algebraic();
            if (!ya)
            {
                rt.type_error();
                return nullptr;
            }

            // Check if 2D or 3D polar, cylindrical or spherical vector
            if (count < 3)
            {
                if (algebraic_g xu = xa->as<unit>())
                    if (algebraic::adjust_angle(xu))
                        xangles |= (1 << count);
                if (algebraic_g yu = ya->as<unit>())
                    if (algebraic::adjust_angle(yu))
                        yangles |= (1 << count);
            }
            xa = xa * ya;
            matvec = VECTOR;
        }
        xa = purge(xa);
        if (!xa || !rt.push(+xa))
            return nullptr;
        count++;
    }

    if (matvec != MATRIX)
    {
        object_p yo = *yi++;
        if (yo)
        {
            rt.dimension_error();
            return nullptr;
        }
    }

    // Check if we actually deal with polar or cylindrical or spherical vectors
    if (count == 2 && (xangles == 2 || yangles == 2))
    {
        rt.drop(count);
        array_g xx = xangles == 2 ? x->to_rectangular() : +x;
        array_g yy = yangles == 2 ? y->to_rectangular() : +y;
        xx = mul(xx, yy);
        if (yangles == 2)
            xx = xx->to_polar();
        return xx;
    }
    else if (count == 3)
    {
        bool xnonrect = (xangles == 2 || xangles == 6);
        bool ynonrect = (yangles == 2 || yangles == 6);
        if (xnonrect || ynonrect)
        {
            rt.drop(count);
            array_g xx = xnonrect ? x->to_rectangular() : +x;
            array_g yy = ynonrect ? y->to_rectangular() : +y;
            xx = mul(xx, yy);
            if (yangles == 2)
                xx = xx->to_cylindrical();
            else if (yangles == 6)
                xx = xx->to_spherical();
            return xx;
        }
    }

    // Normal rectangular case
    array_p result = array_p(list::list_from_stack(count, xty));
    result = purge(result);
    return result;
}




array_p operator+(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Add two arrays: do element-wise addition
// ----------------------------------------------------------------------------
//   This also works when adding matrices as vectors of vectors
//   As a result, this works with non-rectangular arrays, or [[[1]]] + [[[2]]]
{
    return array::add_sub(x, y, false);
}


array_p operator-(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Subtract two arrays: do element-wise subtraction
// ----------------------------------------------------------------------------
{
    return array::add_sub(x, y, true);
}


array_p operator*(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Multiply two arrays
// ----------------------------------------------------------------------------
//  [[a b][c d]] * [x y]        = [a*x+b*y c*x+d*y]
//  [[a b][c d]] * [[e f][g h]] = [[a*e+b*g a*f+b*h][c*e+d*g c*f+d*h]]
//  [a*[e f] + b*[g h] c*[e f]+d*[g h]]
//  [[a*e a*f]+[b*g b*h] [c*e c*f]+[d*g d*h]]
//  [[a*e+b*g a*f+b*h][c*e+d*g c*f+d*h]]
//
{
    return array::mul(x, y);
}

array_p operator/(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Divide two arrays
// ----------------------------------------------------------------------------
{
    array_g yi = y->invert();
    if (!yi)
        return nullptr;
    return array::mul(yi, x);
}
