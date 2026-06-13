# Operations with Ranges

Operations on ranges include traditional mathematical operations such as `+` or
`sin`, standard object operations such as `Obj→`, as well as specific operations
listed below.

## →Range

Build a range (interval in the form `a…b`) out of two individual components for
the low and high value:

```rpl
2 3 →Range
@ Expecting 2…3
```

Input values are sorted so that the range is normalized:

```rpl
3 2 →Range
@ Expecting 2…3
```

This function can also convert another type of range to an interval:

```rpl
6±4 →Range
@ Expecting 2…10
```

This command can be used in algebraic expressions with one or two arguments:

```rpl
'→Range(2;3)' →Num
@ Expecting 2…3
```

```rpl
'→Range(6±4)' →Num
@ Expecting 2…10
```

## →∆Range

Build a delta range (range of the form `a±b`) out of two individual components
for the center and delta value.

```rpl
4 6 →∆Range
@ Expecting 4±6
```

Negative values for the second argument are converted to positive:

```rpl
6 -4 →∆Range
@ Expecting 6±4
```

This function can also convert another type of range to delta form:

```rpl
6±50% →∆Range
@ Expecting 6±3
```

This command can be used in algebraic expressions with one or two arguments:

```rpl
'→∆Range(4;6)' →Num
@ Expecting 4±6
```

```rpl
'→∆Range(6±50%)' →Num
@ Expecting 6±3
```

## →%Range

Build a percentage range (range of the form `a±b%`) out of two individual
components for the center value and percentage of change:

```rpl
10 20 →%Range
@ Expecting 10±20%
```

Input values are sorted so that the range is normalized.

```rpl
1 -200 →%Range
@ Expecting 1±200%
```

This function can also convert another type of range to percentage form:

```rpl
0…4 →%Range
@ Expecting 2±100%
```

This command can be used in algebraic expressions with one or two arguments:

```rpl
'→%Range(10;20)' →Num
@ Expecting 10.±20.%
```

```rpl
'→%Range(0…4)' →Num
@ Expecting 2±100%
```

## →σRange

Build an uncertain number (range of the form `a±σb`) out of two individual
components for the mean and standard deviation value.

```rpl
100 2 →σRange
@ Expecting 100±σ2
```

Input values are not sorted, since a large mean may have a smaller standard
deviation.

Uncertain numbers cannot be converted to and from other types of range.

This command can be used in algebraic expressions with two arguments:

```rpl
'→σRange(100;2)' →Num
@ Expecting 100±σ2
```


## FromRange

Extract the components from a range object, returning the low and high values for
interval ranges, or center and delta values for delta/percent ranges.

```rpl
2…3 Range→ 2 →List
@ Expecting { 2 3 }
```

```rpl
6±4 Range→ 2 →List
@ Expecting { 6 4 }
```

This command can be used in algebraic expressions, in which case it automatically produces a list with two results:

```rpl
'Range→(2…3)' →Num
@ Expecting { 2 3 }
```


## RangeUnion

Perform the union of two ranges

```rpl
1…3 2…6 ∪
@ Expecting 1…6
```

This command can be used in algebraic expressions:

```rpl
'∪(1…3;2…6)' →Num
@ Expecting 1…6
```


## RangeIntersect

Perform the intersection of two ranges

```rpl
1…3 2…6 ∩
@ Expecting 2…3
```

This command can be used in algebraic expressions:

```rpl
'∩(1…3;2…6)' →Num
@ Expecting 2…3
```
