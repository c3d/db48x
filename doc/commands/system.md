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


## VERSION
Get newRPL version string


## MEM
Get available memory in bytes


## Bytes

Return the size of the object and a hash of its value. On classic RPL systems,
teh hash is a 5-nibbles CRC32. On DB48X, the hash is a based integer of the
current [wordsize](#stws) corresponding to the binary representation of the
object.

For example, the integer `7` hash will be in the form `#7xx`, where `7` is the
value of the integer, and `xx` represents the integer type.

`X` ▶ `Hash` `Size`


## PEEK
Low-level read memory address


## POKE
Low level write to memory address


## NEWOB
Make a new copy of the given object


## GARBAGE
Force a garbage collection


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
