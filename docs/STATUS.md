# Validation status

## Hardware baseline

The maintainer reported a working menu, game launch and two played levels
with sound. IJKL input release and port-1 interference fixes were confirmed.
Subsequent real-SID stereo was reported working with:

```ini
STEREO=ON
SID1_ADDRESS=$D400
SID1_TYPE=8580
SID2_ADDRESS=$D500
SID2_TYPE=6581
STEREO_WIDTH=100
```

Baseline kernel: 782,632 bytes, SHA-256
`4104fa8d9d6d0d58c40a37ef3700a9265efd7d1b1e063efa113f83949eb8db02`.
It is not distributed in this source folder. The reported audio hardware is
an Ultimate64 Elite2 with physical SID chips; emulated UltiSID trials were
separate and produced quality problems. The boot detector sometimes reported
6581 for the primary while the working configuration explicitly selected 8580.

## Clean publication build

This source export removes the RAD intro and all external Doom demo/media
dependencies. Kernel and video code are separated from Doom gameplay, with
the inherited bus transfer and default ordered color conversion retained.
Source installation and portable host suites are checked in a fresh isolated
application checkout. Firmware is cross-compiled/linked against the established
Circle44.3/Newlib libraries. A full fresh rebuild of those libraries is not
claimed as part of this packaging validation.

The no-intro kernel is a new hardware candidate. A successful compile or a
working earlier intro build is not proof of its boot behavior. After deployment,
confirm direct Wolf startup, game entry, controls, music/effects and stereo.
No NTSC, C128 matrix, full-game completion or universal SID quality claim is made.

Cleaned no-intro candidate: 762,696 bytes, SHA-256
`becc8e1eef1c39fd662f51b49efb3254e18bf2bd83abbe80463b91fbd9b73d7a`.
Its link exports Wolf's entry and sound functions, with no Doom main or intro;
the image contains no old RADDOOM media-path strings. An earlier cleaned
candidate was deployed after an independently hash-verified backup, preserving
the existing real mixed-SID config, all eight game data files and save state,
and removing obsolete demo assets. The final local interface-only revision
avoids the MAME header as well; its SD update needs readback confirmation.
First hardware boot of this final no-intro candidate is still pending.

## Audio scope

Real SID audio is required for supported operation. 6581/8580 configurations
must match the actual address/type mapping. Ultimate/UltiSID remains experimental:
VOLUME improved effects but has only four-bit resolution; the separate measured
ULTISID curve has not yet received a successful hardware sound-quality report.
Correct stereo routing does not establish correct timbre or waveform conversion.
