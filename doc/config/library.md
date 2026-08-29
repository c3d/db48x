## config/library.csv

With this file you can add your own library entries, or overrule built-in
ones. Entries are RPL objects that the calculator loads on demand, and that
you call by name from the `Lib` menu or with the `Ⓛ` prefix.

The first column is interpreted as either:
- a menu label (when there is no second column);
- a library entry name.

The second column is interpreted as the entry value.

A hierarchy of menus is created by using a forward slash as separator between
the levels in the menu label.

```
"Astronomy"

"Astronomy/Ephemeris Orbital Elements"
	"PeriSel",	"=/library/PeriSel.48s"
```

Only the deepest menu level may contain entries.


### The three forms of a value

The value is read in one of three ways:

- `"="` on its own loads the object from a file named after the entry,
  under `library/`;
- `"=path"` loads it from that path;
- anything else is parsed on the spot as an RPL object.

```
	"KineticEnergy",	"="			; library/KineticEnergy.48s
	"PeriSel",	"=/library/PeriSel.48s"	; that exact path
	"Dedicace",	"""À tous ceux…"""	; a text object, inline
```


### File names must be ASCII

The label may contain any character. The file name may not.

The calculator reads its FAT volume through FatFs, which cannot open a file
whose name is not ASCII. The entry then fails with
`Invalid or unknown library entry`, or, when it is reached indirectly from an
equation, with the far less obvious
`Root error: Unable to solve for all variables`.

This bites hardest with the bare `"="` form, which takes the file name from
the entry name. An entry called `☿Φf` written as

```
	"☿Φf",	"="
```

looks for `library/☿Φf.48s`, which the device cannot open. Give such entries
an explicit ASCII path and leave the label alone:

```
	"☿Φf",	"=/library/Astronomy/MerPhif.48s"
```

Nothing shows this in the Qt simulator: it reads the same files from a
filesystem that accepts those names. See #1722, where 42 of 93 programs were
unusable on a DM32 and all of them worked on the simulator.


### The order of the file matters

Entries are numbered by their position, sections not counted, and `Attach`
and `Detach` accept those numbers. Inserting an entry renumbers everything
after it.

Top-level sections take the soft keys of the `Lib` menu in file order, so
inserting a section moves every section below it by one key. The tests in
`src/tests.cc` navigate that menu by key position and have to follow: they
went from `F3` to `F4` when an `Astronomy` section was inserted, and to `F5`
when `Astronautics` was.

Adding at the end costs nothing. Inserting is fine too, but expect to adjust
those tests in the same commit.


### Files in subdirectories have to be packaged

The Makefile ships `library/*.48[sSbB]` and `library/*/*.48[sSbB]`, which is
one level of subdirectory. A program two levels down is never copied to the
calculator, and `library.csv` then names a file that is not there.
