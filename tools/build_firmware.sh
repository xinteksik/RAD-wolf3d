#!/usr/bin/env bash
# GPL-3.0-or-later. Build Wolf directly; no Doom engine or intro media.
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
stdlib=${STDLIB_ROOT:-$repo/upstream/circle-stdlib}
case "$repo:$stdlib" in *' '*) echo 'Use a Linux build path without spaces (see docs/BUILD.md).' >&2; exit 1;; esac
test -f "$stdlib/Config.mk"
test -f "$stdlib/install/aarch64-none-circle/lib/libcirclenewlib.a"
mkdir -p "$repo/build/wolf-firmware-rad"
cd "$repo"
if ! bash tools/build_wolf_core.sh upstream/Wolf4SDL "$stdlib/install/aarch64-none-circle/include" build/wolf-core-arm64 >build/core-build.log 2>&1; then
    tail -n 50 build/core-build.log; exit 1
fi
cp firmware/Makefile build/wolf-firmware-rad/Makefile
make -C build/wolf-firmware-rad REPO="$repo" STDLIB_ROOT="$stdlib" -j"${JOBS:-4}"
cp build/wolf-firmware-rad/kernel8.img build/kernel_wolf.img
sha256sum build/kernel_wolf.img
