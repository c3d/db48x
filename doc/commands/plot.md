# Scalable plots and graphics

Graphics are rendered on the screen based on a coordinate systems defined in the
`PlotParameters` variables.

DB48X supports multiple plotting modes:
* **Function** - Standard y=f(x) plots
* **Polar** - Polar coordinate plots
* **Parametric** - Parametric curve plots
* **Scatter** - Scatter plots from data
* **Bar** - Bar charts from data
* **Histogram** - Histogram plots for frequency distributions


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

## FunctionPlot

Plot a function in the form y=f(x). The function is taken from the stack.

```rpl
'tan(13*x) * sin(500*x)' FunctionPlot
@ Image fnplot-example
```


## PolarPlot

Plot a function in polar coordinates, where r=f(θ).

```rpl
'6*sin(15*x)*sin(113*x)' PolarPlot
@ Image polarplot-example
```


## ParametricPlot

Plot a parametric curve where x and y are both functions of a parameter.

The input function returns a complex number where the real part is interpreted
as the position along the x axis and the imaginary part is interpreted as the
position along the y axis.


```rpl
'exp((0.25+4.5ⅈ)*x)' ParametricPlot
@ Image parametricplot-example
```

## TruthPlot

Plot a curve where an expression in `x` and `y` is tested.

The input function can return one of:
* A truth value `True` or `False`
* A real value between `0.0` and `1.0`
* A complex value that represents a color on the color wheel

When `Res` is at the default value `0`, the number of bins along the X and Y
axis is set by `XYPlotBins`.

### Truth plot with a truth value

The following example illustrates the use of the `TruthPlot` with a truth value:

```rpl
'x²-2·y²>1.9+38·sin(21·x·y)' TruthPlot
@ Image truthplot
```

### Truth plot with a real value

The following example illustrates the use of the `TruthPlot` with a real value:

```rpl
'sin(21·x·y)' TruthPlot
@ Image truthplot-real
```

### Truth plot with a complex value

The following example illustrates the use of the `TruthPlot` with a real value:

```rpl
« x y RealToComplex 0.15 * »  TruthPlot
@ Image truthplot-complex
```

Note that if `AutoSimplify` is enabled, this graph will show a white horizontal
line corresponding to the real axis. This is because the multiplication by
`0.15` auto-simplifies the result as a real number if the imaginary part is
zero.

## ScatterPlot

Create a scatter plot from the first two columns of a matrix, the first column
representing the horizontal axis, the second column representing the vertical
axis.

```rpl
[ [ -5 -5 ]
  [ -3  0 ]
  [ -5  5 ]
  [  0  3 ]
  [  5  5 ]
  [  3  0 ]
  [  5 -5 ]
  [  0 -3 ]
  [ -5 -5 ] ]
ScatterPlot
@ Image scatterplot-example
```

## BarPlot

Create a bar chart from a vector of values.

The size of the vector defines the number of bars being displayed.
Unlike other plotting commands, the `xrange` setting is not taken into account
when positioning the bars horizontally. However, the `yrange` setting is used to
determine the vertical position of the bars.

```rpl
[1 2 3 4 3 2 1 -1 -3 -5 3 5 1 2 3 ]
BarPlot
@ Image barplot-example
```

When `Res` is at the default value `0`, the number of bins along the X and Y
axis is set by `StatsPlotBins`.


## HistogramPlot

Create a histogram from a data vector on the stack.

The `PPar` variable defines the horizontal range (as defined by `xrange`) and
the vertical range (as defined by `yrange`). The resolution parameter in `PPar`
defines the size of the bins. With the default value of `0`, the range is
subdivided into 25 bins.

```rpl
1 RDZ
ClΣ
1 1000 for i
  RAND 21 * 10 - Σ+
next
RclΣ
-1 100 YRANGE
HistogramPlot
@ Image histogramplot-example
```

When `Res` is at the default value `0`, the number of bins along the X and Y
axis is set by `StatsPlotBins`.
