/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#ifndef RADWOLF_VIDEO_H
#define RADWOLF_VIDEO_H

#include "radwolf_frame.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Consume/copy the RGB frame before returning; do not retain the pointer.
 * Return zero only when presentation succeeded. No asynchronous DMA ownership
 * is implied: a future asynchronous backend needs an explicit release API. */
typedef int (*radwolf_present_fn)(void *user, const uint32_t *rgb,
                                  size_t pitch_pixels);

typedef struct radwolf_video {
    uint32_t *rgb;
    radwolf_present_fn present;
    void *user;
    uint64_t presented;
    uint64_t rejected;
    int busy;
} radwolf_video;

/* rgb must point to at least 320*200 caller-owned uint32_t pixels. */
int radwolf_video_init(radwolf_video *video, uint32_t *rgb,
                       size_t capacity_pixels, radwolf_present_fn present,
                       void *user);

/* Single-threaded, synchronous; nested submissions are rejected. Input and
 * palette are needed only until this call returns and must not overlap rgb. */
int radwolf_video_present(radwolf_video *video, const uint8_t *indices,
                          size_t pitch_bytes, const radwolf_rgb *palette);

#ifdef __cplusplus
}
#endif
#endif
