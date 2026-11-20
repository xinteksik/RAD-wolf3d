"""Validate menu extraction with synthetic chunks, never commercial data."""
import importlib.util
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'tools/convert_menu_midi.py'
SPEC = importlib.util.spec_from_file_location('convert_menu_midi', SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class TestMenuExtraction(unittest.TestCase):
    def extract(self, chunk, start=0, end=None):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            offsets = [0] * 28
            offsets[14] = start
            offsets[15] = len(chunk) if end is None else end
            (root / 'header').write_bytes(struct.pack('<28I', *offsets))
            (root / 'audio').write_bytes(chunk)
            return MODULE.extract_menu(root / 'header', root / 'audio')

    def test_extracts_only_menu_imf_without_metadata(self):
        imf = b'\x04\x00\x20\x01\x01\x00'
        self.assertEqual(self.extract(imf + b'\x01\x00WONDERIN\0'), imf)

    def test_rejects_wrong_song(self):
        with self.assertRaises(ValueError):
            self.extract(b'\x04\x00\x20\x01\x01\x00OTHERSONG')

    def test_rejects_out_of_bounds_offsets(self):
        with self.assertRaises(ValueError):
            self.extract(b'test', end=100)

    def test_rejects_truncated_imf(self):
        with self.assertRaises(ValueError):
            self.extract(b'\xff\xffWONDERIN')

    def test_all_music_indexes_and_names(self):
        names = ('CORNER', 'DUNGEON', 'WARMARCH', 'GETTHEM', 'HEADACHE', 'HITLWLTZ',
                 'INTROCW3', 'NAZI_NOR', 'NAZI_OMI', 'POW', 'SALUTE', 'SEARCHN',
                 'SUSPENSE', 'VICTORS', 'WONDERIN', 'FUNKYOU', 'ENDLEVEL', 'GOINGAFT',
                 'PREGNANT', 'ULTIMATE', 'NAZI_RAP', 'ZEROHOUR', 'TWELFTH', 'ROSTER',
                 'URAHERO', 'VICMARCH', 'PACMAN')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            offsets, payload = [], b''
            for name in names:
                offsets.append(len(payload))
                payload += b'\x04\x00\x20\x01\x01\x00' + name.encode() + b'\0'
            offsets.append(len(payload))
            (root / 'header').write_bytes(struct.pack('<28I', *offsets))
            (root / 'audio').write_bytes(payload)
            for index, name in enumerate(names):
                self.assertEqual(MODULE.extract_song(root / 'header', root / 'audio',
                                                     index, name),
                                 b'\x04\x00\x20\x01\x01\x00')

    def test_shareware_extracts_11_songs_and_checks_empty_slots(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            offsets, payload = [], b''
            imf = b'\x04\x00\x20\x01\x01\x00'
            for name in MODULE.SONGS:
                offsets.append(len(payload))
                if name in MODULE.SHAREWARE_SONGS:
                    payload += imf + b'\x01\x00' + name.encode() + b'\0'
                else:
                    payload += (b'\0\0' + name.encode() + b'\0').ljust(88, b'\0')
            offsets.append(len(payload))
            header, audio = root / 'AUDIOHED.WL1', root / 'AUDIOT.WL1'
            header.write_bytes(struct.pack('<28I', *offsets))
            audio.write_bytes(payload)
            found = []
            for index, name in enumerate(MODULE.SONGS):
                song = MODULE.extract_song(header, audio, index, name)
                if song is not None:
                    self.assertEqual(song, imf)
                    found.append(name)
            self.assertEqual(tuple(found), MODULE.SHAREWARE_SONGS)
            self.assertEqual(MODULE.extract_menu(header, audio), imf)
            damaged = bytearray(payload)
            damaged[offsets[1] + 2] = ord('X')
            audio.write_bytes(damaged)
            with self.assertRaisesRegex(ValueError, 'empty music slot'):
                MODULE.extract_song(header, audio, 1, 'DUNGEON')
            audio.write_bytes(payload)
            with self.assertRaises(ValueError):
                MODULE.extract_song(header, audio, 1, 'DUNGEON', 'WL6')
            # An expected shareware song cannot silently disappear.
            audio.write_bytes(b'\0\0' + payload[2:])
            with self.assertRaises(ValueError):
                MODULE.extract_song(header, audio, 0, 'CORNER')
            audio.write_bytes(payload)
            # The Python CLI must not leave full-version tracks in a WL1 set.
            midi = root / 'output/MIDI'
            midi.mkdir(parents=True)
            sentinel = midi / 'DUNGEON.MID'
            sentinel.write_bytes(b'keep previous output')
            result = subprocess.run(
                [sys.executable, str(SCRIPT.with_name('convert_all_midi.py')),
                 str(root), str(root / 'missing-converter'), str(midi.parent)],
                capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('separate output directory', result.stderr)
            self.assertEqual(sentinel.read_bytes(), b'keep previous output')

    def test_detects_case_insensitive_audio_and_requires_edition_for_mixed_sets(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'audiohed.wl1').write_bytes(b'header')
            with self.assertRaisesRegex(ValueError, 'AUDIOT.WL1'):
                MODULE.find_audio(root)
            (root / 'audiot.wl1').write_bytes(b'audio')
            self.assertEqual(MODULE.find_audio(root)[0], 'WL1')
            for name in ('AUDIOHED.WL6', 'AUDIOT.WL6'):
                (root / name).write_bytes(b'full')
            with self.assertRaisesRegex(ValueError, '--edition'):
                MODULE.find_audio(root)
            self.assertEqual(MODULE.find_audio(root, 'WL1')[0], 'WL1')
            self.assertEqual(MODULE.find_audio(root, 'WL6')[0], 'WL6')


if __name__ == '__main__':
    unittest.main()
