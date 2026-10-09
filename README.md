# xewe-os

Firmware template for ESP32-C3, -C6 and -S3: [XeWeCore](https://github.com/xewe-labs/xewe-os-core)'s
Os (serial console, CLI, NVS, system) plus the [modules](https://github.com/xewe-labs/xewe-os-modules)
you choose. The repository holds only its own code and `xewe.lock`; everything else is fetched at
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
`build/tools/`), XeWeCore, ArduinoJson and the modules repo into `build/`, generates the selected
modules as an Arduino library in `build/modules-lib/` and writes `src/Modules.h`. arduino-cli and
the esp32 core go to `~/.xewe-os/build-tools/` (`XEWE_HOME` overrides `~/.xewe-os`): the **first run
on a machine** downloads them once (~1.7 GB, ~8 GB on disk in total, ~4 min on a fast link); every later
project and re-run reuses them and takes seconds. Already have the core? `./setup.sh --arduino-data
DIR` (or `XEWE_ARDUINO_DATA=DIR`; `DIR` holds `packages/esp32/`). Nothing in `~/.arduino15` or
`~/Arduino` is read or written.

Change modules later with `./setup.sh --modules LIST` or `build/tools/.venv/bin/python -m xewe modules
select LIST|all|none`; `... modules list` shows what exists (`*` = selected). The menu only appears
when nothing is selected yet. Zero modules is a valid firmware. Arduino libraries a module needs
come from the modules repo's `libraries.toml` catalogue; a `[libraries]` pin in your `xewe.lock` wins.

Run commands as `build/tools/.venv/bin/python -m xewe <command>` or `./run.sh`, not the bare `xewe`
script: after moving or renaming the project folder that script breaks (stale venv shebang); re-run
`./setup.sh` to fix it. `--help` lists everything (`build`, `flash`, `serial`, `test`, `doctor`, ...);
`-v` works after the subcommand too (`... -m xewe test -v`).

## No board? Still works

`./run.sh`, `build/tools/.venv/bin/python -m xewe flash` and `... -m xewe test` compile, then print
`compiled, not run: no board attached (c3, build/builds/c3/out/2.0.0-c3-xewe-os.bin)` and exit 0.
The test command runs host tests and reports hardware tests as "compiled, not run".
Set `XEWE_NO_BOARD=1` for compile-only sessions (CI, agents): no port is ever opened, even with a
board plugged in. Firmware lands in `build/builds/<chip>/out/`: `<version>-<chip>-xewe-os.bin` (flash at 0x0),
`manifest.json`, `meta.json` (size, flash %), `compile.log`.

## Committed vs generated

Committed: `xewe-os.ino`, `Config.h`, `xewe.lock`, `setup.sh`, `run.sh`, docs and
`static/firmware/releases/` (written by `xewe release`). Generated, ignored, safe to delete:
`build/` and `src/Modules.h`. The shared toolchain in `~/.xewe-os/build-tools/` is outside the
project; deleting it only means the next setup downloads it again. No submodules.

## Version and settings

The firmware version is `[project] version` in `xewe.lock`; builds never change it. Your settings
and defaults go in `Config.h`. It includes `<XeWeBuildInfo.h>` (generated per build: name, version,
timestamp, chip, `--define` values). Never guard that include with `__has_include`: arduino-cli
then drops the generated library and the defaults win silently. In a plain Arduino IDE build,
delete that line. Override one value per build with `--define KEY=VALUE`; string values must
carry their own quotes:
`build/tools/.venv/bin/python -m xewe build --define 'PROJECT_URL="https://example.com"'`.

## Your own module

Your code goes in `setup()`/`loop()` of `xewe-os.ino`. A reusable module belongs in the modules
repo: see its [README](https://github.com/xewe-labs/xewe-os-modules#adding-a-module) and
[CONTRACT.md](https://github.com/xewe-labs/xewe-os-modules/blob/main/CONTRACT.md). Never edit
`src/Modules.h` or `build/modules-lib/` by hand; setup regenerates them.

## License

GPL-3.0-only (see `LICENSE.txt`).
