# Numerical integration

## Integrate

Perform a numerical integration of a function for a specified variable on a
numerical interval. For example `2 3 'X*(X-3)' 'X' Integrate` returns `-7/6`.

The function takes four arguments:

* The lower bound of the integration range
* The higher bound of the integration range
* The program or expression to evaluate
* The integration variable

### IntegrationImprecision

This setting defines the relative imprecision for the result with respect to the
[Precision](#precision) setting. The default value is `6`, which means that at
the default precision of `24` digits, `Integrate` will try to compute to an
accuracy of 18 digits.

This setting only applies if the result is smaller than the display
settings. Like HP calculators, the display settings limits the precision
requested from the integration algorithm. For example, if the display is set to
`3 FIX`, then only 3 digits of precision are considered necessary in the result.

### IntegrationIterations

This setting limits the number of iterations for the integration algorithm. Each
iteration requires the evaluation of twice as many samples of the function to
integrate as the previous one, so the maximum number of samples taken is in the
order of `2^IntegrationIterations`.


# Numerical conversions

## →Num

Convert fractions and symbolic constants to decimal form.
For example, `1/4 →Num` results in `0.25`.

## →Q

Convert decimal values to fractions. For example `1.25 →Frac` gives `5/4`.
The precision of the conversion in digits is defined by
[→FracDigits](#ToFractionDigits), and the maximum number of iterations for the
conversion is defined by [→FracDigits](#ToFractionIterations)

## →Qπ

Convert decimal values to a rational form, or a rational form with π, square
roots, natural logs, or the Euler constant *e* factored out, whichever yields
the smaller denominator.

The rational result is a "best guess", since there might be more than one
rational expression consistent with the argument. `→Qπ` finds a quotient of
integers that agrees with the argument to the number of decimal places specified
by the display format mode.

For example, `3.14159265359 →Qπ` gives `π`, `1.4142135624 →Qπ` gives `√2`,
and `0.346573590280 →Qπ` gives `ln 2/2`.

For a complex argument, the real or imaginary part (or both) can have a constant
factor.

The [→QπMaxPrime](#→qπmaxprime) setting (default 100, max 10000)
limits which primes are tried when factoring squares for √*n* detection. Lower
values speed up conversion on resource-constrained hardware.

The algorithm attempts: π; any √*n* (by squaring and factoring); ln 2, ln 3,
ln 5, ln 7, ln 10; *e*; *e*^(p/q); and combined factors π·√2, π·√3.

## →Integer

Convert decimal values to integers. For example `1. →Integer` gives `1`.
This command intentionally fails with `Bad argument value` if the input contains
a non-zero fractional part.

## XQ

Find the simplest symbolic representation of a real number. Given a decimal
value, `XQ` returns the expression with the smallest denominator that matches
the input within the precision defined by [→QDigits](#ToFractionDigits). It
tries the following forms in order of preference:

* **rational** `p/q`: e.g. `1.25 XQ` gives `5/4`
* **square root** `√(p/q)`: e.g. `1.41421356237 XQ` gives `√2`
* **pi multiple** `p/q·π`: e.g. `4.71238898038 XQ` gives `3/2·π`
* **natural logarithm** `ln(p/q)`: e.g. `0.405465108108 XQ` gives `ln(3/2)`
* **exponential** `e^(p/q)`: e.g. `1.10517091808 XQ` gives `e^(1/10)`

If no symbolic form is found within the precision threshold, `XQ` returns the
best rational approximation `p/q`.

`XQ` accepts integers, fractions (returned unchanged), decimals, and lists
(applied element-wise).


# Continued fractions

A continued fraction represents a real number as a sequence of integers
`{ a0, a1, a2, ... }` such that:

    x = a0 + 1/(a1 + 1/(a2 + 1/...))

The integer part `a0` may be negative; all remaining coefficients `a1, a2, ...`
are positive. Rational numbers produce finite sequences; irrational numbers
produce infinite (periodic for quadratic surds) sequences that are truncated at
the current precision.

## DFC

Decompose a real number into its continued fraction coefficients. Returns a list
`{ a0, a1, a2, ... }` of integers satisfying the continued fraction expansion
above.

For exact inputs (integers, fractions), the result is the exact finite CF:

* `3/7 DFC` gives `{ 0 2 3 }` since 3/7 = 0 + 1/(2 + 1/3)
* `22/7 DFC` gives `{ 3 7 }` since 22/7 = 3 + 1/7
* `355/113 DFC` gives `{ 3 7 16 }`

For decimal inputs, the sequence is truncated when the remaining denominator
exceeds 10^(Precision/2), giving convergents that match the input to the
available precision:

* `π DFC` begins `{ 3 7 15 1 292 1 1 1 2 ... }`
* `√2 DFC` begins `{ 1 2 2 2 2 2 2 2 2 ... }` (periodic)
* `φ DFC` begins `{ 1 1 1 1 1 1 1 1 1 ... }` (golden ratio, all 1s)

## DFC2F

Reconstruct a number from a continued fraction coefficient list. Given a list
`{ a0, a1, ..., an }`, evaluates the finite continued fraction from the inside
out: `a0 + 1/(a1 + 1/(... + 1/an))`.

When all coefficients are integers the result is an exact fraction:

* `{ 0 2 3 } DFC2F` gives `3/7`
* `{ 3 7 } DFC2F` gives `22/7`
* `{ 3 7 16 } DFC2F` gives `355/113`

`DFC` and `DFC2F` are inverses: `DFC2F(DFC(x))` recovers `x` exactly for
rational inputs and to full precision for decimals.
