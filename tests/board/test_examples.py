"""Board tests of the two project-local example modules: `$your_module` (src/YourModule/) and
`$your_mod_full` (src/YourModuleFull/). Run by `xewe test` with every project test.

Hardware preconditions: a provisioned board (first boot done, `xewe provision`). Nothing wired.
`xewe provision` answers "enable?" with n for a module outside XEWE_PROVISION_MODULES, so either
example may be enabled or disabled: every test works in both states and leaves it as it found it.
The settings table commands (`set`/`get`/`schema`) work while a module is disabled.
The pin-claim test needs the pins module and XEWE_TEST_YOUR_MOD_FULL_PIN (a free output GPIO; the
module toggles it every 10 s while enabled); it is skipped otherwise.

Every disable wipes the module's NVS namespace (core Module::reset): the round trip below resets
`$your_mod_full` settings (level, label, token, pin, presets) to their defaults.
"""
import json
import os
import re

import pytest

from xewe.board.serialio import BOOT_READY, BOOT_UNPROVISIONED, wait_for_banner

ID = "your_module"
NAME = "Your Module"
FULL_ID = "your_mod_full"
FULL_NAME = "Your Module Full"
INPUT = r"^(?:\(y/n\) )?> $"   # the input line of a firmware prompt; answer only after it


def _wait(serial, pattern, timeout=90):
    # expect() across the port drop of a native-USB board during a restart
    return wait_for_banner(serial, pattern, timeout, reset=False)


def _booted(serial):
    m = _wait(serial, f"{BOOT_READY}|{BOOT_UNPROVISIONED}")
    assert m[0] == BOOT_READY, "board came back unprovisioned"
    serial.collect(silence=1.0, limit=30)


def _restart(serial):
    serial.send("$system restart")
    serial.expect(r"Rebooting", timeout=10)
    _booted(serial)


def _get(serial, mod, key):
    return serial.command(f"${mod} get {key}", expect=rf"^{key}=(.*)$", timeout=5)[1].strip()


def _schema(serial, mod):
    start = len(serial.lines)
    end = serial.command(f"${mod} schema", expect=rf'\{{"end":"{mod}","count":(\d+)\}}', timeout=10)
    rows = [json.loads(l) for l in serial.lines[start:] if l.startswith("{") and '"end"' not in l]
    assert int(end[1]) == len(rows), f"schema count {end[1]} != {len(rows)} rows"
    return rows


def _enabled(serial, mod, name):
    return serial.command(f"${mod} status", expect=rf"{name} module (enabled|disabled)", timeout=5)[1] == "enabled"


def _pins_present(serial):
    m = serial.command("$pins claims", expect=r"GPIO \d+: |No GPIO claimed|Unknown command group", timeout=5)
    return not m[0].startswith("Unknown")


# ---- $your_module ----

def test_status(serial):
    serial.command(f"${ID} status", expect=rf"{NAME} module (enabled|disabled)", timeout=5)
    serial.expect(r"\bnumber\s*[:=|]\s*\d+", timeout=5)      # one `key: value` line per table row
    serial.expect(r"\bbeat_s\s*[:=|]\s*\d+", timeout=5)


def test_set_get_number(serial):
    before = _get(serial, ID, "number")
    try:
        serial.command(f"${ID} set number 421", expect=r"^number=421$", timeout=5)
        assert _get(serial, ID, "number") == "421"
        serial.command(f"${ID} show", expect=r"number = 421", timeout=5)   # the module's own command
    finally:
        serial.command(f"${ID} set number {before}", expect=rf"^number={before}$", timeout=5)


def test_set_rejects_out_of_range(serial):
    before = _get(serial, ID, "number")
    serial.command(f"${ID} set number 1001", expect=rf"! \${ID} set number: expected u16 in \[0, 1000\]", timeout=5)
    serial.command(f"${ID} set beat_s 0", expect=rf"! \${ID} set beat_s: expected u16 in \[1, 3600\]", timeout=5)
    serial.command(f"${ID} set nope 1", expect=rf"! \${ID}: no setting 'nope'", timeout=5)
    assert _get(serial, ID, "number") == before


