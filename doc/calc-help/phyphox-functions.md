# Phyphox functions

## PhyphoxLibrary

The Phyphox section of the Function Library works on measurements recorded
by the sensors of a phone. The free application phyphox, of the RWTH Aachen
University, records the accelerometer, the gyroscope, the magnetometer, the
barometer, the microphone and others, and exports each recording as a CSV
file: one line of titles, then one line per sample, the time in seconds
followed by one column per axis of the sensor. The calculator reads such a
file as it is, and treats the recording as a series of repeated measurements:
the result comes with a standard uncertainty of type A, found from the
scatter of the data, which then combines with the uncertainties of the other
quantities of the experiment.

* PendulumG: the acceleration of gravity, from the swing of a pendulum
  whose bob is the phone.
* SensorPeriod: the period of any recorded oscillation, with its standard
  uncertainty.
* PendulumData: a real recording, to try the two others.

**The recording.** It is given either as an array of rows, time first, or as
the name of a CSV file, such as "Pendulum.csv". The file must be in the
directory named data of the calculator. On a phone, this directory is under
Android/data, in the files of the application, and is reached from a computer
through the USB cable. In phyphox, the menu Export Data writes the CSV file;
choose the comma as separator and the point as decimal mark.

**How long a recording.** A recording of 30 to 60 seconds is a good
choice: the uncertainty of a period falls quickly with the number of
oscillations, and the file stays quick to read. The experiment Pendulum of
phyphox only keeps the last 500 samples, about 10 seconds; the experiment
Gyroscope, among the raw sensors, keeps them all. The array must fit several
times in the free memory: with one megabyte free, as on a phone, that is a
recording of about three minutes at 50 samples per second; a calculator
holds much less.


## PendulumG

A laboratory exercise of mechanics that needs no other instrument than a phone
and a tape measure: the acceleration of gravity g, from a pendulum whose bob
is the phone itself. Hung from two strings, the phone records its own swing
with its gyroscope, which measures the rotation rate about its three axes,
in rad/s.

PendulumG takes the recording, as an array or as the name of a CSV file, then
the length L of the pendulum, from the pivot to the centre of the phone, with
its unit and its standard uncertainty. It returns three tagged results:

* Treal, the period of the swing, found by SensorPeriod from the zero
  crossings of the rotation rate, with its standard uncertainty of type A;
* θmax, the amplitude of the swing: for an amplitude θmax and a period T, the
  mean square of the rotation rate is θmax²·(2π/T)²/2, whatever the axes of
  the phone;
* g = 4·π²·L/T², where T = Treal/(1 + θmax²/16 + 11·θmax⁴/3072) is the
  period for small amplitudes, as in the equation Simple Pendulum of the
  Equation Library. The uncertainties of Treal and of L are combined.

The recording of PendulumData was made on 2026-11-23, with a pendulum of
length L = 56.0 cm, measured with a tape to about 1.0 cm.

**1)** The whole exercise: g is on the first level.

```rpl
ⓁPendulumData 56±σ1_cm ⓁPendulumG
@ Expecting g:9.90452 62239 8±σ0.17714 73988 21 m/s↑2
```

**2)** The period, with its standard uncertainty of type A, on the third level:

```rpl
ⓁPendulumData 56±σ1_cm ⓁPendulumG Drop2
@ Expecting Treal:1.49563 32968 2±σ7.52862 83614 5⁳⁻⁴ s
```

**3)** The amplitude of the swing, on the second level:

```rpl
ⓁPendulumData 56±σ1_cm ⓁPendulumG Drop
@ Expecting θmax:7.52247 59321 4 °
```

**4)** The result as it should be written, its uncertainty rounded to two
significant digits, by SciRngText:

```rpl
ⓁPendulumData 56±σ1_cm ⓁPendulumG DeleteTag ⓁSciRngText
@ Expecting "9.90±σ0.18 m/s↑2"
```

**5)** The same recording, had the length been measured to one millimetre:
the uncertainty of g is nine times smaller.

```rpl
ⓁPendulumData 56±σ0.1_cm ⓁPendulumG
@ Expecting g:9.90452 62239 8±σ2.03038 36270 8⁳⁻² m/s↑2
```

**6)** The other way round: the length that the period gives, for the
standard acceleration of gravity of the constants library, the amplitude
being left aside. It is known to half a millimetre, and lies within the
centimetre of the tape.

