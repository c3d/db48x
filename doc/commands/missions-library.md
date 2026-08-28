# Missions — Earth To Mars & Back

Pedagogical Earth⇄Mars round-trip planners, built on the Astronautics transfer
machinery (universal-variable Lambert, gravity assist) and the heliocentric ephemerides.
Dates are Julian Days; ΔV in km/s. Two mission classes are covered: **conjunction** (long
stay, lowest ΔV) with `RTPlan`, and **opposition** (short stay, Venus gravity-assist
return) with `OppRoute` and `RTPlanOpp`. `MarsRoundTrip` compares the two side by side,
and `MissionΔV` gives an end-to-end propulsive ΔV budget.

An **Examples** submenu holds **four named worked missions**, each a specific Venus-flyby
return shown with its to-scale trajectory figure: **ETMB1** (2022), **ETMB2** (2035 — the
featured soft-entry twin, where the Venus swing-by does the most work), **ETMB3** (2042)
and **ETMB4** (2048).

## RTPlan

Conjunction-class Earth⇄Mars round-trip optimiser (heliocentric ΔV). It optimises the
outbound Earth→Mars leg, then the return Mars→Earth leg departing after a minimum Mars
surface stay, and returns the full itinerary. Slow, so the example sets a fast search
precision first. It reproduces the real Perseverance outbound window.

`t1lo t1hi stay` → `{ t1 t2 stay t3 t4 ΔVout ΔVret ΔVtot }`

```rpl
1 'AstronTXPrecision' STO
2459030 2459075 500 ⓁRTPlan
@ Expecting { 2 459 055.61728D 2 459 259.8011D 517.55829 904D 2 459 777.3594D 2 460 049.37586D 6.31785 61619D 6.50523 29888 4D 12.82308 91507D }
@ Earth 2020-07-25 → Mars 2021-02-14, stay 517 d, home 2023-04-14; ΔVtot 12.82 km/s
```

## RTPlanOpp

Best Venus-flyby Mars→Earth return over a Mars-departure window (opposition class). A
two-stage search (coarse grid then local refine) that minimises a combined cost
favouring a ballistic and turn-feasible Venus swing-by. Keep the window narrow (~60 d);
it is slow (~2-4 min). The result is feasible when matchErr is near zero and turnReq
does not exceed turnMax.

`t3lo t3hi` → `{ t3 tv t4 ΔVTEI matchErr turnReq turnMax tof }`

```rpl
2459920 2459980 ⓁRTPlanOpp
@ affichage complet : { 2459930 2460150 2460220 5.90 0.00084 30.93 49.72 290 }
@ Expecting { 2 459 930 2 460 150 2 460 220 5.90036 89128 9D 8.44315 38774 6⁳⁻⁴D 30.92503 57352D ° 49.71715 56229D 290 5.44155 12278 3D }
@ Mars 2022-12-16 → Venus 2023-07-24 → Earth 2023-10-02; Earth entry only ~4.9 km/s
```

## OppRoute

Evaluate one Mars→Venus→Earth gravity-assist return: two Lambert arcs patched by a Venus
swing-by. It returns the Mars-departure ΔV, the v∞ mismatch at Venus (near zero for a
valid ballistic flyby), the required versus achievable turn angle, the return time, and the
Earth entry speed. Instant, no search. A route is feasible when matchErr is near zero and
turnReq does not exceed turnMax; a low EarthEntry marks a strong Venus assist (soft entry).

`t3 tv t4` → `{ ΔVTEI matchErr turnReq turnMax tof EarthEntry }`

```rpl
2459950 2460160 2460240 ⓁOppRoute     @ feasible (turnReq 29.6 ≤ turnMax 37.7);
@ Expecting { 6.85680 22977 2 0.26798 63486 98 29.61008 76473 ° 37.69932 0865 290 4.87223 59083 7 }
@ EarthEntry only 4.87 km/s = a soft, Venus-assisted entry
```

## ♀Pf

Venus heliocentric position vector (au) from a Julian Day — the Venus position function,
sibling of the Earth and Mars position functions used by the transfer tools.

`JD` → `[x y z]`

```rpl
2459950 Ⓛ♀Pf     @ (au, 2023-01-05)
@ Expecting [ 0.61400 36319 03 -0.38907 09841 12 -4.07721 22786 7⁳⁻² ]
```

## MarsRoundTrip

Compare a conjunction versus an opposition Earth⇄Mars round trip. It runs RTPlan
(conjunction) and RTPlanOpp (opposition Venus-flyby return), computes the opposition
Earth-entry speed, and leaves five tagged values on the stack for a side-by-side read.
Slow (~90 s) — it chains both optimisers. Then explore your own windows with RTPlan and
RTPlanOpp directly.

(no input) → `Conj_years Conj_dVtot Opp_return_d Opp_TEI Opp_entry`

```rpl
ⓁMarsRoundTrip
@ affichage complet : Conj_years:2.72 Conj_dVtot:12.82 Opp_return_d:290 Opp_TEI:5.90 Opp_entry:5.44
@ Expecting Opp_entry:5.44155 12278 3D
@ Conjunction 2.72 yr / 12.82 km/s; opposition Venus return, Earth entry only 5.4 km/s.
```

## MissionΔV

End-to-end propulsive ΔV budget of a conjunction round trip. Feed it an RTPlan itinerary;
it recomputes the two transfer legs (TrCost) for their departure/arrival v∞, converts the
space burns from parking orbits with TrToOrbi, and adds representative atmospheric phases.
It returns eight tagged ΔV values (seven phases plus the total), in km/s. Chains after
RTPlan; the example uses a fixed conjunction itinerary so it runs instantly.

