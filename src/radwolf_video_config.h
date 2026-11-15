/* GPL-3.0-or-later. Video controls in the same units as the preview tool. */
#ifndef RADWOLF_VIDEO_CONFIG_H
#define RADWOLF_VIDEO_CONFIG_H
#include <stdio.h>
enum radwolf_flash_mode {
    RADWOLF_FLASH_OFF, RADWOLF_FLASH_FRAME, RADWOLF_FLASH_STATUS, RADWOLF_FLASH_FULL
};
typedef struct radwolf_video_config {
    unsigned dither_percent;
    unsigned brightness;
    unsigned exposure;
    unsigned flash_mode;
} radwolf_video_config;

/* Missing input uses 0/20/350. Invalid input returns -1 and all defaults. */
int radwolf_video_config_parse(radwolf_video_config *config, const char *text);
int radwolf_video_config_read(radwolf_video_config *config, FILE *file);
#endif
