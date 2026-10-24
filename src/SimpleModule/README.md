# src/SimpleModule

The smallest complete module. It prints `hello <name>` on the console every `period_s` seconds from `loop()`, with a non-blocking timer. Both values are settings rows, asked at first boot and changed at run time from the console. Copy the folder to start a module of your own.

Open `SimpleModule.h` for the settings table and `SimpleModule.cpp` for the timer. `Config.h` holds the defaults under the `XEWE_SIMPLE_MODULE_` prefix. The module id `simple_module` is the NVS namespace and the console command `$simple_module`. The full pattern is in [doc/WRITING_A_MODULE.md](../../doc/WRITING_A_MODULE.md).

```sh
build/tools/.venv/bin/python -m xewe test --unit-only
```

## First boot

The core asks `Would you like to enable Simple Module module?`. On yes the module asks `Who to greet (Enter for "world", up to 15 characters)?` and `Seconds between greetings (1-3600)?`, each with two attempts of 30 seconds. No answer keeps the default.

A project pre-answers a question by defining the value in its own `Config.h`, for example `#define XEWE_SIMPLE_MODULE_NAME "bench"`, or with `xewe build --define`. The `_PROMPT` define then stays undefined, so the module stores the value without asking.

## Console

```text
$help simple_module             # the module's commands
$simple_module status           # "Simple Module module enabled" (or disabled), name:, period_s:
$simple_module set name bench   # validated and saved: name=bench
$simple_module set period_s 1   # the timer restarts; a greeting every second
$simple_module get period_s     # period_s=1
$simple_module schema           # one JSON line per setting, then {"end":"simple_module",...}
$simple_module reset            # wipes its NVS and restarts; the next boot asks again
$simple_module disable          # asks first; wipes its NVS, stops the greeting, restarts
$simple_module enable           # restarts; that boot asks the two questions again
```
