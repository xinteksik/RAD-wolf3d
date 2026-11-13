# Repository Guidelines

## Structure

`src/` contains RAD-Wolf adapters plus the extracted kernel/video renderer.
`overrides/` contains complete modified upstream files, not an entire engine
checkout. `UPSTREAM.lock.json` records original repositories and revisions;
`overrides/manifest.json` records original and modified file hashes. Generated
sources and binaries belong in ignored `upstream/` and `build/` directories.
Text-only SD presets are in `sd/` and `config/`.

## Development commands

Run `python3 tools/prepare_sources.py --apps-only` for host development or
omit the flag to prepare firmware dependencies. `make test` runs portable C
and Python checks. `make test-opl test-audio-startup` exercises the actual
upstream sound module with synthetic data. `bash tools/build_dependencies.sh`
builds Circle/Newlib; `bash tools/build_firmware.sh` builds the kernel. Use a
Linux path without spaces and the documented AArch64 compiler.

## Style and changes

Use C11/C++11, four spaces, descriptive snake_case and the `radwolf_` public
prefix. Python tools use pathlib and must be safe to repeat. Preserve author
notices. Update override result hashes after editing complete modified files;
never silently reset upstream checkouts or overwrite unknown local changes.

## Hardware and publication

Preserve proven DMA/PAL timing and back up an SD card before deployment.
Never eject the user's card without their instruction. Distinguish host tests
from observed hardware results. Real SIDs are required for supported audio;
UltiSID remains experimental.

Use focused imperative commits and describe test commands, chip mapping and
firmware hash in PRs. Never publish runtime game data, saves, raw captures,
toolchains or binaries without their required source/license package. The four
documented README screenshots are an explicit maintainer-supplied exception;
retain their separate image notices.
Run `python3 tools/audit_release.py` before preparing a publication archive.
