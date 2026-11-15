# F1 map and traversal history

The full level layout remains visible, including unvisited areas. Gray means
wall, yellow means door, black means unvisited floor, and light green means
floor crossed by the player's center tile. Light blue marks the elevator
switch used to exit the level. Looking into a room does not mark
it as visited. Normal doors retain their yellow color; doors missing a required key have
a separate warning color as described below.

The player marker is a 3x3 square of map dots: eight red dots and one white
perimeter dot indicating one of eight viewing directions. The center stays red.
Each dot uses 2x2 source pixels. There is no black outline or halo; only the
3x3 marker itself covers map pixels. Adjacent walls and path remain visible.
F1 opens/closes the map; the left-arrow key also closes it.

The elevator switch is identified by the same exact ELEVATORTILE test used
by Cmd_Use. Both normal and secret elevator switches are highlighted, without
changing normal door colors or revealing a different layout. The decoration
exists only in the display copy; the collision map and save format stay intact.
The header legend reads BLUE: EXIT. This addition still needs a hardware check.

## Keys and locked doors

Keys and lock warnings occupy exactly one map tile (2x2 source pixels), so
they do not cover neighboring walls or floor.

| Item | Map color |
|---|---|
| Uncollected gold key | Yellow |
| Uncollected silver key | White |
| Door requiring a missing gold key | Orange |
| Door requiring a missing silver key | White |

The display reads live statobjlist bonus entries (bo_key1/bo_key2). Removed
entries (shapenum < 0), non-bonus decorations and other pickups are ignored.
Collected keys disappear immediately and stay absent after save/load because
the engine already saves the object list and inventory. No new save fields
are added. Keys are visible throughout the full-level overview, just like the
layout and elevator.

Lock warnings use doorobjlist's dr_lock1/dr_lock2 and the same inventory bits
as OperateDoor. Once the player has the required key, the warning disappears;
fully open doors also have no missing-key warning. Normal doors retain yellow.
Only the private display copy receives these flags; game objects and collision
state remain unchanged. Additional nonstandard third/fourth key types are not
assigned map colors by this WL6-specific feature.

## Lifecycle

The traversal bitmap belongs to the current level. SetupGameLevel clears it
for a new game, a level transition or a fresh attempt after death. The spawn
and every completed Thrust movement record the player's current tile. Opening
the map does not mark neighboring rooms or tiles as visited.

Saving records the bitmap with the actual game. Loading restores it for the
same level and map dimensions. Legacy saves contain no earlier traversal
history: they load normally and start tracking at the restored player tile.
The previous route cannot be reconstructed from a legacy save.

## Save compatibility

The original save data and checksum layout are unchanged. The development
kernel appends an optional 528-byte RWMAP version-1 trailer after the original
music offset. Its header contains the level identity, dimensions, 512-byte
payload size and CRC32. Numeric fields are explicitly little-endian; no native
C structure or pointers are stored in the trailer.

Missing, truncated, unknown-version, mismatched-level or corrupt trailers are
ignored, without changing health, ammunition, position or the original game
checksum. Older kernels ignore the appended bytes when loading; saving again
with an older kernel discards the traversal trailer.

## Validation

`make test` covers bitmap boundaries, revisits, resets, rendering colors,
all eight marker directions, trailer round trips and corruption/truncation.
A synthetic fixture exercises the actual Wolf SaveTheGame/LoadTheGame code for
new saves, legacy saves and damaged optional histories, checking player state,
an actor, walls and the original checksum without commercial game data.

The green source color is explicitly tagged as C64 light green (13) during
map conversion, preserving the existing brightness/dither-independent map path.
On 2026-11-14 the maintainer confirmed traversal coloring, route preservation
after saving/loading, and loading a historical save without prior history on
hardware. One current map screenshot was captured from Ultimate64. See STATUS.md
and local build/map-visited/hardware-evidence.json. Level transitions, a power
cycle and 30-minute stability remain separate checks.

The latest host checks also cover gold/silver keys, missing-key doors, removal
after pickup/save/load, final C64 color tags, and walls/path immediately around
the halo-free player marker. On 2026-11-14 the maintainer accepted the deployed
key/lock annotations and halo removal. This general hardware acceptance is
recorded separately from the detailed automated scenarios.
