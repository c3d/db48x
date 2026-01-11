# Overview

## DB48X on DM42

The DB48X project intends to rebuild and improve the user experience of the
legendary HP48 family of calculators, notably their *"Reverse Polish Lisp"*
 [(RPL)](#Introduction-to-RPL)
language with its rich set of data types and built-in functions.

This project is presently targeting the **SwissMicro DM42 calculator**
and leveraging its built-in software platform, known as **DMCP**. This is
presumably the calculator you are currently running this software on.
You can also [try it in your browser](http://48calc.org).

## Table of contents

* [Using the on-line help](#help)
* [Quickstart guide](#quickstart-guide)
* [Programming examples](#rpl-programming-examples)
* [State of the project](#state-of-the-project)
* [Design overview](#design-overview)
* [Keyboard interaction](#keyboard-interaction)
* [Soft menus](#soft-menus)
* [Differences with other RPLs](#differences-with-other-RPLs)
* [Built-in help](#help)
* [Acknowledgements and credits](#acknowledgements-and-credits)
* [Release notes](#release-notes)
* [Performance](#performance-measurements)
* [Keyboard overlays](https://www.hpmuseum.org/forum/thread-20113.html)


## State of the project

This is currently **SEMI-STABLE** software, meaning that the implemented
features appear to work somewhat reliably, but that some features are still
being added with each new release. This is **NOT PRODUCTION READY** and should
not be used for any mission-critical computation.

At this point in time, you should only install this if you are interested in
contributing to the project, whether it is in the form of code, feedback or
documentation changes. Please refer to the web site of the project on GitHub or
GitLab for details and updates. The best way to
[report an issue](https://github.com/c3d/db48x/issues),
[request an improvement](https://github.com/c3d/db48x/issues/new)
or [submit a proposed change](https://github.com/c3d/db48x/pulls) is
on the project's [GitHub page](https://github.com/c3d/db48x).

The [implementation status](#implementation-status) section categorizes all the
RPL commands in the HP50G and in DB48X into
[implemented](#implemented-commands),
[not implemented yet](#unimplemented-commands),
[unapplicable commands](#unapplicable-commands) and
[DB48X only](#additional-commands) lists.

## Design overview

The objective is to re-create an RPL-like experience, but to optimize it for the
existing DM42 physical hardware.
Ideally, DB48X should be fully usable without a keyboard overlay, though one is
[being worked on](https://github.com/c3d/db48x/blob/stable/Keyboard-Layout.png).

Compared to the original HP48, the DM42 has a much larger screen, but no
annunciators (it is a fully bitmap screen). It has a keyboard with dedicated
soft-menu (function) keys, but only one shift key (whereas the HP48 has two),
lacks a dedicated alpha key, does not provide left or right arrow keys (only up
and down), and has no space key (_SPC_ on the HP48).



## Keyboard interaction

The keyboard differences force us to revisit the user interaction with the
calculator compared to the HP48:

* When running DB48X on the DM42, the single yellow shift key cycles between
  three states, *Shift* (shown in the documentation as 🟨), *Right Shift* (shown
  in the documentation as 🟦), and no shift.  This double-shift shortcut appears
  necessary because RPL calculators like the HP48 have a rather full keyboard
  even with two shift keys.


* A first press on the shift key is shown as 🟨 in the documentation, and
  activates functions shown in yellow in the keyboard overlay. A second press is
  shown as 🟦 in the documentation, and activates functions shown in blue in the
  keyboard overlay. On the screen, the shift state is indicated in the header
  area. When a [soft menu](#soft-menus) is visible on the screen, the selected
row of functions is highlighted.

In the rest of this document, the shift key is referred to as 🟨, and pressing
it twice is referred to as 🟦, irrespective of the appearance of the physical
shift key on your particular hardware. While running the firmware, this will display in the annunciator area as follows:

![Shifts](shifts.bmp)

Other aspects of the keyboard interaction are fine-tuned for RPL usage:

* Since RPL uses alphabetic entry (also called *Alpha* mode) a lot more
  frequently than on RPN models like the HP41 or HP42, making it quickly
  accessible seems important, so there are
  [three distinct ways to activate it](#alpha-mode).

* The _▲_ and _▼_ keys move the cursor *left* and *right* while editing
  instead of *up* and *down*. These cursor movements are much more useful for a
  text-based program editing as found in RPL. In the rest of this document,
  they are described as _◀︎_ and _▶︎_ respectively.

* Using 🟨 _◀︎_ and 🟨 _▶︎_ moves the cursor up and down.  When not editing, _◀︎_
  and _▶︎_ behave like _▲_ and _▼_ on the HP48, i.e. _◀︎_ enters the *interactive
  stack*  and _▶︎_ edits the object on the first level of the stack.

* Long-pressing arrow keys, the _←_ (also known as *Backspace*) or text entry
  keys in Alpha mode activates auto-repeat.

* Long-pressing keys that would directly trigger a function (e.g. _SIN_),
  including function keys associated with a soft-menu, will show up the
  [built-in help](#help) for the corresponding function.


### Alpha mode

Entering alphabetic characters is done using *Alpha* mode. These alphabetic
characters are labeled on the right of each key on the DM42's keyboard.

When *Alpha* mode is active, an _ABC_ indicator shows up in the annunciator area
at the top of the screen. For lowercase entry, the indicator changes to
_abc_.

There are three ways to enter *Alpha* mode:

* The first method is to use 🟨 _ENTER_ as indicated by the _ALPHA_ yellow label
  on the DM42 ENTER key. This cycles between *Alpha* _ABC_, *Lowercase* _abc_
  and *Normal* entry modes.

* The second method is to hold 🟨 for more than half a second. This cycles
  between *Alpha* _ABC_ and *Normal* entry modes, and cannot be used to type
  lowercase characters.

* The third method is to hold one of the arrow keys _◀︎_ or _▶︎_ *while* typing on
  the keyboard. This is called *transient alpha mode* because *Alpha* mode ends
  as soon as the arrow key is released. Using _◀︎_ enters uppercase characters,
  while _▶︎_ uses lowercase characters.

There is no equivalent of the HP48's "single-Alpha" mode. Alpha mode is either
_transient_ (when you hold one of the arrow keys) or _sticky_ (with 🟨 _ENTER_
or by holding 🟨).

Alpha mode is cancelled when pressing _ENTER_ or _EXIT_.

Since the DM42's alphabetic keys overlap with the numeric keys (unlike the
HP48), as well as with operations such as _×_ and _÷_, using 🟨 in Alpha mode
brings back numbers. This means 🟨 cannot be used for lowercase, but as
indicated above, there are two other methods to enter lowercase
characters.

Using 🟨 or 🟦 in combination with keys other than the numeric keypad gives a
variety of special characters. The `CharactersMenu` and `Catalog` can be used to
enter special characters in a more comfortable way.


### Key mapping

The layout of keys on DB48X was carefully chosen to offer a good compromise
between immediate applicability for calculations and giving access to numerous
advanced functions through menus.

DB48X keyboard overlays for DM-42 and DM-32 SwissMicros calculators are
[already available](https://www.hpmuseum.org/forum/thread-20113.html).

![DB48X keyboard layout](keyboard.png)

In the rest of this document, keys bindings will usually be described using the
alphabetic key, to make it easier to locate on the keyboard, followed by the
standard label on the DB48X layout. For example, the assignment for the `sin`
function will be described as _J_ (_SIN_). The shifted functions of the same key
will be described as 🟨 _J_ (_SIN⁻¹_) or 🟦 _J_ (_HYP_) respectively.

In some cases, the label between parentheses may refer to another calculator
model, which will be indicated as follows. For example, the _A_ key can be
described as _A_ (_⚙️_, DM-42 _Σ+_, DM-32 _√x_).

However, if you are using DB48X on a DM42, it is possible to do it without a
keyboard overlay, because great care was taken to have the DB48X keboard layout
remain close to that of the DM42, in order to preserve muscle memory. New
features were positioned on the keyboard at positions that are close to what is
familiar in the original DM42 firmware.

A few keys that have little use in RPL are reassigned to features that you
should be able to quickly remember. For example, the DM-42 _RCL_ key is used for
the DB48X _VAR_ key, which invokes the [VariablesMenu](#VariablesMenu).

Note that the _LOG_ and _e^x_ keys are swapped relative to the DM-42. The HP42
has _LOG_ and _LN_ with shifted _10^x_ and _e^x_. DB48X has _e^x_ and _LN_
with shifted _10^X_ and _LOG_, so that the more frequently used mathematical
functions are available without shifting. Note that in the future, full
keyboard remapping similar to the HP41 or HP48 will allow you to change that
if you prefer.


Here are a few of the interesting RPL-specific key mappings:

* _A_ (_⚙️_, DM-42 _Σ+_, DM-32 _√x_) is used to invoke a context-sensitive
  [ToolsMenu](#ToolsMenu), which selects a softkey menu based on what is on the
  stack and other context.

* 🟨 _A_ (_←MENU_, DM-42 _Σ-_, DM-32 _x²_) selects the [LastMenu](#LastMenu)
  command, which displays the previously selected menu.

* 🟦 _A_ (_MAIN_, DM-32 _PARTS_) selects the [MainMenu](#MainMenu), a top-level
  menu giving indirect access to all other menus and features in DB48X (see
  also the [Catalog](#Catalog) feature).

* _F_ (_' ()_, DM-42 _XEQ_, DM-32 _Σ+_) opens an algebraic expression, i.e. it
  shows `''` on the command-line and switches to equation entry. If already
  inside an equation, it inserts a pair of parentheses. This can be used to
  evaluate expressions in [algebraic mode](#algebraic-mode) instead of
  RPN.

* 🟨 _G_ (_CPLX_, DM-42 _COMPLEX_, DM-32 _CMPLX_) lets you work with complex
  numbers. It opens the [ComplexMenu](#ComplexMenu), which can be used to enter
  complex numbers in rectangular or polar form, and perform common operations on
  these numbers. The same menu can be accessed without shift using _A_ (_⚙️_)
  when there is a complex number on the stack.

* _H_ (_VAR_, DM-42 and DM-32 _RCL_) opens the [VariablesMenu](#VariablesMenu)
  showing user variables in the current directory.

* _I_ (_STK_, DM-42 and DM-32 _R↓_) will open the [StackMenu](#StackMenu),
  giving access to stack operations.

* 🟨 _I_ (_CONST_, DM-42 _π_, DM-32 _HYP_) shows a
  [ConstantsMenu](#ConstantsMenu) giving access to various constants. You can
  provide your own constants in a `config/constants.csv` file on disk.

* _M_ (_X⇆Y_) executes the RPL [Swap](#swap) function

* 🟨 _M_ (_LAST_, DM-42 _LAST x_, DM-32 _MEM_) is [LastArg](#LastArguments),
  which recalls the arguments of the last command.

* 🟦 _M_ (_Undo_, DM-32 _X⇆_) restores the previous state of the stack. This is
  like `Last Stack` on the HP48, but on DB48X, it is a real command that can be
  used in programs.

* _N_ (_+/-_) executes the equivalent RPL `Negate` function. While editing, it
  changes the sign of the current number on the command-line.

* _O_ (_×10ⁿ_, _EEX_ or _E_ depending on keyboard labeling, referred to as
  _×10ⁿ_ in the rest of this document) is used to enter the exponent of a number
  in scientific notation. However, when not entering or editing values, it
  invokes the [Cycle](#Cycle) command, which cycles between various
  representations of a number, for example polar and rectangular for a complex
  number, or fraction and decimal for a decimal number.

* _EXIT_ (DM-32 _ON_) corresponds to what the HP48 manual calls _Attn_, and
  typically cancels the current activity. It can also be used to interrupt a
  running program.

* 🟨 _EXIT_ (_OFF_) shuts down the calculator. The state of the calculator is
  preserved.

* 🟦 _EXIT_ (_SAVE_) saves the current state of the calculator to disk. This
  state can be transferred to another machine, and survives system reset or
  firmware upgrades.

* 🟨 _0_ (_SETUP_) shows the firmware's [SystemMenu](#SystemMenu), for example
  to load the original DM-42 or DM-32 program, activate USB disk, and to access
  some calculator preferences.

* The _R/S_ keys inserts a space in the editor, an `=` sign inside equations,
 and maps to [Evaluate](#evaluate) otherwise.

* 🟨 _R/S_ (_«PROG»_, DM-42 and DM-32 _PRGM_) inserts the delimiters for an RPL
  program, `«` and `»`, while 🟦 _R/S_ (_{LIST}_) inserts the list delimiters,
  `{` and `}`.

* 🟨 _+_ (_CAT_, DM-42 _CATALOG_, DM-32 _LBL_) shows a complete
  context-sensitive [catalog](#Catalog) of all available functions, and
  enables auto-completion using the soft-menu keys. Note that the `+` key alone
  (without shift) activates the catalog while in *Alpha* mode. When inside text,
  the catalog presents alternates for the character at the left of the cursor,
  providing a convenient way to select diacritics and accents..

* 🟦 _+_ (_HELP_, DM-32 _RTN_) activates the context-sensitive help system.


## Soft menus

The DM42 has 6 dedicated soft-menu keys at the top of the keyboard. Most of the
advanced features of DB48X can be accessed through these soft menus. Soft menu
keys have no label on the physical calculator, but in this documentation, they
may sometimes be referred to as _F1_ through _F6_.

All built-in soft-key menus are named, with names ending in [Menu](#Menu). For
example, the [VariablesMenu](#VariablesMenu) is the menu listing global
variables in the current directory. Unlike HP RPL calculators, menus cannot be
accessed by number, but they can be accessed by name. In a future version of the
firmware, a [Menu](#Menu) special variable will return the name of the current
menu. The [LastMenu](#LastMenu) command selects the previous menu.

Menus are organized internally as a hierarchy, where menus can refer to other
menus. A special menu, [MainMenu](#MainMenu), accessible via the 🟦 _A_,
contains all other menus.

Menus can contain up to 18 entries at once, 6 being directly accessible, 6
more being shown when using the 🟨 key, and 6 more with 🟦. Three rows of
functions are shown on screen, with the active row highlighted.

A long press on a function key invokes the on-line help for the associated
function.

When a menu contains more than 18 entries, then the _F6_ function key turns into
a `▶︎`, and 🟨 _F6_ turns into `◀`︎. These keys can be used to
navigate across the available menu entries. This replaces the _NXT_ and _PREV_
keys on HP calculators.

The [VariablesMenu](#VariablesMenu) is used to access global variables. It is
invoked using the _H_ key, which is labeled _RCL_ on SwissMicros hardware. This
menu is special in the sense that:

* Selecting an entry *evaluates* that menu entry, for example to run a program

* The 🟨 function *recalls* its name without evaluating it.

* The 🟦 function *stores* into the variable.


## Differences with other RPLs

Multiple implementations of RPL exist, most of them from Hewlett-Packard.
A good reference to understand the differences between the various existing
implementations from HP is the
[HP50G Advanced User's Reference Manual](https://www.hpcalc.org/details/7141).

There are a number of intentional differences in design between DB48X and the
HP48, HP49 or HP50G's implementations of RPL. There are also a number of
unintentional differences, since the implementation is completely new.

### User interface

* DB48X features an extensive built-in help system, which you are presently
  using. Information for that help system is stored using a regular *markdown*
  file named `/help/db48x.md`, stored in the calculator's flash storage.

* DB48X features auto-completion for commands while typing, through
  the  _CAT_ key (a [catalog](#catalog) of all commands).

* Many RPL words exist in short and long form, and a user preference selects how
  a program shows. For example, the ` Negate ` command, which the HP48 calls
  ` NEG `, can display, based on [user preferences](#command-display), as
  ` NEG `, ` neg `, ` Neg ` or ` Negate `. In the help, commands are shown using
  the current spelling preference, and all possible aliases are shown on the
  right of the default spelling.

* The DB48X dialect of RPL is not case sensitive, but it is case-respecting.
  For example, if your preference is to display built-in functions in long form,
  typing ` inv ` or ` INV ` will show up as `Invert` in the resulting program.
  This means that the space of "reserved words" is larger in DB48X than in other
  RPL implementations. Notably, on HP's implementations, ` DUP ` is a keyword
  but you can use ` DuP ` as a valid variable name. This is not possible in
  DB48X.

* The saving of the stack arguments for the `LastArg` command is controled
  independently by two distinct settings, `SaveLastArguments` and
  `ProgramLastArguments`. The first one controls if `LastArg` is saved for
  interactive operations, and is enabled by default. The second one controls if
  `LastArg` is saved before executing commands while running a program or
  evaluating an expression, and is disabled by default. This impacts commands
  that evaluate programs, such as `ROOT`. On the HP48, `LastArg` after running
  `ROOT` interactively gives arguments used by some operation within `ROOT`,
  whereas on DB48X with the default settings, it returns the arguments to
  `ROOT`.

* When parsing the `Σ` or `∏` functions (HP calculators only have `Σ`), all
  arguments are separated by semi-colons like for all other functions. HP
  calculators have a special syntax in that case, where an `=` sign separates
  the index and its initial value. In other words, where an HP calculator would
  show `Σ(i=1;10;i^2)`, which corresponds to the 4-argument sequence `i 1 10
  'i^2' Σ`, the DB48X implementation shows and requires the `Σ(i;1;10;i^2)`
  syntax. Note that an `=` sign may appear inside an expression, but it always
  denotes equality.


### Evaluation

* Local names are evaluated on DB48X, unlike in the HP versions of RPL. This
  makes it easier to use local subprograms in larger programs as if they were
  normal operations. In the less frequent case where you do not want evaluation,
  you need to use `RCL` like for global variables.

* Lists do not evaluate as programs by default, like on the HP28, but unlike on
  the HP48 and later HP models. This can be controlled using the
  `ListEvaluation` setting. Note that a list can be converted to a program using
  the `→Program` command, which makes it easy to build programs from lists.
  See also the differences regarding quoted names in
  [Representation of objects](#representation-of-objects)

* The `case` statement can contain `when` clauses as a shortcut for the frequent
  combination of duplicating the value and testing against a reference. For
  example, `case dup "A" = then "Alpha" end` can be replaced with
  `case "A" when "Alpha" end`.

* There are no _compiled local variables_. A program like `→ ←x « Prog »`
  may perform incorrectly if `Prog` attempts to access `←x`. Compiled local
  variables are a rather obscure feature with a very limited use, and might be
  replaced with true closures (which have a well-defined meaning) if there is
  enough demand.

* Logical operations such as `AND` or `NOT` will apply bitwise when given
  integer arguments. For example, `42 4 AND` returns `0` on DB48x, but `1.`
  (logical truth) on HP calculators. Both DB48x and HP calculators will perform
  bitwise operations when given based numbers, e.g. `#42d #4d AND` returns `#0`
  in the two implementations. This can be changed using `TruthLogicForIntegers`,
  which restores the HP-like behaviour.


### Numbers

* DB48X has several separate representations for numbers:
  [integers](#integers), [fractions](#fractions),
  [decimal](#decimal-numbers) and [complex](#complex-numbers) in polar or
  rectangular form.Notably, this implementation of RPL keeps integer values
  and fractions in exact form for as long as possible to optimize both
  performance and memory usage.  This is closer to the HP50G in exact mode than
  to the HP48. Like the HP50G, DB48X will distinguish `1` (an integer) from `1.`
  (a decimal value), and the `TYPE` or `TypeName` commands will return distinct
  values.

* Integer and fraction arithmetic can be performed with arbitrary
  precision, similar to the HP50G. The `MaxNumberBits` setting controls how much
  memory can be used for integer arithmetic.

* DB48X has true fractions. From a user's perspective, this is somewhat similar
  to fractions on the HP50G, except that fractions are first-class numbers,
  whereas the HP50G treats them like expressions. On the HP50G, `1 3 / TYPE`
  returns `9.`, like for `'A + B'`. On DB48X, the `TYPE` for fractions is
  different than for expressions. Fractions can be shown either as
  `MixedFractions` or `ImproperFractions`.

* On HP50G, decimal numbers often outperform integers or fractions, and
  benchmark code will contain `1.` instead of `1` for that reason. On DB48X,
  arithmetic on integers and fractions is generally faster.

* Like the HP Prime, DB48X displays a leading zero for decimal values by
  default, whereas HP RPL calculators do not. For example, DB48x with default
  settings will display `0.5` and not `.5`. This is controlled by the
  `LeadingZero` flag.

* DB48X has two distinct representations for complex numbers, polar and
  rectangular, and transparently converts between the two formats as needed.
  The polar representation internally uses fractions of pi for the
  angle, which allows exact computations. By contrast, HP RPL implementations
  always represent complex numbers in rectangular form internally, possibly
  converting it to polar form at display time.

* DB48X features arbitrary-precision decimal floating-point. The `Precision`
  command (in the `DisplayModesMenu`) can be used to select the precision for
  numerical operations. In addition, it supports 32-bit and 64-bit
  hardware-accelerated binary floating-point.


### Representation of objects

* The storage of data in memory uses a denser format than on the HP48.
  Therefore, objects will almost always use less space on DB48X. Notably, the
  most frequently used functions and data types consume only one byte on DB48X,
  as opposed to 5 nibbles (2.5 bytes) on the HP48. A number like `123` consumes
  2 bytes on DB48X vs. 7 on the HP50 and 10.5 on the HP48.

* Numerical equality can be tested with `=`,  whereas object equality is tested
  using `==`. For example, `0=0.0` is true, but `0==0.0` is false, because `0`
  is an integer whereas `0.0` is a floating-point.

* Because of differences in internal representation that would require expensive
  computations to mimic the HP50G behaviour with limited benefit, `Size` returns
  1 for integers, algebraic expressions and unit objects.

* The `Type` command can return HP-compatible values that are sometimes
  imprecise (e.g. it cannot distinguish between polar and rectangular complex
  values), or numerical values that distinguish all the types in DB48X. This is
  controlled by the `CompatibleTypes` setting. The `TypeName` command is a
  DB48X-only extension that returns more precise textual information, and should
  be preferred both for readability and future compatibility.

* Expressions between quotes are always algebraic expressions, unlike on HP
  calculators, where a number or a name in quotes is parsed as a number or
  name. The `Type` for `'N'` is `9` on DB48x vs. `6` on HP calculators.
  Parsing names always behaves like in programs, and is consistent for arrays
  and lists as well. By contrast, on HP calculators, if you enter `« 'N' N »`
  and edit it, you get a quoted name followed by an unquote, but if you enter
  `{ 'N' N }`, you get `{ N N }` as a resulting object, which is not very
  consistent, and makes it harder to programmatically use lists to create
  programs (e.g. using `→Program`).

* DB48X has a dedicated data type to represent multi-variate polynomials, in
  addition to the classical RPL-based algebraic expressions.


### Alignment with SwissMicros calculators

* DB48X will borrow to the DM-42 the idea of _special variables_ for settings,
  which are variables with a special meaning. For example, the `Precision`
  special variable is the current operating precision for floating point, in
  number of digits. While there is a `Precision` command that sets the value, it
  is also possible to use `'Precision' STO` to set it, and `'Precision' RCL` to
  fetch the current value. This does not imply that there is an internal
  `Precision` variable somewhere. This applies to all settings and
  flags. Additionally, binary settings can be set with `SF` and `CF`, and
  queried with `SF?` and `CF?`. For example, `'HideDate' CF` will clear the
  `HideDate` flag, meaning that the date will show in the header.

* The DB48X also provides full-screen setup menus, taking advantage of the DM42
  existing system menus. It is likely that the same menu objects used for
  softkey menus will be able to control system menus, with a different function
  to start the interaction.

* The whole banking and flash access storage mechanism of the HP48 will be
  replaced with a system that works well with FAT USB storage. It is possible to
  directly use a part of the flash storage to store RPL programs or data, either
  in source or compiled form. Using a text argument to `STO` and `RCL` accesses
  files on the USB disk, e.g. `1 "FOO.TXT" STO` stores the text representation
  of `1` in a file named `DATA/FOO.TXT` on the USB flash storage.

### List operation differences

The application of a same operation on arrays or matrices has never been very
consistent nor logical across RPL models from HP.

* On HP48 and HP50, `{1 2 3} 4 +` gives `{1 2 3 4}`. However, `{1 2 3} 4 *`
  gives a type error on the HP48 but applies the operation to list elements on
  the HP50, yielding `{4 8 12}`. Finally, `{1 2 3} 4 add` will give `{5 6 7}` on
  the HP50, but that command does not exist on HP48.

* For arrays, `[ 1 2 3 ] 4 +` fails on both the HP48 and HP50, but
  `[ 1 2 3 ] 4 *` works.

* The HP50 has a `MAP` function, which works both for list and matrices.
  `[ 1 2 3 ] « 3 + »` will return `[ 4 5 6 ]`, and `{ 1 2 3 } « 3 * »` will
  return `{ 3 6 9 }`. That function has no direct equivalent on the HP48.

DB48X considers lists as bags of items and treat them as a whole when it makes
sense, whereas arrays are focusing more on the values they contain, and will
operate on these items when it makes sense. Therefore:

* `{ 1 2 3 } 4 +` gives `{ 1 2 3 4 }`, `{ 1 2 3 } 2 -` gives `{ 1 3 }` (not yet
  implemented), and `{ 1 2 3 } 3 ×` gives `{ 1 2 3 1 2 3 1 2 3 }`. The `÷`
  operator is equivalent to the `ListDivide` function, and partitions a list in
  chunks of the given size and returns the number of partitions so generated
  (the last partition being possibly shorter), i.e. `{ 1 2 3 4 5 } 2 ÷` will
  generate `{1 2} {3 4} {5} 3` on the stack (this is not yet implemented).

* `[ 1 2 3 ] 4 +` gives `[ 5 6 7 ]`, `[ 1 2 3 ] 2 -` gives `[ -1 0 1 ]`,
  `[ 1 2 3 ] 3 ×` gives `[ 3 6 9 ]` and `[ 1 2 3 ] 5 ÷` gives
  `[ 1/5 2/5 3/5 ]`.

* Two lists can be compared using lexicographic order. This also applies to the
  `Min` and `Max` functions, which compares the entire lists, whereas on HP50G,
  it compares element by element (element-wise comparison applies to arrays).

* The `|` operator to apply variables to an expression can be used without
  parentheses and chained. For example, `'A+B|A=X|B=Y+Z|Z=42'` evaluates as
  `'X+Y+42'`. This differs from the way the HP50G evaluates it, where the last
  substitution `Z=42` would not be applied since `Z` was not part of the
  original expression. In other words, on HP50G, `'A+B|(A=X;B=Y+Z;Z=42)'`
  evaluates as `'X+Y+Z'`, not as `'X+Y+42'`.The HP-style parenthesized notation
  is accepted, but is converted to the DB48X sequence of `|` form during
  parsing.

* On HP calculators, the behavior on tagged lists is not very consistent.
  For example, `{ 1 2 } :A:{ 3 4 } +` gives `{ 1 2 :A:{ 3 4 } }` but
  `:A:{ 1 2 } :B:{ 3 4 } +` gives `{ 1 2 3 4 }`, and so does
  `:A:{ 1 2 } { 3 4 } +`. DB48x strips the tags in all cases, i.e. the first
  case gives the same `{ 1 2 3 4 }` as the other two.

* As indicated [earlier](#representation-of-objects), quoted names in lists
  remain quoted, whereas on HP calculators, the quotes are removed.


### Vectors and matrices differences

* On DB48X, vectors like `[ 1 2 3 ]` are very similar to lists. The primary
  difference is the behavior in the presence of arithmetic operators.
  On lists, addition is concatenation, e.g. `{ 1 2 3} { 4 5 6} +` is
  `{ 1 2 3 4 5 6 }`, whereas on vectors represents vector addition, e.g.
  `[1 2 3] [4 5 6] +` is `[5 7 9]`. However, unlike on the HP original
  implementation, a vector can contain any type of object, so that you can
  do `[ "ABC" "DEF" ] [ "GHI" "JKL" ] +` and obtain `[ "ABCGHI" "DEFJKL" ]`.

* Size enforcement on vectors only happens _during these operations_, not while
  you enter vectors from the command line. It is legal in DB48X to have a
  non-rectangular array like `[[1 2 3] [4 5]]`, or even an array with mixed
  objects like `[ "ABC" 3 ]`. Size or type errors on such objects may occur
  if/when arithmetic operations are performed.

* In particular, a matrix is nothing but a vector of vectors. DB48X also
  supports arrays with dimensions higher than 2, like `[[[1 2 3]]]`.

* As a consequence, The `GET` and `GETI` functions work differently on
  matrices. Consider a matrix like `[[ 7 8 9 ][ 4 5 6 ][ 1 2 3 ]]`. On the HP48,
  running `1 GET` on this object gives `7`, and the valid range of index values
  is 1 through 9. On DB48X, that object is considered as an array of vectors, so
  `1 GET` returns `[7 8 9]`.  This is intentional. The behavior of `{ 1 1 } GET`
  is identical on both platforms, and is extended to multi-dimensional arrays,
  so that `[[[4 5 6]]] { 1 1 2 } GET` returns `5`.

* Matrices and vectors can contain integer values or fractions. This is closer
  to the HP50G implementation than the HP48's. In some cases, this leads to
  different results between the implementations. If you compute the inverse of
  `[[1 2 3][4 5 6][7 8 9]` on the HP48, you get a matrix with large values, and
  the HP48 finds a small, but non-zero determinant for that matrix. The HP50G
  produces a matrix with infinities. DB48X by default produces a `Divide by
  zero` error.

* DB48X accepts matrices and vectors as input to algebraic functions, and returns
  a matrix or vector with the function applied to all elements. For example,
  `[a b c] sin ` returns `[ 'sin a' 'sin b' 'sin c' ]`.

* Similarly, DB48X accepts operations between a constant and a vector or matrix.
  This applies the same binary operation to all components of the vector or
  matrix. `[ a b c ] x +` returns `[ 'a+x' 'b+x' 'c+x' ]`. Consistent with that
  logic, `inv` works on vectors, and inverts each component, so that
  `[1 2 3] inv` gives `[1/1 1/2 1/3]`.

* The `Min` and `Max` operations on arrays apply element by element, in a way
  similar to how these operations apply to lists on the HP50G (which seems to
  be undocumented).

* The `^` operation performs matrix operations if given an integer value as the
  right argument, but will perform element-wise otherwise. For example,
  `[[a b][c d] 2 ^` is the same as multiplying the array by itself, in other
  words it returns `[[ 'a²+b·c' 'a·b+b·d' ] [ 'c·a+d·c' 'c·b+d²' ]]`. However,
  `[[a b][c d] 2. ^` performs the operation element-wise and returns
  `[[ 'a↑2.' 'b↑2.' ] [ 'c↑2.' 'd↑2.' ]]`.

### Mathematics

* The `Σ` operation behaves differently between the HP48 and the HP50. On the
  HP48, `I A B 'I^3' Σ` gives an expression, `Σ(I=A;B;I^3)`, and an empty range
  like `I 10 1 'I^3' Σ` gives `0` as a value. On the HP50, this sum is
  simplified as a polynomial expression, so that you get a negative value if
  `A>B`. The HP50G behaviour seems surprising and undesirable. DB48X follows the
  HP48 approach.

* The `↑Match` and `↓Match` operations return the number of replacement
  performed, not just a binary `0` or `1` value. In addition, the patterns can
  identify specific kinds of values based on the first letter of the pattern
  variable name, e.g. `i` or `j` for positive integers, or `u` and `v` for
  unique terms, i.e. terms that are only matched once in the expression.

* When differentiating a user-defined function, HP calculators would replace `F`
  with `d1F`. The HP50G advanced reference manual suggests it should be `derF`.
  Thanks to Unicode support, DB48X will instead use `F′` as the name
  for the derivative function, making it closer to the standard mathematical
  notation. If `F` has multiple parameters, then the partial derivative relative
  to the first argument will be denoted as `F′₁`, the partial derivative
  relative to the second argument will be denoted as `F′₂` and so on.

* For built-in functions that have no known derivative, such as `GAMMA`, the
  HP50G would generate `d1GAMMA`, whereas DB48X will generate an
  `Unknown derivative` error.

* HP calculators would also accept `d1` for standard functions, which is only
  the name of the derivative relative to the first argument, but does not
  actually compute a partial derivative. For example, `d1SIN(2*X)` gives
  `COS(2*X)`, whereas `∂X(SIN(2*X))` evaluates as `2*COS(2*X)`. DB48X does not
  recognize this `dn` notation.

* The behaviour of the HP derivative function `∂` depends on whether it is in an
  algebraic object (stepwise differentiation) or whether it is used in stack
  syntax (full differentiation). The DB48X variant always perform full
  differentiation irrespective of the syntax used.

* The _HP50G advanced reference manual_ indicates that `∂` substitutes the value
  of global variables. For example, if `Y` contains `X+3*X^2`, `'Y' 'X' ∂` is
  expected to return `1+6*X`. It actually returns `0`, unless you evaluate `Y`
  first. DB48X matches the actual behaviour of the HP50G and not the documented
  one.


### Unicode support

DB48X has almost complete support for Unicode, and stores text internally using
the UTF-8 encoding. The built-in font has minor deviations in appearance for a
few RPL-specific glyphs.

Overall, a text file produced by DB48X should appear reliably in your
favorite text editor, which should normally be GNU Emacs. This is notably the
case for state files with extension `.48S` which you can find in the `STATE`
directory on the calculator.

The `Size` operation when applying to text counts the number of Unicode
characters, not the number of bytes. The number of bytes can be computed using
the `Bytes` command.

The `Num` and `Chr` commands deal with Unicode codepoints, and do not use the
special HP characters codes. In addition, `Num` return `-1` for an empty string,
not `0`. `0` is only returned for a string that begins with a ` NUL ` codepoint.

The `Code→Char` command can also be spelled as `Code→Text`, and take a list of
Unicode codepoints as input. Conversely, `Text→Code` will generate a list of all
the codepoints in a text.

## Help

The DB48X project includes an extensive built-in help, which you are presently
reading. This help is stored as a `help/db48x.md` file on the calculator. You
can also read it from a web browser directly on the GitHub page of the project.

The `Help` command makes it possible to access the built-in help in a contextual
way. It is bound to 🟦 _+_. If the first level of the stack contains a text
corresponding to a valid help topic, this topic will be shown in the help
viewer. Otherwise, a help topic corresponding to the type of data in the stack
will be selected.

The DB48X help viewer works roughly similarly to the DM42's, but with history
tracking and the ability to directly access help about a given function by
holding a key for more than half a second.

To navigate the help on the calculator, use the following keys:

* The soft menu keys at the top of the keyboard, references as _F1_ through
  _F6_, correspond to the functions shown in the six labels at the bottom of the
  screen.

* While the help is shown, the keys _◀︎_ and _▶︎_ on the keyboard scroll
  through the text.

* The _F1_ key returns to the [Home](#overview) of the help file, which gives an
  overview of the project and top-down navigation links.

* The _F2_ and _F3_ keys (labels `Page▲` and `Page▼`) scroll the text one full
  page at a time.

* The _F4_ and _F5_ keys (labels `Link▲` and `Link▼`) select the previous and
  next link respectively. The keys _÷_ and _9_ also select the previous
  link, while the keys _×_ and _3_ can also be used to select the next link.

* The _F6_ key corresponds to the `←Menu` label, and returns one step back in
  the help history. The _←_ key achieves the same effect.

* To follow a highlighted link, click on the _ENTER_ key.



## Acknowledgements and credits

DB48X is Free Software, see the LICENSE file for details.
You can obtain the source code for this software at the following URL:
https://github.com/c3d/db48x.


### Authors

This software is (C) 2022-2025 Christophe de Dinechin and the DB48X team.

Additional contributors to the project include (in order of appearance):

* Jeff, aka spiff72 (keyboard overlay)
* Camille Wormser (complex number fixes)
* Conrado Seibel (help file fix)
* Kjell Christenson (simulator fix)
* Václav Kadlčík (documentation fix)
* Franco Trimboli (WASM port)
* Jean Wilson (Equation Library and associated documentation)
* GitHub user mipa83 (Windows documentation)
* Philippe Martens (be.philippe@icloud.com)
* LdBeth (andpuke@foxmail.com) (Bug fixes)
* Thomas Eberhardt (sneakywumpus@gmail.com) (Bug fixes)
* Ed van Gasteren (Ed@vanGasteren.net) (Bug fixes)
* Jerome Ibanes (jibanes@gmail.com) (Dockerfile)
* Jesus Cano (jcanovel@gmail.com) (bug fixes)
* Riccardo Lucatuorto (gnuduncan@gmail.com)
* Ralf Ahlbrink (raprism@users.noreply.github.com)

The authors would like to acknowledge

* [Hewlett and Packard](#hewlett-and-packard)
* [The Maubert Team](#the-maubert-team)
* [Museum of HP calculators](#hp-museum)
* [HPCalc](#hpcalc)
* [The newRPL project](#newrpl-project)
* [The WP43 and C47 projects](#wp43-and-c47-projects)
* [SwissMicro's DMCP](#swissmicros-dmcp)

This work was placed by Christophe de Dinechin under the patronage of
[Saint Carlo Acutis](http://www.miracolieucaristici.org/en/Liste/list.html)


### Hewlett and Packard

Hand-held scientific calculators changed forever when Bill Hewlett and Dave Packard asked
their engineers to design and produce the HP35, then again when their company
introduced the first programmable hand-held calculator with the HP65, and
finally when they introduced the RPL programming language with the HP28.

Christophe de Dinechin, the primary author of DB48X, was lucky enough to meet
both Bill Hewlett and Dave Packard in person, and this was a truly inspiring experience.
Launching the Silicon Valley is certainly no small achievement, but this pales
in comparison to bringing RPN and RPL to the world.


### The Maubert Team

Back in the late 1980s and early 1990s, a team of young students with a passion
for HP calculators began meeting on a regular basis at or around a particular
electronics shop in Paris called "Maubert Electronique", exchanging
tips about how to program the HP28 or HP48 in assembly language or where to get
precious technical documentation.

It started with Paul Courbis, who carefully reverse-engineered and documented
[the internals of RPL calculators](https://literature.hpcalc.org/items/1584),
encouraging his readers to boldly cut open these wonderful little machines
to solder IR receivers acting as makeshift PC connection tools, or to waste
countless hours debugging [video games](https://www.hpcalc.org/hp48/games).

There were more serious efforts as well, notably the
[HP48 Metakernel](https://www.hpcalc.org/hp48/apps/mk/), which completely
reinvented the HP48 user interface, making it both much faster and better.  It
is fair to see DB48X as a distant descendent from such efforts. The Metakernel
was the work of many now well-known names in the HP community, such as Cyrille
de Brébisson, Jean-Yves Avenard, Gerald Squelart and Étienne de Foras. Many of
these early heroes would go on to actually change the [history of
Hewlett-Packard calculators](https://www.hpcalc.org/goodbyeaco.php) for the
better.

The original author of DB48X, Christophe de Dinechin, was part of this loose
team, focusing on [cross-development tools](https://github.com/c3d/HPDS),
which he used at the time to write several games for the HP48, notably
[PacMan](https://www.hpcalc.org/details/553) or
[Lemmings](https://www.hpcalc.org/details/530) clones. If DB48X exists, it's
largely because of that community.


### HP Museum

The [HP Museum](https://www.hpmuseum.org) not only extensively documents the
history of RPN and RPL calculators, it also provides a
[very active forum](https://www.hpmuseum.org/forum/) for calculator enthusiasts
all over the world.


### HPCalc

Much of the work from [early enthusiasts](#the-maubert-team) can still be found
on [hpcalc.org](https://www.hpcalc.org) to this day.

Back in the 1990s, long before Internet was widely available, HP48 programs were
busily swapped over floppy disks, or propagated from machine to machine using
the built-in infrared ports. This may have been the first case of large-scale
viral distribution of software. This is probably the reason why all this
software. which originated from all over the world, can still be downloaded
and used today.


### newRPL project

[newRPL](https://newrpl.wiki.hpgcc3.org/doku.php) is a project initiated by
Claudio Lapilli to implement a native version of RPL, initially targeting
ARM-based HP calculators such as the HP50G.

DB48X inherits many ideas from newRPL, including, but not limited to:

* Implementing RPL natively for ARM CPUs
* Adding indicators in the cursor to indicate current status
* Integrating a [catalog](#catalog) of functions to the command line

A first iteration of DB48X started as a
[branch of newRPL](https://github.com/c3d/db48x/), although the
current implementation had to restart from scratch due to heavy space
constraints on the DM42.


### WP43 and C47 projects

The DB48X took several ideas and some inspiration from the
[WP43](https://gitlab.com/rpncalculators/wp43) and
[C47](https://47calc.com) projects.

Walter Bonin initiated the WP43 firmware for the DM42 as a "superset of the
legendary HP42S RPN Scientific".

C47 (initially called C43) is a variant of that firmware initiated by Jaco
Mostert, which focuses on compatibility with the existing DM42, notably with
respect to keyboard layout.

DB48X borrowed at least the following from these projects:

* The very idea of writing a new firmware for the DM42
* The idea of converting standard Unicode TrueType fonts into bitmaps
  (with some additional contributions from newRPL)
* How to recompute the CRC for QSPI images so that the DM42 loads them,
  thanks to Ben Titmus
* At least some aspects of the double-shift logic and three-level menus
* The original keyboard layout template and styling, with special thanks
  to DA MacDonald.


### SwissMicros DMCP

[SwissMicros](https://www.swissmicros.com/products) offers a range of
RPN calculators that emulate well-known models from Hewlett-Packard.
This includes the [DM42](https://www.swissmicros.com/product/dm42),
which is currently the primary target for the DB48X firmware.

Special thanks and kudos to Michael Steinmann and his team for keeping
the shining spirit of HP RPN calculators alive.

The DM42 version of the DB48X software relies on
[SwissMicro's DMCP SDK](https://github.com/swissmicros/SDKdemo), which
is released under the following BSD 3-Clause License:

Copyright (c) 2015-2022, SwissMicros
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# Introduction to RPL

The original RPL (*Reverse Polish Lisp*) programming language was designed and
implemented by Hewlett-Packard for their calculators from the mid-1980s until
2015 (the year the HP50g was discontinued). It is based on older calculators
that used RPN (*Reverse Polish Notation*). Whereas RPN had a limited stack size
of 4, RPL has a stack size only limited by memory and also incorporates
programmatic concepts from the Lisp and Forth programming languages.

The first implementation of RPL accessible by the user was on the HP28C, circa
1987, which had an HP Saturn processor. More recent implementations (e.g., HP49,
HP50g) run through a Saturn emulation layer on an ARM based processor. These
ARM-based HP calculators would be good targets for a long-term port of DB48X.

DB48X is a fresh implementation of RPL on ARM, initially targeting the
SwissMicros DM42 calculator.
This has [consequences on the design](#design-overview) of this particular
implementation of RPL.

## The RPL stack

The RPL stack can grow arbitrarily in size.

By convention, and following RPN usage, this document gives the names `X`, `Y`,
`Z` and `T` to the first four levels of the stack. This is used to describe the
operations on the stack with synthetic stack diagrams showing the state of the
stack before and after the operation.

For example, the addition of two objects in levels 1 and 2 with the result
deposited in stack level 1 can be described in synthetic form using the
following stack diagram:

`Y` `X` ▶ `Y+X`

The duplication operation `Duplicate` can be described in synthetic form
using the following synthetic stack diagram:

`X` ▶ `X` `X`


## Algebraic mode

Unlike earlier RPN calculators from Hewlett-Packard, RPL calculators from HP
include complete support for algebraic objects written using the standard
precedence rules in mathematics. This gives you the best of both worlds,
i.e. the keyboard efficiency of RPN, requiring less keystrokes for a given
operation, as well as the mathematical readability of the algebraic
notation. Better yet, it is possible and easy to build an algebraic expression
from RPN keystrokes. These nice properties are also true for DB48X.

In RPL, algebraic expressions are placed between ticks. For
example, `'2+3×5'` will evaluate as `17`: the multiplication `3×5`, giving `15`,
is performed before the addition `2+15`, which gives `17`. An algebraic
expression can also be symbolic and contain unevaluated variables. For example,
`2+x` is a valid algebraic operation. If, having this expression on the stack,
you type `3` and then hit the `×` key, you will end up with `(2+x)×3`, showing
how the algebraic expression was built from RPN keystrokes.

Algebraic expressions are not evaluated automatically. The _R/S_ key (bound to
the [Evaluate](#evaluate) function) will compute their value as needed. On the
DB48X keyboard overlay, this key is also marked as `=` for that reason.

## Rich data types

Since introducing the first scientific pocket calculator, the HP-35, in 1972,
and with it the reverse polish notation (RPN), Hewlett-Packard perfected its
line-up for decades. This led to such powerhouse pocket computers as
the HP-41C series, or tiny wonders of pocket efficiency such as the HP-15C. Many
of these calculators, including the models we just cited, were capable of
advanced mathematics, including dealing with complex numbers, matrix operations,
root finding or numeric integration.

Then in 1986, everything changed with the HP-28C, which introduced a new user
interface called RPL. While the most evidently visible change was an unlimited
stack, what instantly made it both more powerful and easier to use than all its
RPN predecessors was the introduction of [data types](#types). Every value
on the stack, instead of having to be a number, could be a text, a name or an
equation. This made operations completely uniform irrespective of the data being
operated on. The same `+` operation that adds numbers can also add complex
numbers, vectors, matrices, or concatenate text. The exact same logic applies in
all case. This solved a decade-long struggle to extend the capabilities of
pocket calculators.

For example, whereas the HP-41C had some support for text, with an "Alpha" mode
and an alpha register, text operations were following their own logic, with for
example `ARCL` and `ASTO` dealing with at most 6 characters at a time, because
they were artificially fitted in a register designed to hold a numerical value.
Dealing with complex numbers on the HP-41C was
[similarly clunky](https://coertvonk.com/sw/hp41/complex-arithmetic-xmem-4426).
Even the HP-15C, which had built-in support for complex numbers, remained a bit
awkward to use in "complex mode" because its display could only show one half of
a complex number, e.g. the real or imaginary part. Similarly, matrix or
statistic operations had non-obvious interactions with numbered data registers.

All this was solved with RPL, because now a complex number, a matrix or a text
would occupy a single entry on the stack. So whereas adding two integers would
require a sequence like _1_ _ENTER_ _2_ _+_ like in RPN, a very similar sequence
would add two texts: `"ABC"` _ENTER_ `"DEF"` _+_, and the exact same logic would
also add two vectors in `[1 2 3]` _ENTER_ `[4 5 6]` _+_.

DB48X adopts this extremely powerful idea, with a focus on making it as
efficient as possible for interactive calculations as well as for custom
programmed solution.
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

The pre-built firmware for the DM-42 is split into two components, `db48x.pgm`
and `db48x_qspi.bin`. The built-in help is stored in a file called `db48x.md`.


In addition, a file called `Demo.48s` contains a few sample RPL programs to
illustrate the capabilities of this new firmware, two comma-separated values
files `units.csv` and `constants.csv`, which define the units and constants
respectively.

### Connecting the calculator to a computer

The DM-42 calculator connects to your computer using a standard micro-USB cable.



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

* `db48x.pgm` and `db48x_qspi.bin` in the root directory of the calculator's USB
  disk.

* `db48x.md` should be placed in a directory called `help`.


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
# Types

DB48X, [like HP RPL](#rich-data-types), supports a wide variety of data types.


## Integers

The DB48X version of RPL distinguishes between integer values, like `123`, and
[decimal values](#decimal-numbers), like `123.` Integer values are represented
internally in a compact and efficient format, saving memory and making
computations faster. All values between -127 and 127 can be stored in two bytes.
All values between -16383 and 16383 in three bytes.

Integers can be [as large as memory permits](#big-integers). They represent
values in the mathematical sets known as ℕ (natural numbers, i.e. positive
integers) and ℤ (positive and negative integers).


## Big integers

The DB48X version of RPL can perform computations on arbitrarily large integers,
limited only by available memory, enabling for example the exact computation of
`100!` and making it possible to address problems that require exact integer
computations, like exploring the Syracuse conjecture.

## Fractions

Fractions represent a ratio between two integers, like `2/3`. They represent
values in the mathematical set known as ℚ. In DB48X, fractions are first-class
objects, allowing exact and relatively inexpensive operations in many common
cases.


## Decimal numbers

Decimal numbers are used to represent values with a fractional part.
DB48X supports three internal representations for decimal numbers:

* Hardware accelerated 32-bit IEEE-754 with a binary representation. This is
  similar to the `float` type in a programming language like C.

* Hardware accelerated 64-bit IEEE-754 with a binary representation. This is
  similar to the `double` type in a programming language like C.

* Software variable-precision decimal representation, which is much slower.
  The default configuration selects 24 digits of precision.

Decimal numbers of various size and formats can be mixed and matched
transparently. It is valid and safe to adjust `Precision` settings along a
computation to only pay the cost of increased precision where it matters.

For example, you can compute the following expression at various precisions:

```rpl
'(SQRT(2)-1)^10-(3363-2378*SQRT(2))' DUP
512 Precision EVAL 'Precision' PURGE
@ Expecting 5.99488 3⁳⁻⁵⁰⁹
```

In a calculator, decimal numbers are always part of the mathematical set known
as ℚ, and more precisely of a subset sometimes written as 𝔻. However, they are
often improperly considered as an approximation of the set of *real* numbers, ℝ.


### Variable-precision decimal

DB48X features a variable-precision decimal implementation of floating point,
like [newRPL](#newRPL-project).

The [Precision](#precision) command selects the precision for computations,
given as a number of digits.

Internally, computations are not done in base 10, but in base 1000, which allows
a more compact decimal representation. Each base-1000 "kigit" uses 10 bits in
memory. As a result:

* From a performance point of view, the cost of computations is determined by
  the `Precision` divided by 3 and rounded up to the next integer, corresponding
  to one new base-1000 kigit being added.

* From a memory point of view, `Precision` values that are multiples of 12 are
  optimal, because four base-1000 kigit require 40 bits, which line up perfectly
  with five 8-bit bytes. `Precision` values that are not multiples of 12 will
  contain a few unused bits.

The `typename` command returns `"decimal"` for variable-precision decimal
floating-point values.

When `HardwareFloatingPoint` is disabled, numerical values entered on the command
line or in a program are always stored internally with all the digits entered,
and only those. For example, if you enter a decial number with 3 digits or less,
it will only use 5 bytes:

```rpl
1.23 BYTES
@ Expecting 5
```

On the other hand, if you enter a constant with a high number of digits, then
all the digits will be preserved internally irrespective of the `Precision`
setting:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
BYTES
@ Expecting 37
```

Computations are performed according to the `Precision` setting, irrespective of
the precision of input values. For example, the following computation is
guaranteed to gives `0.` irrespective of `Precision`, even if one of the inputs
to `+` has a larger number of digits stored internally:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
DUP 0 SWAP - +
@ Expecting 0.
```

If desired, the larger number of digits in the user-entered constant can be
exploited by setting `Precision` before the digit-cancelling operation, as
shown below:

```rpl
3.141592653589793238462643383279502884197169399375105820974944592307816406286208
DUP 0 SWAP -     @ Recompute value at 24 digits
60 Precision +   @ Computation at 60 digits
'Precision' PURGE
@ Expecting 3.38327 95028 8⁳⁻²⁴
```


### Binary floating-point

The 32-bit format offers a 7 digits mantissa and has a maximum exponent of 96.
The 64-bit format offers a 16 digits mantissa and has a maximum exponent of 384.

The benefits of the binary floating-point representation are:

* It delivers higher performance using hardware acceleration.
* It is compatible with the IEEE-754 representation used in most computers.
  If you need to replicate results computed by a desktop computer, this may be
  the best format to use for that reason.

The primary drawback of this representation is that it cannot represent some
decimal values exactly, in the same way as `1/3` cannot be represented exactly
in decimal. For example, `0.2` cannot be represented exactly using a binary
floating-point representation.

Using a binary format is not recommended if you need exact results on decimal
values, for example adding monetary amounts. As an illustration, if you enable
16-digit hardware binary floating-point and add `0.20` and `0.45`, the result is
`0.65000 00000 00000 022`. This is not a bug, but a limitation of the
floating-point representation. The same computation with `SoftwareFloatingPoint`
gives the exact expected result `0.65`.

The `typename` command returns `"hwfloat"` for 32-bit binary floating-point
numbers, and `"hwdouble"` for 64-bit binary floating-point numbers.

### HardwareFloatingPoint

This command enables IEEE-754 accelerated binary floating point. It is the
opposite of `SoftwareFloatingPoint`.

When this setting is active, a 32-bit IEEE-754 format is used for `Precision`
values below 7, and a 64-bit IEEE-754 format is used for `Precision` values
below 16. Variable-precision decimal computations are used for `Precision`
settings of 17 and higher.

The `HardwareFloatingPoint` setting is disabled by default because of the
inherent precision loss incurred by the binary format when dealing with decimal
numbers. For example, `0.2` cannot be represented exactly using a binary format.

### SoftwareFloatingPoint

This command disables accelerated binary floating point, and ensures that the
variable-precision decimal floating-point format is used even for `Precision`
values below 16.. This setting is the opposite of `HardwareFloatingPoint`.


## Based numbers

Based numbers are used to perform computations in any base. The most common
bases used in computer science, 2, 8, 10 and 16, have special shortcuts.
The `BasesMenu` list operations on based numbers.

Like integers, based numbers can be [arbitrary large](#big-integers).
However, operations on based numbers can be truncated to a specific number of
bits using the `WordSize` setting. This makes it possible to perform
computations simulating a 16-bit or 256-bit processor.


## Boolean values

DB48X has two boolean values, `True` and `False`. These values are typically
returned by operations such as tests that return a truth value.

In addition, numerical values are interpreted as being `False` if the value is
0, and `True` otherwise. This applies to conditional tests, conditional loops,
and other operations that consume a truth value.

## Complex numbers

Complex numbers can be represented in rectangular form or polar form.
The rectangular form will show as something like `2+3ⅈ` on the display, where
`2` is the real part and `3` is the imaginary part. The polar form will show as
something like `1∡90°` on the display, where `1` is the modulus and `90°` is the
argument. The two forms can be mixed and matched in operations. The calculator
typically selects the most efficient form for a given operation.

Available operations on complex numbers include basic arithmetic, trigonometric,
logarithms, exponential and hyperbolic functions, as well as a few specific
functions such as `conj` or `arg`. These functions are available in
the `ComplexMenu`.

## Ranges

Ranges (also called *intervals*) represent a range of numbers that show as `x…y`
and represent any value that is known to be between `x` and `y`. There are three range types that differ in the way they display:

* Explicit ranges show the lower and upper bounds, e.g. `2…4` to represent
  the range of numbers between `2` and `4`.
* Delta ranges show the middle point and the distance between that middle point
  and the lower or upper bounds. For example `3±1` also represents values
  between `2` and `4`.
* Percent ranges show the middle point and a percentage of that middle value to
  the lower and upper bound. For example, `2±50%` represents values between `1`
  and `3`, the distance between `1` and `2` being `50%` of `2`. Note that if the
  central value is `0`, the percentage will be shown relative to `1` and not
  `0`, so that `-3…3` will show as `0±300%`

The `Cycle` command (bound to the _EEX_ key) will cycle between the three range
formats.

Operations on ranges will compute the upper and lower bound of the result to
create a new range. This applies to arithmetic operations:

```rpl
1…3 2…6 ÷
@ Expecting ¹/₆…1 ¹/₂
```


## Uncertain numbers

Uncertainties represent a range of numbers that show as `a±σs` and represent a
value using its average `a` and a standard deviation `s`.

Computations on uncertain numbers will [propagate uncertainty](https://en.wikipedia.org/wiki/Propagation_of_uncertainty) using generalized quadratic superposition.

The value of the variable `ρ` determines the correlation between the two
variables. If the variable is not set, a value of `0` is assumed
(no correlation):

```rpl
4±σ3 7±σ5 *
@ Expecting 28±σ29.
```

When `ρ` is set to 1, we have correlation:

```rpl
ρ=1 4±σ3 7±σ5 *
@ Expecting 28±σ41.
```

When `ρ` is set to -1, we have anti-correlation:

```rpl
ρ=-1 4±σ3 7±σ5 *
@ Expecting 28±σ1.
```

## Expressions

Algebraic expressions and equations are represented between quotes, for example
`X+1` or `A+B=C`. Many functions such as circular functions, exponential, logs
or hyperbolic functions can apply to algebraic expressions.

An expression that contains an equal sign, e.g. `sin X + 1 = cos X`, is called
an *equation*. It can be given as an argument to the solver.


## Symbols

A symbol is a sequence of characters such as `Hello` or `A→B` that can be used
ot identify variables. Evaluating a symbol evaluates the underlying variable if
it exists, or evaluates as itself otherwise.

## Text

Text is represented as a sequence of Unicode characters placed between double
quotes, such as `"Hello"`. Operations on text include concatenation using the
`+` operator, and multiplication by a positive integer to repeat a text.

## Programs

Programs are represented as RPL objects enclosed between the `«` and `»`
delimiters, and containing a sequence of RPL objects. Running (or evaluating) a
program is done using the `Run` (the _=_ key) or `Eval` commands, which evaluate
each RPL object in the program in turn.

See [RPL Programming](#rpl-programming) for more details and examples about
programming your DB48x.

## Lists

Lists are sequence of items between curly braces, such as `{ 1 'A' "Hello" }`.
They can contain an arbitrary number of elements, and can be nested.

Operations such as `sin` apply to all elements on a list.


## Vectors and matrices

Vector and matrices represent tables of numbers, and are represented between
square brackets, for example `[1 2 3]` for a vector and `[[1 2] [3 4]` for a 2x2
matrix.

Vector and matrices follow their own arithmetic rules. Vectors are
one-dimensional, matrices are two-dimensional. DB48X also supports tables with a
higher number of dimensions, but only offers limited operations on them.

DB48X implements vector addition, subtraction, multiplication and division,
which apply component-wise. Multiplication and division are an extension
compared to the HP48.

DB48X also implements matrix addition, subtraction, multiplication and
division. Like on the HP48, the division of matrix `A` by matrix `B` is
interpreted as left-multiplying `A` by the inverse of `B`.

As another extension, algebraic functions such as `sin` apply to all elements in
a vector or matrix in turn.


## Units

Unit objects represent values with an associated unit. They are represented
using the `_` operator, e.g. `1_km/s`, although on display this operator is
shown as a thin space, e.g. `1 km/s`.

Units as implemented in DB48X are modernized compared to what the HP48
implements, and differ from the HP RPL implementation in the following ways:

* Add [recent SI prefixes](https://www.nist.gov/pml/owm/metric-si-prefixes),
  Z (zetta), Y (yotta), R (ronna) and Q (quetta) for large scale,
  z (zepto), y (yocto), r (ronto) and q (quecto) for small scale.

* Take into account the impact on unit conversions of the
  [revised 2023 definition of the foot](https://www.nist.gov/pml/us-surveyfoot/revised-unit-conversion-factors).

* Use exact (fraction-based) conversions wherever possible. This notably matters
  for the conversions of pre-2023 US Survey units, where the ratio is
  `1_ft = 1200/3937_m`, which is not well represented using decimal values.

* Add computer-related units, like the `byte`, the `bit`, the `baud`, as well
  as a menu supporting these units.

* In order to support the computer-related units better, also recognize the
  [power-of-two variants](https://en.wikipedia.org/wiki/Kilobyte),
  e.g. `1_kiB` is `1024_B`. Also recogize the `K` prefix in addition to `k`.

### Units file

The built-in units can be overriden by your own set of units, which is defined
in a CSV file called `config/units.csv` in the calculator. CSV stands for "Comma
separated values, and is a common interchange format for spreadsheet data.

Here is an example of file that would let you have a units menu called `Money`
to convert between various monetary units:

```
"Money"
"USD", "1_USD"
"EUR", "1.07_USD"
"GBP", "1.24_USD"
"CAD", "0.73_USD"
"AUD", "0.65_USD"
"CHF", "1.11_USD"
```

* All values must be placed between quotes. Separators between text values are
  mostly ignored.

* Rows in a file containing a single value denote unit menus, unless the value
  begins with an `=` sign.

* Rows in a file containing two ore more values denote unit menu entries, which
  will be added to the previous menu.

* The first column in these rows give the name of the unit as shown in the menu.

* The second column in these rows gives the definition of the unit.

* The definition should be reduced to `=` if the first column contains what
  would be a valid unit expression. For example, to place `km/h` in a menu, use
  `"km/h", "="` since `km` can be deduced from existing unit `m` using the
  standard "kilo" unit prefix, and `h` is an existing unit.

A unit where the value is `1` of the same unit is a base unit. This is the case
for `USD` in the example above, which is considered the base units for monetary
exchanges. Units that refer to the same base unit can be converted with one
another. For example, you can convert between `GBP` and `AUD` because they both
have the same `USD` base unit.

The commands `ShowBuiltinUnits` and `HideBuiltinUnits` indicate if the built-in
uits should be shown after the units loaded from the file. The default is that
when a units file is present, the built-in units are hidden. This only affects
the menus. Built-in units can always be used in expressions if they are typed
manually. However, units loaded from file will be looked up first, so that a
built-in unit can be overriden by the units file, which can be useful if a
definition changes like the US Survey foot changed on January 1st, 2023.

If you build a units file, it is recommended that you do not exceed 17 unit
submenus so that all unit categories fit on a single screen.


### Cycle command customization

The menu name `"=Cycle"` is reserved to define sequences of units that the
`Cycle` command (bound to the _×10ⁿ_ key) will recognize as special. For
example, you can ensure that `mm` and `in` convert to one another as follows:

```
"=Cycle"
"in", "mm"
"mm", "in"
"USD", "EUR"
"EUR", "CHF"
"CHF", "USD"
```

If you do provide a `Cycle` customization for a unit, other normal behaviours of
the `Cycle` command for units are disabled, notably conversion between various
relevant scales and conversion between fractions and decimal. To force a
particular conversion to happen in decimal, you can override the definition of
the corresponding unit in the units file, for example:

```
"in",   "25.4_mm"
```


## Constants

Constant objects represent named values like Euler's constant `e`, the ratio
between circumference and diameter of a circle `π`, or the speed of light `c`.
They are represented by a name, and have an associated value.

Like units, there are some built-in constants, and additional constants can be
provided by a `config/constants.csv` file, which has the same format as
for the units file.

On the command-line, constant names are prefixed with _CST_, which is a way to
distinguish them from normal symbols.

You can edit the constants catalog by recalling its content on the stack using
`"config:equations.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:equations.csv" STO`.


## Infinite results

Some operations such as `1/0` or `tan 90 °` are said to produce an
*infinite result*. Like HP calculators, DB48X can either generate an error or
produce a special result in these cases.

* If the `InfinityValue` (-22) flag is clear, corresponding to the
  `InfinityError` setting, then the operation generates a
  `Division by zero` error. Note that the text of the error is different than
  for Hewlett-Packard calculators, which generate an `Infinite result` error.

* If the `InfinityValue` flag is set and `NumericalConstants` (-2) flag
  is clear, corresponding to the `SymbolicConstants` setting, then the operation
  generates the `∞` (infinity) constant with the appropriate sign for the
  result, and the `InfiniteResultIndicator` (-26) flag is set.

* If the `InfinityValue` flag is set and `NumericalConstants` flag is set,
  then the operation generates the numerical value associated to the `∞`
  constant with the appropriate sign for the result, and set the
  `InfiniteResultIndicator` flag.

By default, the numerical value of the `∞` constant is set to `9.99999E999999`,
which is significantly smaller than what would actually cause a numerical
[overflow](#overflow-and-underflow), but is easy to read. This value can be
changed in the `config/constants.csv` file.


## Overflow and underflow

There is a maximum representable value for [decimal numbers](#decimal-numbers).
This value is significantly larger than on HP calculators. Whereas HP RPL
implementations could not represent decimal numbers with an exponent bigger than
499 or smaller than -499, DB48X supports exponents ranging from -2^60 to 2^60
(±1 152 921 504 606 846 976).

An *overflow* happens if the result would have an exponent higher than the
maximum. An *underflow* happens if the result would have an exponent lower than
the minimum. Like HP calculators, DB48X can either generate an error or
produce a special result in these cases.

* If the `UnderflowValue` (-20) or `OverflowValue` (-21) is clear,
  corresponding to the `UnderflowError` or `OverflowError`
  setting, then the operation generates a `Positive numerical underflow`,
  `Negative numerical underflow` or `Numerical overflow` error depending on the
  computation. Note that the text of the error is different than for
  Hewlett-Packard calculators, which generate an `Overflow`, `Positive
  Underflow` or `Negative Underflow` error.

* If the `UnderflowValue` or `OverflowValue` is set,
  and `NumericalConstants` (-2) flag is clear, corresponding to the
  `SymbolicConstants` setting, then overflowing operations generate the `∞`
  (infinity) constant with the appropriate sign for the result, and underflowing
  operations generate a zero value. The operation also sets the
  `NegativeUnderflowIndicator` (-23), `PositiveOverflowIndicator` (-24) or
  `OverflowIndicator` (-25) flag.

* If the `UnderflowValue` or `OverflowValue` is set, and
  `NumericalConstants` flag is set, then overflowing operations generate the
  numerical value associated to the  `∞` constant, and underflowing operations
  generate a zero value. The operation also sets the
  `NegativeUnderflowIndicator` (-23), `PositiveOverflowIndicator` (-24) or
  `OverflowIndicator` (-25) flag.


## Undefined

Some operations such as `0/0` are *undefined*, meaning that there isn't a single
possible answer.

If the `UndefinedValue` flag is set, such operations return the constant
`?`, and further operations on the value will keep returning the same undefined
result.

If the `UndefinedValue` flag is is clear, which corresponds to `UndefinedError`
being set, such operations will generate an `Undefined operation` error.


## Library

The `Library` is a catalog of frequently used and rarely modified objects that
are stored on disk in the `config/library.csv` file.

You can edit the library content by recalling its content on the stack using
`"config:library.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:library.csv" STO`.

Library entries are cached in memory for efficiency.
See [library management](#library-management) for operations to use when you
update the library content.


## Equations Library

The equations library is a catalog of common equations that are stored on disk
in the `config/equations.csv` file.

You can edit it by recalling its content on the stack using
`"config:equations.csv" RCL`, editing the values, and then storing the content
back to disk using `"config:equations.csv" STO`.
# RPL Programming

If you’ve used a calculator or computer before, you’re probably familiar with
the idea of programs. Generally speaking, a program is something that gets the
calculator or computer to do certain tasks for you — more than a built-in
command might do. In DB48x, like in the HP 48gII, HP 49g+, and HP 50g
calculators, a program is an object that does the same thing.

## Understanding programming

A calculator program is an object with `«` `»` delimiters containing a sequence
of numbers, commands, and other objects you want to execute automatically to
perform a task.

For example, a program that takes a number from the stack, finds its factorial,
and divides the result by 2 would look like this: `« ! 2 / »`:

```rpl
6
«
	!  2 /
»
EVAL
@ Expecting 360
```

### The Contents of a Program

As mentioned above, a program contains a sequence of objects. As each object is
processed in a program, the action depends on the type of object, as summarized
below:

* Commands like `sin` are executed
* Numbers like `12.34` are put on the stack
* Quoted names and algebraics like `'A'` or `'X+Y'` are put on the stack
* Texts like `"Hello"` are put on the stack
* Lists and arrays like `{ 1 2 3}` or `[ A B C]` are put on the stack
* Programs like `« 1 + »` are put on the stack
* Unquoted names like `Foo` are evaluated. Programs and commands in variables
  are executed, names are evaluated, directories become current, other objects
  are put on the stack.

Note that on DB48x, the behaviour is identical for local and global names. This
is a [difference](#differences-with-other-RPLs) relative to HP calculators.

As you can see from this list, most types of objects are simply put on the
stack, but built-in commands and programs called by name cause *execution*. The
following examples show the results of executing programs containing different
sequences of objects.


### Program Structures

Programs can also contain structures. A structure is a program segment with a
defined organization. Two basic kinds of structure are available:

* *Local variable structure*. The `→` operator defines
  [local variables](#local-variables), followed by an algebraic or
  program object that’s evaluated using those variables.
* *Branching structures*. Structure words (like `DO`…`UNTIL`…`END`) define
  [conditional](#conditionals) or [loop](#loops) structures to control the order
  of execution within a program.


### Local variables

A local variable structure has `→` followed by local names, followed by either a
program or an algebraic. This removes values from the stack, puts them in the
local variables, and then evaluates the algebraic or program.

For example, the following program takes two numbers from the stack and returns
the absolute value of their difference:

```rpl
3 5
« → a b 'ABS(a-b)' »
EVAL
@ Expecting 2
```

When evaluating the algebraic expression `'ABS(a-b)'`, the value `3` is put in
local variable `a` and the value `5` is put in local variable `b`.


### Calculations in a program

Many calculations in programs take data from the stack. Two typical ways to
manipulate stack data are:

* *Stack commands* that operate directly on the objects on the stack.
* *Local variable structures* that store the stack objects in temporary local
  variables, then use the variable names to represent the data in the following
  algebraic or program object.

Numeric calculations provide convenient examples of these methods. The following
programs use two numbers from the stack to calculate the hypotenuse of a right
triangle using the square root of the sum of the squares (Pythagorean theorem).

The first program uses stack commands to manipulate the numbers on the stack,
and the calculation uses stack syntax.

```rpl
3 4
« SQ SWAP SQ + √ »
EVAL
@ Expecting 5.
```

The second program uses a local variable structure to store and retrieve the
numbers, while the calculation uses stack syntax. In that case, the value `3` is
stored in local variable `x`, and the value `4` is stored in local variable
`y`.

```rpl
3 4
« → x y « x SQ y SQ + √ » »
EVAL
@ Expecting 5.
```

The third program also uses a local variable structure, but this time the
calculation uses algebraic syntax.

```rpl
« → x y '√(x^2+y^2)' »
```

Note that the underlying formula is most apparent in the third program. This
third method is often the easiest to write, read, and debug.


### Efficiency vs. readability

Programmers should be be aware that the DB48x implementation of local variables
makes accessing them as efficient as accessing a stack value. Furthermore, using
local variables often makes it possible to avoid stack manipulation commands.

Consequently, this programming style can often lead to an implementation that is
both more readable and more efficient than using complicated stack manipulations
with commands such as `Swap`, `Rot` or `Over`.

This is particularly true when a value is used multiple times, as shown in the
following example:

```rpl
2 3
« → x y 'x^y-(x+y)/(x^2-y^2)' »
EVAL
@ Expecting 9
```

An equivalent program using stack operations could be written as follows:

```rpl
2 3
« DUP2 POW UNROT DUP2 + UNROT SQ SWAP SQ SWAP - / - »
EVAL
@ Expecting 9
```

The first program is more readable. Both implementations run at almost exactly
the same speed. However, the second version uses half the number of bytes in
program memory (17 vs. 34), primarily because each local variable reference uses
at least two bytes, whereas most stack manipulation operations only use
one. This difference may become lower as the program grows larger, since the
program may require more complicated stack operations such as `Pick`.

## Entering and running programs

The programs in this section demonstrate basic programming concepts in
[RPL](#introduction-to-rpl). They are intended to develop and improve your
programming skills, and to provide supplementary functions for your
calculator. The DB48X calculator features a library of introductory programs
covering mathematics, physics and computer science, which is accessible using
the `Library` command, 🟦 _H_ (_LIB_).

### What defines a RPL program?

A RPL program is a regular [RPL object](#programs) describing a procedure
consisting in a space-separated sequence of RPL objects such as numbers,
algebraic and RPN instructions. The whole sequence is enclosed between the `«`
and `»` delimiters.

### Entering a program

To enter a program, use the 🟨 _=_ (`«PROG»`) key, which puts `« »` in the text
editor, with the cursor in the middle. One enters the sequence of instructions
defining the procedure at the position indicated by the cursor. The _Enter_ key
then enters the sequence as an object on the stack. If there is an error in the
program, it will be reported, and the cursor will be positioned next to it.

### Naming programs

Programs can be stored in variables, like any RPL object. To store a program in
a variable, enter the program to put it on the stack, then use the _'()_ key and
alphabetic mode to enter a name, and store the program on the stack using the
`Store` command (_STO_).

### Running programs

A program can be executed by evaluating it, typically using the _=_ key, which
is bound to the `Run` command. The `Run` and `Eval` commands also execute
programs.

Evaluating the name of the variable evaluates the program. The program can also
be executed quickly using function keys while the `VariablesMenu` (_VAR_ key) is
active.

There are four ways to run a program:

* Press the _VAR_ key, and then the menu key for the program name
* Enter the program name without delimiters, then press _Enter_
* Put the program on the stack and press _=_ to activate the `Run` command
* Put the program name on the stack and press _=_ to activate the `Run` command

### Stopping a program

A program can be interrupted while it's running using the _EXIT_ key.
If `DebugOnError` is active, then the program can be single-stepped with `Step`,
and execution can be resumed using `Continue`. These commands are available from
the `DebugMenu`.


## Volume of a sphere

The following program computes the volume of a sphere given the radius put on
the stack, using stack-based programming, and stores it in a variable named
`VOL`:

```rpl
« 3 ^ Ⓒπ * 4 3 / * →NUM »
'VOL' STO
4 VOL
@ Expecting 268.08257 3106
```

We need the `→NUM` command in this program to get a numerical result, because
the `Ⓒπ` constant, by default, evaluates symbolically.
This can be changed using the `NumericalConstants` or `NumericalResults`
settings, or, for compatibility with the HP48 calculator, using the `SF` command
on flags `-2` or `-3` respectively.

The following is the same program using an algebraic expression for readability:

```rpl
« → r '4/3*Ⓒπ*r^3' →NUM »
'VOL' STO
4 VOL
@ Expecting 268.08257 3106
```


## Volume of a spherical cap

Instead of local variables, a program can take input from global variables.
The following program, `SPH`, calculates the volume of a spherical cap of height
*h* within a sphere of radius *R* using values stored in variables `H` and `R`.
We can then use assignments like `R=10` and `H=3` to set the values before we
run the program.

```rpl
« '1/3*Ⓒπ*H^2*(3*R-H)' →NUM »
'SPH' STO

R=10 H=3 SPH
@ Expecting 254.46900 4941
```

Alternatively, we can use the `STO` command sto initialize the values for `R`
and `H`:

```rpl
« '1/3*Ⓒπ*H^2*(3*R-H)' →NUM »
'SPH' STO

10 'R' STO
3 'H' STO
SPH
@ Expecting 254.46900 4941
```

## Viewing and editing programs

You view and edit programs the same way you view and edit other objects, using
the command line.

### Edit from level 1

To view or edit a program placed in level 1 on the stack, use the _▶_ key to
place the program in the text editor. Then edit it normally, and press _ENTER_
to put it back on the stack, or _EXIT_ to close the text editor leaving the
program unmodified on the stack.

### Edit from the stack

To view or edit a program placed at some other level on the stack, first enter
the [interactive stack](#interactive-stack) using the _◀︎_ key, and then moving
up and down using the _◀︎_ and _▶_ keys. Then use the _F6_ key to edit the
program. After editing the program, you can validate the modifications with the
_ENTER_ key, or cancel the changes with the _EXIT_ key.

### Edit from variables

When a program is placed in a variable, you need to recall it first to place
it on the stack, and then store it back. To that effect, you can use the
`Recall` and `Store` commands with a name on the stack. A faster way is to use
the `VariablesMenu`, which you can access using the _VAR_ key. Using 🟨 and a
function key recalls the corresponding variable. Using 🟦 and a function key
stores into the corresponding variable.

## Creating programs on a computer

It is convenient to create programs and other objects on a computer and then
load them into the calculator. This is typically done by editing a text file
with extension `.48s`, and then storing them on the internal storage of DM42.
The state files stored under `STATE` are such files, and example being the file
named `STATE/Demo.48s` that comes with the DB48x distribution.


### Comments

If you are creating programs on a computer, you can include *comments* in the
computer version of the program. Comments are free text annotations that a
programmer can add to document a program.

Comments in a DB48x program begin with `@` or `@@`, and finish at the end of a
line.  Comments that begin with `@` are preserved in a program, while comments
that begin with `@@` are removed while loading a program.

The following program is the program from an
[earlier section](#calculations-in-a-program) computing the hypothenuse of a
square rectangle, with comments added:

```rpl
«
@@ Compute the hypothenuse of a square rectangle
@@ Input is from the two levels of the stack
@@ Output is left on the first level of the stack
@@ These comment will be removed from the program
@@ The comments below will remain the program

@ Square first side
SQ

@ Get second side and square it
SWAP SQ

@ Add the two squares
+

@ Take the square root
√
»
```

You can check when you enter this program from the help file that all the
`@@` comments at the top are removed, while the `@` comments in the middle
remain in the resulting program.


## Using local variables

The program SPH in the previous example uses global variables for data storage
and recall. There are disadvantages to using global variables in programs:

* After program execution, global variables that you no longer need to use must
be purged if you want to clear the `VariablesMenu` (_VAR_ key) and free user
memory.

* You must explicitly store data in global variables prior to program execution,
  or have the program execute `STO`.

*Local variables* address the disadvantages of global variables in
programs. Local variables are temporary variables *created by a program*.
They exist only while the program is being executed and cannot be used outside
the program.  They never appear in the `VariablesMenu` (_VAR_ key).
In addition, local variables are accessed faster than global variables, with a
cost very similar to a stack access, and they generally require use less memory.

### Creating Local Variables

In a program, a [local variable structure](#local-variable) creates local
variables.  To enter a local variable structure in a program, the fastest method
is to use the `ProgramMenu` (🟨 _3_), and then the `→  «»` data entry key
(_F4_). To enter a local variable structure for an algebraic expression, use the
`→  ''` data entry key instead (_F5_).

On the right of the `→` symbol, use the names of the local variables, separated
by spaces. Then, inside `«»`, enter an RPL program that uses the local
variables, or alternatively, inside `''`, enter an algebraic expression.

For example, if the stack contains `10` in level 3, `6` in level 2 and `20` in
level 1, then:

* `→ a « a a ^ »` or `→ a 'a^a'` will execute with `a` containing `20`, and then
  compute evaluate `20^20`, i.e. `104 857 600 000 000 000 000 000 000`.

* `→ a b « a b ^ »` or `→ a b 'a^b'` will execute with `a` containing `6`, and
  `b` containing `20`, then compute evaluate `6^20`, i.e.
  `3 656 158 440 062 976`.

* `→ a b c « a b ^ c + »` or `→ a b c 'a^b+c'` will execute with `a` containing
  `10`, `b` containing `6` and `c` containing `20`, then compute evaluate
  `10^6+20`, i.e.  `1 000 020`.

### Advantages of Local Variables

Local variable structures have these advantages:

* The `→` command stores the values from the stack in the corresponding
  variables: you don’t need to explicitly execute `STO`.

* Local variables automatically disappear when the defining procedure for which
  they are created has completed execution. Consequently, local variables don’t
  appear in the `VariablesMenu`, and they occupy user memory only during program
  execution.
* Local variables exist only within their defining structure. Different local
  variable structures can use the same variable names without conflict,
  including in the same program.
* Local variables consume less memory than a global variable, both in a program
  and in memory.

### Memory Usage Comparison

Each global variable name with `n` characters uses `n+2` bytes (assuming `n` is
less than 128). An additional `n+2` bytes are also used in the directory
containing the global variable. If a program uses a global variable `VOLUME`
three times, that's 24 bytes of memory in the program, and 8 additional bytes
in the directory.

Each local variable name with `n` characters uses `n+1` bytes in the local
variables structure, an additional 4 bytes of temporary storage, and each
reference in a program uses two bytes (assuming there are less than 128 local
variables). If a program uses a local variable `volume` three times, that's 13
bytes of memory in the program (insstead of 24) and 4 bytes in temporary storage
(instead of 8).


### Spherical cap using local variables

The following program SPHLV calculates the volume of a spherical cap using local
variables. The defining procedure is an algebraic expression.

```rpl
«
  @@ Creates local variables r and h for the radius
  @@ of the sphere and height of the cap.
  @@ Expresses the defining procedure. In this
  @@ program, the defining procedure for the local
  @@ variable structure is an algebraic expression.
  → r h
  '1/3*Ⓒπ*h^2*(3*r-h)'

  @@ Converts expression to a number.
  →NUM
»
'SPHLV' STO
@ Save for later use
```

Now, we can use `SPHLV` to calculate the volume of a spherical cap of radius
`r=10` and height `h=3`. We enter the data on the stack in the correct order,
then execute the program either by name or by pressing the corresponding
unshifted key in the `VariablesMenu`:

```rpl
10 3 SPHLV
@ Expecting 254.46900 4941
```


### Evaluating Local Names

On DB48x, local names evaluate just like global names.

By contrast, on HP calculators, local names are evaluated differently from
global names. When a global name is evaluated, the object stored in the
corresponding variable is itself evaluated. When a local name is evaluated on HP
calculators, the object stored in the corresponding variable is returned to the
stack but is not evaluated. When a local variable contains a number, the effect
is identical to evaluation of a global name, since putting a number on the stack
is equivalent to evaluating it. However, if a local variable contains a program,
algebraic expression, or global variable name, and if you want it evaluated,
programs on HP calculators would typically execute `EVAL` after the object is
put on the stack. This is not necessary on DB48x.


### Defining the Scope of Local Variables

Local variables exist *only* inside the defining procedure.

The following program excerpt illustrates the availability of local variables in
nested defining procedures (procedures within procedures). Because local
variables `a`, `b`, and `c` already exist when the defining procedure for local
variables `d`, `e`, and `f` is executed, they’re available for use in that
procedure.

```rpl
«
  → a b c «
    a b + c +
    → d e f 'a/(d*e+f)'
    a c / -
 »
»
'Nested' STO

'X' 'Y' 'Z' 'T' 'U' Nested
@ Expecting 'Z÷(X·Y+(Z+T+U))-Z÷U'
```

In the following program excerpt, the defining procedure for local variables
`d`, `e`, and `f` calls a program that you previously created and stored in
global variable `P1`.


```rpl
«
  a b - c * → d e f '(a+d)/(d*e+f)' a c / -
»
'P1' STO

«
  → a b c « a b + c + → d e f 'P1+a/(d*e+f)' a c / - »
»
'Nested' STO

'X' 'Y' 'Z' 'T' 'U' 'V' 'W' Nested
@ Expecting '(a+X)÷(X·Y+(a-b)·c)-a÷c+U÷(Z·T+(U+V+W))-U÷W'
```

The six local variables `a`, `b`, `c`, `d`, `e` and `f` are not available in
program `P1` because they didn’t exist when you created `P1`. Consequently, `a`
in `P1` evaluates as itself. The objects stored in the local variables are
available to program `P1` only if you put those objects on the stack for `P1` to
use or store those objects in global variables.  Conversely, program `P1` can
create its own local variable structure (with any names, such as `d`, `e`, and
`f`, for example) without conflicting with the local variables of the same name
in the procedure that calls `P1`.


### User-defined functions

A program that consists solely of a local variable structure whose defining
procedure is an algebraic expression is a user-defined function. It can be used
in an algebraic expression like a built-in function.

```rpl
« → x y 'x+2*y/x'» 'MyFN' STO
'MyFN(A;B)' EVAL
@ Expecting 'A+2·B÷A'
```

### Compiled Local Variables

DB48x does not support compiled local variables.

On the HP50G, it is possible to create a special type of local variable that can
be used in other programs or subroutines. This type of local variable is called
a *compiled local variable*.

Compiled local variables have a special naming convention: they must begin with
a `←`. For example, in the code below, the variable `←y` is a compiled local
variable that can be used in the two programs `BELOW` and `ABOVE`.

```rpl
« ←y " is negative" + »
'BELOW' STO
« ←y " is positive" + »
'ABOVE' STO

« → ←y 'IFTE(←y<0;BELOW;ABOVE)' »
'Incompatible' STO
-33 Incompatible

@ Expecting "'←y' is negative"
@ HP50G: "-33 is negative"
```

The rationale for not supporting them is that they sound like a complex
non-solution to a simple non-problem, encouraging a terrible programming style.
The above example is trivially rewritten using user-defined functions as
follows:

```rpl
« → y « y " is negative" + » »
'BELOW' STO
« → y « y " is positive" + » »
'ABOVE' STO

« → y 'IFTE(y<0;BELOW(y);ABOVE(y))' »
'Compatible' STO
-33 Compatible

@ Expecting "-33 is negative"
```

## Volume of a cylinder

The following programs take the values of the radius `r` and the height `h` of a
cylinder to compute the total area of the corresponding cylinder according to
the equation `ACyl=2·π·R↑2+2·π·R·H`. We use the symbolic constant `Ⓒπ`,
which we convert to its numerical value using the `→Num` function.

The following code stores the program in the `ACyl` variable, and then supplies
the value for `R` and `H` on level 1 and 2 of the stack respectively. In the
examples, we will use `R=2_m` and `H=3_m`.

See also the [cylinder](#cylinder) entry in the equation library.

### ACyl: RPN style

The following code computes the cylinder area using *stack RPN instructions*,
i.e. manipulating values on the stack directly. This approach is the most
similar to traditional HP calculators.

```rpl
« Duplicate Rot + * 2 * Ⓒπ →Num * »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

### ACyl: Using global variables

The following implementation computes the cylinder area using *RPN instructions*
and global variables to store `R` and `H`. It then stores the result in a global
variable named `A`, using the `Copy` command that copies the result from the
stack into global variable `A` without removing it from the stack..

Using global variables is rarely the most efficient, but it has the benefit that
it leaves the inputs and output of the program avaiable for later use. This can
be beneficial if these values are precious and should be preserved.

```rpl
«
  'R' Store 'H' Store
  2 Ⓒπ →Num * R * R H + *
  'A' Copy
»
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

Note that global variables stick around in the current directory after the
program executes. They can be purged using `{ R H A } Purge`.

### ACyl: Using algebraic expressions

The following example computes the cylinder area using an *algebraic expression*
and global variables. Using algebraic expressions can make programs easier to
read, since the operations look similar to normal mathematical expressions.

```rpl
« 'R' Store 'H' Store
'2*Ⓒπ*R*(R+H)' →Num 'A' Copy »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

### ACyl: Using local variables

The following example computes the cylinder area using *local variables*, which
make it easier to reuse the same value multiple times, and do so much faster
than global variables. The code otherwise uses regular RPN instructions.

```rpl
« → H R « 2 Ⓒπ →Num * R * R H + * » »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

Notice that when we declare local variables, the order of the arguments is the
order in which they are given on the command line, not the order in which they
appear on the stack. In that case, we enter `H` first, and `R` second, meaning
that `R` is on level 1 of the stack and `H` on level 2, yet we must use the
`→ H R` notation instead of `→ R H`. This is the opposite order compared to the
`Store` commands we used for global variables.

### ACyl: Local algebraics

The following example computes the cylinder area using *local variables*, along
with an *algebraic expression*.

```rpl
« → H R '2*→Num(Ⓒπ)*R*(R+H)' »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```


## Tests and Conditional Structures

You can use commands and branching structures that let programs ask questions
and make decisions. *Comparison functions* and *logical functions* test whether
or not specified conditions exist. *Conditional structures* and
*conditional commands* use test results to make decisions.

### Testing conditions

A test is an algebraic or a command sequence that returns a test result to the
stack. A test result is either `True` or `False`. Alternatively, a non-zero
numerical value is interpreted as `True`, and a zero numerical value is
interpreted as `False`.

Note: This is a difference from HP calculators, where a test returns `0` or `1`.

#### To include a test in a program

Tests can be entered using the stack or algebraic syntax.

To use the stack syntax, enter the two arguments then enter the test command.

```rpl
2 3 >
@ Expecting False
```

To use algebraic syntax, enter the test expression between single quotes:

```rpl
'2<3'
@ Expecting True
```

You often use test results in conditional structures to determine which clause
of the structure to execute.  Conditional structures are described under
[Using Conditional Structures and Commands](#using-conditional-structures-and-commands).


### Comparison Functions

Comparision functions compare two objects, and are most easily accessed from the
`TestsMenu` (🟦 _3_):

* `=`: value equality
* `==`: representation equality
* `≠`: value inequality
* `<`: less than
* `≤`: less than or equal to
* `>`: greater than
* `≥`: greater than or equal to
* `Same`: object identity

When used in stack syntax, the order of comparison is `level2` *test* `level1`,
where *test* is the comparison function, and where `level2` and `level1`
represent the values in stack levels 2 and 1 respectively. Inside algebraic
expressions, the test is placed between the two values, e.g. `'X<5'`.

The comparison commands all return `True` or `False`, although when used for
testing purpose, e.g. in an `IFTE` command, non-zero numerical values are
interpreted as `True`, and zero numerical values are interpreted as false. For
example, the following code will interpret `42` as `True` and `0+0ⅈ` as `False`.

```rpl
IF 42 THEN
    'IFTE(0+0ⅈ;"KO";"OK")' EVAL
ELSE
    "WRONG!"
END
@ Expecting "OK"
```

All comparisons except `Same` return the following:

* If neither object is an algebraic or a name, return `True` if the two objects
  are the same type and have values that compare according to the operation, or
  `False` otherwise. Lists, arrays and text are compared in lexicographic order.
  Programs can only be compared with `==` or `Same`, order comparisons will
  error out with `Bad argument type`.
* If one object is algebraic or name, and the other object is an algebraic, name
  or number, then the command returns an expression that must be evaluated to
  get a test result based on numeric value. This evalution is automatic if the
  resulting expression is used as a test in a conditional statement such as
  `IF THEN ELSE END`.

For example, `'X' 5 <` returns `'X<5'`:

```rpl
'X' 5 <
@ Expecting 'X<5'
```

if `6` is stored in `X`, then this evaluates as `False`:

```rpl
X=6
'X' 5 < EVAL
@ Expecting False
```

This evaluation is automatic inside a conditional statement:

```rpl
X=6
IF 'X' 5 > THEN "OK" ELSE "KO" END
@ Expecting "OK"
```

### Equality comparisons

There are three functions that compare if two objects are equal, `=`, `==` and
`same`. They play slightly different roles, corresponding to different use
cases.

Inside an algebraic expression, `=` creates an equation that can be
used in the solver or in `Isol`, but also evaluates as numerical equality
(unlike HP calculators). This would be the case for `'sin X=0.5'`. Finally, the
`==` command is used for non-numerical comparisons.

Note that `=` on the command-line creates an [assignment object](#assignment),
e.g. `X=6`, which is a DB48x extension to RPL used notably in the examples for
the [equations library](#equations-library).

The `=` function tests *value* equality of two objects, irrespective of their
representation. This is typically the operation you would use if you are
interested in the mathematical aspect of equality. For example, integer and
decimal numbers with the same values are considered as equal by `=` if their
value is the same.

```rpl
'1=1.'
@ Expecting True
```

The `==` function, by contrast, tests *representation* equality of two objects,
i.e. that two objects have the same internal structure. This function will
distinguish two objects that do not have the same type, even if their numerical
value is the same.

```rpl
'1==1.'
@ Expecting False
```

The `same` function, finally, tests *identity* of two objects. It differs from
`==` in that it does not even attempt to evaluate names.

```rpl
2 'A' STO
2 'B' STO
'A' 'B' == EVAL
'A==B' EVAL
'A' 'B' SAME EVAL
3 →List
@ Expecting { True True False }
```

#### Equality: Differences with HP

The DB48x `=` operator differs from HP calculators, that use `=` only to build
equations. As a result, `2=3` returns `False` on DB48x, but evaluates as `2=3`
on HP50G, and as `-1` (i.e. `2-3`) if using `→Num`. For numerical values, `=`
on DB48x behaves roughly like `==` on HP50G, e.g. `1=1.` returns `True` just
like `1==1.` returns `1` on HP50G.

The DB48x `==` comparison is roughly equivalent to `SAME`, except that it
evaluates names. In other words, `A==B` returns `True` if variables `A` and `B`
contain identical objects, whereas `SAME(A,B)` returns `False` because the names
are different.


### Using Logical Functions

Logical functions return a test result based on the outcomes of one or two
previously executed tests. Note that these functions interpret any
*nonzero numerical argument* as a true result, with the important exception that
if the two arguments are integers, then the operation is performed bitwise.
For non-based integers, this [deviates from HP calculators](#evaluation), and
can be changed with the `TruthLogicForIntegers` flag.

* `and` returns `True` only if both arguments are true
* `or` returns `True` if either or both arguments are true
* `xor` returns `True` if either argument, but not both, is true
* `not` returns `True` if its argument is false
* `nand` returns the `not` of `and`
* `nor` returns the `not` of `and`
* `implies` returns `True` if `not` the first argument `or` the other
  (i.e. if the first result logically implies the other)
* `equiv` returns the `not` of `xor`, i.e. it returns `True` iff the two
  arguments are logically equivalent.
* `excludes` returns `True` if the first one is true and `not` the second one,
  i.e. if the first result excludes the second one.

`AND`, `OR`, and `XOR` combine two test results. For example, if `4` is stored
in `Y`, `Y 8 < 5 AND` returns `True`. First, `Y 8 <` returns `True`, then `AND`
removes `True` and `5` from the stack, interpreting both as true results, and
returns `True` to the stack.

```rpl
Y=4
Y 8 < 5 AND
@ Expecting True
```

`NOT` returns the logical inverse of a test result. For example, if `1` is
stored in `X` and `2` is stored in `Y`, `X Y < NOT` returns `False`:

```rpl
X=1 Y=2
X Y < NOT
@ Expecting False
```

You can use `AND`, `OR`, and `XOR` in algebraics as infix functions,
for example the following code returns `True`:

```rpl
'3<5 XOR 4>7'
@ Expecting True
```

You can use `NOT` as a prefix function in algebraics.
For example, the following code returns `False`:

```rpl
Z=2
'NOT(Z≤4)'
@ Expecting False
```

### Logical Functions on Integers

When given integer or based numbers as input, logical operations applies
bitwise, where each bit set is interpreted as `True` and each bit clear is
interpreted as `False`.

For example, the `Xor` operation will apply bit by bit in the following:

```rpl
16#32 16#F24 XOR
@ Expecting #F16₁₆
```

This can be made more explicity visible using binary operations:

```rpl
                 2#11 0010
           2#111 0010 0100 XOR
@ Expecting #111 0001 0110₂
```

Unless the `TruthLogicForIntegers` flag is set, this is also true for integer
values:

```rpl
42 7 XOR
@ Expecting 45
```

## Testing Object Types

The `Type` command takes any object as an argument and returns a number that
identifies that object type. You can find it in the `ObjectsMenu` (🟦 _-_),
using the _Type_ (_F2_) softkey.

### DB48x detailed types

In DB48x detailed type mode (`DetailedTypes`), the returned value is a negative
value that uniquely identifies each DB48x type. For example:

```rpl
DetailedTypes
"Hello" TYPE
@ Expecting -3
```

### HP-compatible types

In HP compatibility mode (`CompatibleTypes`), the returned value is a positive
integer that matches the value returned by HP's implementation of RPL, and may
group distinct DB48x types under the same number. For example:

```rpl
CompatibleTypes
"Hello" TYPE
@ Expecting 2
```

See the description of the `Type` command for a list of the returned values.

### Type names

The `TypeName` command is a DB48x-only command that returns a text describing
the type. For example:

```rpl
"Hello" TYPENAME
@ Expecting "text"
```

Using `TypeName` can make code easier to read.

You can find `TypeName` in the `ObjectsMenu` (🟦 _-_), using the
_TypeName_ (_F3_) softkey.


## Conditional Structures & Commands

*Conditional structures* let a program make a decision based on the result of
tests.

*Conditional commands* let you execute a true-clause or a false-clause, each of
which are a *single* command or object.

These conditional structures and commands are contained in the `TestsMenu`
(🟦 _3_):

* `IF…THEN…END` structure
* `IF…THEN…ELSE…END` structure
* `CASE…END` structure
* `IFT` command (if-then)
* `IFTE` function (if-then-else)

### The IF…THEN…END Structure

The syntax for this structure is:
`IF` *test-clause* `THEN` *true-clause* `END`

```rpl
Condition='1<2'
IF Condition THEN Success END
@ Expecting 'Success'
```

`IF…THEN…END` executes the sequence of commands in the *true-clause* only if the
*test-clause* evaluates to a true value, i.e. `True` or a non-zero numerical
value. The test-clause can be a command sequence, for example, `A B <`, or an
algebraic expression, for example, `'A<B'`. If the *test-clause* is an
algebraic expression, it’s *automatically evaluated* — you don’t need `→NUM` or
`EVAL`.

`IF` begins the *test-clause*, which leaves a test result on the stack.
`THEN` conceptually removes the test result from the stack. If the value is
`True` or a non-zero numerical value, the *true-clause* is executed. Otherwise,
program execution resumes following `END`.

To enter `IF…THEN…END` in a program, select the  `TestsMenu` (🟦 _3_) and then
the _IfThen_ command (🟨 _F1_).

### The IFT Command

The `IFT` command takes two arguments: a *test-result* in level 2 and a
*true-clause* object in level 1. If the *test-result* is true, the *true-clause*
object is executed:

```rpl
Condition='2<3'
Condition Success IFT
@ Expecting 'Success'
```

Otherwise, the two arguments are removed from the stack:

```rpl
Condition='2>3'
Success
Condition Failure IFT
@ Expecting 'Success'
```

To enter `IFT` in a program, select the `TestsMenu` (🟦 _3_) and then the _IFT_
command (🟨 _F5_).


### The IF…THEN…ELSE…END Structure

The syntax for this structure is:
`IF` *test-clause* `THEN` *true-clause* `ELSE` *false-clause* `END`

```rpl
Condition='1<2'
IF Condition THEN Success END
@ Expecting 'Success'
```

`IF…THEN…END` executes either the *true-clause* sequence of commands if the
*test-clause* is `True` or a non-zero numerical value, or the *false-clause*
if the *test-clause* is `False` or a zero numerical value. If the *test-clause*
is an algebraic expression, it is automatically evaluated.

the sequence of commands in the *true-clause* only if the
*test-clause* evaluates to a true value, i.e. `True` or a non-zero numerical
value. The test-clause can be a command sequence, for example, `A B <`, or an
algebraic expression, for example, `'A<B'`. If the *test-clause* is an
algebraic expression, it’s *automatically evaluated* — you don’t need `→NUM` or
`EVAL`.

`IF` begins the *test-clause*, which leaves a test result on the stack.
`THEN` conceptually removes the test result from the stack. If the value is
`True` or a non-zero numerical value, the *true-clause* is executed. Otherwise,
program execution resumes following `END`.

To enter `IF…THEN…ELSE…END` in a program, select the `TestsMenu` (🟦 _3_) and
then the _IfElse_ command (🟨 _F2_).

### The IFTE Function

The `IFTE` command takes three arguments: a *test-result* in level 3, a
*true-clause* object in level 2 and a *false-clause* in level 1. It can also be
entered in an algebraic expression as
`'IFTE(test-result;true-clause;false-clause)'.

If the *test-result* is `True` or a non-zero number, the *true-clause* object is
executed and left on the stack:

```rpl
Condition='2<3'
Condition Success Failure IFTE
@ Expecting 'Success'
```

Otherwise, the arguments are removed from the stack, and the *false-clause*
object is executed and left on the stack:

```rpl
Condition='2>3'
Condition Failure Success IFTE
@ Expecting 'Success'
```

In an algebraic expression, `IFTE` is used as a function that returns its first
second if the condition in the first argument is `True` or a non-zero number:

```rpl
'IFTE(0<1;Success;Failure)'
@ Expection 'Success'
```

If the condition is `False` or a zero number, the `IFTE` function returns its
third argument:

```rpl
'IFTE(0.0;Failure;Success)'
@ Expection 'Success'
```

To enter `IFTE` in a program, select the `TestsMenu` (🟦 _3_) and then
the _IFTE_ command (🟨 _F6_).
# Release notes

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
# Implementation status

This section documents the implementation status for all HP50 RPL commands as
listed in the HP50G Advanced Reference Manual. This is a strict superset of the
HP48 implementation.

* [Implemented](#implemented-commands)
* [Not implemented](#unimplemented-commands)
* [Unapplicable commands](#unapplicable-commands)


## Implemented commands


The following is an extensive list of commands.

* `!`
* `%`
* `%T`
* `%Total`
* `*`
* `+`
* `-`
* `/`
* `<`
* `=`
* `==`
* `>`
* `^`
* `abs`
* `acos`
* `acosh`
* `AlarmMenu`
* `AlgebraMenu`
* `AllEquationVariables`
* `AllSolutions`
* `alog`
* `AlphaForeground`
* `Amort`
* `AmortTable`
* `and`
* `AngleMode`
* `AnglesMenu`
* `Apply`
* `ApplyInverseUnit`
* `ApplyUnit`
* `arg`
* `ArithmeticMenu`
* `Arry→`
* `asin`
* `asinh`
* `Asn`
* `ASR`
* `ASRB`
* `ASRC`
* `atan`
* `atanh`
* `Attach`
* `AutomaticTemporariesCleanup`
* `AutoScaleStack`
* `AutoSimplify`
* `Avg`
* `Background`
* `Bar`
* `Base`
* `BasedDotOrComma`
* `BasedSeparatorCommand`
* `BasedSpaces`
* `BasedSpacing`
* `BasedTicks`
* `BasedUnderscore`
* `BasesMenu`
* `BatteryBackground`
* `BatteryForeground`
* `BatteryLevelForeground`
* `BatteryRefresh`
* `BatteryVoltage`
* `Beep`
* `BeepOff`
* `BeepOn`
* `BestFit`
* `BigFractions`
* `Bin`
* `Bins`
* `BitwiseLogicForIntegers`
* `Blank`
* `BlankBitmap`
* `BlankGrob`
* `BusyIndicatorRefresh`
* `Bytes`
* `B→R`
* `CalculationMenu`
* `Capitalized`
* `CapitalizedNames`
* `CarefulEvaluation`
* `Catalog`
* `ceil`
* `Centr`
* `CF`
* `CharactersMenu`
* `ChargingForeground`
* `Chr`
* `ChronoTime`
* `Circle`
* `CircularMenu`
* `ClassicExponent`
* `Clear`
* `ClearBit`
* `ClearThingsMenu`
* `Clip`
* `ClLCD`
* `Clone`
* `ClΣ`
* `CNrm`
* `Col+`
* `Col-`
* `Collect`
* `ColΣ`
* `Col→`
* `comb`
* `CommandBackground`
* `CommandDisplayMode`
* `CommandForeground`
* `CompareMenu`
* `CompatibleBasedNumbers`
* `CompatibleGROBs`
* `CompatibleTypes`
* `ComplexIAfterImaginary`
* `ComplexIBeforeImaginary`
* `ComplexMenu`
* `ComplexResults`
* `Con`
* `conj`
* `ConLib`
* `Const`
* `ConstantName`
* `Constants`
* `ConstantValue`
* `cont`
* `Convert`
* `ConvertToUnit`
* `ConvertToUnitPrefix`
* `Corr`
* `cos`
* `cosh`
* `CountBits`
* `Cov`
* `CrDir`
* `Cst`
* `CstRU`
* `CstSU`
* `CSwp`
* `CurrentClip`
* `CurrentDirectory`
* `CurrentEquationVariables`
* `CurrentFitSums`
* `CursorAlphaBackground`
* `CursorAlphaBorder`
* `CursorAlphaForeground`
* `CursorBackground`
* `CursorBlinkRate`
* `CursorBorder`
* `CursorFont`
* `CursorForeground`
* `CursorSelBackground`
* `CurveFilling`
* `CustomHeaderRefresh`
* `Cycle`
* `C→R`
* `Date`
* `Date+`
* `DateDash`
* `DateDot`
* `DateForeground`
* `DateMenu`
* `DateSeparatorCommand`
* `DateSlash`
* `DateSpace`
* `DateTime`
* `DayBeforeMonth`
* `dbug`
* `DDays`
* `DebugMenu`
* `DebugOnError`
* `Dec`
* `DecimalComma`
* `DecimalDot`
* `Decr`
* `Deg`
* `DelKeys`
* `Depth`
* `det`
* `Detach`
* `DetailedTypes`
* `DifferentialSolverMenu`
* `Disp`
* `DisplayDigits`
* `DisplayMode`
* `DisplayModesMenu`
* `DispXY`
* `DistinguishSymbolCase`
* `Div2`
* `DMCPDisplayRefresh`
* `DMCPLowBattery`
* `DMS+`
* `DMS-`
* `DMS→`
* `doerr`
* `DoList`
* `DoSubs`
* `dot`
* `Draw`
* `DrawPlotAxes`
* `Drax`
* `Drop`
* `Drop2`
* `DropN`
* `DTag`
* `Dup`
* `Dup2`
* `DupDup`
* `DupN`
* `D→R`
* `Edit`
* `EditMenu`
* `EditorBackground`
* `EditorBegin`
* `EditorClear`
* `EditorCopy`
* `EditorCut`
* `EditorEnd`
* `EditorFlip`
* `EditorFont`
* `EditorForeground`
* `EditorHistory`
* `EditorHistoryBack`
* `EditorLineForeground`
* `EditorPaste`
* `EditorReplace`
* `EditorSearch`
* `EditorSelect`
* `EditorWordLeft`
* `EditorWordRight`
* `EditorWrapColumn`
* `Ellipse`
* `EndSub`
* `Eng`
* `Eq`
* `EqnLib`
* `EquationName`
* `EquationSolver`
* `EquationValue`
* `equiv`
* `erf`
* `erfc`
* `err0`
* `errm`
* `errn`
* `ErrorBackground`
* `ErrorBeepDuration`
* `ErrorBeepFrequency`
* `ErrorBorder`
* `ErrorForeground`
* `Eval`
* `EvalEq`
* `excludes`
* `ExitClearsMenu`
* `ExitKeepsMenu`
* `exp`
* `exp2`
* `Expan`
* `ExpFit`
* `ExplicitWildcards`
* `ExpLogIdentitiesMenu`
* `ExpLogMenu`
* `expm`
* `False`
* `FancyExponent`
* `FC?`
* `FC?C`
* `FC?S`
* `FF`
* `FilesMenu`
* `Filter`
* `FinalAlgebraResults`
* `FinanceRounding`
* `FirstBitSet`
* `Fix`
* `FixedWidthDigits`
* `FlagsMenu`
* `FlatMenus`
* `FlipBit`
* `floor`
* `FoldConstants`
* `FolderCornerForeground`
* `Foreground`
* `FourDigitYear`
* `FP`
* `FractionsMenu`
* `FractionSpacing`
* `FreeMemory`
* `Freeze`
* `FS?`
* `FS?C`
* `FS?S`
* `Function`
* `Gamma`
* `GAnd`
* `GC`
* `GCIconForeground`
* `GCStats`
* `GCStatsClearAfterRead`
* `GCStatsKeepAfterRead`
* `GCTemporariesCleanup`
* `Get`
* `GetI`
* `GOr`
* `Grad`
* `GraphicAppend`
* `GraphicExponent`
* `GraphicIntegral`
* `GraphicNorm`
* `GraphicParentheses`
* `GraphicProduct`
* `GraphicRatio`
* `GraphicResultDisplay`
* `GraphicRoot`
* `GraphicsMenu`
* `GraphicStackDisplay`
* `GraphicSubscript`
* `GraphicSum`
* `GraphingTimeLimit`
* `GraphRenderingSizeLimit`
* `Gray`
* `GrobAdd`
* `GXor`
* `HalfBatteryForeground`
* `Halt`
* `HaltedIconForeground`
* `Hdr`
* `Head`
* `HeaderBackground`
* `HeaderFont`
* `HeaderForeground`
* `Help`
* `Hex`
* `HFP`
* `HideBuiltinCharacters`
* `HideBuiltinConstants`
* `HideBuiltinEquations`
* `HideBuiltinLibrary`
* `HideBuiltinUnits`
* `HideDate`
* `HideDayOfWeek`
* `HideEmptyMenu`
* `HideEquationBody`
* `HideMonthName`
* `HideSeconds`
* `HideTime`
* `HideVoltage`
* `HMS+`
* `HMS-`
* `HMS→`
* `Home`
* `HorizontalLists`
* `HorizontalProgramRendering`
* `HorizontalVectors`
* `HyperbolicMenu`
* `Idn`
* `IFT`
* `IFTE`
* `IgnorePrecisionLoss`
* `IgnoreSymbolCase`
* `im`
* `ImplicitWildcards`
* `Implies`
* `ImproperFractions`
* `Incr`
* `IndexDidNotWrap`
* `IndexWrapped`
* `InfiniteResultIndicator`
* `InfinityError`
* `InfinityValue`
* `Input`
* `IntegrationImprecision`
* `IntegrationIterations`
* `IntegrationMenu`
* `Intercept`
* `inv`
* `IOMenu`
* `IP`
* `Isol`
* `JDN`
* `JDN→`
* `Keys`
* `Kill`
* `KillOnError`
* `LastArg`
* `LastBitSet`
* `LastMenu`
* `LastX`
* `LaxArrayResizing`
* `LazyEvaluation`
* `LCD→`
* `LeadingZero`
* `LeftShiftBackground`
* `LeftShiftForeground`
* `lgamma`
* `LibEq`
* `Library`
* `Libs`
* `Line`
* `LinearFitSums`
* `LinearSolverMenu`
* `LineWidth`
* `LinFit`
* `ListAsData`
* `ListAsProgram`
* `ListMenu`
* `List→`
* `ln`
* `ln1p`
* `LName`
* `log`
* `log2`
* `LogFit`
* `LongForm`
* `LongFormNames`
* `LoopsMenu`
* `LowBattery`
* `LowBatteryForeground`
* `LowerAlphaForeground`
* `LowerCase`
* `LowerCaseNames`
* `LR`
* `MainMenu`
* `mant`
* `MantissaSpacing`
* `Map`
* `MatchUp`
* `MathMenu`
* `MathModesMenu`
* `MatrixMenu`
* `Max`
* `MaxFlags`
* `MaxH`
* `MaximumDecimalExponent`
* `MaxNumberBits`
* `MaxRewrites`
* `MaxW`
* `MaxΣ`
* `Median`
* `Mem`
* `MemoryMenu`
* `Menu`
* `MenuAppearance`
* `MenuBackground`
* `Min`
* `MinimumBatteryVoltage`
* `MinimumSignificantDigits`
* `MinΣ`
* `MixedFractions`
* `mod`
* `ModernBasedNumbers`
* `Modes`
* `ModesMenu`
* `MonthBeforeDay`
* `MRoot`
* `MSlv`
* `MSolvr`
* `MultilineCursorFont`
* `MultilineEditorFont`
* `MultiLineResult`
* `MultiLineStack`
* `MultipleSteps`
* `MultiSolverMenu`
* `NameDisplayMode`
* `NAnd`
* `NDupN`
* `NeedToClearErrors`
* `neg`
* `NegativeUnderflowIndicator`
* `NextEq`
* `Nip`
* `NoAngleUnits`
* `NoAutoSimplify`
* `NoCurveFilling`
* `NoErrorForTooManyDigits`
* `NoInfiniteResult`
* `NoLastArguments`
* `NoLeadingZero`
* `NoNeedToClearErrors`
* `NoNegativeUnderflow`
* `NoNumberedVariables`
* `NoOverflow`
* `NoPlotAxes`
* `NoPositiveUnderflow`
* `NoProgramLastArguments`
* `NOr`
* `NormalPolynomialRender`
* `NoStackAutoScale`
* `NoStackSave`
* `not`
* `NoTrailingDecimal`
* `NoUndefinedResult`
* `NSub`
* `Num`
* `NumberDotOrComma`
* `NumberedVariables`
* `NumberSeparatorCommand`
* `NumbersMenu`
* `NumberSpaces`
* `NumberTicks`
* `NumberUnderscore`
* `NumericalConstants`
* `NumericalIntegration`
* `NumericalResults`
* `NumericalSolver`
* `NumericalSolverMenu`
* `NΣ`
* `ObjectMenu`
* `Obj→`
* `Oct`
* `Off`
* `OffWithImage`
* `OnesComplement`
* `or`
* `Over`
* `OverflowError`
* `OverflowIndicator`
* `OverflowValue`
* `PackedBitmaps`
* `Parametric`
* `PartsMenu`
* `Path`
* `PCov`
* `perm`
* `PgDir`
* `Pick`
* `Pick3`
* `Pict`
* `PiRadians`
* `Pix?`
* `PixC?`
* `PixOff`
* `PixOn`
* `PlotErrorBackground`
* `PlotMenu`
* `PlotRefreshRate`
* `PMax`
* `PMin`
* `Polar`
* `PolynomialsMenu`
* `PolynomialSolverMenu`
* `Poly→`
* `PositiveUnderflowIndicator`
* `PowerVoltage`
* `PPar`
* `PrcRnd`
* `Prec`
* `PredX`
* `PredY`
* `PrefixPolynomialRender`
* `PrincipalSolution`
* `PrintingMenu`
* `ProbabilitiesMenu`
* `ProgramLastArguments`
* `ProgramMenu`
* `Prompt`
* `PSDev`
* `Purge`
* `PurgeAll`
* `PushEvaluatedAssignment`
* `PushOriginalAssignment`
* `Put`
* `PutI`
* `PVar`
* `PwrFit`
* `P→R`
* `QuickSort`
* `Rad`
* `rand`
* `Random`
* `RandomGeneratorBits`
* `RandomGeneratorOrder`
* `RangeMenu`
* `RanM`
* `RcEq`
* `Rcl`
* `Rcl*`
* `Rcl+`
* `Rcl-`
* `Rcl/`
* `RclF`
* `RclKeys`
* `RclMenu`
* `RclΣ`
* `rcws`
* `rdz`
* `re`
* `RealMenu`
* `RealResults`
* `Rect`
* `Reduce`
* `RelRnd`
* `rem`
* `ReorderTerms`
* `ReportPrecisionLoss`
* `ResetModes`
* `ResultBackground`
* `ResultFont`
* `ResultForeground`
* `ResultGraphingTimeLimit`
* `ReverseQuickSort`
* `ReverseSort`
* `RevList`
* `RGB`
* `RightShiftBackground`
* `RightShiftForeground`
* `Risch`
* `RL`
* `RLB`
* `RLC`
* `Rnd`
* `Roll`
* `RollD`
* `Root`
* `Rot`
* `RoundedMenus`
* `RoundMenuBackground`
* `RoundMenuForeground`
* `Row+`
* `Row-`
* `RowNorm`
* `Row→`
* `RR`
* `RRB`
* `RRC`
* `RRect`
* `RSwp`
* `Run`
* `RunningIconForeground`
* `RunStats`
* `RunStatsClearAfterRead`
* `RunStatsKeepAfterRead`
* `R→B`
* `R→C`
* `R→D`
* `R→I`
* `R→P`
* `same`
* `SaveLastArguments`
* `SaveStack`
* `SaveState`
* `Scale`
* `ScaleH`
* `ScaleW`
* `Scatter`
* `Sci`
* `ScreenCapture`
* `SDev`
* `SearchBackground`
* `SearchForeground`
* `SelectedMenuForeground`
* `SelectionBackground`
* `SelectionForeground`
* `SeparatorModesMenu`
* `SetAngleUnits`
* `SetBit`
* `SetDate`
* `SetTime`
* `SF`
* `SFP`
* `Show`
* `ShowAsDecimal`
* `ShowBuiltinCharacters`
* `ShowBuiltinConstants`
* `ShowBuiltinEquations`
* `ShowBuiltinLibrary`
* `ShowBuiltinUnits`
* `ShowDate`
* `ShowDayOfWeek`
* `ShowEmptyMenu`
* `ShowEquationBody`
* `ShowIntegersAndFractions`
* `ShowMonthName`
* `ShowSeconds`
* `ShowTime`
* `ShowTimeLimit`
* `ShowVoltage`
* `Sig`
* `SigDig`
* `sign`
* `SignalProcessingMenu`
* `SilentBeepOff`
* `SilentBeepOn`
* `Simplify`
* `sin`
* `SingleLineResult`
* `SingleLineStack`
* `SingleRowMenus`
* `sinh`
* `Size`
* `SKey`
* `SkippedMenuBackground`
* `SL`
* `SLB`
* `SLC`
* `Slope`
* `SmallFractions`
* `SoftwareDisplayRefresh`
* `SolverImprecision`
* `SolverIterations`
* `SolvingMenu`
* `SolvingMenuRecall`
* `SolvingMenuSolve`
* `SolvingMenuStore`
* `Sort`
* `sq`
* `SquareMenuBackground`
* `SquareMenuForeground`
* `SquareMenus`
* `SR`
* `SRB`
* `SRC`
* `sst`
* `StackBackground`
* `StackEditor`
* `StackFont`
* `StackForeground`
* `StackGraphingTimeLimit`
* `StackLevelBackground`
* `StackLevelForeground`
* `StackLineForeground`
* `StackMenu`
* `StandardExponent`
* `StateNameForeground`
* `StatisticsMenu`
* `Std`
* `StdRnd`
* `StepByStepAlgebraResults`
* `StepOut`
* `StepOver`
* `SteppingIconForeground`
* `StEq`
* `Sto`
* `Sto*`
* `Sto+`
* `Sto-`
* `Sto/`
* `StoF`
* `StoKeys`
* `StoreAtEnd`
* `StoreAtStart`
* `StoVX`
* `StoΣ`
* `StrictArrayResizing`
* `Str→`
* `stws`
* `Sub`
* `Subst`
* `Swap`
* `SymbolicConstants`
* `SymbolicIntegration`
* `SymbolicMenu`
* `SymbolicResults`
* `SymbolicSolver`
* `SymbolicSolverMenu`
* `SystemMemory`
* `SystemSetup`
* `TabWidth`
* `Tag→`
* `Tail`
* `tan`
* `tanh`
* `TestsMenu`
* `TEval`
* `TextMenu`
* `TextRenderingSizeLimit`
* `TextResultDisplay`
* `TextStackDisplay`
* `Text→Algebraic`
* `Text→Code`
* `Text→Expression`
* `Text→Integer`
* `Text→Number`
* `Text→Object`
* `Text→Positive`
* `Text→Real`
* `ThreeRowsMenus`
* `Ticks`
* `Time`
* `Time12H`
* `Time24H`
* `TimeForeground`
* `TimeMenu`
* `TMenu`
* `ToArray`
* `ToBitmap`
* `ToCylindrical`
* `ToDMS`
* `ToggleCustomMenu`
* `ToggleUserMode`
* `ToGrads`
* `ToGrob`
* `ToHMS`
* `ToHPGrob`
* `ToLCD`
* `ToList`
* `ToolsMenu`
* `TooManyDigitsErrors`
* `ToPolar`
* `ToSpherical`
* `ToStr`
* `Tot`
* `ToTag`
* `ToUnit`
* `TrailingDecimal`
* `Tran`
* `TrigIdentitiesMenu`
* `Trn`
* `Trnc`
* `True`
* `TruthLogicForIntegers`
* `TVars`
* `TVM`
* `TVMBeg`
* `TVMEnd`
* `TVMEquation`
* `TVMRoot`
* `TwoDigitYear`
* `TwosComplement`
* `Type`
* `TypeName`
* `UBase`
* `UFact`
* `UIAlgebraic`
* `UIAlpha`
* `UIBackspace`
* `UIDecimal`
* `UIDelete`
* `UIDown`
* `UIEnter`
* `UIExit`
* `UILeft`
* `UILeftShift`
* `UILowercase`
* `UINoAlpha`
* `UINoShift`
* `UIRight`
* `UIRightShift`
* `UIText`
* `UIUp`
* `UndefinedError`
* `UndefinedResultIndicator`
* `UndefinedValue`
* `UnderflowError`
* `UnderflowValue`
* `Undo`
* `Unimplemented`
* `UnimplementedForeground`
* `UnitsConversionsMenu`
* `UnitsMenu`
* `UnitsSIPrefixCycle`
* `UnPick`
* `UnRot`
* `UpDir`
* `UpperCase`
* `UpperCaseNames`
* `USBPowered`
* `UseCrossForMultiplication`
* `UseDotForMultiplication`
* `UserCommandBackground`
* `UserCommandBorder`
* `UserCommandForeground`
* `UserInterfaceModesMenu`
* `UserMode`
* `UserModeLock`
* `UserModeMenu`
* `UserModeOff`
* `UserModeOnce`
* `UVal`
* `Var`
* `VariablesMenu`
* `VariablesMenuExecute`
* `VariablesMenuRecall`
* `VariablesMenuStore`
* `VariableWidthDigits`
* `Vars`
* `VectorMenu`
* `Version`
* `VerticalLists`
* `VerticalProgramRendering`
* `VerticalVectors`
* `VoltageForeground`
* `VX`
* `V→`
* `Wait`
* `XCol`
* `XLib`
* `XlibName`
* `XlibValue`
* `Xor`
* `xpon`
* `XRng`
* `xroot`
* `XVars`
* `x³`
* `YCol`
* `YearFirst`
* `YearLast`
* `YRng`
* `ZeroOverZeroIsError`
* `ZeroOverZeroIsUndefined`
* `ZeroPowerZeroIsOne`
* `ZeroPowerZeroIsUndefined`
* `|`
* `Σ`
* `Σ+`
* `Σ-`
* `ΣDat`
* `ΣLine`
* `ΣList`
* `ΣPar`
* `ΣX`
* `ΣX2`
* `ΣXY`
* `ΣY`
* `ΣY2`
* `→2D`
* `→3D`
* `→Cartesian`
* `→Col`
* `→Deg`
* `→Num`
* `→Poly`
* `→Prg`
* `→Q`
* `→QDigits`
* `→QIterations`
* `→Rad`
* `→Row`
* `→Ur`
* `→Us`
* `→πr`
* `↓Match`
* `∂`
* `∆List`
* `∏`
* `∏List`
* `√`
* `∛`
* `∠`
* `∫`
* `≠`
* `≤`
* `≥`
* `⊿`
* `Ⓓ`
* `▶`
* `⨯`

## Unimplemented commands

The following is a list of unimplemented HP50G RPL commands which should be
implemented by the time the project reaches version 1.0.

* `ABCUV`
* `ACK`
* `ACKALL`
* `ACOS2S`
* `ADDTMOD`
* `ADDTOREAL`
* `ALGB`
* `ANIMATE`
* `ANS`
* `ARC`
* `ARCHIVE`
* `ARIT`
* `ASIN2C`
* `ASIN2T`
* `ASSUME`
* `ATAN2S`
* `ATICK`
* `AUGMENT`
* `AUTO`
* `AXL`
* `AXM`
* `AXQ`
* `BASIS`
* `BINS`
* `BOX`
* `C2P`
* `CASCFG`
* `CASCMD`
* `CHINREM`
* `CHOLESKY`
* `CHOOSE`
* `CIRC`
* `CLUSR`
* `CLVAR`
* `CMPLX`
* `COLCT`
* `COND`
* `CONIC`
* `CORR`
* `CURL`
* `CYCLOTOMIC`
* `CYLIN`
* `C→PX`
* `DARCY`
* `DEF`
* `DEFINE`
* `DEGREE`
* `DELALARM`
* `DEPND`
* `DERIV`
* `DERVX`
* `DESOLVE`
* `DIAG→`
* `→DIAG`
* `DIAGMAP`
* `DIFF`
* `DIFFEQ`
* `DISTRIB`
* `DIV`
* `DIV2MOD`
* `DIVIS`
* `DIVMOD`
* `DIVPC`
* `dn`
* `DOMAIN`
* `DRAW3DMATRIX`
* `DROITE`
* `EDITB`
* `EGCD`
* `EGV`
* `EGVL`
* `ENDSUB`
* `EPSX0`
* `EQW`
* `EQ→`
* `ERASE`
* `EULER`
* `EXLR`
* `EXP&LN`
* `EXP2HYP`
* `EXP2POW`
* `EXPANDMOD`
* `EXPLN`
* `EYEPT`
* `F0λ`
* `FACTOR`
* `FACTORMOD`
* `FACTORS`
* `FANNING`
* `FAST3D`
* `FCOEF`
* `FDISTRIB`
* `FFT`
* `FINDALARM`
* `FONT6`
* `FONT7`
* `FONT8`
* `FONT→`
* `→FONT`
* `FOURIER`
* `FROOTS`
* `FXND`
* `GAUSS`
* `GBASIS`
* `GCD`
* `GCDMOD`
* `GRAD`
* `GRAMSCHMIDT`
* `GRAPH`
* `GREDUCE`
* `GRIDMAP`
* `*H`
* `HADAMARD`
* `HALFTAN`
* `HEADER→`
* `→HEADER`
* `HERMITE`
* `HESS`
* `HILBERT`
* `HISTOGRAM`
* `HISTPLOT`
* `HORNER`
* `IABCUV`
* `IBASIS`
* `IBERNOULLI`
* `IBP`
* `ICHINREM`
* `IEGCD`
* `IFFT`
* `ILAP`
* `IMAGE`
* `INDEP`
* `INFORM`
* `INT`
* `INTEGER`
* `INTVX`
* `INVMOD`
* `IQUOT`
* `IREMAINDER`
* `ISOM`
* `ISPRIME?`
* `I→R`
* `JORDAN`
* `KER`
* `KEY`
* `KEYEVAL`
* `LABEL`
* `LAGRANGE`
* `LAP`
* `LAPL`
* `LAST`
* `LCM`
* `LCXM`
* `LDEC`
* `LEGENDRE`
* `LGCD`
* `lim`
* `LIMIT`
* `LIN`
* `LININ`
* `LINSOLVE`
* `LNCOLLECT`
* `LOCAL`
* `LQ`
* `LSQ`
* `LU`
* `LVAR`
* `MAD`
* `MAIN`
* `MATHS`
* `MATR`
* `MAXR`
* `MCALC`
* `MENUXY`
* `MINIFONT→`
* `→MINIFONT`
* `MINIT`
* `MINR`
* `MITM`
* `MKISOM`
* `MODSTO`
* `MODULAR`
* `MOLWT`
* `MSGBOX`
* `MSLV`
* `MSOLVR`
* `MULTMOD`
* `MUSER`
* `NDIST`
* `NEXTPRIME`
* `NOVAL`
* `NUMX`
* `NUMY`
* `ORDER`
* `P2C`
* `PA2B2`
* `PARSURFACE`
* `PARTFRAC`
* `PCAR`
* `PCOEF`
* `PCONTOUR`
* `PDIM`
* `PERINFO`
* `PERTBL`
* `PEVAL`
* `PICTURE`
* `PLOT`
* `PLOTADD`
* `PMINI`
* `POLYNOMIAL`
* `POP`
* `POS`
* `POTENTIAL`
* `POWEXPAND`
* `POWMOD`
* `PREDV`
* `PREVAL`
* `PREVPRIME`
* `PROMPTSTO`
* `PROOT`
* `PROPFRAC`
* `PSDEV`
* `PSI`
* `Psi`
* `PTAYL`
* `PTPROP`
* `PUSH`
* `PVIEW`
* `PX→C`
* `→Qπ`
* `qr`
* `QR`
* `QUAD`
* `QUOT`
* `QUOTE`
* `QXA`
* `RANK`
* `RATIO`
* `RCI`
* `RCIJ`
* `RCLALARM`
* `RCLF`
* `RCLVX`
* `RDM`
* `REF`
* `REMAINDER`
* `RENAME`
* `REORDER`
* `REPL`
* `RES`
* `RESULTANT`
* `RKF`
* `RKFERR`
* `RKFSTEP`
* `RNRM`
* `ROW–`
* `ROW+`
* `ROW→`
* `→ROW`
* `rref`
* `RREF`
* `RREFMOD`
* `RRK`
* `RRKSTEP`
* `RSBERR`
* `RSD`
* `RSWP`
* `RULES`
* `R→I`
* `SCATRPLOT`
* `SCATTER`
* `SCHUR`
* `SCLΣ`
* `SCONJ`
* `SCROLL`
* `SDEV`
* `SEQ`
* `SERIES`
* `SEVAL`
* `SHOW`
* `SIDENS`
* `SIGMA`
* `SIGMAVX`
* `SIGNTAB`
* `SIMP2`
* `SIMPLIFY`
* `SINCOS`
* `SINV`
* `SIZE`
* `SLOPEFIELD`
* `SNEG`
* `SNRM`
* `SOLVE`
* `SOLVEQN`
* `SOLVER`
* `SOLVEVX`
* `SPHERE`
* `SRAD`
* `SREPL`
* `STOALARM`
* `STOF`
* `STOVX`
* `STREAM`
* `STURM`
* `STURMAB`
* `SUB`
* `SUBTMOD`
* `SVD`
* `SVL`
* `SYLVESTER`
* `SYST2MAT`
* `TABVAL`
* `TABVAR`
* `TAN2CS2`
* `TAN2SC`
* `TAN2SC2`
* `TAYLOR0`
* `TAYLR`
* `TCHEBYCHEFF`
* `TCOLLECT`
* `TDELTA`
* `TESTS`
* `TEXPAND`
* `TEXT`
* `TINC`
* `TLIN`
* `TLINE`
* `TRACE`
* `TRAN`
* `TRIG`
* `TRIGCOS`
* `TRIGO`
* `TRIGSIN`
* `TRIGTAN`
* `TRN`
* `TRUNC`
* `TRUTH`
* `TSIMP`
* `TSTR`
* `UNASSIGN`
* `UNASSUME`
* `UNBIND`
* `UTPC`
* `UTPF`
* `UTPN`
* `UTPT`
* `VANDERMONDE`
* `VER`
* `VISIT`
* `VISITB`
* `VPOTENTIAL`
* `VTYPE`
* `*W`
* `WIREFRAME`
* `XNUM`
* `XQ`
* `XVOL`
* `XXRNG`
* `YSLICE`
* `YVOL`
* `YYRNG`
* `ZEROS`
* `ZFACTOR`
* `ZVOL`
* `;` (Semicolon)

## Unapplicable commands

The following commands are not applicable to the DB48X implementation of RPL,
for example because they are dealing with hardware details or system-level
features that have no equivalent on the hardware DB48X runs on.
As a result, they behave like normal names on DB48X.

* `BAUD`
* `BUFLEN`
* `C$`
* `CKSM`
* `CLKADJ`
* `CLOSEIO`
* `CR`
* `DELAY`
* `FILER`
* `FINISH`
* `FLASHEVAL`
* `FREE`
* `KERRM`
* `→KEYTIME`
* `KEYTIME→`
* `KGET`
* `LANGUAGE→`
* `→LANGUAGE`
* `LIBEVAL`
* `LIBS`
* `MERGE`
* `MINEHUNT`
* `→NDISP`
* `OLDPRT`
* `OPENIO`
* `PARITY`
* `PINIT`
* `PKT`
* `PR1`
* `PRLCD`
* `PRST`
* `PRSTC`
* `PRVAR`
* `PVARS`
* `RECN`
* `RECV`
* `RESTORE`
* `ROMUPLOAD`
* `RPL>`
* `SBRK`
* `SEND`
* `SERVER`
* `SRECV`
* `STIME`
* `STRM`
* `SYSEVAL`
* `TRANSIO`
* `UFL1→MINIF`
* `WSLOG`
* `XGET`
* `XMIT`
* `XPUT`
* `XRECV`
* `XSEND`
* `XSERV`
# Performance measurements

This sections tracks some performance measurements across releases.

# DB48X Documentation Review

## Summary
This document contains a comprehensive review of the DB48X documentation for grammar, spelling, technical accuracy, and stylistic consistency.

## Critical Issues

### 1. Inconsistent Product Naming

**Issue**: The documentation inconsistently refers to "DM42" vs "DM-42" and "DM32" vs "DM-32" throughout.

**Current usage**:
- `0-Overview.md` line 10: "SwissMicro DM42 calculator" (no hyphen)
- `0-Overview.md` line 196: "DM-42 and DM-32" (with hyphens)  
- `0-Overview.md` line 212: "DB48X on a DM42" (no hyphen)
- `7-Performance.md`: Consistently uses "DM42" and "DM32" (no hyphens)

**Recommendation**: 
- Use **DM42**, **DM32**, and **DM42n** (no hyphens) consistently throughout
- Exception: When referring to physical key labels on the calculator, use "DM-42 _RCL_" format to clarify it's about the physical hardware

**Locations to fix**:
- `0-Overview.md`: lines 196, 208, 209, 219, and many others
- Search for "DM-" and replace with "DM" except in key label contexts

### 2. Missing "DM42n" References

**Issue**: The new DM42n variant is not mentioned in the documentation.

**Recommendation**:
- Add DM42n to product lists in `0-Overview.md` section "DB48X on DM42"
- Update keyboard interaction sections to include DM42n
- Add DM42n to performance tables in `7-Performance.md`

### 3. Grammar Issues

#### 3.1 `0-Overview.md` line 257
**Current**: "giving indicrect access"  
**Fix**: "giving **indirect** access"

#### 3.2 `0-Overview.md` line 363
**Current**: "access global varibales"  
**Fix**: "access global **variables**"

#### 3.3 `0-Overview.md` line 512
**Current**: "Based numbers with an explicit base, like `#123h` keep their base"  
**Fix**: "Based numbers with an explicit base, like `#123h`**,** keep their base" (add comma)

#### 3.4 `0-Overview.md` line 933
**Current**: "Walter Bonin initiated the WP43 firwmare"  
**Fix**: "Walter Bonin initiated the WP43 **firmware**"

#### 3.5 `1-introduction.md` line 4
**Current**: "implemented by Hewlett Packard"  
**Fix**: "implemented by Hewlett-Packard" (company name uses hyphen)

### 4. Technical Consistency Issues

#### 4.1 Key Reference Format

**Issue**: Inconsistent formatting for key references.

**Current mixed usage**:
- "_SIN_" (italic with underscores)
- `SIN` (code format)
- _SIN_ (just italic)

**Recommendation**: Use **_SIN_** (italic with underscores) consistently for physical keys, as established in line 203 of `0-Overview.md`.

#### 4.2 Stack Level Names

**Issue**: Sometimes uses X, Y, Z, T without formatting.

**Recommendation**: Use backticks for stack level names: `X`, `Y`, `Z`, `T` (already done in most places, but check consistency).

#### 4.3 Menu Command Names

**Issue**: Inconsistent capitalization and formatting of menu names.

**Examples**:
- "VariablesMenu" vs "Variables menu"
- "MainMenu" vs "main menu"

**Recommendation**: Use **PascalCase** with "Menu" suffix for all menu references: `VariablesMenu`, `MainMenu`, `StackMenu`, etc.

### 5. Style and Readability

#### 5.1 Unicode Arrows

**Current**: Uses both `▶︎` and `▶` inconsistently.

**Recommendation**: Choose one arrow style and use consistently:
- For navigation: ◀︎ and ▶︎ (with variation selector)
- For stack diagrams: `X` ▶ `Y+X`

#### 5.2 Shift Key Notation

**Current**: Uses 🟨 and 🟦 consistently (good!).

**Recommendation**: Keep this consistent. Always explain the meaning on first use in each major section.

#### 5.3 Long Lines

**Issue**: Many lines exceed 80 characters, making diffs harder to read.

**Recommendation**: Consider wrapping at 80 columns where practical (this is a minor style issue, not critical).

### 6. Technical Accuracy Issues

#### 6.1 `0-Overview.md` line 507-516 (DM32-specific section)

**Issue**: This entire section is marked as DM32-specific:
```markdown
```

**Problem**: This content applies to **all platforms**, not just DM32. Based numbers work the same on DM42, DM32, and DM42n.

* based_integer (based number with current base)
* based_bignum (large based number with current base)
* bignum (large integer, typically more than 64 bits)
* neg_bignum (negative large integer)
* integer (typically for integers where value takes less than 64 bits)
* neg_integer (negative small integer)
* fraction
* neg_fraction
* big_fraction
* neg_big_fraction
* hwfloat (hardware-accelerated 32-bit binary floating point)
* hwdouble (hardware-accelerated 64-bit binary floating point)
* decimal (variable precision decimal)
* neg_decimal (variable precision negative decimal)
* comment
* grob (HP-compatible graphic object)
* bitmap (hardware-optimized bitmap)
* Drop (commands have their name as type name)
* font
* dense_font
* sparse_font
* dmcp_font
* tag
* polynomial
* standard_uncertainty
* relative_uncertainty


## PEEK
Low-level read memory address


## POKE
Low level write to memory address


## NEWOB
Make a new copy of the given object


## USBFWUPDATE


## PowerOff

Turn calculator off programmatically


## SystemSetup

Display the built-in system setup


## SaveState

Save the machine's state to disk, using the current state if one was previously
loaded. This is intended to quickly save the state for example before a system
upgrade.


## ScreenCapture

Capture the current state of the screen in a dated file stored on the flash storage under the `SCREENS/` directory. This is activated by *holding* 🟨 and _O_ simultaneously. Pressing the keys one after another activates the [DisplayMenu](#displaymenu).


## BatteryVoltage

Return the current battery voltage as a decimal value.

## USBPowered

Returns `True` if the calculator is connected to USB power.

Programmers can use this command in a long-running program to alter the
frequency of power-hungry operations such as displaying on the screen.

For example, the `CollatzConjecture` library program only displays the amount of
memory used when powered by USB:

```rpl
ⓁCollatzBenchmark
```

## LowBattery

Returns `True` if the calculator is running low on battery, which is defined as
having less than 1/4th of the charge between 3000 mV and the value defined in
`MinimumBatteryVoltage`.

Programmers can use this command in long-running programs to automatically pause
their programs in order to avoid draining the battery and losing memory.

## DMCPLowBattery

Returns `True` if the calculator is running low on battery according to the DMCP
`get_lowbat_state()` function. Experimentally, this function is not very
reliable in detecting low-battery conditions. Use `LowBattery` instead.

## MinimumBatteryVoltage

This setting defines the minimum battery voltage in millivolts where the
calculator will automatically switch off to preserve battery. The default
value is 2600mV, which appears to be safe even with no-brand batteries.

Experimentally, the DM42 can operate at much lower voltages than 2.4V, but some
operations become unreliable or even cause a reset. Notably, the calculator may
not be able to wake up without rebooting, losing user data in the process.

If the battery goes below `MinimumBatteryVoltage`, the calculator will
automatically switch off with a message on the screen requesting to connect to
USB power or to change the battery. Selecting a higher value than the
default can be used to have an early reminder that you need to purchase
replacement batteries.


## BatteryRefresh

This setting defines the refresh interval in milliseconds between checks or updates of the battery levels. The default is `5000` (5 seconds).

Note that explicitly calling `BatteryVoltage`, `USBPowered` or `LowBattery`
causes the corresponding values to be immediatley refreshed, but does not
necessarily cause the battery status on screen to update.


## DMCPDisplayRefresh

On hardware calculators, use the DMCP system background display refresh.
This is the default setting, and presumably should use less energy.


## SoftwareDisplayRefresh

On hardware calculator, use the software display refresh.
This should be used for debugging purpose only.
# Tagged objects

Tagged objects are a way to indicate what a value represents, using a *tag*
between colons and preceding the object. For example, `:X:3` is a tagged
integer, where the tag is `X` and the object is `3`.

When displayed on the stack, tags are shown without the leading colon for
readability. For example, the object above shows as `X:3` on the stack.

## →Tag

Apply a tag to an object. The tag is in level 1 as text or name. The object to
be tagged is in level 2. For example, `"Hello" 1 →Tag` results in `:Hello:1`.
Like on the HP calculators, it is possible to next tags.

## Tag→

Expand a tagged object in level 1 into its object and tag. The object will be in
level 2, the tag will be in level 1 as a text object.

For example, `:Hello:1 Tag→` results in `"Hello"` in level 1 and `1` in level 2.

## DeleteTag

Remove a tag from an object. For example, `:Hello:1 DeleteTag` results in `1`.
If there is no tag, the object is returned as is.
# Operations on text


## TOUTF
Create a Utf8 string from a list of code points


## FROMUTF
List all code points in a Utf8 string


## ToText

Convert an object to its text representation.

## ToDecimal

Convert an object to its decimal representation.

## ToFraction

Convert an object to its fractional representation.

## Compile

Compile and evaluate the text, as if it was typed on the command line.

```rpl
" 1 2 + 4 * " TEXT→
@ Expecting 12
```

## CompileToNumber

Compile and evaluate a text argument to get a number.
If the argument is not a number, then an `Invalid input` error is generated.

```rpl
"25.5" Text→Number
@ Expecting 25.5
```

## CompileToAlgebraic

Compile and evaluate a text argument to get an algebraic value.
If the argument is not an algebraic value, then an `Invalid input` error is
generated.

```rpl
"[ 25.5 2 ]" Text→Algebraic
@ Expecting [ 25.5 2 ]
```

## CompileToInteger

Compile and evaluate a text argument to get an integer value.
If the argument is not an integer, then an `Invalid input` error is generated.

```rpl
"25" Text→Integer
@ Expecting 25
```

This command is typically used in validation code for the `Input` command. For
example, the following code will only accept integers that are multiple of 3.

```rpl
«
	"Enter a multiple of 3"
    { 42 0 « Text→Integer → x « if x 3 mod 0 = then x end » » }
    INPUT
»
```

## CompileToPositive

Compile and evaluate a text argument to get a positive integer.
If the argument is not a positive integer, then an `Invalid input` error is
generated.

```rpl
"25" Text→Positive
@ Expecting 25
```

## CompileToReal

Compile and evaluate a text argument to get an real number, which includes
integers, fractions and decimal values.
If the argument is not a real number, then an `Invalid input` error is
generated.

```rpl
"25/3" Text→Real
@ Expecting 8 ¹/₃
```

## CompileToObject

Compile and evaluate a text argument to get a single object.
If the argument is not a single object, then an `Invalid input` error is
generated.

```rpl
"{ 1 2 3 }" Text→Object
@ Expecting { 1 2 3 }
```

This command is typically used for `Input` validation. The HP48-compatible
approach suggested the use of `Compile`, which made it difficult to prevent
users from inputing values that would have bad side effects, e.g. placing
additional values on the stack or changing global variables.

## CompileToExpression

Compile and evaluate a text argument to get an expression.
If the argument is not an expression, then an `Invalid input` error is
generated.

```rpl
"2+3*ABC" Text→Expression
@ Expecting '2+3·ABC'
```

This command is typically used for `Input` validation.

## Char→Code

Return the Unicode codepoint of the first character in the text, or `-1` if the
text is empty. `"Hello" NUM` returns `72`.

## Text→Code

Return a list of the Unicode codepoints for all codepoints in the given text.
`"Hello" Text→Code` returns `{ 72 101 108 108 111 }`.


## Code→Text

Build a text out of a Unicode codepoint. The argument can be either a single
numerical value or a list of numerical values. A negative numerical value
produces an empty text. `{ 87 111 114 -22 108 100 }` returns `"World"`, the
value `-22` producing no character. `42 CHR` returns `"*"`, and `34 CHR` returns
`""""`, which is a 1-character text containing an ASCII quote `"`.

## SREV
Reverse the characters on a string


## NTOKENS
Number of tokens in a string


## NTHTOKEN
Token at position N in a string


## NTHTOKENPOS
Position of token N in a string


## TRIM
Remove characters at end of string


## RTRIM
Remove characters at start of string


## SSTRLEN
Length of string in characters


## STRLENCP
Length of string in Unicode code points


## TONFC
Normalize a string to Unicode NFC


## SREPL
Find and replace text in a string


## TODISPSTR
Decompile formatted for display


## TOEDITSTR
Decompile formatted for edit
# Analytic functions

In mathematics, an analytic function is a function that is locally given by a
convergent power series. There exist both real analytic functions and complex
analytic functions. Functions of each type are infinitely differentiable, but
complex analytic functions exhibit properties that do not generally hold for
real analytic functions.

DB48X analytic functions generally have a symbolic differentiation form.
Many of them also have a symbolic primitive for symbolic integration.

# Transcendental functions

## sin

[Analytic function](#analytic-functions) returning the sine of the argument.

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. If the argument for `sin` is a unit object, then
the specified angular unit overrides the angle mode to determine the result.

For complex arguments, the computation is always done considering that the real
part is in radians.

`z` ▶ *sin* `z`

`x_angleunit` ▶ *sin* `x_angleunit`

`symbexpr` ▶ `sin symbexpr`

## cos

[Analytic function](#analytic-functions) returning the cosine of the argument

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. If the argument for `cos` is a unit object, then
the specified angular unit overrides the angle mode to determine the result.

`z` ▶ *cos* `z`

`x_angleunit` ▶ *cos* `x_angleunit`

`symbexpr` ▶ `cos symbexpr`


## tan

[Analytic function](#analytic-functions) returning the tangent of the argument

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. If the argument for `tan` is a unit object, then
the specified angular unit overrides the angle mode to determine the result.

For a real argument that is an odd-integer multiple of 90 degrees, an
[infinite result](#infinite-result) is generated.

`z` ▶ *tan* `z`

`x_angleunit` ▶ *tan* `x_angleunit`

`symbexpr` ▶ `tan symbexpr`


## ASIN
Compute the arcsine


## ACOS
Compute the arccosine


## ATAN
Compute the arctangent


## ATAN2
Compute arctangent(y/x)


## LN
Compute natural logarithm


## EXP
Compute exponential function


## SINH
Compute the hyperbolic sine


## COSH
Compute the hyperbolic cosine


## TANH
Compute the hyperbolic tangent


## ASINH
Compute the hyperbolic arcsine


## ACOSH
Compute the hyperbolic arccosine


## ATANH
Compute the hyperbolic arctangent


## LOG10
Compute logarithm in base 10


## EXP10
Compute anti-logarithm in base 10


## SQRT
Compute the square root

## CBRT
Compute the cube root


## EXPM
Compute exp(x)-1


## LNP1
Compute ln(x+1)


## Ln1p
Compute ln(x+1)


## Expm1
Compute exp(x)-1


## Exp2
Compute 2^x


## HYPOT

Calculate the hypotenuse given two values (x, y), i.e., sqrt(x^2 + y^2).

 * Only works for numeric types (not integers, big integers, fractions, or complex numbers yet).
 * Not optimized for special integer cases (e.g., 3-4-5 triangle).
 * Not implemented for ranges or complex numbers (to be defined as sqrt(x^2 + y^2)).

*This command is defined as `hypot` in the code and as `⊿` on the calculator.*
# User Interface

## EditorCopy

Copy the selected text to the clipboard.

## EditorCut

Cut the text selection to the clipboard.


## EditorPaste

Insert the clipboard contents on the command-line.

## EditorSearch

Incremental search in the text editor.


## Wait

Wait for a key press or a time lapse.

When the argument is greater than 0, interrupt the program for the given number
of seconds, which can be fractional.

When the argument is 0 or negative, wait indefinitely until a key is
pressed. The key code for the key that was pressed will be pushed in the
stack. If the argument is negative, the current menu will be displayed on the
screen during the wait.

For example, the following program will count for approximately 3 seconds:

```rpl
1 30 for i
   i 1 disp
   0.1 wait
next
```

## Prompt

Display a message in the header area, then stop the currently executing program
to wait for user input. The user should press the `Run` key to resume execution
once the user has performed the requested operations.

For example, the following program displays a message at the top of the screen,
then waits for the user to put a number on the stack and hit the `Run` key. The
program then deposits the next number on the stack.

```rpl
«
	"Enter number then press Run" Prompt
    1 +
    "Next number" →Tag
»
```

Once you run the program, you can type a value, then use the `Run` key left of
`+` to resume execution:

```rpl
123
```


## Input

Prompts for data input to the command line and prevents the user access to stack
operations.

When `INPUT` is executed, the stack or history area is blanked and program
execution is suspended for data input to the command line. The contents of the
prompt argument are displayed at the top of the screen. Depending on the second
argument, the command line may also contain the contents of a string, or it may
be empty.

Pressing _ENTER_ resumes program execution and returns the contents of the
command line in string form.

The command line argument can be a text, which defines the original content of
the command line, or a list containing up to three elements:

* The object defining the initial content of the command line
* A cursor position on the command line, the index starting at 1, where 0 is the
  end of the command-line; or a list or array containing the row and column,
  starting at 1, where 0 indicates the end of of the row or the last column.
* A validation object

The validation object indicates how the input is validated. If the validation
object is a text or symbol, it can be one of the following (the comparison being
case-insensitive):

* `α`, `alpha` or `text` selects text mode and enable alpha mode. The
  value is retured as text.
* `alg`, `algebraic` or `expression` selects algebraic mode, which can be used
  to to enter an expression. The value is returned as text for `alg` (for
  compatibility with HP), as an algebraic value (including numbers) if
  `algebraic` is used, as an expression object for `expression` is used.
* `value` or `object` checks that the command line describes a single valid RPL
  program. That object is returned on the stack.
* `v`, `values` or `objects` checks that the command line describes a valid RPL
  syntax, which may contain any sequence of objects. The result is returned as
  text, which can be evaluated using `Obj→`. This mode is compatible with the HP
  usage.
* `p`, `prog` or `program` checks that the command line is a valid sequence of
  commands in RPL syntax, and returns a program object, as if the user input had
  been surrounded by `«»`
* `n` or `number` checks that the input is a numerical value, which includes
  integers, decimal, fractions and complex numbers, and returns the numerical
  values on the stack.
* `r` or `real` checks that the input is a real numerical value.
* `i` or `integer` checks that the input is an integer numerical value, which
  can be positive or negative.
* `positive` or `natural` checks that the input is a positive numerical
  value.

In the more general case, the validation program is a program that drops a value
on the stack if successful. The input is only accepted after the program pushes
a single value on the stack.

For example, the following program lets the user enter a number, with a default
value of `42`, and then adds one to it:

```rpl
«
	"Enter number" { 42 0 number } INPUT
    1 +
    "Next number" →Tag
»
```

The program as written above will reject non-number input values.

The program below shows how to use custom validation. It will only
accept an input value that is a multiple of 42.

```rpl
«
	"Enter multiple of 42"
    { 42 0 « Text→Integer → x « if x 42 mod 0 = then x end » » }
    INPUT
    1 +
    "Next number" →Tag
»
```

## KEYEVAL
Simulate a keypress from within a program


## Key

Get instantaneous state of the keyboard


## DOFORM
Take a variable identifier with a form list


## EDINSERT
Insert given text into the editor


## EDREMOVE
Remove characters in the editor at the cursor position


## EDLEFT
Move cursor to the left in the editor


## EDRIGHT
Move cursor to the right in the editor


## EDUP
Move cursor up in the editor


## EDDOWN
Move cursor down in the editor


## EDSTART
Move cursor to the start of text in the editor


## EDEND
Move cursor to the end of text in the editor


## EDLSTART
Move cursor to the start of current line in the editor


## EDLEND
Move cursor to the end of current line in the editor


## EDTOKEN
Extract one full word at the cursor location in the editor


## EDACTOKEN
Extract one word at the left of cursor location (suitable for autocomplete)


## EDMODE
Change the cursor mode in the editor


## SETTHEME
Set system color theme


## GETTHEME
# Operations with Units

## UnitValue

Return the numeric part of a unit object.

`3_km`  ▶ `3`

## BaseUnits

Expand all unit factors to their base units.

`3_km`  ▶ `3000_m`


## Convert

Convert value from one unit to another. This convert the values in the second level of the stack to the unit of the object in the first level of the stack. Only the unit in the first level of the stack is being considered, the actual value is ignored. For example:

`3_km` `2_m` ▶ `3000_m`



## FactorUnit

Factor the unit in level 1 from the unit expression of the level 2 unit object.

`1_W` `1_N` ▶ `1_N*m/s`


## →Unit

Creates a unit object from a real number and the unit part of a unit object.
`→Unit` adds units to a number, combining the number and the unit part of a
unit object. The numerical part of the unit object is ignored.

`→Unit` is the reverse of `Unit→` or of `Obj→` applied to a unit object.

```rpl
@ Will be 3000_km
3000 2_km →Unit
```

## UnitsSIPrefixCycle

Sets a prefix cycle for SI units when using the `Cycle` key (_EEX_ or _×10ⁿ_,
next to _O_) on the command-line to cycle through SI prefixes. The cycle is
specified as a text string containing all the desired prefixes, in the order in
which they should appear. This preference is stored in the `UnitsSIPrefixCycle`
variable in the current directory.

For example, after executing `"cmkμ" UnitsSIPrefixCycle`, typing _1_ ___ _m_ __
_×10ⁿ_ will show `1_cm` on the command line. Hitting _×10ⁿ_ again will change
that command line to `1_mm`, and again to `1_km`.
# USB Communications

## USBSTATUS
Get status of the USB driver


## USBRECV
Receive an object through USB link


## USBSEND
Send an object through the USB link


## USBOFF
Disable USB port


## USBON
Enable USB port


## USBAUTORCV
Receive an object and execute it


## USBARCHIVE
Create a backup on a remote machine


## USBRESTORE
Restore a backup from a remote machine

# UserModeMenu

Like most HP calculators, DB48X features a "user mode", where the keyboard can
be redefined in whole or in part. This lets you customize the keyboard for
specific tasks, for example if the default key bindings are not optimal for a
specific situation.

User mode is activated using 🟨 _2_ (`ToggleUserMode`).
When `UserModeOnce` is selected, the first press enables user mode for one key
only, and a second press locks user mode.
When `UserModeLock` is selected, each use toggles user mode on or off.

## KeyMap

Key assignments on DB48x are not global, unlike what happens with `ASN` on HP
claculators. Instead, assignments are stored in a special variable (really a
directory containing numbered variables) called `KeyMap`. This approach makes it
possible to have per-directory key assignments, and to use normal tools such as
`Store` and `Recall` to manipulate key assignments.

If a `KeyMap` is present in the current directory, it overrides assignments
while you are in that directory. However, key assignments from the enclosing
directories are still considered when they are not overriden. In other words,
key assignments are a hierarchy. When no key assignment is found in any of the
`KeyMap` variables in any of the enclosing directories, then the default key
binding for that key applies.

## Key positions

Like on HP calculators, key assignments are specified using a key position.
The key position is given in the form `rc.ph`, where `r` is a row number, `c` is
a column number, `p` is a shift plane and `h` indicates if we held the key.
The shift plane has the following values, where `7`, `8` and `9` are DB48X
extensions.

* `0`: Unshifted (only used on input, values returned use `1`)
* `1`: Unshifted
* `2`: Left shift
* `3`: Right shift
* `4`: Uppercase alpha
* `5`: Uppercase alpha left shift
* `6`: Uppercase alpha right shift
* `7`: Lowercase alpha
* `8`: Lowercase alpha left shift
* `9`: Lowercase alpha right shift

For example, the key for `SIN` is `34.1`, the key for the right-shifted `MAT`
menu above the `9` key is `54.3`, and the key for `Z` in Alpha mode is `74.4`.


## Behaviour of assigned object

When using the key in user mode, the associated object is evaluated.
However, when editing, the associated object is inserted into the command
line. This is different from HP calculators, which do nothing in that case.

If the associated object is a text, then the text is always inserted as is into
the text editor. In that case, if the text contains a TAB character (`9 CHR`),
then the cursor will automatically be positioned at that location.

## AssignKey

The `ASN` command defines the behaviour of a given key in user mode.
It takes two arguments, an object and a [key position](#key-position).
The assignment is placed in the `KeyMap` variable for the current directory.

To clear an assignment, assign `StandardKey` to a given key.

For example, to assign the `asn` command to the key on the third row and fourth column (which is _sin_ by default), use the following:
```rpl
'ASN' 34 ASN
```

To restore the standard key assignment for that same key, use the following:
```rpl
'StandardKey' 34 ASN
```

## StoreKeys

Defines multiple keys on the user keyboard by assigning objects to specified
keys. The argument is a list where the object to assign is followed by the
position, using the same format as `ASN`.

For example, to assign Greek letters to the shifted keys `A` through `F` when in
alpha mode, use the following code:
```rpl
{ "α" 21.5 "β" 22.5 "γ" 23.5 "δ" 24.5 "ε" 25.5 "φ" 26.5 }
STOKEYS
@ Save for RecallKeys below
```

## RecallKeys

Recall the current user key assignment in the current directory.

With the key assignments given in the example for `StoreKeys`, the output will
match the input list we were given.

```rpl
RCLKEYS
@ Expecting { "α" 21.5 "β" 22.5 "γ" 23.5 "δ" 24.5 "ε" 25.5 "φ" 26.5 }
```

This will merge the key assignments of the current directory and all enclosing
directories, which will appear at end of the list. In that case, the assignments
for enclosing directories will appear after a sequence like `1` `UpDir`, `2`
`UpDir`, and so on. Keys definitions that follow such a sequence are ignored by
`StoreKeys`, which only modifies the keymap for the current directory.

## DeleteKeys

Delete key assignments.

If the argument is a specific key position, then the assignment for that key
position is removed. For example, the following code will remove the assignment
for key `E` that was specified earlier:

```rpl
25.5 DELKEYS
```

If the argument is a list, then all the positions given in the list will be
deleted. For example, to remove assignmens for keys `A`, `B` and `F` assigned
by the earlier example, you can use:

```rpl
{ 21.5 22.5 26.5 } DELKEYS
```

If the argument is `0`, then all assignments in the current directory are
removed. For example, to remove the assignments given above, use:

```rpl
0 DELKEYS
```


## StandardKey

The `StandardKey` is given as an argument to `AssignKey` or `StoreKeys` to erase
a specific key assignment.


## UserMode

Enables user mode. The opposite setting is `UserModeOff`.

## UserModeOff

Disable user mode. The opposite setting is `UserMode`.

## ToggleUserMode

Toggle `UserMode` between on and off states. When `UserModeLock` is not set,
then a first call to `ToggleUserMode` will select user mode for a single
operation, and the second call will select user mode until it is disabled.

`ToggleUserMode` is bound to 🟨 _2_ by default.

## UserModeLock

Lock user mode when using `ToggleUserMode`, meaning that user mode will not
automatically be disabled after the first operation. The opposite setting is
`UserModeOnce`.

## UserModeOnce

Enable temporary user mode, where the first activation of `ToggleUserMode` only
enables user mode for a single operation. The oppsite setting is `UserModeLock`.
