"""Unit tests of the two project-local example modules (src/YourModule/, src/YourModuleFull/):
source facts only, no board and no build. They have no module.properties; the sketch declares them."""
import re
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]          # the project: tests/unit/ -> tests/ -> root
EXAMPLES = {                                         # folder: (id, name, sketch variable)
    "YourModule": ("your_module", "Your Module", "your_module"),
    "YourModuleFull": ("your_mod_full", "Your Module Full", "your_module_full"),
}
ID_ARG = re.compile(r'/\*\s*id\s*\*/\s*"([^"]*)"')
NAME_ARG = re.compile(r'/\*\s*name\s*\*/\s*"([^"]*)"')
SETTING_KEY = re.compile(r'xewe::setting<[^>]*>\s*\(\s*"([^"]*)"')


def _cpp(folder):
    return (ROOT / "src" / folder / f"{folder}.cpp").read_text()


@pytest.mark.unit
@pytest.mark.parametrize("folder", sorted(EXAMPLES))
def test_sketch_declares_example(folder):
    ino = (ROOT / "xewe-os.ino").read_text()
    _, _, var = EXAMPLES[folder]
    assert f'#include "src/{folder}/{folder}.h"' in ino
    assert re.search(rf"^{folder}\s+{var}\s*\(\s*os\b", ino, re.M), f"{folder} {var}(os...) not declared"
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
    assert all(1 <= len(k) <= 15 for k in keys), keys       # NVS key limit


@pytest.mark.unit
def test_full_table_has_secret_and_restart_rows():
    cpp = _cpp("YourModuleFull")
    assert re.search(r'setting<[^>]*>\s*\(\s*"token".*SettingDef::SECRET', cpp)
    assert re.search(r'setting<[^>]*>\s*\(\s*"pin".*SettingDef::RESTART', cpp)


@pytest.mark.unit
def test_readme_mentions_examples():
    readme = (ROOT / "README.md").read_text()
    for folder, (mod_id, _, _) in EXAMPLES.items():
        assert f"src/{folder}/" in readme and f"${mod_id}" in readme
