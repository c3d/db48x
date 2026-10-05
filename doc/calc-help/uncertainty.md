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
* [Applications](#applicationslibrary) — worked examples of the whole
  section: [Projectile](#projectilelibrary), in eight steps.
* [Monte Carlo](#monte-carlolibrary) — propagate uncertainties through any
  model by random draws, with any distribution for each input:
  [MCPropagate](#mcpropagate).

Correlations are given by two global variables. `ρij` holds one coefficient
for all the pairs, or the full correlation matrix, for the functions of
several variables: σRFxjxi and MCPropagate. `ρ` holds a single coefficient
between two values, for the arithmetic of the calculator on `a±σb` values, as
in `10±σ1 12±σ1 +`, and for σConcord. Without them, the values are
independent; purge them after use, or they will weigh on later calculations.


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

**1)**

```rpl
12.34563±0.02347 ⓁSciRngRnd
@ Expecting 12.346±0.023
```

**2)**

```rpl
12.3456±σ0.0234_m ⓁSciRngRnd
@ Expecting 12.346±σ0.023 m
```

**3)**

```rpl
12.3456±1.5% ⓁSciRngRnd
@ Expecting 12.35±1.5%
```

**4)** A value in metres and an uncertainty in centimetres:

```rpl
12.3456_m 2.34_cm ⓁSciRngRnd
@ Expecting 12.346±0.023 m
```

When rounding the uncertainty reaches the next decade, the value follows it —
and the result shows why [SciRngText](#scirngtext) exists: the uncertainty
0.10 is displayed as 0.1.

**5)**

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

**1)**

```rpl
12.3456±0.0996 ⓁSciRngText
@ Expecting "12.35±0.10"
```

**2)**

```rpl
12.2…12.5678 ⓁSciRngText
@ Expecting "12.20…12.56"
```

**3)**

```rpl
2±0.0234 ⓁSciRngText
@ Expecting "2.000±0.023"
```

**4)**

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

**1)**

```rpl
1.456±σ0.023 ⓁSciRngParen
@ Expecting "1.456(23)"
```

**2)**

```rpl
6.67430±σ0.00015 ⓁSciRngParen
@ Expecting "6.67430(15)"
```

**3)**

```rpl
12.3456±σ0.0996_m ⓁSciRngParen
@ Expecting "12.35(10) m"
```

**4)**

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

**1)** A display reading 9.000 kg: the bound is ±0.0005 kg, and a rounded reading is
rectangular:

```rpl
9._kg 0.0005_kg 1 { } ⓁBound→σ
@ Expecting 9.±σ0.00028 86751 35 kg
```

**2)** A certificate gives ±0.2 at 95 % with 10 degrees of freedom:

```rpl
10. 0.2 8 { 10 0.95 } ⓁBound→σ
@ Expecting 10.±σ0.08976 10127 94
```

**3)** Unequal bounds move the value to the middle of the interval:

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

**1)**

```rpl
9._kg 3 ⓁResN0
@ Expecting 0.001 kg
```

**2)**

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

**1)**

```rpl
1…3 2…4 ⓁΔConcord
```

It returns `X∩Y:2…3`, `X%:50`, `Y%:50` and `rel:"X overlaps Y"`.

**2)** A mass measured to ±0.0023 g, against a reference given to ±0.005 g, in grams
both:

```rpl
5.3617±0.0023_g 5.360±0.005_g ⓁΔConcord
```

The first interval lies inside the second: `X∩Y:5.3594…5.364 g`, `X%:100`,
`Y%:46`, `rel:"X containedBy Y"`.


**3)** Random pairs make a good exercise. This draws two intervals between −100 and
100 and compares them; run it several times, or press DMOΔConcord in
Applications/Demos:

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

**1)**

```rpl
10±σ1 12.2±σ1 ⓁσConcord
```

It returns `d:2.2`, `σD:1.41421 35623 7`, `z:1.55563 49186 1` and
`p:0.11979 49304 26`.

**2)** The two ±σ bars do not touch, yet p = 0.12. With a correlation of 0.5, the
difference is better known and the same gap weighs more:

```rpl
0.5 'ρ' STO 10±σ1 12.2±σ1 ⓁσConcord
```

Now `σD:1`, `z:2.2` and `p:0.02780 68950 27`. Purge `ρ` afterwards, or it will
also weigh on the arithmetic of every uncertain value.

**3)** Random measurements make a good exercise too. This draws two measurements of
the same true value 10, with standard deviations between 0.5 and 2; run it
several times, or press DMOσConcord in Applications/Demos. About one run in
twenty gives p < 0.05, although both measure the same thing:

```rpl
'ρ' PURGE
0.5 2 RANDOM 2 Round DUP 10 SWAP ⓁNormlRand 2 Round SWAP →σRange
0.5 2 RANDOM 2 Round DUP 10 SWAP ⓁNormlRand 2 Round SWAP →σRange
ⓁσConcord
```


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

**1)**

```rpl
1…3 2…4 ⓁRngRel
@ Expecting "X overlaps Y"
```

**2)**

```rpl
1…3_m 100…300_cm ⓁRngRel
@ Expecting "X equals Y"
```

**3)**

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
number of draws. M may be omitted, F Vars Vals: 2 000 draws are then made,
which give the standard uncertainty to about 2 %, in a few seconds on the
simulator or a phone and in about a minute on a calculator.

The type of each input tells its distribution:

* a standard deviation, like 10±σ1: normal;
* bounds, like 9…11, 10±1 or 10±10%: rectangular between the bounds, the usual
  choice for a Type B evaluation;
* a program that returns one draw, like « 10 1 ⓁLgNrmRand »: any distribution;
  every distribution of the Probability section has such a Rand function;
* a plain number: an exact value.

Inputs may carry units. They are independent, unless a global variable ρij
holds correlations, as for σRFxjxi: one coefficient for all the pairs, or the
full correlation matrix, in the order of Vals. Normal inputs are then
correlated exactly, through the Cholesky factor of ρij; inputs given by bounds
through a Gaussian copula, as in the NIST Uncertainty Machine. An input given
by a program draws by itself and cannot be correlated. A matrix that is not
symmetric, or that no set of quantities could have, is refused.

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

**1)** One distribution alone: the model is the variable itself. A
rectangular distribution between 9 and 11 has a mean of 10 and a standard
deviation of 1/√3 = 0.577:

```rpl
12345 RDZ
'x' { x } { '9…11' } →Num 2000 ⓁMCPropagate
@ Expecting 10.02378 18042±σ0.58487 21349 4
```

**2)** The sum of two normal inputs. The exact result is 30±σ2.236:

```rpl
12345 RDZ
'a+b' { a b } { 10±σ1 20±σ2 } 2000 ⓁMCPropagate
@ Expecting 30.12500 40006±σ2.24588 10666 3
```

**3)** The product of a rectangular input and a normal one:

```rpl
12345 RDZ
'a*b' { a b } { '9…11' 10±σ1 } →Num 2000 ⓁMCPropagate
@ Expecting 100.26278 5406±σ11.39132 08051
```

**4)** Inputs with units: a resistance from a voltage and a current.

