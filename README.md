# XeWe OS

XeWe OS is ready-to-flash firmware for ESP32 boards: a serial and web command line, WiFi, network
time, weekly schedules, button bindings and direct pin control. It is built on the
[XeWeOS framework](https://github.com/xewe-labs/xewe-library-os) and doubles as the reference
example of assembling a device from XeWe modules.

## Overview

The framework provides the module lifecycle, serial console, NVS storage, command line and the
`$system` module. This repository installs the chosen modules into `src/modules/` and assembles
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

   ```bash
   git clone https://github.com/xewe-labs/xewe-os
   cd xewe-os
   ./setup.sh
   ```

   `setup.sh` assembles the firmware:

   1. shows a checklist of the modules listed in the
      [xewe-os-modules registry](https://github.com/xewe-labs/xewe-os-modules); modules required
      by your choice are added automatically,
   2. installs each chosen module into `src/modules/<Module>/` (only its source, without git history)
      and generates `src/modules/Modules.h`, which declares them in dependency order, and
      `src/modules/modules.lock`, which records what was installed and from where,
   3. copies `scripts/` and `tools/code_formatter/` of the build toolchain
      ([xewe-os-build-toolchain](https://github.com/xewe-labs/xewe-os-build-toolchain)) into `build/`,
   4. runs the toolchain's `build/scripts/<mac|linux>/setup.sh` (arduino-cli, ESP32 core, Python
      venv, required libraries, `build_config`, `build/.gitignore`).

   Re-run it any time to change modules. Useful options:

   ```bash
   ./setup.sh --modules wifi,scheduler               # skip the checklist
   ./setup.sh --modules all
   ./setup.sh --modules-index ./repositories.txt      # a different registry list (file or URL)
   ./setup.sh --modules-source ~/code/modules        # skip the registry; use local xewe-os-module-* clones
   ./setup.sh --modules-ref 0.1.0                    # modules from a tag instead of main
   ./setup.sh --skip-build-setup                     # only install modules and toolchain
   ./setup.sh -h                                     # all options
   ```

   `setup.sh` needs bash, git and curl (and `whiptail` for the checklist; otherwise a numbered
   menu is shown). On Windows run it from Git Bash or WSL, then run
   `build\scripts\windows\setup.ps1` in PowerShell.

   Build with the toolchain scripts. `-c` selects the chip (`c3`, `c6`, `s3`); `-p <port>` also
   uploads and opens the serial monitor.

   ```bash
   build/scripts/mac/build.sh -c c3                           # macOS, compile only
   build/scripts/mac/build.sh -c c3 -p /dev/cu.usbmodem1101   # compile + upload + monitor
   build/scripts/linux/build.sh -c c3 -p /dev/ttyUSB0         # Linux
   ```

   ```powershell
   build\scripts\windows\build.ps1 -c c3 -p COM5                # Windows
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

* **Libraries:** `build/required_libraries.txt` lists the XeWe libraries (and
  ArduinoJson) with pinned release tags. Rerun the setup script after changing it.

  ```text
  https://github.com/xewe-labs/xewe-library-utils --branch 0.1.0
  https://github.com/xewe-labs/xewe-library-os --branch 0.1.0
  ```

* **Modules:** choose them with `setup.sh`. `src/modules/` (installed modules, `Modules.h` and
  `modules.lock`) is generated and not committed.
* **Version:** `build/version_state` holds the version counter (`MAJOR`, `MINOR`, `PATCH`,
  `BUILD_ID`). `build.sh` bumps `PATCH` and `BUILD_ID` on every build and writes the result into
  `Config.h`; `release.sh` reads it when picking a release version. It is committed, so each build
  leaves a change in it.

* **Debug output:** each module has a `DEBUG_<Module>` flag that defaults to `0` (e.g.
  `DEBUG_Wifi` in the Wifi module). Enable one from the build instead of editing the code, e.g.
  with `--build-property "compiler.cpp.extra_flags=-DDEBUG_Wifi=1"`.

## Project Structure

* `xewe-os.ino` - declares the framework's `ModuleController` and includes the generated `src/modules/Modules.h`
* `Config.h` - project name, version and build timestamp written by the build script
* `setup.sh` - chooses and installs modules and the build toolchain
* `src/modules/` - installed modules, `Modules.h` and `.gitignore` (generated by setup.sh)
* `build/` - `required_libraries.txt`, `release_matrix.csv` and `version_state` (committed); the
  toolchain's `scripts/` and `tools/` and the generated build state are installed there by setup.sh
  and ignored
* `static/` - released firmware for the web flasher, README media

Additional documentation:

* `doc/MODULES.md` - modules and commands
* `doc/ADDING_A_MODULE.md` - writing and registering a module
* `doc/PROJECT_STRUCTURE.md` - layout and where each piece comes from

## Development

`xewe-os.ino` declares an `xewe::os::ModuleController`; `src/modules/Modules.h` declares the chosen
modules, each of which registers itself and begins in declaration order. Framework behaviour
lives in the XeWe libraries, module behaviour in the `xewe-os-module-*` repos.

* Change a module in its own repo (each has `scripts/validate.sh` to build it on its own), then
  re-run `setup.sh` here; `--modules-source` points it at local clones
* Change library versions in `build/required_libraries.txt`
* See `doc/ADDING_A_MODULE.md` to create a module and add it to the registry, and the
  [XeWeOS README](https://github.com/xewe-labs/xewe-library-os) for the module API

## License

Copyright (C) 2026 Maxim Dokukin.

This project is licensed under the GNU General Public License
version 3. See [LICENSE.txt](LICENSE.txt) for the complete license text.