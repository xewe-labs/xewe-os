// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModuleFull/YourModuleFull.h
//
// Project-local module, the full tour: it overrides every hook of xewe::Module and shows each
// pattern once (config struct, init-setup prompt, NVS settings with a schema, timed loop action,
// validated commands, a same-name command with two arg counts, report_error, a listener).
// Copy what you need into your own module and delete the rest. For the minimal version see
// src/YourModule/. Reference: XeWeCore doc/os/module.md and doc/nvs/blob-format.md.
#pragma once

#include <functional>
#include <string>

#include <XeWeCore.h>

// Compile-time settings, passed from the sketch: YourModuleFull m(os, {.tick_ms = 5000});
// They never change at run time and are not stored; anything the user changes goes to NVS below.
struct YourModuleFullConfig {
    uint32_t tick_ms           = 10000;  // loop(): period of the timed action
    bool     print_ticks       = false;  // loop(): print a line on every tick
    uint32_t prompt_timeout_ms = 30000;  // init-setup prompt: per attempt; 0 would wait forever
};

// Run-time settings, stored as ONE NVS blob (key "settings" in namespace "your_mod_full").
// FlexData writes the fields in fields() order, without names or types: adding, removing,
// reordering or retyping a field changes the layout. When you do, bump `schema` (first field, so it
// is decoded first even when the rest moved). The defaults below are what a fresh device gets.
struct YourModuleFullSettings : xewe::FlexData<YourModuleFullSettings> {
    uint8_t     schema = 1;        // bump on every layout change
    uint16_t    level  = 50;       // int setting   ($your_mod_full level <0-100>)
    bool        active = true;     // bool setting  ($your_mod_full active <0|1>)
    std::string label  = "hello";  // string setting ($your_mod_full label <1-15 chars>)

    static constexpr auto fields() {
        return std::make_tuple(xewe::fld("schema", &YourModuleFullSettings::schema),
                               xewe::fld("level",  &YourModuleFullSettings::level),
                               xewe::fld("active", &YourModuleFullSettings::active),
                               xewe::fld("label",  &YourModuleFullSettings::label));
    }
};

class YourModuleFull : public xewe::Module {
public:
    // Listener: the sketch (or another module) sets it; called after `level` changes.
    using LevelListener = std::function<void(uint16_t level)>;

    // Name the Os parameter `host`, never `os` (it would hide the protected member `os`).
    explicit YourModuleFull(xewe::Os& host, YourModuleFullConfig config = {});

    // ---- begin: os.begin() calls Module::begin(), which calls these in this order ----
    // (none run while the module is disabled, or while a required module is disabled)
    void        begin_routines_required()                   override;  // every boot, first
    void        begin_routines_init()                       override;  // until init completes once
    void        begin_routines_regular()                    override;  // every boot after that
    void        begin_routines_common()                     override;  // every boot, last

    // ---- run time ----
    void        loop()                                      override;  // os.loop(), while enabled; never block

    // ---- flow: the generic $your_mod_full enable / disable / reset commands call these ----
    void        enable (const bool verbose = false, const bool do_restart = true) override;
    void        disable(const bool verbose = false, const bool do_restart = true) override;
    void        reset  (const bool verbose = false, const bool do_restart = true,
                        const bool keep_enabled = true)                          override;

    // ---- info: `$your_mod_full status` (verbose) and the `$system status` table (one line) ----
    std::string status(const bool verbose = false)    const override;

    // ---- public API: the sketch and other modules may call these, even while disabled ----
    void        set_level      (uint16_t value);
    void        set_active     (bool value);
    void        set_label      (const std::string& value);
    void        on_level_change(LevelListener listener);

private:
    void        register_commands();
    void        load_settings();
    void        save_settings();

    YourModuleFullConfig      config;
    YourModuleFullSettings    settings;                   // RAM copy; NVS is written on change
    bool                      settings_owned = true;      // false: NVS holds a blob we must not touch
    LevelListener             level_listener;
    xewe::AsyncTimer<uint8_t> tick_timer;                 // non-blocking: started, then polled
    uint32_t                  ticks          = 0;
};
