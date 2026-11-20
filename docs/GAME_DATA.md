# Game data and file fingerprints

This repository includes no Wolfenstein game data. Obtain your own licensed
copy, for example [Wolfenstein 3D on GOG](https://www.gog.com/en/game/wolfenstein_3d).
The maintainer identifies the tested local files as coming from a purchased
GOG copy. These are measured local fingerprints, not a certification of every
installer or update supplied by GOG.

New local builds contain separate Wolfenstein 3D v1.4 engine variants for
full GT/ID/Activision WL6 (`GOODTIMES`, `CARMACIZED`) and shareware WL1
(`UPLOAD`, `CARMACIZED`). Startup checks all eight files in `RADWOLF/` and
selects a compatible complete set; WL6 has priority if both are present.
VGAHEAD is 450 bytes/150 offsets for this WL6 edition and 471 bytes/157 offsets
for shareware. Sound header and dictionary sizes are checked too; full content
checks remain in the normal engine loaders. Spear, older shareware versions,
other full editions and mods are not supported by this detector.

Both variants passed actual-engine host tests through menu, keyboard input,
E1M1 loading and rendering. The maintainer also reports working gameplay and
MIDI on hardware with WL1 and then WL6; see STATUS.md for the tested kernel
and limits of this confirmation. Earlier release ZIPs retain the previous kernel.

For shareware, use the same eight runtime filenames below with `.WL1`
extensions. Its separately measured reference fingerprints are in
`game-data-shareware-hashes.json`; no game data is supplied. To verify them:

```sh
python3 tools/verify_game_data.py /path/to/WL1 --edition WL1
```

Configuration and saves retain the selected extension (`CONFIG.WL1` and
`savegam*.wl1`, or their WL6 equivalents). The engines keep edition-specific
graphics, menus and gameplay state separate. MIDI uses the shared song names;
prepare the 11 WL1 or 27 WL6 tracks using the updated converter.

Copy only the following eight runtime files to `RADWOLF/`. Uppercase names
are shown; the verifier also accepts lowercase. Do not publish the files.
CONFIG.WL6 and savegam*.wl6 are user state and deliberately not hashed here.

| File | Bytes | MD5 |
|---|---:|---|
| AUDIOHED.WL6 | 1156 | `a41af25a2f193e7d4afbcc4301b3d1ce` |
| AUDIOT.WL6 | 320209 | `2385b488b18f8721633e5b2bdf054853` |
| GAMEMAPS.WL6 | 150652 | `a4e73706e100dc0cadfb02d23de46481` |
| MAPHEAD.WL6 | 402 | `b8d2a78bc7c50da7ec9ab1d94f7975e1` |
| VGADICT.WL6 | 1024 | `dec8939cff5a4ec27ae7b43e8f52ec28` |
| VGAGRAPH.WL6 | 275774 | `8b40b5b785f898e229bf1c2f2e3ee003` |
| VGAHEAD.WL6 | 450 | `8e75e3ffb842ed3d08abe6ffea97b231` |
| VSWAP.WL6 | 1544376 | `b8ff4997461bafa5ef2a94c11f9de001` |

MD5 is provided for legacy data identification, not security. SHA-256 values
are in [game-data-hashes.json](game-data-hashes.json). Check your installation:

```sh
python3 tools/verify_game_data.py /path/to/WL6
```

A mismatch can mean another valid edition, an update or altered data. Do not
rename an incompatible edition into WL6 or bypass chunk checks blindly.
Hashes are neither a purchase check nor a redistribution license.

GOG sale snapshot: the official page checked on **2026-11-12** advertised
70% off, displaying 4.99 -> 1.49, with an offer end of 2026-11-16 09:59 EET.
Currency, region, availability and prices can change; use the live store
page rather than treating this note as a current offer.
