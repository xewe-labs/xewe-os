# xewe-os

The firmware template of XeWe OS, for ESP32-C3, -C6 and -S3. You clone it and own the result: it
is [XeWeCore](https://github.com/xewe-labs/xewe-os-core)'s Os (serial console, `$group command`
CLI, NVS storage, system commands) plus the [modules](https://github.com/xewe-labs/xewe-os-modules)
you choose, plus your own code. The repository holds only your code and `xewe.toml`; everything
else is fetched into `build/` by `./setup.sh`, and the toolchain is shared by every project on the
machine.

## What is XeWe's, and what is borrowed

Most of the plumbing in this template exists elsewhere in the 2026 ESP32 ecosystem: arduino-cli
pins versions and flashes without touching NVS, arduino-cli and ESP-IDF ship serial monitors,
Espressif's pytest-embedded flashes and talks to boards from pytest, and WiFiManager,
MycilaESPConnect and ESP32-sveltekit already do Wi-Fi portals, web UIs and NTP. XeWe OS does not
compete there. The Wi-Fi, web and time modules are thin adapters over such libraries, and the
`xewe` tools wrap arduino-cli, esptool and pytest rather than replace them.

What no other library you can `#include` offers is the **module contract**. Every module declares
one id that is both its CLI group and its NVS namespace, answers the same verbs as every other
module (`status`, `enable`, `disable`, `reset`, and `set`/`get`/`schema` for its settings), keeps
typed settings whose type mismatches are errors instead of silent defaults, and ships three tests
a host runs over serial. Tasmota, ESPHome and WLED have this pattern, but as finished firmwares you
configure, under GPL or EUPL. Around the contract sit a few things that are also XeWe's alone:
first-boot prompts bounded by a timeout and a retry count, with `xewe provision` answering them
from flags or a dotenv file; a `schema` command that lets a host or an agent read every setting's
name, type, default and range at runtime; and a test runner that compiles and reports
`compiled, not run` when no board is attached instead of failing.

So the useful mental model is not "an OS for the ESP32" but "a contract plus the tools to prove
a module keeps it". The survey behind this, capability by capability, and what it implies for the
roadmap are in [doc/architecture.md](doc/architecture.md#8-where-xewe-os-stands).

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
./setup.sh                                  # installs everything, writes run.sh; on a terminal: the module menu
./run.sh --chip c3                          # build, erase the flash, flash, open the console
```

Needs Python >= 3.11 and git; no sudo. `run.sh` is not in the repository: every `./setup.sh` writes it.

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
their tests) in `build/modules/`, `src/Modules.h` and `run.sh`. arduino-cli, the esp32 core and the
modules repository go to `~/.xewe-os/build-tools/` (`XEWE_HOME` overrides `~/.xewe-os`). The **first run on
a machine** downloads them once (~1.7 GB, ~8 GB on disk, a few minutes on a fast link); every later
project and re-run reuses them and takes seconds. Already have the esp32 core? `./setup.sh
--arduino-data DIR` (or `XEWE_ARDUINO_DATA=DIR`; `DIR` holds `packages/esp32/`). Nothing in
`~/.arduino15` or `~/Arduino` is read or written.

**First boot.** `./run.sh` erases the whole flash before flashing, NVS included, so every run is a
true first boot. The board then asks, on the console:

1. `Name your device (Enter for "xewe-device"):` (and `Confirm`);
2. for each module, in declaration order: `Would you like to enable <Name> module?` (for a module
   that can be disabled), then that module's own setup (Wi-Fi network and password, timezone, or
   the example module's `Starting level (0-100)?`);
3. `Initial Setup Complete`, then a restart into normal operation.

The name prompt waits for an answer; build with
`--define 'XEWE_CORE_DEVICE_NAME="Kitchen Lights"'` to skip it (uncommenting the
`XEWE_CORE_DEVICE_NAME` line in `Config.h`, or a dotenv value of that name, does the same). Module prompts are bounded, so they time
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
`compiled, not run: no board attached (c3, build/builds/c3/out/3.0.0-c3-xewe-os.bin)` and exit 0.
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
| Committed | `xewe-os.ino`, `Config.h` (your settings: the core block with the core's values, and one marked block per selected module that `./setup.sh` appends; you edit both in place), `xewe.toml`, `setup.sh`, `src/YourModule/`, `src/YourModuleFull/`, `tests/`, `static/media/` (Git LFS, `.gitattributes`), `static/firmware/releases/.gitkeep`, `releases/<version>/` (`notes.txt` and the generated `release_card.md`), `scripts/release.sh`, `README.md`, `doc/`, `LICENSE.txt`, `.github/workflows/ci.yml`, `.agents/` |
| Generated, ignored, safe to delete | `build/`, `src/Modules.h`, `run.sh` (`./setup.sh` writes all three again); `releases/<version>/logs/` are local run logs, ignored |
| Ignored, yours | `.env`: the credentials `xewe provision` reads, optional; you create it here or in the tools checkout (`./setup.sh` says where and which keys when there is none, and never writes one) |
| Shared, outside the project | `~/.xewe-os/build-tools/`: deleting it only means the next setup downloads it again |

No submodules. The full `build/` tree is in [doc/architecture.md](doc/architecture.md#build-layout).
`xewe format` uses the C++ style shipped with the tools.

## Versions and settings

`xewe.toml` pins one ref each for XeWeCore, the modules and the tools, plus third-party libraries:
core `3.1.0`, modules `3.0.0` (moves to `3.1.0` once the modules set is tagged), tools `3.1.2`, ArduinoJson `v7.4.3`. A tag freezes a ref; `latest`
follows the default branch; `xewe manifest update` moves the refs to the newest tags. The ref
policy is in [doc/architecture.md](doc/architecture.md#6-versioning-and-release-policy-d12).

The firmware version is `[project] version` in `xewe.toml`; builds never change it. Your settings
and defaults go in `Config.h`, which holds the core block and one block per selected module (the
[configuration spec](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/config.md) describes
the scheme). `xewe build` puts three headers in front of every source file: the generated
`XeWeBuildInfo.h` (version, timestamp, chip), then `Config.h`, then the generated `XeWeOverrides.h`
(dotenv values and `--define`s). A first-boot value is commented out in `Config.h`; uncomment it to
pre-answer the question, or set it to `INJECTED_FROM_DOTENV` to take it from the dotenv file. A
plain Arduino IDE build uses the defaults. Override one value per build with `--define KEY=VALUE`;
string values carry their own quotes:
`build/tools/.venv/bin/python -m xewe build --define 'XEWE_CORE_DEVICE_NAME="Bench"'`. The project
URL is the `.url` on the `XeWeOs os({...})` line of `xewe-os.ino`.

The modules' compile-time values (a LED data pin, a buffer size, listener slots) live in `Config.h`
too: `./setup.sh` and `xewe modules select` append one marked block per selected module
(`// ---- wifi (xewe modules generate) ----` … `// ---- end wifi ----`) the first time the module is
selected, and never touch it again. Edit the numbers, keep the markers. `xewe build` passes
`Config.h` to every translation unit, so the module sources see your values; a `--define` of the
same name still wins. First-boot defaults that a module stores in NVS (a strip length, a
sensor address) are not in `Config.h`; each module README lists its `--define` names.

## Releases

Tags are `X.Y.Z` with no `v`, equal to `[project] version` in `xewe.toml`. A release is
`scripts/release.sh --prepare X.Y.Z --hardware c3,c6,s3` (sets the version, creates
`releases/X.Y.Z/notes.txt`, then runs setup, module validation, the unit tests, the board tests of
every chip, the builds, format and brand lint once, and renders `releases/X.Y.Z/release_card.md`
from that run), a commit and push of that, then `scripts/release.sh X.Y.Z` (`--dry-run` first): it
verifies the card against the tree, checks CI, and pushes an annotated tag whose message is
`notes.txt`. The card lists every test per chip and what was not covered; chips without a hardware
run are marked compile only. The run logs stay in `releases/X.Y.Z/logs/` on your machine and are
not committed or published.

CI does the rest (`ci.yml`, release job): it builds c3, c6 and s3 and publishes a GitHub Release.
The images are release assets only and are never committed; `static/firmware/releases/` holds
just a `.gitkeep`, and the CI lint fails on any other file there. The assets are, per image, the
merged `.bin` (written at 0x0), its `meta.json` and its ESP Web Tools `manifest.json`, plus
`firmware_map.csv`, `firmware-X.Y.Z.tar.gz` and `release_card.md`. Each has a direct URL:

```
https://github.com/xewe-labs/xewe-os/releases/download/<tag>/<asset>
```

Nothing needs Git LFS for firmware; LFS covers `static/media/` only. A `release_matrix.csv` at the
root adds image rows beyond the plain build per chip; the tools' `doc/spec.md` §11 has the format.
The committed 3.0.0 folder and the old `binaries` branch are legacy. The decisions are D26 and D27
in [doc/architecture.md](doc/architecture.md#7-decisions-log).

## Your own module

Project-local modules are first class (D28 in [doc/architecture.md](doc/architecture.md#7-decisions-log)):
a module in this project's `src/` uses the same `xewe::Module` API and contract as one from the
modules repository, with its own `$group` and NVS namespace, and needs no entry anywhere else.
`src/YourModuleFull/` is the pattern to copy; `src/YourModule/` is the smallest form.

Two project-local example modules are declared in `xewe-os.ino`, after the generated ones
(`YourModule your_module(os);`, `YourModuleFull your_module_full(os);`). They are yours; setup
never touches them.

A project-local module keeps its values in `src/<Folder>/Config.h`, in the module form of the
[configuration spec](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/config.md): pure
preprocessor, the three section markers (runtime defaults, first-boot values, compile-time
constants), one `#ifndef` group per value. Its names start with `XEWE_<ID>_`, the module id in
upper case: `XEWE_YOUR_MODULE_` for `your_module`, `XEWE_YOUR_MOD_FULL_` for `your_mod_full`. You
edit that file in place; `xewe modules generate` never touches a project-local module, so its
values get no block in the project's `Config.h`. A `--define` or a line in the project's
`Config.h` still wins over it.

- **`src/YourModule/`** (`$your_module`): the smallest complete module. A two-row settings table
  (`number`, `beat_s`, which gives it `set`/`get`/`schema` and the status lines; their defaults are
  `XEWE_YOUR_MODULE_NUMBER` and `XEWE_YOUR_MODULE_BEAT_S`), one command of its own (`show`) and one
  compile-time constant, `XEWE_YOUR_MODULE_HEARTBEAT`: 0 by default, 1 prints the number every
  `beat_s` seconds. Copy this one for a simple module.
- **`src/YourModuleFull/`** (`$your_mod_full`; ids are NVS namespaces, at most 15 characters): the
  full tour. It overrides every `xewe::Module` hook (all four `begin_routines_*`, `loop`, `enable`,
  `disable`, `reset`, `status`) and shows a `Config.h` with all three sections, a bounded
  first-boot prompt for the first-boot value `XEWE_YOUR_MOD_FULL_LEVEL` (asked only under
  `XEWE_YOUR_MOD_FULL_LEVEL_PROMPT`; `--define 'XEWE_YOUR_MOD_FULL_LEVEL=70'` pre-answers it), a
  settings table with a `SECRET` row (`token`, never printed) and a `RESTART` row (`pin`),
  `on_setting_changed`, a FlexData blob with a `schema` field checked with `has()` (a foreign blob
  is never overwritten) and reported through `schema_extra`, a GPIO claimed in the core pin
  registry, an `AsyncTimer` in `loop`, `validate<>`, one command name with two argument counts,
  `os.report_error` and a `xewe::ListenerSet` the sketch subscribes to. Copy what you need and delete
  the rest.

Both examples are tested by `xewe test` (`tests/board/` and `tests/unit/test_examples.py`).

Rename the folder, the class, the id and the prefix of its `Config.h`, or delete a folder and its
lines in `xewe-os.ino`. Plain code can also go in `setup()`/`loop()` of `xewe-os.ino`.

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
[contract](https://github.com/xewe-labs/xewe-os-modules/blob/main/doc/contract.md). Never edit
`src/Modules.h` or `build/modules/` by hand; setup regenerates them.

## Documentation

| What | Where |
|---|---|
| Why the ecosystem is shaped this way, the decisions, where it stands against other ESP32 tools | [doc/architecture.md](doc/architecture.md) |
| Naming rules for every repository | [doc/naming.md](doc/naming.md), checked by `xewe brand-lint` |
| XeWeCore: Os, modules, settings, console, NVS | [xewe-os-core `doc/`](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/README.md) |
| The modules and how to write one | [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules#readme) |
| Every `xewe` command, `xewe.toml`, the build layout | [xewe-os-tools](https://github.com/xewe-labs/xewe-os-tools#readme), [`doc/spec.md`](https://github.com/xewe-labs/xewe-os-tools/blob/main/doc/spec.md) |
| Organization guidelines | [xewe-labs/.github](https://github.com/xewe-labs/.github) |
| Working in this repository as a coding agent | [`.agents/AGENTS.md`](.agents/AGENTS.md) |

## License

GPL-3.0-only (see `LICENSE.txt`).
