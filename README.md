# xewe-os

The firmware template of XeWe OS for ESP32-C3, -C6 and -S3.

## Problem

- You install arduino-cli, the esp32 core and the libraries by hand on every machine, and nothing records which versions the firmware was built with.
- ESP32 sketches grow ad-hoc serial commands and settings, with no structure per feature.
- Testing needs a board on the desk.

## Solution

This template pins every input, gives each feature its own console commands and settings, and puts build, flash, test and release behind one command. A product starts as a copy of this repository, so you own the result. Three ideas carry it:

- `xewe.toml` pins the core, the modules, the tools and the libraries to tags, and `./setup.sh` installs exactly those.
- Every feature is an `xewe::Module` with its own `$group` console commands, its own NVS namespace and typed settings.
- The `xewe` command builds, flashes, tests and releases, and it works with no board attached: the result is reported as "compiled, not run".

## See it

The sample module `SimpleModule` greets on the serial console and takes commands under `$simple_module`:

```text
$simple_module status
Simple Module module enabled
name: world
period_s: 10
hello world
hello world
$simple_module set name bench
name=bench
hello bench
```

## Why not X

| Alternative | What it lacks here |
|---|---|
| Arduino IDE, or arduino-cli with `sketch.yaml` profiles | Profiles cannot pin git refs or content hashes, and need one profile per board. `xewe.toml` pins both, and `xewe build --all-chips` builds c3, c6 and s3. |
| PlatformIO | It has no module contract: console commands, settings and first-boot questions are yours to invent per project. |
| ESPHome | It describes a device in YAML. Here you write C++ against `xewe::Module`, and the firmware has a runtime console. |

## Install

You need Python 3.11 or newer and git. Setup installs into the project folder and `~/.xewe-os`, so it never needs sudo.

```sh
git clone https://github.com/xewe-labs/xewe-os my-fw && cd my-fw
./setup.sh
```

You can also click "Use this template" on GitHub and clone your copy. `./setup.sh` asks which modules to include. The first run on a machine downloads arduino-cli and the esp32 core, about 1.7 GB, and later projects reuse them.

## Usage

Run the tools as `build/tools/.venv/bin/python -m xewe <command>`.

Build, flash and watch the console:

```sh
build/tools/.venv/bin/python -m xewe build --chip c3
build/tools/.venv/bin/python -m xewe flash --chip c3
build/tools/.venv/bin/python -m xewe serial
```

To add a feature, copy `src/SimpleModule/` and follow [doc/WRITING_A_MODULE.md](doc/WRITING_A_MODULE.md). To run the tests, see [doc/TESTING.md](doc/TESTING.md). Repositories, `Config.h` and every everyday command are in [doc/XEWE_OS_ECOSYSTEM.md](doc/XEWE_OS_ECOSYSTEM.md).

## Status

Version 3.1.0 builds for the c3, c6 and s3 chips. It pins core 3.1.2, tools 3.1.6 and the modules tag `wifi/3.1.2` in `xewe.toml`. Firmware updates over the air are not supported.

---

## Development

```sh
./setup.sh
build/tools/.venv/bin/python -m xewe test --unit-only
build/tools/.venv/bin/python -m xewe format
build/tools/.venv/bin/python -m xewe brand-lint .
```

CI runs from [.github/workflows/ci.yml](.github/workflows/ci.yml). The release procedure is in [scripts/README.md](scripts/README.md).

## Layout

- [src/](src/README.md) holds the project's own modules and the two samples.
- [tests/](tests/README.md) holds the unit tests and the board tests.
- [scripts/](scripts/README.md) holds the release script.
- [doc/](doc/) holds the guides linked above.
- `xewe-os.ino` is the sketch, `Config.h` the configuration, `xewe.toml` the manifest and `setup.sh` the installer.
- XeWeCore lives in [xewe-os-core](https://github.com/xewe-labs/xewe-os-core), the ready-made modules in [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules) and the `xewe` command in [xewe-os-tools](https://github.com/xewe-labs/xewe-os-tools). `./setup.sh` fetches them into `build/`.

## License

GPL-3.0-only, see [LICENSE.txt](LICENSE.txt).
