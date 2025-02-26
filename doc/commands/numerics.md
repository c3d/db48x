# Numerical integration

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

This setting only applies if the result is smaller than the display
settings. Like HP calculators, the display settings limits the precision
requested from the integration algorithm. For example, if the display is set to
`3 FIX`, then only 3 digits of precision are considered necessary in the result.

### IntegrationIterations

This setting limits the number of iterations for the integration algorithm. Each
iteration requires the evaluation of twice as many samples of the function to
integrate as the previous one, so the maximum number of samples taken is in the
order of `2^IntegrationIterations`.


# Numerical conversions

## →Num

Convert fractions and symbolic constants to decimal form.
For example, `1/4 →Num` results in `0.25`.

## →Q

Convert decimal values to fractions. For example `1.25 →Frac` gives `5/4`.
The precision of the conversion in digits is defined by
[→FracDigits](#ToFractionDigits), and the maximum number of iterations for the
conversion is defined by [→FracDigits](#ToFractionIterations)

## →Integer

Convert decimal values to integers. For example `1. →Integer` gives `1`.
This command intentionally fails with `Bad argument value` if the input contains
a non-zero fractional part.
