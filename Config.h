// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/Config.h
//
// Build settings. `xewe build` generates <XeWeBuildInfo.h> (PROJECT_NAME, BUILD_VERSION,
// BUILD_TIMESTAMP, BUILD_CHIP and every `--define KEY=VALUE`); it wins over the defaults below.
// The version lives in xewe.lock [project]; do not edit it here.
#pragma once

// Unconditional on purpose: arduino-cli only adds a library to the build when an #include of it
// fails, so a __has_include() guard would silently drop the generated header. Plain Arduino IDE
// (no xewe): delete this line; the defaults below then apply.
#include <XeWeBuildInfo.h>

#ifndef PROJECT_NAME
#define PROJECT_NAME "xewe-os"
#endif

#ifndef BUILD_VERSION
#define BUILD_VERSION "0.0.0"
#endif

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

#ifndef BUILD_CHIP
#define BUILD_CHIP "unknown"
#endif

// Printed in the boot header. Override: xewe build --define 'PROJECT_URL="https://..."'
#ifndef PROJECT_URL
#define PROJECT_URL "https://github.com/xewe-labs/xewe-os"
#endif

// Serial console baud rate (the tools' `xewe serial` defaults to 115200 as well).
#ifndef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 115200
#endif
