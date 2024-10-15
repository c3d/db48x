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
#include "expression.h"
#include "functions.h"
#include "grob.h"
#include "stats.h"
#include "tag.h"
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

bool array::is_vector(size_t *size, bool push, bool strip) const
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
            obj = tag::strip(obj);

            id oty = obj->type();
            if (is_array_or_list(oty))
                result = false;
            else if (!obj->is_algebraic())
                result = false;
            else if (push && !rt.push(strip ? obj : tagged))
                result = false;
            if (!result)
                break;
            count++;
        }
        if (!result)
            rt.drop(push ? count : 0);
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
            obj = tag::strip(obj);

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
                 : count == 3 ? (angles == 0 || angles == 7   ? ID_To3DVector
                                 : angles == 2 || angles == 4 ? ID_ToCylindrical
                                 : angles == 6                ? ID_ToSpherical
                                                              : ID_object)
                              : ID_object);
        }

        if (!r)
            rt.drop(push ? count : 0);
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


array_p array::from_stack(size_t rows, size_t columns)
// ----------------------------------------------------------------------------
//   Build an array from items on the stack
// ----------------------------------------------------------------------------
{
    array_p result = build(rows, columns, item_from_stack);
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
                        if (!rt.append(it))
                            return nullptr;
                    }
                    row = list::make(ID_array, srow.scratch(), srow.growth());
                }
                if (!rt.append(row))
                    return nullptr;
            }
        }
        else
        {
            for (size_t r = 0; r < rows; r++)
            {
                object_g it = items(rows, columns, r, 0, data);
                if (!rt.append(it))
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


static bool add_sub_dimension(size_t rx, size_t cx,
                              size_t ry, size_t cy,
                              size_t *rr, size_t *cr)
// ----------------------------------------------------------------------------
//   For addition and subtraction, we need identical dimensions
// ----------------------------------------------------------------------------
{
    *rr = cx;
    *cr = rx;
    return cx == cy && rx == ry;
}


static algebraic_p matrix_op(object::id op,
                             size_t r, size_t c,
                             size_t rx, size_t cx,
                             size_t ry, size_t cy)
// ----------------------------------------------------------------------------
//   Perform a matrix component-wise operation
// ----------------------------------------------------------------------------
{
    size_t py = cx*rx;
    size_t px = py + cy*ry;
    size_t i = r * cx + c;
    object_p x = rt.stack(px + ~i);
    object_p y = rt.stack(py + ~i);
    if (!x || !y)
        return nullptr;
    algebraic_g xa = x->as_algebraic();
    algebraic_g ya = y->as_algebraic();
    if (!xa || !ya)
    {
        rt.type_error();
        return nullptr;
    }
    switch(op)
    {
    case object::ID_add: return xa + ya;
    case object::ID_sub: return xa - ya;
    case object::ID_mul: return xa * ya;
    case object::ID_div: return xa / ya;
    default:             rt.type_error(); return nullptr;
    }
}


static algebraic_p vector_op(object::id op, size_t c, size_t cx, size_t cy)
// ----------------------------------------------------------------------------
//   Add two elements in a vector
// ----------------------------------------------------------------------------
{
    return matrix_op(op, 0, c, 1, cx, 1, cy);
}


static algebraic_p vector_add(size_t c, size_t cx, size_t cy)
// ----------------------------------------------------------------------------
//   Addition of vector elements
// ----------------------------------------------------------------------------
{
    return vector_op(object::ID_add, c, cx, cy);
}


static algebraic_p matrix_add(size_t r, size_t c,
                              size_t rx, size_t cx,
                              size_t ry, size_t cy)
// ----------------------------------------------------------------------------
//   Addition of matrix elements
// ----------------------------------------------------------------------------
{
    return matrix_op(object::ID_add, r, c, rx, cx, ry, cy);
}


static algebraic_p vector_sub(size_t c, size_t cx, size_t cy)
// ----------------------------------------------------------------------------
//   Subtraction of vector elements
// ----------------------------------------------------------------------------
{
    return vector_op(object::ID_sub, c, cx, cy);
}


static algebraic_p matrix_sub(size_t r, size_t c,
                              size_t rx, size_t cx,
                              size_t ry, size_t cy)
// ----------------------------------------------------------------------------
//   Subtraction of matrix elements
// ----------------------------------------------------------------------------
{
    return matrix_op(object::ID_sub, r, c, rx, cx, ry, cy);
}



// ============================================================================
//
//    Matrix multiplication
//
// ============================================================================

static bool mul_dimension(size_t rx, size_t cx,
                          size_t ry, size_t cy,
                          size_t *rr, size_t *cr)
// ----------------------------------------------------------------------------
//   For multiplication, need matching rows and columns
// ----------------------------------------------------------------------------
//   We accept matrices with matching sizes, or vectors of the same size
{
    *rr = rx;
    *cr = cy;

    return cx == ry || (!rx && !ry && cx == cy);
}


static algebraic_p vector_mul(size_t c, size_t cx, size_t cy)
// ----------------------------------------------------------------------------
//   Multiplication of vector elements
// ----------------------------------------------------------------------------
{
    return vector_op(object::ID_mul, c, cx, cy);
}


static algebraic_p matrix_mul(size_t r, size_t c,
                              size_t rx, size_t cx,
                              size_t ry, size_t cy)
// ----------------------------------------------------------------------------
//   Compute one element in matrix multiplication
// ----------------------------------------------------------------------------
{
    size_t py = cy*ry;
    size_t px = py + cx*rx;

    algebraic_g e;
    if (ry != cx)
        record(matrix_error,
               "Inconsistent matrix size rx=%u cx=%u ry=%u cy%=u",
               rx, cx, ry, cy);
    for (size_t i = 0; i < cx; i++)
    {
        size_t ix = r * cx + i;
        size_t iy = cy * i + c;
        object_p x = rt.stack(px + ~ix);
        object_p y = rt.stack(py + ~iy);
        if (!x || !y)
            return nullptr;
        algebraic_g xa = x->as_algebraic();
        algebraic_g ya = y->as_algebraic();
        if (!xa || !ya)
        {
            rt.type_error();
            return nullptr;
        }
        e = i ? e + xa * ya : xa * ya;
        if (!e)
            return nullptr;
    }
    return e;
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
        algebraic_g det;
        algebraic_g tot;

        // Make space for temporary elements
        for (size_t j = 0; j < n; j++)
            if (!rt.push(this))
                goto err;

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

        size_t      n   = cx;
        size_t      pt  = n * n;    // n temporary elements to save diagonal
        size_t      pm  = 2 * pt;
        bool        neg = false;
        algebraic_g det;
        algebraic_g tot;
        algebraic_g one = integer::make(1);
        algebraic_g zero = integer::make(0);

        // Create an identity matrix of the right size
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < n; j++)
                if (!rt.push(i == j ? +one: +zero))
                    goto err;

        dump_matrix("Inverse of %u x %u matrix", n, n);

        // Loop across the diagonal
        for (size_t i = 0; i < n; i++)
        {
            // Find the index of first non-zero element
            bool zero = true;
            size_t index;

            record(matrix, "Row %u", i);
            for (index = i; zero && index < n; index++)
            {
                size_t ix = index * n + i;
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
            size_t oii = i * n + i;
            object_p a = rt.stack(pm + ~oii);
            record(matrix, "m[%u,%u]     a=%t", i, i, a);
            if (!a)
                goto err;
            algebraic_g aa = a->as_algebraic();
            if (!aa)
                goto err;

            // Loop below row i to compute r[j] = r[j] * a - r[i] * c
            record(matrix, "Zeroing sub-diagonals below %u", i);
            for (size_t j = i + 1; j < n; j++)
            {
                // Fetch value on diagonal and in next row
                size_t oji = j * n + i;
                object_p c = rt.stack(pm + ~oji);
                record(matrix, "m[%u,%u]     c=%t", j, i, c);
                if (!c)
                    goto err;
                algebraic_g ca = c->as_algebraic();
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
                        size_t ojk = j * n + k;
                        size_t oik = i * n + k;

                        object_p mjk = rt.stack(p + ~ojk);
                        object_p mik = rt.stack(p + ~oik);
                        if (!mjk || !mik)
                            goto err;
                        algebraic_g mjka = mjk->as_algebraic();
                        algebraic_g mika = mik->as_algebraic();
                        if (!mjka || !mika)
                            goto err;
                        mjka = aa * mjka - ca * mika;
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
                    size_t oik = i * n + k;
                    object_p mik = rt.stack(p + ~oik);
                    if (!mik)
                        goto err;
                    algebraic_g mika = mik->as_algebraic();
                    if (!mika)
                        goto err;
                    mika = mika / aa;
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
                algebraic_g za = z->as_algebraic();
                if (!za)
                    goto err;

                // This is only needed on the right matrix
                for (uint mat = 0; mat < 2; mat++)
                {
                    size_t p = mat ? pt : pm;
                    for (size_t k = mat ? 0 : i; k < n; k++)
                    {
                        size_t oik = i * n + k;
                        size_t ojk = j * n + k;
                        object_p mik = rt.stack(p + ~oik);
                        object_p mjk = rt.stack(p + ~ojk);
                        if (!mik || !mjk)
                            goto err;
                        algebraic_g mika = mik->as_algebraic();
                        algebraic_g mjka = mjk->as_algebraic();
                        if (!mika || !mjka)
                            goto err;
                        mjka = mjka  - za * mika;
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


COMMAND_BODY(dot)
// ----------------------------------------------------------------------------
//   Implement a dot product
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(1);
    object_p y = rt.stack(0);
    if (x && y)
    {
        array_g xa = x->as<array>();
        array_g ya = y->as<array>();
        if (xa && ya && rt.drop(2))
        {
            size_t      depth = rt.depth();
            size_t      xs    = 0;
            size_t      ys    = 0;
            algebraic_g xi, yi;
            if (xa->is_vector(&xs) && ya->is_vector(&ys))
            {
                if (xs == ys)
                {
                    algebraic_g result;
                    for (size_t i = 0; i < xs; i++)
                    {
                        xi = rt.stack(xs + ys + ~i)->as_algebraic();
                        yi = rt.stack(     ys + ~i)->as_algebraic();
                        result = result ? result + xi * yi : xi * yi;
                    }
                    rt.drop(rt.depth() - depth);
                    if (result && rt.push(+result))
                        return OK;
                }
                else
                {
                    rt.dimension_error();
                }
            }
            rt.drop(rt.depth() - depth);
            rt.push(+xa);
            rt.push(+ya);
        }
        else if ((xa || x->is_symbolic()) && (ya || y->is_symbolic()))
        {
            algebraic_g xe = x->as_algebraic_or_list();
            algebraic_g ye = y->as_algebraic_or_list();
            xe = expression::make(ID_dot, xe, ye);
            if (xe && rt.drop(2) && rt.push(+xe))
                return OK;
        }
        else
        {
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
    object_p x = rt.stack(1);
    object_p y = rt.stack(0);
    if (x && y)
    {
        array_g xa = x->as<array>();
        array_g ya = y->as<array>();
        if (xa && ya && rt.drop(2))
        {
            size_t      depth = rt.depth();
            size_t      xs    = 0;
            size_t      ys    = 0;
            algebraic_g xi, yi;
            if (xa->is_vector(&xs) && ya->is_vector(&ys))
            {
                if ((xs == 2 || xs == 3) && (ys == 2 || ys == 3))
                {
                    algebraic_g x1 = rt.stack(xs + ys + ~0)->as_algebraic();
                    algebraic_g x2 = rt.stack(xs + ys + ~1)->as_algebraic();
                    algebraic_g x3 = xs == 3
                        ? rt.stack(xs + ys + ~2)->as_algebraic()
                        : integer::make(0);
                    algebraic_g y1 = rt.stack(ys + ~0)->as_algebraic();
                    algebraic_g y2 = rt.stack(ys + ~1)->as_algebraic();
                    algebraic_g y3 = ys == 3
                        ? rt.stack(ys + ~2)->as_algebraic()
                        : integer::make(0);
                    algebraic_g r1 = x2 * y3 - x3 * y2;
                    algebraic_g r2 = x3 * y1 - x1 * y3;
                    algebraic_g r3 = x1 * y2 - x2 * y1;
                    algebraic_g r = list::make(ID_array, r1, r2, r3);
                    rt.drop(rt.depth() - depth);
                    if (r && rt.push(+r))
                        return OK;
                }
                else
                {
                    rt.dimension_error();
                }
            }
            rt.drop(rt.depth() - depth);
            rt.push(+ya);
            rt.push(+xa);
        }
        else if ((xa || x->is_symbolic()) && (ya || y->is_symbolic()))
        {
            algebraic_g xe = x->as_algebraic();
            algebraic_g ye = y->as_algebraic();
            xe = expression::make(ID_cross, xe, ye);
            if (xe && rt.drop(2) && rt.push(+xe))
                return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
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
        if (array_p v = obj->as<array>())
            if (v->is_2Dor3D(true))
                return OK;
        rt.push(obj);
        rt.type_error();
    }
    return ERROR;
}



// ============================================================================
//
//    Division
//
// ============================================================================

static bool div_dimension(size_t rx, size_t cx,
                          size_t ry, size_t cy,
                          size_t *rr, size_t *cr)
// ----------------------------------------------------------------------------
//   Divide vectors component-wide, or square matrices of same size
// ----------------------------------------------------------------------------
{
    *rr = rx;
    *cr = cx;
    return (rx == cx && ry == cy && rx == ry) || (!rx && !ry && cx == cy);
}


static algebraic_p vector_div(size_t c, size_t cx, size_t cy)
// ----------------------------------------------------------------------------
//   Division of vector elements
// ----------------------------------------------------------------------------
{
    return vector_op(object::ID_div, c, cx, cy);
}


static algebraic_p matrix_div(size_t r, size_t c,
                              size_t rx, size_t cx,
                              size_t ry, size_t cy)
// ----------------------------------------------------------------------------
//   Compute one element in matrix division
// ----------------------------------------------------------------------------
{
    return matrix_op(object::ID_div, r, c, rx, cx, ry, cy);
}


array_p array::do_matrix(array_r x, array_r y,
                         dimension_fn dim, vector_fn vec, matrix_fn mat)
// ----------------------------------------------------------------------------
//   Perform a matrix or vector operation
// ----------------------------------------------------------------------------
{
    size_t rx = 0, cx = 0, ry = 0, cy = 0, rr = 0, cr = 0;
    size_t depth = rt.depth();

    // Check if either argument is non-rectangular.
    // If so, convert to rectangular for computation, convert back to Y format
    id xty = x->is_2Dor3D(false);
    if (is_non_rectangular(xty))
    {
        array_g xr = x->to_rectangular();
        return do_matrix(xr, y, dim, vec, mat);
    }
    id yty = y->is_2Dor3D(false);
    if (is_non_rectangular(yty))
    {
        array_g yr = y->to_rectangular();
        yr =  do_matrix(x, yr, dim, vec, mat);
        if (yty == ID_ToSpherical)
            yr = yr->to_spherical();
        else
            yr = yr->to_polar();
        return yr;
    }

    object::id ty = x->type();
    if (x->is_vector(&cx))
    {
        if (!y->is_vector(&cy))
        {
            rt.type_error();
            goto err;
        }
        if (!dim(0, cx, 0, cy, &rr, &cr))
        {
            rt.dimension_error();
            goto err;
        }

        scribble scr;
        for (size_t c = 0; c < cx; c++)
        {
            algebraic_g e = vec(c, cx, cy);
            if (!rt.append(e))
                goto err;
        }

        rt.drop(rt.depth() - depth);
        return array_p(list::make(ty, scr.scratch(), scr.growth()));
    }

    if (x->is_matrix(&rx, &cx))
    {
        bool vector = false;
        if (!y->is_matrix(&ry, &cy))
        {
            if (mat == matrix_mul && y->is_vector(&ry))
            {
                // We can multiply a matrix by a vector
                cy = 1;
                vector = true;
            }
            else
            {
                rt.type_error();
                goto err;
            }
        }
        if (!dim(rx, cx, ry, cy, &rr, &cr))
        {
            rt.dimension_error();
            goto err;
        }

        // Special case of matrix division
        if (mat == matrix_div)
        {
            rt.drop(rt.depth() - depth);
            array_g ya = y->invert();
            return do_matrix(ya, x, mul_dimension, vector_mul, matrix_mul);
        }

        scribble scr;
        for (size_t r = 0; r < rr; r++)
        {
            object_g row = nullptr;
            if (vector)
            {
                row = object_p(mat(r, 0, rx, cx, ry, cy));
            }
            else
            {
                scribble sr;
                for (size_t c = 0; c < cr; c++)
                {
                    algebraic_g e = mat(r, c, rx, cx, ry, cy);
                    if (!rt.append(e))
                        goto err;
                }
                row = object_p(list::make(ty, sr.scratch(), sr.growth()));
            }
            if (!rt.append(row))
                goto err;
        }

        rt.drop(rt.depth() - depth);
        return array_p(list::make(ty, scr.scratch(), scr.growth()));
    }

err:
    rt.drop(rt.depth() - depth);
    return nullptr;
}


array_p operator+(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Add two arrays
// ----------------------------------------------------------------------------
{
    return array::do_matrix(x, y, add_sub_dimension, vector_add, matrix_add);
}


array_p operator-(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Subtract two arrays
// ----------------------------------------------------------------------------
{
    return array::do_matrix(x, y, add_sub_dimension, vector_sub, matrix_sub);
}


array_p operator*(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Multiply two arrays
// ----------------------------------------------------------------------------
{
    return array::do_matrix(x, y, mul_dimension, vector_mul, matrix_mul);
}


array_p operator/(array_r x, array_r y)
// ----------------------------------------------------------------------------
//   Divide two arrays
// ----------------------------------------------------------------------------
{
    return array::do_matrix(x, y, div_dimension, vector_div, matrix_div);
}
