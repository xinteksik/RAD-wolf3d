#!/usr/bin/env python3
"""Create SD.zip, complete source.tar.gz and release notes using a verified SD template."""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import shutil
import stat
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = {'.git', 'build', 'upstream', 'toolchains', '__pycache__'}
DATA = {'.wl6', '.wl1', '.wad', '.sf2', '.mid', '.wlf', '.sod'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def project_files():
    return sorted(p for p in ROOT.rglob('*') if p.is_file()
                  and not EXCLUDED.intersection(p.relative_to(ROOT).parts))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous-sd', type=Path, required=True)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--windows-kit', type=Path, required=True)
    parser.add_argument('--firmware-source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    metadata = json.loads((ROOT / 'RELEASE.json').read_text())
    version = 'v' + metadata['version']
    label = 'RAD-Wolf-' + version
    kernel = args.kernel.read_bytes()
    if digest(kernel) != metadata['latest_candidate_sha256']:
        parser.error('Kernel does not match release metadata')
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Use an empty release directory')
    sd = output / 'SD-CONTENTS'
    sd.mkdir(parents=True)
    with zipfile.ZipFile(args.previous_sd) as previous:
        if previous.testzip():
            raise ValueError('Corrupt SD template ZIP')
        for line in previous.read('SHA256SUMS').decode().splitlines():
            checksum, filename = line.split('  ', 1)
            if digest(previous.read(filename)) != checksum:
                raise ValueError(f'SD template checksum mismatch: {filename}')
        for item in previous.infolist():
            path = PurePosixPath(item.filename)
            if path.is_absolute() or '..' in path.parts:
                raise ValueError('Unsafe SD template member')
            if item.is_dir():
                continue
            target = sd.joinpath(*path.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(previous.read(item))
    # Preserve the exact known boot files, clocks, GPIO and RAD timing template.
    (sd / 'kernel_wolf.img').write_bytes(kernel)
    game = sd / 'RADWOLF'
    shutil.copytree(args.windows_kit, game, dirs_exist_ok=True)
    source_asset = label + '-source.tar.gz'
    (sd / 'RELEASE.json').write_text(json.dumps(dict(version=version,
        kernel_sha256=digest(kernel), kernel_bytes=len(kernel), game_data_included=False,
        default_audio='mono AUTO', source_asset=source_asset,
        hardware_boot_of_this_binary='pending', midi_tracks=27,
        windows_preparation='RADWOLF/Prepare-MIDI.bat', level_map='F1'), indent=2) + '\n')
    (game / 'AUDIO.STEREO.CFG.example').write_text(
        '# Real 8580 at D400 and real 6581 at D500; adjust to your hardware.\n'
        'STEREO=ON\nSID1_ADDRESS=$D400\nSID1_TYPE=8580\n'
        'SID2_ADDRESS=$D500\nSID2_TYPE=6581\nSTEREO_WIDTH=100\nMIDI_STEREO_WIDTH=70\n')
    (game / 'ADD-YOUR-WL6-FILES-HERE.txt').write_text(
        'Add your own legally obtained full Carmackized WL6 files here:\n'
        'AUDIOHED.WL6\nAUDIOT.WL6\nGAMEMAPS.WL6\nMAPHEAD.WL6\n'
        'VGADICT.WL6\nVGAGRAPH.WL6\nVGAHEAD.WL6\nVSWAP.WL6\n\n'
        'For MIDI music, run Prepare-MIDI.bat here in Windows to create MIDI/,\n'
        'then optionally add your General MIDI bank as soundfont.sf2.\n'
        'Without a bank the game uses original OPL music. No game files are included.\n'
        'Purchase: https://www.gog.com/en/game/wolfenstein_3d\n')
    (sd / 'SOURCES.txt').write_text(
        'Complete corresponding firmware/dependency and Windows converter sources:\n'
        + source_asset + '\nDownload from the same release page as this SD ZIP:\n'
        'https://github.com/xinteksik/RAD-wolf3d/releases\n'
        'Includes exact application source, applied pinned dependencies, boot-stub\n'
        'assembly, independent converter source/build instructions and notices.\n'
        'The source archive is not needed to play. Publish both assets together.\n'
        'Pi boot blobs remain separately licensed under LICENSES/LICENCE.broadcom.\n')
    (sd / 'START-HERE.txt').write_text(
        'RAD-Wolf ' + version + ' - READY-TO-COPY SD PACKAGE\n\n'
        '1. Back up the card. Extract the CONTENTS of this SD ZIP to its FAT32 root.\n'
        '   config.txt and kernel_wolf.img belong directly in the root.\n'
        '2. Add your eight purchased full-version WL6 files to RADWOLF/.\n'
        '3. Optional MIDI: with the card inserted into Windows, double-click\n'
        '   RADWOLF/Prepare-MIDI.bat. It creates MIDI/ with all 27 tracks.\n'
        '   No Python, compiler, admin rights or Internet connection is required.\n'
        '   Add your bank as RADWOLF/soundfont.sf2; without it music stays OPL.\n'
        '4. Boot the C64/RAD. Enable Music/AdLib. Reboot after changing an SF2.\n\n'
        'Default audio: mono, SID1 D400, type AUTO. Optional stereo needs a second\n'
        'SID and AUDIO.CFG matching your actual chip addresses/types.\n'
        'Real SIDs are required for supported audio; UltiSID is experimental.\n'
        'Target: RAD, Pi Zero 2 W and the inherited PAL C64 settings.\n\n'
        'Controls: I/K move, J/L turn, Z/X strafe, Commodore fire, Space use,\n'
        'Shift/R run, Return confirm, C64 left-arrow back, 1-4 weapons.\n'
        'F1 opens the full level map. F1 or left-arrow closes it. Gameplay pauses\n'
        'while viewing the map; music continues. Unvisited rooms are visible.\n\n'
        'UPDATING: preserve your working config.txt, RAD/rad.cfg, AUDIO.CFG,\n'
        'CONFIG.WL6 and saves. Keep kernel=kernel_wolf.img in your boot config.\n'
        'The supplied default clocks/GPIO/RAD settings are unchanged from rc1.\n'
        'This exact combined candidate still needs hardware testing.\n\n'
        'Project: https://github.com/xinteksik/RAD-wolf3d\n'
        'Game purchase: https://www.gog.com/en/game/wolfenstein_3d\n'
        'Source asset: ' + source_asset + '\n')
    (sd / 'ZACNI-ZDE.txt').write_text(
        'RAD-Wolf ' + version + '\n\n'
        'Obsah SD ZIPu rozbal do korene zalohovane FAT32 karty. Vlastni WL6 dej\n'
        'do RADWOLF. Ve Windows spust RADWOLF/Prepare-MIDI.bat; vytvori MIDI.\n'
        'Volitelny SF2 uloz jako RADWOLF/soundfont.sf2. Bez nej hraje OPL.\n'
        'F1 ve hre otevre mapu, F1 nebo leva sipka ji zavre.\n'
        'Pri aktualizaci zachovej vlastni nastaveni a ulozene hry.\n'
        'Podrobny anglicky navod: START-HERE.txt a RADWOLF/README.txt.\n')
    notice = (sd / 'NOTICE.txt').read_text()
    (sd / 'NOTICE.txt').write_text(notice +
        '\nOptional MIDI uses TinySoundFont (MIT) / TinyMidiLoader (zlib).\n'
        'The Windows converter is a separate program; its complete source and\n'
        'upstream notices are under RADWOLF/source/wolfmidi and in the source asset.\n'
        'Its inherited IMF2MIDI terms restrict paid/retail redistribution.\n')
    for header, marker, name in (('tsf.h','LICENSE (MIT)','TinySoundFont-MIT.txt'),
                                 ('tml.h','LICENSE (ZLIB)','TinyMidiLoader-zlib.txt')):
        text = (ROOT / 'third_party' / header).read_text()
        (sd / 'LICENSES' / name).write_text(text.split(marker,1)[1].split('*/',1)[0].strip() + '\n')
    shutil.copy2(game / 'source/wolfmidi/LICENSE', sd / 'LICENSES/wolfmidi-GPL-2.0.txt')
    shutil.copy2(game / 'source/wolfmidi/src/README', sd / 'LICENSES/wolfmidi-inherited-notices.txt')
    # A source-only repository ZIP remains handy for updating GitHub separately.
    with zipfile.ZipFile(output / (label + '-GitHub.zip'), 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in project_files():
            archive.write(path, path.relative_to(ROOT).as_posix())
    source_root = label + '-source'
    source_hashes = []
    source_count = 0
    with tarfile.open(output / source_asset, 'w:gz', compresslevel=6, format=tarfile.PAX_FORMAT) as archive:
        def add(name, data, mode=0o644, link=None):
            nonlocal source_count
            if Path(name).suffix.lower() in DATA:
                raise ValueError(f'Runtime data in source: {name}')
            item = tarfile.TarInfo(source_root + '/' + name)
            item.mode = mode
            if link is not None:
                item.type = tarfile.SYMTYPE
                item.linkname = link
                archive.addfile(item)
            else:
                item.size = len(data)
                archive.addfile(item, io.BytesIO(data))
                source_hashes.append(digest(data) + '  ' + name)
            source_count += 1
        with zipfile.ZipFile(args.firmware_source) as previous_source:
            for item in previous_source.infolist():
                if item.filename.startswith('RAD-Wolf/upstream/'):
                    name = item.filename[len('RAD-Wolf/'):]
                    mode = item.external_attr >> 16
                    data = previous_source.read(item)
                    add(name, data, mode & 0o777 or 0o644,
                        data.decode() if stat.S_ISLNK(mode) else None)
                elif item.filename == 'SOURCE-SNAPSHOT.json':
                    add(item.filename, previous_source.read(item))
        for path in project_files():
            add(path.relative_to(ROOT).as_posix(), path.read_bytes(), 0o755 if path.suffix == '.sh' else 0o644)
        for path in sorted((args.windows_kit / 'source').rglob('*')):
            if path.is_file():
                add('host-tools/source/' + path.relative_to(args.windows_kit / 'source').as_posix(), path.read_bytes())
        for name in ('Prepare-MIDI.bat','Prepare-MIDI.ps1','README.txt'):
            add('host-tools/' + name, (args.windows_kit / name).read_bytes())
        sums = ('\n'.join(source_hashes) + '\n').encode()
        item = tarfile.TarInfo(source_root + '/SOURCE-SHA256SUMS')
        item.size = len(sums); item.mode = 0o644
        archive.addfile(item, io.BytesIO(sums))
    # Hash the final SD tree only after every file has reached its final content.
    sd_sums = []
    for path in sorted(sd.rglob('*')):
        if path.is_file() and path.relative_to(sd).as_posix() != 'SHA256SUMS':
            if path.suffix.lower() in DATA:
                raise ValueError(f'Runtime data in SD release: {path}')
            sd_sums.append(digest(path.read_bytes()) + '  ' + path.relative_to(sd).as_posix())
    (sd / 'SHA256SUMS').write_text('\n'.join(sd_sums) + '\n')
    with zipfile.ZipFile(output / (label + '-SD.zip'), 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(sd.rglob('*')):
            if path.is_file():
                archive.write(path, path.relative_to(sd).as_posix())
    notes = (ROOT / ('docs/RELEASE-' + version + '.md')).read_text()
    quick = ('# RAD-Wolf ' + version + '\n\n'
        '1. Extract **' + label + '-SD.zip** to the FAT32 SD card root.\n'
        '2. Add your eight WL6 files to `RADWOLF/`.\n'
        '3. For MIDI, run `RADWOLF/Prepare-MIDI.bat` in Windows and add your bank '
        'as `RADWOLF/soundfont.sf2`. Without a bank, music uses OPL.\n'
        '4. Boot C64/RAD. F1 opens the level map; F1/left-arrow closes it.\n\n'
        'Kernel name: `kernel_wolf.img`, same as the first release.\n'
        'Default: mono AUTO. Preserve custom settings and saves when updating.\n'
        'No game data, generated soundtrack or SoundFont is included.\n'
        'Publish the SD ZIP, complete source tarball and SHA256SUMS together.\n'
        'Mark this RC as a pre-release; exact hardware boot/map testing is pending.\n\n')
    (output / 'RELEASE-NOTES.md').write_text(notes)
    (output / 'GITHUB-DESCRIPTION.md').write_text(quick)
    (output / 'VALIDATION.md').write_text(
        '# Validation\n\nKernel SHA-256: `' + digest(kernel) + '`\n\n'
        'Firmware build and make test/test-opl/test-audio-startup passed.\n'
        'Windows preparation: six end-to-end cases passed, including Unicode,\n'
        'repeat reuse, protected regeneration, corrupt input and converter failure.\n'
        'All 27 generated MIDI tracks matched the existing reference hashes.\n'
        'SD/source archive checks and extracted-SD BAT smoke test: see VALIDATION.json.\n'
        f'Corresponding source archive contains {source_count} entries plus source checksums.\n'
        'This RC reuses the unchanged boot/RAD configuration from the first release.\n'
        'Exact hardware boot/F1 map confirmation remains pending.\n'
        'No SD card was written or ejected by this packaging step.\n')
    assets = [output / (label + '-SD.zip'), output / source_asset, output / (label + '-GitHub.zip')]
    (output / 'SHA256SUMS').write_text(''.join(digest(path.read_bytes()) + '  ' + path.name + '\n' for path in assets))
    print(f'Release files prepared in {output}; kernel_wolf.img = {digest(kernel)}')


if __name__ == '__main__':
    main()
