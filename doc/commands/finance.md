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




# Cash flow modeling

DB48x implements HP-17bii-style cash-flow modeling (CFLO). A cash-flow series is
an ordinary list where each element is either a number (a single flow) or a
two-element list `{ amount count }` describing `count` identical consecutive
flows (the HP "#TIMES" / `Nj` feature). The first flow is the period-zero flow
`CF0`, the next is `CF1`, and so on.

The active cash-flow series is stored in the reserved `CFData` variable, in the
same way statistics use the statistics data variable. The `CashFlowMenu` (also
reachable as `CFLO`) gives interactive access to the editing and computation
commands. The same commands can be used programmatically by leaving a list on
the stack.

All rates used by `NPV` and `IRR` are *periodic* percentages, exactly like the
HP 17bii. `NPV` results are rounded according to the `FinanceRounding` setting;
`IRR` is returned at full precision.


## NPV

Compute the net present value of a cash-flow series for a periodic interest
rate, given as a percentage:

`NPV = Σ CFt / (1 + I%/100)^t`, for `t` from `0` to the last period.

The interest rate is taken from the first level of the stack. The cash-flow
series is taken from the second level of the stack if it is a list, otherwise
from the active `CFData` variable.

```rpl
@ CF0=-100, CF1=60, CF2=60 at 10% per period
{ -100 60 60 } 10 NPV
@ Expecting 4.13
```


## IRR

Compute the internal rate of return of a cash-flow series, i.e. the periodic
interest rate (in percent) for which the net present value is zero.

The cash-flow series is taken from the first level of the stack if it is a list,
otherwise from the active `CFData` variable. The series must contain at least
one positive and one negative flow, otherwise a `No internal rate of return`
error is reported.

```rpl
@ CF0=-100, CF1=60, CF2=60
{ -100 60 60 } IRR
@ Expecting 13.0662
```


## CFData

The reserved variable holding the active cash-flow series. Evaluating `CFData`
recalls the current series onto the stack. It is also available as the `Data`
soft key of the `CashFlowMenu`.


## CFAdd

Append a single cash flow to the active series (`CF+` soft key).

```rpl
CFClear   -20000 CFAdd   5000 CFAdd
@ Active series is now { -20000 5000 }
```


## CFAddTimes

Append a repeated cash flow `{ amount count }` to the active series, where the
amount is taken from level 2 and the repetition count from level 1 (`CFxN` soft
key). This is the HP "#TIMES" / `Nj` feature.

```rpl
CFClear   -20000 CFAdd   9000 3 CFAddTimes
@ Active series is now { -20000 { 9000 3 } }
```


## CFDrop

Remove the last entry of the active series and return it on the stack (`CF-`
soft key).


## CFInsert

Insert a cash flow before a given 1-based index in the active series. The amount
is taken from level 2 and the index from level 1 (`CFINS` soft key).


## CFClear

Clear the active cash-flow series (`Clr` soft key).


## CFStore

Save the active cash-flow series under a name (`STO` soft key, also `CFName`).

```rpl
@ Save the current series as MYCF
'MYCF' CFStore
```


## CFRecall

Load a named series as the active cash-flow series (`RCL` soft key, also
`CFGet`).

```rpl
@ Make MYCF the active series
'MYCF' CFRecall
```


## CashFlowMenu

Display the cash-flow editor menu. It is also available as `CFLO`. The menu
gives access to `CFAdd`, `CFAddTimes`, `CFDrop`, `CFInsert`, `CFClear`,
`CFData`, `NPV`, `IRR`, `CFStore` and `CFRecall`.
