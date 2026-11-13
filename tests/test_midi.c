/* GPL-3.0-or-later. Run from a directory with private MIDI/SF2 test assets. */
#include "radwolf_midi.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void radwolf_boot_trace(const char *stage) { puts(stage); }

int main(int argc, char **argv)
{
    int16_t buffer[2 * 1024 + 2];
    if (argc > 1 && !strcmp(argv[1], "--partial")) {
        assert(!radwolf_midi_start(14, 0));
        assert(radwolf_midi_start(0, 0));
        unsigned energy = 0;
        for (unsigned block = 0; block < 4; ++block) {
            memset(buffer, 0, sizeof(buffer));
            radwolf_midi_mix(buffer, 1024);
            for (unsigned i = 0; i < 2048; ++i)
                energy += buffer[i] < 0 ? -(int)buffer[i] : buffer[i];
        }
        assert(energy > 1000);
        assert(!radwolf_midi_start(14, 0));
        assert(!radwolf_midi_active());
        assert(radwolf_midi_start(0, 1));
        puts("Individual malformed-track fallback passed");
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "--fallback")) {
        for (unsigned song = 0; song < RADWOLF_MIDI_SONGS; ++song)
            assert(!radwolf_midi_start(song, 0));
        assert(!radwolf_midi_active());
        for (unsigned i = 0; i < 2050; ++i) buffer[i] = 123;
        radwolf_midi_mix(buffer, 1024);
        for (unsigned i = 0; i < 2050; ++i) assert(buffer[i] == 123);
        assert(!radwolf_midi_start(14, 0));
        puts("MIDI fallback passed");
        return 0;
    }
    radwolf_midi_configure_stereo(70);
    assert(radwolf_midi_start(14, 0));
    assert(radwolf_midi_active());
    /* Rendering must produce identical audio regardless of callback size. */
    enum { FRAMES = 22050 * 5 };
    int16_t *reference = calloc(FRAMES * 2, sizeof(*reference));
    int16_t *split = calloc(FRAMES * 2, sizeof(*split));
    assert(reference && split);
    radwolf_midi_mix(reference, FRAMES);
    radwolf_midi_stop();
    assert(!radwolf_midi_active());
    int16_t expected_next[1024 * 2] = {0};
    assert(radwolf_midi_start(14, 1));
    radwolf_midi_mix(expected_next, 1024);
    assert(radwolf_midi_start(14, 0));
    for (unsigned i = 0; i < FRAMES;) {
        unsigned count = FRAMES - i > 31 ? 31 : FRAMES - i;
        radwolf_midi_mix(split + i * 2, count);
        i += count;
    }
    assert(!memcmp(reference, split, FRAMES * 2 * sizeof(*reference)));
    radwolf_midi_stop();
    assert(radwolf_midi_start(0, 0));
    memset(buffer, 0, sizeof(buffer));
    radwolf_midi_mix(buffer, 1024);
    assert(radwolf_midi_start(14, 1));
    memset(buffer, 0, sizeof(buffer));
    radwolf_midi_mix(buffer, 1024);
    assert(!memcmp(buffer, expected_next, sizeof(expected_next)));
    free(reference);
    free(split);
    assert(radwolf_midi_start(14, 0));
    FILE *output = argc > 1 ? fopen(argv[1], "wb") : NULL;
    uint64_t energy = 0;
    unsigned clipped = 0, peak = 0;
    /* Long enough to cross the WONDERIN loop boundary several times. */
    for (unsigned frame = 0; frame < 22050u * 240u;) {
        unsigned count = 22050u * 240u - frame;
        if (count > 1024) count = 1024;
        memset(buffer, 0, sizeof(buffer));
        buffer[count * 2] = 12345;
        buffer[count * 2 + 1] = -12345;
        radwolf_midi_mix(buffer, count);
        assert(buffer[count * 2] == 12345 && buffer[count * 2 + 1] == -12345);
        for (unsigned i = 0; i < count * 2; ++i) {
            unsigned magnitude = buffer[i] < 0 ? -(int)buffer[i] : buffer[i];
            energy += magnitude;
            if (magnitude > peak) peak = magnitude;
            if (buffer[i] == 32767 || buffer[i] == -32768) ++clipped;
        }
        if (output && frame < 22050u * 30u) {
            unsigned write_count = 22050u * 30u - frame;
            if (write_count > count) write_count = count;
            assert(fwrite(buffer, sizeof(int16_t) * 2, write_count, output) == write_count);
        }
        frame += count;
    }
    if (output) assert(!fclose(output));
    assert(energy > 100000);
    assert(clipped == 0);
    radwolf_midi_stop();
    memset(buffer, 0, sizeof(buffer));
    radwolf_midi_mix(buffer, 1024);
    for (unsigned i = 0; i < 2050; ++i) assert(buffer[i] == 0);
    printf("MIDI render passed: peak=%u, clipped=%u, energy=%llu\n",
           peak, clipped, (unsigned long long)energy);
    return 0;
}
