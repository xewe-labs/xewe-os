// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModule/YourModule.cpp
//
// Project-local module: rename or delete this folder; it is yours, setup never touches it.
// `$your_module disable` asks "OK?" (two tries, 15 s each; no clear yes = cancelled). On yes it
// wipes the module's NVS namespace, marks it disabled and restarts; while disabled loop() does
// not run and set_number() refuses. `$your_module enable` brings it back.
#include "YourModule.h"

YourModule::YourModule(xewe::Os& host, YourModuleConfig config)
    : Module(host,                      // registers this module with the Os, right here
          /* id                  */ "your_module", // command group ($your_module ...) and NVS namespace; <= 15 chars
          /* name                */ "Your Module",
          /* description         */ "Remembers one number in NVS",
          /* requires_init_setup */ false,         // true would run begin_routines_init() once
          /* can_be_disabled     */ true,          // adds $your_module enable / disable
          /* has_cli_commands    */ true)          // adds $your_module status / reset, allows our own
    , config(config) {
    // `$your_module set number 42` / `get number` / `schema` come from the table below.
    // $your_module show: our own command, no arguments; the Cli checks the count.
    register_command({"show", "Print the stored number", "$your_module show", 0,
        [this](xewe::span<const std::string>) {
            os.serial.printf("number = %u", number);
        }});
}

xewe::Settings YourModule::settings() const {
    // one row per setting: key (= NVS key, <= 15 chars), min, max, default, doc; checked at build time
    static constexpr xewe::SettingDef table[] = {
        xewe::setting<&YourModule::number>("number", 0, 1000, 0, "The remembered number"),
        xewe::setting<&YourModule::beat_s>("beat_s", 1, 3600, 10, "Heartbeat period, s"),
    };
    return {table, this};
}

void YourModule::loop() {
    if (!config.heartbeat || millis() - last_beat_ms < beat_s * 1000UL) return;   // non-blocking timer
    last_beat_ms = millis();
    os.serial.printf("your_module: number is %u", number);
}

std::string YourModule::status(const bool verbose) const {
    // compose, don't replace: the base prints "enabled" and one `key: value` line per table row
    std::string s = Module::status(false);
    if (verbose) os.serial.print(s);
    return s;
}

void YourModule::set_number(uint16_t value) {
    if (is_disabled(true)) return;      // disabled modules stay callable; refuse politely
    apply_setting("number", std::to_string(value), true);   // validated (0-1000), saved, printed
}
