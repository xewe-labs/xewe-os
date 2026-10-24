# FullModule

The full tour of `xewe::Module`: it overrides every hook and shows each pattern once, so you copy
what you need and delete the rest. Read [writing a module](../../doc/WRITING_A_MODULE.md) first;
[SimpleModule](../SimpleModule/README.md) is the smallest form.

Module id `full_module` (NVS namespace, console command `$full_module`), display name Full
Module, define prefix `XEWE_FULL_MODULE_`.

## Settings

| Key | Type | Range | Default |
|---|---|---|---|
| `level` | integer | 0 to 100 | `XEWE_FULL_MODULE_LEVEL` (`50`), asked at first boot |
| `active` | bool | 0 or 1 | `XEWE_FULL_MODULE_ACTIVE` (`1`) |
| `label` | string | 0 to 15 characters | `XEWE_FULL_MODULE_LABEL` (`"hello"`) |
| `token` | string, secret | 0 to 31 characters | empty; never printed |
| `pin` | integer, restart | 0 to 255, 255 = none | `XEWE_FULL_MODULE_PIN` (`255`) |

Three preset levels live in one FlexData blob. `Config.h`, in the form of the core's
[configuration page][config], also holds the tick period, tick printing and prompt timeout.

## First boot

The core asks `Would you like to enable Full Module module?`; on yes the module asks
`Starting level (0-100)?` (two attempts, 30 s each; no answer keeps 50). A project pre-answers it
with `#define XEWE_FULL_MODULE_LEVEL 70` in its own `Config.h` or `xewe build --define`.

## Hooks

| Hook | When the core calls it | What the sample does |
|---|---|---|
| constructor | static construction | registers `preset` |
| `settings()` | at begin, before every routine | returns the table |
| `begin_routines_required()` | every boot, first | loads the presets, claims the pin |
| `begin_routines_init()` | until init completes once | asks or stores the level |
| `begin_routines_regular()` | every later boot | prints level, active, label |
| `begin_routines_common()` | every boot, last | starts the tick timer |
| `loop()` | every `os.loop()` while enabled | counts a tick, toggles the pin |
| `disable()` / `reset()` | `$full_module disable / reset` | stop the timer, drop the pin claim |
| `enable()` | `$full_module enable` | the base only |
| `status()` | `$full_module status`, `$system status` | adds `ticks:` |
| `schema_extra()` | `$full_module schema`, `$system schema` | adds the preset rows |
| `on_setting_changed()` | after a row is set | tells the level listeners |

## Listener

The sketch's `SketchLevelListener` implements `LevelListener` and is added with
`full_module.listeners.add(...)` before `os.begin()`; it prints `sketch: level is now <n>`.

## Console

`$full_module status | reset | enable | disable | set | get | schema | preset <0-2> [<0-100>]`;
`$help full_module` lists them.

[config]: https://github.com/xewe-labs/xewe-os-core/blob/main/doc/config.md
