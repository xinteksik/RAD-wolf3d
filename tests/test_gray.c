/* GPL-3.0-or-later. Test real Wolf floor/ceiling colors and neutral ramps. */
#include <assert.h>
#include "radwolf_gray.h"
#include "radwolf_flat.h"

void test_gray(void)
{
    /* Wolf's 6-bit palette expands to 8 bits: floor 28, ceiling 14. */
    assert(radwolf_solid_gray_pair(0x717171u, 0x717171u) == 12);
    assert(radwolf_solid_gray_pair(0x383838u, 0x383838u) == 11);
    assert(radwolf_solid_gray_pair(0, 0) == 0);
    assert(radwolf_solid_gray_pair(0xffffffu, 0xffffffu) == 1);
    assert(radwolf_solid_gray_pair(0x717171u, 0x383838u) == -1);
    assert(radwolf_solid_gray_pair(0x717170u, 0x717170u) == -1);
    assert(radwolf_solid_gray_pair(0xff0000u, 0xff0000u) == -1);
    assert(radwolf_solid_gray_pair(0x0b717171u, 0x0b717171u) == -1);
    const int levels[16] = {0, 255, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 68, 108, 0, 0, 149};
    int previous = 0;
    for (unsigned gray = 0; gray < 256; ++gray) {
        uint32_t pixel = gray * 0x010101u;
        int index = radwolf_solid_gray_pair(pixel, pixel);
        assert(index == 0 || index == 11 || index == 12 || index == 15 || index == 1);
        assert(levels[index] >= previous);
        previous = levels[index];
    }

    /* Every exact C64 color must round-trip, including colored ceilings. */
    for (int i = 0; i < 16; ++i) {
        uint32_t rgb = ((uint32_t)radwolf_c64_palette[i][0] << 16) |
                       ((uint32_t)radwolf_c64_palette[i][1] << 8) |
                       (uint32_t)radwolf_c64_palette[i][2];
        assert(radwolf_flat_color(rgb) == i);
    }
    uint32_t black = radwolf_flat_tag(0);
    uint32_t gray = radwolf_flat_tag(0x717171u);
    assert(radwolf_flat_pair(black, black) == 0);
    assert(radwolf_flat_pair(gray, gray) == 12);
    assert(radwolf_flat_pair(gray, 0x717171u) == -1);
    assert(radwolf_flat_pair(gray, black) == -1);
    assert(radwolf_flat_pair(0x717171u, 0x717171u) == -1);
}
