/* RAD-Wolf bring-up: GPL-3.0-or-later. Original adapter implementation. */
#include "radwolf_frame.h"

static int frame_error;

int radwolf_frame_last_error(void)
{
    return frame_error;
}

int radwolf_blit_indexed_320x200(uint32_t *dst, size_t dst_pitch_pixels,
                                 const uint8_t *src, size_t src_pitch_bytes,
                                 const radwolf_rgb palette[RADWOLF_PALETTE_SIZE])
{
    frame_error = RADWOLF_FRAME_OK;
    if (dst == NULL)
        frame_error = RADWOLF_FRAME_NULL_DST;
    else if (src == NULL)
        frame_error = RADWOLF_FRAME_NULL_SRC;
    else if (palette == NULL)
        frame_error = RADWOLF_FRAME_NULL_PALETTE;
    else if (dst_pitch_pixels < RADWOLF_WIDTH)
        frame_error = RADWOLF_FRAME_DST_PITCH;
    else if (src_pitch_bytes < RADWOLF_WIDTH)
        frame_error = RADWOLF_FRAME_SRC_PITCH;
    if (frame_error != RADWOLF_FRAME_OK)
        return -1;

    /* LUT makes rendering O(pixels), with no three-channel lookups per pixel. */
    uint32_t rgb_lut[RADWOLF_PALETTE_SIZE];
    for (size_t i = 0; i < RADWOLF_PALETTE_SIZE; ++i)
        rgb_lut[i] = ((uint32_t)palette[i].r << 16) |
                     ((uint32_t)palette[i].g << 8) |
                     (uint32_t)palette[i].b;

    for (size_t y = 0; y < RADWOLF_HEIGHT; ++y) {
        const uint8_t *row = src + y * src_pitch_bytes;
        uint32_t *out = dst + y * dst_pitch_pixels;
        for (size_t x = 0; x < RADWOLF_WIDTH; ++x)
            out[x] = rgb_lut[row[x]];
    }
    return 0;
}

int radwolf_palette_from_rgba(radwolf_rgb palette[RADWOLF_PALETTE_SIZE],
                              const uint8_t *rgba, size_t entry_stride)
{
    if (palette == NULL || rgba == NULL || entry_stride < 3)
        return -1;
    for (size_t i = 0; i < RADWOLF_PALETTE_SIZE; ++i) {
        const uint8_t *p = rgba + i * entry_stride;
        palette[i].r = p[0];
        palette[i].g = p[1];
        palette[i].b = p[2];
    }
    return 0;
}

int radwolf_blit_indexed_to_rgb24(uint8_t *dst, size_t dst_pitch_bytes,
                                   const uint8_t *src, size_t src_pitch_bytes,
                                   const radwolf_rgb palette[RADWOLF_PALETTE_SIZE])
{
    if (dst == NULL || src == NULL || palette == NULL ||
        dst_pitch_bytes < (RADWOLF_WIDTH * 3u) || src_pitch_bytes < RADWOLF_WIDTH)
        return -1;

    for (size_t y = 0; y < RADWOLF_HEIGHT; ++y) {
        const uint8_t *in = src + y * src_pitch_bytes;
        uint8_t *out = dst + y * dst_pitch_bytes;
        for (size_t x = 0; x < RADWOLF_WIDTH; ++x) {
            const radwolf_rgb rgb = palette[in[x]];
            out[3 * x + 0] = rgb.r;
            out[3 * x + 1] = rgb.g;
            out[3 * x + 2] = rgb.b;
        }
    }
    return 0;
}
