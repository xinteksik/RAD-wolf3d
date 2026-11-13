/* GPL-3.0-or-later. Bounds, tile classification and orientation checks. */
#include "radwolf_map.h"
#include "radwolf_flat.h"
#include <assert.h>
#include <string.h>
static unsigned char pixels[200][320];
static unsigned calls;
static void box(void *user, int x, int y, int width, int height, unsigned color)
{
    (void)user;
    assert(x >= 0 && y >= 0 && width > 0 && height > 0);
    assert(x + width <= 320 && y + height <= 200);
    ++calls;
    for (int row = y; row < y + height; ++row)
        memset(&pixels[row][x], (int)color, (unsigned)width);
}
void test_map(void)
{
    uint16_t tiles[64 * 64] = {0};
    tiles[1 * 64 + 2] = 1;
    tiles[3 * 64 + 4] = 128;
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, 10, 12, 0, box, 0) == 0);
    assert(pixels[36+4][96+2] == 7);
    assert(pixels[36+8][96+6] == 14);
    assert(pixels[36+24][96+20] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+22][96+18] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+27][96+23] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+19][96+15] == 0);
    assert(pixels[36+24][96+23] == RADWOLF_MAP_DIRECTION_COLOR);
    assert(pixels[36+25][96+24] == RADWOLF_MAP_DIRECTION_COLOR);
    /* VGA bright red is C64 light red, distinct from gray walls/yellow doors. */
    assert(radwolf_flat_color(0xff5555u) == 10);
    assert(pixels[36][96] == 0);
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, 0, 0, 90, box, 0) == 0);
    assert(pixels[36-3][96] == RADWOLF_MAP_DIRECTION_COLOR);
    static const int direction_x[8] = {1,1,0,-1,-1,-1,0,1};
    static const int direction_y[8] = {0,-1,-1,-1,0,1,1,1};
    for (unsigned direction = 0; direction < 8; ++direction) {
        memset(pixels, 0, sizeof(pixels));
        assert(radwolf_map_draw(tiles, 64, 64, 64, 128,
                               10, 12, (int)direction * 45, box, 0) == 0);
        unsigned white_pixels = 0;
        for (int y = 60 - 3; y < 60 + 5; ++y)
            for (int x = 116 - 3; x < 116 + 5; ++x)
                white_pixels += pixels[y][x] == RADWOLF_MAP_DIRECTION_COLOR;
        assert(white_pixels == 4);
        assert(pixels[60 + direction_y[direction] * 3]
                     [116 + direction_x[direction] * 3] == RADWOLF_MAP_DIRECTION_COLOR);
    }
    /* All eight directions, wrapped angles and four map corners fit. */
    for (int angle = -360; angle <= 720; angle += 45) {
        for (int x = 0; x <= 63; x += 63)
            for (int y = 0; y <= 63; y += 63)
                assert(radwolf_map_draw(tiles, 64, 64, 64, 128,
                                       x, y, angle, box, 0) == 0);
    }
    assert(radwolf_map_draw(tiles, 1, 1, 1, 128, 0, 0, 270, box, 0) == 0);
    unsigned before = calls;
    assert(radwolf_map_draw(tiles, 64, 65, 64, 128, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(tiles, 1, 2, 2, 128, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(0, 64, 64, 64, 128, 0, 0, 0, box, 0) == -1);
    assert(calls == before);
}
