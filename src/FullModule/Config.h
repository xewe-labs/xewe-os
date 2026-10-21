// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/FullModule/Config.h
/*
The values of FullModule (id full_module, prefix XEWE_FULL_MODULE_), in the module form of
XeWeCore's doc/config.md. A --define or a line in the project's Config.h wins.
*/
#pragma once

/* ---- start default runtime values ---- */

/* Count ticks (full_module/active), 0 or 1 */
#ifndef XEWE_FULL_MODULE_ACTIVE
#define XEWE_FULL_MODULE_ACTIVE 1
#endif

/* Label shown at boot (full_module/label), string literal, up to 15 characters */
#ifndef XEWE_FULL_MODULE_LABEL
#define XEWE_FULL_MODULE_LABEL "hello"
#endif

/* Tick LED GPIO (full_module/pin), 0 to 255, 255 = none; takes effect after a restart */
#ifndef XEWE_FULL_MODULE_PIN
#define XEWE_FULL_MODULE_PIN 255
#endif

/* Starting level (full_module/level), %, 0 to 100; asked at first boot */
#ifndef XEWE_FULL_MODULE_LEVEL
#define XEWE_FULL_MODULE_LEVEL 50
#define XEWE_FULL_MODULE_LEVEL_PROMPT 1
#endif

/* ---- end default runtime values ---- */

/* ---- start compile time defined constants ---- */

/* Period of the timed action in loop(), ms */
#ifndef XEWE_FULL_MODULE_TICK_MS
#define XEWE_FULL_MODULE_TICK_MS 10000
#endif

/* Print a line on every tick, 0 or 1 */
#ifndef XEWE_FULL_MODULE_PRINT_TICKS
#define XEWE_FULL_MODULE_PRINT_TICKS 0
#endif

/* First-boot prompt timeout per attempt, ms; 0 would wait forever */
#ifndef XEWE_FULL_MODULE_PROMPT_TIMEOUT_MS
#define XEWE_FULL_MODULE_PROMPT_TIMEOUT_MS 30000
#endif

/* ---- end compile time defined constants ---- */
