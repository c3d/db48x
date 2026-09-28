# Probability functions

The Probability section of the Function Library holds the thirty
distributions of the Equation Library, one submenu per law, as ordinary
functions callable from a program. Each law offers its density, its distribution
function, the upper tail, the quantile, a random draw, its two moments, and two
entries that answer with names rather than with bare numbers.

They exist because an equation system is not always what one wants: a value
needed inside a computation is easier to obtain from a function than from a
solve, and the quantile of a discrete law cannot be obtained by solving at all,
a staircase having no slope for the solver to follow.

Parameters go on the stack first, in the order the Equation Library lists them,
and the varying argument last — X for the three direct functions, a probability
for the quantile. The two moment functions and the draw take the parameters
alone, since that is all they depend on.

**Tags, and the two entries that use them.** A five-parameter law leaves five
bare numbers on the stack and nothing says which is which. `<Law>Args` gives
them their names back: `0 1 -1 2 0.5 ⓁNormTruncArgs` returns `x0:0 sc:1
Xmin:-1 Xmax:2 X:0.5`.

Args labels; it does not recall. A distribution function consumes its arguments
like any other, so calling Args straight after one finds nothing left to label
and says Too few arguments. Undo is what brings the values back, and the two
belong together:

    0 ENTER 1 ENTER 1.5 ENTER
    ⓁNormlPDF       gives 0.12951 75956 66
    Undo            brings back 0, 1 and 1.5
    ⓁNormlArgs      gives μ:0 sx:1 X:1.5

This works on values you enter yourself, one line at a time. It does not work on
an example transferred from this help: the whole block arrives in the editor at
once and runs as a single command line, and the stack is saved once per command
line, so there is nothing between the values and the result for Undo to return
to. Nothing in RPL can force a save either — it is a gesture of the editor, not
of the language.

**Recalling the arguments themselves.** `LastArguments` reports what a call
consumed, as a list, and unlike Undo it works inside a single command line -
including a block transferred from this help:

```rpl
0 1 1.5 ⓁNormlPDF LastArguments
@ Expecting { 0 1 1.5 }
```

The density is left on the stack below.

**Loose or bundled.** Every entry accepts its arguments either way: five bare
numbers, or one list of five. So the list travels, and can be named:

```rpl
0 1 1.5 ⓁNormlPDF LastArguments ⓁNormlArgs
@ Expecting X:1.5
```

μ:0 and sx:1 are left below it. Stored under a name, the same bundle serves
every entry of that law - `'P' STO` then `P ⓁNormlCDF`, `P ⓁNormlQ`.

A list is checked for length before use, which bare values cannot be. A bundle
of the wrong size says so and stays on the stack for you to redirect, instead
of being consumed to compute something nobody asked for.

`<Law>Mu`, `<Law>Sx` and `<Law>Rand` take the parameters alone, so they accept
a bundle of their own size or one element longer, dropping the trailing varying
argument. A point computed with `ⓁNormlPDF` can therefore be drawn from without
retyping anything:

```rpl
0 1 1.5 ⓁNormlPDF LastArguments ⓁNormlArgs 3 →List
@ Expecting { μ:0 sx:1 X:1.5 }
```

That works only because the parameters come first and the varying argument
last, so the surplus element is always the one to drop. Anything shorter, or
longer by more than one, is refused by name.

This is why the entries end by pushing their own arguments and dropping them
again. Without that suffix `LastArguments` reports the last value the program
computed internally - an expression full of `LocalVariable0` - rather than what
the caller supplied. That is issue #1738. `PhitMu`, `PhitSx` and `PhitRand` take
no arguments, so there is nothing for them to report.

`<Law>All` goes further and answers everything about one point in a single call,
tagged the same way — distribution function, density, upper tail, mean and
standard deviation.

Tagged values are accepted everywhere in this section: each entry strips the
tags from its arguments before using them, so a labelled row can be passed
straight back in. `50 10 5 1 ⓁHyperArgs ⓁHyperCDF` gives 0.74189 99792 33,
exactly as the unlabelled call does.

**The moments are the reason to prefer these to a table.** Mean and standard
deviation are computed from the parameters rather than looked up, which is what
makes the section usable for propagating uncertainty: the mean and deviation of
a truncated normal are not those of the normal it was cut from, and the two
functions say so. Where a law has no moments the function returns the undefined
constant rather than a plausible number — Cauchy is the case that matters.

**How the quantiles are obtained.** Nine laws have a closed-form inverse and use
it: Expon, Weibl, Rayleigh, Logis, Pareto, UnifCont, GEV, Bernoulli and UnifDis.
Those are exact and instant. Sixteen bisect over an interval that brackets the
whole support, then take two Newton steps with the density as the derivative;
they cannot fail, since a monotone function from zero to one is bracketed by
construction. The remaining five are discrete and add up the mass function until
it reaches the requested probability.

Chi2, Gamma and Beta once had no closed-form distribution function, integrated at
every step, and cost seconds. They now call the GammaP and BetaI primitives and
return in about a tenth of a second, like most of the others. Student and Fisher
are the slowest at roughly four tenths.

**Drawing from a law.** `<Law>Rand` takes the parameters alone and returns one
draw. It works by inversion: a uniform variate is fed to the law's own quantile,
so nothing new is assumed about the law and every guard already built into the
quantile still applies. The cost is therefore the cost of that quantile.

Seed the generator with RDZ to make a sequence reproducible. The examples in this
section all seed with 42, which is why their results can be checked at all;
without a seed every run differs. To collect several draws, put the call in a
loop: `42 rdz 1 5 START 0 1 ⓁNormlRand NEXT 5 →List` returns five standard
normal variates.

For the seven discrete laws the quantile returns the smallest integer m for
which CDF(m) is at least the probability given, as R's qbinom does.

