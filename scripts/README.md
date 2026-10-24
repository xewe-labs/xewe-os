# scripts/

`release.sh` tests the firmware once, tags the commit with the release notes and the release card in the tag message, and waits for CI to build the images and publish the GitHub Release.

```sh
scripts/release.sh 3.2.0 --notes "what changed"
```

The version is `X.Y.Z` or `X.Y.Z-suffix` for a pre-release, with no `v`. The notes are one to three lines: `--notes` once per line, and the lines of `--notes-file`. Everything a run writes goes under `build/release/<version>/`, which is git-ignored. `--hardware c3` adds build, flash and board tests for that chip. `--ci-timeout <min>` bounds the CI wait (default 60).

## Stages

1. Preflight lists every problem before it refuses. It requires the `main` branch with a clean work tree (untracked files included), `main` level with `origin/main` after a fetch, and a tag that is free here and on origin. It also requires `ci.yml` to call the tools workflows at the `[tools]` ref of `xewe.toml` (a tag, unless `XEWE_TOOLS_SOURCE` is set), notes that pass `xewe release-notes check`, no tracked file under `static/`, no active `INJECTED_FROM_DOTENV` line in `Config.h`, an ignored `build/`, Python 3.11 or newer, and a logged-in `gh`.
2. Tests set the version lines, then record each step with `xewe release-card record`: `./setup.sh`, the checkers, `xewe format --check`, `xewe brand-lint .`, `xewe modules validate` and `xewe test --unit-only`. With `--hardware C` they also build, flash and run the board tests for chip `C` on a board that has had its first boot (`xewe provision --chip C`). Flashing keeps NVS, so do not erase it. Without `--hardware` the board tests are compiled, not run, for the `xewe.toml` chip, and CI builds every other chip. A failing step does not stop the others; the card refuses it.
3. Release card: `xewe release-card render` writes `release_card.md` from those results, or refuses.
4. Commit: only the version files are added, as `release <version>`. Any other changed file is a refusal.
5. Tag: the message is the notes, a blank line and the card. Then `main` is pushed, `origin/main` is checked against `HEAD`, and the tag is pushed.
6. Release CI: the script waits for `ci.yml` on the tag. Its release job builds every image, checks the card against its own runs and creates the GitHub Release. The script then checks that the card check passed and that every asset is there, and prints the release URL.

A run that stops before stage 4 puts the version lines back. `--resume` pushes what is missing and runs stage 6 when the tag exists. When `HEAD` is the `release <version>` commit and there is no tag, it reuses `build/release/<version>/` if the card is there, and continues from stage 3.

Exit code 0 means done and 1 means a refusal or a failed check.

## Environment

- `XEWE_CORE_SOURCE`, `XEWE_MODULES_SOURCE` and `XEWE_TOOLS_SOURCE` point to local checkouts and are passed to `./setup.sh`.
- `XEWE_RELEASE_DEFINES_<CHIP>` holds space-separated `KEY=VALUE` defines for that chip's steps.
- `XEWE_ENV` names the dotenv file for the board steps (default `.env`). The tools read it; the script never does.

## In a project made from this template

A project keeps a copy of the script and changes two lines: `REPO`, its GitHub repository, and `BOARD_GATES`, the gated board tests its card session runs. The name in `xewe.toml` `[project]` must equal the repository name in `REPO`.