def test_schema_lists_table(serial):
    rows = {r["key"]: r for r in _schema(serial, ID)}
    assert list(rows) == ["number", "beat_s"]
    assert (rows["number"]["min"], rows["number"]["max"], rows["number"]["default"]) == (0, 1000, 0)
    assert (rows["beat_s"]["min"], rows["beat_s"]["max"], rows["beat_s"]["default"]) == (1, 3600, 10)


def test_settings_survive_restart(serial):
    # one restart for both modules: a your_module row and a your_mod_full row
    number, level = _get(serial, ID, "number"), _get(serial, FULL_ID, "level")
    try:
        serial.command(f"${ID} set number 777", expect=r"^number=777$", timeout=5)
        serial.command(f"${FULL_ID} set level 63", expect=r"^level=63$", timeout=5)
        _restart(serial)
        serial.command(f"${ID} show", expect=r"number = 777", timeout=5)
        assert _get(serial, FULL_ID, "level") == "63"
    finally:
        serial.command(f"${ID} set number {number}", expect=rf"^number={number}$", timeout=5)
        serial.command(f"${FULL_ID} set level {level}", expect=rf"^level={level}$", timeout=5)


# ---- $your_mod_full ----

def test_full_status(serial):
    serial.command(f"${FULL_ID} status", expect=rf"{FULL_NAME} module (enabled|disabled)", timeout=5)
    serial.expect(r"\blevel\s*[:=|]\s*\d+", timeout=5)
    serial.expect(r"\btoken\s*[:=|]\s*(\*{8})?\s*\|?\s*$", timeout=5)  # SECRET: masked (empty when unset)
    serial.expect(r"\bticks\s*[:=|]\s*\d+", timeout=5)            # added by the module's status()


def test_full_schema_secret_restart_presets(serial):
    rows = _schema(serial, FULL_ID)
    table = {r["key"]: r for r in rows if "group" not in r}
    assert list(table) == ["level", "active", "label", "token", "pin"]
    token = table["token"]
    assert token["secret"] is True and token["value"] == "********" and "default" not in token
    assert table["pin"]["restart"] is True and (table["pin"]["min"], table["pin"]["max"]) == (0, 255)
    assert "restart" not in table["level"]
    presets = [r for r in rows if r.get("group", "").startswith("preset:")]   # schema_extra
    assert [r["group"] for r in presets] == ["preset:0", "preset:1", "preset:2"]
    assert all(r["key"] == "level" and r["min"] == 0 and r["max"] == 100 for r in presets)
    assert presets[1]["set"] == f"${FULL_ID} preset 1 <0-100>"


def test_full_secret_never_printed(serial):
    try:
        serial.command(f"${FULL_ID} set token s3cr3t-xyz", expect=r"^token=\*{8}$", timeout=5)
        start = len(serial.lines)
        serial.command(f"${FULL_ID} get token", expect=r"^token=\*{8}$", timeout=5)
        serial.command(f"${FULL_ID} schema", expect=rf'\{{"end":"{FULL_ID}"', timeout=10)
        assert not any("s3cr3t" in l for l in serial.lines[start:])
    finally:
        serial.command(f"${FULL_ID} set token \"\"", expect=r"^token=", timeout=5)


def test_full_restart_row_says_so(serial):
    pin = _get(serial, FULL_ID, "pin")
    serial.command(f"${FULL_ID} set pin {pin}", expect=rf"^pin={pin}$", timeout=5)   # same value: harmless
    serial.expect(r"Takes effect after \$system restart", timeout=5)
    serial.command(f"${FULL_ID} set pin 256", expect=rf"! \${FULL_ID} set pin: expected u8 in \[0, 255\]", timeout=5)


