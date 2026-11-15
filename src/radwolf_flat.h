/* GPL-3.0-or-later. Flat-area conversion; Pepto palette from RAD-Doom. */
#ifndef RADWOLF_FLAT_H
#define RADWOLF_FLAT_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "radwolf_gray.h"

static const int radwolf_c64_palette[16][3] = {
    {0x00, 0x00, 0x00}, {0xff, 0xff, 0xff}, {0x68, 0x37, 0x2b}, {0x70, 0xa4, 0xb2},
    {0x6f, 0x3d, 0x86}, {0x58, 0x8d, 0x43}, {0x35, 0x28, 0x79}, {0xb8, 0xc7, 0x6f},
    {0x6f, 0x4f, 0x25}, {0x43, 0x39, 0x00}, {0x9a, 0x67, 0x59}, {0x44, 0x44, 0x44},
    {0x6c, 0x6c, 0x6c}, {0x9a, 0xd2, 0x84}, {0x6c, 0x5e, 0xb5}, {0x95, 0x95, 0x95}
};

static inline int radwolf_flat_color(uint32_t rgb)
{
    int neutral = radwolf_solid_gray_pair(rgb, rgb);
    if (neutral >= 0)
        return neutral;
    int r = (int)((rgb >> 16) & 255u);
    int g = (int)((rgb >> 8) & 255u);
    int b = (int)(rgb & 255u);
    int source_min = r < g ? r : g;
    if (b < source_min) source_min = b;
    int source_max = r > g ? r : g;
    if (b > source_max) source_max = b;
    int chroma = source_max - source_min;
    int source_channels[3] = {r, g, b};
    /* A clearly colored dark ceiling must not become gray merely because
     * gray has a closer brightness. Compare normalized chroma as well.
     * Near-neutral colors keep the ordinary RGB rule, including fades. */
    int preserve_hue = chroma >= 24;
    int best = 0, best_distance = 6 * 256 * 256;
    for (int i = 0; i < 16; ++i) {
        int dr = r - radwolf_c64_palette[i][0];
        int dg = g - radwolf_c64_palette[i][1];
        int db = b - radwolf_c64_palette[i][2];
        int distance = dr * dr + dg * dg + db * db;
        if (preserve_hue) {
            int target_min = radwolf_c64_palette[i][0];
            int target_max = target_min;
            for (int channel = 1; channel < 3; ++channel) {
                int value = radwolf_c64_palette[i][channel];
                if (value < target_min) target_min = value;
                if (value > target_max) target_max = value;
            }
            int target_chroma = target_max - target_min;
            if (target_chroma < 24) continue;
            for (int channel = 0; channel < 3; ++channel) {
                int source_hue = (source_channels[channel] - source_min) * 255 / chroma;
                int target_hue = (radwolf_c64_palette[i][channel] - target_min) * 255 / target_chroma;
                int delta = source_hue - target_hue;
                distance += delta * delta;
            }
        }
        if (distance < best_distance) {
            best_distance = distance;
            best = i;
        }
    }
    return best;
}

/* The high byte explicitly tags surfaces, including direct C64 black. */
static inline uint32_t radwolf_flat_tag(uint32_t rgb)
{
    return (rgb & UINT32_C(0x00ffffff)) |
           ((uint32_t)(0x80 | radwolf_flat_color(rgb)) << 24);
}

static inline int radwolf_flat_pair(uint32_t first, uint32_t second)
{
    unsigned tag = first >> 24;
    return (tag & 0x80u) && tag == (second >> 24) ? (int)(tag & 15u) : -1;
}
#endif
