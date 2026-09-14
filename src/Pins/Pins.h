// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/Pins/Pins.h
#pragma once

#include <XeWeOS.h>

#include <Wire.h>


class Pins : public xewe::os::Module {
public:
    explicit                    Pins                        (xewe::os::ModuleController& controller);

private:
    void                        gpio_read_cli               (std::span<const std::string> args);
    void                        gpio_write_cli              (std::span<const std::string> args);
    void                        gpio_toggle_cli             (std::span<const std::string> args);
    void                        gpio_mode_cli               (std::span<const std::string> args);
    void                        adc_read_cli                (std::span<const std::string> args);
    void                        pwm_setup_cli               (std::span<const std::string> args);
    void                        pwm_write_cli               (std::span<const std::string> args);
    void                        pwm_stop_cli                (std::span<const std::string> args);
    void                        i2c_scan_cli                (std::span<const std::string> args);
};
