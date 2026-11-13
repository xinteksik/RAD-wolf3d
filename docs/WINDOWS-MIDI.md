# Windows MIDI preparation and test package

The `v0.3.1-rc1` package provides a new MIDI-capable kernel plus an
independent Windows converter. Unzip the entire package before running it.

## Prepare your card

1. Back up the working SD card. The SD ZIP includes the same compatible Pi
   boot files and RAD prefetch stub as v0.3.0-rc1, with retained notices.
2. Extract the CONTENTS of the SD ZIP to the card root. `config.txt` and
   `kernel_wolf.img` must be directly in that root. The default selection is
   `kernel=kernel_wolf.img`. When updating an existing card, preserve your
   working clock/GPIO/PAL settings, AUDIO.CFG, game configuration and saves.
3. Put your eight full-version WL6 files into `RADWOLF/` beside
   `Prepare-MIDI.bat`. You can also test preparation in the extracted package
   on the PC before copying to a card. Files placed beside the BAT are found
   automatically; dragging a game-data directory onto it is supported too.
4. Double-click `Prepare-MIDI.bat` in Windows. It runs built-in Windows
   PowerShell and the supplied converter, validates all required files and
   27 named audio chunks, and creates `MIDI/` in the game-data directory.
   No Python, compiler, admin account or Internet access is required.
5. Optionally supply a General MIDI bank as `RADWOLF/soundfont.sf2`. Boot the
   C64/RAD and enable Music/AdLib in Sound. Without a font the game uses OPL.

The BAT runs on Windows, including directly on a card inserted into the PC.
It does not run on C64/RAD. The firmware does not generate MIDI at boot.
Changing SoundFonts does not require another music conversion; reboot to load
the new font. All 27 converted songs use the same selected bank.

During gameplay, F1 opens a paused full-level overview with walls, doors and
your position/direction. Music continues. F1 or the C64 left-arrow key closes
it. This is a full layout, including unvisited rooms, rather than a discovered
automap; enemies and items are not shown. Other developer/debug keys are not
enabled by this feature. Hardware map/input testing is pending for this build.

## Safety and repeat runs

The tool checks all eight files exist and are nonempty, validates the WL6 audio
header/chunk bounds and expected song names, and verifies each converted MIDI
track structure. It supports the tested full Carmackized WL6 layout; it is
not a universal detector for every Wolf/Spear/mod edition or proof of purchase.

Original WL6, saves, audio config and fonts are untouched. Matching generated
MIDI is reused based on input/tool/output hashes. A damaged or stale set is
replaced only after all 27 conversions succeed; the old directory remains in
`MIDI-backup-<date>-<id>`. Failed conversions leave existing MIDI unchanged.
`MIDI-conversion.log` contains the conversion details.

Paths with spaces, ampersands and Unicode have been exercised through the
actual BAT under Windows PowerShell. Native converter arguments use generated
ASCII filenames in a working directory to avoid its ANSI-path limitations.
Only standard Windows OS DLLs are imported by the statically linked converter.

## Source and licenses

The source-only GitHub ZIP is separate from the test binary ZIP. The latter
includes the corresponding firmware source snapshot, upstream dependency
sources, and the complete host-converter source/notices. Toolchain binaries,
WL6, generated game MIDI and SoundFonts are excluded from distribution.
The converter is an independent program, not part of the firmware executable.

wolfmidi declares GPL-2.0-only; its inherited src/README also retains older
IMF2MIDI restrictions on paid/retail distribution. Both are preserved in the
kit. This free test package does not claim unrestricted commercial permission.
The launcher is GPL-3.0-or-later; compiler runtime notices are retained too.
See LICENSING.md for the firmware component inventory.

To rebuild only the host kit, use a pinned wolfmidi checkout and MinGW-w64:

```sh
python tools/build_windows_midi.py --wolfmidi /path/to/wolfmidi --compiler /path/to/g++.exe --output build/windows-midi-kit
```

The generated kit contains exact build instructions and the supplied upstream
source. Generated executables remain outside the source-only GitHub tree.
