# Time and Date Commands

## DateMenu

Show a softkey menu for date-related commands, including:

* `Date`
* `DateTime`
* `DateAdd`
* `DateSub`
* `SetDate`
* `JulianDayNumber`
* `DateFromJulianDayNumber`

## TimeMenu

Show a softkey menu for time-related commands:

* `Time`
* `ToHMS`
* `FromHMS`
* `HMSAdd`
* `HMSSub`
* `ChronoTime`
* `Ticks`
* `DateTime`
* `Wait`
* `TimedEval`
* `SetTime`


## Date format

The date format is `YYYYMMDD`, with an optional fractional part defining the
time, as in `YYYYMMDD.HHMMSS`.

Note: the date format is intentionally different from the format on the HP-48.

## Time format

The time format is `HH.MMSS` with optional hundredths of a second as
in `HH.MMSSCC`.


## SetDate

Set current system date.

The date format is `YYYYMMDD`, with an optional fractional part defining the
time, as in `YYYYMMDD.HHMMSS`. If the fractional part is zero, then the time is
not changed.

Note: the date format is intentionally different from the format on the HP-48.


## Date+

Add days to a [date](#date). The date format is `YYYYMMDD`, with an optional
fractional part defining the time, as in `YYYYMMDD.HHMMSS`, and an optional
unit, as in `YYYMMDD_date`. The unit enables special rendering as a date.

For example, to compute the date corresponding to 22222 days in the life of some
anonymous programmer, you can use the following code:

```rpl
19681205 22222 DATE+
@ Expecting Mon 8/Oct/2029
```

The opposite commands to compute the difference between dates is `DDays`.

The command accepts the inputs in any order. However, if both inputs can be
intepreted as dates, then the first one is a date and the second one a number of
days.

This command can be used in algebraic expressions:

```rpl
'DATEADD(19681205;22222)'
@ Expecting Mon 8/Oct/2029
```


## SetTime

Set current time from a stack value `HH.MMSSCC`.

An HMS value can also be given, as returned by the `Time` command.


## →HMS

Convert decimal time to `HH.MMSS` (Hours, Minutes, Seconds) format.

For example, twelve hours and a half are converted to twelve hours and thirty
minutes as follows:

```rpl
12.5 →HMS
@ Expecting 12:30:00
```

This command can be used in algebraic expressions:

```rpl
'→HMS(12.5)'
@ Expecting 12:30:00
```


## HMS→

Convert time in `HH.MMSS` (Hours, Minutes, Seconds) format to numerical time.

DB48x converts the time as a fraction to keep the result exact:

```rpl
12.30 HMS→
@ Expecting 12 ¹/₂
```

This command can be used in algebraic expressions:

```rpl
'HMS→(12.30)'
@ Expecting 12.5
```


## HMS+

Add time in `HH.MMSS` format.

```rpl
10.30 2.40 HMS+
@ Expecting 13:10:00
```

This command can be used in algebraic expressions:

```rpl
'HMS+(10.30;2.40)'
@ Expecting 13:10:00
```


## HMS-

Subtract time in `HH.MMSS` format

```rpl
10.30 2.40 HMS-
@ Expecting 7:50:00
```

This command can be used in algebraic expressions:

```rpl
'HMS-(10.30;2.40)'
@ Expecting 7:50:00
```

## →DMS

Convert value to DMS format.

The `→DMS` command converts a decimal value to DMS (Degrees, Minutes, Seconds)
format.
The input is interpreted as decimal degrees and converted to the DMS format.
The result is returned as a unit object with the `_dms` unit.

```rpl
12.5 →DMS
@ Expecting 12°30′00″
```

This command can be used in algebraic expressions:

```rpl
'→DMS(12.5)'
@ Expecting 12°30′00″
```

## DMS→

Convert value from DMS format.

The `DMS→` command converts a DMS (Degrees, Minutes, Seconds) value to
decimal format.
The input can be a unit object with the `_dms` unit.
The result is returned as a decimal value representing degrees.

```rpl
12.30 DMS→
@ Expecting 12 ¹/₂
```

This command can be used in algebraic expressions:

```rpl
'DMS→(12.30)'
@ Expecting 12.5
```

## DMS+

Add values in DMS format.

```rpl
12.30 10.40 DMS+
@ Expecting 23°10′00″
```

This command can be used in algebraic expressions:

```rpl
'DMS+(12.30;10.40)'
@ Expecting 23°10′00″
```

## DMS-

Subtract values in DMS format.

```rpl
12.30 10.40 DMS-
@ Expecting 1°50′00″
```

This command can be used in algebraic expressions:

```rpl
'DMS-(12.30;10.40)'
@ Expecting 1°50′00″
```

## Ticks

Return system clock in milliseconds


## TEVAL
Perform EVAL and measure elapsed time


## Date

Return the current system date as a unit object in the form `YYYYMMDD_date`.
This displays on the stack according to date format settings, in a way similar
to what is shown in the header, e.g. `23/Feb/2024` or `2024-02-23`.


## DateTime

Return the current system date as a unit object in the form `YYYYMMDD_date`.
This displays on the stack according to date format settings, in a way similar
to what is shown in the header, e.g. `23/Feb/2024` or `2024-02-23`.

## DDays

Number of days between dates.

To compute the number of days in 2024, use:
```rpl
20250101 20240101 DDAYS
@ Expecting 366 d
```

This command can be used in algebraic expressions:

```rpl
'DDAYS(20250101;20240101)'
@ Expecting 366 d
```

## Time

Return the current system time as a unit object in the form `HH.MMSS_hms`.
This displays on the stack as `HH:MM:SS`.


## JulianDayNumber

Return the Julian day number for the given date and time.

For dates the Gregorian calendar is assumed.
The Gregorian calendar jumps from 1582-10-04 to 1582-10-15.
This command ignores that gap, so the Julian day number given by this command for dates on or before 1582-10-14 may deviate from other converters like
[The NASA Julian Date/Time Converter](https://ssd.jpl.nasa.gov/tools/jdc).

To compute the Julian Day Number for the first day of the millenium:
```rpl
20000101 JDN
@ Expecting 2 451 545
```

This command can be used in algebraic expressions:

```rpl
'JDN(20000101)'
@ Expecting 2 451 545
```

## Datefromjuliandaynumber

Return the date for a given Julian day number.

This command converts a Julian day number to a date in the Gregorian calendar.
It is the opposite of the `JDN` command.

```rpl
2451545 JDN→
@ Expecting Sat 1/Jan/2000
```

This command can be used in algebraic expressions:

```rpl
'JDN→(2451545)'
@ Expecting Sat 1/Jan/2000
```
