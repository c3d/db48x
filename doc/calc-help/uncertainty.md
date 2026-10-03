# Uncertainty

The Uncertainty section of the Function Library works on measured values and
their uncertainties. The calculator already knows four ways of writing them,
and they do not say the same thing:

* **bounds** — `a…b`, `a±b`, `a±p%`: the true value lies somewhere in the
  interval, for certain. No distribution is assumed; this is what a tolerance,
  a resolution or a safety margin gives.
* **statistics** — `a±σb`: the value is normally distributed with standard
  deviation `b`. This is what a repeated measurement or a GUM uncertainty
  budget gives.

The core arithmetic already propagates both through a single operation. This
section adds what the core leaves to the user, starting with the rounding of a
result for presentation.


## UncertaintyLibrary

Tools for measured values and their uncertainties, whether written as bounds
(`a…b`, `a±b`, `a±p%`) or as a standard deviation (`a±σb`).

* [Rounding](#roundinglibrary) — present a result with the digits its
  uncertainty justifies: [SciRngRnd](#scirngrnd), and as text
  [SciRngText](#scirngtext) (`12.35±0.10`) or
  [SciRngParen](#scirngparen) (`12.35(10)`).
* [TypeB](#typeblibrary) — turn a resolution, a tolerance or a stated
  uncertainty into a standard uncertainty: [ResN0→σR](#resn0→σr) guides
  the choice step by step, [Bound→σ](#bound→σ) and [ResN0](#resn0) do the
  same work from the stack.
* [Compare](#comparelibrary) — do two measurements agree? [ΔConcord](#Δconcord)
  for bounds, [σConcord](#σconcord) for standard deviations, [RngRel](#rngrel)
  for how two intervals sit on the line.
* [Functions](#functionslibrary) — propagate uncertainties through a function
  written with x, or x1, x2…, followed by its values: [σRf](#σrf) and
  [σRFxjxi](#σrfxjxi), and for intervals [ΔRf](#Δrf), [ΔRFxjxi](#Δrfxjxi) and
  [Exmnf](#exmnf).
* [Monte Carlo](#monte-carlolibrary) — propagate uncertainties through any
  model by random draws, with any distribution for each input:
  [MCPropagate](#mcpropagate).


## RoundingLibrary

A result such as `12.34563±0.02347` claims more than anyone measured: the last
digits of the value lie far below its own uncertainty. The rule followed here is
the one NIST and the GUM (JCGM 100:2008, §7.2.6) recommend:

* round the **uncertainty to two significant digits**;
* round the **value to the same decimal place** as the rounded uncertainty.

So `12.34563±0.02347` becomes `12.346±0.023`, and `123456.7±789.1` becomes
`123460±790`. Two digits rather than one because rounding 0.15 to one digit
would change it by a third.

The value is rounded after the uncertainty, since rounding the uncertainty can
move it to the next decade: 0.0996 becomes 0.10, which fixes the value to two
decimals, not three.

A zero is significant in these results: `12.35±0.10` says the uncertainty is
known to the hundredth. A number cannot carry that zero — 0.10 and 0.1 are the
same number to the calculator. [SciRngRnd](#scirngrnd) returns numbers to go
on computing with; [SciRngText](#scirngtext) returns the same result written as
text, zeros included, for a report or a label; [SciRngParen](#scirngparen)
writes it in the concise notation of the GUM and CODATA, `12.35(10)`.

After Jean Wilson's SciRngRnd (2025–2026).


## SciRngRnd

Rounds a measured value and its uncertainty by the NIST rule: the uncertainty
to two significant digits, the value to the same decimal place. See
Rounding for the rule and why.

It accepts every interval type and returns the same type, units included:

* `a±b`, `a±σb` — the uncertainty is `b`;
* `a…b` — the uncertainty is the half-width, and the bounds are rounded through
  the centre;
* `a±p%` — the value is rounded by its absolute uncertainty, and the percentage
  itself to two significant digits;
* two values `X` `U` on the stack — read as `X±U`. If both carry units, `U` is
  first expressed in the unit of `X`.

An uncertainty of zero leaves the value unchanged: nothing then says which
digits are meaningful.

Stack: an interval, or `X` `U`.

```rpl
12.34563±0.02347 ⓁSciRngRnd
@ Expecting 12.346±0.023
```

```rpl
12.3456±σ0.0234_m ⓁSciRngRnd
@ Expecting 12.346±σ0.023 m
```

```rpl
12.3456±1.5% ⓁSciRngRnd
@ Expecting 12.35±1.5%
```

A value in metres and an uncertainty in centimetres:

```rpl
12.3456_m 2.34_cm ⓁSciRngRnd
@ Expecting 12.346±0.023 m
```

When rounding the uncertainty reaches the next decade, the value follows it —
and the result shows why [SciRngText](#scirngtext) exists: the uncertainty
0.10 is displayed as 0.1.

```rpl
12.3456±0.0996 ⓁSciRngRnd
@ Expecting 12.35±0.1
```

See also: [Rounding](#roundinglibrary).

## SciRngText

The result of SciRngRnd written as text, with the trailing zeros
that the rounding makes significant and that a number cannot carry. The display
settings are not changed. Use it for a report, a label or a printed table; use
SciRngRnd to go on computing. Without a unit, the text reads back as a number
with STR→; the unit is written after a plain space, as in a report, and does
not read back.

Stack: an interval, or `X` `U`, as for SciRngRnd.

```rpl
12.3456±0.0996 ⓁSciRngText
@ Expecting "12.35±0.10"
```

```rpl
12.2…12.5678 ⓁSciRngText
@ Expecting "12.20…12.56"
```

```rpl
2±0.0234 ⓁSciRngText
@ Expecting "2.000±0.023"
```

```rpl
12.3456±σ0.0996_m ⓁSciRngText
@ Expecting "12.35±σ0.10 m"
```

See also: [SciRngRnd](#scirngrnd).

## SciRngParen

The result of SciRngRnd in concise notation, the form the GUM
(JCGM 100:2008, §7.2.2) and the CODATA tables use: the digits in parentheses
are the uncertainty, in units of the last digit of the value. `1.456±0.023` is
written `1.456(23)`; the Newtonian constant of gravitation appears in the
CODATA tables as `6.67430(15)×10⁻¹¹`.

When the uncertainty reaches the tens, the last digits of the value are no
longer significant and `123460(79)` would be ambiguous; a power of ten then
keeps it clear, `1.2346(79)×10⁵`. A zero uncertainty gives the value alone.

**By convention the parentheses denote a standard uncertainty, σ.** For `a±σb`
the notation says exactly what the value is. For bounds, `a±b`, `a…b` or `a±p%`,
the entry writes the half-width in the same way, but a reader will take it for a
standard deviation: say so next to the value, or write it with
SciRngText instead.

The result is text, as for SciRngText; the display settings are not changed.

Stack: an interval, or `X` `U`, as for SciRngRnd.

```rpl
1.456±σ0.023 ⓁSciRngParen
@ Expecting "1.456(23)"
```

```rpl
6.67430±σ0.00015 ⓁSciRngParen
@ Expecting "6.67430(15)"
```

```rpl
12.3456±σ0.0996_m ⓁSciRngParen
@ Expecting "12.35(10) m"
```

```rpl
123456.7±σ789.1 ⓁSciRngParen
@ Expecting "1.2346(79)×10⁵"
```

See also: [SciRngRnd](#scirngrnd), [SciRngText](#scirngtext).

## TypeBLibrary

A standard uncertainty obtained otherwise than from the scatter of repeated
readings is what the GUM calls a **Type B** evaluation (JCGM 100:2008, §4.3):
the resolution of a display, the divisions of a scale, a manufacturer's
tolerance, an uncertainty quoted in a certificate. What one knows is a bound;
what the propagation needs is a standard deviation. Going from one to the other
takes a law: the bound says where the value can be, the law says how it is
spread inside.

* [ResN0→σR](#resn0→σr) asks for the reading, where the bound comes from, and
  the law, then returns `x±σu`.
* [Bound→σ](#bound→σ) does the conversion from the stack, for programs.
* [ResN0](#resn0) finds the resolution of a reading from its significant zeros.

Every factor comes from the Probability section: the standard deviation of
the chosen law on the bounds (its Sx entry), or the quantile that turns a
confidence level into a coverage factor (its Q entry). The rectangular law gives
the familiar a/√3, the triangular a/√6, the U-shaped a/√2; a normal at 95 %
gives U/1.96, a Student law with 10 degrees of freedom U/2.228.

A digital display that shows the last digit δ does not say the value is within
±δ, but within **±δ/2**: the reading was rounded. The standard uncertainty is
then δ/√12, not δ/√3.

After Jean Wilson's ResN0→σR (2025–2026).


## ResN0→σR

A guided conversion of a reading into a value with its standard uncertainty.
It asks three questions. A number already on the stack is offered as the
answer to the first, to complete or confirm with ENTER.

1. **The value, as the instrument shows it.** Type it with its zeros and its
   unit — `9.000_kg`, `16.00_mm`, `7.00E3`. The calculator would store 9.000 as
   9; typed here, it is kept as text, so that the zeros that were measured are
   counted. Only an integer such as `7000` is ambiguous: the screen then asks
   how many of its trailing zeros are significant, and a key from 0 to 3
   answers. Typing `7.000E3`, or `7000.` with a point, says it in advance.
2. **Where the bound comes from** — a key from 1 to 4: a digital display
   (±half the last digit), a graduated scale (±half the smallest division,
   which it asks for), a stated tolerance ±a (or two values when the bounds
   are unequal), or an expanded uncertainty ±U at a confidence level p.
3. **The law** — a key from 0 to 9, among those that fit: inside bounds,
   rectangular, triangular, triangular with another mode, U-shaped, beta,
   or a normal truncated at the bounds; for an expanded uncertainty, normal,
   Student, Rayleigh or log-normal. It asks for the parameters a law needs.

It returns `x±σu`, where x is the mean of the law — the reading itself, except
for the asymmetric ones. Any other key cancels.

The same work, without the questions, is done by [ResN0](#resn0) and
[Bound→σ](#bound→σ).


## Bound→σ

Standard uncertainty from a bound, for a given law — the calculation behind
ResN0→σR, for programs.

Stack: `X`, `A`, `law`, `P`.

* `X` — the value, with or without unit.
* `A` — the half-width a of the bounds, or `{ a- a+ }` when they are unequal
  below and above X; in any unit compatible with X. For laws 7 to 0, the
  expanded uncertainty U.
* `law` — 1 rectangular, 2 triangular, 3 triangular with mode, 4 U-shaped,
  5 beta, 6 truncated normal, 7 normal, 8 Student, 9 Rayleigh, 0 log-normal.
* `P` — a list of the parameters the law needs: `{ mode }` for 3, `{ α β }` for
  5, `{ k }` for 6 (the bounds at k standard deviations, 2 by default),
  `{ p }` for 7, 9 and 0, `{ ν p }` for 8. The level p is 0.95 by default and
  the mode is X. Give `{ }` when nothing is needed.

Laws 7 and 8 read ±U as a two-sided interval holding the probability p, as a
certificate does; laws 9 and 0 read X+U as the one-sided quantile p, the value
being a magnitude, or positive.

A display reading 9.000 kg: the bound is ±0.0005 kg, and a rounded reading is
rectangular:

```rpl
9._kg 0.0005_kg 1 { } ⓁBound→σ
@ Expecting 9.±σ0.00028 86751 35 kg
```

A certificate gives ±0.2 at 95 % with 10 degrees of freedom:

```rpl
10. 0.2 8 { 10 0.95 } ⓁBound→σ
@ Expecting 10.±σ0.08976 10127 94
```

Unequal bounds move the value to the middle of the interval:

```rpl
10. { 0.1 0.3 } 1 { } ⓁBound→σ
@ Expecting 10.1±σ0.11547 00538 38
```

See also: [ResN0→σR](#resn0→σr).

## ResN0

The resolution of a reading: the unit of its last significant digit, when N of
its trailing zeros are significant. A calculator stores 9.000 kg as 9 kg; N
says how many zeros the instrument actually showed.

Stack: `X`, `N` — or `{ X N }`.

```rpl
9._kg 3 ⓁResN0
@ Expecting 0.001 kg
```

```rpl
7000. 2 ⓁResN0
@ Expecting 10.
```

A value of zero has no significant digit to count from: type its resolution
directly.

## CompareLibrary

Do two measurements of the same quantity agree? The question has two forms,
and they must not be confused.

* With **bounds** — `a…b`, `a±b`, `a±p%` — the true value is somewhere inside
  each interval, so the question is geometric: do the intervals meet, and by
  how much? That is [ΔConcord](#Δconcord), and [RngRel](#rngrel) for the bare
  relation.
* With **standard deviations** — `a±σb` — the bars are not bounds: the value is
  outside ±σ one time in three. The question is statistical: is the difference
  D = Y − X compatible with zero? That is [σConcord](#σconcord).

Reading ±σ bars as bounds is a classic error. 10±σ1 and 12.2±σ1 do not touch,
yet the difference is only 1.56 standard deviations of D, and p = 0.12: the two
measurements are compatible at any usual threshold. Two ±σ bars that just touch
give p = 0.16; incompatibility at 5 % needs a gap of about 0.77σ between the
bars. This is why ΔConcord refuses `a±σb` values, and σConcord refuses bounds.

After Jean Wilson's Concord (2025).


## ΔConcord

Draws two bounded intervals X and Y on the same scale, with their intersection,
and states how much of each it covers and how they sit on the line. Y is first
expressed in the unit of X.

Stack: `X`, `Y` — each `a…b`, `a±b`, `a±p%` or a plain number.

It returns, tagged: `X∩Y` (a range, or `"∅"` when they do not meet), the part of
X and the part of Y that the intersection covers, in per cent, and the relation
as RngRel names it. The drawing stays on screen until a key is
pressed.

```rpl
1…3 2…4 ⓁΔConcord
```

It returns `X∩Y:2…3`, `X%:50`, `Y%:50` and `rel:"X overlaps Y"`.

A mass measured to ±0.0023 g, against a reference given to ±0.005 g, in grams
both:

```rpl
5.3617±0.0023_g 5.360±0.005_g ⓁΔConcord
```

The first interval lies inside the second: `X∩Y:5.3594…5.364 g`, `X%:100`,
`Y%:46`, `rel:"X containedBy Y"`.


Random pairs make a good exercise. This draws two intervals between −100 and
100 and compares them; run it several times:

```rpl
-100 100 RANDOM -100 100 RANDOM DUP2 MIN UNROT MAX →Range
-100 100 RANDOM -100 100 RANDOM DUP2 MIN UNROT MAX →Range
ⓁΔConcord
```

See also: [RngRel](#rngrel).

## σConcord

Are two measurements X = a±σb and Y = c±σd compatible? The answer is read on
their difference D = Y − X, normal with mean d = c − a and standard deviation

σD = √(σX² + σY² − 2ρ·σX·σY)

where ρ is the global variable `ρ` that the arithmetic of the calculator also
uses for correlated values (0 when it does not exist). The entry draws the law
of D, marks zero, and shades the two tails beyond ±|d| around d: their area is
the p-value.

Stack: `X`, `Y` — each `a±σb` or a plain number, taken as exact.

It returns, tagged: `d`, `σD`, `z` = |d|/σD, and `p` = 2·UTPN(0,1,z), the
probability of a difference at least this large if the two measure the same
thing. There is no verdict: the threshold belongs to the user. A small p
says the measurements disagree; a large p does not prove they agree — it says
the data cannot tell them apart.

```rpl
10±σ1 12.2±σ1 ⓁσConcord
```

It returns `d:2.2`, `σD:1.41421 35623 7`, `z:1.55563 49186 1` and
`p:0.11979 49304 26`.

The two ±σ bars do not touch, yet p = 0.12. With a correlation of 0.5, the
difference is better known and the same gap weighs more:

```rpl
0.5 'ρ' STO 10±σ1 12.2±σ1 ⓁσConcord
```

Now `σD:1`, `z:2.2` and `p:0.02780 68950 27`. Purge `ρ` afterwards, or it will
also weigh on the arithmetic of every uncertain value.


## RngRel

How two bounded intervals X and Y sit on the line: one of the thirteen cases of
Allen's interval algebra, which the IEEE 1788-2015 standard for interval
arithmetic calls the *overlap* state, returned as a sentence that says which
way it goes: `"X overlaps Y"`, `"X metBy Y"`. Exactly one case holds for any
pair, which avoids the ambiguity of < between intervals.

| Case | X = [a₁, a₂], Y = [b₁, b₂] |
|---|---|
| before | a₂ < b₁ |
| meets | a₂ = b₁ |
| overlaps | a₁ < b₁ < a₂ < b₂ |
| starts | a₁ = b₁, a₂ < b₂ |
| containedBy | b₁ < a₁, a₂ < b₂ |
| finishes | b₁ < a₁, a₂ = b₂ |
| equals | a₁ = b₁, a₂ = b₂ |
| finishedBy | a₁ < b₁, a₂ = b₂ |
| contains | a₁ < b₁, b₂ < a₂ |
| startedBy | a₁ = b₁, b₂ < a₂ |
| overlappedBy | b₁ < a₁ < b₂ < a₂ |
| metBy | a₁ = b₂ |
| after | b₂ < a₁ |

A point interval takes the first case that matches, in the order equals,
before, after, starts, finishes, meets, then the others.

Stack: `X`, `Y` — each `a…b`, `a±b`, `a±p%` or a plain number; Y is expressed in
the unit of X. `a±σb` values are refused, as for ΔConcord.

```rpl
1…3 2…4 ⓁRngRel
@ Expecting "X overlaps Y"
```

```rpl
1…3_m 100…300_cm ⓁRngRel
@ Expecting "X equals Y"
```

```rpl
2 1…3 ⓁRngRel
@ Expecting "X containedBy Y"
```


## Monte CarloLibrary

When the model is not linear, when the inputs are not normal, or when the
result is not symmetric, the usual propagation formula is only an
approximation. The Monte Carlo method draws values of the inputs at random,
computes the model for each draw, and reads the result on the values obtained.

* [MCPropagate](#mcpropagate): the mean, the standard uncertainty and the 95 %
  coverage interval of a model of several inputs, each with its own
  distribution.

## MCPropagate

Propagation of uncertainty by the Monte Carlo method, as described in
Supplement 1 of the GUM (JCGM 101:2008).

This is a light version of what the NIST Uncertainty Machine does. That
machine draws from 100 000 to 5 million values, which a calculator cannot do.
MCPropagate is made for a few thousand draws, best run on the simulator, on a
phone or on a computer. In return, each input can follow any of the 30
distributions of the Probability section, where the Uncertainty Machine
offers 16.

Stack: F Vars Vals M ▶ N, the 95 % interval, Y±σu. F is the model, an
expression of the names listed in Vars, or a program that takes one value per
input from the stack. Vals lists the inputs in the same order, and M is the
number of draws.

The type of each input tells its distribution:

* a standard deviation, like 10±σ1: normal;
* bounds, like 9…11, 10±1 or 10±10%: rectangular between the bounds, the usual
  choice for a Type B evaluation;
* a program that returns one draw, like « 10 1 ⓁLgNrmRand »: any distribution;
  every distribution of the Probability section has such a Rand function;
* a plain number: an exact value.

Inputs may carry units. The inputs are independent: correlations are not
handled yet.

The results are the number of draws used, tagged N; the interval that leaves
2.5 % of the draws on each side, tagged 95%; and on the first level the mean
and the standard deviation of the draws, as Y±σu.

With M = 0, MCPropagate stops by itself when the mean and the uncertainty are
stable to two significant digits of the uncertainty, after 20 000 draws, or
after one minute, whichever comes first. A distribution with a long tail may
never be stable: N then tells where it stopped.

Each run gives a slightly different result, and this is how to judge its
stability. The examples below first set the seed of the random generator, so
that they always give the same result; leave that line out for real use.

**Example 1.** One distribution alone: the model is the variable itself. A
rectangular distribution between 9 and 11 has a mean of 10 and a standard
deviation of 1/√3 = 0.577:

```rpl
12345 RDZ
'x' { x } { '9…11' } →Num 2000 ⓁMCPropagate
@ Expecting 10.02378 18042±σ0.58487 21349 4
```

**Example 2.** The sum of two normal inputs. The exact result is 30±σ2.236:

```rpl
12345 RDZ
'a+b' { a b } { 10±σ1 20±σ2 } 2000 ⓁMCPropagate
@ Expecting 30.12500 40006±σ2.24588 10666 3
```

**Example 3.** The product of a rectangular input and a normal one:

```rpl
12345 RDZ
'a*b' { a b } { '9…11' 10±σ1 } →Num 2000 ⓁMCPropagate
@ Expecting 100.26278 5406±σ11.39132 08051
```

**Example 4.** Inputs with units: a resistance from a voltage and a current.

```rpl
12345 RDZ
'V/I' { V I } { '10±σ0.1_V' '5±σ0.05_A' } →Num 2000 ⓁMCPropagate
@ Expecting 1.99888 76261 4±σ0.02860 28043 8 V/A
```

**Example 5.** A model that is not linear. For the square of a normal input
centered on zero, the propagation formula gives 0±σ0, since the derivative is
zero there. The true mean is 1, the standard deviation 1.414, and the interval
is far from symmetric:

```rpl
12345 RDZ
'x^2' { x } { 0±σ1 } 2000 ⓁMCPropagate
@ Expecting 0.97201 48896 25±σ1.40367 28151 1
```

**Example 6.** Any distribution, through its draw program: here a lognormal
distribution. The draw programs are slower than the normal and rectangular
inputs, about 60 draws per second on the simulator:

```rpl
12345 RDZ
'x' { x } { « 0 1 ⓁLgNrmRand » } 500 ⓁMCPropagate
@ Expecting 1.88793 72628 2±σ2.38766 59582 6
```

The same distribution is obtained much faster as the exponential of a normal
input, with the model 'exp(x)' and the input 0±σ1.

**Example 7.** Automatic stop, with M = 0. Here the result is stable after
8 500 draws:

```rpl
12345 RDZ
'a+b' { a b } { 10±σ1 20±σ2 } 0 ⓁMCPropagate
@ Expecting 30.03900 15233±σ2.25391 96331 3
```

A model given as a program takes its values from the stack, and the list of
names is empty: « * » { } { 10±σ1 '4±0.5' } →Num 1000 ⓁMCPropagate.

A model that contains a constant, like 'm*Ⓒg*h', is evaluated by
substitution, which is slower.

See also: the Rand functions of the Probability section, and the Rounding
functions to present the result.


## FunctionsLibrary

Functions of uncertain values and of intervals, without any list to build:
the function is written with the names x or x1, x2, …, and its values follow
it on the stack. Standard uncertainties are propagated by the law of
propagation of the GUM (JCGM 100:2008, section 5), intervals by a search of
the global minimum and maximum of the function.

* [σRf](#σrf): a composite function of one variable, f(x);
* [σRFxjxi](#σrfxjxi): a function of several variables, F(x1, x2, …),
  possibly correlated;
* [ΔRf](#Δrf): the range of a function of one variable over an interval;
* [ΔRFxjxi](#Δrfxjxi): the range of a function of several variables over a
  box;
* [Exmnf](#exmnf): the drawing of a function over an interval, to check ΔRf.

Evaluating a function directly on uncertain numbers treats each occurrence of
a variable as a new, independent variable: x·x does not get the uncertainty
of x², and (x−1)·(x−2)·(x−3) gets twice its uncertainty. These functions count each
variable once.

The law of propagation is a first order approximation, good when the function
is close to linear over the uncertainties. Otherwise, the Monte Carlo method of
[MCPropagate](#mcpropagate) gives the reference result; comparing the two is
the check recommended by Supplement 1 of the GUM.

After the σRf, σRFx2x1, σRFxjxi, ΔRf and Exmnf functions of Jean Wilson's Proposition for
interval implementation in the RPL environment (2025).


## σRf

The uncertainty of a composite function of one variable.

Stack: 'f(x)' X ▶ Y±σu, where f is an expression of the name x and X its
value a±σb, with or without units.

The contribution of x is the derivative of f times the standard deviation,
obtained by five point central differences on the value with its unit, with a
step of σ/1000: it comes out in the unit of f, works with any function,
constant or unit, and also at x = 0.

**Example 1.** The gamma function at 7±σ0.05. The exact uncertainty is
Γ(7)·ψ(7)·0.05 = 720 × 1.87278 43351 × 0.05 = 67.42023 60636, where
ψ(7) = −γ + 1 + 1/2 + 1/3 + 1/4 + 1/5 + 1/6 is the digamma function:

```rpl
'tgamma(x)' 7.0±σ0.05 ⓁσRf
@ Expecting 720.±σ67.42023 60635
```

**Example 2.** A polynomial near one of its roots. Evaluated directly on
2.755±σ0.05, it gets about twice this uncertainty, since its three factors are
treated as independent:

```rpl
'(x-1)*(x-2)*(x-3)' 2.755±σ0.05 ⓁσRf
@ Expecting -0.32463 1125±σ0.03550 375
```

**Example 3.** The difference of a quantity with itself is exactly zero:

```rpl
'x-x' 2.0±σ0.05_m ⓁσRf
@ Expecting 0±σ0. m
```


## σRFxjxi

The uncertainty of a function of several variables.

Stack: 'F' Xn … X2 X1 ▶ Y±σu, where F is an expression of the names x1, x2, …
xn, and each Xj is the value of xj: a±σb, or a plain number for an exact
value, with or without units. X1 is on the first level, as in a formula read
from the right.

The variables are independent, unless a global variable ρij holds either one
correlation coefficient for all the pairs, or the full correlation matrix, in
the order x1 … xn. The contributions cj, derivative times standard deviation,
are combined as u² = Σ ci·cj·ρij. The variable ρij stays in the current
directory and applies to the next calls: purge it when done, as the examples
do.

**Example 1.** A mass from a linear density and a length, with units:

```rpl
'x2*x1' 2.5±σ0.5_g/cm 2.0±σ0.2_cm ⓁσRFxjxi
@ Expecting 5.±σ1.11803 39887 5 g
```

**Example 2.** Three lengths fully correlated, for instance measured with the
same rule: the uncertainties add instead of combining in quadrature.

```rpl
1 'ρij' Sto
'x3+x2+x1' 1.2±σ0.002 2.5±σ0.005 1.600±σ0.012 ⓁσRFxjxi
'ρij' Purge
@ Expecting 5.3±σ0.019
```

**Example 3.** Two variables correlated at 0.5, given as a matrix: the
uncertainty is √(1² + 2² + 2·0.5·1·2) = √7:

```rpl
[[ 1 0.5 ] [ 0.5 1 ]] 'ρij' Sto
'x2+x1' 10±σ1 20±σ2 ⓁσRFxjxi
'ρij' Purge
@ Expecting 30±σ2.64575 13110 6
```

**Example 4.** The potential energy of a mass raised along a slope, with a
constant and an angle in degrees:

```rpl
'x3*Ⓒg*x2*sin(x1)' 2±σ0.01_kg 10±σ0.1_m 30±σ0.5_° ⓁσRFxjxi
@ Expecting 98.0665±σ1.84371 03556 2 kg·m↑2/s↑2
```

The same functions with MCPropagate take the names and the values as lists:
'x2*x1' { x2 x1 } { 2.5±σ0.5 2.0±σ0.2 } 0 ⓁMCPropagate.


## ΔRf

Interval arithmetic for a composite function of one variable: the smallest and
largest values of f over an interval.

Stack: 'f(x)' X ▶ f(X), where f is an expression of the name x and X an
interval a…b, a±b or a±p%, with or without units. The result has the form of
X, except that a±p% gives a±b.

Evaluating f directly on an interval counts each occurrence of x as a new
variable, and the result can be far too wide. ΔRf searches the global minimum
and maximum of f over X instead: f is evaluated at 65 points spread over X,
ends included, and each local maximum or minimum of these values is refined
by a golden section search. An extremum narrower than 1/64 of X could be
missed; Exmnf draws the function to check it.

**Example 1.** A polynomial with three occurrences of x. Evaluated directly on
1…3, it gives −4…4, since its factors are taken as independent; the true
range is ±2/(3√3):

```rpl
'(x-1)*(x-2)*(x-3)' '1…3' →Num ⓁΔRf
@ Expecting -0.38490 01794 6…0.38490 01794 6
```

**Example 2.** A maximum inside the interval: sin reaches 1 at π/2.

```rpl
'sin(x)' 1.5±0.15_r ⓁΔRf
@ Expecting 0.98786 16789 13±0.01213 83210 87
```

**Example 3.** A minimum inside the interval, at x = 0:

```rpl
'exp(x^2)' 0.5±0.9 ⓁΔRf
@ Expecting 4.04966 35325 8±3.04966 35325 8
```

**Example 4.** A function that is not differentiable at its minimum:

```rpl
'abs(x^3)' -0.25±0.35 ⓁΔRf
@ Expecting 0.108±0.108
```

**Example 5.** With units:

```rpl
'x^4' 1.10±1.15_m ⓁΔRf
@ Expecting 12.81445 3125±12.81445 3125 m↑4
```

**Example 6.** An interval given in percent gives a±b:

```rpl
'x^2' '10±10%' →Num ⓁΔRf
@ Expecting 101.±20.
```


## Exmnf

Examine a function over an interval: Exmnf draws f over X, with the smallest
and largest values found by ΔRf as dashed lines, and leaves the result of ΔRf
on the stack.

Stack: 'f(x)' X ▶ f(X), as ΔRf.

The curve uses 161 points. Each dashed line should touch the curve; a line
that does not, or a peak of the curve beyond a line, would show an extremum
that ΔRf missed.

**Example 1.** The polynomial of the first example of ΔRf, with its two
extrema inside the interval:

```rpl
'(x-1)*(x-2)*(x-3)' '1…3' →Num ⓁExmnf
```

**Example 2.** A power with units:

```rpl
'x^4' 1.10±1.15_m ⓁExmnf
```


## ΔRFxjxi

Interval arithmetic for a function of several variables: the smallest and
largest values of F over a box, each variable in its own interval.

Stack: 'F' Xn … X2 X1 ▶ F(X), where F is an expression of the names x1 … xn
and each Xj an interval a…b, a±b or a±p%, or a plain number for an exact
value, with or without units. X1 is on the first level, as for σRFxjxi. The
result is a…b when all the intervals are a…b, a±b otherwise.

F is first evaluated at the 2ⁿ vertices of the box. A coordinate search then
starts from the centre and from the best vertex, and improves the minimum and
the maximum one variable at a time, over its whole interval, until nothing
improves. This finds an extremum inside the box, on a face or on an edge, one
per subspace, which is what small intervals call for. It is a search, not a
proof: a function with many extrema over a wide box could hide one.

**Example 1.** Two occurrences of x1. Evaluated directly, x1·x2 − x1 on
x1 = 1…2 and x2 = 0…3 gives −2…5; the true range is −2…4:

```rpl
'x1*x2-x1' 1.5±1.5 1.5±0.5 ⓁΔRFxjxi
@ Expecting 1.±3.
```

**Example 2.** A minimum inside the box, at x1 = 1 and x2 = 2:

```rpl
'(x1-1)^2+(x2-2)^2' 2±1 1±1 ⓁΔRFxjxi
@ Expecting 1±1
```

**Example 3.** A maximum on an edge, at x1 = 0.5 and x2 = 1:

```rpl
'x1*(1-x1)+x2' 0.5±0.5 0.5±0.5 ⓁΔRFxjxi
@ Expecting 0.625±0.625
```

**Example 4.** Small intervals around a point where F changes slowly:

```rpl
'(x1-1)*(x2-2)*(x1+x2-3)' 2.5±0.1 1.5±0.1 ⓁΔRFxjxi
@ Expecting 0.28±0.152
```

**Example 5.** With units and an angle in degrees:

```rpl
'x2*sin(x1)' 10±1_m 30±5_° ⓁΔRFxjxi
@ Expecting 5.05645 25777 6±1.25288 82221 m
```
