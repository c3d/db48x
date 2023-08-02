# Operations with Symbolic Expressions

## Rewrite

Applies an arbitrary transformation on equations. The first argument is the
equation to transform. The second argument is the pattern to match. The third
argument is the replacement pattern. Patterns can contain variable names, which
are substituted with the corresponding sub-expression.

In the matching pattern, variables with a name that begins with `i`, `j`, `k`,
`l`, `m`, `n`, `p` or `q` must match a non-zero positive integer. When such a
match happens, the expression is evaluated after rewrite in order to compute
values such as `3-1`.

Additionally, variables with a name that begins with `u`, `v` or `w` must
be _unique_ within the pattern. This is useful for term-reordering rules,
such as `'x*u*x' 'x*x*u'`, which should not match `a*a*a` where it is a no-op.

`Eq` `From` `To` ▶ `Eq`

Examples:
* `'A+B+0' 'X+0' 'X' rewrite` returns `'A+B'`
* `'A+B+C' 'X+Y' 'Y-X' rewrite` returns `'C-(B-A)`
* `'(A+B)^3' 'X^N' 'X*X^(N-1)' rewrite` returns `(A+B)*(A+B)^2`.


## AUTOSIMPLIFY
Reduce numeric subexpressions


## RULEMATCH
Find if an expression matches a rule pattern


## RULEAPPLY
Match and apply a rule to an expression repeatedly


## →Num (→Decimal, ToDecimal)

Convert fractions and symbolic constants to decimal form.
For example, `1/4 →Num` results in `0.25`.

## →Frac (→Q, ToFraction)

Convert decimal values to fractions. For example `1.25 →Frac` gives `5/4`.
The precision of the conversion in digits is defined by
[→FracDigits](#ToFractionDigits), and the maximum number of iterations for the
conversion is defined by [→FracDigits](#ToFractionIterations)

## RULEAPPLY1
Match and apply a rule to an expression only once


## TRIGSIN
Simplify replacing cos(x)^2+sin(x)^2=1


## ALLROOTS
Expand powers with rational exponents to consider all roots


## CLISTCLOSEBRACKET


## RANGE
Create a case-list of integers in the given range.


## ASSUME
Apply certain assumptions about a variable to an expression.
