# Module & Command Reference

XeWe OS is built on the [XeWeOS framework](https://github.com/xewe-labs/xewe-library-os), which
provides the serial console, NVS storage, the command line and the `System` module. On top of
that, this firmware adds its own modules. Modules are listed in the order they begin
(their declaration order in `xewe-os.ino`).

<img src="../static/media/resources/readme/system_status.webp" style="max-width:300px;width:100%;height:auto;">

Every module with commands gets `status` and `reset`; modules that can be disabled also get
`enable` and `disable`. Command syntax: `$<module> <command> [args...]`; `$help` lists everything.

## Framework

### Serial console, NVS, command line
**Internal infrastructure** (libraries XeWeSerial, XeWeNvs, XeWeCli)

Non-blocking serial I/O with prompts and tables, persistent settings in ESP32 NVS (each module
stores its data under its own id), and the `$group command args` parser used by the serial
monitor, the web interface, buttons and schedules.

### System
**Prefix:** `$system`

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`status`** | Table of all modules and their state. | `$system status` |
| **`reset`** | Factory reset: resets every module and erases NVS. | `$system reset` |
| **`restart`** / **`reboot`** | Restart the ESP32. | `$system restart` |
| **`info`** | Chip model, revision, IDF version, flash, MAC. | `$system info` |
| **`set_device_name`** | Set the device name (also used as the WiFi hostname). | `$system set_device_name "Kitchen Lights"` |
| **`mac`** | Print the device MAC addresses. | `$system mac` |
| **`uid`** | Device UID from the eFuse base MAC (and SHA256-64). | `$system uid` |

## Firmware modules

### Wifi
**Prefix:** `$wifi` · asks for a network on first boot · can be disabled

Joins a WiFi network (the chosen network is remembered in NVS) and keeps the connection alive.

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`connect`** | Connect or reconnect; prompts for a network if needed. | `$wifi connect` |
| **`disconnect`** | Disconnect from WiFi. | `$wifi disconnect` |
| **`scan`** | List available networks. | `$wifi scan` |

### Web Interface
**Prefix:** `$web_interface` · requires Wifi

HTTP server on port 80 that serves a small page and accepts CLI commands from other devices on
the network (`GET /cmd?c=<command>`).

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`status`** | Server uptime and memory usage. | `$web_interface status` |

### Time
**Prefix:** `$time` · requires Wifi · asks for the timezone on first boot · can be disabled

Synchronises the clock over NTP and detects or stores the timezone.

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`set_zone`** | Set the timezone offset. | `$time set_zone GMT-08:00` |
| **`fetch`** | Sync the current time from the network. | `$time fetch` |

### Scheduler
**Prefix:** `$schedule` · requires Time

Runs stored commands on a weekly schedule. Times are minutes from midnight (0-1439), days are
0 (Monday) to 6 (Sunday); several commands are separated by `|`.

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`add`** | Add a schedule: `<start> <end> <day> <RRGGBB> "<cmd1\|cmd2>"`. | `$schedule add 480 1020 1 FF0000 "$pins gpio_write 8 1"` |
| **`remove`** | Remove a schedule by id. | `$schedule remove 1` |

### Buttons
**Prefix:** `$buttons` · can be disabled

Binds commands to physical buttons with software debouncing.

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`add`** | Add a mapping: `<pin> "<cmd>" <pullup\|pulldown> <on_press\|on_release\|on_change> <debounce_ms>`. | `$buttons add 9 "$system reboot" pullup on_press 50` |
| **`remove`** | Remove a mapping by its id (see `$buttons status`). | `$buttons remove 0` |

### Pins
**Prefix:** `$pins` · can be disabled

Direct hardware access without writing code.

| Command | Description | Sample Usage |
| :--- | :--- | :--- |
| **`gpio_read`** | Read the digital level (0 or 1); configures the pin as INPUT. | `$pins gpio_read <pin>` |
| **`gpio_write`** | Set HIGH (1) or LOW (0); configures the pin as OUTPUT. | `$pins gpio_write <pin> <0\|1>` |
| **`gpio_toggle`** | Invert the current state; forces OUTPUT. | `$pins gpio_toggle <pin>` |
| **`gpio_mode`** | Set the mode: `in`, `out`, `in_pullup`, `in_pulldown`. | `$pins gpio_mode <pin> <mode>` |
| **`adc_read`** | Read the raw ADC value (usually 0-4095). | `$pins adc_read <pin>` |
| **`pwm_setup`** | Attach PWM. Frequency 1 Hz-40 MHz, resolution 1-16 bits. | `$pins pwm_setup <pin> <hz> <bits>` |
| **`pwm_write`** | Set the duty cycle (max `2^bits - 1`). | `$pins pwm_write <pin> <duty>` |
| **`pwm_stop`** | Stop PWM (duty 0) and detach the timer. | `$pins pwm_stop <pin>` |
| **`i2c_scan`** | Init I2C on the given pins and scan 0x01-0x77. | `$pins i2c_scan <sda> <scl>` |
