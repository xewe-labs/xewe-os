// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModuleFull/Config.h
//
// The values of YourModuleFull, in the module Config.h form of XeWeCore's doc/config.md: id
// your_mod_full, so every name starts with XEWE_YOUR_MOD_FULL_. Pure preprocessor and no
// includes. Each value is a fallback: a --define or a line in the project's Config.h wins. Edit
// it in place; setup never touches a project-local module.
#pragma once

// ---- runtime defaults ----

// Count ticks (your_mod_full/active), 0 or 1
#ifndef XEWE_YOUR_MOD_FULL_ACTIVE
#define XEWE_YOUR_MOD_FULL_ACTIVE 1
#endif

// Label shown at boot (your_mod_full/label), string literal, up to 15 characters
#ifndef XEWE_YOUR_MOD_FULL_LABEL
#define XEWE_YOUR_MOD_FULL_LABEL "hello"
#endif

// Tick LED GPIO (your_mod_full/pin), 0 to 255, 255 = none; takes effect after a restart
#ifndef XEWE_YOUR_MOD_FULL_PIN
#define XEWE_YOUR_MOD_FULL_PIN 255
#endif

// ---- first-boot values ----

// Starting level (your_mod_full/level), %, 0 to 100; suggested at first boot
#ifndef XEWE_YOUR_MOD_FULL_LEVEL
#define XEWE_YOUR_MOD_FULL_LEVEL 50
#define XEWE_YOUR_MOD_FULL_LEVEL_PROMPT 1
#endif

// ---- compile-time constants ----

// Period of the timed action in loop(), ms
#ifndef XEWE_YOUR_MOD_FULL_TICK_MS
#define XEWE_YOUR_MOD_FULL_TICK_MS 10000
#endif

// Print a line on every tick, 0 or 1
#ifndef XEWE_YOUR_MOD_FULL_PRINT_TICKS
#define XEWE_YOUR_MOD_FULL_PRINT_TICKS 0
#endif

// First-boot prompt timeout per attempt, ms; 0 would wait forever
#ifndef XEWE_YOUR_MOD_FULL_PROMPT_TIMEOUT_MS
#define XEWE_YOUR_MOD_FULL_PROMPT_TIMEOUT_MS 30000
#endif
