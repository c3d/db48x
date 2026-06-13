# Operations with Complex Numbers

## Re
Real part of a complex number

```rpl
3+4ⅈ Re
@ Expecting 3
```

## Im
Imaginary part of a complex number

```rpl
3+4ⅈ Im
@ Expecting 4
```

## Arg
Argument of a complex number

```rpl
3+4ⅈ Arg
@ Expecting 53.13010 23542 °
```

## Conj
Conjugate of a complex number

```rpl
3+4ⅈ Conj
@ Expecting 3-4ⅈ
```

## RectangularToReal
Extract real and imaginary parts from a complex number in rectangular form.

```rpl
3+4ⅈ ℂ→ℝ 2 →List
@ Expecting { re:3 im:4 }
```

This command can be used in algebraic expressions, in which case it automatically produces a list with two results:

```rpl
'ℂ→ℝ(3+4ⅈ)'
@ Expecting { re:3 im:4 }
```

## RealToRectangular
Make a complex number in rectangular form from real and imaginary part

```rpl
3 4 ℝ→ℂ
@ Expecting 3+4ⅈ
```

This command can be used in algebraic expressions:

```rpl
'ℝ→ℂ(3;4)'
@ Expecting 3+4ⅈ
```

## PolarToReal
Extract modulus and argument from a complex number in polar form.

```rpl
5∡90° Polarℂ→ℝ 2 →List
@ Expecting { mod:5 arg:90 ° }
```

This command can be used in algebraic expressions, in which case it automatically produces a list with two results:

```rpl
'Polarℂ→ℝ(5∡90°)'
@ Expecting { mod:5 arg:90 ° }
```

## RealToPolar
Make a complex number in polar form from modulus and argument

```rpl
5 90 ℝ→Polarℂ
@ Expecting 5∡90°
```

This command can be used in algebraic expressions:

```rpl
'ℝ→Polarℂ(5;90)'
@ Expecting 5∡90°
```

## ToRectangular
Convert a complex number or vector to rectangular (cartesian) form.

```rpl
5∡90° →Rectangular
@ Expecting 5ⅈ
```

This command can be used in algebraic expressions:

```rpl
'→Rectangular(5∡90°)'
@ Expecting 5ⅈ
```

## ToPolar
Convert a complex number or a 2D or 3D vector to polar form.

```rpl
3+4ⅈ →Polar
@ Expecting 5.∡53.13010 23542°
```

This command can be used in algebraic expressions:

```rpl
'→Polar(3+4ⅈ)'
@ Expecting 5.∡53.13010 23542°
```

## ToCylindrical
Convert a 3D vector to cylindrical form

## ToSpherical
Convert a 3D vector to spherical form.

## To2DVector
\Make a 2D vector from two components

```rpl
3 4 →V2
@ Expecting [ 3 4 ]
```

This command can be used in algebraic expressions:

```rpl
'→V2(3;4)'
@ Expecting [ 3 4 ]
```


## To3DVector
Make a 3D vector from three components

```rpl
3 4 5 →V3
@ Expecting [ 3 4 5 ]
```

This command can be used in algebraic expressions:

```rpl
'→V3(3;4;5)'
@ Expecting [ 3 4 5 ]
```


## FromVector
Expand a vector into its individual components

```rpl
[ 3 4 ] V→ Depth →List
@ Expecting { 3 4 }
```

This command returns a variable number of items on the stack

```rpl
[ 3 4 5 6 7 8 ] V→ Depth →List
@ Expecting { 3 4 5 6 7 8 }
```

This command preserves formats such as cylindrical:

```rpl
[ 2 3 4 ] →Cylindrical V→ Depth →List
@ Expecting { 3.60555 12754 6 56.30993 2474 ° 4 }
```
