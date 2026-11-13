/* GPL-3.0-or-later. TinySoundFont/TinyMidiLoader retain their own licenses. */
#include "radwolf_midi.h"
#include "radwolf_downsample.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#include "../third_party/tsf.h"
#define TML_IMPLEMENTATION
#define TML_NO_STDIO
#include "../third_party/tml.h"
#ifndef RADWOLF_MIDI_DIRECTORY
#define RADWOLF_MIDI_DIRECTORY "SD:RADWOLF/"
#endif
#define MIDI_RATE 44100u
extern void radwolf_boot_trace(const char *stage);
static const char *const song_names[RADWOLF_MIDI_SONGS] = {
    "CORNER", "DUNGEON", "WARMARCH", "GETTHEM", "HEADACHE", "HITLWLTZ",
    "INTROCW3", "NAZI_NOR", "NAZI_OMI", "POW", "SALUTE", "SEARCHN",
    "SUSPENSE", "VICTORS", "WONDERIN", "FUNKYOU", "ENDLEVEL", "GOINGAFT",
    "PREGNANT", "ULTIMATE", "NAZI_RAP", "ZEROHOUR", "TWELFTH", "ROSTER",
    "URAHERO", "VICMARCH", "PACMAN"
};
typedef struct {
    tsf *synth;
    tml_message *song, *next_message;
    uint64_t frame, loop_frames;
    radwolf_downsampler filter;
    float pending[256];
    unsigned pending_frames, pending_position;
    int attempted, initialized;
    unsigned applied_width, explicit_pan;
    uint8_t programs[16];
} midi_track;
static tsf *font;
static midi_track tracks[RADWOLF_MIDI_SONGS], *active;
static int font_attempted;
static unsigned stereo_width;

void radwolf_midi_configure_stereo(unsigned width)
{
    stereo_width = width > 100 ? 100 : width;
}

static void apply_pan(midi_track *track, unsigned channel)
{
    /* MIDI CC10/42 takes priority over automatic instrument placement. */
    if (track->explicit_pan & (1u << channel)) return;
    static const float positions[8] = {-0.35f, 0.35f, -0.20f, 0.20f,
                                       -0.45f, 0.45f, -0.30f, 0.30f};
    unsigned program = track->programs[channel];
    float offset = (channel == 9 || (program >= 32 && program <= 39) || program == 58)
        ? 0 : positions[channel & 7] * stereo_width / 100.0f;
    tsf_channel_set_pan(track->synth, (int)channel, 0.5f + offset);
}

static void apply_all_pans(midi_track *track)
{
    for (unsigned channel = 0; channel < 16; ++channel) apply_pan(track, channel);
    track->applied_width = stereo_width;
}

static void *read_asset(const char *path, long limit, int *size)
{
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    long length = -1;
    if (!fseek(file, 0, SEEK_END)) length = ftell(file);
    if (length <= 0 || length > limit || fseek(file, 0, SEEK_SET)) {
        fclose(file);
        return NULL;
    }
    void *data = malloc((size_t)length);
    int valid = data && fread(data, 1, (size_t)length, file) == (size_t)length;
    fclose(file);
    if (!valid) { free(data); return NULL; }
    *size = (int)length;
    return data;
}

static int load_font(void)
{
    if (font_attempted) return font != NULL;
    font_attempted = 1;
    int size;
    void *data = read_asset(RADWOLF_MIDI_DIRECTORY "soundfont.sf2", 64 * 1024 * 1024, &size);
    if (data) {
        font = tsf_load_memory(data, size);
        free(data);
    }
    if (!font) {
        radwolf_boot_trace("music: no usable soundfont.sf2, original OPL playback");
        return 0;
    }
    tsf_set_output(font, TSF_STEREO_INTERLEAVED, MIDI_RATE, -9.0f);
    return 1;
}

static int load_track(unsigned index)
{
    midi_track *track = &tracks[index];
    if (track->attempted) return track->synth != NULL;
    track->attempted = 1;
    char path[128], report[128];
    snprintf(path, sizeof(path), RADWOLF_MIDI_DIRECTORY "MIDI/%s.MID", song_names[index]);
    int size;
    void *data = read_asset(path, 1024 * 1024, &size);
    if (data) {
        track->song = tml_load_memory(data, size);
        free(data);
    }
    unsigned length = 0;
    if (!track->song || !tml_get_info(track->song, NULL, NULL, NULL, NULL, &length) || !length)
        goto fail;
    track->loop_frames = (((uint64_t)length * MIDI_RATE / 1000u) + 2u) & ~(uint64_t)1;
    track->synth = tsf_copy(font);
    if (!track->synth || !tsf_set_max_voices(track->synth, 64)) goto fail;
    for (int channel = 0; channel < 16; ++channel)
        if (!tsf_channel_set_presetnumber(track->synth, channel, 0, channel == 9)) goto fail;
    return 1;
fail:
    if (track->synth) tsf_close(track->synth);
    if (track->song) tml_free(track->song);
    track->synth = NULL;
    track->song = NULL;
    snprintf(report, sizeof(report), "music: %s MIDI unavailable, OPL fallback", song_names[index]);
    radwolf_boot_trace(report);
    return 0;
}