```rpl
12345 RDZ
'V/I' { V I } { '10±σ0.1_V' '5±σ0.05_A' } →Num 2000 ⓁMCPropagate
@ Expecting 1.99888 76261 4±σ0.02860 28043 8 V/A
```

**5)** A model that is not linear. For the square of a normal input
centered on zero, the propagation formula gives 0±σ0, since the derivative is
zero there. The true mean is 1, the standard deviation 1.414, and the interval
is far from symmetric:

```rpl
12345 RDZ
'x^2' { x } { 0±σ1 } 2000 ⓁMCPropagate
@ Expecting 0.97201 48896 25±σ1.40367 28151 1
```

**6)** Any distribution, through its draw program: here a lognormal
distribution. The draw programs are slower than the normal and rectangular
inputs, about 60 draws per second on the simulator:

```rpl
12345 RDZ
'x' { x } { « 0 1 ⓁLgNrmRand » } 500 ⓁMCPropagate
@ Expecting 1.88793 72628 2±σ2.38766 59582 6
```

The same distribution is obtained much faster as the exponential of a normal
input, with the model 'exp(x)' and the input 0±σ1.

**7)** Automatic stop, with M = 0. Here the result is stable after
8 500 draws:

```rpl
12345 RDZ
'a+b' { a b } { 10±σ1 20±σ2 } 0 ⓁMCPropagate
@ Expecting 30.03900 15233±σ2.25391 96331 3
```

**8)** Correlated inputs: the comparison loss in microwave power meter
calibration of JCGM 101 (9.4), δY = X1² + X2², with x1 = 0.010, x2 = 0,
u = 0.005 for both and a correlation of 0.9. The propagation formula gives
1.0×10⁻⁴; the Supplement finds 1.21×10⁻⁴ by the Monte Carlo method with a
million draws, and 2 000 draws give it to a few per cent:

```rpl
0.9 'ρij' STO 12345 RDZ
'a^2+b^2' { a b } { 0.010±σ0.005 0±σ0.005 } 2000 ⓁMCPropagate
'ρij' PURGE
@ Expecting 1.49769 28210 1⁳⁻⁴±σ1.25092 01608 2⁳⁻⁴
```

**9)** Without M: 2 000 draws, the sum of example 2:

