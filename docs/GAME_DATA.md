# Game data and file fingerprints

This repository includes no Wolfenstein game data. Obtain your own licensed
copy, for example [Wolfenstein 3D on GOG](https://www.gog.com/en/game/wolfenstein_3d).
The maintainer identifies the tested local files as coming from a purchased
GOG copy. These are measured local fingerprints, not a certification of every
installer or update supplied by GOG.

The engine is configured for Wolfenstein 3D full v1.4 GT/ID/Activision data
(`GOODTIMES` and `CARMACIZED`). The tested VGAHEAD has 450 bytes/150 offsets.
Spear of Destiny, WL1 shareware and other editions need different engine
settings and are not validated by this package.

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
