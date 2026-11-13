/* GPL-3.0-or-later. Full-song render and exact loop checks with private assets. */
#include "radwolf_midi.h"
#include "../third_party/tml.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
static const char *const names[] = {
    "CORNER", "DUNGEON", "WARMARCH", "GETTHEM", "HEADACHE", "HITLWLTZ",
    "INTROCW3", "NAZI_NOR", "NAZI_OMI", "POW", "SALUTE", "SEARCHN",
    "SUSPENSE", "VICTORS", "WONDERIN", "FUNKYOU", "ENDLEVEL", "GOINGAFT",
    "PREGNANT", "ULTIMATE", "NAZI_RAP", "ZEROHOUR", "TWELFTH", "ROSTER",
    "URAHERO", "VICMARCH", "PACMAN"
};
void radwolf_boot_trace(const char *message) { puts(message); }
int main(void)
{
    setbuf(stdout, NULL);
    radwolf_midi_configure_stereo(70);
    clock_t begin = clock();
    double seconds = 0;
    for (unsigned song = 0; song < RADWOLF_MIDI_SONGS; ++song) {
        char path[80];
        snprintf(path, sizeof(path), "MIDI/%s.MID", names[song]);
        FILE *file = fopen(path, "rb");
        assert(file && !fseek(file, 0, SEEK_END));
        long size = ftell(file);
        assert(size > 0 && size < 1024 * 1024 && !fseek(file, 0, SEEK_SET));
        void *data = malloc((size_t)size);
        assert(data && fread(data, 1, (size_t)size, file) == (size_t)size);
        fclose(file);
        tml_message *events = tml_load_memory(data, (int)size);
        free(data);
        unsigned duration;
        assert(events && tml_get_info(events, NULL, NULL, NULL, NULL, &duration));
        tml_free(events);
        uint64_t length = ((((uint64_t)duration * 44100 / 1000) + 2) & ~(uint64_t)1) / 2;
        assert(length > 512 && radwolf_midi_start(song, 0));
        int16_t first[512 * 2] = {0}, buffer[1024 * 2];
        radwolf_midi_mix(first, 512);
        uint64_t energy = 0;
        unsigned peak = 0, clipped = 0;
        for (uint64_t frame = 512; frame < length;) {
            unsigned count = length - frame > 1024 ? 1024 : (unsigned)(length - frame);
            memset(buffer, 0, sizeof(buffer));
            radwolf_midi_mix(buffer, count);
            for (unsigned i = 0; i < count * 2; ++i) {
                unsigned sample = buffer[i] < 0 ? -(int)buffer[i] : buffer[i];
                if (sample > peak) peak = sample;
                energy += sample;
                if (sample >= 32767) ++clipped;
            }
            frame += count;
        }
        memset(buffer, 0, sizeof(buffer));
        radwolf_midi_mix(buffer, 512);
        assert(!memcmp(buffer, first, sizeof(first)));
        printf("%s: %.3f seconds, peak=%u, clipped=%u, exact loop=yes\n",
               names[song], length / 22050.0, peak, clipped);
        assert(energy > 100000 && !clipped);
        seconds += length / 22050.0;
    }
    assert(!radwolf_midi_start(RADWOLF_MIDI_SONGS, 0));
    assert(!radwolf_midi_active());
    printf("All 27 tracks passed: %.1f audio seconds, %.2f host CPU seconds\n",
           seconds, (clock() - begin) / (double)CLOCKS_PER_SEC);
    return 0;
}
