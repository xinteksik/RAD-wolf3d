/* GPL-3.0-or-later. Verify gain, rejection, streaming and stereo isolation. */
#include "radwolf_downsample.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define FRAMES 8192
static float input[FRAMES * 2], output[FRAMES + 2], split[FRAMES + 2];

static double tone_gain(double frequency)
{
    radwolf_downsampler state;
    radwolf_downsample_reset(&state);
    for (unsigned i = 0; i < FRAMES; ++i) {
        input[i * 2] = (float)sin(6.283185307179586 * frequency * i / 44100);
        input[i * 2 + 1] = 0;
    }
    assert(radwolf_downsample(&state, input, FRAMES, output) == FRAMES / 2);
    double energy = 0;
    for (unsigned i = 128; i < FRAMES / 2; ++i) {
        energy += output[i * 2] * output[i * 2];
        assert(output[i * 2 + 1] == 0);
    }
    return sqrt(energy * 2 / (FRAMES / 2 - 128));
}

int main(void)
{
    double pass = tone_gain(1000), stop = tone_gain(12000);
    assert(pass > 0.995 && pass < 1.005);
    assert(stop < 0.001); /* At least 60 dB rejection before aliasing. */
    assert(tone_gain(18000) < 0.001);
    radwolf_downsampler state;
    radwolf_downsample_reset(&state);
    for (unsigned i = 0; i < FRAMES * 2; ++i) input[i] = 0.3f;
    output[FRAMES] = 123;
    output[FRAMES + 1] = -123;
    assert(radwolf_downsample(&state, input, FRAMES, output) == FRAMES / 2);
    for (unsigned i = 128; i < FRAMES; ++i) assert(fabs(output[i] - 0.3) < 0.00001);
    assert(output[FRAMES] == 123 && output[FRAMES + 1] == -123);
    radwolf_downsample_reset(&state);
    unsigned count = 0;
    for (unsigned i = 0; i < FRAMES; ++i)
        count += radwolf_downsample(&state, input + i * 2, 1, split + count * 2);
    assert(count == FRAMES / 2);
    assert(!memcmp(output, split, FRAMES * sizeof(float)));
    printf("Downsampler passed: 1 kHz gain %.6f, 12 kHz rejection %.1f dB\n",
           pass, 20 * log10(stop));
    return 0;
}
