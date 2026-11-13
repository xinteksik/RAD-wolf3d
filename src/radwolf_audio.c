/* GPL-3.0-or-later. Cooperative Wolf mixer feeding the RAD SID sample ring. */
#include "baremetal/SDL_mixer.h"
#include "radwolf_audio_config.h"
#include "radwolf_midi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
void radwolf_boot_trace(const char *stage)
{
    FILE *file = fopen("SD:RADWOLF/WOLFBOOT.LOG", "a");
    if (file) { fprintf(file, "%s\n", stage); fclose(file); }
}
#define RATE 22050u
#define RING_SIZE 16384u
#define AHEAD 2205u
extern uint64_t GetuSec(void);
extern uint64_t soundFirstuSecTick;
extern uint8_t soundRingBuffer[];
extern const uint8_t *mahoneyLUT;
extern void startSIDSamplePlayer(void);
extern void stopSIDSamplePlayer(void);
extern void disableInterrupts(void);
extern void enableInterrupts(void);
unsigned char radwolf_right_ring[RING_SIZE];
static void (*music_callback)(void *, uint8_t *, int);
static void (*post_callback)(void *, uint8_t *, int);
static void (*finished_callback)(int);
static void *music_user, *post_user;
static struct {
    Mix_Chunk *chunk;
    unsigned position;
    uint64_t age;
    uint8_t left, right;
} voices[MIX_CHANNELS];
static uint64_t written, serial;
static int opened, started, pumping, reserved = 2;
static int traced_pump;

static int16_t clamp(int value)
{
    if (value > 32767) return 32767;
    if (value < -32768) return -32768;
    return (int16_t)value;
}

void radwolf_audio_pump(void)
{
    if (!opened || pumping) return;
    int trace = !traced_pump;
    if (trace) traced_pump = 1;
    if (trace) radwolf_boot_trace("pump: begin");
    pumping = 1;
    if (!started) {
        stopSIDSamplePlayer();
        if (trace) radwolf_boot_trace("pump: timer stopped");
        char text[1024];
        radwolf_audio_config config;
        radwolf_audio_config_parse(&config, NULL);
        FILE *file = fopen("SD:RADWOLF/AUDIO.CFG", "rb");
        if (file) {
            size_t size = fread(text, 1, sizeof(text)-1, file);
            int valid = !ferror(file) && feof(file);
            fclose(file);
            text[size] = 0;
            if (!valid || radwolf_audio_config_parse(&config, text) != 0) {
                config.stereo = 0;
                radwolf_boot_trace("stereo: invalid config, mono fallback");
            }
        }
        disableInterrupts();
        unsigned boot_model = radwolf_sid1_boot_model();
        int primary_ok = radwolf_sid1_initialize(config.primary_address, config.primary_model);
        enableInterrupts();
        char primary_report[96];
        snprintf(primary_report, sizeof(primary_report), "audio: SID1 base=$%04X boot=%u requested=%u selected=%u ok=%d",
                 config.primary_address, boot_model, config.primary_model, radwolf_sid1_model, primary_ok);
        radwolf_boot_trace(primary_report);
        if (config.stereo && primary_ok) {
            radwolf_stereo_width = config.width;
            disableInterrupts();
            int enabled = radwolf_sid2_initialize(config.address, config.model);
            enableInterrupts();
            radwolf_boot_trace(enabled ? "stereo: SID2 enabled" : "stereo: detection failed, mono fallback");
            char report[96];
            snprintf(report, sizeof(report), "stereo: SID2 base=$%04X requested=%u selected=%u width=%u",
                     config.address, config.model, radwolf_sid2_model, config.width);
            radwolf_boot_trace(report);
        }
        radwolf_midi_configure_stereo(radwolf_sid2_address ? config.music_width : 0);
        soundFirstuSecTick = GetuSec();
        memset(soundRingBuffer, mahoneyLUT[128], RING_SIZE);
        memset(radwolf_right_ring, radwolf_sid2_lut ? radwolf_sid2_lut[128] : 0, RING_SIZE);
        written = 0;
        started = 1;
        startSIDSamplePlayer();
        if (trace) radwolf_boot_trace("pump: timer started");
    }
    uint64_t now = (GetuSec() - soundFirstuSecTick) * RATE / 1000000u;
    /* Drop elapsed output after a long load; never overwrite live ring data. */
    if (written < now) written = now;
    uint64_t target = now + AHEAD;
    while (written < target) {
        int16_t stereo[256 * 2];
        unsigned n = target - written > 256 ? 256 : (unsigned)(target - written);
        memset(stereo, 0, n * 4);
        if (trace) radwolf_boot_trace("pump: OPL render begin");
        if (music_callback) music_callback(music_user, (uint8_t *)stereo, n * 4);
        if (trace) radwolf_boot_trace("pump: OPL render complete");
        for (int c = 0; c < MIX_CHANNELS; ++c) {
            for (unsigned i = 0; i < n && voices[c].chunk; ++i) {
                Mix_Chunk *chunk = voices[c].chunk;
                int sample = chunk->samples[voices[c].position++];
                stereo[2*i] = clamp(stereo[2*i] + sample * voices[c].left / 255);
                stereo[2*i+1] = clamp(stereo[2*i+1] + sample * voices[c].right / 255);
                if (voices[c].position >= chunk->count) {
                    voices[c].chunk = NULL;
                    if (finished_callback) finished_callback(c);
                }
            }
        }
        if (trace) radwolf_boot_trace("pump: PC render begin");
        if (post_callback) post_callback(post_user, (uint8_t *)stereo, n * 4);
        if (trace) radwolf_boot_trace("pump: PC render complete");
        for (unsigned i = 0; i < n; ++i) {
            unsigned position = written++ & (RING_SIZE-1);
            int left = stereo[2*i], right = stereo[2*i+1];
            int output = radwolf_sid2_address ? left : (left + right) / 2;
            if (radwolf_sid2_address)
                radwolf_right_ring[position] = radwolf_sid2_lut[(right + 32768) >> 8];
            soundRingBuffer[position] = mahoneyLUT[(output + 32768) >> 8];
        }
        if (trace) radwolf_boot_trace("pump: ring write complete");
        trace = 0;
    }
    pumping = 0;
    if (traced_pump == 1) {
        traced_pump = 2;
        radwolf_boot_trace("pump: complete");
    }
}

