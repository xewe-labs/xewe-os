# Naming

One brand word, every form derived from it. Enforced by `scripts/brand-lint.sh` in xewe-os-tools.

| Context | Form | Examples |
|---|---|---|
| Prose | **XeWe OS** (product), **XeWe** (brand) | "XeWe OS firmware", "the XeWe tools" |
| Repos, packages, folders, files, CLI, env home | lowercase `xewe`, kebab `xewe-os-*` | `xewe-os-core`, `xewe.toml`, `xewe`, `~/.xewe-os` |
| C++ namespace | `xewe::` | `xewe::Module`, `xewe::str::parse_hex_color` |
| C++ and Python type names | `XeWe` prefix | `XeWeCore`, `XeWeOs`, `XeWeModules`, `XeWeError`, `XeWeContext` |
| Macros, build defines, environment variables | `XEWE_*` | `XEWE_TESTING`, `XEWE_HOME`, `XEWE_DEVICE_NAME` |
| Module ids, CLI groups, NVS namespaces | lowercase slug, never branded | `$wifi`, `$led`, `led_modes` |
| Arduino library name | `XeWeCore` (registered; never changes) | |

Banned spellings: `Xewe` (as a prefix), `XeweError`, `xewe os` (lowercase with a space), `Xewe OS`.
`XeWeOS` is the *old* library name and appears only in migration notes and validator rules.

Product version story: every repo tags the same ecosystem generation (`v3.0.0` next); modules
version together with their repo tag.
