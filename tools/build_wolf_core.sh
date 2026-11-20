#!/usr/bin/env bash
# Compile the SDL-independent gameplay slice using the firmware toolchain.
set -eu
if [ "$#" -ne 3 ]; then
    echo "Usage: bash tools/build_wolf_core.sh WOLF_SOURCE NEWLIB_INCLUDE OUTPUT" >&2
    exit 2
fi
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
wolf_dir=$1
newlib_include=$2
output_dir=$3
edition_flags=(-DVERSIONALREADYCHOSEN -DCARMACIZED)
case "${RADWOLF_EDITION:-WL6}" in
    WL6) edition_flags+=(-DGOODTIMES);;
    WL1) edition_flags+=(-DUPLOAD);;
    *) echo 'RADWOLF_EDITION must be WL1 or WL6' >&2; exit 2;;
esac
cc=${RADWOLF_CC:-aarch64-none-elf-gcc}
cxx=${RADWOLF_CXX:-aarch64-none-elf-g++}
nm=${RADWOLF_NM:-aarch64-none-elf-nm}
mkdir -p "$output_dir"
runtime_flags=(-ffreestanding)
if [ "${RADWOLF_HOSTED:-0}" = 1 ]; then runtime_flags=(); fi
objects=()
for source in id_ca id_in id_pm id_sd id_us id_vh id_vl signon wl_agent wl_act1 wl_act2 wl_state wl_game wl_draw wl_plane wl_scale wl_shade wl_inter wl_main wl_menu wl_play wl_text wl_utils wl_atmos wl_cloudsky wl_parallax wl_debug; do
    "$cc" -std=gnu99 -O2 "${runtime_flags[@]}" -DRADWOLF_BAREMETAL "${edition_flags[@]}" \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$wolf_dir/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
for source in radwolf_input radwolf_rad_input radwolf_wolf_event_input radwolf_map radwolf_map_history radwolf_wolf_map radwolf_surface radwolf_capture radwolf_file radwolf_stdio_file; do
    "$cc" -std=gnu99 -O2 "${runtime_flags[@]}" -DRADWOLF_BAREMETAL "${edition_flags[@]}" \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$repo_dir/src/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
"$cxx" -std=gnu++11 -O2 "${runtime_flags[@]}" -fno-exceptions -fno-rtti \
    -I"$repo_dir/src" -c "$repo_dir/src/radwolf_wolf_rad_input.cpp" \
    -o "$output_dir/radwolf_wolf_rad_input.o"
objects+=("$output_dir/radwolf_wolf_rad_input.o")
# Remove the superseded stub so wildcard firmware linking cannot retain it.
rm -f "$output_dir/radwolf_silent_audio.o"
for source in radwolf_audio radwolf_audio_config radwolf_video_config radwolf_flash radwolf_midi radwolf_downsample radwolf_wolf_sdl; do
    "$cc" -std=gnu99 -O2 "${runtime_flags[@]}" -DRADWOLF_BAREMETAL "${edition_flags[@]}" \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$repo_dir/src/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
for source in "$repo_dir/src/radwolf_opl.cpp" "$wolf_dir/dosbox/dbopl.cpp"; do
    object="$output_dir/$(basename "${source%.cpp}").o"
    "$cxx" -std=gnu++11 -O2 "${runtime_flags[@]}" -fno-exceptions -fno-rtti \
        -DUSE_GPL -I"$repo_dir/src/baremetal" -I"$newlib_include" \
        -c "$source" -o "$object"
    objects+=("$object")
done
"$nm" -u "${objects[@]}"
if "$nm" -u "${objects[@]}" | grep -E ' U (SDL_|Mix_)'; then
    echo "Unexpected SDL runtime dependency in gameplay slice" >&2
    exit 1
fi
