# Solid gray and colored areas

Historical experiments: superseded by [VIDEO-SURFACES.md](VIDEO-SURFACES.md).
The global gray/run rules described below are no longer active in gameplay.

The C64 converter now maps a pair of identical neutral RGB pixels directly to
black, dark gray (11), medium gray (12), light gray (15), or white. It chooses
the nearest Pepto neutral level using source brightness, without ordered
dithering or the exposure boost used for textures. This applies to neutral
pixels throughout the image, including menus and gray texture details.
Colored runs of at least eight identical source pixels now also use one
nearest C64 color at source brightness. This covers colored ceilings, menu
and intermission backgrounds, and flat surfaces during bonus/damage palette
shifts. Short colored runs and mixed-color pixel pairs retain texture
dithering. Classification uses the current RGB frame and is rebuilt on every
frame; it has no level-specific palette indices or cached flash colors.
It costs an additional 64,000-byte classification buffer and one linear
frame scan. The scan can also classify long solid spans within a texture;
it does not identify game objects or surfaces semantically.

Wolf's default floor color (palette index 0x19, RGB 113/113/113) becomes solid
medium gray. The usual ceiling (0x1d, RGB 56/56/56) becomes solid dark gray.
They remain distinct. Palette fades and bonus/damage flashes still change
source brightness and color; large flat areas change to a single nearest
C64 color instead of a spatial pattern. Quantization can make weak palette
shifts invisible or produce discrete color changes.

The final VIC-II multicolor constraints still apply: each 8x8 source block
has the shared background plus three local colors. A gray may still be
replaced when a block also contains a detailed wall, sprite, or weapon.
This change does not guarantee a uniform color along every boundary.

Validation must distinguish host tests/build success from real hardware
appearance. Check open floor, ceiling, gray walls, doors, weapon edges,
menus, fades, and damage flashes on the C64 before promoting the candidate.

The maintainer confirmed that `docs/images/wolf-game.png` and `wolf-menu.png`
come from a real computer. They are the existing hardware reference, not
captures of this change. The game image shows the ceiling pattern and a
greenish floor pattern; the menu shows scattered light marks on its brown
background. The later supplied `screen-secret-level.png` shows solid gray
floor but a patterned colored ceiling; `screen-secret-get-level.png` shows
patterns during a bonus flash; `screen-end-level.png` shows the patterned
intermission background. These motivate the colored-run extension. The raw
captures remain outside the publication source tree.

On 2026-11-13, `make test` passed (the six Windows-only MIDI checks were
skipped under Linux) and the AArch64 firmware build succeeded. The candidate
is `build/solid-grays/kernel_wolf.img`, SHA-256
`7635c4373e84ba60e8fc3b5e921e2d4ea264e3d882c7afa4240bf10b897f80e9`.
This is the first gray-only build, deployed to SD with backup and readback
verification. Its deployment record remains in that build directory. The
colored-run extension is a separate candidate in `build/solid-colors` and
requires another hardware check, including gameplay performance.
Its `make test` and AArch64 firmware build also passed on 2026-11-13
(six Windows-only MIDI checks skipped). Candidate SHA-256:
`53e149f2e9a8ce8559fb59e2314b4ead0deff852c95ac1d222ebb03237a10208`.

# Next task: visited map cells

The current player marker is now an 8x8 bright-red VGA square with a black
12x12 halo and a separate 2x2 white direction indicator. The map footer says
`RED: YOU`. The red span takes the solid-color conversion path (C64 light
red, index 10). Marker overlays may extend beyond the map boundary into the
screen margin so the indicator remains visible at corners. A small area of
nearby map tiles is covered by the enlarged marker.

The combined color/marker candidate is `build/solid-colors-map/kernel_wolf.img`,
SHA-256 `e962db588be593d4e42f48878ccfcde56556d0d229a0188daa53207a23c2caef`.
Host tests and firmware build passed; hardware appearance remains pending.

Keep the full F1 map visible. Tint floor cells the player has occupied,
recording the current tile during gameplay, even when the map is closed.
Keep the player arrow and doors readable. Clear visitation on a new level,
restart, new game, and loading a save. Do not change the save format or
persist visitation. Add host checks for marking, repeated marking, reset,
map bounds, and drawing visited versus unvisited cells.
