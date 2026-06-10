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
  before putting its result(s). In contrast to earlier HP calculators, the RPL
  stack is practically unlimited.

![Stack and menus](img/stack-and-menus.bmp)

* Unlike simpler earlier calculators, DB48x uses a _command line_ for data
  entry, with advanced text editing capabilities, and a rich text-based command
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
matched during computations. Using one or the other is strictly a matter of
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
expression shows up on the stack graphically. It can then be evaluated using
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

On the command-line, this shows up as `10°20′30_dms`. Once you hit the _ENTER_
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

DB48X supports exact arithmetic on whole numbers, allowing you to perform calculations with arbitrary precision without rounding errors.

### Entering whole numbers

Whole numbers are entered directly using the numeric keys. For example, type _1_, _2_, _3_, _ENTER_ to enter the number 123. Large numbers can be entered without any special notation - DB48X handles them automatically.

### Arithmetic on integers

Basic arithmetic operations work exactly as you'd expect. Type _1_, _2_, _3_, _ENTER_, _4_, _5_, _6_, _+_ to add 123 and 456, giving you 579. The result is always exact when working with integers.

### Changing the sign of a number with +/-

Use the _+/-_ key to change the sign of a number. Type _1_, _2_, _3_, _ENTER_, then _+/-_ to get -123. This works on any number type, including integers, fractions, and decimals.

### Exact division

When you divide two integers, DB48X creates a fraction by default. Type _1_, _0_, _ENTER_, _3_, _÷_ to get `3¹/₃` (3 and 1/3). This preserves exact arithmetic without rounding errors.

### Computing on large numbers: 2^40, 25!

DB48X can handle very large numbers. Type _2_, _ENTER_, _4_, _0_, _↑_ to compute 2^40, which gives you 1,099,511,627,776. For factorials, type _2_, _5_, _ENTER_, then use the factorial function (🟨 _F3_ in the Math menu) to compute 25!, which is approximately 1.551121×10^25.

### Separators to make large numbers more readable

Large numbers are automatically formatted with separators for readability. The number 1,099,511,627,776 is displayed with commas or spaces separating groups of digits, making it easier to read.

### Built-in functions: example of 1/x

Many mathematical functions work on integers. Type _2_, _ENTER_, then _1/x_ (🟨 _F1_) to get `¹/₂` (1/2). This maintains exact arithmetic by creating fractions rather than decimal approximations.


## Using the shift key

The shift key on DB48X provides access to multiple functions on each key. Understanding how the shift key works is essential for efficient use of the calculator.

### Primary function: 1/x

The primary function is accessed without pressing the shift key. For example, the _1/x_ key directly performs the reciprocal operation. Type _2_, _ENTER_, then _1/x_ to get `¹/₂`.

### First shifted function: y^x and square

Press the shift key once (🟨) to access the first shifted function. For example, with the _1/x_ key, pressing 🟨 _1/x_ gives you the power function (y^x). Type _2_, _ENTER_, _3_, then 🟨 _1/x_ to compute 2^3 = 8.

### Second shifted function: Help

Press the shift key twice (🟦) to access the second shifted function. With the _1/x_ key, pressing 🟦 _1/x_ provides help about the reciprocal function. This is a quick way to get context-sensitive help for any function.

### The shift annunciator

The current shift state is displayed in the annunciator area at the top of the screen. You'll see indicators for:
 * No shift: Normal operation
 * 🟨: First shift (yellow functions)
 * 🟦: Second shift (blue functions)

This helps you keep track of which shift level is active and what functions are available.


## Invoking the on-line Help

DB48X provides comprehensive built-in help that can be accessed in multiple ways. The help system is context-sensitive and provides detailed information about functions, commands, and features.

### Long-press on a function key

The easiest way to get help is to long-press any function key. Hold down a key for about half a second, and the help for that function will appear. For example, long-press the _SIN_ key to see help about the sine function.

### Moving up and down

Once in the help system, use the arrow keys _◀︎_ and _▶︎_ to navigate through the help text. These keys move the cursor up and down through the help content, allowing you to read through longer help entries.

### Following links

Help entries often contain links to related topics. Use the _ENTER_ key to follow a link, which will take you to the help for that related function or concept. This creates a web-like navigation experience within the help system.

### Navigating back to a previous topic

Use the _EXIT_ key to go back to the previous help topic you were viewing. This allows you to retrace your steps through the help system and return to earlier topics.

### Exiting the on-line help

Press _EXIT_ when you're done reading help to return to the calculator's normal operation. The help system preserves your current stack state, so you can continue your calculations exactly where you left off.

### Contextual help

The help system is context-sensitive, meaning it provides relevant information based on what you're currently doing. For example, if you're working with complex numbers, the help will include information about complex number operations.


## The annunciator area

The annunciator area at the top of the screen provides important status information about the calculator's current state and operation.

### Battery level

The battery level is displayed as a battery icon in the annunciator area. The icon shows the current charge level and indicates when the battery is low. When the battery is critically low, a warning will appear.

### USB vs. battery power

When the calculator is connected to USB power, a USB icon appears in the annunciator area. This indicates that the calculator is being powered by USB rather than the internal battery, which can affect performance and power management.

### Showing or hiding the date and time

The current date and time are displayed in the annunciator area by default. You can hide this information using the settings menu if you prefer a cleaner display. The time format can also be customized.

### Current state file name

When a state file is loaded, its name appears in the annunciator area. This helps you keep track of which state file is currently active, especially when working with multiple saved states.

### Future direction

