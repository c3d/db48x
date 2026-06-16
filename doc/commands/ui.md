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

## KeyEval

Simulate a key press from within a program.

`KeyEval` takes a [key position](#Key-positions) from the stack and processes
that key exactly as if it had been typed on the keyboard, including the shift,
alpha, and lowercase modifiers encoded in the key position. The key position is
popped from the stack.

Use `KeyEval` in programs to trigger keyboard actions programmatically, for
example to invoke a command bound to a key, insert text from a user key
assignment, or open a menu.

When `UserMode` is active, user key assignments stored in `KeyMap` are
honored. Otherwise, the standard keyboard bindings apply.

The key position uses the same `rc.ph` format documented for `AssignKey` and
`RecallKeys` in the [User Mode](#UserModeMenu) section. For an unshifted key,
an integer row/column value such as `34` is enough. Shifted and alpha keys use
a decimal suffix; for example `45.2` is the left-shifted backspace key.

For example, the following program pushes `1` on the stack, then simulates
pressing the key that evaluates `SIN`:

```rpl
« 1 34 KeyEval »
```

See also `Wait`, which waits for a real key press and returns its key position,
and `KeyCode`, which returns the object bound to a key without executing it.


## KeyCode

Return the object bound to a key without executing it.

`KeyCode` takes a [key position](#Key-positions) from the top of the stack and
replaces it with the object currently assigned to that key: a command, program,
text, or menu object. The object is not evaluated.

The lookup uses the current soft menu, any keyboard layout loaded with the
simulator `-k` option, and the standard keyboard bindings. It does not read
user key assignments from the `KeyMap` directory; use `RecallKeys` to inspect
those.

The key position format is the same as for `AssignKey`, `KeyEval`, and
`Wait`. See [Key positions](#Key-positions) in the User Mode section.

For example, the following returns the command bound to the unshifted `SIN`
key (row 3, column 4):

```rpl
34 KeyCode
@ Expecting sin
```

The left-shifted backspace key is at position `45.2`:

```rpl
45.2 KeyCode
@ Expecting ClearThingsMenu
```


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