The seven propulsive phases, in chronological order:

* **AscentEarth** — Earth surface to Low Earth Orbit (the launch to a parking orbit).
* **TMI** — Trans-Mars Injection: from LEO onto the interplanetary transfer to Mars.
* **MOI** — Mars Orbit Insertion: braking capture from the arrival hyperbola into Mars orbit.
* **EDL_Mars** — Entry, Descent and Landing: from Mars orbit down to the surface.
* **AscentMars** — Mars surface back up to Mars orbit.
* **TEI** — Trans-Earth Injection: from Mars orbit onto the return transfer to Earth.
* **EntryEarth** — Earth atmospheric entry on return (aerobraked, so ~0).

The space phases (TMI, MOI, TEI) are computed rigorously; the atmospheric phases
(ascents, EDL, entry) are representative values (Earth ascent 9.4, Mars EDL 0.6, Mars
ascent 4.1, Earth entry 0 km/s), with parking orbits LEO 6678 km and Mars 3689 km.

`{ itinerary }` → `1_AscentEarth … 8_TOTAL`

```rpl
{ 2459055.617 2459259.801 517.558 2459777.359 2460049.376 6.31786 6.50523 12.82309 } ⓁMissionΔV     @ AscentEarth 9.4, TMI 3.81, MOI 2.07, EDL 0.6,
@ Expecting 8_TOTAL:22.46514 53281D
@ AscentMars 4.1, TEI 2.48, Entry 0. A typical chemical Mars round-trip budget.
```

## ETMB1

Earth-To-Mars-&-Back mission 2022 (launch), Venus-flyby return — a worked example. It runs
the validated opposition-class return (Mars→Venus→Earth) at fixed dates via OppRoute and
returns the return-leg itinerary as tagged values. The original 2022 mission. Venus turns ~31°, giving a gentle Earth entry of ~5.4 km/s. Its outbound Earth→Mars leg follows the Mars 2020 / Perseverance window (optimised by `RTPlan`).

![ETMB1 trajectory](img/etmb1.bmp)

`(no input)` → `MarsDeparture VenusFlyby EarthArrival ReturnDays dVTEI turnReq EarthEntry`

```rpl
ⓁETMB1
@ affichage complet : MarsDeparture:2459930 VenusFlyby:2460150 EarthArrival:2460220 ReturnDays:290
@ Expecting EarthEntry_kms:5.44155 12169 6
@ dVTEI_kms:5.90 turnReq_deg:30.9° EarthEntry_kms:5.44
```

## ETMB2

Earth-To-Mars-&-Back mission 2035 (launch), Venus-flyby return — a worked example. It runs
the validated opposition-class return (Mars→Venus→Earth) at fixed dates via OppRoute and
returns the return-leg itinerary as tagged values. **★ Featured mission.** The strongest twin found: Venus works even harder than in ETMB1 (~39° turn) for the *softest* entry — ~4.0 km/s, gentler than ETMB1 itself. It uniquely combines the Venus synodic resonance with a **perihelic Mars opposition** (Mars near the Sun), which is what makes the swing-by so strong — the best future crewed-return candidate in the ephemeris range.

![ETMB2 trajectory](img/etmb2.bmp)

`(no input)` → `MarsDeparture VenusFlyby EarthArrival ReturnDays dVTEI turnReq EarthEntry`

```rpl
ⓁETMB2
@ affichage complet : MarsDeparture:2464635 VenusFlyby:2464825 EarthArrival:2464915 ReturnDays:280
@ Expecting EarthEntry_kms:3.97248 80606 2
@ dVTEI_kms:7.14 turnReq_deg:39.3° EarthEntry_kms:3.97
```

## ETMB3

Earth-To-Mars-&-Back mission 2042 (launch), Venus-flyby return — a worked example. It runs
the validated opposition-class return (Mars→Venus→Earth) at fixed dates via OppRoute and
returns the return-leg itinerary as tagged values. Venus-light: the 6.4-year resonance yields a feasible passage but a weak assist (~7° turn), leaving a brutal ~14.7 km/s entry.

![ETMB3 trajectory](img/etmb3.bmp)

`(no input)` → `MarsDeparture VenusFlyby EarthArrival ReturnDays dVTEI turnReq EarthEntry`

```rpl
ⓁETMB3
@ affichage complet : MarsDeparture:2466965 VenusFlyby:2467105 EarthArrival:2467170 ReturnDays:205
@ Expecting EarthEntry_kms:14.68803 65576
@ dVTEI_kms:9.19 turnReq_deg:7.5° EarthEntry_kms:14.69
```

## ETMB4

Earth-To-Mars-&-Back mission 2048 (launch), Venus-flyby return — a worked example. It runs
the validated opposition-class return (Mars→Venus→Earth) at fixed dates via OppRoute and
returns the return-leg itinerary as tagged values. The cheapest full mission (outbound v∞ only ~3.6 km/s), with a moderate ~9.6 km/s entry.

![ETMB4 trajectory](img/etmb4.bmp)

`(no input)` → `MarsDeparture VenusFlyby EarthArrival ReturnDays dVTEI turnReq EarthEntry`

```rpl
ⓁETMB4
@ affichage complet : MarsDeparture:2469315 VenusFlyby:2469470 EarthArrival:2469535 ReturnDays:220
@ Expecting EarthEntry_kms:9.57918 83462 1
@ dVTEI_kms:6.24 turnReq_deg:9.7° EarthEntry_kms:9.58
```