def test_full_listener_sees_level(serial):
    # the sketch's SketchLevelListener prints every level change (table set -> on_setting_changed)
    level = _get(serial, FULL_ID, "level")
    try:
        serial.command(f"${FULL_ID} set level 41", expect=r"^level=41$", timeout=5)
        serial.expect(r"sketch: level is now 41", timeout=5)
        serial.command(f"${FULL_ID} set level 101", expect=rf"! \${FULL_ID} set level: expected u16 in \[0, 100\]",
                       timeout=5)
    finally:
        serial.command(f"${FULL_ID} set level {level}", expect=rf"^level={level}$", timeout=5)


def test_full_preset_argcounts(serial):
    # one command name, two argument counts; validate<> rejects out of range
    serial.command(f"${FULL_ID} preset 3", expect=rf"Usage: \${FULL_ID} preset <0-2>", timeout=5)
    serial.command(f"${FULL_ID} preset 0 101", expect=rf"Usage: \${FULL_ID} preset <0-2> <0-100>", timeout=5)


def _enable(serial):
    # enable restarts; that boot runs begin_routines_init() (disable wiped init_complete): answer it
    serial.command(f"${FULL_ID} enable", expect=rf"{FULL_NAME} module enabled", timeout=5)
    _wait(serial, r"Starting level \(0-100\)\?")
    _wait(serial, INPUT, 10)
    serial.send("37")
    serial.expect(r"sketch: level is now 37", timeout=10)   # set_level -> listeners, at boot
    _booted(serial)


def _disable(serial):
    serial.command(f"${FULL_ID} disable", expect=r"OK\?", timeout=5)
    serial.expect(INPUT, timeout=5)
    serial.send("y")
    serial.expect(rf"{FULL_NAME} module disabled", timeout=10)
    _wait(serial, r"Rebooting", 10)
    _booted(serial)


def test_full_enable_disable_round_trip(serial):
    # disabled unless provisioning enabled it (provision answers n outside XEWE_PROVISION_MODULES)
    if _enabled(serial, FULL_ID, FULL_NAME):
        _disable(serial)
        assert not _enabled(serial, FULL_ID, FULL_NAME)
        _enable(serial)
        assert _enabled(serial, FULL_ID, FULL_NAME)
    else:
        _enable(serial)
        try:
            assert _enabled(serial, FULL_ID, FULL_NAME)
            serial.command(f"${FULL_ID} enable", expect=rf"{FULL_NAME} module already enabled", timeout=5)
        finally:
            _disable(serial)
        assert not _enabled(serial, FULL_ID, FULL_NAME)
        serial.command(f"${FULL_ID} preset 1", expect=rf"{FULL_NAME} module disabled; to enable:", timeout=5)


def test_full_pin_claim_listed(serial):
    if not _pins_present(serial):
        pytest.skip("needs the pins module ($pins claims)")
    pin = os.environ.get("XEWE_TEST_YOUR_MOD_FULL_PIN")
    if not pin:
        pytest.skip("needs XEWE_TEST_YOUR_MOD_FULL_PIN (a free output GPIO)")
    was_enabled = _enabled(serial, FULL_ID, FULL_NAME)
    if not was_enabled:
        _enable(serial)
    try:
        serial.command(f"${FULL_ID} set pin {pin}", expect=rf"^pin={pin}$", timeout=5)
        _restart(serial)                                        # RESTART row: claimed at the next boot
        start = len(serial.lines)
        serial.command("$pins claims", expect=rf"^GPIO {pin}: {FULL_ID}\b", timeout=5)
        assert any(re.match(rf"GPIO {pin}: {FULL_ID}", l) for l in serial.lines[start:])
    finally:
        serial.command(f"${FULL_ID} set pin 255", expect=r"^pin=255$", timeout=5)
        if was_enabled:
            _restart(serial)
        else:
            _disable(serial)                                    # wipes the pin row and releases the claim
