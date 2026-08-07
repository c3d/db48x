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
2459050.5 '♁Φf' '♂Φf' DABf
@ Expecting 0.707458 au                               (Earth–Mars, 2020-07-20)
```

## T₀SDAToB

Julian Day of closest approach between two bodies inside a search window
(minimum-distance date). `T₀SDEarthToA` is the Earth-referenced variant.

`'Af'  'Bf'  JD_lo  JD_hi` → `JD_min`

```rpl
'♁Φf' '♂Φf' 2459120 2459140 T₀SDAToB
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
