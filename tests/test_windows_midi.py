"""Opt-in Windows end-to-end tests with private, legitimately obtained WL6 data."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ENABLED = (os.name == 'nt' and os.environ.get('RADWOLF_WINDOWS_MIDI_KIT')
           and os.environ.get('RADWOLF_WL6_DATA'))
FILES = ('AUDIOHED.WL6', 'AUDIOT.WL6', 'GAMEMAPS.WL6', 'MAPHEAD.WL6',
         'VGADICT.WL6', 'VGAGRAPH.WL6', 'VGAHEAD.WL6', 'VSWAP.WL6')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


@unittest.skipUnless(ENABLED, 'Requires Windows, built kit and private WL6 test data')
class TestWindowsMidi(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(dir=os.environ.get('RADWOLF_TEST_DIRECTORY'))
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'SD card & test - Žluťoučký'
        self.kit = self.root / 'RADWOLF'
        shutil.copytree(os.environ['RADWOLF_WINDOWS_MIDI_KIT'], self.kit)
        self.data = Path(os.environ['RADWOLF_WL6_DATA'])

    def copy_game(self, directory=None):
        target = directory or self.kit
        target.mkdir(parents=True, exist_ok=True)
        for name in FILES:
            shutil.copy2(self.data / name, target / name)
        (target / 'soundfont.sf2').write_bytes(b'private font sentinel')
        (target / 'savegam0.wl6').write_bytes(b'private save sentinel')
        return {name: digest(target / name) for name in (*FILES, 'soundfont.sf2', 'savegam0.wl6')}

    def run_batch(self, argument=None):
        env = dict(os.environ, RADWOLF_NO_PAUSE='1')
        args = ['cmd.exe', '/d', '/c', 'Prepare-MIDI.bat']
        if argument is not None:
            args.append(str(argument))
        return subprocess.run(args, cwd=self.kit, env=env, capture_output=True,
                              text=True, encoding='utf-8', errors='replace', timeout=120)

    def check_result(self, directory):
        tracks = sorted((directory / 'MIDI').glob('*.MID'))
        self.assertEqual(len(tracks), 27)
        manifest = json.loads((directory / 'MIDI/MIDI-INFO.json').read_text(encoding='utf-8-sig'))
        self.assertEqual(len(manifest['tracks']), 27)
        for item in manifest['tracks']:
            self.assertEqual(digest(directory / 'MIDI' / (item['name'] + '.MID')), item['sha256'])
        expected = os.environ.get('RADWOLF_EXPECTED_MIDI_MANIFEST')
        if expected:
            for item in json.loads(Path(expected).read_text()):
                self.assertEqual(digest(directory / 'MIDI' / (item['song'] + '.MID')), item['sha256'])

    def test_missing_files_fail_without_outputs(self):
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Place all 8 WL6', result.stdout)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_incomplete_game_names_missing_files(self):
        shutil.copy2(self.data / FILES[0], self.kit / FILES[0])
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('AUDIOT.WL6', result.stdout)
        self.assertFalse((self.kit / 'MIDI').exists())

    def test_corrupt_data_preserves_existing_midi(self):
        self.copy_game()
        (self.kit / 'AUDIOHED.WL6').write_bytes(b'broken')
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


if __name__ == '__main__':
    unittest.main()
