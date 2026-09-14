# Project Structure

XeWe OS is firmware assembled from the XeWe libraries plus its own modules.

```text
xewe-os/
├── xewe-os.ino                     # Assembly: ModuleController + firmware modules, in begin order
├── Config.h                        # PROJECT_NAME, BUILD_VERSION, BUILD_TIMESTAMP (written by build.sh)
│
├── src/                            # Firmware modules, one folder per class
│   ├── Wifi/Wifi.{h,cpp}           # WiFi connection ($wifi)
│   ├── WebInterface/…              # HTTP page + command endpoint ($web_interface), requires Wifi
│   ├── Time/Time.{h,cpp}           # NTP time and timezone ($time), requires Wifi
│   ├── Scheduler/Scheduler.{h,cpp} # Weekly command schedules ($schedule), requires Time
│   ├── Buttons/Buttons.{h,cpp}     # Commands bound to physical buttons ($buttons)
│   └── Pins/Pins.{h,cpp}           # GPIO, ADC, PWM, I2C ($pins)
│
├── build/
│   ├── libraries/
│   │   └── required_libraries.txt  # Library repos and pinned tags, cloned by the setup script
│   ├── release_matrix.csv          # Board/config rows built by release.sh
│   ├── scripts/{mac,linux,windows} # Setup, build, upload, serial monitor, release
│   └── tools/code_formatter/       # Formatter used by format.sh
│
├── doc/                            # Module reference, adding a module, this file
└── static/                         # Released firmware (web flasher) and README media
```

## Where things come from

| Piece | Source |
|---|---|
| Module base, controller, `$system` | [xewe-library-os](https://github.com/xewe-labs/xewe-library-os) (`XeWeOS`) |
| Serial console, prompts, tables | [xewe-library-serial](https://github.com/xewe-labs/xewe-library-serial) (`XeWeSerial`) |
| NVS storage, FlexData | [xewe-library-nvs](https://github.com/xewe-labs/xewe-library-nvs) (`XeWeNvs`) |
| `$group command args` parser | [xewe-library-cli](https://github.com/xewe-labs/xewe-library-cli) (`XeWeCli`) |
| String, validation, timer, debug helpers | [xewe-library-utils](https://github.com/xewe-labs/xewe-library-utils) (`XeWeUtils`) |
| Wifi, WebInterface, Time, Scheduler, Buttons, Pins | this repo (`src/`) |

The versions used by a build are the tags in `build/libraries/required_libraries.txt`.
