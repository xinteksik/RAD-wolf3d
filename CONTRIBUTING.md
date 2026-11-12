# Contributing

Read the build, audio and licensing documents before changing the hardware
path. Keep game files, saves, generated checkouts and toolchains out of Git.
New glue is C11/C++11; use four-space indentation, `snake_case` and the
`radwolf_` public prefix. Preserve upstream author/license notices.

Run `make test`, plus `make test-opl test-audio-startup` with prepared sources.
Changes to an upstream override must update its post-change SHA-256 in
`overrides/manifest.json` while retaining the original base hash and revision.
Source installation must stay idempotent and refuse unknown local edits.

Do not change proven DMA/PAL timing to solve an unrelated problem. Test on
a backed-up SD card. Record Pi/C64 model, SID mapping/types, exact firmware
hash, audio config, commands run and what was actually seen/heard. Separate
host checks from hardware claims; a clean link is not a boot test.

Use short imperative commit subjects. Pull requests should describe the
problem, changed behavior, test commands and hardware status. Do not attach
commercial assets to issues or PRs. Link upstream work and preserve licensing
when importing code or data tables.