```rpl
12345 RDZ
'a+b' { a b } { 10±σ1 20±σ2 } ⓁMCPropagate
@ Expecting 30.12500 40006±σ2.24588 10666 3
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
* [ExmnFxjxi](#exmnfxjxi) and [ExmnF2D](#exmnf2d): the profile and the plane
  slice of a function of several variables through its extrema, to check
  ΔRFxjxi.

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

**1)** The gamma function at 7±σ0.05. The exact uncertainty is
Γ(7)·ψ(7)·0.05 = 720 × 1.87278 43351 × 0.05 = 67.42023 60636, where
ψ(7) = −γ + 1 + 1/2 + 1/3 + 1/4 + 1/5 + 1/6 is the digamma function:

```rpl
'tgamma(x)' 7.0±σ0.05 ⓁσRf
@ Expecting 720.±σ67.42023 60635
```

**2)** A polynomial near one of its roots. Evaluated directly on
2.755±σ0.05, it gets about twice this uncertainty, since its three factors are
treated as independent:

```rpl
'(x-1)*(x-2)*(x-3)' 2.755±σ0.05 ⓁσRf
@ Expecting -0.32463 1125±σ0.03550 375
```

**3)** The difference of a quantity with itself is exactly zero:

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

Thanks to Ed van Gasteren, whose Propagate program and discussions led to this
function.

**1)** A mass from a linear density and a length, with units:

```rpl
'x2*x1' 2.5±σ0.5_g/cm 2.0±σ0.2_cm ⓁσRFxjxi
@ Expecting 5.±σ1.11803 39887 5 g
```

**2)** Three lengths fully correlated, for instance measured with the
same rule: the uncertainties add instead of combining in quadrature.

```rpl
1 'ρij' Sto
'x3+x2+x1' 1.2±σ0.002 2.5±σ0.005 1.600±σ0.012 ⓁσRFxjxi
'ρij' Purge
@ Expecting 5.3±σ0.019
```

**3)** Two variables correlated at 0.5, given as a matrix: the
uncertainty is √(1² + 2² + 2·0.5·1·2) = √7:

```rpl
[[ 1 0.5 ] [ 0.5 1 ]] 'ρij' Sto
'x2+x1' 10±σ1 20±σ2 ⓁσRFxjxi
'ρij' Purge
@ Expecting 30±σ2.64575 13110 6
```

**4)** The potential energy of a mass raised along a slope, with a
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

**1)** A polynomial with three occurrences of x. Evaluated directly on
1…3, it gives −4…4, since its factors are taken as independent; the true
range is ±2/(3√3):

```rpl
'(x-1)*(x-2)*(x-3)' '1…3' →Num ⓁΔRf
@ Expecting -0.38490 01794 6…0.38490 01794 6
```

**2)** A maximum inside the interval: sin reaches 1 at π/2.

```rpl
'sin(x)' 1.5±0.15_r ⓁΔRf
@ Expecting 0.98786 16789 13±0.01213 83210 87
```

**3)** A minimum inside the interval, at x = 0:

```rpl
'exp(x^2)' 0.5±0.9 ⓁΔRf
@ Expecting 4.04966 35325 8±3.04966 35325 8
```

**4)** A function that is not differentiable at its minimum:

```rpl
'abs(x^3)' -0.25±0.35 ⓁΔRf
@ Expecting 0.108±0.108
```

**5)** With units:

```rpl
'x^4' 1.10±1.15_m ⓁΔRf
@ Expecting 12.81445 3125±12.81445 3125 m↑4
```

**6)** An interval given in percent gives a±b:

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
that ΔRf missed. The highest and the lowest points of the curve are marked by
filled triangles ▲ ▼, the other local maxima and minima by hollow ones, each
centred on its point.

**1)** The polynomial of the first example of ΔRf, with its two
extrema inside the interval:

```rpl
'(x-1)*(x-2)*(x-3)' '1…3' →Num ⓁExmnf
```

**2)** A power with units:

```rpl
'x^4' 1.10±1.15_m ⓁExmnf
```

**A peak narrower than the sampling.** No search can promise an extremum
narrower than the steps it takes. This function rises gently from 1 to 1.9,
with a peak of height 3 and width 0.008 at x = 2.37:

**3)** Over 0…3, the 161 points of the curve fall on the flank of the peak and
show it as a small spike, marked by hollow triangles, but ΔRf misses it: the dashed line of the
maximum stays at 1.9, below the spike.

```rpl
'1+x^2/10+3*exp(-((x-2.37)/0.004)^2)' '0…3' →Num ⓁExmnf
```

**4)** A spike that the dashed lines do not explain calls for a closer look.
Over 2.3…2.45, around the spike, the peak is drawn whole and ΔRf finds the
true maximum, 4.56:

```rpl
'1+x^2/10+3*exp(-((x-2.37)/0.004)^2)' '2.3…2.45' →Num ⓁExmnf
```

Over a wide interval, examine the drawing as well as the numbers, and narrow
the interval around anything they do not account for.


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

**1)** Two occurrences of x1. Evaluated directly, x1·x2 − x1 on
x1 = 1…2 and x2 = 0…3 gives −2…5; the true range is −2…4:

```rpl
'x1*x2-x1' 1.5±1.5 1.5±0.5 ⓁΔRFxjxi
@ Expecting 1.±3.
```

**2)** A minimum inside the box, at x1 = 1 and x2 = 2:

```rpl
'(x1-1)^2+(x2-2)^2' 2±1 1±1 ⓁΔRFxjxi
@ Expecting 1±1
```

**3)** A maximum on an edge, at x1 = 0.5 and x2 = 1:

```rpl
'x1*(1-x1)+x2' 0.5±0.5 0.5±0.5 ⓁΔRFxjxi
@ Expecting 0.625±0.625
```

**4)** Small intervals around a point where F changes slowly:

```rpl
'(x1-1)*(x2-2)*(x1+x2-3)' 2.5±0.1 1.5±0.1 ⓁΔRFxjxi
@ Expecting 0.28±0.152
```

**5)** With units and an angle in degrees:

```rpl
'x2*sin(x1)' 10±1_m 30±5_° ⓁΔRFxjxi
@ Expecting 5.05645 25777 6±1.25288 82221 m
```


## ExmnFxjxi

Examine a function of several variables over a box: the profile of F along
the straight segment that joins its global minimum to its global maximum.

Stack: 'F' Xn … X2 X1 ▶ F(X), as ΔRFxjxi, whose result it leaves on the stack.

The minimum and the maximum are found as by ΔRFxjxi; the segment that joins
them fixes the n−1 other degrees of freedom, and F is drawn at 161 of its
points, from the minimum on the left to the maximum on the right. The values
and the points of both extrema are written below, as { x1 … xn }. The minimum
and the maximum, at the ends, are marked by filled triangles ▼ ▲, the local
extrema along the profile by hollow ones.

**1)** A maximum on an edge of the box:

```rpl
'x1*(1-x1)+x2' 0.5±0.5 0.5±0.5 ⓁExmnFxjxi
```

**2)** Several extrema inside a wide box: the minimum −1 is found at
x1 = 3π/2, the maximum 1 at x1 = π/2, both with x2 = 0:

```rpl
'sin(x1)*cos(x2)' 3±3_r 3±3_r ⓁExmnFxjxi
```

**3)** Robustness: a bump and a dip right on the way from the minimum to the
maximum. The profile shows both, with their hollow marks, yet the search keeps the global extrema, at (0, 0) and
(3, 3):

```rpl
'(x1^2+x2^2)/10+0.4*exp(-((x1-1)^2+(x2-1)^2)/0.1)-0.4*exp(-((x1-2)^2+(x2-2)^2)/0.1)' 1.5±1.5 1.5±1.5 ⓁExmnFxjxi
```


## ExmnF2D

Examine a function of several variables over a box: the plane slice that
contains its global minimum and its global maximum.

Stack: 'F' Xn … X2 X1 ▶ F(X), as ΔRFxjxi, whose result it leaves on the stack;
at least two variables must be intervals.

Each variable is scaled to its interval, the box becoming a cube. The slice is
the plane that contains the segment from the minimum to the maximum, and the
direction of the variable least involved in that segment, named on the last
line; the other degrees of freedom are fixed by the plane. Seven contour
lines, at 1/8, 2/8 … 7/8 of the way from the minimum to the maximum, show the
shape of F, as on a map: close together where F changes fast. The edge of the
box is drawn too.
The minimum and the maximum are marked by filled triangles ▼ ▲, joined by the
segment that ExmnFxjxi draws. A point of the grid higher than its eight
neighbours, or lower, is marked by a hollow triangle pointing up or down: the
local summits and pits of the slice, which tell whether a ring of contours
surrounds a bump or a dip. About a thousand values of F are computed: a few seconds on
the simulator.

**1)** A minimum inside the box, a maximum at a vertex:

```rpl
'(x1-1)^2+(x2-2)^2' 2±1 1±1 ⓁExmnF2D
```

**2)** Three variables: the slice cuts the cube along a polygon:

```rpl
'x3*(x1-1)^2+x2' 1±0.5 0.5±0.5 1±1 ⓁExmnF2D
```

**3)** A saddle, over a box longer in x1: the minimum −1 at x1 = 0,
x2 = ±1, the maximum 2.25 at x1 = ±1.5, x2 = 0, and hyperbolas between them:

```rpl
'x1^2-x2^2' 0±1 0±1.5 ⓁExmnF2D
```

**4)** A bump: the maximum 1 at the origin, inside the box, with
circles around it, and the minimum at a far corner:

```rpl
'exp(-(x1^2+x2^2))' 0.5±1.5 0±1.5 ⓁExmnF2D
```

**5)** Robustness: the same bump and dip as in example 3 of ExmnFxjxi. The
contours ring both of them, on the segment, and hollow triangles mark the
bump and the dip. The
search still finds the global minimum at (0, 0) and the global maximum 1.8 at
(3, 3):

```rpl
'(x1^2+x2^2)/10+0.4*exp(-((x1-1)^2+(x2-1)^2)/0.1)-0.4*exp(-((x1-2)^2+(x2-2)^2)/0.1)' 1.5±1.5 1.5±1.5 ⓁExmnF2D
```

**A hidden peak, in two steps.** A narrow peak of height 3 stands at
x1 = 2.5, x2 = 1.9, not far from the corner (3, 3), on a gentle slope from 1
to 1.9:

**6)** Over the whole box 0…3 × 0…3, the search misses the peak and gives the
maximum 1.9 at the corner (3, 3). The drawing does not: rings of contours near
the maximum, with a hollow triangle at their centre, show a summit that the numbers do not
account for.

```rpl
'1+x1*x2/10+3*exp(-((x1-2.5)^2+(x2-1.9)^2)/0.03)' 1.5±1.5 1.5±1.5 ⓁExmnF2D
```

**7)** A smaller box centred on the hollow triangle, x1 = 2.5±0.3 and
x2 = 1.9±0.3, given as X2 X1: the peak is now within the reach of the search,
and the true maximum appears, 4.475 at (2.5, 1.9):

```rpl
'1+x1*x2/10+3*exp(-((x1-2.5)^2+(x2-1.9)^2)/0.03)' 1.9±0.3 2.5±0.3 ⓁExmnF2D
```

The search starts from the centre and from the best vertex of the box, one
variable at a time; a summit narrow compared with the box, off the lines it
follows, may escape it. When a hollow triangle appears away from the filled ones, or rings
of contours that the numbers do not explain, examine a smaller box around it.


## ApplicationsLibrary

Worked examples of the whole section, after Part 2, Applications, of Jean
Wilson's Proposition for interval implementation in the RPL environment
(2025). Each submenu holds worked examples or demonstrations.

* [GUMS1](#gums1library): the four examples of Supplement 1 to the GUM
  (JCGM 101:2008), by the propagation formula and by Monte Carlo.
* [Demos](#demoslibrary): commands of the section run on random inputs, a new
  case at each press.
* [Projectile](#projectilelibrary): a projectile experiment, in eight steps, from the
  Type B evaluation of the measurements to the Monte Carlo method.


## DemosLibrary

Demonstrations: each key runs a command of the section on inputs drawn at
random, so that every press shows a new case. Seed the generator with RDZ to
replay the same sequence.

* DMOΔConcord: two bounded intervals, compared by ΔConcord;
* DMOσConcord: two measurements of the same quantity, compared by σConcord.


## DMOΔConcord

Draws two intervals at random between −100 and 100 and compares them with
ΔConcord: the drawing, the intersection, the parts covered and the relation.
Press the key again for a new pair; in a few presses most of the thirteen
relations of RngRel show up, the rare ones (meets, starts, equals…) excepted.

**1)** A pair, reproducible with its seed:

```rpl
42 RDZ ⓁDMOΔConcord
```

**2)** Then as many new pairs as wanted:

```rpl
ⓁDMOΔConcord
```


## DMOσConcord

Two measurements of the same true value, 10. Each has a standard deviation
drawn between 0.5 and 2, and a value drawn from the normal law around 10 with
that standard deviation, as a real measurement would. σConcord then says
whether the difference is compatible with zero.

Since both measure the same thing, p is uniform between 0 and 1: it falls below
0.05 about one press in twenty, and below 0.01 one in a hundred, although
nothing is wrong. This is what a threshold of 5 % means, and why one small p
alone does not prove a disagreement.

**1)** A pair, reproducible with its seed:

```rpl
42 RDZ ⓁDMOσConcord
```

**2)** Then as many new pairs as wanted:

```rpl
ⓁDMOσConcord
```


## GUMS1Library

The four worked examples of Supplement 1 to the GUM, JCGM 101:2008 (clause 9),
published by the BIPM with their data and results. Each one is solved twice:
by the law of propagation of uncertainty, as the GUM does (σRFxjxi), and by
the Monte Carlo method of the Supplement (MCPropagate). Comparing the two with
the published tables is a validation of both, and each example teaches when
the first method may be trusted.

* [S1Additive](#s1additive): the additive model (9.2);
* [S1Mass](#s1mass): the mass calibration (9.3), where the propagation formula
  underestimates the uncertainty by 40 %;
* [S1Power](#s1power): the comparison loss in microwave power meter calibration
  (9.4), where it gives 0, with and without correlation;
* [S1Gauge](#s1gauge): the gauge block calibration (9.5), nine inputs and five
  kinds of distributions.

The Supplement uses 10⁵ to 10⁶ draws, the pages below 1 000 to 2 000, so that
each example runs in seconds: their Monte Carlo results come within a few per
cent of the published ones. With 10 000 draws, DB48x gives:

| Example | Propagation formula, u | Monte Carlo, u | JCGM 101, Monte Carlo |
|---|---|---|---|
| 9.2 normal inputs | 2 | 2.006 | 2.00 |
| 9.2 rectangular inputs | 2 | 2.015 | 2.00 |
| 9.2 one dominant input | 10.15 | 10.23 | 10.1–10.2 |
| 9.3 mass | 0.0539 mg | 0.0744 mg | 0.0754 mg |
| 9.4, x1 = 0.010 | 100 × 10⁻⁶ | 112 × 10⁻⁶ | 112 × 10⁻⁶ |
| 9.4, x1 = 0.010, r = 0.9 | 100 × 10⁻⁶ | 120 × 10⁻⁶ | 121 × 10⁻⁶ |
| 9.5 gauge block | 32.1 nm | 35.7 nm | 36 nm |


## S1Additive

JCGM 101, 9.2: the additive model Y = X1 + X2 + X3 + X4, with independent
inputs of expectation 0. The exact answer is known, which makes it the test of
the method itself.

**1)** Four normal inputs of standard deviation 1: the propagation formula gives u = 2.

```rpl
'x1+x2+x3+x4' 0±σ1 0±σ1 0±σ1 0±σ1 ⓁσRFxjxi
@ Expecting 0±σ2.
```

**2)** The Monte Carlo method agrees; the Supplement gives 0.00±σ2.00 and the 95 % interval [−3.92, 3.92]:

```rpl
12345 RDZ
'a+b+c+d' { a b c d } { 0±σ1 0±σ1 0±σ1 0±σ1 } ⓁMCPropagate
@ Expecting 6.17138 85639 4⁳⁻²±σ2.00826 44578 6
```

**3)** Four rectangular inputs of standard deviation 1, between −√3 and √3. The propagation formula is the same, u = 2; the exact 95 % interval is [−3.88, 3.88], a little narrower than the Gaussian one:

```rpl
12345 RDZ
'a+b+c+d' { a b c d }
{ 0±1.73205080757 0±1.73205080757 0±1.73205080757 0±1.73205080757 } ⓁMCPropagate
@ Expecting -0.01433 08706 8±σ1.98765 66807 8
```

**4)** The same, with a fourth input ten times wider. The propagation formula gives u = 10.15, and with k = 1.96 the interval ±19.9:

```rpl
'x1+x2+x3+x4' 0±σ10 0±σ1 0±σ1 0±σ1 ⓁσRFxjxi
@ Expecting 0±σ10.14889 15651
```

**5)** The Monte Carlo method finds the interval ±17.0, as the Supplement does: when one rectangular input dominates, the result is not normal, and the Gaussian interval is far too wide.

```rpl
12345 RDZ
'a+b+c+d' { a b c d }
{ 0±1.73205080757 0±1.73205080757 0±1.73205080757 0±17.3205080757 } ⓁMCPropagate
@ Expecting 0.15361 87355 27±σ10.24279 74855
```

## S1Mass

JCGM 101, 9.3: a weight W of nominal mass 100 g is calibrated against a
reference weight R, in air of density ρa; the buoyancy depends on the densities
ρW and ρR of the two weights. The deviation from the nominal mass is

δm = (mR + δmR)·(1 + (ρa − ρa0)·(1/ρW − 1/ρR)) − 100 000 mg

with ρa0 = 1.2 kg/m³, mR = 100 000±σ0.050 mg, δmR = 1.234±σ0.020 mg, and
rectangular densities: ρa between 1.1 and 1.3, ρW between 7 000 and 9 000,
ρR between 7 950 and 8 050 kg/m³ (table 5 of the Supplement).

**1)** The propagation formula gives u = 0.0539 mg, as the GUM uncertainty framework of the Supplement (table 6):

```rpl
'(x1+x2)*(1+(x3-1.2)*(1/x4-1/x5))-100000'
8000±σ28.8675134595 8000±σ577.350269190 1.2±σ0.0577350269190 1.234±σ0.020 100000±σ0.050
ⓁσRFxjxi
@ Expecting 1.234±σ5.38516 48071 3⁳⁻²
```

**2)** The Monte Carlo method gives about 0.075 mg; the Supplement finds 0.0754 mg:

```rpl
12345 RDZ
'(m+d)*(1+(densA-1.2)*(1/densW-1/densR))-100000'
{ m d densA densW densR }
{ 100000±σ0.050 1.234±σ0.020 1.2±0.1 8000±1000 8000±50 } ⓁMCPropagate
@ Expecting 1.23850 18718±σ0.07615 43941 04
```

The propagation formula underestimates the uncertainty by 40 %. At the
estimates, ρa = ρa0, so the derivatives of δm with respect to the three
densities are all zero: to first order, the densities do not count. They do,
through the product of their deviations, which only a second order formula or
the Monte Carlo method sees.


## S1Power

JCGM 101, 9.4: the comparison loss of a microwave power meter, δY = X1² + X2²,
where X1 and X2 are the real and imaginary parts of a reflection coefficient,
normal, with x2 = 0 and u(x1) = u(x2) = 0.005 (tables 8 and 9 of the
Supplement).

**1)** At x1 = 0, the derivatives are zero, and the propagation formula gives an uncertainty of 0, which is wrong:

```rpl
'x1^2+x2^2' 0±σ0.005 0±σ0.005 ⓁσRFxjxi
@ Expecting 0±σ1.17851 13019 8⁳⁻⁸
```

**2)** The Monte Carlo method gives δy ≈ 50×10⁻⁶ and u ≈ 50×10⁻⁶, as the Supplement and the exact solution:

```rpl
12345 RDZ
'a^2+b^2' { a b } { 0±σ0.005 0±σ0.005 } ⓁMCPropagate
@ Expecting 5.07012 99776 7⁳⁻⁵±σ5.03014 46082 1⁳⁻⁵
```

**3)** At x1 = 0.010, the propagation formula gives u = 100×10⁻⁶:

```rpl
'x1^2+x2^2' 0±σ0.005 0.010±σ0.005 ⓁσRFxjxi
@ Expecting 0.0001±σ1.00000 00034 7⁳⁻⁴
```

**4)** The Monte Carlo method finds about 112×10⁻⁶, as the Supplement:

```rpl
12345 RDZ
'a^2+b^2' { a b } { 0.010±σ0.005 0±σ0.005 } ⓁMCPropagate
@ Expecting 1.50469 96371 1⁳⁻⁴±σ1.14549 79356 9⁳⁻⁴
```

**5)** X1 and X2 correlated at 0.9: the Supplement finds 121×10⁻⁶ (table 9).

```rpl
0.9 'ρij' STO 12345 RDZ
'a^2+b^2' { a b } { 0.010±σ0.005 0±σ0.005 } ⓁMCPropagate
'ρij' PURGE
@ Expecting 1.49769 28210 1⁳⁻⁴±σ1.25092 01608 2⁳⁻⁴
```

The distribution of δY is far from normal: at x1 = 0, it is a χ² law with
two degrees of freedom, which starts at 0. The Supplement gives its shortest
95 % interval, [0, 150]×10⁻⁶; MCPropagate gives the interval that leaves 2.5 %
on each side, about [1.3, 184]×10⁻⁶ in theory. Both are right, for two
definitions.


## S1Gauge

JCGM 101, 9.5: the length of a nominally 50 mm gauge block, compared with a
reference block, the example H.1 of the GUM itself. In nm,

δL = Ls + D + d1 + d2 − Ls·(δα·(θ0 + Δ) + αs·δθ) − 50 000 000

with nine inputs of five kinds (table 10 of the Supplement): Ls, D, d1 and d2
follow scaled and shifted t-distributions (StudentRand), αs is rectangular,
θ0 normal, Δ follows an arc sine law (UShapeRand), and δα and δθ are
rectangular with inexactly known limits: a half-width drawn in [w − d, w + d],
then a value drawn in ±that half-width.

**1)** The propagation formula gives 838±σ32 nm, as the Supplement (table 11):

```rpl
'x1+x2+x3+x4-x1*(x8*(x6+x7)+x5*x9)-50000000'
0±σ0.0300462 0±σ5.78315E-7 0±σ0.353553390593 -0.1±σ0.2 11.5E-6±σ1.15470053838E-6
0±σ7 0±σ4 215±σ6 50000623±σ25 ⓁσRFxjxi
@ Expecting 838±σ32.13796 12095
```

**2)** The Monte Carlo method, with 1 000 draws for speed: the Supplement finds 838±σ36 nm. The difference with the propagation formula comes from the heavy tails of the t-distributions.

```rpl
12345 RDZ
'gLs+gD+gd1+gd2-gLs*(gDa*(gT0+gDl)+gAs*gDt)-50000000'
{ gLs gD gd1 gd2 gAs gT0 gDl gDa gDt }
{ « 18 ⓁStudentRand 25 * 50000623 + » « 24 ⓁStudentRand 6 * 215 + »
  « 5 ⓁStudentRand 4 * » « 8 ⓁStudentRand 7 * »
  11.5E-6±2E-6 -0.1±σ0.2 « -0.5 0.5 ⓁUShapeRand »
  « RAND 0.2E-6 * 0.9E-6 + RAND 2 * 1 - * » « RAND 0.05 * 0.025 + RAND 2 * 1 - * » } 1000 ⓁMCPropagate
