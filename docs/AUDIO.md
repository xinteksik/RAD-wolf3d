# SID stereo audio

Hardware-confirmed real-chip configuration: stereo ON, SID1 `$D400`/8580,
SID2 `$D500`/6581, width 100. That development snapshot is recorded in
STATUS.md; compiled snapshots are not part of this source folder. For this optional
mixed-SID stereo preset, keep the types explicit:
the captured boot detection reported 6581 for the primary while the confirmed
configuration requests 8580. This confirmation does not cover UltiSID audio.

SID1 supplies mono, or the left output in stereo. SID2 supplies the right
output in stereo. Configure each chip's actual address and type.
Connect the two physical audio outputs to the corresponding
left and right inputs of your audio equipment.

Copy `config/AUDIO.CFG.example` to `RADWOLF/AUDIO.CFG` on the card:

```ini
STEREO=OFF
SID1_ADDRESS=$D400
SID1_TYPE=AUTO
SID2_ADDRESS=$D500
SID2_TYPE=AUTO
STEREO_WIDTH=100
```

This is the default: mono through SID1 with automatic detection. SID2 is not
initialized or used for sample output in mono mode. For the tested 8580/6581
stereo setup, use `config/AUDIO.STEREO.REAL-SID.CFG.example` instead and match
its addresses/types to your actual hardware.

Reboot after changing the file. `STEREO=OFF`, a missing file, or invalid
configuration selects mono on SID1 (default `$D400`, AUTO). Stereo is enabled only if the
configured SID initializes successfully. Check `RADWOLF/WOLFBOOT.LOG` for
`stereo: SID2 enabled` or a fallback reason.
The log records requested and selected types, addresses and stereo width.
Selected type 4 means VOLUME and 8 means RAD's detected primary SIDKick DAC.

`SID1_ADDRESS` and `SID2_ADDRESS` are hexadecimal, 32-byte-aligned base addresses.
Supported ranges are `$D400`–`$D7E0` and `$DE00`–`$DFE0`; specify actual mappings.
The parser rejects identical stereo addresses, VIC/CIA ranges, unknown options and
invalid values. It does not scan the address space. Use confirmed hardware
mappings: SID2 must accept its configured bank without accepting SID1's bank.
The player writes SID2 first and SID1 last, so SID1 may also accept the
secondary-bank write without losing its final left-channel sample. This
ordering applies to both the timer and blitter playback paths.

Both type settings accept AUTO, 6581, 8580, VOLUME or ULTISID.
`AUTO` repeats RAD's oscillator test three times at the configured
address. Consistent 6581/8580 responses select the matching Mahoney table.
This is a type test, not proof of a separately decoded second device. If it
fails for SID2, playback stays mono. A relocated SID1 that fails initialization
retains the original primary output and disables stereo. SID1 AUTO at `$D400`
preserves RAD's original detection, including SIDKick DAC support.
An explicit `6581` or `8580` skips that test.
At `$D400`, an explicit model matching RAD's boot detection also preserves
the boot-established register/envelope state, just like AUTO. The log's
`boot=` field records that original type for comparison. The mono hardware
test preset is `config/AUDIO.MONO8580.CFG.example`.
`VOLUME` uses RAD's four-bit volume playback setup for compatible replacements;
it does not enable SIDKick's direct eight-bit DAC mode on the second SID.
Each SID has its own conversion table, allowing different first/second types.

For Ultimate64 UltiSID, `config/AUDIO.ULTISID.CFG.example` sets both types to
ULTISID. This uses a separate 256-entry curve measured on Ultimate64 UltiSID
by Kelly Fox's c64cast project, with the same Mahoney register environment.
The measured effective resolution is approximately 6–7 bits, rather than
VOLUME's 4 bits. It is not intended for physical SID chips. Select only the
two emulated cores in the Ultimate mixer and use the calibration's Medium
digis level for the initial comparison. Firmware/settings differences may
still require calibration on the actual system; listening is pending.

The table and MIT license are preserved in `src/radwolf_ultisid_lut.h` and
`third_party/C64CAST-LICENSE.txt`. Source: [measurement and conditions at
revision 34d350de](https://github.com/kfox/c64cast/blob/34d350de020a3fc1ccfba61396402b12f9beb3e8/scripts/diags/mahoney_measured_tables.json).
AUTO tests oscillator behavior and cannot select this implementation-specific
curve; ULTISID must be explicit. VOLUME remains available as a four-bit
comparison mode. The OPL synthesis level and playback timing are unchanged
by selecting the new curve.

`STEREO_WIDTH=100` strengthens digitized positioning using source direction
relative to the player. A source directly to one side is muted in the other
channel, retaining the louder original distance attenuation. Values 1–99
reduce separation; 0 restores Wolf's original tables, which often give equal
gains for close sounds. Positions update as the player turns or moves.
Mono retains the original positioning/mix.

The original mono OPL2 music and AdLib effects are duplicated to the
center; this feature does not add spatialization to that music. The RAD intro
continues using original `$D400` playback before game audio configuration.

Stereo adds a second SID write to the timer and blitter sample paths. Existing
DMA timing constants are kept, and the blitter accounts for the extra write.
Hardware validation must check channel separation, sound continuity and video
stability. Host tests cannot establish electrical output or realtime timing.

Validation commands (upstream checkouts required for the latter two):

```sh
make test
make test-opl
make test-audio-startup
```

For rollback, keep `kernel_wolf_mono_verified.img`. Select that image in the
card's boot `config.txt`, or use `STEREO=OFF` with this firmware.
