# Tests ZQuest.
#
# To run:
#
#   python tests/test_zquest.py

import sys
import os
import json
import subprocess
import unittest
from pathlib import Path
from common import ReplayTestResults

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent


class TestReplays(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def get_build_folder(self):
        build_folder = root_dir / 'build/Release'
        if 'BUILD_FOLDER' in os.environ:
            build_folder = Path(os.environ['BUILD_FOLDER']).absolute()
        return build_folder

    def quick_assign(self, qst_path):
        build_folder = self.get_build_folder()
        exe_name = 'zquest.exe' if os.name == 'nt' else 'zquest'
        args = [
            build_folder / exe_name,
            qst_path,
            '-quick-assign',
        ]
        output = subprocess.run(args, cwd=build_folder, encoding='utf-8',
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if output.returncode != 0:
            raise Exception(f'got error: {output.returncode}\n{output.stdout}')

    def run_replay(self, output_dir, args):
        args = [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--test_results', output_dir,
            *args,
        ]
        if 'BUILD_FOLDER' in os.environ:
            args.append('--build_folder')
            args.append(os.environ['BUILD_FOLDER'])
        output = subprocess.run(args, stdout=subprocess.DEVNULL)

        test_results_path = output_dir / 'test_results.json'
        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        return output.returncode, ReplayTestResults(**test_results_json)

    def test_zquest_compile_and_quick_assign(self):
        # TODO: set this via CLI
        include_paths = [
            str(root_dir / 'tests/scripts'),
            str(root_dir / 'resources/include'),
            str(root_dir / 'resources/headers'),
        ]
        (self.get_build_folder() / 'includepaths.txt').write_text(';'.join(include_paths))

        # Re-compile and assign slots.
        self.quick_assign(root_dir / 'tests/replays/playground.qst')

        # Ensure replays continue to pass.
        for replay_path in (root_dir / 'tests/replays').glob('playground_*.zplay'):
            with self.subTest(msg=f'{replay_path.name}'):
                output_dir = root_dir / '.tmp/test_zquest/output' / replay_path.name
                output_dir.mkdir(exist_ok=True, parents=True)
                status, results = self.run_replay(
                    output_dir, ['--filter', replay_path.name])
                run = results.runs[0][0]
                if not run.success:
                    failing_str = f'failure at frame {run.failing_frame}'
                    if run.unexpected_gfx_segments:
                        segments_str = [
                            f'{r[0]}-{r[1]}' for r in run.unexpected_gfx_segments]
                        failing_str += ': ' + ', '.join(segments_str)
                    self.fail(failing_str)
                self.assertEqual(status, 0)


if __name__ == '__main__':
    unittest.main()
