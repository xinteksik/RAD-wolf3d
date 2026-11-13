#!/usr/bin/env python3
"""Refresh candidate metadata and raw-byte source checksums after a build."""
from pathlib import Path
import argparse
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kernel', type=Path, required=True)
    args = parser.parse_args()
    payload = args.kernel.read_bytes()
    release = ROOT / 'RELEASE.json'
    data = json.loads(release.read_text())
    data.update(latest_candidate_sha256=hashlib.sha256(payload).hexdigest(),
                latest_candidate_bytes=len(payload), latest_candidate_hardware_test='pending')
    release.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    files = sorted(path for path in ROOT.rglob('*') if path.is_file()
                   and path.name != 'SHA256SUMS'
                   and not {'.git', 'upstream', 'build', 'toolchains', '__pycache__'}
                   .intersection(path.relative_to(ROOT).parts))
    (ROOT / 'SHA256SUMS').write_text(''.join(
        hashlib.sha256(path.read_bytes()).hexdigest() + '  '
        + path.relative_to(ROOT).as_posix() + '\n' for path in files), encoding='utf-8')
    print(f"Candidate {data['latest_candidate_sha256']}; {len(files)} source checksums")


if __name__ == '__main__':
    main()
