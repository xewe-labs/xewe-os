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

    void begin_routines_init() override;
    void begin_routines_common() override;
    void loop() override;
    xewe::Settings settings() const override;

protected:
    void on_setting_changed(const xewe::SettingDef& def) override;

private:
    std::string who;
    uint16_t period_s = XEWE_SIMPLE_MODULE_PERIOD_S;
    xewe::AsyncTimer<uint8_t> greet_timer;
};
