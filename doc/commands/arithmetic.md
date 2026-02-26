# Arithmetic

## Add

Add two values.

`Y` `X` ▶ `Y+X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical addition. For example, `1 2 +` is `3`.
* For equations and symbols, build a sum, eliminating zero additions if
  [autosimplify](#autosimplify) is active.
* For lists, concatenate lists, or add objects to a list. For example, `{ A } { B
  } +` is `{ A B }`, and `{ A B C } "D" +` is `{ A B C "D" }`.
* For text, concatenate text, or concatenate the text representation of an
  object to an existing text. For example `"X" "Y" + ` gives `"XY"`, and
  `"X=" 1 +` gives `"X=1"`.

*Note*: The ` Add ` command and the ` + ` operation are identical on DB48X.
[This differs from HP calculators](#list-operation-differences).



## Subtract

Subtract two values.

`Y` `X` ▶ `Y-X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical subtraction. For example, `1 2 -` is `-1`.
* For equations and symbols, build a difference, eliminating subtraction of 0 if
  [autosimplify](#autosimplify) is active.


## Multiply

Multiply two values.

`Y` `X` ▶ `Y×X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical multiplication. For example, `3 2 *` is `6`.
* For vectors, multiply individual elements (this is a deviation from HP48).
  For example, `[ 1 2 3 ] [ 4 5 6 ] *` is `[ 4 10 18 ]`.
* For matrices, perform a matrix multiplication.
* For a matrix and a vector, apply the matrix to the vector.
* For equations and symbols, build a product, eliminating multiplication by 1
  or 0 when [autosimplify](#autosimplify) is active.
* For a list and a positive integer, repeat the list. For example, `{ A } 3 *`
  is `{ A A A }`.
* For a text and a positive integer, repeat the text. For example `"X" 3 * `
  gives `"XXX"`.


## Divide

Divide two values.

`Y` `X` ▶ `Y÷X`

* For integer, build a fraction. For example `1 7 /` gives `1/7`.
* For fractional, decimal or complex numbers, this performs the
  expected numerical division. For example, `1. 2. /` is `0.5`.
* For vectors, divide individual elements. For example,
  `[ 1 2 3 ] [ 3 2 1 ] /` is `[ 1/3 1 3 ]`.
* For equations and symbols, build a ratio, eliminating division by one
  and division of 0 when [autosimplify](#autosimplify) is active.


## Pow

Raise to the power.

`Y` `X` ▶ `Y^X`

* For integer, fractional, decimal or complex numbers, this raises the
  value in level 2 to the value in level 1. For example, `2 3 ↑` is `8`.
* For vectors, raise individual elements in the first vector to the power of the
  corresponding element in the second vector.
* For equations and symbols, build an expression, eliminating special cases
  when [autosimplify](#autosimplify) is active.


## xroot

Compute the X-th root of Y. `Y X xroot` is equivalent to `Y (1/X) pow`.

`Y` `X` ▶ `Y↑(1/X)`

```rpl
8 3 xroot
@ Expecting 2
```


## Floor

Return the largest integer less than or equal to the input. This rounds toward
negative infinity.

`X` ▶ `⌊X⌋`

```rpl
3.7 Floor
@ Expecting 3
```

```rpl
-3.2 Floor
@ Expecting -4
```

For complex numbers, `Floor` is applied independently to the real and imaginary
parts. For example, `(3.7,−2.1) Floor` gives `(3,−3)`.


## Ceil

Return the smallest integer greater than or equal to the input. This rounds
toward positive infinity.

`X` ▶ `⌈X⌉`

```rpl
3.2 Ceil
@ Expecting 4
```

```rpl
-3.7 Ceil
@ Expecting -3
```

For complex numbers, `Ceil` is applied independently to the real and imaginary
parts.


## IntPart

Return the integer part of a number, truncating toward zero. This differs from
[Floor](#floor) for negative numbers.

Also known as `IntegerPart` or `IP`.

`X` ▶ `IP(X)`

```rpl
3.7 IntPart
@ Expecting 3
```

```rpl
-3.7 IntPart
@ Expecting -3
```


## FracPart

Return the fractional part of a number, defined as `X - IntPart(X)`, so that
`IntPart(X) + FracPart(X) = X`. The result has the same sign as the input.

Also known as `FractionalPart` or `FP`.

`X` ▶ `X - IP(X)`

```rpl
3.7 FracPart
@ Expecting 0.7
```

```rpl
-3.7 FracPart
@ Expecting -0.7
```


## Abs

Return the absolute value of a real number, or the magnitude (Euclidean norm)
of a complex number, vector or matrix.

`X` ▶ `|X|`

* For a real number, returns the non-negative value. For example, `−5 Abs`
  gives `5`.
* For a complex number `(a,b)`, returns `√(a²+b²)`. For example, `(3,4) Abs`
  gives `5`.
* For a vector, returns the Euclidean norm (square root of the sum of squares
  of the elements). For example, `[ 3 4 ] Abs` gives `5`.
* For a matrix, returns the Frobenius norm.

Also accessible as `norm`, `modulus`, or `abs`.


# Integer arithmetic and polynomials

This section documents newRPL commands that are not implemented yet in DB48X.

## SETPREC
Set the current system precision


## GETPREC
Get the current system precision


## MODSTO
Set the current system modulo for all MOD operations


## MODRCL
Get the current system modulo


## POWMOD
Power operator MOD the current system modulo


## MOD

Remainder of the integer division. Returns the remainder with the same sign as
the divisor (floor division convention). For real numbers, this is equivalent to
`X - Floor(X/Y) * Y`.

`Y` `X` ▶ `Y mod X`

```rpl
17 5 mod
@ Expecting 2
```

```rpl
-7 3 mod
@ Expecting 2
```


## SQ

Square of the input. Equivalent to `X X *` or `X 2 ↑`, but more efficient.

Also accessible as `x²` or `Square`.

`X` ▶ `X²`

```rpl
7 sq
@ Expecting 49
```

```rpl
(3,4) sq
@ Expecting (−7,24)
```


## NEXTPRIME

Return the smallest prime number strictly greater than the input. Also
accessible as `NextPr`.

`N` ▶ `next_prime(N)`

```rpl
10 NextPrime
@ Expecting 11
```

```rpl
2 NextPrime
@ Expecting 3
```

For large integers, uses the [Miller-Rabin primality test](#isprime-1).


## Factorial

Compute the factorial of a non-negative integer `n`, defined as `n! = 1×2×…×n`,
with `0! = 1`.

Also accessible as `x!` or `!`.

`N` ▶ `N!`

```rpl
5 fact
@ Expecting 120
```

For large integers, the result is an exact big integer. For example, `50 fact`
returns a 65-digit number.


## FACTORS

Decompose an integer into its prime factors. Returns a list alternating prime
factors and their exponents, sorted in ascending order of the prime.

`N` ▶ `{ p₁ e₁ p₂ e₂ … }`

```rpl
12 Factors
@ Expecting { 2 2 3 1 }
```

```rpl
100 Factors
@ Expecting { 2 2 5 2 }
```

```rpl
2147483647 Factors
@ Expecting { 2 147 483 647 1 }
```

For large semiprime numbers (products of two large primes), the algorithm uses
Pollard's Rho method with Brent's cycle detection, which runs in O(n^¼) time.

If the factorization would require too much memory or too many iterations,
`Factors` returns an error instead of running indefinitely.


## ISPRIME

Test whether the input is a prime number. Returns `1` (true) if the number is
prime, `0` (false) if it is composite, or an error for invalid input (negative
numbers, non-integers).

`N` ▶ `0` or `1`

```rpl
17 IsPrime
@ Expecting 1
```

```rpl
15 IsPrime
@ Expecting 0
```

```rpl
2147483647 IsPrime
@ Expecting 1
```

For small numbers, trial division against a table of small primes is used.
For larger numbers, a deterministic [Miller-Rabin](#miller-rabin) test with
carefully chosen witnesses is used, giving correct results for all integers up
to at least 3.3 × 10²⁴. For even larger numbers, a probabilistic version is
used that is extremely unlikely to give a wrong answer.

Carmichael numbers (pseudoprimes for naive tests) are correctly identified as
composite:

```rpl
561 IsPrime
@ Expecting 0
```


## MANT
Mantissa of a real number

```rpl
1.2345E123 MANT
@Expecting 1.2345
```


## XPON
Exponent of a real number

```rpl
1.2345E123 XPON
@Expecting 123
```


## SigDig
Number of significant digits in a real number

```rpl
1.2345E123 SigDig
@Expecting 5
```

## Sign

Return the sign of a number: `−1`, `0` or `1`.

For complex numbers, returns the unit complex number with the same argument
(i.e. the number divided by its absolute value).

`X` ▶ `sign(X)`

```rpl
-42 sign
@ Expecting -1
```

```rpl
0 sign
@ Expecting 0
```

```rpl
(3,4) sign
@ Expecting (3/5,4/5)
```


## Percent

Compute a percentage of a base value. Returns `Y × (X ÷ 100)`.

`Y` `X` ▶ `Y×(X÷100)`

```rpl
200 15 %
@ Expecting 30
```


## PercentChange

Compute the percentage change from a base to a new value. Returns `(X÷Y−1)×100`.

`Y` `X` ▶ `(X÷Y−1)×100`

```rpl
200 250 %Change
@ Expecting 25
```


## PercentTotal

Compute what percentage of a total a value represents. Returns `(X÷Y)×100`.

`Y` `X` ▶ `(X÷Y)×100`

```rpl
200 50 %Total
@ Expecting 25
```


## GCD

Greatest common divisor of two integers.

`Y` `X` ▶ `GCD(Y, X)`

```rpl
48 36 GCD
@ Expecting 12
```

```rpl
100 75 GCD
@ Expecting 25
```


## LCM

Least common multiple of two integers.

`Y` `X` ▶ `LCM(Y, X)`

```rpl
12 8 LCM
@ Expecting 24
```


## IDIV2
Integer division, get quotient and remainder.
On DB48X, this is an alias for [div2](#div2).

`Y` `X` ▶ `IP(Y/X)` `Y rem X`

## IQUOT
Quotient of the integer division


## ADDTMOD
Addition operator MOD the current system modulo


## SUBTMOD
Subtraction operator MOD the current system modulo


## MULTMOD
Multiplication operator MOD the current system modulo


## PEVAL
Evaluation of polynomial given as vector of coefficients


## PCOEF
Coefficients of monic polynomial with the given roots


## IEGCD
Extended euclidean algorithm


## IABCUV
Find integers u,v to solve a*u+b*v=c


## PTCHEBYCHEFF
Nth Tchebycheff polynomial


## PLEGENDRE
Nth Legendre polynomial


## PHERMITE
Nth Hermite polynomial as used by physics


## PTCHEBYCHEFF2
Nth Tchebycheff polynomial of the second kind


## PHERMITE2
Nth Hermite polynomial as used in probabilities


## QuotientRemainder

Euclidean division, returning quotient and remainder.

On HP calculators, this only applies for polynomials.

On DB48X, this applies to integers, big integers, decimals, fractions and
polynomials. It puts both the quotient and the remainder on the stack. For many
data types, this operation is significantly more efficient than computing the
quotient and the remainder separately.

`Y` `X` ▶ `IP(Y/X)` `Y rem X`


## PDIV2
Polynomial euclidean division as coefficient vector


## PDER
Derivative of polynomial as coefficient vector


## PINT
Integration of polynomials as coefficient vector


## PMUL
Multiplication of polynomials as coefficient vectors


## PADD
Addition of polynomials as coefficient vector


## PSUB
Subtraction of polynomials as coefficient vector


## Min

Return the smaller of two values.

`Y` `X` ▶ `min(Y, X)`

```rpl
3 5 Min
@ Expecting 3
```

Works with integers, decimals, fractions, and text (lexicographic order).


## Max

Return the larger of two values.

`Y` `X` ▶ `max(Y, X)`

```rpl
3 5 Max
@ Expecting 5
```

Works with integers, decimals, fractions, and text (lexicographic order).


## RND

Round a number to the specified number of decimal places or significant figures,
depending on the display mode. Also accessible as `Round` or `Rnd`.

`X` `N` ▶ `round(X, N)`

In `FIX` mode, `N` specifies the number of digits after the decimal point.
In `SCI` or `ENG` mode, `N` specifies the number of significant digits.

```rpl
3.14159 2 Round
@ Expecting 3.14
```

```rpl
3.14159 4 Round
@ Expecting 3.1416
```


## TRNC

Truncate a number to the specified number of decimal places, discarding the
remaining digits without rounding. Also accessible as `Truncate` or `Trnc`.

`X` `N` ▶ `trunc(X, N)`

```rpl
3.14159 2 Truncate
@ Expecting 3.14
```

```rpl
3.99999 2 Truncate
@ Expecting 3.99
```


## DIGITS
Extract digits from a real number


## PROOT
All roots of a polynomial


## PREVIOUSPRIME

Return the largest prime number strictly less than the input. Also accessible
as `PrevPr`.

`N` ▶ `prev_prime(N)`

```rpl
10 PreviousPrime
@ Expecting 7
```

```rpl
3 PreviousPrime
@ Expecting 2
```

Returns an error if there is no prime smaller than the input (i.e. for inputs
≤ 2).

For large integers, uses the [Miller-Rabin primality test](#isprime-1).
