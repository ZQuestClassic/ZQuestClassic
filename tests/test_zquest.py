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

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

class TestReplays(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def quick_assign(self, qst_path):
        args = [
            '-quick-assign',
            qst_path,
        ]
        run_target.check_run('zquest', args)

    def run_replay(self, output_dir, args):
        args = [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--build_folder', run_target.get_build_folder(),
            '--test_results', output_dir,
            *args,
        ]
        output = subprocess.run(args, stdout=subprocess.PIPE, encoding='utf-8')
        test_results_path = output_dir / 'test_results.json'
        if not test_results_path.exists():
            print(output.stdout)
            raise Exception('could not find test_results.json')

        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        return output.returncode, ReplayTestResults(**test_results_json)

    def test_zquest_compile_and_quick_assign(self):
        return #TEMPORARY newsubscr2 BRANCH
        # TODO: set this via CLI
        include_paths = [
            str(root_dir / 'tests/scripts'),
            str(root_dir / 'resources/include'),
            str(root_dir / 'resources/headers'),
        ]
        (run_target.get_build_folder() / 'includepaths.txt').write_text(';'.join(include_paths))

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
