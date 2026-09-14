// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/xewe-os.ino
//
// XeWe OS firmware: the XeWeOS framework (serial console, NVS, CLI, system) plus this
// project's modules. Modules begin in the order they are declared; declare a module
// before the modules that take it as a dependency.

#include <XeWeOS.h>

#include "Config.h"
#include "src/Wifi/Wifi.h"
#include "src/WebInterface/WebInterface.h"
#include "src/Time/Time.h"
#include "src/Scheduler/Scheduler.h"
#include "src/Buttons/Buttons.h"
#include "src/Pins/Pins.h"


xewe::os::ModuleController os({
    .project_name    = PROJECT_NAME,
    .version         = BUILD_VERSION,
    .build_timestamp = BUILD_TIMESTAMP,
    .url             = "https://github.com/xewe-labs/xewe-os",
});

Wifi         wifi          (os);
WebInterface web_interface (os, wifi);
Time         time_module   (os, wifi);
Scheduler    scheduler     (os, time_module);
Buttons      buttons       (os);
Pins         pins          (os);


void setup() {
    os.begin();
}

void loop() {
    os.loop();
}
