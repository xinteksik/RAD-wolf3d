# Solid floor and ceiling only

The earlier global gray and eight-pixel-run rules also changed door and wall
textures. The maintainer reported worse doors in the hardware screenshots.
Those heuristics are removed from the active converter.

For a normal 3D frame, the engine marks the viewport as background after
clearing the floor/ceiling. Actual wall/door writes in ScalePost and
sprite/weapon writes in ScaleLine clear ownership for the pixels they draw,
even if their palette index is identical to the background. Transparent
parts of sprites retain background ownership. At presentation, only remaining
floor/ceiling pixels are tagged with a nearest C64 color using the current
palette. Bonus and damage flashes still affect them. The renderer bypasses
dithering only when both pixels in a C64 pair have the same explicit tag.

All other gameplay pixels follow the original texture conversion, including
gray texture pixels and long flat spans. Menu, intermission, death/fizzle,
and unrelated surfaces do not inherit the 3D ownership mask. The optional
textured floor/ceiling and sky builds do not enable this solid-surface mask.
The final shared background and per-block VIC-II color limits still apply;
object boundaries can therefore differ even with original texture conversion.

The map is an explicit separate display mode. It keeps solid map colors and
an 8x8 red player square with a black halo. A white 2x2 direction square sits
inside it, at one of the eight positions around the center. This does not
depend on detecting long color runs. The marker color remains C64 light red
and the direction square white, even if gameplay stopped during a palette
flash. Visitation tracking is still a future task, with no save persistence.

The surface mask occupies 64,000 bytes. Clearing ownership is inlined in the
wall/sprite drawing loops. A per-presentation palette LUT avoids searching
the C64 palette for each floor pixel. DMA/PAL timings are unchanged.

Validation on 2026-11-13: make test and AArch64 firmware build passed. Six
Windows-only MIDI checks were skipped under Linux. Tests cover ownership of
same-color foreground/background, transparent/uncovered pixels, invalid
views, mask lifetime, current flash colors, direct black, mixed pixel pairs,
and all eight marker directions inside the square. Override payload hashes
were refreshed from the pinned Wolf4SDL revision.

Candidate: build/floor-ceiling-only/kernel_wolf.img

SHA-256: 0618e055cfb2ca319c924a3bdceab7717857a1026bcb8573123303df248ff72e

Hardware appearance and gameplay performance require checking after booting
this candidate. Compare doors and gray walls, bonus/damage flashes on the
floor and ceiling, weapon/sprite edges, menu, and the F1 direction marker.
