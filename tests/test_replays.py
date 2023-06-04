import json
import os
import shutil
import subprocess
import sys
import unittest
from pathlib import Path
from common import ReplayTestResults

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_dir = root_dir / '.tmp/test_replays'
failing_replay = test_dir / 'failing.zplay'


class TestReplays(unittest.TestCase):

    def setUp(self) -> None:
        self.maxDiff = None
        if test_dir.exists():
            shutil.rmtree(test_dir)
        test_dir.mkdir(parents=True)
        failing_replay_contents = (
            root_dir / 'tests/replays/classic_1st_lvl1.zplay').read_text()
        failing_replay_contents = failing_replay_contents.replace(
            'C 549 g H!V', 'C 549 g blah')
        failing_replay.write_text(failing_replay_contents)

    def test_failing_replay(self):
        output_dir = test_dir / 'output'
        args = [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--test_results', output_dir,
        ]
        if 'BUILD_FOLDER' in os.environ:
            args.append('--build_folder')
            args.append(os.environ['BUILD_FOLDER'])
        args.append(failing_replay)
        output = subprocess.run(args, stdout=subprocess.DEVNULL)
        self.assertEqual(output.returncode, 1)

        test_results_path = test_dir / 'output/test_results.json'
        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        test_results = ReplayTestResults(**test_results_json)

        result = test_results.runs[0][0]
        snapshots = (s.relative_to(output_dir).as_posix()
                     for s in output_dir.rglob('*.png'))
        self.assertEqual(result.name, 'failing.zplay')
        self.assertEqual(result.success, False)
        self.assertEqual(result.failing_frame, 549)
        self.assertEqual(sorted(snapshots), [
            '0/failing/failing.zplay.539.png',
            '0/failing/failing.zplay.540.png',
            '0/failing/failing.zplay.541.png',
            '0/failing/failing.zplay.542.png',
            '0/failing/failing.zplay.543.png',
            '0/failing/failing.zplay.544.png',
            '0/failing/failing.zplay.545.png',
            '0/failing/failing.zplay.546.png',
            '0/failing/failing.zplay.547.png',
            '0/failing/failing.zplay.548.png',
            '0/failing/failing.zplay.549-unexpected.png',
            '0/failing/failing.zplay.550.png',
            '0/failing/failing.zplay.551.png',
            '0/failing/failing.zplay.552.png',
            '0/failing/failing.zplay.553.png',
            '0/failing/failing.zplay.554.png',
            '0/failing/failing.zplay.555.png',
            '0/failing/failing.zplay.556.png',
            '0/failing/failing.zplay.557.png',
            '0/failing/failing.zplay.558.png',
        ])


if __name__ == '__main__':
    unittest.main()
