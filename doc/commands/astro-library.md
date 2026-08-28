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

Each `##` heading is the exact function name, so long-press help resolves it.
Two of the entries stand for a whole family — the eight planetary element
functions (`…Φf`) and the six coordinate transforms — and list their
other members in the entry body.

---

## ♁Φf

Mean orbital elements of a planet from a Julian Day (Standish/JPL), returned as
`[a e I L ϖ Ω]` (au and degrees). The full family shares this format:
`☿Φf` `♀Φf` `♁Φf` `♂Φf` `♃Φf` `♄Φf` `⛢Φf` `♆Φf`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2451545 Ⓛ♁Φf        @ Earth mean elements at J2000 → [ a e I L ϖ Ω ]
```

## ☾Φf

Mean orbital elements of the Moon (geocentric), same output shape as a planet.

`JD` → `[a e I L ϖ Ω]`

```rpl
2451545 Ⓛ☾Φf        @ Moon mean elements at J2000
```

## ☾Pos

Geocentric ecliptic position of the Moon from the Meeus lunar theory (longitude
`λ`, latitude `β`, distance `Δ`), accurate to ≈ 1′. Sums the periodic terms via
the helpers `MSumS`/`MSumC`.

`JD` → `[ λ β Δ ]`   (deg, deg, km)

```rpl
2451545 Ⓛ☾Pos       @ Moon geocentric ecliptic longitude / latitude / distance
```

## ☾Hf

Heliocentric ecliptic position of the Moon `[x y z]` in au — the Moon expressed
as a **position function**, so it plugs into the transfer machinery exactly like
a planet.

`JD` → `[x y z]`   (au)

```rpl
2459050.5 Ⓛ☾Hf      @ Moon heliocentric position (au)
```

---

## PosΦf

Heliocentric ecliptic position `[x y z]` (au) from an orbital-element vector.
Solves Kepler's equation and chains the six coordinate transforms. Guards the
angle mode to degrees. The transforms are also callable individually: `EaΦf`
(eccentric anomaly, Kepler root), `νΦf` (true anomaly), `rΦf` (radius), `uΦf`
(argument of latitude), `λΦf` (ecliptic longitude), `βΦf` (ecliptic latitude).

`[a e I L ϖ Ω]` → `[x y z]`

```rpl
2459050.5 Ⓛ♁Φf ⓁPosΦf     @ Earth heliocentric position on that date (au)
```

## ♁Pf

Position-function wrapper: element function composed with `PosΦf`, mapping a date
directly to a heliocentric `[x y z]`. Define one per body as needed (`♂Pf`, …).
Fixed check: `2459050.5 Ⓛ♁Pf` → `[0.468733637 -0.901591683 0.000042104]`.

`JD` → `[x y z]`

```rpl
DateTime JDN Ⓛ♁Pf     @ Earth's heliocentric position right now (au); its norm ≈ 1 au
```

## VΦf

Heliocentric velocity of a body by centred finite difference of a **position
function** (`h = 0.5 day`).

`JD  'Pf'` → `[vx vy vz]`   (au/day)

```rpl
2459050.5 'Ⓛ♁Pf' ⓁVΦf     @ (= 29.307 km/s)
@ Expecting [ 0.01498 46942 23 7.87138 25698 8⁳⁻³ -3.62016 40016 7⁳⁻⁷ ]
```

---

## DABf

Distance between two bodies at one date, from their **element functions**.

`JD  'Af'  'Bf'` → `distance`   (au)

```rpl
2459050.5 'Ⓛ♁Φf' 'Ⓛ♂Φf' ⓁDABf     @ (Earth–Mars, 2020-07-20)
@ Expecting 0.70745 83398 38 au
```

## T₀SDAToB

Julian Day of closest approach between two bodies inside a search window
(minimum-distance date). `T₀SDEarthToA` is the Earth-referenced variant.

`'Af'  'Bf'  JD_lo  JD_hi` → `JD_min`

```rpl
'Ⓛ♁Φf' 'Ⓛ♂Φf' 2459120 2459140 ⓁT₀SDAToB     @ (2020-10-06)
@ Expecting 2 459 129.07195
```

## θs

Mean sidereal time from a **Universal-Time** Julian Date `JD` and a longitude
`L`, using the IAU sidereal-time polynomial: `θs = GMST(JD) + L`, reduced to
`[0, 360)°`. With `L = 0` this returns Greenwich mean sidereal time (GMST); with
the observer’s longitude (East positive, West negative) it returns the local
mean sidereal time. Sidereal time is the hour angle of the vernal equinox.

`JD  L` → `θs` (°)

```rpl
2451545 0 Ⓛθs     @ (GMST at the J2000.0 epoch, 2000-01-01 12:00 UT)
@ Expecting 280.46061 837 °
```

⚠️ Feed `θs` a **Universal-Time** Julian Date. DB48x’s `JDN` gives the Julian Date
directly (this build carries the JDN half-day fix). Convert your civil (local)
clock time to UT first. For example `20201006.120000_date JDN` (2020-10-06
12:00 UT) → `2459129`, which `-73.58 θs` turns into a local mean sidereal time.

---

## Hohmann

Two-impulse Hohmann transfer between two circular orbits of radii `r₁`, `r₂`
about a body of parameter `μ`.

`r₁  r₂  μ` → `{ ΔV₁  ΔV₂  ΔV_tot  tof }`

```rpl
6678_km 42164_km ⒸGM♁ ⓁHohmann     @ (LEO→GEO)
@ Expecting { 2 425.76890 112 m/s 1 466.83863 837 m/s 3 892.60753 949 m/s 18 990.05283 42 s }
```

## BiElliptic

Three-impulse bi-elliptic transfer via an intermediate apoapsis `r_b`.

`r₁  r₂  r_b  μ` → `{ ΔV₁  ΔV₂  ΔV₃  ΔV_tot  tof }`

```rpl
6678_km 42164_km 200000_km ⒸGM♁ ⓁBiElliptic
@ Expecting { 3 022.18246 923 m/s 474.20023 8827 m/s 876.93561 391 m/s 4 373.31832 197 m/s 374 954.39105 8 s }
```

## TrToOrbi

Orbital insertion / capture from a hyperbolic approach: burn from the arrival
hyperbola into a capture orbit (set `r_a = r_p` for circular capture).

`v∞  r_p  r_a  μ` → `{ ΔV_ins  v_hyp  v_cap }`

```rpl
2.5_km/s 3689_km 3689_km ⒸGM♂ ⓁTrToOrbi     @ (circular capture)
@ Expecting { 2 021.27352 7 m/s 5 428.58216 969 m/s 3 407.30864 269 m/s }
```

## LambertU

Lambert's problem by the **universal-variable** formulation (Stumpff functions),
robust for elliptic, parabolic and hyperbolic transfers including near-180°
geometry. Given two position vectors and a time of flight, returns the terminal
velocity vectors.

`[r₁]  [r₂]  tof  μ` → `{ [v₁]  [v₂] }`

```rpl
[ 5000 10000 2100 ] [ -14600 2500 7000 ] 3600 398600 ⓁLambertU
@ full result: { [ -5.99249 46396 7 1.92536 34152 8 3.24563 65284 9 ] [ -3.31246 03109 4 -4.19661 73079 3 -0.38528 76170 68 ] }
1 GET     @ departure velocity, the vector the test compares
@ Expecting [ -5.99249 46396 7 1.92536 34152 8 3.24563 65284 9 ]
@ (Curtis, Example 5.2)
```

Helpers: `StumpC` (`z` → C(z)), `StumpS` (`z` → S(z)), `LamUF` (universal Kepler
residual used by the internal root).

## rv2coe

State vector to classical orbital elements.

`[r]  [v]  μ` → `{ a  e  i  Ω  ω  ν }`

```rpl
[ -6045 -3490 2500 ] [ -3.457 6.618 2.533 ] 398600 Ⓛrv2coe
@ Expecting { 8 788.09511 738 0.17121 23462 84 153.24922 8518 255.27928 5334 20.06831 66506 28.44562 83066 }
```

---

## TrCost

ΔV cost of a ballistic transfer between two dates, using **position functions**
for the departure and arrival bodies. Solves Lambert between the two positions
and differences against each body's velocity. The example below reproduces the
Earth→Mars leg of NASA's **Mars 2020 (Perseverance)** mission — departure in the
late-July 2020 window, ~200-day cruise.

`t₁  t₂  'Af'  'Bf'` → `{ ΔV_tot  ΔV_dep  ΔV_arr }`   (km/s)

```rpl
2459050.5 2459250.5 'Ⓛ♁Pf' 'Ⓛ♂Pf' ⓁTrCost     @ (Earth→Mars ~200 d — Mars 2020 / Perseverance)
@ Expecting { 6.37381 3392 3.64458 48380 2 2.72922 85539 7 }
```

The Moon plugs in identically:

```rpl
2459050.5 2459250.5 'Ⓛ☾Hf' 'Ⓛ♂Pf' ⓁTrCost     @ (Moon→Mars)
@ Expecting { 7.40385 78493 9 4.67123 39159 3 2.73262 39334 5 }
```

## MinTofDV

For a fixed departure date, the flight time that minimises `TrCost` ΔV
(ternary search).

`t₁  'Af'  'Bf'  tof_lo  tof_hi` → `{ ΔV  tof }`

```rpl
1 'AstronTXPrecision' STO
2459054 'Ⓛ♁Pf' 'Ⓛ♂Pf' 150 225 ⓁMinTofDV
@ Expecting { 6.32098 67868 5 205.65843 6214 }
```

## MinΔVTraj

Optimal launch window: minimise ΔV over **both** departure date and flight time
(2-D ternary search over `TrCost`). Keep the window under the 180° ridge. The
example scans the real **Mars 2020 (Perseverance)** launch window of late July 2020.

`'Af'  'Bf'  t₁_lo  t₁_hi  tof_lo  tof_hi` → `{ ΔV  t₁  tof }`

```rpl
1 'AstronTXPrecision' STO
'Ⓛ♁Pf' 'Ⓛ♂Pf' 2459030 2459075 150 225 ⓁMinΔVTraj     @ (Mars 2020 = Perseverance window; level 1)
@ Expecting { 6.31701 94166 9 2 459 055.61728 205.65843 6214 }
```

Runtime is set by the variable `AstronTXPrecision` (below): with the native
`PosPhifN` / `LambertUN` commands of this branch, this example runs at level 1 in
≈ 70 ms on the simulator, level 3 in ≈ 220 ms. Level 3 only sharpens the dates —
the minimum ΔV is nearly identical at every level (6.31702 at level 1 versus
6.31693 at level 3, i.e. 1e-4 km/s, while t₁ moves by 0.08 d and tof by 0.28 d).

## MinΔDTraj

Minimum-energy transfer for a departure date: the flight time giving the
smallest semi-major axis `a` (via `aTr`). A 1-D optimum by geometry.

`t₁  'Af'  'Bf'  tof_lo  tof_hi` → `{ a  tof }`

```rpl
1 'AstronTXPrecision' STO
2459054 'Ⓛ♁Pf' 'Ⓛ♂Pf' 180 270 ⓁMinΔDTraj
@ Expecting { 1.31742 87172 4 228.76543 2099 }
```

## aTr

Semi-major axis of the transfer between two dates (vis-viva on the `LambertU`
solution). Used by `MinΔDTraj`.

`t₁  t₂  'Af'  'Bf'` → `a`   (au)

```rpl
2459054 2459282 'Ⓛ♁Pf' 'Ⓛ♂Pf' ⓁaTr
@ Expecting 1.31742 78755 9
```

## MinΔtTraj

Fastest transfer under a ΔV budget: smallest flight time whose `TrCost` stays
within `budget` (bisection).

`t₁  'Af'  'Bf'  budget  tof_lo  tof_hi` → `{ tof  ΔV }`

```rpl
1 'AstronTXPrecision' STO
2459054 'Ⓛ♁Pf' 'Ⓛ♂Pf' 7 150 205 ⓁMinΔtTraj     @ (7 km/s budget)
@ Expecting { 173.63281 25 6.98966 53628 5 }
```

## AstronTXPrecision

Global variable controlling the iteration count of the window optimizers
(`MinTofDV`, `MinΔVTraj`, `MinΔtTraj`, `MinΔDTraj`): `1`/`2`/`3` → `6`/`9`/`12`
iterations. The search is 2-D, so the cost grows as the *square* of the iteration
count: measured simulator runtime for `MinΔVTraj` ≈ 70 / 130 / 220 ms with the
native `PosPhifN` / `LambertUN` commands of this branch (hardware ≈ ×8). On the
pure-RPL path (branches without those commands) the same three runs take
≈ 16 / 35 / 63 s.
Use level 1 to explore, level 3 to refine. Set it before calling an optimizer
(`1 'AstronTXPrecision' STO`); the routines read it to size their search and
provide no built-in default.

---

## GAssist

Gravity-assist (swing-by) in the patched-conic model. In the planet frame the
hyperbolic excess speed `v∞` keeps its magnitude but turns by `δ`, with
`sin(δ/2) = 1/e` and `e = 1 + r_p·v∞²/μ`. The free heliocentric velocity change
is `Δv = 2·v∞/e`, maximal near `v∞ ≈ √(μ/r_p)`.

`v∞  r_p  μ` → `{ δ  Δv  e }`   (deg, m/s, –)

```rpl
5_km/s 3500_km ⒸGM♂ ⓁGAssist     @ (Mars flyby, 3500 km periapsis)
@ Expecting { 38.36994 72818 ° 3 286.18963 813 m/s 3.04303 80170 3 }
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
2000_m ⓁT♁StdAtm     @ Expecting 275.154088844 K
60_km  ⓁP♁StdAtm     @ Expecting 2.195879E-5 MPa
2000_m Ⓛρ♁StdAtm     @ Expecting 1.00655 36966 7 kg/m↑3
60_km  Ⓛμ♁StdAtm     @ Expecting 1.58371 893⁳⁻⁵ Pa·s
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
0_m    ⓁT♂DayAtm      @ Expecting 228.50 K
50_km  ⓁP♂DayAtm      @ Expecting 3.28196 Pa
0_m    ⓁT♂NightAtm    @ Expecting 198. K
50_km  Ⓛρ♂NightAtm    @ Expecting 9.19789 41696 2⁳⁻⁵ kg/m↑3
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
0_m    ⓁT♀Atm         @ Expecting 735.000 K
50_km  ⓁP♀Atm         @ Expecting 106176 Pa
87_km  Ⓛρ♀Atm         @ Expecting 2.39307 64108 9⁳⁻³ kg/m↑3
```


<!-- ========== Per-function reference fiches (added 2026-08-08) ========== -->

## P♂DayAtm

Mars **dayside** (Viking) pressure vs geometric altitude `Z`, Braeunig Tables 13/15 (−8…300 km).

`Z` → pressure (`Pa`)

```rpl
0_m ⓁP♂DayAtm     @ Expecting 610.5 Pa
```

## ρ♂DayAtm

Mars **dayside** density vs `Z`, `ρ=P/(R·T)` ≤120 km + Braeunig Table 15 (−8…300 km).

`Z` → density (`kg/m³`)

```rpl
0_m Ⓛρ♂DayAtm     @ Expecting 0.01397 50939 1 kg/m↑3
```

## T♂NightAtm

Mars **nightside** (Pathfinder) temperature vs `Z`, Braeunig Tables 12/13.

`Z` → temperature (`K`)

```rpl
0_m ⓁT♂NightAtm     @ Expecting 198. K
```

## P♂NightAtm

Mars **nightside** (Pathfinder) pressure vs `Z`, Braeunig Table 13.

`Z` → pressure (`Pa`)

```rpl
0_m ⓁP♂NightAtm     @ Expecting 610.5 Pa
```

## ρ♂NightAtm

Mars **nightside** density vs `Z`, `ρ=P/(R·T)` ≤120 km.

`Z` → density (`kg/m³`)

```rpl
0_m Ⓛρ♂NightAtm     @ Expecting 1.61278 23022 9⁳⁻² kg/m↑3
```

## P♀Atm

Venus pressure (single global model) vs `Z`, Braeunig Tables 18/20 (−3…300 km).

`Z` → pressure (`Pa`)

```rpl
0_m ⓁP♀Atm     @ Expecting 9 332 000 Pa
```

## ρ♀Atm

Venus density vs `Z`, `ρ=P/(R·T)` ≤100 km + Braeunig Table 20 (−3…300 km).

`Z` → density (`kg/m³`)

```rpl
0_m Ⓛρ♀Atm     @ Expecting 66.35032 23789 kg/m↑3
```

## P♁StdAtm

Earth pressure vs geometric altitude `Z` — U.S. Standard Atmosphere 1976 (≤86 km) + Braeunig fit to 1000 km. Part of the Earth family (`T/P/ρ/μ♁StdAtm`).

`Z` → pressure (`MPa`)

```rpl
2000_m ⓁP♁StdAtm     @ Expecting 0.07950 14065 71 MPa
```

## ρ♁StdAtm

Earth density vs `Z`, `ρ=P/(R·T)` in the homosphere (USSA 1976). Earth family (`T/P/ρ/μ♁StdAtm`).

`Z` → density (`kg/m³`)

```rpl
2000_m Ⓛρ♁StdAtm     @ Expecting 1.00655 36966 7 kg/m↑3
```

## μ♁StdAtm

Earth dynamic viscosity vs `Z` (Sutherland law on USSA-1976 temperature), valid −5…86 km. Earth family.

`Z` → viscosity (`Pa·s`)

```rpl
2000_m Ⓛμ♁StdAtm     @ Expecting 1.72598 1622⁳⁻⁵ Pa·s
```

## ☿Φf

Mercury mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]` (au, degrees). Planet family: `☿♀♁♂♃♄⛢♆Φf`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ☿Φf     @ Expecting [ 0.38709 93460 31 0.20563 98466 28 7.00375 68739 30 967.29907 22 77.49077 25784 48.30500 97233 ]
```

## ♀Φf

Venus mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ♀Φf     @ Expecting [ 0.72333 64614 09 6.76828 05507 2⁳⁻³ 3.39451 39394 2 12 206.76933 6 131.60301 8568 76.62277 93421 ]
```

