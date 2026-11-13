# Optional MIDI + SoundFont music

All 27 songs in the tested full Carmackized WL6 edition can use General MIDI
conversions from wolfmidi. TinySoundFont renders instruments at 44100 Hz in
floating point. A 95-tap Blackman FIR low-pass filter (9500 Hz cutoff) precedes
2:1 conversion to the existing 22050 Hz SID stream. This supersedes the earlier
menu-only experiment, which synthesized directly at 22050 Hz.

## Automatic selection

- Without `RADWOLF/soundfont.sf2`, all music uses the original OPL playback.
- With a valid `RADWOLF/soundfont.sf2`, songs use `RADWOLF/MIDI/<NAME>.MID`.
  The prepared private SD package supplies all 27 files, including WONDERIN
  for the menu, level music, title, victory and end-of-level music.
- A missing or invalid MIDI file falls back to OPL for that individual song.
  An invalid SoundFont selects OPL for all songs.

Restart after adding, replacing or renaming files; assets are cached for that
boot. Renaming the font to `soundfont.off` disables MIDI after reboot without
switching the kernel. The original Music/AdLib on/off control still applies.
No MIDI bank or font is distributed with the public source. The private test
package uses the existing local RAD-Doom font. General MIDI instrument/drum
coverage is expected; arbitrary specialty banks can omit instruments.

