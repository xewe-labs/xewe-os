"""Board tests of $simple_module and $full_module on a provisioned board; see tests/README.md."""

import json
import os
import re
from contextlib import contextmanager

import pytest

from xewe.board.serialio import BOOT_READY, BOOT_UNPROVISIONED, wait_for_banner

ID = "simple_module"
NAME = "Simple Module"
FULL_ID = "full_module"
FULL_NAME = "Full Module"
INPUT = (
    r"^(?:\(y/n\) )?> $"  # the input line of a firmware prompt; answer only after it
)


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
    return serial.command(f"${mod} get {key}", expect=rf"^{key}=(.*)$", timeout=5)[
        1
    ].strip()


def _schema(serial, mod):
    start = len(serial.lines)
    end = serial.command(
        f"${mod} schema", expect=rf'\{{"end":"{mod}","count":(\d+)\}}', timeout=10
    )
    rows = [
        json.loads(l)
        for l in serial.lines[start:]
        if l.startswith("{") and '"end"' not in l
    ]
    assert int(end[1]) == len(rows), f"schema count {end[1]} != {len(rows)} rows"
    return rows


def _enabled(serial, mod, name):
    return (
        serial.command(
            f"${mod} status", expect=rf"{name} module (enabled|disabled)", timeout=5
        )[1]
        == "enabled"
    )


def _pins_present(serial):
    m = serial.command(
        "$pins claims",
        expect=r"GPIO \d+: |No GPIO claimed|Unknown command group",
        timeout=5,
    )
    return not m[0].startswith("Unknown")


def _simple_enable(serial, name="world", period="10"):
    # enable restarts; that boot runs begin_routines_init(): answer both questions
    serial.command(f"${ID} enable", expect=rf"{NAME} module enabled", timeout=5)
    _wait(serial, r"Who to greet")
    _wait(serial, INPUT, 10)
    serial.send(name)
    serial.expect(rf"^name={name}$", timeout=10)
    _wait(serial, r"Seconds between greetings \(1-3600\)\?", 10)
    _wait(serial, INPUT, 10)
    serial.send(period)
    serial.expect(rf"^period_s={period}$", timeout=10)
    _booted(serial)


@contextmanager
def _simple_enabled(serial):
    # enabled for the block; disabled again afterwards when it was disabled before
    was_enabled = _enabled(serial, ID, NAME)
    if not was_enabled:
        _simple_enable(serial)
    try:
        yield
    finally:
        if not was_enabled:
            _disable(serial, ID, NAME)


def test_status(serial):
    if not _enabled(serial, ID, NAME):
        # disabled: no greeting, even with the shortest period
        period = _get(serial, ID, "period_s")
        try:
            serial.command(f"${ID} set period_s 1", expect=r"^period_s=1$", timeout=5)
            assert not [
                l for l in serial.collect(silence=3.0, limit=3.0) if "hello" in l
            ]
        finally:
            serial.command(
                f"${ID} set period_s {period}",
                expect=rf"^period_s={period}$",
                timeout=5,
            )
    with _simple_enabled(serial):
        serial.command(f"${ID} status", expect=rf"{NAME} module enabled", timeout=5)
        serial.expect(
            r"\bname\s*[:=|]\s*\S*", timeout=5
        )  # one `key: value` line per table row
        serial.expect(r"\bperiod_s\s*[:=|]\s*\d+", timeout=5)


def test_set_get_period_and_name(serial):
    period, name = _get(serial, ID, "period_s"), _get(serial, ID, "name")
    try:
        serial.command(f"${ID} set period_s 77", expect=r"^period_s=77$", timeout=5)
        assert _get(serial, ID, "period_s") == "77"
        serial.command(f"${ID} set name bench", expect=r"^name=bench$", timeout=5)
        assert _get(serial, ID, "name") == "bench"
    finally:
        serial.command(
            f"${ID} set period_s {period}", expect=rf"^period_s={period}$", timeout=5
        )
        serial.command(f"${ID} set name {name}", expect=rf"^name={name}$", timeout=5)


