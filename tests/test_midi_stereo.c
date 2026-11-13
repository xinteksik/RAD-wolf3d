/* GPL-3.0-or-later. Render synthetic MIDI fixtures with a private test SF2. */
#include "radwolf_midi.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FRAMES 22050
static int16_t original[FRAMES * 2], spread[FRAMES * 2];
void radwolf_boot_trace(const char *message) { puts(message); }
static double left_share(const int16_t *samples)
{
    double left = 0, right = 0;
    for (unsigned i = 0; i < FRAMES; ++i) {
        left += (double)samples[i * 2] * samples[i * 2];
        right += (double)samples[i * 2 + 1] * samples[i * 2 + 1];
    }
    assert(left + right > 100000);
    return left / (left + right);
}
static void render(unsigned song, unsigned width, int16_t *samples)
{
    memset(samples, 0, FRAMES * 2 * sizeof(*samples));
    radwolf_midi_configure_stereo(width);
    assert(radwolf_midi_start(song, 0));
    radwolf_midi_mix(samples, FRAMES);
}
int main(void)
{
    render(0, 0, original);
    render(0, 100, spread);
    assert(left_share(spread) > left_share(original) + 0.15);
    render(1, 0, original);
    render(1, 100, spread);
    assert(left_share(spread) < left_share(original) - 0.15);
    render(0, 70, original);
    render(0, 100, spread);
    assert(left_share(spread) > left_share(original));
    /* Bass, drums and tuba remain centered; explicit MIDI panorama wins. */
    for (unsigned song = 2; song < 9; ++song) {
        render(song, 0, original);
        render(song, 100, spread);
        assert(!memcmp(original, spread, sizeof(original)));
        assert(left_share(spread) >= 0);
    }
    radwolf_midi_stop();
    puts("MIDI stereo passed: automatic L/R spread, width, centered bass/drums, explicit pan precedence");
    return 0;
}
