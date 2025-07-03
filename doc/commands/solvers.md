# Numeric solvers

## NUMINT
Numerical integration (adaptive Simpson)


## Root

Find the root of an equation or of a system of equations.

'Expr' 'Var' 'Guess' ▶ 'Solution'

The `Root` command takes an expression or an equation `Expr`, a variable `Var`
and an initial guess for the value `Guess`, and searches for a value of `Var`
that will zero the expression or verify the equation.

For example, to find the value of `x` where the value of `sin(x)` matches that
of `cos(x)/3` close to `1.5_°`, use the following:

```rpl
'sin(x)=cos(x)/3'
'x' 0.5_° ROOT
@ Expecting x=18.43494 88229 °
```

### Guess range

The `Guess` can be given as a list or vector containing two values, in which
case the algorithm will first evaluate the two given values. Otherwise, it will
first evaluate the guess value and a value close to it.


### Multiple variables

Unlike on HP calculators, the DB48x `Root` command can solve for multiple
variables, playing the role of the `Root` command, the _multiple equation
solver_ and the `MROOT` command. The `MROOT` command, which uses the `Eq`
variable, remains available for compatibility.

The `Root` command is adaptative. It will shift to the multiple equation solver
mode, solving one variable at a time, if that is possible

### Multiple equation solver

If equations can be solved one at a time, then the `Root` command will use the
multiple equation solver to solve them in turn.

For example, in the following code, the multiple equation solver can first solve
for `y` using the second equation, the solve for `x`. In both cases, it can use
the `Isolate` command to get an exact expression for the solution.

```rpl
{ 'sin(x)=y' '7*y^3=2' } { x y } { 0 0 } ROOT
@ Expecting { x=41.19575 8315 ° y=0.65863 37560 08 }
```

### Jacobian solver

If equations cannot be solved one at a time, then the `Root` command will
compute the Jacobian of the equations given as input, and use that to solve the
systerm iteratively. This is necessary when there is "crosstalk" between
variables across equations.

For example, to find the coordinates of the intersection between two circles,
you can use the following code:

```rpl
[ 'X^2+Y^2=1' '(X-1)^2+Y^2=1' ] [ X Y ] [ 0 0 ] ROOT
@ Expecting [ X=0.5 Y=0.86602 54037 84 ]
```


### Algebraic isolation

When possible, the numerical solver will attempt to use the `Isolate` command to
find results more rapidly and accurately using an exact symbolic expression of
the solution. This is controlled by the `SymbolicSolver` flag.


### Unit management

Specifying a unit for the initial value forces the calculator to compute the
result using the given unit.

In the example above, the guess was . Otherwise, the result would depend on the
angle mode, e.g. in `Radians` mode:

```rpl
RAD
'sin(x)=cos(x)/3'
'x' 0.5 ROOT
@ Expecting x=0.32175 05543 97
```

### Algebraic form

Unlike the HP version, `ROOT` can be used as an algebraic function,
using a syntax similar to what is used in algebraic mode on the HP50G.

The previous example can be written in algebraic form as follows:

```rpl
'ROOT(sin(x)=cos(x)/3;x;1.5_°)'
EVAL
@ Expecting x=18.43494 88229 °
```

### Solver (im)precision

The desired precision is given by the `SolverImprecision` setting relative to
the current `Precision` setting, allowing the solver to stop earlier. For
example, the default value `6` for `SolverImprecision`, combined with the
default 24-digit `Precision` means that the solver will seek at most 18-digit
precision (i.e. 24-6). Solving precision may be further reduced according to
display settings, like on HP calculators.

For example, the following will find a "solution" to `1/x=0` once it reaches the
desired precision:

```rpl
NumericalSolver
'ROOT(1/x;x;1)' EVAL
SymbolicSolver
@Expect: :x:1.10008 77783 66101 93099 87⁳18
```

### Updating global variables

Whether the solver found a solution or not, `Root` updates the value of the
target variable with the closest value it found so far. When no solution was
found, the value stored in the global variable is where the computed value was
the closest to zero, and is usually close to a local extremum.


### Finding complex solutions

If the guess value for the variable is a complex number, the DB48X version of
`ROOT` will explore the complex plane for solutions. This is an extension
compared to HP implementations of RPL.

For example, you can find a complex root for the following equation:
```rpl
'ROOT((X-5)²+3;X;0+0ⅈ)'
@ Expecting X=5+1.73205 08075 7ⅈ
```

### Differences with HP calculators

Since the root-finding algorithm is different in DB48X, there is no guarantee
that the numerical solver will find the same solution as HP calculators.

According to the HP50G Advanced Reference Manual, `ROOT` produces an error if it
cannot find a solution. However, experimentally, HP calculators will silently
return an extremum, whereas DB48X will indicate if it failed to find a
solution. For example, the following code returns a value close to 5 on HP48SX
or HP50G, but returns an error on DB48X, with the variable `X` containing a
value close to 5.

```rpl
@ Expecting "DB48X"
IFERR
 'sq(X-5)+3' 'X' 0 ROOT
THEN
 "DB48X"
ELSE
 "HP50G"
END
```

As an extension to the HP implementation, `ROOT` can solve systems of equations
and multiple variables by solving them one equation at a time, a programmatic version of what the HP50G Advanced Reference Manual calls the Multiple Equation Solver (`MINIT`, `MITM` and `MSOLVR` commands).

## SolvingMenuSolve

Solve the system of equations for the given variable.

## SolvingMenuRecall

Recall the current value of a variable in a system of equations. The value is returned as an assignment.

## MultipleEquationsSolver

Solve a system of multiple equations simultaneously.

## MultipleVariablesSolver

Solve for multiple variables in a system of equations.

## MSlv

On HP50G, a special command is dedicated to solving systems of equations.

On DB48x, the `MSlv` command is provided for comptability. It behaves almost
exactly like `Root`, except that it leaves the equations and variable lists on
the stack in addition to the result.

```rpl
RAD
[ 'sin(x)+y' 'x+sin(y)=1' ] [x y] [0 0] MSLV
"" + + +
@ Expecting "[ 'sin x+y' 'x+sin y=1' ][ x y ][ x=1.82384 11261 1 y=-0.96815 46361 75 ]"
```

Since `Root` on DB48x accepts a wider range of inputs and automatically detects
when it needs to solve [systems of simultaneous equations](#jacobian-solver),
`MSLV` will work on cases accepted on the HP50G, but will also accept other
inputs accepted by `Root`, e.g. single variables, and will benefit from symbolic
solving using `isol` (resulting in the generation of an angle units in the
example below):

```rpl
DEG
'sin(x)=0.3' x 0 MSLV
@ Expecting x=17.45760 31237 °
```
