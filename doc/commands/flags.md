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
