# Scalable plots and graphics

## BEGINPLOT
Initialize a new current plot object


## EDITPLOT
Set the current plot object to the given graphic


## ENDPLOT
Finish current plot object and leave it on the stack


## STROKECOL
Change the current stroke color


## STROKETYPE
Change current stroke type


## FILLCOL
Change the current fill color


## FILLTYPE
Change the current fill type


## FILL
Fill the last polygon


## STROKE
Draw the outline of the last polygon


## FILLSTROKE
Draw the outline and fill the last polygon


## MOVETO
Move current coordinates


## LINETO
Draw a line


## CIRCLE
Draw a circle


## RECTANG
Draw a rectangle


## CTLNODE
Add a control node to the current polygon


## CURVE
Draw a curve using all previous control points


## BGROUP


## EGROUP


## DOGROUP


## BASEPT


## TRANSLATE


## ROTATE


## SCALE


## CLEARTRANSF


## SETFONT


## TEXTHEIGHT


## TEXTOUT


## INITRENDER
Set which library will be used as default renderer


## DORENDER
Render a graphics object using the current renderer


## PANVIEW
Shift the center of viewport to render graphics


## ROTVIEW


## SCLVIEW
Set scale to render graphics


## VIEWPORT


## VIEWALL


## PlotMin

Set the minimum value for the current plot range.

The `PMin` value is typically a complex number giving the coordinates of the
point that will show as the bottom-left corner of the screen.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using `PMin`
to ensure that the lower-left corner of the screen corresponds to coordinates
`(-1;-1)`, which brings the circle closer to the lower-left corner:

```rpl
(0;0) 1 CIRCLE
(-1;-1) PMIN
(0;0) 1 CIRCLE
@ Image plotmin-circle
```

## PlotMax

Set the maximum value for the current plot range.

The `PMax` value is typically a complex number giving the coordinates of the
point that will show as the top-right corner of the screen.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using `PMax`
to ensure that the lower-left corner of the screen corresponds to coordinates
`(3;2)`, which brings the circle closer to the top-right corner:

```rpl
(0;0) 1 CIRCLE
(3;2) PMAX
(0;0) 1 CIRCLE
@ Image plotmax-circle
```


## XRange

Set the X-axis range for the current plot.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using
`XRange` to ensure that the horizontal range is between `-1.5` and `1.5`, which
causes the circle to be elongated horizontally:

```rpl
(0;0) 1 CIRCLE
-1.5 1.5 XRNG
(0;0) 1 CIRCLE
@ Image xrange-circle
```

## YRange

Set the Y-axis range for the current plot.

The following code draws a circle centered on `(0;0)` and with radius `1`, first
with the default coordinates, where it appears centered, then after using
`YRange` to ensure that the horizontal range is between `-1.5` and `1.5`, which
causes the circle to be elongated vertically:

```rpl
(0;0) 1 CIRCLE
-1.5 1.5 YRNG
(0;0) 1 CIRCLE
@ Image yrange-circle
```
