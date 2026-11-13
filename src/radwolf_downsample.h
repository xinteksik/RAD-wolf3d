/* GPL-3.0-or-later. Streaming stereo FIR decimator, exactly 44100 -> 22050 Hz. */
#ifndef RADWOLF_DOWNSAMPLE_H
#define RADWOLF_DOWNSAMPLE_H
typedef struct {
    float history[2][128];
    unsigned next, phase;
} radwolf_downsampler;
void radwolf_downsample_reset(radwolf_downsampler *state);
/* Output capacity must be at least (input_frames + 1) / 2 stereo frames. */
unsigned radwolf_downsample(radwolf_downsampler *state, const float *input,
                           unsigned input_frames, float *output);
#endif
