# Compiles ZASM scripts with the JIT backends and asserts the resulting code is as expected.
#
# This does not test the execution of the JIT'd scripts, just prints the output.
# Execution is covered by running the replay tests (which default to using JIT).
#
# This requires git-lfs. See the comment at top of run_replay_tests.py
#
# To update the snapshots:
#
#   python tests/test_jit.py --update

import argparse
import os
import re
import platform
import shutil
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
expected_dir = test_dir / 'jit'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestJIT(ZCTestCase):
    def setUp(self):
        self.maxDiff = None

    def compile_zasm_in_qst(self, replay_path: Path):
        output_dir = root_dir / '.tmp/test_jit' / replay_path.name
        output_dir.parent.mkdir(exist_ok=True, parents=True)
        if output_dir.exists():
            shutil.rmtree(output_dir)
        args = [
            '-headless',
            '-replay', replay_path,
            '-script-runtime-debug-folder', str(output_dir),
            '-frame', '0',
            '-replay-exit-when-done',
            '-jit',
            '-jit-log',
            # TODO: why do threads make output files sometimes be empty?
            '-jit-threads', '0',
            '-jit-env-windows',
            '-jit-precompile',
            '-jit-print-asm',
        ]
        p = run_target.run('zplayer', args)
        if p.returncode:
            raise Exception(f'error: {p.returncode}\n{p.stderr}')

        return output_dir

    def run_for_qst(self, qst_name: str, replay_path: Path):
        jit_output_path = self.compile_zasm_in_qst(replay_path)
        for output_path in jit_output_path.rglob('*.txt'):
            with self.subTest(msg=f'compile {qst_name} {output_path.stem}'):
                # The first two lines have timing information, so remove them.
                # Third line is code size, but with `-jit-env-windows` that can vary by ~1kb, so remove that too.
                lines = output_path.read_text().splitlines()[3:]
                for i, line in enumerate(lines):
                    instruction = line.split(';')[0]
                    if instruction.startswith('call') or (instruction.startswith('mov r') and not 'dword' in instruction):
                        p = r'-?\d{6,}'
                        m = re.search(p, instruction)
                        if m:
                            replace_with = '<addr>'.ljust(len(m[0]), ' ')
                            lines[i] = re.sub(p, replace_with, line, 1).strip()
                output = '\n'.join(lines)

                expected_path = expected_dir / qst_name / f'{output_path.stem}.txt'
                self.expect_snapshot(expected_path, output, args.update)

    def test_jit_x64(self):
        if platform.system() == 'Windows' and platform.architecture()[0] != '64bit':
            raise unittest.SkipTest('unsupported platform')
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('unsupported platform')

        self.run_for_qst('playground', test_dir / 'replays/playground_maths.zplay')
        self.run_for_qst('freedom_in_chains', test_dir / 'replays/freedom_in_chains.zplay')
        self.run_for_qst('hollow_forest', test_dir / 'replays/hollow_forest.zplay')
        self.run_for_qst('stellar_seas_randomizer', test_dir / 'replays/stellar_seas_randomizer.zplay')


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
