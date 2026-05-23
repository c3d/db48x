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

### Menu Tree

The complete hierarchy of all built-in menus is documented in the
[Menu Tree](#soft-menus-tree).


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
  you need to quote the name and use `RCL` like for global variables. However,
  as a convenience in algebraic mode, expressions are not evaluated. See the
  `Quote` command for details.

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

* The `MAP` operation on the HP50G recurses on inner lists. On DB48x, it does
  not by default. This is controlled by the `ListDepthIteration` setting.
  A value of `0` restores HP50G-style infinite recursion.

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

* When applied to a matrix or vector, `abs` returns a matrix or vector with the
  `abs` of elements, whereas HP calcualtors compute the Euclidean norm. The
  `norm` command performs like `abs` on HP calculators, computing the absolute
  value for real numbers, the modulus for complex numbers, and the Euclidean
  norm for arrays and vectors

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
* Mikael Djurfeldt (mikael@djurfeldt.com)
* Pasquale Pigazzini (pasquale.pigazzini@gmail.com)
* Wolf (wolfwings@wolfwings.us)
- Alex Sergeev (alexander.sergeev@live.com)

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
* `Factors`
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
* `NextPr`
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
* `PrevPr`
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
* `TRIGSIN`
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

# Constants library

The DB48X calculator features a library of constants covering mathematics,
physics, chemistry, astronomy, materials science and computer science.
The built-in constants can be extended using the `config/constants.csv`
configuration file.

## Mathematics constants

### π constant

The ratio between the circumference and the diameter of a circle.

![Definition of π](img/definition_of_pi.bmp)

### e constant

Euler's constant is the base for the natural logarithm.

![Definition of e](img/definition_of_e.bmp)

### ⅈ constant

The imaginary unit, such that ⅈ²=-1.

The picture below shows imaginary unit ⅈ in the complex plane: Real numbers are
conventionally drawn on the horizontal axis, and imaginary numbers on the
vertical axis.

![Imaginary unit](img/imaginary_unit.bmp)

### ⅉ constant

An alternative notation, often preferred by physicists, for the imaginary unit ⅈ, verifying ⅉ²=-1.

### ∞ constant

Infinity is something that is larger than any natural number.
Its numerical value is an arbitrary large number that is not really infinite.

### ? constant

The undefined constant is used to represent undefined values, such as the result of undefined operations.

### rad constant

This constant defines corresponds to one radian.

### twoπ constant

This constant holds the value of two times [π](#π-constant).

### angl constant

This constants holds one half-turn expressed in degrees.

### −∞ constant

Minus infinity, the negative counterpart of ∞.
Its numerical value is an arbitrary large negative number that
is not really infinite.

### γ constant

The Euler-Mascheroni constant is a mathematical constant
approximately equal to 0.5772, appearing in analysis and
number theory, notably in the Laurent expansion of the Riemann
zeta function and in integrals involving the natural logarithm.
Its exact nature (rational or irrational) remains unknown.


## Chemistry constants

### NA constant

Avogradro constant is the number of constituent particles per mole.

Since the redefinition of the mole in 2019, the `NA` constant is therefore
defined as an exact value. [CODATA](#codata)

### k constant

The Boltzmann constant is the proportionality factor that relates the average relative thermal
energy of particles in a gas with the thermodynamic temperature of the gas.

Since the 2019 redefinition of the SI units, the `k` constant is therefore
defined as an exact value. [CODATA](#codata)

### Vm constant

Molar volume of an ideal gas at 1 atmosphere of pressure and 0°C.

By convention, it is chosen as to be the exact result of `R·StdT/StdP`.

### R constant

The universal gas constant is the molar equivalent to the Boltzmann constant,
expressed in units of energy per temperature increment per amount of substance,
rather than energy per temperature increment per particle.

Since the 2019 redefinition of the SI units, the `R` constant is therefore
defined as an exact quantity.

Since the 2019 redefinition of the SI units, the `σ` constant is defined as an
exact result. [CODATA](#codata)

### F constant

Faraday constant. In physical chemistry, the Faraday constant is a
physical constant defined as the quotient of the total electric charge
(`qe`) by the amount (`n`) of elementary charge carriers in any given
sample of matter. It's an exact constant. [CODATA](#codata)

### StdT constant

Standard temperature as defined by IUPAC in 1982.

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value `StdT=273.15_K`.

### StdP constant

Standard pressure as defined by IUPAC in 1982, corresponding to 1 atm

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value `StdP=101.325_kPa`.

### Mu constant

Molar mass constant, defined as one twelfth of the molar mass of carbon-12.

The molar mass of an element or compound is its relative atomic mass (or atomic
weight, noted `Ar`) or relative molecular mass multiplied by the molar mass
constant.

Following the 2019 revision of the SI system, the
[Avogadro constant](#NA-constant) became exact. As a result the molar mass
constant is no longer exactly `1_g/mol`. For internal consistency this value
depends therefore on the mass unit `u` [u-constant](#u-constant) and is
calculated by the following expression: `NA·u`. [CODATA](#codata)

### MC12 constant

Molar mass of carbon-12. Since 1960, mole is the amount of substance of
a system which contains as many elementary entities as there are atoms in
12 gram of carbon-12. Since 2019, the SI definition of mole changed such
that the molar mass of carbone-12 remains nearly but no longer exactly
12 g/mol. For internal consistency this value depends therefore on the
mass unit `u` [u-constant](#u-constant) and is calculated by the following
expression: `12·Mu`. [CODATA](#codata)

### Da constant

The Dalton constant is the unit mass defined as one twelfth the mass of an
unbound neutral atom of carbon-12 in its nuclear and electronic ground state and
at rest. [CODATA](#codata)

### n0 constant

Loschmidt constant or Loschmidt's number is the number of particles
(atoms or molecules) of an ideal gas per volume (the number density),
and usually quoted at standard temperature and pressure. Since 2019 with
the redifinition of the mole, it is calculated exactly as: `NA·Vm`. [CODATA](#codata)

### SoR constant

Sakur-Tetrode constant gives the absolute entropy at an absolute temperature
of `T=1_K` and standard atmospheric pressure `StdP` for one mole of an ideal
monoatomic gas composed of particles of mass equal to the atomic mass constant.

This constant is used in the Sakur-Tetrode equation expression the entropy of a
monoatomic ideal gas in terms of its thermodynamic state. [CODATA](#codata)

### kq constant

This constant expresses the ratio of Boltzmann constant [k](#k-constant) to the
elementary charge [qe](#qe-constant). It has the exact value `kq=k/qe`.


## Physics constants

### ⅉ constant

Notation often used in physics for the imaginary verifying  `ⅉ²=-1`.

### c constant

Speed of light in vaccuum, a universal physical constant that is exactly equal
to 299,792,458 metres per second (by definition of the metre).

According to the [special theory of relativity](http://en.wikipedia.org/wiki/Special_relativity),
`c` is the upper limit for the speed at which conventional matter or energy
(and thus any signal carrying information) can travel through space.

The [theory of incomplete measurements](http://physics.dinechin.org) presents
space and time as having no existence on their own. We only derive them from
*measurements* of distance and duration made using photons. In that viewpoint,
`c` is the limit of what can be measured using photons. Anything travelling
faster than light is indistinguishable from anti-matter. This is illustrated by
the thought experiment known (or not) as the *bat and the supersonic jet*.
This little tidbit is only mentioned here because this particular theory was
devised by the primary author of DB48X.

### G constant

The gravitational constant is an empirical physical constant involved in the
calculation of gravitational effects in Sir Isaac Newton's law of universal
gravitation and in Albert Einstein's theory of general relativity.

According to Newton's law of universal gravitation, the magnitude of the
attractive force `F` between two bodies each with a spherically symmetric
density distribution is directly proportional to the product of their masses,
`m₁` and `m₂`, and inversely proportional to the square of the distance, `r`,
directed along the line connecting their centres of mass [CODATA](#codata):

![Newton Gravitation](img/NewtonGravitation.bmp)

### g constant

Acceleration of Earth gravity, equivalent to the free-fall acceleration.
Its value is exactly defined by convention as `g=9.80665_m/s²`.  It is also
a comparative units for the common concept of g-force.

### σ constant

Stefan-Boltzmann constant, the factor of proportionality in the Stefan-Boltzmann
law describing the intensity of the thermal radiation emitted by matter in terms
of that matter's temperature.

For an ideal absorber/emitter or black body, the Stefan–Boltzmann law states
that the total energy radiated per unit surface area per unit time (also known
as the radiant exitance) is directly proportional to the fourth power of the
black body's temperature, T:`M°=σ·T⁴`. [CODATA](#codata)

### ε₀ constant

Vacuum permittivity, commonly denoted ε₀ (pronounced "epsilon nought" or
"epsilon zero"), is the value of the absolute dielectric permittivity of
classical vacuum. It may also be referred to as the permittivity of free space,
the electric constant, or the distributed capacitance of the vacuum. It is an
ideal (baseline) physical constant. It is a measure of how dense of an electric
field is "permitted" to form in response to electric charges and relates the
units for electric charge to mechanical quantities such as length and force.
Its value depends on the vaccuum permeability constant `μ0`. [CODATA](#codata)

### μ₀ constant

The vacuum magnetic permeability (variously vacuum permeability, permeability of
free space, permeability of vacuum, magnetic constant) is the magnetic
permeability in a classical vacuum. It is a physical constant, conventionally
written as μ₀ (pronounced "mu nought" or "mu zero"). It quantifies the strength
of the magnetic field induced by an electric current. Its value depends on the
fine structure constant measurement `α`. [CODATA](#codata)

### ke constant

Coulomb constant as it appears in the expression of the Coulomb force:
`Fe=ke·q1·q2/r^2`. Its value depends on the vacuum permittivity `ε0`. [CODATA](#codata)

### Z₀ constant

Vaccuum characteristic impedance, also called impedance of free space.

This constant relates the magnitudes of the electric and magnetic fields of
electromagnetic radiation travelling through free space: `Zo=|E|/|H|`. Its
value depends on the vacuum permittivity `μ0`. [CODATA](#codata)

### ε₀q constant

Ratio of the vacuum permittivity to the elementary charge: `ε₀q=ε₀/qe`.
Its uncertainty is inherited from `ε₀`. [CODATA](#codata)

### qε₀ constant

Product of the vacuum permittivity by the elementary charge: `qε₀=ε₀·qe`.
Its uncertainty inherited from `ε₀`. [CODATA](#codata)

### I₀ constant

The reference sound intensity corresponds to the level of the intensity
of a sound relative to a minimal reference value, most often expressed
in decibels `dB`. `I₀=1E-12 w/m²` corresponds then to `β=0_dB` in the
definition `β=10·LOG10(I/I₀)`. By convention it has an exact value.


## Mass constants

### u constant

Unified atomic mass unit.

A unit of mass defined as one twelfth of the mass of an unbound neutral atom of
carbon-12 in its nuclear and electronic ground state and at rest. It is a non-SI
unit accepted for use with SI. It is identical to the
[Dalton](#dalton constant). [CODATA](#codata)

### me constant

Electron mass. In particle physics, the electron mass is the mass of a
stationary electron, also known as the invariant mass of the electron and it is
one of the fundamental constants of physics. Its value is closely related to
the unit mass measurement through the electron relative atomic mass:
`me=u·Ar(e)` where `Ar(e)`, i.e. [Are](#Are-constant), is determined
iteratively by frequency measurements using Penning trap. Its value can also be
calculated with the fine structure constant `α` and the Rysberg constant `R∞`. [CODATA](#codata)

### mn constant

Neutron mass measurement. Its value is determined experimentally by mass
spectrometry. [CODATA](#codata)

### mp constant

Proton mass measurement which is the hydrogen H-1 nucleus. Its value is
determined by particle drag race experiments. [CODATA](#codata)

### mH constant

Hydrogen mass measurement. The electrically neutral hydrogen H-1
atom contains a single positively charged proton in the nucleus, and
a single negatively charged electron bound to the nucleus by the Coulomb
force. Its value is measured by spectrometry.

Its value can be determined from the calculation of the electron rest mass `me`
and the measurement of the electron relative atomic mass `Ar(e)`
[Are-constant](#Are-constant) (that is, the mass of electron divided by the
atomic mass constant). [CODATA](#codata)

### mD constant

Mass of the neutral deuterium atom which is a stable isotope of hydrogen.
The electrically neutral deuterium atom H-2 (also known as heavy hydrogen)
contains a single positively charged proton and a neutron in the nucleus,
and a single negatively charged electron bound to the nucleus by the
Coulomb force. Precise measurements of deuterium is obtained by
spectrometry. [CODATA](#codata)

### mT constant

Mass of the neutral tritium atom which is an unstable isotope of hydrogen
H-3 . Its nucleus contains a single positively charged proton and two
neutrons, surrounded by a single negatively charged electron bound to
the nucleus by the Coulomb force. Its mass is measured by spectrometry. [CODATA](#codata)

### mHe constant

Mass of the neutral helium atom. The electrically neutral helium atom
He-4 contains two positively charged protons and two neutrons, and two
negatively charged electrons bound to the nucleus by the Coulomb force.
Its mass is measured by spectrometry. [CODATA](#codata)

### mμ constant

Mass of the muon which is an unstable elementary particle similar
to the electron (both are classified as leptons), with an electric
charge of `−qe` and spin -1/2, but with a much greater mass. Its mass
is evaluated from energy conservation budget in pair creation reaction. [CODATA](#codata)

### mτ constant

Mass of the tau which is an elementary particle similar to the
electron (both are classidief as leptons), with an electric charge
of `−qe` and spin -1/2, but it is heaviest of leptons. Its mass is
evaluated from energy conservation budget in pair creation reaction. [CODATA](#codata)

### mpme constant

Dimensionless ratio between the mass of the proton `mp` and the mass
of the electron `me`. Currently, the most precise measurements of the
charge-to-mass ratio of a proton still use a magnetic field like Thompson
did, but rely on measuring (cyclotron) frequencies rather than deflection. [CODATA](#codata)

### Are constant

Electron relative atomic mass. The experimental value of the electron
relative atomic mass is an important dimensionless constant (usually noted
as `Ar(e)`) which is needed to calculate the unit mass value `u`
[u-constant](#u-constant). According to COTATA2022, it is a quantity which
is determined iteratively by frequency measurements using Penning trap. [CODATA](#codata)

### mQtop constant

The top quark is the heaviest known elementary particle and decays before
it can hadronize. Its mass is measured at particle colliders via decay
product reconstruction. [Particle Data Group 2024](#particle-data-group-2024)

### mQbot constant

Bottom quark mass (also called beauty quark). It is a heavy quark whose
mass is determined from quarkonium spectroscopy and lattice QCD
calculations. [Particle Data Group 2024](#particle-data-group-2024)

### mQcha constant

Charm quark mass. A heavy quark whose mass is determined from charmonium
spectroscopy and QCD sum rules. [Particle Data Group 2024](#particle-data-group-2024)

### mQstr constant

Strange quark mass. A light quark whose mass is determined from QCD sum
rules and lattice calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mQdwn constant

Down quark mass. One of the two lightest quarks forming ordinary matter.
Its mass is determined from lattice QCD calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mQup constant

Up quark mass. The lightest quark and a constituent of protons and neutrons.
Its mass is determined from lattice QCD calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mνmax constant

Maximum neutrino mass upper bound from the KATRIN experiment (April 2025),
expressed in `eV/c²`. Neutrino mass is non-zero but extremely small; only
upper bounds are currently measurable. [Particle Data Group 2024](#particle-data-group-2024)

### mνmin constant

Minimum neutrino mass lower bound consistent with neutrino oscillation
experiments, expressed in `eV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mHig constant

Combined measurement of the Higgs boson mass from the `H → γγ` and
`H → ZZ* → 4ℓ` (4 leptons) decay channels with the ATLAS detector using 7,
8 and 13 TeV proton-proton collision data. [Reference 15](#reference-15)

### mW constant

The new 2026 CMS measurement, the W boson mass is considered the most precise
single measurement to date at the LHC and strongly confirms the Standard Model,
decreasing the likelihood that the 2022 CDF II result indicated new physics. [Reference 16](#reference-16)

### mZ° constant

The measured value of the Z° boson mass is determined primarily by the scan
method performed at e+e- colliders (LEP) using: resonance scan, cross-section
mapping & Breit-Wigner fit. [Reference 17](#reference-17)


## Electromagnetism constants

### qe constant

The elementary electric charge is a fundamental physical constant, defined as
the electric charge carried by a single proton or, equivalently, the magnitude
of the negative electric charge carried by a single electron, which has charge
`−qe`.

In the SI system of units, the value of the elementary charge is exactly defined
as `qe=1.602176634⁳⁻¹⁹` coulombs. Since the 2019 redefinition of SI base units,
the seven SI base units are defined by seven fundamental physical constants, of
which the elementary charge is one. As a consequence of this change, the value
of that constant in DB48X differs from the value in the HP50G, which named it q,
with value `1.60217733⁳⁻¹⁹` coulombs. [CODATA](#codata)

### λ₀ constant

Photon wavelength. Photon energy can be expressed using any unit of energy
such as the electronvolt (eV) or the Joule (J). For short wavelength sources,
researchers often discuss photon energies in units of eV (or keV for hard
X-rays) out of convenience.  The SI definition for 1 eV derives from the
[definitional value of the electron charge](#me-constant). Photon energy `E`
in eV can be computed from wavelength `λ` in nm as: `E=λ0/λ`. This is an
exact constant.

### f₀ constant

Photon frequency. This is the exact frequency associated to the
[photon wavelength λ0][#λ₀-constant].

### ge constant

Electron g-factor. It is a dimensionless quantity that characterizes
the magnetic moment and angular momentum of an electron. It is the
ratio of the magnetic moment (or, equivalently, the gyromagnetic ratio)
of the electron to that expected of a classical particle of the same
charge and angular momentum. The electron g-factor is one of the most
precisely measured values in physics. [CODATA](#codata)

### R∞ constant

Rydberg constant. In spectroscopy, the Rydberg constant is a physical
constant relating to the electromagnetic spectra of an atom. The constant
first arose as an empirical fitting parameter in the Rydberg formula for
the hydrogen spectral series, but Niels Bohr later showed that its value
is related to more fundamental constants according to his model of the
atom. The Rydberg constant value is inferred from measurements of atomic
transition frequencies in three different atoms (hydrogen, deuterium,
and antiprotonic helium). [CODATA](#codata)

### qme constant

Ratio between the electron charge `qe` and its mass `me`. The uncertainty
of `qme` dépends on the one of `me`. [CODATA](#codata)

### μe constant

Electron magnetic moment. The electron magnetic moment, or more
specifically the electron magnetic dipole moment, is the magnetic
moment of an electron resulting from its intrinsic properties of spin
and electric charge. Its angular momentum comes from two types of
rotation: spin and orbital motion. Therefore an external magnetic field
exerts a torque on the electron magnetic moment revealing its existence.
It's a measured quantity. [CODATA](#codata)

### μp constant

Proton magnetic moment. It is the magnetic dipole moment of the proton
resulting from its intrinsic properties of spin and electric charge. Its
angular momentum comes from two types of rotation: spin and orbital motion.
Therefore an external magnetic field exerts a torque on the proton magnetic
moment revealing its existence. It's a measured quantity.

### μn constant

Neutron magnetic moment. It is the magnetic dipole moment of the meutron
resulting from its intrinsic properties of spin. Normally it sould be
zero for an elementary neutral particle because of zero charge. The fact
that it was non-vanishing prooves that the neutron is a composite particle.
Its angular momentum comes from two types of rotation: spin and orbital
motion. Therefore an external magnetic field exerts a torque on the
neutron magnetic moment revealing its existence.
It's a measured quantity. [CODATA](#codata)

### μμ constant

Muon magnetic moment. It is the magnetic dipole moment of the meutron
resulting from its intrinsic properties of spin and electric charge.
Its angular momentum comes from two types of rotation: spin and orbital
motion. Therefore an external magnetic field exerts a torque on the muon
magnetic moment revealing its existence. It's a measured quantity. [CODATA](#codata)

### μB constant

Bohr magneton. In atomic physics, the Bohr magneton is a physical constant
and the natural unit for expressing the magnetic moment of an electron caused
by its orbital or spin angular momentum. In SI units, the Bohr magneton
depends on the electron mass `me`. [CODATA](#codata)

### μN constant

The nuclear magneton is a physical constant of magnetic moment. It
is the standard unit used to measure the magnetic dipole moment of
atomic nuclei and nucleons (protons and neutrons), essentially acting
as a scale to quantify their magnetic strength. Defined in SI units,
it depends on the measured value of the proton mass `mp`. [CODATA](#codata)

### γe constant

Electron gyromagnetic ratio. It is the ratio of the electron's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of an electron in a magnetic
field. Its value depends on the electron magnetic moment `μe`. [CODATA](#codata)

### γp constant

Proton gyromagnetic ratio. It is the ratio of the proton's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of a proton in a magnetic field.
Its value depends on the proton magnetic moment `μp`. [CODATA](#codata)

### γn constant

Neutron gyromagnetic ratio. It is the ratio of the Neutron's magnetic
moment to its angular momentum. It is a characteristic of the neutron's
nuclear spin and its sign determines the direction of precession. Its
value depends on the neutron magnetic moment `μn`. [CODATA](#codata)

### c1 constant

First radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant. [CODATA](#codata)

### c2 constant

Second radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant. [CODATA](#codata)

### c3 constant

Wien's constant also knowm as the third radiation constant. In physics, Wien's
displacement law states that the black-body radiation curve for different
temperatures will peak at different wavelengths that are inversely proportional
to the temperature. The shift of that peak is a direct consequence of the
Planck radiation law, which describes the spectral brightness or intensity of
black-body radiation as a function of wavelength at any given temperature.
However, it had been discovered by German physicist Wilhelm Wien several years
before Max Planck developed that more general equation, and describes the
entire shift of the spectrum of black-body radiation toward shorter
wavelengths as temperature increases.

Formally, the wavelength version of Wien's displacement law states that the
spectral radiance of black-body radiation per unit wavelength, peaks at the
wavelength `λpeak=c3/T` where `T` is absolute temperature. From a theoretic
expression (using Lambert W function), it's an exact constant. [CODATA](#codata)

### c3f constant

Wien's frequency constant is the frequency version of the third radiation
constant. In physics, Wien's frequency displacement law states that the
black-body radiation curve for different temperatures will peak at different
frequencies that are directly proportional to the temperature. The shift of
that peak is a direct consequence of the Planck radiation law, which describes
the spectral brightness or intensity of black-body radiation as a function
of frequency at any given temperature.

Formally, the frequency version of Wien's displacement law states that the
spectral radiance of black-body radiation per unit frequency, peaks at the
frequency `fpeak=Ⓒc3f·T` where `T` is absolute temperature. From a theoretic
expression (using Lambert W function), it's an exact constant. [CODATA](#codata)

### Rk constant

Von Klitzing constant. It appears in the expression of the Hall resistance
`Rxy=Rk/ν` (`ν` being either an integer or a fraction) of the quantum Hall
effect, a quantized version of the Hall effect which is observed in
two-dimensional electron systems subjected to low temperatures and strong
magnetic fields. It's an exact constant. [CODATA](#codata)

### G0 constant

Conductance quantum constant. It is the quantized unit of electrical
conductance. It is required when measuring the conductance of a quantum
point contact, and also, it appears explicitly in the Landauer formula:
`G(μ)=G0·ΣTn(μ) sum over n` which relates the electrical conductance of
a quantum conductor to its quantum properties. It's an exact constant. [CODATA](#codata)

### G0F constant

Fermi reduced coupling constant. It is a fundamental physical constant
that represents the strength of the weak nuclear interaction, essentially
indicating how readily particles can interact via the weak force; a
larger value signifies a stronger interaction, and it is a key parameter
in the Standard Model of particle physics, primarily used to calculate
the decay rates of particles involved in weak interactions like beta
decay. It's a measured quantity. [CODATA](#codata)

### ø constant

Magnetic flux quantum. The (superconducting) magnetic flux quantum is a
combination of fundamental physical constants: the Planck constant `h`
and the electron charge `qe`. Its value is, therefore, the same for
any superconductor. It's an exact constant. [CODATA](#codata)

### KJ constant

Josephson constant. The Josephson constant is a constant of
proportionality that relates the potential difference across a
Josephson junction to the frequency of irradiation. It's also
the inverse of the magnetic flux quantum `ø`. It's an exact
constant. [CODATA](#codata)

### Kc constant

Quantum of circulation constant. It represents the discrete unit
of circulation in a superfluid, meaning that such circulation around
a vortex can only occur in multiples of this value. The existence
of quantum vortices was first predicted by Lars Onsager in 1949 in
connection with superfluid helium. It is defined as the ratio of
Planck's constant `h` to the mass of the relevant particle `m`
chosen here as the electron. [CODATA](#codata)

## Size constants

### re constant

The classical electron radius. Through the Bohr radius `a0`, it
depends on fine structure constant `α`. [CODATA](#codata)

### rp constant

Proton charge radius. A direct measure of the proton radius. Since 2010,
the measure was done using either spectroscopy method with muonic hydrogen,
and then with deuterium atom, or either using a more recent electron-proton
scattering experiment. [CODATA](#codata)

### a0 constant

Bohr radius. The Bohr radius is a physical constant, approximately equal to
the most probable distance between the nucleus and the electron in a hydrogen
atom in its ground state. Its value depends on the vacuum electric
permittivity `ε0`. [CODATA](#codata)


## Scattering constants

### σe constant

The Thomson cross-section. This type of scattering is valid when the field
energy `h·ν` is much less than the rest mass of the electron `m0·c^2`, the
electric field of the incident wave accelerates the charged target-particle,
causing it, in turn, to emit radiation at the same frequency `ν` as the
incident wave, hence the scattering of the wave. Through the classical
electron radius `re`, its value depends on fine structure constant `α`. [CODATA](#codata)

### λc constant

Electron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of a
photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the electron, it depends on the value of the electron
mass `me`. [CODATA](#codata)

### λcp constant

Proton Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the proton, it depends on the measured value of the
proton mass `mp`. [CODATA](#codata)

### λcn constant

Neutron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the neutron, it depends on the measured value of the
neutron mass `mn`. [CODATA](#codata)

### λcμ constant

Muon Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the muon particle, it depends on the measured value of
the muon mass `mμ`. [CODATA](#codata)

### λcτ constant

Tau Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the tau particle, it depends on the measured value of
the tau mass `mτ`. [CODATA](#codata)


## Quantum constants

### h constant

The Planck constant is a fundamental physical constant that appears in quantum
mechanics. A photon's energy is equal to its frequency multiplied by the Planck
constant (`E=h·ν`), and the wavelength of a matter wave equals the Planck
constant divided by the associated particle momentum (`λ=h/p`). Since the 2019
SI redefinition, it has an exact value. [CODATA](#codata)

### ℏ constant

The reduced Planck constant, ℏ, also known as the Dirac Constant, is
exactly defined as `ℏ=h/2π`. [CODATA](#codata)

### α constant

Fine-structure constant. In physics, the fine-structure constant, also known as
the Sommerfeld constant, commonly denoted by α (the Greek letter alpha), is a
fundamental physical constant which quantifies the strength of the
electromagnetic interaction between elementary charged particles. It is measured
by observing the recoil frequency of atoms, like cesium or rubidium, when they
absorb a photon, essentially gauging how strongly the atoms recoil, which
provides an accurate determination of the constant's value using high precision
measurements achieved through techniques like matter-wave interferometry.

It is a dimensionless quantity, independent of the system of units used, which
is related to the strength of the coupling of an elementary charge `qe` with
the electromagnetic field. [CODATA](#codata)

### ΔfCs constant

Caesium (Cs) hyperfine transition. It is the transition between the two
hyperfine ground states of the caesium atom. The frequency `ΔfCs` of this
transition is used to define the second as the official time unit of the
International System of Units (SI): one second is therefore the duration
of exactly 9192631770 cycles of this radiation. It represents the "tick"
of an extremely accurate atomic clock based on the properties of the
Caesium atom. By convention this constant is exact. [CODATA](#codata)

### θw constant

The weak mixing angle or Weinberg angle. It is a parameter in the
Weinberg–Salam theory of the electroweak interaction, part of the
Standard Model of particle physics. It is the angle by which spontaneous
symmetry breaking rotates the original W0 and B0 vector boson plane,
producing as a result the Z0 boson, and the photon. Its value is
calculated from the mesurement of the following dimensionless parameter
for the W and Z bosons: `(sinθw)^2 = 1 - (mW/mZ)^2 = 0.22305 ± 0.00023`. [CODATA](#codata)

### Lpl constant

Planck length unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants. It
is the smallest distance that can be measured, and it represents the
scale at which quantum gravity effects become dominant. It is the distance
travelled by light during one Planck time `Tpl`. Its value depends on the
measured value of the gravitational constant `G`. [CODATA](#codata)

### Tpl constant

Planck time unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It is the shortest time interval that can be measured and it is fundamental
in the study of the universe beginning. It is the time required for light
to travel one Planck length `Lpl`. Its value depends on the measured value
of the gravitational constant `G`. [CODATA](#codata)

### Mpl constant

Planck mass unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It can be viewed as the mass of a black hole with a Swarzhchild radius
of 2 Planck lengths (`rs=2·Lpl`) or, the minimum mass of a black hole
is one half of the Planck mass, the latter having a Planck Length radius.
Its value depends on the measured value of the gravitational constant
`G`. [CODATA](#codata)

### Epl constant

Planck energy unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
According to the mass-energy equivalence: `Epl=Mpl·c^2` it is the energy
equivalent to the Planck mass. Considered to be the smallest possible unit
of energy, which is theoretically meaningful within the framework of quantum
gravity, where both effects of quantum mechanics and general relativity
become significant; and also the energy scale at which the universe is thought
to have existed at times near the start of the Big Bang, characterized by
extremely high densities and temperatures. Its value depends on the measured
value of the gravitational constant `G`. [CODATA](#codata)

### T°pl constant

Planck temperature unit. As an attempt to devise a universal and natural
units system, the Planck units are combinations of basic universal constants.
It is the highest temperature that conventional physics can describe. It's a
fundamental limit of quantum mechanics and is considered the temperature of
the universe during the Big Bang when quantum gravity effects became
dominant. Its value depends on the measured value of the gravitational
constant `G`. [CODATA](#codata)

### qpl constant

Planck charge

The Planck charge unit is the natural unit of electric charge in Planck units,
defined as `qpl=√(4π·ε₀·ℏ·c)`. Its value depends on the vacuum permittivity `ε₀`. [CODATA](#codata)

### ppl constant

Planck momentum

The Planck momentum unit is the natural unit of momentum in Planck units, defined
as `ppl=√(ℏ·c³/G)`. Its value depends on the gravitational constant `G`. [CODATA](#codata)

### Fpl constant

Planck force

The Planck force unit is the natural unit of force in Planck units, defined as
`Fpl=c⁴/G`. It is the maximum force postulated in general relativity. Its value
depends on the gravitational constant `G`. [CODATA](#codata)

### ρvpl constant

Planck density

The Planck density unit is the natural unit of density in Planck units, defined
as `ρvpl=c⁵/(ℏ·G²)`. Its value depends on the gravitational constant `G`. [CODATA](#codata)

### Eh constant

Hartree energy constant. It is a unit of energy used in atomic physics
and computational chemistry, which is also used in molecular orbital
calculations. It is approximately the negative electric potential energy
of an electron in a hydrogen atom's ground state, and also approximately
twice the ionization energy of a hydrogen atom. Its value depends on the
measured value of the Rydberg constant `R∞`. [CODATA](#codata)


## Astronomy constants

## Mercury constants

### GM☿ constant

Mercury gravitational parameter

It is measured by radio tracking of spacecraft (Mariner 10, MESSENGER). [Reference 4](#reference-4)

### Req☿ constant

Mercury equatorial radius

It is measured by radar altimetry and spacecraft imaging. [Reference 22](#reference-22)

### Rp☿ constant

Mercury polar radius

Mercury's polar radius is also measured, since Mercury being essentially
spherical; equatorial and polar radii are equal within measurement precision. [Reference 22](#reference-22)

### f☿ constant

Mercury oblateness

Mercury's oblateness is calculated from its equatorial and polar radii measurement.
Due to Mercury's near-spherical shape, this value is effectively zero. [Reference 22](#reference-22)

### ρv☿ constant

Mercury mean density

It is calculated from measurement of its gravitational parameter and volume.
Its high density indicates a large metallic core. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g☿ constant

Mercury surface gravity

It is calculated from measurement of `GM☿` and the equatorial radius
`Req☿`. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve☿ constant

Mercury escape velocity

Mercury's escape velocity at the equatorial surface is calculated from
measured quantities. It is the minimum speed needed to break free from
Mercury's gravitational pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot☿ constant

Mercury sidereal rotation period

Mercury's sidereal rotation period is a measured quantity. It is the true time
it takes to spin 360° on its axis. Mercury is in a 3:2 spin-orbit resonance with
the Sun, rotating three times for every two orbits. [Reference 22](#reference-22)

### ϵ☿ constant

Mercury axial tilt

Mercury's axial tilt (obliquity) is measured from spacecraft observations.
The nearly zero value is consistent with its spin-orbit resonance. It is the
angle of the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a☿ constant

Mercury semi-major axis

Mercury's orbital semi-major axis is a measured quantity. It is half the length
of the longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e☿ constant

Mercury orbital eccentricity

Mercury's orbital eccentricity is a measured quantity, it is the largest of
all major planets. When different from 0, it is a measure of how much a
planet's elliptical path deviates from a perfect circle. [Reference 23](#reference-23)

### i☿ constant

Mercury orbital inclination

Mercury's orbital inclination with respect to the ecliptic plane is a measured
angle. [Reference 23](#reference-23)

### Ω☿ constant

Mercury longitude of ascending node

Mercury's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Mercury crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω☿ constant

Mercury argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀☿ constant

Mercury last perihelion passage

Mercury's most recent time of perihelion passage is a measured quantity.
Mercury passes perihelion approximately four times per year. Value in JDN
(Julian Day Number, Gregorian calendar). It is the most recent point in its
orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Venus constants

### GM♀ constant

Venus gravitational parameter

Venus's gravitational parameter, measured by radio tracking of spacecraft
(Magellan, Venus Express). [Reference 4](#reference-4)

### Req♀ constant

Venus equatorial radius

Venus's equatorial radius is a measured quantity. Venus is essentially spherical.
[Reference 22](#reference-22)

### Rp♀ constant

Venus polar radius

Venus's polar radius is a measured quantity, essentially equal to the equatorial
radius. [Reference 22](#reference-22)

### f♀ constant

Venus oblateness

Venus's oblateness obtained from measurement is effectively zero given its near-
spherical shape. [Reference 22](#reference-22)

### ρv♀ constant

Venus mean density

Venus's mean density, calculated from measurement of its gravitational parameter and
volume. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♀ constant

Venus surface gravity

Venus's surface gravity is calculated from measurement of `GM♀` and `Req♀`.
[Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♀ constant

Venus escape velocity

Venus's escape velocity at the equatorial surface is calculated from measured
quantities. It is the minimum speed needed to break free from Venus’s gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♀ constant

Venus sidereal rotation period

Measured. Venus's sidereal rotation period is the true time it takes to spin 360°
on its axis. Venus rotates retrograde (opposite to its orbital motion), with a
rotation period longer than its orbital period. [Reference 22](#reference-22)

### ϵ♀ constant

Venus axial tilt

Measured. Venus's axial tilt. The near-180° value reflects its retrograde rotation.
It is the angle of the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a♀ constant

Venus semi-major axis

Measured. Venus's orbital semi-major axis is half the length of the longest
diameter of the elliptical orbit, which is also the average distance from one
of the radii to the orbit. [Reference 23](#reference-23)

### e♀ constant

Venus orbital eccentricity

Measured. Venus's orbital eccentricity, nearly circular (therefore, near 0). [Reference 23](#reference-23)

### i♀ constant

Venus orbital inclination

Venus's orbital inclination with respect to the ecliptic plane is a measured angle.
[Reference 23](#reference-23)

### Ω♀ constant

Venus longitude of ascending node

Venus's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Venus crosses northward
through Earth's orbital plane. [Reference 23](#reference-23)

### ω♀ constant

Venus argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the Sun),
measured in the direction of motion. [Reference 23](#reference-23)

### T₀♀ constant

Venus last perihelion passage

Measured. Venus's most recent time of perihelion passage. Venus passes perihelion
approximately twice per year. Value in JDN. It is the most recent point in its
orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Earth constants

### GM♁ constant

Earth gravitational parameter

Exact nominal value (IAU 2015). Earth's gravitational parameter. An exact nominal
value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req♁ constant

Earth equatorial radius

Exact nominal value (IAU 2015). Earth's equatorial radius. An exact nominal value
defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp♁ constant

Earth polar radius

Exact nominal value (IAU 2015). Earth's polar radius. An exact nominal value
defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### f♁ constant

Earth oblateness

Calculated from nominal value. Earth's oblateness, reflecting its equatorial
bulge due to rotation. Calculated from `Req♁` and `Rp♁`. When greater than 0, it
describes how much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Particle Data Group 2023](#particle-data-group-2023)

### ρv♁ constant

Earth mean density

Calculated from nominal value. Earth's mean density, calculated from its gravitational
parameter and volume. [Particle Data Group 2023](#particle-data-group-2023)

### g♁ constant

Earth surface gravity

Calculated from nominal value. Earth's surface gravity at the equator, calculated
from `GM♁` and `Req♁`. [Particle Data Group 2023](#particle-data-group-2023)

### Ve♁ constant

Earth escape velocity

Calculated from nominal value, it is valid at the equatorial surface. It is the
minimum speed needed to break free from Earth's gravitational pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot♁ constant

Earth sidereal rotation period

Measured. Earth's sidereal rotation period (one sidereal day), the time for one
rotation relative to the fixed stars. [Reference 24](#reference-24)

### ϵ♁ constant

Earth axial tilt

Measured. Earth's axial tilt (obliquity of the ecliptic), the primary driver of the
seasons. It is the angle of the planet's rotational axis relative to its orbital plane.
[Reference 24](#reference-24)

### a♁ constant

Earth semi-major axis

Exact nominal value (1 AU by definition). Earth's orbital semi-major axis, defined
as exactly 1 astronomical unit (AU). It is half the length of the longest diameter
of the elliptical orbit, which is also the average distance from one of the radii
to the orbit. [Particle Data Group 2023](#particle-data-group-2023)

### e♁ constant

Earth orbital eccentricity

Measured. Earth's orbital eccentricity (low since the orbit is nearly circular) . [Reference 23](#reference-23)

### i♁ constant

Earth orbital inclination

Measured (reference plane constant

near zero relative to the J2000.0 ecliptic plane).
The Earth's orbital inclination is often defined by convention with respect to
the reference plane (effectively zero) as being the ecliptic plane (in such case
this angle is zero). [Reference 23](#reference-23)

### Ω♁ constant

Earth longitude of ascending node

Earth’s longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where the Earth crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♁ constant

Earth argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♁ constant

Earth perihelion passage

Measured. Earth's most recent time of perihelion passage (early January each
year). Value in JDN. It is the most recent point in its orbit when it was
closest to the Sun. [Materials 20](#materials-20)


## Moon constants

### GM☽ constant

Moon gravitational parameter

Measured. Moon's gravitational parameter, measured by lunar laser ranging
and spacecraft radio tracking. [Reference 4](#reference-4)

### Req☽ constant

Moon equatorial radius

Measured. Moon's equatorial radius, measured by laser altimetry (LRO mission). [Reference 22](#reference-22)

### Rp☽ constant

Moon polar radius

Measured. Moon's polar radius. [Reference 22](#reference-22)

### f☽ constant

Moon oblateness

Calculated from measurement. Moon's oblateness, reflecting its slight equatorial
bulge. Calculated from `Req☽` and `Rp☽`. When greater than 0, it describes how
much a celestial body deviates from a perfect sphere, forming an "oblate spheroid".
[Reference 22](#reference-22)

### ρv☽ constant

Moon mean density

Calculated from measurement. Moon's mean density, reflecting its predominantly
silicate composition with a small iron core. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g☽ constant

Moon surface gravity

Calculated from measurement. Moon's surface gravity, approximately one sixth
of Earth's. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve☽ constant

Moon escape velocity

Calculated from measurement, it is valid at the equatorial surface. It is
the minimum speed needed to break free from Moon's gravitational pull and
never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot☽ constant

Moon sidereal rotation period

Measured. Moon's sidereal rotation period is the true time it takes to spin
360° on its axis. The Moon is tidally locked to Earth, so its rotation period
equals its orbital period. [Reference 22](#reference-22)

### ϵ☽ constant

Moon axial tilt

Measured. Moon's axial tilt is the angle of the planet's rotational axis
relative to its orbital plane. [Reference 22](#reference-22)

### a☽ constant

Moon semi-major axis

Measured. Moon's orbital semi-major axis (mean Earth-Moon distance). It is half
the length of the longest diameter of the elliptical orbit, which is also the
average distance from one of the radii to the orbit. [Reference 25](#reference-25)

### e☽ constant

Moon orbital eccentricity

Measured. Moon's orbital eccentricity. The Moon's elliptical orbit causes
noticeable variation in its apparent size (supermoon and micromoon effects).
When different from 0, it is a measure of how much the moon's elliptical path
deviates from a perfect circle. [Reference 25](#reference-25)

### i☽ constant

Moon orbital inclination

Moon's orbital inclination is a measured angle with respect to the ecliptic
plane. [Reference 25](#reference-25)

### Ω☽ constant

Moon longitude of ascending node

Moon's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where the Moon crosses
northward through Earth's orbital plane.. [Reference 25](#reference-25)

### ω☽ constant

Moon argument of perigee

Measured. Moon's argument of perigee. It is the most recent point in its
orbit when it was closest to the Earth. [Reference 25](#reference-25)

### T₀☽ constant

Moon perigee passage

Calculated from measurement (Meeus ch.50, ±1 hour). See MPERC.txt. Moon's
most recent time of perigee passage (closest approach to Earth). The perigee
recurs approximately every 27.55 days. Computed using the Meeus algorithm
(15 terms, ±1 hour accuracy). Value in JDN. [Materials 20](#materials-20) [Materials 22](#materials-22)


## Mars constants

### GM♂ constant

Mars gravitational parameter

Measured. Mars system gravitational parameter, including the contribution of
its moons Phobos and Deimos, measured by spacecraft radio tracking. [Reference 4](#reference-4)

### Req♂ constant

Mars equatorial radius

Measured. Mars's equatorial radius. [Reference 22](#reference-22)

### Rp♂ constant

Mars polar radius

Measured. Mars's polar radius. Mars is noticeably oblate due to its rotation. [Reference 22](#reference-22)

### f♂ constant

Mars oblateness

Calculated from measurement. When greater than 0, it describes how much a
celestial body deviates from a perfect sphere, forming an "oblate spheroid". [Reference 22](#reference-22)

### ρv♂ constant

Mars mean density

Calculated from measurement. Mars's mean density, intermediate between rocky
and metallic compositions. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♂ constant

Mars surface gravity

Calculated from measurement. Mars's surface gravity, approximately 38% of
Earth's. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♂ constant

Mars escape velocity

Calculated from measurement, it is valid at the equatorial surface. It is the
minimum speed needed to break free from Mars's gravitational pull and never
return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♂ constant

Mars sidereal rotation period

Measured. Mars's sidereal rotation period is the true time it takes to spin
360° on its axis. A Martian day (sol) is very similar in length to an Earth day.
[Reference 22](#reference-22)

### ϵ♂ constant

Mars axial tilt
Measured. Mars's axial tilt, similar to Earth's, gives Mars comparable
seasonal variations. It is the angle of the planet's rotational axis
relative to its orbital plane. [Reference 22](#reference-22)

### a♂ constant

Mars semi-major axis

Measured. Mars's orbital semi-major axis is half the length of the longest
diameter of the elliptical orbit, which is also the average distance from
one of the radii to the orbit. [Reference 23](#reference-23)

### e♂ constant

Mars orbital eccentricity

Measured. Mars's orbital eccentricity, significantly larger than Earth's.
When different from 0, it is a measure of how much a planet's elliptical
path deviates from a perfect circle. [Reference 23](#reference-23)

### i♂ constant

Mars orbital inclination

Mars's orbital inclination is a measured angle with respect to the ecliptic
plane. [Reference 23](#reference-23)

### Ω♂ constant

Mars longitude of ascending node
Mars's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Mars crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♂ constant

Mars argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♂ constant

Mars perihelion passage

Measured. Mars's most recent time of perihelion passage. Mars's orbital
period is approximately 1.88 years. Value in JDN. It is the most recent point
in its orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Jupiter constants

### GM♃ constant

Jupiter gravitational parameter

Exact nominal value (IAU 2015). Jupiter system gravitational parameter.
An exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req♃ constant

Jupiter equatorial radius

Exact nominal value (IAU 2015). Jupiter's equatorial radius. An exact
nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp♃ constant

Jupiter polar radius

Exact nominal value (IAU 2015). Jupiter's polar radius. An exact nominal
value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### f♃ constant

Jupiter oblateness

Calculated from nominal value. It is the largest of any planet due to its
rapid rotation and gaseous composition. When greater than 0, it describes
how much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Particle Data Group 2023](#particle-data-group-2023)

### ρv♃ constant

Jupiter mean density

Calculated from nominal value. Jupiter's mean density, lower than water
due to its gaseous composition. [Particle Data Group 2023](#particle-data-group-2023)

### g♃ constant

Jupiter surface gravity

Calculated from nominal value. Jupiter's surface gravity at the 1-bar
pressure level. [Particle Data Group 2023](#particle-data-group-2023)

### Ve♃ constant

Jupiter escape velocity

Calculated from nominal value, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Jupiter's gravitational
pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot♃ constant

Jupiter sidereal rotation period

Measured. Jupiter's sidereal rotation period is the true time it takes to
spin 360° on its axis (System III, based on radio emissions from its
magnetosphere). [Reference 22](#reference-22)

### ϵ♃ constant

Jupiter axial tilt

Measured. Jupiter's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. It is very small compared to Earth's.
[Reference 22](#reference-22)

### a♃ constant

Jupiter semi-major axis

Measured. Jupiter's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♃ constant

Jupiter orbital eccentricity

Measured. Jupiter's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i♃ constant

Jupiter orbital inclination

Jupiter's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω♃ constant

Jupiter longitude of ascending node

Jupiter's longitude of the ascending node is a measured angle
quantifying the horizontal orientation of the orbit. It represents
where Jupiter crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♃ constant

Jupiter argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♃ constant

Jupiter perihelion passage

Measured. Jupiter's most recent time of perihelion passage. Jupiter's
orbital period is approximately 11.86 years. It is the most recent point
in its orbit when it was closest to the Sun. Value in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Saturn constants

### GM♄ constant

Saturn gravitational parameter

Measured. Saturn system gravitational parameter, measured by radio
tracking of the Cassini spacecraft. [Reference 4](#reference-4)

### Req♄ constant

Saturn equatorial radius

Measured. Saturn's equatorial radius at the 1-bar pressure level. [Reference 22](#reference-22)

### Rp♄ constant

Saturn polar radius

Measured. Saturn's polar radius at the 1-bar pressure level. [Reference 22](#reference-22)

### f♄ constant

Saturn oblateness

Calculated from measurement. It is the second largest of any planet.
When greater than 0, it describes how much a celestial body deviates
from a perfect sphere, forming an "oblate spheroid". [Reference 22](#reference-22)

### ρv♄ constant

Saturn mean density

Calculated from measurement. Saturn's mean density, the lowest of any
planet constant

less than that of water. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♄ constant

Saturn surface gravity

Calculated from measurement. Saturn's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♄ constant

Saturn escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Saturn's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♄ constant

Saturn sidereal rotation period

Measured. Saturn's sidereal rotation period is the true time it takes
to spin 360° on its axis (System III, based on Cassini radio
measurements). [Reference 22](#reference-22)

### ϵ♄ constant

Saturn axial tilt

Measured. Saturn's axial tilt, which causes its rings to appear at
varying angles from Earth, It is the angle of the planet's rotational
axis relative to its orbital plane.. [Reference 22](#reference-22)

### a♄ constant

Saturn semi-major axis

Measured. Saturn's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♄ constant

Saturn orbital eccentricity

Measured. Saturn's orbital eccentricity. When different from 0, it is
a measure of how much a planet's elliptical path deviates from a perfect
circle. [Reference 23](#reference-23)

### i♄ constant

Saturn orbital inclination

Saturn's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω♄ constant

Saturn longitude of ascending node

Saturn's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Saturn
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♄ constant

Saturn argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♄ constant

Saturn perihelion passage

Measured. Saturn's most recent time of perihelion passage. Saturn's
orbital period is approximately 29.46 years. It is the most recent point
in its orbit when it was closest to the Sun. Value in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Uranus constants

### GM⛢ constant

Uranus gravitational parameter

Measured. Uranus system gravitational parameter, measured by Voyager
2 radio tracking. [Reference 4](#reference-4)

### Req⛢ constant

Uranus equatorial radius

Measured. Uranus's equatorial radius, measured by Voyager 2 and stellar
occultations. [Reference 22](#reference-22)

### Rp⛢ constant

Uranus polar radius

Measured. Uranus's polar radius. [Reference 22](#reference-22)

### f⛢ constant

Uranus oblateness

Calculated from measurement. When greater than 0, it describes how much
a celestial body deviates from a perfect sphere, forming an "oblate
spheroid". [Reference 22](#reference-22)

### ρv⛢ constant

Uranus mean density

Calculated from measurement. Uranus's mean density, characteristic of
an ice giant. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g⛢ constant

Uranus surface gravity

Calculated from measurement. Uranus's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve⛢ constant

Uranus escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Uranus's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot⛢ constant

Uranus sidereal rotation period

Measured. Uranus's sidereal rotation period is the true time it takes
to spin 360° on its axis. Uranus rotates retrograde relative to its
orbital motion. [Reference 22](#reference-22)

### ϵ⛢ constant

Uranus axial tilt

Measured. Uranus's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. Its extreme obliquity (≈98°) means
it rotates almost on its side. [Reference 22](#reference-22)

### a⛢ constant

Uranus semi-major axis

Measured. Uranus's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e⛢ constant

Uranus orbital eccentricity

Measured. Uranus's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i⛢ constant

Uranus orbital inclination

Uranus's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω⛢ constant

Uranus longitude of ascending node

Uranus's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Uranus
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω⛢ constant

Uranus argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀⛢ constant

Uranus perihelion passage

Calculated from measurement (JPL DE440 orbital propagation). Uranus's
predicted next time of perihelion passage (~2050). Uranus's orbital
period is approximately 84 years. Last perihelion: 1966. It is the most
recent point in its orbit when it was closest to the Sun. Value in JDN.
[Reference 4](#reference-4) [Materials 21](#materials-21)


## Neptune constants

### GM♆ constant

Neptune gravitational parameter

Measured. Neptune system gravitational parameter, measured by Voyager
2 radio tracking and Hubble Space Telescope astrometry of Triton. [Reference 4](#reference-4)

### Req♆ constant

Neptune equatorial radius

Measured. Neptune's equatorial radius, measured by Voyager 2 and stellar
occultations. [Reference 22](#reference-22)

### Rp♆ constant

Neptune polar radius

Measured. Neptune's polar radius. [Reference 22](#reference-22)

### f♆ constant

Neptune oblateness

Calculated from measurement. When greater than 0, it describes how
much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Reference 22](#reference-22)

### ρv♆ constant

Neptune mean density

Calculated from measurement. Neptune's mean density, characteristic of
an ice giant. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♆ constant

Neptune surface gravity

Calculated from measurement. Neptune's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♆ constant

Neptune escape velocity

Calculated from measurement, it is valid at the equatorial surface. It
is the minimum speed needed to break free from Neptune's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♆ constant

Neptune sidereal rotation period

Measured. Neptune's sidereal rotation period is the true time it takes
to spin 360° on its axis (System III, from Voyager 2 radio measurements).
[Reference 22](#reference-22)

### ϵ♆ constant

Neptune axial tilt

Measured. Neptune's axial tilt, comparable to Earth's, is the angle of
the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a♆ constant

Neptune semi-major axis

Measured. Neptune's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♆ constant

Neptune orbital eccentricity

Measured. Neptune's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i♆ constant

Neptune orbital inclination

Neptune's orbital inclination is a measured angle with respect to
the ecliptic plane. [Reference 23](#reference-23)

### Ω♆ constant

Neptune longitude of ascending node

Neptune's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Neptune
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♆ constant

Neptune argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♆ constant

Neptune perihelion passage

Calculated from measurement (JPL DE440 orbital propagation). Neptune's
predicted next time of perihelion passage (~2042). Neptune's orbital
period is approximately 164.8 years. Last perihelion: 1876. It is the
most recent point in its orbit when it was closest to the Sun. Value
in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Pluto constants

### GM♇ constant

Pluto gravitational parameter

Measured. Pluto system gravitational parameter, measured by New Horizons
radio tracking. [Reference 4](#reference-4)

### Req♇ constant

Pluto equatorial radius

Measured. Pluto's equatorial radius, measured by New Horizons occultation
and imaging. [Reference 26](#reference-26)

### Rp♇ constant

Pluto polar radius

Measured. Pluto's polar radius. Pluto is essentially spherical. [Reference 26](#reference-26)

### f♇ constant

Pluto oblateness

Calculated from measurement. Pluto being spherical, its oblateness is
effectively zero. [Reference 26](#reference-26)

### ρv♇ constant

Pluto mean density

Calculated from measurement. Pluto's mean density, consistent with
a mixture of rock and ice. [Reference 4](#reference-4) [Reference 26](#reference-26)

### g♇ constant

Pluto surface gravity

Calculated from measurement. Pluto's surface gravity. [Reference 4](#reference-4) [Reference 26](#reference-26)

### Ve♇ constant

Pluto escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Pluto's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 26](#reference-26)

### Prot♇ constant

Pluto sidereal rotation period

Measured. Pluto's sidereal rotation period is the true time it takes to
spin 360° on its axis. Pluto rotates retrograde and is tidally locked to
its moon Charon. [Reference 26](#reference-26)

### ϵ♇ constant

Pluto axial tilt

Measured. Pluto's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. [Reference 26](#reference-26)

### a♇ constant

Pluto semi-major axis

Measured. Pluto's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♇ constant

Pluto orbital eccentricity

Measured. Pluto's orbital eccentricity, large enough to bring it inside
Neptune's orbit near perihelion. When different from 0, it is a measure
of how much a planet's elliptical path deviates from a perfect circle. [Reference 23](#reference-23)

### i♇ constant

Pluto orbital inclination

Pluto's orbital inclination is a measured angle with respect to the ecliptic
plane, ir is the highest of any solar system body traditionally listed. [Reference 23](#reference-23)

### Ω♇ constant

Pluto longitude of ascending node

Pluto's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Pluto crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♇ constant

Pluto argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♇ constant

Pluto perihelion passage

Measured. Pluto's last time of perihelion passage (1989 Sep 05). Pluto's
orbital period is approximately 248 years. It is the most recent point in
its orbit when it was closest to the Sun. Next perihelion: ~2237. Value
in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)

## Sun constants

### M☉ constant

Solar mass

It is derived from the exact nominal solar gravitational parameter
`GM☉` and the measured gravitational constant `G`. [Particle Data Group 2024](#particle-data-group-2024) [Particle Data Group 2023](#particle-data-group-2023) [Reference 5](#reference-5)

### GM☉ constant

Solar gravitational parameter

It is an exact nominal value defined by the International Astronomical
Union (IAU 2015 Resolution B3). [Particle Data Group 2023](#particle-data-group-2023)

### L☉ constant

Solar luminosity

It is an exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req☉ constant

Solar equatorial radius

It is an exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp☉ constant

Solar polar radius

It is measured by helioseismology and limb-darkening observations.
[Reference 18](#reference-18) [Reference 19](#reference-19)

### f☉ constant

Solar oblateness

The Solar oblateness (or flattening) is calculated from the nominal
equatorial radius and the polar radius measurement as
`f☉=1−Rp☉/Req☉`. Its uncertainty is inherited from Jupiter `♃Rp☉`.
When greater than 0, it describes how much a celestial body deviates
from a perfect sphere, forming an "oblate spheroid". [Reference 18](#reference-18) [Reference 19](#reference-19)

### ρv☉ constant

Solar mean density

It is calculated from the nominal gravitational parameter `GM☉`, the
gravitational constant `G`, and the solar radii measurement. [Particle Data Group 2023](#particle-data-group-2023)

### g☉ constant

Solar surface gravity

The Solar surface gravity at the equator is calculated from `GM☉`
and the measurement of the equatorial radius `Req☉`. [Particle Data Group 2023](#particle-data-group-2023)

### Ve☉ constant

Solar escape velocity

The Solar escape velocity at the equatorial surface is calculated from
`GM☉` and the measurement of solar radii. It is the minimum speed needed
to break free from Sun's gravitational pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot☉ constant

Solar sidereal rotation period

The Solar sidereal rotation period at the equator (System I) is measured
by tracking surface features using Doppler techniques. It is the true
time it takes to spin 360° on its axis. [Reference 20](#reference-20) [Reference 21](#reference-21)


## Cosmology constants

### Λ constant

Cosmological constant

It represents the measured energy density of empty space (dark energy)
in Einstein's field equations. Its value is inferred from observations
of the large-scale structure and expansion of the universe. [Planck 2020](#planck-2020) [Particle Data Group 2024](#particle-data-group-2024)

### H₀ constant

Hubble constant

It is the measured current rate of expansion of the universe, expressed
in `km/(s·Mpc)`. Its value is measured from observations of the cosmic
microwave background and Type Ia supernovae. [Planck 2020](#planck-2020)

### ωb constant

Baryon density parameter

It is the measured physical density of baryonic matter (ordinary matter)
in the universe, measured from the cosmic microwave background power
spectrum. [Planck 2020](#planck-2020)

### ωc constant

Dark matter density parameter

It is the measured physical density of cold dark matter in the universe,
measured from the cosmic microwave background power spectrum. [Planck 2020](#planck-2020)

### 100θ* constant

CMB acoustic scale

The CMB Acoustic scale corresponds to the measure of one hundred times
the angular size of the sound horizon at recombination, measured from
the position of the first acoustic peak in the CMB power spectrum. [Planck 2020](#planck-2020)

### τr constant

Reionization optical depth

It is the measured integrated optical depth to Thomson scattering from
the epoch of reionization, measured from the CMB polarization spectrum.
[Planck 2020](#planck-2020)

### ns constant

Scalar spectral index

It is the measured tilt of the primordial power spectrum of density
fluctuations, where ns=1 corresponds to a scale-invariant Harrison-
Zel'dovich spectrum. [Planck 2020](#planck-2020)

### (ln(10¹⁰As)) constant

Power spectrum amplitude

Measured amplitude of the primordial power spectrum. The logarithm
of the scalar power spectrum amplitude, normalized at a pivot scale
of `0.05 Mpc⁻¹`. [Planck 2020](#planck-2020)

### MG constant

Milky Way total mass

Measured total mass of the Milky Way galaxy, including baryonic
and dark matter components, estimated from stellar kinematics and
satellite galaxy dynamics. [Reference 8](#reference-8) [Reference 9](#reference-9) [Reference 10](#reference-10)

### fDMG constant

Milky Way dark matter fraction

Measured fraction of dark matter in the Milky Way galaxy. Estimated
from rotation curves, satellite dynamics, and gravitational lensing.
[Reference 8](#reference-8) [Reference 9](#reference-9)

### RG constant

Milky Way disk radius

Measure of the equatorial radius of the Milky Way galaxy's disk,
estimated from star count surveys and kinematic models. [Reference 8](#reference-8) [Reference 9](#reference-9)

### D●SgrA* constant

Distance to Sgr A*

Measured distance from the Sun to the black hole Sagittarius A* at
the Galactic centre, measured by stellar orbit monitoring with the
GRAVITY instrument. [Reference 6](#reference-6) [Reference 7](#reference-7)

### R●SgrA* constant

Sgr A* Schwarzschild radius

Measured Schwarzschild radius of the black hole Sagittarius A*, derived
from its mass measurement by the Event Horizon Telescope collaboration.
[Reference 7](#reference-7)

### M●SgrA* constant

Sgr A* black hole mass

Measured mass of the supermassive black hole Sagittarius A* at the
Galactic centre, measured from the orbits of stars in its immediate vicinity.
[Reference 7](#reference-7)

### MNGC224 constant

Andromeda galaxy total mass

Measure of the total mass of the Andromeda galaxy (M31, NGC 224), including
dark matter, estimated from satellite galaxy kinematics and gravitational
timing arguments. [Reference 11](#reference-11) [Reference 12](#reference-12)

### fDMNGC224 constant

Andromeda dark matter fraction

Measured fraction of dark matter in the Andromeda galaxy (M31), estimated
from dynamical modelling of its halo. [Reference 11](#reference-11) [Reference 12](#reference-12)

### RNGC224 constant

Andromeda galaxy disk radius

Measurement of the mean equatorial radius of the Andromeda galaxy (M31),
estimated from stellar population surveys. [Reference 11](#reference-11)

### DNGC224 constant

Distance to Andromeda galaxy

Measurement of the mean distance from the Milky Way to the Andromeda
galaxy (M31), measured using Cepheid variable stars and tip of the red
giant branch. [Reference 11](#reference-11)

### GM★Vega constant

Vega gravitational parameter

Measurement of the gravitational parameter of the star Vega (mass times
gravitational constant `G`). Measured by interferometry and spectral
analysis. [Reference 13](#reference-13) [Reference 14](#reference-14)

### L★Vega constant

Vega luminosity

Measured luminosity of the star Vega, used as a photometric reference
standard in stellar astronomy. [Reference 13](#reference-13) [Reference 14](#reference-14)



## Materials constants

## A. GASES (at `StdT = 273.15 K = 0°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n*`, `cp`, `cv`, `γ`, `R`, `ε`, `μ`, `ρ`, `Ed`
   *: where relevant
  `γ = cp/cv` and `R = cp − cv` apply to gases only.
  `γ` is given as a calculation `cp/cv` for diatomic gases.
  `R` is given as a calculation `cp-cv` for all gases.

## Dry Air constants

Gas

### ρvAir constant

Dry air density

Measured. Volumetric density of dry air at standard temperature (0°C)
and pressure (101.325 kPa).
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### nAir constant

Refractive index of dry air

Measured. Refractive index of dry air at 589 nm (sodium D line), 0°C,
1 atm. Varies slightly with humidity and CO2 content.
[Materials 2](#materials-2)

### cpAir constant

Specific heat cp of dry air

Measured. Specific heat of dry air at constant pressure at 300 K.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### cvAir constant

Specific heat cv of dry air

Specific heat of dry air at constant volume, calculated as
`cpAir − RAir`.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### γAir constant

Adiabatic index of dry air

Calculated from measurement. Adiabatic index (heat capacity ratio) of
dry air, `γ=cp/cv`, at 300 K.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### RAir constant

Specific gas constant of dry air

Exact calculation: `R/M_Air`. Specific gas constant of dry air,
`RAir=R/M_Air`, where `M_Air=28.9647 g/mol`. [Materials 1](#materials-1)

### εAir constant

Electric permittivity of dry air

Measured. Electric permittivity of dry airat STP. The relative
permittivity `εr≈1.000536` is very close to vacuum permittivity `ε₀`.
It is a measure of how dense an electric field is "permitted" to
form in response to electric charges.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### μAir constant

Magnetic permeability of dry air

Measured. Magnetic permeability of dry air at STP. The relative
permeability `μr≈1.0000004`, essentially identical to `μ₀`. It
quantifies the strength of the magnetic field induced by an electric
current.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### ρAir constant

Resistivity of dry air

Measured. Electrical resistivity of dry air at STP. Dry air is an
excellent insulator; resistivity ranges from 10¹³ to 10¹⁶ Ω·m
depending on ionisation level. It quantifies how strongly a material
opposes the flow of electric current.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### EdAir constant

Dielectric rigidity of dry air

Measured. Dielectric breakdown field strength of dry air at STP.
Above this threshold, dry air undergoes avalanche ionisation
(lightning, spark discharge).
[Materials 1](#materials-1)
[Materials 2](#materials-2)

## N2 constant

Gas

### ρvN2 constant

Dinitrogen N2 density

Measured. Volumetric density of dinitrogen (N2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpN2 constant

Dinitrogen N2 specific heat cp

Measured. Specific heat of N2 at constant pressure.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvN2 constant

Dinitrogen N2 specific heat cv

Measured. Specific heat of N2 at constant volume.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γN2 constant

Dinitrogen N2 adiabatic index

Measured. Adiabatic index of N2, γ=cp/cv.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RN2 constant

Dinitrogen N2 specific gas constant

Exact calculation: `R/M_N2`. Specific gas constant of N2, `RN2=R/M_N2`,
where `M_N2=28.014 g/mol`. [Materials 2](#materials-2)

### εN2 constant

Electric permittivity of N2

Measured. Electric permittivity of N2 at STP. Relative permittivity
`εr≈1.000580`; N2 is widely used as an insulating gas in high-voltage
equipment. It is a measure of how dense an electric field is "permitted"
to form in response to electric charges. [Materials 2](#materials-2)

### μN2 constant

Magnetic permeability of N2

Measured. Magnetic permeability of N2 at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρN2 constant

Resistivity of N2

Measured. Electrical resistivity of dry N2 at STP. Excellent insulator
used in purged electrical enclosures. It quantifies how strongly a
material opposes the flow of electric current. [Materials 2](#materials-2)

### EdN2 constant

Dielectric rigidity of N2

Measured. Dielectric breakdown field of N2 at STP. Slightly higher
than dry air; preferred for high-voltage insulation. It is the electric
field threshold above which an avalanche ionisation occurs. [Materials 2](#materials-2)

## O2 constant

Gas

### ρvO2 constant

Dioxygen O2 density

Measured. Volumetric density of dioxygen (O2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpO2 constant

Dioxygen O2 specific heat cp

Measured. Specific heat of O2 at constant pressure.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvO2 constant

Dioxygen O2 specific heat cv

Measured. Specific heat of O2 at constant volume.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γO2 constant

Dioxygen O2 adiabatic index

Measured. Adiabatic index of O2, `γ=cp/cv`, at `300 K`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RO2 constant

Dioxygen O2 specific gas constant

Exact calculation: `R/M_O2`. Specific gas constant of O2, `RO2=R/M_O2`,
where `M_O2=31.999 g/mol`. [Materials 2](#materials-2)

### εO2 constant

Electric permittivity of O2

Measured. Electric permittivity of O2 at STP. Relative permittivity
`εr≈1.000530`, comparable to N2. It is a measure of how dense an electric
field is "permitted" to form in response to electric charges. [Materials 2](#materials-2)

### μO2 constant

Magnetic permeability of O2

Measured. Magnetic permeability of O2 at STP. Paramagnetic with
`μr≈1.0000019` due to its unpaired electrons. It quantifies the strength
of the magnetic field induced by an electric current. [Materials 2](#materials-2)

### ρO2 constant

Resistivity of O2

Measured. Electrical resistivity of dry O2 at STP. Good insulator
under standard conditions. It quantifies how strongly a material
opposes the flow of electric current. [Materials 2](#materials-2)

### EdO2 constant

Dielectric rigidity of O2

Measured. Dielectric breakdown electric field of O2 at STP.
Slightly lower than N2. [Materials 2](#materials-2)

## Argon constant

Gas

### ρvAr constant

Argon density

Measured. Volumetric density of argon (Ar) at `0°C`,
`101.325 kPa`. [Materials 2](#materials-2)

### cpAr constant

Argon specific heat cp

Exact calculation: `(5/2)·R/M_Ar` (monatomic ideal gas). Specific
heat of argon at constant pressure. [Materials 2](#materials-2)

### cvAr constant

Argon specific heat cv

Specific heat of argon at constant volume. Exactly `(3/2)·R/M_Ar` for
a monatomic ideal gas. [Materials 2](#materials-2)

### γAr constant

Argon adiabatic index

Adiabatic index of argon. Exactly `5/3` for any monatomic ideal gas. [Materials 2](#materials-2)

### RAr constant

Argon specific gas constant

Exact calculation. Specific gas constant of argon, `RAr=R/M_Ar`, where
`M_Ar=39.948 g/mol`. [Materials 2](#materials-2)

### εAr constant

Electric permittivity of Ar

Measured. Electric permittivity of Ar at STP. Relative permittivity
`εr≈1.000516`; used as an inert dielectric medium. It is a measure of how
dense an electric field is "permitted" to form in response to electric charges.
[Materials 2](#materials-2)

### μAr constant

Magnetic permeability of Ar

Measured. Magnetic permeability of Ar at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρAr constant

Resistivity of Ar

Measured. Electrical resistivity of argon at STP. Excellent insulator; used
in plasma physics and dielectric studies. It quantifies how strongly a
material opposes the flow of electric current. [Materials 2](#materials-2)

### EdAr constant

Dielectric rigidity of Ar

Measured. Dielectric breakdown electric field of argon at STP. Lower than
air due to absence of polar molecules. [Materials 2](#materials-2)

## CO2 constant

Gas

### ρvCO2 constant

Carbon dioxide CO2 density

Measured. Volumetric density of carbon dioxide (CO2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpCO2 constant

Carbon dioxide CO2 specific heat cp

Measured. Specific heat of CO2 at constant pressure at `300 K`.
Strongly temperature-dependent.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvCO2 constant

Carbon dioxide CO2 specific heat cv

Measured. Specific heat of CO2 at constant volume at `300 K`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γCO2 constant

Carbon dioxide CO2 adiabatic index

Measured. Adiabatic index of CO2 at `300 K`. Lower than diatomic gases
due to its additional vibrational modes.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RCO2 constant

Carbon dioxide CO2 specific gas constant

Exact calculation. Specific gas constant of CO2, `RCO2=R/M_CO2`,
where `M_CO2=44.010 g/mol`. [Materials 2](#materials-2)

### εCO2 constant

Electric permittivity of CO2

Measured. Electric permittivity of CO2 at STP. Relative permittivity `εr≈1.000921`,
slightly higher than N2 and Ar due to CO2's quadrupole moment. It is a measure
of how dense an electric field is "permitted" to form in response to electric
charges.
[Materials 2](#materials-2)
[Materials 3](#materials-3)

### μCO2 constant

Magnetic permeability of CO2

Measured. Magnetic permeability of CO2 at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρCO2 constant

Resistivity of CO2

Measured. Electrical resistivity of CO2 at STP. Lower than N2 and Ar; sometimes
used as arc-quenching medium in circuit breakers. It quantifies how strongly
a material opposes the flow of electric current. [Materials 2](#materials-2)

### EdCO2 constant

Dielectric rigidity of CO2

Measured. Dielectric breakdown electric field of CO2 at STP. Comparable to dry
air. [Materials 2](#materials-2)


## B. LIQUIDS (at `T = 293.15 K = 0°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n`, `cp`, `ρ`
  For liquids: `cv ≈ `cp (difference < 1%), `γ ≈ 1`, `R = cp - cv ≈ 0`.
  These parameters are therefore not listed for liquids.

## H2O constant

Liquid

### ρvH2O constant

Liquid water density

Measured (IAPWS-IF97). Volumetric density of liquid water at `20°C`,
`101.325 kPa`. [Materials 4](#materials-4)
[Materials 2](#materials-2)

### nH2O constant

Refractive index of liquid water

Measured. Refractive index of liquid water at `589 nm` (sodium D line), `20°C`.
[Materials 4](#materials-4)

### cpH2O constant

Specific heat cp of liquid water

Measured (IAPWS-IF97). Specific heat of liquid water at constant pressure
at `20°C`. Near its minimum value in the `0–100°C` range. [Materials 4](#materials-4)
[Materials 2](#materials-2)

### ρH2O constant

Resistivity of liquid water

Measured. Electrical resistivity of ultra-pure water at `25°C` . The value
`1.8×10⁵ Ω·m (18 MΩ·cm)` is the theoretical maximum for pure water. It
quantifies how strongly a material opposes the flow of electric current.
[Materials 4](#materials-4) [Materials 2](#materials-2)


## C. SOLIDS (at `T = 293.15 K = 20°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n`, `cp`, `ε`, `ρ*`, `Ed*`  * when relevant

## C1. Materials for optical fibers and waveguides

## Silicon constant

Dielectric

### ρvSi constant

Crystalline silicon density

Measured. Volumetric density of crystalline silicon, the foundation
of modern microelectronics and an important infrared photonic
material. [Materials 10](#materials-10) [Materials 11](#materials-11)

### nSi constant

Refractive index of crystalline silicon

Measured (Silicon is transparent only for `λ > 1.1 µm`). Refractive
index of crystalline silicon at `1150 nm`, `20°C`. [Materials 11](#materials-11)

### cpSi constant

Specific heat of silicon

Measured. Specific heat of crystalline silicon at `300 K`. [Materials 10](#materials-10)

### εsi constant

Dielectric constant of silicon

Exact convention. Dielectric constant of silicon. The relative permittivity
measures how well an applied electric field penetrates the material
compared to vacuum. Has an exact value by convention. [HP50g Manual]

### ρSi constant

Resistivity of crystalline silicon

Measured. Electrical resistivity of intrinsic crystalline silicon at `300 K`.
Semiconductor; resistivity varies over many orders of magnitude with doping.
It quantifies how strongly a material opposes the flow of electric current.
[Materials 10](#materials-10) [Materials 11](#materials-11)

## SiO2 constant

Dielectric

### ρvSiO2 constant

Fused silica density

Measured. Volumetric density of amorphous fused silica (vitreous SiO2),
the primary material for standard optical fibers. [Materials 5](#materials-5) [Materials 7](#materials-7)

### nSiO2 constant

Refractive index of fused silica

Measured (Malitson 1965 Sellmeier fit). Refractive index of fused silica
at `589.3 nm` (sodium D line), `20°C`. The Sellmeier-calibrated value
from Malitson (1965) is the standard reference for fiber optic design. [Materials 5](#materials-5)

### cpSiO2 constant

Specific heat of fused silica

Measured. Specific heat of fused silica at `20°C`. [Materials 7](#materials-7)

### εox constant

Dielectric constant of SiO2

Exact convention. Dielectric constant of SiO2 (silicon dioxide). The relative
permittivity of thermally grown or deposited silicon dioxide, used in MOS
device modelling. Has an exact value by convention. It measures how well
an applied electric field penetrates the material compared to vacuum. [HP50g Manual]

### ρSiO2 constant

Resistivity of fused silica

Measured. Electrical resistivity of fused silica at `20°C`. Among the best
solid insulators; used in high-voltage isolation and MEMS. It quantifies
how strongly a material opposes the flow of electric current. [Materials 7](#materials-7)

### EdSiO2 constant

Dielectric rigidity of fused silica

Measured. Dielectric breakdown electric field of fused silica. Very high;
SiO2 is the standard gate dielectric in MOSFET technology. [Materials 7](#materials-7)

## GeO2 constant

Dielectric

## ρvGeO2 constant

Germanate glass density

Measured. Volumetric density of germanate glass (GeO2), used as a dopant
in the core of standard single-mode optical fibers to raise the refractive
index. [Materials 8](#materials-8) [Materials 9](#materials-9)

### nGeO2 constant

Refractive index of germanate glass

Measured. Refractive index of pure GeO2 glass at `589 nm`, `20°C`. The
actual core index of a doped fiber is interpolated between nSiO2
and nGeO2. [Materials 8](#materials-8)

### cpGeO2 constant

Specific heat of germanate glass

Measured. Specific heat of germanate glass. [Materials 9](#materials-9)

### ρGeO2 constant

Resistivity of germanate glass

Measured. Electrical resistivity of pure GeO2 glass at `20°C`. Lower
than fused silica but still an excellent insulator. It quantifies how
strongly a material opposes the flow of electric current. [Materials 9](#materials-9)

### EdGeO2 constant

Dielectric rigidity of germanate glass

Measured. Dielectric breakdown electric field of GeO2 glass. Similar
order of magnitude to SiO2; varies with purity. [Materials 9](#materials-9)

## Si3N4 constant

Dielectric

### ρvSi3N4 constant

Silicon nitride density

Measured. Volumetric density of silicon nitride (Si3N4) deposited by
LPCVD, the standard platform for integrated photonic waveguides.
[Materials 18](#materials-18) [Materials 19](#materials-19)

### nSi3N4 constant

Refractive index of LPCVD Si3N4

Measured; depends on deposition conditions. Refractive index of LPCVD Si3N4
at `633 nm` (He-Ne laser), `20°C`. The value depends on deposition conditions
and stoichiometry. [Materials 18](#materials-18)

### cpSi3N4 constant

Specific heat of Si3N4

Measured. Specific heat of silicon nitride. [Materials 19](#materials-19)

### ρSi3N4 constant

Resistivity of Si3N4

Measured. Electrical resistivity of LPCVD Si3N4 at `20°C`. Excellent insulator
used for passivation and etch-stop layers in integrated circuits. It quantifies
how strongly a material opposes the flow of electric current. [Materials 19](#materials-19)

### EdSi3N4 constant

Dielectric rigidity of Si3N4

Measured. Dielectric breakdown electric field of Si3N4 thin films. Higher than
SiO2; used as storage dielectric in SONOS flash memory. [Materials 19](#materials-19)


## C2. Reference optical glasses

##BK7 Glass constant

Dielectric

### ρvBK7 constant

Schott BK7 glass density

Measured. Volumetric density of Schott BK7 borosilicate glass, the
most widely used optical glass for visible-range optics. [Materials 15](#materials-15)

### nBK7 constant

Refractive index of Schott BK7 glass

Measured (Schott catalogue nominal nd). Refractive index of Schott BK7 at
`589.3 nm` (sodium d line), `20°C`. This is the nominal nd value used in
optical design. [Materials 15](#materials-15)

### cpBK7 constant

Specific heat of BK7 glass

Measured. Specific heat of BK7 glass. [Materials 15](#materials-15)

### ρBK7 constant

Resistivity of BK7 glass

Measured. Electrical resistivity of BK7 borosilicate glass at `20°C`. Used
in high-voltage optical isolators. It quantifies how strongly a material
opposes the flow of electric current. [Materials 15](#materials-15)

### EdBK7 constant

Dielectric rigidity of BK7 glass

Measured. Dielectric breakdown electric field of BK7 glass. Lower than
crystalline dielectrics due to its amorphous microstructure. [Materials 15](#materials-15)


## C3. Optical crystals (UV & IR)

## Al2O3 Sapphire constant

Dielectric

### ρvAl2O3 constant

Sapphire density

Measured. Volumetric density of single-crystal sapphire (alpha-Al2O3),
used for UV-to-IR windows, substrates, and high-power laser rods. [Materials 16](#materials-16)

### nAl2O3 constant

Ordinary refractive index of sapphire

Measured (ordinary axis; birefringent material). Ordinary refractive index
of sapphire at `589 nm`, `20°C`. Sapphire is birefringent; the extraordinary
index is slightly higher. [Materials 16](#materials-16)

### cpAl2O3 constant

Specific heat of sapphire

Measured. Specific heat of sapphire at `300 K`. [Materials 16](#materials-16) [Materials 2](#materials-2)

### ρAl2O3 constant

Resistivity of sapphire

Measured. Electrical resistivity of single-crystal sapphire at `20°C`. One
of the best insulating substrates; used in power electronics and RF
substrates. It quantifies how strongly a material opposes the flow of electric
current. [Materials 16](#materials-16)

### EdAl2O3 constant

Dielectric rigidity of sapphire

Measured. Dielectric breakdown electric field of sapphire. High strength;
enables use in high-field capacitors and microwave devices. [Materials 16](#materials-16)

## CaF2 constant

Dielectric

### ρvCaF2 constant

Calcium fluoride density

Measured. Volumetric density of calcium fluoride (CaF2), a key material
for deep ultraviolet optics (`193` nm excimer lithography) and infrared
windows. [Materials 17](#materials-17) [Materials 15](#materials-15)

### nCaF2 constant

Refractive index of CaF2

Measured. Refractive index of CaF2 at `589 nm`, `20°C`. CaF2 is transparent
from `130 nm` to `10 µm`, making it useful across a very wide spectral range.
[Materials 17](#materials-17)

### cpCaF2 constant

Specific heat of CaF2

Measured. Specific heat of calcium fluoride. [Materials 15](#materials-15)

### ρCaF2 constant

Resistivity of CaF2

Measured. Electrical resistivity of single-crystal CaF2 at `20°C` . Excellent
insulator; ionic crystal structure provides high purity. It quantifies
how strongly a material opposes the flow of electric current. [Materials 17](#materials-17)

### EdCaF2 constant

Dielectric rigidity of CaF2

Measured. Dielectric breakdown electric field of CaF2 crystal. Very high;
consistent with use in high-power UV laser optics. [Materials 17](#materials-17)

## C Diamond constant

Dielectric

### ρvC constant

Diamond density

Measured. Volumetric density of cubic (type IIa) diamond. Diamond has
the highest known thermal conductivity of any material at room
temperature. [Materials 13](#materials-13) [Materials 14](#materials-14)

### nC constant

Refractive index of diamond

Measured (type IIa diamond). Refractive index of diamond at `589 nm`,
`20°C`. [Materials 14](#materials-14)

### cpC constant

Specific heat of diamond

Measured. Specific heat of diamond at `300 K`. [Materials 13](#materials-13)

### ρC constant

Resistivity of diamond

Measured (type IIa natural diamond). Electrical resistivity of type IIa diamond
at `20°C`. Resistivity varies strongly with crystal purity. It quantifies how
strongly a material opposes the flow of electric current. [Materials 13](#materials-13) [Materials 14](#materials-14)

### EdC constant

Dielectric rigidity of diamond

Measured (type IIa diamond). Dielectric breakdown electric field of diamond.
Highest of any known solid; enables use in next-generation high-power
electronics. [Materials 13](#materials-13) [Materials 14](#materials-14)


## C4. Reference metals (thermal and mechanical engineering)
    Parameters: `ρv`, `cp`
    For metals: `n` not listed (complex `n`, strongly wavelength-dependent).
    `cv ≈ cp` for all metals at room temperature (difference < 0.5%).

## Al constant

Metal

### ρvAl constant

Aluminium density

Measured. Volumetric density of pure aluminium, a standard reference
material in mechanical and thermal engineering. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpAl constant

Specific heat of aluminium

Measured. Specific heat of pure aluminium at `20°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

## Cu constant

Metal

### ρvCu constant

Copper density

Measured. Volumetric density of copper, the reference material for
thermal and electrical conductivity in engineering. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpCu constant

Specific heat of copper

Measured. Specific heat of copper at `20°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

## Fe constant

Metal

### ρvFe constant

Iron density (alpha phase)

Measured (alpha phase, BCC). Volumetric density of iron in its alpha
phase (body-centred cubic), stable below `912°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpFe constant

Specific heat of iron (alpha phase, BCC)

Measured (alpha phase). Specific heat of iron (alpha phase) at `20°C`.
[Materials 12](#materials-12) [Materials 2](#materials-2)


## Dates constants

### BastilleDay constant

French national day

### MartinLutherKingDeath constant

Martin Luther King Jr., an African-American clergyman and civil rights movement
leader, was fatally shot at the Lorraine Motel in Memphis, Tennessee, on April
4, 1968, at 6:01 p.m.

### IndependenceDay constant

Independence Day, known colloquially as the Fourth of July, is a federal holiday
in the United States which commemorates the ratification of the Declaration of
Independence by the Second Continental Congress on July 4, 1776, establishing
the United States of America.


## Computing constants

### No constant

The value of "No" is false

### Yes constant

The value of "Yes" is true

### UnixEpoch constant

Date from which all dates are counted in Unix computer systems.

### SinclairZX81RAM constant

Amount of memory in the Sinclair ZX81 personal computer.
This is also the amount of memory used in the video memory for the mouse cursor
at its smallest size on modern computers.

### PageSize constant

Base page size in the virtual memory system of most modern computers. Note that
for performance reason, many modern computers now support multiple page sizes.

### HelloWorld constant

A constant often used in computer programs to denote joy, happiness and the
emergence into the world of some new computer language.


## Primary physics data reference

### CODATA

CODATA 2022: Fundamental Physical Constants. National Institute of Standards and Technology (NIST). [Table](https://physics.nist.gov/cuu/pdf/wall_2022.pdf)

Applies to all unspecified sources for fundamental physical constants.

### Planck 2020

Planck Collaboration (2020). "Planck 2018 results. VI. Cosmological parameters". Astronomy & Astrophysics, 641, A6. arXiv:1807.06209 — DOI: 10.1051/0004-6361/201833910

### Particle Data Group 2024

Particle Data Group (2024). "Review of Particle Physics — Astrophysical Constants". Physical Review D, 110, 030001. DOI: 10.1103/PhysRevD.110.030001

### Particle Data Group 2023

Particle Data Group 2023 Prša, A., et al. (2016). "Nominal values for selected solar and planetary quantities: IAU 2015 Resolution B3". The Astronomical Journal, 152(2), 41. arXiv:1605.09788 — DOI: 10.3847/0004-6256/152/2/41

### Reference 4

Park, R.S., et al. (2021). "The JPL Planetary and Lunar Ephemerides DE440 and DE441". The Astronomical Journal, 161(3), 105. DOI: 10.3847/1538-3881/abd414

### Reference 5

Tiesinga, E., et al. (2021). "CODATA recommended values of the fundamental physical constants: 2018". Reviews of Modern Physics, 93(2), 025010. DOI: 10.1103/RevModPhys.93.025010

## Galactic and extragalactic data

### Reference 6

GRAVITY Collaboration (2021). "Mass distribution in the Galactic Center based on interferometric astrometry of multiple stellar orbits". Astronomy & Astrophysics, 647, A59. arXiv:2012.02169 — DOI: 10.1051/0004-6361/202040208

### Reference 7

Event Horizon Telescope Collaboration (2022). "First Sagittarius A* Event Horizon Telescope Results. I. The Shadow of the Supermassive Black Hole in the Center of the Milky Way". The Astrophysical Journal Letters, 930(2), L12. arXiv:2311.08680 — DOI: 10.3847/2041-8213/ac6674

### Reference 8

Eilers, A.-C., et al. (2019). "The Circular Velocity Curve of the Milky Way from 5 to 25 kpc". The Astrophysical Journal, 871(1), 120. arXiv:1810.09466 — DOI: 10.3847/1538-4357/aaf648

### Reference 9

Watkins, L.L., et al. (2019). "Evidence for an Intermediate-Mass Milky Way from Gaia DR2 Halo Globular Cluster Motions". The Astrophysical Journal, 873(2), 118. arXiv:1804.11348 — DOI: 10.3847/1538-4357/ab089f

### Reference 10

Posti, L., & Helmi, A. (2019). "Mass and shape of the Milky Way's dark matter halo with globular clusters from Gaia and Hubble". Astronomy & Astrophysics, 621, A56. arXiv:1805.01408 — DOI: 10.1051/0004-6361/201833355

### Reference 11

Peñarrubia, J., et al. (2014). "A timing constraint on the (total) mass of the Large Magellanic Cloud". Monthly Notices of the Royal Astronomical Society, 443(3), 2204–2222. arXiv:1405.4662 — DOI: 10.1093/mnras/stu879

### Reference 12

Veljanoski, J., et al. (2014). "The M31 satellite plane: Evidence for a recent flyby?" Monthly Notices of the Royal Astronomical Society: Letters, 442(1), L86–L90. arXiv:1404.5988 — DOI: 10.1093/mnrasl/slu050

### Reference 13

Yoon, J., et al. (2010). "A New View of Vega's Composition, Mass, and Age". The Astrophysical Journal, 708(1), 71–79. arXiv:0910.3124 — DOI: 10.1088/0004-637X/708/1/71

### Reference 14

Aufdenberg, J.P., et al. (2006). "First Results from the CHARA Array. VII. Long-Baseline Interferometric Measurements of Vega Consistent with a Pole-On, Rapidly Rotating Star". The Astrophysical Journal, 645(1), 664–675. arXiv:astro-ph/0603327 — DOI: 10.1086/504149

### Particle physics data

### Reference 15

ATLAS-CONF-2023-037: "ATLAS sets record precision on Higgs boson's mass" (CERN, July 21, 2023).

### Reference 16

CMS Collaboration (2026): "High-precision measurement of the W boson mass with the CMS experiment", Nature (published online April 8, 2026).

### Reference 17

LEP Electroweak Working Group, "A Combination of Preliminary Electroweak Measurements and Constraints on the Standard Model," arXiv:hep-ex/0605011, often summarized in Physics Reports 427 (2006) 257–454.
Solar data

### Reference 18

Haberreiter, M., Schmutz, W., & Kosovichev, A.G. (2008). "Solving the Discrepancy between the Seismic and Photospheric Solar Radius". The Astrophysical Journal Letters, 675(1), L53–L56. DOI: 10.1086/529492

### Reference 19

Mamajek, E.E., et al. (2015). "On the Age and Binarity of Fomalhaut". The Astrophysical Journal, 800(2), L17. arXiv:1412.5018 — DOI: 10.1088/2041-8205/800/2/L17

### Reference 20

Snodgrass, H.B., & Ulrich, R.K. (1990). "Rotation of Doppler features in the solar photosphere". The Astrophysical Journal, 351, 309–316. DOI: 10.1086/168467

### Reference 21

Beck, J.G. (2000). "A comparison of differential rotation measurements". Solar Physics, 191(1), 47–70. DOI: 10.1023/A:1005226402796

## Planetary and lunar data

### Reference 22

Archinal, B.A., et al. (2018). "Report of the IAU Working Group on Cartographic Coordinates and Rotational Elements: 2015". Celestial Mechanics and Dynamical Astronomy, 130(3), 22. DOI: 10.1007/s10569-017-9805-5

### Reference 23

NASA JPL Solar System Dynamics — Horizons On-Line Ephemeris System. URL: https://ssd.jpl.nasa.gov/horizons/ Accessed: February 2026. Epoch: J2000.0 (JD 2451545.0)

### Reference 24

Seidelmann, P.K., & Fukushima, T. (1992). "Why new time scales?" Astronomy & Astrophysics, 265(2), 833–838.

### Reference 25

Williams, J.G., et al. (2014). "Lunar interior properties from the GRAIL mission". Journal of Geophysical Research: Planets, 119(7), 1546–1578. DOI: 10.1002/2013JE004559

### Reference 26


Nimmo, F., et al. (2017). "Mean radius and shape of Pluto and Charon from New Horizons images". Icarus, 287, 12–29. arXiv:1603.00821 — DOI: 10.1016/j.icarus.2016.06.027

## Materials & Fluids data

### Materials 1

Lemmon, E.W., Jacobsen, R.T., Penoncello, S.G., & Friend, D.G. (2000). "Thermodynamic Properties of Air and Mixtures of Nitrogen, Argon, and Oxygen from 60 to 2000 K at Pressures to 2000 MPa". J. Phys. Chem. Ref. Data, 29(3), 331–385. DOI: 10.1063/1.1285884

### Materials 2

NIST WebBook of Chemistry (2024). National Institute of Standards and Technology. [Source](https://webbook.nist.gov/chemistry) — Accessed: March 2026

### Materials 3

Chase, M.W. (1998). NIST-JANAF Thermochemical Tables, 4th ed. J. Phys. Chem. Ref. Data, Monograph 9. American Chemical Society / AIP.

### Materials 4

Wagner, W., & Pruss, A. (2002). "The IAPWS Formulation 1995 for the Thermodynamic Properties of Ordinary Water Substance for General and Scientific Use". J. Phys. Chem. Ref. Data, 31(2), 387–535. DOI: 10.1063/1.1461829

### Materials 5

Malitson, I.H. (1965). "Interspecimen Comparison of the Refractive Index of Fused Silica". J. Opt. Soc. Am., 55(10), 1205–1209. DOI: 10.1364/JOSA.55.001205

### Materials 6

Palik, E.D. (Ed.) (1985). Handbook of Optical Constants of Solids. Academic Press, New York. ISBN: 0-12-544420-6

### Materials 7

Corning Incorporated (2023). Corning HPFS Fused Silica — Standard Grade Optical Properties. Product datasheet. [Source](https://www.corning.com)

### Materials 8

Fleming, J.W. (1984). "Dispersion in GeO2-SiO2 glasses". Applied Optics, 23(24), 4486–4493. DOI: 10.1364/AO.23.004486

### Materials 9

Dianov, E.M., & Guryanov, A.N. (2010). "High-power Raman fiber lasers". Quantum Electronics, 40(7), 579–599. DOI: 10.1070/QE2010v040n07ABEH014211

### Materials 10

Glassbrenner, C.J., & Slack, G.A. (1964). "Thermal Conductivity of Silicon and Germanium from 3°K to the Melting Point". Physical Review, 134(4A), A1058–A1069. DOI: 10.1103/PhysRev.134.A1058

### Materials 11

Hull, R. (Ed.) (1999). Properties of Crystalline Silicon. EMIS Datareviews Series No. 20. IEE, London. ISBN: 0-85296-933-3

### Materials 12

Ho, C.Y., Powell, R.W., & Liley, P.E. (1972). "Thermal Conductivity of the Elements". J. Phys. Chem. Ref. Data, 1(2), 279–421. DOI: 10.1063/1.3253100

### Materials 13

Slack, G.A. (1973). "Nonmetallic crystals with high thermal conductivity". J. Phys. Chem. Solids, 34(2), 321–335. DOI: 10.1016/0022-3697(73)90092-9

### Materials 14

Zaitsev, A.M. (2001). Optical Properties of Diamond: A Data Handbook. Springer, Berlin. ISBN: 3-540-66603-8

### Materials 15

Schott AG (2023). Optical Glass Data Sheets. [Source](https://www.schott.com/en-gb/products/optical-glass) — Accessed: March 2026

### Materials 16
 Dobrovinskaya, E.R., Lytvynov, L.A., & Pishchik, V. (2009). Sapphire: Material, Manufacturing, Applications. Springer, Boston. ISBN: 978-0-387-85694-0

### Materials 17

Burnett, J.H., Levine, Z.H., & Shirley, E.L. (2001). "Intrinsic birefringence in calcium fluoride and barium fluoride". Physical Review B, 64(24), 241102. DOI: 10.1103/PhysRevB.64.241102

### Materials 18

Luke, K., Okawachi, Y., Lamont, M.R.E., Gaeta, A.L., & Lipson, M. (2015). "Broadband mid-infrared frequency comb generation in a Si3N4 microresonator". Optics Letters, 40(21), 4823–4826. DOI: 10.1364/OL.40.004823

### Materials 19

NIST Materials Measurement Laboratory (2024). Materials Properties Database. [Source](https://trc.nist.gov) — Accessed: March 2026

### Materials 20

Espenak, F. (2025–2026). Sky Event Almanacs — Greenwich Mean Time. AstroPixels.com. [Source](https://www.astropixels.com/almanac/almanac21) Accessed: April–May 2026. (Source for perihelion/perigee dates of inner planets and Moon.)

### Materials 21

Park, R.S., et al. (2021). "The JPL Planetary and Lunar Ephemerides DE440 and DE441". The Astronomical Journal, 161(3), 105. DOI: 10.3847/1538-3881/abd414 (Source for perihelion dates of outer planets: Jupiter to Pluto.)

### Materials 22

Meeus, J. (1998). Astronomical Algorithms, 2nd ed. Willmann-Bell Inc., Richmond, Virginia. ISBN: 978-0-943396-61-3. Chapter 50: Perigee and Apogee of the Moon, pp. 355–358. (Formula for lunar perigee — implemented in MPERC.txt.)
# Equations library

The DB48X calculator features a library of equations covering mathematics,
physics, chemistry and computer science. The built-in equations can be extended
using the `config/equations.csv` configuration file.


## Columns and Beams

The 21 variables in the Columns and Beams section are:

* `o`: Eccentricity (offset) of load (dim.: length)
* `σcr`: Critical stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `σmax`: Maximum stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `θ`: Slope at `x` (dim.: angle)
* `A`: Cross-sectional area
* `a`: Distance to point load
* `ε`: Eccentricity (dim.: length)
* `c`: Distance to edge fiber ([Eccentric Columns](#Eccentric Columns)), or Distance to applied moment ([Beams](#Beams))
* `E`: Modulus of elasticity (dim.: pressure=force/area, in SI: pascal, Pa)
* `I`: Moment of inertia (dim.: length^4, in SI: m^4)
* `K`: Effective length factor of column
* `L`: Length of column or beam
* `M`: Applied moment (dim.: length·force, in SI: N·m)
* `Mx`: Internal bending moment at `x` (dim.: length·force, in SI: N·m)
* `P`: Load ([Eccentric Columns](#Eccentric Columns)), or Point load ([beams](#Beams)) (dim.: force)
* `Pcr`: Critical load (dim.: force)
* `r`: Radius of gyration
* `V`: Shear force at `x`
* `w`: Distributed load (dim.: force)
* `x`: Distance along beam
* `y`: Deflection at `x` (dim.: length)

For simply supported beams and cantilever beams ([Simple Deflection](#Simple Deflection)) through ([Cantilever Shear](#Cantilever Shear)), the calculations differ depending upon the location of `x` relative to the loads.

* Applied loads are positive downward.
* The applied moment is positive counterclockwise.
* Deflection is positive upward.
* Slope is positive counterclockwise
* Internal bending moment is positive counterclockwise on the left-hand part.
* Shear force is positive downward on the left-hand part.

### Elastic Buckling

These equations apply to a slender column (`K·L/r>100`) with length factor `K`.

![Elastic Buckling](img/ElasticBuckling.bmp)

* To calculate `[Pcr_kN;σcr_kPa]` (Critical load; Critical stress) from 6 known variables:
```rpl
L=7.3152_m  r=4.1148_cm  E=199947961.502_kPa  A=53.0967_cm^2  K=0.7  I=8990598.7930_mm^4
@ Expecting [ Pcr=676.60192 6324 kN σcr=127 428.24437 8 kPa ]
'ROOT(ⒺElastic Buckling;[Pcr;σcr];[1_kN;1_kPa])'
```

### Eccentric Columns

These equations apply to a slender column (`K·L/r>100`) with length factor `K`.

![Eccentric Columns](img/EccentricColumns.bmp)

* To calculate `[σmax_kPa;I_mm^4]` (Maximum stress; Moment of inertia) from 8 known variables:
```rpl
L=6.6542_m  A=187.9351_cm^2  r=8.4836_cm  E=206842718.795_kPa  K=1  P=1908.2571_kN  c=15.24_cm  ε=1.1806_cm
@ Expecting [ σmax=140 853.09700 6 kPa I=135 259 652.161 mm↑4 ]
'ROOT(ⒺEccentric Columns;[σmax;I];[1_kPa;1_mm^4])'
```

### Simple Deflection

![Simple Deflection](img/SimpleDeflection.bmp)

* To calculate `[y_in]` (Deflection at `x`) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ y=-0.60048 54094 96 in ]
'ROOT(ⒺSimple Deflection;[y];[1_in])'
```

### Simple Slope

![Simple Slope](img/SimpleSlope.bmp)

* To calculate `[Θ_°]` (Slope at `x`) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf
 w=102.783_lbf/ft  x=9_ft
@ Expecting [ θ=-8.76317 82526 7⁳⁻² ° ]
'ROOT(ⒺSimple Slope;[θ];[0_°])'
```

### Simple Moment

![Simple Moment](img/SimpleMoment.bmp)

* To calculate `[Mx_ft*lbf]` (Internal bending moment at `x`) from 7 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ Mx=9 782.1945 ft·lbf ]
'ROOT(ⒺSimple Moment;[Mx];[1_ft*lbf])'
```

### Simple Shear

![Simple Shear](img/SimpleShear.bmp)

* To calculate `[V_lbf]` (Shear force at `x`) from 6 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ V=624.387 lbf ]
'ROOT(ⒺSimple Shear;[V];[1_lbf])'
```

### Cantilever Deflection

![Cantilever Deflection](img/CantileverDeflection.bmp)

* To calculate `[y_in]` (Deflection at `x`) from 9 known variables:
```rpl
L=10_ft  E=29000000_psi  I=15_in^4  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ y=-0.33163 03448 28 in ]
'ROOT(ⒺCantilever Deflection;[y];[0_in])'
```

### Cantilever Slope

![Cantilever Slope](img/CantileverSlope.bmp)

* To calculate `[Θ_°]` (Slope at `x`) from 9 known variables:
```rpl
L=10_ft  E=29000000_psi  I=15_in^4  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ θ=-0.26522 01876 49 ° ]
'ROOT(ⒺCantilever Slope;[θ];[0_°])'
```

### Cantilever Moment

![Cantilever Moment](img/CantileverMoment.bmp)

* To calculate `[Mx_ft*lbf]` (Internal bending moment at `x`) from 7 known variables:
```rpl
L=10_ft  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ Mx=-200 ft·lbf ]
'ROOT(ⒺCantilever Moment;[Mx];[1_ft*lbf])'
```

### Cantilever Shear

![Cantilever Shear](img/CantileverShear.bmp)

* To calculate `[V_lbf]` (Shear force at `x`) from 5 known variables:
```rpl
L=10_ft  P=500_lbf  a=3_ft  x=8_ft  w=100_lbf/ft
@ Expecting [ V=200 lbf ]
'ROOT(ⒺCantilever Shear;[V];[1_lbf])'
```


## Electricity

The 78 variables in the Electricity section are:

* `∈r`: Relative permittivity
* `μr`: Relative permeability
* `ω`: Angular frequency (dim.: angle/time)
* `ω₀`: Resonant angular frequency (dim.: angle/time)
* `φ`: Phase angle
* `φp`: Parallel phase angle
* `φs`: Series phase angle
* `θ₁`: First subtended angle relative to the left end of the wire
* `θ₂`: Second subtended angle relative to the right end of the wire
* `ρ`: Resistivity (dim.: resistance·length; in SI: ohm·meter, Ω·m), or Volumic charge density ([Drift Speed & Current Density](#Drift Speed & Current Density)) (dim.: charge/volume, in SI: C/m^3)
* `ρ0`: Resistivity at the reference temperature `T0` (dim.: resistance·length; in SI: ohm·meter, Ω·m)
* `ΔI`: Current Change (dim.: charge/time; in SI: ampere, A)
* `Δt`: Time Change
* `ΔV`: Voltage Change (dim.: energy/charge; in SI: volt, V)
* `A`: Wire cross-section area ([Wire Resistance](#Wire Resistance)), or Solenoid cross-section area ([Solenoid Inductance](#Solenoid Inductance)), or Plate area ([Plate Capacitor](#Plate Capacitor))
* `C`: Capacitance (dim.: charge^2/energy; in SI: farad, F)
* `C1`: First capacitance (dim.: charge^2/energy; in SI: farad, F)
* `C2`: Second capacitance (dim.: charge^2/energy; in SI: farad, F)
* `Cp`: Parallel capacitance (dim.: charge^2/energy; in SI: farad, F)
* `Cs`: Series capacitance (dim.: charge^2/energy; in SI: farad, F)
* `d`: Plate separation (dim.: length)
* `E`: Energy (dim.: charge·voltage, in SI: joule, J)
* `Er`: Electric field at distance `r` from the source (dim.: force/charge; in SI: N/C = V/m)
* `Ep`: Electric field over an infinite plate (dim.: force/charge; in SI: N/C = V/m)
* `Ein`: Electric field inside a plate capacitor (dim.: force/charge; in SI: N/C = V/m)
* `F`: Force between charges
* `f`: Frequency (dim.: time^-1)
* `f0`: Resonant frequency (dim.: time^-1)
* `I`: Current, or Total current ([Current Divider](#Current Divider)) (dim.: charge/time; in SI: ampere, A)
* `I1`: Current in `R1` (dim.: charge/time; in SI: ampere, A)
* `Imax`: Maximum current (dim.: charge/time; in SI: ampere, A)
* `L`: Inductance (dim.: energy/current^2; in SI: henry, H), or Length ([Wire Resistance](#Wire Resistance)), ([Cylindrical Capacitor](#Cylindrical Capacitor))
* `Lp`: Parallel inductance (dim.: energy/current^2; in SI: henry, H)
* `Ls`: Series inductance (dim.: energy/current^2; in SI: henry, H)
* `N`: Number of turns
* `P`: Power (dim.: energy/time)
* `q`: Charge (in SI: coulomb, C)
* `q1`: First point charge (in SI: coulomb, C)
* `q2`: Second point charge (in SI: coulomb, C)
* `qtest`: Test point charge (in SI: coulomb, C)
* `Qp`: Parallel quality factor
* `Qs`: Series quality factor
* `r`: Charge distance
* `R`: Resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `R1`: First resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `R2`: Second resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `ri`: Inside radius
* `ro`: Outside radius
* `Rp`: Parallel resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `Rs`: Series resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `t`: Time
* `ti`: Initial time
* `tf`: Final time
* `V`: Voltage, or Total voltage (dim.: energy/charge; in SI: volt, V)
* `V1`: Voltage accross `R1` (dim.: energy/charge; in SI: volt, V)
* `Vi`: Initial voltage (dim.: energy/charge; in SI: volt, V)
* `Vf`: Final voltage (dim.: energy/charge; in SI: volt, V)
* `Vmax`: Maximum voltage (dim.: energy/charge; in SI: volt, V)
* `XC`: Reactance of capacitor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `XL`: Reactance of inductor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `σ`: Conductivity (dim.: (resistance·length)^-1; in SI: siemens/meter, S/m), or Surface charge density ([E field infinite plate](#E field infinite plate)), ([Plate capacitor](#Plate capacitor)) (dim.: charge/area, in SI: C/m^2)
* `T`: Temperature (in SI: K)
* `T0`: Fixed reference temperature for the measurement of resistivity (In SI: K)
* `uE`: Volumic density of electric energy (dim.: energy/volume)
* `αT`: Temperature coefficient of resistivity (dim.: temperature^-1, in SI: K^-1)
* `λ`: Linear charge density (dim.: charge/length, in SI: C/m)
* `J`: Current density (dim.: current/area, in SI: A/m^2)
* `Je`: Electron current density (dim.: current/area, in SI: A/m^2)
* `Jh`: Hole current density (dim.: current/area, in SI: A/m^2)
* `vd`: Drift speed
* `n`: Charge-carrier number density (dim.: volume^-1)
* `ne`: Electron number density (dim.: volume^-1)
* `nh`: Hole number density (dim.: volume^-1)
* `μe`: Electron mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `μh`: Hole mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `τc`: Mean free time between collisions
* `meeff`: Electron effective mass
* `mheff`: Hole effective mass

### Coulomb’s Law & E Field

These equations describe the electrostatic force between two point charged particles and the electric field observed at the position of a test charge which replaces one of the two charges `q1` or `q2` in the expression of the electric force. A finite object carrying a net charge `q1` can be considered as a point charge if the distance to the position of the point charge `q2` is much greater than the object dimension, see example 2, for the approximate calculations of the electric force and electric field far away from a charged plate.

* **Example 1**. To calculate `[F_N;Er_N/C]` (Electric force; Electric Field at position `r`) from 5 known variables:
```rpl
q1=1.6E-19_C  q2=1.6E-19_C  r=4.00E-13_cm  εr=1  qtest=1.6E-19_C
@ Expecting [ F=14.38008 28579 N Er=8.98755 17861 7⁳¹⁹ N/C ]
'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
```

* **Example 2**. A square metal plate `L = 8_cm` on a side carries a charge of `q1 = 6_μC`. Approximate values of the electric force & electric field for a point charge `q2 = 1_μC` located at `r = 3_m` can be calculated with Coulomb’s law if the separation distance is much greater than the plate dimension `r >> L`. The whole plate is indeed considered as being a point charge providing that `r > 10 · L`. Therefore, to calculate `[F_N;Er_N/C]`:
```rpl
L=8_cm r=3_m q1=6E-6_C  q2=1E-6_C  r=3_m  εr=1  qtest=1E-6_C
@ Expecting [ F=5.99170 11907 8⁳⁻³ N Er=5 991.70119 078 N/C ]
if 'r > 10*L' then
 'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
end
```

### E Field Infinite Line

The expression for the radial electric field at the distance `r` is approximately valid if this distance is such that `r << L` and therefore also applies to a wire of finite length `L`.

* To calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) from 4 known variables:
```rpl
Q=5E-6_C  L=3_m  r=0.05_m  εr=1
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=599 170.11907 8 N/C ]
'ROOT(ⒺE Field Infinite Line;[λ;Er];[1_C/m;1_N/C])'
@ Keep
```
The code below saves the reference value for comparison with the example 2 in [E Field Finite Line](#E Field Finite Line):
```rpl
@ Save the reference value for comparison below
Er0=Er
```

### E Field Finite Line

The expression of the radial electric field at the distance `r` depends on the subtended angles `θ₁` and `θ₂` relative to the ends of the wire of finite length `L`.

![E field finite line](img/EFieldFiniteLine.bmp)

* **Example 1.** To calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) from 6 known variables and also with the distance `r=(L/2)/tanθ₁` and angle `θ₂=360°-θ₁` (see figure):
```rpl
r='(3_m)/(2*tan 30_°)' θ₂='360_°-30_°'
Q=5E-6_C  L=3_m  r=2.5981_m  εr=1  θ₁=30_°
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=5 765.46436 894 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
@ Keep
```

* **Example 2.** To show that the infinite line of the previous section can approximate the finite case (if `r << L` realised when `r < L/10`), we calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) with the angles `θ₁=ATAN((L/2)/r)` and `θ₂=360°-θ₁` (see figure):
```rpl
Q=5E-6_C  L=3_m  r=5_cm  εr=1  θ₁='atan(L/2/r)' θ₂='360_°-θ₁'
if 'r < L/10' then
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=598 837.52392 7 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
end
@ Keep
```
Verify relative difference under condition `5_cm << 3_m` with the example of [E Field Infinite Line](#E Field Infinite Line)
```rpl
Er0 Er %Ch
@ Expecting -0.05550 93020 85
@ % of relative difference
```

### E Field Infinite Plate

The expression of the perpendicular electric field is constant over an infinite plate and can approximate the field at a distance `d` from a finite plate if it is very small compare to the dimensions (length or width `L`) of the plate. On the contrary if `d >> L`, `Ep` can be approximated if we consider the whole plate as being a point charge with `q = σ·A` (where `σ` is the surface charge density), see example 2 of [Coulomb’s Law & E Field](#Coulomb’s Law & E Field).

* To calculate `[Ep_N/C;σ_C/m^2]` (Electric Field; Linear charge density) at position `[d=5_mm]` above a square plate of width `[L=8_cm]` and surface `A=L^2` where `d << L` when `d < L/10` is verified:
```rpl
L=8_cm A='L^2' d=5_mm Q=6E-6_C  A=64_cm^2  εr=1
@ Expecting [ σ=0.00000 00937 5 C/cm↑2 Ep=52 941 049.9972 N/C ]
if 'd < L/10' then
 'ROOT(ⒺE Field Infinite Plate;[σ;Ep];[1_C/cm^2;1_N/C])'
end
```

### Ohm’s Law & Power

* To calculate `[R_Ω;P_W]` (Resistance; Power) from 2 known variables:
```rpl
V=24_V  I=16_A
@ Expecting [ R=1.5 Ω P=384 W ]
'ROOT(ⒺOhm’s Law & Power;[R;P];[1_Ω;1_W])'
```

### Volt Divider

* To calculate `[V1_V]` (Voltage) from 3 known variables:
```rpl
R1=40_Ω  R2=10_Ω  V=100_V
@ Expecting [ V1=80. V ]
'ROOT(ⒺVolt Divider;[V1];[1_V])'
```

### Current Divider

* To calculate `[I1_A]` (Current) from 3 known variables:
```rpl
R1=10_Ω  R2=6_Ω  I=15_A
@ Expecting [ I1=5.625 A ]
'ROOT(ⒺCurrent Divider;[I1];[1_A])'
```

### Wire Resistance

* To calculate `[R_Ω]` (Resistance) from 3 known variables:
```rpl
ρ=0.0035_Ω*cm  L=50_cm  A=1_cm^2
@ Expecting [ R=0.175 Ω ]
'ROOT(ⒺWire Resistance;[R];[1_Ω])'
```

### Resistivity & Conductivity

The electrical resistivity `ρ` of most materials changes with temperature. If the temperature `T` does not vary too much, a linear approximation can be used around the reference point (`ρ0`; `T0`).

* To calculate `[ρ_(Ω*m);σ_(S/m)]` (Resistivity; Conductivity) from 4 known variables:
```rpl
ρ0=1.68E-8_Ω*m  αT=4.04E-3_K^-1  T0=293,15_K  T=373,15_K
@ Expecting [ ρ=2.22297 6⁳⁻⁸ Ω·m σ=44 984 741.1758 S/m ]
'ROOT(ⒺResistivity & Conductivity;[ρ;σ];[1_(Ω*m);1_(S/m)])'
```

### Series & Parallel R

![Series & Parallel R](img/Series&ParallelR.bmp)

* To calculate `[Rs_Ω;Rp_Ω]` (Series & Parallel Resistances) from 2 known variables:
```rpl
R1=2_Ω  R2=3_Ω
@ Expecting [ Rs=5 Ω Rp=1.2 Ω ]
'ROOT(ⒺSeries & Parallel R;[Rs;Rp];[1_Ω;1_Ω])'
```

### Series & Parallel C

![Series & Parallel C](img/Series&ParallelC.bmp)

* To calculate `[Cs_μF;Cp_μF]` (Series & Parallel Capacitances) from 2 known variables:
```rpl
C1=2_μF  C2=3_μF
@ Expecting [ Cs=1.2 μF Cp=5 μF ]
'ROOT(ⒺSeries & Parallel C;[Cs;Cp];[1_μF;1_μF])'
```

### Series & Parallel L

![Series & Parallel L](img/Series&ParallelL.bmp)

* To calculate `[Ls_mH;Lp_mH]` (Series & Parallel Inductances) from 2 known variables:
```rpl
L1=17_mH  L2=16.5_mH
@ Expecting [ Ls=33.5 mH Lp=8.37313 43283 6 mH ]
'ROOT(ⒺSeries & Parallel L;[Ls;Lp];[1_mH;1_mH])'
```

### Capacitive Energy

* To calculate `[V_V;q_μC]` (Potential; Charge) from 2 known variables:
```rpl
E=0.025_J  C=20_μF
@ Expecting [ V=50. V q=1 000. μC ]
'ROOT(ⒺCapacitive Energy;[V;q];[1_V;1_μC])'
```

### Volumic Density Electric Energy

* To calculate `[uE_(J/m^3)]` (Volumic density electric energy) from 2 known variables:
```rpl
E=5_V/m  εr=1
@ Expecting [ uE=1.10677 34773 5⁳⁻¹⁰ J/m↑3 ]
'ROOT(ⒺVolumic Density Electric Energy;[uE];[1_(J/m^3)])'
```

### Inductive Energy

* To calculate `[I_A]` (Current) from 2 known variables:
```rpl
E=4_J L=15_mH
@ Expecting [ I=23.09401 07676 A ]
'ROOT(ⒺInductive Energy;[I];[1_A])'
```

### RLC Current Delay

![RLC Current Delay](img/RLCCurrentDelay.bmp)

* To calculate `[ω_r/s;φs_°;φp_°;XC_Ω;XL_Ω]` (Series & Parallel phase angles; Reactances of capacitor & inductance) from 4 known variables:
```rpl
f=107_Hz  C=80_μF  L=20_mH  R=5_Ω
@ Expecting [ ω=672.30082 7868 r/s φs=-45.82915 71488 ° φp=-5.87715 65317 1 ° XC=18.59286 71836 Ω XL=13.44601 65574 Ω ]
'ROOT(ⒺRLC Current Delay;[ω;φs;φp;XC;XL];[1_r/s;1_°;1_°;1_Ω;1_Ω])'
```

### DC Capacitor Current

These equations approximate the dc current required to charge the voltage on a capacitor in a certain time interval.

* To calculate `[ΔV_V;Δt_s;tf_s]` (Voltage difference; Time difference; Final time) from 5 known variables:
```rpl
C=15_μF  Vi=2.3_V  Vf=3.2_V  I=10_A  ti=0_μs
@ Expecting [ ΔV=0.9 V Δt=1.35 μs tf=1.35 μs ]
'ROOT(ⒺDC Capacitor Current;[ΔV;Δt;tf];[1_V;1_μs;1_μs])'
```

### Capacitor Charge

* To calculate `[q_C]` (Voltage difference; Time difference; Final time) from 2 known variables:
```rpl
C=20_μF  V=100_V
@ Expecting [ q=0.002 C ]
'ROOT(ⒺCapacitor Charge;[q];[1_C])'
```

### DC Inductor Voltage

These equations approximate the dc voltage induced in an inductor by a change in current in a certain time interval.

![DC Inductor Voltage](img/DCInductorVoltage.bmp)

* To calculate `[ΔIL_A;ILf_A;tf_μs]` (Current difference; Final current; Final time) from 5 known variables:
```rpl
L=100_mH  V=52_V  Δt=32_μs  ILi=23_A  ti=0_μs
@ Expecting [ ΔIL=-1.664⁳⁻² A ILf=22.98336 A tf=32 μs ]
'ROOT(ⒺDC Inductor Voltage;[ΔIL;ILf;tf];[1_A;1_A;1_μs])'
```

### RC Transient

![RC Transient](img/RCTransient.bmp)

* To calculate `[V_V]` (Voltage) from 5 known variables:
```rpl
Vi=0_V  C=50_μF  Vf=10_V  R=100_Ω  t=2_ms
@ Expecting [ V=3.29679 95396 4 V ]
'ROOT(ⒺRC Transient;[V];[1_V])'
```

### RL Transient

![RL Transient](img/RLTransient.bmp)

* To calculate `[I_A]` (Current) from 5 known variables:
```rpl
Vi=0_V  Vf=5_V  R=50_Ω  L=50_mH  t=75_μs
@ Expecting [ I=7.22565 13671 4⁳⁻³ A ]
'ROOT(ⒺRL Transient;[I];[1_A])'
```

### Resonant Frequency

* To calculate `[ω₀;Qs;Qp;f0]` (Resonant angular velocity; Parallel & Series quality factors; Resonant frequency) from 3 known variables:
```rpl
L=500_mH  C=8_μF  R=10_Ω
@ Expecting [ ω₀=500. r/s Qs=25. Qp=0.04 f0=79.57747 15459 Hz ]
'ROOT(ⒺResonant Frequency;[ω₀;Qs;Qp;f0];[1_r/s;1;1;1_Hz])'
```

### Plate Capacitor

![Plate Capacitor](img/PlateCapacitor.bmp)

* To calculate `[d_cm;ΔV_V;Ein_(N/C);σ_(μC/m^2)]` (Distance; Voltage; Internal E field; Surface charge density) from 4 known variables:
```rpl
C=25_μF  εr=2.26  A=1_cm^2  Q=75_μC
@ Expecting [ d=80.04185 7882 pm σ=750 000. μC/m↑2 Ein=3.74803 89378 6⁳¹⁰ N/C ΔV=3. V ]
'ROOT(ⒺPlate Capacitor;[d;σ;Ein;ΔV];[1_pm;1_(μC/m^2);1_(N/C);1_V])'
```

### Cylindrical Capacitor

![Cylindrical Capacitor](img/CylindricalCapacitor.bmp)

* To calculate `[C_μF;ΔV_V]` (Capacitance; Voltage) from 5 known variables:
```rpl
εr=1  Q=75_μC  ro=1_cm  ri=.999_cm  L=10_cm
@ Expecting [ C=5 560.46819 203 pF ΔV=13 488.07284 02 V ]
'ROOT(ⒺCylindrical Capacitor;[C;ΔV];[1_pF;1_V])'
```

### Solenoid Inductance

![Solenoid Inductance](img/SolenoidInductance.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:
```rpl
μr=2.5  n=40_1/cm  A=0.2_cm^2  h=3_cm
@ Expecting [ L=0.03015 92894 7 mH ]
'ROOT(ⒺSolenoid Inductance;[L];[1_mH])'
```

### Toroid Inductance

![Toroid Inductance](img/ToroidInductance.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:
```rpl
μr=1  N=5000  h=2_cm  ri=2_cm  ro=4_cm
@ Expecting [ L=69.31471 80468 mH ]
'ROOT(ⒺToroid Inductance;[L];[1_mH])'
```

### Sinusoidal Voltage

* To calculate `[ω_r/s;V_V]` (Angular velocity; Voltage) from 4 known variables:
```rpl
Vmax=110_V  t=30_μs  f=60_Hz  φ=15_°
@ Expecting [ ω=376.99111 8431 r/s V=29.66992 85671 V ]
'ROOT(ⒺSinusoidal Voltage;[ω;V];[1_r/s;1_V])'
```

### Sinusoidal Current

* To calculate `[I_A;f_Hz]` (Current; Frequency) from 4 known variables:
```rpl
t=3.2_s  Imax=10_A  ω=636_r/s  φ=30_°
@ Expecting [ f=101.22254 3806 Hz I=-0.28436 91656 8 A ]
'ROOT(ⒺSinusoidal Current;[f;I];[1_Hz;1_A])'
```

### Drift Speed & Current Density

* To calculate `[vd_m/s;J_(A/m^2);E_(V/m)]` (Drift speed; Current density; E field) from 5 known variables:
```rpl
I=1_A  n=8.5e28_(m^-3)  r=0.1_cm  A='Ⓒπ*(r_cm)^2'  ρ='Ⓒqe*n' σ='CONVERT(→NUM(Ⓒqe*(n_(m^-3))*40_(cm^2/(V*s)));1_S/m)'
@ Expect [ vd=2.33733 41683 6⁳⁻⁵ m/s J=31.83098 86184 A/cm↑2 E=5.84333 54209⁳⁻³ V/m ]
'ROOT(ⒺDrift Speed & Current Density;[vd;J;E];[1_m/s;1_(A/cm^2);1_(V/m)])'
```

### Electron & Hole Mobilities

In accordance with microscopic Ohm's law, the current density is proportional to the electric field. Holes and electrons therefore move at their average drift speeds during the mean free time between collisions. As long as the electric fields are not very high, the mobilities of holes and electrons are constant.

* To calculate `[μe_(cm^2/(V*s));μe_(cm^2/(V*s));J_(A/m^2);Je_(A/m^2);Jh_(A/m^2);σ_(S/m)]` (Electron & hole mobilities; Current densities; Conductivity) from 5 known variables:
```rpl
τc=4.09365 36801 40e-15_s meeff=1.09312 60456 68e-31 kg mheff=4.55469 18569 5e-31 kg nh=6.0e18_(m^-3) ne=1.04e19_(m^-3) E=6.0e-9_V/m
@ Expecting [ μe=60. cm↑2/(V·s) μh=14.4 cm↑2/(V·s) Je=5.99854 93177⁳⁻¹¹ A/m↑2 Jh=8.30568 36706 6⁳⁻¹² A/m↑2 J=6.82911 76847 6⁳⁻¹¹ A/m↑2 σ=0.01138 18628 08 S/m ]
'ROOT(ⒺElectron & Hole Mobilities;[μe;μh;Je;Jh;J;σ];[1_(cm^2/(V*s));1_(cm^2/(V*s));1_(A/m^2);1_(A/m^2);1_(A/m^2);1_(S/m)])'
```

## Fluids

The 30 variables in the Fluids section are:

* `ε`: Roughness (dim.: length)
* `μ`: Dynamic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `ρ`: Density (dim.: mass/volume, in SI: kg/m^3)
* `ΔP`: Pressure change (dim.: force/area, in SI: pascal, Pa)
* `Δy`: Height change
* `ΣK`: Total fitting coefficients
* `A`: Cross-sectional area
* `A1`: Initial cross-sectional area
* `A2`: Final cross-sectional area
* `D`: Diameter
* `D1`: Initial diameter
* `D2`: Final diameter
* `f`: Fanning friction factor
* `h`: Depth relative to P0 reference depth
* `hL`: Head loss (dim.: area/time^2, in SI: m^2/s^2)
* `L`: Length
* `M`: Mass flow rate (dim.: mass/time, in SI: kg/s)
* `n`: Kinematic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `P`: Pressure at `h` (dim.: force/area, in SI: pascal, Pa)
* `P0`: Reference pressure (dim.: force/area, in SI: pascal, Pa)
* `P1`: Initial pressure (dim.: force/area, in SI: pascal, Pa)
* `P2`: Final pressure (dim.: force/area, in SI: pascal, Pa)
* `Q`: Volume flow rate (dim.: volume/time, in SI: m^3/s)
* `Reynolds`: Reynolds number
* `v1`: Initial velocitie
* `v2`: Final velocitie
* `vavg`: Average velocity
* `W`: Power input (dim.: energy/time, in SI: watt, W)
* `y1`: Initial height
* `y2`: Final height

### Pressure At Depth

This equation describes hydrostatic pressure for an incompressible fluid. Depth `h` is positive downwards from the reference.

![Pressure At Depth](img/PressureAtDepth.bmp)

* To calculate `[P_kPa]` (Pressure) from 3 known variables:
```rpl
h=100_m  ρ=1025.1817_kg/m^3  P0=1_atm
@ Expecting [ P=1 106.68481 183 kPa ]
'ROOT(ⒺPressure at Depth;[P];[1_kPa])'
```

### Bernoulli Equation

These equations represent the streamlined flow of an incompressible fluid.

![Bernoulli Equation](img/BernoulliEquation.bmp)

* To calculate `[A1_in^2;ΔP_psi;Δy_ft;Q_ft^3/min;M_lb/min;v2_ft/s;A2_in^2;D2_in]` (Initial cross-sectional area; Pressure change; Height change; Volume & Mass flow rates; Final velocity; Final cross-sectional area; Final diameter) from 7 known variables:
```rpl
P2=25_psi  P1=75_psi  y2=35_ft  y1=0_ft  D1=18_in  ρ=64_lb/ft^3  v1=100_ft/s
@ Expect [ A1=254.46900 4941 in↑2 ΔP=-50. psi Δy=35. ft Q=10 602.87520 59 ft↑3/min M=678 584.01317 5 lb/min v2=122.42131 1569 ft/s A2=207.86332 19 in↑2 D2=16.26836 81217 in ]
'ROOT(ⒺBernoulli Equation;[A1;ΔP;Δy;Q;M;v2;A2;D2];[1_in^2;1_psi;1_ft;1_ft^3/min;1_lb/min;1_ft/s;1_in^2;1_in])'
```

### Flow with Losses

These equations extend Bernoulli’s equation to include power input (or output) and head loss.

![Flow with Losses](img/FlowwithLosses.bmp)

* To calculate `[A1_in^2;ΔP_psi;Δy_ft;Q_ft^3/min;M_lb/min;v2_ft/s;A2_in^2;D2_in]` (Initial cross-sectional area; Pressure change; Height change; Volume & Mass flow rates; Final velocity; Final cross-sectional area; Final diameter) from 9 known variables:
```rpl
P2=30_psi  P1=65_psi  y2=100_ft  y1=0_ft  ρ=64_lb/ft^3  D1=24_in  hL=2.0_(ft/s)^2  W=25_hp  v1=100_ft/s
@ Expecting [ A1=452.38934 2117 in↑2 ΔP=-35 psi Δy=100 ft Q=18 849.55592 15 ft↑3/min M=1 206 371.57898 lb/min v2=93.12684 14502 ft/s A2=485.77760 7264 in↑2 D2=24.86988 66004 in ]
'ROOT(ⒺFlow with Losses;[A1;ΔP;Δy;Q;M;v2;A2;D2];[1_in^2;1_psi;1_ft;1_ft^3/min;1_lb/min;1_ft/s;1_in^2;1_in])'
```

### Flow In Full Pipes

These equations adapt Bernoulli’s equation for flow in a round, full pipe, including power input (or output) and frictional losses (with the Fanning friction factor `f`).

![Flow In Full Pipes](img/FlowInFullPipes.bmp)

* To calculate `[ΔP_Psi;Δy_ft;A_in^2;n_ft^2/s;Q_ft^3/min;M_lb/min;Reynolds;f;W_hp]` (Pressure & Height changes; Cross-sectional area; Kinematic viscosity; Volume & Mass flow rates; Reynolds number; Fanning factor; Power input) from 11 known variables:
```rpl
ρ=62.4_lb/ft^3  D=12_in  vavg=8_ft/s  P2=15_psi  P1=20_psi  y2=40_ft  y1=0_ft  μ=0.00002_lbf*s/ft^2  ΣK=2.25  ϵ=0.02_in  L=250_ft
@ Expect [ ΔP=-5. psi Δy=40. ft A=113.09733 5529 in↑2 n=1.03121 95050 1⁳⁻⁵ ft↑2/s Q=376.99111 8431 ft↑3/min M=23 524.24579 01 lb/min Reynolds=775 780.51628 2 f=4.32637 07915 4⁳⁻³ W=24.95162 12864 hp ]
'ROOT(ⒺFlow In Full Pipes;[ΔP;Δy;A;n;Q;M;Reynolds;f;W];[1_psi;1_ft;1_in^2;1_ft^2/s;1_ft^3/min;1_lb/min;1;1;1_hp])'
```
'Reynolds='(D_m)*(vavg_(m/s))*(ρ_(kg/m^3))/(μ_(kg/(m*s)))'  Reynolds=775 780.51628 23186 30725 06
## Forces and Energy

The 37 variables in the Force and Energy section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity (dim.: angle/time, in SI: r/s))
* `ωi`: Initial angular velocitie (dim.: angle/time, in SI: r/s)
* `ωf`: Final angular velocitie (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `τ`: Torque (dim.: force·length, in SI: N·m)
* `Θ`: Angular displacement (dim.: angle, in SI: r)
* `A`: Projected area relative to flow
* `ar`: Centripetal acceleration at `r`
* `at`: Tangential acceleration at `r`
* `Cd`: Drag coefficient
* `E`: Energy (dim.: force·length, in SI: joule, J)
* `F`: Force at `r` or `x`, or Spring force ([Hooke’s Law](#Hooke’s Law)), or Attractive
force ([Law of Gravitation](#Law of Gravitation)), or Drag force ([Drag force](#Drag force))
* `I`: Moment of inertia (dim.: mass·length^2, in SI: kg·m^2)
* `k`: Spring constant (dim.: force/length, in SI: N/m)
* `Ki`: Initial kinetic energy (dim.: force·length, in SI: joule, J)
* `Kf`: Final kinetic energy (dim.: force·length, in SI: joule, J)
* `m`: Mass
* `m1`: First mass
* `m2`: Second mass
* `N`: Rotational speed (dim.: turn/time, in SI: rpm)
* `Ni`: Initial rotational speed (dim.: turn/time, in SI: rpm)
* `Nf`: Final rotational speed (dim.: turn/time, in SI: rpm)
* `P`: Instantaneous power (dim.: energy/time, in SI: watt, W)
* `Pavg`: Average power (dim.: energy/time, in SI: watt, W)
* `r`: Radius from rotation axis, or Separation distance ([Law of Gravitation](#Law of Gravitation))
* `t`: Time
* `v`: Velocity
* `vf`: Final velocity
* `v1f`: First final velocity
* `v2f`: Second final velocity
* `vi`: Initial velocity
* `v1i`: Initial velocity of mass `m1`
* `W`: Work (dim.: force·length, in SI: joule, J)
* `x`: Displacement
* `UGi`: Initial gravitational potential energy (dim.: force·length, in SI: joule, J)
* `UGf`: Final gravitational potential energy (dim.: force·length, in SI: joule, J)

### Linear Mechanics

* To calculate `[F_lbf;Ki_ft*lbf;vf_ft/s;Kf_ft*lbf;W_ft*lbf;x_ft;Pavg_hp]` (Force at `x`; Initial kinetic energy; Final speed; Final kinetic energy; Work; Average Power) from 4 known variables:
```rpl
t=10_s  m=50_lb  a=12.5_ft/s^2  vi=0_ft/s
@ Expecting [ F=19.42559 38572 lbf Ki=0. ft·lbf vf=125. ft/s Kf=12 140.99616 08 ft·lbf W=12 140.99616 08 ft·lbf x=625. ft Pavg=2.20745 38474 1 hp ]
@ Note Ki is approximately 0
'ROOT(ⒺLinear Mechanics;[F;Ki;vf;Kf;W;x;Pavg];[1_lbf;1_ft*lbf;1_ft/s;1_ft*lbf;1_ft*lbf;1_ft;1_hp])'
```

### Angular Mechanics

* To calculate `[τ_ft*lbf;Ki_ft*lbf;W_ft*lbf;Kf_ft*lbf;at_ft/s^2;Ni_rpm;ωf_r/min;t_min;Nf_rpm;Pavg_hp]` (Torque; Initial kinetic energy; Work; Final kinetic energy; Tangential acceleration; Initial rotational speed; Final angular velocity; Time; Final rotational speed; Average power) from 5 known variables:
```rpl
I=1750_lb*in^2  θ=360_°  r=3.5_in  α=10.5_r/min^2  ωi=0_r/s
@ Expecting [ τ=1.10168 29849 6⁳⁻³ ft·lbf Ki=0. ft·lbf W=6.92207 83442 6⁳⁻³ ft·lbf Kf=6.92207 83442 6⁳⁻³ ft·lbf at=8.50694 44444 4⁳⁻⁴ ft/s↑2 Ni=0. rpm ωf=11.48681 38076 r/min t=1.09398 22673 9 min Nf=0.29096 43928 17 rpm Pavg=1.91739 80792 8⁳⁻⁷ hp ]
'ROOT(ⒺAngular Mechanics;[τ;Ki;W;Kf;at;Ni;ωf;t;Nf;Pavg];[1_ft*lbf;1_ft*lbf;1_ft*lbf;1_ft*lbf;1_ft/s^2;1_rpm;1_r/min;1_min;1_rpm;1_hp])'
```

### Centripetal Force

* To calculate `[ω_r/s;F_N;v_m/s;ar_m/s]` (Angular velocity; Force; Velocity; Centripetal accélération at `r`) from 3 known variables:
```rpl
m=1_kg  r=5_cm  N=2000_Hz
@ Expecting [ ω=12 566.37061 44 r/s F=7 895 683.52087 N v=628.31853 0718 m/s ar=7 895 683.52087 m/s↑2 ]
'ROOT(ⒺCentripetal Force;[ω;F;v;ar];[1_r/s;1_N;1_m/s;1_m/s^2])'
```

### Hooke’s Law

The force is that exerted by the spring.

![Hooke’s Law](img/Missing name.bmp)

* To calculate `[F_lbf;W_ft*lbf]` (Force; Work) from 2 known variables:
```rpl
k=1725_lbf/in  x=1.25_in
@ Expecting [ F=-2 156.25 lbf W=-112.30468 75 ft·lbf ]
'ROOT(ⒺHooke’s Law;[F;W];[1_lbf;1_ft*lbf])'
```

### 1D Elastic Collisions

![1D Elastic Collisions](img/1DElasticCollisions.bmp)

* To calculate `[v1f_m/s;v2f_m/s]` (Final velocities of mass `m1` & `m2`) from 3 known variables:
```rpl
m1=10_kg  m2=25_kg  v1i=100_m/s
@ Expecting [ v1f=-42.85714 28571 m/s v2f=57.14285 71429 m/s ]
'ROOT(Ⓔ1D Elastic Collisions;[v1f;v2f];[1_m/s;1_m/s])'
```

### Drag Force

* To calculate `[F_N]` (Drag force) from 4 known variables:
```rpl
Cd=0.05  ρ=1000_kg/m^3  A=7.5E6_cm^2  v=35_m/s
@ Expecting [ F=22 968 750. N ]
'ROOT(ⒺDrag Force;[F];[1_N])'
```

### Gravitation Law

* To calculate `[F_N;UGf_J;UGi_J;W_J]` (Gravitational force; Work; Final & Initial potential energy) from 5 known variables:
```rpl
m1=2E15_kg  m2=2E18_kg  r=1000000_km  ri=1000000_km  rf=5000000_km
@ Expecting [ F=266 972. N UGf=-5.33944⁳¹³ J UGi=-2.66972⁳¹⁴ J W=2.13577 6⁳¹⁴ J ]
'ROOT(ⒺGravitation Law;[F;UGf;UGi;W];[1_N;1_J;1_J;1_J])'
```

### Relativity Mass Energy

* To calculate `[E_J]` (Relativistic energy) from 1 known variable:
```rpl
m=9.1E-31_kg
@ Expecting [ E=8.17867 21265 1⁳⁻¹⁴ J ]
'ROOT(ⒺRelativity Mass Energy;[E];[1_J])'
```

## Gases

The 38 variables in the Gases section are:

* `λ`: Mean free path (dim.: length)
* `ρ`: Flow density (dim.: mass/volume, in SI: kg/m^3)
* `ρr`: Reduced state factor
* `ρri`: Initial reduced state factor
* `ρrf`: Final reduced state factor
* `ρ0`: Stagnation density (dim.: mass/volume, in SI: kg/m^3)
* `A`: Flow area
* `At`: Throat area
* `d`: Molecular diameter
* `k`: Specific heat ratio
* `M`: Mach number
* `m`: Mass
* `MW`: Molecular weight (dim.: mass/mole, in SI: g/gmol)
* `n`: Number of moles, or Polytropic constant ([Polytropic Processes](#Polytropic Processes))
* `P`: Pressure, or Flow pressure ([Isentropic Flow](#Isentropic Flow)) (dim.: force/area, in SI: pascal, Pa)
* `P0`: Stagnation pressure (dim.: force/area, in SI: pascal, Pa)
* `Pc`: Pseudocritical pressure (dim.: force/area, in SI: pascal, Pa)
* `Pin`: Initial pressure (dim.: force/area, in SI: pascal, Pa)
* `Pf`: Final pressure (dim.: force/area, in SI: pascal, Pa)
* `Pr`: Reduced pressure
* `Pri`: Initial reduced ressure
* `Prf`: Final reduced pressure
* `T`: Temperature, or Flow temperature ([Isentropic Flow](#Isentropic Flow))
* `T0`: Stagnation temperature
* `Tc`: Pseudocritical temperature
* `Tr`: Reduced temperature
* `Tri`: Initial reduced temperature
* `trf`: Final reduced temperature
* `Ti`: Initial temperature
* `Tf`: Final temperature
* `V`: Volume
* `Vi`: Initial volume
* `Vf`: Final volume
* `vrms`: Root-mean-square (rms) velocity
* `W`: Work (dim.: force·length, in SI: joule, J)
* `Z`: Gas compressibility correction factor
* `Zi`: Initial gas compressibility correction factor
* `Zf`: Final gas compressibility correction factor

### Ideal Gas

* To calculate `[n_mol;m_kg]` (Number of moles; Mass) from 4 known variables:
```rpl
T=16.85_°C  P=1_atm  V=25_l  MW=36_g/mol
@ Expecting [ n=1.05056 86529 9 mol m=3.78204 71507 7⁳⁻² kg ]
'ROOT(ⒺIdeal Gas;[n;m];[1_mol;1_kg])'
```

### Ideal Gas Law Change

* To calculate `[Vf_l]` (Volume final) from 5 known variables:
```rpl
Pin=1.5_kPa  Pf=1.5_kPa  Vi=2_l  Ti=100_°C  Tf=373.15_K
@ Expecting [ Vf=2. l ]
'ROOT(ⒺIdeal Gas Law Change;[Vf];[1_l])'
```

### Isothermal Expansion

These equations apply to an ideal gas.

* To calculate `[m_kg;W_J]` (Mass; Work) from 5 known variables:
```rpl
Vi=2_l  Vf=125_l  T=300_°C  n=0.25_mol  MW=64_g/mol
@ Expecting [ m=0.016 kg W=4 926.46608 432 J ]
'ROOT(ⒺIsothermal Expansion;[m;W];[1_kg;1_J])'
```

### Polytropic Processes

These equations describe a reversible pressure-volume change of an ideal gas such that `P·Vn` is constant. Special cases include isothermal processes (`n = 1`), isentropic processes (`n = k`, the specific heat ratio), and constant-pressure processes (`n = 0`).

* To calculate `[n_1;Tf_°F]` (Polytropic number; Final temperature) from 5 known variables:
```rpl
Pin=15_psi  Pf=35_psi  Vi=1_ft^3  Vf=0.50_ft^3  Ti=75_°F
@ Expecting [ n=1.22239 24213 4 Tf=303.98333 3333 K ]
'ROOT(ⒺPolytropic Processes;[n;Tf];[1;1_K])'
```

### Isentropic Flow

![Isentropic Flow](img/IsentropicFlow.bmp)

The calculation differs at velocities below and above Mach 1. The Mach number is based on the speed of sound in the compressible fluid.

* To calculate `[P_kPa;ρ_kg/m^3;At_cm^2]` (Flow pressure; Flow density; Throat area) from 7 known variables:
```rpl
k=2  M=0.9  T0=26.85_°C  T=373.15_K  ρ0=100_kg/m^3  P0=100_kPa  A=1_cm^2
@ Expecting [ P=154.71213 6111 kPa ρ=124.38333 3333 kg/m↑3 At=0.99280 71853 34 cm↑2 ]
'ROOT(ⒺIsentropic Flow;[P;ρ;At];[1_kPa;1_kg/m^3;1_cm^2])'
```

### Real Gas Law

These equations adapt the ideal gas law to emulate real-gas behavior.

* To calculate `[Z_1;n_mol;m_kg]` (Gas compressibility correction factor; Number of mole; Mass) from 7 known variables:
```rpl
Pc=48_atm  Tc=298_K  P=5_kPa  V=10_l  MW=64_g/mol  T=348.15_K
@ Expecting [ Z=0.99977 57972 69 n=1.72769 39032 1⁳⁻² mol m=1.10572 40980 5⁳⁻³ kg ]
'ROOT(ⒺReal Gas Law;[Z;n;m];[1;1_mol;1_kg])'
```

### Real Gas State Change

This equation adapts the ideal gas state-change equation to emulate real-gas behavior.

* To calculate `[Zi_1;Zf_1;Vf_l]` (Initial & Final gas compressibility correction factor; Final volume) from 7 known variables:
```rpl
Pc=48_atm  Pin=100_kPa  Pf=50_kPa  Ti=348.15_K  Tc=298_K  Vi=10_l  Tf=523.15_K
@ Expecting [ Zi=0.99550 62096 36 Zf=0.99938 68303 14 Vf=30.17028 92973 l ]
'ROOT(ⒺReal Gas State Change;[Zi;Zf;Vf];[1;1;1_l])'
```

### Kinetic Theory

These equations describe properties of an ideal gas.

* To calculate `[vrms_m/s;n_mol;m_kg;λ_nm]` (Root-mean-square velocity; Number of mole; Mean free path) from 7 known variables:
```rpl
P=100_kPa  V=2_l  T=300_K  MW=18_g/mol  d=2.5_nm
@ Expecting [ vrms=644.76595 0487 m/s n=8.01815 70028 5⁳⁻² mol m=1.44326 82605 1⁳⁻³ kg λ=1.49162 49859 nm ]
'ROOT(ⒺKinetic Theory;[vrms;n;m;λ];[1_m/s;1_mol;1_kg;1_nm])'
```


## Heat transfer

The 31 variables in the Heat Transfer section are:

* `α`: Expansion coefficient (dim.: 1/temperature, in SI: K^-1)
* `δ`: Elongation (dim.: length)
* `λ1`: Lower wavelength limits (dim.: length)
* `λ2`: Upper wavelength limits (dim.: length)
* `λmax`: Wavelength of maximum emissive power (dim.: length)
* `ΔT`: Temperature difference
* `A`: Area
* `c`: Specific heat (dim.: energy/(mass·temperature), in SI: J/(kg·K))
* `eb12`: Emissive power in the range λ1 to λ2 (dim.: power/area, in SI: W/m^2)
* `eb`: Total emissive power (dim.: power/area, in SI: W/m^2)
* `f`: Fraction of emissive power in the range λ1 to λ2
* `h`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `h1`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `h3`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `k`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k1`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k2`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k3`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `L`: Length
* `L1`: Length
* `L2`: Length
* `L3`: Length
* `m`: Mass
* `Q`: Heat capacity (dim.: force·length, in SI: joule, J)
* `qr`: Heat transfer rate (dim.: power=energy/time, in SI: watt, W)
* `T`: Temperature
* `Tc`: Cold surface temperature ([Conduction^](#Conduction)), or Cold fluid temperature ([Convection](#Convection))
* `Th`: Hot surface temperature, or Hot fluid temperature ([Conduction + Convection](#Conduction + Convection))
* `Ti`: Initial temperature
* `Tf`: Final temperature
* `U`: Overall heat transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))

### Heat Capacity

* To calculate `[Tf_°C;c_kJ/(kg*K)]` (Final temperature; Specific heat) from 4 known variables:
```rpl
ΔT=15_°C  Ti=0_°C  m=10_kg  Q=25_kJ
@ Expecting [ Tf=15 °C c=0.16666 66666 67 kJ/(kg·K) ]
'ROOT(ⒺHeat Capacity;[Tf;c];[1_°C;1_kJ/(kg*K)])'
```

### Thermal Expansion

![Thermal Expansion](img/ThermalExpansion.bmp)

* To calculate `[α_K^-1;Ti_°C]` (Expansion coefficient; Initial temperature) from 4 known variables:
```rpl
ΔT=15_°C  L=10_m  Tf=25_°C  δ=1_cm
@ Expecting [ α=6.66666 66666 7⁳⁻⁵ K⁻¹ Ti=10 °C ]
'ROOT(ⒺThermal Expansion;[α;Ti];[1_K^-1;1_°C])'
```

### Conduction

![Conduction](img/Conduction.bmp)

* To calculate `[qr_W;ΔT_°C]` (Heat transfer rate; Temperature difference) from 5 known variables:
```rpl
Tc=25_°C  Th=75_°C  A=12.5_m^2  L=1.5_cm  k=0.12_W/(m*K)
@ Expecting [ qr=5 000. W ΔT=50 °C ]
'ROOT(ⒺConduction;[qr;ΔT];[1_W;1_°C])'
```

### Convection

![Convection](img/Convection.bmp)

* To calculate `[Th_°C;ΔT_°C]` (Hot surface temperature; Temperature difference) from 4 known variables:
```rpl
Tc=26.85_°C  A=200_m^2  h=0.005_W/(m^2*K)  qr=10_W
@ Expecting [ Th=36.85 °C ΔT=10. °C ]
'ROOT(ⒺConvection;[Th;ΔT];[1_°C;1_°C])'
```

### Conduction & Convection

If you have fewer than three layers, give the extra layers a zero thickness and any nonzero conductivity. The two temperatures are fluid temperatures – if instead you know a surface temperature, set the corresponding convective coefficient to 10^999999.

![Conduction + Convection](img/Conduction+Convection.bmp)

* To calculate `[Tc_°C;qr_W;U_W/(m^2*K)]` (Cold surface temperature; Heat transfer rate; Overall heat transfer coefficient) from 11 known variables:
```rpl
ΔT=35_°C  Th=55_°C  A=10_m^2  h1=0.05_W/(m^2*K)  h3=0.05_W/(m^2*K)  L1=3_cm  L2=5_cm  L3=3_cm  k1=0.1_W/(m*K)  k2=.5_W/(m*K)  k3=0.1_W/(m*K)
@ Expecting [ qr=8.59950 85995 1 W Tc=20 °C U=0.02457 00245 7 W/(m↑2·K) ]
'ROOT(ⒺConduction & Convection;[qr;Tc;U];[1_W;1_°C;1_W/(m^2*K)])'
```

### Black Body Radiation

F0λ(λ_m, T_K) is the black body emissive power Function which returns the fraction of total black-body emissive power at temperature `T_K` between wavelengths 0 and `λ_m`. It is the integral of the Planck distribution.

![Black Body Radiation](img/BlackBodyRadiation.bmp)

* To calculate `[λmax_nm;eb_W/m^2;f_1;eb12_W/m^2;q_W]` (Wavelength of maximal emission; Total emissive power; Fraction of emissive power between λ1 & λ2; Emissive power between λ1 & λ2; Heat transfer rate) from 4 known variables:
```rpl
T=1273,15_K  Tmax=1273,15_K  λ1=1000_nm  λ2=600_nm  A=1_cm^2
@ Expecting [ λmax=2 276.06484 325 nm eb=148 980.70811 W/m↑2 f=3.60911 40468 1⁳⁻³ eb12=537.68836 6343 W/m↑2 q=14.89807 0811 W ]
'ROOT(ⒺBlack Body Radiation;[λmax;eb;f;eb12;q];[1_nm;1_W/m^2;1;1_W/m^2;1_W])'
```


## Magnetism

The 28 variables in the Magnetism section are:

* `α1`: Subtended internal left angle relative to the top ends of the solenoid
* `α2`: Subtended internal right angle relative to the top ends of the solenoid
* `θ₁`: Subtended left angle relative to the top ends of the solenoid
* `θ₂`: Subtended right angle relative to the ends of the wire
* `θ`: Angle between the line of the magnetic field and the speed of the moving charge
* `μr`: Relative permeability
* `B`: Magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `d`: Separation distance
* `Dpitch`: Pitch of the helicoidal motion (dim.: length)
* `Fba`: Force
* `fc`: Cyclotron frequency (dim.: time^-1, in SI: hertx, Hz)
* `I`: Current (dim.: charge/time, in SI: ampere, A)
* `Ia`: Current (dim.: charge/time, in SI: ampere, A)
* `Ib`: Current (dim.: charge/time, in SI: ampere, A)
* `L`: Length
* `m`: Mass
* `N`: Total number of turns
* `nl`: Number of turns per unit length (dim.: length^-1)
* `q`: Charge of the moving charge (dim.: charge, in SI: coulomb, C)
* `r`: Distance from center of wire
* `Rc`: Radius of the circular part of the motion
* `ri`: Inside radius of toroid
* `ro`: Outside radius of toroid
* `rw`: Radius of wire
* `uB`: Volumic density of magnetic energy (dim.: energy/volume, in SI: J/m^3)
* `v`: Speed of the moving charge
* `T`: Period (dim.: time)
* `VH`: Hall tension (dim.: energy/charge, in SI: volt, V)

#### Straight Wire Infinite

The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly. The expression for the magnetic field at the distance `r` is approximately valid if the distance is such that `r << L` and therefore also applies for a wire of finite length `L` providing that `r < L/10` (see the following example 2 compared to example 2 of "Straight Wire Finite"). Note that if an electric current passes through a straight wire, one must use the following right-hand rule te determine the direction of the `B` field: when the thumb is pointed in the direction of conventional current (from positive to negative), the curled fingers will then point in the direction of the magnetic field (see fig.).


![Straight Wire Infinite](img/StraightWireInfinite.bmp)

* **Example 1.** Inside the wire, to calculate `[B_T]` (Magnetic field) from 4 known variables:
```rpl
μr=1  rw=0.25_cm  r=0.2_cm  I=25_A
@ Expecting [ B=1.59999 99997 9⁳⁻³ T ]
'ROOT(ⒺStraight Wire Infinite;[B];[1_T])'
```

* **Example 2.** Outside the wire, to calculate `[B_T]` (Magnetic field) from 4 known variables:
```rpl
μr=1  rw=0.25_cm  r=5_cm  I=25_A
@ Expecting [ B=9.99999 99986 8⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Infinite;[B];[1_T])'
@ Save B for later
```

The code below saves the reference value for comparison with the example 2 in [B Field Finite Wire](#B Field Finite Wire):
```rpl
@ Save the reference value for comparison below
B0=B
```

#### Straight Wire Finite

The expression for the magnetic field at the distance `r` depends on the subtended angles `θ₁` and `θ₂` relative to the ends of the wire of finite length `L`. The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly.

![B Field From Finite Wire](img/B_Field_From_Finite_Wire.bmp)

* **Example 1.** To calculate `[B_T]` (Magnetic field) from 6 known variables:
```rpl
μr=1_1  rw=0.25_cm  r=5_cm  I=25_A  θ₁=30_°  θ₂=150_°
@ Expecting [ B=8.66025 40367⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Finite;[B];[1_T])'
@ Save for test below
```

* **Example 2.** When 'r << L' which means 'r < L/10', we can verify that the value of B for a infinite wire approximates the exact value calculated for a finite wire of length 'L'.
```rpl
L=3_m  μr=1  rw=0.25_cm  r=5_cm  I=25_A  θ₁='atan(r/L/2)'  θ₂='180_°-θ₁'
if 'r < L/10' then
@ Expecting [ B=9.99965 27945 4⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Finite;[B];[1_T])'
end
@ Save for test below
```
Verify relative difference under condition `5_cm << 3_m` with the example 2 of [Straight Wire Infinite](#Straight Wire Infinite)
```rpl
B0 B %Ch
@ Expecting -3.47204 13877 8⁳⁻³
@ % of relative difference
```

#### Force Between Wires

The force between wires is positive for an attractive force (for currents having the same sign) and negative otherwise, corresponding to a repulsive force.

![Force Between Wires](img/ForceBetweenWires.bmp)

* To calculate `[Fba_N]` (Magnetic force) from 4 known variables:
```rpl
Ia=10_A  Ib=20_A  μr=1  L=50_cm  d=1_cm
@ Expecting [ Fba=1.99999 99997 4⁳⁻³ N ]
'ROOT(ⒺForce Between Wires;[Fba];[1_N])'
```

#### B Field In Infinite Solenoid

The expression for the magnetic field in the center is approximately valid if the radius of the solenoid < `L` and therefore also applies inside a solenoid of finite length `L`. The right-hand rule applies also here: when the fingers curl around the solenoid in the sense of the current, the thumb points in the direction of the magnetic field (see fig.).

![B Field In Infinite Solenoid](img/BFieldInInfiniteSolenoid.bmp)

* To calculate `[B_T]` (Magnetic field) from 3 known variables:
```rpl
μr=10  nl=5000_m^-1  I=1.25_A
@ Expecting [ B=0.07853 98163 29 T ]
'ROOT(ⒺB Field In Infinite Solenoid;[B];[1_T])'
@ Save B for later use
```
The code below saves the reference value for comparison with the example 2 in [B Field Finite Solenoid](#B Field Finite Solenoid):
```rpl
@ Save the reference value for comparison below
B0=B
@ Save B0 for later use
```

#### B Field In Finite Solenoid

The expression for the magnetic field in the center depends on the subtended internal angles `α1` and `α2` relative to the top ends of the solenoid of finite length `L`.

![B Field In Finite Solenoid](img/B_Field_In_Finite_Solenoid.bmp)

* **Example 1.** Inside the wire, to calculate `[B_T]` (Magnetic field) from 5 known variables:
```rpl
μr=10  nl=5_mm^-1  I=1.25_A  α1=150_°  α2=30_°
@ Expecting [ B=0.06801 74761 5 T ]
'ROOT(ⒺB Field In Finite Solenoid;[B];[1_T])'
@ Save variables for later use
```

* **Example 2.** When `r << L` which means `r < L/10`, we can verify that the value of `B` for a infinite solenoid approximates the exact value calculated for a finite solenoid of length `L`.
```rpl
L=3_m  μr=10  r=10_cm  nl=5000_m^-1  I=1.25_A  α2='atan(r/L/2)'  α1='180_°-α2'
if 'r < L/10' then
@ Expecting [ B=0.07852 89102 94 T ]
'ROOT(ⒺB Field In Finite Solenoid;[B];[1_T])'
end
@ Save variables for later use
```
Verify relative difference under condition 10_cm << 3_m with the example of [B Field In Infinite Solenoid](#B Field In Infinite Solenoid)
```rpl
B0 B %Ch
@ Expecting -0.01388 59960 4
@ % of relative difference
```

#### B Field In Toroid

The magnetic field `B` is calculated in the center of the torroid. The right-hand rule applies also here: when the fingers curl around the outer circle of the torroid following the sense of the current, the thumb points in the direction of the central magnetic field (see fig.).

![B Field In Toroid](img/BFieldInToroid.bmp)

* To calculate `[B_T]` (Magnetic field) from 5 known variables:
```rpl
μr=10  N=50  ri=5_cm  ro=7_cm  I=10_A
@ Expecting [ B=1.66666 66664 5⁳⁻² T ]
'ROOT(ⒺB Field In Toroid;[B];[1_T])'
```

#### Hall Effect

When a conductor carrying a current (to the right) is suddenly plunged into a perpendicular magnetic field (towards the bottom), there is a deviation (towards the top) of the free electrons which unbalances the distribution of electric charges. This transfer of charges from top to bottom gives rise to an electric force which in return balances the magnetic force. And it is precisely the appearance of this new electric field which is revealed by the Hall voltage `VH` measured between the top and the bottom of the conductor (small multimeter on the right).

![Hall Effect](img/Hall Effect VH.bmp)

* To calculate `[VH_V]` (Hall tension) from 5 known variables:
```rpl
n=5e28_(1/m^3)  B=0.641_T  q=1.60217 6634e-19_C  L=2_mm  I=10_A
@ Expecting [ VH=4.00080 73167 3⁳⁻⁷ V ]
'ROOT(ⒺHall Effect;[VH];[1_V])'
```

#### Cyclotron Motion

Under the perpendicular magnetic field, the moving charge has a circular trajectory and turns at the cyclotron frequency with the rotation period `T`.

![Cyclotron Motion](img/Cyclotron_BW.bmp)

* To calculate `[Rc_m;fc_Hz;T_s]` (Radius of the circular path; Cyclotron frequency; Period) from 4 known variables:
```rpl
m=1.67262 19259 5e-27_kg  B=0.8_T  q=1.60217 6634e-19_C  v=4.6e7_m/s
@ Expecting [ Rc=0.60028 18834 15 m fc=12 196 149.1501 Hz T=8.19930 93696 5⁳⁻⁸ s ]
'ROOT(ⒺCyclotron Motion;[Rc;fc;T];[1_m;1_Hz;1_s])'
```

#### Helicoidal Motion

Under the magnetic field lines (at angle `θ` with the speed vector), the moving charge has an helicoidal trajectory of pitch `Dpitch`, radius `Rc` and period `T`.

![Helicoidal Motion](img/Helicoidal Motion BW.bmp)

* To calculate `[Rc_m;T_s;Dpitch_m]` (Radius of the circular path; Period, pitch of the helicoidal motion) from 4 known variables:
```rpl
m=1.67262 19259 5e-27_kg  B=0.8_T  q=1.60217 6634e-19_C  v=4.6e7_m/s  θ=30_°
@ Expecting [ Rc=0.30014 09417 08 m T=8.19930 93696 5⁳⁻⁸ s Dpitch=3.26637 26955 m ]
'ROOT(ⒺHelicoidal Motion;[Rc;T;Dpitch];[1_m;1_s;1_m])'
```

#### Volumic Density Magnetic Energy

* To calculate `[uB_(J/m^3)]` (Volumic density of magnetic energy) from 2 known variables:
```rpl
μr=3.0  B=0.8_T
@ Expecting [ uB=84 882.63632 69 J/m↑3 ]
'ROOT(ⒺVolumic Density Magnetic Energy;[uB];[1_(J/m^3)])'
```


## Motion

The 38 variables in the Motion section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity ([Circular Motion](#Circular Motion)), or Angular velocity at `t` ([Angular Motion](#Angular Motion)) (dim.: angle/time, in SI: r/s)
* `ω₀`: Initial angular velocity (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `θ`: Angular position at `t` (dim.: angle)
* `θ0`: Initial angular position ([Angular Motion](#Angular Motion)), or Initial vertical angle ([Projectile Motion](#Projectile Motion))
* `φ`: Latitude (dim.: angle)
* `a`: Acceleration
* `Ah`: Projected horizontal area
* `ar`: Centripetal acceleration at `r`
* `Cd`: Drag coefficient
* `fr`: Fraction of the terminal velocity `vt`
* `gloc`: local gravitational acceleration of a planet or star (dim.: length/time^2)
* `gearth`: local gravitational acceleration on Earth (dim.: length/time^2)
* `h`: altitude (dim.: length)
* `hmax`: maximum height of the projectile Motion
* `m`: Mass
* `Mp`: Planet or star mass
* `N`: Rotational speed (dim.: time^-1, in SI: hertz, Hz)
* `R`: Horizontal range ([Projectile Motion](#Projectile Motion)), or Planet or Star radius ([Object in Free Fall](#Object in Free Fall)) & ([Escape and Orbital Velocities](#Escape and Orbital Velocities))
* `r`: Radius
* `rc`: Radius of circular motion
* `t`: Time
* `tf`: Time of flight of a projectile
* `tfr`: time required to reach the fraction `fr` of the terminal velocity
* `v`: Velocity at `t` ([linear Motion](#linear Motion)), or Tangential velocity at `r` ([Circular Motion](#Circular Motion)), or Falling velocity at time `t` ([Terminal Velocity](#Terminal Velocity)) & ([Buoyancy & Terminal Velocity](#Buoyancy & Terminal Velocity))
* `v0`: Initial velocity
* `ve`: Escape velocity in a gravitational field
* `vcx`: Horizontal (component x) of velocity at `t`
* `vcy`: Vertical (component y) of velocity at `t`
* `vo`: Orbital velocity in a gravitational field
* `Vol`: Volume of the moving object
* `vt`: Terminal velocity reached in a vertical fall
* `x`: Horizontal position at `t`
* `x0`: Initial horizontal position
* `xfr`: Displacement during `tfr`
* `y`: Vertical position at `t`
* `y0`: Initial vertical position

#### Linear Motion

* To calculate `[a_m/s^2;v_m/s]` (Acceleration; Velocity at time `t`) from 4 known variables:
```rpl
x0=0_m  x=100_m  t=10_s  v0=1_m/s
@ Expecting [ a=1.8 m/s↑2 v=19. m/s ]
'ROOT(ⒺLinear Motion;[a;v];[1_m/s^2;1_m/s])'
```

#### Object In Free Fall

By definition, an object in free fall only experiences local gravitational acceleration `gloc`. This depends on the mass of the star or planet and the distance `r` center to center (where we assume that the position is greater than the radius of the mass). For the Earth, we can calculate an approximate value `gearth` of the acceleration of gravity as a function of latitude `φ` and for an altitude `h` low compared to the Earth's radius (typically: a few thousand meters, valid in commercial aviation).

* To calculate `[t_s;v_ft/s;r_m;gearth_m/s^2]` (Time; Velocity at time `t`; Radius; Acceleration of gravity at latitude `φ` & altitude `h`) from 7 known variables:
```rpl
y0=1000_ft  y=0_ft  v0=0_ft/s  gloc=9.80665_m/s↑2  φ=45_°  h=1000_m  Mp=5.9722e24_kg
@ Expecting [ t=7.88428 18533 5 s v=-253.66926 7182 ft/s r=6 375 433.14029 m gearth=9.80321 00310 8 m/s↑2 ]
'ROOT(ⒺObject In Free Fall;[t;v;r;gearth];[1_s;1_ft/s;1_m;1_m/s^2])'
```

#### Projectile Motion

During the time of flight `tf`, the motion of a projectile follows a symetric parabole of horizontal range `R` and of maximum height `hmax`.

![Projectile Motion](img/Projectile_BW.bmp)

* To calculate `[R_ft;vcx_ft/s;vcy_ft/s;x_ft;y_ft;hmax_ft;tf_s]` (Range, Components `x` & `y` of velocity at time `t`; Positions `x` & `y` at time `t`; Maximum height; Time of flight) from 5 known variables:
```rpl
x0=0_ft  y0=0_ft  θ0=45_°  v0=200_ft/s  t=10_s
@ Expecting [ R=1 243.23800 686 ft vcx=141.42135 6237 ft/s vcy=-180.31912 9327 ft/s x=1 414.21356 237 ft y=-194.48886 5448 ft hmax=310.80950 1716 ft tf=8.79102 02528 1 s ]
'ROOT(ⒺProjectile Motion;[R;vcx;vcy;x;y;hmax;tf];[1_ft;1_ft/s;1_ft/s;1_ft;1_ft;1_ft;1_s])'
```

#### Angular Motion

* To calculate `[ω_r/min;Θ_°]` (Angular velocity at time `t`; Angular position at time `t`) from 4 known variables:
```rpl
θ0=0_°  ω₀=0_r/min  α=1.5_r/min^2  t=30_s
@ Expecting [ ω=0.75 r/min θ=10.74295 86587 ° ]
'ROOT(ⒺAngular Motion;[ω;θ];[1_r/min;1_°])'
```

#### Uniform Circular Motion

* To calculate `[ω_r/min;ar_ft/s^2;N_rpm]` (Angular velocity; Centripetal acceleration at `r`; Rotational speed) from 2 known variables:
```rpl
rc=25_in  v=2500_ft/s
@ Expecting [ ω=72 000. r/min ar=3 000 000. ft/s↑2 N=11 459.15590 26 rpm ]
'ROOT(ⒺUniform Circular Motion;[ω;ar;N];[1_r/min;1_ft/s^2;1_rpm])'
```

#### Terminal Velocity

Terminal velocity is the maximum speed attainable by an object as it falls through a fluid like air for instance. It is reached when the sum of the increasing drag force is equal to the downward force of gravity acting on the object (neglecting buoyancy), leading to a zero net force at the resulting terminal velocity.

* **Example 1**. For a falling big mass, to calculate `[vt_ft/s;v_ft/s;tfr_s;xfr_ft]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 6 known variables:
```rpl
Cd=0.15  ρ=0.025_lb/ft^3  Ah=100000_in^2  m=1250_lb  t=5_s  fr=0.95
@ Expecting [ vt=175.74722 3631 ft/s v=127.18655 2185 ft/s tfr=10.00590 25332 s xfr=1 117.39339 247 ft ]
'ROOT(ⒺTerminal Velocity;[vt;v;tfr;xfr];[1_ft/s;1_ft/s;1_s;1_ft])'
```

* **Example 2**. For a human skydiving head first, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 6 known variables:
```rpl
Cd=0.7  ρ=1.29_kg/m^3  Ah=0.18_m^2  m=75_kg  t=5_s  fr=0.95
@ Expecting [ vt=312.11229 4878 ft/s v=147.99138 9715 ft/s tfr=17.76964 17471 s xfr=3 524.12177 429 ft ]
'ROOT(ⒺTerminal Velocity;[vt;v;tfr;xfr];[1_ft/s;1_ft/s;1_s;1_ft])'
```

#### Buoyancy & Terminal Velocity

Terminal velocity is the maximum speed attainable by an object as it falls through a fluid like air for instance. It is reached when the sum of the increasing drag force plus the buoyancy is equal to the downward force of gravity acting on the object, leading to a zero net force at the resulting terminal velocity. If the terminal velocity is found to be negative, the motion is upward because buoyancy dominates gravity (see example 2).

* **Example 1**. For a golf ball falling in water, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 8 known variables:
```rpl
Cd=0.5  ρ=1077,5_(kg/m^3)  ρf=1000_(kg/m^3)  d=4.282_cm  Ah=14.40068 68745_cm↑2  Vol=41.10916 07978_cm↑3  t=3e-2_s  fr=0.95
@ Expecting [ vt=0.29459 06011 51 m/s v=0.22419 40616 41 m/s tfr=0.05502 64783 43 s xfr=1.03003 49562 7⁳⁻² m ]
'ROOT(ⒺBuoyancy & Terminal Velocity;[vt;v;tfr;xfr];[1_m/s;1_m/s;1_s;1_m])'
```

* **Example 2**. For a CO2 bubble in a glass of champagne, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 8 known variables:
```rpl
Cd=0.01  ρ=1.98_(kg/m^3)  ρf=998_(kg/m^3)  d=0.1_cm  Ah=7.85398 16339 7e-3_cm↑2  Vol=5.23598 77559 8e-4_cm↑3  t=0.1_s  fr=0.95
@ Expecting [ vt=-1.14234 81034 5 m/s v=-0.79446 37698 68 m/s tfr=0.21337 88142 9 s xfr=-0.15488 56277 51 m ]
'ROOT(ⒺBuoyancy & Terminal Velocity;[vt;v;tfr;xfr];[1_m/s;1_m/s;1_s;1_m])'
```

#### Escape and Orbital Velocities

The escape velocity is the speed required to completely free oneself from the gravitational field of a star, planet, etc. It is defined as the initial speed allowing you to move away to infinity. The orbital velocity is the speed nneded to maintain a stable circular orbit in a gravitational field.

* To calculate `[ve_m/s;vo_m/s]` (Escape velocity; Orbital velocity) from 2 known variables:
```rpl
Mp=1.5E23_lb  R=5000_mi
@ Expecting [ ve=3 485.55509 628 ft/s vo=2 464.65964 478 ft/s ]
'ROOT(ⒺEscape & Orbital Velocity;[ve;vo];[1_ft/s;1_ft/s])'
```


## Optics

The 40 variables in the Optics section are:

* `Δα`: Phase difference between top and bottom rays of light from a single slit (dim.: angle, in SI: r)
* `Δφ`: Phase difference between two rays of light separated by an angle θ (dim.: angle, in SI: r)
* `Δyint`: Distance between bright fringes on the interference screen
* `Δydiff`: Width of the central maximum of diffraction
* `λ`: Light wavelength
* `θ`: Angle between initial light polarisation direction and polarizer transmission axis ([Malus Law](#Malus Law)), or Angle subtended by two points separated by y on the screen and the middle of one slit ([2 Slits Young Interference](#2 Slits Young Interference)), or two slits ([One Slit Diffraction](#One Slit Diffraction))
* `θ0`: Acceptance angle to enter an optical fiber in the outer medium of refraction index `n0f`
* `θ₁`: Angle of incidence in the medium of refraction index n1
* `θ₂`: Angle of refraction in the medium of refraction index n2
* `θB`: Brewster angle
* `θc`: Critical angle
* `θr`: Rayleigh's criterion limiting
* `a`: Width of a diffraction slit
* `d`: Distance between two interference slits
* `E₀`: Incident light electrtc field
* `f`: Focal length
* `fx₀`: Frequency of the incident X-ray (dim.: time^-1, in SI: hertz, Hz)
* `fx`: Frequency of the transmitted X-ray (dim.: time^-1, in SI: hertz, Hz)
* `I`: Transmitted irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `I₀`: Incident irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix`: Transmitted irradiance or polarized X rays radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix0`: Incident irradiance or polarized X rayx radiance flux (dim.: power/area, in SI: W/m^2)
* `m`: Magnification
* `n`: Index of refraction
* `n1`: Index of refraction of medium 1
* `n2`: Index of refraction of medium 2
* `nf0`: Index of refraction of the outer medium at the entry of an optic fiber
* `nf1`: Index of refraction of the core medum of an optic fiber
* `nf2`: Index of refraction of the cladding medium of an optic fiber
* `r`: Radius of curvature
* `r1`: Radius of curvature
* `r2`: Radius of curvature
* `u`: Distance to object
* `v`: Distance to image
* `v1`: Speed of light in medium of refraction index n1
* `v2`: Speed of light in medium of refraction index n2
* `vn`: Speed of light in medium of refraction index n
* `L`: distance to the observation screen or to the light detector
* `NA`: Numerical aperture of the optic fiber
* `y`: distance between two image points on the observation screen (perpendicular to the optical axis) or distance between two point source

For reflection and refraction problems, the focal length and radius of curvature are positive in the direction of the outgoing light (reflected or refracted). The object distance is positive in front of the surface. The image distance is positive in the direction of the outgoing light (reflected or refracted). The magnification is positive for an upright image.

#### Refraction Law

![Refraction Law](img/RefractionLaw.bmp)

* To calculate `[θ₂_°;v1_m/s;v2_m/s]` (Refraction angle; Speed of light in media of refraction index `n1` & `n2`) from 3 known variables:
```rpl
n1=1  n2=1.333  θ₁=45_°
@ Expecting [ θ₂=32.03672 30399 ° v1=299 792 458 m/s v2=224 900 568.642 m/s ]
'ROOT(ⒺRefraction Law;[θ₂;v1;v2];[1_°;1_m/s;1_m/s])'
```

#### Critical Angle

![Critical Angle](img/CriticalAngle.bmp)

* To calculate `[θc_°;v1_m/s;v2_m/s]` (Critical angle; Speed of light in media of refraction index `n1` & `n2`) from 2 known variables:
```rpl
n1=1  n2=1.5
@ Expecting [ θc=41.81031 48958 ° v1=299 792 458 m/s v2=199 861 638.667 m/s ]
'ROOT(ⒺCritical Angle;[θc;v1;v2];[1_°;1_m/s;1_m/s])'
```

#### Fiber Optic

![Fiber Optic](img/Fiber Optic BW.bmp)

* To calculate `[vf0_m/s;vf1_m/s;vf2_m/s;NA;θ0_°]` (Speed of light in media of refraction index `n1f` & `n2f`; Numerical aperture; Acceptance angle) from 3 known variables:
```rpl
nf0=1.2  nf1=1.5  nf2=1.45
@ Expecting [ vf0=249 827 048.333 m/s vf1=199 861 638.667 m/s vf2=206 753 419.31 m/s NA=0.32004 77394 95 θ0=18.66581 19909 ° ]
'ROOT(ⒺFiber Optic;[vf0;vf1;vf2;NA;θ0];[1_m/s;1_m/s;1_m/s;1;1_°])'
```

#### Brewster’s Law

The Brewster angle is the angle of incidence at which the reflected wave is completely polarized.

![Brewster’s Law](img/Brewster’sLaw.bmp)

* To calculate `[θB_°;θ₂_°;v1_m/s;v2_m/s]` (Brewster input angle; Refraction angle; Speed of light in media of refraction index `n1` & `n2`) from 2 known variables:
```rpl
n1=1  n2=1.5
@ Expecting [ v1=299 792 458 m/s v2=199 861 638.667 m/s θB=56.30993 2474 ° θ₂=33.69006 7526 ° ]
'ROOT(ⒺBrewster’s Law;[v1;v2;θB;θ₂];[1_m/s;1_m/s;1_°;1_°])'
```

#### Spherical Reflection

![Spherical Reflection](img/SphericalReflection.bmp)

* To calculate `[m;f_cm]` (Magnification; Focal distance) from 3 known variables:
```rpl
u=10_cm  v=300_cm  r=19.35_cm
@ Expecting [ m=-30 f=9.67741 93548 4 cm ]
'ROOT(ⒺSpherical Reflection;[m;f];[1;1_cm])'
```

#### Spherical Refraction

![Spherical Refraction](img/SphericalRefraction.bmp)

* To calculate `[n2;v1_m/s;v2_m/s]` (Refraction index in medium 2, speed of light in media of refraction index `n1`, `n2`) from 4 known variables:
```rpl
u=8_cm  v=12_cm  r=2_cm  n1=1
@ Expecting [ n2=1.5 v1=299 792 458 m/s v2=199 861 638.667 m/s ]
'ROOT(ⒺSpherical Refraction;[n2;v1;v2];[1;1_m/s;1_m/s])'
```

#### Thin Lens

![Thin Lens](img/ThinLens.bmp)

* To calculate `[f_cm;v_cm;m;vn_m/s]` (Focal distance, distance to image, magnification, speed of light in media of refraction index `n`) from 4 known variables:
```rpl
r1=5_cm  r2=20_cm  n=1.5  u=50_cm
@ Expecting [ f=13.33333 33333 cm v=18.18181 81818 cm m=-0.36363 63636 36 vn=199 861 638.667 m/s ]
'ROOT(ⒺThin Lens;[f;v;m;vn];[1_cm;1_cm;1;1_m/s])'
```

#### Rayleigh’s Criterion

![Rayleigh’s Criterion](img/Rayleigh's Criterion_BW.bmp)

* To calculate `[θr_°;y_m]` (Rayleigh’s criterion angle; Distance between two point source) from 3 known variables:
```rpl
λ=550_nm  d=9_mm  L=18.7_km
@ Expecting [ θr=4.27056 28265⁳⁻³ ° y=1.39381 16503 9 m ]
'ROOT(ⒺRayleigh’s Criterion;[θr;y];[1_°;1_m])'
```

#### Malus Law

If lineraly polarized light is incident on a perfect linear polarizer the transmitted light is the component at angle `θ` between the light polarisation direction and the polarizer transmission axis. The Malus law is given in terms of light irradiances. A relavistic version of the laws applies for X rays and more energetic electromagnetic radiations (with loss up to 10% in irradiance). The decrease in frequency (`fx < fx₀`) and therefore in energy (`h·fx`) of a transmitted photon is due to the movement of the interacting electron of the polarizer (Compton scattering).

![Malus Law](img/Malus Law BW.bmp)

* To calculate `[I_(W/m^2);Ix_(W/m^2),E₀_V/m]` (Polarized light radiance flux; Polarized radiance flux of emitted Xrays; Electric field) from 5 known variables:
```rpl
θ=30_°  I₀=10_(W/m^2)  fx₀=3e17_Hz  fx=2.7e17_Hz  I₀x=0.1_(W/m^2)
@ Expecting [ I=7.5 W/m↑2 Ix=0.06751 63889 32 W/m↑2 E₀=86.80210 98145 V/m ]
'ROOT(ⒺMalus Law;[I;Ix;E₀];[1_(W/m^2);1_(W/m^2);1_V/m])'
```

#### 2 Slits Young Interference

* To calculate `[Δφ_r;I_(W/m^2);y_m;Δyint_m]` (Phase difference between two rays of light separated by an angle θ; Irradiance of emitted light; Distance between two image points on the observation screen, Distance between bright fringes) from 5 known variables:
```rpl
L=2_m  d=800._μm  λ=600_nm  θ='ASIN(0.6*(λ_nm)/(d_μm))'  Imax=10_(W/m^2)
@ Expecting [ Δφ=3.76991 11843 1 r I=6.54508 49718 7 W/m↑2 y=9.00000 09112 5⁳⁻⁴ m Δyint=0.0015 m ]
'ROOT(Ⓔ2 Slits Young Interference;[Δφ;I;y;Δyint];[1_r;1_(W/m^2);1_m;1_m])'
```

#### One Slit Diffraction

* To calculate `[Δα_r;I_(W/m^2);y_m;Δydiff_m]` (Phase difference between top and bottom rays of light; Irradiance of emitted light; Distance between two image points on the observation screen; Width of the central maximum) from 5 known variables:
```rpl
L=3_m  a=1000._μm  λ=600_nm  θ='ASIN(0.3*(λ_nm)/(a_μm))'  Imax=10_(W/m^2)
@ Expecting [ Δα=1.88495 55921 5 r I=7.36839 72932 2 W/m↑2 y=5.40000 00874 8⁳⁻⁴ m Δydiff=0.0036 m ]
'ROOT(ⒺOne Slit Diffraction;[Δα;I;y;Δydiff];[1_r;1_(W/m^2);1_m;1_m])'
```


## Oscillations

The 25 variables in the Oscillations section are:

* `ω`: Angular frequency (dim.: angle/time, in SI: r!s)
* `ω₀`: Natural angular frequency (dim.: angle#time, in SI: r!s)
* `ωu`: Underdamped angular frequency (dim.: angle/time, in SI: r!s)
* `γ`: Reduced damping coefficient (dim.: angle/time, in SI: r/s)
* `φ`: Phase angle
* `θ`: Cone angle
* `a`: Acceleration at `t`
* `E`: Total energy (kinetic plus potential) (dim.: force·length, in SI: joule, J)
* `f`: Frequency (dim.: time^-1; in SI: hertz, Hz)
* `G`: Shear modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `h`: Cone height
* `I`: Moment of inertia (dim.: mass·area, in SI: kg·m^2)
* `J`: Polar moment of inertia (dim.: length^4; in SI: m^4)
* `k`: Spring constant (dim.: force/length, in SI: N/m)
* `L`: Length of pendulum
* `m`: Mass
* `Q`: Quality factor
* `t`: Time
* `T`: Period, or Period for small amplitude ([Simple Pendulum](#Simple Pendulum))
* `Treal`: Real period for large amplitude ([Simple Pendulum](#Simple Pendulum))
* `v`: Velocity at `t`
* `x`: Displacement at `t`
* `xm`: Displacement amplitude
* `xh`: Displacement amplitude of harmonic motion
* `xp`: Resulting displacement amplitude of driven & damped oscillations

#### Mass-Spring System

![Mass-Spring System](img/Mass-SpringSystem.bmp)

* To calculate `[ω_(r/s);T_s;f_Hz]` (Angular frequency; Oscillation period; Frequency) from 2 known variables:
```rpl
k=20_N/m  m=5_kg
@ Expecting [ ω=2. r/s T=3.14159 26535 9 s f=0.31830 98861 84 Hz ]
'ROOT(ⒺMass‐Spring System;[ω;T;f];[1_(r/s);1_s;1_Hz])'
```

#### Simple Pendulum

![Simple Pendulum](img/SimplePendulum.bmp)

* To calculate `[ω_(r/s);Treal_s;T_s;f_Hz]` (Angular frequency; Real period for large amplitude; Period for small amplitude; Frequency) from 2 known variables:
```rpl
L=15_cm  θmax=80_°
@ Expecting [ ω=8.08564 57173 6 r/s  Treal=0.88361 42622 96 s   T=0.77707 89775 87 s  f=1.28687 04840 1 Hz ]
'ROOT(ⒺSimple Pendulum;[ω;Treal;T;f];[1_(r/s);1_s;1_s;1_Hz])'
```
```rpl
@ Without Treal, MSOLVER works fine for the remaining 3 unknowns:
L=15_cm  θmax=80_°
@ Expecting [ ω=8.08564 57173 6 r/s T=0.77707 89775 87 s f=1.28687 04840 1 Hz ]
'ROOT(ⒺSimple Pendulum;[ω;T;f];[1_(r/s);1_s;1_Hz])'
```

#### Conical Pendulum

![Conical Pendulum](img/ConicalPendulum.bmp)

* To calculate `[θ_°;ω_r/s;T_s;f_Hz]` (Cone angle; Angular frequency; Oscillation period; Frequency) from 2 known variables:
```rpl
L=25_cm  h=20_cm
@ Expecting [ θ=36.86989 76458 ° ω=7.00237 45972 3 r/s T=0.89729 35137 83 s f=1.11446 25305 3 Hz ]
'ROOT(ⒺConical Pendulum;[θ;ω;T;f];[1_°;1_r/s;1_s;1_Hz])'
```

#### Torsional Pendulum

![Torsional Pendulum](img/TorsionalPendulum.bmp)

* To calculate `[ω_r/s;T_s;f_Hz]` (Angular frequency; Oscillation period; Frequency) from 4 known variables:
```rpl
G=1000_kPa  J=17_mm^4  L=26_cm  I=50_kg*m^2
@ Expecting [ ω=1.14354 37497 9⁳⁻³ r/s T=5 494.48616 051 s f=1.82000 64042 2⁳⁻⁴ Hz ]
'ROOT(ⒺTorsional Pendulum;[ω;T;f];[1_r/s;1_s;1_Hz])'
```

#### Simple Harmonic

* To calculate `[x_cm;v_cm/s;a_m/s^2;m_kg;E_J]` (Displacement; Velocity & Acceleration at `t`; Mass; Total energy) from 5 known variables:
```rpl
xm=10_cm  ω₀=15_r/s  φ=25_°  t=25_μs  k=10_N/m
@ Expecting [ x=9.06149 24146 7 cm v=-63.44371 46156 cm/s a=-20.38835 7933 m/s↑2 m=4.44444 44444 4⁳⁻² kg E=0.05 J ]
'ROOT(ⒺSimple Harmonic;[x;v;a;m;E];[1_cm;1_cm/s;1_m/s^2;1_kg;1_J])'
```

#### Underdamped Oscillations

We are considering here a damped mass-spring oscillator having the natural angular frequency `ω₀`. The corresponding differential equation : `−k*x − b*dx/dt = m*d^2x/dt^2` describes the underdamped oscillations.

* To calculate `[m_kg;γ_(r/s);ωu_(r/s);x_cm;v_cm/s;a_m/s^2;E_J;Q]` (Mass; Reduced damping coefficient; Underdamped angular frequency; Displacement; Velocity & Acceleration at `t`; Mass; Total energy at `t`; Quality factor) from 6 known variables:
```rpl
xm=10_cm  ω₀=15_r/s  φ=25_°  t=25_μs  k=10_N/m  b=0.2_(kg/s)
@ Expecting [ m=4.44444 44444 4⁳⁻² kg γ=4.5 r/s ωu=14.83028 995 r/s x=9.06100 06640 3 cm v=-83.10906 53488 cm/s a=-16.64734 35534 m/s↑2 E=5.64000 14834 9⁳⁻² J Q=3.33333 33333 3 ]
'ROOT(ⒺUnderdamped Oscillations;[m;γ;ωu;x;v;a;E;Q];[1_kg;1_(r/s);1_(r/s);1_cm;1_cm/s;1_m/s^2;1_J;1])'
@ Save E for later use
```

The code below saves the reference value for comparison with the example in [Driven Damped Oscillations](#Driven Damped Oscillations):
```rpl
@ Save the reference value for comparison below
E₀=E
@ Save E₀ for later
```

#### Driven Damped Oscillations

We are considering here a damped mass-spring oscillator where the external driving force is of the form `Fdriving = Fd*cos(ω*t)` acting at the angular frequency `ω`. The corresponding differential equation : `−k*x − b*dx/dt + Fd*cos(ω*t) = m*d^2x/dt^2` describes the driven damped oscillations. When the driving frequency `ω` comes close to the natural frequency `ω₀` this is the onset of resonance with amplitude increase and the total energy accumulates up to a possible catastrophy when the structure is overcome (see fig)

![Driven Damped Oscillations](img/Driven Damped Oscillations2_BW.bmp)

* To calculate `[m_kg;γ_(r/s);ωu_(r/s);φ_°;xp_m;x_cm;v_cm/s;a_m/s^2;E_J;Q]` (Mass; Reduced damping coefficient; Underdamped angular frequency; Phase angle; Resulting amplitude; Displacement; Velocity & Acceleration at `t`; Total energy at `t`; Quality factor) from 9 known variables which correspond to the values of the previous section:
```rpl
ω=14.99_r/s  ω₀=15_r/s  θ=25_°  t=500_s  k=10_N/m  b=0.2_(kg/s)  xh=10_cm  Fd=0.9_N
@ Expecting [ m=4.44444 44444 4⁳⁻² kg γ=4.5 r/s ωu=14.83028 995 r/s φ=-89.74526 88301 ° xp=0.30019 71665 48 m x=-22.26611 11734 cm v=301.81823 6224 cm/s a=50.03197 40728 m/s↑2 E=0.45032 15149 87 J Q=3.33333 33333 3 ]
'ROOT(ⒺDriven Damped Oscillations;[m;γ;ωu;φ;xp;x;v;a;E;Q];[1_kg;1_(r/s);1_(r/s);1_°;1_m;1_cm;1_cm/s;1_m/s^2;1_J;1])'
@ Save E for comparison
```

Verify relative difference with the total energy of the case [Underdamped Oscillations](#Underdamped Oscillations)
```rpl
E₀ E %Ch
@ Expecting 698.44219 2445
@ % of relative augmentation which illustrates the huge energy gain due to the driving force acting very near the resonance frequency.
```


## Plane Geometry

The 21 variables in the Plane Geometry section are:

* `β`: Central angle of polygon
* `θ`: Vertex angle of polygon
* `A`: Area
* `b`: Base length ([Rectangle](#Rectangle)) & ([Triangle](#Triangle)), or Length of semiaxis in x direction ([Ellipse](#Ellipse))
* `C`: Circumference
* `d`: Distance to rotation axis in y direction
* `h`: Height (Rectangle, Triangle), or Length of semiaxis in y direction (Ellipse)
* `I`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Ix`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance `d` (dim.: mass·area, in SI: kg·m^2)
* `Iy`: Moment of inertia about y axis (dim.: mass·area, in SI: kg·m^2)
* `J`: Polar moment of inertia at centroid (dim.: mass·area, in SI: kg·m^2)
* `L`: Side length of polygon
* `n`: Number of sides
* `P`: Perimeter
* `r`: Radius
* `ri`: Inside radius
* `ro`: Outside radius
* `rs`: Distance to side of polygon
* `rv`: Distance to vertex of polygon
* `v`: Horizontal distance to vertex

#### Circle

![Circle](img/Circle.bmp)

* To calculate `[C_cm;A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Circonference; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 2 known variables:
```rpl
r=5_cm  d=1.5_cm
@ Expecting [ C=31.41592 65359 cm A=78.53981 63397 cm↑2 I=4 908 738.52123 mm↑4 J=9 817 477.04247 mm↑4 Id=6 675 884.38888 mm↑4 ]
'ROOT(ⒺCircle;[C;A;I;J;Id];[1_cm;1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Ellipse

![Ellipse](img/Ellipse.bmp)

* To calculate `[C_cm;A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Circonference; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
b=17.85_μm  h=78.9725_μin  d=.00000012_ft
@ Expecting [ C=7.98046 33593 6⁳⁻³ cm A=1.12485 79868⁳⁻⁶ cm↑2 I=1.13150 61302 6⁳⁻¹⁰ mm↑4 J=9.07327 72104 7⁳⁻⁹ mm↑4 Id=1.13301 09695 2⁳⁻¹⁰ mm↑4 ]
'ROOT(ⒺEllipse;[C;A;I;J;Id];[1_cm;1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Rectangle

![Rectangle](img/Rectangle.bmp)

* To calculate `[C_cm;A_cm^2;I_km^4;J_km^4;Id_km^4]` (Perimeter; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
b=4_chain  h=7_rd  d=39.26_in
@ Expecting [ P=23 134.32 cm A=28 327 994.9568 cm↑2 I=2.92569 11916 2⁳⁻⁷ km↑4 J=1.82109 34968 2⁳⁻⁶ km↑4 Id=2.95386 09978 8⁳⁻⁷ km↑4 ]
'ROOT(ⒺRectangle;[P;A;I;J;Id];[1_cm;1_cm^2;1_km^4;1_km^4;1_km^4])'
```

#### Regular Polygon

![Regular Polygon](img/RegularPolygon.bmp)

* To calculate `[P_cm;A_cm^2;rs_cm;rv_cm;θ_°;β_°]` (Perimeter; Area; Distance to side; Distance to vertex of polygon; Vertex and central angles of polygon) from 2 known variables:
```rpl
n=8  L=0.5_yd
@ Expecting [ P=365.76 cm A=10 092.95006 19 cm↑2 rs=55.18892 20358 cm rv=59.73605 87541 cm θ=135. ° β=45 ° ]
'ROOT(ⒺRegular Polygon;[P;A;rs;rv;θ;β];[1_cm;1_cm^2;1_cm;1_cm;1_°;1_°])'
```

#### Circular Ring

![Circular Ring](img/CircularRing.bmp)

* To calculate `[A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
ro=4_μm  ri=25_Å  d=0.1_mil
@ Expecting [ A=5.02654 62822 5⁳⁻⁷ cm↑2 I=2.01061 92983⁳⁻¹⁰ mm↑4 J=4.02123 85965 9⁳⁻¹⁰ mm↑4 Id=5.25354 58977 5⁳⁻¹⁰ mm↑4 ]
@ Note these answers are OK, the manual HP50g_AUR (p.5-46) is definitively in error (as calculated  directly)
'ROOT(ⒺCircular Ring;[A;I;J;Id];[1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Triangle

![Triangle](img/Triangle.bmp)

* To calculate `[b_in;A_in^2;Ix_in^4;Iy_in^4;J_in^4;Id_in^4]` (Base length; Area; Moment of inertia about `x` and `y` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
h=4.33012781892_in  v=2.5_in  P=15_in  d=2_in
@ Expecting [ b=4.99999 90762 4 in A=10.82531 75473 in↑2 Ix=11.27637 66118 in↑4 Iy=11.27636 82785 in↑4 J=22.55274 48902 in↑4 Id=54.57764 6801 in↑4 ]
'ROOT(ⒺTriangle;[b;A;Ix;Iy;J;Id];[1_in;1_in^2;1_in^4;1_in^4;1_in^4;1_in^4])'
```


## Solid geometry
The 12 variables in the Solid Geometry section are:

* `A`: Total surface area
* `b`: Base length
* `d`: Distance to rotation axis in z direction
* `h`: Height in z direction ([Cone](#Cone)) & ([Cylinder](#Cylinder)), or Height in y direction ([Parallelepiped](#Parallelepiped))
* `I`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Ixx`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance `d` (dim.: mass·area, in SI: kg·m^2)
* `Izz`: Moment of inertia about z axis (dim.: mass·area, in SI: kg·m^2)
* `m`: Mass
* `r`: Radius
* `t`: Thickness in z direction (dim.: length)
* `V`: Volume

#### Cone

![Cone](img/Cone.bmp)

* To calculate `[V_cm^3;A_cm^2;Ixx_kg*m^2;Izz_kg*m^2;Id_kg*m^2]` (Volume; Area; Moment of inertia about `x` axis & `z` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
r=7_cm  h=12.5_cm  m=12.25_kg  d=3.5_cm
@ Expecting [ V=641.40850 0108 cm↑3 A=468.99530 2857 cm↑2 Ixx=0.01618 14843 75 kg·m↑2 Izz=0.01800 75 kg·m↑2 Id=0.03118 77343 75 kg·m↑2 ]
'ROOT(ⒺCone;[V;A;Ixx;Izz;Id];[1_cm^3;1_cm^2;1_kg*m^2;1_kg*m^2;1_kg*m^2])'
```

#### Cylinder

![Cylinder](img/Cylinder.bmp)

* To calculate `[V_in^3;A_in^2;Ixx_lb*in^2;Izz_lb*in^2;Id_lb*in^2]` (Volume; Area; Moment of inertia about `x` axis & `z` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
r=8.5_in  h=65_in  m=12000_lb  d=2.5_in
@ Expecting [ V=14 753.70449 94 in↑3 A=3 925.42002 066 in↑2 Ixx=4 441 750. lb·in↑2 Izz=433 500. lb·in↑2 Id=4 516 750. lb·in↑2 ]
'ROOT(ⒺCylinder;[V;A;Ixx;Izz;Id];[1_in^3;1_in^2;1_lb*in^2;1_lb*in^2;1_lb*in^2])'
```

#### Parallelepiped

![Parallelepiped](img/Parallelepiped.bmp)

* To calculate `[V_in^3;A_in^2;I_lb*in^2;Id_lb*in^2]` (Volume; Area; Moment of inertia about `x` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
b=36_in  h=12_in  t=72_in  m=83_lb  d=7_in
@ Expecting [ V=31 104 in↑3 A=7 776 in↑2 I=36 852. lb·in↑2 Id=40 919. lb·in↑2 ]
'ROOT(ⒺParallelepiped;[V;A;I;Id];[1_in^3;1_in^2;1_lb*in^2;1_lb*in^2])'
```

#### Sphere

![Sphere](img/Sphere.bmp)

* To calculate `[I_kg*m^2;r_cm;V_cm^3;A_cm^2]` (Moment of inertia about `x` axis; Radius; Volume; Area) from 3 known variables:
```rpl
d=14_cm  m=3.75_kg  Id=486.5_lb*in^2
@ Expecting [ I=0.06886 91913 93 kg·m↑2 r=21.42727 10027 cm V=41 208.72679 31 cm↑3 A=5 769.57188 64 cm↑2 ]
'ROOT(ⒺSphere;[I;r;V;A];[1_kg*m^2;1_cm;1_cm^3;1_cm^2])'
```


## Solid State Devices
The 54 variables in the Solid State Devices section are:

* `αF`: Forward common-base current gain
* `αR`: Reverse common-base current gain
* `γ`: Body factor (dim.: (energy/charge)^0.5, in SI: V^0.5)
* `λ`: Modulation parameter (dim.: charge/energy, in SI: V^-1)
* `μn`: Electron mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `φp`: Fermi potential (dim.: energy/charge, in SI: volt, V)
* `ΔL`: Length adjustment ([PN Step Junctions](#PN Step Junctions)), or Channel encroachment ([NMOS Transistors](#NMOS Transistors)) (dim.: length)
* `ΔW`: Width adjustment ([PN Step Junctions](#PN Step Junctions)), or Width contraction ([NMOS Transistors](#NMOS Transistors))
* `a`: Channel thickness
* `Aj`: Effective junction area
* `BV`: Breakdown voltage (dim.: energy/charge, in SI: volt, V)
* `Cj`: Junction capacitance per unit area (dim.: charge^2/(energy·area); in SI: F/m^2)
* `Cox`: Silicon dioxide capacitance per unit area (dim.: charge^2/(energy·area); in SI: F/m^2)
* `E1`: Breakdown-voltage field factor (dim.: force/charge, in SI: N/C=V/m)
* `Emax`: Maximum electric field (dim.: force/charge, in SI: N/C=V/m)
* `G0`: Channel conductance (dim.: resistance^-1, in SI: siemens, S)
* `gds`: Output conductance (dim.: resistance^-1, in SI: siemens, S)
* `gm`: Transconductance (dim.: resistance^-1, in SI: siemens, S)
* `I`: Diode current (dim.: charge/time, in SI: ampere, A)
* `IB`: Total base current (dim.: charge/time, in SI: ampere, A)
* `IC`: Total collector current (dim.: charge/time, in SI: ampere, A)
* `ICEO`: Collector current (collector-to-base open) (dim.: charge/time, in SI: ampere, A)
* `ICO`: Collector current (emitter-to-base open) (dim.: charge/time, in SI: ampere, A)
* `ICS`: Collector-to-base saturation current (dim.: charge/time, in SI: ampere, A)
* `ID, IDS`: Drain current (dim.: charge/time, in SI: ampere, A)
* `IE`: Total emitter current (dim.: charge/time, in SI: ampere, A)
* `IES`: Emitter-to-base saturation current (dim.: charge/time, in SI: ampere, A)
* `IS`: Transistor saturation current (dim.: charge/time, in SI: ampere, A)
* `J`: Current density (dim.: current/area, in SI: A/m^2)
* `Js`: Saturation current density (dim.: current/area, in SI: A/m^2)
* `L`: Drawn mask length ([PN Step Junctions](#PN Step Junctions)), or Drawn gate length ([NMOS Transistors](#NMOS Transistors)), or Channel length ([JFETs](#JFETs))
* `Le`: Effectives gate length
* `NA`: P-side doping ([PN Step Junctions](#PN Step Junctions)), or Substrate doping ([NMOS Transistors](#NMOS Transistors)) (dim.: 1/volume, in SI: m^-3)
* `ND`: N-side doping ([PN Step Junctions](#PN Step Junctions)), or N-channel doping ([JFETs](#JFETs)) (dim.: 1/volume, in SI: m^-3)
* `ni`: Silicon density (dim.: 1/volume, in SI: m^-3)
* `T`: Temperature
* `tox`: Gate silicon dioxide thickness
* `Va`: Applied voltage (dim.: energy/charge, in SI: volt, V)
* `VBC`: Base-to-collector voltage (dim.: energy/charge, in SI: volt, V)
* `VBE`: Base-to-emitter voltage (dim.: energy/charge, in SI: volt, V)
* `Vbi`: Built-in voltage (dim.: energy/charge, in SI: volt, V)
* `VBS`: Substrate voltage (dim.: energy/charge, in SI: volt, V)
* `VCEsat`: Collector-to-emitter saturation voltage (dim.: energy/charge, in SI: volt, V)
* `VDS`: Applied drain voltage (dim.: energy/charge, in SI: volt, V)
* `VDsat`: Saturation voltage (dim.: energy/charge, in SI: volt, V)
* `VGS`: Applied gate voltage (dim.: energy/charge, in SI: volt, V)
* `Vt`: Threshold voltage (dim.: energy/charge, in SI: volt, V)
* `Vt0`: Threshold voltage (at zero substrate voltage) (dim.: energy/charge, in SI: volt, V)
* `W`: Drawn mask width ([PN Step Junctions](#PN Step Junctions)), or Drawn width ([NMOS Transistors](#NMOS Transistors)), or Channel width ([JFETs](#JFETs))
* `We`: Effective width
* `xd`: Depletion-region width
* `xdmax`: Depletion-layer width
* `xj`: Junction depth

#### PN Step Junctions

These equations for a silicon PN-junction diode use a “two-sided step-junction” model–the doping density changes abruptly at the junction. The equation assume the current density is determined by minority carries injected across the depletion region and the PN junction is rectangular in its layout, The temperature should be between 77 and 500 K.

![PN Step Junctions](img/PNStepJunctions.bmp)

* To calculate `[ni_m^-3;Vbi_V;xd_μ;Cj_pF/cm^2;Emax_V/cm;BV_V;J_A/cm^2;Aj_cm^2;I_mA]` (Silicon density; Built-in voltage; Depletion-region width; Junction capacitance per unit area; Maximum electric field; Breakdown voltage; Current density; Effective junction area; Diode current) from 11 known variables:
```rpl
ND=1E22_cm^-3  NA=1E15_cm^-3  T=300_K  Js=1e-6_μA/cm^2  Va=-20_V  E1=3.3E5_V/cm  W=10_μ  ΔW=1_μ  L=10_μ  ΔL=1_μ  xj=2_μ
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ Vbi=1.01379 20414 3 V xd=5.25726 51776 7 μ Cj=2 004.17577 358 pF/cm↑2 Emax=79 941.91402 29 V/cm BV=358.08260 5881 V J=-1.⁳⁻¹² A/cm↑2 Aj=2.57097 33552 9⁳⁻⁶ cm↑2 I=-2.57097 33552 9⁳⁻¹⁵ mA ]
'ROOT(ⒺPN Step Junctions;[ni;Vbi;xd;Cj;Emax;BV;J;Aj;I];[1_cm^-3;1_V;1_μ;1_pF/cm^2;1_V/cm;1_V;1_A/cm^2;1_cm^2;1_mA])'
```

#### NMOS Transistor

These equations for a silicon NMOS transistor use a two-port network model. They include linear and nonlinear regions in the device characteristics and are based on a gradual-channel approximation (the electric fields in the direction of current flow are small compared to those perpendicular to the flow). The drain current and transconductance calculations differ depending on whether the transistor is in the linear, saturated, or cutoff region. The equations assume the physical geometry of the device is a rectangle, second-order length-parameter effects are negligible, shot-channel, hot-carrier, and velocity-saturation effects are negligible, and subthreshold currents are negligible.

![NMOS Transistor](img/NMOSTransistor.bmp)

* To calculate `[ni_(cm^-3);We_μ;Le_μ;Cox_pF/cm^2;γ_V^.5;φp_V;Vt_V;VDsat_V;IDS_mA;gds_S;gm_mA/V]` (Silicon density; Effective width; Effectives gate length; Silicon dioxide capacitance per unit area; Body factor; Fermi potential; Threshold voltage; Saturation voltage; Drain current; Output conductance; Transconductance) from 13 known variables:
```rpl
tox=700_Å  NA=1e15_1/cm^3  μn=600_(cm^2)/(V*s)  T=26.85_°C  Vt0=0.75_V  VGS=5_V  VBS=0_V  VDS=5_V  W=25_μ  ΔW=1_μ  L=4_μ  ΔL=0.75_μ  λ=0.05_1/V
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ We=23 μ Le=2.5 μ Cox=49 330.47499 05 pF/cm↑2 γ=0.37247 98153 1 V↑(¹/₂) φp=-0.29855 35180 52 V Vt=0.75 V VDsat=4.25 V IDS=2.97832 74275 5 mA gds=1.48916 37137 7⁳⁻⁴ S gm=1.42391 28597 5 mA/V ]
'ROOT(ⒺNMOS Transistor;[ni;We;Le;Cox;γ;φp;Vt;VDsat;IDS;gds;gm];[1_cm^-3;1_μ;1_μ;1_pF/cm^2;1_V^(1/2);1_V;1_V;1_V;1_mA;1_S;1_mA/V])'
```

#### Bipolar Transistors

These equations for an NPN silicon bipolar transistor are based on large-signal models developed by J.J. Ebers and J.L. Moll. The offset-voltage calculation differs depending on whether the transistor is saturated or not. The equations also include the special conditions when the emitter-base or collector-base junction is open, which are convenient for measuring transistor parameters.

![Bipolar Transistors](img/BipolarTransistors.bmp)

* To calculate `[VBE_V;IS_nA;ICO_nA;ICEO_nA;IE_mA;IC_mA;IB_mA;VCEsat_V]` (Base-to-emitter voltage; Transistor saturation current; Collector current (emitter-to-base open); Collector current (collector-to-base open); Total emitter current; Total collector current; Total base current; Collector-to-emitter saturation voltage) from 7 known variables:
```rpl
IES=1e-5_nA  ICS=2e-5_nA  T=26.85_°C  αF=0.98  αR=0.49  IC=1_mA  VBC=-10_V
@ Expecting [ VBE=0.63739 37622 77 V IS=0.00000 98 nA ICO=0.00001 0396 nA ICEO=0.00051 98 nA IE=-0.51020 40816 27 mA IB=-0.48979 59183 73 mA VCEsat=-5.95264 29332 3 V ]
'ROOT(ⒺBipolar Transistors;[VBE;IS;ICO;ICEO;IE;IB;VCEsat];[1_V;1_nA;1_nA;1_nA;1_mA;1_mA;1_V])'
```

#### JFETs

These equations for a silicon N-channel junction field-effect transistor (JFET) are based on the single-sided stepjunction
approximation, which assumes the gates are heavily doped compared to the channel doping,. The draincurrent
calculation differs depending on whether the gate-junction depletion-layer thickness is less than or greater
than the channel thickness. The equations assume the channel is uniformly doped and end effects (such as contact,
drain, and source resistances) are negligible.

![JFETs](img/JFETs.bmp)

* To calculate `[ni_(cm^-3);Vbi_V;xdmax_μ;G0_S;ID_mA;VDsat_V;Vt_V;gm_mA/V]` (Silicon density; Built-in voltage; Depletion-layer width; Channel conductance; Drain current; Saturation voltage; Threshold voltage; Transconductance) from 8 known variables:
```rpl
ND=1e16_1/cm^3  W=6_μ  a=1_μ  L=2_μ  μn=1248_cm^2/(V*s)  VGS=-4_V  VDS=4_V  T=26.85_°C
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ Vbi=0.35807 99473 84 V xdmax=1.04848 18266 6 μ G0=5.99854 93177⁳⁻⁴ S ID=0.21443 71687 23 mA VDsat=3.24491 50809 6 V Vt=-7.24491 50809 6 V gm=0.14570 26745 73 mA/V ]
'ROOT(ⒺJFETs;[ni;Vbi;xdmax;G0;ID;VDsat;Vt;gm];[1_(cm^-3);1_V;1_μ;1_S;1_mA;1_V;1_V;1_mA/V])'
```


## Stress Analysis
The 28 variables in the Stress Analysis section are:

* `δ`: Elongation
* `ϵ`: Normal strain
* `γ`: Shear strain (dim.: angle)
* `φ`: Angle of twist
* `σ`: Normal stress (dim.: pressure, in SI: pascal, Pa)
* `σ1`: Maximum principal normal stress (dim.: pressure, in SI: pascal, Pa)
* `σ2`: Minimum principal normal stress (dim.: pressure, in SI: pascal, Pa)
* `σavg`: Normal stress on place of maximum shear stress (dim.: pressure, in SI: pascal, Pa)
* `σx`: Normal stress in x direction (dim.: pressure, in SI: pascal, Pa)
* `σx1`: Normal stress in rotated-x direction (dim.: pressure, in SI: pascal, Pa)
* `σy`: Normal stress in y direction (dim.: pressure, in SI: pascal, Pa)
* `σy1`: Normal stress in rotated-y direction (dim.: pressure, in SI: pascal, Pa)
* `τ`: Shear stress (dim.: pressure, in SI: pascal, Pa)
* `τmax`: Maximum shear stress (dim.: pressure, in SI: pascal, Pa)
* `τx1y1`: Rotated shear stress (dim.: pressure, in SI: pascal, Pa)
* `τxy`: Shear stress (dim.: pressure, in SI: pascal, Pa)
* `θ`: Rotation angle
* `θp1`: Angle 1 to plane of maximum principal normal stress
* `θp2`: Angle 2 to plane of minimum principal normal stress
* `θs`: Angle to plane of maximum shear stress
* `A`: Area
* `E`: Modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `G`: Shear modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `J`: Polar moment of inertia (dim.: length^4, in SI: m^4)
* `L`: Length
* `P`: Load (dim.: force, in SI: newton, N)
* `r`: Radius
* `T`: Torque (dim.: force·length, in SI: N·m)

#### Normal Stress

![Normal Stress](img/NormalStress.bmp)

* To calculate `[δ_in;ε;σ_psi]` (Elongation; Normal strain; Normal stress) from 4 known variables:
```rpl
P=40000_lbf  L=1_ft  A=3.14159265359_in^2  E=10e6_psi
@ Expecting [ σ=12 732.39544 73 psi δ=0.01527 88745 37 in ε=1.27323 95447 3⁳⁻³ ]
'ROOT(ⒺNormal Stress;[σ;δ;ε];[1_psi;1_in;1])'
```

#### Shear Stress

![Shear Stress](img/ShearStress.bmp)

* To calculate `[T_ft*lbf;γ_°;φ_°]` (Torque; Shear strain; Angle of twist) from 5 known variables:
```rpl
L=6_ft  r=2_in  J=10.4003897419_in^4  G=12000000_psi  τ=12000_psi
@ Expecting [ T=5 200.19487 095 ft·lbf γ=0.05729 57795 13 ° φ=2.06264 80624 7 ° ]
'ROOT(ⒺShear Stress;[T;γ;φ];[1_ft*lbf;1_°;1_°])'
```

#### Stress On An Element

Stresses and strains are positive in the directions shown in the picture.

![Stress On An Element](img/StressOnAnElement.bmp)

* To calculate `[σx1_kPa;σy1_kPa;τx1y1_kPa]` (Normal stress in rotated-`x` direction; Normal stress in rotated-`y` direction; Rotated shear stress) from 4 known variables:
```rpl
σx=15000_kPa  σy=4755_kPa  τxy=7500_kPa  θ=30_°
@ Expecting [ σx1=18 933.94052 84 kPa σy1=821.05947 1617 kPa τx1y1=-686.21513 0886 kPa ]
'ROOT(ⒺStress On An Element;[σx1;σy1;τx1y1];[1_kPa;1_kPa;1_kPa])'
```

#### Mohr’s Circle

![Mohr’s Circle](img/Mohr’sCircle.bmp) ```![Mohr’s Circle](img/Mohr’sCircle.bmp)```

* To calculate `[σ1_psi;σ2_psi;θp1_°;θp2_°;τmax_psi;θs_°;σavg_psi]` (Maximum principal normal stress; Minimum principal normal stress; Angle to plane of maximum principal normal stress; Angle to plane of minimum principal normal stress; Maximum shear stress; Angle to plane of maximum shear stress; Normal stress on place of maximum shear stress) from 3 known variables:
```rpl
σx=-5600_psi  σy=-18400_psi  τxy=4800_psi
@ Expecting [ σ1=-1 755.00122 011 psi σ2=-22 244.99877 99 psi θp1=18.43494 88229 ° θp2=108.43494 8823 ° τmax=10 244.99877 99 psi θs=-26.56505 11771 ° σavg=-12 000 psi ]
'ROOT(ⒺMohr’s Circle;[σ1;σ2;θp1;θp2;τmax;θs;σavg];[1_psi;1_psi;1_°;1_°;1_psi;1_°;1_psi])'
```


## Waves
The 39 variables in the Waves section are:

* `β`: Sound intensity level (dim.: intensity in logarithmic scale, in SI: decibel, dB)
* `Δp`: Sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `Δpm`: Amplitude of sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `λ`: Wavelength
* `ω`: Angular frequency (dim.: angle/time. in SI: r/s^2)
* `ρ`: Volumic density of medium (dim.: mass/volume, in SI: kg/m^3)
* `θcone`: Angle at the tip of the cone formed by the supersonic shockwave
* `μ`: Linear density of a string (dim.: mass/length, in SI: kg/m)
* `as`: Acceleration at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Acceleration at `x` and `t` of air particles ([Sound Waves](#Sound Waves))
* `ay`: Acceleration at `x` and `t` of vibrating particles for transversal waves
* `B`: Bulk modulus of elasticity ([Sound Waves](#Sound Waves)) (dim.: pressure, in SI: Pa), or Magnetic field ([Electromagnetic Waves](#Electromagnetic Waves)) (dim.: mass/(time^2·current), in SI: T)
* `E`: Electric field at `x` and `t` of an electromagnetic wave (dim.: force/charge, in SI: N/C=V/m)
* `Em`: Electric Field amplitude (dim.: force/charge, in SI: N/C=V/m)
* `f`: Frequency (dim.: 1/time; in SI: hertz, Hz)
* `favg`: Frequency average (dim.: 1/time; in SI: hertz, Hz)
* `fbeat`: Beat frequency (dim.: 1/time; in SI: hertz, Hz)
* `f0`: Emission frequency (dim.: 1/time; in SI: hertz, Hz)
* `ffixed-fixed`: frequency of harmonics on a string fixed at both ends (dim.: 1/time; in SI: hertz, Hz)
* `ffixed-free`: frequency of harmonics on a string fixed at one end and free at the other end (dim.: 1/time; in SI: hertz, Hz)
* `fopen-open`: frequency of harmonics in a tube open at both ends (dim.: 1/time; in SI: hertz, Hz)
* `fopen-close`: frequency of harmonics in a tube open at one end and close at the other end (dim.: 1/time; in SI: hertz, Hz)
* `I`: Sound intensity (dim.: energy/(area·time), in SI: W/m^2)
* `k`: Angular wave number (dim.: agle/length, in SI: r/m)
* `M`: Mach number
* `ninteger`: Harmonics number being an integer number
* `nodd`: Harmonics number being an odd number
* `Ps`: Power of the source (dim.: energy/time, in SI: watt, W)
* `s`: Longitudinal displacement at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Longitudinal displacement at `x` and `t` of air particles ([Sound Waves](#Sound Waves))
* `sm`: Longitudinal displacement amplitude of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Longitudinal displacement amplitude of air particles ([Sound Waves](#Sound Waves))
* `t`: Time
* `T`: Tension ([String Standing Waves](#String Standing Waves))
* `Tair`: Air temperature ([Doppler Effect](#Doppler Effect)), ([Mach Number](#Mach Number)) & ([Sound Wave Harmonics](#Sound Wave Harmonics))
* `u`: Mass or flow velocity ([Mach Number](#Mach Number))
* `v`: Velocity of the propagating sound in medium ([Sound Waves](#Sound Waves)), or Wave speed ([Transverse Waves](#Transverse Waves)) & ([Longitudinal Waves](#Longitudinal Waves))
* `vr`: Speed of the receiver relative to the medium
* `vs`: Velocity at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Velocity at `x` and `t` of air particles ([Sound Waves](#Sound Waves)), or Speed of the source relative to the medium ([Doppler Effect](#Doppler Effect))
* `vsair`: Velocity of the propagating sound in the ait as a function of temperature
* `vy`: Velocity at `x` and `t` of vibrating particles for transversal waves
* `x`: Position
* `y`: Transverse displacement at `x` and `t`

#### Transverse Waves

* To calculate `[f_Hz;λ_cm;v_cm/s;y_cm;vy_cm/s;ay_cm/s^2]` (Frequency; Wavelength; Transverse displacement at `x` and `t`; Wave speed; Velocity & Acceleration at `x` and `t` of vibrating particles) from 5 known variables:
```rpl
ym=6.37_cm  k=32.11_r/cm  x=0.03_cm  ω=7000_r/s  t=1_s φ=0_r
@ Expecting [ f=1 114.08460 164 Hz λ=0.19567 69015 cm v=218.00062 2859 cm/s y=2.66549 26430 5 cm vy=-40 498.52342 4 cm/s ay=-312 130 000. cm/s↑2 ]
'ROOT(ⒺTransverse Waves;[f;λ;v;y;vy;ay];[1_Hz;11_cm;1_cm/s;1_cm;1_cm/s;1_cm/s^2])'
```

#### Longitudinal Waves

* To calculate `[s_cm;λ_cm;f_Hz;v_m/s;vs_cm/s;as_cm/s^2]` (Frequency; Wavelength; Transverse displacement at `x` and `t`; Wave speed; Velocity & Acceleration at `x` and `t` of vibrating particles) from 5 known variables:
```rpl
sm=6.37_cm  k=32.11_r/cm  x=0.03_cm  ω=7000_r/s  t=1_s φ=0_r
@ Expecting [ s=5.78550 33462 8 cm λ=0.19567 69015 cm f=1 114.08460 164 Hz v=2.18000 62285 9 m/s vs=18 658.44850 13 cm/s as=-283 489 663.968 cm/s↑2 ]
'ROOT(ⒺLongitudinal Waves;[s;λ;f;v;vs;as];[1_cm;1_cm;1_Hz;1_m/s;1_cm/s;1_cm/s^2])'
```

#### Sound Waves

* **Example 1**. To calculate for an unknomn substance `[v_m/s;f_Hz;I_W/m^2;s_cm;vs_cm/s;as_cm/s^2;Δpm_Pa;Δp_Pa;Ps_W;β_dB]` (Wave speed; Frequency; Sound intensity; Longitudinal displacement, Velocity & Acceleration at `x` and `t` of vibrating particles; Amplitude of sound pressure variafion around atmospheric pressure; Sound pressure variafion; Power of the source; Sound level in dB) from 9 known variables:
```rpl
sm=10_cm  ω=6000_r/s  B=12500_kPa  ρ=65_kg/m^3   x=2_cm   t=0_s  r=10_m   k=13.6821_r/m  φ=2_r
@ Expecting [ v=438.52900 9654 m/s f=954.92965 8551 Hz I=5.13078 94129 5⁳⁹ W/m↑2 s=7.63005 85995 9 cm vs=-38 783.49400 98 cm/s as=-274 682 109.585 cm/s↑2 Δpm=17 102 631.3765 Pa Δp=11 054 996.6924 Pa Ps=6.44754 01307 3⁳¹² W β=217.10184 1899 dB ]
'ROOT(ⒺSound Waves;[v;f;I;s;vs;as;Δpm;Δp;Ps;β];[1_m/s;1_Hz;1_(W/(m^2));1_cm;1_cm/s;1_cm/s^2;1_Pa;1_Pa;1_W;1_dB])'
```
* **Example 2**. For a sound wave propagating in air at a pressure of 1 atm and temperature of 20°C, to calculate `[v_m/s;f_Hz;I_W/m^2;s_cm;vs_cm/s;as_cm/s^2;Δpm_Pa;Δp_Pa;Ps_W;β_dB]` (Wave speed; Frequency; Sound intensity; Sound intensity; Longitudinal displacement, Velocity & Acceleration at `x` and `t` of vibrating particles; Amplitude of sound pressure variafion around atmospheric pressure; Sound pressure variafion; Power of the source; Sound level in dB) from 9 known variables:
```rpl
sm=1e-5_m  ω=2 513.27412 287_r/s  B=141_kPa  ρ=1.29_kg/m^3   x=2_cm   t=0_s  r=5_m   k=7.59297 31808 8_r/m  φ=2_r
@ Expecting [ v=330.60902 2232 m/s f=400. Hz I=0.13469 58251 68 W/m↑2 s=8.35879 55288 8⁳⁻⁴ cm vs=-1.37956 85902 cm/s as=-5 279.87232 893 cm/s↑2 Δpm=10.71875 31947 Pa Δp=5.88366 19129 6 Pa Ps=42.31594 14818 W β=111.29354 1352 dB ]
'ROOT(ⒺSound Waves;[v;f;I;s;vs;as;Δpm;Δp;Ps;β];[1_m/s;1_Hz;1_(W/(m^2));1_cm;1_cm/s;1_cm/s^2;1_Pa;1_Pa;1_W;1_dB])'
```
#### Doppler Effect

In the classical Doppler effect it is assumed that the speed of the observer and the source are lower than the speed of sound in the air. The speed of the receiver relative to the air `vr` is added to the speed of sound in the air if the receiver is moving towards the source, subtracted if the receiver is moving away from the source whatever the movement of the source. The speed of the source relative to the air `vs` is subtracted from the speed of sound in the air if the source is moving towards the receiver, added if the source is moving away from the receiver whatever the movement of the receiver.

* **Example 1**. A police car with a siren frequency of 1200 Hz is driving at 180 km/h in the same direction as a truck moving at 90 km/h. To calculate the frequency heard by the trucker when the police are behind him: `[vsair_(m/s);f_Hz]` (Propagation speed of sound waves; Frequency) from 4 known variables:
```rpl
Tair=20_°C  f0=1200_Hz  vr=-90_km/h  vs=180_km/h
@ Expecting [ vsair=343.23616 5602 m/s f=1 302.30661 671 Hz ]
'ROOT(ⒺDoppler Effect;[vsair;f];[1_(m/s);1_Hz])'
```

* **Example 2**. A police car with a siren frequency of 1200 Hz is driving at 180 km/h in the same direction as a truck moving at 90 km/h. To calculate the frequency heard by the trucker when the police are in front of him: `[vsair_(m/s);f_Hz]` (Propagation speed of sound waves; Frequency) from 4 known variables:
```rpl
Tair=20_°C  f0=1200_Hz  vr=90_km/h  vs=-180_km/h
@ Expecting [ vsair=343.23616 5602 m/s f=1 123.70996 713 Hz ]
'ROOT(ⒺDoppler Effect;[vsair;f];[1_(m/s);1_Hz])'
```

#### Mach Number

For an object moving at a supersonic speed, the shockwave describes a cone having the angle `θcone` at its tip where the opposite side is the distance travelled by the sound and the hypothenuse is the distance travelled by the object. The Mach number `M` is the speed ratio to the speed of sound.

* To calculate `[vsair_(m/s);M;θcone_°]` (Propagation speed of sound waves; Mach number; Angle at the tip of the cone formed by the supersonic shockwave) from 2 known variables:
```rpl
Tair=-20_°C  u=2200_km/h
@ Expecting [ vsair=318.96061 3718 m/s M=1.91594 53701 4 θcone=31.46217 41236 ° ]
'ROOT(ⒺMach Number;[vsair;M;θcone];[1_(m/s);1;1_°])'
```

#### String Standing Waves

A string being fixed or free at its ends admits only discrete harmonics as standing waves on the string. A string being fixed (or free) at both ends admits all integer harmonics. A string being fixed at one end and free at the other end admits only all odd integer harmonics.

* To calculate `[v_m/s;k_(r/m);ω_(r/s);Ts_N;y_m;ffixedfixed_Hz;ffixedfree_Hz]` (Propagation speed of waves, Wave number; Angular frequency; Tension; Frequency of harmonics on a string fixed at both ends; Frequency of harmonics on a string fixed at one end and free at the other end) from 9 known variables:
```rpl
λ=1.2_m  f=112_Hz  μ=1.8_(g/m)  L=0.6_m  ninteger=2  nodd=3  x=10_cm  t=5_s  ym=2_cm
@ Expecting [ v=134.4 m/s k=5.23598 77559 8 r/m ω=703.71675 4404 r/s Ts=32.51404 8 N y=0.01 m ffixedfixed=224. Hz ffixedfree=168. Hz ]
'ROOT(ⒺString Standing Waves;[v;k;ω;Ts;y;ffixedfixed;ffixedfree];[1_m/s;1_(r/m);1_(r/s);1_N;1_m;1_Hz;1_Hz])'
```

#### Sound Wave Harmonics

A tube being open or closed at its ends admits only discrete harmonics as standing waves of the sound in the air within the tube. A tube being open (or closed) at both ends admits all integer harmonics. A tube being open at one end and closed at the other end admits only all odd integer harmonics.

* To calculate `[v_m/s;k_(r/m);ω_(r/s);Tair_°C;s_m;fopenopen_Hz;fopenclose_Hz]` (Propagation speed of sound waves; Wave number; Angular frequency, Temperature; Frequency of harmonics in a tube open at both ends; Frequency of harmonics in a tube open at one end and close at the other end) from 8 known variables:
```rpl
λ=3_m f=110_Hz L=0.6_m ninteger=2 nodd=3 x=10_cm t=5_s sm=2e-6_m
@ Expecting [ vsair=330 m/s k=2.09439 51023 9 r/m ω=691.15038 379 r/s Tair=-2.17345 88932 4 °C s=4.15823 38163 6⁳⁻⁷ m fopenopen=550. Hz fopenclose=412.5 Hz ]
'ROOT(ⒺSound Wave Harmonics;[vsair;k;ω;Tair;s;fopenopen;fopenclose];[1_m/s;1_(r/m);1_(r/s);1_°C;1_m;1_Hz;1_Hz])'
```

#### Beat Acoustics

In acoustics, a beat is an interference pattern between two sounds of slightly different frequencies, perceived as a periodic variation in amplitude whose rate is the difference of the two frequencies. The sum of two unit-amplitude sine waves can be expressed as a carrier wave of frequency `favg` whose amplitude is modulated by an envelope wave of frequency `fbeat`.

* To calculate `[favg_Hz;fbeat_Hz;s_m]` (Frequency average; Beat frequency; Longitudinal displacement) from 4 known variables:
```rpl
f1=400_Hz f2=402_Hz t=5_s sm=2e-6_m
@ Expecting [ favg=401 Hz fbeat=2 Hz s=0.00000 2 m ]
'ROOT(ⒺBeat Acoustics;[favg;fbeat;s];[1_Hz;1_Hz;1_m])'
```

#### Electromagnetic Waves

* To calculate `[f_Hz;k_(r/m);ω_(r/s);E_(N/C);B_T]` (Frequency; Wave number; Angular Frequency; Electric & Magnetic fields at `s` & `t`) from 5 known variables:
```rpl
λ=500_nm  Em=5_N/C  x=1e-8_m  t=5e-13_s  φ=25_°
@ Expecting [ f=5.99584 916⁳¹⁴ Hz k=12 566 370.6144 r/m ω=3.76730 31346 2⁳¹⁵ r/s E=4.78368 41812 N/C B=1.59566 52856 2⁳⁻⁸ T ]
'ROOT(ⒺElectromagnetic Waves;[f;k;ω;E;B];[1_Hz;1_(r/m);1_(r/s);1_(N/C);1_T])'
```


## Relativity
The 110 variables in the Relativity section are:

* `α`: Light Doppler effect, light arrival angle in the rest frame
* `β`: Relativistic speed ratio
* `βe`: Escape velocity relativistic speed ratio
* `βp`: Plane velocity relativistic speed ratio
* `βg`: Ground velocity relativistic speed ratio
* `Δt`: Proper time interval ([Time Dilation](#Time Dilation)), or Duration of the circumnavigation trip at latitude `φ` ([Circumnavigating Airplanes](#Circumnavigating Airplanes))
* `Δtp`: Dilated time interval
* `ΔtpG`: Gravitationally dilated time interval
* `Δx`: Proper space interval
* `Δxp`: Contracted space interval
* `λ1`: Wavelength of the photon as measured by the observer at position `R1`
* `λ2`: Wavelength of the photon as measured by the observer at position `R2`
* `λ∞`: Wavelength of the photon as measured by the observer at infinity
* `γ`: Lorentz factor
* `γ21`: Factor of combined special and general relativity effects
* `γv1`: Lorentz factor for velocity `v1`
* `γv2`: Lorentz factor for velocity `v2`
* `γG`: Lorentz factor associated to gravitational dilation
* `γG1`: Lorentz factor associated to gravitational dilation at height `h1`
* `γG2`: Lorentz factor associated to gravitational dilation at height `h2`
* `ω`: Angular velocity associated to planet rotation (dim.: angle/time, in SI: r/s)
* `φ`: Latitude (dim.: angle)
* `θ` : Aberration of light, emission angle in the frame at rest
* `θp` : Aberration of light, emission angle in the moving frame
* `ΔτWE`: Time difference between westward and eastward flights
* `ΔτE`: Flight time in the eastward direction
* `ΔτW`: Flight time in the westward direction
* `Δτg`: Elapsed time variation due to the ground tangential velocity
* `ΔτpE`: Elapsed time variation due to the plane altitude and velocity in the eastward direction
* `ΔτpW`: Elapsed time variation due to the plane altitude and velocity in the westward direction
* `As`: Schwarzschild  black hole surface area
* `ax`: X component of the acceleration
* `ay`: Y component of the acceleration
* `az`: Z component of the acceleration
* `apx`: Transformed x component of the acceleration observed in the moving frame
* `apy`: Transformed y component of the acceleration observed in the moving frame
* `apz`: Transformed z component of the acceleration observed in the moving frame
* `Bx`: X component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `By`: Y component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bz`: Z component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bp`: Norm of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpx`: Transformed x component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpy`: Transformed y component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpz`: Transformed z component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `E`: Total energy (dim.: force·length, in SI: joule, J) or, Norm of the Electric field ([E & B Fields Transformation](#E & B Fields Transformation)) (dim.: force/charge, in SI: N/C=V/m)
* `Ep`: Transformed total energy (dim.: force·length, in SI: joule, J)
* `E₀`: Total energy associated to the rest mass (dim.: force·length, in SI: joule, J)
* `Ex`: X component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Ey`: Y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Ez`: Z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epx`: Transformed x component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epy`: Transformed y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epz`: Transformed z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `f`: Light Doppler effect, frequency received in the frame at rest (dim.: 1/time, in SI: hertz, Hz)
* `fpr`: Light Doppler effect, frequency emitted in the moving frame (dim.: 1/time, in SI: hertz, Hz)
* `fs`: Wave frequency of the source (dim.: 1/time, in SI: hertz, Hz)
* `frl`: Relativistic longitudinal Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `frt`: Relativistic transverse Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `h1`: Height of point 1
* `h2`: Height of point 2
* `hp`: Altitude of plane (dim.: length)
* `K`: Kinetic energy (dim.: force·length, in SI: joule, J)
* `M`: Mass of planet
* `m0`: Rest mass
* `MGu`: Reduced gravitational mass given in geometrized units (dim.: length)
* `Mxearth`: Mass factor as a multiple of Earth mass
* `Mxsun`: Mass factor as a multiple of Sun mass
* `MxSagA`: Mass factor as a multiple of SagitariusA* mass
* `Pθ` = Light relativistic beaming, angular distribution of photon in the moving frame from a source isotropic and stationary
* `p`: Momentum (dim.: mass·speed, in SI: kg·m/s)
* `px`: X component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `py`: Y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `pz`: Z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppx`: Transformed x component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppy`: Transformed y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppz`: Transformed z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `PBH`: Black hole evaporation power (dim.: energy/time, in SI: watt, W)
* `R`: Planet radius
* `R1`: Radius to point 1
* `R2`: Radius to point 2
* `Remp`: Radius at which the photon is emitted
* `rs`: Schwarzschild  radius
* `rxearth`: Radius factor as a multiple of Earth radius
* `SBH`: Black hole entropy (dim.: energy/temperature, in SI: J/K)
* `Tday`: Day duration
* `tev`: Evaporation time of a black hole
* `TH`: Black hole temperature
* `tp`: Transformed time coordinate in the moving frame
* `txyr`: Duration factor as a multiple of a year
* `ux`: X component of the velocity
* `uy`: Y component of the velocity
* `uz`: Z component of the velocity
* `upx`: Transformed x component of the velocity observed in the moving frame
* `upy`: Transformed y component of the velocity observed in the moving frame
* `upz`: Transformed z component of the velocity observed in the moving frame
* `v1`: Velocity at height `h1`
* `v2`: Velocity at height `h2`
* `vg`: Tangential speed of the ground of a rotatong planet
* `Vs`: Schwarzschild  volume of a black hole
* `Vxsun`: Volume factor as a multiple of Sun volume
* `x`: X coordinate in the rest frame
* `y`: Y coordinate in the rest frame
* `z`: Z coordinate in the rest frame
* `xp`: Transformed X coordinate in the moving frame
* `yp`: Transformed Y coordinate in the moving frame
* `zp`: Transformed Z coordinate in the moving frame
* `v`: Velocity along the x axis
* `ve`: Escape velocity in a gravitational field
* `z`: Gravitational redshift parameter
* `znl`: Newtonian limit of the gravitational redshift parameter

The relativistic transformations are parametrized by the real constant `v` representing a velocity confined to the x-direction. The respective inverse transformation is then parameterized by the negative of this velocity.

#### Lorentz Transformation

The primed reference frame `[xp yp zp]` is travelling with velocity `v` in the positive x direction. Therefore, the y and z coordinates of the rest frame remain unchanged.

* To calculate `[β;γ;xp_m;tp_s;yp_m;zp_m]` (Relativistic speed ratio; Lorentz factor; Transformed time, x, y & z coordinate in the moving frame) from 5 known variables:
```rpl
x=1_m  y=2_m  z=3_m  t=4_s  v=239 833 966.4_m/s
@ Expecting [ β=0.8 γ=1.66666 66666 7 xp=-1.59889 31076 7⁳⁹ m tp=6.66666 66622 2 s yp=2 m zp=3 m ]
'ROOT(ⒺLorentz Transformation;[β;γ;xp;tp;yp;zp];[1;1;1_m;1_s;1_m;1_m])'
```

#### Time Dilation

The dilation comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper time interval is multiplied by this factor.

* To calculate `[β;γ;Δtp_s]` (Relativistic speed ratio; Lorentz factor; dilated time interval) from 3 known variables:
```rpl
Δt=4_s  v=239 833 966.4_m/s
@ Expecting [ β=0.8 γ=1.66666 66666 7 Δtp=6.66666 66666 7 s ]
'ROOT(ⒺTime Dilation;[β;γ;Δtp];[1;1;1_s])'
```

#### Space Contraction

The contraction comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper space interval is divided by this factor.

* To calculate `[β;γ;Δxp_m]` (Relativistic speed ratio; Lorentz factor; Contracted space interval) from 3 known variables:
```rpl
Δx=2_m  v=284 802 835.1_m/s
@ Expecting [ β=0.95 γ=3.20256 30761 Δxp=0.62449 97998 4 m ]
'ROOT(ⒺSpace Contraction;[β;γ;Δxp];[1;1;1_m])'
```

#### Velocity Superposition

These expressions replace the usual Galilean addition of velocities. It can be checked that superposing with `v = c` leads to `upx = c`, hence the impossibility to superpose velocities to go beyond the velocity limit `c`. Even if the velocity `v` is confined to the x-direction, all components of the observed velocity are transformed in the moving frame.

* **Example 1** To calculate `[β;γ;upx_(m/s);upy_(m/s);upz_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 3 known variables:
```rpl
v=296 794 533.42_m/s  ux=284 802 835.1_(m/s)  uy=200 000 000_(m/s)  uz=250 000 000_(m/s)
@ Expecting [ β=0.99 γ=7.08881 20500 8 upx=-201 541 148.235 m/s upy=474 175 999.317 m/s upz=592 719 999.146 m/s ]
'ROOT(ⒺVelocity Superposition;[β;γ;upx;upy;upz];[1;1;1_(m/s);1_(m/s);1_(m/s)])'
```
* **Example 2** First attempt to cross the `c` speed limit, to calculate `[upx_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 2 known variables:
```rpl
v=299 792 458_m/s  ux=284 802 835.1_m/s
@ Expecting [ upx=-299 792 458. m/s ]
'ROOT(ⒺVelocity Superposition;[upx];[1_(m/s)])'
```
* **Example 3** Second attempt to cross the `c` speed limit, to calculate `[upx_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 2 known variables:
```rpl
ux=299 792 458_m/s  v=284 802 835.1_m/s
@ Expecting [ upx=299 792 458. m/s ]
'ROOT(ⒺVelocity Superposition;[upx];[1_(m/s)])'
```


#### Acceleration Superposition

Even if the velocity `v` is confined to the x-direction, all components of the observed acceleration are transformed in the moving frame.

* To calculate `[β;γ;apx_(m/s^2);apy_(m/s^2);apz_(m/s^2)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the acceleration observed in the moving frame) from 3 known variables:
```rpl
v=298 293 495.71_m/s  ax=100_(m/s^2)  ay=200_(m/s^2)  az=300_(m/s^2)  ux=284 802 835.1_(m/s)  uy=200 000 000_(m/s)  uz=250 000 000_(m/s)
@ Expecting [ β=0.995 γ=10.01252 34864 apx=607.03954 8234 m/s↑2 apy=4 700.07036 316 m/s↑2 apz=6 041.47314 191 m/s↑2 ]
'ROOT(ⒺAcceleration Superposition;[β;γ;apx;apy;apz];[1;1;1_(m/s^2);1_(m/s^2);1_(m/s^2)])'
```

#### E & B Fields Transformation

Even if the velocity `v` is confined to the x-direction, only the `y` & `z` transformed components of both B & E fields are modified. One can check that ratio the `EDB = E/B = c` & `EpDBp = Ep/Bp = c` remains the same.

* To calculate `[β;γ;Epx_(N/C);Epy_(N/C);Epz_(N/C);Bpx_T;Bpy_T;Bpz_T;E_(N/C);B_T;Ep_(N/C);Bp_T]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the electric field & of the magnetic field;Norm of the Electric field & Magnetic field;Norm of the transformed Electric field & Magnetic field) from 7 known variables:
```rpl
v=298 293 495.71_m/s  Ex=100_(N/C)  Ey=200_(N/C)  Ez=300_(N/C)  Bx=50e-8_T  By=80e-8_T Bz=8.17135 28774 3⁳⁻⁷ T
@ Expecting [ β=0.995 γ=10.01252 34864 Epx=100 N/C Epy=-438.00926 8698 N/C Epz=5 393.09355 125 N/C Bpx=0.00000 05 T Bpy=1.79793 76526 3⁳⁻⁵ T Bpz=1.53534 77686 9⁳⁻⁶ T E=374.16573 8677 N/C B=1.24808 25607 6⁳⁻⁶ T Ep=5 411.77514 056 N/C Bp=1.80517 38781 8⁳⁻⁵ T EDB=299 792 458. m/s EpDBp=299 792 458. m/s ]
'ROOT(ⒺE & B Fields Transformation;[β;γ;Epx;Epy;Epz;Bpx;Bpy;Bpz;E;B;Ep;Bp;EDB;EpDBp];[1;1;1_(N/C);1_(N/C);1_(N/C);1_T;1_T;1_T;1_(N/C);1_T;1_(N/C);1_T;1;1])'
```

#### Longitudinal Doppler Effect

* To calculate `[β;frl]` (Relativistic speed ratio; Transformed longitudinal Doppler frequency) from 2 known variables:
```rpl
v=298 293 495.71_m/s  fs=2e3_Hz
@ Expecting [ β=0.995 frl=100.12523 4864 Hz ]
'ROOT(ⒺLongitudinal Doppler Effect;[β;frl];[1;1_Hz])'
```

#### Transverse Doppler Effect

* To calculate `[β;γ;frt]` (Relativistic speed ratio; Lorentz factor; Transformed transverse Doppler frequency) from 2 known variables:
```rpl
v=298 293 495.71_m/s  fs=2e3_Hz
@ Expecting [ β=0.995 γ=10.01252 34864 frt=20 025.04697 29 Hz ]
'ROOT(ⒺTransverse Doppler Effect;[β;γ;frt];[1;1;1_Hz])'
```

#### Light Propagation

* To calculate `[β;γ;fp;θp;Pθ]` (Relativistic speed ratio; Lorentz factor; Transformed Doppler frequency; Emission angle in the moving frame for light aberration; Angular distribution of photon in the moving frame from a source isotropic and stationary) from 4 known variables:
```rpl
v=298 293 495.71_m/s  f=2e3_Hz  α=20_°  θ=10_°
@ Expecting [ β=0.995 γ=10.01252 34864 fpr=38 748.34889 98 Hz θp=120.44203 7302 ° Pθ=2.14021 57038 4 ]
'ROOT(ⒺLight Propagation;[β;γ;fpr;θp;Pθ];[1;1;1_Hz;1_°;1])'
```

#### Energy & Momentum

The total relativistic energy `E` and the norm of the momentum `p` form the invariant `mo·c^2` which remains the same in all frames. The kinetic energy `K` is the difference between the total relativistic energy `E` and the rest energy `E₀ = mo·c^2`.

* To calculate `[β;γ;ppx_(kg*(m/s));ppy_(kg*(m/s));ppz_(kg*(m/s));Ep_J;E_J;K_J]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the momentum, Transformed total energy; Total & Kinetic energy of the moving mass) from 5 known variables:
```rpl
v=299 192 873.084 m/s  px=10_(kg*(m/s))  py=20_(kg*(m/s))  pz=30_(kg*(m/s))  E=1.42176 77735 4e19_J
@ Expecting [ β=0.998 γ=15.81929 99292 ppx=-7.48730 91346 7⁳¹¹ kg·m/s ppy=20 kg·m/s ppz=30 kg·m/s Ep=2.24913 70834 6⁳²⁰ J E₀=8.98755 17873 9⁳¹⁷ J m0=10. kg p=4.73302 17960 1⁳¹⁰ kg·m/s K=1.33189 22556 7⁳¹⁹ J ]
'ROOT(ⒺEnergy & Momentum;[β;γ;ppx;ppy;ppz;Ep;E₀;m0;p;K];[1;1;1_(kg*(m/s));1_(kg*(m/s));1_(kg*(m/s));1_J;1_J;1_kg;1_(kg*(m/s));1_J])'
```

#### Ultrarelativistic Cases

* **Example 1** In the 27 km circonference accelerator of LHC, protons are accelerated to kinetic energy of 6.8 TeV. To calculate `[E₀_J;γ;β;v_m/s;Δt_s;Δxp_m]` (Rest energy; Lorentz factor; Relativistic speed ratio; Speed; Proper time; Contracted space interval) from 4 known variables, one can calculate the speed, the contracted space interval and proper time of the protons:
```rpl
K=6.8_TeV  m0='Ⓒmp'  Δx=27_km  Δtp='Δx/(299 792 455.147_m/s)'  Δtp=0.00009 00623 07_s
@ Expecting [ E₀=1.50327 76180 2⁳⁻¹⁰ J γ=7 248.36782 709 β=0.99999 99904 83 v=299 792 455.147 m/s Δt=1.24251 84420 6⁳⁻⁸ s Δxp=3.72497 65249 3 m ]
'ROOT(ⒺUltrarelativistic Cases;[E₀;γ;β;v;Δt;Δxp];[1_J;1;1;1_(m/s);1_s;1_m])'
```
* **Example 2** The "Oh-My-God" particle (probably a proton) had a kinetic energy of 3.2e20 eV. To calculate `[E₀_J;γ;β;v_m/s;Δt_s;Δxp_m]` (Rest energy; Lorentz factor; Relativistic speed ratio; Speed; Proper time; Contracted space interval) from 4 known variables, in order to calculate the speed, the contracted space interval and proper time of the proton, the precision needs to be set to 32 digits and 28 significant digits:
```rpl
Modes 'MyModes' STO
32 Precision 28 Sig @ Need high precision for this one
K=3.2e20_eV  m0='Ⓒmp'  Δx=100_km  Δtp='Δx/(299 792 457.99999 99999 99998 7113_m/s)'  Δtp=0.00033 35640 95198 15204 95755 781 s
781_s
@ Expecting [ E₀=1.50327 76180 2⁳⁻¹⁰ J γ=3.41052 60362 9⁳¹¹ β=1. v=299 792 458 m/s Δt=9.78042 95187 6⁳⁻¹⁶ s Δxp=2.93209 90057 2⁳⁻⁷ m ]
'ROOT(ⒺUltrarelativistic Cases;[E₀;γ;β;v;Δt;Δxp];[1_J;1;1;1_(m/s);1_s;1_m])'
ResetModes MyModes @ Restore initial state
```

#### Gravitational Time Dilation

* To calculate `[ve_m/s;βe;γG;ΔtpG_s]` (Excape speed; Relativistic escape speed ratio; Lorentz factor associated to gravitational dilation; Gravitational dilated time interval) from 3 known variables:
```rpl
M=2.32e30_kg  r=6.96e3_m  Δt=1e6_s
@ Expecting [ ve=210 939 169.746 m/s βe=0.70361 73329 83 γG=1.40730 28724 7 ΔtpG=1 407 302.87247 s ]
'ROOT(ⒺGravitational Time Dilation;[ve;βe;γG;ΔtpG];[1_m/s;1;1;1_s])'
```

#### Gravitational Redshift

* To calculate `[rs_m;λ∞_nm;z;λ1_nm;zNL]` (Schwarzschild  radius; Wavelength of the photon as measured by the observer at infinity; Gravitational redshift; Wavelength of the photon as measured by the observer at position `R1`) from 7 known variables:
```rpl
λe=550_nm  λ2=550_nm  M=2.32e30_kg  Remp=70_km  R2=50e6_km  R1=10e6_km  M=4.10227 55e30_kg
@ Expecting [ rs=6 092.83106 622 m λ∞=575.62138 4944 nm z=0.04658 43362 62 λ1=549.99986 5958 nm zNL=4.35202 21901 5⁳⁻² ]
'ROOT(ⒺGravitational Redshift;[rs;λ∞;z;λ1;zNL];[1_m;1_nm;1;1_nm;1])'
```

#### Circumnavigating Airplanes

It is assumed that the planes are circumnavigating at the same altitude `h`, same latitude `φ` and the during same flight duration `Δt` measured in the airplanes. The ground is rotating with the planet at the angular frequency `ω`. The Schwarzschild metric is taken into account. The calculation should formally invokes an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the velocities of the plane `vp` and the planet ground `vg` are << `c` (slow airplanes over a slow rotating planet).

* **Example 1** To calculate for a standard jet (500_mph) `[Δt_s;vg_m/s;βp;βg;MGu_m;Δτg_ns;ΔτpE_ns;ΔτpW_ns;ΔτE_ns;ΔτW_ns;ΔτWE_ns]` (Flight time duration of the circumnavigation trip at latitude `φ`; Ground speed of rotating earth at latitude `φ`; Plane speed ratio; Ground speed ratio; Reduced gravitational mass given in geometrized units; Elapsed time variation due to the ground tangential velocity; Elapsed time variation due to the plane altitude and velocity in the Eastward & Westward direction; Flight time in the Eastward & Westward direction; Time difference between westward and eastward flights) from 6 known variables:
```rpl
vp=500_mph  Tday=86400_s  R=6371_km  hp=1e4_m  M=5.972168e24_kg  φ=7_°
@ Expecting [ Δt=177 754.98724 2 s vg=459.85873 5513 m/s βp=7.45582 46558 7⁳⁻⁷ βg=1.53392 36303⁳⁻⁶ MGu=4.43502 76722 1⁳⁻³ m Δτg=177 754.98711 8 s ΔτpE=177 754.98711 8 s ΔτpW=177 754.98711 8 s ΔτE=-58.77919 8501 ns ΔτW=347.80567 188 ns ΔτWE=406.58487 0381 ns ]
'ROOT(ⒺCircumnavigating Airplanes;[Δt;vg;βp;βg;MGu;Δτg;ΔτpE;ΔτpW;ΔτE;ΔτW;ΔτWE];[1_s;1_m/s;1;1;1_m;1_s;1_s;1_s;1_ns;1_ns;1_ns])'
```
* **Example 2** To calculate for the circumnavigation of the Concorde at maximal speed (Mach 2.04) flying at an altitude of 60000 feet `[Δt_s;vg_m/s;βp;βg;MGu_m;Δτg_ns;ΔτpE_ns;ΔτpW_ns;ΔτE_ns;ΔτW_ns;ΔτWE_ns]` (Flight time duration of the circumnavigation trip at latitude `φ`; Ground speed of rotating earth at latitude `φ`; Plane speed ratio; Ground speed ratio; Reduced gravitational mass given in geometrized units; Elapsed time variation due to the ground tangential velocity; Elapsed time variation due to the plane altitude and velocity in the Eastward & Westward direction; Flight time in the Eastward & Westward direction; Time difference between westward and eastward flights) from 6 known variables:
```rpl
vp=605.27777 77777 77777 77777_m/s  Tday=86400_s  R=6371e3_m  hp=18288_m  M=5.972168e24_kg  φ=12_°
@ Expecting [ Δt=64 689.99803 66 s vg=453.18771 1296 m/s βp=2.01898 93428 8⁳⁻⁶ βg=1.51167 14887 4⁳⁻⁶ MGu=4.43502 76722 1⁳⁻³ m Δτg=64 689.99799 14 s ΔτpE=64 689.99799 12 s ΔτpW=64 689.99799 16 s ΔτE=-200.38946 8271 ns ΔτW=194.48457 0936 ns ΔτWE=394.87403 9207 ns ]
'ROOT(ⒺCircumnavigating Airplanes;[Δt;vg;βp;βg;MGu;Δτg;ΔτpE;ΔτpW;ΔτE;ΔτW;ΔτWE];[1_s;1_m/s;1;1;1_m;1_s;1_s;1_s;1_ns;1_ns;1_ns])'
```

#### Clocks at different heights

It is assumed that the two clocks are at rest with respect to the ground at a latitude `φ` and are rotating with the planet at the angular frequency `ω`. The clocks are at their respective heights `h1` and `h2` for instance at the bottom & top of a mountain. For simplicity, the planet is assumed to have a spherical distribution. The Schwarzschild metric is taken into account. The calculation should formally invoke an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the tangential velocities at height `h1` and `h2` are << `c` (slow rotating planet). As a consequence, the rate of time dilation per meter of height is calculated.

* **Example 1a)** (Earth): To calculate `[ω_r/s;v1_m/s;v2_m/s;MGu_m;γv1;γv2;γG1;γG2;γ21]` (Angular velocity associated to planet rotation; Velocity at height `h1` & `h2` and latitude `φ`; Reduced gravitational mass given in geometrized units; Lorentz factor for velocity `v1` & `v2`; Lorentz factor associated to gravitational dilation at height `h1` & `h2`; Factor of combined special and general relativity effects) from 6 known variables (choose `h2 > h1`):
```rpl
Tday=86400_s  R=6371e3_m  h1=0_m  h2=2000_m  M=5.972168e24_kg  φ=15_°
@ Expecting [ ω=7.27220 52166 4⁳⁻⁵ r/s v1=447.52521 416 m/s v2=447.66570 2376 m/s MGu=4.43502 76722 1⁳⁻³ m γv1=1. γv2=1. γG1=1.00000 00007 γG2=1.00000 00007 γ21=1. ]
'ROOT(ⒺClocks at different heights;[ω;v1;v2;MGu;γv1;γv2;γG1;γG2;γ21];[1_r/s;1_m/s;1_m/s;1_m;1;1;1;1;1])'
@ Save result for later use
```
* **Consequence** To check the validity of a well-known APPROXIMATION which is valid when `Δh/R < 0.1%` => special relativity corrections are negligible which means `ABS(γv1/γv2-1) < 0.1%` => `γ21=γG1/γG2` Then the APPROXIMATE RESULT is `γG1/γG2 ≈ 1 + gloc*Δh/Ⓒc^2` with `gloc=ⒸG*M/R^2`. Let's verify precisely these relations in 3 steps with the final CONSEQUENCE:

* **Example 1b)** The prerequisite conditions & their direct consequence are given by:
```rpl
Δh='ABS((h2_m)-(h1_m))'  gloc='ⒸG*(M_kg)/((R_m)+(h1_m))^2'  approx1='ABS(γG1/γG2-1)'  approx2='gloc*Δh/Ⓒc^2'
"1a) Prerequisite conditions:"
"(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100"
'(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 1c)** EVAL => True, then APPROXIMATION can be checked (EVAL => True) by:
```rpl
"1b) APPROXIMATION check:"
"→NUM(ABS(approx1/approx2-1)) < 0.1/100"
'→NUM(ABS(approx1/approx2-1)) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 1d)** The important CONSEQUENCE is that the following value is the RATE OF TIME DILATION per meter of height due to a gravitational field `gloc` at height `h1`:
```rpl
"=> Rate of t Dilation /m - vert :"
"ABS(γ21-1)/Δh≈gloc/Ⓒc^2="
@ Expecting 1.09265 01350 9⁳⁻¹⁶ m⁻¹
'→NUM(gloc/Ⓒc^2)'
@ Save result for later use
```
* **Example 2a)** (Earth, Mount Everest):  This mount has an height of 3660_m with repect to the surrounding ground which is at an altitude of 5200_m. To calculate `[ω_r/s;v1_m/s;v2_m/s;MGu_m;γv1;γv2;γG1;γG2;γ21]` (Angular velocity associated to planet rotation; Velocity at height `h1` & `h2` and latitude `φ`; Reduced gravitational mass given in geometrized units; Lorentz factor for velocity `v1` & `v2`; Lorentz factor associated to gravitational dilation at height `h1` & `h2`; Factor of combined special and general relativity effects) from 6 known variables (maintain 24 digits of precision & choose `h2 > h1`):
```rpl
Tday=86400_s  R=6371e3_m  h1=5200_m  h2=8860_m  M=5.972168e24_kg  φ=15_°
@ Expecting [ ω=7.27220 52166 4⁳⁻⁵ r/s v1=447.89048 3523 m/s v2=448.14757 6959 m/s MGu=4.43502 76722 1⁳⁻³ m γv1=1. γv2=1. γG1=1.00000 00007 γG2=1.00000 00007 γ21=1. ]
'ROOT(ⒺClocks at different heights;[ω;v1;v2;MGu;γv1;γv2;γG1;γG2;γ21];[1_r/s;1_m/s;1_m/s;1_m;1;1;1;1;1])'
@ Save result for later use
```
* **Consequence** To check the validity of a well-known APPROXIMATION which is valid when `Δh/R < 0.1%` => special relativity corrections are negligible which means `ABS(γv1/γv2-1) < 0.1%` => `γ21=γG1/γG2` Then the APPROXIMATE RESULT is `γG1/γG2 ≈ 1 + gloc*Δh/Ⓒc^2` with `gloc=ⒸG*M/R^2`. Let's verify precisely these relations in 3 steps with the final CONSEQUENCE:
* **2b)** The prerequisite conditions & their direct consequence are given by:
```rpl
Δh='ABS((h2_m)-(h1_m))'  gloc='ⒸG*(M_kg)/((R_m)+(h1_m))^2'  approx1='ABS(γG1/γG2-1)'  approx2='gloc*Δh/Ⓒc^2'
"2a) Prerequisite CONDITIONS:"
"(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100"
'(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 2c)** EVAL => True, then APPROXIMATION can be checked (EVAL => True) by:
```rpl
"1b) APPROXIMATION check:"
"→NUM(ABS(approx1/approx2-1)) < 0.1/100"
'→NUM(ABS(approx1/approx2-1)) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 2d)** The important CONSEQUENCE is that the following value is the RATE OF TIME DILATION per meter of height due to a gravitational field  `gloc` at height `h1`:
```rpl
"=> Rate of t Dilation /m - vert :"
"ABS(γ21-1)/Δh≈gloc/Ⓒc^2="
'→NUM(gloc/Ⓒc^2)'
@ Expecting 1.09086 86778 4⁳⁻¹⁶ m⁻¹
```

#### B H Schwarzschild Geometry

* For Sagittarius A*, the supermassive black hole at the Galactic Center of the Milky Way to calculate `[rs_m;ve_(m/s);Vs_(m^3);Vxsun;rxearth;Mxsun;Mxearth]` (Schwarzschild radius; Escape velocity; Schwarzschild volume; Factor multiplicative of Sun volume, of Earth radius, of Sun mass & of Earth mass) from 3 known variables (maintain 24 digits of precision):
```rpl
M=8.54e36_kg  r=12e9_m  V=8.54105 09309e30_m^3
@ Expecting [ rs=1.26838 81739 7⁳¹⁰ m ve=299 792 458. m/s Vs=8.54761 91182 1⁳³⁰ m↑3 Vxsun=6 048.90292 557 rxearth=1 883.53476 691 Mxsun=4 294 694.49334 Mxearth=1.42996 64711 4⁳¹² ]
'ROOT(ⒺB H Schwarzschild Geometry;[rs;ve;Vs;Vxsun;rxearth;Mxsun;Mxearth];[1_m;1_(m/s);1_(m^3);1;1;1;1])'
```

#### B H Thermodynamics

* **Example 1** For M31*, the supermassive black hole at the Galactic Center of Andromeda Galaxy to calculate `[rs_m;As_(m^2);TH_K;PBH_W;SBH_(J/K);tev_s;Mxsun;MxSagA;Mxearth;txyr]` (Schwarzschild radius; Schwarzschild area; Black hole temperature; Black hole evaporation power; Black hole entropy; Evaporation time; Multiplicative factor of Sun mass, of Sagittarius A* mass & of Earth mass; Multiplicative factor of a year) from 2 known variables (maintain 24 digits of precision):
```rpl
M=1.708e45_kg  t=4.18902 53989e119_s
@ Expecting [ rs=2.53677 63479 3⁳¹⁸ m As=8.08675 38442 8⁳³⁷ m↑2 TH=5.⁳⁻²³ K PBH=1.22087 75567 7⁳⁻⁵⁸ W SBH=1.06850 79039 3⁳⁸⁴ J/K tev=4.19117 60841 4⁳¹¹⁹ s Mxsun=8.58938 89866 7⁳¹⁴ MxSagA=200 000 000. Mxearth=2.85993 29422 7⁳²⁰ txyr=1.32742 20469 6⁳¹¹² ]
'ROOT(ⒺB H Thermodynamics;[rs;As;TH;PBH;SBH;tev;Mxsun;MxSagA;Mxearth;txyr];[1_m;1_(m^2);1_K;1_W;1_(J/K);1_s;1;1;1;1])'
```
* **Example 2** For a very small black hole having the mass of 1000_kg, to calculate `[rs_m;As_(m^2);TH_K;PBH_W;SBH_(J/K);tev_s;Mxsun;MxSagA;Mxearth;txyr]` (Schwarzschild radius; Schwarzschild area; Black hole temperature; Black hole evaporation power; Black hole entropy; Evaporation time; Factor multiplicative of Sun mass, of Sagittarius A* mass & of Earth mass; Multiplicative factor of a year) from 2 known variables (maintain 24 digits of precision):
```rpl
M=1000_kg  t=8.40716 15834 7⁳⁻⁸ s
@ Expecting [ rs=1.48523 20538 2⁳⁻²⁴ m As=2.77203 36055 6⁳⁻⁴⁷ m↑2 TH=1.22690 06705 9⁳²⁰ K PBH=3.56162 21447 8⁳²⁶ W SBH=0.36627 05548 53 J/K tev=8.41147 78997⁳⁻⁸ s Mxsun=5.02891 62685 4⁳⁻²⁸ MxSagA=1.17096 01873 5⁳⁻³⁴ Mxearth=1.67443 38069 5⁳⁻²² txyr=2.66406 87452 4⁳⁻¹⁵ ]
'ROOT(ⒺB H Thermodynamics;[rs;As;TH;PBH;SBH;tev;Mxsun;MxSagA;Mxearth;txyr];[1_m;1_(m^2);1_K;1_W;1_(J/K);1_s;1;1;1;1])'
```

## Modern Physics

The 43 variables in the Modern Physics section are :

* `β`: Relativistic speed ratio
* `φ`: Work function of the substance([Photoelectric Effect](#Photoelectric Effect))(dim.: charge·voltage, in SI : eV); or Angle of the scattered electron ([Compton Scattering](#Compton Scattering))
* `θ`: Scattered photon angle of deflection ([Photoelectric Effect](#Photoelectric Effect)) & ([Compton Scattering](#Compton Scattering)), or Angle between incident photon and cristallographic plane ([DeBroglie Wave](#DeBroglie Wave))
* `γ`: Lorentz relativistic factor
* `%rFr12`: Relative % of change between distribution fractions integrated from `f1` to `f2`
* `%rFr34`: Relative % of change between distribution fractions integrated from `f3` to `f4`
* `λ`: Wavelength of the incident photon ([Compton Scattering](#Compton Scattering))
* `λp`: Wavelength of the diffused photon
* `A`: Area, or Total activity ([Radioactivity](#Radioactivity)) (dim.: number of decay/time, in SI: becquerel, Bq)
* `d`: Interatomic distance between cristallographic plane
* `En`: Electron energy in level `n` (dim.: charge·voltage, in SI: eV)
* `Enp`: Electron energy in level `np` (dim.: charge·voltage, in SI: eV)
* `ebfafb`: Emissive power in the range `fa` to `fb` (dim.: energy/time·area, in SI: W/m^2)
* `eb`: Total emissive power for the entire Planck spectrum (dim.: energy/time·area, in SI: W/m^2)
* `Eph`: Inident photon energy (dim.: charge·voltage; in SI: eV)
* `Epph`: Diffused photon energy (dim.: charge·voltage; in SI: eV)
* `f`: Frequency of the photon (dim.: 1/time, in SI: hertz, Hz)
* `f0`: Threshold frequency (dim.: 1/time, in SI: hertz, Hz)
* `f1`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f2`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f3`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f4`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fa`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fb`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fpeak`: Frequency of maximum emissive power for the Planck distribution (dim.: 1/time, in SI: hertz, Hz)
* `Frfafb`: Fraction of total emissive power in the range `fa` to `fb`
* `Frfafb`: Fraction of Planck emissive power in the range `fa` to `fb`
* `FrPl12`: Fraction of Planck emissive power in the range `f1` to `f2`
* `FrWn12`: Fraction of Wien emissive power in the range `f1` to `f2`
* `FrRJ12`: Fraction of Rayleign-Jeans emissive power in the range `f1` to `f2`
* `FrPl34`: Fraction of Planck emissive power in the range `f3` to `f4`
* `FrWn34`: Fraction of Wien emissive power in the range `f3` to `f4`
* `FrRJ34`: Fraction of Rayleign-Jeans emissive power in the range `f3` to `f4`
* `K`: Kinetic energy of scattered electron (dim.: charge·voltage; in SI: eV)
* `Kmax`: Maximum kinetic energy of photoelectron ([Photoelectric Effect](#Photoelectric Effect)) (dim.: charge·voltage; in SI: eV)
* `p`: Momentum (dim.: mas·speed, in SI: kg·m/s)
* `m`: Mass
* `q`: Heat transfer rate (dim.: energy/time, in SI: watt, W)
* `r`: Radius of electron orbit in level `n`
* `T`: Temperature
* `V0`: Stopping potential (dim.: energy/charge, In SI: volt,V)
* `v`: Speed of emitted electron
* `vmax`: Maximum speed of ejected photoelectron

#### Planck & Wien Comparison

In this section, two comparisons are done between the Planck and Wien spectral distributiona. Based on a incomplete thermodynamic argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` & `f2`, or between `f3` & `f4`. One shall determine in which frequency interval both distribution differs notably or agree. The asymptotic agreement for large frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power `ebfafb` and the heat transfer rate `q` from the black-body. Or, For the Planck distribution, one can choose any other values for `fa` & `fb` and compute the corresponding `FrPlab`, `ebfafb` and `q`.

![Planck & Wien Comparison](img/Planck_and_Wien_Distributions.bmp)

* To calculate `[fpeak_THz;f1_THz;f2_THz;FrPl12;FrWn12;%rFr12;f3_THz;f4_THz;FrPl34;FrWn34;%rFr34;FrPlab;eb_(W/m^2);ebfafb_(W/m^2);q_W]` (Frequency of maximum emissive power for the Planck distribution; Lower & Upper frequency limits of integration; Fractions of Planck & Wien emissive power in the range `f1` to `f2`; Relative % of change between distribution fractions integrated from `f3` to `f4`; Lower & Upper frequency limits of integration; Fractions of Planck & Wien emissive power in the range `fa` to `fb`; Total emissive power for the entire Planck spectrum; Emissive power in the range `fa` to `fb`; Heat transfer rate) from 5 known variables:
```rpl
T=1273.15_K  A=100_cm^2  fa=7.48475 43283 5⁳¹³ Hz  fb=3.18337 69964 2⁳¹⁴ Hz  Frfafb=0.64388 90934 2
@ Expecting [ fpeak=74.84754 32835 THz f1=106.11256 6547 THz f2=238.75327 4732 THz FrPl12=0.38336 04816 94 FrWn12=0.38088 77248 71 %rFr12=0.64502 13155 81 f3=0.26528 14163 69 THz f4=66.32035 40922 THz FrPl34=0.28402 76245 74 FrWn34=0.22398 47200 01 %rFr34=21.13981 15457 FrPlab=0.64388 90934 2 eb=148 980.70811 W/m↑2 ebfafb=95 927.05308 19 W/m↑2 q=959.27053 0819 W ]
'ROOT(ⒺPlanck & Wien Comparison;[fpeak;f1;f2;FrPl12;FrWn12;%rFr12;f3;f4;FrPl34;FrWn34;%rFr34;FrPlab;eb;ebfafb;q];[1_THz;1_THz;1_THz;1;1;1;1_THz;1_THz;1;1;1;1;1_(W/m^2);1_(W/m^2);1_W])'
```

#### Planck & Rayleigh-Jeans Comparison

In this section, two comparisons are done between the Planck and Rayleigh-Jeans spectral distributiona. Based on the equipartition theorem argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` & `f2`, or between `f3` & `f4`. One shall determine in which frequency interval both distribution agree or differs considerably, leading to a divergence called UV catastrophy corresponding to unphysical fractions greather than one. The asymptotic agreement for small frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power `ebfafb` and the heat transfer rate `q` from the black-body. Or, For the Planck distribution, one could choose any other values for `fa` & `fb` and computes the corresponding `FrPlab`, `ebfafb` and `q`.

![Planck & Rayleigh-Jeans Comparison](img/Planck_and_Rayleigh-Jeans_Distributions.bmp)

* To calculate `[fpeak_THz;f1_THz;f2_THz;FrPl12;FrRJ12;%rFr12;f3_THz;f4_THz;FrPl34;FrRJ34;%rFr34;FrPlab;eb_(W/m^2);ebfafb_(W/m^2);q_W]` (Frequency of maximum emissive power for the Planck distribution; Lower & Upper frequency limits of integration; Fractions of Planck & Rayleigh‐Jeans emissive power in the range `f1` to `f2`; Relative % of change between distribution fractions integrated from `f3` to `f4`; Lower & Upper frequency limits of integration; Fractions of Planck & Rayleigh‐Jeans emissive power in the range `fa` to `fb`; Total emissive power for the entire Planck spectrum; Emissive power in the range `fa` to `fb`; Heat transfer rate) from 5 known variables:
```rpl
T=1273.15_K  A=100_cm^2  fa=2.65281 41636 9⁳¹⁰ Hz  fb=7.48475 43283 5⁳¹³ Hz  Frfafb=0.35399 34269 15
@ Expecting [ fpeak=74.84754 32835 THz f1=45.09784 07827 THz f2=98.15412 40564 THz FrPl12=0.41306 62386 78 FrRJ12=2.34783 01416 5 %rFr12=468.39071 3596 f3=2.65281 41636 9⁳⁻² THz f4=1.32640 70818 4 THz FrPl34=6.29668 51249 6⁳⁻⁶ FrRJ34=6.41618 75792 7⁳⁻⁶ %rFr34=1.89786 29538 3 FrPlab=0.35399 34269 15 eb=148 980.70811 W/m↑2 ebfafb=52 738.19140 8 W/m↑2 q=527.38191 408 W ]
'ROOT(ⒺPlanck & Rayleigh‐Jeans Comparison;[fpeak;f1;f2;FrPl12;FrRJ12;%rFr12;f3;f4;FrPl34;FrRJ34;%rFr34;FrPlab;eb;ebfafb;q];[1_THz;1_THz;1_THz;1;1;1;1_THz;1_THz;1;1;1;1;1_(W/m^2);1_(W/m^2);1_W])'
```

#### Photoelectric Effect

Einstein explained the photoelectric effect with the energy quantification of the electromagnetic wave. The photoelectron is then emitted only if the energy `E` of the incident photon is greather or equal to the work function `φ` of the material. A simple energy budget equation determines the maximum kinetic energy `Kmax` of the photoelectron.

* To calculate `[f_THz;Eph_eV;f0_THz;λ0_nm;Kmax_eV;Vo_V;vmax_m/s]` (Frequency, Photon energy; Threshold frequency; Threshold wavelength; Maximum kinetic energy of photoelectron; Stoping potential; Maximum speed of photoelectron) from 2 known variables:
```rpl
φ=4.01_eV  λ=207_nm
@ Expecting [ f=1 448.27274 396 THz Eph=5.98957 22016 1 eV f0=969.61410 7285 THz λ0=309.18739 2951 nm Kmax=1.97957 22016 1 eV Vo=1.97957 30615 6 V vmax=834 471.41457 5 m/s ]
'ROOT(ⒺPhotoelectric Effect;[f;Eph;f0;λ0;Kmax;Vo;vmax];[1_THz;1_eV;1_THz;1_nm;1_eV;1_V;1_m/s])'
```

#### Compton Scattering

In the Compton Scattering, both energy and momentum are conserved during the collision of the incident photon and the electron, which underlines the fact that the photon must henceforth be considered as a particle. When a high frequency `f` (or energy `E = hf`) photon scatters due to an interaction with an electron, there is a decrease in the energy of the photon scattered at an angle `θ` and thus, an increase in its wavelength `λp`. The kinetic energy of the scattered electron `Kmax` is relativist.

![Compton Scattering_BW](img/Compton_Scattering_BW.bmp)

* To calculate `[λp_nm;K_eV;γ;β;v_m/s;Eph_eV;Epph_eV;p_(kg*m/s);φ_°]` (Wavelength of scattered photon; Kinetic energy of scattered electron; Lorentz factor; Speed of the scattered electron; Energy of the Incident & Scattered photon; Momentum of the scattered electron; Angle of scattering of the electron) from 2 known variables:
```rpl
θ=40_°  λ=0.024_nm
@ Expecting [ λp=2.45676 48762 3⁳⁻² nm K=1 193.63352 749 eV γ=1.00233 58835 6 β=6.82308 49980 3⁳⁻² v=20 455 094.227 m/s Eph=51 660.06023 89 eV Epph=50 466.42671 14 eV p=1.86768 55511 5⁳⁻²³ kg·m/s φ=68.16075 25239 ° ]
'ROOT(ⒺCompton Scattering;[λp;K;γ;β;v;Eph;Epph;p;φ];[1_nm;1_eV;1;1;1_m/s;1_eV;1_eV;1_(kg*m/s);1_°])'
```

#### De Broglie Wave

At all scales where measurements have been possible, matter exhibits wave-like behavior (e.g. Young interference for protons, even for molecules). More precisely, a beam of neutron can be diffracted just like a beam of light or a water wave as it is the case in the Bragg diffraction. Here, the interference is constructive when the phase difference between the matter wave reflected off different atomic planes at an angle `θ` is a multiple of 2π, giving the Following condition: `2·d·SINθ = n·λ` with `n` integer and where `θ` is mesured between the refected waves trajectory and the atomic plane.

* To calculate `[λ_nm;K_eV;v_m/s;d_nm]` (De Broglie wavelength of scattered photon; Kinetic energy of scattered electron; Speed of the scattered electron; Distance between adjacent atomic planes) from 3 known variables:
```rpl
θ=40_°  p=1e-23_kg*m/s m=Ⓒme n=2
@ Expecting [ λ=0.06626 07015 nm K=342.58664 2473 eV v=10 977 691.0426 m/s d=0.10308 33521 08 nm ]
'ROOT(ⒺDe Broglie Wave;[λ;K;v;d];[1_nm;1_eV;1_m/s;1_nm])'
```

#### Bohr Atomic Model

Since the hydrogen atom is a bound system between the proton of the nucleus and the electron located at a level `n`, the binding energy is calculated with a negative expression, the value 0 corresponding to a free electron or occupying a level n which tends to infinity. A transition which gives a positive energy difference corresponds to the emission of a final photon `np` > initial `n`, or if it is negative, to the absorption of a photon.

* **Example 1** In the case of an emission, to calculate `[Enp_eV;En_eV;r_m;f_THz;Eph_eV;λ_nm;r_m]` (Energy of the final atomic level `np`; Energy of the initial atomic level `n`; Radius of the initial atomic level `n`; Frequency, Energy & Wavelength of the emitted photon) from 3 known variables:
```rpl
np=2  n=1  Z=1
@ Expecting [ Enp=-3.40142 18031 3 eV En=-13.60568 72125 eV r=5.29177 21054 4⁳⁻¹¹ m f=2 467.38147 018 THz Eph=10.20426 54094 eV λ=121.50227 3411 nm ]
'ROOT(ⒺBohr Atomic Model;[Enp;En;r;f;Eph;λ];[1_eV;1_eV;1_m;1_THz;1_eV;1_nm])'
```
* **Example 2** In the case of an absorption, to calculate `[Enp_eV;En_eV;r_m;f_THz;Eph_eV;λ_nm]` (Energy of the final atomic level `np`; Energy of the initial atomic level `n`; Radius of the initial atomic level n; Frequency, Energy & Wavelength of the absorbed photon) from 3 known variables (Note: instead to `n→∞` one can choose `n=9.99999E999999`):
```rpl
np=2  n=9.99999E999999  Z=1
@ Expecting [ Enp=-3.40142 18031 3 eV En=-1.36057 14423 9⁳⁻¹⁹⁹⁹⁹⁹⁹ eV r=5.29176 15219⁳¹⁹⁹⁹⁹⁸⁹ m f=-822.46049 0061 THz Eph=-3.40142 18031 3 eV λ=-364.50682 0234 nm ]
'ROOT(ⒺBohr Atomic Model;[Enp;En;r;f;Eph;λ];[1_eV;1_eV;1_m;1_THz;1_eV;1_nm])'
```

## Nuclear Physics
The 52 variables in the Nuclear Physics section are:

* `λ`: Decay constant (Radioactivity) (dim.: 1/time, in SI: s^-1)
* `Δm`: Mass default
* `A`: Total activity at time `t` ([Radioactivity](#Radioactivity)) (dim.: number of decay/time, in SI: becquerel, Bq), or Mass number ([Nuclear Physics](#Nuclear Physics))
* `abdaughter`: Abundance of the daughter nuclide
* `abparent`: Abondance of the parent radionuclide
* `abtot`: Total abondance of the nuclide
* `AY1`: Mass number of the first nuclide `Y1` produced by a fission
* `AY2`: Mass number of the second nuclide `Y2` produced by a fission
* `AXα`: Mass number of the radionuclide `X` undergoing α decay
* `AYα`: Mass number of the daughter nuclide `Y` from α decay
* `AXβ⊖`: Mass number of the radionuclide `X` undergoing β⊖ decay
* `AYβ⊖`: Mass number of the daughter nuclide `Y` from β⊖ decay
* `AXβ⊕`: Mass number of the radionuclide `X` undergoing β⊕ decay
* `AYβ⊕`: Mass number of the daughter nuclide `Y` from β⊕ decay
* `Aa`: Mass number of the incident nuclide or particle `a`
* `Ab`: Mass number of the product nuclide or particle `b`
* `AX`: Mass number of the reactant nuclide `X`
* `AY`: Mass number of the product nuclide `Y`
* `A0`: Initial total activity (dim.: number of decay/time, in SI: becquerel, Bq)
* `EB`: Nuclear binding energy (dim.: charge·voltage, in SI: MeV)
* `EBse`: Semiempirical nuclear binding energy (dim.: charge·voltage, in SI: MeV)
* `K`: Kinetic energy of the emitted `α` particle (dim.: charge·voltage, in SI: MeV)
* `Kmax`: Maximum kinetic energy of the electron ([β⊖ Decay](#β⊖ Decay)), or of the positron ([β⊕ Decay](#β⊕ Decay)) (dim.: charge·voltage, in SI: MeV)
* `ΔKtot`: Variation of total kinetic energy (dim.: charge·voltage, in SI: MeV)
* `ma`: Mass of incident nuclide or particle `a` (In SI: u)
* `mb`: Mass of outgoing nuclide or particle `b` (In SI: u)
* `mX`: Mass of reactant neutral atom `X` (In SI: u)
* `mY`: Mass of product neutral atom `X` (In SI: u)
* `MW`: Mass weight (dim.: mass/mol, in SI: g/mol)
* `N`: Number of nuclide at time `t` ([Radioactivity](#Radioactivity)), or Number of neutron ([Nuclear Physics](#Nuclear Physics))
* `nn`: Number of neutrons produced in a fission reaction
* `N0`: Initial number of nuclide
* `Q`: Reaction energy of a nuclear reaction (dim.: charge·voltage, in SI: MeV)
* `Qα`: Reaction energy of an α decay (dim.: charge·voltage, in SI: MeV)
* `Qβ⊖`: Reaction energy of a β⊖ decay (dim.: charge·voltage, in SI: MeV)
* `Qβ⊕`: Reaction energy of a β⊕ decay (dim.: charge·voltage, in SI: MeV)
* `R`: Radius of the nucleus having `A` nucleons
* `t`: Age of the decaying nuclide
* `Thl`: Half-life of radionuclide (dim.: time)
* `Z`: Number of proton
* `ZXα`: Proton number of the radionuclide `X` undergoing α decay
* `ZYα`: Proton number of the daughter nuclide `Y` from α decay
* `ZY1`: Proton number of the first nuclide `Y1` produced by a fission
* `ZY2`: Proton number of the second nuclide `Y2` produced by a fission
* `ZXβ⊖`: Proton number of the radionuclide `X` undergoing β⊖ decay
* `ZYβ⊖`: Proton number of the daughter nuclide `Y` from β⊖ decay
* `ZXβ⊕`: Proton number of the radionuclide `X` undergoing β⊕ decay
* `ZYβ⊕`: Proton number of the daughter nuclide `Y` from β⊕ decay
* `Za`: Proton number of the incident nuclide or charge of the incident particle `a`
* `Zb`: Proton number of the incident nuclide or charge of the product particle `b`
* `ZX`: Proton number of the reactant nuclide `X`
* `ZY`: Proton number of the product nuclide `Y`

For all nuclear reactions, including nuclear decays, we have charge conservation `Zp = Z` and mass number conservation `Mp = M` (the same number of nucleons). This therefore requires assigning numbers `A` and `Z` to incident particles `a` and `b` whether they are for example gamma photons or neutrinos (`A = 0`, `Z = 0`), positrons (`A = 0`, `Z = +1`) or others. The reaction energy `Q` is always calculated with the mass-energy equivalence by the mass default `Δm` between the reactants and the products. For instance, spontaneous decays and fissions are always exothermic `Q > 0` while some nuclear reactions can also be endothermic `Q < 0` (like nucleosynthesis of heavy elements). The mass difference should be computed with at least 9 significant digits because reactants and products masses can be of the same order of magnitude.

#### Radioactivity

* **Example 1** For a sample of 1 g of radium, to calculate `[Thl_s;abtot;N0;A0_Bq;N;A_Bq]` (Half-life of radionuclide, Total abundance, Initial number of nuclide, Initial total activity; Final number of nuclide at time `t`; Final total activity at time `t`) from 6 known variables:
```rpl
m=1_g  MW=226_(g/mol)  λ=1.37364 03205 5⁳⁻¹¹_s^-1  abparent=1  abdaughter=1  t=400_yr
@ Expecting [ Thl=5.04606 02400 1⁳¹⁰ s abtot=1 N0=2.66466 40531⁳²¹ A0=3.66028 99840 5⁳¹⁰ Bq N=2.24047 19403 2⁳²¹ A=3.07760 25942 9⁳¹⁰ Bq ]
'ROOT(ⒺRadioactivity;[Thl;abtot;N0;A0;N;A];[1_s;1;1;1_Bq;1;1_Bq])'
```
* **Example 2** For the C14 datation (present in the atmosphere) of a 10 g sample of carbon having an activity of 30 decays/min, to calculate `[λ_s^-1;abtot;N0;A0_Bq;t_yr]` (Decay constant; Total abundance, Initial number of nuclide, Initial total activity; Final number of nuclide at time `t`; Age of the sample) from 6 known variables:
```rpl
m=10_g  MW=12.01_(g/mol)  Thl=5730_yr  abparent=0.989  abdaughter=1.3e-12  A=30_min^-1
@ Expecting [ λ=3.83332 95627⁳⁻¹² s⁻¹ abtot=1.2857⁳⁻¹² N0=6.44684 96046 1⁳¹¹ A0=2.47128 99175 6 Bq t=13 209.16426 31 yr ]
'ROOT(ⒺRadioactivity;[λ;abtot;N0;A0;t];[1_s^-1;1;1;1_Bq;1_yr])'
```

#### Radius & Binding Energy

* **Example 1** For the C12, to calculate `[A;R_m;V_m^3;EB_MeV;EBse_MeV]` (Mass number of the nuclide; Radius & Volume of the nucleus; Binding energy; Semiempirical binding energy) from 3 known variables:
```rpl
N=6  Z=6  mX=12_u
@ Expecting [ A=12 R=2.74731 41821 3⁳⁻¹⁵ m V=8.68587 53686 5⁳⁻⁴⁴ m↑3 EB=92.16169 75587 MeV EBse=89.61225 87145 MeV ]
'ROOT(ⒺRadius & Binding Energy;[A;R;V;EB;EBse];[1;1_m;1_m^3;1_MeV;1_MeV])'
```
* **Example 2** For the U238, to calculate `[A;R_m;V_m^3;EB_MeV;EBse_MeV]` (Mass number of the nuclide; Radius & Volume of the nucleus; Binding energy; Semiempirical binding energy) from 3 known variables:
```rpl
N=92  Z=146  mX=238.0507847_u
@ Expecting [ A=238 R=7.43658 53216 9⁳⁻¹⁵ m V=1.72269 86147 8⁳⁻⁴² m↑3 EB=1 759.44468 491 MeV EBse=346.41011 9506 MeV ]
'ROOT(ⒺRadius & Binding Energy;[A;R;V;EB;EBse];[1;1_m;1_m^3;1_MeV;1_MeV])'
```

#### α Decay

* **Example 1** For the α decay of U238 into Th234, to calculate `[N;Δm_u;Qα_MeV;Kα_MeV;γ;β;AYα;ZYα]` (Number of neutron of U238; Mass default; Net energy balance; Kinetic energy of the α particle, Lorentz factor; Relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=238  Z=92  AXα=238  ZXα=92  mX=238.0507847_u  mY=234.0436014_u
@ Expecting [ N=146 Δm=0.00458 00458 7 u Qα=4.26628 38693 5 MeV Kα=4.19573 64506 9 MeV γ=1.00112 53451 3 β=4.74014 42424 1⁳⁻² AYα=234 ZYα=90 ]
'ROOT(Ⓔα Decay;[N;Δm;Qα;Kα;γ;β;AYα;ZYα];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```
* **Example 2** For the α decay of Pu239 into U235, to calculate `[N;Δm_u;Qα_MeV;Kα_MeV;γ;β;AYα;ZYα]` (Number of neutron of Pu239; Mass default; Net energy balance; Kinetic energy of the α particle, Lorentz factor; Relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=239  Z=94  AXα=239  ZXα=94  mX=239.052157_u  mY=235.043924_u
@ Expecting[ [ N=145 Δm=0.0064 u Qα=5.9614 MeV Kα=5.86324 MeV γ=1.00157 β=0.05684 AYα=235 ZYα=92 ]
'ROOT(Ⓔα Decay;[N;Δm;Qα;Kα;γ;β;AYα;ZYα];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### β⊖ Decay

The β⊖ Decay reaction has 3 products: the daughter nuclide, an electron & an antineutrino having a negligible mass. Due to their weak masses, the resulting kinetic energy is essentially split between the last two particles. The energy spectrum of the antineutrino being continuous and possibly very small, the energy reaction can therefore be the maximum kinetic energy `Kmax` of the emitted electron.

* **Example 1** For the β⊖ decay of Th234 into Pa234, to calculate `[N;Δm_u;Qβ⊖_MeV;Kmax_MeV;γmax;βmax;AYβ⊖;ZYβ⊖]` (Number of neutron of Th234; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=234  Z=90  AXβ⊖=234  ZXβ⊖=90  mX=234.0436014_u  mY=234.0433085_u
@ Expecting [ N=144 Δm=0.00029 29 u Qβ⊖=0.27283 45044 57 MeV Kmax=0.27283 45044 57 MeV γmax=1.53392 40376 3 βmax=0.75828 53739 86 AYβ⊖=234 ZYβ⊖=91 ]
'ROOT(Ⓔβ⊖ Decay;[N;Δm;Qβ⊖;Kmax;γmax;βmax;AYβ⊖;ZYβ⊖];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```
* **Example 2** For the β⊖ decay of C14 into N14, to calculate `[N;Δm_u;Qβ⊖_MeV;Kmax_MeV;γmax;βmax;AYβ⊖;ZYβ⊖]` (Number of neutron of C14; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=14  Z=6  AXβ⊖=14  ZXβ⊖=6  mX=14.0032419884_u  mY=14.00307400443_u
@ Expecting [ N=8 Δm=0.00016 79839 7 u Qβ⊖=0.15647 60096 MeV Kmax=0.15647 60096 MeV γmax=1.30621 60447 9 βmax=0.64335 24392 36 AYβ⊖=14 ZYβ⊖=7 ]
'ROOT(Ⓔβ⊖ Decay;[N;Δm;Qβ⊖;Kmax;γmax;βmax;AYβ⊖;ZYβ⊖];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### β⊕ Decay

The β⊕ Decay reaction has 3 products: the daughter nuclide, a positron & a neutrino having a negligible mass. Due to their weak masses, the resulting kinetic energy is essentially split between the last two particles. The energy spectrum of the neutrino being continuous and possibly very small, the energy reaction can therefore be the maximum kinetic energy `Kmax` of the emitted positron.

* For the β⊕ decay of K40 (present in bananas) into Ar40, to calculate `[N;Δm_u;Qβ⊕_MeV;Kmax_MeV;γmax;βmax;AYβ⊕;ZYβ⊕]` (Number of neutron of K40; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=40  Z=19  AXβ⊕=40  ZXβ⊕=19  mX=39.963998166_u  mY=39.9623831237_u
@ Expecting [ N=21 Δm=5.17882 48191 5⁳⁻⁴ u Qβ⊕=0.48240 42687 62 MeV Kmax=0.48240 42687 62 MeV γmax=1.94404 20135 3 βmax=0.85755 50653 48 AYβ⊕=40 ZYβ⊕=18 ]
'ROOT(Ⓔβ⊕ Decay;[N;Δm;Qβ⊕;Kmax;γmax;βmax;AYβ⊕;ZYβ⊕];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### General Nuclear Reaction

* **Example 1**  For the nuclear reaction: α + N14 → O17 + p (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of N14; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of the daughter nuclide) from 12 known variables (note: to balance `Z`, the α & p are replaced here by the neutral atoms He4 & H1):
```rpl
A=14  Z=7  AX=14  ZX=7  Aa=4  Ab=1  Za=2  Zb=1  mX=14.00307400443_u  mY=16.99913175650_u  ma=4.00260325413_u  mb=1.00782503223_u
@ Expecting [ N=7 Δm=-0.00127 95301 7 u Q=-1.19187 42911 2 MeV ΔKtot=-1.19187 42911 2 MeV AY=17 ZY=8 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 2**  For the nuclear reaction: α + Al27 → P30 + n (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of Al27; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of the daughter nuclide) from 12 known variables (note: to balance `Z`, α is replaced by the neutral atom He4):
```rpl
A=27  Z=13  AX=27  ZX=13  Aa=4  Ab=1  Za=2  Zb=0  mX=14.00307400443_u  mY=16.99913175650_u  ma='ⒸmHe'  mb='Ⓒmn'
@ Expecting [ N=14 Δm=-2.11941 39960 6⁳⁻³ u Q=-1.97422 07830 5 MeV ΔKtot=-1.97422 07830 5 MeV AY=30 ZY=15 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 3**  For the fusion nuclear reaction: D2 + Li6 → He4 + He4 (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of Li6; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of one of the daughter nuclide) from 12 known variables :
```rpl
A=6  Z=3  AX=6  ZX=3  Aa=2  Ab=4  Za=1  Zb=2  mX=6.0151228874_u  mY=4.00260325413_u  ma=2.01410177812_u  mb='ⒸmHe'
@ Expecting [ N=3 Δm=0.02401 81572 6 u Q=22.37276 21509 MeV ΔKtot=22.37276 21509 MeV AY=4 ZY=2 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```

#### Fission Reaction

* **Example 1** For the fission reaction: n + U235 → Xe140 + Sr94 + 2n (represented as n + X → Y1 + Y2 + `nn`*n where `nn` is the number of produced neutrons), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY1;ZY1]` (Number of neutron of U235; Mass default; Reaction energy; Variation of total kinetic energy; Mass number & Proton number of one of the two products of fission, Xe140) from 10 known variables:
```rpl
A=235  Z=92  AX=235  ZX=92  AY2=94  ZY2=38  mX=235.043924_u  mY1=139.9216458_u  mY2=93.915361_u  nn=2
@ Expecting [ N=143 Δm=0.19825 22839 44 u Q=184.67075 332 MeV ΔKtot=184.67075 332 MeV AY1=140 ZY1=54 ]
'ROOT(ⒺFission Reaction;[N;Δm;Q;ΔKtot;AY1;ZY1];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 2** For the fission reaction: n + Pu239 → Xe134 + Zr103 + 3n (represented as n + X → Y1 + Y2 + `nn`*n where `nn` is the number of produced neutrons), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY1;ZY1]` (Number of neutron of Pu239; Mass default; Reaction energy; Variation of total kinetic energy; Mass number & Proton number of one of the two products of fission, Xe134) from 10 known variables:
```rpl
A=239  Z=94  AX=239  ZX=94  AY2=103  ZY2=40  mX=239.052157_u  mY1=133.90539466_u  mY2=102.926597_u  nn=3
@ Expecting [ N=145 Δm=0.20283 55078 88 u Q=188.93999 7545 MeV ΔKtot=188.93999 7545 MeV AY1=134 ZY1=54 ]
'ROOT(ⒺFission Reaction;[N;Δm;Q;ΔKtot;AY1;ZY1];[1;1_u;1_MeV;1_MeV;1;1])'
```


## Finance

The `Finance` section contains equations to compute time value of money (TVM).
The variables in the Finance section include:

* `n`: The number of compounding periods. For example, a 30 years loan with
       monthly payments has 360 compounding periods.
* `I%Yr`: Interest rate per year in percent
* `PYr`: Payments per year.
* `Pmt`: The periodic payment amount.
* `PV`: The present value of a series of future cash flows,
        or the initial cash flow.
* `FV`: The future value, that is the final cash flow
        (balloon payment or remaining balance),
        or the compounded value of a series of prior cash flows.

### TVMBeg

Time value of money equation when payments are made at the beginning of the
period.

```rpl
n='9*12' I%Yr=5.75 PV=-155 Pmt=0 PYr=12
@ Expecting FV=259.74210 1025
'ROOT(ⒺTVMBeg;FV;0)'
```

### TVMEnd

Time value of money equation when payments are made at the end of the period.

```rpl
n='5*12' I%Yr=13 PV=-63000 FV=10000 PYr=12
@ Expecting Pmt=1 314.24620 468
'ROOT(ⒺTVMEnd;Pmt;0)'
```
# Menus

Menus display at the bottom of the screen, and can be activated using the keys
on the top row of the calculator. Menus can refer to other menus. The calculator
keeps a history of the menus you visited previously, and you can return to an
earlier menu with the `BackMenu` function.


Here are the main menus in DB48X, in alphabetical order.

## MainMenu

The *Main menu* gives access to _all_ the functions in your calculator, sorted
by cathegory. It includes the following submenus:

* [Math](#MathMenu): Mathematical operations
* [Symb](#SymbolicMenu): Symbolic operations
* [Units](#UnitsMenu): Unit conversions
* [System](#SystemMenu): System configuration
* [Prog](#ProggramMenu): Programming
* [Vars](#VariablesMenu): User variables


## MathMenu

The *Math menu* gives access to mathematical functions like [SIN](#sin) in your
calculator. It includes the following submenus:

* [Arith](#ArithmeticMenu): Arithmetic functions
* [Base](#BaseMenu): Based numbers
* [Trans](#TranscendentalMenu): Transcendental functions
* [Stats](#StatisticsMenu): Statistics
* [Lists](#ListsMenu): List operations
* [Matrix](#MatrixMenu): Matrices and vectors
* [Solve](#SolverMenu): Numerical solver


## VariablesMenu

The variables menu, accessed using the _VAR_ key, displays the variables in the
current directory. It shows three rows for each variable:

* The primary function [evaluates the variable](#VariablesMenuExecute)
* The first shifted function [recalls the variable](#VariablesMenuRecall)
* The second shifted function [stores in the variable](#VariablesMenuStore)

## VariablesMenuExecute

Hitting the primary function in the [Vars menu](#VariablesMenu) evaluates the
corresponding variable.

## VariablesMenuRecall

Hitting the first shifted function in the [Vars menu](#VariablesMenu) will
[recall](#Recall) the corresponding variable on the stack.

## VariablesMenuStore

Hitting the second shifted function in the [Vars menu](#VariablesMenu) will
[store](#Store) the top of stack in the corresponding variable.


## ToolsMenu

The `ToolsMenu` invokes a context-dependent menu adapted to the top level of the
stack.


## LastMenu

The `LastMenu` function (🟨 _A_), returns back in the history of past visited menus. The history contains up to 8 entries.
# Operations with Angles

## →Degrees
Convert a number or angle to an angle in degrees.
If given a number, that number is interpreted using the current angle mode.


## →Radians
Convert a number or angle to an angle in radians.
If given a number, that number is interpreted using the current angle mode.


## →Grads
Convert a number or angle to an angle in grads.
If given a number, that number is interpreted using the current angle mode.

## →PiRadians
Convert a number or angle to an angle in multiple of π radians.
If given a number, that number is interpreted using the current angle mode.


## R→D

Convert radians to degrees.

The `RadiansToDegrees` command converts a value from radians to degrees.
The input is interpreted as radians, and the result is in degrees.


## D→R

Convert degrees to radians.

The `DegreesToRadians` command converts a value from degrees to radians.
The input is interpreted as degrees, and the result is in radians.
# Arithmetic

## Add

Add two values.

`Y` `X` ▶ `Y+X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical addition. For example, `1 2 +` is `3`.
* For equations and symbols, build a sum, eliminating zero additions if
  [autosimplify](#autosimplify) is active.
* For lists, concatenate lists, or add objects to a list. For example, `{ A } { B
  } +` is `{ A B }`, and `{ A B C } "D" +` is `{ A B C "D" }`.
* For text, concatenate text, or concatenate the text representation of an
  object to an existing text. For example `"X" "Y" + ` gives `"XY"`, and
  `"X=" 1 +` gives `"X=1"`.

*Note*: The ` Add ` command and the ` + ` operation are identical on DB48X.
[This differs from HP calculators](#list-operation-differences).



## Subtract

Subtract two values

`Y` `X` ▶ `Y-X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical subtraction. For example, `1 2 -` is `-1`.
* For equations and symbols, build a difference, eliminating subtraction of 0 if
  [autosimplify](#autosimplify) is active.


## Multiply

Multiply two values.

`Y` `X` ▶ `Y×X`

* For integer, fractional, decimal or complex numbers, this performs the
  expected numerical multiplication. For example, `3 2 *` is `6`.
* For vectors, multiply individual elements (this is a deviation from HP48).
  For example, `[ 1 2 3 ] [ 4 5 6 ] +` is `[ 4 10 18 ]`.
* For matrices, perform a matrix multiplication.
* For a matrix and a vector, apply the matrix to the vector.
* For equations and symbols, build a product, eliminating multiplication by 1
  or 0 when [autosimplify](#autosimplify) is active.
* For a list and a positive integer, repeat the list For example, `{ A } 3 *`
  is `{ A A A }`.
* For a text and a positive integer, repeat the text. For example `"X" 3 * `
  gives `"XXX"`.


## Divide

Divide two values

`Y` `X` ▶ `Y÷X`

* For integer, build a fraction. For example `1 7 /` gives `1/7`.
* For fractional, decimal or complex numbers, this performs the
  expected numerical division. For example, `1. 2. /` is `0.5`.
* For vectors, divide individual elements. For example,
  `[ 1 2 3 ] [ 3 2 1 ] /` is `[ 1/3 1 3 ]`.
* For equations and symbols, build a ratio, eliminating division by one
  and division of 0 when [autosimplify](#autosimplify) is active.


## Pow

Raise to the power

`Y` `X` ▶ `Y^X`

* For integer, fractional, decimal or complex numbers, this raises the
  value in level 2 to the value in level 1. For example, `2 3 ↑` is `8`.
* For vectors, raise individual elements in the first vector to the power of the
  corresponding element in the second vector.
* For equations and symbols, build an expression, eliminating special cases
  when [autosimplify](#autosimplify) is active.


## xroot

Raise to the inverse power. `X Y xroot` is equivalent to `X Y inv pow`.

`Y` `X` ▶ `Y↑(1/X)`


## Floor

Largest integer less than the input


## Ceil

Smallest integer larger than the input


## IntPart

Integer part of a number


## FracPart

Fractional part of a number


## Abs

Return the absolute value for a real number.
Return the modulus for a complex number
Return the absolute value of elements for a matrix or a vector, which is different from HP calculators, see `NORM`


# Integer arithmetic and polynomials

This section documents integer arithmetic commands, including prime-related
operations. Some commands (e.g. MODSTO, GETPREC) are planned but not yet
implemented in DB48X.

## SETPREC
Set the current system precision


## GETPREC
Get the current system precision


## MODSTO
Set the current system modulo for all MOD operations


## MODRCL
Get the current system modulo


## POWMOD
Power operator MOD the current system modulo


## MOD
Remainder of the integer division


## SQ
Square of the input


## Factorial
Factorial of a number


## MANT
Mantissa of a real number

```rpl
1.2345E123 MANT
@Expecting 1.2345
```


## XPON
Exponent of a real number

```rpl
1.2345E123 XPON
@Expecting 123
```


## SigDig
Number of significant digits in a real number

```rpl
1.2345E123 SigDig
@Expecting 5
```

## Sign

Sign of a number, -1, 0 or 1.

For complex numbers, returns a unit number on the unit circle with the same
argument as the original number.


## Percent

Percentage of a number

`Y` `X` ▶ `Y×(X÷100)`


## PercentChange

Percentage of change on a number

`Y` `X` ▶ `(X÷Y-1)×100`


## PercentTotal

Get percentage of a total

`Y` `X` ▶ `(X÷Y)×100`


## GCD
Greatest common divisor of two integers.

`Y` `X` `GCD` → `gcd(X,Y)`

The result is always non-negative. `gcd(0,0)` is `0`.


## LCM
Least common multiple of two integers.

`Y` `X` `LCM` → `lcm(X,Y)`

The result is always non-negative. If either argument is `0`, the result is `0`.


## IQUOT
Quotient of the integer division


## ADDTMOD
Addition operator MOD the current system modulo


## SUBTMOD
Subtraction operator MOD the current system modulo


## MULTMOD
Multiplication operator MOD the current system modulo


## PEVAL
Evaluation of polynomial given as vector of coefficients


## PCOEF
Coefficients of monic polynomial with the given roots


## IEGCD
Extended euclidean algorithm


## IABCUV
Find integers u,v to solve a*u+b*v=c


## PTCHEBYCHEFF
Nth Tchebycheff polynomial


## PLEGENDRE
Nth Legendre polynomial


## PHERMITE
Nth Hermite polynomial as used by physics


## PTCHEBYCHEFF2
Nth Tchebycheff polynomial of the second kind


## PHERMITE2
Nth Hermite polynomial as used in probabilities


## QuotientRemainder

Euclidean division, returning quotient and remainder.

On HP calculators, this only applies for polynomials.

On DB48X, this applies to integers, big integers, decimals, fractions and
polynomials. It puts both the quotient and the remainder on the stack. For many
data types, this operation is significantly more efficient than computing the
quotient and the remainder separately.

`Y` `X` ▶ `IP(Y/X)` `Y rem X`


## PDIV2
Polynomial euclidean division as coefficient vector


## PDER
Derivative of polynomial as coefficient vector


## PINT
Integration of polynomials as coefficient vector


## PMUL
Multiplication of polynomials as coefficient vectors


## PADD
Addition of polynomials as coefficient vector


## PSUB
Subtraction of polynomials as coefficient vector


## Min
Smallest of 2 objects


## Max
Largest of 2 objects


## RND
Round a number to the given number of figures


## TRNC
Truncate a number to the given number of figures


## DIGITS
Extract digits from a real number


## PROOT
All roots of a polynomial


## IsPrime

Test whether an integer is prime.

`n` ▶ `True` or `False`

* Returns `True` if the integer `n` is prime, `False` if composite.
* Accepts integer inputs, including bignums. The maximum size is controlled by
  the `MaxFactorsBits` setting.
* Uses trial division by small primes first, then Miller-Rabin for larger
  values.
* Deterministic for numbers up to about 82 bits; probabilistic beyond that with
  negligible error probability.

```rpl
17 IsPrime
@ Expecting True
```

```rpl
15 IsPrime
@ Expecting False
```

```rpl
561 IsPrime
@ Expecting False
```


## Factors

Decompose an integer into its prime factorization.

`n` ▶ `{ p₁ e₁ p₂ e₂ … }`

* Returns a list of alternating prime-exponent pairs: each prime factor followed
  by its multiplicity.
* Accepts integer inputs, including bignums. Zero and one return an empty list.
  The maximum size is controlled by the `MaxFactorsBits` setting.
* Uses trial division by small primes, then Pollard's Rho for larger factors.
* The `MaxFactorIterations` setting limits Pollard's
  Rho iterations per attempt; lowering it can avoid long runs on hard semiprimes.
* The product of all `pᵢ^eᵢ` equals the original number.

```rpl
12 Factors
@ Expecting { 2 2 3 1 }
```

```rpl
100 Factors
@ Expecting { 2 2 5 2 }
```


## NextPr

Return the smallest prime strictly greater than the input.

`n` ▶ `p`

* Returns the next prime number after `n`.
* Accepts integers ≥ 1. The maximum input size is controlled by the
  `MaxFactorsBits` setting.
* If no prime exists (e.g. search limit reached), returns an error.

```rpl
10 NextPrime
@ Expecting 11
```

```rpl
2 NextPrime
@ Expecting 3
```


## PrevPr

Return the largest prime strictly smaller than the input.

`n` ▶ `p`

* Returns the previous prime number before `n`.
* Accepts integers > 2 (no prime exists below 2). The maximum input size is
  controlled by the `MaxFactorsBits` setting.
* If no prime exists or the search limit is reached, returns an error.

```rpl
10 PreviousPrime
@ Expecting 7
```

```rpl
3 PreviousPrime
@ Expecting 2
```
# Base functions

## Evaluate

Evaluate the object at stack level 1.

Mapped to the _ R/S _ key

`X` ▶ Result of `X` evaluation

## Negate

Negate the value in level 1.

Mapped to the _ +/- _ key

`X` ▶ `0-X`

## Inv

Invert the value in level 1

Mapped to the _ 1/X _ key

`X` ▶ `1/X`


# Bitwise operations

Bitwise operations represent bit-manipulation operations such as rotations and
shifts. They operate on [based numbers](#based-numbers),
[integers](#integers) or [big integers](#big-integers). When operating on based
numbers, the operation happens on the number of bits defined by the
[WordSize](#wordsize) setting. For integer values, the maximum number of bits is
defined by the [MaxNumberBits](#maxnumberbits) setting.

## ShiftLeft

Shift the value left by one bit.

`Value` ▶ `Value*2`

## ShiftLeftByte

Shift the value left by one byte (8 bits).

`Value` ▶ `Value*256`

## ShiftLeftCount

Shift the value left by a given number of bits.

`Value` `Shift` ▶ `Value*2^Shift`

## ShiftRight

Shift the value right by one bit.

`Value` ▶ `Value/2`

## ShiftRightByte

Shift the value right by one byte (8 bits).

`Value` ▶ `Value/256`

## ShiftRightCount

Shift the value right by a given number of bits.

`Value` `Shift` ▶ `Value/2^Shift`

## ArithmeticShiftRight

Shift the value right by one bit, preserving the sign bit.

`Value` ▶ `Signed(Value)/2`

## ArithmeticShiftRightByte

Shift the value right by one byte (8 bits), preserving the sign bit.

`Value` ▶ `Signed(Value)/256`

## ArithmeticShiftRightCount

Shift the value right by a given number of bits, preserving the sign bit.

`Value` `Shift` ▶ `Signed(Value)/2^Shift`

## RotateLeft

Rotate the value left by one bit.

`Value`  ▶ `RLC(Value, 1)`


## RotateLeftByte

Rotate the value left by one byte (8 bits).

`Value`  ▶ `RL(Value, 8)`

## RotateLeftCount

Rotate the value left by a given number of bits.

`Value`  `Shift` ▶ `RLC(Value, Shift)`


## RotateRight

Rotate the value right by one bit.

`Value`  ▶ `RRC(Value, 1)`

## RotateRightByte

Rotate the value right by one byte (8 bits).

`Value`  ▶ `RRC(Value, 8)`

## RotateRightCount

Rotate the value right by a given number of bits.

`Value` `Shift` ▶ `RRC(Value, Shift)`

# Logical operations

Logical operations operate on [truth values](#boolean-values).
They can either operate on numbers, where a non-zero value represent `True` and
a zero value represents `False`. On [based numbers](#based-numbers), they
operate bitwise on the number of bits defined by the [WordSize](#wordsize)
setting.

## Or

Logical inclusive "or" operation: the result is true if either input is true.

`Y` `X` ▶ `Y or X`


## And

Logical "and" operation: the result is true if both inputs are true.

`Y` `X` ▶ `Y and X`

## Xor

Logical exclusive "or" operation: the result is true if exactly one input is
true.

`Y` `X` ▶ `Y xor X`


## Not

Logical "not" operation: the result is true if the input is false.

`X` ▶ `not X`


## NAnd

Logical "not and" operation: the result is true unless both inputs are true.

`Y` `X` ▶ `Y nand X`


## NOr

Logical "not or" operation: the result is true unless either input is true.

`Y` `X` ▶ `Y nor X`

## Implies

Logical implication operation: the result is true if the first input is false or
the second input is true.

`Y` `X` ▶ `Y implies X`

## Equiv

Logical equivalence operation: the result is true if both inputs are true or
both inputs are false.

`Y` `X` ▶ `Y equiv X`

## Excludes

Logical exclusion operation: the result is true if the first input is true or
the second input is false.

`Y` `X` ▶ `Y excludes X`
# Bitmaps

## TOSYSBITMAP

# Comparisons

Comparison operators take two inputs and return a truth value,
`True` or `False`.

For compound objects, lexicographical order is used.
For text, Unicode code point order is used.


## True

Represent a logical truth, the opposite of `False`.

## False

Represent a logical untruth, the opposite of `True`.


## =

Checks if two values are mathematically equal.
This does not check the type of the object but its value.
The `=` comparison operator can be used to form equations such as `'X=1'`, for
example for use with the `Root` numerical solver.

```rpl
@ Expecting True
'1=2.0-1.0' Evaluate
```

In most cases, `=` is the best replacement for `==` in programs written for
HP variants of RPL.


## ≠

Check if two values are mathematically different. This is the opposite of `=`.

```rpl
@ Expecting False
'1≠2.0-1.0' Evaluate
```

## ==

Checks if two objects are identical after evaluating names
(contrary to `same` which does not evaluate names).

```rpl
A=1
'A==1' Evaluate
@ Expecting True
```

Unlike `=`, `==` takes the type of the object into account, meaning that
`'1==1.0'` evaluates as `False`.

```rpl
'1==2.0-1.0' Evaluate
@ Expecting False
```

## Same

Checks if two objects are identical without evaluating names
(contrary to `==`).

```rpl
A=1
'A' 1 SAME
@ Expecting False
```


## <

Check if the first value is less than the second value.

```rpl
{}
1 2 < + 2.3 1.2 < +
"ABC" "DEF" < + "a" "à" < +
{ 1 2 3 } { 1 2 4 } < + { 1 2 3 } { 1 2 3 } < +
@ Expecting { True False True True True False }
```

## ≤

Check if the first value is less than or equal to the second value.

```rpl
{}
1 2 ≤ + 2.3 1.2 ≤ +
"ABC" "DEF" ≤ + "a" "à" ≤ +
{ 1 2 3 } { 1 2 4 } ≤ + { 1 2 3 } { 1 2 3 } ≤ +
@ Expecting { True False True True True True }
```

## >

Check if the first value is greater than the second value.

```rpl
{}
1 2 > + 2.3 1.2 > +
"ABC" "DEF" > + "a" "à" > +
{ 1 2 3 } { 1 2 4 } > + { 1 2 3 } { 1 2 3 } > +
@ Expecting { False True False False False False }
```

## ≥

Check if the first value is greater than or equal to the second value.

```rpl
{}
1 2 ≥ + 2.3 1.2 ≥ +
"ABC" "DEF" ≥ + "a" "à" ≥ +
{ 1 2 3 } { 1 2 4 } ≥ + { 1 2 3 } { 1 2 3 } ≥ +
@ Expecting { False True False False False True }
```
# Operations with Complex Numbers

## Re
Real part of a complex number

## Im
Imaginary part of a complex number

## Arg
Argument of a complex number

## Conj
Conjugate of a complex number

## RectangularToReal
Extract real and imaginary parts from a complex number in rectangular form

## RealToRectangular
Make a complex number in rectangular form from real and imaginary part

## PolarToReal
Extract modulus and argument from a complex number in polar form

## RealToPolar
Make a complex number in polar form from argument and modulus

## ToRectangular
Convert a complex number or vector to rectangular (cartesian) form.

## ToPolar
Convert a complex number or a 2D or 3D vector to polar form.

## ToCylindrical
Convert a 3D vector to cylindrical form

## ToSpherical
Convert a 3D vector to spherical form.

## To2DVector
Make a 2D vector from two components

## To3DVector
Make a 3D vector from three components

## FromVector
Expand a vector into its individual components
# Lists, Matrix and String commands

## PUTI
Replace an item and increase index


## GETI
Extract an item and increase index


## OBJDECOMP
Explode an object into its components


## REPL
Replace elements in a composite


## POS
Find the position of an element in a composite


## NPOS
Find object in a composite, starting from index N


## POSREV
Find the position of an element, starting from the end


## NPOSREV
Find the position from the end, starting at index N


## Extract

Extract a group of elements from a composite


## SIZE
Number of elements in a composite


## RHEAD
Returns the last element from the composite


## RTAIL
Removes the last element from the composite

# Constants

Constants are defined by the `config/constants.csv`, and accessed using
the `ConstantsMenu` or the `CONST` command.

Library equations are defined by the `config/equations.csv`, and accessed using
the `EquationsMenu` or the `LibEq` command.

Library items are defined by the `config/library.csv`, and accessed using the
`Library` command or the `XLib` command.


## pi

Return the π constant (approximately 3.14159).

By default, this command returns a symbolic constant. The numerical value can be
obtained using `→Num`, or by setting the `NumericalConstants` or
`NumericalResults` flags.

```rpl
pi →Num
@ Expecting 3.14159 26535 9
```

## EulerianNumber

Return the value of Euler's number (approximately 2.71828)

By default, this command returns a symbolic constant. The numerical value can be
obtained using `→Num`, or by setting the `NumericalConstants` or
`NumericalResults` flags.

```rpl
℮ →Num
@ Expecting 2.71828 18284 6
```

## Infinity

Return a positive infinity

By default, this command returns a symbolic constant. The numerical value can be
obtained using `→Num`, or by setting the `NumericalConstants` or
`NumericalResults` flags.

```rpl
infinity
@ Expecting ∞
```

## NegativeInfinity

Return a negative infinity

By default, this command returns a symbolic constant. The numerical value can be
obtained using `→Num`, or by setting the `NumericalConstants` or
`NumericalResults` flags.

```rpl
NegativeInfinity
@ Expecting −∞
```

## Constant

Returns the value of a constant from the constants library.
The name can be given as a symbol or as text.

`'c'` ▶ `299792458_m/s`


## StandardUncertainty

Returns the standard uncertainty of a constant from the constants library.
The name can be given as a symbol or as text. The result is the absolute
uncertainty (denoted Ⓢ*X* in expressions) conforming to CODATA.

`'c'` `StandardUncertainty` ▶ `0_m/s`


## RelativeUncertainty

Returns the relative uncertainty of a constant from the constants library.
The name can be given as a symbol or as text. The result is the relative
uncertainty (denoted Ⓡ*X* in expressions) conforming to CODATA.

`'c'` `RelativeUncertainty` ▶ `0`


## LibraryEquation

Returns the value of a library equation from the equation library.
The name can be given as a symbol or as text.

`"RelativityMassEnergy"` ▶ `"'E=m*c^2'"`


## LibraryItem

Returns the value of a library item from the library.
The name can be given as a symbol or as text.

`'Dedicace'"` ▶ `"À tous ceux qui se souviennent de Maubert électronique"`


## ConstantName

Return the name for a given constant, as text

```rpl
Ⓒπ ConstantName
@ Expecting "π"
```

This also works for library items, library equations, standard and relative
uncertainties.


## ConstantValue

Return the value for a given constant

```rpl
Ⓒπ ConstantValue
@ Expecting 3.14159 26535 9
```

This also works for library items, library equations, standard and relative
uncertainties.

# Precision control

Scientific calculations lead to the numerical evaluation of expressions whose
result is given in the form of a [decimal number](#Decimal-number) for which
[scientific notation](#Entering-a-number-in-scientific-notation-with-_×10ⁿ_) is
often used.

Calculations must be made with sufficient [precision](#Precision) to avoid
rounding errors affecting the validity of the results sought. Since DB48x has
variable precision floating point, the default configuration of 24 digits is
more than enough for most scientific applications.

However, a numerical value `X`, whether a measurement result or a constant, is
most of the time known with limited precision. It is therefore provided with
either an absolute uncertainty `ΔX` which is here designated by standard
uncertainty represented by `UsX`, or, equivalently, with a relative uncertainty
`UrX=UsX/|X|`. This makes it possible to establish an interval (either of a
statistical nature for a given probability distribution or of extreme limits or
other estimations) for the values ​​of `Xval`: `Xval = X ± ΔX = X ± UsX` noted
also as `Xval = X @ (UrX*100)%`.

A constant like `G` when edited shows as `ⒸG`.
Its relative uncertainty `UrG` is shown as `ⓇG` on DB48x, whether on the stack
or while editing. Its standard uncertainty `UsG` is shown as `ⓈG`.

## Calculating uncertainty

The calculation of uncertainty in science (metrology) obeys a certain number of simple rules concerning the significant digits (SD):

### Rule 1

`UsX` and `UrX` must be rounded to count at most 2 SD.

### Rule 2

`UsX` and `UrX` are equivalent and are determined between them knowing the
central value `X`.

### Rule 3

A central value `X` must be rounded so that its last decimal place corresponds
to the last decimal place of `UsX`.

### Rule 4

The final result of a calculation (multiplicative, functional, etc.)  involving
several uncertain values ​​gives a result that cannot be more precise than the
least precise of the uncertain input values. Note that before rounding the
final result, the intermediate calculations can be done at maximum precision,
thus avoiding the accumulation of rounding errors.

### Rule 5

In the case of a sum (or difference) between two uncertain values,
rounding is carried out to the leftmost decimal position of the last
significant digit among the inputs of the final calculation.

### Uncertainty-related commands

DB48x benefits from several features that support uncertainty calculations, SD
display mode and manipulation:

* `SignificantDisplay` is a mode where values can be displayed with a given
  number of SDs regardless of the precision of the calculations.

* `SigDig` is a command (DB48x extension) that returns the number of significant
  digits of its input.

* `Trunc` is a command that truncates its input to a given number of SDs when
  given a negative precision (and to a given number of decimal digits when given
  a positive precision).

* `Round` is a command (from HP50g) that rounds its input in the same way
  `Trunc` truncates it.

If the uncertainty `UsX` and `UrX` result from a calculation, [Rule 1](#rule-1)
is easily implemented by `UsX=ROUND(UsX,-2)` and `UrX=ROUND(UrX,-2)`.

The following five commands are added as extensions of DB48x to support the
remaining 4 rules: `→Us`, `→Ur`, `StandardRound`, `RelativeRound` and
`PrecisionRound`.


## →Us

Calculate standard uncertainty.

This command calculates a standard uncertainty `UsX` given the relative
uncertainty `UrX` and the central value `X`. This implements [Rule 2](#rule-2)
for `UsX`.

```rpl
-3.141592654_m  0.000012  →Us
@ Expecting 0.00003 8 m
```

## →Ur

Calculate relative uncertainty.

This command calculates a relative uncertainty `UrX` given the standard
uncertainty `UsX` and the central value `X`. This implements [Rule 2](#rule-2)
for `UrX`.


```rpl
-3.141592654_m  0.000097_m  →Ur
@ Expecting 0.00003 1
```

## StandardRound

Round a value based on a standard uncertainty, implementing [Rule 3](#rule-3).

```rpl
-3.141592654_m  0.000045_m  StdRnd
@ Expecting -3.14159 3 m
```

```rpl
-3.141592654_m  0.000045  StdRnd
@ Expecting -3.14159 3 m
```

To compute the correct rounding of `Mu='ⒸNA*Ⓒu'`:
```rpl
'ⒸNA*Ⓒu'  Duplicate  ⓇMu  →Us  StdRnd →Num
@ Expecting 1.00000 00010 5⁳⁻³ kg/mol
@ which is the correctly rounded value of ⒸMu
```


## RelativeRound

Round a value based on a relative uncertainty, implementing [Rule 2](#rule-2)
and [Rule 3](#rule-3).

```rpl
-3.141592654_m  0.000012  RelRnd
@ Expecting -3.14159 3 m
```

To calculate `UrMu=ⓇMu` and then the correct rounding of `Mu='ⒸNA*Ⓒu'`,
you can use the following code:
```rpl
'ⒸNA*Ⓒu'  Duplicate  ⓈMu →Ur RelRnd  →Num
@ Expecting 1.00000 00010 5⁳⁻³ kg/mol
@ which is the correctly rounded value of ⒸMu
```

To calculate `Urε₀=Ⓡε₀` and then the correct rounding of
`ε₀='CONVERT(1/(Ⓒc^2·Ⓒμ₀);1_F/m)'`:

```rpl
'CONVERT(1/(Ⓒc^2·Ⓒμ₀);1_F/m)'  Duplicate  Ⓢε₀  →Ur RelRnd  →Num
@ Expecting 8.85418 78188⁳⁻¹² F/m
@ which is the correctly rounded value of Ⓒε₀
```


## PrecisionRound

Round one value to the precision of another one.

This implements [Rule 4](#rule-4) and [Rule 5](#rule-5). The user has to judge
carefully to establish the respective role of `X` and `Y`. There is no automatic
use here, since it depends on the precise nature of the calculation.


```rpl
-3.141592654_m 0.000045_m  PrcRnd
@ Expecting -3.14159 3 m
```

```rpl
-3.141592654_m 0.00045  PrcRnd
@ Expecting -3.14159 m
```
# Debugging

DB48X offers a variety of capabilities to help debugging RPL programs. These are
made available through the [DebugMenu](#debugmenu), which is the menu that
[ToolsMenu](#toolsmenu) selects when a program is on the stack.


## DebugMenu

The debug menu contains operations necessary to debug RPL programs:

* [Debug](#debug)
* [Step](#singlestep)
* [Over](#stepover)
* [Steps](#multiplesteps)
* [Continue](#continue)
* [Halt](#halt)
* [Kill](#kill)
* [Step↑](#stepout)


## Debug

The `Debug` command takes a program or expression as an argument, and starts
debugging execution of it. When a program is halted for debugging, the header
area shows a _♦_ symbol, and the next instruction to be executed is shown above
the stack. When a program is single-stepping, the header area shows a _›_
symbol.

While a program is being debugged, you can use the command line normally and
even execute programs from it. However, only a single debugging session can
exist. If the program being executed halts, e.g. through the `HALT` command or
by being interrupted using the _EXIT_ key, then this is seen as part of the same
debugging session.


## SingleStep

The `SingleStep` command steps through a single instruction in the RPL program.


## StepOver

The `StepOver` command steps over the next instruction. If the next instruction
is a variable name containing a program, execution halts after executing the
program. If the next instruction being shown is a sequence of instructions, for
example the test or the sequence of instructions in tests or loops, then the
entire sequence is evaluated at once.

## StepOut

The `StepOut` command steps out of the current code block.

## MultipleSteps

The `MultipleSteps` instruction evaluates an arbitrary number of steps in the
program being debugged. The number of steps to execute is passed as an argument
in the first level of the stack. For example, `40 MultipleSteps` will execute
the next 40 RPL instructions.

## Continue

The `Continue` command resumes execution of the current RPL program at the
current instruction.

## Halt

The `Halt` instruction interrupts the execution of a program, making it possible
to insert breakpoints at specific locations in a program.

## Kill

The `Kill` instruction stops the execution of the program currently being
debugged.


## Run

The `Run` command is intended to run programs or resume their execution.

* If a program is currently stopped, then it resumes its execution.
  This can be used to resume execution when a program is waiting for input
  from the `Prompt` command.
* Otherwise, `Run` evaluates the top of the stack like `Evaluate`.
# Variables

Variables are named storage for RPL values.

## Store

Store an object into a specified location. For example `2 'ABC' STO` stores the value `2` in a global variable named `ABC`.

`Value` `Name` ▶

The `Value` is copied in a storage location identified by `Name`. The storage location depends on the type of `Name`, which can be quoted in an expression:

* Symbol or integer: The value is stored in a global variable with that name in the current directory, which is created if necessary. Whether integers can be used as variable names depends on the `NumberedVariables` setting.

* Local name: The value is stored in the corresponding local variable.

* Setting name: The value is used for the corresponding setting, as if the setting command had been executed. For example, `16 'Base' STO` has the same effect as `16 Base`.

* Text: `Value` is stored in a named file on the [flash storage](#flash-storage).


## Recall
Recall an object from a specified location. For example `'ABC' RCL` recalls the value from a global variable named `ABC`.

`Name` ▶ `Value`

The `Value` is fetched from a storage location identified by `Name`. The storage location depends on the type of `Name`, which can be quoted in an expression:

* Symbol or integer: The value is fetched from a global variable with that name in the current directory or any enclosing directory. Whether integers can be used as variable names depends on the `NumberedVariables` setting.

* Local name: The value is fetched from the corresponding local variable.

* Setting name: The value is fetched from the corresponding setting. For example, `'Base' RCL` returns the current value as set by `Base`.

* Text: `Value` is fetched from a named file on the [flash storage](#flash-storage)


## Flash storage

SwissMicros calculators have built-in flash storage, that can act as a USB disk when the calculator is connected to a computer. DB48X can read and write to this flash storage using the regular `STO` and `RCL` commands, simply by giving a text containing the file name as the `Name` argument of these commands.

The format of the file depends on how the name ends:

* `.txt`: the value is stored as text.

* `.48s`: the value is stored as source code in text format. This differs from `.txt` files for text objects, which will be quoted.

* `.48b`: the value is stored in version-specific binary format. This format is only guaranteed to be readable by the same firmware version that wrote it, but it is more compact, faster and energy efficient than the source format.

* `.csv`: The value is stored in comma-separated values format. This is mostly interesting for arrays and lists, which can be echanged with spreadsheets and other PC applications that can input or output CSV files.


## STO+
Add a value to the content of a variable


## STO-
Subtract a value from the contents of a variable


## STO×
Multiply the contents of a variable by a value


## STO÷
Divide the contents of a variable by a value


## RCL+
Add the content of a variable to a value on the stack


## RCL-
Subtract the contents of a variable from a value on a stack


## RCL×
Multiply a value on the stack by the contents of a variable
The value in the variable is multiplied on the right, in cases where
multiplication is non-commutative (e.g. matrix multiplication).


## RCL÷
Divide a value on the stack by the contents of a variable


## Increment
Add one to the content of a variable


## Decrement
Subtract one from content of a variable


## Purge

Delete a global variable from the current directory

*Remark*: `Purge` only removes a variable from the current directory, not the
enclosing directories. Since [Recall](#Recall) will fetch variable values from
enclosing directories, it is possible that `'X' Purge 'X' Recall` will fetch a
value for `X` from an enclosing directory. Use [PurgeAll](#PurgeAll) if you want
to purge a variable including in enclosing directories.

## PurgeAll

Delete a global variable from the current directory and enclosing directories.

*Remark*: If a variable with the same name exists in multiple enclosing
directories, `PurgeAll` may purge multiple variables. Use [Purge](#Purge) if you
want to only purge a variable in the current directory.


## CreateDirectory
Create new directory


## PurgeDirectory
Purge entire directory tree


## UpDirectory
Change current directory to its parent


## HomeDirectory
Change current directory to HOME


## DirectoryPath
Get a path to the current directory


## Variables

List all visible variables in a directory

▶ `{ Variables... }`


## TypedVariables

List variables of a specific type

`type` ▶ `{ Variables... }`
`{ types... }` ▶ `{ Variables... }`

See the `Type` command for a list of types.

## Copy

Copy a value into a global variable. This is primarily intended for use in
algebraic notations, although it also works in RPL. The HP50G Advanced Reference
Manual refers to this command as _▶ (Store)_.

```rpl
'(2+5)▶X' EVAL
42 'A' ▶ 1 +
{} X + A + +
@ Expecting { 43 7 42 }
```

The command returns the stored object on the stack. The returned copy is a
direct reference to the stored global object and does not consume additional
memory.

## Assignment

An assignment is an RPL object in the form `Name=Value` that sets a global
variable to a given value. It is intended primarily to facilitate the
interactive use of library equations.

An assignment evaluates `Value`, stores it into `Name`, and puts the original
assignment object on the stack. If the `PushEvaluatedAssignment` setting is
active, then the assignment object pushed on the stack contains the evaluated
value. Otherwise (by default) it contains the unevaluated value.

The `Name=Value` syntax is only accepted outside of expressions. In expressions,
`Name=Value` represents the `=` operator applied to `Name` and `Value`, and is
used among other things to define equations for the solver. If an assignment
object appears in an expression, it will render as `Value▶Name` so that parsing
it would turn it into a `Copy` operation.


### Assignments with the solver

Assignments are useful in conjonction with the solver. For example, the
following example will solve a simple triangle equation for specific values of
`α` and `β`.

```rpl
α=20 β=30
'ROOT(α+β+γ=180;γ;0)' EVAL
@Expecting γ=130.
```

```rpl
β=30 γ=60
'ROOT(α+β+γ=180;α;0)' EVAL
@Expecting α=90.
```

The interactive stack, in combination with the `Eval` or `Edit` buttons, makes
it possible to return to earlier values. This can also be tried with the
interactive `SolvingMenu`:

```rpl
'α+β+γ=180' STEQ RCEQ
SolvingMenu
```



### PushEvaluatedAssignment

When evaluating `A='2+3*5'`, pushes `A=17` on the stack.

### PushOriginalAssignment

When evaluating `A='2+3*5'`, pushes `A='2+3*5'` on the stack.


## ORDER
Sort variables in a directory


## QUOTEID
Add single quotes to a variable name


## UNQUOTEID
Remove single quotes from a variable name


## HIDEVAR
Hide a variable (make invisible)


## UNHIDEVAR
Make a hidden variable visible


## CLVAR
Purge all variables and empty subdirectories in current directory


## LOCKVAR
Make variable read-only


## UNLOCKVAR
Make variable read/write


## RENAME
Change the name of a variable



## SPROP
Store a property to a variable


## RPROP
Recall a property of a variable


## PACKDIR
Pack a directory in an editable object
# Flow control

## If

The `if` statement provides conditional structurs that let a program make
decisions. It comes in two forms:

* `if` *condition* `then` *true-clause* `end`: This evaluates *condition* and,
  if true, evaluates *true-clause*.

* `if` *condition* `then` *true-clause* `else` *false-clause* `end`: This
  evaluates *condition* and, if true, evaluates *true-clause*, otherwise
  evaluates *false-clause*.

A condition is true if:
* It is a number with a non-zero value
* It is the word `True`

A condition is false if:
* It is a number with a zero value
* It is the word `False`


## Case

The `case` statement can be used to select one case among many.
Inside a `case`, there is a list of conditions, each followed by `then` or
`when`. Code following `then` or `when` is executed when the condition is met.

* A condition preceding `then` is a boolean condition, similar to the condition
  in an `if` statement.

* A condition preceding `when` is a value that must match the current value on
  the stack exactly.

For example, `X case dup 0 < then "N" end dup 0 > then "P" end "Z" end`
will return the value `"N"`, `"P"` or `"Z"` depending on whether `X` is
negative, positive or null.

The `when` syntax is useful to test exact values, for example
`X case 0 when "zero" end 1 when "one" end 2 when "two" end end` will compute
the English spelling for value `0`, `1` and `2`.


## THENCASE
Conditional CASE ... THEN ... END THEN ... END END statement


## ENDTHEN
Conditional CASE ... THEN ... END THEN ... END END statement


## ENDCASE
Conditional CASE ... THEN ... END THEN ... END END statement


## FOR
Loop FOR ... NEXT/STEP statement


## START
Loop START ... NEXT/STEP statement


## NEXT
Loop FOR/START ... NEXT statement


## STEP
Loop FOR/START ... STEP statement


## DO
Loop DO ... UNTIL ... END statement


## UNTIL
Loop DO ... UNTIL ... END statement


## ENDDO
Loop DO ... UNTIL ... END statement


## WHILE
Loop WHILE ... REPEAT ... END statement


## REPEAT
Loop WHILE ... REPEAT ... END statement


## ENDWHILE
Loop WHILE ... REPEAT ... END statement


## IFERR
Conditional IFERR ... THEN ... ELSE ... END statement


## THENERR
Conditional IFERR ... THEN ... ELSE ... END statement


## ELSEERR
Conditional IFERR ... THEN ... ELSE ... END statement


## ENDERR
Conditional IFERR ... THEN ... ELSE ... END statement


## FORUP
Loop FORUP ... NEXT/STEP statement


## FORDN
Loop FORUP ... NEXT/STEP statement
# Finance

DB48x features a finance solver for basic compound interest computations
and amortization tables.


## FinanceSolverMenu

Display the Time Value of Money (TVM) solver menu.

This menu can be used to directly resolve payment and interests problems.
It works like the `SolvingMenu`, except that it gives no access to
equation-related features such as `NxEq` or `EvalEq`.

```rpl
TVM
```

## FinanceRounding

This setting defines the number of digits values returned by finance values will be rounded two. The default avalue is `2`.

```rpl
@ Set finance rounding to 10 digits
10 FinanceRounding

@ Reset finance rounding to default
'FinanceRounding' Purge

@ Check current value for finance rounding
'FinanceRounding' RCL
@ Expecting 2
```

## TVMRoot

Solves for the specified TVM variable using values from the remaining TVM
variables.

For example, to compute the interest rate for a 3-years €40000 loan with $1200
monthly payments at end of period, use the following code:

```rpl
@ Set loan conditions
PV=40000 Pmt=-1200 n=36 FV=0 Pyr=12 TVMEnd

@ Solve for interest rate
'I%Yr' TVMRoot

@ Expecting
```


## Amort

Amortizes a loan or investment based upon the current amortization settings.
Values must be stored in the TVM variables (`I%Yr`, `PV`, `Pmt`, and `PYr`). The
number of payments `n` is taken from the input together with flag –14
(`TVMPayAtBeginningOfPeriod` / `TVMPayAtEndOfPeriod`).

Given the number of payments `n`, the command deposits the value of the
principal, interest and balance in stack levels 1, 2 and 3.

For example, to compute the amount of principal, interest and balance 6 months
into a 10-years loan of $15000 at 10% yearly interest rates with monthly
payments at the end of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=10 PV=15000 PYr=12 FV=0 n=120 TVMEnd

@ First solve to get the payment value
'Pmt' TVMRoot

@ Compute amortization data after 6 months and put it in a vector
6 Amort →V3

@ Expecting [ -448.61 -740.74 14 551.39 ]
```


## AmortTable

Build an amortization table based on the current amortization settings.
Values must be stored in the TVM variables (`I%Yr`, `PV`, `Pmt`, and `PYr`). The
number of payments `n` is taken from the input together with flag –14
(`TVMPayAtBeginningOfPeriod` / `TVMPayAtEndOfPeriod`).

Given the number of payments `n`, the command deposits the amortization table
for the first `n` periods on the stack. If `n` is negative, then its value is
read from the `n` variable.

If the first level of the stack is a list, it can contain up to three values:

* The first period for which amortization is computed
* The number of amortization periods
* The step interval between amortization periods

All three values default to 1.

For example, to compute the amortization table of a home loan for $250,000 with
a downpayment of $62,500, an interest rate of 5.25% for 30 years, use the
following code:

```rpl
@ Set loan initial conditions
I%Yr=5.25 n='30*12' PYr=12 FV=0 PV='250000-62500'

@ Compute payment
'Pmt' TVMRoot

@ Compute amortization table
25 AmortTable
```


## TVMPayAtBeginningOfPeriod

This flag indicates that payments occur at the beginning of a payment period.
For compatibility with HP calculators, flag `-14` can also be set.

For example, to compute the amount of principal, interest and balance paid
at the end of a 2-years loan at 1.5% yearly interest rates with monthly payments of $200 at the beginning of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=1.5 Pmt=-200 PYr=12 FV=0 n=24 TVMBeg

@ First solve to get the payment value
'PV' TVMRoot

@ Compute amortization data after 24 months and put it in a vector
n Amort →V3

@ Expecting [ -4 731.71 -68.29 0. ]
```




## TVMPayAtEndOfPeriod

This flag indicates that payments occur at the end of a payment period.
For compatibility with HP calculators, flag `-14` can also be cleared.

For example, to compute the amount of principal, interest and balance paid at
the end of a 5-years loan at 2.5% yearly interest rates with monthly payments of
$500 at the beginning of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=1.5 Pmt=-200 PYr=12 FV=0 n=60 TVMEnd

@ First solve to get the payment value
'PV' TVMRoot

@ Compute amortization data after 60 months and put it in a vector
n Amort →V3

@ Expecting [ -11 554.09 -445.91 0. ]
```
# Flags

Flags are truth value that can be controled and tested by the user.
User flags are identified by a natural number. There are `MaxFlags` user flags (default is 128).
System flags are identified by a settings name or a negative integer.


## SETLOCALE
Change the separator symbols


## SETNFMT
Change the display format for numbers


## SetFlag

Set a user or system flag.

`33 SF` sets user flag 0.
`'MixedFractions' SetFlag` enables the `MixedFractions` setting.

## ClearFlag

Clear a user or system flag

## FlipFlag

Invert a user or system flag

## TestFlagSet

Test if a flag is set

## TestFlagClear

Test if a flag is clear

## TestFlagClearThenClear

Test if a flag is clear, then clear it

## TestFlagSetThenClear

Test if a flag is set, then clear it

## TestFlagClearThenSet

Test if a flag is clear, then set it

## TestFlagSetThenSet

Test if a flag is set, then set it

## FlagsToBinary

Recall all system flags as a binary number.


## BinaryToFlags

Store and replace all system flags from a binary number
# Fonts

## FNTSTO
Install a user font for system use


## FNTRCL
Recall a system font


## FNTPG
Purge a user-installed system font


## FNTSTK
Recall name of current font for stack area


## FNT1STK
Recall name of current font for stack level 1


## FNTMENU
Recall name of current font for menu area


## FNTCMDL
Recall name of current font for command line area


## FNTSTAT
Recall name of current font for status area


## FNTPLOT
Recall name of current font for plot objects


## FNTFORM
Recall name of current font for forms


## STOFNTSTK
Change current font for stack area


## STOFNT1STK
Change current font for stack level 1


## STOFNTMENU
Change current font for menu area


## STOFNTCMDL
Change current font for command line area


## STOFNTSTAT
Change current font for status area


## STOFNTPLOT
Change current font for plot objects


## STOFNTFORM
Change current font for forms


## FNTHELP
Recall name of current font for help


## FNTHLPT
Recall name of current font for help title


## STOFNTHELP
Change current font for help text


## STOFNTHLPT
Change current font for help title

# Graphic commands

DB48X features a number of graphic commands. While displaying graphics, the
stack and headers will no longer be updated.

## Coordinates

DB48X recognizes the following types of coordinates

* *Pixel coordinates* are specified using based numbers such as `#0`, and
  correspond to exact pixels on the screen. Pixels are counted starting
  from the top-left corner of the screen, with the horizontal coordinate going
  from `10#0` to `10#399`, and the vertical coordinate going from `10#0` to
  `10#239`.

* *User unit coordinates* are scaled according to the content of the `PPAR` or
  `PlotParameters` reserved variables.

* *Text coordinates* are given on a square grid with a size corresponding to the
  height of a text line in the selected font. They can be fractional.

Coordinates can be given using one of the following object types:

* A complex number, where the real part represents the horizontal coordinate and
  the imaginary part represents the vertical coordinate.

* A 2-element list or vector containing the horizontal and vertical coordinates.

* A 1-element list of vector containing one of the above.

For some operations, the list or vector can contain additional parameters beyond
the coordinates. The selection of unit or pixel coordinates is done on a per
coordinate basis. For example, `{ 0 0 }` will be the origin in user coordinates,
in the center of the screen if no `PPAR` or `PlotParameters` variable is
present.

Note that unlike on the HP48, a complex value in DB48X can
contain a based number.


## ClearLCD

Clear the LCD display, and block updates of the header or menu areas.

## BlankGraphic

Create a blank graphic object with the specified dimensions.
The graphic is optimized for the device running the program.
On color RPL devices, it will produce a color pixmap.
On black-and-white RPL devices it will produce a color pixmap.

To create a blank graphic 20 pixels wide and 30 pixels high, use:

```rpl
20 30 BlankGraphic
```


## BlankGROB

Create a blank HP48-compatible graphic object (GROB) with the specified dimensions.

To create a blank HP48-compatible GROB 20 pixels wide and 30 pixels high, use:

```rpl
20 30 BlankGROB
@ Expecting Graphic 20 x 30
```

## BlankBitmap

Create a blank packed bitmap graphic object with the specified dimensions.
This object is always a black-and-white pixmap even on color RPL devices.

To create a blank HP48-compatible GROB 20 pixels wide and 30 pixels high, use:

```rpl
20 30 BlankBitmap
@ Expecting Bitmap 20 x 30
```

## BlankPixmap

Create a color pixmap graphic object with the specified dimensions.
THis command only exists on color RPL devices.

To create a color pixmap that is pixels wide and 30 pixels high, use:

```rpl
20 30 BlankPixmap
```

## PixTest

Test the pixel at the specified coordinates and return its gray level.

```rpl
{ 0 0 } PixTest
@ Expecting 1
```

## PixColor

Test the pixel at the specified coordinates and return its color as a three
component values for the red, green and blue levels, each represented as a
decimal value between 0 and 1.

This commands works both in Color RPL and regular RPL, but only Color RPL may
return distinct values for the red, green and blue components.

```rpl
{ 0 0 } PixColor →V3
@ Expecting [ 1 1 1 ]
```

## Gray

Create a gray pattern for graphics operations. The resulting pattern can be
given as input to commands such as `Foreground` or `Background`.

This operation creates a bitmap pattern on black-and-white devices,
and a pixmap with the gray level on color devices.

```rpl
0 LINEWIDTH
0 1 FOR G
	G GRAY FOREGROUND 0 0 R→C 5 1 G - * CIRCLE
0.1 STEP
{ Foreground LineWidth } PURGE
@ Image gray-circles
```

## RGB

Create an RGB (red-green-blue) color pattern for graphics operations. The
resulting pattern can be given as input to commands such as `Foreground` or
`Background`.

This operation creates a bitmap pattern on black-and-white devices,
and a pixmap with the given color on color devices.

```rpl
0 LINEWIDTH
0 1 FOR R
	0 1 FOR G
		0 1 FOR B
			R G B RGB FOREGROUND R 14 * 7 - G 10 * 5 - R→C 1 B - 0.5 * CIRCLE
		0.1 STEP
   0.1 STEP
0.1 STEP
{ Foreground LineWidth } PURGE
@ Image color-circles
```


## HSV

Create an HSV (hue-saturation-value) color pattern for graphics operations. The
resulting pattern can be given as input to commands such as `Foreground` or
`Background`.

This operation creates a bitmap pattern on black-and-white devices,
and a pixmap with the given color on color devices.

```rpl
0 LINEWIDTH
0 1 FOR H
	0 1 FOR S
		0 1 FOR V
			H S V HSV FOREGROUND H 14 * 7 - S 10 * 5 - R→C 1 V - 0.5 * CIRCLE
		0.1 STEP
   0.1 STEP
0.1 STEP
{ Foreground LineWidth } PURGE
@ Image hsv-circles
```

## Color

Create color pattern for graphics operations. The resulting pattern can be given
as input to commands such as `Foreground` or `Background`.

The operation takes an object as input and produces a color pattern that depends on the type of input, using the same convention as for `TruthPlot`:

* Truth value `True` or `False` for foreground and background color
* Real values between `0.0` and `1.0` for grayscales
* Complex values representing saturated HSV (colorwheel) colors
* 3-element vectors representing HSV (colorwheel) colors
* 3-element lists representing RGB colors
* Names or symbols representing common color names

This operation creates a bitmap pattern on black-and-white devices,
and a pixmap with the given color on color devices.

```rpl
0 LINEWIDTH
"NavyBlue" Color Foreground 0+0ⅈ 3 Circle
{ 1 1 1 } Color Foreground 0+0ⅈ 2 Circle
[ 0 1 1 ] Color Foreground 0+0ⅈ 1 Circle
{ Foreground LineWidth } PURGE
@ Image named-circles
```


## FromLCD

Return the content of the screen as a graphic object that is put on the stack.

For example, to extract the area of the screen that contains the battery
indicator and voltage, use the following code:

```rpl
LCD→ { #315₁₀ #0₁₀ } { #400₁₀ #22₁₀ } Extract
@ Image extracted-battery
```

## ToLCD

Display a graphic object on the screen. If the graphic object is smaller than
the screen, it is centered on the screen, surrounded by gray. Note that this is different from HP calculators where it is shown in the top-left.

For example, to draw an expression in the center of the screen, use:

```rpl
'X+(1/sqrt(X-1))' 3 →GROB →LCD
@ Image small-equation
```


## DrawText

Draw the text or object in level 2 at the position indicated by level 1. A text
is drawn without the surrounding quotation marks.

If the position in level 1 is an integer, fraction or real number, it is
interpreted as a line number starting at 1 for the top of the screen. For
example, `"Hello" 1 disp` will draw `Hello` at the top of the screen.
If the position is a based number, it is a row number in pixels. For example,
`"Hello" #120d disp` will show the text in the middle of the screen.

If the position in level 1 is a complex number or a list, it is interpreted as
specifying both the horizontal or vertical coordinates, in either pixel or unit
coordinates. For example `"Hello" { 0 0 } disp` will draw `Hello` starting in
the center of the screen.

Text is drawn using the stack font by default, using the
[foreground](#foreground) and [background](#background) patterns.

If level 1 contains a list with more than 2 elements, additional elements
provide:

* A *font number* for the text

* An *erase* flag (default true) which indicates whether the background for the
  text should be drawn or not.

* An *invert* flag (default false) which, if set, will swap the foreground and
  background patterns.

* A *horizontal align* value, where -1 means align left, 1 means align right,
  and 0 means center the text. Note that fractional values or values below -1 or
  above 1 are allowed for special effects.

* A *vertical align* value, where -1 means align top, 1 means align bottom, and
  0 means center the text vertically. Note that fractional values or values
  below -1 or above 1 are allowed for special effects.

For example, the following code will draw `Hello` in the
top-left corner (`#0 #0`) with the largest (editor) font (font identifier `3`),
erasing the background (the first `true`), in reverse colors (the second
`true`), and then draws `World` in the bottom right corner.

```rpl
"Hello" { #0 #0 3 true true } DrawText
"World" { 10#400 10#240 3 true true -1 -1 } DrawText
@ Image hello-world
```

## DrawStyledText

Draw the text or object in level 3 at the position indicated by level 2, using
the font specified in level 1. This behaves like [DrawText](#drawtext), except
for the additional parameter specifying the font size.

This command is compatible with the HP50G usage, where the position is specified
by a list containing two decimal integer values. A font size of 1 displays with
a small font, a font size of 2 displays with a regular font.

In addition to this HP50G-compatible usage, `DispXY` will also accept:

* A smaller font size, 0, for the help size, and other font sizes above 2, which
  are compatible with the values given to [StackFont](#stackfont).

* The position can accept the same values as [DrawText](#drawtext), including a
  single integer value indicating a line number, a fractional line position, or
  coordinates scaled according to [PlotParameters](#plotparameters).


## Show

Display the first level of the stack using the entire screen, with a possible
scroll using the 4, 8, 6 and 2 keys if the object is larger than the screen.
This makes it possible to comfortably examine very large objects, like `300!`, a
large program or a complicated equation. Arrow keys can also be used for
horizontal or vertical scrolling.

The maximum size of the graphic object is defined by the
[MaximumShowWidth](#maximumshowwidth) and
[MaximumShowHeight](#maximumshowheight) settings.

```rpl
1.0 3 / Show
@ Image many-decimals
```

## DrawLine

Draw a line between two points specified by level 1 and level 2 of the stack.

The width of the line is specified by [LineWidth](#linewidth). The line is drawn
using the [foreground](#foreground) pattern.


## Pict

`Pict` is the name given to the graphics shown on the calculator's screen.
It can only be used as an argument to `Store`, `Recall` and graphical
operations such as `GAnd`.

[Not fully implemented yet: `Store` and `Recall` do not work]


# Bitmap operations

## ToGrob

Creates a graphics object from a specified object on the stack.

The first level of the stack specifies the character size used while
rendering the object, where value `0` indicates the `StackFont` value.

If the second level of the stack is a text, then the quotes that appear
the text is displayed on the stack will not be included in the generated
graphic object. This is similar to the behaviour of `Disp`. The rendering
of objects respects the settings used to render on the stack, e.g.
`FIX` mode or `VerticalVectors`.

The object to draw must fit in a bit map at most `MaxW`-pixels wide and
`MaxH`-pixels high.


```rpl
@ Show font sizes
0 7 for fontID
  "Font " fontID + fontID →Grob
next
@ Image fontsizes
```

## ToHPGrob

Turn an object into a graphic object in HP compatible format (GROB).
When the input is a graphic object, its graphic format is adjusted.
If the input is a color graphic, it is dithered into black and white.

Otherwise, the object is turned into a graphic object, where the font size and
color settings are taken from the `ResultFont`, `Foreground` and `Background`
settings. If the object is text, then the quotes are now shown in the resulting
graphic object.

```rpl
"Hello" →HPGrob
```

## ToBitmap

Turn an object into a graphic object in DB48x `PackedBitmaps` format.
When the input is a graphic object, its graphic format is adjusted.
If the input is a color graphic, it is dithered into black and white.

Otherwise, the object is turned into a graphic object, where the font size and
color settings are taken from the `ResultFont`, `Foreground` and `Background`
settings. If the object is text, then the quotes are now shown in the resulting
graphic object.

```rpl
'sqrt(2*x)/y' →Bitmap
@ Image sqrt
```

## ToPixmap

(This command is only available for color RPL)

Turn an object into a color graphic object in DB50x format.
When the input is a graphic object, its graphic format is adjusted.

Otherwise, the object is turned into a graphic object, where the font size and
color settings are taken from the `ResultFont`, `Foreground` and `Background`
settings. If the object is text, then the quotes are now shown in the resulting
graphic object.

```rpl
'sqrt(2*x)/y' →Pixmap
@ Image sqrt
```


## GXor

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GXor` is used for creating cursors, for example, to make the cursor image
appear dark on a light background and light on a dark background. Executing
`Gxor` again with the same image restores the original picture.

`GXOR` uses a logical exclusive OR to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Blinking cursor
"Hello" 1 DISP
1 20 start
  Pict { #0 #0 } "█" 3 →Grob GXor 0.5 Wait
next
```


## GOr

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GOr` uses a logical OR to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.
On DB48X, pixels that are set appear white.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Erasing cursor
"Hello World" 1 DISP
1 50 for i
  Pict i R→B { #0 } + "▶" 3 →Grob GOr 0.05 Wait
next
```

## GAnd

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GAnd` uses a logical AND to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.
On DB48X, pixels that are set appear white.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Darkening cursor
"Hello World" 1 DISP
1 250 for i
  Pict i R→B { #0 } + "▓" 3 →Grob GAnd 0.05 Wait
12 step
```


## GraphicAppend

Append two graphic objects side by side.
The two graphic objects are vertically centered with respect to one another.

```rpl
@ Juxtapose two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicAppend
@ Image check
```


## GraphicStack

Stack two graphic objects on top of one another.
The two graphic objects are horizontally centered with respect to one another.

```rpl
@ Stack two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicStack
@ Image check
```

## GraphicSubscript

Combine two graphic objects with the second one in subscript position

```rpl
@ Subscript with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicSubscript
@ Image check
```

## GraphicExponent

Combine two graphic objects with the second one in exponent position

```rpl
@ Exponent with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicExponent
@ Image check
```

## GraphicRatio

Combine two graphic objects as if they were in a fraction

```rpl
@ Ratio with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicRatio
@ Image check
```

## GraphicRoot

Generate a square root sign around a graphical object

```rpl
@ Square root sign
"ABC" 4 →Grob
GraphicRoot
@ Image check
```

## GraphicParentheses

Generate parentheses around a graphical object

```rpl
@ Parentheses around graphic
"ABC" 4 →Grob
GraphicParentheses
@ Image check
```

## GraphicNorm

Generate a norm (vertical bars) around a graphical object

```rpl
@ Norm around graphic
"ABC" 4 →Grob
GraphicNorm
@ Image check
```


## GraphicSum

Generate a sum (capital Sigma) sign of the given size

```rpl
@ 128-pixel Sigma sign
128 GraphicSum
@ Image check
```

## GraphicProduct

Generate a product (capital Pi) sign of the given size

```rpl
@ 96-pixel Sigma sign
96 GraphicProduct
@ Image check
```

## GraphicIntegral

Generate an integral sign of the given size

```rpl
@ 45-pixel Sigma sign
45 GraphicIntegral
@ Image check
```
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
'Dedicace' Attach
Libs
@ Expecting { Dedicace SiDensity }
```


## Detach

Unload one or more library items from disk, freeing the memory they used.

The libraries to attach can be identified by one of:
* A library index, e.g. `0`
* A library name, given as a text object or a symbol
* A library object
* A list or array of valid arguments to `detach`

For example, to unload the `Dedicace` and `KineticEnergy` library item, you can use one of:

```rpl
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
# Operations with data

Data in RPL is generally represented using lists, such as `{ {1 2 } 3 "A"}`.
Numerical data can be represented using arrays, such as `[1 2 3 4]`. In the
DB48X implementation of RPL, arrays and list can have any number of dimensions,
and are not necessarily rectangular, although some operations (e.g. matrix
operations using arrays as input) can impose stricter constraints.


## →List

Build a list from elements on the stack. Level 1 of the stack contains the
number of elements in the list. The elements are on the stack, the first element
being deepest in the stack. This is the opposite of [List→](#fromlist).

`A` `B` ... `Count` ▶ `{ A B ... }`

## List→

Expand a list on the stack and return the number of elements. After executing
the command, level 1 contains the number of elements, and a corresponding number
of stack levels contain individual elements of the list, the first element being
at the deepest level in the stack. This is the opposite of [→List](#tolist). The
[Obj→](#explode) command performs the same operation when applied to a list.

`{ A B ... }` ▶ `A` `B` ... `Count`

## Head

Return the first element of a list, or an `Invalid dimension` error if the list
is empty.

`{ A B ... }` ▶ `A`

## Tail

Return all but the first element of a list, or an `Invalid dimension` error if
the list is empty.

`{ A B ... }` ▶ `{ B ... }`

## Map

Apply an operation on all elements in a list or array. The operation on the
first level of the stack should take one argument and return a single value.

`{ A B ... }` `F` ▶ `{ F(A) F(B) ... }`


```rpl
{ 1 2 A 42.2 [ 1 2 3 ]} « →STR » MAP
@ Expecting { "1" "2" "A" "42.2" "[ 1 2 3 ]" }
```

Unlike on HP calculators, `Map` does not apply recursively to inner lists by default.

```rpl
{ 1 2 { 3 4 } } « →STR » MAP
@ Expecting { "1" "2" "{ 3 4 }" }
```

This can be changed by setting `ListRecursionDepth` to `0`.

```rpl
0 ListRecursionDepth
{ 1 2 { 3 4 } }  « →STR » MAP
@ Expecting { "1" "2" { "3" "4" } }
```

`Map` can also be constrained to a given depth.

```rpl
2 ListRecursionDepth
{ 1 2 { 3 { 4 { 5 } } } }  « →STR » MAP
@ Expecting { "1" "2" { "3" "{ 4 { 5 } }" } }
'ListRecursionDepth' Purge
```

## Reduce

Apply a cumulative pairwise operation on all elements in a list or array.
The operation on the first level of the stack should take two arguments and
combine them into a single value. The result is the repeated application of that
operation to all elements.

`{ A B ... }` `F` ▶ `F(F(A, B), ...)`

For example, to sum all the elements in a list as text, one can use:

```rpl
{ X Y Z T 12 { Hello World } } « →STR + » REDUCE
@ Expecting "XYZT12{ Hello World }"
```

Reduce obeys the `ListRecursionDepth` setting:

```rpl
0 ListRecursionDepth
{ X Y Z T 12 { Hello World } } « →STR + » REDUCE
@ Expecting "XYZT12HelloWorld"
'ListRecursionDepth' Purge
```


## Filter

Filter elements in a list of array based on a predicate. The predicate given on
level 1 of the stack takes a value as argument, and returns a truth values. The
resulting list is built with all elements where the predicate is true.

`{ A B ... }` `P` ▶ `{ A ... }` if `P(A)` is true and `P(B)` is false.

For example, to filter all odd numbers in an array, one can use:

```rpl
[ 1 2 3 4 5 6 42 ] « 2 MOD 0 = » FILTER
@ Expecting [ 2 4 6 42 ]
```

`Filter` obeys the `ListRecursionDepth` setting:

```rpl
0 ListRecursionDepth
{ 1 2 3 4 { 5 6 { 42 } } } « 2 MOD 0 = » FILTER
@ Expecting { 2 4 { 6 { 42 } } }
'ListRecursionDepth' Purge
```


## Get

Get an element from composite data, such as list, an array or a text.
Elements are numbered starting at 1. The index can itself be a list, which is
interpreted as successive indices in the data.

For example,
`{ A B C } 2 GET` returns `B`,
`"ABC" 3 GET` returns `"C"`,
`[ [ 1 2 ] [ 4 5 ] [ 5 6 ] ] { 2 2 } GET` returns `5`.

When the data is a name, data is feched directly from the given named variable.

`Data` `Index` ▶ `Element`


## Put

Put an element into composite data, such as list, an array or a text.
This performs the opposite operation compared to `Get`.

Elements are numbered starting at 1. The index can itself be a list, which is
interpreted as successive indices in the data.

For example,
`{ A B C } 2 'X' PUT` returns `{ A X C }`,
`"ABC" 2 "Hello" PUT` returns `"AHelloC"`,
`[ [ 1 2 ] [ 4 5 ] [ 5 6 ] ] { 2 2 } 7 PUT` returns `[ [ 1 2 ] [4 7 ] [5 6] ]`.

When the data is a name, data is feched directly from the given named variable.

`Data` `Index` ▶ `Element`


## Sort

Sort elements in a list or array, sorting by increasing values when comparing
numers, text or symbols.

This may be a little slower than `QuickSort`, but is useful to sort
lists or arrays of numerical values or text values.

## QuickSort

Sort elements in a list or array using the memory representation of objects.
This guarantees a consistent sorting order, but one that does not necessarily
preserve numerical or textual properties, unlike `Sort`. Comparisons are,
however, significantly faster than `Sort`.

## ReverseSort

Sort a list or array by value, in reverse order compared to `Sort`.

## ReverseQuickSort

Sort a list or array using the memory representation of objects, in reverse
order compared to `QuickSort`.

## ReverseList

Reverse the order of elements in a list


## ADDROT
Add elements to a list, keep only the last N elements


## SEQ
Assemble a list from results of sequential procedure

## ListSum

Return the sum of a list or array.

The `ListSum` command calculates the sum of all elements in a list or array.
It uses the `+` operation to add all elements together.
The result is a single value representing the sum of all elements.

In the following example, values `1`, `3` and `5` are added, which gives `9`.
Then the vector `[ 1 2 3 ]` is added to `9`, which gives another vector with
value `[ 10 11 12 ]`. Finally, this vector is added to the `"ABC"` text,
resulting in a text containing `"[ 10 11 12 ]ABC|`:

```rpl
{ 1 3 5 [ 1 2 3 ] "ABC" } ΣList
@ Expecting "[ 10 11 12 ]ABC"
```

## ListProduct

Return the product of a list or array.

The `ListProduct` command calculates the product of all elements in a list or array.
It uses the `×` operation to multiply all elements together.
The result is a single value representing the product of all elements.

In the following example, we multiply values `1`, `3`, `6`, `42` and
finally the complex value `2+3ⅈ`:

```rpl
[ 1 3 6 42 2+3ⅈ ] ∏List
@ Expecting 1 512+2 268ⅈ
```

## ListDifferences

Return the differences between successive elements in a list or array.

The `ListDifferences` command calculates the differences between consecutive
elements in a list or array.
It uses the `-` operation to subtract each element from the previous one.
The result is a list with one fewer element than the input, containing the
pairwise differences.

For example to compute the increase in value between successive elements in a
vector, you can use:

```rpl
[ 1 3 4 7 9 ] ∆List
@ Expecting [ 2 1 3 2 ]
```

Note that on HP calculators, this command only works with lists and not with vectors. On DB48x, it works both with list and arrays.
    Matrices are seen as arrays of vectors, as illustrated below:

```rpl
[[ 1 2 3 ]
 [ 4 5 6 ]
 [ 7 9 0 ]] ∆List
@ Expecting [[ 3 3 3 ] [ 3 4 -6 ]]
```
# Operations with Matrices and vectors

## →Array

Stack to Array Command: Returns a vector or matrix built from individual
elements placed on the stack and dimensions.

If the dimension is given as a positive integer, then `→Array` returns a
vector built from the given number of individual items.

```rpl
x y z 3 →Array
@ Expecting [ 'x' 'y' 'z' ]
```

The number of items can also be given as a list or array containing one or two
positive integers. If it contains one item, then `→Array` returns a vector:

```rpl
1.2 3.4 5.6 { 2.5 } →Array
@ Expecting [ 3.4 5.6 ]
```

If the list contains two items, the first one is the number of rows, the second
one the number of columns. Elements of the result array should be entered on the
stack in row order.

```rpl
1 2 3 4 5 6 [ 2 3 ] →Array
@ Expecting [[ 1 2 3 ] [ 4 5 6 ]]
```

## Array→

Array to Stack Command: Takes an array and returns its elements as separate
values. Also returns a list of the dimensions of the array.

If the argument is a vector, elements are placed on the stack with the first one
higher in the stack and the last one on the second level of the stack.

```rpl
[ 1 2 3 ] Array→ + + +
@ Expecting { 1 2 3 3 }
```

If the argument is a matrix, elements are placed on the stack in row order:

```rpl
[[1 2 3][4 5 6]] Array→ + + + + + +
@ Expecting { 1 2 3 4 5 6 2 3 }
```

## →Columns

Split an array into column vectors

If the input is a vector, `→Columns` returns the individual elements.

```rpl
[ 1 2 3 ] →Columns
4 →List
@ Expecting { 1 2 3 3 }
```

If the input is a matrix, `→Columns` returns the individual columns.

```rpl
[[ 1 2 3 ][ 4 5 6 ]] →Columns
4 →List
@ Expecting { [ 1 4 ] [ 2 5 ] [ 3 6 ] 3 }
```



## COL+

Insert Columns Command: Insert columns into an existing array. The `COL+`
command takes three arguments:
* an input array or list where the columns will be inserted
* the columns to insert
* the insertion position

If the input is a matrix, the columns can be an individual vector:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ 22 33 ]              @ Column to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 2 3 ] [ 4 33 5 6 ]]
```

or a matrix with the same number of rows:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ [22 33 ] [ 44 55 ] ] @ Columns to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 33 2 3 ] [ 4 44 55 5 6 ]]
```

If the input is a vector, then the columns can be an individual value:

```rpl
[ 1 2 3 ]  @ Input vector
4          @ Value to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 2 3 ]
```

The columns can also be another vector:

```rpl
[ 1 2 3 ]  @ Input vector
[ 4 5 ]    @ Values to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 5 2 3 ]
```


## COL-

Delete Columns Command: Deletes one or more columns from an array.

The `COL-` command takes an input array and a column index in the array, and
returns an array with the given column removed.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
3 COL-
@ Expecting [[ 11 12 14 15 16 ] [ 21 22 24 25 26 ] [ 31 32 34 35 36 ]]
```

Multiple columns can be removed by giving the first one and the number of
columns to remove.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
[ 3 2 ] COL-
@ Expecting [[ 11 12 15 16 ] [ 21 22 25 26 ] [ 31 32 35 36 ]]
```

## Columns→

Columns to Matrix Command: Transforms a series of column vectors and a column
count into a matrix containing those columns, or transforms a sequence of
numbers and an element count into a vector with those numbers as elements.

```rpl
[ 1 2 ]
[ 4 5 ]
[ 7 8 ]
3 Columns→
@ Expecting [[ 1 4 7 ] [ 2 5 8 ]]
```

If the individual values are not arrays, then a vector is produced:

```rpl
1 2 3 4
4 Columns→
@ Expecting [ 1 2 3 4 ]
```

If not all vectors have the same length, the number of rows of the array
returned by `Columns→` is the maximum size of all input vectors, and the
remaining elements are padded with `0`.

```rpl
[ 1     ]
[ 2 3 4 ]
[ 5 6   ]
3 Columns→
@ Expecting [[ 1 2 5 ] [ 0 3 6 ] [ 0 4 0 ]]
```


## →Rows

Split an array into row vectors

If the input is a vector, `→Rows` returns the individual elements.

```rpl
[ 1 2 3 ] →Rows
4 →List
@ Expecting { 1 2 3 3 }
```

If the input is a matrix, `→Rows` returns the individual rows.

```rpl
[[ 1 2 3 ][ 4 5 6 ]] →Rows
3 →List
@ Expecting { [ 1 2 3 ] [ 4 5 6 ] 2 }
```



## ROW+

Insert Rows Command: Insert rows into an existing array. The `COL+`
command takes three arguments:
* an input array or list where the rows will be inserted
* the rows to insert
* the insertion position

If the input is a matrix, the rows can be an individual vector:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ 22 33 ]              @ Row to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 2 3 ] [ 4 33 5 6 ]]
```

or a matrix with the same number of rows:

```rpl
[[ 1 2 3 ] [ 4 5 6 ]]  @ Input matrix
[ [22 33 ] [ 44 55 ] ] @ Rows to insert
2                      @ Insertion position
COL+
@ Expecting [[ 1 22 33 2 3 ] [ 4 44 55 5 6 ]]
```

If the input is a vector, then the rows can be an individual value:

```rpl
[ 1 2 3 ]  @ Input vector
4          @ Value to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 2 3 ]
```

The rows can also be another vector:

```rpl
[ 1 2 3 ]  @ Input vector
[ 4 5 ]    @ Values to insert
2          @ Insertion position
COL+
@ Expecting [ 1 4 5 2 3 ]
```


## ROW-

Delete Rows Command: Deletes one or more rows from an array.

The `COL-` command takes an input array and a row index in the array, and
returns an array with the given row removed.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
3 COL-
@ Expecting [[ 11 12 14 15 16 ] [ 21 22 24 25 26 ] [ 31 32 34 35 36 ]]
```

Multiple rows can be removed by giving the first one and the number of
rows to remove.

```rpl
[[11 12 13 14 15 16]
 [21 22 23 24 25 26]
 [31 32 33 34 35 36]]
[ 3 2 ] COL-
@ Expecting [[ 11 12 15 16 ] [ 21 22 25 26 ] [ 31 32 35 36 ]]
```

## Rows→

Rows to Matrix Command: Transforms a series of row vectors and a row
count into a matrix containing those rows, or transforms a sequence of
numbers and an element count into a vector with those numbers as elements.

```rpl
[ 1 2 ]
[ 4 5 ]
[ 7 8 ]
3 Rows→
@ Expecting [[ 1 2 ] [ 4 5 ] [ 7 8 ]]
```

If the individual values are not arrays, then a vector is produced:

```rpl
1 2 3 4
4 Rows→
@ Expecting [ 1 2 3 4 ]
```

If not all vectors have the same length, a non-rectangular array will be
produced. Unlike `Rows→`, no padding with `0` will occur for missing elements.

```rpl
[ 1     ]
[ 2 3 4 ]
[ 5 6   ]
3 Rows→
@ Expecting [[ 1 ] [ 2 3 4 ] [ 5 6 ]]
```

## TODIAG
Extract diagonal elements from a matrix


## FROMDIAG
Create a matrix with the given diagonal elements



## AXL
Convert a matrix to list and vice versa


## BASIS
Find vectors forming a basis of the subspace represented by the matrix


## CHOLESKY
Perform Cholesky decomposition on a matrix


## ColumnNorm
Column norm (one norm) of a matrix


## RowNorm
Row norm (infinity norm) of a matrix


## Norm

Euclidean norm of a vector of matrix


## ConstantArray

Returns a constant array, defined as an array whose elements all have the same
value.

The constant value is an object taken from argument 2/level 1. The resulting
array is either a new array, or an existing array with its elements replaced by
the constant, depending on the object in argument 1/level 2.

* Creating a new array: If level 2 contains a list of one or two integers, `CON`
  returns a new array. If the list contains a single integer `n`, `CON` returns
  a constant vector with `n` elements. If the list contains two integers `n` and
  `m`, `CON` returns a constant matrix with `n` rows and `m` columns.

* Replacing the elements of an existing array: If level 2 contains an array,
  `CON` returns an array of the same dimensions, with each element equal to the
  constant.

* If level 2 contains a name, the name must identify a variable that contains a
  valid input for `con`, such as an array. In this case, the content of the
  variable is replaced with the value generated by `CON`

`n` `k` ▶ `[ k ... k ]`

`{ n }` `k` ▶ `[ k ... k ]`

`{ n m }` `k` ▶ `[ [ k ... k ] [ k ... k ] ... [ k ... k ] ]`

`[ vec ]` `k` ▶ `[ k ... k]`

`[ [ mat ] ]` `k` ▶ `[ [ k ... k ]]`

`'name'` `k` ▶



## Cond
Column norm condition number of a matrix


## Cross
Cross produce of vectors


## CSWP
Swap two columns in an array or vector. The `CSWP` command takes an array and
two column numbers.

If the input is a vector, then the elements at the given index are swapped with
one another, much like `RSWP`.

```rpl
[11 22 33 44 55] 2 3 CSWP
@ Expecting [ 11 33 22 44 55 ]
```

If the input is an array with more than one dimension, then the columns at the
given index are swapped with one another:

```rpl
[[ 11 12 13 14 ]
 [ 21 22 23 24 ]
 [ 31 32 33 34 ]]
1 4 CSWP
@ Expecting [[ 14 12 13 11 ] [ 24 22 23 21 ] [ 34 32 33 31 ]]
```

As an extension relative to HP's implementation, the command also works with
lists.

```rpl
{ { "A" 2 } { "C" 4 "X" } }
1 2 CSWP
@ Expecting { { 2 "A" } { 4 "C" "X" } }
```

## Determinant

Compute the determinant of a matrix

## DOT

Internal product (dot product) of vectors


## EGV


## EGVL
Compute the eigenvalues of a matrix


## GRAMSCHMIDT


## HADAMARD
Multiply corresponding elements in a matrix


## HILBERT
Assemble a Hilbert symbolic array


## IBASIS
Find a basis of the intersection of two vector spaces


## IdentityMatrix

Identity Matrix Command: Returns an identity matrix, that is, a square matrix
with its diagonal elements equal to 1 and its off-diagonal elements equal to 0.

The result is either a new square matrix, or an existing square matrix with its
elements replaced by the elements of the identity matrix, according to the
argument.

* Creating a new matrix: If the argument is an integer `n`, a new real identity
  matrix is returned, with its number of rows and number of columns equal to
  `n`.

* Replacing the elements of an existing matrix: If the argument is a square
  matrix, an identity matrix of the same dimensions is returned.

* Generating the identity matrix for a vector: If the argument is a vector with
  `n` elements, an identity matrix with `n` rows and `n` columns is created.

* If the argument is a name, the name must identify a variable containing on of the valid inputs. In this case, it is replaced with the result.


`n`  ▶ `IDN(n)`

`{ n }` ▶ `IDN(n)`

`{ n n }`  ▶ `IDN(n)`

`[ n-vec ]` ▶ `IDN(n)`

`[[ nxn-mat ]]` ▶ `IDN(n)`

`'name'` ▶


## IMAGE
Find a basis of the image of a linear application


## ISOM


## JORDAN


## KER
Find a basis for the kernel of a linear application


## LQ


## LSQ


## LU
LU factorization of a matrix


## MAD


## MKISOM


## PMINI
Minimal polynomial of a matrix


## QR
QR Decomposition of a matrix


## RANK
Rank of a matrix


## RandomMatrix

Returns an array containing random integer values between -9 and 9.


## RCI
Multiply a row by a constant


## RCIJ
Multiply a row by a constant and add to other row


## RDM
Change dimensions of an array


## REF
Reduce matrix to echelon form (upper triangular form)


## RREF
Fully reduce to row-reduced echelon form


## RREFMOD


## RSD
Residual R=B-A*X' on a system A*X=B


## RSWP
Swap two rows in an array or vector. The `RSWP` command takes an array and
two column numbers.

If the input is a vector, then the elements at the given index are swapped with
one another, much like `CSWP`.

```rpl
[11 22 33 44 55] 2 3 RSWP
@ Expecting [ 11 33 22 44 55 ]
```

If the input is an array with more than one dimension, then the columns at the
given index are swapped with one another:

```rpl
[[ 11 12 13 14 ]
 [ 21 22 23 24 ]
 [ 31 32 33 34 ]]
1 2 RSWP
@ Expecting [[ 21 22 23 24 ] [ 11 12 13 14 ] [ 31 32 33 34 ]]
```

As an extension relative to HP's implementation, the `RSWP` command also works with
lists.

```rpl
{ { "A" 2 } { "C" 4 "X" } }
1 2 RSWP
@ Expecting { { "C" 4 "X" } { "A" 2 } }
```



## SCHUR


## SNRM


## SRAD


## SVD


## SVL


## SYLVESTER


## TRACE
Sum of the items in the diagonal of a matrix


## TRAN
Transpose a matrix (without comjugate, unlike `TRN`)


## TRN
Complex conjugate transpose of a matrix


## VANDERMONDE


## LDUP
Decompose A into LDUP such that P*A=L*D<sup>-1</sup>*U


## MMAP
Apply expression or program to the elements of a matrix
## Catalog

The `Catalog` command is triggered by the `+` command while in _Alpha_ mode, or
using 🟨 _+_ otherwise (_CAT_). It is an auto-completing catalog of all the
available commands or characters.

When entering a program, all commands matching the text currently matched will
be shown. For example, if you typed `A`, all commands containing an `A` are
displayed in alphabetical order. If you type ` ACO `, ` ACOS `, ` ACOSH ` and
` DataColumn ` (a possible spelling for ` ColΣ `) will all be shown.

Inside text, all characters related to the last entered character will be shown.
For example, if you typed `A`, options to add accents to `A` or to select the
Greek `α` or the related `ª` character. If no matching character is found,
the `CharactersMenu` is presented, allowing you to select specific characters
from predefined categories.


## CharactersMenu

The `CharactersMenu`, which can be accessed using 🟦 _2_ (_CHAR_), presents a
convenient way to insert characters.

While entering text, the [Catalog](#Catalog) also presents matching characters,
or the characters menu if none is found.

The characters menus and the catalog behaviour can be configured by modifying
the `config/characters.csv` file. Each row in that file contains two texts:

* The first is the name of the menu. If the name is empty, the menu is not
  shown, but is used to find characters related to a given character.

* The second is the list of characters to be shown.

Some language-specific submenus in the characters menu enable *transliteration*,
which select alphabetic characters in that language. For example, if you use the
character menu for Greek and lowercase, then typing `A` will insert
`α` (greek lowercase alpha). Repeated use of the same key cycles through
options, in that case `ά` and `a` before cycling back to `α`.


## ConstantsMenu

The `ConstantsMenu` gives access to numerous [constants](#constants), shown in
various categories. This includes basic mathematical constants such as `π` or
`e`, as well as constants related to chemistry, physics, computer science or
history.

The constants menu is defined by the `config/constants.csv` file. You are
encouraged to tailor this file to suit your own needs.


## ToggleCustomMenu

The `ToggleCustomMenu` command toggles between the `VariablesMenu` and the
`CustomMenu`. It is normally bound to the _VAR_ key, so that a first press on
this key shows the variables in the current directory, and a second press shows
any user-defined custom menu if it exists.


## CustomMenu

The `CustomMenu` command activates a menu defined by the variable of the same
name. The content of the variable should evaluate as an array or list, where
each item is either an array or list of the form `{ Name Value }`, or an object
that is made directly accessible in the menu. See `Menu` below for a description
of the format of menu descriptions.

The `Menu` command defines the `CustomMenu` variable and shows the corresponding
custom menu.


## Menu

This command shows up a system menu or defines a custom menu.

If the object on the stack is a menu name, typically returned by `RecallMenu`,
then `Menu` will activate that menu.

If the object on the stack is `0`, then `Menu` has the same effect as
`LastMenu`, showing the last menu selected. Other numerical values, which select
built-in menus on HP's implementation, will cause an `Unimplemented` error.

If the object on the stack is an array or a list, `Menu` will use that array or
list to set the value of the `CutomMenu` variable, and then behave like the
`CustomMenu` command, i.e. show the menu defined by the list.

The following defines a `CustomMenu` adding or subtracting powers of `10`

```rpl
{ { "1+"   «   1 + » } { "1-"   «   1 - » }
  { "10+"  «  10 + » } { "10-"  «  10 - » }
  { "100+" « 100 + » } { "100-" « 100 - » } }
MENU
```

You can define a menu of vertically-stacked menu items using a matrix.
For example, you can define a menu that lets you store, evaluate or purge
variables `X` and `Y as follows:

```rpl
[ [ X                      Y                      ]
  [ { ">X" « 'X' Store » } { ">Y" « 'Y' Store » } ]
  [ { "-X" « 'X' Purge » } { "-Y" « 'Y' Purge » } ] ]
MENU
```

## TMenu

The `TMenu` command works like [Menu](#menu), but does not define the
`CustomMenu` variable. The resulting menu is temporary and cannot be found by
`RecallMenu` or by the menu history in `LastMenu`.

For example, the following defines a temporary menu showing

```rpl
{ 2 3 5 7 11 13 97 197 397 797 997 1097 1297 1597 1697 1997 2297 2797 }
TMENU
```


## RecallMenu

Return the last menu entry, or `0` if there is no current menu. The returned
value can be used as an argument to `Menu` or `TMenu`.
# Numerical integration

## Integrate

Perform a numerical integration of a function for a specified variable on a
numerical interval. For example `2 3 'X*(X-3)' 'X' Integrate` returns `-7/6`.

The function takes four arguments:

* The lower bound of the integration range
* The higher bound of the integration range
* The program or expression to evaluate
* The integration variable

### IntegrationImprecision

This setting defines the relative imprecision for the result with respect to the
[Precision](#precision) setting. The default value is `6`, which means that at
the default precision of `24` digits, `Integrate` will try to compute to an
accuracy of 18 digits.

This setting only applies if the result is smaller than the display
settings. Like HP calculators, the display settings limits the precision
requested from the integration algorithm. For example, if the display is set to
`3 FIX`, then only 3 digits of precision are considered necessary in the result.

### IntegrationIterations

This setting limits the number of iterations for the integration algorithm. Each
iteration requires the evaluation of twice as many samples of the function to
integrate as the previous one, so the maximum number of samples taken is in the
order of `2^IntegrationIterations`.


# Continued fractions

## DFC

Decompose a real number into its continued fraction coefficients (Décomposition
en Fraction Continue).

`X` ▶ `{ a0 a1 a2 … }`

Returns a list of integers such that *X* ≈ a₀ + 1/(a₁ + 1/(a₂ + …)). For
integers the result is a single-element list. For rationals the expansion is
exact and finite. For decimals and irrationals (π, *e*, √2, etc.), the expansion
stops when the next convergent would exceed the precision of the input.

Examples: `22/7 DFC` → `{ 3 7 }`, `2 √ DFC` → `{ 1 2 2 2 … }`.

The inverse `DFC2F` reconstructs the value from the coefficient list.

## DFC2F

Reconstruct a real number from its continued fraction coefficient list.

`{ a0 a1 … an }` ▶ `a0 + 1/(a1 + 1/(… + 1/an))`

Evaluates the list right-to-left to produce a rational or decimal result.
`DFC2F(DFC(x))` gives an exact round-trip for integers and fractions, and an
approximate round-trip for irrationals within the current precision.

Example: `{ 3 7 } DFC2F` → `22/7`.


# Numerical conversions

## →Num

Convert fractions and symbolic constants to decimal form.
For example, `1/4 →Num` results in `0.25`.

## →Q

Convert decimal values to fractions. For example `1.25 →Frac` gives `5/4`.
The precision of the conversion in digits is defined by
[→FracDigits](#ToFractionDigits), and the maximum number of iterations for the
conversion is defined by [→FracDigits](#ToFractionIterations)

## →Qπ

Convert decimal values to a rational form, or a rational form with π, square
roots, natural logs, or the Euler constant *e* factored out, whichever yields
the smaller denominator.

The rational result is a "best guess", since there might be more than one
rational expression consistent with the argument. `→Qπ` finds a quotient of
integers that agrees with the argument to the number of decimal places specified
by the display format mode.

For example, `3.14159265359 →Qπ` gives `π`, `1.4142135624 →Qπ` gives `√2`,
and `0.346573590280 →Qπ` gives `ln 2/2`.

For a complex argument, the real or imaginary part (or both) can have a constant
factor.

The [→QπMaxPrime](#→qπmaxprime) setting (default 100, max 10000)
limits which primes are tried when factoring squares for √*n* detection. Lower
values speed up conversion on resource-constrained hardware.

The algorithm attempts: π; any √*n* (by squaring and factoring); ln 2, ln 3,
ln 5, ln 7, ln 10; *e*; *e*^(p/q); and combined factors π·√2, π·√3.

## →Integer

Convert decimal values to integers. For example `1. →Integer` gives `1`.
This command intentionally fails with `Bad argument value` if the input contains
a non-zero fractional part.
# Objects

## Cycle

Cycle through various representations of the object on the first level of the stack.

* Polar <-> Rectangular for complex numbers
* Decimal <-> Fraction
* Integer <-> Based (cycles through the 2, 8, 10 and 16 base)
* Array <-> List <-> Program
* Text <-> Symbol

For unit objects, `Cycle` will cycle through all SI prefixes such that the
decimal representations fits within the `StandardExponent` range (i.e. that
would not display in scientific mode), increasing the numerical value, and then
switch the value to a fraction and cycle through all fraction representations
that fit within the same numerical range.

For example, if the `StandardExponent` is set to `6`, the value `0.1_m` will
cycle as follows:

* `0.1_m` being a decimal, we move to next scale up as decimal
* `1._dm`
* `10._cm`
* `100._mm`
* `100000._μm`, which is the limit of what can be displayed with 6 digits, so we
  switch to a fractional representation.
* `100000_μm` being a fraction, we go through the prefixes going down.
* `100_mm`
* `10_cm`
* `1_dm`
* `1/10_m`
* `1/100_dam`
* `1/1000_hm`
* `1/10000_km`, at which point the conversion would be out of range, so we
  switch back to decimal.
* `0.0001_km`
* `0.001_hm`
* `0.01_dam`
* `0.1_m` at which point the cycle repeats.


## Explode

Explode an object into its sub-components. The various sub-components are placed
on the stack, and if necessary, information about the size is places on the
first level of the stack.

* Complex numbers are split into real and imaginary part (for rectangular form)
  or modulus and argument (for polar form). The latter is an extension compared
  to classical RPL, which always represent complex numbers in rectangular form.

* Unit objects are split into the value and unit expression. This is a deviation
  from standard RPL, which places a unit object on the first level of the stack.

* Lists, programs and expressions are split into their individual components,
  and the number of components is placed on the first level of the stack. For
  programs, this is an extension of standard RPL. For expressions, this is a
  deviation: HP calculators instead place only the top level object and
  the arity.

* Arrays and vectors are split into their individual components, and the number
  of elements is placed as a list on the first level of the stack. The dimension
  list has one element for vectors, and two for matrices. If a given matrix is
  not rectangular, then the command reports an `Invalid dimension` error.

* Text is evaluated as if it had been executed on the command line, in a way
  similar to the `STR→` command.


## →Program

Convert an object into a program.

* Algebraic expressions or polynomials are converted to an equivalent
  RPL program. For example, `'sin(X+2*Y)'` will be converted as
  `« X 2 Y × + sin »`

* Lists are converted to a program with the exact same structure.
  For example, `{ 1 2 + }` will be converted to `« 1 2 + »`.

* Other objects are simply wrapped in a program.
  For example, `1.2` will be converted to `« 1.2 »`
  Note that this applies to programs as well.
# Scalable plots and graphics

Graphics are rendered on the screen based on a coordinate systems defined in the
`PlotParameters` variables.

DB48X supports multiple plotting modes:
* **Function** - Standard y=f(x) plots
* **Polar** - Polar coordinate plots
* **Parametric** - Parametric curve plots
* **Scatter** - Scatter plots from data
* **Bar** - Bar charts from data
* **Histogram** - Histogram plots for frequency distributions


## PlotMin

Set the minimum value for the current plot range.

The `PMin` value is typically a complex number giving the coordinates of the
point that will show as the bottom-left corner of the screen.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using `PMin`
to ensure that the lower-left corner of the screen corresponds to coordinates
`(-1;-1)`, which brings the circle closer to the lower-left corner:

```rpl
(0;0) 1 CIRCLE
(-1;-1) PMIN
(0;0) 1 CIRCLE
@ Image plotmin-circle
```

## PlotMax

Set the maximum value for the current plot range.

The `PMax` value is typically a complex number giving the coordinates of the
point that will show as the top-right corner of the screen.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using `PMax`
to ensure that the lower-left corner of the screen corresponds to coordinates
`(3;2)`, which brings the circle closer to the top-right corner:

```rpl
(0;0) 1 CIRCLE
(3;2) PMAX
(0;0) 1 CIRCLE
@ Image plotmax-circle
```


## XRange

Set the X-axis range for the current plot.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using
`XRange` to ensure that the horizontal range is between `-1.5` and `1.5`, which
causes the circle to be elongated horizontally:

```rpl
(0;0) 1 CIRCLE
-1.5 1.5 XRNG
(0;0) 1 CIRCLE
@ Image xrange-circle
```

## YRange

Set the Y-axis range for the current plot.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using
`YRange` to ensure that the horizontal range is between `-1.5` and `1.5`, which
causes the circle to be elongated vertically:

```rpl
(0;0) 1 CIRCLE
-1.5 1.5 YRNG
(0;0) 1 CIRCLE
@ Image yrange-circle
```


## PlotParameters

The `PlotParameters` reserved variable defines the plot parameters, as a list,
with the following elements:

* *Lower Left* coordinates as a complex (default `-10-6i`)
* *Upper Right* coordinates as a complex (default `10+6i`)
* *Independent variable* name (default `x`)
* *Resolution* specifying the interval between values of the independent
  variable (default `0`). A binary numnber specifies a resolution in pixels.
* *Axes* which can be a complex giving the origin of the axes (default `0+0i`),
  or a list containing the origin, the tick mark specification, and the names of
  the axes.
* *Type* of plot (default `function`)
* *Dependent variable* name (default `y`)

To reset the `PlotParameters` to the default values, it is necessary to purge
the current directory as well as the parents from any `PlotParameters` value:

```rpl
'PPAR' PGALL
```

## FunctionPlot

Plot a function in the form y=f(x). The function is taken from the stack.

```rpl
'tan(13*x) * sin(500*x)' FunctionPlot
@ Image fnplot-example
```


## PolarPlot

Plot a function in polar coordinates, where r=f(θ).

```rpl
'6*sin(15*x)*sin(113*x)' PolarPlot
@ Image polarplot-example
```


## ParametricPlot

Plot a parametric curve where x and y are both functions of a parameter.

The input function returns a complex number where the real part is interpreted
as the position along the x axis and the imaginary part is interpreted as the
position along the y axis.


```rpl
'exp((0.25+4.5ⅈ)*x)' ParametricPlot
@ Image parametricplot-example
```

## TruthPlot

Plot a curve where an expression in `x` and `y` is tested.

The input function can return:

* Truth value `True` or `False` for foreground and background color
* Real values between `0.0` and `1.0` for grayscales
* Complex values representing saturated HSV (colorwheel) colors
* 3-element vectors representing HSV (colorwheel) colors
* 3-element lists representing RGB colors
* Names or symbols representing common color names

Drawing is done incrementally, by splitting the horizontal and vertical
coordinate space again and again. When `Res` is at the default value `0`, the
maximum number of bins along the X and Y axis is set by `XYPlotBins`. Otherwise
`Res` defines the minimum size of bins.


### Truth plot with a truth value

The following example illustrates the use of the `TruthPlot` with a truth value:

```rpl
'x²-2·y²>1.9+38·sin(21·x·y)' TruthPlot
@ Image truthplot
```

### Truth plot with a real value

The following example illustrates the use of the `TruthPlot` with a real value:

```rpl
'sin(21·x·y)' TruthPlot
@ Image truthplot-real
```

### Truth plot with a complex value

The following example illustrates the use of the `TruthPlot` with a real value:

```rpl
« x y RealToComplex 0.15 * »  TruthPlot
@ Image truthplot-complex
```

Note that if `AutoSimplify` is enabled, this graph will show a white horizontal
line corresponding to the real axis. This is because the multiplication by
`0.15` auto-simplifies the result as a real number if the imaginary part is
zero.

### Truth plot with a vector

The following example illustrates the use of the `TruthPlot` with a vector of
components to generate arbitrary hue-saturation-values between 0 and 1.

```rpl
«
    x y atan2 0 + 90 / 1 REM
    x y HYPOT
    DUP 300 * SIN
    SWAP 10 /
    →3D
»  TruthPlot
@ Image truthplot-vector
```

### Truth plot with a list

The following example illustrates the use of the `TruthPlot` with a list of
color components specifying red, green and blue values between 0 and 1.

```rpl
«
    {} x 36 * COS + y 45 * COS + x y HYPOT 100 * COS +
»  TruthPlot
@ Image truthplot-list
```

### Truth plot with a name

The following example illustrates the use of the `TruthPlot` with names for
colours.

```rpl
«
    case
        x -3.4 < THEN "Red"     END
        x  3.4 < THEN "White"   END
                      "NavyBlue"
    end
»  TruthPlot
@ Image truthplot-list
```


## ScatterPlot

Create a scatter plot from the first two columns of a matrix, the first column
representing the horizontal axis, the second column representing the vertical
axis.

```rpl
[ [ -5 -5 ]
  [ -3  0 ]
  [ -5  5 ]
  [  0  3 ]
  [  5  5 ]
  [  3  0 ]
  [  5 -5 ]
  [  0 -3 ]
  [ -5 -5 ] ]
ScatterPlot
@ Image scatterplot-example
```

## BarPlot

Create a bar chart from a vector of values.

The size of the vector defines the number of bars being displayed.
Unlike other plotting commands, the `xrange` setting is not taken into account
when positioning the bars horizontally. However, the `yrange` setting is used to
determine the vertical position of the bars.

```rpl
[1 2 3 4 3 2 1 -1 -3 -5 3 5 1 2 3 ]
BarPlot
@ Image barplot-example
```

When `Res` is at the default value `0`, the number of bins along the X and Y
axis is set by `StatsPlotBins`.


## HistogramPlot

Create a histogram from a data vector on the stack.

The `PPar` variable defines the horizontal range (as defined by `xrange`) and
the vertical range (as defined by `yrange`). The resolution parameter in `PPar`
defines the size of the bins. With the default value of `0`, the range is
subdivided into 25 bins.

```rpl
1 RDZ
ClΣ
1 1000 for i
  RAND 21 * 10 - Σ+
next
RclΣ
-1 100 YRANGE
HistogramPlot
@ Image histogramplot-example
```

When `Res` is at the default value `0`, the number of bins along the X and Y
axis is set by `StatsPlotBins`.
# Operations with Ranges

Operations on ranges include traditional mathematical operations such as `+` or
`sin`, standard object operations such as `Obj→`, as well as specific operations
listed below.

## →Range

Build a range (interval in the form `a…b`) out of two individual components for
the low and high value:

```rpl
2 3 →Range
@ Expecting 2…3
```

Input values are sorted so that the range is normalized:

```rpl
3 2 →Range
@ Expecting 2…3
```

This function can also convert another type of range to an interval:

```rpl
6±4 →Range
@ Expecting 2…10
```

## →∆Range

Build a delta range (range of the form `a±b`) out of two individual components
for the center and delta value.

```rpl
4 6 →∆Range
@ Expecting 4±6
```

Negative values for the second argument are converted to positive:

```rpl
6 -4 →∆Range
@ Expecting 6±4
```

This function can also convert another type of range to delta form:

```rpl
6±50% →∆Range
@ Expecting 6±3
```

## →%Range

Build a percentage range (range of the form `a±b%`) out of two individual
components for the center value and percentage of change:

```rpl
10 20 →%Range
@ Expecting 10±20%
```

Input values are sorted so that the range is normalized.

```rpl
1 -200 →%Range
@ Expecting 1±200%
```

This function can also convert another type of range to percentage form:

```rpl
0…4 →%Range
@ Expecting 2±100%
```

## →σRange

Build an uncertain number (range of the form `a±σb`) out of two individual
components for the mean and standard deviation value.

```rpl
100 2 →σRange
@ Expecting 100±σ2
```

Input values are not sorted, since a large mean may have a smaller standard
deviation.

Uncertain numbers cannot be converted to and from other types of range.


## RangeUnion

Perform the union of two ranges

```rpl
1…3 2…6 ∪
@ Expecting 1…6
```


## RangeIntersect

Perform the intersection of two ranges

```rpl
1…3 2…6 ∩
@ Expecting 2…3
```
# SD Card

## SDRESET
Reset the file system module


## SDSETPART
Set active partition


## SDSTO
Store a an object into a file


## SDRCL
Recall an object from a file


## SDCHDIR
Change current directory


## SDUPDIR
Change to parent directory


## SDCRDIR
Create a new directory


## SDPGDIR
Delete an entire directory


## SDPURGE
Delete a file


## SDOPENRD
Open a file for read-only operation


## SDOPENWR
Open a file for writing


## SDOPENAPP
Open a file in append mode


## SDOPENMOD
Open a file in modify mode


## SDCLOSE
Close an open file


## SDREADTEXT
Read text from an open file (UTF-8 encoding)


## SDWRITETEXT
Write text to a file (UTF-8 encoding)


## SDREADLINE
Read one line of text from a file


## SDSEEKSTA
Move position to given offset from start of file


## SDSEEKEND
Move position to given offset from end of file


## SDSEEKCUR
Move position to given offset from the current point.


## SDTELL
Get the current position


## SDFILESIZE
Get the file size in bytes


## SDEOF
Return true if last operation reached end of file


## SDOPENDIR
Open a directory to scan entries


## SDNEXTFILE
Get the next entry in a directory that is a file


## SDNEXTDIR
Get the next entry in a directory that is a subdirectory


## SDNEXTENTRY
Get the next entry in a directory


## SDMOVE
Move or rename a file


## SDCOPY
Copy a file


## SDPATH
Get the path to current directory


## SDFREE
Get the free space in the current volume


## SDARCHIVE
Create a full calculator backup on a file


## SDRESTORE
Restore from a backup stored in a file


## SDGETPART
Get the current partition number

# Settings

The calculator has a number of user-configurable settings:

* [Display](#display-settings)
* [Angles](#angle-settings)
* [Command display](#command-display)
* [Decimal separator](#decimal-separator-settings)
* [Precision](#precision-settings)
* [Base](#base-settings)
* [User interface](#user-interface)
* [Compatibility](#compatibility)

The current preferences can be retrieved and saved using the `Modes` command.

## Modes

Returns a program that will restore the current settings.
This program can be saved into a variable to quickly restore user settings.
Use this command along with `ResetModes` to restore settings, as shown in the
example below, which displays `1.3` in `FIX` mode with 2 digits, then restores
the settings and displays `1.3` again using the user's initial settings:

```rpl
«
    Modes
    → M
    «
        @ Fixed mode
        2 FixedDisplay
        1.3 1 DrawText

        @ Restore standard mode
        ResetModes M Evaluate
        1.3 2 DrawText
    »
»
```

Note that the calculator automatically restores the mode when it
[loads a state](#States).

## ResetModes

Reset all the settings to their default value.

This can be used in combination with a stored value from the `Modes` command in
order to save and restore user preferences that a program may modify.

The following code creates a `SaveModes` program that evaluates a program on the stack and restores the settings to what they were before:

```rpl
«
    Modes
    → P M
    «
        P ResetModes M Evaluate
    »
» 'SafeRun' STO


@ Example of use: change display mode twice, restore it afterwards
1.3
«
    6 ENG
    «
        2 FIX
        1.3 1 DISP
    »
    SafeRun

    1.3 2 DISP
» SafeRun

@ Expecting: 1.3
```

## ModesMenu

The `ModesMenu` controls the primary modes of the calculator.

It includes commands to select [trigonometric angle units](#angle-settings), as
well as submenus for mathematical (`MathModesMenu`), user interface
(`UIModesMenu`) or display separators (`SeparatorModesMenu`) preferences .

# Display settings

The display mode controls how DB48X displays numbers. Regardless of the display
mode, numbers are always stored with full precision.

DB48X has five display mode (one more than the HP48)s:

* [Standard mode](#StandardDisplay)
* [Fixed mode](#FixedDisplay)
* [Scientific mode](#ScientificDisplay)
* [Engineering mode](#EngineeringDisplay)
* [Significant digits mode](#SignificantDisplay)

DB48X also features digit [grouping and spacing](#display-grouping-and-spacing)


## DisplayModesMenu

The `DisplayModesMenu` is accessible through 🟨_O_ and gives a quick access to
the various [display settings](#display-settings).

## StandardDisplay

Display numbers using full precision. All significant digits to the right of the
decimal separator are shown, up to 34 digits.

## FixedDisplay

Display numbers rounded to a specific number of decimal places.

## ScientificDisplay

Display numbers in scientific notation, i.e. with a mantissa and an
exponent. The mantissa has one digit to the left of the decimal separator and
shows the specified number of decimal places.

## EngineeringDisplay

Display numbers as a mantissa with a specified number of digits, followed by an
exponent that is a multiple of 3.

## SignificantDisplay

Display up to the given number of digits without trailing zero. This mode is
useful because DB48X can compute with large precision, and it may be useful to
not see all digits. `StandardDisplay` is equivalent to `34 SignificantDisplay`,
while `12 SignificantDisplay` should approximate the HP48 standard mode using
12 significant digits.

## StandardExponent

Select the maximum exponent before switching to scientific notation. The default value is 9, meaning that display uses scientific notation for exponents outside of -9..9.

## MinimumSignificantDigits

Select the minimum number of significant digits before switching to scientific
notation in `FIX` and `SIG` mode. The default value is `3`, meaning that at
least 3 significant digits will be shown.

A value of 0 is similar to how HP calculators before the HP Prime perform. For
example, with `2 FIX`, the value `0.055` will display as `0.06`, and `0.0055`
will display as `0.01`.

A higher value will switch to scienfic mode to show at least the given number of
digits. For instance, with `2 FIX`, if the value is `1`, then `0.055` will still
display as `0.06` but `0.0055` will display as `5.50E-3`. If the value is `2`,
then `0.055` will display as `5.5E-2`. A setting of `1` correspond to what the
HP Prime does.

A value of `-1` indicates that you do not want `FIX` mode to ever go to
scientific notation for negative exponents. In that case, `0.00055` will display
as `0.00`. This corresponds to how older HP calculators render numbers.


## TrailingDecimal

Display a trailing decimal separator to distinguish decimal from integer types. With this setting, `1.0` will display as `1.`. This can be disabled with [NoTrailingDecimal](#NoTrailingDecimal).


## NoTrailingDecimal

Hide the trailing decimal separator for decimal values with no fractional part. In that mode, `1.0` and `1` will both display identically, although the internal representation is different, the former being a floating-point value while the latter is an integer value.

## FancyExponent

Display the exponent in scientific mode using a fancy rendering that is visually similar to the normal mathematical notation.

## ClassicExponent

Display the exponent in scientific mode in a way reminiscent of classical HP48 calculators, for example `1.23E-4`.

## LeadingZero

Display a leading zero for fractional decimal values, i.e. display `0.5` and not
`.5`. This corresponds to the way most HP calculators display decimal values,
with the notable exception of RPL calculators in ` STD ` mode.

## NoLeadingZero

Do not display the leading zero for fractional decimal values, i.e. display `.5`
and not `0.5`. This corresponds to the way HP RPL calculators display numbers in
` STD ` mode.

## MixedFractions

Display fractions as mixed fractions when necessary, e.g. `3/2` will show up as `1 1/2`.

## ImproperFractions

Display fractions as improper fractions, e.g. `3/2` will show up as `3/2` and not `1 1/2`.

## SmallFractions

Show fractions using smaller characters, for example `¹²/₄₃`

## BigFractions

Show fractions using regular characters, for example `12/43`

## ModernBasedNumbers

Display based numbers using the modern DB48x syntax, i.e. `#12AB₁₆` for
hexadecimal on the stack, and `16#12AB` on the command line.
This is the opposite of `CompatibleBasedNumbers`.

## CompatibleBasedNumbers

Display based numbers using the HP syntax, i.e. `#12ABh` for hexadecimal.
This is the opposite of `ModernBasedNumbers`.

## ShowAsDecimal

Show integer numbers like `25` and fractions like `3/2` as decimal values.
This enables formatting with `FIX` or `SCI` to apply to integer numbers and
fractions. For example, if `25` is on the stack, after `2 FIX`, it will show on
the stack as `25.00`.

This is the opposite of `ShowIntegersAndFractions`.

## ShowIntegersAndFractions

Show integer numbers like `25` and fractions like `3/2` as is.
With this setting in effect, `25` after `2 FIX` will show as `25` and not
`25.00`.

# Display Grouping and Spacing

DB48X can group digits in a way similar to the HP business calculators like the
HP17B. The DB48X version is fully configurable through the `SeparatorModesMenu`.

## SeparatorModesMenu

This menu contains the configuration of separators used when displaying numbers.

## MantissaSpacing

Select the spacing for the non-fractional part of the mantissa.

The default value is `3`, meaning that a spacing separator is inserted every
third digit. `123456789` will show as `123 456 789`. After `4 MantissaSpacing`,
it would show as `1 2345 6789`. A value of `0` disables spacing.

## FractionSpacing

Select the spacing for the fractional part of the mantissa.

The default value is `5`, meaning that a spacing separator is inserted every
fifth digit. `1.23456789` will show as `1.23456 789`. After `3 FractionSpacing`,
it would show as `1.234 567 89`. A value of `0` disables spacing.

## BasedSpacing

Select the spacing for based numbers.

The default value is `4`, meaning that a spacing separator is inserted every
fourth digit. `#1234ABCDE` will show as `#1 234A BCDE₁₆`.
After `2 BasedSpacing`, it would show as `#1 23 4A BC DE₁₆`.
A value of `0` disables spacing.

## NumberSpaces

Separate digits with thin spaces. This is the default.

For example, `1234.567890123` will display as `1 234.56789 012`.

## NumberDotOrComma

Separate digits with dots if `DecimalComma` is active, and with commas if
`DecimalDot` is active.

For example, `1234.567890123` will display as `1,234.56789,012` when the decimal
separator is a `.`, and as `1.234,56789.012` if it is `,`.

## NumberTicks

Separate digits with ticks `’`.

For example, `1234.567890123` will display as `1’234’567’890’123`.

## NumberUnderscore

Separate digits with underscores `_`.

For example, `1234.567890123` will display as `1_234_567_890_123`.


## DecimalDot

Select the dot as a decimal separator, e.g.  `1.23`

## DecimalComma

Select the comma as a decimal separator, e.g.  `1,23`


# Angle settings

The angle mode determines how the calculator interprets angle arguments and how
it returns angle results.

DB48X has four angle modes:

* [Degrees](#Degrees): A full circle is 360 degress
* [Radians](#Radians): A full circle is 2π radians
* [Grads](#Grads): A full circle is 400 radians
* [PiRadians](#PiRadians): Radians shown as multiple of π

## Degrees

Select degrees as the angular unit. A full circle is 360 degrees.

## Radians

Select radians as the angular unit. A full circle is 2π radians,
and the angle is shown as a numerical value.

## Grads

Select grads as the angular unit. A full circle is 400 grads.

## PiRadians

Select multiples of π as the angular unit. A full circle is 2π radians,
shown as a multiple of π.

## SetAngleUnits

When this setting is active, inverse trigonometric functoins `asin`, `acos` and
`atan` return a unit value with a unit corresponding to the current `AngleMode`.
This makes it possible to have values on the stack that preserve the angle mode
they were computed with. The opposite setting is `NoAngleUnits`.

Note that the `sin`, `cos` and `tan` will copmute their value according to the
unit irrespective of this setting. In other words, `30_° SIN` will always give
`0.5`, even when computed in `Rad` or `Grad` mode,

## NoAngleUnits

This is the opposite setting to `SetAngleUnits`. Inverse trigonometric functions
behave like on the original HP-48 calculator, and return a numerical value that
depends on the current angle mode.


# Command display

DB48X can display commands either using a short legacy spelling, usually
identical to what is used on the HP-48 series of calculators, or use an
alternative longer spelling. For example, the command to store a value in a
variable is called `STO` in the HP-48, and can also be spelled `Store` in DB48X.

Commands are case insensitive, and all spellings are accepted as input
irrespective of the display mode.

DB48X has four command spelling modes:

* [Lowercase](#LowerCase): Display ` sto `
* [Uppercase](#UpperCase): Display ` STO `
* [Capitalized](#Capitalized): Display ` Sto `
* [LongForm](#LongForm): Display ` Store `

There are four parallel settings for displaying a variable name such as
`varName`:

* [LowercaseNames](#LowerCaseNames): Display as ` varname `
* [UppercaseNames](#UpperCaseNames): Display as ` VARNAME `
* [CapitalizedNames](#CapitalizedNames): Display as ` VarName `
* [LongFormNames](#LongFormNames): Display as ` varName `


## LowerCase

Display comands using the short form in lower case, for example `sto`.

## UpperCase

Display comands using the short form in upper case, for example `STO`.

## Capitalized

Display comands using the short form capitalized, for example `Sto`.

## LongForm

Display comands using the long form, for example `Store`.

## LowerCaseNames

Display names using the short form in lower case, for example `varName` will show as `varname`.

## UpperCaseNames

Display names using the short form in upper case, for example `varName` will show as `VARNAME`.

## CapitalizedNames

Display names using the short form capitalized, for example `varName` will show as `VarName`.

## LongFormNames

Display names using the long form, for example `varName` will show as `varName`.



# Precision settings

## Precision

Set the default computation precision, given as a number of decimal digits. For
example, `7 Precision` will ensure at least 7 decimal digits for computation,
and `1.0 3 /` will compute `0.3333333` in that case.

DB48X supports an arbitrary precision for [decimal numbers](#decimal-numbers),
limited only by memory, performance and the size of built-in constants needed
for the computation of transcendental functions.

## SolverImprecision

Set the number of digits that can be ignored when solving. The default value is
6, meaning that if the current precision is 24, we only solve to an accuracy of
18 digits (i.e. 24-6).

See also `IntegrationImprecision`

## MaxNumberBits

Define the maxmimum number of bits for numbers.

Large integer operations can take a very long time, notably when displaying them
on the stack. With the default value of 1024 bits, you can compute `100!` but
computing `200!` will result in an error, `Number is too big`. You can however
compute it seting a higher value for `MaxNumberBits`, for example
`2048 MaxNumberBits`.

This setting applies to integer components in a number. In other words, it
applies separately for the numerator and denominator in a fraction, or for the
real and imaginary part in a complex number. A complex number made of two
fractions can therefore take up to four times the number of bits specified by
this setting.

## MaxFactorsBits

Maximum number of bits for integers accepted by prime-related commands:
`IsPrime`, `Factors`, `NextPr`, and `PrevPr`.

The value can range from 64 to 1024 bits (default 160). Integers exceeding this
limit trigger a `Number is too big` error. Raising the value allows factoring
larger numbers but increases memory use and computation time for Pollard's Rho
and Miller-Rabin. Lowering it can prevent excessive CPU usage on large inputs.

## MaxFactorIterations

Maximum number of Pollard's Rho iterations per attempt when factoring integers
with `Factors`.

The value can range from 1024 to 16,777,216 (default 4,194,304). If no factor
is found within this limit for a given Rho attempt, the algorithm tries the
next random starting point. Exhausting all attempts yields a `Bad argument
value` error. Lowering the value speeds up failure detection for numbers that
are hard to factor (e.g. products of two large primes); raising it allows
factoring tougher semiprimes at the cost of longer run time.

## SolverIterations

Number of times the solver will try to find a solution.

## SolverImprecision

Relative imprecision that is tolerated by the solver

## SolverShuffles

Number of times the solver will shuffle the test vector around errors and
singularities.


## MathModesMenu

The `MathModesMenu` controls settings related to mathematical computations.

* `SymbolicResults`
* `AutoSimplify`


# Base settings

Integer values can be reprecended in a number of different bases:

* [Binary](#Binary) is base 2
* [Ocgtal](#Octal) is base 8
* [Decimal](#Decimal) is base 10
* [Hexadecimal](#Hexadecimal) is base 16

## Binary

Selects base 2

## Octal

Selects base 8

## Decimal

Selects base 10

## Hexadecimal

Selects base 16

## Base

Select an arbitrary base for computations

## WordSize

Store the current [word size](#wordsize) in bits. The word size is used for
operations on based numbers. The value must be greater than 1, and the number of
bits is limited only by memory and performance.

## RecallWordSize

Return the current [word size](#wordsize) in bits.


# Command tuning

Various settings can be used to tune specific commands.
See also `IntegrationIterations`

## MaxRewrites

Defines the maximum number of rewrites in an equation.

[Equations rewrites](#rewrite) can go into infinite loops, e.g. `'X+Y' 'A+B'
'B+A' rewrite` can never end, since it keeps rewriting terms. This setting
indicates how many attempts at rewriting will be done before erroring out.

## →QIterations

Define the maximum number of iterations converting a decimal value to a
fraction. For example, `1 →FracIterations 3.1415926 →Frac` will give `22/7`,
whereas `3 →FracIterations 3.1415926 →Frac` will give `355/113`.

## →QDigits

Define the maximum number of digits of precision converting a decimal value to a
fraction. For example, `2 →FracDigits 3.1415926 →Frac` will give `355/113`.

## →QπMaxPrime

Define the largest prime used when extracting square factors during [→Qπ](#toqπ)
conversion. When converting a decimal to a rational form with π, √*n*, ln, or *e*
factors, the algorithm squares the value, converts to a fraction, then factors out
perfect squares from the numerator and denominator. This setting limits which
primes are tried (2 to 10000, default 100). Lower values speed up conversion on
DM32/DM42 at the cost of missing some √*n* simplifications for numbers whose
square has large prime factors.


# User interface

Various user-interface aspects can be customized, including the appearance of
Soft-key menus. Menus can show on one or three rows, with 18 (shifted) or 6
(flat) functions per page, and there are two possible visual themes for the
labels, rounded or square.

## Header

The `Header` command updates a special variable also called `Header`.

When that variable is present, it must evaluate to something that can render
graphically, either directly a graphic object or a (possibly multi-line) text.

When a header is provided, the normal content of the header, i.e. date, time and
name of the state file, is no longer shown.  However, annunciators and battery
status are still overimposed.

It is the responsibility of the programmer to ensure that the header program
does not draw important data at these locations, and also to make sure that the
header program is "well behaved", i.e. does not leave things on stack. If the
header program generates an error, then that error may get in the way of normal
calculator operations.

For example, the following shows the current time, the current path, the date
and free memory in a two-line header, with a 10 second `CustomHeaderRefresh`.

```rpl
« TIME " " PATH TAIL TOTEXT + + "
" + DATE + " Mem: " + MEM + »
HEADER
10000 CustomHeaderRefresh
```

## CustomHeaderRefresh

This setting indicates how frequently a custom `Header` should be evaluated. The
time interval is expressed in milliseconds.

## ThreeRowsMenus

Display menus on up to three rows, with shift and double-shift functions showns
above the primary menu function.

## SingleRowMenus

Display menus on a single row, with labels changing using shift.

## FlatMenus

Display menus on a single row, flattened across multiple pages.

## RoundedMenus

Display menus using rounded black or white tabs.

## SquareMenus

Display menus using square white tabs.

## CursorBlinkRate

Set the cursor blink rate in millisecond, between 50ms (20 blinks per second)
and 5000ms (blinking every 5 seconds).

## ShowBuiltinUnits

Show built-in units in the `UnitsMenu` even when a units file was loaded.

## HideBuiltinUnits

Hide built-in units in the `UnitsMenu` when a units file was loaded.
The built-in units will still show up if the units file fails to load.

## MultiLineResult

Show the result (level 1 of the stack) using multiple lines.
This is the opposite of [SingleLineResult](#singlelineresult).
Other levels of the stack are controled by [MultiLineStack](#multilinestack)

## SingleLineResult

Show the result (level 1 of the stack) on a single line.
This is the opposite of [MultiLineResult](#multilineresult).
Other levels of the stack are controled by [SingleLineStack](#singlelinestack)

## MultiLineStack

Show the levels of the stack after the first one using multiple lines.
This is the opposite of [SingleLineStack](#singlelinestack).
Other levels of the stack are controled by [MultiLineResult](#multilineresult)

## SingleLineStack

Show the levels of the stack after the first one on a single line
This is the opposite of [MultiLineStack](#multilinestack).
Other levels of the stack are controled by [SingleLineResult](#singlelineresult)

## GraphicResultDisplay

Display the first level of the stack (result) using a graphical representations
for objects such as expressions or matrices. Note that enabling this setting may
increase CPU utilization and reduce battery life compared to text-only
rendering.

This is the opposite of [TextResultDisplay](#textresultdisplay)

## TextResultDisplay

Display the first level of the stack (result) using a text-only representations.

This is the opposite of [TextResultDisplay](#textresultdisplay)

## GraphicStackDisplay

Display the stack levels above the first one using a graphical representations
for objects such as expressions or matrices. Note that enabling this setting may
increase CPU utilization and reduce battery life compared to text-only
rendering.

This is the opposite of [TextStackDisplay](#textstackdisplay)

## TextStackDisplay

Display the stack levels above the first one using a text-only representation.

This is the opposite of [GraphicStackDisplay](#graphicstackdisplay)

## AutoScaleStack

When using [graphic result display](#graphicresultdisplay), automatically scale
down the font size in order to make stack elements fit. Enabling this setting
may increase CPU utilization and reduce battery life compared to fixed-size
rendering.

This is the opposite of [NoStackAutoScale](#nostackautoscale).


## NoStackAutoScale

When using [graphic result display](#graphicresultdisplay), do not automatically
scale down the font size in order to make stack elements fit.

This is the opposite of [AutoScaleStack](#autoscalestack).

## MaximumShowWidth

Maximum number of horizontal pixels used to display an object with
[Show](#show).

## MaximumShowHeight

Maximum number of vertical pixels used to display an object with [Show](#show).

## EditorWrapColumn

Column at which the editor will start wrapping long lines of code.
Wrapping occurs at the end of an object, not in the middle of it.

## TabWidth

Width of a tab in the editor, in pixels.

## ExitKeepsMenu

By default, the `EXIT` key clears the current menu if not editing.
When `ExitKeepsMenu` is set, the `EXIT` key does not clear the menu.

## ExitClearsMenu

Restore the default behaviour where `EXIT` clears the current menu when not
editing.

## ShowEmptyMenu

Show empty menu entries. For example, when selecting the `VariablesMenu` and
there is no variable defined, an empty menu shows up.

## HideEmptyMenu

Restore the default behaviour where empty menus entries are not shown, leaving
more space for the stack display.


# Statistics settings

## LinearFitSums

When this setting is active, statistics functions that return sums, such as
`ΣXY` or `ΣX²`, operate without any adjustment to the data, i.e. as if the
fitting model in `ΣParameters` was `LinearFit`.

## CurrentFitSums

When this setting is active, statistics functions that return sums, such as
`ΣXY` or `ΣX²`, will adjust their input according to the current fitting model
in special variable `ΣParameters`, in the same way as required for
`LinearRegression`.

## RandomGeneratorBits

Define the number of random bits generated by ACORN random number generator,
between 32 and 4096 bits. The default value is 128 bits. A lower value uses less
memory and accelerates computations, but some digits in the resulting numbers
may not be adequately random. Conversely, a higher value increases true
randomness in the resulting numbers and should be used if you need random
numbers with higher [Precision](#precision) than the default 24 digits.

For example, with a value of 64, the generated random numbers will very often
contain zeros after the 12th digit. The reason is that 64 bits of randomness
corresponds to only 20 digits, which is less than the default 24-digits
precision of DB48X.

## RandomGeneratorOrder

Define the order of the ACORN random number generator, i.e. the value called `k`
in the ACORN documentation. This is the number of seed numbers preserved between
generations. A higher value requires more memory and takes more time to compute
but generates higher-quality random numbers. On DB48X, the ACORN order can be
set between 10 and 256. The default value is 32.


# Compatibility settings

Various settings control the compatibility of DB48X with various classes of HP calculators.

## DetailedTypes

The `Type` command returns detailed DB48X type values, which can distinguish
between all DB48X object types, e.g. distinguish between polar and rectangular
objects, or the three internal representations for decimal numbers. Returned
values are all negative, which distinguishes them from RPL standard values, and
makes it possible to write code that accepts both the compatible and detailed
values.

This is the opposite of [CompatibleTypes](#compatibletypes).

## CompatibleTypes

The `Type` command returns values as close to possible to the values documented
on page 3-262 of the HP50G advanced reference manual. This is the opposite of
[NativeTypes](#nativetypes).

## CompatibleGROBs

When this flag is set, graphic operations will generate HP48 compatible GROB
objects. Note that HP-compatible objects take slightly more memory. This is the
opposite of `PackedBitmaps`.

## PackedBitmaps

When this flag is set, graphic operations will generate DB48x graphic objects,
which are slightly denser and more efficient than HP graphic objects, notably
when the width is not a multiple of 8. For example, if you render the number 32
in HP format, it takes 328 bytes, vs. only 281 bytes in DB48x format.  This is
the opposite of `CompatibleGROBs`.

## NumberedVariables

This flag enables numbered variables similar to what existed on earlier RPN calculators. For example, when the setting is active, `2.5 0 STO` stores the value 2.5 in numbered register `0`.

## NoNumberedVariables

This flag disables numbered variables, behaving closer to the way RPL calculators work. For example, when the setting is active, `2.5 0 STO` generates an `Invalid name` error.

## IgnoreSymbolCase

Ignore the case in symbols, i.e. variables `X` and `x` are the same.
Note that this is different from the way RPL in HP calculators works.

## DistinguishSymbolCase

Distinguish the case in symbols, i.e. variables `X` and `x` are distinct.
This is the way RPL in HP calculators works.


## ListAsProgram

When this setting is set, DB48X behaves like the HP48S and later HP devices and
evaluates lists as if they were programs. For example, `{ 1 2 + } EVAL` returns
`3`. The default is [ListAsData](#listasdata).

## ListAsData

When this setting is set, DB48X behaves like the HP28 and evaluates lists as
data. For example, `{ 1 2 + } EVAL` returns `{ 1 2 + }`.


## KillOnError

An error kills the program without giving you the possibility to debug the
problem. If you want to debug, you need to start the program with `Debug` and
then use the `Continue` command.


## DebugOnError

An error during a program enters the debugger, letting you correct the problem
before resuming execution. This is the default setting.


## TruthLogicForIntegers

When this flag is set, [logical operations](#logical-operations) such as `and`
or `not` applied to integers return `True` or `False`, for compatibility with HP
implementations of RPL.

The opposite setting is `BitwiseLogicForIntegers`.

## BitwiseLogicForIntegers

When this flag is set, [logical operations](#logical-operations) such as `and`
or `not` applied to integers return a bitwise numerical result, which deviates
from the HP implementations of RPL.

The opposite setting is `TruthLogicForIntegers`.

## ListRecursionDepth

This setting selects the depth of recursion for `Map`, `Reduce` or `Filter`.
By default, it is set to `1`, indicating that recursion only applies to the
top-level of the list. This matches the way `Map`, `Reduce` and `Filter` work in
most programming languages.

A value larger than `1` can be used to recurse beyond the first level.
On the HP50G, `Map` applies recursively to all levels. This can be achieved by
setting `ListRecursionDepth` to `0`.


# Evaluation settings

The following settings are related to evaluation of programs and objects.

## SaveLastArguments

Save last arguments when evaluating commands interactively.
This is disabled by `NoLastArguments`.

Saving arguments during program execution is controlled by a separate setting,
`ProgramLastArguments`.

## NoLastArguments

Disable the saving of last arguments when interactively evaluating commands.

## ProgramLastArguments

Save last arguments during program execution. This may impact performance,
and is disabled by default (`NoProgramLastArguments`).

## NoProgramLastArguments

Disable the saving of command arguments during program execution.
If `SaveLastArguments` is set, arguments to interactive commands will still be
saved.


# Plot settings

The following settings are related to plotting

## GraphingTimeLimit

Maximum number of milliseconds that can be spent rendering an object
graphically. The default is 250ms.

## ShowTimeLimit

Maximum number of milliseconds that can be spent rendering an object for the
`Show` command. The default is 10000 (10s)

## ResultGraphingTimeLimit

Maximum amount of time that can be spent rendering the result (level 1 of the
stack) graphically. The default value is 1500 (1.5s)

## StackGraphingTimeLimit

Maximum amount of time that can be spent rendering the levels of the stack above
level 1. The default value is 250ms.

## TextRenderingSizeLimit

Limit in bytes for the size of objects to be rendered on the stack. Objects that are larger than this size are shown on the stack as something like
`Large text (399 bytes)`.

## GraphRenderingSizeLimit

Limit in bytes for the size of objects to be rendered on the stack
graphically. Objects that are larger than this size are text.


## XYPlotBins

Number of bins used to draw XY plots (e.g. `TruthPlot`) when the `Resolution` is
at its default value of `0`.


## StatsPlotBins

Number of bins used to draw statistical plots (e.g. `BarPlot`) when the
`Resolution` is at its default value of `0`.


# States

The calculator can save and restore state in files with extension `.48S`.
This feature is available through the `Setup` menu (Shift-`0`).

The following information is stored in state files:

* Global variables
* Stack contents
* Settings
# Numeric solvers

## NUMINT
Numerical integration (adaptive Simpson)


## Root

Find the root of an equation or of a system of equations.

'Expr' 'Var' 'Guess' ▶ 'Solution'

The `Root` command takes an expression or an equation `Expr`, a variable `Var`
and an initial guess for the value `Guess`, and searches for a value of `Var`
that will zero the expression or verify the equation.

For example, to find the value of `x` where the value of `sin(x)` matches that
of `cos(x)/3` close to `1.5_°`, use the following:

```rpl
'sin(x)=cos(x)/3'
'x' 0.5_° ROOT
@ Expecting x=18.43494 88229 °
```

### Guess range

The `Guess` can be given as a list or vector containing two values, in which
case the algorithm will first evaluate the two given values. Otherwise, it will
first evaluate the guess value and a value close to it.


### Multiple variables

Unlike on HP calculators, the DB48x `Root` command can solve for multiple
variables, playing the role of the `Root` command, the _multiple equation
solver_ and the `MROOT` command. The `MROOT` command, which uses the `Eq`
variable, remains available for compatibility.

The `Root` command is adaptative. It will shift to the multiple equation solver
mode, solving one variable at a time, if that is possible

### Multiple equation solver

If equations can be solved one at a time, then the `Root` command will use the
multiple equation solver to solve them in turn.

For example, in the following code, the multiple equation solver can first solve
for `y` using the second equation, the solve for `x`. In both cases, it can use
the `Isolate` command to get an exact expression for the solution.

```rpl
{ 'sin(x)=y' '7*y^3=2' } { x y } { 0 0 } ROOT
@ Expecting { x=41.19575 8315 ° y=0.65863 37560 08 }
```

### Jacobian solver

If equations cannot be solved one at a time, then the `Root` command will
compute the Jacobian of the equations given as input, and use that to solve the
systerm iteratively. This is necessary when there is "crosstalk" between
variables across equations.

For example, to find the coordinates of the intersection between two circles,
you can use the following code:

```rpl
[ 'X^2+Y^2=1' '(X-1)^2+Y^2=1' ] [ X Y ] [ 0 0 ] ROOT
@ Expecting [ X=0.5 Y=0.86602 54037 84 ]
```


### Algebraic isolation

When possible, the numerical solver will attempt to use the `Isolate` command to
find results more rapidly and accurately using an exact symbolic expression of
the solution. This is controlled by the `SymbolicSolver` flag.


### Unit management

Specifying a unit for the initial value forces the calculator to compute the
result using the given unit.

In the example above, the guess was . Otherwise, the result would depend on the
angle mode, e.g. in `Radians` mode:

```rpl
RAD
'sin(x)=cos(x)/3'
'x' 0.5 ROOT
@ Expecting x=0.32175 05543 97
```

### Algebraic form

Unlike the HP version, `ROOT` can be used as an algebraic function,
using a syntax similar to what is used in algebraic mode on the HP50G.

The previous example can be written in algebraic form as follows:

```rpl
'ROOT(sin(x)=cos(x)/3;x;1.5_°)'
EVAL
@ Expecting x=18.43494 88229 °
```

### Solver (im)precision

The desired precision is given by the `SolverImprecision` setting relative to
the current `Precision` setting, allowing the solver to stop earlier. For
example, the default value `6` for `SolverImprecision`, combined with the
default 24-digit `Precision` means that the solver will seek at most 18-digit
precision (i.e. 24-6). Solving precision may be further reduced according to
display settings, like on HP calculators.

For example, the following will find a "solution" to `1/x=0` once it reaches the
desired precision:

```rpl
NumericalSolver
'ROOT(1/x;x;1)' EVAL
SymbolicSolver
@Expect: :x:1.10008 77783 66101 93099 87⁳18
```

### Updating global variables

Whether the solver found a solution or not, `Root` updates the value of the
target variable with the closest value it found so far. When no solution was
found, the value stored in the global variable is where the computed value was
the closest to zero, and is usually close to a local extremum.


### Finding complex solutions

If the guess value for the variable is a complex number, the DB48X version of
`ROOT` will explore the complex plane for solutions. This is an extension
compared to HP implementations of RPL.

For example, you can find a complex root for the following equation:
```rpl
'ROOT((X-5)²+3;X;0+0ⅈ)'
@ Expecting X=5+1.73205 08075 7ⅈ
```

### Differences with HP calculators

Since the root-finding algorithm is different in DB48X, there is no guarantee
that the numerical solver will find the same solution as HP calculators.

According to the HP50G Advanced Reference Manual, `ROOT` produces an error if it
cannot find a solution. However, experimentally, HP calculators will silently
return an extremum, whereas DB48X will indicate if it failed to find a
solution. For example, the following code returns a value close to 5 on HP48SX
or HP50G, but returns an error on DB48X, with the variable `X` containing a
value close to 5.

```rpl
@ Expecting "DB48X"
IFERR
 'sq(X-5)+3' 'X' 0 ROOT
THEN
 "DB48X"
ELSE
 "HP50G"
END
```

As an extension to the HP implementation, `ROOT` can solve systems of equations
and multiple variables by solving them one equation at a time, a programmatic version of what the HP50G Advanced Reference Manual calls the Multiple Equation Solver (`MINIT`, `MITM` and `MSOLVR` commands).

## Solving Menu Solve Key

A variable name followed by "?" will solve the system of equations for the given
variable.

## Solving Menu Recall Key

A variable name followed by ▶ will recall the value of the given equation. The
value is returned as an assignment.

## Solving Menu Store Key

A variable name preceded by ▶ will store the value of the given equation. The
stored value is placed on the stack as an assignment.

## MultipleEquationsSolver

Solve a system of multiple equations simultaneously.

## MultipleVariablesSolver

Solve for multiple variables in a system of equations.

On HP50G, `MSlv` is dedicated to solving systems of equations.
On DB48x, it behaves almost exactly like `Root`, except that it
leaves the equations and variable lists on the stack in addition to the result.

```rpl
RAD
[ 'sin(x)+y' 'x+sin(y)=1' ] [x y] [0 0] MSLV
"" + + +
@ Expecting "[ 'sin x+y' 'x+sin y=1' ][ x y ][ x=1.82384 11261 1 y=-0.96815 46361 75 ]"
```

Since `Root` on DB48x accepts a wider range of inputs and automatically detects
when it needs to solve [systems of simultaneous equations](#jacobian-solver),
`MSLV` will work on cases accepted on the HP50G, but will also accept other
inputs accepted by `Root`, e.g. single variables, and will benefit from symbolic
solving using `isol` (resulting in the generation of an angle units in the
example below):

```rpl
DEG
'sin(x)=0.3' x 0 MSLV
@ Expecting x=17.45760 31237 °
```
# Stack manipulation

## ClearStack

Remove all objects from the stack


## Depth
Get the current stack depth


## Drop
Remove an object from the stack

`X` ▶


## Drop2
Remove two objects form the stack


## DropN
Remove N objects from the stack, N being given in level 1.


## Duplicate
Duplicate an object on the stack


## Duplicate2
Duplicate two objects on the stack


## DuplicateTwice
Duplicate the same object twice on the stack

## DuplicateN
Duplicate a group of N objects, N being given in stack level 1

## LastArguments
Put the last arguments back on the stack.
Command arguments are saved based on the `SaveLastArguments` and
`ProgramLastArguments` setttings.

## LastX
Put the last first argument on the stack.

This command does not exist on HP RPL calculators, and is here to make it easier
to adapt RPN programs that use LastX a bit more often.

## Undo
Restore the stack to its last state before executing an interactive command.
Note that this command can be used from a program, but it will restore the state
prior to program execution.

## NDUPN
Replicate one object N times and return N


## Nip
Remove object at level 2 on the stack


## Over
Duplicate object at level 2 on the stack


## Pick
Duplicate object at position N on the stack


## Pick3
Duplicate object at level 3 on the stack


## Roll
Move object at level N to level 1


## RollD
Move object from level 1 to level N


## Rot
Move object from level 3 to level 1


## Swap
Exchange objects in levels 1 and 2

Mapped to _X⇆Y_ key

`Y` `X` ▶ `X` `Y`


## Unpick
Move object from level 1 to level N.


## UnRot
Move object from level 1 to level 3


## IFT
Evaluate objects on the stack conditionally


## IFTE
Evaluate objects on the stack conditionally


## STKPUSH
Push a snapshot of the current stack on the undo stack


## STKPOP
Pop a stack snapshot from the undo stack


## STKDROP
Drop a snapshot from the undo stack


## STKPICK
Copy snapshot in level N to the current stack


## STKDEPTH
Get the depth of the undo stack


## STKNEW
Push a snapshot of the current stack on the undo stack and clears the current stack
# Statistics

## RDZ

Initialize random number generator with a seed. If the given seed is zero, then
a truly random value based on the internal system clock is used, and the value
changes every millisecond.

For any given non-zero value, the sequence of numbers generated by
[RAND](#rand) or [Random](#random) will always be identical.


## RAND

Generate a random real number between 0 and 1, 1 being excluded.

DB48X uses an additive congruential random number generator (ACORN), which is
configured by two settings, [RandomGeneratorOrder](#randomgeneratororder) and
[RandomGeneratorBits](#randomgeneratorbits), and uses a seed provided by `RDZ`.


## Random

Generate a pseudo-random number between two bounds.

If the two bounds are integers, then the result is an integer, and the upper
bound is included. For example, `2 4 Random` can generate `2`, `3` or `4`.

Otherwise, the upper bound is excluded. For example, `2. 4. Random` can generate
any number between `2.0` and `4.0`, but the upper bound `4.0` cannot be
generated.

DB48X uses an additive congruential random number generator (ACORN), which is
configured by two settings, [RandomGeneratorOrder](#randomgeneratororder) and
[RandomGeneratorBits](#randomgeneratorbits), and uses a seed provided by `RDZ`.

## ΣData

The `ΣData` variable contains the statistics data, in the form of a matrix.

*Note*: The `ΣData` name is considered a command internally, and as such,
is subject to `CommandDisplayMode` and not `NamesDisplayMode`.

## ΣParameters

The `ΣParameters` variable contains the statistics parameters, as a list with
five elements:

`{ xcol ycol intercept slope fit }`

The `xcol` value is an integer starting at 1, indicating the independent column.
The `ycol` value similarly indicates the dependent column.

The `intercept` and `slope` are the parameters for the linear regression.
The `fit` value is the type of fit being used:
(`LinFit`, `ExpFit`, `PwrFit`, `LogFit`);

*Note*: The `ΣParameters` name is considered a command internally, and as such,
is subject to `CommandDisplayMode` and not `NamesDisplayMode`.

## Σ+

Add data to the statistics data array `ΣData`.

* If data is a real or complex number, statistics data is single-column

* If data is a vector, statistics data has the same number of columns as the
  size of the vector.

## Σ-

Remove the last data entered in the statistics array, and pushes it on the stack.

## RecallΣ (RCLΣ)

Recall statistics data and puts it on the stack

## StoreΣ (STOΣ)

Stores an array from the stack as statistics data in the `ΣData` variable.

## ClearΣ (CLΣ)

Clear statistics data.

## Average

Compute the average (mean) of the values in the statistics data.
If there is a single column of data, the result is a real number.
Otherwise, it is a vector for each column of data.

## Median

Compute the median of the values in the statistics data array `ΣData`.

## MinΣ

Compute the smallest of the values in the statistics data array `ΣData`.

## MaxΣ

Compute the largest of the values in the statistics data array `ΣData`.

## ΣSize

Return the number of data rows in the statistics data array `ΣData`.

## ΣX

Return the sum of values in the `XCol` column of the statistics data array
`ΣData`. The values are adjusted according to the current fitting model defined
in `ΣParameters` if the `CurrentFitSums` setting is active.

## ΣY

Return the sum of values in the `YCol` column of the statistics data array
`ΣData`. The values are adjusted according to the current fitting model defined
in `ΣParameters` if the `CurrentFitSums` setting is active.

## ΣXY

Return the sum of the product of values in the `XCol` and `YCol` columns of the
statistics data array `ΣData`. The values are adjusted according to the current
fitting model defined in `ΣParameters` if the `CurrentFitSums` setting is
active.

## ΣX²

Return the sum of the squares of the values in the `XCol` column of the
statistics data array `ΣData`. The values are adjusted according to the current
fitting model defined in `ΣParameters` if the `CurrentFitSums` setting is
active.

## ΣY²

Return the sum of the squares of the values in the `YCol` column of the
statistics data array `ΣData`. The values are adjusted according to the current
fitting model defined in `ΣParameters` if the `CurrentFitSums` setting is
active.

## ΣTotal

Returns the sum of all columns in the statistics data array `ΣData`.

## Variance

Calculates the sample variance of the coordinate values in each of the columns
in the current statistics matrix (`ΣData`).

## Correlation

Returns the correlation coefficient of the independent and dependent data
columns in the current statistics matrix (reserved variable `ΣData`).

The columns are specified by the first two elements in the reserved variable
`ΣParameters`, set by `XCol` and `YCol`, respectively. If `ΣParameters` does not
exist, `Correlation` creates it and sets the elements to their default values
(1 and 2).

## Covariance

Returns the sample covariance of the independent and dependent data columns in
the current statistics matrix (reserved variable `ΣData`).

The columns are specified by the first two elements in the reserved variable
`ΣParameters`, set by `XCol` and `YCol`, respectively. If `ΣParameters` does not
exist, `Correlation` creates it and sets the elements to their default values
(1 and 2).

## StandardDeviation

Calculates the sample standard deviation of each of the columns of coordinate values in the current statistics matrix (reserved variable `ΣData`).

`StandardDeviation`  returns a vector of numbers, or a single number there is only one column of data.

The standard deviation is the square root of the `Variance`.

## LinearRegression

Uses the currently selected statistical model to calculate the linear regression
coefficients (intercept and slope) for the selected dependent and independent
variables in the current statistics matrix (reserved variable `ΣData`).

The columns of independent and dependent data are specified by the first two
elements in the reserved variable `ΣParameters`, set by `XCol` and `YCol`, respectively.
The default independent and dependent columns are 1 and 2.

The selected statistical model is the fifth element in `ΣParameters`.
LR stores the intercept and slope (untagged) as the third and fourth elements,
respectively, in `ΣParameters`.

The coefficients of the exponential (`ExpFit`), logarithmic (`LogFit`),
and power (`PwrFit`) models are calculated using transformations that allow
the data to be fitted by standard linear regression.

The equations for these transformations are:

* `LinFit`: `y = slope * x + intercept`
* `LogFit`: `y = slope * ln(x) + intercept`
* `ExpFit`: `y = intercept * exp(slope * x)`
* `PwrFit`: `y = intercept * x ^ slope`

where b is the intercept and m is the slope. The logarithmic model requires
positive x-values (XCOL), the exponential model requires positive y-values
(YCOL), and the power model requires positive x- and y-values.

## Intercept

Return the intercept value last computed by `LinearRegression`

This is a DB48X extension, not present on HP calculators

## Slope

Return the slope value last computed by `LinearRegression`

This is a DB48X extension, not present on HP calculators

## BestFit

Select the best linear regression mode based on current data, i.e. the
regression mode where the correlation value is the highest.

## LinearFit

Select linear fit, i.e. try to model data with a linear equation `y = a*x+b`.

## ExponentialFit

Select exponential fit, i.e. try to model data with an equation `y = b*exp(a*x)`

## LogarithmicFit

Select logarithmic fit, i.e. try to model data with an equation `y = a*ln(x)+b`.

## PowerFit

Select power fit, i.e. try to model data with an equation `y = x^a * b`.


## FrequencyBins

Sort into frequency bins.

Sorts the elements of the independent column (`XCol`) of the current statistics
matrix (the reserved variable `ΣData`) into `nbins + 2` bins, where the left
edge of bin 1 starts at value `xmin` and each bin has width `xwidth`.

`FrequencyBins` returns a matrix containing the frequency of occurrences in each
bin, and a 2-element array containing the frequency of occurrences falling below
or above the defined range of x-values.

*Note*: The array can be stored into the reserved variable `ΣData` and used to
plot a bar histogram of the bin data, for example, by executing `BarPlot`,
as suggested in the *HP50G Advanced Reference Manual*. However, this binning is
performed directly by the `HistogramPlot` command.

`xmin` `xwidth` `nbins` ▶ `[[ n1 .. n2 ]]` `[ nlow nhigh ]`

In the following example, the data is sorted into 5 bins of width 2, starting at
x-value of 1 and ending at x-value 11. The first element of the resulting matrix
shows that 5 x-values (namelly 2, 1, 1, 1 and 2) fell in bin 1, where bin 1
ranges from x-value `1` inclusive through value `3` exclusive. The outliers
vectors shows that one x-value was less than the minimum 1, and one was greater
than the maximum value 13.

```rpl
[ 7 2 3 1 4 6 9 0 1 1 3 5 13 2 6 9 5 8 5 ] StoΣ
1 2 5 BINS
2 ToList
@ Expecting { [[ 5 ] [ 3 ] [ 5 ] [ 2 ] [ 2 ]] [ 1 1 ] }
```


## PopulationVariance

Calculates the population variance of the coordinate values in each of the m
columns in the current statistics matrix (`ΣData`).

The population variance (equal to the square of the population standard
deviation) is returned as a vector of m real numbers, or as a single real number
if there is a single column of data.

## PopulationStandardDeviation

Calculates the population standard deviation of each of the m columns of
coordinate values in the current statistics matrix (reserved variable `ΣData`).

The command returns a vector of m real numbers, or a single real number if there
is a single column of data.

## PopulationCovariance

Computes the population covariance of the independent and dependent data columns
in the current statistics matrix (reserved variable `ΣData`).

The columns are specified by the first two elements in reserved variable
`ΣParameters`, set by `XCol` and `YCol` respectively. If `ΣParameters` does not
exist, `PCOV` creates it and sets the elements to their default values, 1 and 2.

## IndependentColumn

Set the independent variable column in the reserved variable `ΣParameters`.

`XCol` ▶ (Update `ΣParameters`)

## DependentColumn

Set the dependent variable column in the reserved variable `ΣParameters`.

`YCol` ▶ (Update `ΣParameters`)

## DataColumns

Set both the independent and dependent data columns in the reserved variable
`ΣParameters`.

`XCol` `YCol` ▶ (Update `ΣParameters`)
# Operations with Symbolic Expressions

## ↑Match

Match pattern up and [rewrite expression](#expression-rewrite), starting with
the innermost subexpressions first. This approach works well for
simplifications. A subexpression simplified during one execution will be a
simpler argument of its parent expression, so the parent expression can be
simplified further.

`Expr` { `From` `To` } ▶ `Expr` `Count`
`Expr` { `From` `To` `Cond` } ▶ `Expr` `Count`

The first argument `Expr` is the expression to transform.

The second argument is a list containing the pattern to match `From`,
the replacement pattern `To`,
and an optional condition `Cond` on the pattern.


## ↓Match

Match pattern down and [rewrite expression](#expression-rewrite), starting with
the outermost expression first. This approach works well for expansion. An
expression expanded during one execution of `↓Match` will contain additional
subexpressions, and those subexpressions can be expanded further.

`Expr` { `From` `To` } ▶ `Expr` `Count`
`Expr` { `From` `To` `Cond` } ▶ `Expr` `Count`

The first argument `Expr` is the expression to transform.

The second argument is a list containing the pattern to match `From`,
the replacement pattern `To`,
and an optional condition `Cond` on the pattern.

## Expression rewrite

Operations such as `↑Match` and `↓Match` apply arbitrary transformations on
algebraic expressions. The way this operates is similar to how HP
calculators perform, but with important differences, controlled by flags.

First, patterns can contain wildcards, which are substituted with the
corresponding sub-expression in the matched `Expr`.

* On HP calculators, the wildcard names must begin with `&`, and only an
  optional external conditions can control what matches or not.

* On DB48X, by default, any name in the pattern acts as a wildcard, and we use
  `&` to refer to a specific variable i.e. `&A` only matches the variable named
  `A`.

Rationale: The default DB48X approach makes it easier to write transformation
rules for the common cases, the `&` character being a bit harder to access on
the calculator's keyboard. The assumption is that we rarely write patterns to
match a specific variable, i.e. replace an expression only if it refers to `X`
but not to `Y`. The HP behaviour can be restored using the `ExplicitWildcards`
flag.

A further extension in DB48X is to give a semantic meaning to the first letter
of variable names:

* `a`, `b` and `c` denote numerical constants, like real or integer values.
* `i`, `j` are positive integer values which may be zero and may be the result
  of a computation, i.e. `3-3` will match.
* `k`, `l`, `m` are non-zero positive integer values, which may be the result of
  a compuation, i.e. `3-2` will match.
* `n`, `o`, `p` are symbols or names like `'ABC'`.
* `u`, `v`, `w` are unique terms, i.e. terms that cannot be presented more than
  once in an expression.
* Names where the initial is lowercase must be sorted, so that `x+y` will match
  `A+B` but not `B+A`.

Another important difference is that on HP calculators, the number of rewrites
of subexpressions is limited to a single pass, irrespective of flag `-100`
(step-by-step CAS mode), whereas DB48X will repeat application by default (this
can be changed by setting `StepByStepAlgebraResults`, or alternatively, by
clearing flag `-100`).

This leads to the last important difference. On HP calculators, `↑Match` and
`↓Match` return either `0` or `1` in the first level of the stack to indicate if
a replacement occurred. On DB48X, the number of replaced subexpressions is
returned, and it can be greater than 1.


Examples:
* `'A+B+0' {'X+0' 'X' } ↓Match` returns `'A+B' 1`
* `'A+B+C' { 'X+Y' 'Y-X' } ↓Match` returns `'C-(B-A)' 2`
* `'(A+B)^3' { 'X^N' 'X*X^(N-1)' } ↓Match` returns `(A+B)*(A+B)^2`.


## Isolate

Isolate variable: Returns an expression that rearranges an expression given in
stack level 2 to "isolate" a variable specified in stack level 1.

For example, `A+1=sin(X+B)+C' 'X' ISOL` will produce `X=sin⁻¹(A-C+1)+2·i1·π-B`.

When the `PrincipalSolution` flag is not set, the resulting expression may
contain numbered variables such as `i1` as parameters. Variables that begin with
`i` represent arbitrary signed integers. Variables that begin with `n` represent
arbitrary natural numbers. Variables that begin with `s` represent arbitrary
signs.

The command will generate `Unable to isolate` if the expression cannot be
reorganized, for example because it contains functions that have no known
inverse.


## Quote

Return an object unevaluated (quoted).

`Obj` `Quote` → `QuotedObj`

The result is an algebraic expression containing `Obj` without evaluating it.
This is used to pass unevaluated names or formulas as arguments, for example
when building symbolic functions or when a CAS command must not evaluate its
parameters prematurely.

`5` `QUOTE` → `'5'`
`'A+B'` `QUOTE` → `'A+B'` (unchanged if already quoted)

`Quote` is mostly useful in algebraic expressions:

```rpl
@ Create an ArcLen commnand
@ Note that the HP50G ARM erroneously lacks ∫ at end of the program
@ We changed the names compared to ARM example because DB48x is case-insensitive,
@ and START..END or VAR would cause a syntax error with original example names.
« → arcstart arcend arcexpr arcvar
  « arcstart arcend arcexpr arcvar ∂ SQ 1 + SQRT arcvar ∫ »
» 'ArcLen' STO

@ Evaluate the result numerically in radians
'ArcLen(0,π,QUOTE(SIN(X)),QUOTE(X))'
RAD →Decimal DEG
@ Expecting 3.82019 77890 3
```

However, the rules for local name evaluations make the use of `Quote` less often necessary than on HP calculator. Consider the following example:

```rpl
@ Example program
« → A B C 'A+B*C' » 'MyFn' STO

@ Evaluation of MyFn with symbolic names
'MyFn(X;Y;Z)' EVAL

@ Evaluation of MyFn with expressions
'MyFn(X+1;Y-1;Z*2)' EVAL

@ Store a numerical value in X
42 'X' STO

@ Evaluation of MyFn without quotes will evaluate X
'MyFn(X+1;Y-1;Z*2)' EVAL

@ Evaluation of MyFn with quote will not evaluate X
'MyFn(Quote(X+1);Y-1;Z*2)' EVAL

4 →List
@ Expecting { 'X+Y·Z' 'X+1+(Y-1)·(Z·2)' '43+(Y-1)·(Z·2)' 'X+1+(Y-1)·(Z·2)' }
```

## Derivative

Compute the derivative function for an expression. The algebraic syntax for `∂` is `'∂name(expr)'` For example, `'∂x(sin(2*x^2)'` computes `4*X*cos(2*X^2)`

When differentiating a user-defined function named `F`, DB48X will generate `F′`
as the name for the derivative function. Note that this differ from HP
calculators, which would use `d1F`. If `F` has multiple parameters, then the
partial derivative relative to the first argument will be denoted as `F′₁`,
the partial derivative relative to the second argument will be denoted as
`F′₂` and so on.

For built-in functions that have no known derivative, such as `GAMMA`, DB48X
will generate an `Unknown derivative` error. Note that this differs from HP
calculators, which would generate a `d1GAMMA` function in that case.

The behaviour of the HP derivative function `∂` depends on whether it is in an
algebraic object (stepwise differentiation) or whether it is used in stack
syntax (full differentiation). The DB48X variant always perform full
differentiation irrespective of the syntax used.

The _HP50G advanced reference manual_ indicates that `∂` substitutes the value
of global variables. For example, if `Y` contains `X+3*X^2`, `'Y' 'X' ∂` is
expected to return `1+6*X`. It actually returns `0`, unless you evaluate `Y`
first. DB48X matches the actual behaviour of the HP50G and not the documented
one. To get the documented behaviour, evaluate the expression prior to computing
its derivative.


## AutoSimplify

Enable automatic reduction of numeric subexpressions according to usual
arithmetic rules. After evaluating `AutoSimplify` `'X+0'` will evaluate as `'X'`
and '`X*1-B*0'` witll evaluate as `'X'`.

The opposite setting is [NoAutoSimplify](#noautosimplify)

## NoAutoSimplify

Disable automatic reduction of numeric subexpressions according to usual
arithmetic rules. After evaluating `NoAutoSimplify`, equations such as`'X+0'`
or `X*1-B*0` will no longer be simplified during evaluation.

The opposite setting is [AutoSimplify](#autosimplify)


## SymbolicResults

Enable the generation of symbolic results, as opposed to `NumericalResults`.
For example, `2 3 /` gives exact result `2/3` as a result with
`SymbolicResults`, as opposed to a numerical approximation like `0.66667`.

## NumericalResults

Enable the generation of symbolic results, as opposed to `SymbolicResults`.  For
example, `2 3 /` gives the numerical approximation `0.66667` when this flag is
set, as opposed to exact result `2/3`.


## FinalAlgebraResults

Evaluate algebraic rules on symbolic expressions repeatedly until no futher change results from applying them.


## StepByStepAlgebraResults

Evaluate algebraic rules on symbolic expressions one step at a time.

## ListExpressionNames

List all variables used in an expression or polynomial, leaving the original
object on the stack, and returning the result as an array.
Names are sorted in decreasing order of size, and for the same size,
in alphabetical order.

`'A*Foo*G(X;Y;cos Z)`  ▶ `'A*Foo*G(X;Y;cos Z)` `[ Foo A G X Y Z ]`
`'(U_V)=(I_A)*(R_Ω)'`  ▶ `'(U_V)=(I_A)*(R_Ω)'` `[ I R U ]`

As a DB48X extension, this command also applies to programs, list and
arrays. Another extension is the `ExpressionVariables` command, which extracts
the units associated with the variables if there are any. The `LName` command
only returns the names, without the associated unit.


## ExpressionVariables

List all variables used in an expression or polynomial, returning the result as
a list.
Names are sorted in decreasing order of size, and for the same size,
in alphabetical order.
If there are units in the expression, the units are returned in the resulting
list.

`'A*Foo*G(X;Y;cos Z)`  ▶ `{ Foo A G X Y Z }`
`'(U_V)=(I_A)*(R_Ω)'`  ▶ `{ (I_A) (R_Ω) (U_V) }`

This is a variation of the HP command `LNAME`, which is a bit more convenient to
use in programs.

## RULEAPPLY1
Match and apply a rule to an expression only once


## TRIGSIN
Simplify replacing `cos(x)²` with `1-sin(x)²`

This command applies the Pythagorean identity to rewrite expressions so that
cosine squares are replaced by sine squares. This favors the use of `sin` over
`cos` in the expression.

`'cos(X)^2'` ▶ `'1-(sin X)²'`
`'cos(A+B)^2'` ▶ `'1-(sin(A+B))²'`


## ALLROOTS
Expand powers with rational exponents to consider all roots


## CLISTCLOSEBRACKET


## RANGE
Create a case-list of integers in the given range.


## ASSUME
Apply certain assumptions about a variable to an expression.

## AlgebraConfiguration

Name reserved for the current algebra configuration directory.

The `AlgebraConfiguration` command provides access to the directory that stores
the current Computer Algebra System (CAS) configuration.
This directory contains settings and variables used for symbolic computations.
If no configuration directory exists, one will be created when needed.
The configuration directory is stored in the global variable with the name
`AlgebraConfiguration`.

## AlgebraVariable

Recall the current algebra variable.

The `AlgebraVariable` command returns the current variable used for polynomial
evaluation and symbolic computations.
If no variable is set, it defaults to `X`.
The variable is stored in the algebra configuration directory.

## StoreAlgebraVariable

Store the current algebra variable.

The `StoreAlgebraVariable` command sets the variable used for polynomial evaluation and symbolic computations.
The variable must be a quoted symbol (e.g., `'X'`).
The variable is stored in the algebra configuration directory.

## Equation

Define an equation for use in solving or integration operations.
This is generally used as the name of a variable, and can manipulated using the `RcEQ` (recall equation) and `StEQ` (store equation) commands.


## StEq

Store expression in `Equation` variable.

The `StEq` command stores an expression, polynomial, or equation in the reserved
`Equation` variable.
The stored equation can be a single equation or a list of equations.
All stored equations must be of type expression, polynomial, or equation.
The stored equation is used by the solving menu and other equation-related
commands.

## RcEq

Recall expression from `Equation` variable.

The `RcEq` command recalls the currently stored equation from the `Equation`
variable.
If no equation is stored, it returns an error, `EQ variable not found`.
The recalled equation can be used for further manipulation or solving.

## NextEq

Cycle equations in the `Equation` variable if it is a list.

The `NextEq` command cycles through equations if the `Equation` variable
contains a list of equations.
It rotates the equations in the list, making the next equation the current one.
This is useful when working with multiple equations and wanting to solve them
one by one, see `MultipleEquationsSolver`.

## EvalEq

Evaluate the current equation.

The `EvalEq` command evaluates the currently stored equation.

For equations (expressions with `=`), it evaluates both sides and returns an
equation evaluating both sides, where the right-hand side may be a sum to adjust
it to the left-hand side:

```rpl
'A+1=B^2' STEQ
A=3 B=4 EVALEQ {} +
@ Expecting { '4=16-12' }
```

For other expressions, it evaluates the expression normally:

```rpl
'A+1-B^2' STEQ
A=3 B=4 EVALEQ {} +
@ Expecting { -12 }
```

## Where

Perform a substitution and evaluate the resulting expression.

The `Where` command performs symbolic substitution in expressions.
It takes an expression and a substitution rule (or list of rules) and applies
the substitution.
Substitutions can be expressed as equations or as a list .

In equation form, variables matching the left-hand side of the equation are
replaced by the expression on the rigth of the equation:

```rpl
'sin(Z)=2*(Z-1)^2' 'Z=3-A' Where {} +
@ Expecting { 'sin(3-A)=2·(3-A-1)↑2' }
```

Equations can be used in algebraic form and chained easily:

```rpl
'sin(Z)=2*(Z-A)^2|Z=3-A|A=B^3-1'
@ Expecting 'sin(3-(B↑3-1))=2·(3-(B↑3-1)-(B↑3-1))↑2'
```

In list form, variables given in odd positions in the list are replaced by the
expression given in the following even position in the list:

```rpl
'sin(Z)=2*(Z-1)^2' { Z '3-A' A 'B^3' } Where {} +
@ Expecting { 'sin(3-B↑3)=2·(3-B↑3-1)↑2' }
```
# Time, Date, Alarms and System Commands

## DateMenu

Show a softkey menu for date-related commands, including:

* `Date`
* `DateTime`
* `DateAdd`
* `DateSub`
* `SetDate`
* `JulianDayNumber`
* `DateFromJulianDayNumber`

## TimeMenu

Show a softkey menu for time-related commands:

* `Time`
* `ToHMS`
* `FromHMS`
* `HMSAdd`
* `HMSSub`
* `ChronoTime`
* `Ticks`
* `DateTime`
* `Wait`
* `TimedEval`
* `SetTime`


## Date format

The date format is `YYYYMMDD`, with an optional fractional part defining the
time, as in `YYYYMMDD.HHMMSS`.

Note: the date format is intentionally different from the format on the HP-48.

## Time format

The time format is `HH.MMSS` with optional hundredths of a second as
in `HH.MMSSCC`.


## SetDate

Set current system date.

The date format is `YYYYMMDD`, with an optional fractional part defining the
time, as in `YYYYMMDD.HHMMSS`. If the fractional part is zero, then the time is
not changed.

Note: the date format is intentionally different from the format on the HP-48.


## Date+

Add days to a [date](#date). The date format is `YYYYMMDD`, with an optional
fractional part defining the time, as in `YYYYMMDD.HHMMSS`, and an optional
unit, as in `YYYMMDD_date`. The unit enables special rendering as a date.

For example, to compute the date corresponding to 22222 days in the life of some
anonymous programmer, you can use the following code:

```rpl
19681205 22222 DATE+
@ Expecting Mon 8/Oct/2029
```

The opposite commands to compute the difference between dates is `DDays`.

The command accepts the inputs in any order. However, if both inputs can be
intepreted as dates, then the first one is a date and the second one a number of
days.


## SetTime

Set current time from a stack value `HH.MMSSCC`.

An HMS value can also be given, as returned by the `Time` command.


## →HMS

Convert decimal time to `HH.MMSS` (Hours, Minutes, Seconds) format.

For example, twelve hours and a half are converted to twelve hours and thirty
minutes as follows:

```rpl
12.5 →HMS
@ Expecting 12:30:00
```


## HMS→

Convert time in `HH.MMSS` (Hours, Minutes, Seconds) format to numerical time.

DB48x converts the time as a fraction to keep the result exact:

```rpl
12.30 HMS→
@ Expecting 12 ¹/₂
```


## HMS+

Add time in `HH.MMSS` format.

```rpl
10.30 2.40 HMS+
@ Expecting 13:10:00
```


## HMS-

Subtract time in `HH.MMSS` format


```rpl
10.30 2.40 HMS-
@ Expecting 7:50:00
```

## →DMS

Convert value to DMS format.

The `→DMS` command converts a decimal value to DMS (Degrees, Minutes, Seconds)
format.
The input is interpreted as decimal degrees and converted to the DMS format.
The result is returned as a unit object with the `_dms` unit.

```rpl
12.5 →DMS
@ Expecting 12°30′00″
```

## DMS→

Convert value from DMS format.

The `DMS→` command converts a DMS (Degrees, Minutes, Seconds) value to
decimal format.
The input can be a unit object with the `_dms` unit.
The result is returned as a decimal value representing degrees.

```rpl
12.30 DMS→
@ Expecting 12 ¹/₂
```

## Ticks

Return system clock in milliseconds


## TEVAL
Perform EVAL and measure elapsed time


## Date

Return the current system date as a unit object in the form `YYYYMMDD_date`.
This displays on the stack according to date format settings, in a way similar
to what is shown in the header, e.g. `23/Feb/2024` or `2024-02-23`.


## DateTime

Return the current system date as a unit object in the form `YYYYMMDD_date`.
This displays on the stack according to date format settings, in a way similar
to what is shown in the header, e.g. `23/Feb/2024` or `2024-02-23`.

## DDays

Number of days between dates.

To compute the number of days in 2024, use:
```rpl
20250101 20240101 DDAYS
@ Expecting 366 d
```

## Time

Return the current system time as a unit object in the form `HH.MMSS_hms`.
This displays on the stack as `HH:MM:SS`.


## JulianDayNumber

Return the Julian day number for the given date and time.

For dates the Gregorian calendar is assumed.
The Gregorian calendar jumps from 1582-10-04 to 1582-10-15.
This command ignores that gap, so the Julian day number given by this command for dates on or before 1582-10-14 may deviate from other converters like
[The NASA Julian Date/Time Converter](https://ssd.jpl.nasa.gov/tools/jdc).

To compute the Julian Day Number for the first day of the millenium:
```rpl
20000101 JDN
@ Expecting 2 451 545
```

## Datefromjuliandaynumber

Return the date for a given Julian day number.

This command converts a Julian day number to a date in the Gregorian calendar.
It is the opposite of the `JDN` command.

```rpl
2451545 JDN→
@ Expecting Sat 1/Jan/2000
```

## ACK
Acknowledge oldest alarm (dismiss)


## ACKALL
Acknowledge (dismiss) all alarms


## RCLALARM
Recall specified alarm


## STOALARM
Create a new alarm


## DELALARM
Delete an existing alarm


## FINDALARM
Get first alarm due after the given time


## Version

Return DB48X version information as text.

 ▶ `"Version information"`

## Chuck

The most powerful command in DB48x.

 ▶ `"Chuck pearl of wisdom"`

The `CHUCK` command is by far the most powerful command in DB48x.
Users must run it regularly to keep their calculator in shape.

* `CHUCK` does not have arguments, it wins every single one.
* `CHUCK` does not merely produce a result, it produces the best result
every single time, effortlessly.
* `CHUCK` does not need help, it provides help.
* `CHUCK` never fails, any error is a user error.
* `CHUCK` does not drain the battery, it exercises power.
* `CHUCK` is so fast it has to slow down for you to keep up.
* `CHUCK` can identify if your DB48x build is genuine.


## FreeMemory

Return the number of bytes immediately available in memory, without performing a
cleanup of temporary values (garbage collection).

See also: [GarbageCollect](#GarbageCollect), [FreeMemory](#FreeMemory)


## AvailableMemory

Return the number of bytes available in memory.

*Remark*: The number returned is only a rough indicator of usable memory.
In particular, [recovery features](#LastThingsMenu) consume or release varying
amounts of memory with each operation.

Before it can assess the amount of memory available, `AvailableMemory` removes
objects in temporary memory that are no longer being used. Like on the HP48, you
can therfore use `MEM` `DROP` to force garbage collection. However, there is
also a dedicated command for that, [GarbageCollect](#GarbageCollect).

See also: [FreeMemory](#FreeMemory), [GarbageCollect](#GarbageCollect)


## GarbageCollect

Perform a clean-up of temporary objects and return number of bytes reclaimed.

In order to speed up normal operations, temporaries are only discarded when
necessary to make room. This clean-up process, also called *garbage collection*,
occurs automatically when memory is full. Since garbage collection can slow down
calculator operation at undesired times, you can force it to occur at a desired
time by executing [GarbageCollect](#GarbageCollect).

See also: [FreeMemory](#FreeMemory), [Purge](#Purge)

## GarbageCollectorStatistics

Return an array containing garbage collector statistics, including:

* The number of garbage collection cycles
* The total number of bytes collected
* The total time spent collecting garbage
* The number of bytes collected during the last collection cycle
* The duration of the last collection cycle
* The number of bytes cleared by temporaries cleaning


## RuntimeStatistics

Return an array containing runtime statistics, including:

* The time spent running (i.e. the calculator is in high-power state)
* The time spent sleeping (i.e. the calculator is in low-power state)
* The number of times the calculator entered high-power state

Note that the calculator tends to spend more time in active state when on USB
power, because of additional animations or more expensive graphical rendering.


## Bytes

Return the size of the object and a hash of its value. On classic RPL systems,
the hash is a 5-nibbles CRC32. On DB48X, the hash is a based integer of the
current [wordsize](#stws) corresponding to the binary representation of the
object.

For example, the integer `7` hash will be in the form `#7xx`, where `7` is the
value of the integer, and `xx` represents the integer type, as returned by the
[Type](#type) command.

`X` ▶ `Hash` `Size`


## Type

Return the type of the object as a numerical value. The value is not guaranteed
to be portable across versions of DB48X (and pretty much is guarantteed to _not_
be portable at the current stage of development).

### HP-compatible types

If the `CompatibleTypes` setting is active, the returned value roughly matches
the value returned by the HP50G. It always returns `29` for arrays, not `3`
(real array) nor `4` (complex array). It returns `1` for both polar and
rectangular complex numbers, irrespective of their precision. 128-bit decimal
values return `21` (extended real), 32-bit and 64-bit return `0` (real number).
The separation between `18` (built-in function) and `19` (built-in command) may
not be accurate.

The values returned by `Type` in HP-compatible mode are as follows:

* 0: Real number
* 1: Complex number
* 2: Text (called *character string* in HP manuals)
* 3: Real array (DB48x returns 29)
* 4: Complex array (DB48x returns 29)
* 5: List
* 6: Global name
* 7: Local name
* 8: Program
* 9: Expression
* 10: Based integer (binary integer in HP manuals)
* 11: Graphic object
* 12: Tagged object
* 13: Unit object
* 14: Library reference (XLIB name)
* 15: Directory
* 16: Library (not on DB48x)
* 17: Backup object (not on DB48x)
* 18: Built-in function (DB48x returns 19)
* 19: Built-in command
* 20: System binary (not on DB48x)
* 21: Extended real (DB48x returns 0)
* 22: Extended complex (DB48x returns 1)
* 23: Linked array (not on DB48x)
* 24: Character (not on DB48x)
* 25: Code object (not on DB48x)
* 26: Library data (not on DB48x)
* 27: Mini font (dense font on DB48x)
* 28: Real integer
* 29: Symbolic vector/matrix (all arrays on DB48x)
* 30: Font (sparse fonts on DB48x)
* 31: Extended object (not on DB48x)


### DB48x detailed types

If the `DetailedTypes` setting is active, the return value is negative, and
matches the internal representation precisely. For example, distinct values will
be returned for fractions and expressions.

*Note* The [TypeName](#typename) command returns the type as text, and
this is less likely to change from one release to the next. DB48X-only code
should favor the use of `TypeName`, both for portability and readability.

The values returned by `Type` are negative, beginning at `-2`, in the order
shown in the table for `TypeName` below. Commands each have their individual
type number.  Returned values are not guaranteed from release to release, and
they may differ depending on build options or hardware platform (e.g. the values
on DM32 or DM42) may differ. The values for data types are not necessarily
contiguous either. For example, the `Type` for a tagged object may be `-1473` on
version 0.9.1, and may change over time (the reason being that rarely used types
have a two-byte type prefix).

Use `TypeName` for portability.

## TypeName

Return the [type](#Type) of the object as text. For example:

```rpl
12 typename`
@ Expecting "integer"
```

The values returned by `Type` in detailed mode are as follows, where the
spelling is what `TypeName` returns:

* directory
* text
* list
* program
* block (unquoted code block)
* locals (local variables structure)
* expression
* funcall (function call in expression, e.g. `F(1;2;3)`
* local (local variable name)
* symbol (global variable name)
* constant (reference to constant library, e.g. `ⒸNA`)
* equation (reference to equation library, e.g. `ⒺSimple Shear`)
* xlib (reference to library, e.g. `ⓁDedicace`)
* array
* menu
* unit (e.g. `1_km`)
* assignment (e.g. `X=3`)
* rectangular (complex numbers such as `2+3ⅈ`)
* polar (complex numbers such as `2∡30°`)
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


## sec

[Analytic function](#analytic-functions) returning the secant of the argument (1/cos).

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. At odd-integer multiples of 90° in the current angle
units, an [infinite result](#infinite-result) is generated.

`z` ▶ *sec* `z`

`symbexpr` ▶ `sec symbexpr`


## csc

[Analytic function](#analytic-functions) returning the cosecant of the argument (1/sin).

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. At integer multiples of 180° in the current angle
units, an [infinite result](#infinite-result) is generated.

`z` ▶ *csc* `z`

`symbexpr` ▶ `csc symbexpr`


## cot

[Analytic function](#analytic-functions) returning the cotangent of the argument (cos/sin).

For real arguments, the current angle mode determines the number's units, unless
angular units are specified. At integer multiples of 180° in the current angle
units, an [infinite result](#infinite-result) is generated.

`z` ▶ *cot* `z`

`symbexpr` ▶ `cot symbexpr`


## ASIN
Compute the arcsine


## ACOS
Compute the arccosine


## ATAN
Compute the arctangent


## sec⁻¹

Compute the arc-secant. The argument must satisfy |x| ≥ 1.


## csc⁻¹

Compute the arc-cosecant. The argument must satisfy |x| ≥ 1.


## cot⁻¹

Compute the arc-cotangent.


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


## Expm1
Compute exp(x)-1


## Ln1p
Compute ln(x+1)


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
# Soft Menus Tree


Soft-menu hierarchy of the DB48X calculator for dm42 keyboard

Buttons are shown as they appear on the calculator screen:
**6 columns** (F1–F6) and **3 rows** per page.

**Legend**

| Style | Meaning |
|:------|:--------|
| \[Menu\] | Opens a sub-menu (defined in this document) |
| [Command](#add) | Executes a command — link leads to the reference doc |
| Command | Command with no documentation entry found |
| _Unimplemented_ | Not yet implemented |
| `text` | Inserts literal text in the command line |
| 🟨 KEY | Press the yellow shift key once, then KEY |
| 🟦 KEY | Press the yellow shift key twice, then KEY |
| ▶ Fn | Navigate to next page, then press Fn |

---

## Contents

- [AlarmMenu](#alarmmenu)
- [AlgebraMenu](#algebramenu)
- [AnglesMenu](#anglesmenu)
- [ArithmeticMenu](#arithmeticmenu)
- [BasesMenu](#basesmenu)
- [CalculationMenu](#calculationmenu)
- [CharactersMenu](#charactersmenu-reference)
- [CircularMenu](#circularmenu)
- [ClearThingsMenu](#clearthingsmenu)
- [CompareMenu](#comparemenu)
- [ComplexMenu](#complexmenu)
- [ConstantsMenu](#constantsmenu-reference)
- [DateMenu](#datemenu-reference)
- [DebugMenu](#debugmenu-reference)
- [DifferentialSolverMenu](#differentialsolvermenu)
- [DisplayModesMenu](#displaymodesmenu-reference)
- [EditMenu](#editmenu)
- [EquationsMenu](#equationsmenu)
- [ExpLogIdentitiesMenu](#explogidentitiesmenu)
- [FilesMenu](#filesmenu)
- [FlagsMenu](#flagsmenu)
- [FractionsMenu](#fractionsmenu)
- [GraphicsMenu](#graphicsmenu)
- [HyperbolicMenu](#hyperbolicmenu)
- [IntegrationMenu](#integrationmenu)
- [IOMenu](#iomenu)
- [Library](#library-reference)
- [LinearSolverMenu](#linearsolvermenu)
- [ListMenu](#listmenu)
- [LoopsMenu](#loopsmenu)
- [MainMenu](#mainmenu-reference)
- [MathMenu](#mathmenu-reference)
- [MathModesMenu](#mathmodesmenu-reference)
- [MatrixMenu](#matrixmenu)
- [MemoryMenu](#memorymenu)
- [ModesMenu](#modesmenu-reference)
- [MultiSolverMenu](#multisolvermenu)
- [NumbersMenu](#numbersmenu)
- [NumericalSolverMenu](#numericalsolvermenu)
- [ObjectMenu](#objectmenu)
- [PartsMenu](#partsmenu)
- [PlotMenu](#plotmenu)
- [PolynomialsMenu](#polynomialsmenu)
- [PolynomialSolverMenu](#polynomialsolvermenu)
- [PowersMenu](#powersmenu)
- [PrintingMenu](#printingmenu)
- [ProbabilitiesMenu](#probabilitiesmenu)
- [ProgramMenu](#programmenu)
- [RangeMenu](#rangemenu)
- [RealMenu](#realmenu)
- [SeparatorModesMenu](#separatormodesmenu-reference)
- [SignalProcessingMenu](#signalprocessingmenu)
- [SolverMenu](#solvermenu)
- [StackMenu](#stackmenu)
- [StatisticsMenu](#statisticsmenu)
- [SymbolicMenu](#symbolicmenu)
- [SymbolicSolverMenu](#symbolicsolvermenu)
- [TestsMenu](#testsmenu)
- [TextMenu](#textmenu)
- [TimeMenu](#timemenu-reference)
- [TrigIdentitiesMenu](#trigidentitiesmenu)
- [UnitsConversionsMenu](#unitsconversionsmenu)
- [UnitsMenu](#unitsmenu)
- [UserInterfaceModesMenu](#userinterfacemodesmenu)
- [UserModeMenu](#usermodemenu-reference)
- [VectorMenu](#vectormenu)

---

### AlarmMenu

Access: [DateMenu](#datemenu-reference) 🟨 F6; [TimeMenu](#timemenu-reference) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| _AckAll_ | \[[Time](#timemenu-reference)\] | \[[Date](#datemenu-reference)\] |   |   |   |
| _Alarm_ | _Ack_ | _→Alarm_ | _Alarm→_ | _FindAlm_ | _DelAlm_ |

### AlgebraMenu

Access: 🟦 D (LOG); [SymbolicMenu](#symbolicmenu) F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Ⓓ](#algebraconfiguration) | [ⓧ](#algebravariable) | [Stoⓧ](#storealgebravariable) | [Final](#finalalgebraresults) | &Wild | \[[Symb](#symbolicmenu)\] |
| [∂](#derivative) | ∫ | ∑ | ∏ | _∆_ | [→Qπ](#q-1) |
| [↓Match](#match-1) | [↑Match](#match) | [Isolate](#isolate) | Apply | Subst | [\|](#where) |

### AnglesMenu

Access: 🟦 L (TAN); [ComplexMenu](#complexmenu) 🟦 F5; [MathMenu](#mathmenu-reference) 🟦 F3; [ModesMenu](#modesmenu-reference) 🟨 F2; [RealMenu](#realmenu) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→DMS](#dms) | [DMS→](#dms-1) | DMS+ | DMS- | [Hypot](#hypot) | [Atan2](#atan2) |
| →Deg | →Rad | →Grad | →πr | [→Polar](#topolar) | [→Rect](#torectangular) |
| Deg | Rad | Grad | [πr](#piradians) | [D→R](#dr) | [R→D](#rd) |

### ArithmeticMenu

Access: [SymbolicMenu](#symbolicmenu) 🟨 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [Show](#show) | [Quote](#quote) | [\|](#where) | `=` | _Rules_ | \[[Symb](#symbolicmenu)\] |
| [∂](#derivative) | ∫ | ∑ | ∏ | _∆_ | _Taylor_ |

### BasesMenu

Access: 🟨 T (4); [MathMenu](#mathmenu-reference) 🟦 F2; [ProgramMenu](#programmenu) 🟨 F5

*3 pages · 45 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [WordSize](#wordsize) | [NAnd](#nand) | [NOr](#nor) | [Implies](#implies) | [Excludes](#excludes) |   |
| [Base](#base) | Bin | Oct | Dec | Hex | ◀ |
| `#` | [And](#and) | [Or](#or) | [Xor](#xor) | [Not](#not) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [SLC](#shiftleftcount) | [SRC](#shiftrightcount) | [ASRC](#arithmeticshiftrightcount) | [RLC](#rotateleftcount) | [RRC](#rotaterightcount) |   |
| [SLB](#shiftleftbyte) | [SRB](#shiftrightbyte) | [ASRB](#arithmeticshiftrightbyte) | [RLB](#rotateleftbyte) | [RRB](#rotaterightbyte) | ◀ |
| [SL](#shiftleft) | [SR](#shiftright) | [ASR](#arithmeticshiftright) | [RL](#rotateleft) | [RR](#rotateright) | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| CntBits | 1-comp | 2-comp | [Modern](#modernbasednumbers) | [TruthLogicForIntegers](#truthlogicforintegers) |   |
| SetBit | ClrBit | FlipBit | FstSet | LstSet | ◀ |
| `#` | R→B | B→R | [Base](#base) | [WordSize](#wordsize) | ▶ |

### CalculationMenu

Access: [SymbolicMenu](#symbolicmenu) 🟨 F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| _Limit_ | _Serie_ | _Taylor_ | \[[Symb](#symbolicmenu)\] |   |   |
| [LName](#listexpressionnames) | [XVars](#expressionvariables) | [Deriv](#derivative) | _DerivX_ | _IBF_ | _IntVX_ |

### CircularMenu

Access: 🟦 K (COS); [MathMenu](#mathmenu-reference) F3; [RealMenu](#realmenu) 🟦 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [sec](#sec) | [csc](#csc) | [cot](#cot) | [sec⁻¹](#sec-1) | [csc⁻¹](#csc-1) | [cot⁻¹](#cot-1) |
| [sin](#sin) | [cos](#cos) | [tan](#tan) | [sin⁻¹](#asin) | [cos⁻¹](#acos) | [tan⁻¹](#atan) |

### ClearThingsMenu

Access: 🟨 (⌫)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Stack](#clearstack) | [Purge](#purge) | Stats | [Mem](#availablememory) | Error | [LCD](#clearlcd) |

### CompareMenu

Access: [ProgramMenu](#programmenu) 🟨 F3; [TestsMenu](#testsmenu) 🟦 F4

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [true](#true) | [false](#false) | \[[Tests](#testsmenu)\] | \[[Loops](#loopsmenu)\] | \[[Prog](#programmenu)\] |   |
| [and](#and) | [or](#or) | [xor](#xor) | [not](#not) | [==](#-2) | _Unimplemented_ |
| [<](#-3) | [=](#) | [>](#-5) | [≤](#-4) | [≠](#-1) | [≥](#-6) |

### ComplexMenu

Access: 🟨 G (STO); [MathMenu](#mathmenu-reference) F2; [VectorMenu](#vectormenu) 🟨 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| 2+i3 | [ℝ∡ℝ→ℂ](#realtopolar) | [ℂ→ℝ∡ℝ](#polartoreal) | Auto ℂ | \[[Angles](#anglesmenu)\] |   |
| [→Rect](#torectangular) | [→Polar](#topolar) | [conj](#conj) | [sign](#sign) | [\|z\|](#abs) | [arg](#arg) |
| `ⅈ` | `∡` | [ℝ→ℂ](#realtorectangular) | [ℂ→ℝ](#rectangulartoreal) | [re](#re) | [im](#im) |

### DateMenu Reference

See also: [DateMenu](#datemenu) user documentation

Access: [AlarmMenu](#alarmmenu) 🟨 F3; [TimeMenu](#timemenu-reference) 🟦 F4

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [→Time](#settime) | [→Date](#setdate) | [JDN](#juliandaynumber) | [JDN→](#datefromjuliandaynumber) | \[[Time](#timemenu-reference)\] | \[[Alarms](#alarmmenu)\] |
| `_date` | `_d` | [Date](#date-1) | [Dt+Tm](#datetime) | ∆Date | [Date+](#date) |

### DebugMenu Reference

See also: [DebugMenu](#debugmenu) user documentation

Access: 🟦 X (1); [ProgramMenu](#programmenu) 🟦 F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Run](#run) | [ErrDbg](#debugonerror) | \[[Prog](#programmenu)\] |   |   |   |
| [Halt](#halt) | [Step↑](#stepout) | DoErr | ErrMsg | ErrNum | ClrErr |
| [Debug](#debug) | [Step](#step) | [Over](#over) | [Steps](#multiplesteps) | [Continue](#continue) | [Kill](#kill) |

### DifferentialSolverMenu

Access: [SolverMenu](#solvermenu) 🟨 F4; [SymbolicMenu](#symbolicmenu) 🟦 F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Eq](#equation) | _Indep_ | [Root](#root) | \[[SolverMenu](#solvermenu)\] |   |   |

### DisplayModesMenu Reference

See also: [DisplayModesMenu](#displaymodesmenu) user documentation

Access: 🟨 O (EEX); [MathModesMenu](#mathmodesmenu-reference) 🟦 F6; [ModesMenu](#modesmenu-reference) 🟨 F5; [SeparatorModesMenu](#separatormodesmenu-reference) 🟨 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [1 1/2](#mixedfractions) | [3/2](#improperfractions) | [1/3](#bigfractions) | [¹/₃](#smallfractions) | \[[UI](#userinterfacemodesmenu)\] | \[[Math](#mathmodesmenu-reference)\] |
| [MantissaSpacing](#mantissaspacing) | [FractionSpacing](#fractionspacing) | [BasedSpacing](#basedspacing) | [StandardExponent](#standardexponent) | [MinimumSignificantDigits](#minimumsignificantdigits) | \[[Seps](#separatormodesmenu-reference)\] |
| Std | Fix | Sci | Eng | Sig | [Precision](#precision) |

### EditMenu

Access: 🟦 (▼)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Stack | Hist↑ | Hist↓ |   |   |   |
| Csr⇄Sel | \|← | →\| | Replace | [Copy](#copy) | Clear |
| Select | ←Word | Word→ | [Search](#editorsearch) | [Cut](#editorcut) | [Paste](#editorpaste) |

### ExpLogIdentitiesMenu

Access: [SymbolicMenu](#symbolicmenu) 🟨 F4

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| _ExpLn_ | _Lin_ | _LnCollect_ | _SinCos_ | _TExpand_ | \[[Symb](#symbolicmenu)\] |

### FilesMenu

Access: 🟦 (0)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| _Seek_ | _Dir_ |   |   |   |   |
| _Save_ | _Load_ | _Open_ | _Close_ | _Read_ | _Write_ |
| [Libs](#libs) | [Attach](#attach) | [Detach](#detach) | [Voltage](#batteryvoltage) | [USB?](#usbpowered) | [Low?](#lowbattery) |

### FlagsMenu

Access: 🟨 V (6); [ProgramMenu](#programmenu) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Prog](#programmenu)\] | \[[Loops](#loopsmenu)\] | \[[Modes](#modesmenu-reference)\] |   |   |   |
| [F→Bin](#flagstobinary) | [Bin→F](#binarytoflags) | \[[Tests](#testsmenu)\] | [Flip](#flipflag) | [Set?Set](#testflagsetthenset) | [Clr?Set](#testflagclearthenset) |
| [Set](#setflag) | [Clear](#clearflag) | [Set?](#testflagset) | [Clear?](#testflagclear) | [Set?Clr](#testflagsetthenclear) | [Clr?Clr](#testflagclearthenclear) |

### FractionsMenu

Access: 🟦 I (R↓); [MathMenu](#mathmenu-reference) 🟦 F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Frac→](#explode) | [1 1/2](#mixedfractions) | [¹/₃](#smallfractions) | FractionIterations | FractionLargestPrime | FractionDigits |
| [%Total](#percenttotal) | [%Chg](#percentchange) | DMS+ | DMS- | [→HMS](#hms) | [HMS→](#hms-1) |
| `/` | [%](#percent) | [→DMS](#dms) | [DMS→](#dms-1) | [→Num](#num) | →Frac |

### GraphicsMenu

Access: 🟦 (.)

*4 pages · 59 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| PixOn | PixOff | [Pix?](#pixtest) | [PixCol?](#pixcolor) | _Arc_ |   |
| [RGB](#rgb) | [Gray](#gray) | Foregnd | Bckgnd | LnWidth | ◀ |
| [Line](#drawline) | Rect | RndRect | Ellipse | Circle | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→LCD](#tolcd) | [LCD→](#fromlcd) | [Pict](#pict) | Clip | CurClip |   |
| Freeze | [Show](#show) | [→Grob](#togrob) | [MaximumShowWidth](#maximumshowwidth) | [MaximumShowHeight](#maximumshowheight) | ◀ |
| [ClLCD](#clearlcd) | [Disp](#drawtext) | [DispXY](#drawstyledtext) | [Input](#input) | [Prompt](#prompt) | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Σ](#graphicsum) | [∏](#graphicproduct) | [∫](#graphicintegral) | [Subscript](#graphicsubscript) | [Exponent](#graphicexponent) |   |
| [(.)](#graphicparentheses) | [\|.\|](#graphicnorm) | [÷](#graphicratio) | [√.](#graphicroot) | [Stack](#graphicstack) | ◀ |
| [GOr](#gor) | [GXor](#gxor) | [GAnd](#gand) | [Extract](#extract) | [Append](#graphicappend) | ▶ |

**Page 4**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [BlankPixmap](#blankpixmap) | _Unimplemented_ | [GraphicPicture](#pict) | \[[Plot](#plotmenu)\] |   |   |
| _Unimplemented_ | [→Grob](#togrob) | [Blank](#blankgraphic) | [BlBitmap](#blankbitmap) | [BlGrob](#blankgrob) | ◀ |
| [LCD→](#fromlcd) | [→LCD](#tolcd) | [→Bitmap](#tobitmap) | [→HPGrob](#tohpgrob) | [→Pixmap](#topixmap) | ▶ |

### HyperbolicMenu

Access: 🟦 J (SIN); [MathMenu](#mathmenu-reference) 🟨 F1; [PowersMenu](#powersmenu) 🟦 F6; [RealMenu](#realmenu) 🟦 F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| \[[Powers](#powersmenu)\] |   |   |   |   |   |
| [sinh](#sinh) | [cosh](#cosh) | [tanh](#tanh) | [sinh⁻¹](#asinh) | [cosh⁻¹](#acosh) | [tanh⁻¹](#atanh) |

### IntegrationMenu

Access: 🟨 Q (8); [SymbolicMenu](#symbolicmenu) 🟦 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| Σ | ∏ |   |   |   |   |
| [∂](#derivative) | ∫ | [Num ∫](#integrate) | Symb ∫ | [Eq](#equation) | _Indep_ |

### IOMenu

Access: 🟦 (-); [MainMenu](#mainmenu-reference) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [Save](#savestate) | [MinimumBatteryVoltage](#minimumbatteryvoltage) |   |   |   |   |
| [Save](#savestate) | _Load_ | _Print_ | [Voltage](#batteryvoltage) | [USB?](#usbpowered) | [Low?](#lowbattery) |

### LinearSolverMenu

Access: [SolverMenu](#solvermenu) 🟨 F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Eq](#equation) | _Indep_ | [Root](#root) | \[[SolverMenu](#solvermenu)\] |   |   |

### ListMenu

Access: 🟨 (-); [ProgramMenu](#programmenu) 🟦 F4

*2 pages · 30 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Sort](#sort) | [RSort](#reversesort) | [Map](#map) | [Reduce](#reduce) | [Filter](#filter) |   |
| [QSort](#quicksort) | [RQSort](#reversequicksort) | [∑List](#listsum) | [∏List](#listproduct) | [∆List](#listdifferences) | ◀ |
| [→List](#list) | [List→](#list-1) | [Size](#size) | [Head](#head) | [Tail](#tail) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Obj→](#explode) | _Find_ | \[[Objects](#objectmenu)\] | \[[Matrix](#matrixmenu)\] | \[[Vector](#vectormenu)\] |   |
| DoList | DoSubs | NSub | EndSub | [Extract](#extract) | ◀ |
| [Get](#get) | [Put](#put) | [GetI](#geti) | [PutI](#puti) | [Reverse](#reverselist) | ▶ |

### LoopsMenu

Access: 🟦 Y (2); [CompareMenu](#comparemenu) 🟦 F4; [FlagsMenu](#flagsmenu) 🟦 F2; [ProgramMenu](#programmenu) 🟨 F4; [TestsMenu](#testsmenu) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| \[[Compare](#testsmenu)\] | \[[Prog](#programmenu)\] | _Label_ | _Goto_ | _Gosub_ | _Return_ |
| [Start](#start) | StStep | [For](#for) | ForStep | [Until](#until) | [While](#while) |

### MainMenu Reference

See also: [MainMenu](#mainmenu) user documentation

Access: 🟦 A (Σ+)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Lib](#library-reference)\] | \[[Eqns](#equationsmenu)\] | \[[Const](#constantsmenu-reference)\] | \[[Time](#timemenu-reference)\] | \[[I/O](#iomenu)\] | \[[Chars](#charactersmenu-reference)\] |
| [Cat](#catalog) | \[[Real](#realmenu)\] | \[[Matrix](#matrixmenu)\] | \[[Symb](#symbolicmenu)\] | \[[Stack](#stackmenu)\] | \[[UI](#userinterfacemodesmenu)\] |
| Help | \[[Math](#mathmenu-reference)\] | \[[Prog](#programmenu)\] | \[[Plot](#plotmenu)\] | \[[Solve](#solvermenu)\] | \[[Modes](#modesmenu-reference)\] |

### MathMenu Reference

See also: [MathMenu](#mathmenu) user documentation

Access: E (LN); [MainMenu](#mainmenu-reference) F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Signal](#signalprocessingmenu)\] | \[[Bases](#basesmenu)\] | \[[Angles](#anglesmenu)\] | \[[Poly](#polynomialsmenu)\] | \[[Symb](#symbolicmenu)\] | \[[Frac](#fractionsmenu)\] |
| \[[Hyper](#hyperbolicmenu)\] | \[[Proba](#probabilitiesmenu)\] | \[[Stats](#statisticsmenu)\] | \[[Solver](#solvermenu)\] | \[[Const](#constantsmenu-reference)\] | \[[Eqns](#equationsmenu)\] |
| \[[Real](#realmenu)\] | \[[Cmplx](#complexmenu)\] | \[[Trig](#circularmenu)\] | \[[Powers](#powersmenu)\] | \[[Matrix](#matrixmenu)\] | \[[Ranges](#rangemenu)\] |

### MathModesMenu Reference

See also: [MathModesMenu](#mathmodesmenu) user documentation

Access: [DisplayModesMenu](#displaymodesmenu-reference) 🟦 F6; [ModesMenu](#modesmenu-reference) F4

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Lazy | Lossy | [LinFitΣ](#linearfitsums) | x·y | [Angles](#setangleunits) | \[[Disp](#displaymodesmenu-reference)\] |
| [MaxNumberBits](#maxnumberbits) | [MaxRewrites](#maxrewrites) | FractionIterations | FractionDigits | [1 1/2](#mixedfractions) | [¹/₃](#smallfractions) |
| [Sym](#symbolicresults) | [Simpl](#autosimplify) | 0^0=1 | HwFP | Auto ℂ | Princ |

### MatrixMenu

Access: 🟦 R (9); [ListMenu](#listmenu) ▶ 🟦 F4; [MainMenu](#mainmenu-reference) 🟨 F3; [MathMenu](#mathmenu-reference) F5; [VectorMenu](#vectormenu) 🟨 F6

*3 pages · 35 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [RowNrm](#rownorm) | [ColNrm](#columnnorm) | _CondNum_ | [Size](#size) | \[[Vector](#vectormenu)\] |   |
| [Det](#determinant) | [Norm](#norm) | [→Array](#array) | [Array→](#array-1) | [Random](#random) | ◀ |
| `[]` | [Idnty](#identitymatrix) | [Const](#constantarray) | Transp | TrConj | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| _SVD_ | _SVL_ | _Diag→_ | _→Diag_ | _SpecRad_ |   |
| _LU_ | _LQ_ | _QR_ | _Schur_ | _Cholesky_ | ◀ |
| [Col+](#col) | [Col-](#col-) | →Col | Col→ | ColSwp | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   | ◀ |
| [Row+](#row) | [Row-](#row-) | →Row | Row→ | RowSwp | ▶ |

### MemoryMenu

Access: 🟨 H (RCL); [ProgramMenu](#programmenu) 🟨 F1

*3 pages · 37 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Free](#freememory) | [TVars](#typedvariables) | [PgAll](#purgeall) | [RunStats](#runtimestatistics) | [GCStats](#garbagecollectorstatistics) |   |
| [Avail](#availablememory) | [Vars](#variables) | [Home](#homedirectory) | [Path](#directorypath) | [GC](#garbagecollect) | ◀ |
| [Store](#store) | [Recall](#recall) | [Purge](#purge) | [CrDir](#createdirectory) | [UpDir](#updirectory) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [▶](#copy) | Clone | [Incr](#increment) | [Decr](#decrement) | CurDir |   |
| [Recall](#recall) | Recall+ | Recall- | Recall× | Recall÷ | ◀ |
| [Store](#store) | Store+ | Store- | Store× | Store÷ | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| GC Clr | RT Clr |   |   |   | ◀ |
| [GCStats](#garbagecollectorstatistics) | [RunStats](#runtimestatistics) | [Avail](#availablememory) | System | [Bytes](#bytes) | ▶ |

### ModesMenu Reference

See also: [ModesMenu](#modesmenu) user documentation

Access: 🟨 N (+/-); [FlagsMenu](#flagsmenu) 🟦 F3; [MainMenu](#mainmenu-reference) F6; [SeparatorModesMenu](#separatormodesmenu-reference) 🟨 F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Modes](#modes) | [Reset](#resetmodes) | [System](#systemsetup) |   |   |   |
| Grad | \[[Angles](#anglesmenu)\] | Beep | Flash | \[[Display](#displaymodesmenu-reference)\] | \[[Seps](#separatormodesmenu-reference)\] |
| Deg | Rad | [n×π](#piradians) | \[[Math](#mathmodesmenu-reference)\] | \[[User](#usermodemenu-reference)\] | \[[UI](#userinterfacemodesmenu)\] |

### MultiSolverMenu

Access: [SolverMenu](#solvermenu) 🟦 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| _Eqs_ | _Indeps_ | [MRoot](#multipleequationssolver) | \[[SolverMenu](#solvermenu)\] |   |   |

### NumbersMenu

Access: 🟦 W (×)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| →Int | [IsPrime](#isprime) | [NextPr](#nextpr) | [PrevPr](#prevpr) | [GCD](#gcd) | [LCM](#lcm) |
| [→Num](#num) | [→Q](#q) | [→Qπ](#q-1) | R#Seed | [RandomGeneratorBits](#randomgeneratorbits) | [RandomGeneratorOrder](#randomgeneratororder) |
| Σ | ∏ | QuoRem | [Factors](#factors) | Ran# | [Random](#random) |

### NumericalSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Eq](#equation) | _Indep_ | [Root](#root) | \[[SolverMenu](#solvermenu)\] |   |   |

### ObjectMenu

Access: [ListMenu](#listmenu) ▶ 🟦 F3; [ProgramMenu](#programmenu) 🟦 F3

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Eval](#evaluate) | [Run](#run) | Clone | [DTag](#deletetag) | [Tag→](#tag-1) | \[[Tools](#toolsmenu)\] |
| [→List](#list) | [→Text](#totext) | [→Tag](#tag) | [→Graph](#togrob) | [→Prog](#program) | [→Array](#array) |
| [Bytes](#bytes) | [Type](#type) | [TypeName](#typename) | [Obj→](#explode) | [→Num](#num) | →Frac |

### PartsMenu

Access: 🟦 E (LN); [RealMenu](#realmenu) 🟦 F6

*3 pages · 45 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| R→I | →ℚ | [→Qπ](#q-1) | [→Decimal](#decimal) | [SignificantDigits](#sigdig) |   |
| Truncate | [Mantissa](#mant) | [Exponent](#xpon) | [ceil](#ceil) | [floor](#floor) | ◀ |
| [abs](#abs) | [sign](#sign) | [IntegerPart](#intpart) | [FractionalPart](#fracpart) | Round | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→2D](#to2dvector) | [→3D](#to3dvector) | [→Rectangular](#torectangular) | [→Cylindrical](#tocylindrical) | [→Spherical](#tospherical) |   |
| [Object→](#explode) | Range→ | [List→](#list-1) | Arry→ | [Vector→](#fromvector) | ◀ |
| [re](#re) | [im](#im) | [arg](#arg) | [Size](#size) | [Get](#get) | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Tag→](#tag-1) | [→Tag](#tag) | [DeleteTag](#deletetag) | Polynomial→ | →Polynomial |   |
| [StdRnd](#standardround) | [RelRnd](#relativeround) | [→StdUnc](#us) | [→RelUnc](#ur) | [PrcRnd](#precisionround) | ◀ |
| [CstName](#constantname) | [CstValue](#constantvalue) | CstRng | [StdUnc](#standarduncertainty) | [RelUnc](#relativeuncertainty) | ▶ |

### PlotMenu

Access: 🟦 O (EEX); [GraphicsMenu](#graphicsmenu) ▶×3 🟦 F4; [MainMenu](#mainmenu-reference) F4; [SolverMenu](#solvermenu) 🟦 F3; [StatisticsMenu](#statisticsmenu) 🟦 F5; [SymbolicMenu](#symbolicmenu) 🟨 F6

*2 pages · 20 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Lines | Axes | [StatsPlotBins](#statsplotbins) | [XYPlotBins](#xyplotbins) | PlotRefreshRate |   |
| Foregnd | Backgnd | LineWdth | [Xrange](#xrange) | [Yrange](#yrange) | ◀ |
| [Function](#functionplot) | [Polar](#polarplot) | [Param](#parametricplot) | [Scatter](#scatterplot) | [Truth](#truthplot) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   | ◀ |
| [Bar](#barplot) | [Histogrm](#histogramplot) | [Clear](#clearlcd) | Freeze | DrAxes | ▶ |

### PolynomialsMenu

Access: 🟦 Q (8); [MathMenu](#mathmenu-reference) 🟦 F4; [SymbolicMenu](#symbolicmenu) 🟨 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Solve](#root) | [TVMRoot](#tvmroot) | [XRoot](#xroot) | _Zeros_ | _FCoef_ |   |
| _FRoots_ | [MRoot](#multipleequationssolver) | \[[MSolvr](#solvermenu)\] | _PCoef_ | _PRoot_ | [Root](#root) |
| `Ⓟ''` | →Poly | Poly→ | [Obj→](#explode) | Display | QuoRem |

### PolynomialSolverMenu

Access: [SolverMenu](#solvermenu) 🟨 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Eq](#equation) | _Indep_ | [Root](#root) | \[[SolverMenu](#solvermenu)\] |   |   |

### PowersMenu

Access: 🟨 E (LN); [HyperbolicMenu](#hyperbolicmenu) 🟨 F1; [MathMenu](#mathmenu-reference) F4; [RealMenu](#realmenu) 🟦 F3

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [pow](#pow) | [xroot](#xroot) | FstSet | LstSet | popcnt | \[[Hyper](#hyperbolicmenu)\] |
| [exp2](#exp2) | log2 | [expm1](#expm1) | [ln1p](#ln1p) | x³ | [∛](#cbrt) |
| [exp](#exp) | [ln](#ln) | [exp10](#exp10) | [log10](#log10) | [x²](#sq) | [√](#sqrt) |

### PrintingMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| _Print_ | [Screen](#screencapture) | _Disk_ | _IR_ |   |   |

### ProbabilitiesMenu

Access: 🟨 W (×); [MathMenu](#mathmenu-reference) 🟨 F2; [RealMenu](#realmenu) 🟦 F4; [StatisticsMenu](#statisticsmenu) 🟦 F4

*2 pages · 22 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| _Normal_ | _Student_ | _Chi²_ | _F-Distr_ | _FFT_ |   |
| Γ | ln(Γ) | erf | erfc | RSeed | ◀ |
| Comb | Perm | x! | Ran# | [Random](#random) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [RandomGeneratorBits](#randomgeneratorbits) | [RandomGeneratorOrder](#randomgeneratororder) |   |   |   | ◀ |
| _Normal⁻¹_ | _Studnt⁻¹_ | _Chi²⁻¹_ | _F-Dist⁻¹_ | _FFT⁻¹_ | ▶ |

### ProgramMenu

Access: 🟨 Z (3); [CompareMenu](#comparemenu) 🟦 F5; [DebugMenu](#debugmenu-reference) 🟦 F3; [FlagsMenu](#flagsmenu) 🟦 F1; [LoopsMenu](#loopsmenu) 🟨 F2; [MainMenu](#mainmenu-reference) F3; [TestsMenu](#testsmenu) 🟦 F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Stack](#stackmenu)\] | \[[Debug](#debugmenu-reference)\] | \[[Objects](#objectmenu)\] | \[[List](#listmenu)\] | \[[Flag](#flagsmenu)\] | [Version](#version) |
| \[[Mem](#memorymenu)\] | \[[Test](#testsmenu)\] | \[[Cmp](#comparemenu)\] | \[[Loop](#loopsmenu)\] | \[[Base](#basesmenu)\] | [Eval](#evaluate) |
| `«»` | `{}` | `[]` | `→  «»` | `→  ''` | [Run](#run) |

### RangeMenu

Access: [MathMenu](#mathmenu-reference) F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Size](#size) |   |   |   |   |   |
| [→Range](#range) | [→∆Range](#range-1) | [→%Range](#range-2) | [→σRange](#range-3) | [∪](#rangeunion) | [∩](#rangeintersect) |
| `…` | `±` | `±%` | `±σ` | Range→ | `ρ` |

### RealMenu

Access: 🟦 G (STO); [MainMenu](#mainmenu-reference) 🟨 F2; [MathMenu](#mathmenu-reference) F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Trig](#circularmenu)\] | \[[Hyper](#hyperbolicmenu)\] | \[[Powers](#powersmenu)\] | \[[Prob](#probabilitiesmenu)\] | \[[Angles](#anglesmenu)\] | \[[Parts](#partsmenu)\] |
| [Ceil](#ceil) | [Floor](#floor) | rem | [%](#percent) | [%Chg](#percentchange) | [%Total](#percenttotal) |
| [Min](#min) | [Max](#max) | [mod](#mod) | [abs](#abs) | [→Num](#num) | [→Q](#q) |

### SeparatorModesMenu Reference

See also: [SeparatorModesMenu](#separatormodesmenu) user documentation

Access: [DisplayModesMenu](#displaymodesmenu-reference) 🟨 F6; [ModesMenu](#modesmenu-reference) 🟨 F6

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [1.2x10³²](#fancyexponent) | [1.2E32](#classicexponent) | [1.0→1.](#trailingdecimal) | [1→1.0](#showasdecimal) | Fixed0 |   |
| #1 000 | #1.000 | #1'000 | #1_000 | \[[Disp](#displaymodesmenu-reference)\] | \[[Modes](#modesmenu-reference)\] |
| [1 000](#numberspaces) | [1.000,](#numberdotorcomma) | [1'000](#numberticks) | [1_000](#numberunderscore) | [2.3](#decimaldot) | [2,3](#decimalcomma) |

### SignalProcessingMenu

Access: [MathMenu](#mathmenu-reference) 🟦 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| _FFT_ | _InvFFT_ |   |   |   |   |

### SolverMenu

Access: 🟨 P (7); [DifferentialSolverMenu](#differentialsolvermenu) F4; [LinearSolverMenu](#linearsolvermenu) F4; [MainMenu](#mainmenu-reference) F5; [MathMenu](#mathmenu-reference) 🟨 F4; [MultiSolverMenu](#multisolvermenu) F4; [NumericalSolverMenu](#numericalsolvermenu) F4; [PolynomialsMenu](#polynomialsmenu) 🟨 F3; [PolynomialSolverMenu](#polynomialsolvermenu) F4; [SymbolicSolverMenu](#symbolicsolvermenu) F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Multi](#multisolvermenu)\] | \[[Finance](#financesolvermenu)\] | \[[Plot](#plotmenu)\] | \[[Eqns](#equationsmenu)\] | [SolverImprecision](#solverimprecision) | [SolverIterations](#solveriterations) |
| [▶Eq](#steq) | [Stoⓧ](#storealgebravariable) | \[[Symb](#symbolicsolvermenu)\] | \[[Diff](#differentialsolvermenu)\] | \[[Poly](#polynomialsolvermenu)\] | \[[Linear](#linearsolvermenu)\] |
| [Eq▶](#rceq) | [ⓧ](#algebravariable) | [Root](#root) | [EvalEq](#evaleq) | [NxtEq](#nexteq) | \[[Solve](#solvingmenu)\] |

### StackMenu

Access: I (R↓); [MainMenu](#mainmenu-reference) 🟨 F5; [ProgramMenu](#programmenu) 🟦 F1

*2 pages · 22 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [DupN](#duplicaten) | [DropN](#dropn) | [Depth](#depth) | [Nip](#nip) | [Pick3](#pick3) |   |
| [Dup2](#duplicate2) | [Drop2](#drop2) | [Rot↓](#unrot) | [Roll↓](#rolld) | [Pick](#pick) | ◀ |
| [Dup](#duplicate) | [Drop](#drop) | [Rot↑](#rot) | [Roll↑](#roll) | [Over](#over) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [NDupN](#ndupn) | [DupDup](#duplicatetwice) |   |   |   | ◀ |
| [Swap](#swap) | [LastArg](#lastarguments) | [LastX](#lastx) | [Undo](#undo) | [ClrStk](#clearstack) | ▶ |

### StatisticsMenu

Access: 🟨 S (÷); [MathMenu](#mathmenu-reference) 🟨 F3

*3 pages · 35 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| StoΣ | RclΣ | ClrΣ | \[[Proba](#probabilitiesmenu)\] | \[[Plot](#plotmenu)\] |   |
| [XCol](#independentcolumn) | [YCol](#dependentcolumn) | [MinΣ](#min) | [MaxΣ](#max) | [ΣSize](#size) | ◀ |
| [Σ+](#) | [Σ-](#-) | Total | [Mean](#average) | [StdDev](#standarddeviation) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [ΣX](#x) | [ΣY](#y) | [ΣXY](#xy) | [ΣX²](#x-1) | [ΣY²](#y-1) |   |
| [BestFit](#bestfit) | [LinFit](#linearfit) | [ExpFit](#exponentialfit) | [LogFit](#logarithmicfit) | [PwrFit](#powerfit) | ◀ |
| [LR](#linearregression) | ΣLine | PredX | PredY | [Corr](#correlation) | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   | ◀ |
| [Median](#median) | [Bins](#frequencybins) | [PopVar](#populationvariance) | [PopSDev](#populationstandarddeviation) | [PCovar](#populationcovariance) | ▶ |

### SymbolicMenu

Access: 🟦 P (7); [AlgebraMenu](#algebramenu) 🟦 F6; [ArithmeticMenu](#arithmeticmenu) 🟨 F6; [CalculationMenu](#calculationmenu) 🟨 F4; [ExpLogIdentitiesMenu](#explogidentitiesmenu) F6; [MainMenu](#mainmenu-reference) 🟨 F4; [MathMenu](#mathmenu-reference) 🟦 F5; [TrigIdentitiesMenu](#trigidentitiesmenu) 🟨 F1

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| \[[Integ](#integrationmenu)\] | \[[DSolve](#differentialsolvermenu)\] | [Simplify](#autosimplify) | [KeepAll](#noautosimplify) |   |   |
| \[[Arith](#arithmeticmenu)\] | \[[Calc](#calculationmenu)\] | \[[Trig](#trigidentitiesmenu)\] | \[[Exp/Ln](#explogidentitiesmenu)\] | \[[Poly](#polynomialsmenu)\] | \[[Graph](#plotmenu)\] |
| Collect | Expand | Simplify | →Poly | [→Prog](#program) | \[[Algbra](#algebramenu)\] |

### SymbolicSolverMenu

Access: [SolverMenu](#solvermenu) 🟨 F3

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   |   |
| [Eq](#equation) | _Indep_ | [Root](#root) | [Isolate](#isolate) | \[[SolverMenu](#solvermenu)\] |   |

### TestsMenu

Access: 🟦 Z (3); [CompareMenu](#comparemenu) 🟦 F3; [FlagsMenu](#flagsmenu) 🟨 F3; [LoopsMenu](#loopsmenu) 🟨 F1; [ProgramMenu](#programmenu) 🟨 F2

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Case](#case) | Then | When | \[[Compare](#comparemenu)\] | \[[Loops](#loopsmenu)\] | \[[Prog](#programmenu)\] |
| IfThen | IfElse | [IfErr](#iferr) | IfErrElse | [IFT](#ift) | [IFTE](#ifte) |
| [<](#-3) | [=](#) | [>](#-5) | [≤](#-4) | [≠](#-1) | [≥](#-6) |

### TextMenu

Access: 🟦 T (4)

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [T→Real](#compiletoreal) | [T→Expr](#compiletoexpression) | [T→Int](#compiletointeger) | [T→Pos](#compiletopositive) |   |   |
| [T→Code](#textcode) | [Code→T](#codetext) | [Extract](#extract) | [T→Obj](#compiletoobject) | [T→Alg](#compiletoalgebraic) | [T→Num](#compiletonumber) |
| [→Text](#totext) | [Text→](#compile) | [Length](#size) | [Append](#add) | [Repeat](#repeat) | [C→Code](#charcode) |

### TimeMenu Reference

See also: [TimeMenu](#timemenu) user documentation

Access: 🟦 V (6); [AlarmMenu](#alarmmenu) 🟨 F2; [DateMenu](#datemenu-reference) 🟨 F5; [MainMenu](#mainmenu-reference) 🟦 F4

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→Time](#settime) | [→Date](#setdate) | _ClkAdj_ | \[[Dates](#datemenu-reference)\] | \[[Alarms](#alarmmenu)\] |   |
| Chrono | [Ticks](#ticks) | [Dt+Tm](#datetime) | [T→Str](#totext) | [Wait](#wait) | [TEval](#teval) |
| _hms | [Time](#time) | [→HMS](#hms) | [HMS→](#hms-1) | [HMS+](#hms-2) | [HMS-](#hms-) |

### TrigIdentitiesMenu

Access: [SymbolicMenu](#symbolicmenu) 🟨 F3

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| \[[Symb](#symbolicmenu)\] |   |   |   |   |   |
| _HalfTan_ | _Tan→SinCos_ | _Tan→SinCos²_ | _TExpand_ | _TLin_ | [TrigSin](#trigsin) |

### UnitsConversionsMenu

Access: 🟦 U (5)

*3 pages · 36 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| µ (-6) | n (-9) | p (-12) | T (+12) | P (+15) |   |
| m (-3) | c (-2) | k (+3) | M (+6) | G (+9) | ◀ |
| [Convert](#convert) | [Base](#base) | [Value](#unitvalue) | [Factor](#factorunit) | [→Unit](#unit) | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Ki | Mi | Gi | Ti | [Pi](#pi) |   |
| y (-24) | z (-21) | a (-18) | Z (+21) | Y (+24) | ◀ |
| f (-15) | d (-1) | da (+1) | h (+2) | E (+18) | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [SIPfx](#unitssiprefixcycle) |   |   |   |   | ◀ |
| Ei | Zi | Yi | Ri | Qi | ▶ |

### UserInterfaceModesMenu

Access: 🟦 N (+/-); [DisplayModesMenu](#displaymodesmenu-reference) 🟦 F5; [MainMenu](#mainmenu-reference) 🟨 F6; [ModesMenu](#modesmenu-reference) F6

*4 pages · 47 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [3-lines](#threerowsmenus) | [1-line](#singlerowmenus) | [Flat](#flatmenus) | [Round](#roundedmenus) | \[[Hide](#hideemptymenu)\] |   |
| ResultFont | StackFont | EditorFont | MultilineEditorFont | [CursorBlinkRate](#cursorblinkrate) | ◀ |
| [GrRes](#graphicresultdisplay) | [GrStk](#graphicstackdisplay) | Beep | Flash | \[[User](#usermodemenu-reference)\] | ▶ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Fixed0 | VProg | BusyIndicatorRefresh | \[[ExitMenu](#exitclearsmenu)\] | [ListEval](#listasprogram) |   |
| [EditorWrapColumn](#editorwrapcolumn) | [TabWidth](#tabwidth) | [MaximumShowWidth](#maximumshowwidth) | [MaximumShowHeight](#maximumshowheight) | ErrorBeepFrequency | ◀ |
| [cmd](#lowercase) | [CMD](#uppercase) | [Cmd](#capitalized) | [Command](#longform) | ErrorBeepDuration | ▶ |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [TextRenderingSizeLimit](#textrenderingsizelimit) | [GraphRenderingSizeLimit](#graphrenderingsizelimit) | PlotRefreshRate | AllVars | [SIPrefixCycle](#unitssiprefixcycle) |   |
| [ResultGraphingTimeLimit](#resultgraphingtimelimit) | [StackGraphingTimeLimit](#stackgraphingtimelimit) | [GraphingTimeLimit](#graphingtimelimit) | [ShowTimeLimit](#showtimelimit) | [MinimumBatteryVoltage](#minimumbatteryvoltage) | ◀ |
| [Units](#showbuiltinunits) | Const | Eqns | [Libs](#libs) | Chars | ▶ |

**Page 4**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
|   |   |   |   |   | ◀ |
| [Header](#header) | [CustomHeaderRefresh](#customheaderrefresh) |   |   |   | ▶ |

### UserModeMenu Reference

See also: [UserModeMenu](#usermodemenu) user documentation

Access: [ModesMenu](#modesmenu-reference) F5; [UserInterfaceModesMenu](#userinterfacemodesmenu) F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
|   |   |   |   |   |   |
| [DelKeys](#deletekeys) | [KeyMap](#keymap) |   |   |   |   |
| [Toggle](#toggleusermode) | [User](#usermode) | [Lock](#usermodelock) | [RclKeys](#recallkeys) | [StoKeys](#storekeys) | [Assign](#assignkey) |

### VectorMenu

Access: [ListMenu](#listmenu) ▶ 🟦 F5; [MatrixMenu](#matrixmenu) 🟦 F5

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Hypot](#hypot) | [Atan2](#atan2) |   |   |   |   |
| [→Vec2](#to2dvector) | [→Vec3](#to3dvector) | [Vec→](#fromvector) | [→Cylind](#tocylindrical) | \[[Complex](#complexmenu)\] | \[[Matrix](#matrixmenu)\] |
| [Norm](#norm) | [Dot](#dot) | [Cross](#cross) | [→Rect](#torectangular) | [→Polar](#topolar) | [→Spher](#tospherical) |

### CharactersMenu Reference

See also: [CharactersMenu](#charactersmenu) user documentation

_Content loaded dynamically at runtime._

### ConstantsMenu Reference

See also: [ConstantsMenu](#constantsmenu) user documentation

_Content loaded dynamically at runtime._

### EquationsMenu

_Content loaded dynamically at runtime._

### Library Reference

See also: [Library](#library) user documentation

_Content loaded dynamically at runtime._

### UnitsMenu

_Content loaded dynamically at runtime._

