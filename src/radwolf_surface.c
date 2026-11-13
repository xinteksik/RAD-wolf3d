/* GPL-3.0-or-later. Track surfaces by drawing ownership, not RGB patterns. */
#include "radwolf_surface.h"
#include <string.h>

uintptr_t radwolf_surface_base;
size_t radwolf_surface_length;
uint8_t radwolf_surface_mask[320 * 200];
int radwolf_map_colors;

void radwolf_surface_end(void)
{
    radwolf_surface_length = 0;
    radwolf_surface_base = 0;
}

void radwolf_surface_begin(const uint8_t *base, size_t pitch, size_t height,
                           size_t offset, size_t width, size_t view_height)
{
    radwolf_surface_end();
    if (!base || !pitch || !height || pitch > 320 || height > 200 ||
        !width || !view_height || offset >= pitch * height ||
        width > pitch - offset % pitch || view_height > height - offset / pitch)
        return;
    memset(radwolf_surface_mask, 0, pitch * height);
    for (size_t y = 0; y < view_height; ++y)
        memset(radwolf_surface_mask + offset + y * pitch, 1, width);
    radwolf_surface_base = (uintptr_t)base;
    radwolf_surface_length = pitch * height;
}
