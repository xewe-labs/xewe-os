# SimpleModule

The smallest complete module: it prints `hello <name>` on the console every `period_s` seconds
from `loop()`, with a non-blocking timer. Both values are settings rows, asked at first boot and
changed at run time from the console. Copy the folder to start a module of your own; the full
pattern is in [writing a module](../../doc/WRITING_A_MODULE.md).

| | |
|---|---|
| Class, files | `SimpleModule`: `SimpleModule.h`, `SimpleModule.cpp`, `Config.h` |
| Module id | `simple_module` (NVS namespace and console command `$simple_module`) |
| Display name | Simple Module |
| Define prefix | `XEWE_SIMPLE_MODULE_` |

## Settings

| Key | Type | Range | Default |
|---|---|---|---|
| `name` | string | 0 to 15 characters | `XEWE_SIMPLE_MODULE_NAME` (`"world"`) |
| `period_s` | integer | 1 to 3600 | `XEWE_SIMPLE_MODULE_PERIOD_S` (`10`) |

The defaults live in this folder's `Config.h`, in the module form of the core's
[configuration page](https://github.com/xewe-labs/xewe-os-core/blob/main/doc/CONFIG.md).

## First boot

The core asks `Would you like to enable Simple Module module?`; on yes the module asks two
questions, each with two attempts of 30 seconds; no answer keeps the default:

1. `Who to greet (Enter for "world", up to 15 characters)?` (Enter takes the default)
2. `Seconds between greetings (1-3600)?`

A project pre-answers a question by defining the value in its own `Config.h`, for example
`#define XEWE_SIMPLE_MODULE_NAME "bench"`, or with `xewe build --define`. The `_PROMPT` define
then stays undefined, so the module stores the value without asking.

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
