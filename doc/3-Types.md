# Types

DB48X, [like HP RPL](#rich-data-types), supports a wide variety of data types.


## Integers

The DB48X version of RPL distinguishes between integer values, like `123`, and
[decimal values](#decimal-numbers), like `123.` Integer values are represented
internally in a compact and efficient format, saving memory and making
computations faster. All values between -127 and 127 can be stored in two bytes.
All values between -16383 and 16383 in three bytes.

Integers can be [as large as memory permits](#big-integers). They represent
values in the mathematical sets known as ℕ (natural numbers, i.e. positive
integers) and ℤ (positive and negative integers).


## Big integers

The DB48X version of RPL can perform computations on arbitrarily large integers,
limited only by available memory, enabling for example the exact computation of
`100!` and making it possible to address problems that require exact integer
computations, like exploring the Syracuse conjecture.

## Fractions

Fractions represent a ratio between two integers, like `2/3`. They represent
values in the mathematical set known as ℚ. In DB48X, fractions are first-class
objects, allowing exact and relatively inexpensive operations in many common
cases.


## Decimal numbers

Decimal numbers are used to represent values with a fractional part.
DB48X supports three internal representations for decimal numbers:

* Hardware accelerated 32-bit IEEE-754 with a binary representation. This is
  similar to the `float` type in a programming language like C.

* Hardware accelerated 64-bit IEEE-754 with a binary representation. This is
  similar to the `double` type in a programming language like C.

* Software variable-precision decimal representation, which is much slower.
  The default configuration selects 24 digits of precision.

Decimal numbers of various size and formats can be mixed and matched
transparently. It is valid and safe to adjust `Precision` settings along a
computation to only pay the cost of increased precision where it matters.

For example, you can compute the following expression at various precisions:

```rpl
'(SQRT(2)-1)^10-(3363-2378*SQRT(2))' DUP
512 Precision EVAL 'Precision' PURGE
@ Expecting 5.99480 35⁳⁻⁵⁰⁹
```

In a calculator, decimal numbers are always part of the mathematical set known
as ℚ, and more precisely of a subset sometimes written as 𝔻. However, they are
often improperly considered as an approximation of the set of *real* numbers, ℝ.


### Variable-precision decimal

DB48X features a variable-precision decimal implementation of floating point,
like [newRPL](#newRPL-project).

The [Precision](#precision) command selects the precision for computations,
given as a number of digits.

Internally, computations are not done in base 10, but in base 1000, which allows
a more compact decimal representation. Each base-1000 "kigit" uses 10 bits in
memory. As a result:

* From a performance point of view, the cost of computations is determined by
  the `Precision` divided by 3 and rounded up to the next integer, corresponding
  to one new base-1000 kigit being added.

* From a memory point of view, `Precision` values that are multiples of 12 are
  optimal, because four base-1000 kigit require 40 bits, which line up perfectly
  with five 8-bit bytes. `Precision` values that are not multiples of 12 will
  contain a few unused bits.

The `typename` command returns `"decimal"` for variable-precision decimal
floating-point values.

When `HardwareFloatingPoint` is disabled, numerical values entered on the command
line or in a program are always stored internally with all the digits entered,
and only those. For example, if you enter a decial number with 3 digits or less,
it will only use 5 bytes:

```rpl
1.23 BYTES
@ Expecting 5
```

On the other hand, if you enter a constant with a high number of digits, then
all the digits will be preserved internally irrespective of the `Precision`
setting:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
BYTES
@ Expecting 37
```

Computations are performed according to the `Precision` setting, irrespective of
the precision of input values. For example, the following computation is
guaranteed to gives `0.` irrespective of `Precision`, even if one of the inputs
to `+` has a larger number of digits stored internally:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
DUP 0 SWAP - +
@ Expecting 0.
```

If desired, the larger number of digits in the user-entered constant can be
exploited by setting `Precision` before the digit-cancelling operation, as
shown below:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
DUP 0 SWAP -     @ Recompute value at 24 digits
60 Precision +   @ Computation at 60 digits
'Precision' PURGE
@ Expecting 3.38327 95028 8⁳⁻²⁴
```


### Binary floating-point

The 32-bit format offers a 7 digits mantissa and has a maximum exponent of 96.
The 64-bit format offers a 16 digits mantissa and has a maximum exponent of 384.

The benefits of the binary floating-point representation are:

* It delivers higher performance using hardware acceleration.
* It is compatible with the IEEE-754 representation used in most computers.
  If you need to replicate results computed by a desktop computer, this may be
  the best format to use for that reason.

The primary drawback of this representation is that it cannot represent some
decimal values exactly, in the same way as `1/3` cannot be represented exactly
in decimal. For example, `0.2` cannot be represented exactly using a binary
floating-point representation.

Using a binary format is not recommended if you need exact results on decimal
values, for example adding monetary amounts. As an illustration, if you enable
16-digit hardware binary floating-point and add `0.20` and `0.45`, the result is
`0.65000 00000 00000 022`. This is not a bug, but a limitation of the
floating-point representation. The same computation with `SoftwareFloatingPoint`
gives the exact expected result `0.65`.

The `typename` command returns `"hwfloat"` for 32-bit binary floating-point
numbers, and `"hwdouble"` for 64-bit binary floating-point numbers.

### HardwareFloatingPoint

This command enables IEEE-754 accelerated binary floating point. It is the
opposite of `SoftwareFloatingPoint`.

When this setting is active, a 32-bit IEEE-754 format is used for `Precision`
values below 7, and a 64-bit IEEE-754 format is used for `Precision` values
below 16. Variable-precision decimal computations are used for `Precision`
settings of 17 and higher.

The `HardwareFloatingPoint` setting is disabled by default because of the
inherent precision loss incurred by the binary format when dealing with decimal
numbers. For example, `0.2` cannot be represented exactly using a binary format.

### SoftwareFloatingPoint

This command disables accelerated binary floating point, and ensures that the
variable-precision decimal floating-point format is used even for `Precision`
values below 16.. This setting is the opposite of `HardwareFloatingPoint`.


## Based numbers

Based numbers are used to perform computations in any base. The most common
bases used in computer science, 2, 8, 10 and 16, have special shortcuts.
The [Bases Menu](#bases-menu) list operations on based numbers.

Like integers, based numbers can be [arbitrary large](#big-integers).
However, operations on based numbers can be truncated to a specific number of
bits using the [WordSize](#wordsize) setting. This makes it possible to perform
computations simulating a 16-bit or 256-bit processor.


## Boolean values

DB48X has two boolean values, `True` and `False`. These values are typically
returned by operations such as tests that return a truth value.

In addition, numerical values are interpreted as being `False` if the value is
0, and `True` otherwise. This applies to conditional tests, conditional loops,
and other operations that consume a truth value.

## Complex numbers

Complex numbers can be represented in rectangular form or polar form.
The rectangular form will show as something like `2+3ⅈ` on the display, where
`2` is the real part and `3` is the imaginary part. The polar form will show as
something like `1∡90°` on the display, where `1` is the modulus and `90°` is the
argument. The two forms can be mixed and matched in operations. The calculator
typically selects the most efficient form for a given operation.

Available operations on complex numbers include basic arithmetic, trigonometric,
logarithms, exponential and hyperbolic functions, as well as a few specific
functions such as [conj](#conj) or [arg](#arg). These functions are available in
the [Complex Menu](#complex-menu).


## Expressions

Algebraic expressions and equations are represented between quotes, for example
`X+1` or `A+B=C`. Many functions such as circular functions, exponential, logs
or hyperbolic functions can apply to algebraic expressions.

An expression that contains an equal sign, e.g. `sin X + 1 = cos X`, is called
an *equation*. It can be given as an argument to the solver.


## Lists

Lists are sequence of items between curly braces, such as `{ 1 'A' "Hello" }`.
They can contain an arbitrary number of elements, and can be nested.

Operations such as `sin` apply to all elements on a list.


## Vectors and matrices

Vector and matrices represent tables of numbers, and are represented between
square brackets, for example `[1 2 3]` for a vector and `[[1 2] [3 4]` for a 2x2
matrix.

Vector and matrices follow their own arithmetic rules. Vectors are
one-dimensional, matrices are two-dimensional. DB48X also supports tables with a
higher number of dimensions, but only offers limited operations on them.

DB48X implements vector addition, subtraction, multiplication and division,
which apply component-wise. Multiplication and division are an extension
compared to the HP48.

DB48X also implements matrix addition, subtraction, multiplication and
division. Like on the HP48, the division of matrix `A` by matrix `B` is
interpreted as left-multiplying `A` by the inverse of `B`.

As another extension, algebraic functions such as `sin` apply to all elements in
a vector or matrix in turn.


## Units

Unit objects represent values with an associated unit. They are represented
using the `_` operator, e.g. `1_km/s`, although on display this operator is
shown as a thin space, e.g. `1 km/s`.

Units as implemented in DB48X are modernized compared to what the HP48
implements, and differ from the HP RPL implementation in the following ways:

* Add [recent SI prefixes](https://www.nist.gov/pml/owm/metric-si-prefixes),
  Z (zetta), Y (yotta), R (ronna) and Q (quetta) for large scale,
  z (zepto), y (yocto), r (ronto) and q (quecto) for small scale.

* Take into account the impact on unit conversions of the
  [revised 2023 definition of the foot](https://www.nist.gov/pml/us-surveyfoot/revised-unit-conversion-factors).

* Use exact (fraction-based) conversions wherever possible. This notably matters
  for the conversions of pre-2023 US Survey units, where the ratio is
  `1_ft = 1200/3937_m`, which is not well represented using decimal values.

* Add computer-related units, like the `byte`, the `bit`, the `baud`, as well
  as a menu supporting these units.

* In order to support the computer-related units better, also recognize the
  [power-of-two variants](https://en.wikipedia.org/wiki/Kilobyte),
  e.g. `1_kiB` is `1024_B`. Also recogize the `K` prefix in addition to `k`.

### Units file

The built-in units can be overriden by your own set of units, which is defined
in a CSV file called `config/units.csv` in the calculator. CSV stands for "Comma
separated values, and is a common interchange format for spreadsheet data.

Here is an example of file that would let you have a units menu called `Money`
to convert between various monetary units:

```
"Money"
"USD", "1_USD"
"EUR", "1.07_USD"
"GBP", "1.24_USD"
"CAD", "0.73_USD"
"AUD", "0.65_USD"
"CHF", "1.11_USD"
```

* All values must be placed between quotes. Separators between text values are
  mostly ignored.

* Rows in a file containing a single value denote unit menus, unless the value
  begins with an `=` sign.

* Rows in a file containing two ore more values denote unit menu entries, which
  will be added to the previous menu.

* The first column in these rows give the name of the unit as shown in the menu.

* The second column in these rows gives the definition of the unit.

* The definition should be reduced to `=` if the first column contains what
  would be a valid unit expression. For example, to place `km/h` in a menu, use
  `"km/h", "="` since `km` can be deduced from existing unit `m` using the
  standard "kilo" unit prefix, and `h` is an existing unit.

A unit where the value is `1` of the same unit is a base unit. This is the case
for `USD` in the example above, which is considered the base units for monetary
exchanges. Units that refer to the same base unit can be converted with one
another. For example, you can convert between `GBP` and `AUD` because they both
have the same `USD` base unit.

The commands `ShowBuiltinUnits` and `HideBuiltinUnits` indicate if the built-in
uits should be shown after the units loaded from the file. The default is that
when a units file is present, the built-in units are hidden. This only affects
the menus. Built-in units can always be used in expressions if they are typed
manually. However, units loaded from file will be looked up first, so that a
built-in unit can be overriden by the units file, which can be useful if a
definition changes like the US Survey foot changed on January 1st, 2023.

If you build a units file, it is recommended that you do not exceed 17 unit
submenus so that all unit categories fit on a single screen.


### Cycle command customization

The menu name `"=Cycle"` is reserved to define sequences of units that the
`Cycle` command (bound to the _×10ⁿ_ key) will recognize as special. For
example, you can ensure that `mm` and `in` convert to one another as follows:

```
"=Cycle"
"in", "mm"
"mm", "in"
"USD", "EUR"
"EUR", "CHF"
"CHF", "USD"
```

If you do provide a `Cycle` customization for a unit, other normal behaviours of
the `Cycle` command for units are disabled, notably conversion between various
relevant scales and conversion between fractions and decimal. To force a
particular conversion to happen in decimal, you can override the definition of
the corresponding unit in the units file, for example:

```
"in",   "25.4_mm"
```


## Constants

Constant objects represent named values like Euler's constant `e`, the ratio
between circumference and diameter of a circle `π`, or the speed of light `c`.
They are represented by a name, and have an associated value.

Like units, there are some built-in constants, and additional constants can be
provided by a `config/constants.csv` file, which has the same format as
for the units file.

On the command-line, constant names are prefixed with _CST_, which is a way to
distinguish them from normal symbols.

You can edit the constants catalog by recalling its content on the stack using
`"config:equations.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:equations.csv" STO`.


## Infinite results

Some operations such as `1/0` or `tan 90 °` are said to produce an
*infinite result*. Like HP calculators, DB48X can either generate an error or
produce a special result in these cases.

* If the `InfinityValue` (-22) flag is clear, corresponding to the
  `InfinityError` setting, then the operation generates a
  `Division by zero` error. Note that the text of the error is different than
  for Hewlett-Packard calculators, which generate an `Infinite result` error.

* If the `InfinityValue` flag is set and `NumericalConstants` (-2) flag
  is clear, corresponding to the `SymbolicConstants` setting, then the operation
  generates the `∞` (infinity) constant with the appropriate sign for the
  result, and the `InfiniteResultIndicator` (-26) flag is set.

* If the `InfinityValue` flag is set and `NumericalConstants` flag is set,
  then the operation generates the numerical value associated to the `∞`
  constant with the appropriate sign for the result, and set the
  `InfiniteResultIndicator` flag.

By default, the numerical value of the `∞` constant is set to `9.99999E999999`,
which is significantly smaller than what would actually cause a numerical
[overflow](#overflow-and-underflow), but is easy to read. This value can be
changed in the `config/constants.csv` file.


## Overflow and underflow

There is a maximum representable value for [decimal numbers](#decimal-numbers).
This value is significantly larger than on HP calculators. Whereas HP RPL
implementations could not represent decimal numbers with an exponent bigger than
499 or smaller than -499, DB48X supports exponents ranging from -2^60 to 2^60
(±1 152 921 504 606 846 976).

An *overflow* happens if the result would have an exponent higher than the
maximum. An *underflow* happens if the result would have an exponent lower than
the minimum. Like HP calculators, DB48X can either generate an error or
produce a special result in these cases.

* If the `UnderflowValue` (-20) or `OverflowValue` (-21) is clear,
  corresponding to the `UnderflowError` or `OverflowError`
  setting, then the operation generates a `Positive numerical underflow`,
  `Negative numerical underflow` or `Numerical overflow` error depending on the
  computation. Note that the text of the error is different than for
  Hewlett-Packard calculators, which generate an `Overflow`, `Positive
  Underflow` or `Negative Underflow` error.

* If the `UnderflowValue` or `OverflowValue` is set,
  and `NumericalConstants` (-2) flag is clear, corresponding to the
  `SymbolicConstants` setting, then overflowing operations generate the `∞`
  (infinity) constant with the appropriate sign for the result, and underflowing
  operations generate a zero value. The operation also sets the
  `NegativeUnderflowIndicator` (-23), `PositiveOverflowIndicator` (-24) or
  `OverflowIndicator` (-25) flag.

* If the `UnderflowValue` or `OverflowValue` is set, and
  `NumericalConstants` flag is set, then overflowing operations generate the
  numerical value associated to the  `∞` constant, and underflowing operations
  generate a zero value. The operation also sets the
  `NegativeUnderflowIndicator` (-23), `PositiveOverflowIndicator` (-24) or
  `OverflowIndicator` (-25) flag.


## Undefined

Some operations such as `0/0` are *undefined*, meaning that there isn't a single
possible answer.

If the `UndefinedValue` flag is set, such operations return the constant
`?`, and further operations on the value will keep returning the same undefined
result.

If the `UndefinedValue` flag is is clear, which corresponds to `UndefinedError`
being set, such operations will generate an `Undefined operation` error.


## Library

The `Library` is a catalog of frequently used and rarely modified objects that
are stored on disk in the `config/library.csv` file.

You can edit it by recalling its content on the stack using
`"config:library.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:library.csv" STO`.


## Equations Library

The equations library is a catalog of common equations that are stored on disk
in the `config/equations.csv` file.

You can edit it by recalling its content on the stack using
`"config:equations.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:equations.csv" STO`.
