// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/SimpleModule/Config.h
/*
The values of SimpleModule (id simple_module, prefix XEWE_SIMPLE_MODULE_), in the module form of
XeWeCore's doc/config.md. A --define or a line in the project's Config.h wins.
*/
#pragma once

/* ---- start default runtime values ---- */

/* Who to greet (simple_module/name), string literal, up to 15 characters; asked at first boot */
#ifndef XEWE_SIMPLE_MODULE_NAME
#define XEWE_SIMPLE_MODULE_NAME "world"
#define XEWE_SIMPLE_MODULE_NAME_PROMPT 1
#endif

/* Seconds between greetings (simple_module/period_s), 1 to 3600; asked at first boot */
#ifndef XEWE_SIMPLE_MODULE_PERIOD_S
#define XEWE_SIMPLE_MODULE_PERIOD_S 10
#define XEWE_SIMPLE_MODULE_PERIOD_S_PROMPT 1
#endif

/* ---- end default runtime values ---- */

/* ---- start compile time defined constants ---- */

/* ---- end compile time defined constants ---- */
