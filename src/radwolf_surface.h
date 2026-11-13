/* GPL-3.0-or-later. Ownership of solid floor/ceiling pixels in a 3D frame. */
#ifndef RADWOLF_SURFACE_H
#define RADWOLF_SURFACE_H
#include <stddef.h>
#include <stdint.h>

extern uintptr_t radwolf_surface_base;
extern size_t radwolf_surface_length;
extern uint8_t radwolf_surface_mask[320 * 200];
extern int radwolf_map_colors;

void radwolf_surface_begin(const uint8_t *base, size_t pitch, size_t height,
                           size_t offset, size_t width, size_t view_height);
void radwolf_surface_end(void);

static inline int radwolf_surface_background(const uint8_t *pixel)
{
    uintptr_t address = (uintptr_t)pixel;
    if (address < radwolf_surface_base ||
        address - radwolf_surface_base >= radwolf_surface_length)
        return 0;
    return radwolf_surface_mask[address - radwolf_surface_base] != 0;
}

static inline void radwolf_surface_cover(const uint8_t *pixel)
{
    uintptr_t address = (uintptr_t)pixel;
    if (address >= radwolf_surface_base &&
        address - radwolf_surface_base < radwolf_surface_length)
        radwolf_surface_mask[address - radwolf_surface_base] = 0;
}
#endif