def test_set_rejects_out_of_range(serial):
    period = _get(serial, ID, "period_s")
    serial.command(
        f"${ID} set period_s 0",
        expect=rf"! \${ID} set period_s: expected u16 in \[1, 3600\]",
        timeout=5,
    )
    serial.command(
        f"${ID} set period_s 3601",
        expect=rf"! \${ID} set period_s: expected u16 in \[1, 3600\]",
        timeout=5,
    )
    serial.command(
        f"${ID} set name abcdefghijklmnop",
        expect=rf"! \${ID} set name: expected str of 0-15 chars",
        timeout=5,
    )
    serial.command(
        f"${ID} set nope 1", expect=rf"! \${ID}: no setting 'nope'", timeout=5
    )
    assert _get(serial, ID, "period_s") == period


def test_schema_lists_table(serial):
    rows = {r["key"]: r for r in _schema(serial, ID)}
    assert list(rows) == ["name", "period_s"]
    assert (rows["name"]["min"], rows["name"]["max"], rows["name"]["default"]) == (
        0,
        15,
        "world",
    )
    assert (
        rows["period_s"]["min"],
        rows["period_s"]["max"],
        rows["period_s"]["default"],
    ) == (1, 3600, 10)


def test_greeting(serial):
    # period_s 1: the next greeting is at most a second away and carries the new name
    with _simple_enabled(serial):
        period, name = _get(serial, ID, "period_s"), _get(serial, ID, "name")
        try:
            serial.command(f"${ID} set name bench", expect=r"^name=bench$", timeout=5)
            serial.command(f"${ID} set period_s 1", expect=r"^period_s=1$", timeout=5)
            serial.expect(r"^hello bench$", timeout=5)
            serial.expect(r"^hello bench$", timeout=5)  # and again, a second later
        finally:
            serial.command(
                f"${ID} set period_s {period}",
                expect=rf"^period_s={period}$",
                timeout=5,
            )
            serial.command(
                f"${ID} set name {name}", expect=rf"^name={name}$", timeout=5
            )


def test_settings_survive_restart(serial):
    # one restart for both modules: a simple_module row and a full_module row
    period, level = _get(serial, ID, "period_s"), _get(serial, FULL_ID, "level")
    try:
        serial.command(f"${ID} set period_s 777", expect=r"^period_s=777$", timeout=5)
        serial.command(f"${FULL_ID} set level 63", expect=r"^level=63$", timeout=5)
        _restart(serial)
        assert _get(serial, ID, "period_s") == "777"
        assert _get(serial, FULL_ID, "level") == "63"
    finally:
        serial.command(
            f"${ID} set period_s {period}", expect=rf"^period_s={period}$", timeout=5
        )
        serial.command(
            f"${FULL_ID} set level {level}", expect=rf"^level={level}$", timeout=5
        )


def test_full_status(serial):
    serial.command(
        f"${FULL_ID} status",
        expect=rf"{FULL_NAME} module (enabled|disabled)",
        timeout=5,
    )
    serial.expect(r"\blevel\s*[:=|]\s*\d+", timeout=5)
    serial.expect(
        r"\btoken\s*[:=|]\s*(\*{8})?\s*\|?\s*$", timeout=5
    )  # SECRET: masked (empty when unset)
    serial.expect(r"\bticks\s*[:=|]\s*\d+", timeout=5)  # added by the module's status()


def test_full_schema_secret_restart_presets(serial):
    rows = _schema(serial, FULL_ID)
    table = {r["key"]: r for r in rows if "group" not in r}
    assert list(table) == ["level", "active", "label", "token", "pin"]
    token = table["token"]
    assert (
        token["secret"] is True
        and token["value"] == "********"
        and "default" not in token
    )
    assert table["pin"]["restart"] is True and (
        table["pin"]["min"],
        table["pin"]["max"],
    ) == (0, 255)
    assert "restart" not in table["level"]
    presets = [
        r for r in rows if r.get("group", "").startswith("preset:")
    ]  # schema_extra
    assert [r["group"] for r in presets] == ["preset:0", "preset:1", "preset:2"]
    assert all(
        r["key"] == "level" and r["min"] == 0 and r["max"] == 100 for r in presets
    )
    assert presets[1]["set"] == f"${FULL_ID} preset 1 <0-100>"


