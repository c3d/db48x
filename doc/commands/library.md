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

## DemoLibrary

Demonstration programs, in eleven submenus:
[Mathematics](#mathematicslibrary), [Physics](#physicslibrary),
[Graphics](#graphicslibrary), [Sound](#soundlibrary),
[Performance](#performancelibrary), [Examples](#exampleslibrary),
[Anagram](#anagramlibrary), [Probability](#probabilitylibrary),
[StatTests](#stattestslibrary), [Uncertainty](#uncertaintylibrary) and
[Astronomy](#astronomylibrary).

Most of them were stored in the Demo state file. They are now library
entries, which that state file refers to, so that they can be run from the
library without loading the state.

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
Reynolds number: `16/Reynolds` in laminar flow (Reynolds up to 2100), and
above, Serghides' explicit approximation of the Colebrook equation, which
gives the Darcy factor, divided by 4.

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

### EquationSolving

Stores in `EQ` a system of ten equations for a triangle: Pythagoras' theorem
for the sides `AA`, `BB` and `CC` of a right triangle, and for any triangle
the laws of sines and cosines, the sum of the angles and Heron's formula for
the area `A`. It sets `AA` to 10 m, `BB` to 85 ft and `CC` to 1 cm, units
mixed on purpose, and opens the solver menu: solving for `CC` gives the
hypotenuse, 27.77 m.

### MultiSolver

Solves with the multiple-equation solver the system `sin(X)=0.5·cos(Y)` and
`sin(X)=0.3·cos(X)` for `X` and `Y`, from 1 and 1, after opening the solver
menu. In degrees, `X` is 16.70° and `Y` 54.92°.

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

### XYPlots

Draws and times three plots of a function of `x` and `y` with `Truth`: the
region where an inequality holds, the shades of gray of a real function, and
a complex function. Each is drawn with 32, 128 and 64 for `XYPlotBins`, its
time shown for 5 seconds; the nine times are left in a list.

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

### MatrixMultiplyBenchmark

Times the product of two random N×N matrices, N being 20 when the stack is
empty. Each matrix is made with `RanM`, then the sine of its elements is
taken; the times are shown on the screen, and the garbage collector runs
between the steps. Its statistics before and after the product are stored in
`PreGC` and `PostGC`.

### InteractiveMenu

An example of interactive program, in the Examples section: it stops in the
solver menu for `C=√(A²+B²)`; once `A`, `B` and `C` are solved, `=` resumes
it, and it displays the results.

### PathHeader

Replaces the header of the screen with the time, the current directory, the
date and the free memory, refreshed every 10 seconds.

### BatteryLevelHeader

Shows the battery voltage in the header, and whether the battery is low,
refreshed every 10 seconds.

### RuntimeStatsHeader

Shows in the header the statistics of the main loop given by
`RuntimeStatistics`: the number of runs, and the time spent running and
sleeping, refreshed every 10 seconds.

### BatteryStatsHeader

Records the time and the battery voltage every 10 seconds, and shows in the
header the number of records, the time, the voltage and whether the battery
is low: a discharge curve. The records go to `ΣData`, which it first sets to
the file `BATTERY.CSV`, so that they survive a crash at low battery.

### IfTETest

The two forms of `IFTE`. With programs, it tells whether the number on the
stack is 1, less or more: "One", "Less" or "More". With expressions,
`'1=1' '1+1' '1*3' IFTE` gives 2.

### IfThenElseEval

An `IF` whose test is an expression on a local variable, `'x<1'`: it gives
"less" for a number below 1, and "more" otherwise.

### CaseTest

Gives "Alpha", "Beta", "Gamma" or "Delta" for the texts "A", "B", "C" or
"D", with `CASE`; the last one uses the `WHEN` form. The text stays below
the answer, and any other text is left alone.

### Prompting

Stops with `PROMPT` and its message. Enter a value and press the Run key: the
program adds 1 to it and tags the result "Next value".

### Inputting

The same with `INPUT`, which opens the command line under the message, with
the default text "What?".

## AnagramLibrary

A recursive program in three parts, which shows all the anagrams of a word.

### ANAG

Shows on the screen, one after the other, all the anagrams of the text on the
stack, made by permuting its characters: six for "ABC", 24 for "ABCD". It
keeps its counters in the global variables `B` and `C`, which it purges at the
end.

### PRANAG

The recursive part of `ANAG`: it permutes the characters on the stack, and
shows each arrangement.

### PRDEPTH

The number of characters on the stack above the depth that `ANAG` recorded
in `C`.

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
