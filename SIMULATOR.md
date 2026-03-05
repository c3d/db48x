# Using the simulator

The DB48X project comes with a simulator.
It brings calculator functionality to your computer (Linux, macOS, Windows + WSL + Fedora).

This document describes some of its additional functionality.
Building the simulator is described in [BUILD](BUILD.md).

Run the simulator with the command `<path>/sim/db48x [Option]* [argument]*`.
The simulator uses a few [Environment variables](#environment-variables), and its behavior can be further
influenced with [Options](#options) and/or [Arguments](#arguments).

The simulator uses the [recorder](https://github.com/c3d/recorder) subsystem to [trace](#tracing) what is happening under the hood.


## Environment variables

| Variable | Description |
| --- | --- |
| DB48X_TRACES | A trace filter value for the recorder. Default = ".*(error\|warn(ing)?)s?" |
| DB48X_INSTALL | Setting this variable is equivalent to using the `-I` option. |


## Options

The simulator can be started with the command-line options listed in the table below.

| Option | Description |
| --- | --- |
| -d[[`<whitespace>`]`<integer>`] | Set key delay time in milliseconds to slow down tests. Default = 0 ms. |
| -i[[`<whitespace>`]`<integer>`] | Set the maximum time waiting for an image to match the expected image during testing. Default = 500 ms. |
| -k[[`<whitespace>`]`<argument>`] | Load a saved keymap to change keyboard layout. |
| -K | Simulate typing of individual keys during tests. |
| -m[[`<whitespace>`]`<integer>`] | Set the memory size available to the calculator in megabytes. |
| -n | Enable beeps while running the test suite (noisy testing). |
| -r[[`<whitespace>`]`<integer>`] | Set the time to wait for a screen update in milliseconds. Default = 20 ms. |
| -s[[`<whitespace>`]`<real>`] | Scale the application window by the given floating-point scaling factor. |
| -t`<trace>` | Enable the named recorder trace. Multiple traces can be enabled by using multiple `-t` options, or by using a regular expression as an argument to `-t`. |
| -w[[`<whitespace>`]`<integer>`] | Set the default time the test suite waits for a command to complete, in milliseconds. You can increase this to run the test suite on a very slow machine. The default = 1000 ms. |
| -D[[`<whitespace>`]`<argument>`] | Set the pattern of recorder traces to show when a test fails. |
| -I | Initialize the user's environment. WARNING: This may overwrite user configuration with defaults. |
| -N | Disable beeps. Use this if you get error messages about audio drivers. |
| -T[`<test>` \| all] | Run tests. Without a suffix, run the full test suite. With a suffix, select individual test suites. For example `-Tmatrices` will run the `matrices` portion of the test suite. |
| -O[`<test>` \| all] | Only configure test-suite recorder traces (`est_<test>` or `est_.*` for `all`) without enabling automatic test execution. |


## Arguments

Command-line arguments that do not represent simulator options set the initial value for an RPL setting or flag.

Settings are specified as `<argument>=<value>`.
Values "yes" or "true" (case insensitive) are interpreted as true.
Values of the form `<positive integer>` are interpreted as such.

A few examples:

* `DecimalDot=false ShowMonthName=true` will start with a decimal comma separator and show the month name in the header.
* `AngleMode=1` will start the simulator in Radians mode.

Use the `Modes` command in the application to check the effect.

Note that this only sets the _default_ settings for the simulator, which can then be overridden by any state file being loaded.


## Computer keyboard

You can click the calculator buttons with the mouse, as well as use the keyboard to control operations.

### Calculator keys

The majority of the keys map directly to calculator keys.

| Key(s) | DB48X button / Description |
| --- | --- |
| digit | _digit_ |
| . + - * / | _._ _+_ _-_ _×_ (multiply) _÷_ (divide) |
| Backspace | ← |
| Enter | _ENTER_ |
| Tab | Shift (yellow) key |
| Up Down Left Right | ◀ ▶︎ ◀︎ ▶︎ (the DM42(n) / DM32 only have up and down keys) |
| Esc | _EXIT_ |
| Space | _=_ (also serves as the space or `RUN` key depending on context) |
| F1-F6 | Simulates DM42(n) / DM32 softmenu keys from left to right |
| A-Z | Corresponding key on calculator, e.g. _J_ maps to `SIN` |

### Alpha modes

Alphabetic keys map directly to the corresponding calculator key. To cycle through alpha modes, use Tab + Enter in the same way you would use Shift + Enter on the physical device.

In addition, the simulator emulates _transient alpha_ mode using Shift or Alt:

| Key(s) | Description |
| --- | --- |
| Shift + Alpha | Uppercase Alpha |
| Alt + Alpha | Lowercase Alpha |

For example, you can quickly type `Hello` by holding Shift-H, followed by Alt-e-l-l-o. You can use this to enter names or commands.

### Simulator-specific commands

Some keyboard shortcuts are specific to the simulator.

| Key(s) | Description |
| --- | --- |
| Ctrl + C | Copy the top of the stack or edit line to the clipboard. |
| Ctrl + Shift + C | Copy the screen contents to the clipboard |
| Ctrl + V | Paste the clipboard to the edit line. |
| F8 | Save state  (`SaveState`) |
| F9 | Take a screenshot snapshot (saved in `screens/`) |
| F10 | Load another keyboard map. Cycle through db48x, legacy, 42style, and true42. |
| F11 | Run / Interrupt the current test. |
| F12 | Run / Interrupt the test suite. |
| F13 | Run / Interrupt demo 1 |
| F14 | Run / Interrupt demo 2 |
| F15 | Run / Interrupt demo 3 |
| F16 | Dump the flight recorder |


## Initialize the user's environment

A set of directories (`/help, /library, /state, /config`) and the files in them
are copied by the `-I` option to an application-specific location, which then holds the virtual filesystem for the calculator:

* `~/.local/share/DB48X/DB48X/` on Linux or Windows + WSL
* `./Library/Application Support/DB48X/DB48X` on macOS

The simulator works with the files in these locations.


## Run / interrupt the test suite

There is a comprehensive test suite integrated in the simulator, which you can run with the F12 key, or by launching the simulator with the `-Tall` option.

If you make changes, run the test suite to verify you did not break existing functionality. If you add a feature, add tests for it.


## Tracing

The calculator code frequently calls the `record(<name>, ...);` procedure to record what is happening at that point.

If you run the simulator with the command `<path>/sim/db48x -t<name>`, the recorder prints those trace messages (see also [Environment variables](#environment-variables) and [Options](#options)).

## Post-mortem flight recorder dump

In case of crash, a "recorder dump" will often provide useful information about what led to the crash, and should be attached to a bug report if it appears to contain useful information.

You can also trigger a recorder dump using the F16 key in the simulator. On macOS, you can artificially trigger a recorder dump at any time by hitting Control-T from the terminal window, and on Linux by sending `SIGUSR1`, e.g. using `pkill -USR1 db48x`.
