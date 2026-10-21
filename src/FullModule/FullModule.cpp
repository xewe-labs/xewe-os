// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/FullModule/FullModule.cpp
#include "FullModule.h"

FullModule::FullModule(xewe::Os& host)
    : Module(host,
          /* id                  */ "full_module",
          /* name                */ "Full Module",
          /* description         */ "Every Module hook, commented",
          /* requires_init_setup */ true,
          /* can_be_disabled     */ true,
          /* has_cli_commands    */ true),
      tick_timer(XEWE_FULL_MODULE_TICK_MS) {
    // static construction: store and register only, no serial and no NVS
    register_commands();
}

// One row per plain setting; defaults from Config.h.
xewe::Settings FullModule::settings() const {
    static constexpr xewe::SettingDef table[] = {
        xewe::setting<&FullModule::level>("level", 0, 100, XEWE_FULL_MODULE_LEVEL, "Level, %"),
        xewe::setting<&FullModule::active>("active", XEWE_FULL_MODULE_ACTIVE, "Count ticks"),
        xewe::setting<&FullModule::label>("label", 15, XEWE_FULL_MODULE_LABEL, "Shown at boot"),
        xewe::setting<&FullModule::token>("token", 31, "", "API token", xewe::SettingDef::SECRET),
        xewe::setting<&FullModule::pin>(
            "pin", 0, 255, XEWE_FULL_MODULE_PIN, "GPIO, 255 = none", xewe::SettingDef::RESTART),
    };
    return {table, this};
}

// The presets as schema rows, each with its `set` hint.
void FullModule::schema_extra(xewe::SchemaOut& out) const {
    for (size_t i = 0; i < presets.levels.size(); ++i) {
        out.row("\"key\":\"level\",\"group\":\"preset:" + std::to_string(i) +
            "\",\"type\":\"u16\",\"min\":0,"
            "\"max\":100,\"value\":" +
            std::to_string(presets.levels[i]) + ",\"set\":\"$full_module preset " +
            std::to_string(i) + " <0-100>\"");
    }
}

// ---- begin ----

// Loads the presets blob and claims the tick LED pin.
void FullModule::begin_routines_required() {
    load_presets();
    pin_claimed = pin != 255 && xewe::pins::claim(pin, id.c_str());
    if (pin_claimed)
        pinMode(pin, OUTPUT);
}

// Asks the starting level (bounded), or stores the build's level without a question.
void FullModule::begin_routines_init() {
#ifdef XEWE_FULL_MODULE_LEVEL_PROMPT
    bool answered = false;
    uint16_t value = os.serial.get_uint16("Starting level (0-100)?", 0, 100, 2,
        XEWE_FULL_MODULE_PROMPT_TIMEOUT_MS, XEWE_FULL_MODULE_LEVEL, answered);
    if (!answered)
        os.serial.printf("No answer: level stays %u", value);
    set_level(value);
#else
    set_level(XEWE_FULL_MODULE_LEVEL);
#endif
}

// Prints the stored values.
void FullModule::begin_routines_regular() {
    os.serial.printf("%s: level %u, active %s, label '%s'", name.c_str(), level,
        active ? "yes" : "no", label.c_str());
}

// Starts the tick timer.
void FullModule::begin_routines_common() {
    tick_timer.initiate();
}

// ---- run time ----

// Counts a tick and toggles the LED when the timer is done.
void FullModule::loop() {
    if (!active || tick_timer.is_not_done())
        return;
    tick_timer.initiate();
    ++ticks;
    if (pin_claimed)
        digitalWrite(pin, ticks & 1);
    if (XEWE_FULL_MODULE_PRINT_TICKS)
        os.serial.printf("%s: tick %lu", name.c_str(), (unsigned long)ticks);
}

// ---- flow: always call the base, it owns the NVS flags ----

// Nothing to add to the base.
void FullModule::enable(const bool verbose, const bool do_restart) {
    Module::enable(verbose, do_restart);
}

// Stops the tick timer once disabled.
void FullModule::disable(const bool verbose, const bool do_restart) {
    Module::disable(verbose, do_restart);
    if (is_disabled())
        tick_timer.terminate();
}

// Drops the presets and the pin claim, then lets the base wipe the namespace.
void FullModule::reset(const bool verbose, const bool do_restart, const bool keep_enabled) {
    presets = FullModulePresets{};
    presets_owned = true;
    if (pin_claimed)
        xewe::pins::release(pin, id.c_str());
    pin_claimed = false;
    Module::reset(verbose, do_restart, keep_enabled);
}

// ---- info ----

// Adds the tick count and the presets state to the base status.
std::string FullModule::status(const bool verbose) const {
    std::string s = Module::status(false) + "\nticks: " + std::to_string(ticks);
    if (!presets_owned)
        s += "\npresets: foreign blob in NVS, not saving";
    if (verbose)
        os.serial.print(s);
    return s;
}

// ---- public API ----

void FullModule::set_level(uint16_t value, const void* origin) {
    if (is_disabled(true))
        return;
    level_origin = origin;
    apply_setting("level", std::to_string(value), true);
    level_origin = nullptr;
}

void FullModule::use_preset(uint8_t index) {
    if (index < presets.levels.size())
        set_level(presets.levels[index]);
}

void FullModule::on_setting_changed(const xewe::SettingDef& def) {
    if (std::string_view(def.key) != "level")
        return;
    const void* origin = level_origin;
    listeners.notify([&](LevelListener& l) { l.on_level(level, origin); });
}

// ---- commands ----

// `preset <i>` uses a preset, `preset <i> <level>` stores one.
void FullModule::register_commands() {
    register_command({"preset", "Use a preset level: <0-2>", "$full_module preset 1", 1,
        [this](xewe::span<const std::string> args) {
            if (auto i = xewe::validate<uint8_t>(args[0], 0, 2))
                use_preset(*i);
            else
                os.serial.print("Usage: $full_module preset <0-2>");
        }});
    register_command({"preset", "Store a preset level: <0-2> <0-100>", "$full_module preset 1 75",
        2, [this](xewe::span<const std::string> args) {
            auto i = xewe::validate<uint8_t>(args[0], 0, 2);
            auto v = xewe::validate<uint16_t>(args[1], 0, 100);
            if (!i || !v) {
                os.serial.print("Usage: $full_module preset <0-2> <0-100>");
                return;
            }
            presets.levels[*i] = *v;
            save_presets();
        }});
}

// ---- NVS ----

// Defaults when the blob is missing or foreign; a foreign blob is never overwritten.
void FullModule::load_presets() {
    FullModulePresets stored;
    const bool read = os.nvs.read_flex(id, "presets", stored);
    presets_owned = !read || (stored.has("schema") && stored.schema == FullModulePresets{}.schema);
    presets = (read && presets_owned && stored.levels.size() == 3) ? stored : FullModulePresets{};
    if (!presets_owned)
        os.report_error(
            "! %s: presets schema %u, expected %u: defaults, not saving (reset to take over)",
            name.c_str(), stored.schema, presets.schema);
}

// Writes the blob unless it belongs to another firmware.
void FullModule::save_presets() {
    if (!presets_owned) {
        os.serial.print("Changed for this boot only (foreign presets in NVS)");
        return;
    }
    if (!os.nvs.write_flex(id, "presets", presets))
        os.report_error("! %s: presets not saved (NVS write failed)", name.c_str());
}
