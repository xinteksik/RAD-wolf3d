"""Opt-in actual BAT tests with separately supplied WL1 and WL6 game data."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ENABLED = os.name == 'nt' and os.environ.get('RADWOLF_WINDOWS_MIDI_KIT')
FILES = ('AUDIOHED.WL6', 'AUDIOT.WL6', 'GAMEMAPS.WL6', 'MAPHEAD.WL6',
         'VGADICT.WL6', 'VGAGRAPH.WL6', 'VGAHEAD.WL6', 'VSWAP.WL6')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


@unittest.skipUnless(ENABLED, 'Requires Windows and built kit')
class TestWindowsMidi(unittest.TestCase):
    edition = 'WL6'
    count = 27

    def setUp(self):
        data = os.environ.get(f'RADWOLF_{self.edition}_DATA')
        if not data:
            self.skipTest(f'Requires separately supplied {self.edition} test data')
        self.temp = tempfile.TemporaryDirectory(dir=os.environ.get('RADWOLF_TEST_DIRECTORY'))
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'SD card & test - Žluťoučký'
        self.kit = self.root / 'RADWOLF'
        shutil.copytree(os.environ['RADWOLF_WINDOWS_MIDI_KIT'], self.kit)
        self.data = Path(data)
        self.files = tuple(name.replace('WL6', self.edition) for name in FILES)

    def copy_game(self, directory=None):
        target = directory or self.kit
        target.mkdir(parents=True, exist_ok=True)
        for name in self.files:
            shutil.copy2(self.data / name, target / name)
        (target / 'soundfont.sf2').write_bytes(b'private font sentinel')
        (target / 'savegam0.wl6').write_bytes(b'private save sentinel')
        return {name: digest(target / name) for name in (*self.files, 'soundfont.sf2', 'savegam0.wl6')}

    def run_batch(self, argument=None):
        env = dict(os.environ, RADWOLF_NO_PAUSE='1')
        args = ['cmd.exe', '/d', '/c', 'Prepare-MIDI.bat']
        if argument is not None:
            args.append(str(argument))
        return subprocess.run(args, cwd=self.kit, env=env, capture_output=True,
                              text=True, encoding='utf-8', errors='replace', timeout=120)

    def check_result(self, directory):
        tracks = sorted((directory / 'MIDI').glob('*.MID'))
        self.assertEqual(len(tracks), self.count)
        manifest = json.loads((directory / 'MIDI/MIDI-INFO.json').read_text(encoding='utf-8-sig'))
        self.assertEqual(manifest['edition'], self.edition)
        self.assertEqual(len(manifest['tracks']), self.count)
        self.assertEqual(set(manifest['inputs']),
                         {f'AUDIOHED.{self.edition}', f'AUDIOT.{self.edition}'})
        for item in manifest['tracks']:
            self.assertEqual(digest(directory / 'MIDI' / (item['name'] + '.MID')), item['sha256'])
        expected = os.environ.get('RADWOLF_EXPECTED_MIDI_MANIFEST')
        if expected and self.edition == 'WL6':
            for item in json.loads(Path(expected).read_text()):
                self.assertEqual(digest(directory / 'MIDI' / (item['song'] + '.MID')), item['sha256'])

    def test_missing_files_fail_without_outputs(self):
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Place all 8 WL6', result.stdout)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_incomplete_game_names_missing_files(self):
        shutil.copy2(self.data / self.files[0], self.kit / self.files[0])
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(f'AUDIOT.{self.edition}', result.stdout)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_corrupt_data_preserves_existing_midi(self):
        self.copy_game()
        (self.kit / f'AUDIOHED.{self.edition}').write_bytes(b'broken')
        (self.kit / 'MIDI').mkdir()
        sentinel = self.kit / 'MIDI/user-file.txt'
        sentinel.write_bytes(b'keep me')
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Invalid AUDIOHED', result.stdout)
        self.assertEqual(sentinel.read_bytes(), b'keep me')

    def test_actual_batch_unicode_path_repeat_and_safe_regeneration(self):
        before = self.copy_game()
        result = self.run_batch()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.check_result(self.kit)
        times = {p.name: p.stat().st_mtime_ns for p in (self.kit / 'MIDI').iterdir()}
        result = self.run_batch()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('already matches', result.stdout)
        self.assertEqual(times, {p.name: p.stat().st_mtime_ns for p in (self.kit / 'MIDI').iterdir()})
        (self.kit / 'MIDI/CORNER.MID').write_bytes(b'damaged')
        (self.kit / 'MIDI/user-file.txt').write_bytes(b'preserve extra file')
        result = self.run_batch()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.check_result(self.kit)
        backups = list(self.kit.glob('MIDI-backup-*'))
        self.assertEqual(len(backups), 1)
        self.assertEqual((backups[0] / 'user-file.txt').read_bytes(), b'preserve extra file')
        self.assertEqual(before, {name: digest(self.kit / name) for name in before})

    def test_external_folder_argument(self):
        target = self.root / 'External game data'
        self.copy_game(target)
        result = self.run_batch(target)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.check_result(target)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_converter_failure_preserves_existing_folder(self):
        self.copy_game()
        (self.kit / 'tools/dro2midi.exe').write_bytes(b'invalid executable')
        (self.kit / 'MIDI').mkdir()
        sentinel = self.kit / 'MIDI/keep.txt'
        sentinel.write_bytes(b'keep')
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(sentinel.read_bytes(), b'keep')
        self.assertEqual(list(self.kit.glob('.radwolf-midi-*')), [])


class TestWindowsMidiShareware(TestWindowsMidi):
    edition = 'WL1'
    count = 11

    def test_mixed_editions_require_selection(self):
        self.copy_game()
        (self.kit / 'AUDIOHED.WL6').write_bytes(b'other edition')
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('-Edition WL1', result.stdout)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_bad_empty_slot_preserves_existing_midi(self):
        self.copy_game()
        audio = self.kit / 'AUDIOT.WL1'
        payload = audio.read_bytes()
        # DUNGEON is an empty named metadata record in shareware 1.4.
        self.assertIn(b'DUNGEON\0', payload)
        audio.write_bytes(payload.replace(b'DUNGEON\0', b'INVALID\0', 1))
        (self.kit / 'MIDI').mkdir()
        sentinel = self.kit / 'MIDI/keep.txt'
        sentinel.write_bytes(b'keep')
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Invalid WL1 empty music slot', result.stdout)
        self.assertEqual(sentinel.read_bytes(), b'keep')

    def test_explicit_selection_switches_editions_with_backup(self):
        full = os.environ.get('RADWOLF_WL6_DATA')
        if not full:
            self.skipTest('Requires both editions')
        self.copy_game()
        for name in FILES:
            shutil.copy2(Path(full) / name, self.kit / name)

        def select(edition):
            return subprocess.run(
                ['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                 '-File', 'Prepare-MIDI.ps1', '-Edition', edition], cwd=self.kit,
                capture_output=True, text=True, encoding='utf-8', errors='replace',
                timeout=120)

        result = select('WL6')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(len(list((self.kit / 'MIDI').glob('*.MID'))), 27)
        result = select('WL1')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.check_result(self.kit)
        self.assertFalse((self.kit / 'MIDI/DUNGEON.MID').exists())
        backups = list(self.kit.glob('MIDI-backup-*'))
        self.assertEqual(len(backups), 1)
        self.assertEqual(len(list(backups[0].glob('*.MID'))), 27)
        result = select('WL1')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('already matches', result.stdout)


if __name__ == '__main__':
    unittest.main()
