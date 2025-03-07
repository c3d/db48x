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

* *Local variable structure*. The `→` command defines local variable names and a
  corresponding algebraic or program object that’s evaluated using those
  variables.
* *Branching structures*. Structure words (like `DO`…`UNTIL`…`END`) define
  conditional or loop structures to control the order of execution within a
  program.

A local variable structure has `→` followed by local names, followed by either a
program or an algebraic. This removes values from the stack, puts them in the
local variables, and then evaluates the algebraic or program.

For example, the following program takes two numbers from the stack and returns
a numeric result:

```rpl
« → a b 'ABS(a-b)' »
```


### Calculations in a program

Many calculations in programs take data from the stack. Two typical ways to
manipulate stack data are:

* *Stack commands* that operate directly on the objects on the stack.
* *Local variable structures* that store the stack objects in temporary local
  variables, then use the variable names to represent the data in the following
  algebraic or program object.

Numeric calculations provide convenient examples of these methods. The following
programs use two numbers from the stack to calculate the hypotenuse of a right
triangle using the square root of the sum of the squares.


The first program uses stack commands to manipulate the numbers on the stack,
and the calculation uses stack syntax.

```rpl
« SQ SWAP SQ + √ »
```

The second program uses a local variable structure to store and retrieve the
numbers, while the calculation uses stack syntax.

```rpl
« → x y « x SQ y SQ + √ » »
```

The third program also uses a local variable structure, but this time the
calculation uses algebraic syntax.

```rpl
« → x y '√(x^2+y^2)' »
```

Note that the underlying formula is most apparent in the third program. This
third method is often the easiest to write, read, and debug.


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


## Volume of a cylinder

The following programs take the values of the radius `r` and the height `h` of a
cylinder to compute the total area of the corresponding cylinder according to
the equation `ACyl=2·π·R↑2+2·π·R·H`. We use the symbolic constant `Ⓒπ`,
which we convert to its numerical value using the `→Num` function.

The following code stores the program in the `ACyl` variable, and then supplies
the value for `R` and `H` on level 1 and 2 of the stack respectively. In the
examples, we will use `R=2_m` and `H=3_m`.

### RPN style

The following code computes the cylinder area using _stack RPN instructions_,
i.e. manipulating values on the stack directly. This approach is the most
similar to traditional HP calculators.

```rpl
« Duplicate Rot + * 2 * Ⓒπ →Num * »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

### Using global variables

The following implementation computes the cylinder area using _RPN instructions_
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

### Using algebraic expressions

The following example computes the cylinder area using an _algebraic expression_
and global variables. Using algebraic expressions can make programs easier to
read, since the operations look similar to normal mathematical expressions.

```rpl
« 'R' Store 'H' Store
'2*Ⓒπ*R*(R+H)' →Num 'A' Copy »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```

### Using local variables

The following example computes the cylinder area using _local variables_, which
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

### Local algebraics

The following example computes the cylinder area using _local variables_, along
with an _algebraic expression_.

```rpl
« → H R '2*→Num(Ⓒπ)*R*(R+H)' »
'ACyl' Store

3_m 2_m ACyl
@ Expecting 62.83185 30718 m↑2
```
