# xewe-os

The firmware template of XeWe OS, for ESP32-C3, -C6 and -S3: XeWeCore (serial console, NVS
storage, `$group command` CLI) plus the modules you select, built and flashed by the `xewe` tools.
A product starts as a copy of this repository, so you own the result.

## First five minutes

```sh
git clone https://github.com/xewe-labs/xewe-os my-fw && cd my-fw   # or "Use this template"
./setup.sh                                          # installs everything; asks which modules
build/tools/.venv/bin/python -m xewe build --chip c3
build/tools/.venv/bin/python -m xewe flash --chip c3
build/tools/.venv/bin/python -m xewe serial
```

Needs Python >= 3.11 and git, no sudo. Without a board the commands compile and say
`compiled, not run`.

## Documentation

| What | Where |
|---|---|
| Repositories, setup, `Config.h`, commands | [xewe_os_ecosystem.md](doc/xewe_os_ecosystem.md) |
| Writing your own module, the naming rules | [doc/writing_a_module.md](doc/writing_a_module.md) |
| Unit tests, board tests, CI | [doc/testing.md](doc/testing.md) |
| The smallest module | [src/SimpleModule/README.md](src/SimpleModule/README.md) |
| A module with every hook | [src/FullModule/README.md](src/FullModule/README.md) |
| Releasing | the header of [scripts/release.sh](scripts/release.sh) |
| Working here as a coding agent | [.agents/AGENTS.md](.agents/AGENTS.md) |
| Every `xewe` command | [tools commands][tools] |
| XeWeCore | [xewe-os-core](https://github.com/xewe-labs/xewe-os-core) |
| The modules | [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules) |

[tools]: https://github.com/xewe-labs/xewe-os-tools/blob/main/src/xewe/README.md

## License

GPL-3.0-only (see `LICENSE.txt`).
