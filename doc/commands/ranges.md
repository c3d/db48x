# Operations with Ranges

Operations on ranges include traditional mathematical operations such as `+` or
`sin`, standard object operations such as `Obj→`, as well as specific operations
listed below.

## →Range

Build a range (interval in the form `a…b`) out of two individual components for
the low and high value.

```rpl
2 3 →Range
@ Expecting 2…3
```

Input values are sorted so that the range is normalized:

```rpl
3 2 →Range
@ Expecting 2…3
```

## →∆Range

Build a delta range (range of the form `a±b`) out of two individual components
for the low and high value.

```rpl
4 6 →∆Range
@ Expecting 5±1
```

Input values are sorted so that the range is normalized.

```rpl
6 4 →∆Range
@ Expecting 5±1
```

## →%Range

Build a percentage range (range of the form `a±b%`) out of two individual
components for the low and high value.

```rpl
0 2 →%Range
@ Expecting 1±100%
```

Input values are sorted so that the range is normalized.

```rpl
2 0 →%Range
@ Expecting 1±100%
```
## →σRange

Build an uncertain number ( `a±b%`) out of two individual
components for the low and high value.

```rpl
1 2 →σRange
@ Expecting 1±σ2
```


## RangeUnion

Perform the union of two ranges

```rpl
1…3 2…6 ∪
@ Expecting 1…6
```


## RangeIntersect

Perform the intersection of two ranges

```rpl
1…3 2…6 ∩
@ Expecting 2…3
```
