# v0.3.1-rc1

Test release for full Carmackized WL6 data and physical SID audio.

- Optional MIDI/SoundFont music for all 27 tracks, with original OPL fallback.
- 44100 Hz synthesis and a low-pass FIR before the existing 22050 Hz SID output.
- Configurable dual-SID MIDI instrument placement; bass/drums centered and
  authored MIDI/SF2 panorama retained. No SID/DMA/PAL timing change.
- Direct-menu music start and visible menu-loading panel.
- F1 full-level map, including unvisited rooms, with player direction; F1 or
  C64 left-arrow returns to gameplay. Music continues while viewing it.
- Windows `Prepare-MIDI.bat` and an independent static converter inside
  `RADWOLF/` on the card; creates MIDI from the user's own WL6 files on the PC.

Assets for a GitHub release:

1. `RAD-Wolf-v0.3.1-rc1-SD.zip`: ready-to-copy SD card contents, including Pi
   boot files, `kernel_wolf.img`, the Windows MIDI kit, defaults and notices.
2. `RAD-Wolf-v0.3.1-rc1-source.tar.gz`: complete firmware/dependency and host-tool
   sources. No compiler, game data, generated soundtrack or SoundFont is included.
3. `SHA256SUMS`: archive integrity checks.
4. `RAD-Wolf-v0.3.1-rc1-GitHub.zip`: source-only repository update.

Back up the card and preserve its working clock/GPIO/RAD settings. The test
image is `kernel_wolf.img`, as in the first release. The supplied boot and audio
defaults match the original release's mono/AUTO setup. Preserve working custom
configuration and saves when updating a card; stereo examples are supplied too.
See WINDOWS-MIDI.md for setup. The kernel fingerprint is in RELEASE.json.

The preceding MIDI stereo build was reported working on real C64 hardware.
Host tests passed; this combined map/loading-screen test candidate still needs
hardware confirmation. Source delivery retains pinned dependency working
sources and host-converter notices, including inherited IMF2MIDI conditions.
This is a free test package, not a claim of unrestricted commercial rights to
the independent converter or user-supplied game/music/font assets.
