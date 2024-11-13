## Catalog

The `Catalog` command is triggered by the `+` command while in _Alpha_ mode, or
using 🟨 _+_ otherwise (_CAT_). It is an auto-completing catalog of all the
available commands or characters.

When entering a program, all commands matching the text currently matched will
be shown. For example, if you typed `A`, all commands containing an `A` are
displayed in alphabetical order. If you type `ACO`, `ACOS, `ACOSH` and
`DataColumn` (a possible spelling for `ColΣ`) will all be shown.

Inside text, all characters related to the last entered character will be shown.
For example, if you typed `A`, options to add accents to `A` or to select the
Greek `α` or the related `ª` character. If no matching character is found,
the `CharactersMenu` is presented, allowing you to select specific characters
from predefined categories.


## CharactersMenu

The `CharactersMenu`, which can be accessed using 🟦 _2_ (_CHAR_), presents a
convenient way to insert characters.

While entering text, the [Catalog](#Catalog) also presents matching characters,
or the characters menu if none is found.

The characters menus and the catalog behaviour can be configured by modifying
the `config/characters.csv` file. Each row in that file contains two texts:

* The first is the name of the menu. If the name is empty, the menu is not
  shown, but is used to find characters related to a given character.

* The second is the list of characters to be shown.

Some language-specific submenus in the characters menu enable *transliteration*,
which select alphabetic characters in that language. For example, if you use the
character menu for Greek and lowercase, then typing `A` will insert
`α` (greek lowercase alpha). Repeated use of the same key cycles through
options, in that case `ά` and `a` before cycling back to `α`.


## ConstantsMenu

The `ConstantsMenu` gives access to numerous [constants](#constants), shown in
various categories. This includes basic mathematical constants such as `π` or
`e`, as well as constants related to chemistry, physics, computer science or
history.

The constants menu is defined by the `config/constants.csv` file. You are
encouraged to tailor this file to suit your own needs.


## ToggleCustomMenu

The `ToggleCustomMenu` command toggles between the `VariablesMenu` and the
`CustomMenu`. It is normally bound to the _VAR_ key, so that a first press on
this key shows the variables in the current directory, and a second press shows
any user-defined custom menu if it exists.


## CustomMenu

The `CustomMenu` command activates a menu defined by the variable of the same
name. The content of the variable should evaluate as an array or list, where
each item is either an array or list of the form `{ Name Value }`, or an object
that is made directly accessible in the menu. See `Menu` below for a description
of the format of menu descriptions.

The `Menu` command defines the `CustomMenu` variable and shows the corresponding
custom menu.


## Menu

This command shows up a system menu or defines a custom menu.

If the object on the stack is a menu name, typically returned by `RecallMenu`,
then `Menu` will activate that menu.

If the object on the stack is `0`, then `Menu` has the same effect as
`LastMenu`, showing the last menu selected. Other numerical values, which select
built-in menus on HP's implementation, will cause an `Unimplemented` error.

If the object on the stack is an array or a list, `Menu` will use that array or
list to set the value of the `CutomMenu` variable, and then behave like the
`CustomMenu` command, i.e. show the menu defined by the list.

The following defines a `CustomMenu` adding or subtracting powers of `10`

```rpl
{ { "1+"   «   1 + » } { "1-"   «   1 - » }
  { "10+"  «  10 + » } { "10-"  «  10 - » }
  { "100+" « 100 + » } { "100-" « 100 - » } }
MENU
```

You can define a menu of vertically-stacked menu items using a matrix.
For example, you can define a menu that lets you store, evaluate or purge
variables `X` and `Y as follows:

```rpl
[ [ X                      Y                      ]
  [ { ">X" « 'X' Store » } { ">Y" « 'Y' Store » } ]
  [ { "-X" « 'X' Purge » } { "-Y" « 'Y' Purge » } ] ]
MENU
```

## TMenu

The `TMenu` command works like [Menu](#menu), but does not define the
`CustomMenu` variable. The resulting menu is temporary and cannot be found by
`RecallMenu` or by the menu history in `LastMenu`.

For example, the following defines a temporary menu showing

```rpl
{ 2 3 5 7 11 13 97 197 397 797 997 1097 1297 1597 1697 1997 2297 2797 }
TMENU
```


## RecallMenu

Return the last menu entry, or `0` if there is no current menu. The returned
value can be used as an argument to `Menu` or `TMenu`.
