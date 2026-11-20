#!/usr/bin/env bash
# GPL-3.0-or-later. Opt-in actual-engine host smoke test; no hardware/SD writes.
# Run in a prepared Linux checkout: bash tools/test_dual_game.sh WL1_DIR WL6_DIR OUTPUT
set -euo pipefail
test "$#" = 3
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
wl1=$(realpath "$1")
wl6=$(realpath "$2")
mkdir -p "$3"
output=$(realpath "$3")
cd "$repo"
export RADWOLF_CC=gcc RADWOLF_CXX=g++ RADWOLF_NM=nm RADWOLF_HOSTED=1
for edition in WL6 WL1; do
    RADWOLF_EDITION=$edition bash tools/build_wolf_core.sh upstream/Wolf4SDL /usr/include "$output/core-$edition" >"$output/core-$edition.log" 2>&1
done
python3 tools/combine_wolf_cores.py "$output/core-WL6" "$output/core-WL1" "$output/combined" --nm nm --objcopy objcopy
for name in radwolf_game_edition radwolf_game_start; do
    gcc -std=c11 -O2 -DRADWOLF_BAREMETAL -Isrc -c "src/$name.c" -o "$output/combined/$name.o"
done
gcc -std=gnu11 -O2 -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL \
    -c tests/test_game_start.c -o "$output/harness.o"
g++ -std=c++11 -O2 -c tests/test_game_start_keys.cpp -o "$output/keys.o"
g++ "$output/harness.o" "$output/keys.o" "$output/combined/"*.o -lm -o "$output/test_game_start"
for edition in WL1 WL6; do
    game=$wl1
    if [ "$edition" = WL6 ]; then game=$wl6; fi
    run="$output/run-$edition"
    mkdir -p "$run/SD:RADWOLF"
    python3 - "$game" "$run/SD:RADWOLF" "$edition" <<'PY'
from pathlib import Path
import shutil, sys
source, target = map(Path, sys.argv[1:3])
edition = sys.argv[3]
for base in ('AUDIOHED', 'AUDIOT', 'GAMEMAPS', 'MAPHEAD', 'VGADICT', 'VGAGRAPH', 'VGAHEAD', 'VSWAP'):
    name = f'{base}.{edition}'
    shutil.copy2(source / name, target / name)
    (target / name.lower()).symlink_to(name)
PY
    (cd "$run" && timeout 60 "$output/test_game_start" "$edition") > "$output/$edition.log" 2>&1 || { cat "$output/$edition.log"; exit 1; }
    cat "$output/$edition.log"
done
