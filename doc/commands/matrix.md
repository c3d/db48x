# Operations with Matrices and vectors

## →Array

Stack to Array Command: Returns a vector or matrix built from individual
elements placed on the stack and dimensions.

If the dimension is given as a positive integer, then `→Array` returns a
vector built from the given number of individual items.

```rpl
x y z 3 →Array
@ Expecting [ 'x' 'y' 'z' ]
```

The number of items can also be given as a list or array containing one or two
positive integers. If it contains one item, then `→Array` returns a vector:

```rpl
1.2 3.4 5.6 { 2.5 } →Array
@ Expecting [ 3.4 5.6 ]
```

If the list contains two items, the first one is the number of rows, the second
one the number of columns. Elements of the result array should be entered on the
stack in row order.

```rpl
1 2 3 4 5 6 [ 2 3 ] →Array
@ Expecting [[ 1 2 3 ] [ 4 5 6 ]]
```

## Array→

Array to Stack Command: Takes an array and returns its elements as separate
values. Also returns a list of the dimensions of the array.

If the argument is a vector, elements are placed on the stack with the first one
higher in the stack and the last one on the second level of the stack.

```rpl
[ 1 2 3 ] Array→ + + +
@ Expecting { 1 2 3 3 }
```

If the argument is a matrix, elements are placed on the stack in row order:

```rpl
[[1 2 3][4 5 6]] Array→ + + + + + +
@ Expecting { 1 2 3 4 5 6 2 3 }
```

## →Columns

Split an array into column vectors

If the input is a vector, `→Columns` returns the individual elements.

```rpl
[ 1 2 3 ] →Columns
4 →List
@ Expecting { 1 2 3 3 }
```

If the input is a matrix, `→Columns` returns the individual columns.

```rpl
[[ 1 2 3 ][ 4 5 6 ]] →Columns
4 →List
@ Expecting { [ 1 4 ] [ 2 5 ] [ 3 6 ] 3 }
```



## COL+

Insert Columns Command: Insert columns into an existing array. The `COL+`
command takes three arguments:
* an input array or list where the columns will be inserted
* the columns to insert
* the insertion position

If the input is a matrix, the columns can be an individual vector:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ 22 33 ]              @ Column to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 2 3 ] [ 4 33 5 6 ]]
```

or a matrix with the same number of rows:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ [22 33 ] [ 44 55 ] ] @ Columns to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 33 2 3 ] [ 4 44 55 5 6 ]]
```

If the input is a vector, then the columns can be an individual value:

```rpl
[ 1 2 3 ]  @ Input vector
4          @ Value to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 2 3 ]
```

The columns can also be another vector:

```rpl
[ 1 2 3 ]  @ Input vector
[ 4 5 ]    @ Values to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 5 2 3 ]
```


## COL-

Delete Columns Command: Deletes one or more columns from an array.

The `COL-` command takes an input array and a column index in the array, and
returns an array with the given column removed.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
3 COL-
@ Expecting [[ 11 12 14 15 16 ] [ 21 22 24 25 26 ] [ 31 32 34 35 36 ]]
```

Multiple columns can be removed by giving the first one and the number of
columns to remove.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
[ 3 2 ] COL-
@ Expecting [[ 11 12 15 16 ] [ 21 22 25 26 ] [ 31 32 35 36 ]]
```

## Columns→

Columns to Matrix Command: Transforms a series of column vectors and a column
count into a matrix containing those columns, or transforms a sequence of
numbers and an element count into a vector with those numbers as elements.

```rpl
[ 1 2 ]
[ 4 5 ]
[ 7 8 ]
3 Columns→
@ Expecting [[ 1 4 7 ] [ 2 5 8 ]]
```

If the individual values are not arrays, then a vector is produced:

```rpl
1 2 3 4
4 Columns→
@ Expecting [ 1 2 3 4 ]
```

If not all vectors have the same length, the number of rows of the array
returned by `Columns→` is the maximum size of all input vectors, and the
remaining elements are padded with `0`.

```rpl
[ 1     ]
[ 2 3 4 ]
[ 5 6   ]
3 Columns→
@ Expecting [[ 1 2 5 ] [ 0 3 6 ] [ 0 4 0 ]]
```


## →Rows

Split an array into row vectors

If the input is a vector, `→Rows` returns the individual elements.

```rpl
[ 1 2 3 ] →Rows
4 →List
@ Expecting { 1 2 3 3 }
```

If the input is a matrix, `→Rows` returns the individual rows.

```rpl
[[ 1 2 3 ][ 4 5 6 ]] →Rows
3 →List
@ Expecting { [ 1 2 3 ] [ 4 5 6 ] 2 }
```



## ROW+

Insert Rows Command: Insert rows into an existing array. The `COL+`
command takes three arguments:
* an input array or list where the rows will be inserted
* the rows to insert
* the insertion position

