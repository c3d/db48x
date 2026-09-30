# Aeronautics functions

## AeronauticsLibrary

The Aeronautics section of the library holds programs for the calculations of
aircraft operation that need lists, loops or comparisons, and do not fit the
equation solver. Its equations are in the Aeronautics section of the Equation
Library.

* [Flight Management](#flight-managementlibrary): weight and balance.

## Flight ManagementLibrary

Weight and balance, from the load sheet to the envelope:

* [WBLoad](#wbload): the total weight, moment and center of gravity of a
  loading, station by station;
* [WBEnv](#wbenv): whether a loading lies within the center of gravity
  envelope, and the forward and aft limits at its weight.

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
