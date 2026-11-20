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

verify_spec = importlib.util.spec_from_file_location(
    'verify_game_data', script.with_name('verify_game_data.py'))
verify = importlib.util.module_from_spec(verify_spec)
verify_spec.loader.exec_module(verify)


class TestGameDataSelection(unittest.TestCase):
    def test_selects_complete_editions_and_ignores_old_saves(self):
        names = {f'{base}.WL1' for base in verify.BASES}
        names.update(('CONFIG.WL6', 'SAVEGAM0.WL6'))
        self.assertEqual(verify.select_edition(names, 'auto'), 'WL1')
        names.update(f'{base}.WL6' for base in verify.BASES)
        self.assertEqual(verify.select_edition(names, 'auto'), 'WL6')
        self.assertEqual(verify.select_edition(names, 'WL1'), 'WL1')
        names.remove('AUDIOT.WL6')
        self.assertEqual(verify.select_edition(names, 'auto'), 'WL1')

    def test_incomplete_set_keeps_its_extension_for_missing_file_report(self):
        self.assertEqual(verify.select_edition({'AUDIOHED.WL1'}, 'auto'), 'WL1')
        self.assertEqual(verify.select_edition({'AUDIOHED.WL6'}, 'auto'), 'WL6')
        self.assertEqual(verify.select_edition(set(), 'auto'), 'WL6')


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
