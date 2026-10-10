// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModule/YourModule.h
//
// Project-local module: rename or delete this folder; it is yours, setup never touches it.
// The smallest complete module: rename YourModule -> YourName and "your_module" -> your id.
#pragma once

#include <XeWeCore.h>

// Settings fixed at compile time, passed from the sketch: YourModule your_module(os, {.heartbeat = false});
struct YourModuleConfig {
    bool heartbeat = true;      // print the number every beat_s seconds from loop()
};

class YourModule : public xewe::Module {
public:
    // Name the Os parameter `host`, never `os`: `os` is the protected member that the
    // method bodies and the [this] command lambdas use.
    explicit YourModule(xewe::Os& host, YourModuleConfig config = {});

    // begin: the Os calls the begin_routines_* hooks from os.begin(), in declaration order.
    //   begin_routines_required()  every boot, first
    //   begin_routines_init()      first boot only, until it completes (needs requires_init_setup)
    //   begin_routines_regular()   every boot after init has completed
    //   begin_routines_common()    every boot, last
    // This module needs none: the core loads the settings table before them (step 0).

    // loop: called from os.loop() while the module is enabled. Must never block.
    void        loop()                                      override;

    // status: used by `$your_module status` and by the `$system status` table.
    std::string status(const bool verbose = false)    const override;

    // Run-time settings (core 2.1): the table in YourModule.cpp. The core loads it at begin (default,
    // then NVS) and adds `$your_module set|get|schema`, one status line per row, `$system schema`.
    xewe::Settings settings()                         const override;

    // Public API: other modules (or the sketch) may call this too.
    void        set_number(uint16_t value);

private:
    YourModuleConfig config;
    uint16_t         number       = 0;  // table rows; NVS keys "number", "beat_s" in namespace "your_module"
    uint16_t         beat_s       = 10;
    uint32_t         last_beat_ms = 0;
};
