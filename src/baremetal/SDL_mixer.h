/* Local mixer boundary; no SDL runtime. */
#ifndef RADWOLF_BAREMETAL_SDL_MIXER_H
#define RADWOLF_BAREMETAL_SDL_MIXER_H
#include "SDL.h"
#define MIX_CHANNELS 8
#define AUDIO_S16 0x8010
#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE 1
typedef struct { const uint8_t *data; int size; } SDL_RWops;
typedef struct { int16_t *samples; unsigned count; } Mix_Chunk;
#define SDL_RWFromMem radwolf_audio_from_mem
#define Mix_OpenAudioDevice radwolf_audio_open
#define Mix_QuerySpec radwolf_audio_query
#define Mix_ReserveChannels radwolf_audio_reserve
#define Mix_GroupChannels radwolf_audio_group
#define Mix_GroupAvailable radwolf_audio_available
#define Mix_GroupOldest radwolf_audio_oldest
#define Mix_HookMusic radwolf_audio_music
#define Mix_ChannelFinished radwolf_audio_finished
#define Mix_SetPostMix radwolf_audio_post
#define Mix_SetPanning radwolf_audio_pan
#define Mix_LoadWAV_RW radwolf_audio_load
#define Mix_FreeChunk radwolf_audio_free
#define Mix_PlayChannel radwolf_audio_play
#define Mix_HaltChannel radwolf_audio_halt
#define Mix_GetError radwolf_audio_error
SDL_RWops *SDL_RWFromMem(void *data, int size);
int Mix_OpenAudioDevice(int rate, uint16_t format, int channels, int size,
                       const char *device, int flags);
int Mix_QuerySpec(int *rate, uint16_t *format, int *channels);
int Mix_ReserveChannels(int count);
int Mix_GroupChannels(int first, int last, int tag);
int Mix_GroupAvailable(int tag);
int Mix_GroupOldest(int tag);
void Mix_HookMusic(void (*callback)(void *, uint8_t *, int), void *user);
void Mix_SetPostMix(void (*callback)(void *, uint8_t *, int), void *user);
void Mix_ChannelFinished(void (*callback)(int));
int Mix_SetPanning(int channel, uint8_t left, uint8_t right);
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *source, int free_source);
void Mix_FreeChunk(Mix_Chunk *chunk);
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
int Mix_HaltChannel(int channel);
const char *Mix_GetError(void);
void radwolf_audio_pump(void);
#endif
