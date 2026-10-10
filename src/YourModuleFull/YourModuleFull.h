// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModuleFull/YourModuleFull.h
//
// Project-local module, the full tour: it overrides every hook of xewe::Module and shows each
// pattern once (config struct, init-setup prompt, a settings table with a SECRET and a RESTART row,
// a FlexData blob with a schema reported through schema_extra, timed loop action, a same-name
// command with two arg counts, a GPIO claim, report_error, a listener set).
// Copy what you need into your own module and delete the rest. For the minimal version see
// src/YourModule/. Reference: XeWeCore doc/os/module.md, doc/os/settings.md, doc/utils/pins.md.
#pragma once

#include <string>
#include <vector>

#include <XeWeCore.h>

// Compile-time settings, passed from the sketch: YourModuleFull m(os, {.tick_ms = 5000});
// They never change at run time and are not stored; anything the user changes goes to NVS below.
struct YourModuleFullConfig {
    uint32_t tick_ms           = 10000;  // loop(): period of the timed action
    bool     print_ticks       = false;  // loop(): print a line on every tick
    uint32_t prompt_timeout_ms = 30000;  // init-setup prompt: per attempt; 0 would wait forever
};

// Plain settings are table rows (YourModuleFull.cpp, settings()). What is not one value per key goes
// in ONE FlexData blob (key "presets" in namespace "your_mod_full"). FlexData writes the fields in
// fields() order, without names or types: on any layout change bump `schema` (first field).
struct YourModuleFullPresets : xewe::FlexData<YourModuleFullPresets> {
    uint8_t               schema = 1;              // bump on every layout change
    std::vector<uint16_t> levels = {25, 50, 100};  // $your_mod_full preset <0-2> [<0-100>]

    static constexpr auto fields() {
        return std::make_tuple(xewe::fld("schema", &YourModuleFullPresets::schema),
                               xewe::fld("levels", &YourModuleFullPresets::levels));
    }
};

// Listener interface: the sketch (or another module) implements it and calls listeners.add(&l).
// `origin` is whoever changed the level (nullptr from the CLI); a listener that also sets the
// level passes `this` and ignores its own echo: if (origin == this) return;
struct LevelListener {
    virtual ~LevelListener() = default;
    virtual void on_level(uint16_t level, const void* origin) = 0;
};

class YourModuleFull : public xewe::Module {
public:
    // Name the Os parameter `host`, never `os` (it would hide the protected member `os`).
    explicit YourModuleFull(xewe::Os& host, YourModuleFullConfig config = {});

    // ---- begin: os.begin() calls Module::begin(), which loads the table, then calls these ----
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

    // ---- info: `$your_mod_full status`, `$your_mod_full schema`, `$system schema` ----
    std::string    status(const bool verbose = false)       const override;
    xewe::Settings settings()                               const override;  // the table
    void           schema_extra(xewe::SchemaOut& out)       const override;  // the presets

    // ---- public API: the sketch and other modules may call these, even while disabled ----
    void        set_level (uint16_t value, const void* origin = nullptr);
    void        use_preset(uint8_t index);

    xewe::ListenerSet<LevelListener> listeners;     // up to 4, no heap

protected:
    // a table row was set (`$your_mod_full set level 75`, apply_setting): act on it now
    void        on_setting_changed(const xewe::SettingDef& def) override;

private:
    void        register_commands();
    void        load_presets();
    void        save_presets();

    YourModuleFullConfig      config;
    uint16_t                  level        = 50;       // table rows; NVS keys = row keys
    bool                      active       = true;
    std::string               label;
    std::string               token;                   // SECRET: never printed
    uint8_t                   pin          = 255;      // RESTART: claimed at boot; 255 = none
    YourModuleFullPresets     presets;
    bool                      presets_owned = true;    // false: NVS holds a blob we must not touch
    bool                      pin_claimed  = false;
    const void*               level_origin = nullptr;  // set_level -> on_setting_changed
    xewe::AsyncTimer<uint8_t> tick_timer;              // non-blocking: started, then polled
    uint32_t                  ticks        = 0;
};