The annunciator area is designed to be expandable, with plans to include additional status indicators such as:
 * Current angle mode (degrees, radians, grads)
 * Active flags and settings
 * Memory usage indicators
 * Network connectivity status (if applicable)


## Decimal values

DB48X supports decimal arithmetic with high precision, allowing you to work with real numbers efficiently while maintaining accuracy.

### Entering a decimal number

To enter a decimal number, type the digits, press the decimal point key _._, then continue with the fractional digits. For example, type _3_, _._, _1_, _4_, _1_, _5_, _9_, _ENTER_ to enter π as a decimal approximation.

### Entering a number in scientific notation with _×10ⁿ_

For very large or small numbers, use scientific notation. Type the mantissa, then press _×10ⁿ_ (🟨 _F4_), then enter the exponent. For example, type _1_, _._, _5_, _×10ⁿ_, _2_, _3_, _ENTER_ to enter 1.5×10^23.

### Arithmetic on decimal values

Decimal arithmetic works exactly like integer arithmetic. Type _3_, _._, _1_, _4_, _ENTER_, _2_, _._, _7_, _1_, _+_ to add 3.14 and 2.71, giving you 5.85. All standard arithmetic operations are supported.

### Arithmetic on fractions

Fractions are handled automatically. Type _1_, _ENTER_, _3_, _÷_ to get `¹/₃`, then _2_, _×_ to multiply by 2, giving you `²/₃`. DB48X maintains exact arithmetic with fractions.

### Cycling between decimal and fraction with _×10ⁿ_

Use the _×10ⁿ_ key to cycle between different representations of the same number. For example, with `²/₃` on the stack, press _×10ⁿ_ to convert to decimal (0.666666666667), press it again to return to fraction form.

### Separators for the fractional part

Large decimal numbers are automatically formatted with separators for readability. For example, 1234567.890123 is displayed as 1,234,567.890123, making it easier to read.

### Live separators during number editing

When entering numbers, separators appear in real-time as you type, helping you keep track of the number's magnitude. This is especially useful when entering very large or very small numbers.


## Soft keys and menus

DB48X uses a menu system with soft keys that provide quick access to functions and commands. Understanding how to navigate menus efficiently is key to using the calculator effectively.

### Soft keys

The six function keys (_F1_ through _F6_) at the top of the keyboard are soft keys that change their function based on the current menu. The labels for these keys appear on the screen above each key, showing you what function each key will perform.

### The DISP menu

Press 🟨 _O_ to access the Display menu (DISP). This menu provides quick access to display mode settings like FIX, SCI, ENG, and SIG. Each soft key corresponds to a different display mode, making it easy to switch between them.

### Effect of shift state on the menu

The shift state affects which menu is displayed. Pressing 🟨 cycles through different menu levels, each with different functions available on the soft keys. This multiplies the number of functions accessible through the menu system.

### Submenus

Many menus have submenus that provide additional functions. For example, the DISP menu might have submenus for different categories of display settings. Use the arrow keys or soft keys to navigate between submenus.

### Menu history (Last Menu)

DB48X remembers the last menu you were using. You can quickly return to the previous menu using the _EXIT_ key or by pressing the appropriate menu key again. This makes it easy to switch between related functions.


## Displaying decimal values

DB48X offers multiple display modes to suit different types of calculations and user preferences. Each mode has specific advantages for different scenarios.

### Standard display mode

Standard mode (STD) displays numbers using full precision. All significant digits are shown, up to the calculator's maximum precision. This is the default mode and is ideal for most calculations.

### FIX display mode

Fixed mode (FIX) displays a specified number of decimal places. Press _3_, _FIX_ to show 3 decimal places. For example, π displays as 3.142. This mode is useful when you need consistent decimal precision.

### Switching to scientific mode

Scientific mode (SCI) displays numbers in scientific notation with a specified number of decimal places. Press _3_, _SCI_ to show 3 decimal places in scientific notation. For example, 1234.567 displays as 1.235×10^3.

### Digits to show for small values

The minimum significant digits setting controls when to switch to scientific notation for small values. With 3 minimum digits, 0.00123 displays as 1.23×10^-3 instead of 0.001.

### SCI display mode

Scientific mode is ideal for very large or very small numbers. It automatically adjusts the exponent to keep the mantissa between 1 and 10, making it easy to compare magnitudes.

### ENG display mode

Engineering mode (ENG) is similar to scientific mode but uses exponents that are multiples of 3. This makes it easier to work with engineering units like kilo, mega, milli, etc.

### SIG display mode

Significant digits mode (SIG) shows a specified number of significant digits. Press _4_, _SIG_ to show 4 significant digits. For example, 123.456 displays as 123.5.

### Emulating HP48 standard display

DB48X can emulate the HP48's standard display mode by setting significant digits to 12. This provides compatibility with HP48 programs and user expectations.


## Scientific functions

DB48X provides a comprehensive set of scientific functions for mathematical calculations. These functions are accessible both directly from the keyboard and through menus.

### Square and power

Use _x²_ (🟨 _C_) to square a number. Type _5_, _ENTER_, then _x²_ to get 25. For other powers, use _↑_ (🟨 _1/x_). Type _2_, _ENTER_, _3_, _↑_ to compute 2^3 = 8.

### Square root and xroot

Use _√x_ (_C_) to compute the square root. Type _1_, _6_, _ENTER_, then _√x_ to get 4. For other roots, use _xroot_. Type _8_, _ENTER_, _3_, _xroot_ to compute the cube root of 8 = 2.

### Exponential and Log

Use _EXP_ (_E_) to compute e^x. Type _1_, _ENTER_, then _EXP_ to get e ≈ 2.718. Use _LN_ (_L_) for the natural logarithm. Type _2_, _ENTER_, then _LN_ to get ln(2) ≈ 0.693.

