# Astronomy & Astronautics Library

RPL library functions for Solar-System ephemerides, coordinate transforms and
interplanetary trajectory design. Every function is a library item (prefix `Ⓛ`),
loaded on demand like `PeriSel`. Angles are handled in **degrees**; heliocentric
quantities use **astronomical units and days** (`1 au/day = 1731.4568 km/s`,
`μ☉ = 2.9591220828559e-4 au³/day²`). Central-body functions are universal: the
gravitational-parameter argument (`ⒸGM♁`, `ⒸGM☉`, `ⒸGM♂`, `ⒸGM☾`…) selects the body.

Two input conventions coexist and must not be mixed:

* **Element functions** `♁Φf`/`♂Φf`… return an orbital-element vector
  `[a e I L ϖ Ω]`. Used by the *distance* functions (`DABf`, `T₀SDAToB`).
* **Position functions** `♁Pf`/`♂Pf`/`☾Hf` return a heliocentric position
  `[x y z]`. Used by the *transfer* functions (`VΦf`, `TrCost`, `Min…`, `aTr`).

*Each `##` title is the exact function name, for long-press lookup. Where a
family shares one entry (the eight `…Φf`, the six transforms), the siblings are
listed in that entry's body.*

---

## ♁Φf

Mean orbital elements of a planet from a Julian Day (Standish/JPL), returned as
`[a e I L ϖ Ω]` (au and degrees). The full family shares this format:
`☿Φf` `♀Φf` `♁Φf` `♂Φf` `♃Φf` `♄Φf` `⛢Φf` `♆Φf`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2451545 ♁Φf        @ Earth mean elements at J2000 → [ a e I L ϖ Ω ]
```

## ☾Φf

Mean orbital elements of the Moon (geocentric), same output shape as a planet.

`JD` → `[a e I L ϖ Ω]`

```rpl
2451545 ☾Φf        @ Moon mean elements at J2000
```

## ☾Pos

Geocentric ecliptic position of the Moon from the Meeus lunar theory (longitude
`λ`, latitude `β`, distance `Δ`), accurate to ≈ 1′. Sums the periodic terms via
the helpers `MSumS`/`MSumC`.

`JD` → `[ λ β Δ ]`   (deg, deg, km)

```rpl
2451545 ☾Pos       @ Moon geocentric ecliptic longitude / latitude / distance
```

## ☾Hf

Heliocentric ecliptic position of the Moon `[x y z]` in au — the Moon expressed
as a **position function**, so it plugs into the transfer machinery exactly like
a planet.

`JD` → `[x y z]`   (au)

```rpl
2459050.5 ☾Hf      @ Moon heliocentric position (au)
```

---

## PosΦf

Heliocentric ecliptic position `[x y z]` (au) from an orbital-element vector.
Solves Kepler's equation and chains the six coordinate transforms. Guards the
angle mode to `DEG`. The transforms are also callable individually: `EaΦf`
(eccentric anomaly, Kepler root), `νΦf` (true anomaly), `rΦf` (radius), `uΦf`
(argument of latitude), `λΦf` (ecliptic longitude), `βΦf` (ecliptic latitude).

`[a e I L ϖ Ω]` → `[x y z]`

```rpl
2459050.5 ♁Φf PosΦf     @ Earth heliocentric position on that date (au)
```

## ♁Pf

Position-function wrapper: element function composed with `PosΦf`, mapping a date
directly to a heliocentric `[x y z]`. Define one per body as needed (`♂Pf`, …).

`JD` → `[x y z]`

```rpl
« → JD « JD ♁Φf PosΦf » » '♁Pf' Sto     @ Earth position function
« → JD « JD ♂Φf PosΦf » » '♂Pf' Sto     @ Mars  position function
```

## VΦf

Heliocentric velocity of a body by centred finite difference of a **position
function** (`h = 0.5 day`).

`JD  'Pf'` → `[vx vy vz]`   (au/day)

```rpl
2459050.5 '♁Pf' VΦf
@ Expecting [ 0.01498469 0.00787138 -0.00000036 ]     (= 29.307 km/s)
```

---

## DABf

Distance between two bodies at one date, from their **element functions**.

`JD  'Af'  'Bf'` → `distance`   (au)

```rpl
2459050.5 'Ⓛ♁Φf' 'Ⓛ♂Φf' DABf
@ Expecting 0.707458 au                               (Earth–Mars, 2020-07-20)
```

## T₀SDAToB

Julian Day of closest approach between two bodies inside a search window
(minimum-distance date). `T₀SDEarthToA` is the Earth-referenced variant.

`'Af'  'Bf'  JD_lo  JD_hi` → `JD_min`

```rpl
'Ⓛ♁Φf' 'Ⓛ♂Φf' 2459120 2459140 T₀SDAToB
@ Expecting 2459129.07                                (2020-10-06)
```

## θs

Sidereal-time helper (Greenwich / local apparent sidereal time).

---

## Hohmann

Two-impulse Hohmann transfer between two circular orbits of radii `r₁`, `r₂`
about a body of parameter `μ`.

`r₁  r₂  μ` → `{ ΔV₁  ΔV₂  ΔV_tot  tof }`

```rpl
6678_km 42164_km ⒸGM♁ Hohmann
@ Expecting { 2425.77_m/s 1466.84_m/s 3892.61_m/s 18990.1_s }     (LEO→GEO)
```

## BiElliptic

Three-impulse bi-elliptic transfer via an intermediate apoapsis `r_b`.

`r₁  r₂  r_b  μ` → `{ ΔV₁  ΔV₂  ΔV₃  ΔV_tot  tof }`

```rpl
6678_km 42164_km 200000_km ⒸGM♁ BiElliptic
@ Expecting { 3022.18_m/s 474.20_m/s 876.94_m/s 4373.32_m/s 374954_s }
```

## TrToOrbi

Orbital insertion / capture from a hyperbolic approach: burn from the arrival
hyperbola into a capture orbit (set `r_a = r_p` for circular capture).

`v∞  r_p  r_a  μ` → `{ ΔV_ins  v_hyp  v_cap }`

## LambertU

Lambert's problem by the **universal-variable** formulation (Stumpff functions),
robust for elliptic, parabolic and hyperbolic transfers including near-180°
geometry. Given two position vectors and a time of flight, returns the terminal
velocity vectors.

`[r₁]  [r₂]  tof  μ` → `{ [v₁]  [v₂] }`

```rpl
[ 5000 10000 2100 ] [ -14600 2500 7000 ] 3600 398600 LambertU
@ Expecting { [ -5.99249 1.92536 3.24564 ] [ -3.31246 -4.19662 -0.38529 ] }
@ (Curtis, Example 5.2)
```

Helpers: `StumpC` (`z` → C(z)), `StumpS` (`z` → S(z)), `LamUF` (universal Kepler
residual used by the internal root).

## rv2coe

State vector to classical orbital elements.

`[r]  [v]  μ` → `{ a  e  i  Ω  ω  ν }`

---

## TrCost

ΔV cost of a ballistic transfer between two dates, using **position functions**
for the departure and arrival bodies. Solves Lambert between the two positions
and differences against each body's velocity.

`t₁  t₂  'Af'  'Bf'` → `{ ΔV_tot  ΔV_dep  ΔV_arr }`   (km/s)

```rpl
2459050.5 2459250.5 '♁Pf' '♂Pf' TrCost
@ Expecting { 6.3738 3.6446 2.7292 }                  (Earth→Mars, 200 days)
```

The Moon plugs in identically:

```rpl
2459050.5 2459250.5 '☾Hf' '♂Pf' TrCost
@ Expecting { 7.40385 4.67123 2.73262 }               (Moon→Mars)
```

## MinTofDV

For a fixed departure date, the flight time that minimises `TrCost` ΔV
(ternary search).

`t₁  'Af'  'Bf'  tof_lo  tof_hi` → `{ ΔV  tof }`

```rpl
3 'AstronTXPrecision' STO
2459054 '♁Pf' '♂Pf' 150 225 MinTofDV
@ Expecting { 6.32086 205.20 }
```

## MinΔVTraj

Optimal launch window: minimise ΔV over **both** departure date and flight time
(2-D ternary search over `TrCost`). Keep the window under the 180° ridge.

`'Af'  'Bf'  t₁_lo  t₁_hi  tof_lo  tof_hi` → `{ ΔV  t₁  tof }`

```rpl
'♁Pf' '♂Pf' 2459030 2459075 150 225 MinΔVTraj
@ Expecting { 6.31693 2459055.70 205.38 }             (Mars 2020 window; level 3)
```

Runtime is set by the variable `AstronTXPrecision` (below) — level 3 ≈ 63 s on
the simulator. The minimum ΔV is nearly identical at all levels; higher levels
only sharpen the dates.

## MinΔDTraj

Minimum-energy transfer for a departure date: the flight time giving the
smallest semi-major axis `a` (via `aTr`). A 1-D optimum by geometry.

`t₁  'Af'  'Bf'  tof_lo  tof_hi` → `{ a  tof }`

```rpl
2459054 '♁Pf' '♂Pf' 180 270 MinΔDTraj
@ Expecting { 1.31743 228.22 }
```

## aTr

Semi-major axis of the transfer between two dates (vis-viva on the `LambertU`
solution). Used by `MinΔDTraj`.

`t₁  t₂  'Af'  'Bf'` → `a`   (au)

```rpl
2459054 2459282 '♁Pf' '♂Pf' aTr
@ Expecting 1.31743
```

## MinΔtTraj

Fastest transfer under a ΔV budget: smallest flight time whose `TrCost` stays
within `budget` (bisection).

`t₁  'Af'  'Bf'  budget  tof_lo  tof_hi` → `{ tof  ΔV }`

```rpl
2459054 '♁Pf' '♂Pf' 7 150 205 MinΔtTraj
@ Expecting { 173.38 7.000 }                          (7 km/s budget)
```

## AstronTXPrecision

Global variable controlling the iteration count of the window optimizers
(`MinTofDV`, `MinΔVTraj`, `MinΔtTraj`, `MinΔDTraj`): `1`/`2`/`3` → `6`/`9`/`12`
iterations. Simulator runtime for `MinΔVTraj` ≈ 16 / 35 / 63 s (hardware ≈ ×8).
Use level 1 to explore, level 3 to refine.

---

## GAssist

Gravity-assist (swing-by) in the patched-conic model. In the planet frame the
hyperbolic excess speed `v∞` keeps its magnitude but turns by `δ`, with
`sin(δ/2) = 1/e` and `e = 1 + r_p·v∞²/μ`. The free heliocentric velocity change
is `Δv = 2·v∞/e`, maximal near `v∞ ≈ √(μ/r_p)`.

`v∞  r_p  μ` → `{ δ  Δv  e }`   (deg, m/s, –)

```rpl
5_km/s 3500_km ⒸGM♂ GAssist
@ Expecting { 38.370_° 3286.19_m/s 3.043 }            (Mars flyby, 3500 km periapsis)
```

---

## T♁StdAtm

Earth atmosphere profiles versus **geometric altitude** `Z` (a length: `2000_m`,
`60_km`…). U.S. Standard Atmosphere 1976 — analytic and exact for `−5 km ≤ Z ≤
86 km` — extended to `1000 km` by R. Braeunig's engineering fit (≈4 significant
figures above 86 km). The Earth family:

* `T♁StdAtm` → temperature (`K`)
* `P♁StdAtm` → pressure (`MPa`)
* `ρ♁StdAtm` → density (`kg/m³`), from `P/(R·T)` in the homosphere
* `μ♁StdAtm` → dynamic viscosity (`Pa·s`), Sutherland's law, `−5 km ≤ Z ≤ 86 km`

Out-of-range `Z` raises an error.

`Z` → value with unit

```rpl
2000_m T♁StdAtm     @ Expecting 275.154088844 K
60_km  P♁StdAtm     @ Expecting 2.195879E-5 MPa
2000_m ρ♁StdAtm     @ Expecting 1.006553696670 kg/m^3
60_km  μ♁StdAtm     @ Expecting 1.5837189E-5 Pa·s
```

## T♂DayAtm

Mars atmosphere profiles versus geometric altitude `Z`, valid `−8 km ≤ Z ≤
300 km` (R. Braeunig, Tables 11–15). Two independent diurnal models below
120 km — dayside from Viking 1&2, nightside from Mars Pathfinder — sharing the
single Table-15 upper model above 120 km (a small step appears there on the
night curves). The Mars family:

* Day:   `T♂DayAtm` (`K`) · `P♂DayAtm` (`Pa`) · `ρ♂DayAtm` (`kg/m³`)
* Night: `T♂NightAtm` (`K`) · `P♂NightAtm` (`Pa`) · `ρ♂NightAtm` (`kg/m³`)

Density below 120 km is `P/(R·TM)` with `R = 191.181 J/(kg·K)`.

`Z` → value with unit

```rpl
0_m    T♂DayAtm      @ Expecting 228.50 K
50_km  P♂DayAtm      @ Expecting 3.28196 Pa
0_m    T♂NightAtm    @ Expecting 198.00 K
50_km  ρ♂NightAtm    @ Expecting 9.1990E-5 kg/m^3
```

## T♀Atm

Venus atmosphere profiles versus geometric altitude `Z`, valid `−3 km ≤ Z ≤
300 km` (R. Braeunig, Tables 16–20). Venus has negligible day/night contrast in
the lower atmosphere, so a single global trio is provided (no diurnal split):

* `T♀Atm` → temperature (`K`)
* `P♀Atm` → pressure (`Pa`)
* `ρ♀Atm` → density (`kg/m³`), `P/(R·TM)` with `R = 191.357 J/(kg·K)` below 100 km

A small documented step appears at the 100 km boundary (`TM` below → kinetic `T`
above). Pioneer-Venus data below 100 km; Bougher et al. 1986 above.

`Z` → value with unit

```rpl
0_m    T♀Atm         @ Expecting 735.000 K
50_km  P♀Atm         @ Expecting 106176 Pa
87_km  ρ♀Atm         @ Expecting 0.00239308 kg/m^3
```


<!-- ========== Per-function reference fiches (added 2026-08-08) ========== -->

## P♂DayAtm

Mars **dayside** (Viking) pressure vs geometric altitude `Z`, Braeunig Tables 13/15 (−8…300 km).

`Z` → pressure (`Pa`)

```rpl
0_m P♂DayAtm     @ Expecting 610.5 Pa
```

## ρ♂DayAtm

Mars **dayside** density vs `Z`, `ρ=P/(R·T)` ≤120 km + Braeunig Table 15 (−8…300 km).

`Z` → density (`kg/m³`)

```rpl
0_m ρ♂DayAtm     @ Expecting 0.0139758 kg/m^3
```

## T♂NightAtm

Mars **nightside** (Pathfinder) temperature vs `Z`, Braeunig Tables 12/13.

`Z` → temperature (`K`)

```rpl
0_m T♂NightAtm     @ Expecting 198.00 K
```

## P♂NightAtm

Mars **nightside** (Pathfinder) pressure vs `Z`, Braeunig Table 13.

`Z` → pressure (`Pa`)

```rpl
0_m P♂NightAtm     @ Expecting 610.5 Pa
```

## ρ♂NightAtm

Mars **nightside** density vs `Z`, `ρ=P/(R·T)` ≤120 km.

`Z` → density (`kg/m³`)

```rpl
0_m ρ♂NightAtm     @ Expecting 0.0161279 kg/m^3
```

## P♀Atm

Venus pressure (single global model) vs `Z`, Braeunig Tables 18/20 (−3…300 km).

`Z` → pressure (`Pa`)

```rpl
0_m P♀Atm     @ Expecting 9.332E6 Pa
```

## ρ♀Atm

Venus density vs `Z`, `ρ=P/(R·T)` ≤100 km + Braeunig Table 20 (−3…300 km).

`Z` → density (`kg/m³`)

```rpl
0_m ρ♀Atm     @ Expecting 66.3503 kg/m^3
```

## P♁StdAtm

Earth pressure vs geometric altitude `Z` — U.S. Standard Atmosphere 1976 (≤86 km) + Braeunig fit to 1000 km. Part of the Earth family (`T/P/ρ/μ♁StdAtm`).

`Z` → pressure (`MPa`)

```rpl
2000_m P♁StdAtm     @ Expecting 0.0795014065710 MPa
```

## ρ♁StdAtm

Earth density vs `Z`, `ρ=P/(R·T)` in the homosphere (USSA 1976). Earth family (`T/P/ρ/μ♁StdAtm`).

`Z` → density (`kg/m³`)

```rpl
2000_m ρ♁StdAtm     @ Expecting 1.006553696670 kg/m^3
```

## μ♁StdAtm

Earth dynamic viscosity vs `Z` (Sutherland law on USSA-1976 temperature), valid −5…86 km. Earth family.

`Z` → viscosity (`Pa·s`)

```rpl
2000_m μ♁StdAtm     @ Expecting 1.7259816E-5 Pa·s
```

## ☿Φf

Mercury mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]` (au, degrees). Planet family: `☿♀♁♂♃♄⛢♆Φf`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ☿Φf     @ Expecting [ 0.38709935 0.20563985 7.00375687 … ]
```

## ♀Φf

Venus mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ♀Φf     @ Expecting [ 0.72333646 0.00676828 3.39451394 … ]
```

