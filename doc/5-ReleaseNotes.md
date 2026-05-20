# Release notes

## Release 0.9.19 "Spirit" - More constants, more space left

This release includes a larger-than-usual number of changes, mostly to
make room for more constants. A side effect is that we gained a lot of
additional space for the DM42, giving us a lot of headroom for future
developments. On the other hand, this may cause unexpected bugs. If
you run DB48x on a DM42, please check for anything out of the ordinary
and report it.

### New features

* Hierarchical constants library, with astronomy data for planets
* fonts: Add astronomy-related glyphs and a few more
* simulator: RPL evaluation with -e, to preload the calculator
* simulator: RPL console evaluation mode with -E (prints to console)
* simulator: RPL evaluation from file with -f and -F
* simulator: headless mode with -H (no window showing)
* simulator: screen capture with -C for scripting
* simulator: Persist window geometry across runs
* simulator: Persist calculator state on exit
* ui: Enhanced behaviour for EEX key (skips to exponent)
* variables: Store and recall to path
* GCD (GreatestCommonDenominator) and LCM (LeastCommonMultiple)
* Quote command
* Hitting EXIT while editing clears menu

### Bug fixes

* variables: Memory corruption when replacing object with subobject
* Fix parsing bug causing partial load of .48k keymaps beyond "@"
* arrays: Strip tags from array and index in `get` and `put`
* solver: Improve solver precision in Std mode
* constants: Correcting the standard uncertainty of several constants.
* constants: Correct the Tau mass relative uncertainty.
* Make sure simulators get the correct help file (distinct .qrc files)
* Show empty menu when entering empty directory in 1-line mode
* Accept decimal / fraction conversions for arrays containing names
* Make it possible to reload `Demo.48s` after initial run on DM42
* Correctly show error message while loading files
* ui: Do not remove `-` when using CHS in an expression like `5-3`.
* bignum: Remove leftover printf statement
* menu: Refresh `CustomMenu` if updated while active
* simulator: Fix race condition in Paste command
* ci: keep Android AABs under `android/`
* ci: package db50x Windows simulator from the correct path
* wasm: bootstrap recorder and host tools correctly
* ci: use libsystre in Windows packaging
* doc: Fix description of simulator -m option
* ids: Fix aliases for EquationValue and XLibValue
* menus: Connect menu entries incorrectly marked as unimplemented
* Fix help topics for various constants / constant groups

### Improvements

* fw: Move all C / C++ strings to the QSPI on DM42 (save >80K flash)
* tests: Replace `LSHIFT, I` with `ID_ConstantsMenu` for readaibility
* tests: Run cstlib / eqlib tests beyond first error
* tests: Use `STD` instead of `12 SIG`
* tests: Do not save state file after running the test suite
* Do not waste memory while converting`<<`, `>>` and `->`
* Improve rebuild in case of QSPI CRC mismatch
* Enable `make TAR_EXTRA_FILES= install` to install only binaries
* makefile: Generate listing files like the old makefiles
* makefile: Add image comparison targets like in old makefile
* makefile: Avoid extraneous / in paths
* makefile: Pass DB48X_VERSION through the command-line
* debug: Change debug_printf to use all 12 rows
* commands: micro-optimize factorization of cmp == 0 test
* chuck: Make the CHUCK command optional for repeatable builds
* install: Add `DISK_NAME` variables to specify volume name
* ui: Simplify display of alpha and user indicators
* doc: Add information on how to start Fedora on WSL
* ui: Fix misspelling in comment
* doc: Make the age-verification LEGAL-NOTICE more general
* chore(ci): bump packages versions
* doc: Add Pasquale Pigazzini as an author
* solver: Fix compiler warning about signed vs unsigned
* library: Fix comment and remove duplicate entry
* commands: Add `Γ` alias for `Gamma`
* menus: Update and reorganize parts menu
* locals: Detect incorrect argument count in algebraic function calls
* locals: Do not evaluate algebraic expression passed as argument
* version: Update copyright year to 2026
* parser: Accept `,` as a separator in algebraic function calls
* Reorder the constants menu for efficiency


## Release 0.9.18 "Chuck" - Sucess begins inside

This release includes a number of under-the-hood improvements that
will make the life of maintainers easier in the long run.

