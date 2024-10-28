# Graphic commands

DB48X features a number of graphic commands. While displaying graphics, the
stack and headers will no longer be updated.

## Coordinates

DB48X recognizes the following types of coordinates

* *Pixel coordinates* are specified using based numbers such as `#0`, and
  correspond to exact pixels on the screen, and . Pixels are counted starting
  from the top-left corner of the screen, with the horizontal coordinate going
  from `10#0` to `10#399`, and the vertical coordinate going from `10#0` to
  `10#239`.

* *User unit coordinates* are scaled according to the content of the `PPAR` or
  `PlotParameters` reserved variables.

* *Text coordinates* are given on a square grid with a size corresponding to the
  height of a text line in the selected font. They can be fractional.

Coordinates can be given using one the following object types:

* A complex number, where the real part represents the horizontal coordinate and
  the imaginary part represents the vertical coordinate.

* A 2-element list or vector containing the horizontal and vertical coordinates.

* A 1-element list of vector containing one of the above.

For some operations, the list or vector can contain additional parameters beyond
the coordinates. The selection of unit or pixel coordinates is done on a per
coordinate basis. For exmaple, `{ 0 0 }` will be the origin in user coordinates,
in the center of the screen if no `PPAR` or `PlotParameters` variable is
present.

Note that unlike on the HP48, a complex value in DB48X can
contain a based number.


## ClearLCD

Clear the LCD display, and block updates of the header or menu areas.


## DrawText

Draw the text or object in level 2 at the position indicated by level 1. A text
is drawn without the surrounding quotation marks.

If the position in level 1 is an integer, fraction or real number, it is
interpreted as a line number starting at 1 for the top of the screen. For
example, `"Hello" 1 disp` will draw `Hello` at the top of the screen.
If the position is a based number, it is a row number in pixels. For example,
`"Hello" #120d disp` will show the test in the middle of the screen.

If the position in level 1 is a complex number or a list, it is interpreted as
specifying both the horizontal or vertical coordinates, in either pixel or unit
coordinates. For example `"Hello" { 0 0 } disp` will draw `Hello` starting in
the center of the screen.

