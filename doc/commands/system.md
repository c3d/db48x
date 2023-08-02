# Time, Alarms and System Commands

## SETDATE
Set current system date in MM.DDYYYY


## DATEADD
Add days to a date in MM.DDYYYY


## SETTIME
Set current time as HH.MMSS


## TOHMS
Convert decimal time to HH.MMSS


## FROMHMS
Convert time in HH.MMSS to decimal


## HMSADD
Add time in HH.MMSS format


## HMSSUB
Subtract time in HH.MMSS format


## TICKS
Return system clock in microseconds


## TEVAL
Perform EVAL and measure elapsed time


## DATE
Current system date as MM.DDYYYY


## DDAYS
Number of days between dates in MM.DDYYYY


## TIME
Current time in HH.MMSS


## TSTR


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


## AvailableMemory (MEM)

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
be portable), nor to ever match the value returned by the `TYPE` command on the
HP48.

*Note* The [TypeName](#typename) command returns the type as text, and
this is less likely to change from one release to the next.

## TypeName

Return the [type](#Type) of the object as text. For example, `12 type` returns
`"integer"`.


## PEEK
Low-level read memory address


## POKE
Low level write to memory address


## NEWOB
Make a new copy of the given object


## USBFWUPDATE


## PowerOff (OFF)

Turn calculator off programmatically


## SystemSetup

Display the built-in system setup


## SaveState

Save the machine's state to disk, using the current state if one was previously
loaded. This is intended to quickly save the state for example before a system
upgrade.


## Help

Access the built-in help in a contextual way. Bound to __XShift-+__

If the first level of the stack contains a text corresponding to a valid help
topic, this topic will be shown in the help viewer. Otherwise, a help topic
corresponding to the type of data in the stack will be selected.