### New features

* Implement the `Chuck` command
* Make `xq` recognize more patterns during simplification
* List the menus in the help file
* Support for tables in the help file
* `ListRecursionDepth` setting for `map`, `reduce`, `filter`

### Bug fixes

* Remove `-fsanitize` option on macOS (hangs on macOS 26.4)
* Fix incorrect evaluation of constants during simplification

### Improvements

* Switch to the make-it-quick build system
* Shared cross-platorm clipboard code
* Automate the build of releases
* Put the variables naming expression patterns in a namespace
* Improved instructions for AI agents
* Repair GitHub actions
* Improve solver shuffle around singularities and errors
* Fix path for grep testing in case of example test failure

## Release 0.9.17 - Rational approximation, indexing, Android fixes

This incremental release delivers new features and fixes a number of bugs.

### New features

* Add `→Qπ` command for rational approximation with π, √n, ln(n) and e
* Apply `→Q` and `→Qπ` to numbers in expressions
* Add array/list indexing support in algebraic expressions e.g. `'L(1)'`
* Add ability to `Store` / `Copy` to `L(1)`
* Add "real" `ConstantName` and `ConstantValue` commands

### Bug fixes

* Fix parentheses around exponents in graph rendering of `'(a+b)^(c+d)'`
* Fix label for `ToRelativeUncertainty`, `→RelRnd` instead of `→RelUnc`
* Fix command alias mapping for `RNrm`
* Improve parsing of implicit multiplication for a number of edge cases
* Fix `utf8_next` for garbage bytes following ASCII characters
* Fix unsatisfied symbol errors with gcc/emcc in the WASM build
* Save Android system state when app is suspended or hidden
* Ensure Android help resources are extracted and available at runtime
* Restore image testing functionality in the test framework
* Updated test images that had become obsolete
* Report errors when saving reference files in tests
* Avoid stack corruption / crash when parsing malformed help index
* Fix computation of `DFC(2.3)`
* Fix null-pointer crash if the multiple equation solver fails


### Enhancements

* Constrain `→Q` and `→Qπ` behavior to FIX/SCI/ENG setting (like HPs)
* Minor optimization for `program::run`
* Save about 3.4K of RAM on physical devices
* Speed up test runs while increasing default wait time for stability
* Documentation cleanup and additions, add more RPL examples
* Add clear build-time error message when a help topic exceeds 80 characters
* Add demo for interactive menu use
* Optimize away redundant argument checking
* Documentation for DCF and DCF2F


## Release 0.9.16 - Factorization, precision fixes, and documentation

This release adds prime and factorization features, improves numeric precision,
and expands simulator and user documentation.

### New features

* Add `IsPrime`, `Factors`, `PrevPrime` and `NextPrime` commands
* Add `MaxFactorsBits` and `MaxFactorIterations` settings for factorization
* Add `TRIGSIN` command for Pythagorean identity rewrites
* Add a Unix man page and simulator documentation
* Add simulator recorder dump on `F16`; move demo playback to `F13`-`F15`
* Paint navigation arrows in `SHOW`
* Accept `CR/LF` line endings when reading Windows files
* Display keyboard access paths in help for commands

### Bug fixes

* Fix `expm1` precision loss for small arguments, including complex values
* Fix `ln1p` precision loss for small arguments
* Fix shift behavior after transient alpha, support shift/xshift for transalpha
* Reset `show_x` and `show_y` when exiting `SHOW`
* Ensure the simulator creates the `screens` directory when needed
* Fix an infinity optimization regression in arithmetic
* Add missing null-dereference checks in factorization code
* Treat `^` as right-associative like on the HP50G

### Enhancements

* Refactor and optimize the decimal factorization code path, with expanded tests
* Add conversion support to `float` and `double` in algebraic code
* Update to recorder v1.2.3
* Add documentation updates (DeepWiki reference, legal notice updates)
* Add build preparation for upcoming make-it-quick integration
* Welcome several new authors to the team, see AUTHORS file
* Add LEGAL-NOTICE.md about compliance with California and Colorado laws
* Add menu entries for `hypot` and `atan2`

## Release 0.9.15 "Myriam" - GitHub automation, portability

This release focused on GitHub and GitLab automation

### New features

