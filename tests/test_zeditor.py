# Tests the editor.
#
# To run:
#
#   python tests/test_zeditor.py

import json
import os
import platform
import shutil
import subprocess
import sys
import unittest

from pathlib import Path

from replays import ReplayTestResults

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/test_zeditor'
tmp_dir.mkdir(exist_ok=True, parents=True)

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestZEditor(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def test_zeditor(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            return

        run_target.check_run(
            'zeditor',
            ['-test-zc', root_dir / 'tests'],
        )

    def quick_assign(self, qst_path):
        args = [
            '-headless',
            '-quick-assign',
            qst_path,
        ]
        run_target.check_run('zeditor', args)

    def run_replay(self, output_dir, args):
        args = [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--build_folder',
            run_target.get_build_folder(),
            '--test_results',
            output_dir,
            '--no_console',
            *args,
        ]
        shutil.rmtree(output_dir, ignore_errors=True)
        output = subprocess.run(args, stdout=subprocess.PIPE, encoding='utf-8')
        test_results_path = output_dir / 'test_results.json'
        if not test_results_path.exists():
            print(output.stdout)
            raise Exception('could not find test_results.json')

        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        results = ReplayTestResults(**test_results_json)

        failing_strs = []
        for run in results.runs[0]:
            if not run.success:
                failing_str = f'failure at frame {run.failing_frame}'
                if run.unexpected_gfx_segments:
                    segments_str = [
                        f'{r[0]}-{r[1]}' for r in run.unexpected_gfx_segments
                    ]
                    failing_str += ': ' + ', '.join(segments_str)
                if run.exceptions:
                    failing_str += '\nExceptions:\n\t' + '\n\t'.join(run.exceptions)
                failing_strs.append(failing_str)

        failing_str = '\n'.join(failing_strs)
        if failing_str:
            self.fail(failing_str)

        self.assertEqual(output.returncode, 0)

    # Simply open the editor with the new quest template.
    def test_file_new(self):
        run_target.check_run(
            'zeditor',
            [
                '-new',
                '-headless',
                '-s',
                '-q',
            ],
        )

    # Resave some quests and assert their replays still pass, to make sure the loading/saving code is not introducing bugs.
    def test_save(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            return

        # TODO: Bad exit code 0xFFFF under windows.
        if platform.system() == 'Windows':
            return

        test_cases = [
            ('classic_1st.zplay', 'quests/Z1 Recreations/classic_1st.qst'),
            # TODO: fails
            # ('freedom_in_chains.zplay', 'freedom_in_chains.qst'),
            ('ss_jenny.zplay', 'ss_jenny.qst'),
        ]

        for zplay_path, qst_path in test_cases:
            with self.subTest(msg=zplay_path):
                load_qst_path = (
                    qst_path
                    if qst_path.startswith('quests/')
                    else root_dir / 'tests/replays' / qst_path
                )
                tmp_qst_dir = tmp_dir / f'resave-{zplay_path}'
                tmp_qst_dir.mkdir(exist_ok=True)
                tmp_qst_path = tmp_qst_dir / 'tmp.qst'

                run_target.check_run(
                    'zeditor',
                    [
                        '-headless',
                        '-s',
                        '-copy-qst',
                        load_qst_path,
                        tmp_qst_path,
                    ],
                )

                replay_content = (root_dir / 'tests/replays' / zplay_path).read_text(
                    'utf-8'
                )
                replay_content = replay_content.replace(qst_path, 'tmp.qst')
                replay_path = tmp_qst_dir / 'tmp.zplay'
                replay_path.write_text(replay_content)

                output_dir = tmp_dir / 'output' / replay_path.name
                self.run_replay(output_dir, [replay_path])

    def test_compile_and_quick_assign(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            return

        # TODO: set this via CLI
        include_paths = [
            str(root_dir / 'resources/include'),
            str(root_dir / 'resources/headers'),
            str(root_dir / 'tests/scripts'),
            str(root_dir / 'tests/scripts/alucard/100_rooms_of_wisdom'),
            str(root_dir / 'tests/scripts/alucard/combo_rotator'),
            str(root_dir / 'tests/scripts/compat'),
        ]
        (run_target.get_build_folder() / 'includepaths.txt').write_text(
            ';'.join(include_paths)
        )

        test_cases = [
            (
                root_dir / 'tests/replays/playground.qst',
                list((root_dir / 'tests/replays').glob('playground_*.zplay')),
            ),
        ]
        for replay_path in (root_dir / 'tests/replays/scripting').glob('*.zplay'):
            test_cases.append((replay_path.with_suffix('.qst'), [replay_path]))

        successful_qsts = []
        for qst_path, _ in test_cases:
            with self.subTest(msg=f'compile and quick assign {qst_path.name}'):
                if not test_cases:
                    raise Exception('where are the replays?')

                tmp_qst_path = tmp_dir / qst_path.name
                shutil.copy(qst_path, tmp_qst_path)
                self.quick_assign(tmp_qst_path)
                successful_qsts.append(qst_path)

        all_replay_paths = []
        for qst_path, replay_paths in test_cases:
            if qst_path in successful_qsts:
                for original_replay_path in replay_paths:
                    replay_path = tmp_dir / original_replay_path.name
                    shutil.copy(original_replay_path, replay_path)
                    all_replay_paths.append(replay_path)

        if not all_replay_paths:
            raise Exception('missing replays')

        with self.subTest(msg='replays still pass'):
            output_dir = tmp_dir / 'replays_output'
            if output_dir.exists():
                shutil.rmtree(output_dir)
            self.run_replay(output_dir, all_replay_paths)

    def test_export_strings(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            return

        tsv_path = tmp_dir / 'strings.tsv'
        if tsv_path.exists():
            tsv_path.unlink()
        args = [
            '-headless',
            '-export-strings',
            'quests/Z1 Recreations/classic_1st.qst',
            tsv_path,
        ]
        run_target.check_run('zeditor', args)
        tsv = tsv_path.read_text().splitlines()
        self.assertEqual(
            tsv[2],
            '  IT\'S DANGEROUS TO GO      ALONE! TAKE THIS.                           	0	0	0	0	0	24	32	192	24	18	1	0	0	0	8 0 0 8	0	0	0	0	1	1	0	0	6',
        )
        self.assertEqual(len(tsv), 37)

    def test_package_export(self):
        if (
            'emscripten' in str(run_target.get_build_folder())
            or platform.system() != 'Windows'
        ):
            raise unittest.SkipTest('unsupported platform')

        run_target.check_run(
            'zeditor',
            [
                '-package',
                'quests/Z1 Recreations/classic_1st.qst',
                'package-test',
            ],
        )

        package_dir = run_target.get_build_folder() / 'packages/package-test'
        args_path: Path = package_dir / 'data/zc_args.txt'
        args_path.write_text(args_path.read_text() + ' -q -headless')
        run_target.check_run(
            'zplayer',
            [
                '-package',
            ],
            package_dir / 'data',
        )


if __name__ == '__main__':
    unittest.main()
