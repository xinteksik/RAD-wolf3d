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
mkdir -p "$output_dir"
objects=()
for source in id_ca id_in id_pm id_sd id_us id_vh id_vl signon wl_agent wl_act1 wl_act2 wl_state wl_game wl_draw wl_plane wl_scale wl_shade wl_inter wl_main wl_menu wl_play wl_text wl_utils wl_atmos wl_cloudsky wl_parallax wl_debug; do
    aarch64-none-elf-gcc -std=gnu99 -O2 -ffreestanding -DRADWOLF_BAREMETAL \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$wolf_dir/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
for source in radwolf_input radwolf_rad_input radwolf_wolf_event_input radwolf_map radwolf_wolf_map radwolf_surface radwolf_file radwolf_stdio_file; do
    aarch64-none-elf-gcc -std=gnu99 -O2 -ffreestanding -DRADWOLF_BAREMETAL \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$repo_dir/src/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
aarch64-none-elf-g++ -std=gnu++11 -O2 -ffreestanding -fno-exceptions -fno-rtti \
    -I"$repo_dir/src" -c "$repo_dir/src/radwolf_wolf_rad_input.cpp" \
    -o "$output_dir/radwolf_wolf_rad_input.o"
objects+=("$output_dir/radwolf_wolf_rad_input.o")
# Remove the superseded stub so wildcard firmware linking cannot retain it.
rm -f "$output_dir/radwolf_silent_audio.o"
for source in radwolf_audio radwolf_audio_config radwolf_midi radwolf_downsample radwolf_wolf_sdl; do
    aarch64-none-elf-gcc -std=gnu99 -O2 -ffreestanding -DRADWOLF_BAREMETAL \
        -Werror=implicit-function-declaration \
        -I"$repo_dir/src/baremetal" -I"$repo_dir/src" \
        -I"$newlib_include" -I"$wolf_dir" \
        -c "$repo_dir/src/$source.c" -o "$output_dir/$source.o"
    objects+=("$output_dir/$source.o")
done
for source in "$repo_dir/src/radwolf_opl.cpp" "$wolf_dir/dosbox/dbopl.cpp"; do
    object="$output_dir/$(basename "${source%.cpp}").o"
    aarch64-none-elf-g++ -std=gnu++11 -O2 -ffreestanding -fno-exceptions -fno-rtti \
        -DUSE_GPL -I"$repo_dir/src/baremetal" -I"$newlib_include" \
        -c "$source" -o "$object"
    objects+=("$object")
done
aarch64-none-elf-nm -u "${objects[@]}"
if aarch64-none-elf-nm -u "${objects[@]}" | grep -E ' U (SDL_|Mix_)'; then
    echo "Unexpected SDL runtime dependency in gameplay slice" >&2
    exit 1
fi
