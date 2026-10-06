# Aeronautics functions

## AeronauticsLibrary

The Aeronautics section of the library holds programs for the calculations of
aircraft operation that need lists, loops or comparisons, and do not fit the
equation solver. Its equations are in the Aeronautics section of the Equation
Library.

* [Flight Management](#flight-managementlibrary): weight and balance, and
  the flight plan, and the check of the airspeed indicator.
* [Aircraft Performance](#aircraft-performancelibrary): the drag polar
  from measurements, the speeds of best climb, and the takeoff roll.

## Flight ManagementLibrary

Weight and balance, from the load sheet to the envelope, and the flight
plan along a route:

* [WBLoad](#wbload): the total weight, moment and center of gravity of a
  loading, station by station;
* [WBEnv](#wbenv): whether a loading lies within the center of gravity
  envelope, and the forward and aft limits at its weight;
* [FPlan](#fplan): the flight plan along the route, leg by leg, with the
  wind: courses, headings, ground speeds, times and fuel;
* [TAS3GS](#tas3gs): the true airspeed and the wind from three GPS ground
  speeds, to check the airspeed indicator.

## WBLoad

The weight and balance of a loading: the total weight `Wt`, the total moment
`Mom` and the center of gravity `CG = Mom/Wt`.

Stack: a list of stations, one `{ "name" weight arm }` list each ▶ `Wt` `Mom`
`CG`, tagged. Arms are measured from the datum, positive aft, and may be
negative. A negative weight removes a load, for instance the fuel burnt
before landing. Weights and arms may carry units, or be plain numbers in
consistent units.

The loading of the FAA example: empty aircraft 2 100 lb at 78.3 in, front
seats 340 lb at 85 in, rear seats 350 lb at 121 in, 75 gal of fuel, that is
450 lb, at 75 in, and 80 lb of baggage at 150 in. The book finds 3 320 lb and
a CG of 84.8 in (*Pilot's Handbook of Aeronautical Knowledge*, FAA-H-8083-25C,
chapter 10):

```rpl
{ { "Empty" 2100_lb 78.3_in } { "Front" 340_lb 85_in } { "Rear" 350_lb 121_in } { "Fuel" 450_lb 75_in } { "Baggage" 80_lb 150_in } } ⓁWBLoad
@ Expecting CG:84.76807 22892 in
```

With plain numbers, and the negative arm of an oil tank ahead of the datum.
The book prints a moment of 122 086 and a CG of 74.0, because its moment of
the empty aircraft, 69 393, differs from 1 011.9 × 68.6 = 69 416.34 (same
handbook):

```rpl
{ { "Empty" 1011.9 68.6 } { "Oil" 11 -31 } { "Fuel" 108 84 } { "Aux" 108 84 } { "Pilot" 170 81 } { "Pax" 170 81 } { "Bag" 70 105 } } ⓁWBLoad
@ Expecting CG:74.05503 06265
```

See also: [WBEnv](#wbenv), and the equations Weight Shift, Weight Change and
CG in % MAC of the Equation Library.

## WBEnv

Whether a loading lies within the center of gravity envelope, and the forward
and aft limits at its weight.

Stack: the envelope, a list of `{ CG weight }` points in order around it, the
weight `W` and the center of gravity `CG` of the loading ▶ `Fwd` `Aft`
`Inside`, tagged, with `Inside` 1 when `CG` lies between the limits and 0
otherwise.

The limits are where the envelope crosses the weight `W`, interpolated
linearly between its points: the envelope must be convex, as center of
gravity envelopes are. A weight above or below the envelope is an error.

The envelope of the FAA example: from 77.0 to 85.7 in up to 2 475 lb, then a
forward limit rising to 77.5 in at 2 525 lb and to 82.1 in at 2 950 lb, and
an aft limit of 85.7 in at 2 525 lb and 84.7 in at 2 950 lb; the lower edge,
1 500 lb, is chosen for the example. At 2 799 lb, the limits are 80.47 and
85.06 in, and a CG of 81.39 in is within them (*Pilot's Handbook of
Aeronautical Knowledge*, chapter 10):

```rpl
{ { 77.0 1500 } { 77.0 2475 } { 77.5 2525 } { 82.1 2950 } { 84.7 2950 } { 85.7 2525 } { 85.7 1500 } } 2799 81.3862 ⓁWBEnv
@ Expecting Inside:1
```

The same loading with its CG at 80 in is too far forward:

```rpl
{ { 77.0 1500 } { 77.0 2475 } { 77.5 2525 } { 82.1 2950 } { 84.7 2950 } { 85.7 2525 } { 85.7 1500 } } 2799 80 ⓁWBEnv
@ Expecting Inside:0
```

See also: [WBLoad](#wbload).

## FPlan

The flight plan along the route, leg by leg: for each leg, its distance, its
course, the heading to fly and the ground speed with the wind, the time en
route and the fuel; then the totals.

Stack: `TAS` `FFR` `wind` ▶ the list of legs, `Dist`, `ETE` and `Fuel`,
tagged. `TAS` is the true airspeed, in knots if a plain number. `FFR` is the
fuel flow, per hour if a plain number, for instance `12_gal/h`. The wind is
one `{ WD WS }` list for the whole route, or a list with one `{ WD WS }` per
leg, `WD` being where the wind comes from, in degrees if a plain number, and
`WS` its speed, in knots if a plain number. Each leg is a list
`{ "A→B" D TC TH GS ETE fuel }`.

The route is the variable `Route`, built with the Route functions of the
Navigation section, and its distances and courses are those of the WGS-84
ellipsoid. On a long leg the course changes along the way: the wind is
corrected on the mean course of each leg, the average of its initial and
final courses. From Albuquerque to New York the course goes from 67.9° to
88.4°, and the mean, 78.2°, gives 1.8 gal less than the initial course. The
fuel for taxi and the reserve are in the Fuel Required equation of the
Equation Library; `→HMS` shows a time in hours, minutes and seconds.

From Los Angeles to New York through Albuquerque, at 250 kt and 12 gal/h,
with a wind from 270° at 30 kt on the first leg and from 300° at 50 kt on
the second:

```rpl
{ { "LAX" 33.9425 -118.4081 } { "ABQ" 35.0402 -106.6090 } { "JFK" 40.6398 -73.7789 } } 'Route' STO
250 12_gal/h { { 270 30 } { 300 50 } } ⓁFPlan
@ Expecting Fuel:92.02427 83446 gal
```

The same route with one wind, from 270° at 30 kt, for the whole flight:

```rpl
{ { "LAX" 33.9425 -118.4081 } { "ABQ" 35.0402 -106.6090 } { "JFK" 40.6398 -73.7789 } } 'Route' STO
250_knot 12 { 270 30 } ⓁFPlan
@ Expecting Fuel:93.39856 62566
```

See also: [WPLegs](#wplegs), the legs of the route without the wind.

## TAS3GS

The true airspeed and the wind from three GPS ground speeds: a way to check
the airspeed indicator in flight, with nothing but a GPS. Fly three legs at
the same indicated airspeed and altitude, on headings 120° apart, and note
the ground speed of each.

Stack: `H1` `GS1` `GS2` `GS3` ▶ `TAS` `WD` `WS`, tagged. `H1` is the first
heading, in degrees if a plain number, and `GS1`, `GS2`, `GS3` the ground
speeds on `H1`, `H1+120°` and `H1+240°`, in knots if plain numbers. `TAS` is
the true airspeed, `WD` the direction the wind comes from and `WS` its
speed.

The true airspeed and the wind speed come from the method of Ed Williams
(*Aviation Formulary*); the wind direction then follows from the ground
speed along each heading. Ground speeds too far from each other to come
from one airspeed and one wind are an error.

At 120 kt, with a wind from 040° at 20 kt, the ground speeds on 000°, 120°
and 240° are 105.47, 118.18 and 138.96 kt:

```rpl
0 105.4659 118.17975 138.96218 ⓁTAS3GS
@ Expecting WS:19.99976 68283 knot
```

The same flight on 030°, 150° and 270°:

```rpl
30_° 100.36395 128.22518 133.73623 ⓁTAS3GS
@ Expecting WS:19.99999 89702 knot
```

## Aircraft PerformanceLibrary

The performance of the aircraft from its drag polar, where the equation
solver is not enough: a least squares fit, an optimum, an integration.

* [PolarFit](#polarfit): the drag polar fitted to measured points;
* [ClimbVxVy](#climbvxvy): the speeds of best rate and of best angle of
  climb of a propeller airplane;
* [TORoll](#toroll): the takeoff ground roll, integrated step by step, with
  the wind and a thrust that falls with the speed.

## PolarFit

The parabolic drag polar `CD = CD0 + Kind·CL²` that best fits a list of
measured lift and drag coefficients, by least squares on `CL²`, and the
maximum lift to drag ratio that follows.

Stack: a list of points, one `{ CL CD }` list each ▶ `CD0` `Kind` `LDmax`
`CLmd`, tagged. At least two points must have different lift coefficients.
Flight test measurements give the coefficients through the Lift and Drag
Polar equations of the Equation Library: the lift is the weight, the drag is
the thrust in level flight.

Three points taken exactly on the polar `CD = 0.03 + 0.055·CL²` of a
textbook example give it back (J. G. Leishman, *Introduction to Aerospace
Flight Vehicles*, Embry-Riddle Aeronautical University, *Takeoff & Landing
Performance*):

```rpl
{ { 0.2 0.0322 } { 0.4 0.0388 } { 0.6 0.0498 } } ⓁPolarFit
@ Expecting CLmd:0.73854 89458 76
```

Five points of a light airplane, read to three significant digits:

```rpl
{ { 0.3 0.0248 } { 0.5 0.0332 } { 0.7 0.0459 } { 0.9 0.0628 } { 1.1 0.0839 } } ⓁPolarFit
@ Expecting CLmd:0.61600 61969 29
```

See also: the Drag Polar and Characteristic Speeds equations of the Equation
Library.

## ClimbVxVy

The speed `Vy` of the best rate of climb, which gains the most height in a
given time, and the speed Vx of the best angle of climb, which gains the
most height in a given distance, for a propeller airplane whose engine gives
a constant power.

![Climb speeds](img/ClimbSpeeds.bmp)

Stack: `CD0` `Kind` `Wt` `Sw` `ρa` `Pav` `ηp` `CLmax` ▶ `Vy` `VSmax` Vx
`FPAmax`, tagged: the drag polar, the mass, the wing area, the air density,
the shaft power available, the propeller efficiency and the maximum lift
coefficient. Values with units are converted; plain numbers are in SI units.

`Vy` is the speed of minimum power. Vx is the root of
`2·a·Vx⁴ + ηp·Pav·Vx − 2·b = 0`, where the drag is `a·V² + b/V²`. With this
model, Vx often falls below the stall speed: the result is then the stall
speed, tagged `VxStall`, as the handbook says that Vx is frequently just
above the stall speed (*Pilot's Handbook of Aeronautical Knowledge*,
FAA-H-8083-25C, chapter 11).

An airplane of 2 550 lb, with a wing of 174 ft², a polar of `CD0` 0.0319 and
`Kind` 0.0610515, 180 hp at 0.8 of efficiency, at sea level. The source finds
1 257 ft/min at 58.8 kt (J. G. Leishman, *Climbing, Ceiling & Gliding*); the
best angle is at the stall speed:

```rpl
0.0319 0.0610515 2550_lb 174_ft^2 1.225 180_hp 0.8 1.6 ⓁClimbVxVy
@ Expecting FPAmax:13.66030 53597 °
```

The same airplane with half the power, in air of 0.9 kg/m³: Vx is now
above the stall speed of 60.7 kt:

```rpl
0.0319 0.0610515 2550_lb 174_ft^2 0.9 90_hp 0.8 1.6 ⓁClimbVxVy
DROP SWAP DROP
@ Expecting Vx:61.43051 8191 knot
```

See also: the Rate & Angle of Climb and Characteristic Speeds equations of
the Equation Library.

## TORoll

The takeoff ground roll, integrated step by step over the ground speed, with
the wind and a thrust that changes with the airspeed, as a propeller thrust
does.

Stack: `Wt` `Sw` `ρa` `CL` `CD` `μr` `Thr` `VLO` `HW` ▶ `GSLO` `tLO` `sLO`,
tagged: the mass, the wing area, the air density, the lift and drag
coefficients during the roll, the rolling friction, the thrust, the liftoff
airspeed and the headwind, negative for a tailwind. `Thr` is a thrust, or a
list `{ T0 TLO }` of the thrust at rest and at liftoff. Values with units are
converted; plain numbers are in SI units. The results are the ground speed
at liftoff, the duration and the length of the roll.

The acceleration is `(T − D − μr·(W − L))/m`; the distance and the time are
the integrals of `GS/a` and `1/a` over the ground speed, by Simpson's rule.
The Takeoff Ground Roll equation estimates the same distance with the forces
averaged at 70 % of the liftoff speed.

An airplane of 6 600 lb with 1 200 lbf of thrust, a wing of 160 ft², rolling
at a `CL` of 0.4 and a `CD` of 0.0388 on a dry runway, lifting off at
176.7 ft/s. The exact solution for a constant thrust is 3 289.2 ft and
36.1 s:

```rpl
6600_lb 160_ft^2 0.002378_slug/ft^3 0.4 0.0388 0.02 1200_lbf 176.702_ft/s 0_knot ⓁTORoll
@ Expecting sLO:3 289.14666 856 ft
```

The same takeoff against a headwind of 10 kt. The rule of the square of the
speed ratio gives 2 678 ft:

```rpl
6600_lb 160_ft^2 0.002378_slug/ft^3 0.4 0.0388 0.02 1200_lbf 176.702_ft/s 10_knot ⓁTORoll
@ Expecting sLO:2 707.12691 754 ft
```

With a propeller thrust falling from 1 300 lbf at rest to 1 000 lbf at
liftoff, at sea level:

```rpl
6600_lb 160_ft^2 1.225 0.4 0.0388 0.02 { 1300_lbf 1000_lbf } 104.7_knot 0 ⓁTORoll
@ Expecting sLO:3 725.45580 94 ft
```

See also: the Takeoff Ground Roll and Wind on Runway Distances equations of
the Equation Library.