Text is drawn using the stack font by default, using the
[foreground](#foreground) and [background](#background) patterns.

If level 1 contains a list with more than 2 elements, additional elements
provide:

* A *font number* for the text

* An *erase* flag (default true) which indicates whether the background for the
  text should be drawn or not.

* An *invert* flag (default false) which, if set, will swap the foreground and
  background patterns.

For example, `"Hello" { #0 #0 3 true true } DrawText` will draw `Hello` in the
top-left corner (`#0 #0`) with the largest (editor) font (font identifier `3`),
erasing the background (the first `true`), in reverse colors (the second
`true`).

## DrawStyledText

Draw the text or object in level 3 at the position indicated by level 2, using
the font specified in level 1. This behaves like [DrawText](#drawtext), except
for the additional parameter specifying the font size.

This command is compatible with the HP50G usage, where the position is specified
by a list containing two decimal integer values. A font size of 1 displays with
a small font, a font size of 2 displays with a regular font.

In addition to this HP50G-compatible usage, `DispXY` will also accept:

* A smaller font size, 0, for the help size, and other font sizes above 2, which
  are compatible with the values given to [StackFont](#stackfont).

* The position can accept the same values as [DrawText](#drawtext), including a
  single integer value indicating a line number, a fractional line position, or
  coordinates scaled according to [PlotParameters](#plotparameters).


## Show

Display the first level of the stack using the entire screen, with a possible
scroll using the 4, 8, 6 and 2 keys if the object is larger than fits on screen.
This makes it possible to comfortably examine very large objects, like `300!`, a
large program or a complicated equation. Arrow keys can also be used for
horizonal or vertical scrolling.

The maximum size of the graphic object is defined by the
[MaximumShowWidth](#maximumshowwidth) and
[MaximumShowHeight](#maximumshowheight) settings.

## DrawLine

Draw a line between two points specified by level 1 and level 2 of the stack.

The width of the line is specified by [LineWidth](#linewidth). The line is drawn
using the [foreground](#foreground) pattern.


## PlotParameters

The `PlotParameters` reserved variable defines the plot parameters, as a list,
with the following elements:

* *Lower Left* coordinates as a complex (default `-10-6i`)

* *Upper Right* coordinates as a complex (default `10+6i`)

* *Independent variable* name (default `x`)

* *Resolution* specifying the interval between values of the independent
  variable (default `0`). A binary numnber specifies a resolution in pixels.

* *Axes* which can be a complex giving the origin of the axes (default `0+0i`),
  or a list containing the origin, the tick mark specification, and the names of
  the axes.

* *Type* of plot (default `function`)

* *Dependent variable* name (default `y`)


## Pict

`Pict` is the name given to the graphics shown on the calculator's screen.
It can only be used as an argument to `Store`, `Recall` and graphical
operations such as `GAnd`.

[Not fully implemented yet: `Store` and `Recall` do not work]


# Bitmap operations

## ToGrob

Creates a graphics object from a specified object on the stack.

The first level of the stack specifies the character size used while
rendering the object, where value `0` indicates the `StackFont` value.

If the second level of the stack is a text, then the quotes that appear
the text is displayed on the stack will not be included in the generated
graphic object. This is similar to the behaviour of `Disp`. The rendering
of objects respects the settings used to render on the stack, e.g.
`FIX` mode or `VerticalVectors`.

The object to draw must fit in a bit map at most `MaxW`-pixels wide and
`MaxH`-pixels high.


```rpl
@ Show font sizes
0 7 for fontID
  "Font " fontID + fontID →Grob
next
```

## GXor

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GXor` is used for creating cursors, for example, to make the cursor image
appear dark on a light background and light on a dark background. Executing
`Gxor` again with the same image restores the original picture.

`GXOR` uses a logical exclusive OR to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Blinking cursor
"Hello" 1 DISP
1 20 start
  Pict { #0 #0 } "█" 3 →Grob GXor 0.5 Wait
next
```


## GOr

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GOr` uses a logical OR to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.
On DB48X, pixels that are set appear white.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Erasing cursor
"Hello World" 1 DISP
1 50 for i
  Pict i R→B { #0 } + "▶" 3 →Grob GOr 0.05 Wait
next
```

## GAnd

Superimposes a source graphic object onto a destination graphic object, which
can be `Pict` to represent what is presently shown on screen. The upper left
corner pixel of the positioned at the specified coordinate in the destination.

`GAnd` uses a logical AND to determine the state of the pixels (on or
off) in the overlapping portion of the argument graphics objects.
On DB48X, pixels that are set appear white.

If the destination is not `Pict`, the resulting graphic object is returned on
the stack. If the destination is `Pict`, the screen is updated and no result
is returned on the stack.

```rpl
@ Darkening cursor
"Hello World" 1 DISP
1 250 for i
  Pict i R→B { #0 } + "▓" 3 →Grob GAnd 0.05 Wait
12 step
```


## GraphicAppend

Append two graphic objects side by side.
The two graphic objects are vertically centered with respect to one another.

```rpl
@ Juxtapose two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicAppend
```


## GraphicStack

Stack two graphic objects on top of one another.
The two graphic objects are horizontally centered with respect to one another.

```rpl
@ Stack two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicStack
```

## GraphicSubscript

Combine two graphic objects with the second one in subscript position

```rpl
@ Subscript with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicSubscript
```

## GraphicExponent

Combine two graphic objects with the second one in exponent position

```rpl
@ Exponent with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicExponent
```

## GraphicRatio

Combine two graphic objects as if they were in a fraction

```rpl
@ Ratio with two font sizes
"ABC" 4 →Grob
"DEF" 2 →Grob
GraphicRatio
```

## GraphicRoot

Generate a square root sign around a graphical object

```rpl
@ Square root sign
"ABC" 4 →Grob
GraphicRoot
```

## GraphicParentheses

Generate parentheses around a graphical object

```rpl
@ Parentheses around graphic
"ABC" 4 →Grob
GraphicParentheses
```

## GraphicNorm

Generate a norm (vertical bars) around a graphical object

```rpl
@ Norm around graphic
"ABC" 4 →Grob
GraphicNorm
```


## GraphicSum

Generate a sum (capital Sigma) sign of the given size

```rpl
@ 128-pixel Sigma sign
128 GraphicSum
```

## GraphicProduct

Generate a product (capital Pi) sign of the given size

```rpl
@ 96-pixel Sigma sign
96 GraphicProduct
```

## GraphicIntegral

Generate an integral sign of the given size

```rpl
@ 45-pixel Sigma sign
45 GraphicIntegral
```


## Header

The `Header` command updates a special variable also called `Header`.

When that variable is present, it must evaluate to something that can render
graphically, either directly a graphic object or a (possibly multi-line) text.

When a header is provided, the normal content of the header, i.e. date, time and
name of the state file, is no longer shown.  However, annunciators and battery
status are still overimposed.

It is the responsibility of the programmer to ensure that the header program
does not draw important data at these locations, and also to make sure that the
header program is "well behaved", i.e. does not leave things on stack. If the
header program generates an error, then that error may get in the way of normal
calculator operations.

```rpl
« TIME " " PATH TAIL TOTEXT + + "
" + DATE + " Mem: " + MEM + » HEADER
```
