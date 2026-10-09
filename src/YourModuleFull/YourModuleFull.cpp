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

// ---- begin ----

// Every boot while enabled, first. Things both init and regular need: pins, buses, NVS reads.
void YourModuleFull::begin_routines_required() {
    load_settings();
}

// First boot (and after `$your_mod_full reset` or disable/enable, which wipe init_complete).
// After it returns, the core writes init_complete = true unless this routine disabled the module.
// Prompts are allowed here (setup time) but must be bounded: a headless device must still boot.
void YourModuleFull::begin_routines_init() {
    bool answered = false;
    uint16_t level = os.serial.get_uint16("Starting level (0-100)?", 0, 100,
                                          2,                          // two attempts
                                          config.prompt_timeout_ms,   // per attempt
                                          settings.level,             // returned on timeout
                                          answered);
    if (!answered) os.serial.printf("No answer: level stays %u", settings.level);
    set_level(level);              // saves to NVS and tells the listener
}

// Every later boot (init already completed). Typical: reconnect, restore state, say hello.
void YourModuleFull::begin_routines_regular() {
    os.serial.printf("%s: level %u, active %s, label '%s'", name.c_str(), settings.level,
                     settings.active ? "yes" : "no", settings.label.c_str());
}

// Every boot while enabled, last: init or regular has run. Start timers and background work.
void YourModuleFull::begin_routines_common() {
    tick_timer.initiate();
}

// ---- run time ----

// Called on every os.loop() pass while enabled; it shares one loop with every module, so it
// checks a timer and returns. Never delay(), never prompt.
void YourModuleFull::loop() {
    if (!settings.active || tick_timer.is_not_done()) return;
    tick_timer.initiate();         // restart: the next tick is config.tick_ms from now
    ++ticks;
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

// Base: erases the WHOLE namespace (our "settings" blob included), keeps not_first_boot,
// re-enables when keep_enabled, restarts. `$your_mod_full reset` calls it without asking.
void YourModuleFull::reset(const bool verbose, const bool do_restart, const bool keep_enabled) {
    settings       = YourModuleFullSettings{};  // RAM matches the wiped NVS until the restart
    settings_owned = true;                      // the namespace is empty now: it is ours
    Module::reset(verbose, do_restart, keep_enabled);
}

// ---- info ----

// One line for the `$system status` table (a cell cannot hold line breaks); the verbose form
// (`$your_mod_full status`) prints a few more lines. Compose: keep the base line first.
std::string YourModuleFull::status(const bool verbose) const {
    std::string s = Module::status(false) + ", level " + std::to_string(settings.level);
    if (verbose) {
        os.serial.print(s);
        os.serial.printf("  active: %s\n  label:  %s\n  ticks:  %lu%s", settings.active ? "yes" : "no",
                         settings.label.c_str(), (unsigned long)ticks,
                         settings_owned ? "" : "\n  NVS:    foreign settings blob, not saving");
    }
    return s;
}

// ---- public API ----

void YourModuleFull::set_level(uint16_t value) {
    if (is_disabled(true)) return;              // registered but disabled: refuse politely
    settings.level = value;
    save_settings();
    if (level_listener) level_listener(value);  // no listener set = nothing to call
}

void YourModuleFull::set_active(bool value) {
    if (is_disabled(true)) return;
    settings.active = value;
    save_settings();
}

void YourModuleFull::set_label(const std::string& value) {
    if (is_disabled(true)) return;
    settings.label = value;
    save_settings();
}

void YourModuleFull::on_level_change(LevelListener listener) { level_listener = std::move(listener); }

// ---- commands ----

void YourModuleFull::register_commands() {
    // Same name, two arg counts: the Cli picks by count (an overload set). 0 args = show, 1 = set.
    register_command({"level", "Show the level", "$your_mod_full level", 0,
        [this](xewe::span<const std::string>) { os.serial.printf("level = %u", settings.level); }});
    register_command({"level", "Set the level (0-100)", "$your_mod_full level 75", 1,
        [this](xewe::span<const std::string> args) {
            // validate<T>(text, min, max): parse + range check; an empty optional on bad input
            if (auto n = xewe::validate<uint16_t>(args[0], 0, 100)) set_level(*n);
            else os.serial.print("Usage: $your_mod_full level <0-100>");
        }});
    register_command({"active", "Turn the tick on or off", "$your_mod_full active 0", 1,
        [this](xewe::span<const std::string> args) {
            // validate<bool> does not exist: take a bool as the integer 0 or 1
            if (auto b = xewe::validate<uint8_t>(args[0], 0, 1)) set_active(*b == 1);
            else os.serial.print("Usage: $your_mod_full active <0|1>");
        }});
    register_command({"label", "Set the label (1-15 chars)", "$your_mod_full label \"my lamp\"", 1,
        [this](xewe::span<const std::string> args) {
            // for std::string, min and max are the length bounds
            if (auto s = xewe::validate<std::string>(args[0], 1, 15)) set_label(*s);
            else os.serial.print("Usage: $your_mod_full label <1-15 chars>");
        }});
}

// ---- NVS ----

// A missing blob (fresh device) or a failed decode means defaults. A blob with another schema
// may belong to another firmware (or another version) that used this id on this board: use
// defaults in RAM but never overwrite it; `$your_mod_full reset` wipes it and takes over.
void YourModuleFull::load_settings() {
    YourModuleFullSettings stored;                 // starts as the defaults, schema included
    const bool read = os.nvs.read_flex(id, "settings", stored);
    settings_owned  = stored.schema == YourModuleFullSettings{}.schema;  // still ours if missing
    settings        = (read && settings_owned) ? stored : YourModuleFullSettings{};
    if (!settings_owned)
        os.report_error("! %s: settings schema %u, expected %u: defaults, not saving (reset to take over)",
                        name.c_str(), stored.schema, settings.schema);
}

void YourModuleFull::save_settings() {
    if (!settings_owned) { os.serial.print("Changed for this boot only (foreign settings in NVS)"); return; }
    if (!os.nvs.write_flex(id, "settings", settings))
        os.report_error("! %s: settings not saved (NVS write failed)", name.c_str());
}
