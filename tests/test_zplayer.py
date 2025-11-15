# Tests ZPlayer.
#
# To run:
#
#   python tests/test_zplayer.py

import os
import platform
import sys
import unittest

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

CI = 'CI' in os.environ
is_mac = platform.system() == 'Darwin'


class TestZPlayer(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def test_zplayer(self):
        run_target.check_run(
            'zplayer',
            ['-test-zc'],
        )

    def test_base_test_runner(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('skipping test because emscripten')
        if is_mac and CI:
            raise unittest.SkipTest(
                'skipping test because CI does not currently package test runner'
            )

        run_target.check_run('base_test_runner', [])


if __name__ == '__main__':
    unittest.main()
