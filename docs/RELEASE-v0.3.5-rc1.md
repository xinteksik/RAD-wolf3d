# v0.3.5-rc1

One firmware for Wolfenstein 3D shareware v1.4 WL1 and full v1.4
GT/ID/Activision WL6, with MIDI preparation for both editions.

## What's new

- **Automatic game startup:** the kernel checks the eight runtime files in
  `RADWOLF/` and launches the matching engine. A complete compatible WL6 set
  takes priority when both editions are present; otherwise it uses WL1.
- **Correct graphics and menus:** separate engine variants preserve each
  edition's graphic indices, menus, episodes and gameplay state. RAD video,
  audio and file adapters are shared. Build-time checks audit the callbacks
  between those adapters and the two engines.
- **MIDI for both editions:** Windows `Prepare-MIDI.bat` detects WL1 or WL6.
  It converts 11 shareware songs or 27 full-version songs, checking and
  skipping the 16 named empty shareware slots. Original per-song conversion
  settings are retained; all 27 WL6 results match the previous conversion.
- **Safe repeat conversion:** matching MIDI is reused. A complete new set is
  prepared before replacement, and the previous MIDI folder is kept as a
  backup. Failed conversions leave existing MIDI untouched.
- **Data checking:** `tools/verify_game_data.py` supports both reference sets;
  use `--edition WL1` or `--edition WL6` to select one explicitly.

The persistent F1 map, keyboard fixes, video defaults and SID audio features
from v0.3.4-rc1 remain included. Proven DMA/PAL timing and the boot/RAD
configuration are retained.

## Installation and update

For a new card, extract **RAD-Wolf-v0.3.5-rc1-SD.zip** to the root of a
backed-up FAT32 card. Add your own eight `.WL1` shareware v1.4 files or eight
compatible full-version `.WL6` files to `RADWOLF/`. See
[supported game data](https://github.com/xinteksik/RAD-wolf3d/blob/main/docs/GAME_DATA.md).
No game data is included.

For an existing working card, back it up and replace **kernel_wolf.img**.
Update `RADWOLF/Prepare-MIDI.bat`, `Prepare-MIDI.ps1`, `README.txt`, `tools/`
and the matching converter `source/` from the SD ZIP to obtain the new MIDI
support. Preserve your own `config.txt`, `RAD/rad.cfg`, `AUDIO.CFG`,
`VIDEO.CFG`, `CONFIG.WL1`/`CONFIG.WL6`, saves, MIDI and SoundFont. The boot
selection remains `kernel=kernel_wolf.img`.

Keep one edition in `RADWOLF/` for automatic MIDI preparation. If both audio
sets are present, the Windows tool asks for an explicit `-Edition WL1` or
`-Edition WL6` selection through `Prepare-MIDI.ps1`; the game's automatic
startup still prefers complete compatible WL6 data. To test WL1 gameplay,
move any WL6 data outside `RADWOLF/`. Configuration and saves keep their
edition-specific extension.

Optional MIDI: run `RADWOLF/Prepare-MIDI.bat` on Windows and supply your own
General MIDI bank as `RADWOLF/soundfont.sf2`. Without a SoundFont, music uses
the original OPL path. Enable Music/AdLib in the game. Default audio is mono
with automatic SID1 detection. Supported audio requires a real SID;
Ultimate/UltiSID remains experimental.

## Release assets

- **RAD-Wolf-v0.3.5-rc1-SD.zip**: ready-to-copy SD files, the confirmed
  combined kernel, Pi boot files, configuration, Windows MIDI kit and notices.
- **RAD-Wolf-v0.3.5-rc1-source.tar.gz**: complete corresponding application,
  applied pinned dependencies, boot-stub and independent converter sources.
- **SHA256SUMS**: checksums for the two release archives.

Publish these assets together. The separate GitHub snapshot updates the
repository itself. No game data, saves, generated game MIDI, SoundFonts,
private captures or toolchain binaries are included. The seven previously
approved README illustrations retain their notices in the sources.

## Validation and scope

The maintainer reports working gameplay and MIDI on hardware with WL1 and
then WL6 using the supplied combined kernel. The exact hardware configuration
was not restated for this test; this is a maintainer report of basic operation,
not an exhaustive hardware matrix or a full-game test.

Actual-engine host tests reached the menu, checked keyboard routing, loaded
E1M1 and rendered a frame for each edition. Portable and audio tests passed.
Windows tests exercised actual conversion for both data sets, repeated runs,
Unicode paths, mixed editions, explicit selection and preservation of previous
MIDI on errors. Packaging checks are supplied in VALIDATION.json.

Full-game completion, every level, a 30-minute stability run and exhaustive
hardware compatibility remain unverified. Mark this release as a **pre-release**.

Kernel SHA-256:
`5fa6c721a855c1b9f023887c5de26cf5f371f9ba2d425bbcd2a340e82c718a1c`
