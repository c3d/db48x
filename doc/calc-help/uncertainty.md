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
@ Expecting 10.1±σ0.11547 00538 38
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
