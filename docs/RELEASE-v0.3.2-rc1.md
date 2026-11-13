# v0.3.2-rc1

Graphics and map readability update for the existing PAL C64/RAD setup.

- Solid colors apply specifically to the floor and ceiling, including their
  bonus/damage palette flashes. Wall, door, sprite and weapon pixels retain
  the original texture conversion. This replaces the earlier global gray and
  long-color-run experiments that also altered door textures.
- The F1 map has a large red player marker with a white square inside it.
  Eight positions around its center indicate the viewing direction.
- The README uses four new maintainer-supplied captures from real Ultimate64
  hardware: gameplay, corridor, secret level and the level map.
- MIDI/SoundFont playback, OPL fallback, the Windows MIDI preparation kit,
  physical-SID audio requirements and boot/DMA/PAL settings are retained.

The map still displays the full layout. Visited-cell coloring is planned;
this release does not record or save map visitation.

Assets:

1. RAD-Wolf-v0.3.2-rc1-SD.zip: ready-to-copy card contents, firmware, Pi boot
   files, Windows MIDI preparation kit, configuration and license notices.
2. RAD-Wolf-v0.3.2-rc1-source.tar.gz: corresponding application, pinned applied
   dependency and independent Windows converter sources, with checksums.
3. RAD-Wolf-v0.3.2-rc1-GitHub.zip: source-only repository snapshot.
4. SHA256SUMS: archive integrity checks.

Back up the card before updating. Preserve working config.txt, RAD/rad.cfg,
RADWOLF/AUDIO.CFG, game configuration and saves. Firmware remains
kernel_wolf.img. Defaults remain mono/AUTO; stereo requires actual matching
SID addresses/types. No purchased WL6 data, generated MIDI, saves, SoundFont,
compiler or toolchain is included. Add your own game data and optionally run
RADWOLF/Prepare-MIDI.bat in Windows to generate all 27 music tracks.

The candidate was deployed locally with a complete SD backup and hash-verified
readback. The maintainer supplied hardware captures and reported improved
colors; the latest restriction to floor/ceiling and revised internal direction
indicator still require final visual and performance confirmation on hardware.
Host tests/builds and packaging checks are recorded separately in VALIDATION.
Keep this release marked as a pre-release. Packaging does not write an SD card.

Publish the SD package with its complete source archive and checksums.
The separate converter retains its inherited IMF2MIDI terms and notices;
this free test package does not grant broader rights to game or music assets.
