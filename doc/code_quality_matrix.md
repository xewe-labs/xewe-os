# Code Quality Report

| Label   | Meaning                                                |
|---------|--------------------------------------------------------|
| 🟢 high | code logic was rigorously reviewed by a human         |
| 🟡 mid  | code is a mix of human logic and ai implementation     |
| 🔴 low  | code is completely ai generated and may be very sloppy |
| ⚪ none | not rated yet                                          |

Framework code (module base, controller, system, serial, NVS, CLI, utils) now lives in the XeWe
libraries and is rated there. The firmware modules were ported to the XeWeOS library API
(renamed calls, dependencies passed through constructors); their logic is unchanged from the
rated versions, except Wifi, Time, Scheduler and Buttons, which come from the newer xewe-led-os
copies.

| File                                   | Code quality |
| -------------------------------------- | ------------ |
| `./xewe-os.ino`                        | ⚪ none      |
| `./src/Buttons/Buttons.cpp`            | 🟢 high      |
| `./src/Buttons/Buttons.h`              | 🟢 high      |
| `./src/Pins/Pins.cpp`                  | 🟡 mid       |
| `./src/Pins/Pins.h`                    | 🟡 mid       |
| `./src/Scheduler/Scheduler.cpp`        | ⚪ none      |
| `./src/Scheduler/Scheduler.h`          | ⚪ none      |
| `./src/Time/Time.cpp`                  | ⚪ none      |
| `./src/Time/Time.h`                    | ⚪ none      |
| `./src/WebInterface/WebInterface.cpp`  | 🟡 mid       |
| `./src/WebInterface/WebInterface.h`    | 🟡 mid       |
| `./src/Wifi/Wifi.cpp`                  | 🟢 high      |
| `./src/Wifi/Wifi.h`                    | 🟢 high      |
