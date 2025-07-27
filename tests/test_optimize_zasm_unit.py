# Same as test_optimize_zasm.py, but this optimizes ZASM found in tests/snapshots/optimize_zasm_unit
# which have been hand crafted as unit tests.
#
# To update the snapshots:
#
#   python tests/test_optimize_zasm_unit.py --update
#
# or:
#
#   python tests/update_snapshots.py
#
# To add a new test, create a file in tests/optimize_zasm and paste some ZASM in it. You need to manually
# adjust GOTO jumps, but other than that, just paste it, run the test, and the input will be normalized for you.

import argparse
import os
import sys
import unittest

from pathlib import Path

from common import ZCTestCase

parser = argparse.ArgumentParser()
parser.add_argument(
    '--update', action='store_true', default=False, help='Update snapshots'
)
parser.add_argument(
    '--print',
    action='store_true',
    default=False,
    help='Print all the input/expected ZASM',
)
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_dir = root_dir / 'tests'
inputs_dir = test_dir / 'optimize_zasm'
expected_dir = test_dir / 'snapshots/optimize_zasm_unit'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestOptimizeZasmUnit(ZCTestCase):
    def setUp(self):
        self.maxDiff = None

    def run_test(self, path: Path):
        run_args = [
            '-headless',
            '-optimize-zasm-experimental',
            '-optimize-zasm-no-parallel',
            '-test-optimize-zasm',
            str(path),
            '-no_console',
        ]
        p = run_target.run('zplayer', run_args)
        if p.returncode:
            raise Exception(f'error: {p.returncode}\n\nSTDERR:\n\n{p.stderr}\n\nSTDOUT:\n\n{p.stdout}')

        output_index = p.stdout.index('output:\n') + len('output:\n')
        output = p.stdout[output_index:].strip() + '\n'
        expected_path = expected_dir / f'{path.stem}_expected.txt'
        self.expect_snapshot(expected_path, output, args.update)

    def test_optimize_zasm(self):
        for path in inputs_dir.rglob('*.txt'):
            with self.subTest(msg=f'optimizing {path.name}'):
                self.run_test(inputs_dir / path)


if __name__ == '__main__':
    if args.print:
        for path in inputs_dir.rglob('*.txt'):
            expected_path = expected_dir / f'{path.stem}_expected.txt'
            print(f'= {path.name}\n')
            print(path.read_text())
            print(f'\n= expected\n')
            print(expected_path.read_text())
            print()

    sys.argv[1:] = args.unittest_args
    unittest.main()
