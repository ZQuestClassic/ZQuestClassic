# Tests ZPlayer.
#
# To run:
#
#   python tests/test_zplayer.py

import os
import shutil
import sys
import tempfile
import unittest

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestZPlayer(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def test_zplayer(self):
        run_target.check_run(
            'zplayer',
            [
                '-test-zc',
            ],
        )

    def test_non_ascii_quest_path(self):
        # On Windows, narrow-char APIs (argv, fopen, ...) read paths in the process code
        # page, which src/windows.manifest sets to UTF-8. Without it these fail to load:
        # 'クエスト' isn't in any Western code page.
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('skipping test because emscripten')

        with tempfile.TemporaryDirectory() as tmp:
            for folder in ['Créateur', 'クエスト']:
                with self.subTest(msg=folder):
                    qst = Path(tmp) / folder / 'playground.qst'
                    qst.parent.mkdir()
                    shutil.copy(root_dir / 'tests/replays/playground/playground.qst', qst)
                    run_target.check_run('zplayer', ['-headless', '-load-and-quit', str(qst)])


if __name__ == '__main__':
    unittest.main()
