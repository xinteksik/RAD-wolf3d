/* GPL-3.0-or-later. Explicit, local pre-conversion video capture. */
#ifndef RADWOLF_CAPTURE_H
#define RADWOLF_CAPTURE_H
#include <stdint.h>
#include <stdio.h>
int radwolf_capture_write(FILE *file, const uint32_t *pixels);
void radwolf_capture_request(void);
void radwolf_capture_frame(const uint32_t *pixels);
#endif
