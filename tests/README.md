# tests/

`unit/` holds pytest files that check source facts on the host, with no board and no build. `board/` holds pytest files that run against a flashed board over serial. `xewe test` runs both.

Open `unit/test_examples.py` and `board/test_examples.py` first; they test the two sample modules.

```sh
build/tools/.venv/bin/python -m xewe test --unit-only
build/tools/.venv/bin/python -m xewe test --chip c3 -- -m board
```

Without a board, the board tests are reported as `compiled, not run` and the command exits 0. [doc/TESTING.md](../doc/TESTING.md) has the flags and the dotenv keys.

The board tests need a provisioned board: the first boot done, then `xewe provision`. `xewe provision` answers `n` for a module outside `XEWE_PROVISION_MODULES`, so a sample module may be enabled or disabled and the tests work in both states. The pin-claim test needs the pins module and `XEWE_TEST_FULL_MODULE_PIN`, a free output GPIO that the module toggles every 10 s; without them it is skipped. Disabling a module wipes its NVS namespace, so the round trip resets the `$full_module` settings to their defaults.
