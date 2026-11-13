#!/usr/bin/env python3
"""Package the source-only update and a data-free Windows/SD test release."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import shutil
import zipfile

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = {'.git', 'upstream', 'build', 'toolchains', '__pycache__'}
DATA = {'.wl6', '.wl1', '.sod', '.wad', '.sf2', '.mid', '.wlf'}


def source_files():
    return sorted(path for path in ROOT.rglob('*') if path.is_file()
                  and not EXCLUDED.intersection(path.relative_to(ROOT).parts))


def audit_zip(payload, name):
    with zipfile.ZipFile(io.BytesIO(payload)) as archive:
        for info in archive.infolist():
            path = Path(info.filename)
            if path.suffix.lower() in DATA:
                raise ValueError(f'Runtime game/music/font data in {name}: {info.filename}')
            if path.suffix.lower() == '.zip':
                audit_zip(archive.read(info), name + '/' + info.filename)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--windows-kit', type=Path, required=True)
    parser.add_argument('--corresponding-source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    release = json.loads((ROOT / 'RELEASE.json').read_text())
    if hashlib.sha256(args.kernel.read_bytes()).hexdigest() != release['latest_candidate_sha256']:
        parser.error('Kernel does not match RELEASE.json')
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Use an empty release output directory')
    output.mkdir(parents=True, exist_ok=True)
    label = 'RAD-Wolf-v' + release['version']
    package = output / 'Test-Package'
    sd = package / 'SD-CONTENTS'
    game = sd / 'RADWOLF'
    shutil.copytree(args.windows_kit, game)
    (package / 'SOURCE').mkdir()
    build_log = game / 'converter-build.log'
    if build_log.exists():
        shutil.move(str(build_log), package / 'SOURCE/windows-converter-build.log')
    shutil.copy2(args.kernel, sd / 'kernel_wolf_midi.img')
    config = (ROOT / 'sd/config.txt').read_text().replace('kernel=kernel_wolf.img', 'kernel=kernel_wolf_midi.img')
    (sd / 'config.txt.example').write_text(config, encoding='utf-8')
    (sd / 'RAD').mkdir()
    shutil.copy2(ROOT / 'sd/RAD/rad.cfg', sd / 'RAD/rad.cfg.example')
    shutil.copy2(ROOT / 'config/AUDIO.CFG.example', game / 'AUDIO.CFG.example')
    (game / 'AUDIO.STEREO.CFG.example').write_text(
        '# Example for a real 8580 at D400 and a real 6581 at D500.\n'
        'STEREO=ON\nSID1_ADDRESS=$D400\nSID1_TYPE=8580\n'
        'SID2_ADDRESS=$D500\nSID2_TYPE=6581\nSTEREO_WIDTH=100\nMIDI_STEREO_WIDTH=70\n')
    shutil.copy2(ROOT / ('docs/RELEASE-v' + release['version'] + '.md'), package / 'RELEASE-NOTES.md')
    shutil.copy2(ROOT / 'RELEASE.json', package / 'BUILD-INFO.json')
    (package / 'README.txt').write_text(
        'RAD-WOLF v' + release['version'] + ' - WINDOWS / SD TEST PACKAGE\n'
        '==========================================================\n\n'
        '1. Extract the entire ZIP. It contains no game data or SoundFont.\n'
        '2. Back up your existing working RAD SD card. Keep its Pi boot files,\n'
        '   rad-prefetch.bin, RAD timing settings and other working configuration.\n'
        '3. Copy SD-CONTENTS onto that card. In the existing config.txt select:\n'
        '       kernel=kernel_wolf_midi.img\n'
        '   Other clock/GPIO/PAL settings stay unchanged. .example files are\n'
        '   references; they do not overwrite your active audio or boot config.\n'
        '4. Put all eight full-version WL6 files into RADWOLF beside the BAT,\n'
        '   then double-click RADWOLF/Prepare-MIDI.bat in Windows. It creates MIDI.\n'
        '   You can also test this inside SD-CONTENTS/RADWOLF before using a card.\n'
        '   No Python, compiler, Internet access or administrator rights needed.\n'
        '5. Optionally add your bank as RADWOLF/soundfont.sf2 and reboot.\n'
        '   Enable Music/AdLib. Without a font the game uses original OPL music.\n\n'
        'CONTROLS: I/K move, J/L turn, Z/X strafe, Commodore fire, Space use,\n'
        'Shift or R run, Return confirm, left-arrow back, 1-4 weapons.\n'
        'F1 during gameplay: full level map. F1 or left-arrow closes it.\n'
        'Gameplay pauses while the map is visible; music continues.\n\n'
        'TEST: prepare all 27 MIDI tracks; repeat the BAT; boot into menu; enter\n'
        'a level; test F1 map open/close, controls, sound effects and stereo.\n'
        'Loading menu... should remain visible during menu/font preparation.\n'
        'The preceding stereo build worked on C64; this combined candidate still\n'
        'needs hardware verification. Host checks and Windows preparation passed.\n\n'
        'SOURCE and LICENSES: firmware corresponding sources are in SOURCE/.\n'
        'The independent Windows converter sources/notices are retained under\n'
        'SD-CONTENTS/RADWOLF/source/. Read RADWOLF/README.txt for its inherited\n'
        'licensing conditions, validation, backup and repeat-run behavior.\n')
    github = output / (label + '-GitHub.zip')
    with zipfile.ZipFile(github, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in source_files():
            archive.write(path, path.relative_to(ROOT).as_posix())
    shutil.copy2(github, package / 'SOURCE' / github.name)
    # Refresh the application portion after release metadata/docs were finalized;
    # preserve the exact pinned dependency working sources from the build export.
    with zipfile.ZipFile(args.corresponding_source) as original, zipfile.ZipFile(
            package / 'SOURCE/Firmware-Corresponding-Source.zip', 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for info in original.infolist():
            if info.filename.startswith('RAD-Wolf/') and not info.filename.startswith('RAD-Wolf/upstream/'):
                continue
            archive.writestr(info, original.read(info))
        for path in source_files():
            archive.write(path, 'RAD-Wolf/' + path.relative_to(ROOT).as_posix())
    checksums = []
    for path in sorted(package.rglob('*')):
        if path.is_file():
            checksums.append(hashlib.sha256(path.read_bytes()).hexdigest() + '  ' + path.relative_to(package).as_posix())
    (package / 'SHA256SUMS.txt').write_text('\n'.join(checksums) + '\n')
    binary = output / (label + '-Test-Package.zip')
    with zipfile.ZipFile(binary, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(package.rglob('*')):
            if path.is_file():
                archive.write(path, label + '/' + path.relative_to(package).as_posix())
    for path in (github, binary):
        audit_zip(path.read_bytes(), path.name)
    (output / 'SHA256SUMS.txt').write_text(''.join(
        hashlib.sha256(path.read_bytes()).hexdigest() + '  ' + path.name + '\n' for path in (github,binary)))
    shutil.copy2(package / 'RELEASE-NOTES.md', output / 'RELEASE-NOTES.md')
    print(f'Ready for review/test (not published): {github}\n{binary}')


if __name__ == '__main__':
    main()
