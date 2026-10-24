"""Source checks of the two sample modules; they have no module.properties, the sketch declares them."""

import re
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]  # the project: tests/unit/ -> tests/ -> root
EXAMPLES = {  # folder: (id, display name, define prefix)
    "SimpleModule": ("simple_module", "Simple Module", "XEWE_SIMPLE_MODULE_"),
    "FullModule": ("full_module", "Full Module", "XEWE_FULL_MODULE_"),
}
SECTIONS = [
    "/* ---- start default runtime values ---- */",
    "/* ---- end default runtime values ---- */",
    "/* ---- start compile time defined constants ---- */",
    "/* ---- end compile time defined constants ---- */",
]
ID_ARG = re.compile(r'/\*\s*id\s*\*/\s*"([^"]*)"')
NAME_ARG = re.compile(r'/\*\s*name\s*\*/\s*"([^"]*)"')
SETTING_KEY = re.compile(r'xewe::setting<[^>]*>\s*\(\s*"([^"]*)"')
DEFINE = re.compile(r"^#define\s+(\w+)\s+(.*)$", re.M)


def _cpp(folder):
    return (ROOT / "src" / folder / f"{folder}.cpp").read_text()


def _config(folder):
    return (ROOT / "src" / folder / "Config.h").read_text()


def _defines(folder):
    return dict(DEFINE.findall(_config(folder)))


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_sketch_declares_example(folder):
    ino = (ROOT / "xewe-os.ino").read_text()
    mod_id = EXAMPLES[folder][0]
    assert f'#include "src/{folder}/{folder}.h"' in ino
    assert re.search(rf"^{folder}\s+{mod_id}\s*\(\s*os\s*\)\s*;", ino, re.M), (
        f"{folder} {mod_id}(os); not declared"
    )
    assert (ROOT / "src" / folder / f"{folder}.h").is_file()


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_id_and_name_match_source(folder):
    mod_id, name, _ = EXAMPLES[folder]
    cpp = _cpp(folder)
    assert ID_ARG.findall(cpp) == [mod_id]
    assert NAME_ARG.findall(cpp) == [name]
    # the id is the CLI group and the NVS namespace: at most 15 characters, lower-case
    assert re.fullmatch(r"[a-z][a-z0-9_]{0,14}", mod_id)


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_setting_keys_fit_nvs(folder):
    keys = SETTING_KEY.findall(_cpp(folder))
    assert keys, "no settings table rows found"
    assert len(keys) == len(set(keys))
    assert all(1 <= len(k) <= 15 for k in keys), keys  # NVS key limit


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_config_h_module_form(folder):
    mod_id, _, prefix = EXAMPLES[folder]
    text = _config(folder)
    lines = text.splitlines()
    assert lines[2] == f"// xewe-os/src/{folder}/Config.h"
    assert "#pragma once" in lines
    assert not re.search(r"^\s*#\s*include\b", text, re.M)
    assert [l for l in lines if l in SECTIONS] == SECTIONS
    assert all(name.startswith(prefix) for name in _defines(folder)), _defines(folder)
    # every runtime default names its settings key as <id>/<key>
    runtime = text.split(SECTIONS[0])[1].split(SECTIONS[1])[0]
    keys = set(re.findall(rf"\({mod_id}/(\w+)\)", runtime))
    assert keys == set(SETTING_KEY.findall(_cpp(folder))) - {"token"}


@pytest.mark.unit
def test_simple_module_values_and_prompts():
    d = _defines("SimpleModule")
    assert d == {
        "XEWE_SIMPLE_MODULE_NAME": '"world"',
        "XEWE_SIMPLE_MODULE_NAME_PROMPT": "1",
        "XEWE_SIMPLE_MODULE_PERIOD_S": "10",
        "XEWE_SIMPLE_MODULE_PERIOD_S_PROMPT": "1",
    }
    assert SETTING_KEY.findall(_cpp("SimpleModule")) == ["name", "period_s"]
    cpp = _cpp("SimpleModule")
    assert "#ifdef XEWE_SIMPLE_MODULE_NAME_PROMPT" in cpp
    assert "#ifdef XEWE_SIMPLE_MODULE_PERIOD_S_PROMPT" in cpp
    assert "delay(" not in cpp


@pytest.mark.unit
def test_full_module_values_and_prompt():
    d = _defines("FullModule")
    assert d == {
        "XEWE_FULL_MODULE_ACTIVE": "1",
        "XEWE_FULL_MODULE_LABEL": '"hello"',
        "XEWE_FULL_MODULE_PIN": "255",
        "XEWE_FULL_MODULE_LEVEL": "50",
        "XEWE_FULL_MODULE_LEVEL_PROMPT": "1",
        "XEWE_FULL_MODULE_TICK_MS": "10000",
        "XEWE_FULL_MODULE_PRINT_TICKS": "0",
        "XEWE_FULL_MODULE_PROMPT_TIMEOUT_MS": "30000",
    }
    assert "#ifdef XEWE_FULL_MODULE_LEVEL_PROMPT" in _cpp("FullModule")


@pytest.mark.unit
def test_full_table_has_secret_and_restart_rows():
    cpp = _cpp("FullModule")
    assert re.search(r'setting<[^>]*>\s*\(\s*"token".*SettingDef::SECRET', cpp)
    assert re.search(r'setting<[^>]*>\s*\(\s*"pin".*SettingDef::RESTART', cpp)


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_sample_readme_mentions_example(folder):
    mod_id, _, prefix = EXAMPLES[folder]
    readme = (ROOT / "src" / folder / "README.md").read_text()
    assert f"${mod_id}" in readme and prefix in readme
