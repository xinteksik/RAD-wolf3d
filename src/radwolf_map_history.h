/* GPL-3.0-or-later. Per-level traversal history and optional save trailer. */
#ifndef RADWOLF_MAP_HISTORY_H
#define RADWOLF_MAP_HISTORY_H
#include <stdint.h>
#include <stdio.h>

#define RADWOLF_MAP_HISTORY_BYTES 512u
#define RADWOLF_MAP_HISTORY_RECORD_BYTES 528u

typedef struct radwolf_map_history {
    uint8_t visited[RADWOLF_MAP_HISTORY_BYTES];
    unsigned width, height, level;
} radwolf_map_history;

extern radwolf_map_history radwolf_current_map_history;
/* Invalid dimensions leave the history disabled and empty. */
void radwolf_map_history_reset(radwolf_map_history *history, unsigned width,
                               unsigned height, unsigned level);
void radwolf_map_history_mark(radwolf_map_history *history, int x, int y);
int radwolf_map_history_contains(const radwolf_map_history *history, int x, int y);
/* Fixed little-endian, versioned, CRC32-checked trailer; no native structs. */
int radwolf_map_history_write(const radwolf_map_history *history, FILE *file);
/* 1 = restored; 0 = absent/invalid, with an empty history for this level.
 * Reads from the current file position, immediately after legacy save data. */
int radwolf_map_history_read(radwolf_map_history *history, FILE *file,
                             unsigned width, unsigned height, unsigned level);
#endif
