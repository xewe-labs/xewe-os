// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/xewe-os.ino
//
// XeWe OS firmware: the XeWeOS framework (serial console, NVS, CLI, system) plus the modules
// chosen with setup.sh. setup.sh installs them into src/modules/ and generates
// src/modules/Modules.h, which includes and declares them in dependency order.

#include <XeWeOS.h>

#include "Config.h"


xewe::os::ModuleController os({
    .project_name    = PROJECT_NAME,
    .version         = BUILD_VERSION,
    .build_timestamp = BUILD_TIMESTAMP,
    .url             = "https://github.com/xewe-labs/xewe-os",
});

#if __has_include("src/modules/Modules.h")
#include "src/modules/Modules.h"
#else
#error "No modules installed. Run setup.sh to choose and install modules."
#endif


void setup() {
    os.begin();
}

void loop() {
    os.loop();
}
