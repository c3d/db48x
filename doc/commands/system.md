# Time, Alarms and System Commands

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

Add days to a [date](#date). The date format is `YYYYMMDD`, with an optional fractional part defining the time, as in `YYYYMMDD.HHMMSS`, and an optional unit, as in `YYYMMDD_date`.


## SetTime

Set current time from a stack value `HH.MMSSCC`.

An HMS value can also be given, as returned by the `Time` command.


## ToHMS

Convert decimal time to `HH.MMSS` format.


## FomHMS

Convert time in `HH.MMSS` format to decimal time.


## HMS+

Add time in `HH.MMSS` format.


## HMS-

Subtract time in `HH.MMSS` format


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


## Time

Return the current system time as a unit object in the form `HH.MMSS_hms`.
This displays on the stack as `HH:MM:SS`.

## TSTR


## JulianDayNumber

Return the Julian day number for the given date and time

## ACK
Acknowledge oldest alarm (dismiss)


## ACKALL
Acknowledge (dismiss) all alarms


## RCLALARM
Recall specified alarm


## STOALARM
Create a new alarm


## DELALARM
Delete an existing alarm


## FINDALARM
Get first alarm due after the given time


## Version

Return DB48X version information as text.

 ▶ `"Version information"`


## FreeMemory

Return the number of bytes immediately available in memory, without performing a
cleanup of temporary values (garbage collection).

See also: [GarbageCollect](#GarbageCollect), [FreeMemory](#FreeMemory)


## AvailableMemory

Return the number of bytes available in memory.

*Remark*: The number returned is only a rough indicator of usable memory.
In particular, [recovery features](#LastThingsMenu) consume or release varying
amounts of memory with each operation.

Before it can assess the amount of memory available, `AvailableMemory` removes
objects in temporary memory that are no longer being used. Like on the HP48, you
can therfore use `MEM` `DROP` to force garbage collection. However, there is
also a dedicated command for that, [GarbageCollect](#GarbageCollect).

See also: [FreeMemory](#FreeMemory), [GarbageCollect](#GarbageCollect)


## GarbageCollect

Perform a clean-up of temporary objects and return number of bytes reclaimed.

In order to speed up normal operations, temporaries are only discarded when
necessary to make room. This clean-up process, also called *garbage collection*,
occurs automatically when memory is full. Since garbage collection can slow down
calculator operation at undesired times, you can force it to occur at a desired
time by executing [GarbageCollect](#GarbageCollect).

See also: [FreeMemory](#FreeMemory), [Purge](#Purge)

## GarbageCollectorStatistics

Return an array containing garbage collector statistics, including:

* The number of garbage collection cycles
* The total number of bytes collected
* The total time spent collecting garbage
* The number of bytes collected during the last collection cycle
* The duration of the last collection cycle
* The number of bytes cleared by temporaries cleaning


## RuntimeStatistics

Return an array containing runtime statistics, including:

* The time spent running (i.e. the calculator is in high-power state)
* The time spent sleeping (i.e. the calculator is in low-power state)
* The number of times the calculator entered high-power state

Note that the calculator tends to spend more time in active state when on USB
power, because of additional animations or more expensive graphical rendering.


## Bytes

Return the size of the object and a hash of its value. On classic RPL systems,
teh hash is a 5-nibbles CRC32. On DB48X, the hash is a based integer of the
current [wordsize](#stws) corresponding to the binary representation of the
object.

For example, the integer `7` hash will be in the form `#7xx`, where `7` is the
value of the integer, and `xx` represents the integer type, as returned by the
[Type](#type) command.

`X` ▶ `Hash` `Size`


## Type

Return the type of the object as a numerical value. The value is not guaranteed
to be portable across versions of DB48X (and pretty much is guarantteed to _not_
be portable at the current stage of development).

### HP-compatible types

If the `CompatibleTypes` setting is active, the returned value roughly matches
the value returned by the HP50G. It always returns `29` for arrays, not `3`
(real array) nor `4` (complex array). It returns `1` for both polar and
rectangular complex numbers, irrespective of their precision. 128-bit decimal
values return `21` (extended real), 32-bit and 64-bit return `0` (real number).
The separation between `18` (built-in function) and `19` (built-in command) may
not be accurate.

The values returned by `Type` in HP-compatible mode are as follows:

* 0: Real number
* 1: Complex number
* 2: Text (called *character string* in HP manuals)
* 3: Real array (DB48x returns 29)
* 4: Complex array (DB48x returns 29)
* 5: List
* 6: Global name
* 7: Local name
* 8: Program
* 9: Expression
* 10: Based integer (binary integer in HP manuals)
* 11: Graphic object
* 12: Tagged object
* 13: Unit object
* 14: Library reference (XLIB name)
* 15: Directory
* 16: Library (not on DB48x)
* 17: Backup object (not on DB48x)
* 18: Built-in function (DB48x returns 19)
* 19: Built-in command
* 20: System binary (not on DB48x)
* 21: Extended real (DB48x returns 0)
* 22: Extended complex (DB48x returns 1)
* 23: Linked array (not on DB48x)
* 24: Character (not on DB48x)
* 25: Code object (not on DB48x)
* 26: Library data (not on DB48x)
* 27: Mini font (dense font on DB48x)
* 28: Real integer
* 29: Symbolic vector/matrix (all arrays on DB48x)
* 30: Font (sparse fonts on DB48x)
* 31: Extended object (not on DB48x)


### DB48x detailed types

If the `DetailedTypes` setting is active, the return value is negative, and
matches the internal representation precisely. For example, distinct values will
be returned for fractions and expressions.

*Note* The [TypeName](#typename) command returns the type as text, and
this is less likely to change from one release to the next. DB48X-only code
should favor the use of `TypeName`, both for portability and readability.

The values returned by `Type` are negative, beginning at `-2`, in the order
shown in the table for `TypeName` below. Commands each have their individual
type number.  Returned values are not guaranteed from release to release, and
they may differ depending on build options or hardware platform (e.g. the values
on DM32 or DM42) may differ. The values for data types are not necessarily
contiguous either. For example, the `Type` for a tagged object may be `-1473` on
version 0.9.1, and may change over time (the reason being that rarely used types
have a two-byte type prefix).

Use `TypeName` for portability.

## TypeName

Return the [type](#Type) of the object as text. For example:

```rpl
12 typename`
@ Expecting "integer"
```

The values returned by `Type` in detailed mode are as follows, where the
spelling is what `TypeName` returns:

* directory
* text
* list
* program
* block (unquoted code block)
* locals (local variables structure)
* expression
* funcall (function call in expression, e.g. `F(1;2;3)`
* local (local variable name)
* symbol (global variable name)
* constant (reference to constant library, e.g. `ⒸNA`)
* equation (reference to equation library, e.g. `ⒺSimple Shear`)
* xlib (reference to library, e.g. `ⓁDedicace`)
* array
* menu
* unit (e.g. `1_km`)
* assignment (e.g. `X=3`)
* rectangular (complex numbers such as `2+3ⅈ`)
* polar (complex numbers such as `2∡30°`)
<!--- DM32 --->
* hex_integer (based number with enforced base 16)
* dec_integer (based number with enforced base 10)
* oct_integer (based number with enforced base 8)
* bin_integer (based number with enforced base 2)
<!--- !DM32 --->
* based_integer (based number with current base)
<!--- DM32 --->
* hex_bignum (large based number with enforced base 16)
* dec_bignum (large based number with enforced base 10)
* oct_bignum (large based number with enforced base 8)
* bin_bignum (large based number with enforced base 2)
<!--- !DM32 --->
* based_bignum (large based number with current base)
* bignum (large integer, typically more than 64 bits)
* neg_bignum (negative large integer)
* integer (typically for integers where value takes less than 64 bits)
* neg_integer (negative small integer)
* fraction
* neg_fraction
* big_fraction
* neg_big_fraction
* hwfloat (hardware-accelerated 32-bit binary floating point)
* hwdouble (hardware-accelerated 64-bit binary floating point)
* decimal (variable precision decimal)
* neg_decimal (variable precision negative decimal)
* comment
* grob (HP-compatible graphic object)
* bitmap (hardware-optimized bitmap)
* Drop (commands have their name as type name)
* font
* dense_font
* sparse_font
* dmcp_font
* tag
* polynomial
* standard_uncertainty
* relative_uncertainty


## PEEK
Low-level read memory address


## POKE
Low level write to memory address


## NEWOB
Make a new copy of the given object


## USBFWUPDATE


## PowerOff

Turn calculator off programmatically


## SystemSetup

Display the built-in system setup


## SaveState

Save the machine's state to disk, using the current state if one was previously
loaded. This is intended to quickly save the state for example before a system
upgrade.


## ScreenCapture

Capture the current state of the screen in a dated file stored on the flash storage under the `SCREENS/` directory. This is activated by *holding* 🟨 and _O_ simultaneously. Pressing the keys one after another activates the [DisplayMenu](#displaymenu).


## BatteryVoltage

Return the current battery voltage as a decimal value.

## USBPowered

Returns `True` if the calculator is connected to USB power.

Programmers can use this command in a long-running program to alter the
frequency of power-hungry operations such as displaying on the screen.

For example, the `CollatzConjecture` library program only displays the amount of
memory used when powered by USB:

```rpl
ⓁCollatzBenchmark
```

## LowBattery

Returns `True` if the calculator is running low on battery, which is defined as
having less than 1/4th of the charge between 3000 mV and the value defined in
`MinimumBatteryVoltage`.

Programmers can use this command in long-running programs to automatically pause
their programs in order to avoid draining the battery and losing memory.

## DMCPLowBattery

Returns `True` if the calculator is running low on battery according to the DMCP
`get_lowbat_state()` function. Experimentally, this function is not very
reliable in detecting low-battery conditions. Use `LowBattery` instead.

## MinimumBatteryVoltage

This setting defines the minimum battery voltage in millivolts where the
calculator will automatically switch off to preserve battery. The default
value is 2600mV, which appears to be safe even with no-brand batteries.

Experimentally, the DM42 can operate at much lower voltages than 2.4V, but some
operations become unreliable or even cause a reset. Notably, the calculator may
not be able to wake up without rebooting, losing user data in the process.

If the battery goes below `MinimumBatteryVoltage`, the calculator will
automatically switch off with a message on the screen requesting to connect to
USB power or to change the battery. Selecting a higher value than the
default can be used to have an early reminder that you need to purchase
replacement batteries.


## BatteryRefresh

This setting defines the refresh interval in milliseconds between checks or updates of the battery levels. The default is `5000` (5 seconds).

Note that explicitly calling `BatteryVoltage`, `USBPowered` or `LowBattery`
causes the corresponding values to be immediatley refreshed, but does not
necessarily cause the battery status on screen to update.


## DMCPDisplayRefresh

On hardware calculators, use the DMCP system background display refresh.
This is the default setting, and presumably should use less energy.


## SoftwareDisplayRefresh

On hardware calculator, use the software display refresh.
This should be used for debugging purpose only.