External SF2 examples are available in
[Miserlou / Rich Jones's RJModules soundfonts folder](https://github.com/Miserlou/RJModules/tree/master/soundfonts).
The repository's [LICENSE.txt](https://github.com/Miserlou/RJModules/blob/master/LICENSE.txt)
contains the MIT license, copyright Rich Jones 2017. This is an external
reference; RAD-Wolf does not bundle those banks or apply its own GPL to them.
Retain the source's applicable notices when copying or redistributing assets.
Check that a chosen bank supplies the General MIDI programs and percussion
needed by the soundtrack; individual-instrument banks are also useful for
experiments but need not cover the full music. Copy a chosen SF2 as
`RADWOLF/soundfont.sf2` and reboot to select it.

The renderer handles notes, programs, controllers, channel-9 drums and pitch
bend. TinySoundFont lacks reverb/chorus sends and some GS extensions, so its
sound is not identical to a Roland Sound Canvas. The gain remains -9 dB and
polyphony is limited to 64 sample voices per song. SoundFont samples are shared;
small per-song synth/filter states are cached as songs are first encountered.

Menu and gameplay have independent cached positions. Returning to gameplay
through SD_ContinueMusic resumes its synth, envelope and FIR state without
rendering forward from the beginning. Starting a new song normally restarts
it. Cached positions do not survive reboot. Song loops use the MIDI's final
parsed event, including a delayed EndOfTrack; the filter resets at the loop.
The FIR adds about 1.07 ms of delay; the existing output ring adds about 100 ms.

AdLib effects, digitized effects and their panning retain existing playback.
The SID rate, lookup tables, DMA timing and PAL settings are unchanged.
Increasing the synthesis rate does not guarantee elimination of audible noise:
physical SID output and realtime performance still require listening tests.

## Stereo placement

MIDI/SF2 stereo remains separate through synthesis, FIR filtering and both SID
outputs: SID1 is left and SID2 is right. With `STEREO=ON` and successful SID2
initialization, `MIDI_STEREO_WIDTH` controls automatic channel placement from
0 to 100 (default 70). The converted OPL songs have no authored MIDI pan, so
their melodic channels are gently alternated across the stereo field. GM bass
programs, tuba and percussion channel 9 stay centered at the channel level;
the SoundFont's own sample/region panorama is retained.

Explicit MIDI pan controllers CC10/42 always take precedence, including an
explicit center. Controller reset restores automatic placement. Width 0
restores the previous MIDI/SF2 panning with no added channel spread. Automatic
placement is disabled for a mono SID output. `STEREO_WIDTH` still controls
digitized gameplay effects independently. No reverb, phase widening, gain or
sample-rate changes accompany this placement feature.

For example, add `MIDI_STEREO_WIDTH=70` to `RADWOLF/AUDIO.CFG`; the same value
is the default if the key is omitted. Restart after changing configuration.
When rolling back to firmware predating this option, remove the optional key
because the older strict parser treats unknown keys as an invalid config.

Direct-to-menu startup now skips the unused title-song start. The previous
brief NAZI_NOR followed by WONDERIN came from the original title/menu sequence
while the title screens were skipped. Normal non-fast title playback remains.
During fast startup, a visible "Loading menu..." panel precedes music/font and
menu-sound preparation. It replaces the long black wait before the menu;
large fonts can still take time to load. Boot tracing distinguishes the visible
loading panel from the completed menu. Dual-SoundFont playback is not implemented.

## Generate all songs

Windows test-release users can run `RADWOLF/Prepare-MIDI.bat` directly from the
card inserted into their PC; see [WINDOWS-MIDI.md](WINDOWS-MIDI.md). The manual
converter build below is for development and is not needed by the prepared kit.

The firmware currently plays prepared MIDI files; it does not convert WL6 to
MIDI at startup. Conversion is a one-time PC preparation step below. Replacing
the SoundFont does not require regenerating the MIDI. A missing MIDI track
falls back to OPL rather than triggering automatic conversion. Integrating
wolfmidi directly into the GPLv3 firmware is not the current design: wolfmidi
declares GPL-2.0-only, and remains an independent offline tool. Automatic
first-run conversion/cache would need a compatible converter or licensing
resolution before becoming a supported firmware feature.

Clone wolfmidi at the revision recorded in NOTICE.md. From that checkout:

```sh
g++ -O2 -include climits src/dro2midi.cpp src/midiio.cpp -o dro2midi
python3 /path/to/RAD-wolf3d/tools/convert_all_midi.py /path/to/WL6 ./dro2midi /path/to/private-assets
```

The extra standard header fixes an undeclared INT_MAX with modern GCC, without
changing upstream source. Keep its `inst.txt`, `drum.txt`, and `patch.txt` in
the working directory. The tool validates all 27 chunk names before conversion
and uses wolfmidi's original per-song loop correction options. Output MIDI
goes into `MIDI/`; extracted IMF goes into `WLF/`, and `music-manifest.json`
records conversion options and hashes. Copy only `MIDI/` to `RADWOLF/MIDI/`.
The older `convert_menu_midi.py` remains usable for one-song extraction.

Generated music, SoundFonts and audio previews retain their owners' rights.
Keep them outside the public tree or inside ignored `build/` directories.
The converter remains a separate GPL-2.0-only tool, not firmware code.

## Validation

`make test test-opl test-audio-startup` covers the existing audio paths,
extraction, OPL fallback, song selection and continuations. Portable FIR tests
check DC gain, stereo isolation, callback independence and at least 60 dB
rejection at 12/18 kHz. Regenerate coefficients with
`python3 tools/generate_downsample_filter.py`.

To test actual private assets, compile from the source root:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -DRADWOLF_MIDI_DIRECTORY='""' -Isrc tests/test_midi.c src/radwolf_midi.c src/radwolf_downsample.c -lm -o build/test_midi
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -DRADWOLF_MIDI_DIRECTORY='""' -Isrc tests/test_all_midi.c src/radwolf_midi.c src/radwolf_downsample.c -lm -o build/test_all_midi
```

Run both binaries from a directory holding `soundfont.sf2` and `MIDI/`.
`test_midi` checks four-minute playback, bounds, restart, callback invariance
and sample-exact resume after another song. Its optional filename writes the
first 30 seconds as signed 16-bit little-endian stereo raw audio.
`test_all_midi` renders every full song, checks exact loop repetition and
rejects clipped output with the supplied font. A different bank can have
different peaks. Run `test_midi --fallback` in empty, invalid-font, missing-font
and missing-track directories to verify graceful OPL fallback.

The upstream startup integration binary accepts `--midi` when assets are
available at its host path `SD:RADWOLF/`. Keep the previous kernel and back up
the card before deployment. Host results cannot establish audible SID quality,
firmware boot success or gameplay frame rate.