* GitHub automation automatically builds various packages
* Preparation for GitLab automation (still largely untested)
* Windows simulator
* Android package
* WebAssembly application (WASM) now has help and configuration files
* DM42n build (differs from DM32 only in the help files)
* Dockerfile to create a container with the simulator
* Add `ρ` in `RangesMenu`
* Accept units when creating a range from components
* Add `Range→` command that expands ranges with units

### Bug fixes

* Corrected typos in some builtin constants
* Spelling and grammar fixes in documentation
* Build failure for wasm due to use of `source`, a `bash`-only syntax
* WebAssembly application no longer uses 100% CPU
* The simulator can now be launched from any directory
* The macOS simulator correctly launches from the Finder or Dock
* Portability bug fixes in the DMCP emulation
* The Settings menu is now correctly refreshed after `ResetModes`
* Fix double-shifted arrow keys
* Correctly show help for `Swap`, `Negate`, and `Cycle`
* Fix order on stack for `Explode` on a range value
* Android build bug fixes

### Enhancements

* Add osx program in lauch.json
* The simulator no longer overwrites configuration files by default
* Builds that use `gcc` now enable more warnings
* Simplify shift-handling logic


## Release 0.9.14 "Latran" - Android preparation

This is a very minor release with bug fixes notably for Android

### New features

* The screen updates when rendering and `PICT` is set

### Bug fixes

* `Wait` for durations longer than 1 second now works in simulator
* `→%Range`, `→∆Range` and `Range→` now use displayed values
* 0.9.13 regression, leftover input value after error during plotting

### Improvements

* Semi-working Android build


## Release 0.9.13 "Saints" - Truth plot, colors, and bug fixes

This release focuses on graphics and a few user-reported bug fixes

### New features

* `bins` computes frequency bins from statistics data
* `Res` sets the resolution when plotting
* `HistogramPlot` draws a histogram (frequency) plot from an array
* `HSV` generates a color pattern from hue, saturation and value
* `Color` generates a color pattern from an object, including names
* `TruthPlot` renders a truth value plot, with grayscale and color options
* Add a new `XYPlots` demo illustrating `TruthPlot` variants

### Bug fixes

* Update the screen in `Show` when `Pict` is set
* Fix crash in `Date+` when adding invalid dates
* Correctly update the `UserModeMenu` and annunciator when changing User mode
* The `STO` operation now works sensibly on flags
* Fixed a rare on-startup crash running tests in the debugger
* Invalidate the font cache when changing settings (notably `FixedWidthDigits`)
* Fix transient `Input` validation errors after text-editor errors
* Remove two extra parentheses in the documentation for settings

### Improvements

* Transient Alpha now preserves existing Alpha / Lowercase mode
* Remove dependency on external `crc32` utility, now use a local implementation
* Remove dependency on `tac`, replace it with `tail -r`
* Rendered graphics are now preserved in case of auto-power-off
* Avoid transient build errors on missing help directory
* Add documentation for `ResetModes` and usage example for `Modes`
* Negative dates now show as `BC` instead of with a minus sign


## Release 0.9.12 "Alive" - Return to active duty

This release focuses on simulator improvements, bug fixes, and enhanced
mathematical functionality, marking a return to active development and
maintenance after the hiatus caused by Christophe de Dinechin's move.

### New features

* Enhanced trigonometric functions with exact angle reduction when possible
* Improved simulator graphics by smoothing the output when rescaling

### Bug fixes

* Compute Julian day number conversion with negative Gregorian years
* Fixed crash in `lgamma` function on DM32 / DM42n platforms
* Fixed undefined symbol issue in WASM builds
* Fixed platform-specific compilation errors on Android
* Fixed incorrect simplification for `sqrt` function
* Fixed display of checksum in the `Info` box
* Fixed hang when reading CSV files containing unit objects
* Do not reorder mean and standard deviation parts in uncertain numbers
* Avoid possible null-dereference in several statistical functions

### Improvements

* The simulator now preserves a fixed aspect ratio while resizing
* The simulator now builds cleanly on Qt5
* Added 2 new tests for Julian day number conversion
* Updated copyright year in About message
* Improved description of "Julian day number" commands in documentation
* Improved build system to select qmake6 on Arch Linux
* Show `→Num` spelling in compatibility mode
* Updated AUTHORS list
* Remove documentation for newRPL commands UDEFINE and UPURGE
* Improve implementation status section of the documentation

