# Using the simulator

The DB48X project comes with a simulator.
It brings the calculator functionality to your computer (Linux, MacOS, Windows + WSL + Fedora).

This document describes some of its additional functionality.
Building the simulator is described in [BUILD](BUILD.md).

Run the simulator with the command `<path>/sim/db48x [Option]* [argument]*`.
The simulator uses a few [Environment variables](#environment-variabels) and its behaviour can be further
influenced with [Options](#options) or/and [Arguments](#arguments).

The simulator uses the [recorder](https://github.com/c3d/recorder) subsystem to [trace](#tracing) what is happening under the hood. 


## Environment variabels

| Variable | Description |
| --- | --- |
| DB48X_TRACES | A value for the tracing by the recorder. Default = ".*(error\|warn(ing)?)s?" |
| DB48X_INSTALL | Setting this variable is equivalent to setting the -I option. |


## Options

The simulator can be started with the command line options listed in the table below.

| Option | Description |
| --- | --- |
| -d[[`<whitespace>`]`<integer>`] | key_delay_time = `<integer>`. Default = 0.|
| -i[[`<whitespace>`]`<integer>`] | image_wait_time = `<integer>`. Default - 500.|
| -k[[`<whitespace>`]`<argument>`] | load_saved_keymap(`<argument>`). |
| -m[[`<whitespace>`]`<integer>`] | memory_size = `<integer>`. |
| -n | Enable noisy testing.|
| -r[[`<whitespace>`]`<integer>`] | refresh_delay_time = `<integer>`. Default = 20. |
| -s[[`<whitespace>`]`<real>`] | userScaling = `<real>`. Scale the application window. | | |
| -t`<trace>` | Set tracing by the recorder to `<trace>`. |
| -w[[`<whitespace>`]`<integer>`] | default_wait_time = `<integer>`. Default = 1000. |
| -D[[`<whitespace>`]`<argument>`] | dump_on_fail = `<argument>`. Default = null. |
| -I | Initialise the user's environment. |
| -N | Disable beep.|
| -T | Enable running the test suite. Process -O option. |
| -O | Without an argument, does nothing. |
| -Oall | Set tracing by the recorder first to "est\_.\*=0" then to "est\_.\*". |
| -O`<test>` | Set tracing by the recorder first to "est\_.\*=0" then to "est\_`<test>`". |


## Arguments

With arguments you can pass initial settings or flags to the application.
An argument is specified as `<argument>` = `<value>`.
Values "yes" or "true" (case insensitive) are interpreted as true.
Values of the form `<positive integer>` will be interpreted as such.
A few examples:

- DecimalDot=false ShowMonthName=true
- AngleMode=1

Use the `Modes` command in the application to check the effect.


## Computer keyboard

You can click the calculator buttons with the mouse.
But there are keyboard shortcuts.
They are listed in the tabel below.

| Key(s) | DB48X button / Description |
| --- | --- |
| digit | _digit_ |
| . + - * / | _._ _+_ _-_ _×_ (multiply) _÷_ (divide) |
| Backspace | ← |
| Enter | _ENTER_ |
| | |
| Esc | _EXIT_ |
| F1 - F6 | _F1_ - _F6_. The soft-menu keys |
| Space | _=_ |
| | |
| Shift + Alpha | Uppercase Alpha |
| Alt + Alpha | Lowercase Alpha |
| | |
| Ctrl + C | Copy the top of the stack or edit line to the clipboard.  |
| Ctrl + V | Paste the clipboard to the edit line. |
| Up Down Left Right | ◀ ▶︎ ◀︎ ▶︎ |
| | |
| F7 | Run / Interrupt demo 1 |
| F8 | Run / Interrupt demo 2 |
| F9 | Run / Interrupt demo 3 |
| F10 | Load an other keyboard map. Cycle through db48x, legacy, 42style and true42. |
| F11 | Run / Interrupt the current test. |
| F12 | Run / Interrupt the test suite. |


Since many commands are tucked away in menus it can be convenient to just type the command,
using Shift / Alt + Alpha.
You then no longer have to first find and activate the right menu and menu row.


## Initialise the users environment

A set of directories (`/help, /library, /state, /config`) and the files in them 
is copied to `~/.local/share/DB48X/DB48X/`.
The simulator works with that directory.


## Run / interrupt the test suite

There is a comprehensive test suite integrated in the simulator.
If you make changes run the test suite to verify you did not break existing functionality.
If you add a feature, add tests for it.


## Tracing

The code of the calculator frequently calls the ```record(<name>, ...);``` procedure to record what is going on at that point.
If you run the simulator with the command ```<path>/sim/db48x -t<name>``` the recorder will print those recorded trace messages
(see also [Environment variables](#environment-variabels); [Options](#options)).
