# Compiles every script in tests/scripts and asserts the resulting ZASM is as expected.
#
# To update the ZASM snapshots:
#
#   python tests/test_zscript.py --update
#
# or:
#
#   python tests/update_snapshots.py
#
# To add a new script to the tests, simply add it to tests/scripts.

import argparse
import json
import os
import platform
import subprocess
import sys
import unittest

from pathlib import Path

from common import ZCTestCase, parse_json

parser = argparse.ArgumentParser()
parser.add_argument(
    '--update', action='store_true', default=False, help='Update ZASM snapshots'
)
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_scripts_dir = root_dir / 'tests/scripts'
expected_dir = test_scripts_dir

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestZScript(ZCTestCase):
    def setUp(self):
        self.maxDiff = None

    def test_zscript_vscode_extension(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            return

        exe_name = 'npm.cmd' if platform.system() == 'Windows' else 'npm'
        subprocess.check_call(
            [exe_name, 'install'],
            cwd=root_dir / 'vscode-extension',
        )
        subprocess.check_call(
            [exe_name, 'run', 'compile'],
            cwd=root_dir / 'vscode-extension',
        )
        subprocess.check_call(
            [exe_name, 'run', 'test'],
            cwd=root_dir / 'vscode-extension',
            env={
                **os.environ,
                'ZC_DISABLE_DEBUG': '1',
                'BUILD_FOLDER': str(run_target.get_build_folder()),
            },
        )

    def compile_script(self, script_path):
        # Change include paths to use resources/ directly, instead of possibly-stale stuff inside a build folder.
        include_paths = [
            str(test_scripts_dir),
            str(root_dir / 'resources/include'),
            str(root_dir / 'resources/headers'),
            str(test_scripts_dir / 'playground'),
        ]
        zasm_path = run_target.get_build_folder() / 'out.zasm'
        zasm_path.unlink(missing_ok=True)
        args = [
            '-input',
            script_path,
            '-zasm',
            'out.zasm',
            '-commented',
            '-include',
            ';'.join(include_paths),
            '-unlinked',
            '-delay_cassert',
            '-json',
            '-metadata',
        ]
        p = run_target.run(
            'zscript',
            args,
            env={**os.environ, 'TEST_ZSCRIPT': '1', 'ZC_DISABLE_DEBUG': '1'},
        )
        stderr = p.stderr.replace(str(script_path), script_path.name).strip()
        stdout = p.stdout.replace(str(script_path), script_path.name)

        # Run the compiler '-metadata' for every script for coverage, but only keep
        # the full results in the stdout for this one script.
        elide_metadata = script_path.name != 'metadata.zs'

        def recursive_len(l):
            if not l:
                return 0
            else:
                c = len(l)
                for child in l:
                    c += recursive_len(child['children'])
                return c

        data = parse_json(stdout, context=f'stderr:\n\n{stderr}')
        metadata = data.get('metadata')
        if metadata:
            if elide_metadata:
                metadata['currentFileSymbols'] = recursive_len(
                    metadata['currentFileSymbols']
                )
                metadata['symbols'] = len(metadata['symbols'])
                metadata['identifiers'] = len(metadata['identifiers'])
                metadata['elided'] = True
            else:
                for symbol in metadata['symbols'].values():
                    symbol['loc']['uri'] = 'URI'
            stdout = json.dumps(data, indent=2)

        if p.returncode:
            return f'stderr:\n\n{stderr}\n\nstdout:\n\n{stdout}\n'

        zasm = zasm_path.read_text()

        # Remove the metadata (ex: `#ZASM_VERSION`) from ZASM.
        # This metadata is unrelated to the -metadata switch.
        zasm = '\n'.join(
            [l.strip() for l in zasm.splitlines() if not l.startswith('#')]
        ).strip()

        return f'stderr:\n\n{stderr}\n\nstdout:\n\n{stdout}\n\nzasm:\n\n{zasm}\n'

    def test_zscript_compiler_expected_zasm(self):
        script_paths = list(test_scripts_dir.rglob('*.zs'))
        script_paths += list((test_scripts_dir / 'newbie_boss').rglob('*.z'))
        for script_path in script_paths:
            if script_path.name in ['auto.zs', 'playground.zs', 'z3.zs']:
                continue

            with self.subTest(msg=f'compile {script_path.name}'):
                output = self.compile_script(script_path)
                script_subpath = script_path.relative_to(test_scripts_dir)
                expected_path = expected_dir / script_subpath.with_name(
                    f'{script_subpath.stem}_expected.txt'
                )
                self.expect_snapshot(expected_path, output, args.update)


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
