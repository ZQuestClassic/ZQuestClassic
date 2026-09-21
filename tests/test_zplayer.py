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
            ['-test-zc', str(root_dir / 'tests')],
        )

    def test_compat_rules_survive_junk_rule_wipe(self):
        # readrules zeroes a large range of the rule array for quests saved
        # before compatrule_version 27, so a compat rule set for old quests must
        # be applied after that wipe or it silently never takes effect. Load real
        # pre-2.55 quests and check that such rules are actually on.
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('skipping test because emscripten')
        if platform.system() == 'Windows':
            raise unittest.SkipTest('zplayer does not print to stdout on Windows')

        cases = [
            # 2.50.1 (build 28)
            ('new2013/new2013.qst', ['qr_TURN_WHILE_CHARGING_HAMMER 1']),
            # 2.10
            ('hero_of_dreams/hero_of_dreams.qst', ['qr_OLD_210_HAMMER_POUND_REACH 1']),
        ]
        for qst, expected_lines in cases:
            with self.subTest(msg=qst):
                p = run_target.check_run(
                    'zplayer',
                    ['-headless', '-dump-qrs', str(root_dir / 'tests/replays' / qst)],
                )
                lines = p.stdout.splitlines()
                for expected in expected_lines:
                    self.assertIn(expected, lines)

    def test_base_test_runner(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('skipping test because emscripten')

        run_target.check_run('base_test_runner', [])


if __name__ == '__main__':
    unittest.main()
