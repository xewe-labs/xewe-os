// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os/Config.h
/*
 * Uncomment #define line and change the value to override the default.
 */
#pragma once

/************************/
/* ---- start core ---- */
/************************/
/* ---- start default runtime values ---- */

/* Device name (system/device_name), string literal, up to 254 characters; asked at first boot */
// #define XEWE_CORE_DEVICE_NAME "xewe-device"

/* ---- end default runtime values ---- */

/* ---- start compile time defined constants ---- */

/* Serial baud rate (SerialPortConfig::baud_rate), bits per second */
// #define XEWE_CORE_SERIAL_BAUD 115200

/* Serial TX buffer (SerialPortConfig::tx_buffer_size), bytes, set before Serial.begin */
// #define XEWE_CORE_SERIAL_TX_BUF 2048

/* Serial RX buffer (SerialPortConfig::rx_buffer_size), bytes; one input burst must fit */
// #define XEWE_CORE_SERIAL_RX_BUF 1024

/* Delay after opening the port (SerialPortConfig::startup_delay_ms), ms; 0 on a real UART */
// #define XEWE_CORE_SERIAL_DELAY_MS 1000

/* Echo typed characters back (SerialPortConfig::echo), 0 or 1 */
// #define XEWE_CORE_SERIAL_ECHO 1

/* Serial line buffer (SerialPort::INPUT_BUFFER_SIZE), bytes including the terminator, 2 to 256 */
// #define XEWE_CORE_INPUT_BUF_SIZE 255

/* Completed input lines queued (SerialPort::INPUT_QUEUE_LINES), 1 to 255 */
// #define XEWE_CORE_INPUT_QUEUE 4

/* AsyncTimer progress recompute interval, ms; 0 recomputes on every call */
// #define XEWE_CORE_TIMER_TICK_MS 5

/* ---- end compile time defined constants ---- */

/**********************/
/* ---- end core ---- */
/**********************/
