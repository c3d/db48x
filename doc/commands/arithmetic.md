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

Subtract two values

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
  For example, `[ 1 2 3 ] [ 4 5 6 ] +` is `[ 4 10 18 ]`.
* For matrices, perform a matrix multiplication.
* For a matrix and a vector, apply the matrix to the vector.
* For equations and symbols, build a product, eliminating multiplication by 1
  or 0 when [autosimplify](#autosimplify) is active.
* For a list and a positive integer, repeat the list For example, `{ A } 3 *`
  is `{ A A A }`.
* For a text and a positive integer, repeat the text. For example `"X" 3 * `
  gives `"XXX"`.


## Divide

Divide two values

`Y` `X` ▶ `Y÷X`

* For integer, build a fraction. For example `1 7 /` gives `1/7`.
* For fractional, decimal or complex numbers, this performs the
  expected numerical division. For example, `1. 2. /` is `0.5`.
* For vectors, divide individual elements. For example,
  `[ 1 2 3 ] [ 3 2 1 ] /` is `[ 1/3 1 3 ]`.
* For equations and symbols, build a ratio, eliminating division by one
  and division of 0 when [autosimplify](#autosimplify) is active.


## Pow

Raise to the power

`Y` `X` ▶ `Y^X`

* For integer, fractional, decimal or complex numbers, this raises the
  value in level 2 to the value in level 1. For example, `2 3 ↑` is `8`.
* For vectors, raise individual elements in the first vector to the power of the
  corresponding element in the second vector.
* For equations and symbols, build an expression, eliminating special cases
  when [autosimplify](#autosimplify) is active.


## xroot

Raise to the inverse power. `X Y xroot` is equivalent to `X Y inv pow`.

`Y` `X` ▶ `Y↑(1/X)`


## Floor

Largest integer less than the input


## Ceil

Smallest integer larger than the input


## IntPart

Integer part of a number


## FracPart

Fractional part of a number


## Abs

Return the absolute value for a real number.
Return the modulus for a complex number
Return the absolute value of elements for a matrix or a vector, which is different from HP calculators, see `NORM`


# Integer arithmetic and polynomials

This section documents integer arithmetic commands, including prime-related
operations. Some commands (e.g. MODSTO, GETPREC) are planned but not yet
implemented in DB48X.

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
Remainder of the integer division


## SQ
Square of the input


## Factorial
Factorial of a number


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

Sign of a number, -1, 0 or 1.

For complex numbers, returns a unit number on the unit circle with the same
argument as the original number.


## Percent

Percentage of a number

`Y` `X` ▶ `Y×(X÷100)`


## PercentChange

Percentage of change on a number

`Y` `X` ▶ `(X÷Y-1)×100`


## PercentTotal

Get percentage of a total

`Y` `X` ▶ `(X÷Y)×100`


## GCD
Greatest common divisor of two integers.

`Y` `X` `GCD` → `gcd(X,Y)`

The result is always non-negative. `gcd(0,0)` is `0`.


## LCM
Least common multiple of two integers.

`Y` `X` `LCM` → `lcm(X,Y)`

The result is always non-negative. If either argument is `0`, the result is `0`.


## IQUOT
Quotient of the integer division


## ADDTMOD
Addition operator MOD the current system modulo


## SUBTMOD
Subtraction operator MOD the current system modulo


## MULTMOD
Multiplication operator MOD the current system modulo


## PEval

Evaluate a polynomial at a point (Horner’s method).

`Poly` `X` ▶ `Value`

* `Poly` (level 2) is a coefficient **array**, **list**, **polynomial**, or
  univariate **expression** (same inputs as `PRoot`; converted to descending
  coefficients).
* `X` (level 1) is the evaluation point (any algebraic).
* Coefficients may be numbers or symbols (e.g. `[ 'A' 'B' 'C' ]` for
  `A·X²+B·X+C` in descending degree order).
* Returns `Value`, the polynomial evaluated at `X`.

```rpl
[ 1 -5 6 ] 2 PEval
@ Expecting 0
```

```rpl
'X^2-5*X+6' 3 PEval
@ Expecting 0
```

```rpl
'x↑4+2·x↑3-25·x↑2-26·x+120' 3 PEval
@ Expecting -48
```


## PCoef

Build the monic polynomial whose roots are the given values.

`Roots` ▶ `Poly` or `Coeffs`

* `Roots` is an array or list of algebraic values (integers, fractions,
  decimals, or complex numbers). Each entry is one root of the polynomial.
* Returns the monic polynomial ∏(x − rᵢ) over all roots rᵢ.
* With [NewStylePolynomials](#newstylepolynomials) active (default), the result
  is a **polynomial** object. Use `ToArray` to obtain coefficients in
  **descending degree order** (highest power first).
* With [CompatiblePolynomials](#compatiblepolynomials) active, the result is a
  coefficient **array** in that same order, as on HP calculators.
* Coefficient order matches `PEval`, `PDIV2`, and `PRoot`.

```rpl
CompatiblePolynomials
[ 2 -3 4 -5 ] PCoef
@ Expecting [ 1 2 -25 -26 120 ]
```

```rpl
NewStylePolynomials
[ 2 -3 4 -5 ] PCoef
@ Expecting x↑4+2·x↑3-25·x↑2-26·x+120
```

The compatible-mode vector is the coefficient list of
`x⁴+2x³−25x²−26x+120`. Use `PRoot` on that vector to recover the roots.


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
Smallest of 2 objects


## Max
Largest of 2 objects


## RND
Round a number to the given number of figures


## TRNC
Truncate a number to the given number of figures


## DIGITS
Extract digits from a real number


## PRoot

Find all roots of a polynomial given by its coefficients.

`Coeffs` ▶ `Roots`

* `Coeffs` is an array, list, polynomial, or univariate expression
  (same inputs as `PEval`; polynomials are converted via `ToArray`) whose
  coefficients are in descending degree order (same convention as `PEval`
  and `PCoef`). The leading coefficient must be non-zero; trailing zeros at the
  high end are ignored.
* Returns `Roots`, an array containing every root, sorted in ascending order.
* Degree is limited to 100. Coefficient elements must be algebraic (real or
  complex).

For low degrees, roots are found by closed forms (linear and quadratic). For
higher degrees, the implementation tries exact rational candidates (when the
constant term allows), then uses a numerical method (Laguerre’s method with
deflation) for remaining roots.

Numeric results are cleaned up using the same imprecision rules as the equation
solver: values within `SolverImprecision` of an integer are snapped to that
integer, and negligible imaginary parts are dropped.

```rpl
[ 1 2 -25 -26 120 ] PRoot
@ Expecting [ -5 -3 2 4 ]
```

```rpl
[ 1 -5 6 ] PRoot
@ Expecting [ 2 3 ]
```

To obtain coefficients from a symbolic expression in one variable, use
`ToPolynomial` and then `ToArray`, or use `Zeros` on the expression directly.
See [NewStylePolynomials](#newstylepolynomials) for how `PCoef` formats its
result.


## IsPrime

Test whether an integer is prime.

`n` ▶ `True` or `False`

* Returns `True` if the integer `n` is prime, `False` if composite.
* Accepts integer inputs, including bignums. The maximum size is controlled by
  the `MaxFactorsBits` setting.
* Uses trial division by small primes first, then Miller-Rabin for larger
  values.
* Deterministic for numbers up to about 82 bits; probabilistic beyond that with
  negligible error probability.

```rpl
17 IsPrime
@ Expecting True
```

```rpl
15 IsPrime
@ Expecting False
```

```rpl
561 IsPrime
@ Expecting False
```


## Factors

Decompose an integer into its prime factorization.

`n` ▶ `{ p₁ e₁ p₂ e₂ … }`

* Returns a list of alternating prime-exponent pairs: each prime factor followed
  by its multiplicity.
* Accepts integer inputs, including bignums. Zero and one return an empty list.
  The maximum size is controlled by the `MaxFactorsBits` setting.
* Uses trial division by small primes, then Pollard's Rho for larger factors.
* The `MaxFactorIterations` setting limits Pollard's
  Rho iterations per attempt; lowering it can avoid long runs on hard semiprimes.
* The product of all `pᵢ^eᵢ` equals the original number.

```rpl
12 Factors
@ Expecting { 2 2 3 1 }
```

```rpl
100 Factors
@ Expecting { 2 2 5 2 }
```


## NextPr

Return the smallest prime strictly greater than the input.

`n` ▶ `p`

* Returns the next prime number after `n`.
* Accepts integers ≥ 1. The maximum input size is controlled by the
  `MaxFactorsBits` setting.
* If no prime exists (e.g. search limit reached), returns an error.

```rpl
10 NextPrime
@ Expecting 11
```

```rpl
2 NextPrime
@ Expecting 3
```


## PrevPr

Return the largest prime strictly smaller than the input.

`n` ▶ `p`

* Returns the previous prime number before `n`.
* Accepts integers > 2 (no prime exists below 2). The maximum input size is
  controlled by the `MaxFactorsBits` setting.
* If no prime exists or the search limit is reached, returns an error.

```rpl
10 PreviousPrime
@ Expecting 7
```

```rpl
3 PreviousPrime
@ Expecting 2
```
