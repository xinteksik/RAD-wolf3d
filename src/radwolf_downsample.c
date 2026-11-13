/* GPL-3.0-or-later. Low-pass before decimation; 47 input samples of delay. */
#include "radwolf_downsample.h"
#include "radwolf_downsample_coeffs.h"
#include <string.h>

void radwolf_downsample_reset(radwolf_downsampler *state)
{
    memset(state, 0, sizeof(*state));
}

unsigned radwolf_downsample(radwolf_downsampler *state, const float *input,
                           unsigned input_frames, float *output)
{
    unsigned written = 0;
    for (unsigned frame = 0; frame < input_frames; ++frame) {
        for (unsigned channel = 0; channel < 2; ++channel)
            state->history[channel][state->next] = *input++;
        state->next = (state->next + 1) & 127;
        state->phase ^= 1;
        if (state->phase) continue;
        for (unsigned channel = 0; channel < 2; ++channel) {
            const float *history = state->history[channel];
            unsigned newest = (state->next - 1) & 127;
            float value = radwolf_fir_coefficients[47] * history[(newest - 47) & 127];
            for (unsigned tap = 0; tap < 47; ++tap)
                value += radwolf_fir_coefficients[tap] *
                    (history[(newest - tap) & 127] + history[(newest - (94 - tap)) & 127]);
            *output++ = value;
        }
        ++written;
    }
    return written;
}
