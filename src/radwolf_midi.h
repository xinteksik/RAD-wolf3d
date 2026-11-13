/* GPL-3.0-or-later. Optional MIDI/SoundFont replacement for WL6 music. */
#ifndef RADWOLF_MIDI_H
#define RADWOLF_MIDI_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RADWOLF_MIDI_SONGS 27u
/* Index follows Wolf3D's musicnames enum; resume preserves in-memory playback. */
int radwolf_midi_start(unsigned song, int resume);
void radwolf_midi_stop(void);
int radwolf_midi_active(void);
/* Width 0 preserves the MIDI/SF2 panorama; use it for a mono output. */
void radwolf_midi_configure_stereo(unsigned width);
void radwolf_midi_mix(int16_t *stereo, unsigned frames);
#ifdef __cplusplus
}
#endif
#endif
