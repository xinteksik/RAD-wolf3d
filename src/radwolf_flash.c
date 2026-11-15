/* GPL-3.0-or-later. Compose feedback only on an explicitly prepared 3D frame. */
#include "radwolf_flash.h"

unsigned radwolf_flash_mode = RADWOLF_FLASH_FRAME;
static unsigned damage_level, pickup_level;
static int view_x, view_y, view_width, view_height, hud_height, pending;

void radwolf_flash_event(unsigned damage, unsigned pickup)
{
    damage_level = damage > 6 ? 6 : damage;
    pickup_level = pickup > 3 ? 3 : pickup;
    if (!damage_level && !pickup_level) pending = 0;
}

void radwolf_flash_prepare_view(int x, int y, int width, int height, int status_height)
{
    pending = 0;
    if (x < 0 || y < 0 || x >= 320 || y >= 200 || width <= 0 || height <= 0 ||
        width > 320 - x || height > 200 - y || status_height < 0 || status_height > 200)
        return;
    view_x = x; view_y = y; view_width = width; view_height = height;
    hud_height = status_height;
    pending = 1;
}

void radwolf_flash_apply(uint32_t *pixels, size_t pitch)
{
    if (!pending) return;
    pending = 0;
    if (!pixels || pitch < 320 || pitch > SIZE_MAX / 200 ||
        (!damage_level && !pickup_level) || radwolf_flash_mode == RADWOLF_FLASH_OFF ||
        radwolf_flash_mode == RADWOLF_FLASH_FULL)
        return;
    if (radwolf_flash_mode == RADWOLF_FLASH_STATUS && hud_height) {
        int amount = (int)(damage_level ? damage_level : pickup_level);
        int steps = damage_level ? 8 : 20;
        int target_g = damage_level ? 0 : 248;
        for (int y = 200 - hud_height; y < 200; ++y) {
            for (int x = 0; x < 320; ++x) {
                uint32_t rgb = pixels[(size_t)y * pitch + (unsigned)x];
                int r = (int)((rgb >> 16) & 255u), g = (int)((rgb >> 8) & 255u);
                int b = (int)(rgb & 255u);
                r += (256 - r) * amount / steps;
                g += (target_g - g) * amount / steps;
                b -= b * amount / steps;
                pixels[(size_t)y * pitch + (unsigned)x] =
                    ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            }
        }
        return;
    }
    /* Prefer the existing margin; full-screen views use a two-pixel inset.
     * Paired horizontal pixels preserve the C64 multicolor pixel width. */
    int left = view_x >= 2 ? view_x - 2 : 0;
    int top = view_y >= 2 ? view_y - 2 : 0;
    int right = view_x + view_width + 2;
    int bottom = view_y + view_height + 2;
    if (right > 320) right = 320;
    if (bottom > 200) bottom = 200;
    uint32_t color = damage_level ? UINT32_C(0x8268372b) : UINT32_C(0x87b8c76f);
    for (int y = top; y < bottom; ++y)
        for (int x = left; x < right; ++x)
            if (x < left + 2 || x >= right - 2 || y < top + 2 || y >= bottom - 2)
                pixels[(size_t)y * pitch + (unsigned)x] = color;
}
