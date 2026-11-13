/* GPL-3.0-or-later. Bounds, tile classification and orientation checks. */
#include "radwolf_map.h"
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
    assert(pixels[36+24][96+20] == 15);
    assert(pixels[36+24][96+23] == 11);
    assert(pixels[36][96] == 0);
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, 0, 0, 90, box, 0) == 0);
    unsigned before = calls;
    assert(radwolf_map_draw(tiles, 64, 65, 64, 128, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(tiles, 1, 2, 2, 128, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(0, 64, 64, 64, 128, 0, 0, 0, box, 0) == -1);
    assert(calls == before);
}