@ Expecting 838.99938 8993±σ37.24850 82155
```

With 10 000 draws, about two minutes on the simulator, the result is
838.0±σ35.7 nm.


## ProjectileLibrary

A worked example of the whole Uncertainty section, after Part 2 of Jean
Wilson's Proposition for interval implementation in the RPL environment
(2025). A spring loaded gun launches a glass marble towards a target at the
same height, 8 m away. Each step is a small experiment with its own
uncertainty budget, on its own page and under its own key; the results of the
previous steps are written in its calculations, so that each step can be run
alone.

* [Step A](#stepa): the mass of the marble;
* [Step B](#stepb): the constant of the spring;
* [Step C](#stepc): the ejection speed;
* [Step D](#stepd): the ejection angle;
* [Step E](#stepe): the height of the laser pointer;
* [Step F](#stepf): a more realistic experiment, with friction and losses;
* [Step G](#stepg): the same result, from the measurements themselves.
* [Step H](#steph): model validation, the predictions against the shots.

At each step, the same tools are used in the same order:

* the Type B evaluation of the data: a resolution or a tolerance is a bound,
  hence a rectangular law of standard deviation bound/√3 (Bound→σ);
* the law of propagation with independent inputs, ρ = 0, the physical case:
  two inputs are correlated only when their measurements share a cause of
  error, not because of the shape of the formula (σRFxjxi);
* the bracket of the correlations, the smallest and the largest uncertainty
  that correlations could give;
* interval arithmetic on intervals of one standard deviation, as in the 2025
  document, which contains the bracket (ΔRFxjxi);
* the Monte Carlo method, which needs no linearization (MCPropagate);
* the rounding of the result, and its comparison with a reference value when
  there is one (SciRngRnd, σConcord).

The data are those of 2025. Three Type B evaluations change: the diameter of
step A was given its resolution divided by √3 instead of the half resolution,
and the tolerances of steps B, D and E were read as standard deviations
instead of bounds.


## StepA

Step A: the mass of the marble, M = ρ·(4/3)·π·(D/2)³, with a diameter
D = 16.000 mm read on a micrometer of resolution 0.001 mm, a density
ρ = 2.500 g/cm³ known to one unit of its last digit, and a reference mass
Mref = 5.36±σ0.01 g.

**1)** The diameter: its last digit is a resolution, the bound is half of it.

```rpl
16_mm 0.0005_mm 1 { } ⓁBound→σ
@ Expecting 16.±σ2.88675 13459 5⁳⁻⁴ mm
```

**2)** The mass, inputs independent:

```rpl
'x2*4/3*Ⓒπ*(x1/2)^3' 2.5±σ0.001_g/cm^3 16±σ0.00028867513459_mm ⓁσRFxjxi
1_g Convert
@ Expecting 5.36165 14621 3±σ2.16420 64708 4⁳⁻³ g
```

**3)** The bracket of the correlations, ρ = −1:

```rpl
-1 'ρij' Sto
'x2*4/3*Ⓒπ*(x1/2)^3' 2.5±σ0.001_g/cm^3 16±σ0.00028867513459_mm ⓁσRFxjxi
1_g Convert 'ρij' Purge
@ Expecting 5.36165 14621 3±σ1.85445 26865 8⁳⁻³ g
```

**4)** and ρ = +1:

```rpl
1 'ρij' Sto
'x2*4/3*Ⓒπ*(x1/2)^3' 2.5±σ0.001_g/cm^3 16±σ0.00028867513459_mm ⓁσRFxjxi
1_g Convert 'ρij' Purge
@ Expecting 5.36165 14621 3±σ2.43486 84831 2⁳⁻³ g
```

**5)** Interval arithmetic:

```rpl
'x2*4/3*Ⓒπ*(x1/2)^3' 2.5±0.001_g/cm^3 16±0.00028867513459_mm ⓁΔRFxjxi
1_g Convert
@ Expecting 5.36165 15834 5±2.43486 84852 5⁳⁻³ g
```

**6)** The Monte Carlo method, the diameter rectangular:

```rpl
12345 RDZ
'r*4/3*Ⓒπ*(d/2)^3' { r d } { '2.5±σ0.001_g/cm^3' '15.9995…16.0005_mm' } →Num
ⓁMCPropagate 1_g Convert
@ Expecting 5.36170 70590 3±σ2.14913 26272 8⁳⁻³ g
```

**7)** Rounded, the mass is 5.3617±σ0.0022 g:

```rpl
5.3616514621±σ0.0021642064708_g ⓁSciRngRnd
@ Expecting 5.3617±σ0.0022 g
```

**8)** It agrees with the reference value:

```rpl
5.3617±σ0.0022_g 5.36±σ0.01_g ⓁσConcord
```


## StepB

Step B: the constant of the spring. It stores U = 0.22 J when it is
compressed by x = 15.00 cm, read on a rule graduated in millimetres:
k = 2·U/x². Its reference value is 20 N/m with a tolerance of ±2.5 %, a bound.

**1)** The compression, read to the millimetre:

```rpl
15_cm 0.05_cm 1 { } ⓁBound→σ
@ Expecting 15.±σ2.88675 13459 5⁳⁻² cm
```

**2)** The reference, ±2.5 % read as a bound: 20±σ0.29 N/m, not 20±σ0.5 N/m.

```rpl
20_N/m 0.5_N/m 1 { } ⓁBound→σ
@ Expecting 20.±σ0.28867 51345 95 N/m
```

**3)** The spring constant, inputs independent:

```rpl
'2*x2/x1^2' 0.22±σ0.00001_J 15±σ0.028867513459_cm ⓁσRFxjxi 1_N/m Convert
@ Expecting 19.55555 55556±σ0.07527 46168 82 N/m
```

**4)** The bracket, ρ = −1:

```rpl
-1 'ρij' Sto
'2*x2/x1^2' 0.22±σ0.00001_J 15±σ0.028867513459_cm ⓁσRFxjxi
1_N/m Convert 'ρij' Purge
@ Expecting 19.55555 55556±σ0.07615 82573 15 N/m
```

**5)** and ρ = +1:

```rpl
1 'ρij' Sto
'2*x2/x1^2' 0.22±σ0.00001_J 15±σ0.028867513459_cm ⓁσRFxjxi
1_N/m Convert 'ρij' Purge
@ Expecting 19.55555 55556±σ0.07438 04795 38 N/m
```

**6)** Interval arithmetic:

```rpl
'2*x2/x1^2' 0.22±0.00001_J 15±0.028867513459_cm ⓁΔRFxjxi 1_N/m Convert
@ Expecting 19.55577 62622±0.07615 88247 46 N/m
```

**7)** The Monte Carlo method:

```rpl
12345 RDZ
'2*u/x^2' { u x } { '0.22±σ0.00001_J' '14.95…15.05_cm' } →Num
ⓁMCPropagate 1_N/m Convert
@ Expecting 19.55534 095±σ0.07717 42988 1 N/m
```

**8)** 19.556±σ0.075 N/m is 1.5 standard deviations of the difference below the reference: compatible.

```rpl
19.556±σ0.075_N/m 20±σ0.29_N/m ⓁσConcord
```


## StepC

Step C: the ejection speed. All the energy of the spring becomes kinetic
energy of the marble: v = √(2·K/m), with K = U = 0.22 J.

From step A: m = 0.0053616514621±σ0.0000021642064708 kg.

**1)** The speed, inputs independent:

```rpl
'√(2*x2/x1)' 0.22±σ0.00001_J 0.0053616514621±σ0.0000021642064708_kg
ⓁσRFxjxi 1_m/s Convert
@ Expecting 9.05893 30239 4±σ1.83985 45958 4⁳⁻³ m/s
```

**2)** The bracket, ρ = −1:

```rpl
-1 'ρij' Sto
'√(2*x2/x1)' 0.22±σ0.00001_J 0.0053616514621±σ0.0000021642064708_kg
ⓁσRFxjxi 1_m/s Convert 'ρij' Purge
@ Expecting 9.05893 30239 4±σ2.03418 36042 1⁳⁻³ m/s
```

**3)** and ρ = +1:

```rpl
1 'ρij' Sto
'√(2*x2/x1)' 0.22±σ0.00001_J 0.0053616514621±σ0.0000021642064708_kg
ⓁσRFxjxi 1_m/s Convert 'ρij' Purge
@ Expecting 9.05893 30239 4±σ1.62241 39213⁳⁻³ m/s
```

**4)** Interval arithmetic:

```rpl
'√(2*x2/x1)' 0.22±0.00001_J 0.0053616514621±0.0000021642064708_kg
ⓁΔRFxjxi 1_m/s Convert
@ Expecting 9.05893 36166 4±2.03418 38025 4⁳⁻³ m/s
```


## StepD

Step D: the ejection angle. The marble must land at R = 8.00 m, the target
accepting ±2 %, a bound: θ = ½·asin(R·g/v²).

From step C: v = 9.0589330239±σ0.0018398545958 m/s.

**1)** The range, ±2 % read as a bound:

```rpl
8_m 0.16_m 1 { } ⓁBound→σ
@ Expecting 8.±σ0.09237 60430 7 m
```

**2)** The angle, inputs independent:

```rpl
'0.5*asin(x2*Ⓒg/x1^2)' 8±σ0.0923760430707_m 9.0589330239±σ0.0018398545958_m/s
ⓁσRFxjxi
@ Expecting 36.46990 40743±σ1.07860 45787 °
```

**3)** Interval arithmetic:

```rpl
'0.5*asin(x2*Ⓒg/x1^2)' 8±0.0923760430707_m 9.0589330239±0.0018398545958_m/s
ⓁΔRFxjxi
@ Expecting 36.54274 28944±1.12547 27707 6 °
```

**4)** The Monte Carlo method shows what the linearization misses: its mean is 0.05° higher, about three times its own standard error, and its uncertainty 3 % larger. The arcsine curves over the range of R.

```rpl
12345 RDZ
'0.5*asin(r*Ⓒg/v^2)' { r v } { '7.84…8.16_m' '9.0589330239±σ0.0018398545958_m/s' } →Num
5000 ⓁMCPropagate
@ Expecting 36.51925 9431±σ1.11125 26992 5 °
```


## StepE

Step E: the height of the laser pointer. The angle is set with a laser
pointed at a mark x = 2.000 m away, within ±0.1 %, a bound, at the height
y = x·tanθ.

From step D: θ = 36.4699040743±σ1.0786045787°.

**1)** The distance, ±0.1 % read as a bound:

```rpl
2_m 0.002_m 1 { } ⓁBound→σ
@ Expecting 2.±σ1.15470 05383 8⁳⁻³ m
```

**2)** The height, inputs independent:

```rpl
'x2*tan(x1)' 2±σ0.0011547005384_m 36.4699040743±σ1.0786045787_° ⓁσRFxjxi
@ Expecting 1.47829 70171 1±σ5.82265 98490 7⁳⁻² m
```

**3)** Interval arithmetic:

```rpl
'x2*tan(x1)' 2±0.0011547005384_m 36.4699040743±1.0786045787_° ⓁΔRFxjxi
@ Expecting 1.47914 11034 3±0.05909 24630 35 m
```

**4)** The Monte Carlo method:

```rpl
12345 RDZ
'd*tan(t)' { d t } { '1.998…2.002_m' '36.4699040743±σ1.0786045787_°' } →Num
ⓁMCPropagate
@ Expecting 1.48055 71150 4±σ0.05788 01664 32 m
```

**5)** Rounded, the laser must point 1.478±σ0.058 m high:

```rpl
1.4782970171±σ0.058226598491_m ⓁSciRngRnd
@ Expecting 1.478±σ0.058 m
```


## StepF

Step F: a more realistic experiment. Which compression x of the spring gives
the speed of step C, when the marble also loses energy by friction along the
barrel (μk = 0.5±σ0.005), rises in it, and keeps only a fraction f = 0.95±σ0.019
of the energy as translation? The energy balance gives

x = m·g/k·(sin θ + μk·cos θ) + 1/k·√((m·g)²·(sin θ + μk·cos θ)² + 2·k·K/f)

with x1 = m, x2 = k, x3 = θ, x4 = μk, x5 = K and x6 = f. This step is written
in SI units without unit objects, except the angle: with six variables, units
make the calculations about fifty times slower.

From step A: m = 0.0053616514621±σ0.0000021642064708 kg; from step B:
k = 19.5555555556±σ0.075274616882 N/m; from step D: θ = 36.4699040743±σ1.0786045787°.

**1)** The compression, inputs independent:

```rpl
'x1*9.80665/x2*(sin(x3)+x4*cos(x3))+1/x2*√((x1*9.80665)^2*(sin(x3)+x4*cos(x3))^2+2*x2*x5/x6)'
0.95±σ0.019 0.22±σ0.00001 0.5±σ0.005 36.4699040743±σ1.0786045787_° 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi
@ Expecting 0.15659 93648 36±σ1.56925 12210 7⁳⁻³
```

**2)** The 2025 document chose ρ51 = ρ52 = ρ65 = +1 and ρ54 = −1. This is not a possible correlation matrix, and σRFxjxi refuses it: if K were perfectly correlated with m, k and f, these three would be perfectly correlated with each other, while the matrix says they are independent. One coefficient for all the pairs cannot go below −1/(n−1), −0.2 here. With ρ = +1 for all the pairs:

```rpl
1 'ρij' Sto
'x1*9.80665/x2*(sin(x3)+x4*cos(x3))+1/x2*√((x1*9.80665)^2*(sin(x3)+x4*cos(x3))^2+2*x2*x5/x6)'
0.95±σ0.019 0.22±σ0.00001 0.5±σ0.005 36.4699040743±σ1.0786045787_° 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi 'ρij' Purge
@ Expecting 0.15659 93648 36±σ1.80367 32962⁳⁻³
```

**3)** The largest uncertainty that correlations can give is the one where all the contributions add up: interval arithmetic gives it.

```rpl
'x1*9.80665/x2*(sin(x3)+x4*cos(x3))+1/x2*√((x1*9.80665)^2*(sin(x3)+x4*cos(x3))^2+2*x2*x5/x6)'
0.95±0.019 0.22±0.00001 0.5±0.005 36.4699040743±1.0786045787_° 19.5555555556±0.075274616882 0.0053616514621±0.0000021642064708
ⓁΔRFxjxi
@ Expecting 0.15662 58788 25±1.88751 71425 7⁳⁻³
```

**4)** The Monte Carlo method:

```rpl
12345 RDZ
'm*9.80665/k*(sin(t)+u*cos(t))+1/k*√((m*9.80665)^2*(sin(t)+u*cos(t))^2+2*k*e/f)'
{ m k t u e f }
{ 0.0053616514621±σ0.0000021642064708 19.5555555556±σ0.075274616882
  '36.4699040743±σ1.0786045787_°' 0.5±σ0.005 0.22±σ0.00001 0.95±σ0.019 } →Num
