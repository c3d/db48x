## config/constants.csv

With this file you can add your own constants, or overrule build in constants.

The first column is interpreted as either:
- a menu label (when there is no second column);
- a constant name.

The second column is interpreted as the constant value.


A hierarchy of menus is created by using a forward slash as separator between the levels in the menu label.
```
"Dates"

"Dates/America"
"IndigenousPeoplesDay",		"14921012_date"
```
Only the deepest menu level may contain constants.
Constants on intermediate levels are ignored.


The constant value can be a single value, as above, or an array containing:
- value;
- standard uncertainty;
- relative uncertainty;
- optionally a label (defaults to the value).

```
"Sample"

"C1",	"[ 3 0.5 0.16667 aLabel ]"
"C2",	"[ '1+2' 0.5 1/6 ]"
"C3",	"[ v sv 'sv / v' ]"
```
As shown, algebraic expressions can be used.
Even variables can be used.
For `C3` the variables `v` and `sv` have to defined.

Using other constants, their absolute or relative uncertainty in the value (array) of a constant, may not work.
It may crash the calculator program.
An rpl program can be put int the value (array) but it will not be evaluated.
