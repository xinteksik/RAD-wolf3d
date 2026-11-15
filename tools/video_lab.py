#!/usr/bin/env python3
"""GPL-3.0-or-later. Local, offline HTML comparison of real C64 bitmap variants."""
import argparse
import base64
import hashlib
import html
import json
from pathlib import Path
import struct
import subprocess
import zlib

PALETTE = [(0,0,0),(255,255,255),(104,55,43),(112,164,178),
           (111,61,134),(88,141,67),(53,40,121),(184,199,111),
           (111,79,37),(67,57,0),(154,103,89),(68,68,68),
           (108,108,108),(154,210,132),(108,94,181),(149,149,149)]
HEADER = b'RWF1LE\0\0' + struct.pack('<HHI', 320, 200, 256000)
VARIANTS = [
    ('current', 'Aktuální · bez rastru', 20, 0, 350, 1),
    ('dither25', 'Rastr 25 %', 20, 25, 350, 1),
    ('dither50', 'Rastr 50 %', 20, 50, 350, 1),
    ('dither100', 'Rastr 100 %', 20, 100, 350, 1),
    ('darker', 'Tmavší textury', 18, 0, 350, 1),
    ('lower-exposure', 'Nižší expozice', 20, 0, 300, 1),
    ('original', 'Původní převod i na podlaze', 20, 100, 350, 0),
]


def png(rgb, width=320, height=200):
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
    rows = b''.join(b'\0' + rgb[y * width * 3:(y + 1) * width * 3] for y in range(height))
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b'')


def decode_koala(payload):
    if len(payload) != 10003 or payload[:2] != b'\0\x60':
        raise ValueError('Expected 10003-byte Koala file at $6000')
    data = payload[2:]
    if data[10000] > 15 or any(color > 15 for color in data[9000:10000]):
        raise ValueError('Invalid C64 color')
    indices = bytearray(64000)
    for y in range(200):
        for x in range(0, 320, 8):
            cell = (y // 8) * 40 + x // 8
            colors = [data[10000], data[8000 + cell] >> 4,
                      data[8000 + cell] & 15, data[9000 + cell]]
            bits = data[(y // 8) * 320 + x + y % 8]
            for pair in range(4):
                color = colors[(bits >> (6 - pair * 2)) & 3]
                offset = y * 320 + x + pair * 2
                indices[offset:offset + 2] = bytes([color, color])
    return bytes(indices), bytes(channel for index in indices for channel in PALETTE[index])


def viewer_prg(payload):
    """A SYS2064 C64 program displaying the exact bitmap; no assembler needed."""
    decode_koala(payload)
    data = payload[2:]
    memory = bytearray(0x4800 - 0x0801)
    memory[:13] = b'\x0b\x08\x0a\x00\x9e2064\x00\x00\x00\x00'
    code = bytearray.fromhex('78 a9 3b 8d 11 d0 a9 18 8d 16 d0 8d 18 d0 ad 00 dd 29 fc 09 03 8d 00 dd a9 00 8d 20 d0')
    code += bytes([0xa9, data[10000], 0x8d, 0x21, 0xd0, 0xa2, 0])
    loop = len(code)
    for source, target in ((0x4000, 0x0400), (0x4400, 0xd800)):
        for page in range(4):
            code += bytes([0xbd, 0, (source >> 8) + page, 0x9d, 0, (target >> 8) + page])
    code += bytes([0xe8, 0xd0, (loop - (len(code) + 3)) & 255])
    idle = 0x0810 + len(code)
    code += bytes([0x4c, idle & 255, idle >> 8])
    memory[0x0810 - 0x0801:0x0810 - 0x0801 + len(code)] = code
    memory[0x2000 - 0x0801:0x2000 - 0x0801 + 8000] = data[:8000]
    memory[0x4000 - 0x0801:0x4000 - 0x0801 + 1000] = data[8000:9000]
    memory[0x4400 - 0x0801:0x4400 - 0x0801 + 1000] = data[9000:10000]
    return b'\x01\x08' + memory


def image_uri(data):
    return 'data:image/png;base64,' + base64.b64encode(data).decode()


def build(renderer, inputs, output):
    output.mkdir(parents=True, exist_ok=False)
    scenes = []
    for index, source in enumerate(inputs):
        raw = source.read_bytes()
        if len(raw) != 256016 or raw[:16] != HEADER:
            raise ValueError(f'Invalid RWF1LE frame: {source}')
        rgb = bytes(channel for pixel in struct.iter_unpack('<I', raw[16:])
                    for channel in ((pixel[0] >> 16) & 255, (pixel[0] >> 8) & 255, pixel[0] & 255))
        scene = dict(name=source.stem, sha256=hashlib.sha256(raw).hexdigest(),
                     source=image_uri(png(rgb)), variants=[])
        baseline = None
        for slug, title, brightness, dither, exposure, tags in VARIANTS:
            stem = f'scene-{index + 1}-{slug}'
            koala = output / (stem + '.koa')
            subprocess.run([str(renderer.resolve()), str(source.resolve()), str(koala.resolve()),
                            str(brightness), str(dither), str(exposure), str(tags), '3'], check=True)
            payload = koala.read_bytes()
            indices, pixels = decode_koala(payload)
            if baseline is None:
                baseline = indices
            (output / (stem + '.prg')).write_bytes(viewer_prg(payload))
            screenshot = png(pixels)
            (output / (stem + '.png')).write_bytes(screenshot)
            scene['variants'].append(dict(title=title, slug=slug, image=image_uri(screenshot),
                prg=stem + '.prg', koala=stem + '.koa', brightness=brightness,
                dither=dither, exposure=exposure, tags=tags,
                changed=round(sum(a != b for a, b in zip(indices, baseline)) / 640, 1)))
        scenes.append(scene)
    template = Path(__file__).with_name('video_lab.html').read_text(encoding='utf-8')
    page = template.replace('/*SCENES*/[]', json.dumps(scenes, ensure_ascii=False).replace('</', '<\\/'))
    (output / 'index.html').write_text(page, encoding='utf-8')
    manifest = dict(kind='offline C64 bitmap simulation', renderer='firmware packer with scalar host arithmetic',
                    temporal_history='three repeated frames, not a recording of prior gameplay frames',
                    source='RWF1LE original RGB with explicit floor/ceiling tags',
                    scenes=[dict(name=s['name'], sha256=s['sha256']) for s in scenes])
    (output / 'simulation.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (output / 'SHA256SUMS').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest() + '  ' + p.name + '\n'
        for p in sorted(output.iterdir()) if p.is_file() and p.name != 'SHA256SUMS'))
    return output / 'index.html'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--renderer', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='New session directory')
    parser.add_argument('frames', type=Path, nargs='+')
    args = parser.parse_args()
    print(build(args.renderer, args.frames, args.output).resolve())
