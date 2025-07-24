# Scalable plots and graphics

Graphics are rendered on the screen based on a coordinate systems defined in the
`PlotParameters` variables.

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

To reset the `PlotParameters` to the default values, it is necessary to purge
the current directory as well as the parents from any `PlotParameters` value:

```rpl
'PPAR' PGALL
```


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
