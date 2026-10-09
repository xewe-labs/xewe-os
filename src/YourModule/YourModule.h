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
    bool heartbeat = true;      // the bool setting: print the number every 10 s from loop()
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
    //   begin_routines_common()    every boot, last   <- the only one this module needs
    void        begin_routines_common()                     override;

    // loop: called from os.loop() while the module is enabled. Must never block.
    void        loop()                                      override;

    // status: one line used by `$your_module status` and by the `$system status` table.
    std::string status(const bool verbose = false)    const override;

    // Public API: other modules (or the sketch) may call this too.
    void        set_number(uint16_t value);

private:
    YourModuleConfig config;
    uint16_t         number       = 0;  // the NVS setting, key "number" in namespace "your_module"
    uint32_t         last_beat_ms = 0;
};
