// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModule/YourModule.cpp
//
// Project-local module: rename or delete this folder; it is yours, setup never touches it.
// `$your_module disable` asks "OK?" (two tries, 15 s each; no clear yes = cancelled). On yes it
// wipes the module's NVS namespace, marks it disabled and restarts; while disabled loop() does
// not run and `$your_module set` refuses. `$your_module enable` brings it back.
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
    // $your_module set <n>: one argument; the Cli checks the count and prints the usage line otherwise.
    register_command({"set", "Store a number (0-1000) in NVS", "$your_module set 42", 1,
        [this](xewe::span<const std::string> args) {
            // validate<T>(text, min, max) parses and range-checks; empty optional on bad input
            if (auto n = xewe::validate<uint16_t>(args[0], 0, 1000)) set_number(*n);
            else os.serial.print("Usage: $your_module set <0-1000>");
        }});

    // $your_module show: no arguments.
    register_command({"show", "Print the stored number", "$your_module show", 0,
        [this](xewe::span<const std::string>) {
            os.serial.printf("number = %u", number);
        }});
}

void YourModule::begin_routines_common() {
    // NVS namespace == module id; the third argument is the value when nothing is stored yet
    number = os.nvs.read<uint16_t>(id, "number", 0);
}

void YourModule::loop() {
    if (!config.heartbeat || millis() - last_beat_ms < 10000) return;   // non-blocking timer
    last_beat_ms = millis();
    os.serial.printf("your_module: number is %u", number);
}

std::string YourModule::status(const bool verbose) const {
    // compose, don't replace: keep the base line (enabled, ...) and add our own state
    std::string s = Module::status(false) + ", number " + std::to_string(number);
    if (verbose) os.serial.print(s);
    return s;
}

void YourModule::set_number(uint16_t value) {
    if (is_disabled(true)) return;      // disabled modules stay callable; refuse politely
    number = value;
    os.nvs.write<uint16_t>(id, "number", number);   // survives reboots until disable/reset
    os.serial.printf("number = %u (saved)", number);
}
