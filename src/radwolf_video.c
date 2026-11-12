/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#include "radwolf_video.h"

int radwolf_video_init(radwolf_video *video, uint32_t *rgb,
                       size_t capacity_pixels, radwolf_present_fn present,
                       void *user)
{
    if (video == NULL)
        return -1;
    *video = (radwolf_video){0};
    if (rgb == NULL || present == NULL ||
        capacity_pixels < RADWOLF_WIDTH * RADWOLF_HEIGHT)
        return -1;
    video->rgb = rgb;
    video->present = present;
    video->user = user;
    return 0;
}

int radwolf_video_present(radwolf_video *video, const uint8_t *indices,
                          size_t pitch_bytes, const radwolf_rgb *palette)
{
    if (video == NULL)
        return -1;
    if (video->busy || video->present == NULL || video->rgb == NULL) {
        ++video->rejected;
        return -1;
    }
    video->busy = 1;
    int result = radwolf_blit_indexed_320x200(video->rgb, RADWOLF_WIDTH,
                                             indices, pitch_bytes, palette);
    if (result == 0)
        result = video->present(video->user, video->rgb, RADWOLF_WIDTH);
    video->busy = 0;
    if (result == 0)
        ++video->presented;
    else
        ++video->rejected;
    return result == 0 ? 0 : -1;
}
