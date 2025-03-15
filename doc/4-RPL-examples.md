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
@ Expecting 'Z÷(X·Y+Z+T+U)-Z÷U'
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
@ Expecting '(a+X)÷(X·Y+(a-b)·c)+U÷(Z·T+U+V+W)-a÷c-U÷W'
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

The `Type` command found in the `ObjectsMenu` (🟦 _-_, _Type_) takes any object
as an argument and returns a number that identifies that object type.

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