## ♂Φf

Mars mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ♂Φf     @ Expecting [ 1.52371 41353 9 9.34102 96673 8⁳⁻² 1.84802 05220 6 3 928.57570 696 -23.85230 7869 49.49941 81741 ]
```

## ♃Φf

Jupiter mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ♃Φf     @ Expecting [ 5.20286 31488 5 4.83590 06491⁳⁻² 1.30401 94372 2 658.00457 362 14.77215 1808 100.51597 0931 ]
```

## ♄Φf

Saturn mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ♄Φf     @ Expecting [ 9.53641 89549 7 0.05375 70089 05 2.48638 97159 5 301.16371 3908 92.51278 39761 113.60310 4226 ]
```

## ⛢Φf

Uranus mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ⛢Φf     @ Expecting [ 19.18876 15191 4.72484 04631 5⁳⁻² 0.77213 86161 29 401.28661 5394 171.03812 6823 74.02563 89905 ]
```

## ♆Φf

Neptune mean orbital elements from a Julian Day (Standish/JPL), `[a e I L ϖ Ω]`.

`JD` → `[a e I L ϖ Ω]`

```rpl
2459050.5 Ⓛ♆Φf     @ Expecting [ 30.06997 67852 8.60097 02334⁳⁻³ 1.77011 61557 1 -10.22892 9721 44.89850 9482 131.78318 0489 ]
```