### Exponential and log in base 10

Use _EXP10_ (🟨 _E_) to compute 10^x. Type _2_, _ENTER_, then _EXP10_ to get 100. Use _LOG10_ (🟨 _L_) for the base-10 logarithm. Type _1_, _0_, _0_, _ENTER_, then _LOG10_ to get 2.

### DM42 layout difference: EXP LN instead of LOG LN

The DB48X layout uses EXP and LN keys instead of the traditional LOG and LN arrangement. This provides more direct access to the most commonly used exponential and logarithmic functions.

### Trigonometric functions and their inverse

Use _SIN_, _COS_, and _TAN_ for trigonometric functions. Type _3_, _0_, _ENTER_, then _SIN_ to get sin(30°) = 0.5. For inverse functions, use _sin⁻¹_ (🟨 _J_), _cos⁻¹_ (🟨 _K_), and _tan⁻¹_ (🟨 _L_).

### Functions in menus: example of hyperbolic functions

Hyperbolic functions are available through menus. Press _MATH_ (🟨 _F3_) to access the Math menu, then use the soft keys to access hyperbolic functions like sinh, cosh, and tanh.


## Using an infinite stack

Unlike traditional calculators with limited stack depth, DB48X provides an unlimited stack that can hold as many values as memory allows. This enables complex calculations and program execution.

### Showing multiple stack levels

The stack display shows multiple levels simultaneously. You can see the top few levels of the stack at all times, making it easy to track your calculations. Use the arrow keys to scroll through more stack levels if needed.

### Result vs. other levels

The result of a calculation appears in level 1 (the top of the stack), while the previous values are pushed down. This allows you to see both the result and the values that led to it, making it easier to verify your work.

### When a result is too large

When a result is too large to display completely, DB48X shows a truncated version with an indicator. You can use the _Show_ command to view the complete result in a full-screen display.

### An example of complicated calculation - The Mach number benchmark

The Mach number calculation demonstrates the power of the infinite stack. This complex calculation involves multiple intermediate results that can all be kept on the stack simultaneously.

### How to proceed with that computation

For complex calculations, work step by step, keeping intermediate results on the stack. Use _Duplicate_ to copy values you'll need later, and use _Drop_ to remove values you no longer need.

### Correcting an error in the middle

If you make an error in the middle of a calculation, you can use _Undo_ to restore the previous stack state. This is much more convenient than starting over from the beginning.

### Saving results for later with Duplicate

Use _Duplicate_ (🟨 _ENTER_) to copy the top value on the stack. This is useful when you need a value for multiple calculations or want to preserve it for later use.

### Dropping results and cleaning up with Drop

Use _Drop_ (_EXIT_) to remove the top value from the stack. This helps keep the stack organized and prevents it from becoming cluttered with intermediate results.

### LastArg to recall last arguments

Use _LastArg_ (🟦 _ENTER_) to recall the arguments of the last operation. This is useful when you want to repeat a calculation with different parameters.

### Undo to restore previous stack state

Use _Undo_ (🟦 _EXIT_) to restore the stack to its previous state. This is invaluable for recovering from mistakes or exploring different calculation paths.


## The command line

The command line in DB48X provides powerful text editing capabilities for entering expressions, commands, and data. Understanding how to use it effectively will greatly improve your productivity.

### Editing an object on the stack with Right key

Press the _Right_ arrow key to edit the top object on the stack. This opens the command line with the object's text representation, allowing you to modify it before re-entering it.

### Moving left and right on the command line

Use the _Left_ and _Right_ arrow keys to move the cursor within the command line. This allows you to position the cursor anywhere in the text for editing or insertion.

### Repeating keys: Insert, left, right, delete

Hold down keys like _Insert_, _Left_, _Right_, or _Delete_ to repeat their actions. This is useful for moving quickly through text or deleting multiple characters at once.

### Inserting characters in the middle

Position the cursor where you want to insert text, then type the new characters. They will be inserted at the cursor position, pushing existing text to the right.

### Deleting characters left and right

Use _Delete_ to remove the character to the right of the cursor, or _Backspace_ to remove the character to the left. This allows precise editing of text.

### Space key on R/S

The _Space_ key (R/S) can be used to insert spaces in the command line. This is useful when entering expressions or commands that require spacing.

### Command line: entering three numbers at once

You can enter multiple numbers on a single command line by separating them with spaces. For example, type "1 2 3" and press _ENTER_ to enter three numbers at once.


## The editor menu

The editor menu provides advanced text editing capabilities that go beyond basic command line editing. These features are essential for working with complex expressions and programs.

### Selecting the editor menu

Press _EDIT_ (🟨 _F6_) to access the editor menu. This provides access to advanced editing functions like text selection, cut/copy/paste, and search operations.

### Moving word by word

Use _Word Left_ and _Word Right_ to move the cursor word by word instead of character by character. This makes navigation much faster when editing long expressions or text.

### Moving to beginning and end

Use _Home_ and _End_ keys to jump to the beginning or end of the current line. This is useful for quickly positioning the cursor at the start or end of text.

### Selecting text

Hold down _Shift_ while using arrow keys to select text. Selected text is highlighted, allowing you to perform operations on the entire selection.

### Cut, copy and paste

Use _Cut_ to remove selected text and store it in the clipboard, _Copy_ to duplicate selected text, and _Paste_ to insert clipboard content at the cursor position.

### Incremental search

Use _Find_ to search for text within the current line. The search is incremental, meaning it finds matches as you type, making it easy to locate specific text quickly.

