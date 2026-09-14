# Adding a Module

XeWe OS modules are built on the [XeWeOS framework](https://github.com/xewe-labs/xewe-library-os).
The framework's README ("Writing a module") covers the full API and lifecycle, and its
`extras/ModuleTemplate` folder is the starting point. This page covers how a module fits into
this firmware.

## 1. Create the module

Copy `extras/ModuleTemplate` from xewe-library-os to `src/<Name>/` and rename the files and
the class, e.g. `src/Relay/Relay.h` and `src/Relay/Relay.cpp` with `class Relay`.

* Firmware modules live in the global namespace, one folder per class, file names matching the
  class.
* Include the framework with `#include <XeWeOS.h>` and other firmware modules relatively, e.g.
  `#include "../Wifi/Wifi.h"`.
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

## 3. Register it in `xewe-os.ino`

Include the header and declare the module after the modules it depends on:

```cpp
#include "src/Relay/Relay.h"

// ...
Scheduler    scheduler     (os, time_module);
Relay        relay         (os, time_module, {.pin = 5});
```

Declaring the object is all it takes: it registers itself, begins in declaration order, and its
commands appear under `$help`.

## 4. Build and document

* Build with the platform build script (see the README) and try the commands over serial.
* Add the module and its commands to [MODULES.md](MODULES.md).
* A module that is useful beyond this firmware should become its own library repo under
  `xewe-labs` (see the rules in publish-arduino-library).
