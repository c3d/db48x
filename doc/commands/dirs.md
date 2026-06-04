# Variables

Variables are named storage for RPL values.

## Store

Store an object into a specified location. For example `2 'ABC' STO` stores the value `2` in a global variable named `ABC`.

`Value` `Name` ▶

The `Value` is copied in a storage location identified by `Name`. The storage location depends on the type of `Name`, which can be quoted in an expression:

* Symbol or integer: The value is stored in a global variable with that name in the current directory, which is created if necessary. Whether integers can be used as variable names depends on the `NumberedVariables` setting.

* List or array: The name is a path through nested directories. Each element except the last must name an existing directory; the last element is the variable name in that directory. For example, `42 { SubDirTest FortyTwo } STO` stores `42` in variable `FortyTwo` inside directory `SubDirTest`. A path may start with [HomeDirectory](#HomeDirectory) to refer to the home directory.

* Directory variable: If the target name already holds a directory, the new value must also be a directory (replacing the old tree). Storing a non-directory value over a directory name reports `No directory`.

* Local name: The value is stored in the corresponding local variable.

* Setting name: The value is used for the corresponding setting, as if the setting command had been executed. For example, `16 'Base' STO` has the same effect as `16 Base`.

* Text: `Value` is stored in a named file on the [flash storage](#flash-storage).


## Recall
Recall an object from a specified location. For example `'ABC' RCL` recalls the value from a global variable named `ABC`.

`Name` ▶ `Value`

The `Value` is fetched from a storage location identified by `Name`. The storage location depends on the type of `Name`, which can be quoted in an expression:

* Symbol or integer: The value is fetched from a global variable with that name in the current directory or any enclosing directory. Whether integers can be used as variable names depends on the `NumberedVariables` setting.

* List or array: The name is a path through nested directories, as for [Store](#Store). Each element except the last must name an existing directory; the last element is the variable to recall. If any path component is missing, recall fails with `Undefined name`.

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

`Name` can be a symbol, a quoted name, or a list (or array) of names to purge
several variables in one command.

If `Name` refers to a directory variable, `Purge` only succeeds when that
directory is empty (no variables inside). Purging a non-empty directory reports
`Non-empty directory`. Use [PurgeDirectory](#PurgeDirectory) to remove a directory
and all of its contents.

*Remark*: `Purge` only removes a variable from the current directory, not the
enclosing directories. Since [Recall](#Recall) will fetch variable values from
enclosing directories, it is possible that `'X' Purge 'X' Recall` will fetch a
value for `X` from an enclosing directory. Use [PurgeAll](#PurgeAll) if you want
to purge a variable including in enclosing directories. The same non-empty
directory restriction applies to `PurgeAll`.

## PurgeAll

Delete a global variable from the current directory and enclosing directories.

*Remark*: If a variable with the same name exists in multiple enclosing
directories, `PurgeAll` may purge multiple variables. Use [Purge](#Purge) if you
want to only purge a variable in the current directory.


## CreateDirectory

Create one or more empty directories in the current directory.

`Name` may be a single directory name or a list (or array) of names. For
example, `{ Tools Examples } CreateDirectory` creates both `Tools` and
`Examples`. An error is reported if any of the names already exists.


## PurgeDirectory

Delete a global variable from the current directory, including directory contents.

`PurgeDirectory` behaves like [Purge](#Purge) for ordinary variables. For a
directory variable, it removes the directory and everything inside it, even when
the directory is not empty. `Name` may be a list (or array) of names, as with
`Purge`.


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

Note that this example requires `ExplicitConstants` since both `α` and `γ` are
the names of built-ibn constants.

```rpl
ExplicitConstants
```

Once this setting is done, you can use `α`, `β` and `γ` without risking a clash
with constant names:

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

You can then restore `AutomaticConstants`.

```rpl
AutomaticConstants
```


### PushEvaluatedAssignment

When evaluating `A='2+3*5'`, pushes `A=17` on the stack.

### PushOriginalAssignment

When evaluating `A='2+3*5'`, pushes `A='2+3*5'` on the stack.


## ORDER

Reorder variables in the current directory.

`{ Names... }` ▶

`Names` is a list (or array) of variable names in the desired order. Each name
must exist in the current directory. Names may be quoted. Variables listed first
appear first in the [Variables](#Variables) display and variable menu; any
variables in the directory that are not named in the list keep their relative
order and are placed after the listed ones.

For example, if the directory contains `C`, `B`, and `A` (in that order),
`{ 'A' 'B' 'C' } ORDER` reorders them to `A`, `B`, `C`.


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