### Search and replace

Use _Replace_ to find and replace text. You can replace individual occurrences or all occurrences at once, making bulk editing operations efficient.


## Command line history

DB48X maintains a history of your command line entries, making it easy to recall and reuse previous commands or expressions.

### Recalling a previous command line

Use the _Up_ arrow key to recall previous command line entries. Each press of _Up_ goes further back in your history. Use _Down_ to move forward in the history.

### Optimization of command-line space

The command line history is optimized to use memory efficiently. Older entries may be automatically removed to preserve memory for current calculations and programs.

### Exiting the command line

Press _EXIT_ to cancel the current command line entry and return to normal calculator operation. The current entry is discarded and not added to history.

## Entering letters and symbols

DB48X provides multiple ways to enter letters and symbols, making it easy to work with text, variable names, and symbolic expressions.

### Alpha mode with Shift Enter

Press _Shift_ + _ENTER_ to enter Alpha mode. In this mode, the numeric keys produce letters instead of numbers. For example, pressing _2_ produces 'A', _3_ produces 'D', etc.

### Alpha mode with Long Shift

Hold down _Shift_ for a longer time to enter a temporary Alpha mode. This allows you to enter a few letters without permanently switching to Alpha mode.

### Transient Alpha mode, upper and lowercase

Use _Shift_ + _Alpha_ to toggle between uppercase and lowercase letters. This is useful when entering variable names that follow different naming conventions.

### Shift on digits and operations while in Alpha mode

While in Alpha mode, you can still access numbers and operations by using the shift key. This allows you to mix letters and numbers in your input.

### Shifted characters

Many keys have shifted characters available. For example, pressing _Shift_ + _1_ might produce '!', _Shift_ + _2_ might produce '@', etc.

### 2nd shifted characters

Some keys have second-level shifted characters accessed with the second shift key. These provide additional symbols and special characters.

### White cursor for Alpha mode

When in Alpha mode, the cursor changes to white to indicate that you're entering text rather than numbers. This visual feedback helps you know the current input mode.

### C and L cursor indicators in text

The cursor shows different indicators when editing text:
 * 'C' indicates command mode
 * 'L' indicates line editing mode

These indicators help you understand the current editing context and available operations.


## Entering names

DB48X allows you to execute commands by typing their names directly. This provides quick access to hundreds of functions without memorizing key combinations.

### Executing a command by typing its name

Simply type the name of any command and press _ENTER_ to execute it. For example, type "SIN" and press _ENTER_ to execute the sine function on the top value of the stack.

### Catalog with + key

Press _+_ to access the catalog of all available commands. This shows a searchable list of commands, making it easy to find and execute functions you might not remember.

### Auto-completion

As you type command names, DB48X provides auto-completion suggestions. This helps you find the right command quickly and reduces typing errors.

### Example: VERSION

Type "VERSION" and press _ENTER_ to display the current version of DB48X. This is a simple example of how command names work.

### What happens if the name is not a command

If you type a name that doesn't correspond to a command, DB48X will create a variable with that name and store the current stack value in it. This is useful for creating custom variables.


## Multi-line text editor

DB48X includes a powerful multi-line text editor for working with longer text, programs, and complex expressions.

### Multi-line Text editor

Press _EDIT_ (🟨 _F6_) to access the multi-line editor. This provides a full-screen editing environment for working with text that spans multiple lines.

### Up and down by shifting

Use the _Up_ and _Down_ arrow keys to navigate between lines in the multi-line editor. This allows you to move through longer documents efficiently.

### Repeat up and down by holding key

Hold down the _Up_ or _Down_ arrow keys to scroll through multiple lines quickly. This is useful for navigating through long programs or text documents.


## Entering text

DB48X provides comprehensive text handling capabilities, allowing you to work with strings, labels, and text-based data.

### Entering text with 2nd shift ENTER

Press 🟦 _ENTER_ to enter text mode. This allows you to type text that will be treated as a string object rather than a command or expression.

### The C and L cursors

When editing text, the cursor shows different indicators:
 * 'C' indicates you're in command mode
 * 'L' indicates you're in line editing mode

These help you understand the current editing context.

### Mixed operations, e.g. adding text

You can perform operations on text objects. For example, you can concatenate strings by adding them together, or extract substrings using various text functions.

### Multiplying text by a number

You can repeat text by multiplying it by a number. For example, if you have the text "Hello" and multiply it by 3, you get "HelloHelloHello".


## Entering an algebraic expression

DB48X supports algebraic notation, allowing you to enter mathematical expressions in a natural, readable format similar to traditional mathematical notation.

### The `' ()` key

Press the _'()_ key to begin entering an algebraic expression. This opens the algebraic editor with quotes and parentheses, ready for you to type your expression.

### Entering an expression

Type your mathematical expression using standard notation. For example, type _2_, _×_, _3_, _+_, _4_ to create the expression `'2×3+4'`. The expression appears between quotes to indicate it's algebraic.

### Evaluating an expression with `=`

Press the _=_ key (or _SPC_ or _EVAL_) to evaluate the algebraic expression. The result appears on the stack, and the expression is replaced by its computed value.

### Cursor in algebraic mode

When in algebraic mode, the cursor shows an 'A' indicator, letting you know you're entering an algebraic expression rather than RPN operations.

### Comparing the `sin` key in direct and algebraic mode

In direct mode, pressing _SIN_ immediately computes the sine of the top stack value. In algebraic mode, pressing _SIN_ inserts the sine function into your expression, which is evaluated later when you press _=_.

### Entering parentheses

Use the _'()_ key to insert parentheses within an algebraic expression. This allows you to control the order of operations and create complex nested expressions.

