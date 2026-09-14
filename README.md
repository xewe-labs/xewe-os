# XeWe OS

XeWe OS is ready-to-flash firmware for ESP32 boards: a serial and web command line, WiFi, network
time, weekly schedules, button bindings and direct pin control. It is built on the
[XeWeOS framework](https://github.com/xewe-labs/xewe-library-os) and doubles as the reference
example of assembling a device from XeWe modules.

## Overview

The framework provides the module lifecycle, serial console, NVS storage, command line and the
`$system` module. This repository adds the firmware's own modules in `src/` and assembles
everything in `xewe-os.ino`. Modules are configured and controlled through the command line, and
their settings persist in ESP32 NVS.

## Features

* Runtime control through a text-based CLI, over serial or HTTP
* WiFi connection management with network selection on first boot
* NTP time with timezone detection, and weekly command schedules
* Button bindings with software debouncing
* GPIO, ADC, PWM and I2C access without writing code
* Persistent settings in ESP32 NVS; modules can be enabled, disabled and reset individually

## Installation

### Prerequisites

* Supported hardware:

  * ESP32-C3
  * ESP32-C6
  * ESP32-S3
* Git
* A macOS, Linux, or Windows environment for the provided build scripts
* A compatible browser and USB connection for web flashing

### Setup

1. Flash a prebuilt binary from the web flasher:

   1. Open `https://maxdokukin.com/projects/xewe-os`
   2. Scroll to **Firmware Flasher**
   3. Connect the board and follow the instructions

2. Or build from source.

   Clone the repo:

   ```bash
   git clone https://github.com/xewe-labs/xewe-os
   cd xewe-os
   ```

   Build scripts are organized by platform under `build/scripts/<platform>/`. Run
   them from inside that platform folder. `-c` selects the target chip (`c3`, `c6`,
   or `s3`); pass `-p <port>` to also upload and open the serial monitor (omit it to
   compile only).

   **macOS** — `build/scripts/mac/`

   ```bash
   cd build/scripts/mac
   ./setup_build_environment.sh                    # one-time: installs arduino-cli, ESP32 core, venv, libraries

   ls /dev/cu.*                                    # find the port the ESP is connected to

   ./build.sh -c c3                                # compile only
   ./build.sh -c c3 -p /dev/cu.usbmodem11143201    # compile + upload + serial monitor
   ```

   **Linux** — `build/scripts/linux/`

   ```bash
   cd build/scripts/linux
   ./setup_build_environment.sh                    # one-time (apt-based installs)

   ls /dev/ttyUSB* /dev/ttyACM*                    # find the port

   ./build.sh -c c3
   ./build.sh -c c3 -p /dev/ttyUSB0
   ```

   **Windows** — `build/scripts/windows/` (PowerShell)

   ```powershell
   cd build\scripts\windows
   .\setup_build_environment.ps1                   # one-time (winget installs arduino-cli, Python, Git)

   arduino-cli board list                          # find the COM port (e.g. COM5)

   .\build.ps1 -c c3
   .\build.ps1 -c c3 -p COM5
   ```

## Usage

Use the CLI through the serial monitor or through other interfaces that send commands, such as the web interface.

Command syntax:

`$<cmd_group> <cmd_name> <param_0> <param_1> ...`

* All commands start with `$`
* Parameters are separated by spaces
* Use `$help` to list commands
* Use `$system` to list system commands

Examples:

```bash
# Get chip model and build info
$system info

# Toggle the built-in LED
$pins gpio_toggle 2

# Scan for available WiFi networks
$wifi scan

# Bind the BOOT button to toggle GPIO 8 on press
$buttons add 0 "$pins gpio_toggle 8" pullup on_press 50
```

For the full module and command reference, see `doc/MODULES.md`.

## Configuration

* **Libraries:** `build/libraries/required_libraries.txt` lists the XeWe libraries (and
  ArduinoJson) with pinned release tags. Rerun the setup script after changing it.

  ```text
  https://github.com/xewe-labs/xewe-library-utils --branch 0.1.0
  https://github.com/xewe-labs/xewe-library-os --branch 0.1.0
  ```

* **Modules:** add, remove or reorder modules in `xewe-os.ino`; a module must be declared after
  the modules it depends on.
* **Debug output:** each module has a `DEBUG_<Module>` flag that defaults to `0` (e.g.
  `DEBUG_Wifi` in `src/Wifi/Wifi.h`). Enable one from the build instead of editing the code, e.g.
  with `--build-property "compiler.cpp.extra_flags=-DDEBUG_Wifi=1"`.

## Project Structure

* `xewe-os.ino` - assembles the framework and the firmware modules
* `Config.h` - project name, version and build timestamp written by the build script
* `src/<Module>/` - firmware modules: Wifi, WebInterface, Time, Scheduler, Buttons, Pins
* `build/` - build scripts, required libraries, release matrix
* `static/` - released firmware for the web flasher, README media

Additional documentation:

* `doc/MODULES.md` - modules and commands
* `doc/ADDING_A_MODULE.md` - writing and registering a module
* `doc/PROJECT_STRUCTURE.md` - layout and where each piece comes from

## Development

`xewe-os.ino` declares an `xewe::os::ModuleController` and the modules; each module registers
itself and begins in declaration order. Module logic lives in `src/`, framework behaviour in the
XeWe libraries.

* Build and upload with the platform build script: `build/scripts/mac/build.sh`,
  `build/scripts/linux/build.sh`, or `build/scripts/windows/build.ps1`
* Change library versions in `build/libraries/required_libraries.txt`
* See `doc/ADDING_A_MODULE.md` to add a module, and the
  [XeWeOS README](https://github.com/xewe-labs/xewe-library-os) for the module API

## License

Copyright (C) 2026 Maxim Dokukin.

This project is licensed under the GNU General Public License
version 3. See [LICENSE.txt](LICENSE.txt) for the complete license text.