/* GPL-3.0-or-later. A whole 64x64 level fits at two pixels per tile. */
#include "radwolf_map.h"
int radwolf_map_draw(const uint16_t *tiles, unsigned stride, unsigned width,
                     unsigned height, unsigned door_bit,
                     const radwolf_map_history *history, int player_x,
                     int player_y, int angle, radwolf_map_box_fn box, void *user)
{
    if (!tiles || !box || !width || !height || width > 64 || height > 64 || stride < height)
        return -1;
    int origin_x = (320 - (int)width * 2) / 2;
    int origin_y = (200 - (int)height * 2) / 2;
    for (unsigned x = 0; x < width; ++x) {
        for (unsigned y = 0; y < height; ++y) {
            unsigned tile = tiles[x * stride + y];
            unsigned base_tile = tile & ~RADWOLF_MAP_DECORATION_MASK;
            unsigned color = base_tile & door_bit ? 14 : base_tile ? 7 : 0;
            if (base_tile == 0 && history && history->width == width &&
                history->height == height && radwolf_map_history_contains(history, (int)x, (int)y))
                color = RADWOLF_MAP_VISITED_COLOR;
            if (tile & RADWOLF_MAP_GOLD_KEY_FLAG) color = RADWOLF_MAP_GOLD_KEY_COLOR;
            else if (tile & RADWOLF_MAP_SILVER_KEY_FLAG) color = RADWOLF_MAP_SILVER_KEY_COLOR;
            else if (tile & RADWOLF_MAP_EXIT_FLAG) color = RADWOLF_MAP_EXIT_COLOR;
            else if (tile & RADWOLF_MAP_GOLD_LOCK_FLAG) color = RADWOLF_MAP_GOLD_LOCK_COLOR;
            else if (tile & RADWOLF_MAP_SILVER_LOCK_FLAG) color = RADWOLF_MAP_SILVER_LOCK_COLOR;
            box(user, origin_x + (int)x * 2, origin_y + (int)y * 2, 2, 2, color);
        }
    }
    if (player_x >= 0 && player_y >= 0 && (unsigned)player_x < width && (unsigned)player_y < height) {
        int x = origin_x + player_x * 2, y = origin_y + player_y * 2;
        /* Nine map dots form a 3x3 square. Each dot uses the same 2x2
         * source pixels as a map tile, keeping horizontal C64 pairs aligned.
         * The white direction dot replaces exactly one of the eight red
         * perimeter dots; the center always remains red. */
        box(user, x - 2, y - 2, 6, 6, RADWOLF_MAP_PLAYER_COLOR);
        static const int dx[8] = {1,1,0,-1,-1,-1,0,1};
        static const int dy[8] = {0,-1,-1,-1,0,1,1,1};
        angle = ((angle % 360) + 360) % 360;
        unsigned direction = (unsigned)((angle + 22) / 45) & 7;
        int tip_x = x + dx[direction] * 2, tip_y = y + dy[direction] * 2;
        box(user, tip_x, tip_y, 2, 2, RADWOLF_MAP_DIRECTION_COLOR);
    }
    return 0;
}