### Automatic elimination of parentheses

DB48X automatically simplifies expressions by removing unnecessary parentheses. For example, `'((2+3))'` becomes `'(2+3)'` when you press _ENTER_.

### Symbolic algebraic expressions

Algebraic expressions can contain variables and symbolic elements. For example, you can enter `'x^2 + 2*x + 1'` as a symbolic expression that can be manipulated or evaluated for specific values of x.

### Performing RPN operations on algebraic expressions

You can perform RPN operations on algebraic expressions. For example, you can add two expressions together, or apply functions to expressions before evaluating them.

### Automatic simplification of `0+x`, `1*x`, etc.

DB48X automatically simplifies expressions by removing trivial operations. For example, `'0+x'` becomes `'x'`, and `'1*x'` becomes `'x'`. This makes expressions cleaner and more readable.


## The Tools menu

The Tools menu (⚙️) provides context-sensitive access to functions and operations that are relevant to the current data type on the stack.

### Tools menu on empty stack

When the stack is empty, the Tools menu shows general system functions like settings, help, and system information. This provides access to calculator-wide features.

### Tools menu for a decimal value

When a decimal number is on the stack, the Tools menu shows mathematical functions like rounding, absolute value, sign, and number formatting options.

### Tools menu for an integer

For integers, the Tools menu provides functions like factorization, primality testing, and base conversion operations.

### Tools menu for a text

When text is on the stack, the Tools menu offers string manipulation functions like case conversion, length calculation, and text formatting options.

### Tools menu for an expression

For algebraic expressions, the Tools menu provides symbolic manipulation functions like expansion, factoring, and simplification tools.


## Computations on complex numbers

DB48X provides comprehensive support for complex number arithmetic and functions, supporting both rectangular and polar forms.

### The complex menu

Press _CPLX_ (🟨 _G_) to access the complex number menu. This provides quick access to complex number operations and conversions.

### Entering numbers in rectangular form

Use _F1_ in the complex menu to enter numbers in rectangular form (a + bi). Type the real part, press _F1_, then type the imaginary part.

### Entering numbers in polar form

Use _F2_ in the complex menu to enter numbers in polar form (r∠θ). Type the magnitude, press _F2_, then type the angle in the current angle mode.

### Switching between polar and rectangular with Cycle key

Use the _×10ⁿ_ key to cycle between polar and rectangular representations of complex numbers. This allows you to view the same complex number in different forms.

### Arithmetic on complex numbers

All standard arithmetic operations work on complex numbers. Addition, subtraction, multiplication, and division are performed using complex arithmetic rules.

### Exact angles and exact computations: 2<45 * 3<90 ^ 8

DB48X can perform exact computations with complex numbers. For example, `2∠45° × 3∠90°` gives an exact result, and you can raise complex numbers to powers while maintaining exact arithmetic.

### Functions on complex numbers, e.g. `sin` and `log`.

Most mathematical functions work on complex numbers. Trigonometric functions, logarithms, exponentials, and others are extended to the complex domain.

### Effect of angle mode on display in polar form

The current angle mode (degrees, radians, grads) affects how angles are displayed in polar form. The angle is always shown in the current mode for consistency.


## Computations on vectors

DB48X supports vector operations, allowing you to work with mathematical vectors for physics, engineering, and mathematical applications.

### Entering a vector

Use the _VECTOR_ key or menu to enter vectors. You can enter vectors as lists of components, or create them using vector-specific functions.

### The M cursor

When working with vectors, the cursor shows an 'M' indicator to indicate you're in matrix/vector mode. This helps you understand the current data type context.

### Adding and subtracting vectors

Vectors can be added and subtracted component-wise. The vectors must have the same dimensions for these operations to work.

### Component-wise multiplication and division

Use component-wise operations to multiply or divide corresponding elements of vectors. This is useful for scaling vectors or applying transformations.

### Operations between vector and a constant

You can perform operations between a vector and a scalar constant. For example, multiplying a vector by a scalar scales all components.

### Component-wise functions: 1/x

Many functions can be applied component-wise to vectors. For example, taking the reciprocal of a vector applies 1/x to each component.

### The tools menu on vectors

When a vector is on the stack, the Tools menu provides vector-specific functions like norm calculation, dot product, cross product, and vector manipulation tools.

### Computing the norm of a vector

Use the _NORM_ function to compute the magnitude (length) of a vector. This is the square root of the sum of the squared components.

### The Matrix menu

Press _MATRIX_ to access the matrix menu, which provides functions for matrix operations, vector operations, and linear algebra computations.


## Computations on matrices

DB48X provides comprehensive matrix operations for linear algebra, engineering calculations, and mathematical modeling.

### Entering a matrix

Use the _MATRIX_ menu to enter matrices. You can specify the dimensions and then enter the elements row by row, or create matrices using matrix construction functions.

### Adding and subtracting matrices

Matrices can be added and subtracted element-wise, provided they have the same dimensions. The result is a matrix with the same dimensions.

### Multiplication and division by a constant

You can multiply or divide a matrix by a scalar constant. This scales all elements of the matrix by the same factor.

### Multiplying square matrices

Matrix multiplication is performed using the standard mathematical rules. For square matrices, this involves computing dot products of rows and columns.

### Multiplying a matrix and a vector

You can multiply a matrix by a vector (or vice versa) to perform linear transformations. The dimensions must be compatible for the operation to succeed.

### Computing a determinant

Use the _DET_ function to compute the determinant of a square matrix. This is useful for determining if a matrix is invertible and for solving systems of equations.

### Computing an inverse with 1/x

