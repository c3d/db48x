### System and user interface

- [x] Functional graphical simulator for the DMCP platform
- [x] Test suite on simulator
- [x] Main calculator loop, on/off, accessing DMCP Setup menu
- [x] RPL-style command-line text editor
- [x] Key processing
- [x] DM42-adapted shift logic
- [x] Virtual annunciators
- [x] Alpha mode and lowercases
- [x] Garbage collector
- [x] Low-memory stress testing
- [x] Built-in commands lookup and parsing
- [x] Soft menus navigation
- [x] Global variables
- [x] Local variables
- [x] Home directory and subdirectories
- [x] Variables menu accessible with **RCL** key
- [x] Complete interactive catalog of available functions
- [x] RPL program evaluattion
- [x] RPL program single-stepping / debugging
- [x] Stack undo / redo
- [x] Accelerated [blitter code with 8x8 patterns](https://github.com/c3d/db48x/blob/dm42/firmware/include/ggl.h)
- [x] Unicode-capable [text drawing replacement](https://github.com/c3d/db48x/blob/dm42/firmware/sys/graphics.c)
- [x] Unicode / UTF-8 support
- [x] [Unicode fonts](https://github.com/c3d/db48x/blob/dm42/firmware/include/unifont.h)
      (e.g. derived from [WP43S](https://gitlab.com/wpcalculators/wp43) project)
- [x] Larger font for editor (old geezer mode)
- [x] Objects in USB-accessible flash storage
- [x] Calculator state load/save
- [x] Global help from [generated markdown file](help/db48x.md)
- [x] [Per-function help](https://github.com/c3d/db48x/blob/dm42/firmware/hal_keyboard.c#L116)
      from [markdown files](doc/calc-help)
- [x] Character menu / table
- [x] Brezenham lines, circles and ellipses
- [ ] Polygon fill
- [x] RPL-controlled soft-menu system
- [x] Domain-specific features / customizaton from files on USB disk
- [x] Single-key object format cycling
- [x] Units and conversion customization from `units.csv` file


### Object system

- [x] Object system with compact storage and dynamic dispatch
- [x] Stack management
- [x] Convenient allocation, initialization and recycling of temporaries
- [x] Object evaluation
- [x] Object parsing
- [x] Object text rendering
- [x] Object graphical rendering
- [ ] Object graphical editor
- [x] Errors and error display
- [x] Error trapping / handling, e.g `DOERR`.

### RPL data types

- [x] Built-in commands (e.g. `Neg`)
- [x] Integer data type, e.g. `123`
- [x] Hexadecimal (`#123h`), decimal (`#123d`), octal (`#123o`) and binary
      (`#1001b`) based numbers
- [x] Based integers (arbitrary base between 2 and 36, e.g. `13#10231AC`)
- [x] Arbitrary-size integers e.g. `123456789012345678901234567890`
- [x] Fractions, e.g. `1/7`
- [x[ Fraction formats, e.g. `22/7` vs `3 1/7` vs. `3,1428571429`
- [x] Decimal floating-point with fixed size (32, 64 and 128 bits)
- [x] Decimal floating-point with arbitrary size
- [x] Binary floating-point with arbitrary size
- [x] Hardware-accelerated 32-bit and 64-bit binary floating-point
- [x] Text / strings, e.g. `"ABC"`
- [x] Name / symbols, e.g. `ABC`
- [x] Complex numbers in rectangular form, e.g. `2 + 3i`
- [x] Complex numbers in polar form, e.g. `2 ∡ 30°`
- [ ] Quaternions
- [ ] Intervals / ranges, e.g. `1..5`
- [x] Angles, e.g. `∡30°`
- [x] Lists, e.g. `{ A 1 "Hello" }`
- [x] Arrays, vectors, matrices and tensors, e.g. `[ [ 1 2 3] [ 4 5 6] ]`
- [x] Program objects, e.g `« IF 1 = THEN "Equal to one" END »`
- [ ] Algebraic programs, e.g. `'if x = 1 then "Equal to one"` based on
      [XL-style parser](https://github.com/c3d/XL)
- [x] Bitmaps, e.g. `GROB 8 8 00000000000000FF`
- [ ] Fonts, e.g. `FONT 000000`
- [ ] Vector graphics
- [x] Tagged objects
- [x] Algebraic expressions
- [x] Unit objects
- [x] Directories
- [x] Software libraries
- [x] Keymaps
- [x] Softkey menus
- [ ] Full-screen menus
- [ ] Forms
- [ ] Object handler (can replace eval, parse, render, graphical editor)
- [ ] HP41/HP42-style key-entry program
- [ ] ARM PIC code


### Basic Arithmetic

- [x] Integer arithmetic
- [x] Floating-point arithmetic
- [ ] Detection of precise vs. imprecise results, like newRPL
- [x] Automatic integer to floating-point promotion
- [x] Automatic selection of entered floating-point size based on user input
- [x] Automatic selection of computed floating-point size based on `Precision`
- [x] Fraction arithmetic
- [x] Automatic fraction promotion
- [ ] Correct rounding (a la [crlibm](https://github.com/taschini/crlibm))
- [ ] Range arithmetic


### Computations on real numbers

- [x] Floating-point arithmetic
- [x] Circular functions, e.g. `sin`, `cos`, `atan`, ...
- [x] Exponentials and logarithms, e.g. `exp`, `ln`, ...
- [x] Hyperbolic functions, e.g. `sinh`, `atanh`, ...
- [x] Probabilities functions, like `Factorial`, `Combinations`, etc.
- [x] Statistics functions, like `Σ+`, `Mean` or `Median`
- [ ] Special functions, like `Bessel`, `FastFourierTransforma`, etc
- [x] Constant tables with arbitrary precision from files on USB disk
- [x] Equation tables with explanatory graphics from files on USB disk
- [x] Unit conversions


### Computations on complex numbers

- [x] Complex arithmetic
- [x] Circular functions, e.g. `sin`, `cos`, `atan`, ...
- [x] Exponentials and logarithms, e.g. `exp`, `ln`, ...
- [x] Hyperbolic functions, e.g. `sinh`, `atanh`, ...
- [x] Probabilities functions, like `Factorial`, `Combinations`, etc.
- [x] Statistics functions, like `Σ+`, `Mean` or `Median`
- [ ] Special functions, like `Bessel`, `FastFourierTransforma`, etc
- [x] Equation tables with explanatory graphics from files on USB disk


### Computations on quaternions

- [ ] Arithmetic
- [ ] Circular functions, e.g. `sin`, `cos`, `atan`, ...
- [ ] Exponentials and logarithms, e.g. `exp`, `ln`, ...
- [ ] Hyperbolic functions, e.g. `sinh`, `atanh`, ...
- [ ] Probabilities functions, like `Factorial`, `Combinations`, etc.
- [ ] Statistics functions, like `Σ+`, `Mean` or `Median`
- [ ] Special functions, like `Bessel`, `FastFourierTransforma`, etc
- [ ] Equation tables with explanatory graphics from files on USB disk

### Computations on vectors, matrices and tensors

- [x] Arithmetic
- [x] Determinant, inverse, transpose
- [ ] Eigenvalues
- [ ] Tables with special matrices, e.g. Pauli matrices, General Relativity


### Computer Algebra system

- [x] Rendering of symbolic expressions
- [x] Parsing of symbolic expressions
- [x] Symbolic operations
- [ ] Integrate existing CAS like [GIAC](https://github.com/geogebra/giac)
- [x] Rules engine
- [x] Symbolic differentiation
- [x] Symbolic integration


### Numerical evaluation

- [x] Integration
- [x] Root finding

### Other RPL functional areas (to be expanded)

- [x] Real-time clock
- [ ] Alarms
- [x] File system access
- [ ] Printing
- [x] Plotting
- [ ] Equation writer
- [ ] Matrix writer
