# Variables

Variables are named storage for RPL values.

## Store (STO)
Store an object into a global variable

## Recall (RCL)
Recall the contents of a variable


## StoreAdd (STO+)
Add to the content of a variable


## StoreSubtract (STO-)
Subtract from the contents of a variable


## StoreMultiply (STO×)
Multiply contents of a variable


## StoreDivide (STO÷)
Divide the content of a variable


## Increment (INCR)
Add one to the content of a variable


## Decrement (DECR)
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


## CreateDirectory (CRDIR)
Create new directory


## PurgeDirectory (PGDIR)
Purge entire directory tree


## UpDirectory (UPDIR)
Change current directory to its parent


## HomeDirectory (HOME)
Change current directory to HOME


## DirectoryPath (PATH)
Get a path to the current directory


## Variables (VARS)
List all visible variables in a directory


## ALLVARS
List all variables in a directory


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


## TVARS
List variables of a specific type


## TVARSE
List all variables with extended type information


## SADD
Apply command ADD to the stored contents of the variable


## SPROP
Store a property to a variable


## RPROP
Recall a property of a variable


## PACKDIR
Pack a directory in an editable object
