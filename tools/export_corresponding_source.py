#!/usr/bin/env python3
"""Export application and the exact pinned dependency working sources for a binary.

GPL-3.0-or-later. Run on Linux with prepared application sources and the
Circle/Newlib source checkout used by the build. No toolchains or build outputs.
"""
import argparse
import json
import os
from pathlib import Path
import stat
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN = {'.wl6', '.wl1', '.wad', '.sf2', '.mid', '.wlf', '.o', '.a', '.elf',
             '.img', '.exe', '.dll', '.pyc'}


def git(path, *args):
    return subprocess.check_output(['git', '-c', f'safe.directory={path}',
                                   '-C', str(path), *args])


def export(application_root, stdlib_root, destination):
    records = json.loads((application_root / 'UPSTREAM.lock.json').read_text())['repositories']
    seen, written, revisions = set(), set(), {}
    archive = zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED, compresslevel=6)

    def write(path, name):
        if name in written or path.suffix.lower() in FORBIDDEN:
            return
        written.add(name)
        if path.is_symlink():
            link = os.readlink(path)
            if os.path.isabs(link):
                raise ValueError(f'Absolute source symlink: {name}')
            item = zipfile.ZipInfo(name)
            item.create_system = 3
            item.external_attr = (stat.S_IFLNK | 0o777) << 16
            archive.writestr(item, link)
        else:
            archive.write(path, name)

    def repository(path, relative, expected):
        path = path.resolve()
        if path in seen:
            return
        actual = git(path, 'rev-parse', 'HEAD').decode().strip()
        if actual != expected:
            raise ValueError(f'Unexpected source revision: {relative}')
        seen.add(path)
        revisions[relative] = actual
        for entry in git(path, 'ls-files', '--stage', '-z').split(b'\0'):
            if not entry:
                continue
            metadata, filename = entry.split(b'\t', 1)
            mode, revision, _ = metadata.decode().split()
            filename = filename.decode()
            source = path / filename
            name = 'RAD-Wolf/' + relative + '/' + filename
            if mode == '160000':
                if source.is_dir() and (source / '.git').exists():
                    repository(source, relative + '/' + filename, revision)
                continue
            if source.is_file() or source.is_symlink():
                write(source, name)

    try:
        for path in application_root.rglob('*'):
            relative = path.relative_to(application_root)
            if any(p in ('.git', 'build', 'upstream', 'toolchains', '__pycache__') for p in relative.parts):
                continue
            if path.is_file():
                write(path, 'RAD-Wolf/' + relative.as_posix())
        for record in records:
            if record['name'] == 'circle-stdlib':
                path = stdlib_root
            elif record['name'] == 'Circle':
                path = stdlib_root / 'libs/circle'
            elif record['name'] == 'circle-newlib':
                path = stdlib_root / 'libs/circle-newlib'
            else:
                path = application_root / record['path']
            repository(path, record['path'], record['revision'])
        archive.writestr('SOURCE-SNAPSHOT.json', json.dumps(dict(repositories=revisions,
            source_files=len(written), generated_binaries_included=False), indent=2) + '\n')
        archive.writestr('READ-ME.txt',
            'Corresponding source snapshot for the accompanying RAD-Wolf test kernel.\n'
            'Includes application changes and pinned dependency working sources, with notices.\n'
            'Read RAD-Wolf/docs/BUILD.md and UPSTREAM.lock.json for the compiler and build.\n'
            'Run build_dependencies.sh to generate configuration/libraries, then build_firmware.sh.\n'
            'Compiler/toolchain binaries and commercial runtime game data are obtained separately.\n'
            'Submodule revisions present in this snapshot are in SOURCE-SNAPSHOT.json.\n')
    finally:
        archive.close()
    print(f'Exported {len(written)} source files from {len(seen)} repositories: {destination}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--application-root', type=Path, default=ROOT)
    parser.add_argument('--stdlib-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    export(args.application_root.resolve(), args.stdlib_root.resolve(), args.output)


if __name__ == '__main__':
    main()
