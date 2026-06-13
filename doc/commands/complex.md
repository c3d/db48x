# Operations with Complex Numbers

## Re
Real part of a complex number

## Im
Imaginary part of a complex number

## Arg
Argument of a complex number

## Conj
Conjugate of a complex number

## RectangularToReal
Extract real and imaginary parts from a complex number in rectangular form.

This command can be used in algebraic expressions, in which case it automatically produces a list with two results:

```rpl
'ℂ→ℝ(3+4ⅈ)' →Num
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
'ℝ→ℂ(3;4)' →Num
@ Expecting 3+4ⅈ
```

## PolarToReal
Extract modulus and argument from a complex number in polar form.

This command can be used in algebraic expressions, in which case it automatically produces a list with two results:

```rpl
'Polarℂ→ℝ(5∡90°)' →Num
@ Expecting { mod:5 arg:90. ° }
```

## RealToPolar
Make a complex number in polar form from modulus and argument

```rpl
5 90 ℝ→Polarℂ
@ Expecting 5∡90°
```

This command can be used in algebraic expressions:

```rpl
'ℝ→Polarℂ(5;90)' →Num
@ Expecting 5∡90.°
```

## ToRectangular
Convert a complex number or vector to rectangular (cartesian) form.

```rpl
5∡90° →Rectangular
@ Expecting 5ⅈ
```

This command can be used in algebraic expressions:

```rpl
'→Rectangular(5∡90°)' →Num
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
'→Polar(3+4ⅈ)' →Num
@ Expecting 5.∡53.13010 23542°
```

## ToCylindrical
Convert a 3D vector to cylindrical form

## ToSpherical
Convert a 3D vector to spherical form.

## To2DVector
Make a 2D vector from two components

## To3DVector
Make a 3D vector from three components

## FromVector
Expand a vector into its individual components
