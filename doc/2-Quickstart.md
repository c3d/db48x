# Quickstart guide

This quickstart guide will rapidly give you an overview of the capabilities of
DB48X, and show you how to use it efficiently. Make sure that you have
[installed the latest version](#installation).

The _ON_ / _EXIT_ button is at the bottom left of the calculator. It can be used
to power the calculator on, but also to exit operations, for example aborting a
data entry.

DB48X is a RPL calculator, which means that:

* It inherits the stack-based "reverse polish" approach to operations that has
  been a central feature of practically all Hewlett-Packard scientific
  calculators since the HP-35. You enter arguments to a functions by pushing
  them on the stack, and the operation removes its arguments from the stack
  before putting its result(s). Unlike earlier HP calculators, the RPL stack is
  practically unlimited.

![Stack and menus](img/stack-and-menus.bmp)

* Unlike simpler calculators, it uses a _command line_ for data entry, with
  advanced text editing capabilities, and a rich text-based command
  language. One way to access the hundreds of available commands is by simply
  typing their name.

![Command-line and catalog](img/command-line-and-catalog.bmp)


[![Long demo of 0.7.0](http://img.youtube.com/vi/kzkjE8BZW10/0.jpg)](https://www.youtube.com/watch?v=kzkjE8BZW10&list=PLz1qkflzABy-Cs1R07zGB8A9K5Yjolmlf "Long demo of v0.7.0")


## Arithmetic operations

Let's compute the area of a room made of a main section that is 6 meters by 8.3
meters, with an additional smaller section that is 3.5 meters by 2.8.

A first way to do it is to use the reverse polish stack-based approach, by
typing the following sequence of keys: _6_, _ENTER_, _8_, _._, _3_, _×_, _3_,
_._, _5_, _ENTER_, _2_, _._, _8_, _×_, _+_. The result, `59.6`, shows on the
stack. Prior to pressing the _+_ key, the intermediate results for the two
multiplications, `49.8` and `9.8`, could be seen on the stack.

<video src="https://github.com/c3d/db48x/assets/1695924/e185f3e8-dd36-4beb-a6c5-03bf489d91a7"></video>

RPL also supports the standard algebraic notation. Begin the computation with
the _'()_ key. The editor contains quotes, `''` with the cursor between
them. The cursor shows the letter `A`, indicating algebraic entry. Type _6_,
_×_, _8_, _._, _3_, _+_, _3_, _._, _5_, _×_, _2_, _._, _8_. At this point,
the text editor should show the whole expression, `'6·8.3+3.5·2.8'`
or `'6×8.3+3.5×2.8'`. Press `ENTER` and the expression shows up on the
stack. Hitting the _=_ / _EVAL_ / _SPC_ key (located between the _._ and _+_
keys) evaluates the expression, to get the result `59.6`.

<video src="https://github.com/c3d/db48x/assets/1695924/ba81f9f0-ec4d-4619-bf95-c56c14210fc3"></video>

Algebraic and reverse-polish computations are equivalent, and can be mixed and
matched during computations. Using one or the other is stricly a matter of
preference. Algebraic data entry makes it easier to view the entire
expression. Reverse-polish makes it easier to see intermediate results.


## Using Fractions

Let us now compute how many pies we need to feed 22 kids if we divide each pie
in 8 slices. Using the RPL approach, we would type _2_, _2_, _ENTER_, _8_,
_÷_. Using the algebraic notation, we would type _'()_, _2_, _2_, _÷_, _8_,
_ENTER_ and then use the _=_ to perform the computation.

<video src="https://github.com/c3d/db48x/assets/1695924/89ebbf7a-f331-4729-a1b9-1527287daa3e"></video>

With the default settings, you should see a mixed fraction, `2 ³/₄`. Unlike many
calculators, DB48X by default perform exact computations on fractions instead of
using approximate decimal numbers.

You can convert that fraction to a decimal value and back using the `Cycle`
command, which is bound to the _×10ⁿ_ key. A first press will show `2.75`, and a
second press will show the value again as fraction `2 ³/₄`.


## Mathematical functions

DB48X features a number of mathematical functions. Some of the functions are
directly available on the keyboard.

We can compute the length of the diagonal of a rectangle with sides 2m and 3m
using the Pythagorean theorem, and display it in millimeters.

<video src="https://github.com/c3d/db48x/assets/1695924/899ad5f3-fd0b-4695-86bb-0b682a191422"></video>

In RPL, we can type the following sequence: _2_, _x²_ (🟨 _C_), _3_, _x²_,
_+_, _√x_ (_C_), _1_, _0_, _0_, _0_, _×_. The decimal result,
`3 605.55127 546`, is shown on the stack. The digits in the whole part of the
decimal number are grouped 3 by 3, while the digits in the fractional part are
grouped 5 by 5.

In algebraic mode, we can type the following sequence:
_'()_, _1_, _0_, _0_, _0_, _×_, _√x_,
_2_, _x²_ (🟨 _C_), _+_, _3_, _x²_, _ENTER_. The mathematical
expresssion shows up on the stack graphically. It can then be evaluated using
the _=_ key, and shows the same result as for RPL mode.


## Mixing algebraic and reverse-polish operations

In the algebraic expression, we have multiplied by 1000 first, whereas in the
RPL case, we multiplied by 1000 last. We can also multiply by 1000 last in
algebraic mode. There are at least two ways to do it.

<video src="https://github.com/c3d/db48x/assets/1695924/88cb7865-87cb-427e-b18b-33086bcbabd5"></video>

A first method is to use the arrow key to exit the parentheses around the
argument of the square root function, as follows: _'()_, _√x_,
_2_, _x²_, _+_, _3_, _x²_, _▶︎_, _×_, _1_, _0_, _0_, _0_,
_ENTER_. The expression with the multiplication is then shown on the stack, and
can then be evaluated with the _=_ key.

A second method is to mix and match algebraic and RPL, by typing
the following sequence: _'()_, _√x_, _2_, _x²_, _+_,
_3_, _x²_, _ENTER_. At this point, the expression without the
multiplication is on the stack. We can then multiply it by 1000 by typing
_1_, _0_, _0_, _0_, _×_. The expression with the multiplication is then shown on
the stack, and can then be evaluated with the _=_ key.


## Trigonometric functions

Consider that we need to build a plank ramp. We can ask a number of questions,
like:

* If the plank is 5 meters in length, and the slope is 10 degrees, how high
  will it reach?

* If we need to reach 60 cm above ground, what is the required slope?

<video src="https://github.com/c3d/db48x/assets/1695924/a90b32c4-a903-4421-a768-c6b6b2afddec"></video>

In RPL, can answer the first question by typing _1_, _0_, _SIN_, _5_,
_×_. The result is shown in scientific format as `8.68240 88833 5×₁₀⁻¹`.
In algebraic mode, we would type _'()_, _5_, _×_, _SIN_, _1_, _0_, _ENTER_
and then evaluating the expression with _=_. This shows the same result.

We can answer the second question using RPL by typing _6_, _0_, _ENTER_, _5_,
_ENTER_, _1_, _0_, _0_, _×_, _÷_, _sin⁻¹_ (🟨 _J_). The result is shown as
`6.89210 25793 5 °`. This is an example of *unit object*: the value is
associated with a unit, in that case the `°` symbol indicating that we use
degrees. DB48X supports three other angle modes, radians, grads and fractions of
pi (piradians).

Answering the same question using algebraic mode introduces a new little
keyboard trick. Type _'()_,  _sin⁻¹_, _6_, _0_, _÷_, _'()_,
 _5_, _×_, _1_, _0_, _0_, _ENTER_, and then evaluating the expression with the
 _=_ key. Observe how the second use of the _'()_ key, which inserts parentheses
 when used inside an expression.


## Selecting display modes

The scientific notation may not be the most readable in that case. How do we
display this result with three digits? We will use a *menu* for that. Menus are
an essential component of the DB48X user interface.

<video src="https://github.com/c3d/db48x/assets/1695924/be997041-74f9-489b-9583-b94036b9dc33"></video>

Let us type 🟨 _O_ (_Disp_). This shows the `DisplayModesMenu`. The menu
occupies three rows at the bottom of the screen, with six columns. Menus can
give a quick access to 18 functions directly, six more with a single shift 🟨,
and yet another six with the second shift 🟦. Hitting the shift key 🟨
repeatedly will highlight the different rows of the menu.

On the lower row of the menu, the second entry from the left is labeled `Fix`.
The `Fix` display mode shows a fixed number of digits after the decimal
separator. There are other modes, `Sci` to display in scientific notation, `Eng`
to display with engineering multiples (the exponent is a multiple of three), and
`Sig` to display at most a given number of digits.

We can type _3_, _F2_, where _F2_ is the second key from the left on the top row
of the keyboard. This activates the `Fix 3` mode, which shows three digits after
the decimal separator. The display changes to `0.868` for the answer to the
first question, and `6.892 °` for the answer to the second question.


## Displaying the on-line help for a function

Since the number of available commands in DB48X is quite high, it is useful to
be able to consult the built-in help. In order to get help on a command, simply
hold the corresponding key until the help shows up. For instance, to get
[help about the command](#std) under the `Std` label, simply hold the _F1_ key.

This also works for normal keyboard operations. For instance, if you hold the
_SIN_ key, you will get the [help about the sine command](#sin).

<video src="https://github.com/c3d/db48x/assets/1695924/55d312a4-3977-421e-9cdf-65d8b5ff5036"></video>

You should refer to the on-line help whenever you have a question about a
specific command.


## Angle operations

The _sin⁻¹_ command we used previously returns an *angle* which was shown in
degrees, the default angle mode for DB48X. When applied to angles, the `Cycle`
command on the _×10ⁿ_ key cycles between various angle units: degrees, radians,
grads and pi-radians, i.e. a number of radians shown as a multiple of π.

<video src="https://github.com/c3d/db48x/assets/1695924/5d23f388-b034-45cd-9d4d-7685b7f211f0"></video>

The function also alternates between decimal and fractional representations of
angles.

In order to access angle-related functions, we can use the Tools key _⚙️_ which
invokes the `ToolsMenu` command. That command picks up a menu that is suited for
the value on the stack. For angles, this shows the `AnglesMenu`, which can be
used to perform angle conversions directly.

We can select the `→Deg` command to convert an angle to degrees by hitting the
🟨 _F1_ key while the `AnglesMenu` is active, and similarly for `→Rad` with
🟨 _F2_, and so on. To convert the angle to a Degrees/Minutes/Seconds (DMS)
representation, we can select the `→DMS` using the 🟦 _F1_ key, since that
function is on the second level of the menu.

There is a quick way to manually enter angles in DMS format by using the _._
more than once during data entry. For example, to enter 10°20′30″, you simply
need to type _1_, _0_, _._, _2_, _0_, _._, _3_, _0_, _ENTER_.

On the command-line, this shows up as `10°20′30_hms`. Once you hit the _ENTER_
key, this shows on the stack as `10°20′30″`.

Using _._ more while on the command-line lets you add fractions of a second, for
example _1_, _0_, _._, _2_, _0_, _._, _3_, _0_, _._, _4_, _0_, _._, _5_, _0_,
_ENTER_, which will show on the stack as `10°20′30″4/5`.

You can add or subtract angles directly using normal arithmetic functions. For
example, hitting the _+_ key will add angles, correctly adjusting the angle
units as necessary.


## Complex number operations

DB48X support complex numbers both in rectangular and polar (phasor) form.
For example, in our effort to build a plank ramp, we may need more than one
plank. How far and how high can you reach if you have a 5 meter plank with a
slope of 10 degrees, followed by a 3 meters plank with a slope of 30 degrees?

<video src="https://github.com/c3d/db48x/assets/1695924/a17d5404-ad05-4a4d-8c62-069f327b3428"></video>

We can add two complex numbers in phasor form to answer that question.
In order to enter the complex number representing the first plank, we need the
`ComplexMenu`, which is activated with the _CPLX_ key (🟨 _G_). The _F1_ key
will be used to enter complex numbers in rectangular form, and the _F2_ key to
enter complex numbers in polar form.

To solve our problem, we simply need to enter _CMPLX_ (🟨 _G_), then _5_, _F2_,
_1_, _0_, _ENTER_ to enter the first complex number. The stack shows the complex
value as `5∡10°`. We can enter the second complex number using _3_, _F2_, _3_,
_0_, and add the two values using the _+_ key. The result shows as
`7.522+2.368ⅈ`, which means that we can reach about 7.5 meters ahead and 2.3
meters above ground.


## Unit conversions

If you are living in the United States, having the results in meters might not
be convenient. You can use the DB48X built-in units in order to convert the
result above into feet, yards or inches.

<video src="https://github.com/c3d/db48x/assets/1695924/1fd54b22-5d1e-42bc-ac3a-2be5770422cf"></video>

Select the `UnitMenu` with 🟨 _5_. This shows a catalog of unit categories. We
can select the `Length` category using the _F4_ key. In order to indicate that
our result is in meters, we select the `m` unit by hitting _F1_. Our result now
shows as `7.522+2.368ⅈ m` We can then convert that result in yards by selecting
the `→yd` command with the 🟨 _F2_ key.

You can convert to other units in the `Length` units menu the same way. This
menu is too large to fit on the screen, so the _F6_ key can be use to select the
next page in the menu with more units, such as `in` or `mm`. Note that DB48X
does not have a `NXT` key unlike HP calculators. Instead, when necessary, the
`NXT` and `PREV` features appear in the menu itself as _F6_ and 🟨 _F6_.


## Operations on whole numbers

[![Demo of v0.4.8](https://img.youtube.com/vi/tT5az2CIcnk/maxresdefault.jpg)](https://www.youtube.com/watch?v=tT5az2CIcnk&list=PLz1qkflzABy-Cs1R07zGB8A9K5Yjolmlf)


### Entering whole numbers

### Arithmetic on integers

### Changing the sign of a number with +/-

### Exact division

### Computing on large numbers: 2^40, 25!

### Separators to make large numbers more readable

### Built-in functions: example of 1/x


## Using the shift key

### Primary function: 1/x

### First shifted function: y^x and square

### Second shifted function: Help

### The shift annunciator


## Invoking the on-line Help

### Long-press on a function key

### Moving up and down

### Following links

### Navigating back to a previous topic

### Exiting the on-line help

### Contextual help


## The annunciator area

### Battery level

### USB vs. battery power

### Showing or hiding the date and time

### Current state file name

### Future direction


## Decimal values

### Entering a decimal number

### Entering a number in scientific notation with _×10ⁿ_

### Arithmetic on decimal values

### Arithmetic on fractions

### Cycling between decimal and fraction with _×10ⁿ_

### Separators for the fractional part

### Live separators during number editing


## Soft keys and menus

### Soft keys

### The DISP menu

### Effect of shift state on the menu

### Submenus

### Menu history (Last Menu)


## Displaying decimal values

### Standard display mode

### FIX display mode

### Switching to scientific mode

### Digits to show for small values

### SCI display mode

### ENG display mode

### SIG display mode

### Emulating HP48 standard display


## Scientific functions

### Square and power

### Square root and xroot

### Exponential and Log

### Exponential and log in base 10

### DM42 layout difference: EXP LN instead of LOG LN

### Trigonometric functions and their inverse

### Functions in menus: example of hyperbolic functions


## Using an infinite stack

### Showing multiple stack levels

### Result vs. other levels

### When a result is too large


### An example of complicated calculation - The Mach number benchmark

### How to proceeed with that computation

### Correcting an error in the middle

### Saving results for later with Duplicate

### Dropping results and cleaning up with Drop

### LastArg to recall last arguments

### Undo to restore previous stack state


## The command line

### Editing an object on the stack with Right key

### Moving left and right on the command line

### Repeating keys: Insert, left, right, delete

### Inserting characters in the middle

### Deleting characters left and right

### Space key on R/S

### Command line: entering three numbers at once


## The editor menu

### Selecting the editor menu

### Moving word by word

### Moving to beginning and end

### Selecting text

### Cut, copy and paste

### Incremental search

### Search and replace


## Command line history

### Recalling a previous command line

### Optimization of command-line space

### Exiting the command line

## Entering letters and symbols

### Alpha mode with Shift Enter

### Alpha mode with Long Shift

### Transient Alpha mode, upper and lowercase

### Shift on digits and operations while in Alpha mode

### Shifted characters

### 2nd shifted characters

### White cursor for Alpha mode

### C and L cursor indicators in text


## Entering names

### Executing a command by typing its name

### Catalog with + key

### Auto-completion

### Example: VERSION

### What happens if the name is not a command


## Multi-line text editor

### Multi-line Text editor

### Up and down by shifting

### Repeat up and down by holding key


## Entering text

### Entering text with 2nd shift ENTER

### The C and L cursors

### Mixed operations, e.g. adding text

### Multiplying text by a number


## Entering an algebraic expression

### The `' ()` key

### Entering an expression

### Evaluating an expression with `=`

### Cursor in algebraic mode

### Comparing the `sin` key in direct and algebraic mode

### Entering parentheses

### Automatic elimination of parentheses

### Symbolic algebraic expressions

### Performing RPN operations on algebraic expressions

### Automatic simplification of `0+x`, `1*x`, etc.


## The Tools menu

### Tools menu on empty stack

### Tools menu for a decimal value

### Tools menu for an integer

### Tools menu for a text

### Tools menu for an expression


## Computations on complex numbers

### The complex menu

### Entering numbers in rectangular form

### Entering numbers in polar form

### Switching between polar and rectangular with Cycle key

### Arithmetic on complex numbers

### Exact angles and exact computations: 2<45 * 3<90 ^ 8

### Functions on complex numbers, e.g. `sin` and `log`.

### Effect of angle mode on display in polar form


## Computations on vectors

### Entering a vector

### The M cursor

### Adding and subtracting vectors

### Component-wise multiplication and division

### Operations between vector and a constant

### Component-wise functions: 1/x

### The tools menu on vectors

### Computing the norm of a vector

### The Matrix menu


## Computations on matrices

### Entering a matrix

### Adding and subtracting matrices

### Multiplication and division by a constant

### Multiplying square matrices

### Multiplying a matrix and a vector

### Computing a determinant

### Computing an inverse with 1/x


## Advanced matrix operations

### Matrix of complex numbers

### Symbolic matrix

### Inverse and determinant of 2x2 symbolic matrix


## Entering data in lists

### Entering a list

### Adding elements to a list

### Applying a function to a list

### Repeating a list (multiply)

### Lists containing lists


## Computations with based numbers

### Entering based numbers

### Entering hexadecimal directly with A-F

### Logical operations

### Setting the word size

### Changing to common bases (2, 8, 10, 16)

### Changing to an arbitray base

### Entering number in arbitrary base

### The tools menu on based number

### Binary operations

### Emulating a 16-bit or 256-bit CPU

### The Cycle key on based numbers

### Adding a suffix to force a base (DM32 only)


## Unit objects

### Entering a value with a unit

### The units menus

### Applying a unit

### Converting to a unit

### Dividing by a unit



## Entering a program

### Computing a VAT

### Evaluating a program with `Evaluate`

### Modifying a program with LastArg

### Modifying a program with Undo

### Modifying a program with command-line history

### The three roles of the R/S key: Space, =, EVAL


## Storing values in global variables

### Storing a value in a new variable 'VATRate'

### Evaluating a variable

### Case insensitivity

### Naming a variable on the command line

### Using quotes to avoid evaluation

### Overwriting a variable value

### Expressions containing variables


## Storing and modifying programs

### Creating a new `VAT` command

### Evaluating a program by name

### Evaluting a program from variables menu

### Taking input and computing output


## The variables menu

### Showing the variables menu

### Evaluating a variable with F1

### Recalling a variable with shift F1

### Storing in an existing variable with xshift F1

### Rationale for the difference with HP48

### Using variables menu while editing a program


## Menus with too many entries

### Adding more variables overflows

### Going from 6 to 7 entries

### No next key, using F6 and shift F6 for next and previous


## Saving your state to disk

### The system menu

### Saving the calculator state

### Restoring another state

### Merging states

### Returning to the calculator

### Saving state quickly with xshift-EXIT


## Plotting a function

### Plotting a wave function sin(x * a) * cos(x * b)

### Plotting a polar function

### Plotting a parameteric function

### Drawing two functions on the same screen

### Changing line width

### Changing line patterm


## The numerical solver

### Solving an equation

### Expressions that must be zero

### Equations A=B

### Solving for different variables


## Numerical integration

### Integrating x^2 from 0 to 1 (exact results)

### What happens with 0.0 to 1.0

### Integration 1/x from 2 to 22

### Comparing with LN(2) - LN(2)


## Symbolic expression manipulation

### Collecting terms

### Expanding terms

### General expression rewriting facility


## Local variables

### Why use local variables

### Inserting local variables in a program

### Inserting local variables in equations


## Localized number display preferences

### Changing the decimal separator

### Changing the spacing for numbers

### Changing the character used for spacing


## User interface preferences

### Square and rounded menu styles

### 3-level, 1-level and flat menu styles

### Changing result font size

### Changing stack font size

### Changing editor font size

### Changing multi-line editor font size



## Comparisons and tests

### Truth: True, False, 0, 1

### Equality tests

### Differences between = and ==

### Relational operators

### Logical operations (AND, OR, NOT)


## More sophisticated programming

### Testing with IF THEN ELSE END

### Conditional expression with IFTE

### Counted loop with START NEXT

### Stepping loop with START STEP

### Named loop with FOR NEXT

### Named loop with FOR STEP

### WHILE conditional loop

### UNTIL conditional loop


## Enjoy the calculator!



<!-- ====================================================================== -->
<!--                                                                        -->
<!--   Installation guide                                                   -->
<!--                                                                        -->
<!-- ====================================================================== -->

## Installation

[![Installation process](https://img.youtube.com/vi/rVWy4N0lBOI/maxresdefault.jpg)](https://www.youtube.com/watch?v=rVWy4N0lBOI&list=PLz1qkflzABy-Cs1R07zGB8A9K5Yjolmlf)


### Downloading the software

You can download pre-built versions of the firmware from the releases page of
the project (https://github.com/c3d/db48x/releases), or alternatively,
you can download the source code and build it yourself.

<!--- DM42 --->
The pre-built firmware for the DM-42 is split into two components, `db48x.pgm`
and `db48x_qspi.bin`. The built-in help is stored in a file called `db48x.md`.
<!--- !DM42 --->

<!--- DM32 --->
The pre-built firmware for the DM-32 is split into two components, `db50x.pg5`
and `db50x_qspi.bin`. The built-in help is stored in a file called `db50x.md`.
<!--- !DM32 --->

In addition, a file called `Demo.48s` contains a few sample RPL programs to
illustrate the capabilities of this new firmware, two comma-separated values
files `units.csv` and `constants.csv`, which define the units and constants
respectively.

### Connecting the calculator to a computer

<!--- DM42 --->
The DM-42 calculator connects to your computer using a standard micro-USB cable.
<!--- !DM42 --->

<!--- DM32 --->
The DM-32 calculator connects to your computer using a standard USB-C cable.
<!--- !DM32 --->


### System menu

The `Setup` menu is displayed by using 🟨 _0_. This key combination is the same
on the stock DM42 firmware and on the new DB48X firmware, and it contains
similar entries. However, the setup menu entries are not necessarily in the same
order.

On the stock firmware, you need to successively select:

* `System`

* `Enter System Menu`

* `Reset to DMCP menu`

On the DB48X firmware, the required options are both directly available from the
`Setup` menu.


### Exposing internal storage as a USB disk

The `Activate USB Disk` option enables the calculator's USB disk mode, and
exposes 6MB of its internal storage as a regular flash disk that you can access
from your computer as an external disk.


### Copying DB48X installation files

The files should be copied as follows:

<!--- DM42 --->
* `db48x.pgm` and `db48x_qspi.bin` in the root directory of the calculator's USB
  disk.

* `db48x.md` should be placed in a directory called `help`.
<!--- !DM42 --->

<!--- DM32 --->
* `db50x.pg5` and `db50x_qspi.bin` in the root directory of the calculator's USB
  disk.

* `db50x.md` should be placed in a directory called `help`.
<!--- !DM32 --->

* `units.csv` and `constants.csv` should be placed in a directory called
  `config`. You can customize these files to add your own [units](#units) and
  [constants](#constants).


### Copying DM42 installation files

Refer to the SwissMicros installation instructions to install or reinstall the
original calculator firmware.


### Installing the DB48X QSPI file

To install the QSPI file, [select the system menu](#system-menu) and then select
the `Load QSPI from FAT` menu entry.

The `QSPI` in the menu label refers to the file ending with `_qspi.bin`. When
upgrading, you should load the new QSPI file first, and only then load the
program file.


### Installing the DB48X program file

To install the program file file, [select the system menu](#system-menu) and
then select the `Load program` menu entry.

After loading the DB48X program, the firmware loaded asks you to press a key,
and the new firmware automatically runs.


## Switching between DM42 and DB48X

Early releases of the DB48X firmware produced a QSPI image file that was capable
of running the stock DM42 program file. Unfortunately, this is no longer the
case due to space constraints.

Unfortunately, the installation procedure for the QSPI file erases the file
from the flash storage. This makes it relatively inconvenient to switch back and
forth between DB48X and original firmware, since that requires copying the
`_qspi.bin` file from your computer every time.


### Saving and restoring DB48X state

The DB48X `Setup` menu is displayed by using 🟨 _0_. It contains a `State` menu
entry to manage the DB48X state, which includes variables, programs and
preferences.

The `State` submenu includes the following options:

* `Load state`
* `Save state`
* `Clear state`
* `Merge state`
* `Activate USB Disk`
* `Show Disk Info`
