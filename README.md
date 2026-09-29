# XeWe OS — ready-to-flash ESP32 firmware you drive from a command line

Personal project (XeWe Labs) · 2025-10-21 → ongoing · Solo: Max Dokukin · Status: Ongoing (release 2.0.0; firmware 2.0.x assembled from module repos)

![XeWe OS boot log on the serial monitor](static/media/resources/readme/boot_log.webp)

## Overview

XeWe OS is ready-to-flash firmware for ESP32 boards: a serial and web command line, WiFi, network
time, weekly schedules, button bindings and direct pin control. It is built on the
[XeWeOS framework](https://github.com/xewe-labs/xewe-library-os) and doubles as the reference
example of assembling a device from XeWe modules. The framework provides the module lifecycle,
serial console, NVS storage, command line and the `$system` module; this repository installs the
chosen modules into `src/modules/` with `setup.sh` and assembles everything in `xewe-os.ino`.
Modules are configured and controlled through the command line, and their settings persist in
ESP32 NVS. The project started in October 2025 as one monolithic sketch and was split in
September 2026 into libraries, one repository per module, a module registry and a shared build
toolchain.

## Highlights

- One firmware for three chips — ESP32-C3, ESP32-C6 and ESP32-S3 (`build/release_matrix.csv`)
- Six modules installed from the [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules) registry, with requirements resolved and declared in dependency order by `setup.sh` (`src/modules/Modules.h`, `modules.lock`)
- Runtime control through a text CLI over serial or HTTP (`$<group> <command> <args>`); every setting persists in NVS
- Releases 1.0.0 (2026-03-15) and 2.0.0 (2026-07-27) built for all three chips and published as merged images for the web flasher (`static/firmware/releases/`)
- Grew from 3,044 to 5,871 lines of in-repo C++ (2025-10 → 2026-09) before the framework and modules moved into their own repositories (`git log`)

## How it works

```
setup.sh ─┬─ registry (repositories.txt) → module.properties → resolve depends_modules → src/modules/<Folder>/ + Modules.h + modules.lock
          └─ xewe-os-build-toolchain → build/scripts, build/tools → build/scripts/<platform>/setup.sh (arduino-cli, ESP32 core, venv, libraries)
xewe-os.ino = ModuleController os + #include "src/modules/Modules.h" → build.sh → compile.sh (arduino-cli) → merged .bin → upload / release
```

- **Sketch** (`xewe-os.ino`) — declares one `xewe::os::ModuleController` with the project name, version, build timestamp and URL from `Config.h`, then includes the generated `Modules.h`; `setup()` calls `os.begin()`, `loop()` calls `os.loop()`. Without installed modules the build stops with an `#error` telling you to run `setup.sh`.
- **Module installer** (`setup.sh`) — reads the registry, shows a checklist (whiptail or a numbered menu), adds required modules, detects dependency cycles and duplicate slugs/folders, stages everything in a temp dir and swaps it into `src/modules/`, then installs the toolchain into `build/` and runs its setup.
- **Modules** (installed, not committed) — Wifi, WebInterface, Time, Scheduler, Buttons, Pins, each from its own `xewe-os-module-<slug>` repository.
- **Framework** — [xewe-library-os](https://github.com/xewe-labs/xewe-library-os) and the libraries it depends on, cloned into `build/libraries/` at the tags pinned in `build/required_libraries.txt`.
- **Toolchain** — [xewe-os-build-toolchain](https://github.com/xewe-labs/xewe-os-build-toolchain): build, upload, serial monitor, format and release scripts for macOS, Linux and Windows.

### Usage

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

`$help` lists every command of every installed module. For the commands and settings of one
module, see the README of its `xewe-os-module-<slug>` repository:

| Module | Prefix | Requires | Repository |
|---|---|---|---|
| Wifi | `$wifi` | — | [xewe-os-module-wifi](https://github.com/xewe-labs/xewe-os-module-wifi) |
| WebInterface | `$web_interface` | Wifi | [xewe-os-module-web-interface](https://github.com/xewe-labs/xewe-os-module-web-interface) |
| Time | `$time` | Wifi | [xewe-os-module-time](https://github.com/xewe-labs/xewe-os-module-time) |
| Scheduler | `$schedule` | Time | [xewe-os-module-scheduler](https://github.com/xewe-labs/xewe-os-module-scheduler) |
| Buttons | `$buttons` | — | [xewe-os-module-buttons](https://github.com/xewe-labs/xewe-os-module-buttons) |
| Pins | `$pins` | — | [xewe-os-module-pins](https://github.com/xewe-labs/xewe-os-module-pins) |

On first boot, modules that need it ask for their setup on the serial console (WiFi network,
timezone); later boots reuse what is stored in NVS.

### Configuration

* **Libraries:** `build/required_libraries.txt` lists the XeWe libraries (and
  ArduinoJson) with pinned release tags. Re-run `setup.sh` after changing it.

  ```text
  https://github.com/xewe-labs/xewe-library-utils --branch 0.1.0
  https://github.com/xewe-labs/xewe-library-os --branch 0.1.0
  ```

* **Modules:** choose them with `setup.sh`. `src/modules/` (installed modules, `Modules.h` and
  `modules.lock`) is generated and not committed.
* **Version:** `build/version_state` holds the version counter (`MAJOR`, `MINOR`, `PATCH`,
  `BUILD_ID`). `build.sh` writes the current version and a build timestamp into `Config.h`, and bumps
  `PATCH` and `BUILD_ID` after a successful upload (`-p`); `release.sh` asks for a release version
  (≥ the current one) and saves it there. The file is committed, so builds leave a change in it.
* **Board options:** `build/release_matrix.csv` has one row per chip (`CHIP`, optional `_BUILD_NOTES`);
  any other column becomes a `Config.h` define for that release build.
* **Debug output:** each module has a `DEBUG_<Module>` flag that defaults to `0` (e.g.
  `DEBUG_Wifi` in the Wifi module). Enable one from the build instead of editing the code, e.g.
  with `--build-property "compiler.cpp.extra_flags=-DDEBUG_Wifi=1"`.

### Project structure

```text
xewe-os/
├── xewe-os.ino                  ModuleController + #include "src/modules/Modules.h"
├── Config.h                     project name, version and build timestamp, written by build.sh
├── setup.sh                     chooses and installs modules and the build toolchain
├── src/modules/                 (installed) modules, Modules.h, modules.lock, .gitignore
├── build/
│   ├── required_libraries.txt   library repos and pinned tags, cloned by the build setup
│   ├── release_matrix.csv       board/config rows built by release.sh
│   ├── version_state            version counter
│   ├── scripts/  tools/         (installed) xewe-os-build-toolchain
│   └── .venv/ libraries/ builds/ build_config   (generated)
└── static/                      released firmware for the web flasher, README media
```

Everything marked installed or generated is created by `setup.sh` and the toolchain's setup, and
is not committed.

| Piece | Source |
|---|---|
| Module base, controller, `$system` | [xewe-library-os](https://github.com/xewe-labs/xewe-library-os) (`XeWeOS`) |
| Serial console, prompts, tables | [xewe-library-serial](https://github.com/xewe-labs/xewe-library-serial) (`XeWeSerial`) |
| NVS storage, FlexData | [xewe-library-nvs](https://github.com/xewe-labs/xewe-library-nvs) (`XeWeNvs`) |
| `$group command args` parser | [xewe-library-cli](https://github.com/xewe-labs/xewe-library-cli) (`XeWeCli`) |
| String, validation, timer, debug helpers | [xewe-library-utils](https://github.com/xewe-labs/xewe-library-utils) (`XeWeUtils`) |
| Wifi, WebInterface, Time, Scheduler, Buttons, Pins | `xewe-os-module-<slug>` repos listed in [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules) |
| Build, upload, release and format scripts | [xewe-os-build-toolchain](https://github.com/xewe-labs/xewe-os-build-toolchain) |

## Results

| Metric | Value | Baseline / note |
|---|---|---|
| Supported chips | ESP32-C3, ESP32-C6, ESP32-S3 | `build/release_matrix.csv` |
| Releases | 1.0.0 (2026-03-15), 2.0.0 (2026-07-27) | `static/firmware/releases/<version>/release_notes.txt` |
| Release images | 4,194,304-byte merged image per chip, flashed at 0x0 | `EraseFlash=all`, `FlashSize=4M`, `PartitionScheme=no_ota` |
| Release compile time | 48–58 s per chip (2.0.0) | 63–64 s for 1.0.0 (`meta.json`) |
| Modules | 6 (Wifi, WebInterface, Time, Scheduler, Buttons, Pins) | 18 module commands; framework adds `$system`, `$help` |
| In-repo C++ before the split | 5,871 lines in 33 files (2026-09-10) | 3,044 lines in 16 files (2025-10-23) |
| Commits | 254 on `main` | 2025-10-21 → 2026-09-15 |

The project is firmware rather than a model, so its results are delivery measures: chips
supported, releases built, modules available and how the code base was reorganised.

## Getting started

### Flash a prebuilt binary

1. Open `https://maxdokukin.com/projects/xewe-os`
2. Scroll to **Firmware Flasher**
3. Connect the board and follow the instructions

Prerequisites: an ESP32-C3, ESP32-C6 or ESP32-S3 board, a USB cable and a browser that supports
web flashing.

### Build from source

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
./setup.sh --toolchain-ref vX.Y.Z                 # a pinned toolchain version
./setup.sh --skip-build-setup                     # only install modules and toolchain
./setup.sh -h                                     # all options
```

`setup.sh` needs bash, git and curl (and `whiptail` for the checklist; otherwise a numbered
menu is shown). On Windows run it from Git Bash or WSL, then run
`build\scripts\windows\setup.ps1` in PowerShell. `GITHUB_TOKEN` is used when set, for reading
private module repositories.

Build with the toolchain scripts. `-c` selects the chip (`c3`, `c6`, `s3`); `-p <port>` also
uploads and opens the serial monitor (115200 baud).

```bash
build/scripts/mac/build.sh -c c3                           # macOS, compile only
build/scripts/mac/build.sh -c c3 -p /dev/cu.usbmodem1101   # compile + upload + monitor
build/scripts/linux/build.sh -c c3 -p /dev/ttyUSB0         # Linux
```

```powershell
build\scripts\windows\build.ps1 -c c3 -p COM5                # Windows
```

### Development

`xewe-os.ino` declares an `xewe::os::ModuleController`; `src/modules/Modules.h` declares the chosen
modules, each of which registers itself and begins in declaration order. Framework behaviour
lives in the XeWe libraries, module behaviour in the `xewe-os-module-*` repos.

* Change a module in its own repo (each has `scripts/validate.sh` to build it on its own), then
  re-run `setup.sh` here; `--modules-source` points it at local clones
* Change library versions in `build/required_libraries.txt`, then re-run `setup.sh`
* To write a module and add it to the registry, see the
  [xewe-os-modules README](https://github.com/xewe-labs/xewe-os-modules) and the
  [XeWeOS README](https://github.com/xewe-labs/xewe-library-os) for the module API
* Other toolchain scripts, next to `build.sh`: `upload.sh` (flash the latest build),
  `listen_serial.sh`, `format.sh` (clang-format plus the project's layout passes over `src/`) and
  `release.sh` (build every `release_matrix.csv` row, tag and publish)

There is no automated test suite; modules are checked by compiling their validation firmware for
all three chips. Note that the web interface executes any CLI command sent to `/cmd` from the local
network without authentication, and WiFi credentials are stored in NVS unencrypted.

## Documents

- [Release notes 2.0.0](static/firmware/releases/2.0.0/release_notes.txt) and [1.0.0](static/firmware/releases/1.0.0/release_notes.txt)
- [Released firmware](static/firmware/releases/) (merged images, `manifest.json` for the web flasher, `meta.json` build info)
- Screenshots: [boot log](static/media/resources/readme/boot_log.webp), [system status](static/media/resources/readme/system_status.webp), [module begin flow](static/media/resources/readme/begin_flow.webp)
- Organization overview: [github.com/xewe-labs](https://github.com/xewe-labs)
- License: Copyright (C) 2026 Maxim Dokukin. GNU General Public License version 3 — see [LICENSE.txt](LICENSE.txt).
