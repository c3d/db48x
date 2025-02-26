# Finance

DB48x features a finance solver for basic compound interest computations
and amortization tables.


## FinanceSolverMenu

Display the Time Value of Money (TVM) solver menu.

This menu can be used to directly resolve payment and interests problems.
It works like the `SolvingMenu`, except that it gives no access to
equation-related features such as `NxEq` or `EvalEq`.

```rpl
TVM
```

## FinanceRounding

This setting defines the number of digits values returned by finance values will be rounded two. The default avalue is `2`.

```rpl
@ Set finance rounding to 10 digits
10 FinanceRounding

@ Reset finance rounding to default
'FinanceRounding' Purge

@ Check current value for finance rounding
'FinanceRounding' RCL
@ Expecting 2
```

## TVMRoot

Solves for the specified TVM variable using values from the remaining TVM
variables.

For example, to compute the interest rate for a 3-years €40000 loan with $1200
monthly payments at end of period, use the following code:

```rpl
@ Set loan conditions
PV=40000 Pmt=-1200 n=36 FV=0 Pyr=12 TVMEnd

@ Solve for interest rate
'I%Yr' TVMRoot

@ Expecting
```


## Amort

Amortizes a loan or investment based upon the current amortization settings.
Values must be stored in the TVM variables (`I%Yr`, `PV`, `Pmt`, and `PYr`). The
number of payments `n` is taken from the input together with flag –14
(`TVMPayAtBeginningOfPeriod` / `TVMPayAtEndOfPeriod`).

Given the number of payments `n`, the command deposits the value of the
principal, interest and balance in stack levels 1, 2 and 3.

For example, to compute the amount of principal, interest and balance 6 months
into a 10-years loan of $15000 at 10% yearly interest rates with monthly
payments at the end of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=10 PV=15000 PYr=12 FV=0 n=120 TVMEnd

@ First solve to get the payment value
'Pmt' TVMRoot

@ Compute amortization data after 6 months and put it in a vector
6 Amort →V3

@ Expecting [ -448.61 -740.74 14 551.39 ]
```


## AmortTable

Build an amortization table based on the current amortization settings.
Values must be stored in the TVM variables (`I%Yr`, `PV`, `Pmt`, and `PYr`). The
number of payments `n` is taken from the input together with flag –14
(`TVMPayAtBeginningOfPeriod` / `TVMPayAtEndOfPeriod`).

Given the number of payments `n`, the command deposits the amortization table
for the first `n` periods on the stack. If `n` is negative, then its value is
read from the `n` variable.

If the first level of the stack is a list, it can contain up to three values:

* The first period for which amortization is computed
* The number of amortization periods
* The step interval between amortization periods

All three values default to 1.

For example, to compute the amortization table of a home loan for $250,000 with
a downpayment of $62,500, an interest rate of 5.25% for 30 years, use the
following code:

```rpl
@ Set loan initial conditions
I%Yr=5.25 n='30*12' PYr=12 FV=0 PV='250000-62500'

@ Compute payment
'Pmt' TVMRoot

@ Compute amortization table
25 AmortTable
```


## TVMPayAtBeginningOfPeriod

This flag indicates that payments occur at the beginning of a payment period.
For compatibility with HP calculators, flag `-14` can also be set.

For example, to compute the amount of principal, interest and balance paid
at the end of a 2-years loan at 1.5% yearly interest rates with monthly payments of $200 at the beginning of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=1.5 Pmt=-200 PYr=12 FV=0 n=24 TVMBeg

@ First solve to get the payment value
'PV' TVMRoot

@ Compute amortization data after 24 months and put it in a vector
n Amort →V3

@ Expecting [ -4 731.71 -68.29 0. ]
```




## TVMPayAtEndOfPeriod

This flag indicates that payments occur at the end of a payment period.
For compatibility with HP calculators, flag `-14` can also be cleared.

For example, to compute the amount of principal, interest and balance paid at
the end of a 5-years loan at 2.5% yearly interest rates with monthly payments of
$500 at the beginning of each month, you can use the following code:

```rpl
@ Set loan conditions
I%Yr=1.5 Pmt=-200 PYr=12 FV=0 n=60 TVMEnd

@ First solve to get the payment value
'PV' TVMRoot

@ Compute amortization data after 60 months and put it in a vector
n Amort →V3

@ Expecting [ -11 554.09 -445.91 0. ]
```