If the input is a matrix, the rows can be an individual vector:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ 22 33 ]              @ Row to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 2 3 ] [ 4 33 5 6 ]]
```

or a matrix with the same number of rows:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ [22 33 ] [ 44 55 ] ] @ Rows to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 33 2 3 ] [ 4 44 55 5 6 ]]
```

If the input is a vector, then the rows can be an individual value:

```rpl
[ 1 2 3 ]  @ Input vector
4          @ Value to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 2 3 ]
```

The rows can also be another vector:

```rpl
[ 1 2 3 ]  @ Input vector
[ 4 5 ]    @ Values to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 5 2 3 ]
```


## ROW-

Delete Rows Command: Deletes one or more rows from an array.

The `COL-` command takes an input array and a row index in the array, and
returns an array with the given row removed.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
3 COL-
@ Expecting [[ 11 12 14 15 16 ] [ 21 22 24 25 26 ] [ 31 32 34 35 36 ]]
```

Multiple rows can be removed by giving the first one and the number of
rows to remove.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
[ 3 2 ] COL-
@ Expecting [[ 11 12 15 16 ] [ 21 22 25 26 ] [ 31 32 35 36 ]]
```

## Rows→

Rows to Matrix Command: Transforms a series of row vectors and a row
count into a matrix containing those rows, or transforms a sequence of
numbers and an element count into a vector with those numbers as elements.

```rpl
[ 1 2 ]
[ 4 5 ]
[ 7 8 ]
3 Rows→
@ Expecting [[ 1 2 ] [ 4 5 ] [ 7 8 ]]
```

If the individual values are not arrays, then a vector is produced:

```rpl
1 2 3 4
4 Rows→
@ Expecting [ 1 2 3 4 ]
```

If not all vectors have the same length, a non-rectangular array will be
produced. Unlike `Rows→`, no padding with `0` will occur for missing elements.

```rpl
[ 1     ]
[ 2 3 4 ]
[ 5 6   ]
3 Rows→
@ Expecting [[ 1 ] [ 2 3 4 ] [ 5 6 ]]
```

## TODIAG
Extract diagonal elements from a matrix


## FROMDIAG
Create a matrix with the given diagonal elements



## AXL
Convert a matrix to list and vice versa


## BASIS
Find vectors forming a basis of the subspace represented by the matrix


## CHOLESKY
Perform Cholesky decomposition on a matrix


## ColumnNorm
Column norm (one norm) of a matrix


## RowNorm
Row norm (infinity norm) of a matrix


## Norm

Euclidean norm of a vector of matrix


## ConstantArray

Returns a constant array, defined as an array whose elements all have the same
value.

The constant value is an object taken from argument 2/level 1. The resulting
array is either a new array, or an existing array with its elements replaced by
the constant, depending on the object in argument 1/level 2.

* Creating a new array: If level 2 contains a list of one or two integers, `CON`
  returns a new array. If the list contains a single integer `n`, `CON` returns
  a constant vector with `n` elements. If the list contains two integers `n` and
  `m`, `CON` returns a constant matrix with `n` rows and `m` columns.

* Replacing the elements of an existing array: If level 2 contains an array,
  `CON` returns an array of the same dimensions, with each element equal to the
  constant.

* If level 2 contains a name, the name must identify a variable that contains a
  valid input for `con`, such as an array. In this case, the content of the
  variable is replaced with the value generated by `CON`

`n` `k` ▶ `[ k ... k ]`

`{ n }` `k` ▶ `[ k ... k ]`

`{ n m }` `k` ▶ `[ [ k ... k ] [ k ... k ] ... [ k ... k ] ]`

`[ vec ]` `k` ▶ `[ k ... k]`

`[ [ mat ] ]` `k` ▶ `[ [ k ... k ]]`

`'name'` `k` ▶



## Cond
Column norm condition number of a matrix


## Cross
Cross produce of vectors


## CSWP
Swap two columns in an array or vector. The `CSWP` command takes an array and
two column numbers.

If the input is a vector, then the elements at the given index are swapped with
one another, much like `RSWP`.

```rpl
[11 22 33 44 55] 2 3 CSWP
@ Expecting [ 11 33 22 44 55 ]
```

If the input is an array with more than one dimension, then the columns at the
given index are swapped with one another:

```rpl
[[ 11 12 13 14 ]
 [ 21 22 23 24 ]
 [ 31 32 33 34 ]]
1 4 CSWP
@ Expecting [[ 14 12 13 11 ] [ 24 22 23 21 ] [ 34 32 33 31 ]]
```

As an extension relative to HP's implementation, the command also works with
lists.

```rpl
{ { "A" 2 } { "C" 4 "X" } }
1 2 CSWP
@ Expecting { { 2 "A" } { 4 "C" "X" } }
```

## Determinant

Compute the determinant of a matrix

## DOT

Internal product (dot product) of vectors


## EGV


## EGVL
Compute the eigenvalues of a matrix


## GRAMSCHMIDT


## HADAMARD
Multiply corresponding elements in a matrix


## HILBERT
Assemble a Hilbert symbolic array


