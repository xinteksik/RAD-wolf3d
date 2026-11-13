# Changes from upstream

This is the initial public source export of the working RAD-Wolf integration.
Complete changed files are included in overrides, with their base and result
hashes. Original copyright/license notices are retained and modifications are
identified at each changed file's beginning.

## Wolf4SDL

- Bare-metal entrypoint and lightweight surface/event adapters.
- Checked FAT-backed data reads and edition/episode handling for tested WL6.
- ARM/newlib compatibility and a 70 Hz clock adapter.
- IJKL controls, persistent key state and forwarding releases consumed by video.
- Native Wolf music/effects using DOSBox DBOPL plus the local SID sample mixer.
- Optional MIDI playback for all 27 full WL6 songs through TinySoundFont at
  44100 Hz, filtered to the 22050 Hz SID stream, with per-song OPL fallback.
  SoundFont presence selects the backend; cached music resumes after the menu.
  The separate wolfmidi converter generates MIDI from the user's own WL6 data.
- Automatic MIDI instrument panning on dual-SID output, respecting authored
  pan and retaining centered bass/drum channels; configurable independently
  of gameplay effects. Direct-menu startup skips the unused title-music snippet.
- Visible menu-loading panel before blocking music/font and menu-sound reads,
  with loading/ready boot milestones instead of a long black startup screen.
- F1 full-level overview during gameplay, with player heading and paused game
  simulation while music continues; no developer/cheat keys enabled.
- Standalone Windows MIDI preparation kit with English BAT/PowerShell messages,
  data validation, hash-based reuse, protected output replacement and backups.
- Per-SID address/type configuration, real mixed-chip stereo, optional stronger
  source positioning and an experimental UltiSID curve.

## RAD-derived integration

- Retains C64 bus transfer, PAL timing, RGB quantization and SID sample output.
- Extracted a small kernel and video adapter into src; no Doom gameplay is linked.
- Removed the RAD demo animation, loading/scroller graphics, intro WAV, external
  soundfont and external blue-noise read. Default ordered dithering is retained.
- Starts the C64/SID directly, remounts storage, then opens Wolf's menu.
- Removes unused WAV decoding and font inclusion from the hardware adapter.
- Preserves required Circle sysconfig overrides; newer stock Circle is not substituted.

## Publication

- Sources, full modified files, dependency lock, license inventory and text SD presets.
- No commercial game data, generated capture, toolchain, boot firmware or kernel binaries.
- Replaced historical smoke-test/patch/staging tools with one hash-checking source installer.
- Data fingerprints are recorded separately from user saves and configuration.
