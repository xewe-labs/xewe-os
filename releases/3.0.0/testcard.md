# xewe-os 3.0.0 test card

What was tested, where, on which commits, and what was not. The template is a sketch and two
example modules on top of XeWeCore and the modules; their own tests are in the
[XeWeCore 3.0.0 test card](https://github.com/xewe-labs/xewe-os-core/blob/3.0.0/releases/3.0.0/testcard.md),
the modules' per-module cards (`modules/<module>/releases/3.0.0/testcard.md` in
[xewe-os-modules](https://github.com/xewe-labs/xewe-os-modules/tree/3.0.0)) and the
[tools 3.0.0 test card](https://github.com/xewe-labs/xewe-os-tools/blob/3.0.0/releases/3.0.0/testcard.md).

| Check | Where | Result | Not covered |
|---|---|---|---|
| Host tests, `xewe test --no-board` | Linux VM | 8 unit passed (`tests/unit/test_examples.py`); 13 board tests compiled, not run (`tests/board/test_examples.py`); 0 failed | the board tests on a board |
| Build c3 (`xewe build`, no modules selected) | Linux VM | 386758 bytes (18%), 0 warnings | |
| Build c6 | Linux VM | 363720 bytes (17%), 0 warnings | |
| Build s3 | Linux VM | 376282 bytes (17%), 0 warnings | |
| `xewe release --version 3.0.0` | Linux VM | `static/firmware/releases/3.0.0/{release_notes.txt,firmware_map.csv}`, `<chip>/{3.0.0-<chip>-xewe-os.bin,manifest.json,meta.json}` for c3, c6, s3, `firmware-3.0.0.tar.gz` | |
| `./setup.sh` with the frozen refs | Linux VM | core 3.0.0, modules 3.0.0 (none selected), tools from the 3.0.1 tree; a second run keeps the tools tag checkout | tools 3.0.1 fetched by its tag (not yet tagged then; the 3.0.0 tag was fetched and installed the same way) |
| `xewe format --check`, `xewe brand-lint .` | Linux VM | 6 C++ and 2 Python files, 0 to change; clean | |
| CI (`ci.yml`) | GitHub, `ubuntu-latest` | build and host tests for c3, c6, s3 and blocking lint on the prepare commit; the release job on the tag | |
| Board tests | none | not run | the 13 example board tests have never run on a board; the firmware is not flashed by any test of this release |
| Modules | none in the template | `selected = []`: no module code in these binaries | each module through its own card |

The format commit changed 5 of 6 C++ files and both Python files; every C++ file has the same
token stream before and after (comments compared by their words) and every Python file the same
AST.

## Environment

- Linux aarch64 VM, Python 3.14.7, arduino-cli 1.5.1, esp32 core 3.3.12, `XEWE_NO_BOARD=1`.
- Formatters: clang-format 23.1.3, ruff 0.17.0, as pinned in the workflows.

## Commit

- The 3.0.0 tree: 3756ed6 plus the work, format and prepare changes (refs, CI, release flow,
  this folder). The CI run of the prepare commit and of the tag are the record for that commit.