## ♂Φf

Mars mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ♂Φf     @ Expecting [ 1.52371414 0.09341030 1.84802052 … ]
```

## ♃Φf

Jupiter mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ♃Φf     @ Expecting [ 5.20286315 0.04835901 1.30401944 … ]
```

## ♄Φf

Saturn mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ♄Φf     @ Expecting [ 9.53641895 0.05375701 2.48638972 … ]
```

## ⛢Φf

Uranus mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ⛢Φf     @ Expecting [ 19.1887615 0.04724840 0.77213862 … ]
```

## ♆Φf

Neptune mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 ♆Φf     @ Expecting [ 30.0699768 0.00860097 1.77011616 … ]
```

## ♂Pf

Mars heliocentric ecliptic position from a Julian Day (`♂Φf` composed with `PosΦf`). Companion of `♁Pf`.

`JD` → `[x y z]` (au)

```rpl
2459050.5 ♂Pf     @ Expecting [ 1.159010882 -0.753079682 -0.044216374 ]
```

## DAToB

Distance between two heliocentric position vectors, `|A−B|`, reduced to au. Primitive used by `DABf`/`T₀SD*`.

`[A] [B]` → `d` (au)

```rpl
[ 1 0 0 ] [ 0 1 0 ] DAToB     @ Expecting 1.41421356 au
```

## DEarthToA

Earth-referenced distance: Earth-to-body separation from two position vectors (thin wrapper of `DAToB`).

`[E] [A]` → `d` (au)

```rpl
[ 1 0 0 ] [ 0 1 0 ] DEarthToA     @ Expecting 1.41421356 au
```

## T₀SDEarthToA

Julian Day of closest approach **from Earth** to a body, over `[JD1 JD2]` (Earth-referenced `T₀SDAToB`; pass the body element fn as `'Ⓛ…'`).

`'Ⓛ♂Φf' JD1 JD2` → `JDmin`

```rpl
'Ⓛ♂Φf' 2459120 2459140 T₀SDEarthToA     @ Expecting 2459129.07  (Mars, 2020-10-06)
```

## EaΦf

Eccentric anomaly `E` by solving Kepler's equation (Root). Building block of `PosΦf`.

`e M` → `E` (°)

```rpl
0.0167 5.0 EaΦf     @ Expecting 5.0848°
```

## νΦf

True anomaly `ν` from eccentricity and eccentric anomaly. Building block of `PosΦf`.

`e E` → `ν` (°)

```rpl
0.0167 5.0848 νΦf     @ Expecting 5.1703°
```

## rΦf

Heliocentric distance `r` from semi-major axis, eccentricity and eccentric anomaly. Building block of `PosΦf`.

`a e E` → `r` (au)

```rpl
1.0 0.0167 5.0848 rΦf     @ Expecting 0.98337 au
```

## uΦf

Argument of latitude `u` from argument of perihelion and true anomaly. Building block of `PosΦf`.

`ω ν` → `u` (°)

```rpl
30 60 uΦf     @ Expecting 60°
```

## λΦf

Ecliptic longitude `λ` from node, inclination and argument of latitude. Building block of `PosΦf`.

`Ω I u` → `λ` (°)

```rpl
100 7 50 λΦf     @ Expecting 48.7786°
```

## βΦf

Ecliptic latitude `β` from inclination and argument of latitude. Building block of `PosΦf`.

`I u` → `β` (°)

```rpl
7.0 50 βΦf     @ Expecting 5.3568°
```

## MSumC

Σ of `c·cos(row·av)` over a Meeus periodic-term table. Internal helper of `☾Pos` (Moon), rarely called alone; input `tbl` is a term matrix.

`av tbl` → `Σ`

```rpl
(internal — see ☾Pos)
```

## MSumS

Σ of `c·sin(row·av)` over a Meeus periodic-term table. Internal helper of `☾Pos` (Moon).

`av tbl` → `Σ`

```rpl
(internal — see ☾Pos)
```

## StumpC

Stumpff function `C(z)` (three numerical regimes). Building block of the universal-variable Lambert solver.

`z` → `C(z)`

```rpl
1.539854 StumpC     @ Expecting 0.439044
```

## StumpS

Stumpff function `S(z)` (three numerical regimes). Building block of the Lambert solver.

`z` → `S(z)`

```rpl
1.539854 StumpS     @ Expecting 0.154295
```

## LamUF

Universal-variable Lambert time-of-flight residual `F(z)` (root-solved by `LambertU`). Args `z r1 r2 A μ dt`.

`z r1 r2 A μ dt` → `F(z)`

```rpl
1.0 11375.85162 16383.22313 12372.27203 398600 3600 LamUF     @ Expecting -192258.13
```
