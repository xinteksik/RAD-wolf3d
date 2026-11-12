#!/usr/bin/env python3
"""GPL-3.0-or-later. Fetch pinned sources and install verified modified files."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(data):
    return hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest()


def install_overrides(root, repositories, entries):
    planned = []
    for item in entries:
        if item['repository'] not in repositories:
            continue
        repo = repositories[item['repository']]
        target = repo / item['file']
        if repo.resolve() not in target.resolve().parents:
            raise ValueError('Override target escapes repository')
        payload = (root / item['override']).read_bytes()
        if sha(payload) != item['sha256']:
            raise ValueError(f"Corrupt override: {item['override']}")
        current = sha(target.read_bytes())
        if current == item['sha256']:
            continue
        if current != item['base_sha256']:
            raise ValueError(f'Refusing to overwrite local changes: {target}')
        planned.append((target, payload))
    # Validate all files before modifying any checkout.
    for target, payload in planned:
        target.write_bytes(payload)
    return len(planned)


def run(*args):
    subprocess.run(args, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apps-only', action='store_true', help='Prepare only RAD/Wolf for host tests')
    parser.add_argument('--offline', action='store_true', help='Verify existing checkouts without fetching')
    args = parser.parse_args()
    if not shutil.which('git'):
        parser.error('git is required')
    records = json.loads((ROOT / 'UPSTREAM.lock.json').read_text())['repositories']
    if args.apps_only:
        records = [r for r in records if r['name'] in ('RAD-Doom', 'Wolf4SDL')]
    paths = {}
    for record in records:
        path = ROOT / record['path']
        if not (path / '.git').exists():
            if args.offline:
                parser.error(f'Missing checkout: {path}')
            if path.exists() and any(path.iterdir()):
                parser.error(f'Refusing nonempty non-Git directory: {path}')
            path.parent.mkdir(parents=True, exist_ok=True)
            run('git', '-c', 'core.autocrlf=false', 'clone', '--filter=blob:none',
                '--no-checkout', record['url'], str(path))
            run('git', '-C', str(path), 'checkout', '--detach', record['revision'])
        revision = subprocess.check_output(['git', '-C', str(path), 'rev-parse', 'HEAD'], text=True).strip()
        if revision != record['revision']:
            parser.error(f'Wrong revision in {path}; no reset performed')
        paths[record['name']] = path
    entries = json.loads((ROOT / 'overrides/manifest.json').read_text())
    print(f'Installed {install_overrides(ROOT, paths, entries)} modified files')
    if not args.apps_only and not args.offline:
        run('git', '-C', str(paths['Circle']), 'submodule', 'update', '--init',
            'addon/wlan/hostap')


if __name__ == '__main__':
    main()