Use the _1/x_ function on a square matrix to compute its inverse. This is equivalent to matrix division and is useful for solving matrix equations.


## Advanced matrix operations

DB48X supports advanced matrix operations that go beyond basic arithmetic, including complex matrices and symbolic manipulation.

### Matrix of complex numbers

Matrices can contain complex numbers as elements. This is useful for electrical engineering, quantum mechanics, and other applications where complex-valued matrices are needed.

### Symbolic matrix

You can create matrices with symbolic elements (variables) instead of numeric values. This allows for symbolic matrix algebra and manipulation of matrix expressions.

### Inverse and determinant of 2x2 symbolic matrix

For symbolic matrices, DB48X can compute exact symbolic expressions for determinants and inverses. For example, a 2×2 symbolic matrix will have its determinant and inverse expressed in terms of the symbolic elements.


## Entering data in lists

DB48X supports list operations, allowing you to work with collections of data efficiently for statistical analysis, data processing, and programming.

### Entering a list

Use the _LIST_ key or menu to create lists. You can enter lists by specifying the elements, or create them using list construction functions.

### Adding elements to a list

You can add elements to existing lists using list manipulation functions. This allows you to build lists dynamically as you collect data.

### Applying a function to a list

Many functions can be applied to lists, operating on each element. For example, applying a mathematical function to a list applies it to each element individually.

### Repeating a list (multiply)

You can repeat a list by multiplying it by a number. This creates a new list with the original elements repeated the specified number of times.

### Lists containing lists

Lists can contain other lists as elements, creating nested data structures. This is useful for representing complex data relationships and hierarchical information.


## Computations with based numbers

DB48X supports number systems in different bases, making it useful for computer science, digital electronics, and programming applications.

### Entering based numbers

Use the _BASE_ menu to enter numbers in different bases. You can specify the base and then enter the digits in that base.

### Entering hexadecimal directly with A-F

You can enter hexadecimal numbers directly using the A-F keys. The calculator automatically recognizes these as hexadecimal digits when in hexadecimal mode.

### Logical operations

Based numbers support logical operations like AND, OR, XOR, and NOT. These operations are performed bit-wise on the binary representation of the numbers.

### Setting the word size

You can set the word size for based number operations. This determines how many bits are used to represent numbers and affects the range of values.

### Changing to common bases (2, 8, 10, 16)

Quick access is provided for common bases: binary (2), octal (8), decimal (10), and hexadecimal (16). These are the most frequently used bases in computing.

### Changing to an arbitrary base

You can work with numbers in any base from 2 to 36. This allows for specialized applications and educational purposes.

### Entering number in arbitrary base

When in a specific base mode, you can enter numbers directly in that base. The calculator automatically interprets the digits according to the current base.

### The tools menu on based number

When a based number is on the stack, the Tools menu provides base-specific functions like base conversion, bit manipulation, and logical operations.

### Binary operations

Binary numbers support specialized operations like bit shifting, rotation, and bit counting. These are essential for low-level programming and digital design.

### Emulating a 16-bit or 256-bit CPU

You can set the word size to emulate different CPU architectures. This is useful for understanding how different processors handle numbers.

### The Cycle key on based numbers

Use the _×10ⁿ_ key to cycle between different representations of the same number in various bases. This provides quick conversion between number systems.

### Adding a suffix to force a base (DM32 only)

On the DM32, you can add suffixes to numbers to force interpretation in a specific base, regardless of the current base setting.


## Unit objects

DB48X supports physical units, making it ideal for engineering, physics, and scientific calculations where dimensional analysis is important.

### Entering a value with a unit

You can enter values with units using the _UNIT_ menu. Type the numeric value, then select the appropriate unit from the menu to attach it to the value.

### The units menus

The units system is organized into categories like Length, Mass, Time, etc. Each category contains relevant units, making it easy to find the unit you need.

### Applying a unit

You can apply units to existing values using the unit conversion functions. This allows you to add units to dimensionless numbers or change units.

### Converting to a unit

Use the conversion functions to change from one unit to another within the same category. For example, convert meters to feet or kilograms to pounds.

### Dividing by a unit

You can divide by a unit to remove it from a value, leaving just the numeric part. This is useful for extracting the magnitude from a unit object.



## Entering a program

DB48X supports RPL programming, allowing you to create custom functions and automate complex calculations.

### Computing a VAT

As an example, let's create a program to compute VAT (Value Added Tax). Enter the program editor and type the sequence of operations needed to calculate VAT on a given amount.

### Evaluating a program with `Evaluate`

Use the _Evaluate_ function to run a program. This executes the program with the current stack contents as input and places the result on the stack.

### Modifying a program with LastArg

Use _LastArg_ to recall the arguments of the last operation, which is useful when debugging or modifying programs to work with different inputs.

### Modifying a program with Undo

Use _Undo_ to restore the previous state when testing program modifications. This allows you to experiment with changes without losing your work.

### Modifying a program with command-line history

The command line history can be used to recall and modify previous program entries, making it easier to iterate on program development.

### The three roles of the R/S key: Space, =, EVAL

The R/S key serves multiple functions:
 * As a space character in text entry
 * As the equals sign (=) in algebraic expressions
 * As the Evaluate function for programs and expressions


## Storing values in global variables

DB48X allows you to store values in variables for later use, making complex calculations more manageable and programs more readable.

### Storing a value in a new variable 'VATRate'

To store a value in a variable, first put the value on the stack, then use the _STO_ command followed by the variable name. For example, store 0.20 in a variable called 'VATRate'.

### Evaluating a variable

