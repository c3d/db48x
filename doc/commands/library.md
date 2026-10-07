# Library Management

DB48x features a [library](#library) that can contain arbitary RPL code,
which is made readily available for use in your programs.

References to library functions are efficient both in terms of memory usage and
execution speed.  Typically, a reference to a library item takes 2 or 3 bytes,
and evaluating it is as fast as if it was on the stack, and faster than if
storedin a global variable.

Library items are also shared across DB48x states.

A key aspect of the execution speed for library items is that they are loaded
from disk only once, and then cached in compiled form in memory. This is how the
next uses of that library item can be as fast as if it was on the stack.

 Library items that are currently loaded in memory can be identified using
`Libs`. The `Attach` command can be used to load items ahead of time. The
`Detach` command can be used to evacuate library elements that are no longer
used.

When you modify the content of the library, you can use the following sequence
to make sure that the new version of thelibrary items are reloaded from disk:

```rpl
LIBS DUP DETACH ATTACH
```


## Attach

Load one or more library items from disk, ensuring that they are ready for use.
This command is not strictly necessary, since library items are loaded on
demand, but it can be used to "preload" library items for performance.

The libraries to attach can be identified by one of:
* A library index, e.g. `0`
* A library name, given as a text object or a symbol
* A library object
* A list or array of valid arguments to `attach`

For example, to preload the `Dedicace` library item, you can use one of:

```rpl
Libs Detach
'Dedicace' Attach
Libs
@ Expecting { Dedicace }
```

The first line detaches whatever was loaded before, so that `Libs` only shows
what this example attached.


## Detach

Unload one or more library items from disk, freeing the memory they used.

The libraries to attach can be identified by one of:
* A library index, e.g. `0`
* A library name, given as a text object or a symbol
* A library object
* A list or array of valid arguments to `detach`

For example, to unload the `Dedicace` and `KineticEnergy` library item, you can use one of:

```rpl
Libs Detach
{ Dedicace SiDensity "KineticEnergy" } Attach
{ Dedicace "KineticEnergy" } Detach
Libs
@ Expecting { SiDensity }
```

## Libs

Returns a list containing the currently attached libraries.

A typical sequence to reload the library items after changing the source files
on disk is:

```rpl
Libs Duplicate Detach Attach
```


## SecretsLibrary

The Secrets section of the library, a small example of library entries that
are plain values rather than programs.

### Dedicace

A dedication, in French, to all those who remember Maubert Électronique.

### LibraryHelp

A reminder that the library is defined by the file `config/library.csv`,
which can be edited to add your own entries.

## PhysicsLibrary

Physics programs and formulas.

### KineticEnergy

Plots the relativistic and the classical kinetic energy side by side, to
compare them as the speed approaches the speed of light.

### SiDensity

The intrinsic carrier density of silicon at a temperature `T`:
`8.35123E20 cm⁻³·exp(-7555.17 K/T)`.

`T` ▶ density

### Fanning

The Fanning friction factor of a pipe, from its relative roughness and the
Reynolds number: `16/Reynolds` in laminar flow (Reynolds up to 2100), an
explicit approximation of the Colebrook equation above.

`Roughness` `Reynolds` ▶ friction factor

## MathematicsLibrary

Mathematical programs, several of which double as benchmarks.

### CollatzBenchmark

Times `CollatzConjecture` from 989345275647. Tail recursion lets the
calculator run it without using memory for each step.

### CollatzConjecture

Follows the Collatz sequence from the number on the stack down to 1, showing
each value. Numbers with a long sequence include 1161, 2223 and 6171. It
checks tail recursion inside a test.

### CountPrimes

Counts the prime numbers below a given value, 1000 if the stack is empty.

### TriangleEquations

Solves a triangle with the multiple-equation solver, from a system of
equations such as Pythagoras' theorem and the law of sines.

### RombergPlot

Plots the function `x³·(sin(5x)+1)/(eˣ-1)`.

## GraphicsLibrary

Examples of graphics programs.

### CurvePlottingExamples

A tour of curve plotting: functions, polar curves and other plot types, with
various line widths.

### WalkMan

A small character walking across the screen, from page 2-39 of the HP 50G
Advanced Reference. It checks compatibility with the HP `GROB` format.

### DrawingLines

Draws a moving pattern of lines.

### DrawingText

Displays lines of text in varying shades of gray.

### DrawingShapes

Draws a moving pattern of shapes, filled with changing patterns.

### DrawingPatterns

Draws rounded rectangles in 256 levels of gray.

### RandomPlot

Draws one random point in each column of the screen.

### RandomXYPlot

Draws 25 000 random points on the screen.

## SoundLibrary

Examples of sound programs.

### Beeps

Plays a rising sequence of 51 notes, each a semitone above the previous one.

## PerformanceLibrary

Benchmarks, most of them timed with `TEval`.

### NQueens

Solves the eight queens problem, a classic calculator benchmark, with a User
RPL program taken from the HP Museum.

### SumTestWithFunction

Sums `∛(exp(sin(atan(x))))` for `x` from 1 to 1000 with the `Σ` function,
and times it. See also `SumTestWithLoop`.

### SumTestWithLoop

The same sum as `SumTestWithFunction`, computed with a `FOR` loop, and timed.

### Recursion

Calls itself endlessly, showing the depth reached and the free memory, which
does not decrease: tail recursion uses no memory. Stop it with `EXIT`.

### UnitsBenchmark

Times 26 conversions of speeds from `m/s` to `km/yr`.

### InteractiveMenu

An example of interactive program, in the Examples section: it stops in the
solver menu for `C=√(A²+B²)`; once `A`, `B` and `C` are solved, `=` resumes
it, and it displays the results.

## ConfigsLibrary

The configuration files of the calculator, which define its catalogues. Each
entry shows the content of one file:

* `Cst`: `config/constants.csv`, the constants;
* `Eqns`: `config/equations.csv`, the equations;
* `Lib`: `config/library.csv`, the library;
* `Units`: `config/units.csv`, the units.

Edit these files to add your own entries.

### Eqns

The content of `config/equations.csv`, which defines the equations added to
the Equation Library. See [ConfigsLibrary](#configslibrary).

### Lib

The content of `config/library.csv`, which defines the Function Library. See
[ConfigsLibrary](#configslibrary).
