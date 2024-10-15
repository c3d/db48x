# Constants

Constants are defined by the `config/constants.csv`, and accessed using
the `ConstantsMenu` or the `CONST` command.

Library equations are defined by the `config/equations.csv`, and accessed using
the `EquationsMenu` or the `LibEq` command.

Library items are defined by the `config/library.csv`, and accessed using the
`Library` command or the `XLib` command.


## Constant

Returns the value of a constant from the constants library.
The name can be given as a symbol or as text.

`'c'` ▶ `299792458_m/s`


## LibraryEquation

Returns the value of a library equation from the equation library.
The name can be given as a symbol or as text.

`"RelativityMassEnergy"` ▶ `"'E=m*c^2'"`


## LibraryItem

Returns the value of a library item from the library.
The name can be given as a symbol or as text.

`'Dedicace'"` ▶ `"À tous ceux qui se souviennent de Maubert électronique"`