static void restart_track(midi_track *track)
{
    tsf_reset(track->synth);
    /* Finish the 10 ms quick release before starting a new instance. */
    float discarded[512 * 2];
    tsf_render_float(track->synth, discarded, 512, 0);
    for (int channel = 0; channel < 16; ++channel)
        tsf_channel_set_presetnumber(track->synth, channel, 0, channel == 9);
    track->next_message = track->song;
    track->frame = 0;
    track->pending_frames = track->pending_position = 0;
    track->initialized = 1;
    track->explicit_pan = 0;
    memset(track->programs, 0, sizeof(track->programs));
    apply_all_pans(track);
    radwolf_downsample_reset(&track->filter);
}

int radwolf_midi_start(unsigned index, int resume)
{
    active = NULL;
    if (index >= RADWOLF_MIDI_SONGS || !load_font() || !load_track(index)) return 0;
    midi_track *track = &tracks[index];
    if (!resume || !track->initialized) restart_track(track);
    active = track;
    char report[128];
    snprintf(report, sizeof(report), "music: %s MIDI + SF2, 44100 -> FIR -> 22050 Hz, stereo-width=%u%s",
             song_names[index], stereo_width, resume ? " (resume)" : "");
    radwolf_boot_trace(report);
    return 1;
}

/* Freeze synth/filter state so gameplay can resume after its menu. */
void radwolf_midi_stop(void) { active = NULL; }
int radwolf_midi_active(void) { return active != NULL; }

static void dispatch(midi_track *track, const tml_message *message)
{
    tsf *synth = track->synth;
    int channel = message->channel;
    switch (message->type) {
    case TML_PROGRAM_CHANGE:
        track->programs[channel] = (uint8_t)message->program;
        tsf_channel_set_presetnumber(synth, channel, message->program, channel == 9);
        apply_pan(track, (unsigned)channel);
        break;
    case TML_NOTE_ON:
        tsf_channel_note_on(synth, channel, message->key, message->velocity / 127.0f);
        break;
    case TML_NOTE_OFF:
        tsf_channel_note_off(synth, channel, message->key);
        break;
    case TML_PITCH_BEND:
        tsf_channel_set_pitchwheel(synth, channel, message->pitch_bend);
        break;
    case TML_CONTROL_CHANGE:
        tsf_channel_midi_control(synth, channel, message->control, message->control_value);
        if (message->control == TML_PAN_MSB || message->control == TML_PAN_LSB)
            track->explicit_pan |= 1u << channel;
        else if (message->control == TML_ALL_CTRL_OFF) {
            track->explicit_pan &= ~(1u << channel);
            apply_pan(track, (unsigned)channel);
        }
        break;
    default: break;
    }
}

void radwolf_midi_mix(int16_t *stereo, unsigned frames)
{
    midi_track *track = active;
    if (!track) return;
    if (track->applied_width != stereo_width) apply_all_pans(track);
    while (frames) {
        while (!track->pending_frames) {
            if (track->frame >= track->loop_frames) restart_track(track);
            while (track->next_message &&
                   (uint64_t)track->next_message->time * MIDI_RATE / 1000u <= track->frame) {
                dispatch(track, track->next_message);
                track->next_message = track->next_message->next;
            }
            uint64_t boundary = track->next_message ?
                (uint64_t)track->next_message->time * MIDI_RATE / 1000u : track->loop_frames;
            unsigned count = boundary - track->frame > 256 ? 256 : (unsigned)(boundary - track->frame);
            float high_rate[256 * 2];
            tsf_render_float(track->synth, high_rate, (int)count, 0);
            track->pending_frames = radwolf_downsample(&track->filter, high_rate, count, track->pending);
            track->pending_position = 0;
            track->frame += count;
        }
        unsigned count = frames < track->pending_frames ? frames : track->pending_frames;
        for (unsigned i = 0; i < count * 2; ++i) {
            float sample = stereo[i] + track->pending[track->pending_position * 2 + i] * 32767.0f;
            stereo[i] = (int16_t)(sample > 32767 ? 32767 : sample < -32768 ? -32768 : sample);
        }
        stereo += count * 2;
        frames -= count;
        track->pending_frames -= count;
        track->pending_position += count;
    }
}
