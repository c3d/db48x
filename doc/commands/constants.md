# Constants

Constants are defined by the `config/constants.csv`, and accessed using
the `ConstantsMenu` or the `CONST` command.

Library equations are defined by the `config/equations.csv`, and accessed using
the `EquationsMenu` or the `LibEq` command.

Library items are defined by the `config/library.csv`, and accessed using the
`Library` command or the `XLib` command.


## Constant

Returns the value of a constant from the constants library.
The name can be given as a symbol or as text.

`'c'` ▶ `299792458_m/s`


## LibraryEquation

Returns the value of a library equation from the equation library.
The name can be given as a symbol or as text.

`"RelativityMassEnergy"` ▶ `"'E=m*c^2'"`


## LibraryItem

Returns the value of a library item from the library.
The name can be given as a symbol or as text.

`'Dedicace'"` ▶ `"À tous ceux qui se souviennent de Maubert électronique"`


# Precision control

Scientific calculations lead to the numerical evaluation of expressions whose
result is given in the form of a [decimal number](#Decimal-number) for which
[scientific notation](#Entering-a-number-in-scientific-notation-with-_×10ⁿ_) is
often used.

Calculations must be made with sufficient [precision](#Precision) to avoid
rounding errors affecting the validity of the results sought. Since DB48x has
variable precision floating point, the default configuration of 24 digits is
more than enough for most scientific applications.

However, a numerical value `X`, whether a measurement result or a constant, is
most of the time known with limited precision. It is therefore provided with
either an absolute uncertainty `ΔX` which is here designated by standard
uncertainty represented by `UsX`, or, equivalently, with a relative uncertainty
`UrX=UsX/|X|`. This makes it possible to establish an interval (either of a
statistical nature for a given probability distribution or of extreme limits or
other estimations) for the values ​​of `Xval`: `Xval = X ± ΔX = X ± UsX` noted
also as `Xval = X @ (UrX*100)%`.

A constant like `G` when edited shows as `ⒸG`.
Its relative uncertainty `UrG` is shown as `ⓇG` on DB48x, whether on the stack
or while editing. Its standard uncertainty `UsG` is shown as `ⓈG`.

## Calculating uncertainty

The calculation of uncertainty in science (metrology) obeys a certain number of simple rules concerning the significant digits (SD):

### Rule 1

`UsX` and `UrX` must be rounded to count at most 2 SD.

### Rule 2

`UsX` and `UrX` are equivalent and are determined between them knowing the
central value `X`.

### Rule 3

A central value `X` must be rounded so that its last decimal place corresponds
to the last decimal place of `UsX`.

### Rule 4

The final result of a calculation (multiplicative, functional, etc.)  involving
several uncertain values ​​gives a result that cannot be more precise than the
least precise of the uncertain input values. Note that before rounding the
final result, the intermediate calculations can be done at maximum precision,
thus avoiding the accumulation of rounding errors.

### Rule 5

In the case of a sum (or difference) between two uncertain values,
rounding is carried out to the leftmost decimal position of the last
significant digit among the inputs of the final calculation.

### Uncertainty-related commands

DB48x benefits from several features that support uncertainty calculations, SD
display mode and manipulation:

* `SignificantDisplay` is a mode where values can be displayed with a given
  number of SDs regardless of the precision of the calculations.

* `SigDig` is a command (DB48x extension) that returns the number of significant
  digits of its input.

* `Trunc` is a command that truncates its input to a given number of SDs when
  given a negative precision (and to a given number of decimal digits when given
  a positive precision).

* `Round` is a command (from HP50g) that rounds its input in the same way
  `Trunc` truncates it.

If the uncertainty `UsX` and `UrX` result from a calculation, [Rule 1](#rule-1)
is easily implemented by `UsX=ROUND(UsX,-2)` and `UrX=ROUND(UrX,-2)`.

The following five commands are added as extensions of DB48x to support the
remaining 4 rules: `→Us`, `→Ur`, `StandardRound`, `RelativeRound` and
`PrecisionRound`.


## →Us

Calculate standard uncertainty.

This command calculates a standard uncertainty `UsX` given the relative
uncertainty `UrX` and the central value `X`. This implements [Rule 2](#rule-2)
for `UsX`.

```rpl
-3.141592654_m  0.000012  →Us
@ Expecting 0.00003 8 m
```

## →Ur

Calculate relative uncertainty.

This command calculates a relative uncertainty `UrX` given the standard
uncertainty `UsX` and the central value `X`. This implements [Rule 2](#rule-2)
for `UrX`.


```rpl
-3.141592654_m  0.000097_m  →Ur
@ Expecting 0.00003 1
```

## StandardRound

Round a value based on a standard uncertainty, implementing [Rule 3](#rule-3).

```rpl
-3.141592654_m  0.000045_m  StdRnd
@ Expecting -3.14159 3 m
```

To compute the correct rounding of `Mu='ⒸNA*Ⓒu'`:
```rpl
'ⒸNA*Ⓒu'  Duplicate  ⓇMu  →Us  StdRnd →Num
@ Expecting 1.00000 00010 5⁳⁻³ kg/mol
@ which is the correctly rounded value of ⒸMu
```


## RelativeRound

Round a value based on a relative uncertainty, implementing [Rule 2](#rule-2)
and [Rule 3](#rule-3).

```rpl
-3.141592654_m  0.000012  RelRnd
@ Expecting -3.14159 3 m
```

To calculate `UrMu=ⓇMu` and then the correct rounding of `Mu='ⒸNA*Ⓒu'`,
you can use the following code:
```rpl
'ⒸNA*Ⓒu'  Duplicate  ⓈMu →Ur RelRnd  →Num
@ Expecting 1.00000 00010 5⁳⁻³ kg/mol
@ which is the correctly rounded value of ⒸMu
```

To calculate `Urε₀=Ⓡε₀` and then the correct rounding of
`ε₀='CONVERT(1/(Ⓒc^2·Ⓒμ₀);1_F/m)'`:

```rpl
'CONVERT(1/(Ⓒc^2·Ⓒμ₀);1_F/m)'  Duplicate  Ⓢε₀  →Ur RelRnd  →Num
@ Expecting 8.85418 78188⁳⁻¹² F/m
@ which is the correctly rounded value of Ⓒε₀
```


## PrecisionRound

Round one value to the precision of another one.

This implements [Rule 4](#rule-4) and [Rule 5](#rule-5). The user has to judge
carefully to establish the respective role of `X` and `Y`. There is no automatic
use here, since it depends on the precise nature of the calculation.


```rpl
-3.141592654_m 0.000045_m  PrcRnd
@ Expecting -3.14159 3 m
```

```rpl
-3.141592654_m 0.00045  PrcRnd
@ Expecting -3.14159 m
```
