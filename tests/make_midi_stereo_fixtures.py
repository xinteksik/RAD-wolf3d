"""Write synthetic single-note MIDI fixtures; no original game music."""
from pathlib import Path
import struct
import sys


def variable(value):
    output = [value & 127]
    while value >> 7:
        value >>= 7
        output.insert(0, (value & 127) | 128)
    return bytes(output)


def main():
    target = Path(sys.argv[1]) / 'MIDI'
    target.mkdir(parents=True, exist_ok=True)
    fixtures = (
        ('CORNER', 0, 0, 60, ()),
        ('DUNGEON', 1, 0, 60, ()),
        ('WARMARCH', 0, 32, 48, ()),
        ('GETTHEM', 9, 0, 38, ()),
        ('HEADACHE', 0, 0, 60, ((10, 96),)),
        ('HITLWLTZ', 0, 0, 60, ((10, 64),)),
        ('INTROCW3', 0, 0, 60, ((42, 100),)),
        ('NAZI_NOR', 0, 58, 48, ()),
        ('NAZI_OMI', 0, 0, 60, ((10, 32), (42, 100))),
    )
    for name, channel, program, note, controls in fixtures:
        track = bytearray(b'\0\xff\x51\x03\x07\xa1\x20')
        for control, value in controls:
            track += bytes((0, 0xb0 | channel, control, value))
        # Program changes after pan must not overwrite that explicit pan.
        track += bytes((0, 0xc0 | channel, program, 0, 0x90 | channel, note, 100))
        track += variable(2000) + bytes((0x80 | channel, note, 0))
        track += variable(2000) + b'\xff\x2f\0'
        header = b'MThd' + struct.pack('>IHHH', 6, 0, 1, 1000)
        (target / f'{name}.MID').write_bytes(header + b'MTrk' + struct.pack('>I', len(track)) + track)


if __name__ == '__main__':
    main()
