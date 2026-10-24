# src/

This folder holds the project's own modules, one folder per module. The build turns each folder into an `xewe::Module` registered with the sketch, and writes the generated, git-ignored `src/Modules.h` that lists them.

Open [SimpleModule/](SimpleModule/README.md) first: it is the smallest complete module and the one to copy. [FullModule/](FullModule/README.md) overrides every hook once. [doc/WRITING_A_MODULE.md](../doc/WRITING_A_MODULE.md) explains the contract.