## ♂Pf

Mars heliocentric ecliptic position from a Julian Day (`♂Φf` composed with `PosΦf`). Companion of `♁Pf`.
Fixed check: `2459050.5 Ⓛ♂Pf` → `[1.159010882 -0.753079682 -0.044216374]`.

`JD` → `[x y z]` (au)

```rpl
DateTime JDN Ⓛ♂Pf     @ Mars' heliocentric position right now (au); its norm ~ 1.5 au
```

## DAToB

Distance between two heliocentric position vectors, `|A−B|`, reduced to au. Primitive used by `DABf`/`T₀SD*`.
Primitive check: `[1 0 0] [0 1 0] DAToB` → `1.41421356 au`.

`[A] [B]` → `d` (au)

The live example chains `♁Pf` and `♂Pf` at today's date, so it returns the
**current Earth-Mars distance** — the value changes every time you run it:

```rpl
DateTime JDN DUP Ⓛ♁Pf SWAP Ⓛ♂Pf ⓁDAToB   @ current Earth-Mars distance (au)
```

## DEarthToA

Earth-to-body distance at one date, with **Earth built in**: give the date and
the target's element function (like `DABf`, but the first body is always Earth).
That is exactly what makes it different from `DAToB`, which takes two arbitrary
vectors. Fixed check: `2459050.5 'Ⓛ♂Φf' DEarthToA` → `0.707458 au` (Earth-Mars).

