# Project Structure

XeWe OS is firmware assembled by `scripts/setup.sh` from the XeWe libraries, module repos and the
build toolchain. Paths marked `(generated)` are created by setup.sh and not committed.

```text
xewe-os/
├── xewe-os.ino                         # ModuleController + #include "src/Modules.h"
├── Config.h                            # PROJECT_NAME, BUILD_VERSION, BUILD_TIMESTAMP (written by build.sh)
├── scripts/setup.sh                    # choose modules, install modules + toolchain, run build setup
│
├── src/                                # (generated) by setup.sh, see src/.gitignore
│   ├── Modules.h                       # includes and declarations of the chosen modules
│   └── <Module>/                       # e.g. Wifi/, Time/ from module repos listed in xewe-os-modules
│
├── build/
│   ├── libraries/required_libraries.txt  # library repos and pinned tags, cloned by the build setup
│   ├── release_matrix.csv              # board/config rows built by release.sh
│   ├── toolchain/                      # (generated) xewe-os-build-toolchain, installed by setup.sh
│   └── modules.lock                    # (generated) what setup.sh installed: slug|folder|source|ref|commit
│
├── doc/                                # module reference, adding a module, this file
└── static/                             # released firmware (web flasher) and README media
```

## Where things come from

| Piece | Source |
|---|---|
| Module base, controller, `$system` | [xewe-library-os](https://github.com/xewe-labs/xewe-library-os) (`XeWeOS`) |
| Serial console, prompts, tables | [xewe-library-serial](https://github.com/xewe-labs/xewe-library-serial) (`XeWeSerial`) |
| NVS storage, FlexData | [xewe-library-nvs](https://github.com/xewe-labs/xewe-library-nvs) (`XeWeNvs`) |
| `$group command args` parser | [xewe-library-cli](https://github.com/xewe-labs/xewe-library-cli) (`XeWeCli`) |
| String, validation, timer, debug helpers | [xewe-library-utils](https://github.com/xewe-labs/xewe-library-utils) (`XeWeUtils`) |
| Wifi, WebInterface, Time, Scheduler, Buttons, Pins | `xewe-os-module-<slug>` repos listed in [xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules) (installed into `src/`) |
| Build, upload, release and format scripts | [xewe-os-build-toolchain](https://github.com/xewe-labs/xewe-os-build-toolchain) (installed into `build/toolchain/`) |

Library versions come from the tags in `build/libraries/required_libraries.txt`; module and
toolchain versions from the `--modules-ref` / `--toolchain-ref` given to `setup.sh` (default
`main`), recorded in `build/modules.lock`.
