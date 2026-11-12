#!/usr/bin/env bash
# GPL-3.0-or-later. Build the pinned Circle44.3/Newlib runtime.
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
stdlib=${STDLIB_ROOT:-$repo/upstream/circle-stdlib}
case "$stdlib" in *' '*) echo 'Build dependencies under a Linux path without spaces.' >&2; exit 1;; esac
command -v aarch64-none-elf-gcc >/dev/null
test -f "$stdlib/libs/circle/include/circle/sysconfig.h"
cmp "$repo/overrides/Circle/include/circle/sysconfig.h" "$stdlib/libs/circle/include/circle/sysconfig.h" || {
    echo 'Run prepare_sources.py first; RAD Circle settings are required.' >&2; exit 1;
}
cd "$stdlib"
./configure --raspberrypi 3 --prefix aarch64-none-elf-
make -j"${JOBS:-2}"