`JD  'Af'` → `d` (au)

```rpl
DateTime JDN 'Ⓛ♀Φf' ⓁDEarthToA   @ current Earth-Venus distance (au)
```

## T₀SDEarthToA

Julian Day of closest approach **from Earth** to a body, over `[JD1 JD2]` (Earth-referenced `T₀SDAToB`; pass the body element fn as `'Ⓛ…'`).

`'Ⓛ♂Φf' JD1 JD2` → `JDmin`

```rpl
'Ⓛ♂Φf' 2459120 2459140 ⓁT₀SDEarthToA     @ (Mars, 2020-10-06)
@ Expecting 2 459 129.07195
```

## EaΦf

Eccentric anomaly `E` by solving Kepler's equation (Root). Building block of `PosΦf`.

`e M` → `E` (°)

```rpl
0.0167 5.0 ⓁEaΦf     @ Expecting EaLΦ=5.08480 48182 5 °
```

## νΦf

True anomaly `ν` from eccentricity and eccentric anomaly. Building block of `PosΦf`.

`e E` → `ν` (°)

```rpl
0.0167 5.0848 ⓁνΦf     @ Expecting 5.17032 19838 5 °
```

## rΦf

Heliocentric distance `r` from semi-major axis, eccentricity and eccentric anomaly. Building block of `PosΦf`.