## IBASIS
Find a basis of the intersection of two vector spaces


## IdentityMatrix

Identity Matrix Command: Returns an identity matrix, that is, a square matrix
with its diagonal elements equal to 1 and its off-diagonal elements equal to 0.

The result is either a new square matrix, or an existing square matrix with its
elements replaced by the elements of the identity matrix, according to the
argument.

* Creating a new matrix: If the argument is an integer `n`, a new real identity
  matrix is returned, with its number of rows and number of columns equal to
  `n`.

* Replacing the elements of an existing matrix: If the argument is a square
  matrix, an identity matrix of the same dimensions is returned.

* Generating the identity matrix for a vector: If the argument is a vector with
  `n` elements, an identity matrix with `n` rows and `n` columns is created.

* If the argument is a name, the name must identify a variable containing on of the valid inputs. In this case, it is replaced with the result.


`n`  ▶ `IDN(n)`

`{ n }` ▶ `IDN(n)`

`{ n n }`  ▶ `IDN(n)`

`[ n-vec ]` ▶ `IDN(n)`

`[[ nxn-mat ]]` ▶ `IDN(n)`

`'name'` ▶


## IMAGE
Find a basis of the image of a linear application


## ISOM


## JORDAN


## KER
Find a basis for the kernel of a linear application


## LQ


## LSQ


## LU
LU factorization of a matrix


## MAD


## MKISOM


## PMINI
Minimal polynomial of a matrix


## QR
QR Decomposition of a matrix


## RANK
Rank of a matrix


## RandomMatrix

Returns an array containing random integer values between -9 and 9.


## RCI
Multiply a row by a constant


## RCIJ
Multiply a row by a constant and add to other row


## RDM
Change dimensions of an array


## REF
Reduce a matrix to echelon form (upper triangular) using Gaussian elimination.
Requires symbolic results.

`M` `REF` → `M'`

See also: `RREF`, `RREFP`.


## RREF
Fully reduce a matrix to row-reduced echelon form using Gauss-Jordan elimination.
For an augmented system matrix, the coefficient block becomes an identity and the
extra column holds the solution. Requires symbolic results.

`M` `RREF` → `M'`

```rpl
[[3 4 5][5 6 7]] RREF
@ Expecting [[ 1 0 -1 ] [ 0 1 2 ]]
```

See also: `REF`, `RREFP`.


## RREFP
Row-reduced echelon form with pivot list. Returns the pivot values and a matrix
whose coefficient block is diagonal but not necessarily scaled to 1. DB48X uses
`RREFP` because commands are case-insensitive and the HP50G name `rref` would
collide with `RREF`. Requires symbolic results.

`M` `RREFP` → `{ pivots }` `M'`

```rpl
[[2 1][3 4]] RREFP 2 →List
@ Expecting { { 10 5 } [[ 10 0 ] [ 0 5 ]] }
```

See also: `RREF`, `REF`, `RREFMOD`.


## RREFMOD
Modular row-reduced echelon form (reduction modulo the current CAS modulus).
Requires symbolic results.


## EchelonFormKeepLastColumn

When set, `RREF` and `RREFP` reduce the coefficient block but leave the last
column unnormalizedw. The last column is still updated by row operations on the
rest of the matrix. The inverse flag is `EchelonFormReduceLastColumn`.

This corresponds to HP50G system flag −126.

## EchelonFormReduceLastColumn

When set, `RREF` and `RREFP` reduce the coefficient block including the last column. The inverse flag is `EchelonFormKeepLastColumn`.

## RSD
Residual R=B-A*X' on a system A*X=B


## RSWP
Swap two rows in an array or vector. The `RSWP` command takes an array and
two column numbers.

If the input is a vector, then the elements at the given index are swapped with
one another, much like `CSWP`.

```rpl
[11 22 33 44 55] 2 3 RSWP
@ Expecting [ 11 33 22 44 55 ]
```

If the input is an array with more than one dimension, then the columns at the
given index are swapped with one another:

```rpl
[[ 11 12 13 14 ]
 [ 21 22 23 24 ]
 [ 31 32 33 34 ]]
1 2 RSWP
@ Expecting [[ 21 22 23 24 ] [ 11 12 13 14 ] [ 31 32 33 34 ]]
```

As an extension relative to HP's implementation, the `RSWP` command also works with
lists.

```rpl
{ { "A" 2 } { "C" 4 "X" } }
1 2 RSWP
@ Expecting { { "C" 4 "X" } { "A" 2 } }
```



## SCHUR


## SNRM


## SRAD


## SVD


## SVL


## SYLVESTER


## TRACE
Sum of the items in the diagonal of a matrix


## TRAN
Transpose a matrix (without comjugate, unlike `TRN`)


## TRN
Complex conjugate transpose of a matrix


## VANDERMONDE


## LDUP
Decompose A into LDUP such that P*A=L*D<sup>-1</sup>*U


## MMAP
Apply expression or program to the elements of a matrix
