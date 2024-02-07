# Optimizes ZASM and asserts the resulting code is as expected in snapshots.
#
# To update the snapshots:
#
#   python tests/test_optimize_zasm.py --update
#
# or:
#
#   python tests/update_snapshots.py

import argparse
import os
import sys
import unittest
from pathlib import Path
from common import ZCTestCase

parser = argparse.ArgumentParser()
parser.add_argument('--update', action='store_true', default=False,
                    help='Update snapshots')
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_dir = root_dir / 'tests'
expected_dir = test_dir / 'snapshots/optimize_zasm'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestOptimizeZasm(ZCTestCase):
    def setUp(self):
        self.maxDiff = None

    def optimize_zasm_in_qst(self, qst_path: Path):
        args = [
            '-headless',
            '-extract-zasm', qst_path,
            '-optimize-zasm',
        ]
        p = run_target.run('zplayer', args)
        if p.returncode:
            raise Exception(f'error: {p.returncode}\n{p.stderr}')

        return p.stdout

    def run_for_qst(self, qst_path: Path):
        with self.subTest(msg=f'optimizing {qst_path.name}'):
            stdout = self.optimize_zasm_in_qst(qst_path)
            filtered_lines = []
            for line in stdout.splitlines():
                if filtered_lines:
                    filtered_lines.append(line.split(',')[0])
                    if '[total]' in line:
                        break

                if line.startswith('Finished optimizing scripts'):
                    filtered_lines.append(line)

            if not filtered_lines:
                if 'No scripts found' not in stdout:
                    raise Exception(f'Error optimizing scripts:\n{stdout}')
                return

            output = '\n'.join(filtered_lines) + '\n'
            expected_path = expected_dir / f'{qst_path.stem}.txt'
            self.expect_snapshot(expected_path, output, args.update)

    def test_optimize_zasm(self):
        for qst_path in test_dir.rglob('*.qst'):
            self.run_for_qst(qst_path)


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
