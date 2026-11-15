#!/usr/bin/env bash
# GPL-3.0-or-later. Local original-RGB examples, with no display or SD writes.
# Usage: bash capture_host_frames.sh prepared-Wolf4SDL own-WL6-folder output
set -euo pipefail
test "$#" = 3
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source=$(realpath "$1")
game=$(realpath "$2")
mkdir -p "$3"
output=$(realpath "$3")
stage=$(mktemp -d /tmp/radwolf-host-capture.XXXXXX)
git -c "safe.directory=$source" -C "$source" archive dc8b250af35fb0ace68db5eb879490b50068c20e | tar -x -C "$stage"
mkdir -p "$stage/data"
for name in AUDIOHED AUDIOT GAMEMAPS MAPHEAD VGADICT VGAGRAPH VGAHEAD VSWAP; do
    cp "$game/$name.WL6" "$stage/data/${name,,}.wl6"
done
python3 - "$repo" "$stage" <<'PY'
from pathlib import Path
import json, shutil, sys
repo, stage = map(Path, sys.argv[1:])
for item in json.loads((repo / 'overrides/manifest.json').read_text()):
    if item['repository'] == 'Wolf4SDL' and item['file'] in ('wl_draw.c', 'wl_scale.c'):
        shutil.copyfile(repo / item['override'], stage / item['file'])
for name in ('radwolf_surface.c', 'radwolf_surface.h', 'radwolf_capture.c',
             'radwolf_capture.h', 'radwolf_flat.h', 'radwolf_gray.h',
             'radwolf_flash.c', 'radwolf_flash.h', 'radwolf_video_config.h'):
    shutil.copyfile(repo / 'src' / name, stage / name)
for name in ('wl_draw.c', 'wl_scale.c'):
    path = stage / name
    path.write_text(path.read_text().replace('RADWOLF_BAREMETAL', 'RADWOLF_CAPTURE_HOST'))
path = stage / 'id_vl.c'
path.write_text(path.read_text().replace(' | SDL_WINDOW_OPENGL', '').replace(
    'SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC', 'SDL_RENDERER_SOFTWARE'))
path = stage / 'version.h'
text = path.read_text()
import re
text = re.sub(r'^\s*(?://)?#define UPLOAD\s*$', '//#define UPLOAD', text, flags=re.M)
text = re.sub(r'^\s*(?://)?#define CARMACIZED\s*$', '#define CARMACIZED', text, flags=re.M)
text = re.sub(r'^\s*(?://)?#define GOODTIMES\s*$', '#define GOODTIMES', text, flags=re.M)
path.write_text(text)
path = stage / 'Makefile'
path.write_text(path.read_text().replace('SRCS += id_ca.c',
    'SRCS += radwolf_surface.c radwolf_capture.c radwolf_flash.c\nSRCS += id_ca.c'))
path = stage / 'id_vh.c'
text = path.read_text().replace('#include "wl_def.h"',
    '#include "wl_def.h"\n#include "radwolf_capture.h"\n#include "radwolf_surface.h"\n#include "radwolf_flat.h"')
anchor = '\tSDL_BlitSurface (surface,NULL,screen,NULL);'
assert text.count(anchor) == 1
capture = r'''
    const char *path = getenv("RADWOLF_HOST_FRAME");
    if (path && radwolf_surface_length && SDL_GetTicks() >= 1500 &&
        surface->w == 320 && surface->h == 200 && surface->format->BytesPerPixel == 1 &&
        surface->format->palette->colors[15].r >= 240) {
        static uint32_t frame[320 * 200];
        for (unsigned y = 0; y < 200; ++y)
            for (unsigned x = 0; x < 320; ++x) {
                const byte *pixel = (const byte *)surface->pixels + y * surface->pitch + x;
                SDL_Color color = surface->format->palette->colors[*pixel];
                uint32_t rgb = ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | color.b;
                frame[y * 320 + x] = radwolf_surface_background(pixel) ? radwolf_flat_tag(rgb) : rgb;
            }
        FILE *file = fopen(path, "wbx");
        int status = file ? radwolf_capture_write(file, frame) : -1;
        if (file && fclose(file)) status = -1;
        exit(status ? 1 : 0);
    }
'''
path.write_text(text.replace(anchor, capture + anchor))
path = stage / 'wl_utils.c'
path.write_text(path.read_text().replace('SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR,"Wolf4SDL",string,NULL);',
    'fprintf(stderr, "Wolf4SDL: %s\\n", string);'))
PY
cd "$stage"
# Upstream's standard C host target is used only locally with dummy audio.
# Its unmodified default sound implementation is not linked into firmware.
if ! make CFLAGS="-O2 -DRADWOLF_CAPTURE_HOST -I. $(sdl2-config --cflags)" -j4 > "$output/host-capture-build.log" 2>&1; then
    tail -n 40 "$output/host-capture-build.log"; exit 1
fi
cd data
for level in 0 9; do
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_RENDER_DRIVER=software \
    RADWOLF_HOST_FRAME="$output/level-$level.rwf" \
        timeout 25 "$stage/wolf4sdl" --tedlevel "$level" --nowait --windowed --res 320 200 \
        > "$output/level-$level.log" 2>&1 || { cat "$output/level-$level.log"; exit 1; }
done
echo "Captured original host-engine frames in $output"
