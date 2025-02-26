# RPL Programming examples

The programs in this section demonstrate basic programming concepts in
[RPL](#introduction-to-rpl). They are intended to develop and improve your
programming skills, and to provide supplementary functions for your
calculator. The DB48X calculator features a library of introductory programs
covering mathematics, physics and computer science, which is accessible using
the `Library` command, 🟦 _H_ (_LIB_).

## What defines a RPL program?

A RPL program is a regular [RPL object](#programs) describing a procedure
consisting in a space-separated sequence of RPL objects such as numbers,
algebraic and RPN instructions. The whole sequence is enclosed between the `«`
and `»` delimiters.

To enter a program, use the 🟨 _=_ (`«PROG»`) key, which puts `« »` in the text
editor, with the cursor in the middle. One enters the sequence of instructions
defining the procedure at the position indicated by the cursor. The _Enter_ key
then enters the sequence as an object on the stack. If there is an error in the
program, it will be reported, and the cursor will be positioned next to it.

A program can be executed by evaluating it, typically using the _=_ key, which
is bound to the `Run` command. The `Run` and `Eval` commands also execute
programs. Programs can also be stored in variables, like any RPL
object. Evaluating the name of the variable evaluates the program.


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
