# Operations with Symbolic Expressions

## ↑Match

Match pattern up and [rewrite expression](#expression-rewrite), starting with
the innermost subexpressions first. This approach works well for
simplifications. A subexpression simplified during one execution will be a
simpler argument of its parent expression, so the parent expression can be
simplified further.

`Expr` { `From` `To` } ▶ `Expr` `Count`
`Expr` { `From` `To` `Cond` } ▶ `Expr` `Count`

The first argument `Expr` is the expression to transform.

The second argument is a list containing the pattern to match `From`,
the replacement pattern `To`,
and an optional condition `Cond` on the pattern.


## ↓Match

Match pattern down and [rewrite expression](#expression-rewrite), starting with
the outermost expression first. This approach works well for expansion. An
expression expanded during one execution of `↓Match` will contain additional
subexpressions, and those subexpressions can be expanded further.

`Expr` { `From` `To` } ▶ `Expr` `Count`
`Expr` { `From` `To` `Cond` } ▶ `Expr` `Count`

The first argument `Expr` is the expression to transform.

The second argument is a list containing the pattern to match `From`,
the replacement pattern `To`,
and an optional condition `Cond` on the pattern.

## Expression rewrite

Operations such as `↑Match` and `↓Match` apply arbitrary transformations on
algebraic expressions. The way this operates is similar to how HP
calculators perform, but with important differences, controlled by flags.

First, patterns can contain wildcards, which are substituted with the
corresponding sub-expression in the matched `Expr`.

* On HP calculators, the wildcard names must begin with `&`, and only an
  optional external conditions can control what matches or not.

* On DB48X, by default, any name in the pattern acts as a wildcard, and we use
  `&` to refer to a specific variable i.e. `&A` only matches the variable named
  `A`.

Rationale: The default DB48X approach makes it easier to write transformation
rules for the common cases, the `&` character being a bit harder to access on
the calculator's keyboard. The assumption is that we rarely write patterns to
match a specific variable, i.e. replace an expression only if it refers to `X`
but not to `Y`. The HP behaviour can be restored using the `ExplicitWildcards`
flag.

A further extension in DB48X is to give a semantic meaning to the first letter
of variable names:

* `a`, `b` and `c` denote numerical constants, like real or integer values.
* `i`, `j` are positive integer values which may be zero and may be the result
  of a computation, i.e. `3-3` will match.
* `k`, `l`, `m` are non-zero positive integer values, which may be the result of
  a compuation, i.e. `3-2` will match.
* `n`, `o`, `p` are symbols or names like `'ABC'`.
* `u`, `v`, `w` are unique terms, i.e. terms that cannot be presented more than
  once in an expression.
* Names where the initial is lowercase must be sorted, so that `x+y` will match
  `A+B` but not `B+A`.

Another important difference is that on HP calculators, the number of rewrites
of subexpressions is limited to a single pass, irrespective of flag `-100`
(step-by-step CAS mode), whereas DB48X will repeat application by default (this
can be changed by setting `StepByStepAlgebraResults`, or alternatively, by
clearing flag `-100`).

This leads to the last important difference. On HP calculators, `↑Match` and
`↓Match` return either `0` or `1` in the first level of the stack to indicate if
a replacement occurred. On DB48X, the number of replaced subexpressions is
returned, and it can be greater than 1.


Examples:
* `'A+B+0' {'X+0' 'X' } ↓Match` returns `'A+B' 1`
* `'A+B+C' { 'X+Y' 'Y-X' } ↓Match` returns `'C-(B-A)' 2`
* `'(A+B)^3' { 'X^N' 'X*X^(N-1)' } ↓Match` returns `(A+B)*(A+B)^2`.


## Isolate

Isolate variable: Returns an expression that rearranges an expression given in
stack level 2 to "isolate" a variable specified in stack level 1.

For example, `A+1=sin(X+B)+C' 'X' ISOL` will produce `X=sin⁻¹(A-C+1)+2·i1·π-B`.

When the `PrincipalSolution` flag is not set, the resulting expression may
contain numbered variables such as `i1` as parameters. Variables that begin with
`i` represent arbitrary signed integers. Variables that begin with `n` represent
arbitrary natural numbers. Variables that begin with `s` represent arbitrary
signs.

The command will generate `Unable to isolate` if the expression cannot be
reorganized, for example because it contains functions that have no known
inverse.


## Quote

Return an object unevaluated (quoted).

`Obj` `Quote` → `QuotedObj`

The result is an algebraic expression containing `Obj` without evaluating it.
This is used to pass unevaluated names or formulas as arguments, for example
when building symbolic functions or when a CAS command must not evaluate its
parameters prematurely.

`5` `QUOTE` → `'5'`
`'A+B'` `QUOTE` → `'A+B'` (unchanged if already quoted)

`Quote` is mostly useful in algebraic expressions:

```rpl
@ Create an ArcLen commnand
@ Note that the HP50G ARM erroneously lacks ∫ at end of the program
@ We changed the names compared to ARM example because DB48x is case-insensitive,
@ and START..END or VAR would cause a syntax error with original example names.
« → arcstart arcend arcexpr arcvar
  « arcstart arcend arcexpr arcvar ∂ SQ 1 + SQRT arcvar ∫ »
» 'ArcLen' STO

@ Evaluate the result numerically in radians
'ArcLen(0,π,QUOTE(SIN(X)),QUOTE(X))'
RAD →Decimal DEG
@ Expecting 3.82019 77890 3
```

However, the rules for local name evaluations make the use of `Quote` less often necessary than on HP calculator. Consider the following example:

```rpl
@ Example program
« → A B C 'A+B*C' » 'MyFn' STO

@ Evaluation of MyFn with symbolic names
'MyFn(X;Y;Z)' EVAL

@ Evaluation of MyFn with expressions
'MyFn(X+1;Y-1;Z*2)' EVAL

@ Store a numerical value in X
42 'X' STO

@ Evaluation of MyFn without quotes will evaluate X
'MyFn(X+1;Y-1;Z*2)' EVAL

@ Evaluation of MyFn with quote will not evaluate X
'MyFn(Quote(X+1);Y-1;Z*2)' EVAL

4 →List
@ Expecting { 'X+Y·Z' 'X+1+(Y-1)·(Z·2)' '43+(Y-1)·(Z·2)' 'X+1+(Y-1)·(Z·2)' }
```

## Derivative

Compute the derivative function for an expression. The algebraic syntax for `∂` is `'∂name(expr)'` For example, `'∂x(sin(2*x^2)'` computes `4*X*cos(2*X^2)`

When differentiating a user-defined function named `F`, DB48X will generate `F′`
as the name for the derivative function. Note that this differ from HP
calculators, which would use `d1F`. If `F` has multiple parameters, then the
partial derivative relative to the first argument will be denoted as `F′₁`,
the partial derivative relative to the second argument will be denoted as
`F′₂` and so on.

For built-in functions that have no known derivative, such as `GAMMA`, DB48X
will generate an `Unknown derivative` error. Note that this differs from HP
calculators, which would generate a `d1GAMMA` function in that case.

The behaviour of the HP derivative function `∂` depends on whether it is in an
algebraic object (stepwise differentiation) or whether it is used in stack
syntax (full differentiation). The DB48X variant always perform full
differentiation irrespective of the syntax used.

The _HP50G advanced reference manual_ indicates that `∂` substitutes the value
of global variables. For example, if `Y` contains `X+3*X^2`, `'Y' 'X' ∂` is
expected to return `1+6*X`. It actually returns `0`, unless you evaluate `Y`
first. DB48X matches the actual behaviour of the HP50G and not the documented
one. To get the documented behaviour, evaluate the expression prior to computing
its derivative.


## AutoSimplify

Enable automatic reduction of numeric subexpressions according to usual
arithmetic rules. After evaluating `AutoSimplify` `'X+0'` will evaluate as `'X'`
and '`X*1-B*0'` witll evaluate as `'X'`.

The opposite setting is [NoAutoSimplify](#noautosimplify)

## NoAutoSimplify

Disable automatic reduction of numeric subexpressions according to usual
arithmetic rules. After evaluating `NoAutoSimplify`, equations such as`'X+0'`
or `X*1-B*0` will no longer be simplified during evaluation.

The opposite setting is [AutoSimplify](#autosimplify)


## SymbolicResults

Enable the generation of symbolic results, as opposed to `NumericalResults`.
For example, `2 3 /` gives exact result `2/3` as a result with
`SymbolicResults`, as opposed to a numerical approximation like `0.66667`.

## NumericalResults

Enable the generation of symbolic results, as opposed to `SymbolicResults`.  For
example, `2 3 /` gives the numerical approximation `0.66667` when this flag is
set, as opposed to exact result `2/3`.


## FinalAlgebraResults

Evaluate algebraic rules on symbolic expressions repeatedly until no futher change results from applying them.


## StepByStepAlgebraResults

Evaluate algebraic rules on symbolic expressions one step at a time.

## ListExpressionNames

List all variables used in an expression or polynomial, leaving the original
object on the stack, and returning the result as an array.
Names are sorted in decreasing order of size, and for the same size,
in alphabetical order.

`'A*Foo*G(X;Y;cos Z)`  ▶ `'A*Foo*G(X;Y;cos Z)` `[ Foo A G X Y Z ]`
`'(U_V)=(I_A)*(R_Ω)'`  ▶ `'(U_V)=(I_A)*(R_Ω)'` `[ I R U ]`

As a DB48X extension, this command also applies to programs, list and
arrays. Another extension is the `ExpressionVariables` command, which extracts
the units associated with the variables if there are any. The `LName` command
only returns the names, without the associated unit.


## ExpressionVariables

List all variables used in an expression or polynomial, returning the result as
a list.
Names are sorted in decreasing order of size, and for the same size,
in alphabetical order.
If there are units in the expression, the units are returned in the resulting
list.

`'A*Foo*G(X;Y;cos Z)`  ▶ `{ Foo A G X Y Z }`
`'(U_V)=(I_A)*(R_Ω)'`  ▶ `{ (I_A) (R_Ω) (U_V) }`

This is a variation of the HP command `LNAME`, which is a bit more convenient to
use in programs.

## RULEAPPLY1
Match and apply a rule to an expression only once


## TRIGSIN
Simplify replacing `cos(x)²` with `1-sin(x)²`

This command applies the Pythagorean identity to rewrite expressions so that
cosine squares are replaced by sine squares. This favors the use of `sin` over
`cos` in the expression.

`'cos(X)^2'` ▶ `'1-(sin X)²'`
`'cos(A+B)^2'` ▶ `'1-(sin(A+B))²'`


## ALLROOTS
Expand powers with rational exponents to consider all roots


## CLISTCLOSEBRACKET


## RANGE
Create a case-list of integers in the given range.


## ASSUME
Apply certain assumptions about a variable to an expression.

## AlgebraConfiguration

Name reserved for the current algebra configuration directory.

The `AlgebraConfiguration` command provides access to the directory that stores
the current Computer Algebra System (CAS) configuration.
This directory contains settings and variables used for symbolic computations.
If no configuration directory exists, one will be created when needed.
The configuration directory is stored in the global variable with the name
`AlgebraConfiguration`.

## AlgebraVariable

Recall the current CAS algebra variable (HP-50G `VX` / `RCLVX`).

The `AlgebraVariable` command returns the current variable used for polynomial
evaluation and symbolic computations.
If no variable is set, it defaults to `X`.
The variable is stored in the `AlgebraConfiguration` directory.

Spellings: `VX`, `RclVX`, `RCLVX`, `ⓧ`.

## ToPolynomial

Convert an algebraic object to a polynomial.

`X` ▶ `Poly`

* If `X` is already a polynomial, it is returned unchanged.
* If `X` is an array or list of coefficients in descending degree order, a
  univariate polynomial in the current `AlgebraVariable` is built (same layout
  as `PRoot` and `PCoef` in [CompatiblePolynomials](#compatiblepolynomials)
  mode).
* Otherwise, `X` is treated as an expression and expanded to a polynomial when
  possible.

```rpl
[ 1 2 -25 -26 120 ] ToPolynomial
@ Expecting x↑4+2·x↑3-25·x↑2-26·x+120
```

Use `ToArray` to recover the coefficient vector. See
[NewStylePolynomials](#newstylepolynomials) for the default `PCoef` result type.

## FromPolynomial

Convert a polynomial to an ordinary expression.

`Poly` ▶ `Expr`

Rewrites the polynomial using normal infix notation (sums and products of
powers). The variable names and term order follow the internal polynomial
representation. Use `ToPolynomial` for the inverse conversion.

## StoreAlgebraVariable

Store the current CAS algebra variable (HP-50G `STOVX`).

The `StoreAlgebraVariable` command sets the variable used for polynomial
evaluation and symbolic computations.
The variable must be a quoted symbol (e.g., `'X'`).
The variable is stored in the `AlgebraConfiguration` directory.

Spellings: `StoVX`, `STOVX`, `Storeⓧ`.

## Equation

Define an equation for use in solving or integration operations.
This is generally used as the name of a variable, and can manipulated using the `RcEQ` (recall equation) and `StEQ` (store equation) commands.


## StEq

Store expression in `Equation` variable.

The `StEq` command stores an expression, polynomial, or equation in the reserved
`Equation` variable.
The stored equation can be a single equation or a list of equations.
All stored equations must be of type expression, polynomial, or equation.
The stored equation is used by the solving menu and other equation-related
commands.

## RcEq

Recall expression from `Equation` variable.

The `RcEq` command recalls the currently stored equation from the `Equation`
variable.
If no equation is stored, it returns an error, `EQ variable not found`.
The recalled equation can be used for further manipulation or solving.

## NextEq

Cycle equations in the `Equation` variable if it is a list.

The `NextEq` command cycles through equations if the `Equation` variable
contains a list of equations.
It rotates the equations in the list, making the next equation the current one.
This is useful when working with multiple equations and wanting to solve them
one by one, see `MultipleEquationsSolver`.

## EvalEq

Evaluate the current equation.

The `EvalEq` command evaluates the currently stored equation.

For equations (expressions with `=`), it evaluates both sides and returns an
equation evaluating both sides, where the right-hand side may be a sum to adjust
it to the left-hand side:

```rpl
'A+1=B^2' STEQ
A=3 B=4 EVALEQ {} +
@ Expecting { '4=16-12' }
```

For other expressions, it evaluates the expression normally:

```rpl
'A+1-B^2' STEQ
A=3 B=4 EVALEQ {} +
@ Expecting { -12 }
```

## Where

Perform a substitution and evaluate the resulting expression.

The `Where` command performs symbolic substitution in expressions.
It takes an expression and a substitution rule (or list of rules) and applies
the substitution.
Substitutions can be expressed as equations or as a list .

In equation form, variables matching the left-hand side of the equation are
replaced by the expression on the rigth of the equation:

```rpl
'sin(Z)=2*(Z-1)^2' 'Z=3-A' Where {} +
@ Expecting { 'sin(3-A)=2·(3-A-1)↑2' }
```

Equations can be used in algebraic form and chained easily:

```rpl
'sin(Z)=2*(Z-A)^2|Z=3-A|A=B^3-1'
@ Expecting 'sin(3-(B↑3-1))=2·(3-(B↑3-1)-(B↑3-1))↑2'
```

In list form, variables given in odd positions in the list are replaced by the
expression given in the following even position in the list:

```rpl
'sin(Z)=2*(Z-1)^2' { Z '3-A' A 'B^3' } Where {} +
@ Expecting { 'sin(3-B↑3)=2·(3-B↑3-1)↑2' }
```
