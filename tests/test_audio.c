/* Host checks for ring scheduling, mixing and sample lifetime. */
#include "baremetal/SDL_mixer.h"
#include "radwolf_audio_config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint64_t soundFirstuSecTick;
uint8_t soundRingBuffer[16384];
static uint8_t lut[256];
const uint8_t *mahoneyLUT = lut;
unsigned radwolf_sid2_address;
unsigned radwolf_sid2_model;
unsigned radwolf_sid1_address = 0xd418, radwolf_sid1_model;
unsigned radwolf_sid1_boot_model(void) { return 8580; }
int radwolf_sid1_initialize(unsigned base, unsigned model)
{ radwolf_sid1_address = base+0x18; radwolf_sid1_model = model; return 1; }
const unsigned char *radwolf_sid2_lut;
int radwolf_sid2_initialize(unsigned base, unsigned model)
{ (void)base; (void)model; return 0; }
void disableInterrupts(void) { }
void enableInterrupts(void) { }
static uint64_t now_us = 1000000;
static unsigned generated, completions, starts;
static unsigned music_stereo_width;
void radwolf_midi_configure_stereo(unsigned width) { music_stereo_width = width; }
uint64_t GetuSec(void) { return now_us; }
void startSIDSamplePlayer(void) { ++starts; }
void stopSIDSamplePlayer(void) { }
static void music(void *user, uint8_t *bytes, int size)
{
    (void)user;
    int16_t *samples = (int16_t *)(void *)bytes;
    for (int i = 0; i < size / 2; ++i) samples[i] = 8192;
    generated += (unsigned)size / 4;
}
static void finished(int channel)
{
    assert(channel == 2);
    ++completions;
}
int main(void)
{
    for (unsigned i = 0; i < 256; ++i) lut[i] = (uint8_t)i;
    radwolf_audio_pump();
    assert(starts == 0);
    assert(Mix_OpenAudioDevice(44100, AUDIO_S16, 2, 2048, NULL, 1) == 0);
    int rate = 0;
    assert(Mix_QuerySpec(&rate, NULL, NULL) && rate == 22050);
    Mix_HookMusic(music, NULL);
    Mix_ChannelFinished(finished);
    radwolf_audio_pump();
    assert(starts == 1 && generated == 2205);
    assert(music_stereo_width == 0);
    assert(soundRingBuffer[0] == 160);
    assert(soundRingBuffer[2204] == 160 && soundRingBuffer[2205] == 128);
    radwolf_audio_pump();
    assert(generated == 2205);

    /* One mono frame must play once, pan to the left and release its channel. */
    int16_t pcm[] = { 8192 };
    Mix_Chunk chunk = { pcm, 1 };
    assert(Mix_SetPanning(2, 255, 0));
    assert(Mix_PlayChannel(2, &chunk, 0) == 2);
    now_us += 10000;
    radwolf_audio_pump();
    assert(soundRingBuffer[2205] == 176);
    assert(soundRingBuffer[2206] == 160 && completions == 1);
    assert(Mix_GroupAvailable(1) == 2);
    assert(Mix_PlayChannel(8, &chunk, 0) == -1);

    /* Stereo preserves position instead of summing it to mono. */
    radwolf_sid2_address = 0xd518;
    radwolf_sid2_lut = lut;
    assert(Mix_SetPanning(2, 0, 255));
    assert(Mix_PlayChannel(2, &chunk, 0) == 2);
    now_us += 10000;
    radwolf_audio_pump();
    assert(soundRingBuffer[2425] == 160);
    assert(radwolf_right_ring[2425] == 192);
    assert(completions == 2);
    radwolf_sid2_address = 0;

    unsigned before = generated;
    now_us += 10000000;
    radwolf_audio_pump();
    assert(generated - before == 2205);

    /* Copy owned WAV samples; reject truncated sample payloads. */
    uint8_t wav[46] = {0};
    memcpy(wav, "RIFF", 4); memcpy(wav+36, "data", 4);
    wav[40] = 2; wav[44] = 0x34; wav[45] = 0x12;
    Mix_Chunk *loaded = Mix_LoadWAV_RW(SDL_RWFromMem(wav, sizeof(wav)), 1);
    assert(loaded && loaded->count == 1 && loaded->samples[0] == 0x1234);
    memset(wav+44, 0, 2);
    assert(loaded->samples[0] == 0x1234);
    Mix_FreeChunk(loaded);
    wav[40] = 4;
    assert(!Mix_LoadWAV_RW(SDL_RWFromMem(wav, sizeof(wav)), 1));
    puts("audio tests passed");
    return 0;
}
