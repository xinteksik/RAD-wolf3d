/* GPL-3.0-or-later. Ownership remains distinct even for identical colors. */
#include "radwolf_surface.h"
#include "radwolf_flat.h"
#include <assert.h>
#include <string.h>

void test_surface(void)
{
    uint8_t frame[20 * 12], unrelated[8];
    memset(frame, 25, sizeof(frame));
    radwolf_surface_begin(frame, 20, 12, 22, 16, 8);
    assert(!radwolf_surface_background(frame));
    assert(radwolf_surface_background(frame + 22));
    assert(radwolf_surface_background(frame + 22 + 7 * 20 + 15));
    assert(!radwolf_surface_background(unrelated));
    /* Door, sprite, and weapon overwrite equal-valued background pixels. */
    radwolf_surface_cover(frame + 42);
    radwolf_surface_cover(frame + 62);
    radwolf_surface_cover(frame + 82);
    assert(!radwolf_surface_background(frame + 42));
    assert(!radwolf_surface_background(frame + 62));
    assert(!radwolf_surface_background(frame + 82));
    assert(radwolf_surface_background(frame + 43));
    assert(frame[42] == frame[43]);
    /* A new frame restores ownership, with any current palette flash. */
    radwolf_surface_begin(frame, 20, 12, 22, 16, 8);
    assert(radwolf_surface_background(frame + 42));
    uint32_t flash = radwolf_flat_tag(0xb8c76fu);
    assert(radwolf_flat_pair(flash, flash) == 7);
    radwolf_surface_end();
    assert(!radwolf_surface_background(frame + 42));
    /* Menus/another surface and invalid views cannot inherit the mask. */
    radwolf_surface_begin(frame, 20, 12, 22, 20, 8);
    assert(!radwolf_surface_background(frame + 42));
    radwolf_surface_begin(frame, 20, 12, 22, 16, 8);
    radwolf_surface_begin(frame, 0, 12, 22, 16, 8);
    assert(radwolf_surface_length == 0);
}
