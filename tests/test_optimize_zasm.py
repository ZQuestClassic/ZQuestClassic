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

from common import ZCTestCase, submit_all

parser = argparse.ArgumentParser()
parser.add_argument(
    '--update', action='store_true', default=False, help='Update snapshots'
)
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_dir = root_dir / 'tests'
expected_dir = test_dir / 'snapshots/optimize_zasm'
tmp_dir = root_dir / '.tmp/test_optimize_zasm'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestOptimizeZasm(ZCTestCase):
    def setUp(self):
        self.maxDiff = None
        tmp_dir.mkdir(exist_ok=True, parents=True)

    def optimize_zasm_in_qst(self, qst_path: Path):
        # Use a log path unique to this qst so parallel runs don't clobber each other.
        slug = qst_path.relative_to(test_dir).as_posix().replace('/', '_')
        allegro_log_path = tmp_dir / f'{slug}.allegro.log'
        if allegro_log_path.exists():
            allegro_log_path.unlink()

        run_args = [
            '-headless',
            '-load-and-quit',
            qst_path,
            '-optimize-zasm',
            '-optimize-zasm-experimental',
            '-no_console',
        ]
        p = run_target.run(
            'zplayer',
            run_args,
            env={
                **os.environ,
                'ALLEGRO_LEGACY_TRACE': str(allegro_log_path),
            },
        )
        if p.returncode:
            raise Exception(
                f'error: {p.returncode}\n\nSTDERR:\n\n{p.stderr}\n\nSTDOUT:\n\n{p.stdout}'
            )

        return allegro_log_path.read_text()

    def run_for_qst(self, qst_path: Path, future):
        with self.subTest(msg=f'optimizing {qst_path.name}'):
            output = future.result()
            filtered_lines = []
            for line in output.splitlines():
                if filtered_lines:
                    filtered_lines.append(line.split(',')[0])
                    if '[total]' in line:
                        break

                if line.startswith('[optimizer] Finished optimizing scripts'):
                    filtered_lines.append(line)

            if not filtered_lines:
                if '[zasm-pipeline] no scripts' not in output:
                    raise Exception(f'Error optimizing scripts:\n{output}')
                return

            output = '\n'.join(filtered_lines) + '\n'
            output = output.replace('[optimizer] ', '')
            expected_path = expected_dir / f'{qst_path.stem}.txt'
            self.expect_snapshot(expected_path, output, args.update)

    def test_optimize_zasm(self):
        qst_paths = [
            qst_path
            for qst_path in test_dir.rglob('*.qst')
            if '.backups' not in str(qst_path) and 'backups.' not in str(qst_path)
        ]
        executor, futures = submit_all(self.optimize_zasm_in_qst, qst_paths)
        with executor:
            for qst_path in qst_paths:
                self.run_for_qst(qst_path, futures[qst_path])


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
