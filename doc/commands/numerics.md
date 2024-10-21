# Numerical functions

## Integrate

Perform a numerical integration of a function for a specified variable on a
numerical interval. For example `2 3 'X*(X-3)' 'X' Integrate` returns `-7/6`.

The function takes four arguments:

* The lower bound of the integration range
* The higher bound of the integration range
* The program or expression to evaluate
* The integration variable

### IntegrationImprecision

This setting defines the relative imprecision for the result with respect to the
[Precision](#precision) setting. The default value is `6`, which means that at
the default precision of `24` digits, `Integrate` will try to compute to an
accuracy of 18 digits.

### IntegrationIterations

This setting limits the number of iterations for the integration algorithm. Each
iteration requires the evaluation of twice as many samples of the function to
integrate as the previous one, so the maximum number of samples taken is in the
order of `2^IntegrationIterations`.


## Root

Root-finder command. Returns a real number that is a value of the specified
variable for which the specified program or algebraic object most nearly
evaluates to zero or a local extremum. For example, `'X^2=3' 'X' 0` returns
`X:1.732050807568877293527446341953458`.

The function takes three arguments:

* The program or expression to evaluate
* The variable to solve for
* An initial guess, or a list containing an upper and lower guess.