To retrieve a variable's value, use the _RCL_ command followed by the variable name. This places the variable's value on the stack.

### Case insensitivity

Variable names are case-insensitive, so 'VATRate', 'vatrate', and 'VatRate' all refer to the same variable.

### Naming a variable on the command line

You can type variable names directly on the command line. If the name doesn't correspond to a command, DB48X will create a variable with that name.

### Using quotes to avoid evaluation

Use quotes around variable names to prevent immediate evaluation. This is useful when you want to work with the variable name as text rather than its value.

### Overwriting a variable value

You can overwrite an existing variable's value by storing a new value with the same name. The old value is replaced with the new one.

### Expressions containing variables

Algebraic expressions can contain variables. When evaluated, the variables are replaced with their current values, allowing for dynamic calculations.


## Storing and modifying programs

DB48X allows you to store programs as named functions, making them easily accessible and reusable.

### Creating a new `VAT` command

Create a program to compute VAT and store it with the name 'VAT'. This creates a custom command that you can call by name, just like built-in functions.

### Evaluating a program by name

Once stored, you can execute a program by typing its name and pressing _ENTER_. This is the same way you execute built-in commands.

### Evaluating a program from variables menu

You can also execute programs from the variables menu, which shows all your stored variables and programs in an organized list.

### Taking input and computing output

Programs can take input from the stack and produce output on the stack. This makes them work seamlessly with the RPL paradigm and other functions.


## The variables menu

The variables menu provides quick access to all your stored variables and programs, making it easy to manage and use your custom functions.

### Showing the variables menu

Press _VARS_ (🟨 _F5_) to display the variables menu. This shows all your stored variables and programs in an organized list.

### Evaluating a variable with F1

Press _F1_ to evaluate (execute) the selected variable or program. This places the result on the stack or executes the program.

### Recalling a variable with shift F1

Press 🟨 _F1_ to recall the selected variable's value without evaluating it. This is useful when you want to see the variable's contents.

### Storing in an existing variable with xshift F1

Press 🟦 _F1_ to store the current stack value in the selected variable. This overwrites the variable's previous value.

### Rationale for the difference with HP48

The DB48X variables menu differs from the HP48 to provide more intuitive access to variables and better integration with the modern interface.

### Using variables menu while editing a program

You can access the variables menu while editing a program to insert variable references or program calls into your code.


## Menus with too many entries

When menus have more entries than can fit on the screen, DB48X provides navigation controls to access all available options.

### Adding more variables overflows

As you add more variables and programs, they may exceed the space available on a single menu screen. When this happens, navigation controls appear.

### Going from 6 to 7 entries

When you have more than 6 entries in a menu, the display automatically shows navigation options to access additional entries.

### No next key, using F6 and shift F6 for next and previous

Instead of a dedicated NEXT key, DB48X uses _F6_ to go to the next page and 🟨 _F6_ to go to the previous page. This keeps the interface consistent and efficient.


## Saving your state to disk

DB48X allows you to save your current state (variables, programs, settings) to disk for later restoration or backup purposes.

### The system menu

Access the system menu through the setup options to manage state files and system operations.

### Saving the calculator state

Use the save state function to store your current calculator state, including all variables, programs, and settings, to a file on the calculator's storage.

### Restoring another state

You can load a previously saved state file to restore your calculator to a previous configuration, including all variables and programs that were saved.

### Merging states

The merge state function allows you to combine the contents of a saved state file with your current state, adding new variables and programs without overwriting existing ones.

### Returning to the calculator

After performing state operations, you can return to normal calculator operation to continue your work with the restored or merged state.

### Saving state quickly with xshift-EXIT

Use 🟦 _EXIT_ as a quick shortcut to save the current state without going through the full menu system.


## Plotting a function

DB48X includes powerful graphing capabilities for visualizing mathematical functions and data.

### Plotting a wave function sin(x * a) * cos(x * b)

You can plot complex functions like wave combinations. Enter the function in algebraic form and use the plotting functions to display it on the calculator's screen.

### Plotting a polar function

Polar functions can be plotted to show relationships in polar coordinates. This is useful for circular and spiral patterns.

### Plotting a parametric function

Parametric functions allow you to plot curves defined by x and y as functions of a parameter t. This enables plotting of complex curves and shapes.

### Drawing two functions on the same screen

You can plot multiple functions simultaneously on the same graph, making it easy to compare different functions or see their relationships.

### Changing line width

The line width can be adjusted to make plots more visible or to distinguish between different functions on the same graph.

### Changing line pattern

Different line patterns (solid, dashed, dotted) can be used to distinguish between multiple functions plotted on the same screen.

## Plotting data

In addition to plotting functions, DB48x can also plot data:
* `ScatterPlot` draw points in an array from x and y coordinates
* `BarPlot` plots a vector of values as a series of bars
* `HistogramPlot` shows the distribution of values along the x axis.


## The numerical solver

DB48X includes a powerful numerical solver that can find solutions to equations and systems of equations.

### Solving an equation

Use the solver to find numerical solutions to equations. Enter the equation in algebraic form and specify which variable to solve for.

### Expressions that must be zero

The solver works by finding values that make an expression equal to zero. You can enter any expression, and the solver will find where it equals zero.

### Equations A=B

For equations in the form A=B, you can rewrite them as A-B=0 and use the solver to find where this expression equals zero.

### Solving for different variables

You can solve for any variable in an equation. The solver will find values of the specified variable that satisfy the equation.


## Numerical integration

DB48X provides numerical integration capabilities for computing definite integrals of functions.

### Integrating x^2 from 0 to 1 (exact results)

