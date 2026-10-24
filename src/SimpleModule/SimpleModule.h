// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/SimpleModule/SimpleModule.h
//
// The smallest complete module: greets a name on the console every period_s seconds.
#pragma once

#include <string>

#include <XeWeCore.h>

#include "Config.h"

class SimpleModule : public xewe::Module {
public:
    explicit SimpleModule(xewe::Os& host);

    void begin_routines_init() override; // first boot: ask the name and the period
    void begin_routines_common() override; // every boot: start the greeting timer
    void loop() override; // prints the greeting when the timer is done
    xewe::Settings settings() const override; // name, period_s

protected:
    void on_setting_changed(const xewe::SettingDef& def) override; // period_s: restart the timer

private:
    std::string who; // row "name"
    uint16_t period_s = XEWE_SIMPLE_MODULE_PERIOD_S; // row "period_s"
    xewe::AsyncTimer<uint8_t> greet_timer;
};
