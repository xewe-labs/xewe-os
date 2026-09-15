# Adding a Module

XeWe OS modules are built on the [XeWeOS framework](https://github.com/xewe-labs/xewe-library-os)
and each lives in its own repo named `xewe-os-module-<slug>`, under any GitHub account.
The [xewe-os-modules registry](https://github.com/xewe-labs/xewe-os-modules) lists them;
`scripts/setup.sh` reads the registry, installs the chosen modules into `src/` and generates the
declarations. The framework's README ("Writing a module") covers the API and lifecycle; its
`extras/ModuleTemplate` folder is the starting point for the code.

## 1. Create the module repo

Copy an existing module repo (e.g. `xewe-os-module-pins`) as `xewe-os-module-<slug>` and replace
its code. A module repo contains:

| Path | |
|---|---|
| `src/<Name>/<Name>.{h,cpp}` | the module; one folder named like the class |
| `module.properties` | metadata read by `scripts/setup.sh` and `scripts/validate.sh` |
| `xewe-os-module-<slug>.ino` | validation firmware: framework + required modules + this module |
| `scripts/validate.sh` | builds that firmware on its own (copy it unchanged) |
| `README.md`, `LICENSE.txt`, `.gitignore` | |

Code conventions:

* Global namespace, one folder per class, file names matching the class.
* Include the framework with `#include <XeWeOS.h>` and other modules relatively, e.g.
  `#include "../Wifi/Wifi.h"` (installed modules sit side by side in `src/`).
* Pick a short, unique `id` (at most 15 characters). It is the CLI group (`$relay`) and the NVS
  namespace, so don't change it once devices store data under it.

```cpp
struct RelayConfig {
    uint8_t pin = 5;
};

class Relay : public xewe::os::Module {
public:
    Relay(xewe::os::ModuleController& controller, RelayConfig config = {});
    void begin_routines_common() override;
    void set(bool on);
private:
    RelayConfig config;
};
```

```cpp
Relay::Relay(xewe::os::ModuleController& controller, RelayConfig config)
    : Module(controller, "relay", "Relay", "Switches a relay",
             /* requires_init_setup */ false,
             /* can_be_disabled     */ true,
             /* has_cli_commands    */ true)
    , config(config) {
    register_command({"on", "Turn the relay on", "$relay on", 0,
                      [this](std::span<const std::string>) { set(true); }});
}
```

## 2. Depend on other modules through the constructor

If the module uses another module, take it by reference and declare the requirement:

```cpp
Relay(xewe::os::ModuleController& controller, Time& time_module, RelayConfig config = {})
    : Module(controller, ...), time_module(time_module) {
    add_requirement(time_module);
}
```

A module whose requirement is disabled is disabled too; disabling the requirement cascades to it.

## 3. Describe it in `module.properties`

```
name=Relay
slug=relay
id=relay
version=0.1.0
description=Switches a relay from the command line and schedules
repo=https://github.com/xewe-labs/xewe-os-module-relay
folder=Relay
include=src/Relay/Relay.h
declare=Relay relay(os, time_module);
depends_modules=time
depends_libraries=XeWeOS (>=0.1.0)
```

* `declare` is the exact line placed in the generated `src/Modules.h`. It may use `os` and the
  variable names from the `declare` lines of its required modules (`wifi`, `time_module`, ...).
* `depends_modules` lists module slugs; `setup.sh` and `validate.sh` add them (and their own
  requirements) automatically and declare them first.

## 4. Validate, publish, register

```bash
cd xewe-os-module-relay
scripts/validate.sh                 # compiles framework + time + wifi + relay for c3, c6, s3
scripts/validate.sh -b c3 -p <port> # run it on a board
```

Push the repo (e.g. `github.com/<you>/xewe-os-module-relay`), then open a pull request that
adds its URL to `repositories.txt` in
[xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules); its README lists what reviewers
check. Once merged, the module appears in every `scripts/setup.sh` checklist.

Before that, try it with a local copy of the registry list, whose entries may also be local
folders:

```bash
cp <xewe-os-modules clone>/repositories.txt /tmp/repositories.txt
echo "$HOME/code/modules/xewe-os-module-relay" >> /tmp/repositories.txt
scripts/setup.sh --modules relay --modules-index /tmp/repositories.txt
```

Add the module and its commands to [MODULES.md](MODULES.md).
