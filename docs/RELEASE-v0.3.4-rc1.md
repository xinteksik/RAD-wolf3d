# v0.3.4-rc1

Persistent map exploration, useful map markers and complete keyboard input in
menu prompts and save-name entry. This release preserves the exact kernel
accepted by the maintainer on 2026-11-14.

## What's new

- **Menu keyboard:** Y/N answer confirmation prompts, including save overwrite.
  Type save names directly with letters, numbers and spaces; INST/DEL erases,
  RETURN accepts and the C64 left-arrow key cancels. Gameplay shortcuts are
  suspended while typing a name.
- **Persistent exploration:** traversed floor is green on the F1 map. The full
  level layout remains visible. The route is saved with the game and restored
  on load. Historical saves load normally and start tracking at the restored
  position; they contain no earlier traversal history.
- **Player marker:** a red 3x3 square of map dots, with exactly one white dot
  on its perimeter indicating one of eight directions. The black halo was
  removed so it no longer covers neighboring walls or the path.
- **Exit:** the elevator switch is light blue, with a BLUE: EXIT legend.
- **Keys:** uncollected gold/silver keys appear as yellow/white map tiles.
  Picked-up keys disappear and stay absent after saving/loading.
- **Missing-key doors:** orange means a gold key is needed; white means a
  silver key is needed. The warning disappears when the required key is owned
  or the door is fully open. Ordinary doors remain yellow.
- Keys and lock warnings occupy only their own map tile. The map renderer
  decorates a private copy and does not alter collision or game state.

F1 opens/closes the map; the C64 left-arrow key also closes it. Simulation
pauses while the map is open and music continues.

## Installation and update

For a new card, extract **RAD-Wolf-v0.3.4-rc1-SD.zip** directly to a backed-up
FAT32 card root. Add your own eight full-version Carmackized WL6 data files to
RADWOLF/. No game data is included.

For an existing working installation, back up the card and replace only
**kernel_wolf.img** for a firmware-only update. Preserve config.txt,
RAD/rad.cfg, customized AUDIO.CFG/VIDEO.CFG, CONFIG.WL6, saves, MIDI and your
SoundFont. Kernel selection remains `kernel=kernel_wolf.img`.

Optional MIDI: run RADWOLF/Prepare-MIDI.bat in Windows and add your own bank as
RADWOLF/soundfont.sf2. Without a bank, original OPL music is used. Default audio
is mono with automatic SID1 detection. Supported audio requires a real SID;
Ultimate/UltiSID remains experimental. Boot, DMA/PAL timing and the existing
video/audio defaults are retained.

Exploration uses an optional versioned, CRC32-checked 528-byte save trailer.
Missing, partial or corrupt optional history does not invalidate the original
game save. Saving again with an older kernel discards exploration history.

## Release assets

- **RAD-Wolf-v0.3.4-rc1-SD.zip** - ready-to-copy SD files, accepted kernel,
  Pi boot files, configuration, Windows MIDI kit and license notices.
- **RAD-Wolf-v0.3.4-rc1-source.tar.gz** - complete corresponding application,
  applied pinned dependencies, boot-stub and independent converter sources.
- **SHA256SUMS** - checksums for the two release archives.

Publish the SD ZIP together with its corresponding source archive and notices.
The separate GitHub snapshot is for updating the repository itself.
No commercial game data, saves, generated MIDI, SoundFonts, private captures
or compiler/toolchain binaries are included. The seven previously approved
README illustrations and their existing notices are retained in the sources.

## Validation and scope

The current kernel was accepted by the maintainer. Traversal/save-load and
historical-save behavior were explicitly reported on hardware; key/lock map
annotations and halo removal were subsequently accepted. Detailed host cases
and packaging checks are recorded in VALIDATION.md and VALIDATION.json.

A 30-minute stability run, level-transition reset and an exhaustive hardware
matrix are not claimed by this release. Keep **This is a pre-release** checked.

Kernel SHA-256:
`ce00a903b8aa9698e123c2df496aa033b8c76da05de8d152a664857a91994222`
