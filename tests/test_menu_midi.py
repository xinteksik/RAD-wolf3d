"""Validate menu extraction with synthetic chunks, never commercial data."""
import importlib.util
from pathlib import Path
import struct
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


if __name__ == '__main__':
    unittest.main()
