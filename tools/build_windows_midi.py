#!/usr/bin/env python3
"""Build a standalone Windows MIDI preparation kit, separate from firmware.

GPL-3.0-or-later. The independently compiled wolfmidi tool retains its own
license/notices; its complete supplied source is copied into the kit.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
REVISION = '2cf4c684d45de2a5f0d1b34fd6b9e64beb0c8e79'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wolfmidi', type=Path, required=True)
    parser.add_argument('--compiler', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    upstream = args.wolfmidi.resolve()
    revision = subprocess.check_output(['git', '-c', f'safe.directory={upstream.as_posix()}',
        '-C', str(upstream), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        parser.error('Use the documented pinned wolfmidi revision')
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Output must be empty; refusing to overwrite an existing kit')
    tools = output / 'tools'
    tools.mkdir(parents=True, exist_ok=True)
    for name in ('Prepare-MIDI.bat', 'Prepare-MIDI.ps1', 'README.txt'):
        data = (ROOT / 'windows-midi' / name).read_text(encoding='utf-8-sig')
        (output / name).write_bytes(data.replace('\r\n', '\n').replace('\n', '\r\n').encode('utf-8-sig' if name.endswith('.ps1') else 'utf-8'))
    command = [str(args.compiler.resolve()), '-std=gnu++11', '-O2', '-static',
        '-static-libstdc++', '-static-libgcc', '-fno-exceptions', '-fno-rtti',
        '-include', 'climits', str(upstream / 'src/dro2midi.cpp'),
        str(upstream / 'src/midiio.cpp'), '-s', '-o', str(tools / 'dro2midi.exe')]
    build = subprocess.run(command, capture_output=True, text=True)
    (output / 'converter-build.log').write_text(build.stdout + build.stderr)
    if build.returncode:
        raise SystemExit('Converter compilation failed; see converter-build.log')
    for name in ('inst.txt', 'drum.txt', 'patch.txt'):
        shutil.copy2(upstream / name, tools / name)
    source = output / 'source/wolfmidi'
    shutil.copytree(upstream / 'src', source / 'src')
    for name in ('inst.txt', 'drum.txt', 'patch.txt', 'README.md', 'LICENSE', 'make.bat'):
        shutil.copy2(upstream / name, source / name)
    shutil.copy2(ROOT / 'LICENSE', output / 'source/RAD-Wolf-GPL-3.0.txt')
    shutil.copy2(ROOT / 'tools/build_windows_midi.py', output / 'source/build_windows_midi.py')
    license_root = args.compiler.resolve().parents[1] / 'share/licenses'
    for name in ('gcc-libs', 'libwinpthread', 'mingw-w64-libraries'):
        if (license_root / name).is_dir():
            shutil.copytree(license_root / name, output / 'source/toolchain-runtime-notices' / name)
    # The exact launcher source is supplied next to the executable, not frozen.
    build_info = dict(wolfmidi_revision=revision,
        compiler=subprocess.check_output([str(args.compiler), '--version'], text=True).splitlines()[0],
        options=command[1:10], executable_sha256=hashlib.sha256((tools / 'dro2midi.exe').read_bytes()).hexdigest())
    (output / 'source/BUILD-INFO.json').write_text(json.dumps(build_info, indent=2) + '\n')
    (output / 'source/BUILD.txt').write_text(
        'Build on Windows with MinGW-w64 GCC/G++. No source modifications are needed.\n'
        'Run from source/wolfmidi:\n'
        'g++ -std=gnu++11 -O2 -static -static-libstdc++ -static-libgcc -fno-exceptions '
        '-fno-rtti -include climits src/dro2midi.cpp src/midiio.cpp -s -o dro2midi.exe\n'
        'The executable is an independent host converter; it is not linked into RAD-Wolf.\n')
    print(f'Windows kit built: {output}')


if __name__ == '__main__':
    main()
