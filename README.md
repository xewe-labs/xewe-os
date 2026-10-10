# xewe-os

The firmware template of XeWe OS, for ESP32-C3, -C6 and -S3. You clone it and own the result: it
is [XeWeCore](https://github.com/xewe-labs/xewe-os-core)'s Os (serial console, `$group command`
CLI, NVS storage, system commands) plus the [modules](https://github.com/xewe-labs/xewe-os-modules)
you choose, plus your own code. The repository holds only your code and `xewe.toml`; everything
else is fetched into `build/` by `./setup.sh`, and the toolchain is shared by every project on the
machine.

## Where this fits: level 3

XeWeCore has three onboarding levels. Levels 1 and 2 need nothing but the Arduino IDE:

| Level | You use | Start with |
|---|---|---|
| 1 | Arduino IDE + Library Manager → `XeWeCore` | the console, NVS, prompts: core example [`01_Hello`](https://github.com/xewe-labs/xewe-os-core/tree/main/examples/01_Hello) |
| 2 | the same, plus one module of your own in the sketch folder | core example [`02_MyModule`](https://github.com/xewe-labs/xewe-os-core/tree/main/examples/02_MyModule) |
| 3 | **this template** and the `xewe` tools | ready-made modules (Wi-Fi, web interface, time, scheduler, …), multi-chip builds, board tests, releases |

The levels are described in the core's [documentation](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/README.md#three-levels).
Ready-made modules are level 3 only: the modules repository is not an Arduino library.

## First five minutes

```sh
git clone https://github.com/xewe-labs/xewe-os my-fw && cd my-fw   # or "Use this template" on GitHub
./setup.sh                                  # first setup: on a terminal it shows the module menu
./run.sh --chip c3                          # build, erase the flash, flash, open the console
```

Needs Python >= 3.11 and git; no sudo.

**Module selection.** The template ships with no modules selected (`[modules] selected = []` in
`xewe.toml`). The first `./setup.sh` on a terminal shows a numbered menu (an empty answer means
none); without a terminal it selects nothing and says so. You can also choose up front:

```sh
./setup.sh --modules wifi,web-interface     # or: all, none; dependencies are added for you
```

The choice is written to your `xewe.toml`, which your project commits. Zero modules is a valid
firmware. Change it later with `./setup.sh --modules LIST` or
`build/tools/.venv/bin/python -m xewe modules select LIST|all|none`; `... modules list` shows what
exists (`*` = selected). The menu appears only while nothing is selected.

**What setup installs.** The xewe tools (a venv in `build/tools/`), XeWeCore and the Arduino
libraries into `build/libraries/`, the selected modules as one generated Arduino library (with
their tests) in `build/modules/`, and `src/Modules.h`. arduino-cli, the esp32 core and the modules
repository go to `~/.xewe-os/build-tools/` (`XEWE_HOME` overrides `~/.xewe-os`). The **first run on
a machine** downloads them once (~1.7 GB, ~8 GB on disk, a few minutes on a fast link); every later
project and re-run reuses them and takes seconds. Already have the esp32 core? `./setup.sh
--arduino-data DIR` (or `XEWE_ARDUINO_DATA=DIR`; `DIR` holds `packages/esp32/`). Nothing in
`~/.arduino15` or `~/Arduino` is read or written.

**First boot.** `./run.sh` erases the whole flash before flashing, NVS included, so every run is a
true first boot. The board then asks, on the console:

1. `Name your device` (and `Confirm`);
2. for each module, in declaration order: `Would you like to enable <Name> module?` (for a module
   that can be disabled), then that module's own setup (Wi-Fi network and password, timezone, or
   the example module's `Starting level (0-100)?`);
3. `Initial Setup Complete`, then a restart into normal operation.

The name prompt waits for an answer; build with
`--define 'XEWE_DEVICE_NAME="Kitchen Lights"'` to skip it. Module prompts are bounded, so they time
out with their default when nobody answers. To keep the stored
name, Wi-Fi and module choices between runs, use `./run.sh --keep-nvs --chip c3`. `xewe provision`
answers the first-boot prompts for you (name, modules, Wi-Fi, timezone from flags, environment or
a dotenv file); see the [tools README](https://github.com/xewe-labs/xewe-os-tools#readme).

## Everyday commands

Run the tools as `build/tools/.venv/bin/python -m xewe <command>`, or `./run.sh`. Avoid the bare
`build/tools/.venv/bin/xewe` script: after the project folder is moved or renamed its shebang is
stale; re-run `./setup.sh` to fix it.

```sh
build/tools/.venv/bin/python -m xewe build --chip c3        # or --all-chips: c3, c6, s3
build/tools/.venv/bin/python -m xewe test --unit-only       # tests that need no board
build/tools/.venv/bin/python -m xewe serial                 # timestamped console
build/tools/.venv/bin/python -m xewe --help                 # everything: flash, provision, doctor, release, ...
```

`-v` works after the subcommand too (`... -m xewe test -v`).

## No board? Still works

`./run.sh`, `xewe flash` and `xewe test` compile, then print
`compiled, not run: no board attached (c3, build/builds/c3/out/2.0.0-c3-xewe-os.bin)` and exit 0.
`xewe test` runs the unit tests and reports board tests as "compiled, not run". Set
`XEWE_NO_BOARD=1` for compile-only sessions (CI, agents): no port is ever opened, even with a board
plugged in.

Firmware lands in `build/builds/<chip>/out/`: `<version>-<chip>-xewe-os.bin` (flash at 0x0),
`manifest.json`, `meta.json` (size, flash %), `compile.log`.

Project tests go in `tests/board/` (pytest files that run against the board through `xewe test`)
and `tests/unit/` (on the developer machine: `@pytest.mark.unit` Python, or C++ built with g++).

## Committed vs generated

| | Paths |
|---|---|
| Committed | `xewe-os.ino`, `Config.h`, `xewe.toml`, `src/YourModule/`, `src/YourModuleFull/`, `setup.sh`, `run.sh`, docs, `static/firmware/releases/` (written by `xewe release`) |
| Generated, ignored, safe to delete | `build/`, `src/Modules.h` (`./setup.sh` rebuilds both from `xewe.toml`) |
| Shared, outside the project | `~/.xewe-os/build-tools/`: deleting it only means the next setup downloads it again |

No submodules. The full `build/` tree is in [ARCHITECTURE.md](ARCHITECTURE.md#build-layout).

## Versions and settings

`xewe.toml` pins one ref each for XeWeCore, the modules and the tools, plus third-party libraries.
`ref = "latest"` follows the newest commit of a repository's default branch; this is how the
template ships until the ecosystem's `v3.0.0` tag set. A tag freezes a ref, and
`xewe manifest update` moves the refs to the newest tags.

The firmware version is `[project] version` in `xewe.toml`; builds never change it. Your settings
and defaults go in `Config.h`. It includes `<XeWeBuildInfo.h>`, generated per build (name,
version, timestamp, chip, `--define` values). Never guard that include with `__has_include`:
arduino-cli then drops the generated library and the defaults win silently. In a plain Arduino IDE
build, delete that line. Override one value per build with `--define KEY=VALUE`; string values
carry their own quotes:
`build/tools/.venv/bin/python -m xewe build --define 'PROJECT_URL="https://example.com"'`.

## Your own module

Two project-local example modules are declared in `xewe-os.ino`, after the generated ones. They
are yours; setup never touches them.

- **`src/YourModule/`** (`$your_module`): the smallest complete module. A two-row settings table
  (`number`, `beat_s`, which gives it `set`/`get`/`schema` and the status lines), one command of its
  own (`show`) and one compile-time setting. Copy this one for a simple module.
- **`src/YourModuleFull/`** (`$your_mod_full`; ids are NVS namespaces, at most 15 characters): the
  full tour. It overrides every `xewe::Module` hook (all four `begin_routines_*`, `loop`, `enable`,
  `disable`, `reset`, `status`) and shows a config struct, a bounded first-boot prompt, a settings
  table with a `SECRET` row (`token`, never printed) and a `RESTART` row (`pin`),
  `on_setting_changed`, a FlexData blob with a `schema` field checked with `has()` (a foreign blob
  is never overwritten) and reported through `schema_extra`, a GPIO claimed in the core pin
  registry, an `AsyncTimer` in `loop`, `validate<>`, one command name with two argument counts,
  `os.report_error` and a `xewe::ListenerSet` the sketch subscribes to. Copy what you need and delete
  the rest.

Rename the folder, the class and the id, or delete a folder and its lines in `xewe-os.ino`. Plain
code can also go in `setup()`/`loop()` of `xewe-os.ino`.

Flash it, open the console and try:

```
$help your_module             # its commands: set, get, schema, show, status, reset, enable, disable
$your_module set number 42    # validated (0-1000) and saved to NVS: number=42
$your_module show             # 42, also after $system restart (or: $your_module get number)
$your_module schema           # one JSON line per setting, then {"end":"your_module","count":2}
$your_mod_full set token abc  # token=******** (SECRET: never printed)
$system status                # your module has a row in the table
$your_module disable          # asks first; wipes its NVS, disables it and restarts
```

A reusable module belongs in the modules repository: see its
[README](https://github.com/xewe-labs/xewe-os-modules#readme) and
[CONTRACT.md](https://github.com/xewe-labs/xewe-os-modules/blob/main/CONTRACT.md). Never edit
`src/Modules.h` or `build/modules/` by hand; setup regenerates them.

## Documentation

| What | Where |
|---|---|
| Why the ecosystem is shaped this way, the decisions | [ARCHITECTURE.md](ARCHITECTURE.md) |
| Naming rules for every repository | [NAMING.md](NAMING.md), checked by the tools' `scripts/brand-lint.sh` |
| XeWeCore: Os, modules, settings, console, NVS | [xewe-os-core `doc/`](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/README.md) |
| The modules and how to write one | [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules#readme) |
| Every `xewe` command, `xewe.toml`, the build layout | [xewe-os-tools](https://github.com/xewe-labs/xewe-os-tools#readme) (`SPEC.md`) |
| Organization guidelines | [xewe-labs/.github](https://github.com/xewe-labs/.github) |
| Working in this repository as a coding agent | [`.agents/AGENTS.md`](.agents/AGENTS.md) |

## License

GPL-3.0-only (see `LICENSE.txt`).
