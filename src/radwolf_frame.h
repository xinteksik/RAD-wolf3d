/* RAD-Wolf bring-up: GPL-3.0-or-later. Original adapter implementation. */
#ifndef RADWOLF_FRAME_H
#define RADWOLF_FRAME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RADWOLF_WIDTH 320
#define RADWOLF_HEIGHT 200
#define RADWOLF_PALETTE_SIZE 256

/* Wolf4SDL provides 8-bit palette indices and an RGB palette. */
typedef struct radwolf_rgb {
    uint8_t r, g, b;
} radwolf_rgb;

/*
 * Converts one frame of indexed 8-bit pixels to 0x00RRGGBB.
 * src_pitch_bytes can be greater than 320; dst_pitch_pixels likewise.
 * Destination is caller-owned. Returns 0 on success and -1 on invalid args.
 * radwolf_frame_last_error() identifies the rejected argument for bring-up.
 * This 0x00RRGGBB ABI is verified by the M2 hardware gradient test against
 * RAD-Doom's downstream RGB-to-C64 conversion routine.
 */
int radwolf_blit_indexed_320x200(uint32_t *dst, size_t dst_pitch_pixels,
                                 const uint8_t *src, size_t src_pitch_bytes,
                                 const radwolf_rgb palette[RADWOLF_PALETTE_SIZE]);

enum radwolf_frame_error {
    RADWOLF_FRAME_OK = 0,
    RADWOLF_FRAME_NULL_DST = 1,
    RADWOLF_FRAME_NULL_SRC = 2,
    RADWOLF_FRAME_NULL_PALETTE = 3,
    RADWOLF_FRAME_DST_PITCH = 4,
    RADWOLF_FRAME_SRC_PITCH = 5
};

int radwolf_frame_last_error(void);

/* Use this when an SDL_Color-like palette has 4-byte RGBA entries. */
int radwolf_palette_from_rgba(radwolf_rgb palette[RADWOLF_PALETTE_SIZE],
                              const uint8_t *rgba, size_t entry_stride);

/*
 * Explicit 24-bit RGB output: 3 bytes per pixel (R, G, B), row-major.
 * Useful for feeding an RGB8 frontend without assumptions about u32 packing.
 * Pitch is in bytes. Caller must allocate dst_pitch_bytes * RADWOLF_HEIGHT.
 * This is a standalone palette adapter, not yet a RAD-Doom renderer hook.
 */
int radwolf_blit_indexed_to_rgb24(uint8_t *dst, size_t dst_pitch_bytes,
                                   const uint8_t *src, size_t src_pitch_bytes,
                                   const radwolf_rgb palette[RADWOLF_PALETTE_SIZE]);
#ifdef __cplusplus
}
#endif

#endif /* RADWOLF_FRAME_H */
