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
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, NULL, 10, 12, 0, box, 0) == 0);
    assert(pixels[36+4][96+2] == 7);
    assert(pixels[36+8][96+6] == 14);
    assert(pixels[36+24][96+20] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+22][96+18] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+27][96+23] == RADWOLF_MAP_PLAYER_COLOR);
    assert(pixels[36+19][96+15] == 0);
    assert(pixels[36+24][96+22] == RADWOLF_MAP_DIRECTION_COLOR);
    assert(pixels[36+25][96+23] == RADWOLF_MAP_DIRECTION_COLOR);
    /* VGA bright red is C64 light red, distinct from gray walls/yellow doors. */
    assert(radwolf_flat_color(0xff5555u) == 10);
    assert(pixels[36][96] == 0);
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, NULL, 0, 0, 90, box, 0) == 0);
    assert(pixels[36-2][96] == RADWOLF_MAP_DIRECTION_COLOR);
    static const int direction_x[8] = {1,1,0,-1,-1,-1,0,1};
    static const int direction_y[8] = {0,-1,-1,-1,0,1,1,1};
    for (unsigned direction = 0; direction < 8; ++direction) {
        memset(pixels, 0, sizeof(pixels));
        assert(radwolf_map_draw(tiles, 64, 64, 64, 128, NULL,
                               10, 12, (int)direction * 45, box, 0) == 0);
        unsigned red_dots = 0, white_dots = 0;
        for (int dot_y = -1; dot_y <= 1; ++dot_y) {
            for (int dot_x = -1; dot_x <= 1; ++dot_x) {
                unsigned expected = dot_x == direction_x[direction] &&
                                    dot_y == direction_y[direction]
                                    ? RADWOLF_MAP_DIRECTION_COLOR
                                    : RADWOLF_MAP_PLAYER_COLOR;
                red_dots += expected == RADWOLF_MAP_PLAYER_COLOR;
                white_dots += expected == RADWOLF_MAP_DIRECTION_COLOR;
                for (int py = 0; py < 2; ++py)
                    for (int px = 0; px < 2; ++px)
                        assert(pixels[60 + dot_y * 2 + py]
                                     [116 + dot_x * 2 + px] == expected);
            }
        }
        assert(red_dots == 8 && white_dots == 1);
        assert(pixels[60][116] == RADWOLF_MAP_PLAYER_COLOR);
        /* Only the perimeter direction dot is white. */
        unsigned white_pixels = 0;
        for (int y = 56; y < 66; ++y)
            for (int x = 112; x < 122; ++x)
                white_pixels += pixels[y][x] == RADWOLF_MAP_DIRECTION_COLOR;
        assert(white_pixels == 4);
    }
    /* All eight directions, wrapped angles and four map corners fit. */
    for (int angle = -360; angle <= 720; angle += 45) {
        for (int x = 0; x <= 63; x += 63)
            for (int y = 0; y <= 63; y += 63)
                assert(radwolf_map_draw(tiles, 64, 64, 64, 128, NULL,
                                       x, y, angle, box, 0) == 0);
    }
    assert(radwolf_map_draw(tiles, 1, 1, 1, 128, NULL, 0, 0, 270, box, 0) == 0);
    /* Only actual visits on traversable floor turn green. */
    radwolf_map_history history;
    radwolf_map_history_reset(&history, 64, 64, 0);
    radwolf_map_history_mark(&history, 20, 21);
    radwolf_map_history_mark(&history, 21, 21);
    radwolf_map_history_mark(&history, 1, 2); /* A wall stays gray. */
    radwolf_map_history_mark(&history, 3, 4); /* A door stays yellow. */
    radwolf_map_history saved = history;
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, &history, -1, -1, 0, box, 0) == 0);
    assert(pixels[36 + 42][96 + 40] == RADWOLF_MAP_VISITED_COLOR);
    assert(pixels[36 + 42][96 + 42] == RADWOLF_MAP_VISITED_COLOR);
    assert(pixels[36 + 40][96 + 40] == 0);
    assert(pixels[36 + 4][96 + 2] == 7);
    assert(pixels[36 + 8][96 + 6] == 14);
    assert(!memcmp(&saved, &history, sizeof(history)));
    /* A mismatched map size cannot paint another map's history. */
    history.width = 32;
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, &history, -1, -1, 0, box, 0) == 0);
    assert(pixels[36 + 42][96 + 40] == 0);
    /* Only the adapter-tagged elevator is blue. Ordinary wall textures,
     * including neighboring numeric values, retain their normal meaning. */
    tiles[30 * 64 + 30] = 21 | RADWOLF_MAP_EXIT_FLAG;
    tiles[31 * 64 + 30] = 20;
    tiles[32 * 64 + 30] = 22;
    tiles[33 * 64 + 30] = 21; /* Untagged data is still just a wall. */
    radwolf_map_history_reset(&history, 64, 64, 0);
    radwolf_map_history_mark(&history, 30, 30);
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, &history, -1, -1, 0, box, 0) == 0);
    for (int dy = 0; dy < 2; ++dy)
        for (int dx = 0; dx < 2; ++dx)
            assert(pixels[36 + 60 + dy][96 + 60 + dx] == RADWOLF_MAP_EXIT_COLOR);
    assert(pixels[36 + 60][96 + 62] == 7);
    assert(pixels[36 + 60][96 + 64] == 7);
    assert(pixels[36 + 60][96 + 66] == 7);
    assert(pixels[36 + 8][96 + 6] == 14);
    /* Keys and missing-key locks are single-tile overlays. */
    tiles[40 * 64 + 40] = RADWOLF_MAP_GOLD_KEY_FLAG;
    tiles[42 * 64 + 40] = RADWOLF_MAP_SILVER_KEY_FLAG;
    tiles[44 * 64 + 40] = 128 | RADWOLF_MAP_GOLD_LOCK_FLAG;
    tiles[46 * 64 + 40] = 128 | RADWOLF_MAP_SILVER_LOCK_FLAG;
    assert(radwolf_map_draw(tiles, 64, 64, 64, 128, &history, -1, -1, 0, box, 0) == 0);
    assert(pixels[36 + 80][96 + 80] == RADWOLF_MAP_GOLD_KEY_COLOR);
    assert(pixels[36 + 80][96 + 84] == RADWOLF_MAP_SILVER_KEY_COLOR);
    assert(pixels[36 + 80][96 + 88] == RADWOLF_MAP_GOLD_LOCK_COLOR);
    assert(pixels[36 + 80][96 + 92] == RADWOLF_MAP_SILVER_LOCK_COLOR);
    assert(pixels[36 + 80][96 + 82] == 0);
    /* No player halo: walls and visited floor directly outside the 3x3
     * marker survive in every direction of view. */
    tiles[8 * 64 + 12] = 1;
    tiles[10 * 64 + 10] = 1;
    radwolf_map_history_mark(&history, 12, 12);
    for (int angle = 0; angle < 360; angle += 45) {
        assert(radwolf_map_draw(tiles, 64, 64, 64, 128, &history,
                               10, 12, angle, box, 0) == 0);
        assert(pixels[60][112] == 7);
        assert(pixels[56][116] == 7);
        assert(pixels[60][120] == RADWOLF_MAP_VISITED_COLOR);
    }
    /* Final C64 colors: both silver symbols are white, not wall gray. */
    static const unsigned map_colors[] = {
        RADWOLF_MAP_GOLD_KEY_COLOR, RADWOLF_MAP_SILVER_KEY_COLOR,
        RADWOLF_MAP_GOLD_LOCK_COLOR, RADWOLF_MAP_SILVER_LOCK_COLOR,
        RADWOLF_MAP_EXIT_COLOR, RADWOLF_MAP_VISITED_COLOR,
        RADWOLF_MAP_PLAYER_COLOR, RADWOLF_MAP_DIRECTION_COLOR
    };
    static const int c64_colors[] = {7, 1, 8, 1, 14, 13, 10, 1};
    for (unsigned i = 0; i < sizeof(map_colors) / sizeof(map_colors[0]); ++i) {
        uint32_t tag = radwolf_map_color_tag(map_colors[i], 0);
        assert(radwolf_flat_pair(tag, tag) == c64_colors[i]);
    }
    uint32_t wall_tag = radwolf_flat_tag(UINT32_C(0xaaaaaa));
    assert(radwolf_map_color_tag(7, wall_tag) == wall_tag);
    assert(radwolf_flat_pair(wall_tag, wall_tag) == 15);
    unsigned before = calls;
    assert(radwolf_map_draw(tiles, 64, 65, 64, 128, NULL, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(tiles, 1, 2, 2, 128, NULL, 0, 0, 0, box, 0) == -1);
    assert(radwolf_map_draw(0, 64, 64, 64, 128, NULL, 0, 0, 0, box, 0) == -1);
    assert(calls == before);
}
