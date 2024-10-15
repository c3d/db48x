# Performance measurements

This sections tracks some performance measurements across releases.

<!--- DMNONE --->
## NQueens comparison across models

The [NQueens benchmark][nqueens] has been used quite a bit to compare various
calcualtors. It is tracked across releases below, the code being executed being
the one [now in the demo library](../library/NQueens.48s) and the time being
best of 5 runs.

The calculators are also quite different in how they behave when on battery and
USB power:

| Configuration | Time     | Version |
|---------------|----------|---------|
| DM42 Battery  | 4539 ms  | 0.8.0   |
| DM42 USB      | 1247 ms  | 0.8.0   |
| DM32 Battery  | 1777 ms  | 0.8.0   |
| DM32 USB      |  444 ms  | 0.8.0   |
| iPhone 12     |   50 ms  | 0.8.0   |
| iPad 9e gen   |   60 ms  | 0.8.0   |
| Macbook Pro M1|   16 ms  | 0.8.0   |

[nqueens]: https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/articles.cgi?read=700)

## NQueens (DM42)

Performance recording for various releases on DM42 with `small` option (which is
the only one that fits all releases). This is for the same `NQueens` benchmark,
all times in milliseconds, best of 5 runs, on USB power, with presumably no GC.


| Version | Time    | PGM Size  | QSPI Size | Note                    |
|---------|---------|-----------|-----------|-------------------------|
| 0.8.0   | 1247    | 574012    |  267284   | Back to -Oz             |
| 0.7.0   | 1214    | 548204    |  223260   | Running at -O2          |
| 0.6.0   | 1183    | 409252    |  187516   | New table-free decimal  |
| 0.5.2   | 1310    | 711228    | 1548076   |                         |
| 0.5.1   |         |           |           |                         |
| 0.4.10+ | 1205    | 651108    |           | RPL stack runloop       |
| 0.4.10  | 1070    | 650116    |           | Focused optimizations   |
| 0.4.9+  | 1175    |           |           | Range-based type checks |
| 0.4.9+  | 1215    |           |           | Remove busy animation   |
| 0.4.9   | 1447    | 646028    | 1531868   | No LastArgs in progs    |
| 0.4.8   | 1401    | 633932    | 1531868   |                         |
| 0.4.7   | 1397    | 628188    | 1531868   |                         |
| 0.4.6   | 1380    | 629564    | 1531868   |                         |
| 0.4.5   | 1383    | 624572    | 1531868   |                         |
| 0.4.4   | 1377    | 624656    | 1531868   | Implements Undo/LastArg |
| 0.4.3S  | 1278    | 617300    | 1523164   | 0.4.3 build "small"     |
| 0.4.3   | 1049    | 717964    | 1524812   | Switch to -Os           |
| 0.4.2   | 1022    | 708756    | 1524284   |                         |
| 0.4.1   | 1024    | 687444    | 1522788   |                         |
| 0.4     |  998    | 656516    | 1521748   | Feature tests 7541edf   |
| 0.3.1   |  746    | 618884    | 1517620   | Faster busy 3f3ab4b     |
| 0.3     |  640    | 610820    | 1516900   | Busy anim 4ab3c97       |
| 0.2.4   |  522    | 597372    | 1514292   |                         |
| 0.2.3   |  526    | 594724    | 1514276   | Switching to -O2        |
| 0.2.2   |  723    | 540292    | 1512980   |                         |


## NQueens (DM32)

Performance recording for various releases on DM32 with `fast` build option.
This is for the same `NQueens` benchmark, all times in milliseconds,
best of 5 runs. There is no GC column, because it's harder to trigger given how
much more memory the calculator has. Also, experimentally, the numbers for the
USB and battery measurements are almost identical at the moment. As I understand
it, there are plans for a USB overclock like on the DM42, but at the moment it
is not there.


| Version | Time    | PGM Size  | QSPI Size | Note                    |
|---------|---------|-----------|-----------|-------------------------|
| 0.8.0   |  444    | 467260    |  187948   |                         |
| 0.7.0   |  449    | 611020    |  223692   | New DMCP runs at 160MHz |
| 0.6.0   | 1751    | 467260    |  187948   | New table-free decimal  |
| 0.5.2   | 1752    | 856228    | 1550436   |                         |
| 0.5.1   | 1746    |           |           |                         |
| 0.5.0   | 1723    |           |           |                         |
| 0.4.10+ | 1804    | 761252    |           | RPL stack runloop       |
| 0.4.10  | 1803    | 731052    |           | Focused optimizations   |
| 0.4.9   | 2156    | 772732    | 1534316   | No LastArg in progs     |
| 0.4.8   | 2201    | 749892    | 1534316   |                         |
| 0.4.7   | 2209    | 742868    | 1534316   |                         |
| 0.4.6   | 2204    | 743492    | 1534316   |                         |
| 0.4.5   | 2171    | 730092    | 1534316   |                         |
| 0.4.4   | 2170    | 730076    | 1534316   | Implements Undo/LastArg |
| 0.4.3   | 2081    | 718020    | 1527092   |                         |
| 0.4.2   | 2242    | 708756    | 1524284   |                         |
| 0.4.1   | 2152    | 687500    | 1522788   |                         |
| 0.4     |         |           |           | Feature tests 7541edf   |
| 0.3.1   |         |           |           |                         |
| 0.3     |         |           |           |                         |
| 0.2.4   |         |           |           |                         |
| 0.2.3   |         |           |           |                         |


## Collatz conjecture check

