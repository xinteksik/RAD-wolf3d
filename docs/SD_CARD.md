# Wolf-only SD layout

Back up your card before changing it. Preserve working Pi/RAD clock, GPIO and
bus timing settings. Do not format or eject a card through these build tools.

```text
SD root/
  bootcode.bin       # Pi boot files acquired separately
  start.elf
  fixup.dat
  rad-prefetch.bin   # RAD stub acquired separately
  config.txt
  kernel_wolf.img    # built locally
  RAD/
    rad.cfg
  RADWOLF/
    AUDIO.CFG
    AUDIOHED.WL6
    AUDIOT.WL6
    GAMEMAPS.WL6
    MAPHEAD.WL6
    VGADICT.WL6
    VGAGRAPH.WL6
    VGAHEAD.WL6
    VSWAP.WL6
    # optional existing saves/config; WOLFBOOT.LOG is generated
```

The Pi's low-memory `gpu_mem=16` mode uses the matching `start_cd.elf` and
`fixup_cd.dat` variant; retain that pair as well when using the supplied
boot configuration. Obtain the compatible boot files, prefetch stub and
RAD timing configuration from the [official RAD-Doom v01 release](https://github.com/frntc/RAD-Doom/releases/tag/v01)
or your already working card. They are not bundled or relicensed here.
The original downloaded release archive's reference SHA-256 is
`ffb04ea018cdde257ec9f195beea85142748176865ef9f25dbfa6618437d204d`.

Text templates are in `sd/`. They retain the tested settings but remove
references to Doom kernels. Kernel selection is `kernel=kernel_wolf.img`.
The supplied AUDIO.CFG defaults to mono and automatic SID1 detection. Use an
optional stereo preset only after matching its addresses and types to your hardware.

**A real SID is required for supported audio. Ultimate/UltiSID playback is
experimental and may sound poor or distorted.** Mono uses SID1 only. Stereo
uses SID1 left and SID2 right. See AUDIO.md for examples and caveats.

The cleaned application initializes the C64 and SID directly, mounts game
storage and opens Wolf's menu. No `RADDOOM/` directory is needed. Do not copy
`doom1.wad`, `soundfont.sf2`, `logo.ani`, scroller/loading raw files, intro WAV,
or the old external blue-noise file. Default ordered dithering is built in.
The public source folder also excludes SD game data and save files.

Existing saves and CONFIG.WL6 belong to the user. Keep them on their own card;
they are not part of the data compatibility hashes or a GitHub release.
