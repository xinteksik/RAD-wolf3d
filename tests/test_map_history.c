/* GPL-3.0-or-later. Traversal, persistence, legacy saves and damaged trailers. */
#include "radwolf_map_history.h"
#include <assert.h>
#include <string.h>

static void reject_record(const uint8_t *record, size_t length)
{
    FILE *file = tmpfile();
    assert(file);
    assert(fwrite(record, 1, length, file) == length);
    rewind(file);
    radwolf_map_history loaded;
    radwolf_map_history_reset(&loaded, 64, 64, 0x1234);
    radwolf_map_history_mark(&loaded, 7, 9);
    assert(radwolf_map_history_read(&loaded, file, 64, 64, 0x1234) == 0);
    assert(!radwolf_map_history_contains(&loaded, 7, 9));
    assert(!feof(file) && !ferror(file));
    fclose(file);
}

void test_map_history(void)
{
    radwolf_map_history history, loaded;
    radwolf_map_history_reset(&history, 64, 64, 0x1234);
    radwolf_map_history_mark(&history, 7, 9);
    radwolf_map_history_mark(&history, 8, 9);
    radwolf_map_history_mark(&history, 7, 9); /* Revisits are idempotent. */
    radwolf_map_history_mark(&history, 0, 0);
    radwolf_map_history_mark(&history, 63, 63);
    radwolf_map_history_mark(&history, -1, 9);
    radwolf_map_history_mark(&history, 64, 9);
    assert(radwolf_map_history_contains(&history, 7, 9));
    assert(!radwolf_map_history_contains(&history, 9, 7));
    assert(!radwolf_map_history_contains(&history, 7, 10));
    assert(!radwolf_map_history_contains(&history, -1, 9));
    assert(!radwolf_map_history_contains(&history, 64, 9));
    assert(history.visited[0] == 1 && history.visited[511] == 128);

    FILE *file = tmpfile();
    assert(file);
    static const char legacy[] = "unchanged legacy game data";
    assert(fwrite(legacy, 1, sizeof(legacy), file) == sizeof(legacy));
    long trailer_offset = ftell(file);
    assert(radwolf_map_history_write(&history, file) == 0);
    assert(ftell(file) == trailer_offset + (long)RADWOLF_MAP_HISTORY_RECORD_BYTES);
    rewind(file);
    char prefix[sizeof(legacy)];
    assert(fread(prefix, 1, sizeof(prefix), file) == sizeof(prefix));
    assert(memcmp(prefix, legacy, sizeof(legacy)) == 0);
    assert(radwolf_map_history_read(&loaded, file, 64, 64, 0x1234) == 1);
    assert(memcmp(loaded.visited, history.visited, sizeof(history.visited)) == 0);
    assert(loaded.level == 0x1234 && loaded.width == 64 && loaded.height == 64);

    assert(fseek(file, trailer_offset, SEEK_SET) == 0);
    uint8_t record[RADWOLF_MAP_HISTORY_RECORD_BYTES];
    assert(fread(record, 1, sizeof(record), file) == sizeof(record));
    assert(!memcmp(record, "RWMAP", 5) && record[5] == 1);
    assert(record[6] == 64 && record[7] == 64);
    assert(record[8] == 0x34 && record[9] == 0x12);
    assert(record[10] == 0 && record[11] == 2);
    assert(record[16] == 1 && record[527] == 128);

    /* Loading a different level or shape must never reuse another route. */
    assert(fseek(file, trailer_offset, SEEK_SET) == 0);
    assert(!radwolf_map_history_read(&loaded, file, 64, 64, 0x1235));
    assert(!radwolf_map_history_contains(&loaded, 7, 9));
    assert(fseek(file, trailer_offset, SEEK_SET) == 0);
    assert(!radwolf_map_history_read(&loaded, file, 32, 64, 0x1234));
    assert(!radwolf_map_history_contains(&loaded, 7, 9));
    fclose(file);

    /* No trailer, partial writes, unknown versions and bit flips fall back. */
    static const size_t lengths[] = {0, 1, 15, 16, 100, 527};
    for (unsigned i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i)
        reject_record(record, lengths[i]);
    static const unsigned mutations[] = {0, 5, 6, 8, 10, 12, 100, 527};
    for (unsigned i = 0; i < sizeof(mutations) / sizeof(mutations[0]); ++i) {
        record[mutations[i]] ^= 1;
        reject_record(record, sizeof(record));
        record[mutations[i]] ^= 1;
    }

    /* A fresh level/restart clears all previous visits, including corners. */
    radwolf_map_history_reset(&loaded, 64, 64, 2);
    for (int x = 0; x < 64; ++x)
        for (int y = 0; y < 64; ++y) {
            assert(!radwolf_map_history_contains(&loaded, x, y));
            radwolf_map_history_mark(&loaded, x, y);
        }
    for (unsigned i = 0; i < sizeof(loaded.visited); ++i)
        assert(loaded.visited[i] == 255);
    radwolf_map_history_reset(&loaded, 3, 2, 3);
    radwolf_map_history_mark(&loaded, 2, 1);
    radwolf_map_history_mark(&loaded, 3, 1);
    assert(radwolf_map_history_contains(&loaded, 2, 1));
    assert(!radwolf_map_history_contains(&loaded, 3, 1));
    radwolf_map_history_reset(&loaded, 65, 64, 3);
    assert(!loaded.width && !radwolf_map_history_contains(&loaded, 0, 0));
    assert(radwolf_map_history_write(&loaded, NULL) == -1);
    assert(radwolf_map_history_write(&history, NULL) == -1);
    radwolf_map_history_mark(NULL, 0, 0);
    radwolf_map_history_reset(NULL, 64, 64, 0);
    assert(!radwolf_map_history_read(NULL, NULL, 64, 64, 0));
}