For simple functions like x², DB48X can compute exact results. The integral of x² from 0 to 1 gives the exact result 1/3.

### What happens with 0.0 to 1.0

When using decimal bounds (0.0 to 1.0 instead of 0 to 1), the integration uses numerical methods and may give approximate results.

### Integration 1/x from 2 to 22

For functions like 1/x, numerical integration gives approximate results. The integral from 2 to 22 gives ln(22) - ln(2) ≈ 2.3979.

### Comparing with LN(2) - LN(2)

You can verify integration results by comparing them with known analytical solutions. For example, the integral of 1/x from a to b equals ln(b) - ln(a).


## Symbolic expression manipulation

DB48X includes symbolic algebra capabilities for manipulating mathematical expressions without evaluating them numerically.

### Collecting terms

Use the collect function to combine like terms in an expression. For example, x + 2x + 3x becomes 6x.

### Expanding terms

Use the expand function to multiply out expressions. For example, (x + 2)(x + 3) becomes x² + 5x + 6.

### General expression rewriting facility

DB48X provides a general expression rewriting system that can simplify, factor, and transform mathematical expressions using algebraic rules.


## Adding Local variables

Local variables provide a way to store temporary values within programs and expressions without affecting global variables.

### Why use local variables

Local variables are useful for storing intermediate results in programs without cluttering the global variable space. They are automatically cleaned up when the program finishes.

### Inserting local variables in a program

Use the local variable syntax within programs to create variables that exist only during program execution. This keeps programs self-contained and prevents variable name conflicts.

### Inserting local variables in equations

Local variables can also be used in algebraic expressions to store intermediate values during complex calculations.


## Localized number display preferences

DB48X allows you to customize number display to match your regional preferences and conventions.

### Changing the decimal separator

You can change the decimal separator from a period (.) to a comma (,) to match European number formatting conventions.

### Changing the spacing for numbers

The spacing used to group digits in large numbers can be customized. You can choose between different grouping patterns or disable grouping entirely.

### Changing the character used for spacing

You can select which character is used for digit grouping, such as spaces, commas, or periods, depending on your regional preferences.


## User interface preferences

DB48X provides extensive customization options for the user interface to suit different preferences and usage patterns.

### Square and rounded menu styles

You can choose between square and rounded menu styles to match your visual preferences. This affects the appearance of menu borders and buttons.

### 3-level, 1-level and flat menu styles

Menu organization can be customized with different levels of hierarchy. You can choose between 3-level menus (with multiple shift states), 1-level menus, or flat menus for simpler navigation.

### Changing result font size

The font size used to display calculation results can be adjusted to improve readability or fit more information on the screen.

### Changing stack font size

The font size for stack display can be customized independently of result display, allowing you to optimize the layout for your needs.

### Changing editor font size

The font size in the command line editor can be adjusted to make text entry more comfortable or to fit longer expressions.

### Changing multi-line editor font size

The multi-line editor font size can be set independently to optimize the display of longer text and programs.



## Comparisons and tests

DB48X provides comprehensive comparison and logical operations for programming and conditional calculations.

### Truth: True, False, 0, 1

DB48X uses 1 to represent true and 0 to represent false. These values can be used in logical operations and conditional statements.

### Equality tests

Use equality operators to compare values. The result is 1 (true) if the values are equal, 0 (false) otherwise.

### Differences between = and ==

The = operator is used for assignment and algebraic equality, while == is used for comparison testing. Understanding this distinction is important for programming.

### Relational operators

Use relational operators like <, >, ≤, ≥ to compare values. These return 1 for true and 0 for false, making them useful in conditional expressions.

### Logical operations (AND, OR, NOT)

Logical operations combine boolean values. AND returns 1 only if both operands are 1, OR returns 1 if either operand is 1, and NOT inverts the truth value.


## More sophisticated programming

DB48X provides advanced programming constructs for creating complex algorithms and automated calculations.

### Testing with IF THEN ELSE END

Use IF-THEN-ELSE-END structures for conditional execution. This allows programs to make decisions based on test conditions.

### Conditional expression with IFTE

IFTE (If-Then-Else) is a compact conditional expression that returns one value if a condition is true, another if it's false.

### Counted loop with START NEXT

Use START-NEXT loops for counted iterations. This executes a block of code a specified number of times.

### Stepping loop with START STEP

START-STEP loops allow you to specify a step size for iterations, useful for creating sequences with non-unit increments.

### Named loop with FOR NEXT

FOR-NEXT loops provide named loop variables that can be used within the loop body, making it easier to track iteration progress.

### Named loop with FOR STEP

FOR-STEP loops combine named variables with custom step sizes, providing maximum flexibility for loop control.

### WHILE conditional loop

WHILE loops continue executing as long as a condition remains true, useful for loops where the number of iterations is not known in advance.

### UNTIL conditional loop

UNTIL loops continue until a condition becomes true, providing an alternative way to express conditional looping.


## Enjoy the calculator!

Congratulations! You've now learned the fundamentals of using DB48X. This powerful calculator combines the best features of traditional RPL calculators with modern enhancements, providing you with:

 * **Exact arithmetic** with fractions and symbolic computation
 * **Multiple input modes** including RPN and algebraic notation
 * **Comprehensive scientific functions** for mathematics, physics, and engineering
 * **Advanced programming capabilities** with RPL
 * **Flexible display options** and user interface customization
 * **Powerful graphing and visualization** tools
 * **Extensive unit support** for dimensional analysis

The more you use DB48X, the more you'll discover its capabilities. Don't hesitate to explore the built-in help system, experiment with different features, and create your own programs to automate repetitive calculations.

Happy calculating!



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
