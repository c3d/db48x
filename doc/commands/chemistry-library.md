# Chemistry

The chemistry section of the library. Its first part, [Elements](#elementslibrary),
holds the periodic table and the data of the 118 elements.

## ChemistryLibrary

The Chemistry section of the library: the [Elements](#elementslibrary), with
the interactive periodic table [PerTbl](#pertbl) and the data of each element.

## ElementsLibrary

The 118 chemical elements:

* [PerTbl](#pertbl): the interactive periodic table;
* [Element](#element): the data of an element, as a tagged list;
* [ElmData](#elmdata): the same data as a plain list, for programs;
* [AtWt](#atwt): the standard atomic weight, as an interval;
* [AtWtσ](#atwtσ): the standard atomic weight, as a value with a standard
  uncertainty;
* [MolarMass](#molarmass) and [MolarMassσ](#molarmassσ): the molar mass of a
  chemical formula, as an interval or with its standard uncertainty;
* [Formula](#formula): the composition of a chemical formula.

An element is given by its atomic number `Z`, its symbol (`"Fe"`) or its name
(`"Iron"`).

**Standard atomic weights.** They come from the Commission on Isotopic
Abundances and Atomic Weights (CIAAW) of the IUPAC, *Standard Atomic Weights
2024*: the values of the *Atomic Weights 2021* report, with gadolinium,
lutetium and zirconium revised in 2024. For fourteen elements whose isotopic
composition varies in nature, from hydrogen to lead, the CIAAW gives an
interval `[a, b]`; for the others, a value with its uncertainty, `v(U)`. The
CIAAW states that both mean the same thing: the atomic weight of any normal
material lies, with great certitude, from `a` to `b`, or from `v − U` to
`v + U`. DB48X therefore keeps every standard atomic weight as an interval
`a…b`, which computes directly with interval arithmetic. When a value with a
standard uncertainty is needed, [AtWtσ](#atwtσ) takes the interval as a
uniform distribution, as recommended by the IUPAC Technical Report of
Possolo *et al.* (2018). The symbol `Ar°` is the one of the CIAAW for the
standard atomic weight. The abridged value, `Abridged`, is the five-digit value
of the CIAAW, as printed in the periodic table of the IUPAC.

Standard atomic weights have no unit: they are relative atomic masses, ratios
to 1/12 of the mass of a carbon 12 atom. The molar mass of an element is
`Ar°·Mu`, where `Mu` is the molar mass constant, `ⒸMu`. Since the SI of 2019,
`Mu` differs from `1 g/mol` by `3.5×10⁻¹⁰` only, far below the uncertainty of
any standard atomic weight: the data page of [PerTbl](#pertbl) shows the molar
mass as `Ar°` in `g/mol`.

**Isotopic compositions.** The most abundant isotope of each element and its
amount fraction (fraction of the number of atoms, not of the mass) come from
the *Isotopic Compositions of the Elements 2024* of the CIAAW, with the same
two notations, `[a, b]` or `v(U)`, kept as `a…b` or `v±U`. The 26 elements
from neptunium on, and those from polonium to actinium, have no standard
isotopic composition.

Thirty-four elements, all radioactive, have no standard atomic weight. For
them, `A` is the mass number of the most stable isotope.

**First ionization energies.** They come from the *Atomic Spectra Database*
(ASD) of the NIST, with their uncertainty, for the 108 elements from hydrogen
to hassium; there is none beyond. The NIST does not state the kind of this
uncertainty; it is used here as a standard uncertainty. The value for radon
has no uncertainty. Most values are evaluated from experiments; the NIST marks
those of hydrogen, rutherfordium and dubnium as theoretical (ab initio), and
those of praseodymium, protactinium, fermium, mendelevium, seaborgium, bohrium
and hassium as semi-empirical (interpolated or extrapolated).

**Other properties.** They come from the *Periodic Table of Elements* of
PubChem (National Library of Medicine, NIH): family, standard state, melting
and boiling points, density, electronegativity (Pauling) and electron
configuration. PubChem gives them without uncertainty: melting and boiling
points and densities depend on the allotrope and on the conditions, and the
electronegativity of Pauling is a semi-empirical scale.

**The data** are kept in twelve small files, `library/Chemistry/ElmD01.48s` to
`ElmD12.48s`, ten elements each, so that reading an element only reads one of
them.

## PerTbl

The interactive periodic table of the 118 elements, with lutetium and
lawrencium in group 3, as recommended by an IUPAC project in 2021, and the
lanthanides and actinides on two rows apart.

The box in the empty part of periods 1 to 3 shows the element under the
cursor: its atomic number, symbol and name, its abridged standard atomic
weight (or `[A]`, the mass number of the most stable isotope, when there is
none), and its family.

* `8`, `4`, `6`, `2`, or `▲` and `▼`: move the cursor, skipping the empty cells;
* `5`: show all the data of the element, as Element does, with its
  molar mass; any key but EXIT comes back to the table;
* `ENTER`: leave the table, with the element under the cursor on the stack;
  the table stays on the screen until the next key.

EXIT, or the Back gesture of a phone, cannot be caught by a program: as
during any Wait, it suspends PerTbl. The table, or the card, stays on the
screen, and a red mark in the status bar shows the suspended program; keys
typed then go to the stack, and the next instruction of the program may show
at the top of the screen. To leave, press Continue, in the Debug menu: PerTbl
ends, from the table as from a card, whose drawing stays on the screen until
the next key. Kill, in the same menu, also ends
it, without the element on the stack.

A list of key positions in level 1 is played first, then the keyboard takes
over. Only a list of numbers is taken: the list that Element, or an earlier
call to PerTbl, left in level 1 stays on the stack. This makes it possible to start on a given element, or to replay a
demonstration. Key positions are given as `rc`: row and column on the
keyboard, for example `61` for `▼` and `64` for `6`.

```rpl
@ Down three times, then right seven times: iron; ENTER leaves the table
{ 61 61 61 64 64 64 64 64 64 64 41 } ⓁPerTbl
```

See also: [Element](#element).

## Element

The data of a chemical element, as a tagged list: atomic number `Z`, symbol,
name, standard atomic weight `Ar°` (an interval), abridged value, group,
period, family, state, melting and boiling points, density,
electronegativity, first ionization energy (with its uncertainty), electron
configuration, and the most abundant isotope with its amount fraction. Missing values
are left out. `A`, the mass number of the most stable isotope, only appears
when there is no standard atomic weight.

`Z` or `"Symbol"` or `"Name"` ▶ `{ Z: Symbol: Name: Ar°: Abridged: … }`

```rpl
"Fe" ⓁElement
@ Expecting { Z:26 Symbol:"Fe" Name:"Iron" Ar°:55.843…55.847 Abridged:55.845±0.002 Group:8 Period:4 Family:"Transition metal" State:"Solid" Tmelt:1 811 K Tboil:3 134 K Density:7.874 g/cm↑3 Electronegativity:1.83 Ionization:7.90246 81±σ0.00000 12 eV Configuration:"[Ar]4s2 3d6" Isotope:"⁵⁶Fe" Fraction:0.91754±0.00106 }
```

## ElmData

The data of a chemical element as a plain list, in a fixed order, for
programs. A missing value is the text `"—"`.

`Z` or `"Symbol"` or `"Name"` ▶
`{ Z "Symbol" "Name" Ar° Abridged A Group Period "Family" "State" Tmelt Tboil Density Electronegativity Ionization "Configuration" "Isotope" Fraction }`

```rpl
@ Technetium has no standard atomic weight: A is 97
43 ⓁElmData
@ Expecting { 43 "Tc" "Technetium" "—" "—" 97 7 5 "Transition metal" "Solid" 2 430 K 4 538 K 11 g/cm↑3 1.9 7.11938±σ0.00003 eV "[Kr]5s2 4d5" "—" "—" "⁹⁷Tc" 96.90636 }
```

## AtWt

The standard atomic weight of an element, as the interval of the CIAAW.

`Z` or `"Symbol"` or `"Name"` ▶ `a…b`

```rpl
"Hydrogen" ⓁAtWt
@ Expecting 1.00784…1.00811
```

As an interval, it computes the relative molecular mass of a compound
directly. For water, H₂O:

```rpl
'2*ⓁAtWt(1)+ⓁAtWt(8)' →Num
@ Expecting 18.01471…18.01599
```

## AtWtσ

The standard atomic weight of an element, as a value and a standard
uncertainty: the interval `a…b` of the CIAAW taken as a uniform distribution,
as recommended by the IUPAC, with the value `(a+b)/2` and the standard
uncertainty `(b−a)/(2√3)`.

`Z` or `"Symbol"` or `"Name"` ▶ `m±σu`

```rpl
"C" ⓁAtWtσ
@ Expecting 12.0106±σ5.77350 26919⁳⁻⁴
```

## MolarMass

The molar mass of a chemical formula, as an interval: the sum of the standard
atomic weights of its atoms, times the molar mass constant `Mu`, that is
`1 g/mol` to `3.5×10⁻¹⁰`. The bounds of the intervals of the CIAAW add up, so
that the molar mass of any normal material lies between the two sums.

The formula is a text: symbols with their counts, parentheses (which may be
nested) with a count after them, and hydrates after a middle dot, a dot or a
star, with their
coefficient, as in `"CuSO4·5H2O"`.

`"Formula"` ▶ `a…b` in g/mol

```rpl
"H2O" ⓁMolarMass
@ Expecting 18.01471…18.01599 g/mol
```

```rpl
@ Copper sulfate pentahydrate
"CuSO4·5H2O" ⓁMolarMass
@ Expecting 249.67167…249.70403 g/mol
```

## MolarMassσ

The molar mass of a chemical formula, as a value with its standard
uncertainty, from AtWtσ. The atoms of one element all have the same
atomic weight: their uncertainties add linearly, `n·u`. The atomic weights of
different elements are independent: their contributions add in quadrature.
Hence `u(M)² = Σ (nᵢ·uᵢ)²`.

`"Formula"` ▶ `M±σu` in g/mol

```rpl
@ Glucose
"C6H12O6" ⓁMolarMassσ
@ Expecting 180.1557±σ3.81019 68453 1⁳⁻³ g/mol
```

See also: [AtWtσ](#atwtσ), [MolarMass](#molarmass).

## Formula

The composition of a chemical formula: each element once, with its number of
atoms, in the order of its first appearance. The atoms of an element found in
several places, as the oxygen of a hydrate, are counted together.

`"Formula"` ▶ `{ { "Symbol" count } … }`

```rpl
"Mg(NO3)2·6H2O" ⓁFormula
@ Expecting { { "Mg" 1 } { "N" 2 } { "O" 12 } { "H" 12 } }
```

**References**

* CIAAW, *Standard Atomic Weights 2024*,
  [ciaaw.org/atomic-weights.htm](https://www.ciaaw.org/atomic-weights.htm),
  and *Abridged Standard Atomic Weights*.
* T. Prohaska *et al.*, *Standard atomic weights of the elements 2021 (IUPAC
  Technical Report)*, Pure Appl. Chem. 94 (2022) 573–600.
* A. Possolo, A. M. H. van der Veen, J. Meija, D. B. Hibbert, *Interpreting and
  propagating the uncertainty of the standard atomic weights (IUPAC Technical
  Report)*, Pure Appl. Chem. 90 (2018) 395–424.
* CIAAW, *Isotopic Compositions of the Elements 2024*,
  [ciaaw.org/isotopic-abundances.htm](https://www.ciaaw.org/isotopic-abundances.htm),
  and J. Meija *et al.*, *Isotopic compositions of the elements 2013 (IUPAC
  Technical Report)*, Pure Appl. Chem. 88 (2016) 293–306.
* A. Kramida, Yu. Ralchenko, J. Reader and NIST ASD Team, *NIST Atomic
  Spectra Database*, ionization energies,
  [physics.nist.gov/PhysRefData/ASD/ionEnergy.html](https://physics.nist.gov/PhysRefData/ASD/ionEnergy.html).
* National Library of Medicine (NIH), PubChem, *Periodic Table of Elements*,
  [pubchem.ncbi.nlm.nih.gov/periodic-table](https://pubchem.ncbi.nlm.nih.gov/periodic-table/).
