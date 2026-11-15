/* GPL-3.0-or-later. A 64x64 traversal bitmap costs 512 bytes. */
#include "radwolf_map_history.h"
#include <string.h>

radwolf_map_history radwolf_current_map_history;

static int valid(const radwolf_map_history *history)
{
    return history && history->width && history->height &&
           history->width <= 64 && history->height <= 64 && history->level <= 65535;
}

void radwolf_map_history_reset(radwolf_map_history *history, unsigned width,
                               unsigned height, unsigned level)
{
    if (!history) return;
    memset(history, 0, sizeof(*history));
    if (!width || !height || width > 64 || height > 64 || level > 65535) return;
    history->width = width;
    history->height = height;
    history->level = level;
}

static int in_bounds(const radwolf_map_history *history, int x, int y)
{
    return valid(history) && x >= 0 && y >= 0 &&
           (unsigned)x < history->width && (unsigned)y < history->height;
}

void radwolf_map_history_mark(radwolf_map_history *history, int x, int y)
{
    if (!in_bounds(history, x, y)) return;
    unsigned bit = (unsigned)x * 64u + (unsigned)y;
    history->visited[bit / 8] |= (uint8_t)(1u << (bit % 8));
}

int radwolf_map_history_contains(const radwolf_map_history *history, int x, int y)
{
    if (!in_bounds(history, x, y)) return 0;
    unsigned bit = (unsigned)x * 64u + (unsigned)y;
    return (history->visited[bit / 8] & (1u << (bit % 8))) != 0;
}

static uint32_t record_crc(const uint8_t *record)
{
    uint32_t crc = UINT32_C(0xffffffff);
    for (unsigned i = 0; i < RADWOLF_MAP_HISTORY_RECORD_BYTES; ++i) {
        if (i >= 12 && i < 16) continue;
        crc ^= record[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & (0u - (crc & 1u)));
    }
    return ~crc;
}

int radwolf_map_history_write(const radwolf_map_history *history, FILE *file)
{
    if (!valid(history) || !file) return -1;
    uint8_t record[RADWOLF_MAP_HISTORY_RECORD_BYTES] = {'R','W','M','A','P',1};
    record[6] = (uint8_t)history->width;
    record[7] = (uint8_t)history->height;
    record[8] = (uint8_t)history->level;
    record[9] = (uint8_t)(history->level >> 8);
    record[10] = 0; /* 512 payload bytes, little-endian. */
    record[11] = 2;
    memcpy(record + 16, history->visited, RADWOLF_MAP_HISTORY_BYTES);
    uint32_t crc = record_crc(record);
    for (unsigned i = 0; i < 4; ++i) record[12 + i] = (uint8_t)(crc >> (8u * i));
    return fwrite(record, 1, sizeof(record), file) == sizeof(record) ? 0 : -1;
}

int radwolf_map_history_read(radwolf_map_history *history, FILE *file,
                             unsigned width, unsigned height, unsigned level)
{
    radwolf_map_history_reset(history, width, height, level);
    if (!valid(history) || !file) return 0;
    uint8_t record[RADWOLF_MAP_HISTORY_RECORD_BYTES];
    if (fread(record, 1, sizeof(record), file) != sizeof(record)) {
        /* EOF is normal when opening a save made before traversal tracking. */
        clearerr(file);
        return 0;
    }
    if (memcmp(record, "RWMAP", 5) || record[5] != 1 ||
        record[6] != width || record[7] != height ||
        ((unsigned)record[8] | ((unsigned)record[9] << 8)) != level ||
        record[10] != 0 || record[11] != 2)
        return 0;
    uint32_t expected_crc = 0;
    for (unsigned i = 0; i < 4; ++i) expected_crc |= (uint32_t)record[12 + i] << (8u * i);
    if (record_crc(record) != expected_crc) return 0;
    memcpy(history->visited, record + 16, RADWOLF_MAP_HISTORY_BYTES);
    return 1;
}