```rpl
ⓁPendulumData ⓁSensorPeriod → per « Ⓒg →Num per 2 ^ * 4 π →Num 2 ^ * / 1_cm Convert »
@ Expecting 55.56626 28747±σ0.05594 12181 46 cm
```

With a recording of your own, exported by phyphox and copied into the data
directory under the name Pendulum.csv:

```
"Pendulum.csv" 56±σ1_cm ⓁPendulumG
```

Three lessons. The tape leads the budget of uncertainty: its 1.8 % on L
give 1.8 % on g, against 0.1 % for the period. Measuring the period better
would change nothing; measuring the length to a millimetre leaves 0.2 %, and
then the length itself has to be defined that well: the phone is not a point,
and the distance that counts runs from the pivot to its centre of mass, plus
a few millimetres for its size. Then, the amplitude: a swing of 7.5° lengthens
the period by 0.11 %, twice its uncertainty; ignoring it would lower g by
0.2 %. At 20° the period would be 0.8 % longer: swing gently, or correct, as
PendulumG does. Last, the evaluation of type A: the 13 zero crossings of
these ten seconds give the period to 0.05 %. The uncertainty of the slope of
a line falls as the power 3/2 of the number of points: a recording four times
longer would give the period eight times better.

See also: [SensorPeriod](#sensorperiod), [PendulumData](#pendulumdata),
[Simple Pendulum](#Simple Pendulum), [SciRngText](#scirngtext).


## SensorPeriod

The period of an oscillation recorded by a sensor, with its standard
uncertainty of type A: a pendulum seen by the gyroscope, a mass on a spring
seen by the accelerometer, a magnet seen by the magnetometer.

SensorPeriod takes the recording, as an array of rows, the time in seconds
then one column per axis of the sensor, or as the name of a CSV file of that
form in the data directory. It returns the period as T±σu, in seconds.

The axis with the largest signal is kept. Its zero crossings are located by
linear interpolation between two samples, and confirmed once the signal has
left the band of ±0.3 times its rms value, so that noise near zero adds none.
Two straight lines of the same slope are then fitted to the times of the
crossings, one for the rising crossings and one for the falling ones. A
constant offset of the sensor, or a slow drift, moves the two lines apart and
leaves their slope, the period, unchanged. The standard uncertainty of the
period comes from the scatter of the crossings about these two lines, with
N − 3 degrees of freedom for N crossings. Four crossings at least are needed,
two periods: with fewer, the error is Too few oscillations.

**1)** The period of the recording of PendulumData:

```rpl
ⓁPendulumData ⓁSensorPeriod
@ Expecting 1.49563 32968 2±σ7.52862 83614 5⁳⁻⁴ s
```

**2)** A computed signal of period 1.5 s, sampled at 50 Hz for 6 s, with an
offset of a sixth of its amplitude: the period is found exactly, the offset
has no effect.

```rpl
0 299 For spn spn 0.02 * Dup 240 * 1_° * Sin 0.3 * 0.05 + Next { 300 2 } →Array
ⓁSensorPeriod
@ Expecting 1.5±σ2.53546 27641 9⁳⁻¹¹ s
```

**3)** The same signal with only one axis out of three in motion, as when the
phone is not aligned with the swing: the right axis is chosen.

```rpl
0 299 For spn spn 0.02 * Dup 240 * 1_° * Sin 0.3 * 0.05 + 0.01 Swap 0.02 Next { 300 4 } →Array
ⓁSensorPeriod
@ Expecting 1.5±σ2.53546 27641 9⁳⁻¹¹ s
```

See also: [PendulumG](#pendulumg), [PendulumData](#pendulumdata).


## PendulumData

A real recording, for PendulumG and SensorPeriod: the swing of a pendulum
whose bob is a phone, measured by its gyroscope with the experiment Pendulum
of phyphox, on 2026-11-23. It holds the last 500 samples of the recording,
9.5 s at 52.7 samples per second. Each row gives the time in seconds, from the
first sample, then the rotation rates about the x, y and z axes of the phone,
in rad/s, rounded to four decimals. The length of the pendulum, from the pivot
to the centre of the phone, was 56.0 cm, measured to about 1.0 cm.

**1)** The size of the recording, 500 rows of four numbers:

```rpl
ⓁPendulumData Size
@ Expecting { 500 4 }
```

**2)** Its last row:

```rpl
ⓁPendulumData 500 Get
@ Expecting [ 9.4716 0.0302 -0.1644 0.0372 ]
```

See also: [PendulumG](#pendulumg), [SensorPeriod](#sensorperiod).
