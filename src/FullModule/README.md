# src/FullModule

The full tour of `xewe::Module`. It overrides every hook and shows each pattern once, so you copy what you need and delete the rest. [SimpleModule](../SimpleModule/README.md) is the smallest form.

Open `FullModule.h` for the settings table and the listener interface, and `FullModule.cpp` for the hooks in call order. `Config.h` holds the defaults under the `XEWE_FULL_MODULE_` prefix. The module id `full_module` is the NVS namespace and the console command `$full_module`. The hook table is in [doc/WRITING_A_MODULE.md](../../doc/WRITING_A_MODULE.md).

```sh
build/tools/.venv/bin/python -m xewe test --unit-only
```

## First boot

The core asks `Would you like to enable Full Module module?`. On yes the module asks `Starting level (0-100)?`, with two attempts of 30 seconds; no answer keeps 50. A project pre-answers it with `#define XEWE_FULL_MODULE_LEVEL 70` in its own `Config.h` or with `xewe build --define`.

## Console

`$full_module status | reset | enable | disable | set | get | schema | preset <0-2> [<0-100>]`; `$help full_module` lists them.
