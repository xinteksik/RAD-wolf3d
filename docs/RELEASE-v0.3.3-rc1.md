# v0.3.3-rc1

Cleaner C64 video and localized pickup/damage feedback, confirmed by the
maintainer on the deployed candidate before packaging.

- Default image: no added dithering, brightness 20/16, exposure 350/256.
- RADWOLF/VIDEO.CFG controls DITHER (0–100), BRIGHTNESS (1–32), EXPOSURE
  (1–512) and FLASH (OFF, FRAME, STATUS, FULL). Reboot after changes.
- FRAME is the default: yellow pickup outline and red damage outline,
  preserving the scene palette. Existing margins are used when available;
  maximum view sizes use a two-pixel inset. Damage takes priority.
- STATUS confines tinting to the HUD; without a HUD it uses FRAME. OFF
  disables pickup/damage flashes. FULL restores the original palette shift.
- Colored solid ceilings retain hue, including the purple secret-level ceiling.
- The map keeps its large red marker and internal white direction indicator.
- P saves a pre-conversion CAPxxxx.RWF under RADWOLF for local comparisons.
- Source tools generate offline HTML comparisons, Koala images and C64 PRGs
  for VICE. Raw captures and generated game-image previews are not distributed.
- README shows the three approved stages of image conversion in compare/,
  alongside the four previously approved real-hardware captures.

Unchanged: MIDI/SoundFont playback, OPL fallback, Windows MIDI preparation,
physical-SID audio requirements, inherited Pi boot settings and DMA/PAL timing.
Visited-map coloring remains planned; visitation is not saved by this release.

Included assets:

1. RAD-Wolf-v0.3.3-rc1-SD.zip: ready-to-copy SD contents, the confirmed kernel,
   Pi boot files, video/audio defaults, Windows MIDI kit and license notices.
2. RAD-Wolf-v0.3.3-rc1-source.tar.gz: complete corresponding application,
   applied pinned dependencies, boot-stub and independent host-converter source.
3. RAD-Wolf-v0.3.3-rc1-GitHub.zip: source-only repository snapshot.
4. SHA256SUMS and validation reports.

Back up before updating. Preserve working clock/GPIO/RAD settings, customized
AUDIO.CFG and VIDEO.CFG, your game configuration and saves. Kernel name remains
kernel_wolf.img. Add your own legally obtained full-version WL6 files under
RADWOLF. Optional MIDI: run Prepare-MIDI.bat in Windows, then add your bank as
soundfont.sf2. The package contains no game data, generated MIDI, saves,
SoundFont, raw captures, compiler or toolchain.

Kernel SHA-256:
7fa75d9ca8bac9d033af3b38ef475356899326286fec197e35218c48fa41915f

The maintainer accepted this deployed candidate on 2026-11-13. This is a
maintainer confirmation, not a complete PAL/NTSC or C64/SID hardware matrix.
Keep the release marked as a pre-release. Ship the binary package with its
complete source archive and checksums; retain all upstream/converter notices.
