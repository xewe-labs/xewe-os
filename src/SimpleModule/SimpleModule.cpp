// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/SimpleModule/SimpleModule.cpp
#include "SimpleModule.h"

namespace {
constexpr uint16_t kPromptTries = 2; // bounded first-boot prompts: a headless board still boots
constexpr uint32_t kPromptTimeoutMs = 30000;
} // namespace

SimpleModule::SimpleModule(xewe::Os& host)
    : Module(host,
          /* id                  */ "simple_module",
          /* name                */ "Simple Module",
          /* description         */ "Greets a name on the console",
          /* requires_init_setup */ true,
          /* can_be_disabled     */ true,
          /* has_cli_commands    */ true),
      greet_timer(XEWE_SIMPLE_MODULE_PERIOD_S * 1000UL) {}

xewe::Settings SimpleModule::settings() const {
    static constexpr xewe::SettingDef table[] = {
        xewe::setting<&SimpleModule::who>("name", 15, XEWE_SIMPLE_MODULE_NAME, "Who to greet"),
        xewe::setting<&SimpleModule::period_s>(
            "period_s", 1, 3600, XEWE_SIMPLE_MODULE_PERIOD_S, "Seconds between greetings"),
    };
    return {table, this};
}

void SimpleModule::begin_routines_init() {
#ifdef XEWE_SIMPLE_MODULE_NAME_PROMPT
    std::string answer = os.serial.get_string("Who to greet (Enter for \"" XEWE_SIMPLE_MODULE_NAME
                                              "\", up to 15 characters)?",
        0, 15, kPromptTries, kPromptTimeoutMs, XEWE_SIMPLE_MODULE_NAME);
    apply_setting("name", answer.empty() ? XEWE_SIMPLE_MODULE_NAME : answer, true);
#else
    apply_setting("name", XEWE_SIMPLE_MODULE_NAME);
#endif
#ifdef XEWE_SIMPLE_MODULE_PERIOD_S_PROMPT
    uint16_t seconds = os.serial.get_uint16("Seconds between greetings (1-3600)?", 1, 3600,
        kPromptTries, kPromptTimeoutMs, XEWE_SIMPLE_MODULE_PERIOD_S);
    apply_setting("period_s", std::to_string(seconds), true);
#else
    apply_setting("period_s", std::to_string(XEWE_SIMPLE_MODULE_PERIOD_S));
#endif
}

void SimpleModule::begin_routines_common() {
    greet_timer.reset(period_s * 1000UL, 0, 0);
    greet_timer.initiate();
}

void SimpleModule::loop() {
    if (greet_timer.is_not_done())
        return;
    greet_timer.initiate();
    os.serial.printf("hello %s", who.c_str());
}

void SimpleModule::on_setting_changed(const xewe::SettingDef& def) {
    if (std::string_view(def.key) == "period_s")
        begin_routines_common();
}
