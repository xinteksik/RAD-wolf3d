# Building RAD-Wolf

Use a Linux filesystem and a path without spaces, for example `~/src/rad-wolf`.
Do not build Circle/Newlib directly inside a Windows path containing spaces.
Host tools: Git, Python 3.9+, Make, GCC/G++, standard build utilities and the
dependencies required by the pinned circle-stdlib configure script.

## Pinned compiler and sources

The working hardware baseline used Arm GNU-A `10.2-2020.11`,
`aarch64-none-elf-gcc 10.2.1 20201103`, targeting Cortex-A53, AArch64.
Compiler archive SHA-256:
`32abfbc7b24c56542f2a6e6969d6b8787e47f7223e8f2097d84151ebd9f86743`.
[Original Arm download](https://developer.arm.com/-/media/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz).
Extract it yourself and put its `bin/` directory on PATH. It is not redistributed.

Exact repository revisions are in `UPSTREAM.lock.json`. Circle is **Step44.3**,
not the latest release. RAD's required `sysconfig.h` is installed from the
included override before building: `NO_SD_HIGH_SPEED`, REALTIME and the other
RAD settings matter. A stock Circle configuration previously produced SD read
errors or a black screen. Preserve the supplied settings.

```sh
python3 tools/prepare_sources.py
bash tools/build_dependencies.sh
bash tools/build_firmware.sh
```

The first command fetches pinned upstreams and copies complete modified files
from `overrides/`, checking base and result hashes. It refuses unknown local
edits or a different HEAD; it never resets a checkout. Repeating it is safe.
`--offline` verifies an already prepared tree without network access.
`--apps-only` fetches only RAD-Doom/Wolf4SDL for host tests, not firmware.

Dependency builds use `STDLIB_SUPPORT=3`, RASPPI=3 and AARCH=64. Do not use
`--no-cpp`; the link needs the C++ runtime. Several GB of temporary disk space
may be needed. `JOBS=2` controls build parallelism.

The final kernel is `build/kernel_wolf.img`. The portable link makefile is
`firmware/Makefile`; a prepared compatible circle-stdlib tree can be selected
with `STDLIB_ROOT=/path/to/circle-stdlib`. A compiler/library change is a new
build candidate, not a verified hardware equivalent.

## Validation

```sh
make test
make test-opl test-audio-startup
python3 tools/verify_game_data.py /path/to/your/WL6/files
python3 tools/audit_release.py
```

Host tests do not establish VIC-II timing, physical SID quality or SD boot.
This cleaned build removes the intro and external media dependencies; its
hardware boot status is recorded separately in STATUS.md. Hashes of earlier
verified kernels are references, not promises of byte-for-byte reproducibility
across paths or toolchains. Build outputs are deliberately ignored by Git.
