# Building the DB48X project

The DB48X project can be built in two variants:

* A simulator that uses Qt to approximately simulate the DM42 platform. There is
  no guarantee that the DMCP functions will behave as they do on the real thing,
  but it seems to be sufficient for software development. The benefit of using
  the simulator is that it facilitates debugging. The simulator also contains a
  [test suite](https://www.youtube.com/watch?v=vT-I3UlROtA) that can be invoked
  by running the simulator with the `-T` option or hitting the `F12` key.

* A firmware for the DM42, which is designed to run on top of SwissMicro's DMCP
  platform, and takes advantage of it.

Each variant can be built in `debug` or `release` mode.


## Prerequisites

The project has the following pre-requisites:

* `make`, `bash`, `dd`, `sed`, `tac`, `printf` and `find`.

  * MacOS: `brew install coreutils`

* Firmware builds require the `arm-none-eabi-gcc` GNU toolchain, which can be
  downloaded [from the ARM site](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
  or installed directly on many platforms.

  * Fedora: `dnf install arm-none-eabi-gcc arm-none-eabi-gcc-cs-c++ arm-none-eabi-newlib`

  * MacOS: `brew install arm-none-eabi-gcc`

* Simulator builds require `g++` or `clang`, as well as Qt6 (Qt5 is likely to
  work as well).

  * Fedora: `dnf install qt-devel qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtmultimedia-devel`

  * MacOS: `brew install qt`

* The FreeType development libraries (we use that to build the DM48 fonts)

  * Fedora: `dnf install freetype-devel`

  * MacOS: `brew install freetype`



## Build

To build the simulator, simply use `make sim`.

To build the firmware, use `make release` or `make debug`. There is also a
macOS-specific target to directly copy on the DM42 filesystem, called
`make install`.

If the build complains about the QSPI contents having changed, which
happens frequently, you will need to re-do a clean build.

Note: QSPI is an area of flash memory in the DM42 which is presenty used to
store, among other things, some important numerical processing routines. In
theory, the QSPI has additional space that could be used by your program, but
the built-in QSPI loader in the DM42 only accepts QSPI contents that matches
the default DM42 QSPI image CRC. The DB48X generates a QSPI image that is
expected to be compatible with the DM42 original PGM, so that you can switch
back and forth between the two. However, the opposite is not true: DB48X
requies the extended QSPI.

In order to achieve that objective, DB48X has to force the CRC to match
the original CRC. This means that the CRC can no longer be used to check
a good match with the correct QSPI. In case of mismatch, you may observe
very strange results, including a firmware crash. _Always flash both the
QSPI and PGM files together_.


## Testing

There is a test suite integrated in the simulator. Run the simulator with the
`-T` option to test changes before submitting patches or pull requests.

You can run the test suite from the simulator using the `F12` key. There is
also a `current` test, which you can run with `F11`. When submitting patches,
ideally, the `current` test should test the feature you added.


## SDKdemo repository

This code is a distance descendant of SwissMicro's SDKDemo.
The latest version of SDKdemo is available on
[SwissMicro's GitHub account](https://github.com/swissmicros/SDKdemo).

The [db48x.md](help/db48x.md) help file can be copied to the DM42's `/HELP`
directory to act as the built-in help for the calculator. It is built
from individual files in the [doc](doc/) directory.