`a e E` → `r` (au)

```rpl
1.0 0.0167 5.0848 ⓁrΦf     @ Expecting 0.98336 57209 61
```

## uΦf

Argument of latitude `u` from argument of perihelion and true anomaly. Building block of `PosΦf`.

`ω ν` → `u` (°)

```rpl
30 60 ⓁuΦf     @ Expecting 90
```

## λΦf

Ecliptic longitude `λ` from node, inclination and argument of latitude. Building block of `PosΦf`.

`Ω I u` → `λ` (°)

```rpl
100 7 50 ⓁλΦf     @ Expecting 149.78878 4155 °
```

## βΦf

Ecliptic latitude `β` from inclination and argument of latitude. Building block of `PosΦf`.

`I u` → `β` (°)

```rpl
7.0 50 ⓁβΦf     @ Expecting 5.35678 17402 6 °
```

## MSumC

Σ of `c·cos(row·av)` over a Meeus periodic-term table. Internal helper of `☾Pos` (Moon), rarely called alone; input `tbl` is a term matrix.

`av tbl` → `Σ`

```
(internal — see Ⓛ☾Pos)
```

## MSumS

Σ of `c·sin(row·av)` over a Meeus periodic-term table. Internal helper of `☾Pos` (Moon).

`av tbl` → `Σ`

