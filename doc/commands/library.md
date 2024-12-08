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
`Libs`. The `Attach` commadn can be used to load items ahead of time. The
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
@ Expecting { Dedicace }
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
@ Expecting { }
```

## Libs

Returns a list containing the currently attached libraries.

A typical sequence to reload the library items after changing the source files
on disk is:

```rpl
Libs Duplicate Detach Attach
```
