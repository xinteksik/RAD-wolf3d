# Validation status

## v0.3.4-rc1 - release preparation, 2026-11-15

Packages preserve the maintainer-accepted kernel `ce00a903b8aa9698e123c2df496aa033b8c76da05de8d152a664857a91994222`.
Menu keyboard, persistent exploration, the 3x3 marker, exit/key/lock colors and
halo removal are included. Hardware evidence is the maintainer's reports in
the sections below; see RELEASE-v0.3.4-rc1.md for release scope. Public packages
exclude the new private Ultimate64 captures. Long stability and level-transition
reset remain open; this is a pre-release.


## v0.3.3-rc1 — maintainer accepted, 2026-11-13

The maintainer accepted the deployed no-dither/FLASH=FRAME candidate and
requested packaging. Kernel SHA-256:
`7fa75d9ca8bac9d033af3b38ef475356899326286fec197e35218c48fa41915f`.
This records acceptance of that binary, separately from the host tests and
earlier historical candidates below. No exhaustive hardware matrix is claimed.
The release preserves the confirmed binary rather than substituting a new
untested rebuild. Source, package and Windows MIDI checks are in VALIDATION.

## MIDI test release — 2026-11-13

`v0.3.1-rc1` includes all-song MIDI/SF2 music, 44100-to-22050 Hz FIR
resampling, separate MIDI instrument placement for dual SIDs, visible menu
loading feedback, the Windows MIDI preparation kit, and an F1 full-level map.
The latest kernel hash and byte count are in RELEASE.json. Host tests and
firmware linking are verified; this complete candidate needs the maintainer's
hardware test, especially map entry/exit and gameplay pause/resume.

The maintainer confirmed the preceding MIDI stereo variant on a real C64,
and reports successful playback with multiple SF2 banks. This does not claim
the new map/loading-screen combination has already been hardware checked.
The actual Windows BAT was tested with private WL6 inputs: successful full
conversion, repeat reuse, damaged-output regeneration with backup, Unicode/
space/ampersand paths, external-folder arguments, missing/corrupt inputs and
converter failure. All 27 outputs matched the existing Linux conversion hashes.

For `v0.3.1-rc1`, the maintainer subsequently copied the release to an actual
SD card, supplied their WL6 files and SF2 bank, and ran Prepare-MIDI.bat directly
from that card in Windows. This real-card preparation workflow was reported
successful on 2026-11-13. The report did not specifically identify new-kernel
boot or F1 map entry/exit; those remain separate hardware checks.

The sections below retain historical bring-up and publication results.

## Hardware baseline

The maintainer reported a working menu, game launch and two played levels
with sound. IJKL input release and port-1 interference fixes were confirmed.
The maintainer also confirms testing on a **real Commodore 64C** and an
**Ultimate64 Elite2**, with physical SID audio. This platform report does not
identify a separate kernel hash for each machine or assert that every
configuration was exercised on both.
Subsequent real-SID stereo was reported working with:

```ini
STEREO=ON
SID1_ADDRESS=$D400
SID1_TYPE=8580
SID2_ADDRESS=$D500
SID2_TYPE=6581
STEREO_WIDTH=100
```

Baseline kernel: 782,632 bytes, SHA-256
`4104fa8d9d6d0d58c40a37ef3700a9265efd7d1b1e063efa113f83949eb8db02`.
It is not distributed in this source folder. The reported audio hardware is
an Ultimate64 Elite2 with physical SID chips; emulated UltiSID trials were
separate and produced quality problems. The boot detector sometimes reported
6581 for the primary while the working configuration explicitly selected 8580.

## Clean publication build

This source export removes the RAD intro and all external Doom demo/media
dependencies. Kernel and video code are separated from Doom gameplay, with
the inherited bus transfer and default ordered color conversion retained.
Source installation and portable host suites are checked in a fresh isolated
application checkout. The initial packaging build used established libraries.
A subsequent independent Ubuntu clone of GitHub revision
`e35f5f4da2ef14baa30b8c438595251aff89d7e8` rebuilt Circle44.3 and Newlib from
their pinned sources, passed all host suites and linked without source edits.
Only the documented compiler and installed host tools were reused.
The result is 762,728 bytes, SHA-256
`6a4209f30b62ca18a8209c332e9ba3969a40374140cc230cb3077594c1ad4ba9`.
These default-preset/documentation updates do not change firmware source code.

The no-intro kernel is a new hardware candidate. A successful compile or a
working earlier intro build is not proof of its boot behavior. After deployment,
confirm direct Wolf startup, game entry, controls, music/effects and stereo.
No NTSC, C128 matrix, full-game completion or universal SID quality claim is made.

Cleaned no-intro candidate: 762,696 bytes, SHA-256
`becc8e1eef1c39fd662f51b49efb3254e18bf2bd83abbe80463b91fbd9b73d7a`.
Its link exports Wolf's entry and sound functions, with no Doom main or intro;
the image contains no old RADDOOM media-path strings. An earlier cleaned
candidate was deployed after an independently hash-verified backup, preserving
the existing real mixed-SID config, all eight game data files and save state,
and removing obsolete demo assets. The final local interface-only revision
avoids the MAME header as well; its SD update needs readback confirmation.
First hardware boot of this final no-intro candidate is still pending.

## Audio scope

Real SID audio is required for supported operation. 6581/8580 configurations
must match the actual address/type mapping. Ultimate/UltiSID remains experimental:
VOLUME improved effects but has only four-bit resolution; the separate measured
ULTISID curve has not yet received a successful hardware sound-quality report.
Correct stereo routing does not establish correct timbre or waveform conversion.

## Menu keyboard and save/load — hardware confirmed, 2026-11-14

