// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/src/YourModule/Config.h
//
// The values of YourModule, in the module Config.h form of XeWeCore's doc/config.md: id
// your_module, so every name starts with XEWE_YOUR_MODULE_. Pure preprocessor and no includes.
// Each value is a fallback: a --define or a line in the project's Config.h wins. Edit it in
// place; setup never touches a project-local module.
#pragma once

// ---- runtime defaults ----

// The remembered number (your_module/number), 0 to 1000
#ifndef XEWE_YOUR_MODULE_NUMBER
#define XEWE_YOUR_MODULE_NUMBER 0
#endif

// Heartbeat period (your_module/beat_s), s, 1 to 3600
#ifndef XEWE_YOUR_MODULE_BEAT_S
#define XEWE_YOUR_MODULE_BEAT_S 10
#endif

// ---- first-boot values ----

// ---- compile-time constants ----

// Heartbeat: 1 prints the number every beat_s seconds from loop(), 0 keeps quiet
#ifndef XEWE_YOUR_MODULE_HEARTBEAT
#define XEWE_YOUR_MODULE_HEARTBEAT 0
#endif
