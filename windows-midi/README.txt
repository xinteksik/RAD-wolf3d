RAD-WOLF: PREPARE MIDI MUSIC ON WINDOWS
=====================================

Requirements: Windows 10/11 with built-in Windows PowerShell 5.1, and your
legitimately obtained full Carmackized Wolfenstein 3D WL6 game files.
No Python, compiler, administrator account or Internet connection is needed.

1. Copy your eight WL6 files into RADWOLF on the SD card, next to
   Prepare-MIDI.bat. When testing the extracted ZIP on your PC, you can place
   them next to the BAT there instead. Required files:
   AUDIOHED.WL6, AUDIOT.WL6, GAMEMAPS.WL6, MAPHEAD.WL6,
   VGADICT.WL6, VGAGRAPH.WL6, VGAHEAD.WL6, VSWAP.WL6.
2. In Windows, double-click Prepare-MIDI.bat. Keep the entire tools folder
   and Prepare-MIDI.ps1 next to it. You may also drag a game-data folder
   onto the BAT to prepare files in that folder.
3. The tool validates the files and all 27 named music chunks, then creates
   MIDI with 27 .MID files and MIDI-INFO.json next to the WL6 files.
   If you prepared them on the PC, copy MIDI to RADWOLF/MIDI on the SD card.
4. Optionally copy a General MIDI SoundFont as RADWOLF/soundfont.sf2.
   Enable Music/AdLib in the game and reboot after changing the font.
   Without a SoundFont, the game uses its original OPL music.

The BAT runs on your Windows PC while the card is inserted, not on the C64.
The firmware does not generate MIDI at boot. Replacing an SF2 does not
require generating MIDI again. Game files, SoundFonts and saves are preserved.

Repeated runs reuse matching MIDI. If input/converter files changed or MIDI
was damaged, a complete new set is prepared before replacing the folder.
The previous MIDI folder is kept as MIDI-backup-<date>-<id>; it is never
deleted by this tool. A failed conversion leaves existing MIDI untouched.
Details of a conversion are written to MIDI-conversion.log next to the BAT.

This kit supports the tested full Carmackized WL6 music layout. Other editions,
WL1 shareware and Spear of Destiny require separate support. It checks file
presence and audio structure, not purchase status or every game-data byte.

LICENSES AND SOURCE
-------------------
The launcher scripts are GPL-3.0-or-later. Their source is the readable PS1/BAT.
tools/dro2midi.exe is compiled independently from ericvids/wolfmidi revision
2cf4c684d45de2a5f0d1b34fd6b9e64beb0c8e79. Full supplied source, instrument
mappings, upstream README and license are retained in source/wolfmidi.
The upstream project declares GPL-2.0-only. Its inherited src/README also
retains older IMF2MIDI terms that disallow paid/retail distribution without
the original author's permission. This free test kit preserves both notices;
do not treat the converter as having unrestricted commercial redistribution.
Build instructions are in source/BUILD.txt. Toolchain runtime notices are
included separately when present. The converter is not part of the firmware.

No Wolfenstein game data, generated game MIDI or SoundFont is included in
the distributed kit. Generated MIDI retains rights associated with the music.