These entries live on the calculator's disk, under Probability in the LIB menu:
the files are library/Probability/*.48s and config/library.csv declares them.
They therefore cost no program memory at all.

## NormlPDF

Density of the Norml distribution at X. Stack: `μ`, `sx`, `X` — or `{ μ sx X }`.

![Normal Distribution](img/Norml_Distribution.bmp)

```rpl
0 1 1.5 ⓁNormlPDF
@ Expecting 0.12951 75956 66
```

## NormlCDF

Distribution function of Norml: the probability of not exceeding X. Stack: `μ`, `sx`, `X` — or `{ μ sx X }`.

```rpl
0 1 1.5 ⓁNormlCDF
@ Expecting 0.93319 27987 31
```

## NormlCDFc

Upper tail of Norml, the complement of its distribution function. Stack: `μ`, `sx`, `X` — or `{ μ sx X }`.

```rpl
0 1 1.5 ⓁNormlCDFc
@ Expecting 0.06680 72012 69
```

## NormlQ

Quantile of Norml: the value below which the probability is the one given. Stack: `μ`, `sx`, `p` — or `{ μ sx p }`.

```rpl
0 1 0.933192798731 ⓁNormlQ
@ Expecting 1.5
```

## NormlRand

Random draw from the Norml distribution. A uniform variate is drawn and fed to NormlQ, so the draw follows the law by inversion. Stack: `μ`, `sx` — or `{ μ sx }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁNormlRand
@ Expecting 0.44143 17567 34
```

## NormlArgs

Labels a row of values with the parameter names of Norml. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `μ`, `sx`, `X` — or `{ μ sx X }`.

```rpl
0 1 1.5 ⓁNormlArgs 3 →List
@ Expecting { μ:0 sx:1 X:1.5 }
```

## NormlAll

Everything Norml has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `μ`, `sx`, `X` — or `{ μ sx X }`.

```rpl
0 1 1.5 ⓁNormlAll 5 →List
@ Expecting { CDF:0.93319 27987 31 PDF:0.12951 75956 66 CDFc:0.06680 72012 69 μ:0 sx:1 }
```

## LgNrmPDF

Density of the LgNrm distribution at X. Stack: `μl`, `σl`, `X` — or `{ μl σl X }`.

![Lognormal Distribution](img/LgNrm_Distribution.bmp)

```rpl
0 1 1.5 ⓁLgNrmPDF
@ Expecting 0.24497 36517 11
```

## LgNrmCDF

Distribution function of LgNrm: the probability of not exceeding X. Stack: `μl`, `σl`, `X` — or `{ μl σl X }`.

```rpl
0 1 1.5 ⓁLgNrmCDF
@ Expecting 0.65743 21694 85
```

## LgNrmCDFc

Upper tail of LgNrm, the complement of its distribution function. Stack: `μl`, `σl`, `X` — or `{ μl σl X }`.

```rpl
0 1 1.5 ⓁLgNrmCDFc
@ Expecting 0.34256 78305 15
```

## LgNrmQ

Quantile of LgNrm: the value below which the probability is the one given. Stack: `μl`, `σl`, `p` — or `{ μl σl p }`.

```rpl
0 1 0.657432169485 ⓁLgNrmQ
@ Expecting 1.5
```

## LgNrmMu

Mean of the LgNrm distribution, computed from its parameters alone. Stack: `μl`, `σl` — or `{ μl σl }`.

```rpl
0 1 ⓁLgNrmMu
@ Expecting 1.64872 12707
```

## LgNrmSx

Standard deviation of the LgNrm distribution, computed from its parameters alone. Stack: `μl`, `σl` — or `{ μl σl }`.

```rpl
0 1 ⓁLgNrmSx
@ Expecting 2.16119 74159
```

## LgNrmRand

Random draw from the LgNrm distribution. A uniform variate is drawn and fed to LgNrmQ, so the draw follows the law by inversion. Stack: `μl`, `σl` — or `{ μl σl }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁLgNrmRand
@ Expecting 1.55493 19097 6
```

## LgNrmArgs

Labels a row of values with the parameter names of LgNrm. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `μl`, `σl`, `X` — or `{ μl σl X }`.

```rpl
0 1 1.5 ⓁLgNrmArgs 3 →List
@ Expecting { μl:0 σl:1 X:1.5 }
```

## LgNrmAll

Everything LgNrm has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `μl`, `σl`, `X` — or `{ μl σl X }`.

```rpl
0 1 1.5 ⓁLgNrmAll 5 →List
@ Expecting { CDF:0.65743 21694 85 PDF:0.24497 36517 11 CDFc:0.34256 78305 15 μ:1.64872 12707 sx:2.16119 74159 }
```

## ExponPDF

Density of the Expon distribution at X. Stack: `λ`, `X` — or `{ λ X }`.

![Exponential Distribution](img/Expon_Distribution.bmp)

```rpl
2 0.5 ⓁExponPDF
@ Expecting 0.73575 88823 43
```

## ExponCDF

Distribution function of Expon: the probability of not exceeding X. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
2 0.5 ⓁExponCDF
@ Expecting 0.63212 05588 29
```

## ExponCDFc

Upper tail of Expon, the complement of its distribution function. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
2 0.5 ⓁExponCDFc
@ Expecting 0.36787 94411 71
```

## ExponQ

Quantile of Expon: the value below which the probability is the one given. Stack: `λ`, `p` — or `{ λ p }`.

```rpl
2 0.632120558829 ⓁExponQ
@ Expecting 0.50000 00000 01
```

## ExponMu

Mean of the Expon distribution, computed from its parameters alone. Stack: `λ` — or `{ λ }`.

```rpl
2 ⓁExponMu
@ Expecting 0.5
```

## ExponSx

Standard deviation of the Expon distribution, computed from its parameters alone. Stack: `λ` — or `{ λ }`.

```rpl
2 ⓁExponSx
@ Expecting 0.5
```

## ExponRand

Random draw from the Expon distribution. A uniform variate is drawn and fed to ExponQ, so the draw follows the law by inversion. Stack: `λ` — or `{ λ }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 2 ⓁExponRand
@ Expecting 0.55516 49946 83
```

## ExponArgs

Labels a row of values with the parameter names of Expon. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
2 0.5 ⓁExponArgs 2 →List
@ Expecting { λ:2 X:0.5 }
```

## ExponAll

Everything Expon has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
2 0.5 ⓁExponAll 5 →List
@ Expecting { CDF:0.63212 05588 29 PDF:0.73575 88823 43 CDFc:0.36787 94411 71 μ:0.5 sx:0.5 }
```

## WeiblPDF

Density of the Weibl distribution at X. Stack: `sh`, `λ`, `X` — or `{ sh λ X }`.

![Weibull Distribution](img/Weibl_Distribution.bmp)

```rpl
2 1 1 ⓁWeiblPDF
@ Expecting 0.73575 88823 43
```

## WeiblCDF

Distribution function of Weibl: the probability of not exceeding X. Stack: `sh`, `λ`, `X` — or `{ sh λ X }`.

```rpl
2 1 1 ⓁWeiblCDF
@ Expecting 0.63212 05588 29
```

## WeiblCDFc

Upper tail of Weibl, the complement of its distribution function. Stack: `sh`, `λ`, `X` — or `{ sh λ X }`.

```rpl
2 1 1 ⓁWeiblCDFc
@ Expecting 0.36787 94411 71
```

## WeiblQ

Quantile of Weibl: the value below which the probability is the one given. Stack: `sh`, `λ`, `p` — or `{ sh λ p }`.

```rpl
2 1 0.632120558829 ⓁWeiblQ
@ Expecting 1.
```

## WeiblMu

Mean of the Weibl distribution, computed from its parameters alone. Stack: `sh`, `λ` — or `{ sh λ }`.

```rpl
2 1 ⓁWeiblMu
@ Expecting 0.88622 69254 53
```

## WeiblSx

Standard deviation of the Weibl distribution, computed from its parameters alone. Stack: `sh`, `λ` — or `{ sh λ }`.

```rpl
2 1 ⓁWeiblSx
@ Expecting 0.46325 13751 76
```

## WeiblRand

Random draw from the Weibl distribution. A uniform variate is drawn and fed to WeiblQ, so the draw follows the law by inversion. Stack: `sh`, `λ` — or `{ sh λ }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 2 1 ⓁWeiblRand
@ Expecting 1.05372 19696 7
```

## WeiblArgs

Labels a row of values with the parameter names of Weibl. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `sh`, `λ`, `X` — or `{ sh λ X }`.

```rpl
2 1 1 ⓁWeiblArgs 3 →List
@ Expecting { sh:2 λ:1 X:1 }
```

## WeiblAll

Everything Weibl has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `sh`, `λ`, `X` — or `{ sh λ X }`.

```rpl
2 1 1 ⓁWeiblAll 5 →List
@ Expecting { CDF:0.63212 05588 29 PDF:0.73575 88823 43 CDFc:0.36787 94411 71 μ:0.88622 69254 53 sx:0.46325 13751 76 }
```

## RayleighPDF

Density of the Rayleigh distribution at X. Stack: `s`, `X` — or `{ s X }`.

![Rayleigh Distribution](img/Rayleigh_Distribution.bmp)

```rpl
1 1 ⓁRayleighPDF
@ Expecting 0.60653 06597 13
```

## RayleighCDF

Distribution function of Rayleigh: the probability of not exceeding X. Stack: `s`, `X` — or `{ s X }`.

```rpl
1 1 ⓁRayleighCDF
@ Expecting 0.39346 93402 87
```

## RayleighCDFc

Upper tail of Rayleigh, the complement of its distribution function. Stack: `s`, `X` — or `{ s X }`.

```rpl
1 1 ⓁRayleighCDFc
@ Expecting 0.60653 06597 13
```

## RayleighQ

Quantile of Rayleigh: the value below which the probability is the one given. Stack: `s`, `p` — or `{ s p }`.

```rpl
1 0.393469340287 ⓁRayleighQ
@ Expecting 0.99999 99999 99
```

## RayleighMu

Mean of the Rayleigh distribution, computed from its parameters alone. Stack: `s` — or `{ s }`.

```rpl
1 ⓁRayleighMu
@ Expecting 1.25331 41373 2
```

## RayleighSx

Standard deviation of the Rayleigh distribution, computed from its parameters alone. Stack: `s` — or `{ s }`.

```rpl
1 ⓁRayleighSx
@ Expecting 0.65513 63775 62
```

## RayleighRand

Random draw from the Rayleigh distribution. A uniform variate is drawn and fed to RayleighQ, so the draw follows the law by inversion. Stack: `s` — or `{ s }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 1 ⓁRayleighRand
@ Expecting 1.49018 79004 8
```

## RayleighArgs

Labels a row of values with the parameter names of Rayleigh. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `s`, `X` — or `{ s X }`.

```rpl
1 1 ⓁRayleighArgs 2 →List
@ Expecting { s:1 X:1 }
```

## RayleighAll

Everything Rayleigh has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `s`, `X` — or `{ s X }`.

```rpl
1 1 ⓁRayleighAll 5 →List
@ Expecting { CDF:0.39346 93402 87 PDF:0.60653 06597 13 CDFc:0.60653 06597 13 μ:1.25331 41373 2 sx:0.65513 63775 62 }
```

## LogisPDF

Density of the Logis distribution at X. Stack: `μ`, `s`, `X` — or `{ μ s X }`.

![Logistic Distribution](img/Logis_Distribution.bmp)

```rpl
0 1 1 ⓁLogisPDF
@ Expecting 0.19661 19332 41
```

## LogisCDF

Distribution function of Logis: the probability of not exceeding X. Stack: `μ`, `s`, `X` — or `{ μ s X }`.

```rpl
0 1 1 ⓁLogisCDF
@ Expecting 0.73105 85786 3
```

## LogisCDFc

Upper tail of Logis, the complement of its distribution function. Stack: `μ`, `s`, `X` — or `{ μ s X }`.

```rpl
0 1 1 ⓁLogisCDFc
@ Expecting 0.26894 14213 7
```

## LogisQ

Quantile of Logis: the value below which the probability is the one given. Stack: `μ`, `s`, `p` — or `{ μ s p }`.

```rpl
0 1 0.73105857863 ⓁLogisQ
@ Expecting 1.
```

## LogisSx

Standard deviation of the Logis distribution, computed from its parameters alone. Stack: `μ`, `s` — or `{ μ s }`.

```rpl
0 1 ⓁLogisSx
@ Expecting 1.81379 93642 3
```

## LogisRand

Random draw from the Logis distribution. A uniform variate is drawn and fed to LogisQ, so the draw follows the law by inversion. Stack: `μ`, `s` — or `{ μ s }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁLogisRand
@ Expecting 0.71067 26414 26
```

## LogisArgs

Labels a row of values with the parameter names of Logis. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `μ`, `s`, `X` — or `{ μ s X }`.

```rpl
0 1 1 ⓁLogisArgs 3 →List
@ Expecting { μ:0 s:1 X:1 }
```

## LogisAll

Everything Logis has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `μ`, `s`, `X` — or `{ μ s X }`.

```rpl
0 1 1 ⓁLogisAll 5 →List
@ Expecting { CDF:0.73105 85786 3 PDF:0.19661 19332 41 CDFc:0.26894 14213 7 μ:0 sx:1.81379 93642 3 }
```

## ParetoPDF

Density of the Pareto distribution at X. Stack: `xm`, `ap`, `X` — or `{ xm ap X }`.

![Pareto Distribution](img/Pareto_Distribution.bmp)

```rpl
1 3 2 ⓁParetoPDF
@ Expecting 0.1875
```

## ParetoCDF

Distribution function of Pareto: the probability of not exceeding X. Stack: `xm`, `ap`, `X` — or `{ xm ap X }`.

```rpl
1 3 2 ⓁParetoCDF
@ Expecting 0.875
```

## ParetoCDFc

Upper tail of Pareto, the complement of its distribution function. Stack: `xm`, `ap`, `X` — or `{ xm ap X }`.

```rpl
1 3 2 ⓁParetoCDFc
@ Expecting 0.125
```

## ParetoQ

Quantile of Pareto: the value below which the probability is the one given. Stack: `xm`, `ap`, `p` — or `{ xm ap p }`.

```rpl
1 3 0.875 ⓁParetoQ
@ Expecting 2.
```

## ParetoMu

Mean of the Pareto distribution, computed from its parameters alone. Stack: `xm`, `ap` — or `{ xm ap }`.

The power law has a heavy tail, so the mean exists only above a certain index.
For `ap>1` it is `ap*xm/(ap-1)`; at `ap≤1` the integral diverges and the entry
answers with the infinite constant rather than a number that means nothing.

```rpl
1 3 ⓁParetoMu
@ Expecting 1.5
```

## ParetoSx

Standard deviation of the Pareto distribution, computed from its parameters alone. Stack: `xm`, `ap` — or `{ xm ap }`.

Three regimes, as in the Equation Library section. For `ap>2` the deviation is
the value below. For `1<ap≤2` the mean is finite but the variance integral
diverges, so the answer is the infinite constant. For `ap≤1` the mean is itself
infinite and there is nothing to take a deviation from: the answer is the
undefined constant, which is not the same as infinite.

```rpl
1 3 ⓁParetoSx
@ Expecting 0.86602 54037 84
```

## ParetoRand

Random draw from the Pareto distribution. A uniform variate is drawn and fed to ParetoQ, so the draw follows the law by inversion. Stack: `xm`, `ap` — or `{ xm ap }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 1 3 ⓁParetoRand
@ Expecting 1.44789 38691
```

## ParetoArgs

Labels a row of values with the parameter names of Pareto. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `xm`, `ap`, `X` — or `{ xm ap X }`.

```rpl
1 3 2 ⓁParetoArgs 3 →List
@ Expecting { xm:1 ap:3 X:2 }
```

## ParetoAll

Everything Pareto has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `xm`, `ap`, `X` — or `{ xm ap X }`.

```rpl
1 3 2 ⓁParetoAll 5 →List
@ Expecting { CDF:0.875 PDF:0.1875 CDFc:0.125 μ:1.5 sx:0.86602 54037 84 }
```

## CauchPDF

Density of the Cauch distribution at X. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

![Cauchy Distribution](img/Cauch_Distribution.bmp)

```rpl
0 1 1 ⓁCauchPDF
@ Expecting 0.15915 49430 92
```

## CauchCDF

Distribution function of Cauch: the probability of not exceeding X. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁCauchCDF
@ Expecting 0.75
```

## CauchCDFc

Upper tail of Cauch, the complement of its distribution function. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁCauchCDFc
@ Expecting 0.25
```

## CauchQ

Quantile of Cauch: the value below which the probability is the one given. Stack: `x0`, `sc`, `p` — or `{ x0 sc p }`.

```rpl
0 1 0.75 ⓁCauchQ
@ Expecting 1.
```

## CauchMu

Mean of the Cauch distribution, computed from its parameters alone. Stack: `x0`, `sc` — or `{ x0 sc }`.

```rpl
0 1 ⓁCauchMu
@ Expecting 'Undefined'
```

## CauchSx

Standard deviation of the Cauch distribution, computed from its parameters alone. Stack: `x0`, `sc` — or `{ x0 sc }`.

```rpl
0 1 ⓁCauchSx
@ Expecting 'Undefined'
```

## CauchRand

Random draw from the Cauch distribution. A uniform variate is drawn and fed to CauchQ, so the draw follows the law by inversion. Stack: `x0`, `sc` — or `{ x0 sc }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁCauchRand
@ Expecting 0.59373 19743 47
```

## CauchArgs

Labels a row of values with the parameter names of Cauch. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁCauchArgs 3 →List
@ Expecting { x0:0 sc:1 X:1 }
```

## CauchAll

Everything Cauch has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁCauchAll 5 →List
@ Expecting { CDF:0.75 PDF:0.15915 49430 92 CDFc:0.25 μ:'Undefined' sx:'Undefined' }
```

## UnifContPDF

Density of the UnifCont distribution at X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![Continuous Uniform Distribution](img/UnifCont_Distribution.bmp)

```rpl
0 1 0.4 ⓁUnifContPDF
@ Expecting 1
```

## UnifContCDF

Distribution function of UnifCont: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.4 ⓁUnifContCDF
@ Expecting 0.4
```

## UnifContCDFc

Upper tail of UnifCont, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.4 ⓁUnifContCDFc
@ Expecting 0.6
```

## UnifContQ

Quantile of UnifCont: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
0 1 0.4 ⓁUnifContQ
@ Expecting 0.4
```

## UnifContMu

Mean of the UnifCont distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁUnifContMu
@ Expecting 0.5
```

## UnifContSx

Standard deviation of the UnifCont distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁUnifContSx
@ Expecting 0.28867 51345 95
```

## UnifContRand

Random draw from the UnifCont distribution. A uniform variate is drawn and fed to UnifContQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁUnifContRand
@ Expecting 0.67054 97719 36
```

## UnifContArgs

Labels a row of values with the parameter names of UnifCont. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.4 ⓁUnifContArgs 3 →List
@ Expecting { Xmin:0 Xmax:1 X:0.4 }
```

## UnifContAll

Everything UnifCont has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.4 ⓁUnifContAll 5 →List
@ Expecting { CDF:0.4 PDF:1 CDFc:0.6 μ:0.5 sx:0.28867 51345 95 }
```

## TriSymPDF

Density of the TriSym distribution at X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![Symmetric Triangular Distribution](img/TriSym_Distribution.bmp)

```rpl
0 1 0.3 ⓁTriSymPDF
@ Expecting 1.2
```

## TriSymCDF

Distribution function of TriSym: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.3 ⓁTriSymCDF
@ Expecting 0.18
```

## TriSymCDFc

Upper tail of TriSym, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.3 ⓁTriSymCDFc
@ Expecting 0.82
```

## TriSymQ

Quantile of TriSym: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
0 1 0.18 ⓁTriSymQ
@ Expecting 0.3
```

## TriSymMu

Mean of the TriSym distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁTriSymMu
@ Expecting 0.5
```

## TriSymSx

Standard deviation of the TriSym distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁTriSymSx
@ Expecting 0.20412 41452 32
```

## TriSymRand

Random draw from the TriSym distribution. A uniform variate is drawn and fed to TriSymQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁTriSymRand
@ Expecting 0.59413 65820 48
```

## TriSymArgs

Labels a row of values with the parameter names of TriSym. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.3 ⓁTriSymArgs 3 →List
@ Expecting { Xmin:0 Xmax:1 X:0.3 }
```

## TriSymAll

Everything TriSym has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.3 ⓁTriSymAll 5 →List
@ Expecting { CDF:0.18 PDF:1.2 CDFc:0.82 μ:0.5 sx:0.20412 41452 32 }
```

## Chi2PDF

Density of the Chi2 distribution at X. Stack: `df`, `X` — or `{ df X }`.

![Chi-Square Distribution](img/Chi2_Distribution.bmp)

```rpl
4 3 ⓁChi2PDF
@ Expecting 0.16734 76201 11
```

## Chi2CDF

Distribution function of Chi2: the probability of not exceeding X. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 3 ⓁChi2CDF
@ Expecting 0.44217 45996 29
```

## Chi2CDFc

Upper tail of Chi2, the complement of its distribution function. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 3 ⓁChi2CDFc
@ Expecting 0.55782 54003 71
```

## Chi2Q

Quantile of Chi2: the value below which the probability is the one given. Stack: `df`, `p` — or `{ df p }`.

```rpl
4 0.442174599629 ⓁChi2Q
@ Expecting 3.
```

## Chi2Mu

Mean of the Chi2 distribution, computed from its parameters alone. Stack: `df` — or `{ df }`.

```rpl
4 ⓁChi2Mu
@ Expecting 4
```

## Chi2Sx

Standard deviation of the Chi2 distribution, computed from its parameters alone. Stack: `df` — or `{ df }`.

```rpl
4 ⓁChi2Sx
@ Expecting 2.82842 71247 5
```

## Chi2Rand

Random draw from the Chi2 distribution. A uniform variate is drawn and fed to Chi2Q, so the draw follows the law by inversion. Stack: `df` — or `{ df }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 4 ⓁChi2Rand
@ Expecting 4.61219 78103 9
```

## Chi2Args

Labels a row of values with the parameter names of Chi2. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 3 ⓁChi2Args 2 →List
@ Expecting { df:4 X:3 }
```

## Chi2All

Everything Chi2 has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 3 ⓁChi2All 5 →List
@ Expecting { CDF:0.44217 45996 29 PDF:0.16734 76201 11 CDFc:0.55782 54003 71 μ:4 sx:2.82842 71247 5 }
```

## BetaPDF

Density of the Beta distribution at X. Stack: `a`, `b`, `X` — or `{ a b X }`.

![Beta Distribution](img/Beta_Distribution.bmp)

```rpl
2 3 0.4 ⓁBetaPDF
@ Expecting 1.728
```

## BetaCDF

Distribution function of Beta: the probability of not exceeding X. Stack: `a`, `b`, `X` — or `{ a b X }`.

```rpl
2 3 0.4 ⓁBetaCDF
@ Expecting 0.5248
```

## BetaCDFc

Upper tail of Beta, the complement of its distribution function. Stack: `a`, `b`, `X` — or `{ a b X }`.

```rpl
2 3 0.4 ⓁBetaCDFc
@ Expecting 0.4752
```

## BetaQ

Quantile of Beta: the value below which the probability is the one given. Stack: `a`, `b`, `p` — or `{ a b p }`.

```rpl
2 3 0.5248 ⓁBetaQ
@ Expecting 0.4
```

## BetaMu

Mean of the Beta distribution, computed from its parameters alone. Stack: `a`, `b` — or `{ a b }`.

```rpl
2 3 ⓁBetaMu
@ Expecting 0.4
```

## BetaSx

Standard deviation of the Beta distribution, computed from its parameters alone. Stack: `a`, `b` — or `{ a b }`.

```rpl
2 3 ⓁBetaSx
@ Expecting 0.2
```

## BetaRand

Random draw from the Beta distribution. A uniform variate is drawn and fed to BetaQ, so the draw follows the law by inversion. Stack: `a`, `b` — or `{ a b }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 2 3 ⓁBetaRand
@ Expecting 0.48882 28829 11
```

## BetaArgs

Labels a row of values with the parameter names of Beta. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `a`, `b`, `X` — or `{ a b X }`.

```rpl
2 3 0.4 ⓁBetaArgs 3 →List
@ Expecting { a:2 b:3 X:0.4 }
```

## BetaAll

Everything Beta has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `a`, `b`, `X` — or `{ a b X }`.

```rpl
2 3 0.4 ⓁBetaAll 5 →List
@ Expecting { CDF:0.5248 PDF:1.728 CDFc:0.4752 μ:0.4 sx:0.2 }
```

## PhitPDF

Density of the Phit distribution at X. Stack: `X` — or `{ X }`.

![Standard Normal Distribution](img/Phit_Distribution.bmp)

```rpl
1 ⓁPhitPDF
@ Expecting 0.24197 07245 19
```

## PhitCDF

Distribution function of Phit: the probability of not exceeding X. Stack: `X` — or `{ X }`.

```rpl
1 ⓁPhitCDF
@ Expecting 0.84134 47460 69
```

## PhitCDFc

Upper tail of Phit, the complement of its distribution function. Stack: `X` — or `{ X }`.

```rpl
1 ⓁPhitCDFc
@ Expecting 0.15865 52539 31
```

## PhitQ

Quantile of Phit: the value below which the probability is the one given. Stack: `p` — or `{ p }`.

```rpl
0.841344746069 ⓁPhitQ
@ Expecting 1.
```

## PhitMu

Mean of the Phit distribution, computed from its parameters alone. Stack: nothing.

```rpl
ⓁPhitMu
@ Expecting 0
```

## PhitSx

Standard deviation of the Phit distribution, computed from its parameters alone. Stack: nothing.

```rpl
ⓁPhitSx
@ Expecting 1
```

## PhitRand

Random draw from the Phit distribution. A uniform variate is drawn and fed to PhitQ, so the draw follows the law by inversion. Stack: nothing.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz ⓁPhitRand
@ Expecting 0.44143 17567 34
```

## PhitArgs

Labels a row of values with the parameter names of Phit. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `X` — or `{ X }`.

```rpl
1 ⓁPhitArgs 1 →List
@ Expecting { X:1 }
```

## PhitAll

Everything Phit has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `X` — or `{ X }`.

```rpl
1 ⓁPhitAll 5 →List
@ Expecting { CDF:0.84134 47460 69 PDF:0.24197 07245 19 CDFc:0.15865 52539 31 μ:0 sx:1 }
```

## TriRightPDF

Density of the TriRight distribution at X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![Right Triangular Distribution](img/TriRight_Distribution.bmp)

```rpl
0 3 2 ⓁTriRightPDF
@ Expecting 0.44444 44444 44
```

## TriRightCDF

Distribution function of TriRight: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 2 ⓁTriRightCDF
@ Expecting 0.44444 44444 44
```

## TriRightCDFc

Upper tail of TriRight, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 2 ⓁTriRightCDFc
@ Expecting 0.55555 55555 56
```

## TriRightQ

Quantile of TriRight: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
0 3 0.444444444444 ⓁTriRightQ
@ Expecting 2.
```

## TriRightMu

Mean of the TriRight distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 3 ⓁTriRightMu
@ Expecting 2
```

## TriRightSx

Standard deviation of the TriRight distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 3 ⓁTriRightSx
@ Expecting 0.70710 67811 87
```

## TriRightRand

Random draw from the TriRight distribution. A uniform variate is drawn and fed to TriRightQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 3 ⓁTriRightRand
@ Expecting 2.45661 31049 5
```

## TriRightArgs

Labels a row of values with the parameter names of TriRight. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 2 ⓁTriRightArgs 3 →List
@ Expecting { Xmin:0 Xmax:3 X:2 }
```

## TriRightAll

Everything TriRight has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 2 ⓁTriRightAll 5 →List
@ Expecting { CDF:0.44444 44444 44 PDF:0.44444 44444 44 CDFc:0.55555 55555 56 μ:2 sx:0.70710 67811 87 }
```

## TriLeftPDF

Density of the TriLeft distribution at X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![Left Triangular Distribution](img/TriLeft_Distribution.bmp)

```rpl
0 3 1 ⓁTriLeftPDF
@ Expecting 0.44444 44444 44
```

## TriLeftCDF

Distribution function of TriLeft: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 1 ⓁTriLeftCDF
@ Expecting 0.55555 55555 56
```

## TriLeftCDFc

Upper tail of TriLeft, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 1 ⓁTriLeftCDFc
@ Expecting 0.44444 44444 44
```

## TriLeftQ

Quantile of TriLeft: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
0 3 0.555555555556 ⓁTriLeftQ
@ Expecting 1.
```

## TriLeftMu

Mean of the TriLeft distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 3 ⓁTriLeftMu
@ Expecting 1
```

## TriLeftSx

Standard deviation of the TriLeft distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 3 ⓁTriLeftSx
@ Expecting 0.70710 67811 87
```

## TriLeftRand

Random draw from the TriLeft distribution. A uniform variate is drawn and fed to TriLeftQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 3 ⓁTriLeftRand
@ Expecting 1.27806 73495 8
```

## TriLeftArgs

Labels a row of values with the parameter names of TriLeft. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 1 ⓁTriLeftArgs 3 →List
@ Expecting { Xmin:0 Xmax:3 X:1 }
```

## TriLeftAll

Everything TriLeft has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 3 1 ⓁTriLeftAll 5 →List
@ Expecting { CDF:0.55555 55555 56 PDF:0.44444 44444 44 CDFc:0.44444 44444 44 μ:1 sx:0.70710 67811 87 }
```

## TriAsymPDF

Density of the TriAsym distribution at X. Stack: `Xmin`, `Xmax`, `xp`, `X` — or `{ Xmin Xmax xp X }`.

![Asymmetric Triangular Distribution](img/TriAsym_Distribution.bmp)

```rpl
0 3 1 0.8 ⓁTriAsymPDF
@ Expecting 0.53333 33333 33
```

## TriAsymCDF

Distribution function of TriAsym: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `xp`, `X` — or `{ Xmin Xmax xp X }`.

```rpl
0 3 1 0.8 ⓁTriAsymCDF
@ Expecting 0.21333 33333 33
```

## TriAsymCDFc

Upper tail of TriAsym, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `xp`, `X` — or `{ Xmin Xmax xp X }`.

```rpl
0 3 1 0.8 ⓁTriAsymCDFc
@ Expecting 0.78666 66666 67
```

## TriAsymQ

Quantile of TriAsym: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `xp`, `p` — or `{ Xmin Xmax xp p }`.

```rpl
0 3 1 0.213333333333 ⓁTriAsymQ
@ Expecting 0.79999 99999 99
```

## TriAsymMu

Mean of the TriAsym distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax`, `xp` — or `{ Xmin Xmax xp }`.

```rpl
0 3 1 ⓁTriAsymMu
@ Expecting 1.33333 33333 3
```

## TriAsymSx

Standard deviation of the TriAsym distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax`, `xp` — or `{ Xmin Xmax xp }`.

```rpl
0 3 1 ⓁTriAsymSx
@ Expecting 0.62360 95644 62
```

## TriAsymRand

Random draw from the TriAsym distribution. A uniform variate is drawn and fed to TriAsymQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax`, `xp` — or `{ Xmin Xmax xp }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 3 1 ⓁTriAsymRand
@ Expecting 1.59404 78783 5
```

## TriAsymArgs

Labels a row of values with the parameter names of TriAsym. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `xp`, `X` — or `{ Xmin Xmax xp X }`.

```rpl
0 3 1 0.8 ⓁTriAsymArgs 4 →List
@ Expecting { Xmin:0 Xmax:3 xp:1 X:0.8 }
```

## TriAsymAll

Everything TriAsym has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `xp`, `X` — or `{ Xmin Xmax xp X }`.

```rpl
0 3 1 0.8 ⓁTriAsymAll 5 →List
@ Expecting { CDF:0.21333 33333 33 PDF:0.53333 33333 33 CDFc:0.78666 66666 67 μ:1.33333 33333 3 sx:0.62360 95644 62 }
```

## UShapePDF

Density of the UShape distribution at X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![U-Shaped Distribution](img/UShape_Distribution.bmp)

```rpl
0 1 0.25 ⓁUShapePDF
@ Expecting 0.73510 51938 96
```

## UShapeCDF

Distribution function of UShape: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.25 ⓁUShapeCDF
@ Expecting 0.33333 33333 33
```

## UShapeCDFc

Upper tail of UShape, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.25 ⓁUShapeCDFc
@ Expecting 0.66666 66666 67
```

## UShapeQ

Quantile of UShape: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
0 1 0.333333333333 ⓁUShapeQ
@ Expecting 0.25
```

## UShapeMu

Mean of the UShape distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁUShapeMu
@ Expecting 0.5
```

## UShapeSx

Standard deviation of the UShape distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
0 1 ⓁUShapeSx
@ Expecting 0.35355 33905 93
```

## UShapeRand

Random draw from the UShape distribution. A uniform variate is drawn and fed to UShapeQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁUShapeRand
@ Expecting 0.75526 36472 52
```

## UShapeArgs

Labels a row of values with the parameter names of UShape. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.25 ⓁUShapeArgs 3 →List
@ Expecting { Xmin:0 Xmax:1 X:0.25 }
```

## UShapeAll

Everything UShape has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
0 1 0.25 ⓁUShapeAll 5 →List
@ Expecting { CDF:0.33333 33333 33 PDF:0.73510 51938 96 CDFc:0.66666 66666 67 μ:0.5 sx:0.35355 33905 93 }
```

## GEVPDF

Density of the GEV distribution at X. Stack: `x0`, `sc`, `xi`, `X` — or `{ x0 sc xi X }`.

![Generalized Extreme Value Distribution](img/GEV_Distribution.bmp)

```rpl
0 1 0.2 1 ⓁGEVPDF
@ Expecting 0.22406 77286 51
```

## GEVCDF

Distribution function of GEV: the probability of not exceeding X. Stack: `x0`, `sc`, `xi`, `X` — or `{ x0 sc xi X }`.

```rpl
0 1 0.2 1 ⓁGEVCDF
@ Expecting 0.66906 26526 68
```

## GEVCDFc

Upper tail of GEV, the complement of its distribution function. Stack: `x0`, `sc`, `xi`, `X` — or `{ x0 sc xi X }`.

```rpl
0 1 0.2 1 ⓁGEVCDFc
@ Expecting 0.33093 73473 32
```

## GEVQ

Quantile of GEV: the value below which the probability is the one given. Stack: `x0`, `sc`, `xi`, `p` — or `{ x0 sc xi p }`.

```rpl
0 1 0.2 0.669062652668 ⓁGEVQ
@ Expecting 1.
```

## GEVMu

Mean of the GEV distribution, computed from its parameters alone. Stack: `x0`, `sc`, `xi` — or `{ x0 sc xi }`.

```rpl
0 1 0.2 ⓁGEVMu
@ Expecting 0.82114 85686 27
```

## GEVSx

Standard deviation of the GEV distribution, computed from its parameters alone. Stack: `x0`, `sc`, `xi` — or `{ x0 sc xi }`.

```rpl
0 1 0.2 ⓁGEVSx
@ Expecting 1.82867 04356 7
```

## GEVRand

Random draw from the GEV distribution. A uniform variate is drawn and fed to GEVQ, so the draw follows the law by inversion. Stack: `x0`, `sc`, `xi` — or `{ x0 sc xi }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 0.2 ⓁGEVRand
@ Expecting 1.00665 16184 9
```

## GEVArgs

Labels a row of values with the parameter names of GEV. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `x0`, `sc`, `xi`, `X` — or `{ x0 sc xi X }`.

```rpl
0 1 0.2 1 ⓁGEVArgs 4 →List
@ Expecting { x0:0 sc:1 xi:0.2 X:1 }
```

## GEVAll

Everything GEV has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `x0`, `sc`, `xi`, `X` — or `{ x0 sc xi X }`.

```rpl
0 1 0.2 1 ⓁGEVAll 5 →List
@ Expecting { CDF:0.66906 26526 68 PDF:0.22406 77286 51 CDFc:0.33093 73473 32 μ:0.82114 85686 27 sx:1.82867 04356 7 }
```

## BernoulliPDF

Probability mass of the Bernoulli distribution at X: the probability of exactly that value. Stack: `p`, `X` — or `{ p X }`.

![Bernoulli Distribution](img/Bernoulli_Distribution.bmp)

```rpl
0.3 0 ⓁBernoulliPDF
@ Expecting 0.7
```

## BernoulliCDF

Distribution function of Bernoulli: the probability of not exceeding X. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 0 ⓁBernoulliCDF
@ Expecting 0.7
```

## BernoulliCDFc

Upper tail of Bernoulli, the complement of its distribution function. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 0 ⓁBernoulliCDFc
@ Expecting 0.3
```

## BernoulliQ

Quantile of Bernoulli: the value below which the probability is the one given. Stack: `p`, `prob` — or `{ p prob }`.

```rpl
0.3 0.7 ⓁBernoulliQ
@ Expecting 0
```

## BernoulliMu

Mean of the Bernoulli distribution, computed from its parameters alone. Stack: `p` — or `{ p }`.

```rpl
0.3 ⓁBernoulliMu
@ Expecting 0.3
```

## BernoulliSx

Standard deviation of the Bernoulli distribution, computed from its parameters alone. Stack: `p` — or `{ p }`.

```rpl
0.3 ⓁBernoulliSx
@ Expecting 0.45825 75694 96
```

## BernoulliRand

Random draw from the Bernoulli distribution. A uniform variate is drawn and fed to BernoulliQ, so the draw follows the law by inversion. Stack: `p0` — or `{ p0 }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0.3 ⓁBernoulliRand
@ Expecting 0
```

## BernoulliArgs

Labels a row of values with the parameter names of Bernoulli. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 0 ⓁBernoulliArgs 2 →List
@ Expecting { p:0.3 X:0 }
```

## BernoulliAll

Everything Bernoulli has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 0 ⓁBernoulliAll 5 →List
@ Expecting { CDF:0.7 PDF:0.7 CDFc:0.3 μ:0.3 sx:0.45825 75694 96 }
```

## BinomPDF

Probability mass of the Binom distribution at X: the probability of exactly that value. Stack: `nt`, `p`, `X` — or `{ nt p X }`.

![Binomial Distribution](img/Binom_Distribution.bmp)

```rpl
20 0.3 8 ⓁBinomPDF
@ Expecting 0.11439 67397 05
```

## BinomCDF

Distribution function of Binom: the probability of not exceeding X. Stack: `nt`, `p`, `X` — or `{ nt p X }`.

```rpl
20 0.3 8 ⓁBinomCDF
@ Expecting 0.88666 85371 23
```

## BinomCDFc

Upper tail of Binom, the complement of its distribution function. Stack: `nt`, `p`, `X` — or `{ nt p X }`.

```rpl
20 0.3 8 ⓁBinomCDFc
@ Expecting 0.11333 14628 77
```

## BinomQ

Quantile of Binom: the value below which the probability is the one given. Stack: `nt`, `p`, `prob` — or `{ nt p prob }`.

```rpl
20 0.3 0.886668537123 ⓁBinomQ
@ Expecting 8
```

## BinomMu

Mean of the Binom distribution, computed from its parameters alone. Stack: `nt`, `p` — or `{ nt p }`.

```rpl
20 0.3 ⓁBinomMu
@ Expecting 6.
```

## BinomSx

Standard deviation of the Binom distribution, computed from its parameters alone. Stack: `nt`, `p` — or `{ nt p }`.

```rpl
20 0.3 ⓁBinomSx
@ Expecting 2.04939 01531 9
```

## BinomRand

Random draw from the Binom distribution. A uniform variate is drawn and fed to BinomQ, so the draw follows the law by inversion. Stack: `nt`, `p0` — or `{ nt p0 }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 20 0.3 ⓁBinomRand
@ Expecting 7
```

## BinomArgs

Labels a row of values with the parameter names of Binom. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `nt`, `p`, `X` — or `{ nt p X }`.

```rpl
20 0.3 8 ⓁBinomArgs 3 →List
@ Expecting { nt:20 p:0.3 X:8 }
```

## BinomAll

Everything Binom has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `nt`, `p`, `X` — or `{ nt p X }`.

```rpl
20 0.3 8 ⓁBinomAll 5 →List
@ Expecting { CDF:0.88666 85371 23 PDF:0.11439 67397 05 CDFc:0.11333 14628 77 μ:6. sx:2.04939 01531 9 }
```

## PoissPDF

Probability mass of the Poiss distribution at X: the probability of exactly that value. Stack: `λ`, `X` — or `{ λ X }`.

![Poisson Distribution](img/Poiss_Distribution.bmp)

```rpl
10 10 ⓁPoissPDF
@ Expecting 0.12511 00357 21
```

## PoissCDF

Distribution function of Poiss: the probability of not exceeding X. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
10 10 ⓁPoissCDF
@ Expecting 0.58303 97501 93
```

## PoissCDFc

Upper tail of Poiss, the complement of its distribution function. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
10 10 ⓁPoissCDFc
@ Expecting 0.41696 02498 07
```

## PoissQ

Quantile of Poiss: the value below which the probability is the one given. Stack: `λ`, `p` — or `{ λ p }`.

```rpl
10 0.583039750193 ⓁPoissQ
@ Expecting 11
```

## PoissMu

Mean of the Poiss distribution, computed from its parameters alone. Stack: `λ` — or `{ λ }`.

```rpl
10 ⓁPoissMu
@ Expecting 10
```

## PoissSx

Standard deviation of the Poiss distribution, computed from its parameters alone. Stack: `λ` — or `{ λ }`.

```rpl
10 ⓁPoissSx
@ Expecting 3.16227 76601 7
```

## PoissRand

Random draw from the Poiss distribution. A uniform variate is drawn and fed to PoissQ, so the draw follows the law by inversion. Stack: `lb` — or `{ lb }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 10 ⓁPoissRand
@ Expecting 11
```

## PoissArgs

Labels a row of values with the parameter names of Poiss. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
10 10 ⓁPoissArgs 2 →List
@ Expecting { λ:10 X:10 }
```

## PoissAll

Everything Poiss has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `λ`, `X` — or `{ λ X }`.

```rpl
10 10 ⓁPoissAll 5 →List
@ Expecting { CDF:0.58303 97501 93 PDF:0.12511 00357 21 CDFc:0.41696 02498 07 μ:10 sx:3.16227 76601 7 }
```

## GeomPDF

Probability mass of the Geom distribution at X: the probability of exactly that value. Stack: `p`, `X` — or `{ p X }`.

![Geometric Distribution](img/Geom_Distribution.bmp)

```rpl
0.3 3 ⓁGeomPDF
@ Expecting 0.147
```

## GeomCDF

Distribution function of Geom: the probability of not exceeding X. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 3 ⓁGeomCDF
@ Expecting 0.657
```

## GeomCDFc

Upper tail of Geom, the complement of its distribution function. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 3 ⓁGeomCDFc
@ Expecting 0.343
```

## GeomQ

Quantile of Geom: the value below which the probability is the one given. Stack: `p`, `prob` — or `{ p prob }`.

```rpl
0.3 0.657 ⓁGeomQ
@ Expecting 3
```

## GeomMu

Mean of the Geom distribution, computed from its parameters alone. Stack: `p` — or `{ p }`.

```rpl
0.3 ⓁGeomMu
@ Expecting 3.33333 33333 3
```

## GeomSx

Standard deviation of the Geom distribution, computed from its parameters alone. Stack: `p` — or `{ p }`.

```rpl
0.3 ⓁGeomSx
@ Expecting 2.78886 67551 1
```

## GeomRand

Random draw from the Geom distribution. A uniform variate is drawn and fed to GeomQ, so the draw follows the law by inversion. Stack: `p0` — or `{ p0 }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0.3 ⓁGeomRand
@ Expecting 4
```

## GeomArgs

Labels a row of values with the parameter names of Geom. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 3 ⓁGeomArgs 2 →List
@ Expecting { p:0.3 X:3 }
```

## GeomAll

Everything Geom has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `p`, `X` — or `{ p X }`.

```rpl
0.3 3 ⓁGeomAll 5 →List
@ Expecting { CDF:0.657 PDF:0.147 CDFc:0.343 μ:3.33333 33333 3 sx:2.78886 67551 1 }
```

## HyperPDF

Probability mass of the Hyper distribution at X: the probability of exactly that value. Stack: `Np`, `Ks`, `nd`, `X` — or `{ Np Ks nd X }`.

![Hypergeometric Distribution](img/Hyper_Distribution.bmp)

```rpl
50 10 5 1 ⓁHyperPDF
@ Expecting 0.43133 71972 29
```

## HyperCDF

Distribution function of Hyper: the probability of not exceeding X. Stack: `Np`, `Ks`, `nd`, `X` — or `{ Np Ks nd X }`.

```rpl
50 10 5 1 ⓁHyperCDF
@ Expecting 0.74189 99792 33
```

## HyperCDFc

Upper tail of Hyper, the complement of its distribution function. Stack: `Np`, `Ks`, `nd`, `X` — or `{ Np Ks nd X }`.

```rpl
50 10 5 1 ⓁHyperCDFc
@ Expecting 0.25810 00207 67
```

## HyperQ

Quantile of Hyper: the value below which the probability is the one given. Stack: `Np`, `Ks`, `nd`, `p` — or `{ Np Ks nd p }`.

```rpl
50 10 5 0.741899979233 ⓁHyperQ
@ Expecting 1
```

## HyperMu

Mean of the Hyper distribution, computed from its parameters alone. Stack: `Np`, `Ks`, `nd` — or `{ Np Ks nd }`.

```rpl
50 10 5 ⓁHyperMu
@ Expecting 1
```

## HyperSx

Standard deviation of the Hyper distribution, computed from its parameters alone. Stack: `Np`, `Ks`, `nd` — or `{ Np Ks nd }`.

```rpl
50 10 5 ⓁHyperSx
@ Expecting 0.85714 28571 43
```

## HyperRand

Random draw from the Hyper distribution. A uniform variate is drawn and fed to HyperQ, so the draw follows the law by inversion. Stack: `Np`, `Ks`, `nd` — or `{ Np Ks nd }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 50 10 5 ⓁHyperRand
@ Expecting 1
```

## HyperArgs

Labels a row of values with the parameter names of Hyper. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Np`, `Ks`, `nd`, `X` — or `{ Np Ks nd X }`.

```rpl
50 10 5 1 ⓁHyperArgs 4 →List
@ Expecting { Np:50 Ks:10 nd:5 X:1 }
```

## HyperAll

Everything Hyper has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Np`, `Ks`, `nd`, `X` — or `{ Np Ks nd X }`.

```rpl
50 10 5 1 ⓁHyperAll 5 →List
@ Expecting { CDF:0.74189 99792 33 PDF:0.43133 71972 29 CDFc:0.25810 00207 67 μ:1 sx:0.85714 28571 43 }
```

## UnifDisPDF

Probability mass of the UnifDis distribution at X: the probability of exactly that value. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

![Discrete Uniform Distribution](img/UnifDis_Distribution.bmp)

```rpl
1 6 3 ⓁUnifDisPDF
@ Expecting 0.16666 66666 67
```

## UnifDisCDF

Distribution function of UnifDis: the probability of not exceeding X. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
1 6 3 ⓁUnifDisCDF
@ Expecting 0.5
```

## UnifDisCDFc

Upper tail of UnifDis, the complement of its distribution function. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
1 6 3 ⓁUnifDisCDFc
@ Expecting 0.5
```

## UnifDisQ

Quantile of UnifDis: the value below which the probability is the one given. Stack: `Xmin`, `Xmax`, `p` — or `{ Xmin Xmax p }`.

```rpl
1 6 0.5 ⓁUnifDisQ
@ Expecting 3
```

## UnifDisMu

Mean of the UnifDis distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
1 6 ⓁUnifDisMu
@ Expecting 3.5
```

## UnifDisSx

Standard deviation of the UnifDis distribution, computed from its parameters alone. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

```rpl
1 6 ⓁUnifDisSx
@ Expecting 1.70782 51276 6
```

## UnifDisRand

Random draw from the UnifDis distribution. A uniform variate is drawn and fed to UnifDisQ, so the draw follows the law by inversion. Stack: `Xmin`, `Xmax` — or `{ Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 1 6 ⓁUnifDisRand
@ Expecting 5
```

## UnifDisArgs

Labels a row of values with the parameter names of UnifDis. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
1 6 3 ⓁUnifDisArgs 3 →List
@ Expecting { Xmin:1 Xmax:6 X:3 }
```

## UnifDisAll

Everything UnifDis has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `Xmin`, `Xmax`, `X` — or `{ Xmin Xmax X }`.

```rpl
1 6 3 ⓁUnifDisAll 5 →List
@ Expecting { CDF:0.5 PDF:0.16666 66666 67 CDFc:0.5 μ:3.5 sx:1.70782 51276 6 }
```

## GammaPDF

Density of the Gamma distribution at X. Stack: `a`, `sc`, `X` — or `{ a sc X }`.

![Gamma Distribution](img/Gamma_Distribution.bmp)

```rpl
2 3 3 ⓁGammaPDF
@ Expecting 0.12262 64803 9
```

## GammaCDF

Distribution function of Gamma: the probability of not exceeding X. Stack: `a`, `sc`, `X` — or `{ a sc X }`.

```rpl
2 3 3 ⓁGammaCDF
@ Expecting 0.26424 11176 57
```

## GammaCDFc

Upper tail of Gamma, the complement of its distribution function. Stack: `a`, `sc`, `X` — or `{ a sc X }`.

```rpl
2 3 3 ⓁGammaCDFc
@ Expecting 0.73575 88823 43
```

## GammaQ

Quantile of Gamma: the value below which the probability is the one given. Stack: `a`, `sc`, `p` — or `{ a sc p }`.

```rpl
2 3 0.264241117657 ⓁGammaQ
@ Expecting 3.
```

## GammaMu

Mean of the Gamma distribution, computed from its parameters alone. Stack: `a`, `sc` — or `{ a sc }`.

```rpl
2 3 ⓁGammaMu
@ Expecting 6
```

## GammaSx

Standard deviation of the Gamma distribution, computed from its parameters alone. Stack: `a`, `sc` — or `{ a sc }`.

```rpl
2 3 ⓁGammaSx
@ Expecting 4.24264 06871 2
```

## GammaRand

Random draw from the Gamma distribution. A uniform variate is drawn and fed to GammaQ, so the draw follows the law by inversion. Stack: `a`, `sc` — or `{ a sc }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 2 3 ⓁGammaRand
@ Expecting 6.91829 67155 8
```

## GammaArgs

Labels a row of values with the parameter names of Gamma. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `a`, `sc`, `X` — or `{ a sc X }`.

```rpl
2 3 3 ⓁGammaArgs 3 →List
@ Expecting { a:2 sc:3 X:3 }
```

## GammaAll

Everything Gamma has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `a`, `sc`, `X` — or `{ a sc X }`.

```rpl
2 3 3 ⓁGammaAll 5 →List
@ Expecting { CDF:0.26424 11176 57 PDF:0.12262 64803 9 CDFc:0.73575 88823 43 μ:6 sx:4.24264 06871 2 }
```

## NormTruncPDF

Density of the NormTrunc distribution at X. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `X` — or `{ x0 sc Xmin Xmax X }`.

![Truncated Normal Distribution](img/NormTrunc_Distribution.bmp)

```rpl
0 1 -1 2 0.5 ⓁNormTruncPDF
@ Expecting 0.43008 50759 23
```

## NormTruncCDF

Distribution function of NormTrunc: the probability of not exceeding X. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `X` — or `{ x0 sc Xmin Xmax X }`.

```rpl
0 1 -1 2 0.5 ⓁNormTruncCDF
@ Expecting 0.65088 04213 37
```

## NormTruncCDFc

Upper tail of NormTrunc, the complement of its distribution function. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `X` — or `{ x0 sc Xmin Xmax X }`.

```rpl
0 1 -1 2 0.5 ⓁNormTruncCDFc
@ Expecting 0.34911 95786 63
```

## NormTruncQ

Quantile of NormTrunc: the value below which the probability is the one given. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `p` — or `{ x0 sc Xmin Xmax p }`.

```rpl
0 1 -1 2 0.650880421337 ⓁNormTruncQ
@ Expecting 0.50000 00000 01
```

## NormTruncMu

Mean of the NormTrunc distribution, computed from its parameters alone. Stack: `x0`, `sc`, `Xmin`, `Xmax` — or `{ x0 sc Xmin Xmax }`.

```rpl
0 1 -1 2 ⓁNormTruncMu
@ Expecting 0.22963 71790 91
```

## NormTruncSx

Standard deviation of the NormTrunc distribution, computed from its parameters alone. Stack: `x0`, `sc`, `Xmin`, `Xmax` — or `{ x0 sc Xmin Xmax }`.

```rpl
0 1 -1 2 ⓁNormTruncSx
@ Expecting 0.72094 55868 59
```

## NormTruncRand

Random draw from the NormTrunc distribution. A uniform variate is drawn and fed to NormTruncQ, so the draw follows the law by inversion. Stack: `x0`, `sc`, `Xmin`, `Xmax` — or `{ x0 sc Xmin Xmax }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 -1 2 ⓁNormTruncRand
@ Expecting 0.54628 12417 04
```

## NormTruncArgs

Labels a row of values with the parameter names of NormTrunc. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `X` — or `{ x0 sc Xmin Xmax X }`.

```rpl
0 1 -1 2 0.5 ⓁNormTruncArgs 5 →List
@ Expecting { x0:0 sc:1 Xmin:-1 Xmax:2 X:0.5 }
```

## NormTruncAll

Everything NormTrunc has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `x0`, `sc`, `Xmin`, `Xmax`, `X` — or `{ x0 sc Xmin Xmax X }`.

```rpl
0 1 -1 2 0.5 ⓁNormTruncAll 5 →List
@ Expecting { CDF:0.65088 04213 37 PDF:0.43008 50759 23 CDFc:0.34911 95786 63 μ:0.22963 71790 91 sx:0.72094 55868 59 }
```

## NegBinomPDF

Probability mass of the NegBinom distribution at X: the probability of exactly that value. Stack: `rs`, `p`, `X` — or `{ rs p X }`.

![Negative Binomial Distribution](img/NegBinom_Distribution.bmp)

```rpl
3 0.5 2 ⓁNegBinomPDF
@ Expecting 0.1875
```

## NegBinomCDF

Distribution function of NegBinom: the probability of not exceeding X. Stack: `rs`, `p`, `X` — or `{ rs p X }`.

```rpl
3 0.5 2 ⓁNegBinomCDF
@ Expecting 0.5
```

## NegBinomCDFc

Upper tail of NegBinom, the complement of its distribution function. Stack: `rs`, `p`, `X` — or `{ rs p X }`.

```rpl
3 0.5 2 ⓁNegBinomCDFc
@ Expecting 0.5
```

## NegBinomQ

Quantile of NegBinom: the value below which the probability is the one given. Stack: `rs`, `p`, `prob` — or `{ rs p prob }`.

```rpl
3 0.5 0.5 ⓁNegBinomQ
@ Expecting 2
```

## NegBinomMu

Mean of the NegBinom distribution, computed from its parameters alone. Stack: `rs`, `p` — or `{ rs p }`.

```rpl
3 0.5 ⓁNegBinomMu
@ Expecting 3.
```

## NegBinomSx

Standard deviation of the NegBinom distribution, computed from its parameters alone. Stack: `rs`, `p` — or `{ rs p }`.

```rpl
3 0.5 ⓁNegBinomSx
@ Expecting 2.44948 97427 8
```

## NegBinomRand

Random draw from the NegBinom distribution. A uniform variate is drawn and fed to NegBinomQ, so the draw follows the law by inversion. Stack: `rs`, `p0` — or `{ rs p0 }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 3 0.5 ⓁNegBinomRand
@ Expecting 4
```

## NegBinomArgs

Labels a row of values with the parameter names of NegBinom. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `rs`, `p`, `X` — or `{ rs p X }`.

```rpl
3 0.5 2 ⓁNegBinomArgs 3 →List
@ Expecting { rs:3 p:0.5 X:2 }
```

## NegBinomAll

Everything NegBinom has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `rs`, `p`, `X` — or `{ rs p X }`.

```rpl
3 0.5 2 ⓁNegBinomAll 5 →List
@ Expecting { CDF:0.5 PDF:0.1875 CDFc:0.5 μ:3. sx:2.44948 97427 8 }
```

## StudentPDF

Density of the Student distribution at X. Stack: `df`, `X` — or `{ df X }`.

![Student t Distribution](img/Student_Distribution.bmp)

```rpl
4 1 ⓁStudentPDF
@ Expecting 0.21466 25258 4
```

## StudentCDF

Distribution function of Student: the probability of not exceeding X. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 1 ⓁStudentCDF
@ Expecting 0.81304 95168 5
```

## StudentCDFc

Upper tail of Student, the complement of its distribution function. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 1 ⓁStudentCDFc
@ Expecting 0.18695 04831 5
```

## StudentQ

Quantile of Student: the value below which the probability is the one given. Stack: `df`, `p` — or `{ df p }`.

```rpl
4 0.81304951685 ⓁStudentQ
@ Expecting 1.
```

## StudentMu

Mean of the Student distribution, computed from its parameters alone. Stack: `df` — or `{ df }`.

```rpl
4 ⓁStudentMu
@ Expecting 0
```

## StudentSx

Standard deviation of the Student distribution, computed from its parameters alone. Stack: `df` — or `{ df }`.

```rpl
4 ⓁStudentSx
@ Expecting 1.41421 35623 7
```

## StudentRand

Random draw from the Student distribution. A uniform variate is drawn and fed to StudentQ, so the draw follows the law by inversion. Stack: `df` — or `{ df }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 4 ⓁStudentRand
@ Expecting 0.47600 95145 71
```

## StudentArgs

Labels a row of values with the parameter names of Student. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 1 ⓁStudentArgs 2 →List
@ Expecting { df:4 X:1 }
```

## StudentAll

Everything Student has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `df`, `X` — or `{ df X }`.

```rpl
4 1 ⓁStudentAll 5 →List
@ Expecting { CDF:0.81304 95168 5 PDF:0.21466 25258 4 CDFc:0.18695 04831 5 μ:0 sx:1.41421 35623 7 }
```

## FisherPDF

Density of the Fisher distribution at X. Stack: `dfn`, `dfd`, `X` — or `{ dfn dfd X }`.

![Fisher F Distribution](img/Fisher_Distribution.bmp)

```rpl
4 6 1 ⓁFisherPDF
@ Expecting 0.41472
```

## FisherCDF

Distribution function of Fisher: the probability of not exceeding X. Stack: `dfn`, `dfd`, `X` — or `{ dfn dfd X }`.

```rpl
4 6 1 ⓁFisherCDF
@ Expecting 0.5248
```

## FisherCDFc

Upper tail of Fisher, the complement of its distribution function. Stack: `dfn`, `dfd`, `X` — or `{ dfn dfd X }`.

```rpl
4 6 1 ⓁFisherCDFc
@ Expecting 0.4752
```

## FisherQ

Quantile of Fisher: the value below which the probability is the one given. Stack: `dfn`, `dfd`, `p` — or `{ dfn dfd p }`.

```rpl
4 6 0.5248 ⓁFisherQ
@ Expecting 1.
```

## FisherMu

Mean of the Fisher distribution, computed from its parameters alone. Stack: `dfn`, `dfd` — or `{ dfn dfd }`.

```rpl
4 6 ⓁFisherMu
@ Expecting 1.5
```

## FisherSx

Standard deviation of the Fisher distribution, computed from its parameters alone. Stack: `dfn`, `dfd` — or `{ dfn dfd }`.

```rpl
4 6 ⓁFisherSx
@ Expecting 2.12132 03435 6
```

## FisherRand

Random draw from the Fisher distribution. A uniform variate is drawn and fed to FisherQ, so the draw follows the law by inversion. Stack: `dfn`, `dfd` — or `{ dfn dfd }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 4 6 ⓁFisherRand
@ Expecting 1.43440 36535 5
```

## FisherArgs

Labels a row of values with the parameter names of Fisher. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `dfn`, `dfd`, `X` — or `{ dfn dfd X }`.

```rpl
4 6 1 ⓁFisherArgs 3 →List
@ Expecting { dfn:4 dfd:6 X:1 }
```

## FisherAll

Everything Fisher has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. The example collects them into a list to be checked here; from the keyboard they arrive one per stack level. Stack: `dfn`, `dfd`, `X` — or `{ dfn dfd X }`.

```rpl
4 6 1 ⓁFisherAll 5 →List
@ Expecting { CDF:0.5248 PDF:0.41472 CDFc:0.4752 μ:1.5 sx:2.12132 03435 6 }
```

## LevyPDF

Density of the Levy distribution at X. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

![Levy Distribution](img/Levy_Distribution.bmp)

```rpl
0 1 1 ⓁLevyPDF
@ Expecting 0.24197 07245 19
```

## LevyCDF

Distribution function of Levy: the probability of not exceeding X. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁLevyCDF
@ Expecting 0.31731 05078 63
```

## LevyCDFc

Upper tail of Levy, the complement of its distribution function. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁLevyCDFc
@ Expecting 0.68268 94921 37
```

## LevyQ

Quantile of Levy: the value below which the probability is the one given. Stack: `x0`, `sc`, `p` — or `{ x0 sc p }`.

The tail is heavy, so the quantile climbs steeply: the median is near 2.2, but nine tenths of the mass lies below 63.

```rpl
0 1 0.5 ⓁLevyQ
@ Expecting 2.19810 93383 2
```

## LevyMu

Mean of the Levy distribution. It is infinite: the law has no mean, and the entry says so with the infinity constant rather than returning a number. Stack: `x0`, `sc` — or `{ x0 sc }`.

```rpl
0 1 ⓁLevyMu
@ Expecting 9.99999⁳⁹⁹⁹⁹⁹⁹
```

## LevySx

Standard deviation of the Levy distribution. Infinite for the same reason as the mean. Stack: `x0`, `sc` — or `{ x0 sc }`.

```rpl
0 1 ⓁLevySx
@ Expecting 9.99999⁳⁹⁹⁹⁹⁹⁹
```

## LevyRand

Random draw from the Levy distribution. A uniform variate is drawn and fed to LevyQ, so the draw follows the law by inversion. Stack: `x0`, `sc` — or `{ x0 sc }`.

Seed the generator with RDZ to make a sequence reproducible; without a seed each run differs.

```rpl
42 rdz 0 1 ⓁLevyRand
@ Expecting 5.52609 18603 8
```

## LevyArgs

Labels a row of values with the parameter names of Levy. It takes the same arguments the other entries take, and hands them straight back with their names attached — it recalls nothing by itself, so the way to see what a call consumed is `LastArguments` straight before it - the list it reports is accepted here as readily as bare values - or Undo first and then this. The example collects the result into a list to be checked here; from the keyboard the values arrive one per stack level. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁLevyArgs 3 →List
@ Expecting { x0:0 sc:1 X:1 }
```

## LevyAll

Everything Levy has to say about one point, tagged: distribution function, density, upper tail, mean and standard deviation. Stack: `x0`, `sc`, `X` — or `{ x0 sc X }`.

```rpl
0 1 1 ⓁLevyAll 5 →List
@ Expecting { CDF:0.31731 05078 63 PDF:0.24197 07245 19 CDFc:0.68268 94921 37 μ:9.99999⁳⁹⁹⁹⁹⁹⁹ sx:9.99999⁳⁹⁹⁹⁹⁹⁹ }
```
