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
The assignment is placed in the `KeyMap` variabe for the current directory.

To clear an assignment, assign `StandardKey` to a given key.

For example, to assign the `MathMenu` to the `exp` key, use the following:
```rpl
'MathMenu' 24 ASN
```

To restore the standard key assignment for that same key, use the following:
```rpl
'StandardKey' 24 ASN
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
