/* GPL-3.0-or-later. A whole 64x64 level fits at two pixels per tile. */
#include "radwolf_map.h"
int radwolf_map_draw(const uint16_t *tiles, unsigned stride, unsigned width,
                     unsigned height, unsigned door_bit, int player_x,
                     int player_y, int angle, radwolf_map_box_fn box, void *user)
{
    if (!tiles || !box || !width || !height || width > 64 || height > 64 || stride < height)
        return -1;
    int origin_x = (320 - (int)width * 2) / 2;
    int origin_y = (200 - (int)height * 2) / 2;
    for (unsigned x = 0; x < width; ++x) {
        for (unsigned y = 0; y < height; ++y) {
            unsigned tile = tiles[x * stride + y];
            unsigned color = tile & door_bit ? 14 : tile ? 7 : 0;
            box(user, origin_x + (int)x * 2, origin_y + (int)y * 2, 2, 2, color);
        }
    }
    if (player_x >= 0 && player_y >= 0 && (unsigned)player_x < width && (unsigned)player_y < height) {
        int x = origin_x + player_x * 2, y = origin_y + player_y * 2;
        box(user, x, y, 2, 2, 15);
        static const int dx[8] = {1,1,0,-1,-1,-1,0,1};
        static const int dy[8] = {0,-1,-1,-1,0,1,1,1};
        angle = ((angle % 360) + 360) % 360;
        unsigned direction = (unsigned)((angle + 22) / 45) & 7;
        int tip_x = x + dx[direction] * 3, tip_y = y + dy[direction] * 3;
        if (tip_x >= origin_x && tip_x < origin_x + (int)width * 2 &&
            tip_y >= origin_y && tip_y < origin_y + (int)height * 2)
            box(user, tip_x, tip_y, 1, 1, 11);
    }
    return 0;
}
