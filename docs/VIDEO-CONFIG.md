# Video settings on the SD card

The default is the selected clean preview: dither 0%, brightness 20/16,
exposure 350/256. Missing or invalid configuration falls back to these values.
Pickup/damage feedback defaults to a localized frame flash.

Edit RADWOLF/VIDEO.CFG on the card, then reboot:

```ini
DITHER=0
BRIGHTNESS=20
EXPOSURE=350
FLASH=FRAME
```

| Setting | Range | Meaning |
|---|---|---|
| DITHER | 0–100 | Strength in percent. 0 adds no ordered dither; 100 restores the earlier full strength. |
| BRIGHTNESS | 1–32 | Integer numerator over 16; 20 means 1.25. |
| EXPOSURE | 1–512 | Integer numerator over 256; 350 means approximately 1.367. |
| FLASH | OFF, FRAME, STATUS, FULL | Pickup/damage feedback; default FRAME. |

FRAME draws a two-pixel yellow frame on pickup or a red frame on damage.
It uses the existing viewport margin where available; at maximum view sizes
it falls back to an inset edge. The scene palette is unchanged. STATUS tints
only the status bar; with no bar it falls back to FRAME. OFF disables this
feedback. FULL restores the original whole-palette flashes. Damage takes
priority over pickup. Original event durations/counters are retained.

Feedback is attached only to a presented 3D gameplay frame, so menus and maps
do not inherit it. HUD text remains unchanged in FRAME. The final VIC-II
per-block color limit can still affect pixels adjoining a colored frame.
Separate death/fizzle transitions retain their original behavior.

Keep uppercase keys and FLASH values, whole decimal numbers for numeric
settings, and one occurrence of each key.
Blank lines, hash comments, CRLF, LF and a UTF-8 BOM are supported. Omitted
keys keep defaults. A malformed/unknown/duplicate key, out-of-range value,
embedded NUL or oversized file rejects the entire configuration. At most
1023 bytes are read as configuration. No file is written by the loader.

These are the same numerical controls as the preview. The default native
host conversion matches the chosen no-dither preview exactly for the local
level-9 reference. The ARM branch applies the same percentage to its dither
offset before quantization; its hardware appearance still needs checking.

The explicit solid floor/ceiling colors and map colors keep their separate
color selection. Brightness/exposure control the texture conversion, as in
the gallery. Colored ceilings retain hue; P capture and the internal white
direction indicator remain available. No DMA/PAL or audio timing changes.

Use the provided sd/RADWOLF/VIDEO.CFG as a template. Preserve a customized
VIDEO.CFG when updating a card. This is a development change after v0.3.2-rc1;
the existing release archives have not been replaced.
