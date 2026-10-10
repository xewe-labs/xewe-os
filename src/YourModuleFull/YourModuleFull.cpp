// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModuleFull/YourModuleFull.cpp
//
// The full tour of xewe::Module (see YourModuleFull.h). Every hook says when the core calls it
// and what the core does around it. Delete what you do not need; every hook is optional.
#include "YourModuleFull.h"

YourModuleFull::YourModuleFull(xewe::Os& host, YourModuleFullConfig config)
    : Module(host,                      // registers with the Os now; the sketch's declaration order is the begin order
          /* id                  */ "your_mod_full",  // CLI group ($your_mod_full) and NVS namespace: <= 15 chars,
                                                   // so not "your_module_full" (16: refused at registration)
          /* name                */ "Your Module Full",
          /* description         */ "Every Module hook, commented",  // shown by the first-boot "enable?" question
          /* requires_init_setup */ true,   // run begin_routines_init() until it completes once
          /* can_be_disabled     */ true,   // first boot asks "enable?"; adds $your_mod_full enable / disable
          /* has_cli_commands    */ true)   // creates the group, adds status / reset; register_command() works
    , config(config)
    , tick_timer(config.tick_ms) {
    // Runs during static construction: Serial is not up and NVS is not read yet, so only
    // store things and register commands here. Anything that talks or reads goes in begin_*.
    register_commands();
}

// The settings table (core 2.1): one row per plain setting, checked at compile time (key <= 15
// chars, default inside [min, max]). The core loads it at begin (default, then NVS), adds
// `$your_mod_full set|get|schema`, one status line per row and the rows of `$system schema`.
xewe::Settings YourModuleFull::settings() const {
    static constexpr xewe::SettingDef table[] = {
        xewe::setting<&YourModuleFull::level> ("level", 0, 100, 50, "Level, %"),
        xewe::setting<&YourModuleFull::active>("active", true, "Count ticks"),
        xewe::setting<&YourModuleFull::label> ("label", 15, "hello", "Shown at boot"),
        xewe::setting<&YourModuleFull::token> ("token", 31, "", "API token", xewe::SettingDef::SECRET),
        xewe::setting<&YourModuleFull::pin>   ("pin", 0, 255, 255, "Tick LED GPIO, 255 = none", xewe::SettingDef::RESTART),
    };
    return {table, this};
}

// Values that are not table rows still appear in the schema, with a "set" hint (like led's modes).
void YourModuleFull::schema_extra(xewe::SchemaOut& out) const {
    for (size_t i = 0; i < presets.levels.size(); ++i) {
        out.row("\"key\":\"level\",\"group\":\"preset:" + std::to_string(i) + "\",\"type\":\"u16\",\"min\":0,"
                "\"max\":100,\"value\":" + std::to_string(presets.levels[i]) +
                ",\"set\":\"$your_mod_full preset " + std::to_string(i) + " <0-100>\"");
    }
}

// ---- begin ----

// Every boot while enabled, first (the table is loaded already). Pins, buses, the blob.
void YourModuleFull::begin_routines_required() {
    load_presets();
    // claim the GPIO in the core registry before touching it: refused (and reported) when another
    // module holds it. A strapping pin is claimed with a warning.
    pin_claimed = pin != 255 && xewe::pins::claim(pin, id.c_str());
    if (pin_claimed) pinMode(pin, OUTPUT);
}

// First boot (and after `$your_mod_full reset` or disable/enable, which wipe init_complete).
// After it returns, the core writes init_complete = true unless this routine disabled the module.
// Prompts are allowed here (setup time) but must be bounded: a headless device must still boot.
void YourModuleFull::begin_routines_init() {
    bool answered = false;
    uint16_t value = os.serial.get_uint16("Starting level (0-100)?", 0, 100,
                                          2,                          // two attempts
                                          config.prompt_timeout_ms,   // per attempt
                                          level,                      // returned on timeout
                                          answered);
    if (!answered) os.serial.printf("No answer: level stays %u", level);
    set_level(value);              // saves to NVS and tells the listeners
}

// Every later boot (init already completed). Typical: reconnect, restore state, say hello.
void YourModuleFull::begin_routines_regular() {
    os.serial.printf("%s: level %u, active %s, label '%s'", name.c_str(), level, active ? "yes" : "no", label.c_str());
}

// Every boot while enabled, last: init or regular has run. Start timers and background work.
void YourModuleFull::begin_routines_common() {
    tick_timer.initiate();
}

// ---- run time ----

// Called on every os.loop() pass while enabled; it shares one loop with every module, so it
// checks a timer and returns. Never delay(), never prompt.
void YourModuleFull::loop() {
    if (!active || tick_timer.is_not_done()) return;
    tick_timer.initiate();         // restart: the next tick is config.tick_ms from now
    ++ticks;
    if (pin_claimed) digitalWrite(pin, ticks & 1);
    if (config.print_ticks) os.serial.printf("%s: tick %lu", name.c_str(), (unsigned long)ticks);
}

