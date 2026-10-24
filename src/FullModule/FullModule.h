// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/FullModule/FullModule.h
//
// The full tour of xewe::Module: every hook overridden, each pattern shown once (README.md).
#pragma once

#include <string>
#include <vector>

#include <XeWeCore.h>

#include "Config.h"

// The presets blob (NVS key "presets"): bump `schema` on every layout change.
struct FullModulePresets : xewe::FlexData<FullModulePresets> {
    uint8_t schema = 1;
    std::vector<uint16_t> levels = {25, 50, 100}; // $full_module preset <0-2> [<0-100>]

    static constexpr auto fields() {
        return std::make_tuple(xewe::fld("schema", &FullModulePresets::schema),
            xewe::fld("levels", &FullModulePresets::levels));
    }
};

// Told on every level change; `origin` is whoever set it (nullptr from the console).
struct LevelListener {
    virtual ~LevelListener() = default;
    virtual void on_level(uint16_t level, const void* origin) = 0;
};

class FullModule : public xewe::Module {
public:
    explicit FullModule(xewe::Os& host);

    void begin_routines_required() override; // every boot, first
    void begin_routines_init() override; // until init completes once
    void begin_routines_regular() override; // every boot after that
    void begin_routines_common() override; // every boot, last

    void loop() override; // while enabled; never blocks

    void enable(const bool verbose = false, const bool do_restart = true) override;
    void disable(const bool verbose = false, const bool do_restart = true) override;
    void reset(const bool verbose = false, const bool do_restart = true,
        const bool keep_enabled = true) override;

    std::string status(const bool verbose = false) const override;
    xewe::Settings settings() const override;
    void schema_extra(xewe::SchemaOut& out) const override;

    void set_level(uint16_t value, const void* origin = nullptr);
    void use_preset(uint8_t index);

    xewe::ListenerSet<LevelListener> listeners; // up to 4, no heap

protected:
    void on_setting_changed(const xewe::SettingDef& def) override; // level: tell the listeners

private:
    void register_commands();
    void load_presets();
    void save_presets();

    uint16_t level = XEWE_FULL_MODULE_LEVEL;
    bool active = XEWE_FULL_MODULE_ACTIVE;
    std::string label;
    std::string token; // SECRET: never printed
    uint8_t pin = XEWE_FULL_MODULE_PIN; // RESTART: claimed at boot; 255 = none

    FullModulePresets presets;
    bool presets_owned = true; // false: NVS holds a blob this module must not touch
    bool pin_claimed = false;
    const void* level_origin = nullptr; // set_level -> on_setting_changed
    xewe::AsyncTimer<uint8_t> tick_timer;
    uint32_t ticks = 0;
};
