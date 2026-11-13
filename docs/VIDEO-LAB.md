# Offline video comparisons

Build tools/video_preview.c with a native C compiler and libm:

```sh
mkdir -p build/video-lab
cc -std=gnu99 -O2 -fno-strict-aliasing tools/video_preview.c -lm -o build/video-lab/video_preview
python3 tools/video_lab.py --renderer build/video-lab/video_preview --output build/comparison frame.rwf
```

Open build/comparison/index.html directly. It embeds its preview images and
provides A/B selection, a wipe slider, original RGB, seven variants, and local
PRG/Koala downloads. PRGs contain a small 6502 display loop and exact bitmap,
screen, color RAM and background data. They run in VICE; they do not run the
ARM game or emulate RAD DMA. Pure PNGs use the Pepto palette without CRT.

Solid-surface color selection now also compares normalized chroma for
clearly colored inputs, rather than allowing RGB brightness distance to
turn a dark colored ceiling gray. The real level-9 RGB 64/0/64 ceiling maps
to C64 purple (4). Exact and near-neutral grays retain their gray mapping.
The preview recomputes surface colors from the retained original RGB, so old
RWF captures can be reused without capturing the scene again.

The host executable includes the firmware's actual video packer. Only its
NEON pixel arithmetic has a scalar conditional branch. The production build
keeps NEON and its existing settings. Host/ARM bit identity is not yet
established. Three repeats of a static frame settle the converter's adaptive
state; actual gameplay history, temporal artifacts and timing are not modeled.

Firmware now defaults to the selected 0/20/350 preview settings. See
[VIDEO-CONFIG.md](VIDEO-CONFIG.md) for changing them through VIDEO.CFG.

P on the C64 requests a pre-conversion capture on the next presentation in
the capture-capable firmware. RWF1LE files retain original 24-bit RGB and the
explicit high-byte surface tags. They are 16 header bytes plus 320x200 little
endian uint32 pixels. The header is magic RWF1LE plus two NULs, uint16 width,
uint16 height, uint32 byte count. New CAP0000.RWF files are written under
SD:RADWOLF without overwriting existing captures. One capture can briefly
stall rendering; SD capture needs hardware verification. Partial files fail
the preview reader's size/header validation. No automatic interval writes run.

For local examples, tools/capture_host_frames.sh accepts a prepared pinned
Wolf source, a private WL6 folder and a fresh output folder. It builds the
standard upstream C host in a temporary directory with dummy audio/video,
copies private data locally, and captures levels 0 and 9 after palette fade-in.
The standard host sound code is not linked into RAD firmware. All hook edits
are confined to its temporary tree; original upstream and game data remain
unchanged. Dependencies include SDL2/SDL2_mixer development libraries.

Raw frames, decoded images and generated C64 programs contain game artwork.
Keep these local under ignored build paths. They are not added to source
publication or existing release archives. The maintainer's four explicitly
supplied README images remain a separate documented exception.