int Mix_OpenAudioDevice(int rate, uint16_t format, int channels, int size,
                       const char *device, int flags)
{
    radwolf_boot_trace("mixer: open");
    (void)rate; (void)size; (void)device; (void)flags;
    if (format != AUDIO_S16 || channels != 2) return -1;
    for (int c = 0; c < MIX_CHANNELS; ++c)
        voices[c].left = voices[c].right = 255;
    opened = 1;
    return 0;
}
int Mix_QuerySpec(int *rate, uint16_t *format, int *channels)
{
    if (rate) *rate = RATE;
    if (format) *format = AUDIO_S16;
    if (channels) *channels = 2;
    return opened;
}
int Mix_ReserveChannels(int count) { reserved = count; return count; }
int Mix_GroupChannels(int first, int last, int tag)
{ (void)tag; return last-first+1; }
int Mix_GroupAvailable(int tag)
{
    (void)tag;
    for (int c = reserved; c < MIX_CHANNELS; ++c) if (!voices[c].chunk) return c;
    return -1;
}
int Mix_GroupOldest(int tag)
{
    (void)tag;
    int oldest = reserved;
    for (int c = reserved+1; c < MIX_CHANNELS; ++c)
        if (voices[c].age < voices[oldest].age) oldest = c;
    return oldest;
}
void Mix_HookMusic(void (*callback)(void *, uint8_t *, int), void *user)
{ music_callback = callback; music_user = user; }
void Mix_SetPostMix(void (*callback)(void *, uint8_t *, int), void *user)
{ post_callback = callback; post_user = user; }
void Mix_ChannelFinished(void (*callback)(int)) { finished_callback = callback; }
int Mix_SetPanning(int channel, uint8_t left, uint8_t right)
{
    if (channel < 0 || channel >= MIX_CHANNELS) return 0;
    voices[channel].left = left; voices[channel].right = right;
    return 1;
}
SDL_RWops *SDL_RWFromMem(void *data, int size)
{
    SDL_RWops *source = malloc(sizeof(*source));
    if (source) { source->data = data; source->size = size; }
    return source;
}
static uint32_t little32(const uint8_t *p)
{ return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24; }
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *source, int free_source)
{
    Mix_Chunk *chunk = NULL;
    if (source && source->size >= 44 && !memcmp(source->data, "RIFF", 4)
        && !memcmp(source->data+36, "data", 4)) {
        unsigned bytes = little32(source->data+40);
        if (bytes && !(bytes & 1) && bytes <= (unsigned)source->size-44) {
            chunk = calloc(1, sizeof(*chunk));
            if (chunk) {
                chunk->samples = malloc(bytes);
                if (!chunk->samples) { free(chunk); chunk = NULL; }
                else { memcpy(chunk->samples, source->data+44, bytes); chunk->count = bytes/2; }
            }
        }
    }
    if (free_source) free(source);
    return chunk;
}
void Mix_FreeChunk(Mix_Chunk *chunk)
{
    if (!chunk) return;
    for (int c = 0; c < MIX_CHANNELS; ++c)
        if (voices[c].chunk == chunk) Mix_HaltChannel(c);
    free(chunk->samples); free(chunk);
}
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
    if (channel == -1) channel = Mix_GroupAvailable(1);
    if (channel < 0 || channel >= MIX_CHANNELS || !chunk || !chunk->count || loops) return -1;
    voices[channel].chunk = chunk; voices[channel].position = 0; voices[channel].age = ++serial;
    return channel;
}
int Mix_HaltChannel(int channel)
{
    for (int c = 0; c < MIX_CHANNELS; ++c) if (channel == -1 || channel == c) {
        int active = voices[c].chunk != NULL;
        voices[c].chunk = NULL;
        if (active && finished_callback) finished_callback(c);
    }
    return 0;
}
const char *Mix_GetError(void) { return "RAD-Wolf audio format or allocation error"; }