// ---- flow: override to add your own work; always call the base, it owns the NVS flags ----

// Base: refuses while a required module is disabled, else writes is_enabled and restarts.
// The next boot runs begin_routines_init() again (disable wiped init_complete).
void YourModuleFull::enable(const bool verbose, const bool do_restart) {
    Module::enable(verbose, do_restart);
}

// Base: with verbose asks "OK?" (bounded, 2 x 15 s), cascades disable to every module that
// add_requirement()'d this one, then calls reset(verbose, do_restart, false) - our override below.
// loop() stops being called once disabled; stop hardware the base does not know about.
void YourModuleFull::disable(const bool verbose, const bool do_restart) {
    Module::disable(verbose, do_restart);       // returns only when cancelled or do_restart is false
    if (is_disabled()) tick_timer.terminate();  // e.g. a cascade from a required module: stop our work
}

// Base: erases the WHOLE namespace (table keys and the presets blob), reloads the table defaults,
// keeps not_first_boot, re-enables when keep_enabled, restarts. `$your_mod_full reset` calls it.
void YourModuleFull::reset(const bool verbose, const bool do_restart, const bool keep_enabled) {
    presets       = YourModuleFullPresets{};    // RAM matches the wiped NVS until the restart
    presets_owned = true;                       // the namespace is empty now: it is ours
    if (pin_claimed) xewe::pins::release(pin, id.c_str());
    pin_claimed   = false;
    Module::reset(verbose, do_restart, keep_enabled);
}

// ---- info ----

// Module::status prints the base line and one `key: value` line per table row (token masked);
// add only what the table cannot show.
std::string YourModuleFull::status(const bool verbose) const {
    std::string s = Module::status(false) + "\nticks: " + std::to_string(ticks);
    if (!presets_owned) s += "\npresets: foreign blob in NVS, not saving";
    if (verbose) os.serial.print(s);
    return s;
}

// ---- public API ----

void YourModuleFull::set_level(uint16_t value, const void* origin) {
    if (is_disabled(true)) return;              // registered but disabled: refuse politely
    level_origin = origin;                      // on_setting_changed passes it to the listeners
    apply_setting("level", std::to_string(value), true);   // validated (0-100), saved, announced
    level_origin = nullptr;
}

void YourModuleFull::use_preset(uint8_t index) {
    if (index < presets.levels.size()) set_level(presets.levels[index]);
}

void YourModuleFull::on_setting_changed(const xewe::SettingDef& def) {
    if (std::string_view(def.key) != "level") return;   // pin: RESTART, the core says so
    const void* origin = level_origin;
    listeners.notify([&](LevelListener& l) { l.on_level(level, origin); });
}

// ---- commands ----

void YourModuleFull::register_commands() {
    // `set`/`get`/`schema` come from the table. Same name, two arg counts: the Cli picks by count.
    register_command({"preset", "Use a preset level: <0-2>", "$your_mod_full preset 1", 1,
        [this](xewe::span<const std::string> args) {
            // validate<T>(text, min, max): parse + range check; an empty optional on bad input
            if (auto i = xewe::validate<uint8_t>(args[0], 0, 2)) use_preset(*i);
            else os.serial.print("Usage: $your_mod_full preset <0-2>");
        }});
    register_command({"preset", "Store a preset level: <0-2> <0-100>", "$your_mod_full preset 1 75", 2,
        [this](xewe::span<const std::string> args) {
            auto i = xewe::validate<uint8_t>(args[0], 0, 2);
            auto v = xewe::validate<uint16_t>(args[1], 0, 100);
            if (!i || !v) { os.serial.print("Usage: $your_mod_full preset <0-2> <0-100>"); return; }
            presets.levels[*i] = *v;
            save_presets();
        }});
}

// ---- NVS (the blob; the table rows need none of this) ----

// A missing blob (fresh device) or a failed decode means defaults. A blob without a `schema` field,
// or with another schema, may belong to another firmware that used this id on this board: use
// defaults in RAM but never overwrite it; `$your_mod_full reset` wipes it and takes over.
void YourModuleFull::load_presets() {
    YourModuleFullPresets stored;
    const bool read = os.nvs.read_flex(id, "presets", stored);
    // has(): "schema was in the blob", not "schema still has its default" (FlexData presence)
    presets_owned = !read || (stored.has("schema") && stored.schema == YourModuleFullPresets{}.schema);
    presets       = (read && presets_owned && stored.levels.size() == 3) ? stored : YourModuleFullPresets{};
    if (!presets_owned)
        os.report_error("! %s: presets schema %u, expected %u: defaults, not saving (reset to take over)",
                        name.c_str(), stored.schema, presets.schema);
}

void YourModuleFull::save_presets() {
    if (!presets_owned) { os.serial.print("Changed for this boot only (foreign presets in NVS)"); return; }
    if (!os.nvs.write_flex(id, "presets", presets))
        os.report_error("! %s: presets not saved (NVS write failed)", name.c_str());
}
