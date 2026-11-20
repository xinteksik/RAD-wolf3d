#!/usr/bin/env bash
# GPL-3.0-or-later. Build Wolf directly; no Doom engine or intro media.
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
stdlib=${STDLIB_ROOT:-$repo/upstream/circle-stdlib}
case "$repo:$stdlib" in *' '*) echo 'Use a Linux build path without spaces (see docs/BUILD.md).' >&2; exit 1;; esac
test -f "$stdlib/Config.mk"
test -f "$stdlib/install/aarch64-none-circle/lib/libcirclenewlib.a"
mkdir -p "$repo/build/wolf-firmware-dual" "$repo/build/wolf-core-dual-arm64"
cd "$repo"
for edition in WL6 WL1; do
    if ! RADWOLF_EDITION=$edition bash tools/build_wolf_core.sh upstream/Wolf4SDL "$stdlib/install/aarch64-none-circle/include" "build/wolf-core-$edition" >"build/core-$edition.log" 2>&1; then
        tail -n 50 "build/core-$edition.log"; exit 1
    fi
done
combined=$(mktemp -d "$repo/build/wolf-combined.XXXXXX")
python3 tools/combine_wolf_cores.py build/wolf-core-WL6 build/wolf-core-WL1 "$combined"
for source in radwolf_game_edition radwolf_game_start; do
    aarch64-none-elf-gcc -std=c11 -O2 -ffreestanding -DRADWOLF_BAREMETAL \
        -Isrc -I"$stdlib/install/aarch64-none-circle/include" \
        -c "src/$source.c" -o "$combined/$source.o"
done
cp "$combined"/*.o build/wolf-core-dual-arm64/
cp firmware/Makefile build/wolf-firmware-dual/Makefile
make -C build/wolf-firmware-dual REPO="$repo" STDLIB_ROOT="$stdlib" -j"${JOBS:-4}"
cp build/wolf-firmware-dual/kernel8.img build/kernel_wolf.img
sha256sum build/kernel_wolf.img