def test_full_secret_never_printed(serial):
    try:
        serial.command(
            f"${FULL_ID} set token s3cr3t-xyz", expect=r"^token=\*{8}$", timeout=5
        )
        start = len(serial.lines)
        serial.command(f"${FULL_ID} get token", expect=r"^token=\*{8}$", timeout=5)
        serial.command(
            f"${FULL_ID} schema", expect=rf'\{{"end":"{FULL_ID}"', timeout=10
        )
        assert not any("s3cr3t" in l for l in serial.lines[start:])
    finally:
        serial.command(f'${FULL_ID} set token ""', expect=r"^token=", timeout=5)


def test_full_restart_row_says_so(serial):
    pin = _get(serial, FULL_ID, "pin")
    serial.command(
        f"${FULL_ID} set pin {pin}", expect=rf"^pin={pin}$", timeout=5
    )  # same value: harmless
    serial.expect(r"Takes effect after \$system restart", timeout=5)
    serial.command(
        f"${FULL_ID} set pin 256",
        expect=rf"! \${FULL_ID} set pin: expected u8 in \[0, 255\]",
        timeout=5,
    )


def test_full_listener_sees_level(serial):
    # the sketch's SketchLevelListener prints every level change (table set -> on_setting_changed)
    level = _get(serial, FULL_ID, "level")
    try:
        serial.command(f"${FULL_ID} set level 41", expect=r"^level=41$", timeout=5)
        serial.expect(r"sketch: level is now 41", timeout=5)
        serial.command(
            f"${FULL_ID} set level 101",
            expect=rf"! \${FULL_ID} set level: expected u16 in \[0, 100\]",
            timeout=5,
        )
    finally:
        serial.command(
            f"${FULL_ID} set level {level}", expect=rf"^level={level}$", timeout=5
        )


def test_full_preset_argcounts(serial):
    # one command name, two argument counts; validate<> rejects out of range
    serial.command(
        f"${FULL_ID} preset 3", expect=rf"Usage: \${FULL_ID} preset <0-2>", timeout=5
    )
    serial.command(
        f"${FULL_ID} preset 0 101",
        expect=rf"Usage: \${FULL_ID} preset <0-2> <0-100>",
        timeout=5,
    )


def _enable(serial):
    # enable restarts; that boot runs begin_routines_init() (disable wiped init_complete): answer it
    serial.command(
        f"${FULL_ID} enable", expect=rf"{FULL_NAME} module enabled", timeout=5
    )
    _wait(serial, r"Starting level \(0-100\)\?")
    _wait(serial, INPUT, 10)
    serial.send("37")
    serial.expect(
        r"sketch: level is now 37", timeout=10
    )  # set_level -> listeners, at boot
    _booted(serial)


def _disable(serial, mod=FULL_ID, name=FULL_NAME):
    serial.command(f"${mod} disable", expect=r"OK\?", timeout=5)
    serial.expect(INPUT, timeout=5)
    serial.send("y")
    serial.expect(rf"{name} module disabled", timeout=10)
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
            serial.command(
                f"${FULL_ID} enable",
                expect=rf"{FULL_NAME} module already enabled",
                timeout=5,
            )
        finally:
            _disable(serial)
        assert not _enabled(serial, FULL_ID, FULL_NAME)
        serial.command(
            f"${FULL_ID} preset 1",
            expect=rf"{FULL_NAME} module disabled; to enable:",
            timeout=5,
        )


def test_full_pin_claim_listed(serial):
    if not _pins_present(serial):
        pytest.skip("needs the pins module ($pins claims)")
    pin = os.environ.get("XEWE_TEST_FULL_MODULE_PIN")
    if not pin:
        pytest.skip("needs XEWE_TEST_FULL_MODULE_PIN (a free output GPIO)")
    was_enabled = _enabled(serial, FULL_ID, FULL_NAME)
    if not was_enabled:
        _enable(serial)
    try:
        serial.command(f"${FULL_ID} set pin {pin}", expect=rf"^pin={pin}$", timeout=5)
        _restart(serial)  # RESTART row: claimed at the next boot
        start = len(serial.lines)
        serial.command("$pins claims", expect=rf"^GPIO {pin}: {FULL_ID}\b", timeout=5)
        assert any(re.match(rf"GPIO {pin}: {FULL_ID}", l) for l in serial.lines[start:])
    finally:
        serial.command(f"${FULL_ID} set pin 255", expect=r"^pin=255$", timeout=5)
        if was_enabled:
            _restart(serial)
        else:
            _disable(serial)  # wipes the pin row and releases the claim
