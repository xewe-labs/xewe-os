// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/Config.h
//
// Build settings and the compile-time values of the core and the selected modules. `xewe build`
// puts three headers in front of every source file: the generated XeWeBuildInfo.h first (version,
// timestamp, chip), this file second, the generated XeWeOverrides.h last (dotenv values and
// `--define`s). A first-boot value is commented out: uncomment it to pre-answer the question, or
// set it to INJECTED_FROM_DOTENV to take it from the dotenv file. A plain Arduino IDE build reads
// none of this and uses the defaults of the core and the modules. The version lives in xewe.toml
// [project]; do not set it here.
#pragma once

// ---- core (xewe modules generate) ----
// Values of the core. Edit the numbers; keep this marker and the end marker.
// ---- runtime defaults ----

// Serial baud rate (SerialPortConfig::baud_rate), bits per second
#ifndef XEWE_CORE_SERIAL_BAUD
#define XEWE_CORE_SERIAL_BAUD 115200
#endif

// Serial TX buffer (SerialPortConfig::tx_buffer_size), bytes, set before Serial.begin
#ifndef XEWE_CORE_SERIAL_TX_BUF
#define XEWE_CORE_SERIAL_TX_BUF 2048
#endif

// Serial RX buffer (SerialPortConfig::rx_buffer_size), bytes; one input burst must fit
#ifndef XEWE_CORE_SERIAL_RX_BUF
#define XEWE_CORE_SERIAL_RX_BUF 1024
#endif

// Delay after opening the port (SerialPortConfig::startup_delay_ms), ms; 0 on a real UART
#ifndef XEWE_CORE_SERIAL_DELAY_MS
#define XEWE_CORE_SERIAL_DELAY_MS 1000
#endif

// Echo typed characters back (SerialPortConfig::echo), 0 or 1
#ifndef XEWE_CORE_SERIAL_ECHO
#define XEWE_CORE_SERIAL_ECHO 1
#endif

// Print the ASCII banner at boot (OsConfig::print_banner), 0 or 1
#ifndef XEWE_CORE_PRINT_BANNER
#define XEWE_CORE_PRINT_BANNER 1
#endif

// ---- first-boot values ----

// Device name (system/device_name), string literal, up to 254 characters; suggested at first boot
// #define XEWE_CORE_DEVICE_NAME "xewe-os"

// ---- compile-time constants ----

// Serial line buffer (SerialPort::INPUT_BUFFER_SIZE), bytes including the terminator, 2 to 256
#ifndef XEWE_CORE_INPUT_BUF_SIZE
#define XEWE_CORE_INPUT_BUF_SIZE 255
#endif

// Completed input lines queued (SerialPort::INPUT_QUEUE_LINES), 1 to 255
#ifndef XEWE_CORE_INPUT_QUEUE
#define XEWE_CORE_INPUT_QUEUE 4
#endif

// AsyncTimer progress recompute interval, ms; 0 recomputes on every call
#ifndef XEWE_CORE_TIMER_TICK_MS
#define XEWE_CORE_TIMER_TICK_MS 5
#endif
// ---- end core ----

// Module values. `./setup.sh` and `xewe modules select` append the values of every selected module
// below, one marked block per module (`// ---- <slug> (xewe modules generate) ----` to
// `// ---- end <slug> ----`). Edit the numbers; keep the markers. A block is added once and never
// rewritten.
