#!/usr/bin/env python3
"""GPL-3.0-or-later. Convert the local menu IMF using a built wolfmidi tool.

Run from a wolfmidi checkout containing inst.txt, drum.txt and patch.txt.
No converted game music is part of the source publication.
"""
import argparse
from pathlib import Path
import struct
import subprocess

SONGS = ('CORNER', 'DUNGEON', 'WARMARCH', 'GETTHEM', 'HEADACHE', 'HITLWLTZ',
         'INTROCW3', 'NAZI_NOR', 'NAZI_OMI', 'POW', 'SALUTE', 'SEARCHN',
         'SUSPENSE', 'VICTORS', 'WONDERIN', 'FUNKYOU', 'ENDLEVEL', 'GOINGAFT',
         'PREGNANT', 'ULTIMATE', 'NAZI_RAP', 'ZEROHOUR', 'TWELFTH', 'ROSTER',
         'URAHERO', 'VICMARCH', 'PACMAN')
SHAREWARE_SONGS = ('CORNER', 'WARMARCH', 'GETTHEM', 'NAZI_NOR', 'POW',
                   'SEARCHN', 'SUSPENSE', 'WONDERIN', 'ENDLEVEL', 'ROSTER',
                   'URAHERO')


def find_audio(directory, edition='auto'):
    """Select one edition, accepting DOS filenames in either letter case."""
    files = {path.name.upper(): path for path in directory.iterdir()
             if path.is_file()}
    candidates = [ext for ext in ('WL1', 'WL6')
                  if any(f'{base}.{ext}' in files
                         for base in ('AUDIOHED', 'AUDIOT'))]
    if edition == 'auto':
        if len(candidates) != 1:
            raise ValueError('Supply one WL1 or WL6 audio set; '
                             'use --edition when both are present')
        edition = candidates[0]
    try:
        return edition, files[f'AUDIOHED.{edition}'], files[f'AUDIOT.{edition}']
    except KeyError as error:
        raise ValueError(f'Missing audio file: {error.args[0]}') from error


def extract_song(audiohed, audiot, index, name, edition=None):
    """Read one of 27 music slots; WL1 has 16 named empty placeholders."""
    edition = edition or ('WL1' if audiohed.suffix.upper() == '.WL1' else 'WL6')
    if edition not in ('WL1', 'WL6'):
        raise ValueError('Unsupported audio edition')
    header = audiohed.read_bytes()
    if len(header) % 4 or len(header) < 28 * 4:
        raise ValueError(f'Invalid {edition} audio header')
    offsets = struct.unpack('<' + 'I' * (len(header) // 4), header)
    if not 0 <= index < len(SONGS) or SONGS[index] != name:
        raise ValueError('Invalid song index or name')
    start = offsets[len(offsets) - 28 + index]
    end = offsets[len(offsets) - 27 + index]
    data = audiot.read_bytes()
    if not 0 <= start < end <= len(data) or end - start < 2:
        raise ValueError('Invalid menu chunk offsets')
    chunk = data[start:end]
    size = int.from_bytes(chunk[:2], 'little')
    if edition == 'WL1' and name not in SHAREWARE_SONGS:
        if (size != 0 or len(chunk) != 88 or
                not chunk[2:].startswith(name.encode('ascii') + b'\0')):
            raise ValueError(f'Invalid WL1 empty music slot: {name}')
        return None
    if not size or size % 4 or size + 2 > len(chunk):
        raise ValueError('Invalid menu IMF length')
    # Carmackized WL1/WL6 chunks carry IMF metadata; check the song name.
    if name.encode('ascii') + b'\0' not in chunk[size + 2:]:
        raise ValueError(f'Expected {name} metadata; unsupported game edition')
    return chunk[:size + 2]


def extract_menu(audiohed, audiot):
    return extract_song(audiohed, audiot, 14, 'WONDERIN')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game_directory', type=Path)
    parser.add_argument('converter', type=Path)
    parser.add_argument('output_directory', type=Path)
    parser.add_argument('--edition', choices=('auto', 'WL1', 'WL6'), default='auto')
    args = parser.parse_args()
    output = args.output_directory.resolve()
    output.mkdir(parents=True, exist_ok=True)
    source = output / 'WONDERIN.WLF'
    _, header, audio = find_audio(args.game_directory, args.edition)
    source.write_bytes(extract_menu(header, audio))
    subprocess.run([str(args.converter.resolve()), '-k', '49', '-i',
                    str(source), str(output / 'WONDERIN.MID')], check=True)


if __name__ == '__main__':
    main()
