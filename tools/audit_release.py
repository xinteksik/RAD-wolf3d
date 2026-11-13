#!/usr/bin/env python3
"""GPL-3.0-or-later. Audit the source publication, not an SD card."""
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN = {'.wl6', '.wl1', '.sod', '.sd1', '.sd2', '.sd3', '.wad', '.sf2', '.mid', '.wlf',
             '.wav', '.raw', '.ani', '.img', '.elf', '.o', '.exe', '.dll', '.pyc', '.bak'}


def main():
    failures = []
    files = []
    for path in ROOT.rglob('*'):
        relative = path.relative_to(ROOT)
        if any(p in ('.git', 'upstream', 'build', '__pycache__') for p in relative.parts):
            continue
        if not path.is_file():
            continue
        files.append(path)
        if path.suffix.lower() in FORBIDDEN:
            failures.append(f'Unexpected binary/data file: {relative}')
        if path.stat().st_size > 2_000_000:
            failures.append(f'Unexpected large file: {relative}')
        if path.suffix in ('.py', '.sh', '.md', '.cpp', '.c', '.h'):
            text = path.read_text(errors='replace')
            if re.search(r'/home/[^/\s]+/radwolf', text) or re.search(r'[A-Za-z]:\\Users\\[^\\\s]+', text):
                failures.append(f'Private absolute build path: {relative}')
    for item in json.loads((ROOT / 'overrides/manifest.json').read_text()):
        data = (ROOT / item['override']).read_bytes().replace(b'\r\n', b'\n')
        if hashlib.sha256(data).hexdigest() != item['sha256']:
            failures.append(f"Override hash mismatch: {item['override']}")
    # The new runtime must not request any RAD-Doom external media.
    for name in ('src/radwolf_kernel.cpp', 'src/radwolf_rad_video.c'):
        text = (ROOT / name).read_text()
        if 'SD:RADDOOM/' in text or 'doIntro(' in text:
            failures.append(f'Legacy runtime media dependency: {name}')
    for failure in failures:
        print(failure)
    if failures:
        raise SystemExit(1)
    print(f'Publication audit passed: {len(files)} source/document/image files; no runtime game data or kernel binaries')


if __name__ == '__main__':
    main()
