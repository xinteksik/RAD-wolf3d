/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#ifndef RADWOLF_STDIO_FILE_H
#define RADWOLF_STDIO_FILE_H
#include "radwolf_file.h"
/* File must not already own an open handle. Opens read-only; no writes. */
int radwolf_stdio_open(radwolf_file *file, const char *path);
#endif
