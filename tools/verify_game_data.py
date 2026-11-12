#!/usr/bin/env python3
"""GPL-3.0-or-later. Check local data; never downloads or modifies game files."""
import argparse
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    manifest = Path(__file__).resolve().parents[1] / 'docs/game-data-hashes.json'
    records = json.loads(manifest.read_text())['files']
    files = {p.name.upper(): p for p in args.directory.iterdir() if p.is_file()}
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
