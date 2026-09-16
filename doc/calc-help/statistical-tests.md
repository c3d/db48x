# Statistical tests

The StatTests section of the Function Library turns the Student, chi-square and
Fisher distributions into the tests that are actually performed with them. The
machinery was already there: ΣData and its sums, LR, Corr on one side,
and the three upper tails UTPT, UTPC and UTPF on the other. What was
missing was the bridge — the statistic to form, and which tail to read.

Entries are grouped by the distribution they lean on, which is also how one
remembers them: a comparison of means is a Student test, a count or a variance
is a chi-square, a ratio of variances is a Fisher.

**What they return.** The statistic, its degrees of freedom, and the p-value,
tagged, one per stack level. **No verdict.** A test does not decide; it
measures how surprising the data would be if the null hypothesis were true.
Whether 0.035 is small enough is a question about the cost of being wrong, and
that belongs to the person, not to the calculator. The examples below say what
the number means at the usual thresholds, and where the answer would change.

**What they take.** Summary statistics — a count, a mean, a standard deviation
— because that is what one reads off a table or a report. As with the
Probability section, the values may be given bare or bundled in a list of the
right length, and the length is checked before anything is computed. Three
entries need the data themselves and say so: `Chi2GOF` takes two lists,
`Chi2Indep` a matrix, `Anova1` a list of lists. Two more take nothing at all and
read the current ΣData: `TTestLR` and `FTestLR`.

The p-value is two-sided for the Student tests, since the usual question is
whether two things differ rather than which is larger; halve it for a one-sided
question. For chi-square and for the analysis of variance it is the upper tail,
which is the only direction those tests can point.

**How to read the p-value.** It is the probability of observing data at least
as extreme as those at hand **if H₀ were true**. A threshold α is fixed
beforehand, and the rule is short:

* **p < α** — reject H₀, and conclude in favour of H₁. The data would be
  surprising if H₀ held.
* **p ≥ α** — do **not** reject H₀. That is not the same as concluding that H₀
  is true: a test that sees nothing may be looking at too few observations.
  Absence of evidence is not evidence of absence.

That asymmetry is the whole discipline of the exercise. A test can convict; it
cannot acquit.

α = 0.05 is a **convention, not a law** — it descends from a remark of Fisher's
and from the tables that could be printed in 1925. Choose it by what the two
mistakes cost: rejecting a true H₀ (a false alarm) against failing to reject a
false one (a miss). A screening test that must miss nothing wants a generous
α; a claim that will be expensive to act on wants 0.01 or less. Fix it
**before** looking at the data — choosing the threshold afterwards, so that the
answer comes out right, is the oldest way to fool oneself with statistics.

Two misreadings worth naming. The p-value is **not** the probability that H₀ is
true; it is computed by assuming H₀ and asking about the data, not the reverse.
And a result that is statistically significant is not thereby important: with a
large enough sample, a difference too small to matter will cross any threshold.
Read the statistic as well as its p-value.

**A word about the examples.** Each one ends with `3 →List` or `4 →List`. That
is an artifice of this documentation, which needs a single value to check
against, and it is **not to be typed**: called from the keyboard, an entry
leaves its results one per stack level, which is what one wants. Typing it
anyway costs more than an odd display — →List becomes the last command to
declare an arity, so LastArguments reports what →List consumed, the results
and the count, instead of what the test consumed. Call the test by itself and
LastArguments answers with the list of its own arguments, which is also what
the Args entries accept.


## StatTestsLibrary