This test checks the tail recursion optimization in the RPL interpreter.
The code can be found in the `CBench` program in the `Demo.48S` state.
The HP48 cannot run the benchmark because it does not have integer arithmetic.

Timing on 0.4.10 are:

* HP50G: 397.438s
* DM32: 28.507s (14x faster)
* DM42: 15.769s (25x faster)

| Version | DM32 ms | DM42 ms | iPhone 12 | Notes                       |
|---------|---------|---------|-----------|-----------------------------|
| 0.7.0   | 25983   |  15061  |       287 | DM32 slower even at 160MHz  |
| 0.6.0   | 26256   |  15355  |           |                             |
| 0.5.2   | 26733   |  15695  |           |                             |
| 0.4.10  | 28507   |  15769  |           |                             |

In this benchmark, the DM32 displays a little more text than the DM42
(one additional character), but that does not seem sufficient to explain the
rather massive difference.

If all text drawing is removed, on 0.7.0, the DM32 runs in 182 ms and the DM42
in 476ms, which is more in line with expectations. It seems that text drawing
might be a little bit slower on the DM32, which may be an issue with the Flash
reading speed (drawing characters will read a lot of data from the flash)?


## SumTest (decimal performance)

VP = Variable Precision
ID = Intel Decimal Library
HW = Hardware-accelerated (`float` or `double` types)


### Variable Precision vs. Intel Decimal

For 100000 loops, we see that the variable-precision implementation at 24-digit
is roughly 10 times slower than the fixed precision implementation at 34 digits
(128 bits).

| Version      | DM32 ms | DM42 ms |
|--------------|---------|---------|
| 0.6.0 (VP24) | 2377390 | 1768510 |
| 0.5.2 (ID)   |  215421 |  143412 |


For 1000 loops, comparing variable-precision decimal with the earlier Intel
decimal

| Version      | DM32 ms | DM42 ms |
|--------------|---------|---------|
| 0.6.4 (VP24) |   32346 |   23011 |
| 0.6.4 (VP12) |   13720 |   10548 |
| 0.6.4 (VP6)  |    6905 |    5623 |
| 0.5.2 (ID)   |    2154 |    1434 |


### 1000 loops in various implementations

Time in millisecond for 1000 loops:

| DM32 Version | HW7  | HW16 |  VP6 | VP12  | VP24  | VP36  |
|--------------|------|------|------|-------|-------|-------|
| 0.6.4        | 1414 | 1719 | 6905 | 13720 | 32346 | 60259 |
| 0.6.2        |      |      | 7436 | 16017 | 34898 | 62012 |
| 0.6.0 (Note) |      |      |      |       | 23773 |       |
| 0.5.2 (ID)   | 2154 |      |      |       |       |       |

| DM42 Version |  HW7 | HW16 | VP6  | VP12  | VP24  |  VP36 |
|--------------|------|------|------|-------|-------|-------|
| 0.6.4        |  422 |  705 | 5623 | 10548 | 23811 | 42363 |
| 0.6.2        |      |      | 5842 | 10782 | 23714 | 42269 |
| 0.6.0 (Note) |      |      |      |       | 17685 |       |
| 0.5.2 (ID)   | 1434 |      |      |       |       |       |

Note: Results for 0.6.0 with variable precision are artificially good because
intermediate computations were not made with increased precision.


### 1M loops and iPhone results

1 million loops (tests performed with 0.7.1 while on battery):

| Version        | Time (ms) | Result                                      |
|----------------|-----------|---------------------------------------------|
| DM32 HW7       | 1748791   | 1'384'348.25                                |
| DM32 HW16      | 2188113   | 1'395'612.15872'53834'6                     |
| DM42 HW7       |  605102   | 1'384'348.25                                |
| DM42 HW16      |  806730   | 1'395'612.15872'53834'6                     |
| iPhone 12 VP48 | 1515800   | 1'395'609.35516'01878'23166'99926'37353'94572'23162'39251'9 |
| iPhone 12 VP36 | 1003884   | 1'395'607.17477'7643'16731'54275'27942'243  |
| iPhone 12 VP24 |  583237   | 1'395'600.94680'92149'84730'06              |
| iPhone 12 HW7  |    9462   | 1'384'348.25                                |
| iPhone 12 HW16 |    9495   | 1'395'612.15872'53836'9                     |

The iPhone 12 HW16 is the same as the Nintendo New 2DS XL


## Drawing `sin X` with `FunctionPlot`

| Configuration   | DM32 ms    | DM42 ms    |
|-----------------|------------|------------|
| HW7             |  1869-2000 | 1681-1744  |
| HW16            |  1928-2067 | 1679-2060  |
| ID              |  2332-5140 |            |
| VP24            |  3683-6005 | 3377-3511  |
| VP36            | 6567-10186 | 4434-4709  |
| VP48            | 8377-10259 | 5964-6123  |

Crash at precision 3


## Unit conversion benchmark

Units involve reading an external file, so this is a bit slow

```
«
  «
    0 25 for i
      i 1_m/s * 1_km/yr convert drop
    next
  »
  TEVAL
»
```

For 5 runs on USB power:

| Configuration          | DM32 ms    | DM42 ms    |   HP48S   |   HP50G   |
|------------------------|------------|------------|-----------|-----------|
| Units in file          |  1539-1616 | 9069-9238  |           |           |
| Units in memory        |   999-1047 | 3325-3503  | 6988-7816 | 2009-2012 |
| No autosimplify        |   688-723  | 2383-2585  |           |           |
| Commit no autosimplify |   691-722  | 2362-2568  |           |           |

<!--- !DMNONE --->
