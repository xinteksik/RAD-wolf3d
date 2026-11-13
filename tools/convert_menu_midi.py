#!/usr/bin/env python3
"""GPL-3.0-or-later. Convert the local menu IMF using a built wolfmidi tool.

Run from a wolfmidi checkout containing inst.txt, drum.txt and patch.txt.
No converted game music is part of the source publication.
"""
import argparse
from pathlib import Path
import struct
import subprocess


def extract_song(audiohed, audiot, index, name):
    """Wolf3D WL6 stores 27 music chunks after its sound chunks."""
    header = audiohed.read_bytes()
    if len(header) % 4 or len(header) < 28 * 4:
        raise ValueError('Invalid WL6 audio header')
    offsets = struct.unpack('<' + 'I' * (len(header) // 4), header)
    if not 0 <= index < 27:
        raise ValueError('Invalid WL6 song index')
    start = offsets[len(offsets) - 28 + index]
    end = offsets[len(offsets) - 27 + index]
    data = audiot.read_bytes()
    if not 0 <= start < end <= len(data):
        raise ValueError('Invalid menu chunk offsets')
    chunk = data[start:end]
    size = int.from_bytes(chunk[:2], 'little')
    if not size or size % 4 or size + 2 > len(chunk):
        raise ValueError('Invalid menu IMF length')
    # Carmackized WL6 chunks carry IMF metadata; check the actual song name.
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
    args = parser.parse_args()
    output = args.output_directory.resolve()
    output.mkdir(parents=True, exist_ok=True)
    source = output / 'WONDERIN.WLF'
    source.write_bytes(extract_menu(args.game_directory / 'AUDIOHED.WL6',
                                   args.game_directory / 'AUDIOT.WL6'))
    subprocess.run([str(args.converter.resolve()), '-k', '49', '-i',
                    str(source), str(output / 'WONDERIN.MID')], check=True)


if __name__ == '__main__':
    main()
