/* GPL-3.0-or-later. Stable neutral colors for the C64 video converter. */
#ifndef RADWOLF_GRAY_H
#define RADWOLF_GRAY_H
#include <stdint.h>

/* Return a C64 color for identical neutral RGB pixels, or -1 for dithering.
 * Use source brightness: the texture-oriented exposure boost would brighten
 * large flat floors unnecessarily. Keep black and white exact as well.
 * Tagged pixels use the renderer's separate direct-C64 path.
 */
static inline int radwolf_solid_gray_pair(uint32_t first, uint32_t second)
{
    if (first != second || (first >> 24) != 0)
        return -1;
    unsigned gray = first & 255u;
    if (((first >> 8) & 255u) != gray || ((first >> 16) & 255u) != gray)
        return -1;
    if (gray <= 34u) return 0;
    if (gray <= 88u) return 11;
    if (gray <= 128u) return 12;
    if (gray <= 202u) return 15;
    return 1;
}
#endif
