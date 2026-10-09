# xewe-os

Firmware template for ESP32-C3, -C6 and -S3: [XeWeCore](https://github.com/xewe-labs/xewe-os-core)'s
Os (serial console, CLI, NVS, system) plus the [modules](https://github.com/xewe-labs/xewe-os-modules)
you choose. The repository holds only its own code and `xewe.toml`; everything else is fetched at
the pinned refs into `build/`, except the toolchain, which is shared by every project on the machine
(`~/.xewe-os/build-tools/`). The layout is in [ARCHITECTURE.md](ARCHITECTURE.md#build-layout).

Why it is shaped this way: [ARCHITECTURE.md](ARCHITECTURE.md) covers the four repos and their
boundaries, the one `<XeWeCore.h>` include, the module contract, the tools, the version policy and
the decisions log.

## First five minutes

```sh
git clone https://github.com/xewe-labs/xewe-os my-fw && cd my-fw   # or "Use this template" on GitHub
./setup.sh                                  # no modules (valid); on a terminal it shows a menu
./setup.sh --modules wifi,web-interface     # or: all, none; dependencies are added for you
./run.sh --chip c3                          # build, flash, open the serial console (interactive: type a command, Enter; Ctrl-C exits)
build/tools/.venv/bin/python -m xewe build --all-chips   # c3, c6, s3
```

Needs Python >= 3.11 and git; no sudo. `./setup.sh` installs the xewe tools (venv in
`build/tools/`), XeWeCore and ArduinoJson into `build/`, generates the selected modules as an Arduino
library (with their tests) in `build/modules/` and writes `src/Modules.h`. arduino-cli, the esp32
core and the modules repo go to `~/.xewe-os/build-tools/` (`XEWE_HOME` overrides `~/.xewe-os`): the **first run
on a machine** downloads them once (~1.7 GB, ~8 GB on disk in total, ~4 min on a fast link); every later
project and re-run reuses them and takes seconds. Already have the core? `./setup.sh --arduino-data
DIR` (or `XEWE_ARDUINO_DATA=DIR`; `DIR` holds `packages/esp32/`). Nothing in `~/.arduino15` or
`~/Arduino` is read or written.

Change modules later with `./setup.sh --modules LIST` or `build/tools/.venv/bin/python -m xewe modules
select LIST|all|none`; `... modules list` shows what exists (`*` = selected). The menu only appears
when nothing is selected yet. The template ships no selection (`selected = []`): each project picks its
modules at first setup and commits its own `xewe.toml`. Zero modules is a valid firmware. Arduino libraries a module needs
come from the modules repo's `libraries.toml` catalogue; a `[libraries]` pin in your `xewe.toml` wins.

Run commands as `build/tools/.venv/bin/python -m xewe <command>` or `./run.sh`, not the bare `xewe`
script: after moving or renaming the project folder that script breaks (stale venv shebang); re-run
`./setup.sh` to fix it. `--help` lists everything (`build`, `flash`, `serial`, `test`, `doctor`, ...);
`-v` works after the subcommand too (`... -m xewe test -v`).

## No board? Still works

`./run.sh`, `build/tools/.venv/bin/python -m xewe flash` and `... -m xewe test` compile, then print
`compiled, not run: no board attached (c3, build/builds/c3/out/2.0.0-c3-xewe-os.bin)` and exit 0.
The test command runs unit tests and reports board tests as "compiled, not run".
Project tests go in `tests/board/` (pytest files that run on the ESP32 through `xewe test`) and
`tests/unit/` (developer machine: `@pytest.mark.unit` Python, or C++ built with g++);
`xewe test --unit-only` runs only the unit tests.
Set `XEWE_NO_BOARD=1` for compile-only sessions (CI, agents): no port is ever opened, even with a
board plugged in. Firmware lands in `build/builds/<chip>/out/`: `<version>-<chip>-xewe-os.bin` (flash at 0x0),
`manifest.json`, `meta.json` (size, flash %), `compile.log`.

## Committed vs generated

Committed: `xewe-os.ino`, `Config.h`, `xewe.toml`, `src/YourModule/`, `src/YourModuleFull/`, `setup.sh`, `run.sh`, docs and
`static/firmware/releases/` (written by `xewe release`). Generated, ignored, safe to delete:
`build/` and `src/Modules.h`. The shared toolchain in `~/.xewe-os/build-tools/` is outside the
project; deleting it only means the next setup downloads it again. No submodules.

## Version and settings

The firmware version is `[project] version` in `xewe.toml`; builds never change it. Your settings
and defaults go in `Config.h`. It includes `<XeWeBuildInfo.h>` (generated per build: name, version,
timestamp, chip, `--define` values). Never guard that include with `__has_include`: arduino-cli
then drops the generated library and the defaults win silently. In a plain Arduino IDE build,
delete that line. Override one value per build with `--define KEY=VALUE`; string values must
carry their own quotes:
`build/tools/.venv/bin/python -m xewe build --define 'PROJECT_URL="https://example.com"'`.

## Your own module

Two project-local examples, declared in `xewe-os.ino` after the generated modules. They are yours;
setup never touches them.

- `src/YourModule/` (`$your_module`): the smallest complete module (two commands, one NVS value, one
  setting). Copy this one for a simple module.
- `src/YourModuleFull/` (`$your_mod_full`; ids are NVS namespaces, at most 15 characters): the full
  tour. It overrides every `xewe::Module` hook (all four `begin_routines_*`, `loop`, `enable`,
  `disable`, `reset`, `status`) and shows a config struct, a bounded init-setup prompt, FlexData
  settings with a `schema` field (a foreign blob is never overwritten), an `AsyncTimer` in `loop`,
  `validate<>` for an int, a bool and a string, one command name with two arg counts,
  `os.report_error` and a listener the sketch sets. Copy it when you need those pieces, then delete
  what you do not use.

Rename the folder, the class and the id, or delete a folder and its lines in `xewe-os.ino`. Plain
code can also go in `setup()`/`loop()` of `xewe-os.ino`.

Flash it, open the console and try:

```
$help your_module           # its commands: set, show, status, reset, enable, disable
$your_module set 42         # validated (0-1000) and saved to NVS
$your_module show           # 42, also after $system restart
$system status              # your module has a row in the table
$your_module disable        # asks first; wipes its NVS, disables it and restarts
```

A reusable module belongs in the modules repo: see its
[README](https://github.com/xewe-labs/xewe-os-modules#adding-a-module) and
[CONTRACT.md](https://github.com/xewe-labs/xewe-os-modules/blob/main/CONTRACT.md). Never edit
`src/Modules.h` or `build/modules/` by hand; setup regenerates them.

## License

GPL-3.0-only (see `LICENSE.txt`).
