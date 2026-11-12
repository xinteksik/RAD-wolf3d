"""Host tests for the current source installer, not historical bring-up patches."""
from pathlib import Path
import hashlib
import importlib.util
import tempfile
import unittest

script = Path(__file__).resolve().parents[1] / 'tools/prepare_sources.py'
spec = importlib.util.spec_from_file_location('prepare_sources', script)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class TestSourceInstaller(unittest.TestCase):
    def test_install_repeat_and_refuse_local_edits(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            repo = root / 'repo'; repo.mkdir()
            (repo / 'file.c').write_bytes(b'base\n')
            (root / 'override.c').write_bytes(b'changed\n')
            entry = dict(repository='test', file='file.c', override='override.c',
                         base_sha256=hashlib.sha256(b'base\n').hexdigest(),
                         sha256=hashlib.sha256(b'changed\n').hexdigest())
            self.assertEqual(module.install_overrides(root, {'test': repo}, [entry]), 1)
            self.assertEqual(module.install_overrides(root, {'test': repo}, [entry]), 0)
            (repo / 'file.c').write_bytes(b'user edit\n')
            with self.assertRaises(ValueError):
                module.install_overrides(root, {'test': repo}, [entry])
            self.assertEqual((repo / 'file.c').read_bytes(), b'user edit\n')

    def test_corrupt_override_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'file.c').write_bytes(b'base')
            (root / 'override.c').write_bytes(b'wrong')
            with self.assertRaises(ValueError):
                module.install_overrides(root, {'test': root}, [dict(repository='test',
                    file='file.c', override='override.c', base_sha256='0', sha256='0')])
