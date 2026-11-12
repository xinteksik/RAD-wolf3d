# Credits and retained notices

RAD-Wolf is a derivative community port. Original author notices remain in
the source; this list does not replace them.

- **Carsten Dachsbacher / frntc** — RAD hardware/firmware, RAD-Doom DMA,
  VIC-II conversion, SID streaming and Mahoney playback setup.
  [RAD](https://github.com/frntc/RAD), [RAD-Doom](https://github.com/frntc/RAD-Doom).
- **id Software** — original Wolfenstein 3D engine and game. Commercial game
  data is separate and is not included.
- **Moritz “Ripper” Kroll, KS-Presto and Wolf4SDL contributors** — Wolf4SDL,
  including inherited engine fixes and audio sequencing.
  [Pinned upstream project](https://github.com/KS-Presto/Wolf4SDL).
- **René Stange and Circle contributors** — bare-metal Raspberry Pi runtime.
  [Circle](https://github.com/rsta2/circle).
- **circle-stdlib/circle-newlib contributors and Newlib authors** — C/C++ runtime
  integration. [circle-stdlib](https://github.com/smuehlst/circle-stdlib),
  [circle-newlib](https://github.com/smuehlst/circle-newlib).
- **DOSBox Team** — GPL DBOPL OPL synthesis, via Wolf4SDL's `dosbox/` sources.
- **Pex “Mahoney” Tufvesson** — sample playback technique and real-chip lookup
  tables as attributed by RAD-Doom's `mahoney_lut.h`.
- **Kelly Fox / c64cast** — measured UltiSID conversion data, MIT licensed;
  see `third_party/C64CAST-LICENSE.txt` and `src/radwolf_ultisid_lut.h`.
- **RAD-Wolf contributors** — frame/file/input adapters, Wolf integration,
  per-SID configuration, stereo path, source packaging and documentation.

`overrides/manifest.json` records original and modified file hashes.
`src/radwolf_kernel.cpp` derives from RAD-Doom's `Source/rad_doom.cpp`;
`src/radwolf_rad_video.c` extracts its reusable RGB/VIC-II converter from
`Source/Doom/doomgeneric_rad.c`. Both retain Carsten Dachsbacher's notices.
`overrides/Circle/.../sysconfig.h` comes from RAD's required Circle settings
at RAD revision `bfa6d489382a33d4ff57edd4b5e690879456de87`.

No endorsement or affiliation by id Software, Bethesda, GOG, frntc, Circle,
Ultimate or any other upstream author/vendor is implied.

The README's two maintainer-supplied screenshots illustrate the running port.
Underlying game artwork retains its original rights and is not included in
the source-code license grant; see `docs/images/README.md`.
