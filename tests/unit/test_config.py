"""Unit tests of the project's Config.h (the core's doc/config.md): source facts only, no board and
no build. The tools append module blocks to this file and never rewrite it, so the template ships it
in the shape they expect."""

import re
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]  # the project: tests/unit/ -> tests/ -> root
CORE_START = "// ---- core (xewe modules generate) ----"
CORE_END = "// ---- end core ----"
SECTIONS = (
    "// ---- runtime defaults ----",
    "// ---- first-boot values ----",
    "// ---- compile-time constants ----",
)
SENTINEL = re.compile(r"^\s*#\s*define\s+\w+\s+INJECTED_FROM_DOTENV", re.M)


def _lines():
    return (ROOT / "Config.h").read_text().splitlines()


@pytest.mark.unit
def test_pragma_once_and_no_include():
    lines = _lines()
    assert "#pragma once" in lines
    assert not [l for l in lines if re.match(r"\s*#\s*include\b", l)]


@pytest.mark.unit
def test_core_block_and_sections_in_order():
    lines = _lines()
    assert lines.count(CORE_START) == 1 and lines.count(CORE_END) == 1
    start, end = lines.index(CORE_START), lines.index(CORE_END)
    assert start < end
    positions = [lines.index(marker) for marker in SECTIONS]
    assert all(lines.count(marker) == 1 for marker in SECTIONS)
    assert start < positions[0] < positions[1] < positions[2] < end


@pytest.mark.unit
def test_first_boot_values_are_commented_out():
    # a fresh project still asks at first boot: every first-boot line ships as a comment
    lines = _lines()
    first, last = lines.index(SECTIONS[1]), lines.index(SECTIONS[2])
    active = [
        l
        for l in lines[first + 1 : last]
        if l.strip() and not l.lstrip().startswith("//")
    ]
    assert not active, active


@pytest.mark.unit
def test_no_prompt_build_macro_or_sentinel():
    text = (ROOT / "Config.h").read_text()
    # only a module's own Config.h defines <NAME>_PROMPT
    assert "_PROMPT" not in text
    # build macros come from the generated XeWeBuildInfo.h, never from this file
    assert not re.search(r"\b(BUILD|PROJECT)_[A-Z0-9_]+", text)
    # a release image takes no value from a dotenv file
    assert not SENTINEL.search(text)
