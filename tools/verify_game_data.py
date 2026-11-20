#!/usr/bin/env python3
"""GPL-3.0-or-later. Check local data; never downloads or modifies game files."""
import argparse
import hashlib
import json
from pathlib import Path

BASES = ('AUDIOHED', 'AUDIOT', 'GAMEMAPS', 'MAPHEAD', 'VGADICT', 'VGAGRAPH',
         'VGAHEAD', 'VSWAP')


def select_edition(files, requested):
    if requested != 'auto':
        return requested
    for edition in ('WL6', 'WL1'):
        if all(f'{base}.{edition}' in files for base in BASES):
            return edition
    for edition in ('WL6', 'WL1'):
        if any(f'{base}.{edition}' in files for base in BASES):
            return edition
    return 'WL6'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--edition', choices=('auto', 'WL1', 'WL6'), default='auto')
    args = parser.parse_args()
    manifest = Path(__file__).resolve().parents[1] / 'docs/game-data-hashes.json'
    files = {p.name.upper(): p for p in args.directory.iterdir() if p.is_file()}
    edition = select_edition(files, args.edition)
    if edition == 'WL1':
        manifest = manifest.with_name('game-data-shareware-hashes.json')
    records = json.loads(manifest.read_text())['files']
    print(f'Verifying {edition} reference data')
    failed = False
    for record in records:
        path = files.get(record['file'])
        if not path:
            print(f"MISSING {record['file']}"); failed = True; continue
        data = path.read_bytes()
        good = len(data) == record['bytes'] and hashlib.sha256(data).hexdigest() == record['sha256']
        print(f"{'OK' if good else 'DIFFERENT'} {record['file']} MD5={hashlib.md5(data).hexdigest()}")
        failed |= not good
    raise SystemExit(1 if failed else 0)


if __name__ == '__main__':
    main()