ⓁMCPropagate
@ Expecting 0.15662 45397 98±σ1.61813 73075 7⁳⁻³
```

**5)** The Monte Carlo method with the same correlation as in 2), ρ = +1 for all
the pairs:

```rpl
1 'ρij' Sto 12345 RDZ
'm*9.80665/k*(sin(t)+u*cos(t))+1/k*√((m*9.80665)^2*(sin(t)+u*cos(t))^2+2*k*e/f)'
{ m k t u e f }
{ 0.0053616514621±σ0.0000021642064708 19.5555555556±σ0.075274616882
  '36.4699040743±σ1.0786045787_°' 0.5±σ0.005 0.22±σ0.00001 0.95±σ0.019 } →Num
ⓁMCPropagate 'ρij' Purge
@ Expecting 0.15664 39577 78±σ1.77935 81418 4⁳⁻³
```

Both methods find that a correlation of +1 between all the inputs would raise
the uncertainty of the compression from 1.62 mm to about 1.8 mm: 1.80 by the
propagation formula, 1.78 by Monte Carlo. Unlike the angle of step D, the
compression is nearly linear in its inputs over their range, and the
propagation formula is enough here.

The compression must be 15.66±σ0.16 cm, not the
15.00 cm of step B: the friction, the rise and the fraction f cost 0.66 cm of
compression, about four standard deviations. The experiment can be redone
with this setting.


## StepG

Step G: the same compression, from the measurements themselves. The six
variables of step F are not all measurements: k, θ and v were computed in
steps B to D from the same energy U = K and the same mass m. Their errors are
correlated for a real reason, a shared cause, and no ρij needs to be guessed
if x is written as a function of the seven measurements, which are
independent: x1 = D, x2 = ρ, x3 = U, x4 = the compression x0 of step B,
x5 = R, x6 = μk and x7 = f, with m = ρ·(4/3)·π·(D/2)³, k = 2·U/x0² and
θ = ½·asin(R·g·m/(2·U)).

**1)** The compression from the measurements:

```rpl
'(x2*4/3*Ⓒπ*(x1/2)^3)*9.80665/(2*x3/x4^2)*(sin(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3)))+x6*cos(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3))))+1/(2*x3/x4^2)*√(((x2*4/3*Ⓒπ*(x1/2)^3)*9.80665)^2*(sin(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3)))+x6*cos(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3))))^2+2*(2*x3/x4^2)*x3/x7)'
0.95±σ0.019 0.5±σ0.005 8±σ0.0923760430707 0.15±σ0.00028867513459 0.22±σ0.00001 2500±σ1 0.016±σ0.00000028867513459
ⓁσRFxjxi
@ Expecting 0.15659 93648 36±σ1.56924 37991⁳⁻³
```

**2)** The same as in step F: with an energy known to 10⁻⁵ J, the correlations weigh nothing. A spring gun rather gives about 2 %. With U = K = 0.22±σ0.0044 J, the calculation of step F, which treats K as independent of k and θ, finds a much larger uncertainty:

```rpl
'x1*9.80665/x2*(sin(x3)+x4*cos(x3))+1/x2*√((x1*9.80665)^2*(sin(x3)+x4*cos(x3))^2+2*x2*x5/x6)'
0.95±σ0.019 0.22±σ0.0044 0.5±σ0.005 36.4699040743±σ1.0786045787_° 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi
@ Expecting 0.15659 93648 36±σ2.19778 08296 2⁳⁻³
```

**3)** while the calculation from the measurements hardly moves:

```rpl
'(x2*4/3*Ⓒπ*(x1/2)^3)*9.80665/(2*x3/x4^2)*(sin(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3)))+x6*cos(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3))))+1/(2*x3/x4^2)*√(((x2*4/3*Ⓒπ*(x1/2)^3)*9.80665)^2*(sin(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3)))+x6*cos(0.5*asin(x5*9.80665*(x2*4/3*Ⓒπ*(x1/2)^3)/(2*x3))))^2+2*(2*x3/x4^2)*x3/x7)'
0.95±σ0.019 0.5±σ0.005 8±σ0.0923760430707 0.15±σ0.00028867513459 0.22±σ0.0044 2500±σ1 0.016±σ0.00000028867513459
ⓁσRFxjxi
@ Expecting 0.15659 93648 36±σ1.57240 83931 7⁳⁻³
```

The energy enters k, K and θ, and its effects almost cancel: ignoring this
correlation overestimates the uncertainty by 40 %. Going back to independent
measurements is the right way to propagate through a chain of calculations.


## StepH

Step H: model validation. The model predicts where the marble lands; the
experiment says where it actually lands. Are the two compatible? The gun is
set, ten shots are fired, and the range R of each is measured with a tape to
the centimetre. The angle is set with the laser pointer of step E to ±0.5°, a
bound, and the compression with the rule of step B.

The range follows from the energy K of the marble at the exit of the barrel:
R = 2·K·sin 2θ/(m·g). The ideal model of steps C and D takes K = ½·k·x²; the
realistic model of step F takes K = f·(½·k·x² − m·g·(sin θ + μk·cos θ)·x).
The shots scatter by about 10 cm from one to the next.

From step A: m = 0.0053616514621±σ0.0000021642064708 kg; from step B:
k = 19.5555555556±σ0.075274616882 N/m; from step D: θ = 36.4699040743°; from
step F: x = 15.66 cm, μk = 0.5±σ0.005 and f = 0.95±σ0.019.

**H-i) The ideal model, at x = 15.00 cm**

**1)** The ideal model predicts the target, 8 m:

```rpl
'x2*x3^2*sin(2*x4)/(x1*9.80665)'
36.4699040743±σ0.28867513459_° 0.15±σ0.00028867513459 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi
@ Expecting 8.00000 00000 9±σ5.01881 86642 8⁳⁻²
```

**2)** Ten shots at 15.00 cm: their mean, and its standard deviation s/√n:

```rpl
[[7.42] [7.41] [7.13] [7.31] [7.39] [7.37] [7.23] [7.25] [7.35] [7.21]] 'ΣData' STO
Average SDev 10 √ / →σRange
@ Expecting 7.307±σ0.03091 38588 12
```

**3)** Prediction and observation compared:

```rpl
8.00000000009±σ0.0501881866428 7.307±σ0.0309138588120 ⓁσConcord
```

The shots fall 69 cm short, about twelve standard deviations of the
difference: p = 6.5·10⁻³². The ideal model is rejected. The
realistic model, at the same compression, explains the short shots:

**4)** The realistic model at 15.00 cm:

```rpl
'x6*(x2*x3^2-2*x1*9.80665*(sin(x4)+x5*cos(x4))*x3)*sin(2*x4)/(x1*9.80665)'
0.95±σ0.019 0.5±σ0.005 36.4699040743±σ0.28867513459_° 0.15±σ0.00028867513459 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi
@ Expecting 7.32849 85110 7±σ0.15380 86796 2
```

**H-ii) The realistic model, at x = 15.66 cm**

**5)** At the compression computed in step F, the realistic model predicts 8 m:

```rpl
'x6*(x2*x3^2-2*x1*9.80665*(sin(x4)+x5*cos(x4))*x3)*sin(2*x4)/(x1*9.80665)'
0.95±σ0.019 0.5±σ0.005 36.4699040743±σ0.28867513459_° 0.1566±σ0.00028867513459 19.5555555556±σ0.075274616882 0.0053616514621±σ0.0000021642064708
ⓁσRFxjxi
@ Expecting 8.00006 60455 6±σ0.16765 20304 97
```

**6)** Ten shots at 15.66 cm:

```rpl
[[7.98] [7.83] [8.00] [7.87] [8.01] [8.14] [8.14] [8.12] [7.83] [8.06]] 'ΣData' STO
Average SDev 10 √ / →σRange
@ Expecting 7.998±σ3.83492 72048 7⁳⁻²
```

**7)** Prediction and observation compared:

```rpl
8.00006604556±σ0.167652030497 7.998±σ0.0383492720487 ⓁσConcord
```

The difference is two millimetres, p is close to 1: the realistic model is
confirmed. Its prediction is less precise than the ideal one, mostly because
of the fraction f, known to 2 %: a series of shots like this one is also a way
to measure f better.

**Do the predictions frame the shots?** The prediction ±2 standard deviations
is an interval of about 95 %. ΔConcord compares it with the range of the ten
shots, from the shortest to the longest:

**8)** The ideal model at 15.00 cm: no shot is in the predicted interval.

```rpl
7.13…7.42 8±0.10037637 ⓁΔConcord
```

**9)** The realistic model at 15.66 cm: all the shots are in it.

```rpl
7.83…8.14 8.00006604556±0.33530406 ⓁΔConcord
```

A model is never proven right: it is confirmed as long as it survives the
comparison, and a test able to reject a model, as in H-i, is what gives weight
to the agreement of the other one, in H-ii. The comparison of a prediction and a measurement, each with its uncertainty,
is the core of the validation methods of metrology, such as the normalized
error Eₙ of ISO 13528 and the validation comparison of ASME V&V 20.