```
(internal — see Ⓛ☾Pos)
```

## StumpC

Stumpff function `C(z)` (three numerical regimes). Building block of the universal-variable Lambert solver.

`z` → `C(z)`

```rpl
1.539854 ⓁStumpC     @ Expecting 0.43904 36562 93
```

## StumpS

Stumpff function `S(z)` (three numerical regimes). Building block of the Lambert solver.

`z` → `S(z)`

```rpl
1.539854 ⓁStumpS     @ Expecting 0.15429 50940 11
```

## LamUF

Universal-variable Lambert time-of-flight residual `F(z)` (root-solved by `LambertU`). Args `z r1 r2 A μ dt`.

`z r1 r2 A μ dt` → `F(z)`

```rpl
1.0 11375.85162 16383.22313 12372.27203 398600 3600 ⓁLamUF     @ Expecting -192 258.13236 1
```

## PeriSel

Time of the lunar perigee passage (Selene) nearest a given date, from Meeus' *Astronomical
Algorithms* (2nd ed., ch. 50, full Table 50.A). The input is a Julian Day (or a date value);
the result is the Julian Day of the perigee.

`JD` → `JDperigee`

```rpl
2464291.5 ⓁPeriSel     @ (lunar perigee 2034-Nov-25)
@ Expecting 2 464 292.42195
```
