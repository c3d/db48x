# Release notes

## Release 0.8.1 "Sale" - Multi-equation solver

### Features

* Multiple-equation solver (HP's MES), solves for variables one at a time
* The `|` operator (aka `where`) applies to library equations
* The `|` operator respects variables with units in expressions
* simulator: Add support for copy and paste (to/from simulator)
* Add support for cylindrical and spherical 2D and 3D vectors
* Positional graphic combination operations (e.g. `GraphicRatio`)
* Switch to Greek or Cyrillic keyboard maps based on character menu
* Add EDIT menu commands to transient alpha (e.g. ▶F3 is Word→)
* Index the  help file for performance (about 5x faster on DM42)
* Accept verbatim code and RPL code snippets in the help file
* Parse and show help topics taking all aliases into account
* Convert lists and equations to RPL programs with `→Program`


### Bug fixes

* Show tagged values for vectors and matrices
* Update the target global variable after running `root`
* Show all variables in the `SolvingMenu` (with a settings to control it)
* Unit-related commands accept tagged objects and expression-enclosed units
* Add angular units (e.g. radians) in angular equations
* Correct unit for `V` in `Cantilever Shear` (was `n` instead of `N`)
* `Purge` now correctly restores UI patterns settings
* Add missing font parameter to `→Grob` (HP calculator compatibility)
* Include equations and xlibs to list of symbolic objects
* Fix precision loss for `atan`, `acos` and `asin` for some values
* Add angles for `atan2` when using hardware-accelerated floating-point
* Remove spaces and separators in the names of library equations
* keyboard: Replace ASN with ->NUM
* Preserve trailing decimal separator in `FIX` mode with `NoTrailingDecimal`
* Fix backspace and delete operation around number separators
* Fix the definition of `cosh` for complex values (was computing `sinh`)
* Accept uppercase and lowercase `.48s` when saving/restoring state
* Accept both `Ω` or `Ω` as spellings for Ohm (different Unicode)
* Only update the state file when disk operations are successful


### Improvements

* Add `arcsin`, `arccos` and `arctan` spellings
* solver: Reorganize solver code
* documentation: Udpate equation documentation with examples
* Save UI patterns as hexadecimal numbers (also in `Modes` command)
* Use only tabs in library.csv
* Keep cursor at end of buffer when moving through history with word right
* Clear selection when BSP is used with a non-empty selection
* Ensure `debug_printf` always refreshes the screen
* Switch to binary search for command parsing (~100x faster)
* Rename some statistical functions for consistency
* Fix the list of authors in the online help
* doc: Update performance data


## Release 0.8.0 "Gabriel" - Symbolic operations, equations and library

This release adds symbolic integration, differentiation and equation
solving, delivers an extensive equation library, and improves the
library feature to enable large library objects to be defined in
separate files and optimize execution speed for library items.
It also improves the rendering of complex equations and fixes a number
of crashes or user-interface problems.

### Features

* Symbolic integration, `Primitive` command, `'∫X(sin(X))'` syntax
  `'∫X(sin(2*X+3)-X)'` evaluates as `'-cos(2*X+3)/2-x²/2'`
* Symbolic differentiation, `Derivative` command, `∂X(sin(X))` syntax
  `'∂X(sin(2*X+3)-X)'` evaluates as `'2·cos(2*X+3)-1'`
* Symbolic equation resolution, `ISOL` (`Isolate`) command
  '`sin(2*X+3)=A' 'X' ISOL` gives `'X=(sin⁻¹ A+2*i1*π-3)/2'`
* Apply arithmetic and functions on both sides of equations like `A=B`
  `'A=B' sin 1 +` results in `'sin A+1=sin B+1'`
* Add possibility to define library objects in separate files
  The various demos from previous releases are now also in the library
* commands: Add `→Prog` command to convert expressions to RPL programs
  `'sin(2*X-Y)' →Prog` gives `« 2 X × Y - sin »`
* Accelerate evaluation of library entries
  Invoking a library object is as fast as if it was on the stack.
  Notice that this is faster than global objects that require lookup.
* Add ``Obj→` support for user-defined function calls
  `'F(A;B;C)' Obj→ DROP Obj→ ` gives `[F A B C]`.
  The first call to `Obj→` expands the expression, leaving an isolated
  function call on the stack, which the second `Obj→` expands.
* Automatically convert polynomial objects to expressions when needed
* Many improvements to the equation library contributed by Jean Wilson
* Document the equation library variables in the help file
* Add classical/relativistic Kinetic Energy comparison demo
  This was suggested by Jean Wilson as an illustration
* parser: Parse and convert HP syntax for `|`
  Typing `'A(X;Y)|(X=1;Y=2)'` converts to `'A(X;Y)|X=1|Y=2'`

### Bug fixes

* Improve graphical rendering of some complex expressions
* Render `sqrt` as `√` and not `squareroot` in HP compatibility mode
* Do not draw parentheses around `|x|` when rendering `abs`
* Redefine `rpm` as `turn/min` (unit consistency of rotational speed)
* Avoid crash in `for` loop if termination condition fails to evaluate
* Fix rendering and saving of directories
* Do not leave incorrect stack content when running `Obj→` on text
* Make size consistent in vectors containing tagged objects
* Accept tagged objects as input in algebraic functions
* Avoid parsing error for `1,234` when `.` is the decimal separator
* Debug output related to expressions is no longer labelled "equations"
* Fix unit error in Drift Speed & Current Density
* Cut the size of the build version ID to avoid DMCP buffer overflows
* Avoid possible corruption when an error occurs in a program
* Improve error messages for file errors, e.g. when loading a state
* Move cursor at correct position after numbers when entering units
* Reject multiple consecutive units, e.g. `1_m_s`
* Fix issue where we insert `=` incorrectly on the command line
* Address simulator warning about potential buffer overflow

### Improvements

* tests: Add tests for the newly added library entries
* primitive: Add test suite for symbolic integration
* tests: Add test suite for symbolic differentiation
* Factor out code to expand objects on the stack
* `make install` now Installs from distribution files
* demo: Reorganize demo files as directories
* tests: Add test for `Obj→` on various object types
* tests: Factor out code checking state of the complete stack
* tests: Avoid emitting something that looks like compilation errors
* Update demo file
* Improve definition of atomic mass unit, add `Da` (Dalton) alias
* expressions: Optimize the way to split an equation
* tests: Adjust tests after changes in `SymbolicMenu`



## Release 0.7.18 "Who" - Subst, Where, Apply

This release implements the `Apply`, `Subst` and `|` (`Where`) commands.

### Features

* ui: Allow Shift-SPC to enter `=` in an expression
* Implement `subst` and `where`
* Implement the `Apply` command
* Implement the `EQNLIB` command
* Add `CONSTANTS` as a way to select mathematical constants menu
* Add `CONLIB` as an alias for `ConstantsMenu`.

### Bug fixes

* solver: Adjust detection of "epsilon" for large values
* ttf2font: Flip x coordinates for dense fonts

### Improvements

* doc: List commands that will never be implemented


## Release 0.7.17 "Open" - DoSubs, DoList, entering units

This release adds `DoSubs`, `DoList`, `NSub` and `EndSub` commands, and fixes
related issues that were found with the associated examples in teh HP50G
Advanced Reference Manual.


### Features

* lists: Implement `DoList`, `DoSubs`, `NSub` and `EndSub`
* units: Add usual "big" units to `Computing` units menu (e.g. KB, MB)
* graphics: Add `Freeze` command
* ui: Make it easier to enter units
* expressions: Accept algebraic forms for `integrate` and `root`
* expressions: Graphical rendering of integrals

### Bug fixes

* case: Preserve the code for the default case (which was lost before)
* parser: Parse arg-less alegbraic commmands such as `NSub`
* equations: Fix syntax errors in built-in equations, e.g. missing parentheses
* equations: Replace variables that match DB48X command names, e.g. `Re`
* equations: Replace imperial units with SI units, e.g. `in` with `cm`
* units: Reject user-defined functions in unit expressions
* constants: Fix definition for the `qε0` constant

### Improvements

* Update help about equations with some additional material
* tests: Run through all the builtin equations
* ui: Do not insert an extra space before parentheses in equation
* simulator: Add macOS icons for DB48X and DB50X
* font: Record saved font with `v` update


## Release 0.7.16 "Clean" - New commands

The focus of this release is on new commands, notably for matrix and vector
operations. This release also ships with a DM48X-specific keymap, which fixes a
problem with teh file selector on the DM32. The equation library has been extended with numerous equations, but they are not fully validated yet.

### Features

* `con`, `idn` and `ranm` matrix generation commands
* `Array→` and `→Array` commands to convert array to/from stack
* `dot` product and `cross` product commands for vectors
* `DispXY` styled text rendering command
* `DupDup` command duplicating top item twice
* Add a large number of equations to equation library

### Bug fixes

* Fix `decimal::to_bignum` for small magnitudes
* Return angle unit for `atan2` and `arg` commands
* The `for` loop on lists no longer ends leaving the debugger active
* Interactive stack `DropN` command returns to level 1
* Fix file selector's "New file" on DM32 so that ENTER terminates it
* A few fixes in equations in the equation library


### Improvements

* Improve graphical rendering of expressions such as multiplication operators
* doc: Update status
* font: Fix `v` glyph vertical placement, add dot and cross glyphs
* ttf2font: Add additional verbose info about source data
* Add `CONSTANTS` as an alias for `ConstantsMenu`
* Replace documentation references to `EEX` with `×10ⁿ`
* Optimize parsing of real numbers in parentheses


## Release 0.7.15 "Teaching" - Bug fixes notably on iOS

This release mostly focuses on issues exposed by the iOS releases.
It's a bug-fixes only release.

### Bug fixes

* ios: Fix a screen refresh bug delaying the display of computation results
* ios: Increase user-accesssible memory for DB50X to match the DM32
* files: Avoid opening two files when a loaded file contains units/constants
* units: Restrict unit expressions further, e.g. forbid `1_km^s`
* build: Do not add unwanted macOS-specific files in release tar files
* build: Remove irrelevant help file from releae tar file
* constants: Fix numerical value for G constant
* tests: Rename `M` demo helper to `D`, since we use `M` in symbolic tests
* makefile: Add `mv` echo to targets doing image comparison
* units: Use pi constant in definition of radians unit
* parser: Detect syntax error on `(inv(x))` in non-expression mode
* units: Avoid infinite loop for bad unit exponents
* files: Avoid crash in `file_closer` if file does not exist
* tests: Add DMS/HMS operations to math demo
* tests: Add keyboard shortcuts to launch the demos


## Release 0.7.14 "Kids" - Equation-related bug fixes

This release fixes a number of issues that were discovered primarily through the
Columns and Beams equations.


### Features

* trigonometrics: Add conversion from non-standard angles, so that `cos(1_turn)`
  gives the correct result.
* debug: Debug on error with `DebugOnError` and `KillOnError` settings. This
  makes it easier to debug an RPL program, by making it possible to single-step
  around the instruction that generated the error.
* tests: Add three 30 second demo of DB48X features. These are to generate
  marketing videos for the iPhone version on the Apple store (to be done).


### Bug fixes:

* ui: Keep a GC pointer in `draw_object` to avoid a memory crash
* equations: Add missing units in some equations, e.g. `I` and `A` in second
  equation of Coilumns and Beams
* equations: Add explicit `radian` unit in Eccentric Columns `cos`, which
  ensures we get the correct result from the HP50G manual even when in Degrees
  mode.
* units: Correctly factor out non-integral powers, so that we can compute
  `1/sqrt(epsilon_0*mu_0)` and get the correct result.
* catalog: Display commands that begin with selection first, so that `FORE`
  shows `Foreground` before `AlphaForeground`.
* stats: When computing a sum, evaluate the expression on all terms. The result
  for `Variance` with single variables was wrong because the first term was
  computed incorrectly due to a misguided optimization.
* graphics: Error out in `RGBPattern` for negative input. The negative values
  were generating an error, but it was not reported, so the next command was
  likely to report it.
* help: Render shift keys correctly in the color version. The bitmap was
  interpreted as containing color data. Colorize it instead.

### Improvement

* ui: Select orange as background color during search. The previous setting of
  showing seardch using white foreground on a white background was probably
  pushing the notion of "blind search" a bit too far.


## Release 0.7.13 "Murderers" - Solver improvements

This releases focuses on improvements to the solver, with the completion of the
Columns and Beams section from the HP50G equation library.

### Features

* equations: Add remaining equations from "Columns and Beams"
* loops: For loops on lists and arrays
* menus: Add `R→D`, `D→R`, `→Polar` and `→Rectangular`
* menus: Automatically select tools menu for library equations
* units: Allow `ubase` to work on expressions
* units: Graphic rendering of units


### Bug fixes

* 48calc.org: Fix mouse click position
* `#ABC #DEF -` now correctly produces a based number with >64-bit wordsize
* conditionals: Parse and evaluate `IFTE` correctly
* equations: Fix Elastic Buckling
* files: Avoid crash in `file_closer` if file was not open
* istack: Enable Swap feature on top two levels
* menu: Update SolvingMenu when updating VariablesMenu
* parser: Deal with negation more "normally", parse `-a²` correctly
* solver: Compute units correctly for inputs
* solver: Do not change unit when storing in a solver variable
* solver: Emit correct error message in `EvalEq` if missing variables
* solver: Preserve errors reported by underlying function
* units: Do not evaluate/render units with names
* units: No longer read variables `m` and `s` while processing `1_m/s`
* units: Simplify units that convert to real numbers


### Improvements

* Update .gitignore
* build: Add BMP files to the distribution
* complex: Optimize exit conditions for parentheses
* solver: Add test for equation library
* solver: Add tests for solving with units
* solver: Improve behaviour of `EvalEq`
* solver: Keep units and constants as-is in equation
* solver: Make precision relative to equation sides magnitude
* stack: Show vectors vertically by default
* ui: Do not persistently hide stack with current equation
* units: Keep power integral, i.e. avoid getting `1_m^2.0/s`
* units: Strip tags from unit conversion functions
* units: The `EvalEq` command should not evaluate dates
* wasm: Add logos to the repository



## Release 0.7.12 "Multiply" - Multiple equations

This release focuses on the equation solver, notably in interaction with the
equation library. The goal is to get one step closer to the equation library in
the HP50G, including the ability to have graphical illustration and multiple
equations.

### Features

* doc: Add documentation for the equation library
* doc: Add images from HP50G equation library (to be used later)
* doc: Some help for the elastic buckling equations.
* equations: Accept lists of equations in library
* help: Add documentation for the various constants
* help: Do not exit help when opening URLs or missing topics
* lists: Add a setting to evaluate list as programs
* menu: Add menu entries for solver imprecision and iterations
* solver: Add support for multiple equation solving
* ui: Add settings to hide/show empty menus and clear menu on EXIT
* units: Add flow units (per user request) in Fluid section of units


### Bug fixes

* blitter: Adjust the right margin
* blitter: Base horizontal adjust on scanline, not width
* characters: Display correct content for built-in menus
* constants: Close current file while parsing values
* constants: Update some outdated values
* doc: Show help for equations and constants
* help: Do not try to load PNG images
* rewrites: Avoid contradictory rules when reordering constants
* solver: Work correctly with units (when in variables but not equation)

### Improvements

* build: Shorten the size of the version abbrev
* config: Use config files only for user configuration
* doc: Record performance data about unit conversion
* help: Avoid slowing down when scrolling through pages of help
* help: Record position for history even without a `\n`
* simulator: Add tweak to show RPL object details
* tests: Increase wait time for tests with blinking cursor
* units: Do not simplify while in unit mode


## Release 0.7.11 "Rest" - Refine interactive stack, graphics in help

This release is a refinement minor release. The primary focus is the interactive
stack, which now lets you edit items, sort either according to memory
representation or by value, display information about objects, and jump directly
to a given stack level using digits.

The simple random number generator implemented in 0.7.10 was replaced with an
additive congruential random number generator (ACORN), which can be configured
in number of bits and number of iterations. A side effect is that there is now
regression testing for single-variable statistics.

The history feature was also improved by automatically enabling the `EditMenu`
when selecting history, and then having the (unshifted) word left and word right
commands automatically cycle through history if used at beginning or end of the
editing buffer.

### Features

* help: Add ability to display BMP images in help files
* images: Convert help images to BMP
* ui: Add `Edit` feature to interactive stack
* ui: Add history menu entries to `EditMenu`
* ui: Accept `UNDO` while in interactive stack mode
* ui: Have word previous/next cycle through history
* ui: Accept digits to select stack level in interactive stack
* random: ACORN random number generator

### Bug fixes

* editor: Fix spacing after number followed by `-` sign
* ui: Do not set the `editing` field from interactive stack
* ui: Replace interactive stack "Edit" with "Echo"
* ui: Block user input while using interactive stack
* ui: Do not draw menu markers when displaying interactive stack
* runtime: Avoid crash running above allocated memory in `move_globals`


### Improvements

* ui: Reorganize code handling interactive stack keys
* help: Adjust help area to new height for menus


## Release 0.7.10 "Hospitality" - Interactive stack

This release primarily adds the "interactive stack" feature of HP calculators.

### Features

* stack: Interactive stack and associated menu
* functions: Very basic random number generator
* demo: Add `CountPrimes` and `RandomPlot` examples

### Bug fixes

* menus: Do not execute if-then-else and similar
* expressions: Avoid error testing for zero/one in power operator
* condidionals: Avoid infinite loop evaluating condition
* menu: Insert `iferr-then-else` correctly from menu
* constants: Skip menu entries

### Improvements

* menus: Reorganize stack menu to put `Dup` and `Drop` on first page
* menu: Add error functions to `Debug` menu


## Release 0.7.9 "Just Asleep" - Online WASM simulator

This release includes a number of bug fixes, facilitates build from scratch by
third-parties, and delivers a WASM-based simulator that runs on the
https://48calc.org web site.

## Features

* menu: Add `Purge` to the `ClearThingsMenu`
* purge: Add support for lists
* units: Accept units for `sqrt`, `cbrt` and `xroot`
* wasm: Add variant for `48calc.org`
* wasm: Implement WASM support

## Bug fixes

* complex: Give `Syntax error` for phasor without an angle
* dms/hms: Parse empty numbers in DMS entry
* dms/hms: Protect display against bad input
* editor: Make sure we reposition after shift-up and shift-down
* parser: Parse degrees as an angle unit in phasors
* simulator: Avoid high CPU usage when a timer is active
* simulator: Use the correct color for firmware text
* ui: Compute the correct refresh rate even when nothing is displayed
* ui: Create a non-empty dirty rectangle when clearing annunciators
* ui: Do not refresh beyond LCD size
* ui: Ensure we redraw the shift region to clear busy
* units: Disable simplification during unit definition evaluation

## Improvements

* build: Fix the clean build
* object: Remove defaults for `as_uint32` and similar
* parser: Cache parser::length when possible
* parser: Merge the `parser::end` and `parser::length` fields
* parser: Optimize integer parsing early exit
* simulator: Avoid `Cancel` in file save dialog
* simulator: Use `https` as a protocol for git submodules
* tests: Add support for degrees sign
* tests: Add test for polar angle conversion.
* units: Add `factoring` variable to limit simplifications
* wasm: Add link to deployed WASM simulator in the README
* wasm: Add script to add COOP / COEP headers
* wasm: Run the RPL thread in a separate thread


## Release 0.7.8 "Mustard" - Constants in equations

This release keeps marching towards full support for an equation library.
The primary focus was support for constants in equations, including constants
with units like the speed of light, as well as fixing various user-reported
issues.


## Features

* cycle: Apply to value of tagged object
* equations: Add units to built-in equations
* equations: Adjust menu label to remove units
* equations: Strip units but not constants when rendering
* expressions: Treat `2X` as an implicit product
* solver: Strip units from constants
* units: Unit prefix should preserve the tag

## Bug fixes

* clearlcd: Fix erasure of screen when not in graphics mode
* constants: Count number of menu entries correctly for builtin menus
* decimal: Accept leading `.` or `,` for decimal values
* expressions: Parse negation after parentheses
* sum: Do not accept a non-name in a sum or product
* type: Return a type value for decimals
* ui: Fix insertion/editing of infix with alphabetic names
* units: Do not read units from variable if they exist
* xroot: Reverse `xroot` arguments in algebraic parsing

## Improvements

* editor: Shuft-up returns to beginning of line if on first row
* errors: Add `error_save` class, use it in `constant::value`
* expressions: Use the `SaveAutoSimplify` class in `simplify_products`
* units: Split `Eng` to `Elec` and `Visc`


## Release 0.7.7 "Forgiving" - Units in equations

This release keeps marching towards full support for an equation library.
The primary focus was support for units in equations.


### New features

* solver: Accept equations in solver menu
* solver: Add shortcut to solve an equation from the library
* solver: Display the current equation above the stack
* solver: Solve expressions containing units
* solver: Add units for solver variables when entering them
* equations: Add option to list variables with units
* programs: Enforce numerical values for solver / plotter
* constants: Implement programmatic lookup
* fonts: Add support for fixed-width digits
* keyboard: Interpret `ASN` as `AsNumber` (convert to decimal)
* complex: Allow insertion of angle while entering phasors
* complex: Implement auto-complex promotion
* graph: Render abs(X) with bars (e.g. |X|)
* functions: Automatic simplification of expressions


### Bug fixes

* arithmetic: Avoid null-dereference in complex operations
* help: Close help file if topic not found
* solver: Do not store tag for tagged values
* graph: Gracefully fallback if fraction integral part does not render
* units: Avoid null-dereference if unit simplification fails
* units: Count parentheses while parsing units
* put: Fix null-dereference checking the index
* fractions: Do not render two negative signs in graphical mode


### Improvements

* cycle: Update behaviour for several data types
* menu: Replace `abs` with `|z|` in complex menu
* ui: Micro-optimization to avoid reading object type twice
* parser: Accelerate and improve object parsing
* recorder: Add recorder entries for evaluation
* build: Remove any leftover references to Intel decimal library
* tests: Add `▶` entry in tests
* tests: Increase default wait time to 1000ms
* tests: do not error out if `teval` takes less than 100ms
* equations: Rename `PerfectGas` equation to `IdealGas`
* menus: Adjust size of menus to make descenders visible
* solver: Replace `SolverPrecision` with `SolverImprecision`


## Release 0.7.6 "United" - Equation Solving Menu

This release is primarily about implementing a dynamic solving menu that makes
it easier to solve equations with multiple variables. This is roughly equivalent
to the "Solve Equation" application in the HP50G.

### New features

* solver: Implement solving menu
* tag: Graphic rendering for tagged objects
* lists: Implement `lname` and `xvars` commands
* arithmetic: Implement `div2` command
* variables: Implement `vars` and `tvars` commands

### Bug fixes

* render: Avoid errors while rendering / graphing
* expressions: Avoid consuming stack levels for invalid expressions
* symbols: Avoid ignoring the list of bad characters in symbols
* command: Fix for `x!` parsing
* stack: Correctly set clipping for stack index
* doc: Fix typo in release note

### Improvements

* menus: Add `PixOn`, `PixOff`, `Pix?` and `PixC?` to `GraphicsMenu`


## Release 0.7.5 "Perfect Joy" - Polynomials and symbolic rewrites

This release contains a lot of groundwork in preparation for future work on
symbolic expressions, symbolic solving and symbolic integration, as well as to
improve compatibility with HP calculators. In particular, `rewrite` has been
replaced with the HP equivalents, `↑Match` and `↓Match`, allowing top-down and
bottom-up replacement, as well as support for conditions. Also, these commands
return the number of replacements performed instead of just `0` or `1`.

THe other major user-visible new feature is the addition of a polynomials data
type, which does not exist on HP calculators, exposing polynomial features in a
way that is more consistent with the spirit of RPL. For example, Euclidean
division of polynomials can be achieved using the regular `/` operation on
polynomials instead of requiring a dedicated `DIV2` command.


### New features

* Support for polynomials as a data type, including Euclidean division
* Arithmetic operations on polynomials, including `sq` and `cubed`
* Conversion functions `→Poly` and `Poly→` to convert to and from polynomials
* Optional case-sensitive symbol matching
* Algebra configuration directory (like `CASDIR` on HP calculators)
* rewrites: Replace `rewrite` command with HP-compatible `↑Match` and `↓Match`
* rewrites: Add `ExplicitWildcards` option to match HP syntax (`&A`)
* rewrites: Add rules to expand powers
* rewrites: Add support for conditions when matching patterns
* rewrites: Add support for step-by-step rewrites
* rewrites: Add support for bottom-up rewrites
* flags: `Purge` now resets system flags to default value


### Bug fixes

* editor: Fix unresponsive keys after using `EXIT` key while searching
* complex: Avoid emitting syntax errors while parsing
* rewrites: Avoid potential garbage collection corruption problem
* rewrites: Disable auto-simplification during rewrites
* rewrites: Factor out rewrite loop
* expressions: Encode expressions with type ID >= 128 correctly
* arithmetic: Add space around `mod` and `rem` in rendering
* graph: Do not add parentheses for `X*(Y/Z)`
* functions: Make percentage operations binary functions
* functions: Turn `min` and `max` into algebraic functions
* cycle: For expressions, cycle graphic/text rendering correctly
* menus: Replace `EquationsMenu` with `ExpressionMenu` in other menus
* ui: Insert space when inserting array inside function


### Improvements

* menus: Updates to `PolynomialMenu` to enter polynomials and for conversions
* menus: Add product and sum to symbolic and algebra menus
* menus: Make `ToolsMenu` select `SymbolicMenu` for symbols
* expressions: Reorganize the code for rewrites
* rewrites: Add recorders for rewrites that are actually done
* tests: Some adjustments on color images
* rewrites: Convert algebraics into expression as needed
* complex: Parse `3i` and `i3` in addition to `i`
* tests: Add support for more characters
* simulator: Separator color and dm32 support
* graph: Add space when rendering simple function
* keyboard: Updated SVG files with latest menu labeling changes



## Release 0.7.4 "Flesh" - Polishing and refinements

This release is mostly about polishing various aspects of the
implementation to make it more convenient and more efficient. It also
adds user-defined functions parsing and evaluation, pixel-manipulation
commands, bit operations on binary numbers, memory operations like
`sto+` or `incr`, loading and saving BMP files, color support in the
simulator, and more.


### New features

* editor: Implement configurable word wrapping
* expressions: Add code for n-ary functions like `sum`
* expressions: Parse user-function calls like `F(1;2;3;4)`
* expressions: Ensure funcall objects are evaluated immediately
* functions: Add `sum` and `product` functions
* functions: Add combinations and permutations
* functions: Implement number rounding operations (`rnd` and `trnc`)
* graph: Add graphical rendering for `cbrt` (cube root) and `xroot`
* graph: Graphical rendering of combinations, permutations
* graph: Graphical rendering of sum and product
* graphics: Add `pixon`, `pixoff` and `pix?` commands
* graphics: Store and recall BMP files with `sto` and `rcl`
* graphics: `ToGrob` command converting object to graphic
* logical: Add `SetBit`, `ClearBit` and `FlipBit` commands
* memory: Implement the `Clone` (`NewOb`) function
* menus: Add `log2` and `exp2` to `ExpLogMenu`
* menus: Place `ListMenu` as a keyboard-accessible menu
* parsing: Parse n-ary functions
* program: Add vertical program rendering mode
* stack: Display error message emitted during stack rendering
* ui: Add colorization parameters for the user interface
* ui: Add some colorization
* variables: Add `Sto+`, `Rcl+` and other variable arithmetic
* variables: Implement `Increment` and `Decrement`


### Bug fixes

* decimal: Fix precision when computing gamma/lgamma
* decimal: Fix rounding bug when rounding increases exponent
* doc: Add missing dependencies on Fedora (submitted by @vkadlcik)
* files: Do not error when opening constants/equation/library files
* files: Open only one configuration file at a time
* graphics: Fix bug drawing a line of width 0
* graphics: Use foreground color for parenthese and ratio
* lists: Separate list sum/product from regular sum/product
* renderer: Make sure `printf` respects target buffer size
* simulator: process double-clicks correctly (submitted by @kjellc)
* tests: Change the height of ignored header


### Improvements

* command: Factor out arity for all commands
* constants: Report parse error location for invalid constants
* demo: Add HP-48 style slow walk to `Walk` demo
* demo: Modernize the code a little
* demo: Modify performance benchmarks to use `TEval`
* demo: Replace imaginary unit constant
* doc: Remove reference to Intel Decimal Library
* files: Convert all file names to lowercase (Linux support)
* functions: Make it possible to interrupt a running sum/product
* graph: Improve rendering of `exp`, `exp2`, `exp10`
* graphics: Separate color conversion step
* help: Do not display command name while editing
* ids: Make room for a few additional 1-byte commands
* locals: Document the absence of compiled local variables
* makefile: Add configuration files to the release `.tgz` file
* parsing: Make the error message for sub-expressions more local
* readme: Remove reference to DM42 from top-level readme
* simulator: Avoid crash rendering %t in recorder
* simulator: Convert simulator code to support color
* simulator: Replicate open files limitations
* simulator: Separate db50x and db48x builds
* tests: Add colorized images to testing
* tests: Avoid occasional errors on some long-running tests
* ui: Define cursor position in `SelfInsert` with `\t`
* ui: Ignore EXIT, BSP and ENTER keys when clearing error


## Release 0.7.3 "Perfume" - Mostly bug fixes

This release is mostly about bug fixes, improving the build on simulator, and accelerating the test suite while keeping it stable.

### New features

* simulator: Add F8 key to save state in the simulator
* errors: Add a beep when an error is shown
* linux: Add `-s` option for screen scaling (when Qt gets scaling wrong)
* equations: Get library equation value for plotter, solver and integrator
* characters: Add constant, equation and lib markers to RPL menu
* characters: Add music-related characters and character menu
* commands: Add missing stack commands (nip, pick3, ndupn, unrot, unpick)


### Bug fixes

* variables: Clone purged objects on stack after `Purge`
* units: unit * symbolic is preserved as is (e.g. `'A'_m`)
* compare: Enforce the `NumericalResults` flag for comparisons
* constants: Fix parsing of constants, equations and library items
* sto: Repair `file exists` error storing to a source file
* simulator: Do not try to create directory if it exists
* Report file errors, e.g. permissions or I/O errors
* equations: Mark `c` and `R` as constants in equations
* simulator: Double clicks are now considered as virtual keyboard touches
* ui: Emit only one beep, not two, for a syntax error on the command line
* ui: Avoid rare null-dereference crash when menu label is not set
* complex: Report a syntax error if the second half is empty
* utf8: Do not accept constant/equation/library codepoint in names
* units: Multiplying by `1_m` is OK even with algebraic
* tests: Avoid case where CLEAR does not clear errors
* audio: Improve audio reliability on the simulator
* linux: Rewrite the audio-generation code to avoid crashes
* linux: Avoid infinite recursion in the tests
* linux: Fix warnings about unused variables
* linux: Avoid warnings about null pointer in strcmp
* linux: Avoid build error due to bad `ularge` overload in settings
* linux: Fix type issue for the Insert function (reported as a warning)
* linux: Remove warning about mixing enums and integers
* linux: Avoid error on printf format
* linux: Avoid warnings about type qualifiers
* linux: Remove warnings about incompatible function casts
* linux: Address warnings about missing initializers
* linux: Fix warning about prinf formats
* linux: Address warning about signed vs unsigned
* linux: Remove warning about fall-through switch statement
* linux: Remove warnings about unused arguments
* tests: Repair several tests that were unstable due to scrolling images
* tests: Increase memory size to avoid occasional out of memory failures
* object: Make `as_uint32` and `as_uint64` consistent for negative input


### Improvements

* ui: Emulate HP48/HP50G behavior for errors (do not require key to continue)
* simulator: Add I/O wrapper around file state save/restore
* dmcp: Remove double return in the code
* simulator: Accept numeric keys in DMCP menus
* doc: Add Kjell Christenson to list of authors
* tests: Run command-line tests silently
* tests: Clear settings the fast way for quick tests
* dmcp: Do not treat the buzzer as a recorder error (avoid message noise)
* simulator: Ensure error messages show up in a recorder dump
* tests: Increase memory size to avoid failing tests
* tests: Increase delay waiting for function plots to appear
* tests: Refactor test suite interaction with RPL thread to accelerate it
* tests: Add a delay before launching the test thread to load initial state
* tests: Add missing reference picture for `char-menu`


## Release 0.7.2 "Light" - Libraries, Characters menu

This release introduces four relatively significant features:

1. An Equation Library
2. A more general Library
3. Character menus
4. A character modification catalog

There are also a number of bug fixes and improvements.


### Features

* The *Equation Library* is similar in principle to what is found in the HP50G.
  It is intended to store equations covering a variety of topics.
  The Equation Library is accessible using the _EQS_ key (🟦 _'()'_).
  It is presently only very sparsely populated, but a future release should
  add the equations listed in Chapter 5 of the *HP50G Advanced User's Reference
  Manual*. Elements of the Equation Library appear as named *Equation Objects*.
  The Equation Library is configured by file `config/equations.csv`.

* The *Library* is similar to the Equation Library, but for all kinds of objects
  such as programs or code snippets. The Library is accessible using the _LIB_
  key (🟦 _VAR_). It can be used to customize your calculator, and is intended
  to play the roles of the `CST` variable (quick access to common features)
  and Library Objects / XLIB (external extensions to RPL). Elements of the
  Library appear as named *Library Objects*.
  The Library is configured by file `config/library.csv`.

* The *Characters Menu* lets you enter Unicode characters easily, by presenting
  various classes of characters, such as `RPL`, `Greek` or `Punct`.
  The Characters Menus is accessible using the _CHAR_ key (🟦 _2_).
  It can be used to enter international characters (e.g. Greek or Cyrillic), as
  well as special characters such as arrows or blocks.
  The Characters Menu is configured by file `config/characters.csv`.

* The *Characters Catalog* appears when the `Catalog` is active and the cursor
  is inside some text object. It gives you access to characters that are
  visually close to the character on the left of the cursor. For example, after
  typing `A`, the presented choices include `À`, `a` or `α`.
  The Characters Catalog is configured by file `config/characters.csv`.

* compare: Add comparisons for `true` and `false` values

* Add `TEVAL` command (timed evaluation)


### Bug fixes

* Do not add unnecessary parentheses in ratios, e.g. `(A+B)/(X-Y)`
* Make sure we can save back the configuration files correctly
* Improve access path checks to accept `config:constants.csv`.
* Avoid syntax error in `1/(1+x)` due to `1/` being seen as a fraction
* unitfile: Remove slight risk of bad menu display after garbage collection
* date/time: Make sure we save the stack and last args for `Date`, `Time`, ...
* parser: Skip spacing when parsing numbers (to parse back `→Text` result)
* time: Fix rendering of DMS time in lists, matrices, vectors
* catalog: Fix a subtle bug on DM32 leading to a crash using the catalog


### Improvements

* constants: Get values of special `π` and `e` by name
* constants: Add prefix in editor to identify constants, equations and xlib
* constants: Represent constants with an index for memory and performance
* constants: Parse units containing text
* constants: Allow `RCL` to recall a constant value
* units: Direct insertion of units after numbers
* menu: Update `Roll` and `RollDown` menu entries
* show: Show all decimals for decimal values
* help: Display the correct on-line help topic for constants
* catalog: Use less memory for the sorted IDs
* integrate: Use numerical computations for faster convergence
* locals: Improve error message for bad locals
* graph: Improve graphical rendering of constants (bold) and equations
* graph: Do not add unnecessary parentheses in ratios
* tests: Add tests for characters menu/catalog
* tests: Fix the `.` vs `0.` test
* ui: Do not enter DMS inside text
* tests: Display disabled tests in gray
* catalog: Keep a single spelling, e.g. no `add` duplicates
* tests: Add extra delay in the wait for update
* makefile: Add dependency of 'all' to the decimal constants
* save: Improve rendering control when saving files
* stack: Do not save stack in plot, integration or solver
* debug: Disable debugging when launching a program from function key
* simulator: Avoid piling up QT draw requests
* doc: Update performance numbers for 1M loops
* simulator: Add sound support
* simulator: Lazy screen refresh
* dmcp: Add UI refresh callback
* simulator: Move QT-dependent code out of dmcp.cpp
* Add reduced font
* runtime: Various changes to isolate QT build from the rest


## Release 0.7.1 "Whip" - Bug fixes

Inserting variables, constants or units in a program was broken.
Also a few less critical fixes.

### Bug fixes

* decimal: Apply `MinimumSignificantDigits` to `Sig` modes
* tests: Fix missing `]` at end of vector
* ui: Insert commands for unit conversions, constants and variables
* tests: Adjust help screen snapshot for authors
* menus: Do not clip text for hierarchical menus
* constants: Do not use units that don't parse correctly
* dmcp: Day of week convention adjustment
* help: Fix YouTube video preview


## Release 0.7.0 "Temple" - Graphics Equation Rendering

This release introduces a few major improvements, including graphical
rendering of equations and matrices, the `Show` command to display
large objects full-screen, customizable constants, and date-related
operations.

### New features

* Graphical rendering of equations, fractions, matrices, vectors and
  lists. In graphical rendering mode, variables are showin in italics.
* Constants in the `ConstantsMenu`, split into categories, and loading
  from an optional `config/constants.csv` file, in a way
  similar to what existed for units.
* Inverse trigonometric functions (`asin`, `acos` and `atan`) now
  produce unit objects with the current angle mode as a unit. This can
  be configured by the `SetAngleUnits` / `NoAngleUnits` flags.
* `Cycle` (_×10ⁿ_ key) now cycles between angle units.
* `R→D` and `D→R` commands to convert between degree and radian in a
  purely numerical way (no unit). This is for compatibility with HP.
* Add `→Deg`, `→Rad`, `→Grad`, `→πr` commands, which convert a number
  to the target unit using current angle mode, and convert an angle to
  the target angle unit.
* Conversion from DMS to HMS and from HMS to DMS
* Rendering of dates: `19681205_date` renders as `Fri 5/Dec/1968`,
  with a format configuration using the same flags as for the header.
  Note that the date format is `YYYYMMDD`, _not_ the same as on HP
  calculators. This allows `YYYYMMDD.hhmmss` for dates with time.
* `Date` and `Time` command to return the current date and time.
  Additionally, `DateTime` returns both date and time, and
  `ChronoTime` returns the time with 1/100s precision.
* `→Date` and `→Time` commands to set the system date and time
* `Date+`, `DDays` and date arithmetic using `+` or `-`, using day
  units for the results. As an extension relative to HP calculators,
  these will accept fractional days, or other time units. For example,
  adding `1000000_s` to `19681205_date` generates a date with time
  result, `Tue 16/Dec/1968, 13:46:40`
* `JulianDayNumber` and `DateFromJulianDayNumber` commands to convert
  between dates and Julian day numbers. These commands also accept
  fractional input.
* `Show` command showing a full-screen graphical rendering of the
  result on the stack. The resut is size-adjusted. For example, you
  can display all digits in `200!`. If the result does not fit on the
  screen, you can scroll using the _◀︎_ and _▶︎_, as well as _8_, _6_,
  _4_ and _2_. The maximum pixel size for `Show` is set by `MaxW`
  (default is the width of the LCD), the maximum height is set by
  `MaxH` (default is 2048 pixels).
* `AutoScaleStack` and `NoAutoScaleStack` settings to automatically
  adjust the font size for the stack elements.
* Support for system flags -20 to -26 (infinite results, overflow and
  underflow).


### Bug fixes

* simulator: Adjust DMCP month off-by-one error
* Repair insertion of `while` loops and similar commands on the
  command line
* Use stack format when drawing an object with `DrawText` (`DISP`)
* Arithmetic on unit objects no longer auto-simplifies, e.g.
  `1_s 1_s -` returns `0_s` and not `0`.
* Perform computations for `→Q` using integer values, which avoids an
  issue where increasing the number of iterations with an unachievable
  precision could prodduce `1/1` as the fractional result.
* Repair auto-simplification for `i*i=-1`
* Display a negative mixed fraction as `-1 1/3` and not `1 -1/3`.
* Do not insert `()` after a multiplication in algebraic mode
* Accept units and tagged objects in `PolarToReal` and `RealToPolar`
* Accept angle units as input for `→DMS`
* Off-by-one clipping error in header, erasing the shift annunciator
* Fix help for `FC?` (incorrectly stating that it tested for flat set)
* Lookup units and constants in a case sensitive way
* Fix labels for `ExpFit` and `LinFit` in `RegressionMenu`.


### Improvements

* tests: Adjust tests to match bugs fixed in v0.6.5
* dms: Accept entering minutes without third dot, e.g. `1.2.3 ENTER`
* menus: Split the Time, Date and Alarm menus
* Split rendered objects at space boundaries. This notably ensures
  that large numbers are split at digit grouping boundaries, and makes
  it possible to display larger programs on the stack. The rendering
  of programs and matrices/vectors/lists has also been fine-tuned.
* The "white circle" glyph has a thicker border, makes it more
  readable in menus.
* doc: Update the list of unimplemented features
* menus: Draw a white circle for disabled flags, and allow the menu
  function to toggle the flag. This made it possible to reduce the
  number of menu entries for flag-heavy menus.
* Mixed fractions are now the default, as opposed to improper
  fractions
* doc: Improve the quickstart guide
* doc: Improve the documentation for sin, cos and tan
* tests: Make it possible to interrupt a running test
* help: Skip HTML tags, e.g. <video>
* simulator: Add screenshot capability, and reduce window height
* menus: `ToolsMenu` selects time, date or angle menu based on units


## Release 0.6.5 "Testimony": Small bug fixes

This release does not contain much because FOSDEM took a lot of energy.

## New features

* menu: Connect `ceil` and `floor` functions
* Add real to polar conversions
* units: Add `dms` unit to angles menu

## Bug fixes

* decimal: Compute `ln(0.002)` correctly
* integer: Do not parse degree sign if in a complex
* units: Fix parsing of angle units
* font: Add radian glyph


## Release 0.6.4 "Healing": Testing and fixes

This release focuses on heavy testing of the new variable-precision
decimal stack. Adding tests also means finding bugs.

Another significant change is support for fixed-precision
hardware-accelerated floating point, using 32-bit and 64-bit IEEE754
binary floating-point representation. Since this uses a binary format,
some decimal values do not map to decimal correctly. For example,
displaying 1.2 with a large number of decimals will show residue,
because 1.2 does not have an exact (zero-terminated) representation in
binary.

### New features:

* plotting: Make refresh rate configurable
* menu: Add `/` key to `FractionsMenu`
* hwfp: Add support for hardware-accelerated floating-point
* menus: Add hardware floating-point flag to `MathModesMenu`
* ui: Allow multiple uses of `.` to insert DMS separators
* HMS: Editing of HMS values in HMS format

### Bug fixes:

* stats: Fix crash on `variance` with single-column statistics
* algebraic: Clear error before evaluating the function
* functions: Correctly emit a type error for non-algebraics
* ui: Make sure we save stack if closing the editor
* logical: Fix mask for rotate left with 64-bit size
* logical: Make sure we save args for single-argument logicals
* flags: Update flags on `FlipFlag`, consume them from `BinaryToFlags`
* stack: Show multi-line objects correctly
* lists: Returns `Bad argument value` for index with bad arguments
* lists: Return an empty list for tail of empty list
* arithmetic: `→Frac` should not error on integers
* power: Do not shut down during `WAIT` if on USB power

### Improvements:

* menu: Shorten the labels `→QIter` and `→QPrec` to avoid scrolling
* stack: Avoid running same code twice on simulator
* ids: Add aliases for hardware floating point
* functions: Optimize abs and neg
* ui: Replace calls to `rt.insert` with calls to `insert`
* menu: Reorganize fractions menu
* dms: Do the DMS conversion using fractions
* list: Adjust multi-line rendering
* copyright: Update copyright to 2024
* text: Return null text when indexing past end of text

### Testing:

* tests: Increase the delay for help to draw
* tests: Add tests for hardware-accelerated floating-point
* tests: Add shifts and rotate tests
* tests: Check flag functions
* tests: Test DMS and HMS operations
* tests: Add test for `integrate` using decimal values
* tests: Test multi-line stack display
* tests: Add tests for `GETI`
* tests: Min and max commands
* tests: Repair last regression test
* tests: Check behaviour of 0^0
* tests: Avoid string overflow in case of very long message


## Release 0.6.3 "Come and See": Repair test suite

The focus of this release was to reactivate the test suite and fix the
problems that were found activating it.

### New features:

* HMS and DMS operations
* unit: Rendering of `dms` and `hms` units
* Allow `·` as a multiplication sign in equations
* ui: Display 'E' cursor inside parentheses
* graphics: Accept based integers as position for `DrawText`
* complex: Add setting to switch between `2+3i` and `2+i3`.

### Bug fixes:

* decimal: Fix precision loss for addition with carry
* Base: limit range of bases to 2-36
* files: Do not add a trailing zero when reading a text file
* decimal: Adjust `MinimumSignificantDigits` behaviour
* units: Do not auto-simplify `1.0` during conversion
* decimal: Normalize parsed numbers
* solver: Fix sign error in epsilon exponent for solve/integrate
* parser: Accept `x!` as input
* simulator: Avoid faulty break-through in switch statement
* complex: Make tag higher-priority than complex
* editor: Adjust cursor and select correctly during replace
* decimal: Fix display of 0.2 in NoTrailingDecimal mode
* complex: Save `this` in a GC pointer when it can move
* arithmetic: Do not fail because of surrounding error
* decimal: Clamp int32 conversions from decimal
* commands: Parse `exp10` correctly in expressions
* decimal: Avoid infinite loop computing `expm1`
* ids: Do not allow parsing of structures
* dmcp: Make sure the tests don't block on `wait_for_key`
* decimal: Use correct angle unit for negative gamma values

### Improvements:

* settings: Add classes that save/restore a given setting
* Add `XSHIFT` to shift to XSHIFT state directly
* Add NOSHIFT constant to simplify test writing
* ui: Add a variant of `close_editor` without trailing zero
* decimal: Cache gamma_ck values (accelerate gamma and lgamma)
* Replace magic constant `-1` with `EXIT_PGM`
* doc: Record performance data for 1000 iterations of SumTest
* decimal: Add tracing for gamma / lgamma function

### New tests:

* Add tests for arithmetic truncation on short bitsizes
* Add tests for on-line help
* Add test for plotting flags
* Add test for Unicode to text conversions
* Add test for rectangular complex display options
* Add tests for plot scaling functions
* Add tests for the sorting functions
* Add test for parsing text with quotes inside
* Add test for file-based `STO` and `RCL`
* Add test for 2^256 computation (buf #460)
* Fix indentation of `[PASS]` or `[FAIL]` for UTF8 characters
* Add tests for units and conversions
* Add a keyboard test checking the single-colon insert in text
* Add test for i*i=-1 auto-simplification
* Add basic test for numerical integration
* Add test for fraction formats
* Add solver test
* Add missing tests
* Add test for immediate `STO` (#390)
* Add tests for the `Cycle` command
* Add test for catalog feature
* Add test for "smart" keyboard shortcuts introduced in 0.4.6
* Add regression test for #371
* Add tests for editor operations
* Test stack operations
* Add test for `GXor`, `GOr` and `GAnd`
* Add test for `ResetModes`
* Add plotting test for every decimal function
* Add image checking for graphical tests
* Add tests for graphic and text drawing commands
* Make it possible to individually run tests.
* Add plotting tests
* Add test parsing the various spellings for commands

## Release 0.6.2 "Kephas": Restoring the test suite

The focus of this release was to complete the transition to the
variable-precision decimal code. The full regression test suite was
restored, with only two tests still failing (complex `asin` and
`asinh`). The two missing functions, `Gamma` and `LogGamma`, were
implemented. This also exposed many bugs that were fixed.

### New features:

* `NumberedVariables` setting to allow `0 STO` to ease RPN transcoding (#690)
* `ClearStack` (CLEAR) command
* Variable-precision `gamma` and `lgamma` functions (#710)

### Bug fixes:

* Do not leave bad expression on stack after error (#663)
* Reject unit conversion between undefined units (#664)
* Accept names of menus on the command line (#669)
* tests: CLEAR clears stack and errors (#672)
* The `:` characters now starts a tag on empty command line (#673)
* Do not evaluate tagged objects too early (#674)
* test: Entry and editing of equations (#677)
* test: `type` returns negative values in "detailed" mode (#678)
* Run loop end pointer not GC-adjusted for last allocated object (#679)
* Do not erase alpha indicator when clearing busy cursor (#680)
* Do not emit trailing space when rounding `1.999` to `2.` (#687)
* Allow expression rewrites to deal with `pow` (#694)
* Reject `«` character in symbols and commands (#695)
* Rendering of spacing for decimal numbers (#699)
* Improve precision of `atan` function
* Rendering of `+` and `-` operators in compatibility modes (#700)
* Rounding of decimal numbers no longer generates bad characters (#702)
* No longer error out rendering symbolic complex (#703)
* Parse complex numbers correctly inside equations (#704)
* Ensure `sqrt(-1)` has a zero real part (#705)
* Do not drop last digit of decimal subtraction (#706)
* Evaluate expressions during expression pattern matching (#708)
* `LastMenu` executes immediately while editing (#711)
* Add back missing angle glyphs in font (#712)
* Return correct angle quadrant for `atan2` (#715)
* `Get` no longer rejects arrays as index values (#718)
* `Get` returns correct error for bad argument types (#718)

### Improvements:

* Update `.gitignore` entry
* Automated testing of setting flags (#657)
* Automated testing of non-flag settings (#670)
* Test fixed-base based numbers using HP-compatible lowercase notation (#675)
* Test fraction rendering using fancy digits (#676)
* Add test for "compatible" variant of `type` command (#678)
* test: Recover from error during data entry (#681)
* Increase default `MaxNumberBits` to 4096 (#686)
* tests: Add test for cube root of -8 (#685)
* tests: Data entry for `Σ`, `∏` and `∆` (#689)
* Restrict `STO` to names, natural numbers and specific IDs (#688)
* doc: Document flas file access for `STO` and `RCL` (#691)
* list: Optimize iterators to avoid copies (#692)
* tests: spacing-independent testing of structure rendering (#693)
* tests: adjust test suite for variable-precision decimals (#696)
* tests: Automatically exit if `-T` option is given (#697)
* tests: Check we don't accept `.` as meaning `0.` for RPL compatibility
* Add `modulus` alias back for `abs` (#707)
* Improve convergence speed and accuracy for logarithms
* Add `decimal::make` to minimize risk of sign errors
* Display stack level at top of stack level, not bottom (#709)
* Improve complex `sqrt` accuracy on real axis (#714)
* tests: Test decimal functions at full 34 digits precision (#716)
* Complex `sqrt` and `cbrt` with full precision (#717)
* tests: Take into account additional expression simplifications

## Release 0.6.1 "Happy New Year": Quick bug fixes

A few quick bug fixes that make DB48X a bit more usable.

### New features

* Setting to display `2+i3` instead of `2+3i` (#660)
* HMS and DMS operations (#654)
* Special `1_dms` and `1_hms` units rendering in DMS / HMS (#650)

### Bug fixes

* Improve behavior of `+/-` key while editing (#658)
* Do not accept base 37 (#656)
* Insert `for` statement in program instead of executing it (#655)
* Hide trailing decimal separator for decimals with integer values (#653)
* Fix display of `19.8` with `0 FIX` (#652)
* Implement true decimal to integer conversion (#648)

### Improvements

* doc: Record performance data for 0.6.0
* doc: Udpate status file

## Release 0.6.0 "Christmas": Introducing variable precision

This release was a bit longer in coming than earlier ones, because we are about
to reach the limits of what can fit on a DM42. This release uses 711228 bytes
out of the 716800 (99.2%).

Without the Intel Decimal Library code, we use only 282980 bytes. This means
that the Intel Decimal Library code uses 60.2% of the total code space. Being
able to move further requires a rather radical rethinking of the project, where
we replace the Intel Decimal Library with size-optimized decimal code.

As a result, release 0.6.0 introduces a new table-free and variable-precision
implementation of decimal computations. In this release, most operations are
implemented, but some features are still missing (e.g. Gamma function). This
release will be simultaneous with 0.5.2, which is functionally equivalent but
still uses the Intel Decimal library. The new implementation is much more
compact, allowing us to return to normal optimizations for the DM42 and regain
some of the lost performance. On the other hand, having to switch to a table
free implementation means that it's significantly slower than the Intel Decimal
Library. The upside of course is that you can compute with decimal numbers that
have up to 9999 digits, and a decimal exponent that can be up to 2^60
(1 152 921 504 606 846 976).


### New features

Variable precision decimal floating point implementation for arithmetic,
trigonometrics, logs, exponential and integer factorial. Other functions may
still return "unimplemented error".

### Bug fixes

None. If anything, this release introduces bugs in computations and performance
regressions. However, it frees *a lot* of space for further DM42 development.

### Improvements

The `Precision` setting now sets the number of digits with a granularity of one,
between 3 and 9999. Ideal use of memory is with multiples of 12 digits, e.g. 12,
24 or 36 digits, where decimal packing does not cause lost bits.

Performance on the DM42 is somewhat improved, since it is now possible to return
to a higher level of optimization.

### Regressions

In addition to lower performance and unimplemented functions, this version no
longer builds a compatible QSPI. This means that returning to the DM42 requires
flashing *both* the QSPI and the PGM file.


## Release 0.5.2 "Christmas Eve": Reaching hard limits on the DM42

This release was a bit longer in coming than earlier ones, because we are about
to reach the limits of what can fit on a DM42. This release uses 711228 bytes
out of the 716800 (99.2%).

Without the Intel Decimal Library code, we use only 282980 bytes. This means
that the Intel Decimal Library code uses 60.2% of the total code space. Being
able to move further requires a rather radical rethinking of the project, where
we replace the Intel Decimal Library with size-optimized decimal code.

As a result, release 0.5.2 will be the last one using the Intel Decimal Library,
and is release in parallel with 0.6.0, which switches to a table-free and
variable-precisions implementation of decimal code that uses much less code
space. The two releases should otherwise be functionally identical

### New features

* Shift and rotate instructions (#622)
* Add `CompatibleTypes` and `DetsailedTypes` setting to control `Type` results
* Recognize HP-compatible negative values for flags, e.g. `-64 SF` (#625)
* Add settings to control multiline result and stack display (#634)

### Bug fixes

* Truncate to `WordSize` the small results of binary operations (#624)
* Fix day-of-week shortcut in simulator
* Avoid double-evaluation of immediate commands when there is no help
* Generate an error when selecting base 1 (#628)
* Avoid `Number too big` error on based nunbers
* Correctly garbage-collect menu entries (#630)
* Select default settings that allow solver to find solutions (#627)
* Fix display of decimal numbers (broken by multi-line display)
* Fix rendering of menu entries for `Fix`, `Std`, etc
* Detect non-finite results in arithmetic, e.g. `(-8)^0.3`m (#635, #639)
* Fix range-checking for `Dig` to allow `-1` value
* Accept large values for `Fix`, `Sci` and `Eng` (for variable precision)
* Restore missing last entry in built-in units menu (#638)
* Accept `Hz` and non-primary units as input for `ConvertToUnitPrefix` (#640)
* Fix LEB128 encoding for signed value 64 and similar (#642)
* Do not parse `IfThenElse` as a command
* Do not consider `E` as a digit in decimal numbers (#643)
* Do not parse `min` as a function in units, but as minute (#644)

### Improvements

* Add `OnesComplement` flag for binary operation (not used yet)
* Add `ComplexResults` (-103) flag (not used yet)
* Accept negative values for `B→R` (according to `WordSize`)
* Add documentation for `STO` and `RCL` accessing flash storage
* Mention `True` and `False` in documentation
* Rename `MaxBigNumBits` to `MaxNumberBits`
* Return HP-compatible values from `Type` function
* Minor optimization of flags implementation
* Catalog auto-completion now suggests all possible spellings (#626)
* Add aliases for `CubeRoot` and `Hypothenuse`
* Align based number promotion rules to HP calculators (#629)
* Expand the range of garbage collector integrity check on simulator
* Show command according to preferences in error messages (#633)
* Avoid crash in `debug_printf` if used before font initialization
* Update performance data in documentation
* Add ability to disable any reference to Intel Decimal Floating-point library
* Simplify C++ notations for safe pointers (`+x` and `operartor bool()`)
* Fix link to old `db48x` project in `README.md`


## Release 0.5.1 "Talents": More RPL commands

This release focuses on rounding up various useful RPL commands
and bringing RPL a bit closer to feature-complete.

### New features

* Portable bit pattern generation commands, `gray` and `rgb` (#617)
* Add support for packed bitmaps (#555)
* Implement RPL `case` statement, extended with `case when` (#374)
* `Beep` command (#50)
* `List→` command (#573)
* `Size` command (#588)
* `Str→` command (#590)
* `Obj→` command (#596)
* Add flag to control if `0^0` returns `1` or undefined behaviour (#598)
* Unicode-based `Num` and `Chr` commands, `Text→Code` and `Code→Text` (#597)
* `IP` and `FP` commands (#601)
* Percentage operations `%`, `%CH` and `%T` (#602)
* `Min` and `Max` operations (#603)
* `Floor` and `Ceil` operations (#605)
* `Get` with a name argument (#609)
* `Put` command (#610)
* `Head` and `Tail` commands (#614)
* `Map`, `Reduce` and `Filter` commands (#613)

### Bug fixes

* Ensure rounded rectangles stay within their boundaries (#618)
* Prevent auto-power-off for long-running programs (#587)
* Fix old-style RPL shortcuts for `FS?C` and the like
* Add `FF` shortcut for `FlipFlag`
* Fix rendering of `<`, `>`, etc in old-style RPL compatibility mode (#595)
* Update various menus
* Evaluate program arguments in `IFT` and `IFTE` (#592)
* Evaluate algebraic expressions in `if`, `while` and `case` (#593)
* Load variables from state file in correct order (#591)
* Avoid truncation of state file when ASCII conversions occur (#589)
* Clear debugging state more completely after `kill` (#600)
* `Wait` no longer makes it harder to stop a program (#619)
* `mod` no longer gives wrong result for negative fractions and bignums (#606)
* No longer strip tags in non-numeric arithmetic operations (#607)

### Improvements

* Small updates to demo file
* A long `Wait` command allows the calculator to switch off (#620)
* Centering of variable names in `VariablesMenu` (#610)
* Makefile `check-ids` target to check if commands are in help or menus (#615)


## Release 0.5.0: Statistics and flags

This release provides statistics functions and flags.

### New features

* Statistics (#495) and linear regression (#569)
* File-based statistics (#571)
* `Sort`, `QuickSort`, `ReverseSort`, `ReverseQuickSort` and `RevList` (#572)
* Flags, i.e. `CF`, `SF`, `FS?`, etc (#43)
* Plot scaling commands such as `SCALE`, `CENTR`, `XRNG`, ... (#582)
* Add `CurveFilling` and `DrawPlotAxes` setting flags (#580)
* `ScatterPlot` (#577) and `BarPlot` (#579)

### Bugs

* Save settings enumerations as portable text (#565)
* Avoid infinite loop when reading at end of help file
* Repair behaviour of `-1 DIG`, broken by settings improvements
* Fix definition of `rpm` in units and units file
* Crash in `list::map` when called function errors out (#570)
* Fix editor horizontal movement when inserting commands (#576)
* Repair plotting demo (#583)
* Fix vertical position of axes in `DrawAxes` (#584)
* Very long drawing loop if `ppar` axes are backwards (#585)

### Improvements

* Sets editor selection correctly for command-line errors
* Ability to parse command and setting names in quotes, e.g. `'Radians'`
* Insert command names inside quotes (#575)
* Update documentation of implemented features (#569)
* Make `PlotParameters` a keyword (#578)

## Release v0.4.12: Emergency bug fixes, filesyste, access

A few quick bug fixes related to issues found in 0.4.11.

### New features

* `STO` and `RCL` to file (#375)
* Parsing of text containing quotes (#562)

### Improvements

* Rework file load/save dialog boxes
* Preliminary plumbing for statistics functions

### Bugs

* Error loading state files that contain directories (#559)
* Font setting for result was taken from editor font (#560)
* Crash running the `Shapes` demo (#563)

## Release 0.4.11: Debugging, Units, Settings

This release implements debugging, infinite RPL recursion, mixed
fractions, customizable units cycling, and restores missing entries in
the units menu.

### New features

* Allow customization of `Cycle` for units (#534)
* Allow infinite recursion in RPL code (#537)
* RPL program and expression debugging (#552) including after EXIT
* Mixed fractions such as `1 1/3` (#554)
* `BeepOn` and `SilentBeepOn` features (#280)
* `ScreenCapture` and keyboard shortcut (#434)

### Bugs

* Accept `2.3 FIX` and `#0 Background` (#557)
* Do not parse `67.200525` as a `decimal32` (#551)
* Bump low battery voltage to 2.550V (#553)

### Improvements

* Catalog shows all commands *containing* typed text (#556)
* Reorganize the units menu (#550) and restore missing units
* Remember menu page for `LastMenu` (#545)
* `SPC` key inserts `;` when inside parenthese (#499)
* Settings are now entirely defined by `ids.tbl` (#294)
* Improve user interface code consistency
* Improve GC handling of "just-past-end-of-object" pointers
* Remove the `execute()` RPL callback, rely on `evaluate()`
* Optimize allocation of 1-byte vs 2-byte opcodes
* Render `abs` as `abs` and not `norm` (accept `norm` while parsing)

## Release v0.4.10: Performance and units file

This release focuses on display performance optimization and support
for unit data files.

### New features

* Cycle command for units (#517)
* Possibility to load units from config/units.csv file (#496, #544)
* Internal: debug_printf facility (#541)
* ShowBuiltinUnits and HideBuiltinUnits settings (#542)
* Provide config/units.csv example file (#543)
* Build units menu from units file (#544)

### Bugs

* Possible memory corruption in expressions
* Calculator lock-up trying with unit conversions in numeric mode (#529)
* Numerator and divisor no longer truncated to 32-bit (#539)
* Correctly save LastArg for interactive operations (#540)

### Improvements

* Optimize the font cache, makes stack drawing faster in SCI mode (#526)
* Add TypeName to object menu (#527)
* Regroup units in units menu by scale (#528)
* Status of implementation and performance data in documentation (#530)
* Performance: Reduce frequency of busy cursor drawing (#531)
* Performance: Reimplement range-based type checkig (#532)
* Performance: Focused -O3 optimizations on DM42 (#533)
* Makefile: Install target installs demo and config (#547)
* Garbage collector for menu labels (#548)


## Release 0.4.9 - Support for units

This release focuses on support for units, but also adds a large number of other
fixes and improvements.

### New features

* Power-off message indicating low-battery situation (#521)
* Add ConvertToUnixPrefix command and SI prefix menu keys (#513)
* Recognize all units that exist in the HP48, and a few more (#491)
* UFACT (FactorUnit) command (#512)
* Unit simplification, e.g. turn 1_m^2*s/s/m into 1_m (#506)
* Converting unity units to numbers (#502)
* →Unit command (#501)
* UnitValue (UVAL) command (#493)
* Implement "kibibytes" (KiB) and power-of-two SI prefixes (#492)
* Unit arithmetic (#481)
* Add B->R and R->B to BasesMenu (#488)
* Implement term reordering capability in rewrite (#484)
* BaseUnits (UBase) command (#483)
* Unit parsing for complex units, e.g. 1_cm^2 (#482)
* Unit arithmetic (#481) including automatic conversions (#480)
* Convert command (#480)
* Implement the Cycle command for unit objects
* Add Å character for angstroem (#477)
* Add Merge state to State system menu (#475)
* Use Unicode font to display the name of a program when executing it (#469)
* Allow incremental search to find digits and Unicode (#468)
* Add tool glyph to user interface font

### Bug fixes

* Do not parse symbols beyond input buffer (#524)
* Parse unit menu entries as expressions, not symbols (#523)
* Fix reduced-precision arithmetic (#521)
* Do not parse empty denominator as zero, e.g. 2/s (#520)
* Do not parse a fraction inside a power, e.g. X^2/3 (#519)
* Convert fractions to decimal in numeric mode (#516)
* Do not emit mantissa_error for valid numbers (#515)
* Apply negation correctly on unit objects (#500)
* Do not emit separator after trailing 0 in integer decimals (#489)
* Do not emit extra spacing before decimal separator (#485)
* Fix stack depth in one error case of evaluate_function()
* Fix display of next/previous icons for large menus (#478)
* Clear settings when loading a state (#474)
* Fix separators in whole part of decimal numbers when setting is not 3 (#464)
* Parse (sin x)²+(cos x)² correctly, as well as HP67 Mach example (#427)

### Improvements

* Rename equation as expression (#518) and labelText as label_text
* Do not update LastArg except for command line (#511)
* ToolsMenu: Connect units to the UnitsConversionMenu (#514)
* Display unit using / and ·, e.g. 1_m·s^2/A (#507)
* Show units menu for inverse units as mm⁻¹ (#503)
* Display battery level more accurately, i.e. consider 2.6V "low" (#476)
* No longer acccept empty equations or parentheses, e.g. 1+() (#487)
* Make StandardDisplay mode obey MinimumSignificantDigits (#462)
* Add algebraic evaluation function for easier evaluation in C++ code
* Reimplement unit type as a derivative of complex (#471)
* documentation: Use 🟨 and 🟦 for more commands (#467)
* Swap Search and Copy commands in EditorMenu (#466)
* STO stores variables at beginning of directory (#462)
* documentation: Add quickstart guide, used for video recording
* documentation: Add links to YouTube videos
* documentation: Add release notes
* documentation: Some typos and other improvements
* documentation: Rework section on keyboard mappings
