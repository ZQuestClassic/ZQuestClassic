# Tests ZPlayer.
#
# To run:
#
#   python tests/test_zplayer.py

import os
import sys
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
            ['-test-zc'],
        )


if __name__ == '__main__':
    unittest.main()
