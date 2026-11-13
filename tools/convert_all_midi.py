#!/usr/bin/env python3
"""Convert all full WL6 songs with wolfmidi's original per-song options.

GPL-3.0-or-later. Run from wolfmidi's directory with its mapping files.
wolfmidi remains a separate GPL-2.0-only executable, not firmware code.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from convert_menu_midi import extract_song

SONGS = ('CORNER', 'DUNGEON', 'WARMARCH', 'GETTHEM', 'HEADACHE', 'HITLWLTZ',
         'INTROCW3', 'NAZI_NOR', 'NAZI_OMI', 'POW', 'SALUTE', 'SEARCHN',
         'SUSPENSE', 'VICTORS', 'WONDERIN', 'FUNKYOU', 'ENDLEVEL', 'GOINGAFT',
         'PREGNANT', 'ULTIMATE', 'NAZI_RAP', 'ZEROHOUR', 'TWELFTH', 'ROSTER',
         'URAHERO', 'VICMARCH', 'PACMAN')
# Loop corrections from wolfmidi make.bat at revision 2cf4c684.
LOOPS = {
    'CORNER': (1, 1), 'DUNGEON': (4, 12), 'GETTHEM': (10, 25, 22),
    'HITLWLTZ': (8, 1, 2), 'NAZI_NOR': (2, 4, 4), 'NAZI_OMI': (10, 9),
    'NAZI_RAP': (10, 13), 'PACMAN': (2, 9, 8), 'POW': (6, 3),
    'ROSTER': (10, 49), 'SALUTE': (4, 3), 'SEARCHN': (10, 7, 8),
    'SUSPENSE': (10, 13), 'VICMARCH': (8, 2), 'VICTORS': (2, 2, 3),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game_directory', type=Path)
    parser.add_argument('converter', type=Path)
    parser.add_argument('output_directory', type=Path)
    args = parser.parse_args()
    output = args.output_directory.resolve()
    midi_dir, imf_dir = output / 'MIDI', output / 'WLF'
    midi_dir.mkdir(parents=True, exist_ok=True)
    imf_dir.mkdir(parents=True, exist_ok=True)
    header, data = (args.game_directory / 'AUDIOHED.WL6',
                    args.game_directory / 'AUDIOT.WL6')
    # Validate the entire edition before starting external conversions.
    songs = [extract_song(header, data, index, name)
             for index, name in enumerate(SONGS)]
    records = []
    for name, song in zip(SONGS, songs):
        source, target = imf_dir / f'{name}.WLF', midi_dir / f'{name}.MID'
        source.write_bytes(song)
        options = ['-k', '49', '-i']
        for key, value in zip(('-l', '-m', '-n'), LOOPS.get(name, ())):
            options += [key, str(value)]
        subprocess.run([str(args.converter.resolve()), *options,
                        str(source), str(target)], check=True)
        payload = target.read_bytes()
        if not payload.startswith(b'MThd'):
            raise ValueError(f'Converter did not produce MIDI: {name}')
        records.append(dict(song=name, options=options,
                            sha256=hashlib.sha256(payload).hexdigest()))
    (output / 'music-manifest.json').write_text(json.dumps(records, indent=2) + '\n')
    print(f'Converted all {len(records)} WL6 songs')


if __name__ == '__main__':
    main()
