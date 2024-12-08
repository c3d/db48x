# Comparisons

Comparison operators take two inputs and return a truth value,
`True` or `False`.

For compound objects, lexicographical order is used.
For text, Unicode code point order is used.


## True

Represent a logical truth, the opposite of `False`.

## False

Represent a logical untruth, the opposite of `True`.


## =

Checks if two values are mathematically equal.
This does not check the type of the object but its value.
The `=` comparison operator can be used to form equations such as `'X=1'`, for
example for use with the `Root` numerical solver.

```rpl
@ Expecting True
'1=2.0-1.0' Evaluate
```

In most cases, `=` is the best replacement for `==` in programs written for
HP variants of RPL.


## ≠

Check if two values are mathematically different. This is the opposite of `=`.

```rpl
@ Expecting False
'1≠2.0-1.0' Evaluate
```

## ==

Checks if two objects are identical after evaluating names
(contrary to `same`).

```rpl
A=1
'A==1' Evaluate
@ Expecting True
```

Unlike `=`, `==` takes the type of the object into account, meaning that
`'1==1.0'` evaluates as `False`.

```rpl
'1==2.0-1.0' Evaluate
@ Expecting False
```

## Same

Checks if two objects are identical without evaluating names
(contrary to `==`).

```rpl
A=1
'A' 1 SAME
@ Expecting False
```


## <

Check if the first value is less than the second value.

```rpl
{}
1 2 < + 2.3 1.2 < +
"ABC" "DEF" < + "a" "à" < +
{ 1 2 3 } { 1 2 4 } < + { 1 2 3 } { 1 2 3 } < +
@ Expecting { True False True True True False }
```

## ≤

Check if the first value is less than or equal to the second value.

```rpl
{}
1 2 ≤ + 2.3 1.2 ≤ +
"ABC" "DEF" ≤ + "a" "à" ≤ +
{ 1 2 3 } { 1 2 4 } ≤ + { 1 2 3 } { 1 2 3 } ≤ +
@ Expecting { True False True True True True }
```

## >

Check if the first value is greater than the second value.

```rpl
{}
1 2 > + 2.3 1.2 > +
"ABC" "DEF" > + "a" "à" > +
{ 1 2 3 } { 1 2 4 } > + { 1 2 3 } { 1 2 3 } > +
@ Expecting { False True False False False False }
```

## ≥

Check if the first value is greater than or equal to the second value.

```rpl
{}
1 2 ≥ + 2.3 1.2 ≥ +
"ABC" "DEF" ≥ + "a" "à" ≥ +
{ 1 2 3 } { 1 2 4 } ≥ + { 1 2 3 } { 1 2 3 } ≥ +
@ Expecting { False True False False False True }
```
