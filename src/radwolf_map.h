/* GPL-3.0-or-later. Portable full-level overview, independent of debug keys. */
#ifndef RADWOLF_MAP_H
#define RADWOLF_MAP_H
#include <stdint.h>
#include "radwolf_map_history.h"

/* Wolf/VGA palette indices, before the C64 video conversion. */
#define RADWOLF_MAP_GOLD_KEY_COLOR 14u
#define RADWOLF_MAP_SILVER_KEY_COLOR 15u
#define RADWOLF_MAP_GOLD_LOCK_COLOR 6u
#define RADWOLF_MAP_SILVER_LOCK_COLOR 8u
#define RADWOLF_MAP_EXIT_COLOR 9u
#define RADWOLF_MAP_VISITED_COLOR 10u
#define RADWOLF_MAP_PLAYER_COLOR 12u
#define RADWOLF_MAP_DIRECTION_COLOR 15u
/* Presentation flag added by the engine adapter to its private tile copy.
 * This never changes Wolf's tilemap, movement or saved game data. */
#define RADWOLF_MAP_EXIT_FLAG UINT16_C(0x8000)
#define RADWOLF_MAP_GOLD_KEY_FLAG UINT16_C(0x4000)
#define RADWOLF_MAP_SILVER_KEY_FLAG UINT16_C(0x2000)
#define RADWOLF_MAP_GOLD_LOCK_FLAG UINT16_C(0x1000)
#define RADWOLF_MAP_SILVER_LOCK_FLAG UINT16_C(0x0800)
#define RADWOLF_MAP_DECORATION_MASK UINT16_C(0xf800)

/* Explicit C64 tags keep map colors independent of exposure/dithering.
 * Silver locks are white so they remain distinct from the gray walls. */
static inline uint32_t radwolf_map_color_tag(unsigned color, uint32_t fallback)
{
    switch (color) {
        case RADWOLF_MAP_PLAYER_COLOR: return UINT32_C(0x8aff5555);
        case RADWOLF_MAP_DIRECTION_COLOR: return UINT32_C(0x81ffffff);
        case RADWOLF_MAP_VISITED_COLOR: return UINT32_C(0x8d55ff55);
        case RADWOLF_MAP_EXIT_COLOR: return UINT32_C(0x8e5555ff);
        case RADWOLF_MAP_GOLD_KEY_COLOR: return UINT32_C(0x87ffff55);
        case RADWOLF_MAP_GOLD_LOCK_COLOR: return UINT32_C(0x88aa5500);
        case RADWOLF_MAP_SILVER_LOCK_COLOR: return UINT32_C(0x81ffffff);
        default: return fallback;
    }
}

typedef void (*radwolf_map_box_fn)(void *, int, int, int, int, unsigned);
int radwolf_map_draw(const uint16_t *tiles, unsigned stride, unsigned width,
                     unsigned height, unsigned door_bit,
                     const radwolf_map_history *history, int player_x,
                     int player_y, int angle, radwolf_map_box_fn box, void *user);
void radwolf_wolf_map_record_position(void);
void radwolf_wolf_show_map(void);
#endif
