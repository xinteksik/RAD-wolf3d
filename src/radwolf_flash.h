/* GPL-3.0-or-later. Localized pickup/damage feedback, not a scene palette. */
#ifndef RADWOLF_FLASH_H
#define RADWOLF_FLASH_H
#include <stddef.h>
#include <stdint.h>
#include "radwolf_video_config.h"
extern unsigned radwolf_flash_mode;
void radwolf_flash_event(unsigned damage, unsigned pickup);
void radwolf_flash_prepare_view(int x, int y, int width, int height, int status_height);
void radwolf_flash_apply(uint32_t *pixels, size_t pitch);
#endif
