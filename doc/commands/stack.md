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
Put the last arguments back on the stack

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
