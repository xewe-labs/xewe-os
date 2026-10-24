# Code rules

## Sketch and Config.h

`xewe-os.ino` includes `"Config.h"` first and `<XeWeCore.h>` second, which is the umbrella and
the only library include. It declares `XeWeOs os({...})` before any module. `src/Modules.h`
comes after that, then the project-local modules.

The project URL is the `.url` field on the `XeWeOs os({...})` line.

`Config.h` includes nothing and sets no build macros. `xewe build` puts the generated
`XeWeBuildInfo.h`, then `Config.h`, then the generated `XeWeOverrides.h` in front of every
source file with `-include`. `Config.h` holds the core block and one block per selected
module, each framed by start and end lines. Every generated line is a comment, so uncomment a
`#define` to override a value. First-boot lines stay commented out, and no
`INJECTED_FROM_DOTENV` line is active, because the release script refuses one. The format is
specified in `doc/config.md` of xewe-os-core.

The version is `[project] version` in `xewe.toml`. Never write it as a literal in code.

## Sample modules

`src/SimpleModule/` (minimal) and `src/FullModule/` (every hook) are teaching code, and their
comments explain the `xewe::Module` API to the reader. They must compile against the current
core with no warnings. When the core's module API changes, update both and their READMEs in
the same change.

## Project-local modules

A project-local module keeps its values in `src/<Folder>/Config.h`, in the module form of the
core's `doc/config.md`: two sections, `#ifndef` groups, `/* */` comments, and `_PROMPT` only on
a value asked at first boot. Names are `XEWE_<ID>_*` with the module id in upper case, such as
`XEWE_SIMPLE_MODULE_*` and `XEWE_FULL_MODULE_*`. The user edits the file in place, and
`xewe modules generate` never touches it.

The module contract applies to these modules too: the constructor parameter is named `host`,
lambdas capture `[this]`, nothing calls `cli(`, and ids are at most 15 characters. It lives in
`doc/contract.md` and `.agents/RULES.md` of xewe-os-modules.

## Comments and docs

A comment says what the code does or why. It never names an agent, a session, a date, a
decision or finding id, a run, or the code's past. Source files keep their SPDX and path
header lines.

A change to the first-boot flow, the commands, the layout or the sample modules updates
`README.md` in the same change.
