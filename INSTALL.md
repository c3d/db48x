# Installing DB48x

[![Install v0.4.8](http://img.youtube.com/vi/rVWy4N0lBOI/0.jpg)](https://youtu.be/rVWy4N0lBOI?si=lD244TAWJMUmo1Fz)

## DB48x vs DB50x

The DB48x project firmware exists in two versions, named DB48x and DB50x, that target two generations of SwissMicros calculators.

* DB48x can be installed on the SwissMicros [DM42](https://www.swissmicros.com/product/dm42).
* DB50x can be installed on the SwissMicros [DM32](https://www.swissmicros.com/product/model-dm32) and [DM42n](https://www.swissmicros.com/product/model-dm42n).

The two versions of the firmware are, functionally speaking, virtually identical as of this writing. The major difference is available memory, a little over 60K for DB48x on DM42, a little over 430K for DB50x on DM32 and DM42n. In the future, it is possible that some features will only exist on DB50x.


## Download the firmware

First download the installation files for the [latest release](https://github.com/c3d/db48x/releases) of the firmware from the [project's GitHub repository](https://github.com/c3d/db48x):

* For a DM42, download the [DB48x distribution file](https://github.com/c3d/db48x/releases/download/v0.9.12/db48x-v0.9.12.tgz) file.
* For a DM32 or DM42n, download the [DB50x distribution file](https://github.com/c3d/db48x/releases/download/v0.9.12/db50x-v0.9.12.tgz).

There are also source code archives for each release, which are only necessary to rebuild the firwmare.


## Connect the calculator to the computer

Use the appropriate USB cable to connect the calculator to the computer. The DM42 requires a micro-USB port, the DM32 and DM42n requires a USB-C cable.

Once you have connected the cable, enable the USB disk mode of the calculator. The method to do that depends on whether you are running the stock firmware of the calculator, or if you arlready installed another release of DB48x/DB50x.

### On stock DM42 / DM42n firmware

If you are using a DM42 or DM42n and have the stock firmware installed:

1. Select *Setup* (the yellow-shifted function of the _0_ key on the bottom row)
2. Select *5. System*
3. Select *2. Enter System Menu*
4. Select *4. Reset to DMCP Menu*
5. Select *6. Activate USB Disk*


### On stock DM32 firmware

If you are using a DM32 and have the stock firmware installed:

1. Select *Setup* (the yellow-shifted function of the _C_ key at the bottom left of the keyboard)
2. Select *3. System*
3. Select *2. Enter System Menu*
4. Select *4. Reset to DMCP Menu*
5. Select *6. Activate USB Disk*


### On DB48x or DB50x

If you are using another version of DB48x or DB50x:

1. Select *Setup* (the first-shifted command on the _0_ key on the bottom row)
2. Select *5. Activate USB disk*


## Locate the calculator storage volume

Once the calculator is connected over USB in disk mode, the internal storage appears on the computer as an MS-DOS FAT16 volume.

Depending on the operating system, this volume may show up in a variety of ways.  For example, on macOS, if the volume is called `DM42`, it will appears under `/Volumes/DM42`. On Linux, it may show up under a path like `/run/media/username/DM42`, or possibly under `/mnt`. On Windows, it will normally appear as a drive like `F:`. In the rest of these instructions, we will refer to the the location where the volume appears as `/path/to/calc`.

## Extract the distribution files

Extract the content of the distribution `.tgz` file into the `/path/to/calc` directory. From a terminal window, you can extract the file using the following command:

```
tar -C /path/to/calc -xvfz db48x-v0.9.8.tgz
```

You can also extract the files in a folder by double-clicking on the downloaded distribution file, and then copying all the files at the root of `/path/to/calc`. However, beware that I have occasionally observed calculator filesystem corruptions using this method on macOS, which typically manifest as `HELP/` turning into a file (instead of a directory) and various I/O errors. Should this happens, you will need to reformat the calculator volume.

### Files in the distribution

This puts the following files in the calculator volume:

* `db48x.pgm` (for DB48x) or `db50x.pg5` (for DB50x) contains the main program
* `db48x_qspi.bin` (for DB48x) or `db50x_qspi.bin` (for DB50x) contains additional data to be stored in the QSPI flash storage of the calculator
* `keymap.bin` contains the DB48x or DB50x keymap
* `config` contains configuration files
* `help` contains the built-in help files
* `library` contains the library definitions
* `state` contains demo state files

### Additional directories in the calculator volume

The calculator volume may also contain additional directories:

* `offimg` storing Windows bitmap files with extension `.BMP` that are shown by the calculator when you switch it off. Here is a [collection of such off-images](https://files.connorcode.com/Sites/offimg_gallery/index.html)
* `SCREENS` where the calculator records screen snapshots when the user presses _Shift_ and _Disp_ simultaneously.
* `DATA` contains DB48x or DB50x user data files accessed with `STO` or `RCL`

## Eject the calculator volume

Once the distribution files have been copied on the calculator volume, you can eject it from the computer. From the command line you can use:

* `eject /path/to/calc` (Linux)
* `hdiutil eject /path/to/calc` (macOS)


## Keymap installation

If you copied the files at the correct position, ejecting the calculator volume causes the calculator to immediately install the keymap for DB48x or DB50x.

Failure to install the keymap will simply cause a warning requiring confirmation every time you try to run the program, that the program is not targeted for this unit:

```
Currently loaded program is not
targeted for this unit. It may not
work properly. We recommend to
start USB disk mode or DMCP menu
to load another program
1. Start DB50X Ayway (not safe)
2. Activate USB Disk
3. DMCP Menu
```

If this happens, it simply means you did not copy the `keymap.bin` file at the root of the USB disk. Select `Activate USB Disk` and copy the file. When you eject the disk, you will see:

```
New KEYMAP installed
DB48x

Press any key to continue
```


## Load the QSPI file

The QSPI file can be loaded using the DMCP loader.

### On stock firmware (DMCP menu)

In the DMCP menu:

1. Select *4. Load QSPI from FAT*
2. Check that the calculator confirms the validity of the CRC (cyclic redundancy check) and then proceeds loading the QSPI file while showing a progress indicator.
3. When the message `Flashing OK` shows up, press any key to restart as requested by the calculator.

### On DB48x or DB50x

In the *Setup* menu:

1. Select *7. Load QSPI from FAt*
2. Check that the calculator confirms the validity of the CRC (cyclic redundancy check) and then proceeds loading the QSPI file while showing a progress indicator.
3. When the message `Flashing OK` shows up, press any key to restart as requested by the calculator.

### QSPI / Program mismatch

If you were running an earlier version of DB48x or DB50x, loading a new version of the QSPI may cause the program to display a warning indicating a mismatch btween the QSPI and the program.

```
Incompatible DB48x build ID
Please reload program and QSPI
from the same build
```

Simply hit the *ENTER* key, and proceed to the next step.

## Load the Program file

After you load the QSPI file, you need to load the program file. There are two ways to do that.

### From the DMCP System Menu

In the DMCP menu:

1. Select *3. Load Program*
2. Select either the ` DB48X.PGM` or `DB50X.PG5` file and hit _ENTER_
3. Check that the hash is OK and that the program loads correctly while displaying a progress indicator
4. After the message `Loading OK` shows up, hit a key to restart the calculator


### From the DB48x Setup Menu

In the Setup menu:

1. Select *6. Load Program*
2. Select either the ` DB48X.PGM` or `DB50X.PG5` file and hit _ENTER_
3. Check that the hash is OK and that the program loads correctly while displaying a progress indicator
4. After the message `Loading OK` shows up, hit a key to restart the calculator



## Optional: Load the Demo state file

In order to explore some of the capabilities of DB48x, you can load the `Demo.48s` state file.

1. If necessary, enter the DB48x Setup menu using the `Setup` command (_Shift_ then _0_  on the lower row)
2. Select *3. State*
3. Select *1. Load State*
4. Select *Demo.48S*
5. Hit the _ENTER_ key