The C64 scanner now queues raw key edges as well as gameplay actions. Y/N
reach confirmation dialogs, and save names use Wolf4SDL's text-input buffer.
US_LineInput suspends gameplay actions while typing, including I/J/K/L, space
and P. INST/DEL maps to backspace; RETURN accepts and left-arrow cancels.
Host tests exercise down/up taps, the adapter and the actual US_LineInput
function (with rendering stubbed), including editing and cancellation.
On 2026-11-14, the maintainer reported successful hardware verification of
both the menu keyboard checks and the complete save/load cycle: Y/N overwrite
confirmation, name entry (including gameplay-bound letters, spaces and P),
INST/DEL editing, acceptance and cancellation; saving, restarting the computer
and loading again with position, health, ammunition and map progress preserved.
This is maintainer-reported hardware evidence, not an automated hardware test.
Verified deployed kernel SHA-256:
`6cb06bbfadc44a9ff6fb84fe862287063bd5a026f10288098982eaf905bbd952`.
The separate 30-minute stability and level-transition checks remain open.

## Map marker 3x3 — maintainer accepted, 2026-11-14

The requested player marker is a 3x3 square of map dots: eight red dots and
one white dot on the perimeter showing one of eight directions. The center
stays red. Each dot matches a map tile (2x2 source pixels); a black halo
separates the marker from the map. The white dot occupies exactly one of the
eight perimeter positions, with no extra arrow pixels.
This changes the marker only; visited-area coloring is still a separate task.
Host `make test` passed, including exact 3x3 marker checks for all eight
orientations, wrapped angles and map corners. Firmware linked successfully.
Kernel SHA-256: `3ad0f122d0d086ca45c99f5c7a45d344b13495c028d1c34d9ce95a8135071235`.
Deployed to the SD card on 2026-11-14 after a complete hash-verified backup.
Readback verified the kernel and all 228 existing files; only
`kernel_wolf.img` changed. The maintainer then accepted the displayed marker
and authorized a screenshot. One F1 map frame was captured from Ultimate64
memory/registers at 09:09:23 local time on 2026-11-14; see local
`build/map-marker/hardware-evidence.json` for its path and SHA-256.
The screenshot reconstructs the digital display, not the analog monitor output.
Acceptance covers the displayed marker; all-eight-direction verification remains
an automated host-test result. The new capture stays private locally.

## Visited-floor coloring - hardware confirmed, 2026-11-14

Movement records a per-level traversal bitmap; crossed floor turns C64 light
green on F1 while walls, doors and the accepted 3x3 marker retain their colors.
The bitmap persists in an optional 528-byte, versioned CRC32 save trailer.
Old saves load without prior history. Invalid optional records fall back to
the restored player tile; the original game checksum/data layout is unchanged.
See MAP.md for lifecycle and backward-compatibility details.

Host `make test` passed, including the actual SaveTheGame/LoadTheGame path
with synthetic state for new saves, legacy saves and corrupt optional history.
Firmware SHA-256: `cf8a4b0c08648365954a3a0d89aa280ebcb7a2d99a458728bd731c876e5d34b2`.
Deployed after a complete hash-verified SD backup; readback checked all
228 existing files and confirmed that only `kernel_wolf.img` changed.
On 2026-11-14 the maintainer confirmed that traversal coloring works on
hardware, saving and loading a new save preserves the route, and a historical
save loads correctly without prior traversal history. A single current map
frame was captured from Ultimate64 memory/registers at 10:21:02 local time;
its path and SHA-256 are in local `build/map-visited/hardware-evidence.json`.
The screenshot visibly records the green path and remains a private artifact.
The report does not establish a power-cycle test, level-transition reset or
30-minute stability; those remain separate checks.

## Blue elevator exit - candidate, 2026-11-14

The F1 map highlights the exit elevator switch in C64 light blue (14), using
the same exact ELEVATORTILE match as the engine's Cmd_Use. A BLUE: EXIT legend
appears above the map. Normal doors remain yellow, visited floor green and
the accepted 3x3 marker unchanged. Only the private display copy is decorated;
the live tilemap, movement and save format are unchanged.

Host `make test` passed, including the actual modal map adapter with a synthetic
elevator, a normal door, visited floor, restored UI state and an unchanged live
tilemap. The actual new/legacy/corrupt-trailer save/load checks also passed.
Firmware SHA-256: `f36359b1ba61204a2362ae2962f3aa616fe0df4fb0a2e08579f743ad27dde667`.
Deployed to SD after a complete verified backup; readback checked all
229 existing files and confirmed only kernel_wolf.img changed.
Hardware verification of blue exit highlighting remains pending.

## Keys, lock warnings and no player halo - maintainer accepted, 2026-11-14

Uncollected gold/silver keys appear as single yellow/white map tiles, read from
live bonus objects. Removed keys stay absent after save/load. Doors needing a
missing gold/silver key are orange/white; inventory bits and open-door state
remove the warning. The black halo around the 3x3 player marker is removed,
so walls and visited floor immediately outside the marker remain visible.

`make test` passed, including real map-adapter tests for keys, pickup/save/load,
lock warnings and no halo, plus explicit C64 color-tag checks. Existing new,
legacy and damaged-trailer save/load checks passed. The live tilemap and save
format remain unchanged. Deployed after a complete hash-verified SD backup;
readback checked all 229 existing files and confirmed only kernel_wolf.img
changed. On 2026-11-14 the maintainer accepted the deployed candidate,
covering the key/lock annotations and removal of the player halo. This is a
general hardware acceptance, not an exhaustive per-key scenario report;
see local build/map-keys/hardware-evidence.json. No new capture was requested.
Kernel SHA-256: `ce00a903b8aa9698e123c2df496aa033b8c76da05de8d152a664857a91994222`.
