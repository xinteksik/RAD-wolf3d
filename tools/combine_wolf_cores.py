#!/usr/bin/env python3
"""GPL-3.0-or-later. Isolate the two edition-specific Wolf engine variants.

Shared RAD, audio, surface and file adapters are linked only once. Rename
defined engine symbols and their references, leaving libc and hardware imports
alone. Audit the shared-to-engine boundary so a new callback cannot silently
target the wrong edition.
"""
import argparse
from pathlib import Path
import shutil
import subprocess

ENGINE_OBJECTS = (
    'id_ca id_in id_pm id_sd id_us id_vh id_vl signon wl_agent wl_act1 '
    'wl_act2 wl_state wl_game wl_draw wl_plane wl_scale wl_shade wl_inter '
    'wl_main wl_menu wl_play wl_text wl_utils wl_atmos wl_cloudsky '
    'wl_parallax wl_debug radwolf_wolf_event_input radwolf_wolf_map'
).split()


def symbols(nm, paths, defined):
    option = '--defined-only' if defined else '--undefined-only'
    result = subprocess.check_output(
        [nm, '-g', option, *map(str, paths)], text=True)
    return {line.split()[-1] for line in result.splitlines()
            if line.strip() and not line.rstrip().endswith(':')}


def combine(full, shareware, output, nm, objcopy):
    if output.exists() and any(output.iterdir()):
        raise ValueError('Combined output must be empty')
    full_engine = [full / (name + '.o') for name in ENGINE_OBJECTS]
    share_engine = [shareware / (name + '.o') for name in ENGINE_OBJECTS]
    common = sorted(path for path in full.glob('*.o') if path not in full_engine)
    engine_symbols = symbols(nm, share_engine, True)
    shared_imports = symbols(nm, common, False)
    boundary = shared_imports & (engine_symbols | symbols(nm, full_engine, True))
    if boundary != {'radwolf_wolf_input_event'}:
        raise ValueError(f'Unexpected shared-to-engine callbacks: {sorted(boundary)}')
    output.mkdir(parents=True, exist_ok=True)
    share_map = output / 'wl1-symbols.txt'
    share_map.write_text(''.join(f'{name} radwolf_wl1_{name}\n'
                                 for name in sorted(engine_symbols)))
    full_map = output / 'wl6-symbols.txt'
    full_map.write_text('radwolf_wolf_main radwolf_wl6_main\n'
                        'radwolf_wolf_input_event radwolf_wl6_input_event\n')
    for path in full_engine:
        subprocess.run([objcopy, '--redefine-syms=' + str(full_map),
                        str(path), str(output / path.name)], check=True)
    for path in share_engine:
        subprocess.run([objcopy, '--redefine-syms=' + str(share_map),
                        str(path), str(output / ('wl1_' + path.name))], check=True)
    for path in common:
        shutil.copy2(path, output / path.name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('full', type=Path)
    parser.add_argument('shareware', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--nm', default='aarch64-none-elf-nm')
    parser.add_argument('--objcopy', default='aarch64-none-elf-objcopy')
    args = parser.parse_args()
    combine(args.full, args.shareware, args.output, args.nm, args.objcopy)


if __name__ == '__main__':
    main()
