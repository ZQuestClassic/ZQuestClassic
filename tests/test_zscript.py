# Compiles every script in tests/scripts and asserts the resulting ZASM is as expected.
#
# To update the ZASM snapshots:
#
#   python tests/test_zscript.py --update
#
# To add a new script to the tests, simply add it to tests/scripts.

import argparse
import os
import sys
import unittest
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--update', action='store_true', default=False,
                    help='Update ZASM snapshots')
parser.add_argument('--output', action='store_true', default=False,
                    help='Output diff logs to a subfolder')
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_scripts_dir = root_dir / 'tests/scripts'
expected_dir = test_scripts_dir
if args.output or args.update: #Clear old output
    for path in expected_dir.rglob('*_unexpected.txt'):
        os.remove(path)

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestZScript(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def compile_script(self, script_path):
        # Change include paths to use resources/ directly, instead of possibly-stale stuff inside a build folder.
        include_paths = [
            str(test_scripts_dir),
            str(root_dir / 'resources/include'),
            str(root_dir / 'resources/headers'),
        ]
        zasm_path = run_target.get_build_folder() / 'out.zasm'
        zasm_path.unlink(missing_ok=True)
        args = [
            '-input', script_path,
            '-zasm', 'out.zasm',
            '-include', ';'.join(include_paths),
            '-unlinked',
            '-delay_cassert',
        ]
        p = run_target.run('zscript', args)
        stdout = p.stdout.replace(str(script_path), script_path.name)
        if p.returncode:
            return stdout

        zasm = zasm_path.read_text()

        # Remove metadata.
        zasm = '\n'.join([l.strip() for l in zasm.splitlines()
                         if not l.startswith('#')]).strip()

        return '\n'.join([stdout, zasm])

    def test_zscript_compiler_expected_zasm(self):
        for script_path in test_scripts_dir.rglob('*.zs'):
            with self.subTest(msg=f'compile {script_path.name}'):
                zasm = self.compile_script(script_path)
                
                script_subpath = script_path.relative_to(test_scripts_dir)
                expected_path = expected_dir / script_subpath.with_name(f'{script_subpath.stem}_expected.txt')
                unexpected_path = expected_dir / script_subpath.with_name(f'{script_subpath.stem}_unexpected.txt')

                expected_zasm = None
                if expected_path.exists():
                    expected_zasm = expected_path.read_text()

                if args.update:
                    if expected_zasm != zasm:
                        print(f'updating zasm snapshot {expected_path.name}')
                        expected_path.parent.mkdir(parents=True, exist_ok=True)
                        expected_path.write_text(zasm)
                else:
                    if expected_zasm == None:
                        expected_path.parent.mkdir(parents=True, exist_ok=True)
                        expected_path.write_text(zasm)
                    else:
                        if expected_zasm != zasm and args.output:
                            unexpected_path.parent.mkdir(parents=True, exist_ok=True)
                            unexpected_path.write_text(zasm)
                        self.assertEqual(expected_zasm, zasm)


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
