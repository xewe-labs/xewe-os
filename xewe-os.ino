// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/xewe-os.ino

// The project's values, first so a plain Arduino IDE build reads them too.
#include "Config.h"

#include <XeWeCore.h>

// The Os, before every module: modules register with it from their constructors.
XeWeOs os({.url = "https://github.com/xewe-labs/xewe-os"});

// src/Modules.h is generated.
#if __has_include("src/Modules.h")
#include "src/Modules.h"
#endif

#include "src/SimpleModule/SimpleModule.h"
SimpleModule simple_module(os);

#include "src/FullModule/FullModule.h"
FullModule full_module(os);

struct SketchLevelListener : LevelListener {
    void on_level(uint16_t level, const void*) override {
        os.serial.printf("sketch: level is now %u", level);
    }
} sketch_level_listener;

void setup() {
    full_module.listeners.add(&sketch_level_listener);
    os.begin();
}

void loop() {
    os.loop();
}