## Release 0.9.11 "Prayer" - Uncertain number

This release implements arithmetic and basic functions on uncertain numbers.
This propagates uncertainty using quadradic superposition, in a way similar to
OpenRPNCalc, with a generalization for correlated variables.

### New features

* Arithmetic and basic functions on uncertain numbers
* Add support for complex-valued unit objects, e.g. `5+3ⅈ Ω`
* Add support for range-valued unit objects, e.g. `1…3 Ω`
* Support arithmetic operation on infinities, e.g. `∞ 42 +`
* Add commands to generate mathematical constants (π, ℮, ∞ and −∞)
* Parsing and editing of ranges containing infinities
* Accept infinities in `→Range` and variants
* Comparison of infinite values and finite values
* Add union and intersection commands for ranges
* Add operations to build ranges and uncertain numbers from components
* Add derivative for `erf` and `erfc`

### Bug fixes

* Do not leave bad expressions behind after errors on ranges
* Obey the `InfinityError` flag when detecting divide-by-zero in ranges
* Avoid crash pushing null pointer after unit evaluation error
* Avoid spurious inconsistent units error due to date computations
* Null-protection in variadic `list::make`, fixes null-deref crash

### Improvements

* Accelerate error exit from arithmetic operations
* Render uncertain numbers as `110±σ15` instead of `110±15σ`
* Accept `−` as valid input in constants (for `−∞`)
* Record performance data for iPhone 16
* Change spelling to `→σRange` in `RangeMenu`
* Use `ASSERT` for divide by zero inner checks (remove dead code)


## Release 0.9.10  "Best" - Ranges and documentation

This release is primarily a refinement of ranges and a documentation update.

### New features

* Always display infinity ranges with range notation
* Add constant for `-∞`
* Add support for ranges in `Explode` / `Obj→`
* Add support for delta and percent ranges in `ToDecimal`
* Allow `Copy` to target a local variable or setting
* Support `True` and `False` in  `→Num`

### Bug fixes

* Replace `log` with `ln`, make `log` be base-10 logarithm
* Check zero-divide in ranges
* Fix the implementation of trig functions for ranges
* Fix incorrect command associated to the `%T` alias (should be `%Total`)
* Adjust help for `log10` and `ln`
* Avoid pushing a null pointer on the stack on error evaluating constants
* Add missing `Ⓒ` sign in uncertainty for `Epl`
* Fix constant evaluation referencing `this` after it may have moved
* Report missing current directory in `Purge`
* Report errors when purging invalid objects in `Purge` and `PurgeAll`
* Avoid `ClearThingsMenu` in tests, the key maps to forward delete in editor

### Improvements

* keyboard: Update layout to include `σ` character
* Remove the `state/Test.48S` file, it's not longer relevant
* Add `constant` case to `object::is_negative`
* Add `DATE-` as an alias for `DDAYS`
* Add `PgAll` as an alias for `PurgeAll`
* More systematic purging of environment and variables between tests
* Add check in test suite that we actually purged special variables
* Refresh authors list
* Move documentation for `PPAR` to `plot.md`
* Avoid date conversions during constant evaluation
* Accelerate and improve the display of constant values in menus
* Add `launch.json` and `tasks.json` build configuration generated by Cursor
* Send a `START_TEST` command at start of test, refresh simulated battery
* Cursor-assisted review of missing command documentation
* Cursor-assisted generation of content in the Quickstart guide
* Cursor-assisted improvements to grammar and spelling
* Documentation and examples for `Blank` variants
* Documentation and examples for `Gray` and `RGB`
* Documentation and examples for `PMIN`, `PMAX`, `XRNG`, `YRNG`
* Documentation and examples for `Eq` and equations
* Add examples for list sum, product and difference commands

## Release 0.9.9 "Headrest" - Ranges

This release introduces four range types to perform computations on ranges, and
fixes serious graphical regressions that remained undetected by the test suite.

### New features

* Three range types, known as _interval_ (`1…3`), _delta range_ (`2±1`) and
  _percent range_ (`2±50%`), for which arithmetic and most operations have been