Eleven tests built on the Student, chi-square and Fisher distributions, in three
submenus: [StudentTests](#studenttestslibrary) compares means, [Chi2Tests](#chi2testslibrary) handles counts and
variances, [FisherTests](#fishertestslibrary) compares variances and runs the analysis of
variance.

Every entry returns the statistic, its degrees of freedom and the p-value,
tagged, and **no verdict**. The rule is short, and its two halves are not
symmetric:

* **p < α** — reject H₀, conclude in favour of H₁.
* **p ≥ α** — do **not** reject H₀. That is not the same as establishing it: a
  test that sees nothing may be looking at too few observations.

A test can convict; it cannot acquit.

α = 0.05 is a **convention, not a law**. Choose it by what the two mistakes
cost — a false alarm against a miss — and fix it **before** looking at the data.
Choosing the threshold afterwards, so that the answer comes out right, is the
oldest way to fool oneself with statistics.

Two misreadings worth naming. The p-value is **not** the probability that H₀ is
true: it is computed by assuming H₀ and asking about the data, not the reverse.
And statistical significance is not importance — with a large enough sample, a
difference too small to matter crosses any threshold. Read the statistic as
well as its p-value.

Arguments follow the convention of the Probability section: bare or bundled in
a list of the right length, whose length is checked before anything is
computed. In the examples a trailing →List gathers the results so that one
value can be checked; it is not to be typed.


## StudentTestsLibrary

Tests that compare means, when the standard deviation of the population is
unknown and estimated from the sample — which is what separates them from the
normal test. Which one to reach for:

* one sample against a reference value — [TTest1](#ttest1);
* two independent samples — [TTest2](#ttest2) if their variances may be taken
  as equal, [TTestW](#ttestw) otherwise. When in doubt use TTestW: assuming a
  common variance that does not exist is the usual way to get this wrong, and
  [FTestVar](#ftestvar) is there to check;
* the same subjects measured twice — [TTestP](#ttestp), never TTest2;
* the slope of a regression — [TTestLR](#ttestlr).

All of them return a two-sided p-value; halve it for a one-sided question.
See [StatTests](#stattestslibrary) for what that p-value does and does not say.


## Chi2TestsLibrary

Tests on counts and on variance. The statistic is a sum of squared departures
divided by what was expected, and the p-value is the upper tail — the only
direction these tests can point.

* observed counts against a theoretical distribution — [Chi2GOF](#chi2gof);
* two qualitative variables in a contingency table — [Chi2Indep](#chi2indep),
  which also answers the test of homogeneity, the arithmetic being the same;
* a variance against a target — [Chi2Var](#chi2var).

The customary condition for the first two is an expected count of at least five
in every cell. The third is unusually sensitive to non-normality, more so than
the tests of means.

See [StatTests](#stattestslibrary) for how to read the p-value.


## FisherTestsLibrary

Tests built on a ratio of variances.

* two variances against each other — [FTestVar](#ftestvar), larger one first;
* three or more group means at once — [Anova1](#anova1), which asks the
  question once instead of running several Student tests, each with its own
  risk of a false alarm;
* the regression as a whole — [FTestLR](#ftestlr), which in a simple regression
  is [TTestLR](#ttestlr) written twice: F is exactly t².

The analysis of variance says that the groups differ, never which one differs.
For that, compare pairs afterwards — and remember that several comparisons
inflate the very risk Anova1 was chosen to control.

See [StatTests](#stattestslibrary) for how to read the p-value.


## TTest1

One-sample **t** test: is the mean of a sample compatible with a reference
value? Use it when the standard deviation of the population is unknown and
estimated from the sample itself — the usual case, and what separates **t**
from the normal test.

* **H₀**: μ = μ₀, the sample comes from a population whose mean is the reference.
* **H₁**: μ ≠ μ₀.

Conditions: a random sample, and either a roughly normal population or a sample
large enough for its mean to be normal.

t = (x̄ − μ₀) / (s/√n), with n − 1 degrees of freedom.
Stack: `n`, `xbar`, `s`, `mu0` — or `{ n xbar s mu0 }`.

**Example.** A filling machine is set to 500 mL. Sixteen bottles are measured:
mean 498.2 mL, standard deviation 3.1 mL. Is the machine off target?

```rpl
{ 16 498.2 3.1 500 } ⓁTTest1 3 →List
@ Expecting { t:-2.32258 06451 6 df:15 p:0.03467 53357 28 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.035 is below 0.05: at the five per cent level the difference is
significant, H₀ is rejected, the machine fills short. At one per cent it is not
— 0.035 > 0.01 — and the evidence is too thin to conclude. That the verdict
turns on the threshold is precisely why this entry returns a p-value.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## TTest1Args

Labels a row of values with the parameter names of `TTest1`. It takes what the
test takes and hands it back named, which is the way to see what a call
consumed: LastArguments immediately before it, or Undo and then this.
Stack: `n`, `xbar`, `s`, `mu0` — or `{ n xbar s mu0 }`.

```rpl
{ 16 498.2 3.1 500 } ⓁTTest1Args 4 →List
@ Expecting { n:16 xbar:498.2 s:3.1 mu0:500 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## TTest2

Two independent samples, assuming the two populations share one variance
(pooled **t** test). The variance is estimated once from both samples, which is
why it is the more powerful of the two when the assumption holds — and
misleading when it does not. `FTestVar` is there to check it; `TTestW` is there
when it fails.

* **H₀**: μ₁ = μ₂.
* **H₁**: μ₁ ≠ μ₂.

Conditions: two independent random samples, roughly normal populations, and
comparable variances.

s²ₚ = ((n₁−1)s₁² + (n₂−1)s₂²)/(n₁+n₂−2), t = (x̄₁ − x̄₂)/√(s²ₚ(1/n₁ + 1/n₂)),
with n₁ + n₂ − 2 degrees of freedom.
Stack: `n1`, `xbar1`, `s1`, `n2`, `xbar2`, `s2` — or the same six in a list.

**Example.** A treated group of twelve averages 102.4 with standard deviation
4.1; a control group of fifteen averages 98.7 with 3.6. Does the treatment
change the mean?

```rpl
{ 12 102.4 4.1 15 98.7 3.6 } ⓁTTest2 3 →List
@ Expecting { t:2.49561 73768 5 df:25 p:0.01953 43252 08 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.020 rejects H₀ at five per cent, and stays significant at 2.5 per cent.
Compare with `TTestW` on the same data below: the conclusion holds either way,
which is the comfortable case.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## TTest2Args

Labels a row of values with the parameter names of `TTest2`.
Stack: `n1`, `xbar1`, `s1`, `n2`, `xbar2`, `s2` — or the same six in a list.

```rpl
{ 12 102.4 4.1 15 98.7 3.6 } ⓁTTest2Args 6 →List
@ Expecting { n1:12 xbar1:102.4 s1:4.1 n2:15 xbar2:98.7 s2:3.6 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## TTestW

Two independent samples **without** assuming equal variances — the Welch test.
It takes the same six numbers as `TTest2` and differs only in how it weighs
them, and in the degrees of freedom, which are no longer a whole number.

* **H₀**: μ₁ = μ₂.
* **H₁**: μ₁ ≠ μ₂.

Prefer it whenever the variances are visibly unequal or the samples are of very
different sizes. Assuming a common variance that does not exist is the most
common way to get a two-sample test wrong.

t = (x̄₁ − x̄₂)/√(s₁²/n₁ + s₂²/n₂), with the Welch–Satterthwaite degrees of
freedom (s₁²/n₁ + s₂²/n₂)² / ((s₁²/n₁)²/(n₁−1) + (s₂²/n₂)²/(n₂−1)).
Stack: `n1`, `xbar1`, `s1`, `n2`, `xbar2`, `s2` — or the same six in a list.

**Example.** The same two groups as `TTest2`.

```rpl
{ 12 102.4 4.1 15 98.7 3.6 } ⓁTTestW 3 →List
@ Expecting { t:2.45857 57854 8 df:22.13696 48467 p:2.22338 52905 6⁳⁻² }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

The degrees of freedom are 22.14 rather than 25, and the p-value rises from
0.020 to 0.022 — the price of not assuming what was not verified. Both reject
H₀ at five per cent here. When the two disagree, trust this one.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## TTestWArgs

Labels a row of values with the parameter names of `TTestW`.
Stack: `n1`, `xbar1`, `s1`, `n2`, `xbar2`, `s2` — or the same six in a list.

```rpl
{ 12 102.4 4.1 15 98.7 3.6 } ⓁTTestWArgs 6 →List
@ Expecting { n1:12 xbar1:102.4 s1:4.1 n2:15 xbar2:98.7 s2:3.6 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## TTestP

Paired samples: the same subjects measured twice, before and after. What is
tested is the mean of the **differences**, not the difference of the means — so
the entry takes the count, the mean difference and the standard deviation of the
differences, which is one sample's worth of numbers.

* **H₀**: the mean difference is zero.
* **H₁**: it is not.

Pairing is what makes the test powerful: each subject is its own control, and
the variability between subjects drops out. Never feed paired data to `TTest2`.

t = d̄/(s_d/√n), with n − 1 degrees of freedom.
Stack: `n`, `dbar`, `sd` — or `{ n dbar sd }`.

**Example.** Ten patients are weighed before and after a diet. The mean change
is −1.85 kg with a standard deviation of 2.34 kg. Did the diet do anything?

```rpl
{ 10 -1.85 2.34 } ⓁTTestP 3 →List
@ Expecting { t:-2.50009 13125 3 df:9 p:0.03385 67547 17 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.034 rejects H₀ at five per cent: the loss is more than the scatter would
produce by chance. Note that the same 1.85 kg with ten *unpaired* patients
would say much less, because the spread between people would swamp it.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## TTestPArgs

Labels a row of values with the parameter names of `TTestP`.
Stack: `n`, `dbar`, `sd` — or `{ n dbar sd }`.

```rpl
{ 10 -1.85 2.34 } ⓁTTestPArgs 3 →List
@ Expecting { n:10 dbar:-1.85 sd:2.34 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## TTestLR

Is the slope of the linear regression different from zero? It takes no
argument and reads the current ΣData, the same data LR works on, so the
usual sequence is to fill ΣData with Σ+ or STOΣ and then call this.

* **H₀**: the slope is zero — x tells nothing about y.
* **H₁**: it is not.

Conditions: the usual ones of linear regression — a linear relation, residuals
of constant spread and roughly normal.

With Sₓₓ = Σx² − (Σx)²/n and the like, b = Sₓᵧ/Sₓₓ and
s²ₑ = (Sᵧᵧ − bSₓᵧ)/(n−2), the statistic is t = b/√(s²ₑ/Sₓₓ) with n − 2 degrees
of freedom.

**Example.** Six points that rise almost in a straight line.

```rpl
[[ 1 2.1 ][ 2 3.9 ][ 3 6.2 ][ 4 7.8 ][ 5 10.3 ][ 6 11.7 ]] STOΣ
ⓁTTestLR 3 →List
@ Expecting { t:32.28943 99071 df:4 p:5.48450 3526⁳⁻⁶ }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p is five parts in a million: a slope that flat a chance would almost never
produce. LR on the same data reports a slope near 1.966 — this entry says how
firmly that number stands.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## FTestLR

Overall significance of the linear regression on ΣData: does the model
explain a real share of the variance of y? Like `TTestLR` it takes no argument.

* **H₀**: the model explains nothing.
* **H₁**: it explains something.

F = b²Sₓₓ/s²ₑ with 1 and n − 2 degrees of freedom.

**Example.** The same six points.

```rpl
[[ 1 2.1 ][ 2 3.9 ][ 3 6.2 ][ 4 7.8 ][ 5 10.3 ][ 6 11.7 ]] STOΣ
ⓁFTestLR 4 →List
@ Expecting { F:1 042.60792 952 df1:1 df2:4 p:5.48450 3526⁳⁻⁶ }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

In a simple regression this test and `TTestLR` are the same test written
twice: F is exactly t², and the two p-values agree to the last digit. The
distinction only becomes real with several predictors, where F asks about the
model as a whole and each t asks about one coefficient.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## Chi2GOF

Goodness of fit: do the observed counts follow the expected ones? Takes two
lists of the same length — observed first, then expected — and refuses them if
the lengths differ.

* **H₀**: the counts follow the expected distribution.
* **H₁**: they do not.

Conditions: counts, not measurements, and expected values large enough — the
customary rule is at least five in every cell. The degrees of freedom are
k − 1; subtract one more for every parameter estimated from the same data, which
this entry cannot know about and leaves to the user.

χ² = Σ (O − E)²/E.

**Example.** A die is thrown sixty times and gives 8, 12, 9, 14, 7, 10. Is it
loaded?

```rpl
{ 8 12 9 14 7 10 } { 10 10 10 10 10 10 } ⓁChi2GOF 3 →List
@ Expecting { chi2:3.4 df:5 p:0.63856 99231 04 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.64: a fair die produces a departure this large about two times in
three. Nothing here suggests loading. A large p-value is not proof of fairness
— sixty throws simply cannot see a small bias.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## Chi2Indep

Independence of two qualitative variables, from a contingency table given as a
matrix of observed counts. The expected count of a cell is its row total times
its column total divided by the grand total — what independence would predict.

* **H₀**: the two variables are independent.
* **H₁**: they are associated.

The same computation answers the test of **homogeneity** — whether several
samples come from the same population — which differs from independence in how
the data were collected, not in the arithmetic.

χ² = ΣΣ (O − E)²/E, with (r − 1)(c − 1) degrees of freedom.

**Example.** Two groups and three preferences: 30, 20, 50 in the first row and
20, 30, 30 in the second.

```rpl
[[ 30 20 50 ][ 20 30 30 ]] ⓁChi2Indep 3 →List
@ Expecting { chi2:6.8625 df:2 p:0.03234 64823 89 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.032 rejects independence at five per cent: preference and group are
associated. The test says that they are, never how — look back at the table,
where the first group prefers the third column and the second the second.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## Chi2Var

Variance of a normal population against a target value. The test a machinist
wants when the average is right but the scatter is suspect.

* **H₀**: σ = σ₀.
* **H₁**: σ ≠ σ₀.

This test is unusually sensitive to non-normality — more so than the tests of
means — so a visibly skewed sample makes it unreliable.

χ² = (n−1)s²/σ₀², with n − 1 degrees of freedom. The p-value returned is the
upper tail, which answers “is the spread larger than the target?”; for the
two-sided question double the smaller of the two tails.
Stack: `n`, `s`, `sigma0` — or `{ n s sigma0 }`.

**Example.** Twenty parts show a standard deviation of 5.6 μm where the process
is supposed to hold 4.5 μm.

```rpl
{ 20 5.6 4.5 } ⓁChi2Var 3 →List
@ Expecting { chi2:29.42419 75309 df:19 p:0.05959 90104 14 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.060, just above five per cent. The process looks worse than its target
but not yet provably so — the honest answer is to measure more parts rather
than to argue about the threshold.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## Chi2VarArgs

Labels a row of values with the parameter names of `Chi2Var`.
Stack: `n`, `s`, `sigma0` — or `{ n s sigma0 }`.

```rpl
{ 20 5.6 4.5 } ⓁChi2VarArgs 3 →List
@ Expecting { n:20 s:5.6 sigma0:4.5 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## FTestVar

Equality of two variances, the ratio of one to the other. Often run before a
two-sample **t** test, to choose between `TTest2` and `TTestW`.

* **H₀**: σ₁ = σ₂.
* **H₁**: they differ.

**Put the larger variance first.** The entry forms s₁²/s₂² as given and reads
the upper tail, doubled for the two-sided question; with the smaller on top the
p-value would be meaningless. Like `Chi2Var`, this test is sensitive to
departures from normality.

F = s₁²/s₂², with n₁ − 1 and n₂ − 1 degrees of freedom.
Stack: `n1`, `s1`, `n2`, `s2` — or `{ n1 s1 n2 s2 }`.

**Example.** Sixteen measurements scatter by 7.4, twenty-one others by 4.9.

```rpl
{ 16 7.4 21 4.9 } ⓁFTestVar 4 →List
@ Expecting { F:2.28071 63681 8 df1:15 df2:20 p:8.63010 89576 4⁳⁻² }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.086: the ratio is not significant at five per cent, so the equal-variance
assumption of `TTest2` survives — though 0.086 is close enough to be uneasy
about, and `TTestW` costs nothing.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).

## FTestVarArgs

Labels a row of values with the parameter names of `FTestVar`.
Stack: `n1`, `s1`, `n2`, `s2` — or `{ n1 s1 n2 s2 }`.

```rpl
{ 16 7.4 21 4.9 } ⓁFTestVarArgs 4 →List
@ Expecting { n1:16 s1:7.4 n2:21 s2:4.9 }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

## Anova1

One-way analysis of variance: are three or more group means equal? Takes one
list of lists, one inner list per group, and the groups need not be the same
size.

* **H₀**: all the group means are equal.
* **H₁**: at least one differs.

The idea is a ratio: the variance **between** the group means against the
variance **within** the groups. If the groups really came from one population,
the two estimate the same thing and the ratio sits near one.

Why not several **t** tests? Because each one carries its own risk of a false
alarm, and three tests at five per cent give better than one chance in seven of
crying wolf. The analysis of variance asks the question once.

F = (SS_B/(k−1)) / (SS_W/(N−k)), with k − 1 and N − k degrees of freedom.

Conditions: independent groups, roughly normal, of comparable variance.

**Example.** Three groups of four: 5 7 6 9, then 8 9 11 10, then 12 10 13 11.

```rpl
{ { 5 7 6 9 } { 8 9 11 10 } { 12 10 13 11 } } ⓁAnova1 4 →List
@ Expecting { F:10.92 df1:2 df2:9 p:3.91811 00516 8⁳⁻³ }
```

The trailing →List only gathers the results so that one value can be checked here — do not type it: from the keyboard they arrive one per stack level, and it would displace LastArguments.

p = 0.0039 rejects equality well below one per cent. What it does **not** say
is which group differs from which — for that, compare pairs with `TTest2` or
`TTestW`, and remember that several comparisons inflate the risk that this
single test was designed to control.

How to read this p-value, and why α = 0.05 is a choice rather than a law: see [StatTests](#stattestslibrary).
