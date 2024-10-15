# Operations with Units

## UDEFINE
Create a user-defined unit


## UPURGE
Delete a user-defined unit


## UnitValue

Return the numeric part of a unit object.

`3_km`  ▶ `3`

## BaseUnits

Expand all unit factors to their base units.

`3_km`  ▶ `3000_m`


## Convert

Convert value from one unit to another. This convert the values in the second level of the stack to the unit of the object in the first level of the stack. Only the unit in the first level of the stack is being considered, the actual value is ignored. For example:

`3_km` `2_m` ▶ `3000_m`



## FactorUnit

Factor the unit in level 1 from the unit expression of the level 2 unit object.

`1_W` `1_N` ▶ `1_N*m/s`


## →Unit

Creates a unit object from a real number and the unit part of a unit object.
`→Unit` adds units to a number, combining the number and the unit part of a
unit object. The numerical part of the unit object is ignored.

`→Unit` is the reverse of `Unit→` or of `Obj→` applied to a unit object.

```rpl
@ Will be 3000_km
3000 2_km →Unit
```


## ULIST
List all user-defined units
