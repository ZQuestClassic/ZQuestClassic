import functools
import http.server
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
import unittest

from pathlib import Path

from replays import ReplayTestResults

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/test_replays'
failing_replay = tmp_dir / 'failing.zplay'
output_dir = tmp_dir / 'output'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


@functools.cache
def read_replay_meta(path: Path):
    meta = {}
    with path.open('r', encoding='utf-8') as f:
        while True:
            line = f.readline()
            if not line.startswith('M'):
                break
            _, key, value = line.strip().split(' ', 2)
            meta[key] = value
    if not meta:
        raise Exception(f'invalid replay {path}')
    return meta


def get_frame_from_snapshot_index(path: str) -> int:
    return int(re.match(r'.*\.zplay\.(\d+)', path).group(1))


def create_test_replay(contents):
    contents = contents.replace(
        'classic_1st.qst', str(root_dir / 'tests/replays/classic_1st.qst')
    )
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)
    failing_replay.write_text(contents)


def get_snapshots():
    paths = [s.relative_to(output_dir).as_posix() for s in output_dir.rglob('*.png')]
    return sorted(paths, key=get_frame_from_snapshot_index)


class TestReplays(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None

    def run_replay(self):
        args = [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--build_folder',
            run_target.get_build_folder(),
            '--test_results',
            output_dir,
        ]
        args.append(failing_replay)
        shutil.rmtree(output_dir, ignore_errors=True)
        output = subprocess.run(args, stdout=subprocess.PIPE, encoding='utf-8')
        test_results_path = tmp_dir / 'output/test_results.json'
        if not test_results_path.exists():
            print(output.stdout)
            raise Exception('could not find test_results.json')
        self.assertEqual(output.returncode, 2, output.stdout)

        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        return ReplayTestResults(**test_results_json)

    def test_failing_replay_different_gfx_step(self):
        failing_replay_contents = (
            root_dir / 'tests/replays/classic_1st_lvl1.zplay'
        ).read_text()
        failing_replay_contents = failing_replay_contents.replace(
            'C 549 g HNN', 'C 549 g blah'
        )
        failing_replay_contents = failing_replay_contents.replace(
            'C 1574 g "W', 'C 1574 g blah'
        )
        create_test_replay(failing_replay_contents)

        test_results = self.run_replay()
        result = test_results.runs[0][0]
        self.assertEqual(result.name, 'failing.zplay')
        self.assertEqual(result.success, False)
        self.assertEqual(result.stopped, True)
        self.assertEqual(result.failing_frame, 549)
        self.assertEqual(result.unexpected_gfx_frames, [549, 1574])
        self.assertEqual(result.unexpected_gfx_segments, [[549, 549], [1574, 1575]])
        self.assertEqual(
            result.unexpected_gfx_segments_limited, [[549, 549], [1574, 1575]]
        )
        self.assertEqual(
            get_snapshots(),
            [
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
                '0/failing/failing.zplay.559.png',
                '0/failing/failing.zplay.1560.png',
                '0/failing/failing.zplay.1561.png',
                '0/failing/failing.zplay.1562.png',
                '0/failing/failing.zplay.1564.png',
                '0/failing/failing.zplay.1566.png',
                '0/failing/failing.zplay.1567.png',
                '0/failing/failing.zplay.1568.png',
                '0/failing/failing.zplay.1570.png',
                '0/failing/failing.zplay.1572.png',
                '0/failing/failing.zplay.1573.png',
                '0/failing/failing.zplay.1574-unexpected.png',
                '0/failing/failing.zplay.1576.png',
                '0/failing/failing.zplay.1578.png',
                '0/failing/failing.zplay.1579.png',
                '0/failing/failing.zplay.1580.png',
                '0/failing/failing.zplay.1582.png',
                '0/failing/failing.zplay.1584.png',
            ],
        )

    def test_failing_replay_missing_gfx_step(self):
        failing_replay_contents = (
            root_dir / 'tests/replays/classic_1st_lvl1.zplay'
        ).read_text()
        failing_replay_contents = failing_replay_contents.replace('C 549 g HNN\n', '')
        failing_replay_contents = failing_replay_contents.replace('C 1574 g "W\n', '')
        create_test_replay(failing_replay_contents)

        test_results = self.run_replay()
        result = test_results.runs[0][0]
        self.assertEqual(result.name, 'failing.zplay')
        self.assertEqual(result.success, False)
        self.assertEqual(result.stopped, True)
        self.assertEqual(result.failing_frame, 549)
        self.assertEqual(result.unexpected_gfx_frames, [549, 1574])
        self.assertEqual(result.unexpected_gfx_segments, [[549, 549], [1574, 1575]])
        self.assertEqual(
            result.unexpected_gfx_segments_limited, [[549, 549], [1574, 1575]]
        )
        snapshots = get_snapshots()
        self.assertEqual(len(snapshots), 38)
        self.assertEqual(
            [s for s in snapshots if 'unexpected.png' in s],
            [
                '0/failing/failing.zplay.549-unexpected.png',
                '0/failing/failing.zplay.1574-unexpected.png',
            ],
        )

    # There should be a limit to the number of unexpected snapshots written when
    # many frames are failing in a row. If gfx goes back to working as expected,
    # we should also save more unexpected snapshots if needed to again.
    def test_failing_replay_different_gfx_step_limit(self):
        failing_replay_contents = (
            root_dir / 'tests/replays/classic_1st_lvl1.zplay'
        ).read_text()

        segment_1 = [100, 60 * 20 + 500]
        segment_2 = [segment_1[1] + 2000, segment_1[1] + 2200]
        lines = []
        for line in failing_replay_contents.splitlines():
            if not (line.startswith('C') and ' g ' in line):
                lines.append(line)
                continue

            frame = int(line.split(' ')[1])
            if frame >= segment_1[0] and frame <= segment_1[1]:
                line = f'C {frame} g blah'
            if frame >= segment_2[0] and frame <= segment_2[1]:
                line = f'C {frame} g blah'
            lines.append(line)
        failing_replay_contents = '\n'.join(lines)
        create_test_replay(failing_replay_contents)

        test_results = self.run_replay()
        result = test_results.runs[0][0]
        self.assertEqual(result.name, 'failing.zplay')
        self.assertEqual(result.success, False)
        self.assertEqual(result.stopped, True)
        self.assertEqual(result.failing_frame, 102)
        self.assertEqual(result.unexpected_gfx_segments, [[102, 1700], [3700, 3900]])
        self.assertEqual(
            result.unexpected_gfx_segments_limited, [[102, 402], [3700, 3900]]
        )
        self.assertEqual(len(result.unexpected_gfx_frames), 344)
        snapshots = get_snapshots()
        self.assertEqual(
            len(
                [
                    s
                    for s in snapshots
                    if segment_1[0] <= get_frame_from_snapshot_index(s) <= segment_1[1]
                ]
            ),
            143,
        )
        self.assertEqual(
            len(
                [
                    s
                    for s in snapshots
                    if segment_2[0] <= get_frame_from_snapshot_index(s) <= segment_2[1]
                ]
            ),
            201,
        )
        self.assertEqual(
            [s for s in snapshots if not 'unexpected' in s],
            [
                '0/failing/failing.zplay.84.png',
                '0/failing/failing.zplay.85.png',
                '0/failing/failing.zplay.86.png',
                '0/failing/failing.zplay.87.png',
                '0/failing/failing.zplay.88.png',
                '0/failing/failing.zplay.89.png',
                '0/failing/failing.zplay.90.png',
                '0/failing/failing.zplay.91.png',
                '0/failing/failing.zplay.96.png',
                '0/failing/failing.zplay.97.png',
                '0/failing/failing.zplay.1701.png',
                '0/failing/failing.zplay.1702.png',
                '0/failing/failing.zplay.1703.png',
                '0/failing/failing.zplay.1704.png',
                '0/failing/failing.zplay.1705.png',
                '0/failing/failing.zplay.1706.png',
                '0/failing/failing.zplay.1707.png',
                '0/failing/failing.zplay.1708.png',
                '0/failing/failing.zplay.1709.png',
                '0/failing/failing.zplay.1710.png',
                '0/failing/failing.zplay.3690.png',
                '0/failing/failing.zplay.3691.png',
                '0/failing/failing.zplay.3692.png',
                '0/failing/failing.zplay.3693.png',
                '0/failing/failing.zplay.3694.png',
                '0/failing/failing.zplay.3695.png',
                '0/failing/failing.zplay.3696.png',
                '0/failing/failing.zplay.3697.png',
                '0/failing/failing.zplay.3698.png',
                '0/failing/failing.zplay.3699.png',
                '0/failing/failing.zplay.3901.png',
                '0/failing/failing.zplay.3902.png',
                '0/failing/failing.zplay.3903.png',
                '0/failing/failing.zplay.3904.png',
                '0/failing/failing.zplay.3905.png',
                '0/failing/failing.zplay.3906.png',
                '0/failing/failing.zplay.3907.png',
                '0/failing/failing.zplay.3908.png',
                '0/failing/failing.zplay.3909.png',
                '0/failing/failing.zplay.3910.png',
            ],
        )

    def test_never_ending_failing_replay(self):
        failing_replay_contents = (
            root_dir / 'tests/replays/classic_1st_lvl1.zplay'
        ).read_text()
        lines = failing_replay_contents.splitlines()
        failing_replay_contents = '\n'.join(
            l for l in lines if not l.startswith('D') and not l.startswith('U')
        )
        create_test_replay(failing_replay_contents)

        test_results = self.run_replay()
        result = test_results.runs[0][0]
        self.assertEqual(result.name, 'failing.zplay')
        self.assertEqual(result.success, False)
        self.assertEqual(result.stopped, True)
        # This is the frame it stops at because of the loadscr check failing.
        self.assertEqual(result.frame, 634)
        self.assertEqual(result.failing_frame, 1)
        # The screen is static (all input was removed), so snapshots are driven
        # by the frames where the replay recorded a gfx change - capped by the
        # per-segment snapshot limit, even though the segment runs to the end.
        self.assertEqual(len(result.unexpected_gfx_frames), 157)
        self.assertEqual(result.unexpected_gfx_frames[0], 1)
        self.assertEqual(result.unexpected_gfx_frames[-1], 301)
        self.assertEqual(result.unexpected_gfx_segments, [[1, 634]])
        self.assertEqual(result.unexpected_gfx_segments_limited, [[1, 301]])
        snapshots = get_snapshots()
        self.assertEqual(
            snapshots,
            [
                f'0/failing/failing.zplay.{frame}-unexpected.png'
                for frame in result.unexpected_gfx_frames
            ],
        )

    def test_recording(self):
        replay_path = tmp_dir / 'recorded.zplay'
        if replay_path.exists():
            replay_path.unlink()
        run_target.check_run(
            'zplayer',
            [
                '-headless',
                '-v0',
                '-replay-exit-when-done',
                '-record',
                replay_path,
                '-replay-debug',
                '-frame',
                '100',
                '-test',
                root_dir / 'tests/replays/classic_1st.qst',
                '0',
                '119',
            ],
        )
        run_target.check_run(
            'zplayer',
            [
                '-headless',
                '-v0',
                '-replay-exit-when-done',
                '-assert',
                replay_path,
            ],
        )
        meta = read_replay_meta(replay_path)
        self.assertEqual(meta['debug'], 'true')
        self.assertEqual(meta['test_mode'], 'true')
        self.assertEqual(meta['frames'], '100')
        self.assertEqual(meta['qst_title'], 'Original NES 1st Quest')
        self.assertEqual(meta['qst_hash'], '5833FF169985B186D58058417E918408')

    def prepare_upload_test(self) -> tuple[Path, Path]:
        """Returns tuple: replays folder, upload state path"""
        resources_folder = run_target.get_build_folder()
        if resources_folder.name == 'bin':
            resources_folder = resources_folder / '../share/zquestclassic'

        replays_folder = resources_folder / 'replays'
        replays_folder.mkdir(exist_ok=True)
        state_path = replays_folder / 'state.json'
        if state_path.exists():
            state_path.unlink()

        return replays_folder, state_path

    def test_upload(self):
        replays_folder, state_path = self.prepare_upload_test()
        replay_path = root_dir / 'tests/replays/classic_1st_lvl1.zplay'

        # Start with just the first half of the replay, to later simulate the player
        # continuing to play the same save.
        # Only the leading lines are meta: a meta step may also appear mid-replay.
        lines = [l for l in replay_path.read_text().splitlines() if l]
        num_meta = next(i for i, l in enumerate(lines) if not l.startswith('M '))
        meta_lines, steps = lines[:num_meta], lines[num_meta:]

        def replay_text(num_steps: int):
            meta = [
                f'M length {num_steps}' if l.startswith('M length ') else l
                for l in meta_lines
            ]
            return '\n'.join(meta + steps[:num_steps]) + '\n'

        # Written like ZC does, with \n line endings: the upload is sent byte for byte.
        local_replay_path = replays_folder / 'test.zplay'
        local_replay_path.write_text(replay_text(len(steps) // 2), newline='\n')

        # A copied save's replay has no uuid until the copy is played.
        copy_replay_path = replays_folder / 'copy.zplay'
        copy_replay_path.write_text(
            replay_text(len(steps)).replace('M uuid ', 'M parent_uuid '), newline='\n'
        )
        copy_uuid = '6f3c8f0e-2b1a-4c7e-9d2f-5a6b7c8d9e0f'

        api_server_data_folder = tmp_dir / 'api_server'
        if api_server_data_folder.exists():
            shutil.rmtree(api_server_data_folder)
        api_server_data_folder.mkdir()
        shutil.copy(
            root_dir / 'api_server/test_fixtures/manifest.json',
            api_server_data_folder / 'manifest.json',
        )
        server_replay_path = (
            api_server_data_folder
            / 'replays/5833FF169985B186D58058417E918408/c990976a-f5cf-4d2f-994d-c06ab841bc96.zplay'
        )

        with open(tmp_dir / 'api_server.log', 'w') as f:
            p = subprocess.Popen(
                ['flask', '--app', 'server', 'run', '-p', '5000'],
                cwd=root_dir / 'api_server',
                env={**os.environ, 'FLASK_DATA_DIR': str(api_server_data_folder)},
                stdout=f,
                stderr=subprocess.STDOUT,
            )

        def upload_replays():
            run_target.check_run(
                'zplayer',
                [
                    '-headless',
                    '-upload-replays',
                ],
                env={**os.environ, 'TEST_ZC_API_SERVER': 'http://localhost:5000'},
            )
            state = json.loads(state_path.read_text())
            self.assertEqual(state['entries']['test.zplay']['state'], 'tracked')

        try:
            attempts = 0
            while True:
                text = (tmp_dir / 'api_server.log').read_text()
                if 'Running on' in text:
                    break

                time.sleep(1)
                attempts += 1
                if attempts == 30:
                    raise Exception(f'server did not start:\n{text}')

            upload_replays()

            replays = [
                str(p.relative_to(api_server_data_folder).as_posix())
                for p in api_server_data_folder.rglob('*.zplay')
            ]
            self.assertEqual(
                replays,
                [
                    'replays/5833FF169985B186D58058417E918408/c990976a-f5cf-4d2f-994d-c06ab841bc96.zplay'
                ],
            )
            self.assertEqual(
                server_replay_path.read_text(), replay_text(len(steps) // 2)
            )
            state = json.loads(state_path.read_text())
            self.assertEqual(state['entries']['copy.zplay']['state'], 'tracked')

            # Continue playing: the rest of the steps are appended to the same replay.
            # A replay is only re-uploaded if modified after the last upload, which is
            # tracked in seconds, so push the modified time forward.
            local_replay_path.write_text(replay_text(len(steps)), newline='\n')
            future_time = time.time() + 10
            os.utime(local_replay_path, (future_time, future_time))

            # Play the copied save, which gives its replay a new uuid.
            copy_replay_path.write_text(
                f'M uuid {copy_uuid}\n' + copy_replay_path.read_text(), newline='\n'
            )
            os.utime(copy_replay_path, (future_time, future_time))

            upload_replays()

            copy_server_replay_path = (
                api_server_data_folder
                / f'replays/5833FF169985B186D58058417E918408/{copy_uuid}.zplay'
            )
            self.assertTrue(copy_server_replay_path.exists())

            self.assertEqual(server_replay_path.read_text(), replay_text(len(steps)))
            self.assertIn(
                '"PUT /api/v1/replays/c990976a-f5cf-4d2f-994d-c06ab841bc96 HTTP/1.1" 200',
                (tmp_dir / 'api_server.log').read_text(),
            )
        finally:
            p.kill()
            p.wait()

    def test_upload_unexpected_error(self):
        replays_folder, state_path = self.prepare_upload_test()
        shutil.copy(
            root_dir / 'tests/replays/classic_1st_lvl1.zplay',
            replays_folder / 'test.zplay',
        )

        # Errors that aren't the server rejecting the replay (ex: a 404 from a proxy, or a bad
        # deploy) should not make the client give up on the replay forever.
        class Handler(http.server.BaseHTTPRequestHandler):
            def do_GET(self):
                body = json.dumps([{'hash': '5833FF169985B186D58058417E918408'}])
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(body.encode())

            def do_PUT(self):
                self.rfile.read(int(self.headers['Content-Length']))
                self.send_response(404)
                self.end_headers()

            def log_message(self, format, *args):
                pass

        server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        threading.Thread(target=server.serve_forever, daemon=True).start()
        try:
            run_target.check_run(
                'zplayer',
                [
                    '-headless',
                    '-upload-replays',
                ],
                env={
                    **os.environ,
                    'TEST_ZC_API_SERVER': f'http://127.0.0.1:{server.server_port}',
                },
            )
        finally:
            server.shutdown()
            server.server_close()

        state = json.loads(state_path.read_text())
        self.assertEqual(state['entries']['test.zplay']['state'], 'try_later')


if __name__ == '__main__':
    unittest.main()
