"""Offline Koala decode and executable-viewer checks; no game data."""
import importlib.util
from pathlib import Path
import unittest
spec = importlib.util.spec_from_file_location('video_lab', Path(__file__).parents[1] / 'tools/video_lab.py')
lab = importlib.util.module_from_spec(spec)
spec.loader.exec_module(lab)


class VideoLabTests(unittest.TestCase):
    def test_four_pixel_choices_and_viewer_memory_layout(self):
        image = bytearray(10001)
        image[0] = 0b00011011
        image[8000] = 0x23
        image[9000] = 4
        image[10000] = 1
        payload = b'\0\x60' + image
        indices, rgb = lab.decode_koala(payload)
        self.assertEqual(indices[:8], bytes([1, 1, 2, 2, 3, 3, 4, 4]))
        self.assertEqual(len(rgb), 320 * 200 * 3)
        self.assertEqual(rgb[:6], b'\xff' * 6)
        prg = lab.viewer_prg(payload)
        self.assertEqual(prg[:2], b'\x01\x08')
        self.assertIn(b'2064', prg[:16])
        self.assertEqual(prg[2 + 0x2000 - 0x0801:2 + 0x2000 - 0x0801 + 8000], image[:8000])
        self.assertEqual(prg[2 + 0x4000 - 0x0801:2 + 0x4000 - 0x0801 + 1000], image[8000:9000])

    def test_malformed_koala_is_rejected(self):
        with self.assertRaises(ValueError):
            lab.decode_koala(b'bad')
        image = bytearray(10003)
        image[1] = 0x60
        image[-1] = 16
        with self.assertRaises(ValueError):
            lab.decode_koala(image)


if __name__ == '__main__':
    unittest.main()
