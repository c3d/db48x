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
| 0.8.4   | 1183    | 601396    |  271220   | Temporaries cleaner     |
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
| 0.8.4   |  446    | 853020    |  274964   | Temporaries cleaner     |
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
| 0.8.4   | 25203   |  15616  |           | Temporaries cleaner         |
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

| Version      | DM32 ms | DM42 ms | Notes                                     |
|--------------|---------|---------|-------------------------------------------|
| 0.8.4 (VP24) |  496304 | 1427379 | With temporaries cleaner and DM32 @ 160MHz|
| 0.6.0 (VP24) | 2377390 | 1768510 |                                           |
| 0.5.2 (ID)   |  215421 |  143412 |                                           |


For 1000 loops, comparing variable-precision decimal with the earlier Intel
decimal

| Version      | DM32 ms | DM42 ms |
|--------------|---------|---------|
| 0.8.4 (VP24) |   32346 |   23011 |
| 0.8.4 (VP12) |   13720 |   10548 |
| 0.6.4 (VP24) |   32346 |   23011 |
| 0.6.4 (VP12) |   13720 |   10548 |
| 0.6.4 (VP6)  |    6905 |    5623 |
| 0.5.2 (ID)   |    2154 |    1434 |


### 1000 loops in various implementations

Time in millisecond for 1000 loops:

| DM32 Version | HW7  | HW16 |  VP6 | VP12  | VP24  | VP36  |
|--------------|------|------|------|-------|-------|-------|
| 0.8.7 @160MHz|  335 |  435 | 1610 |  2665 |  5703 | 10362 |
| 0.8.4 @160MHz|  401 |  482 | 1339 |  2299 |  5226 |  9655 |
| 0.6.4        | 1414 | 1719 | 6905 | 13720 | 32346 | 60259 |
| 0.6.2        |      |      | 7436 | 16017 | 34898 | 62012 |
| 0.6.0 (Note) |      |      |      |       | 23773 |       |
| 0.5.2 (ID)   | 2154 |      |      |       |       |       |

| DM42 Version |  HW7 | HW16 | VP6  | VP12  | VP24  |  VP36 |
|--------------|------|------|------|-------|-------|-------|
| 0.8.4        |  388 |  879 | 3762 |  6800 | 15164 | 27641 |
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


## Garbage collector performance

This is intended to compare the results before and after implementing the
`cleaner` class.

* IS: Initial State, just loading `Demos` file and displaying initial text
* L: Sum test Test with loop
* S: Sum test with "sigma" function
* N: NQueens benchmark
* K: Kinetic energy
* C: Collatz Benchmark
* R: Romberg plot
* T: Text drawing example

#### DM42 (USB)

Before `cleaner`:

| GC metric     |DM42 I|DM42 L |DM42 F |DM42 N|DM42 K|DM42 C |DM42 R |DM42 T |
|---------------|------|-------|-------|------|------|-------|-------|-------|
| Cycles        |     2|     44|     43|     1|     3|   1349|    119|     30|
| Purged        | 40621|2599816|2553096| 57541|172967| 241872|6874889|1774830|
| Duration      |    33|   3345|   3236|    48|   350|    659|  13579|   5086|
| Last purged   | 19192|  59224|  59521| 57541| 57753|    101|  57741|  59238|
| Last duration |     7|     81|     64|    48|   137|      0|    114|     99|
| Test duration |      |  18834|  18394|  1316|      |  15618|       |  96894|

After `cleaner`:

| GC metric     |DM42 I|DM42 L |DM42 F |DM42 N|DM42 K|DM42 C |DM42 R |DM42 T |
|---------------|------|-------|-------|------|------|-------|-------|-------|
| Cycles        |     2|      2|      3|     1|     2|   1350|     34|     11|
| Purged        | 40621| 115942| 174079| 59553|117515| 246941|1962231| 637293|
| Duration      |    33|     59|    148|   158|   247|    760|   5536|   2185|
| Last purged   | 19192|  59396|  58265| 59553| 59355|     98|  57686|  59167|
| Last duration |     7|     53|     78|   158|   148|      0|    168|    157|
| Cleared       |     7|2382261|2381143|     0|   140|  30261|4488382|1140338|
| Test duration |      |  15098|  15319|  1320|  2935|  15616|       |  93948|

After adding `BusyIndicatorRefresh`:

| `BusyIndicatorRefresh` |  L  |  F  |  N  |
|------------------------|-----|-----|-----|
|                   50ms |15720|15753| 1239|
|                  100ms |15435|15550| 1211|
|                  500ms |15220|15397| 1188|
|                 2500ms |15170|15162| 1183|


#### DM32 (USB)

Before `cleaner`:

| GC metric     |DM32 I|DM32 L |DM32 F |DM32 N|DM32 K|DM32 C |DM32 R |DM32 T|
|---------------|------|-------|-------|------|------|-------|-------|------|
| Cycles        |     0|      6|      6|     0|     0|   1349|     15|     1|
| Purged        |     0|2530293|2618873|     0|     0| 483501|6571504| 57541|
| Duration      |     0|   1462|   1378|     0|     0|    600|   6455|    48|
| Last purged   |     0| 421768| 439698|     0|     0|    102| 438050| 57541|
| Last duration |     0|    248|    295|     0|     0|      0|    428|    48|
| Test duration |     0|   6716|   6660|     0|      |  25391|       |  1316|

After `cleaner`:

| GC metric     |DM32 I|DM32 L |DM32 F |DM32 N|DM32 K|DM32 C |DM32 R |DM32 T |
|---------------|------|-------|-------|------|------|-------|-------|-------|
| Cycles        |     0|      0|      0|     0|     0|   1350|      4|      1|
| Purged        |     0|      0|      0|     0|     0| 382759|1748084| 438443|
| Duration      |     0|      0|      0|     0|     0|    461|   2400|    644|
| Last purged   |     0|      0|      0|     0|     0|     99| 438997| 438443|
| Last duration |     0|      0|      0|     0|     0|      0|    606|    644|
| Clearer       |     0|2383434|2383489|     0|  5145|  30261|4522881|1147855|
| Test duration |     0|   5293|   5401|   465|  2433|  25203|       | 141828|

| `BusyIndicatorRefresh` |  L  |  F  |  N  |
|------------------------|-----|-----|-----|
|                   50ms | 5687| 5697|  495|
|                  100ms | 5464| 5544|  466|
|                  500ms | 5259| 5357|  447|
|                 2500ms | 5218| 5318|  446|


<!--- !DMNONE --->
