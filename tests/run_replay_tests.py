# For more information, see replay.h

# Before this will work, you need to install git-lfs:
#     https://git-lfs.github.com/
# You should also run:
#     git config diff.lfs.textconv cat

# To create a new replay test, run:
#    ./zelda -record path_to_file.zplay -test path_to_game.qst dmap screen
#
# When done recording, just do Quit from the system menu.

# To run a specific test, run:
#   python tests/run_replay_tests.py --filter test.zplay
#
# Add `--frame <frame #>` to run the test only up to the specified frame.
# Instead of the command line, you can also use the ZC > Replay menu.

# To update an existing replay test, run:
#   python tests/run_replay_tests.py --filter test.zplay --update
#
# This will accept any new changes and write the new replay file to disk.
# This is mostly likely only needed for when new calls to `replay_step_comment`
# are added. For anything more involved, see the next example.
# IMPORTANT: make sure you closely watch the new replay test. Ensure it still works!

# To update an existing replay test for a single screen, run:
#   python tests/run_replay_tests.py --filter test.zplay --update --frame 123
#
# To determine the frame number, find the `scr=` comment for the portion of the replay
# you want to re-record. Whatever its frame number is (the number after 'C'), add one
# and use that as the frame argument.

import argparse
import subprocess
import os
import sys
import re
import difflib
import pathlib
import platform
import functools
import shutil
from datetime import datetime, timezone
import time
import logging
from common import ReplayTestResults, RunResult
from time import sleep
from timeit import default_timer as timer
from typing import List, Dict, Any
from common import infer_gha_platform
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

script_dir = pathlib.Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = script_dir / 'replays'


def parse_result_txt_file(path: pathlib.Path):
    if platform.system() == 'Windows':
        # Windows has a tough time reading this file, sometimes resulting in a permission
        # denied error. I suspect MSVC's `std::filesystem::rename` is not atomic like it
        # claims to be. Or maybe the problem lies with Python's mtime.
        for _ in range(0, 10):
            try:
                lines = path.read_text().splitlines()
                if _ != 0:
                    logging.warning('finally was able to read it')
                break
            except:
                logging.exception(f'could not read {path}')
                sleep(0.1)
    else:
        lines = path.read_text().splitlines()

    result = {}
    for line in lines:
        key, value = line.split(': ', 1)
        if value == 'true':
            value = True
        elif value == 'false':
            value = False
        elif value.isdigit():
            value = int(value)
        else:
            try:
                value = float(value)
            except:
                pass

        result[key] = value

    return result


class ReplayResultUpdatedHandler(FileSystemEventHandler):

    def __init__(self, path, callback=None):
        self.path = path
        self.callback = callback
        self.result = None
        self.is_result_stale = False
        self.observer = Observer()
        self.observer.schedule(self, path.parent, recursive=False)
        self.observer.start()

    def update_result(self):
        if self.is_result_stale:
            self.parse_result()

    def parse_result(self):
        if self.result and self.result['stopped']:
            return

        if not self.path.exists():
            return

        self.result = parse_result_txt_file(self.path)
        self.is_result_stale = False
        if self.result['stopped']:
            self.observer.stop()

    def on_modified(self, event):
        if not event.is_directory and event.src_path.endswith(self.path.name):
            self.modified_time = timer()
            self.is_result_stale = True
            if self.callback:
                self.callback(self)

    def on_created(self, event):
        # To avoid reading the file in a bad state, replay.cpp first writes to a temporary file
        # and then moves it atomically. This can present as a file creation event, although it depends
        # on the implementation of the std fs library / possibly on the type of filesystem in use.
        if not event.is_directory and event.src_path.endswith(self.path.name):
            self.modified_time = timer()
            self.is_result_stale = True
            if self.callback:
                self.callback(self)



ASSERT_FAILED_EXIT_CODE = 120

if os.name == 'nt':
    sys.stdout.reconfigure(encoding='utf-8')

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', default='build/Debug',
    help='The folder containing the exe files',metavar='DIRECTORY')
parser.add_argument('--test_results_folder',
    help='Where to save the replay test artifacts. By default, somewhere in .tmp')
parser.add_argument('--filter', action='append', metavar='FILEPATH',
    help='Specify a file to run, instead of running all. Can be supplied multiple times.')
parser.add_argument('--max_duration', type=int, metavar='SECONDS',
    help='The maximum time, in seconds, the replay will test for.')
parser.add_argument('--throttle_fps', action='store_true',
    help='Supply this to cap the replay\'s FPS')
parser.add_argument('--retries', type=int, default=0,
    help='The number of retries (default 0) to give each replay')


mode_group = parser.add_argument_group('Mode','The playback mode')
exclgroup = mode_group.add_mutually_exclusive_group()

exclgroup.add_argument('--replay', action='store_true',
    help='Play back the replay, without updating or asserting.')
exclgroup.add_argument('--update', action='store_true',
    help='Update the replays, accepting any changes.')
exclgroup.add_argument('--assert', dest='assertmode', action='store_true',
    help='Play back the replays in assert mode. This is the default behavior if no mode is specified.')

int_group = parser.add_argument_group('Internal','Use these only if you know what they do.')

int_group.add_argument('--snapshot', action='append')
int_group.add_argument('--frame', action='append')
int_group.add_argument('--ci', nargs='?',
    help='Special arg meant for CI behaviors')
int_group.add_argument('--shard')
int_group.add_argument('--print_shards', action='store_true')

parser.add_argument('replays', nargs='*',
    help='If provided, will only run these replays rather than those in tests/replays')

args = parser.parse_args()

if args.replays:
    tests = [pathlib.Path(x) for x in args.replays]
    replays_dir = tests[0].parent
else:
    tests = list(replays_dir.glob('*.zplay'))

mode = 'assert'
if args.update:
    mode = 'update'
elif args.replay:
    mode = 'replay'
else:
    args.assertmode = True #default true, not handled by argparse

if args.ci and '_' in args.ci:
    runs_on, arch = args.ci.split('_')
else:
    runs_on, arch = infer_gha_platform()


class ReplayTimeoutException(Exception):
    pass


def group_arg(raw_values: List[str]):
    default_arg = None
    arg_by_replay = {}
    if raw_values:
        for raw_value in raw_values:
            if '=' in raw_value:
                for replay, value in [raw_value.split('=')]:
                    replay_full_path = replays_dir / replay
                    if replay_full_path not in tests:
                        raise Exception(f'unknown test {replay}')
                    arg_by_replay[replay_full_path] = value
            else:
                if default_arg != None:
                    raise Exception('can only define one default value')
                default_arg = raw_value

    return (arg_by_replay, default_arg)


def get_arg_for_replay(replay_file, grouped_arg, is_int=False):
    arg_by_replay, default_arg = grouped_arg
    if replay_file in arg_by_replay:
        result = arg_by_replay[replay_file]
    else:
        result = default_arg

    if is_int and result != None:
        return int(result)
    return result


last_progress_str = None

def print_progress_str(s: str):
    global last_progress_str

    if not sys.stdout.isatty():
        return

    clear_progress_str()
    print(s, end='', flush=True)
    last_progress_str = s


def clear_progress_str():
    global last_progress_str

    if not sys.stdout.isatty():
        return
    if last_progress_str:
        print('\b' * len(last_progress_str), end='', flush=True)
        print(' ' * len(last_progress_str), end='', flush=True)
        print('\b' * len(last_progress_str), end='', flush=True)
        last_progress_str = None


is_windows_ci = args.ci and 'windows' in args.ci
if args.test_results_folder:
    test_results_dir = pathlib.Path(args.test_results_folder).absolute()
else:
    test_results_dir = root_dir / f'.tmp/test_results/{int(time.time())}'
test_results_path = test_results_dir / 'test_results.json'
grouped_snapshot_arg = group_arg(args.snapshot)
grouped_frame_arg = group_arg(args.frame)

if args.filter:
    filtered_tests = []
    for filter in args.filter:
        filter = pathlib.Path(filter)
        if (os.curdir / filter).exists():
            filter = filter.absolute()
        if filter.is_absolute():
            filter = filter.relative_to(replays_dir)
        test = next((t for t in tests if str(t.relative_to(replays_dir)) == str(filter)), None)
        if not test:
            raise Exception(f'bad filter: {filter}')
        filtered_tests.append(test)
    tests = filtered_tests

if args.ci:
    skip_in_ci = [
        # nothing right now!
    ]
    tests = [t for t in tests if t.name not in skip_in_ci]

if args.shard:
    shard_index, num_shards = (int(s) for s in args.shard.split('/'))
    if shard_index > num_shards or shard_index <= 0 or num_shards <= 0:
        raise ValueError('invalid --shard')


def time_format(ms: int):
    if ms is not None:
        seconds = int(ms / 1000)
        m = seconds // 60
        s = seconds % 3600 % 60
        if m > 0:
            return '{}m {:02d}s'.format(m, s)
        elif s >= 0:
            return '{}s'.format(s)
    return '-'


def read_last_contentful_line(file):
    f = pathlib.Path(file).open('rb')
    try:  # catch OSError in case of a one line file
        f.seek(-2, os.SEEK_END)
        found_content = False
        while True:
            c = f.read(1)
            if not c.isspace():
                found_content = True
            if found_content and c == b'\n':
                if found_content:
                    break
            f.seek(-2, os.SEEK_CUR)
    except OSError:
        f.seek(0)
    return f.readline().decode()


@functools.cache
def get_replay_data(file):
    last_step = read_last_contentful_line(file)
    frames = int(last_step.split(' ')[1])

    # Based on speed found on win32 in CI. Should be manually updated occasionally.
    estimated_fps = 1500
    estimated_fps_overrides = {
        'classic_1st.zplay': 1400,
        'demosp253.zplay': 800,
        'first_quest_layered.zplay': 1700,
        'nes-remastered.zplay': 1400,
        'stellar_seas_randomizer.zplay': 150,
        'solid.zplay': 800,
    }
    if file.name in estimated_fps_overrides:
        estimated_fps = estimated_fps_overrides[file.name]

    frames_limited = frames
    frame_arg = get_arg_for_replay(file.name, grouped_frame_arg, is_int=True)
    if frame_arg is not None and frame_arg < frames_limited:
        frames_limited = frame_arg

    estimated_duration = frames_limited / estimated_fps
    if args.max_duration and estimated_duration > args.max_duration:
        frames_limited = estimated_fps * args.max_duration
        estimated_duration = args.max_duration

    return {
        'frames': frames,
        'frames_limited': frames_limited,
        'estimated_fps': estimated_fps,
        'estimated_duration': round(estimated_duration),
    }


# https://stackoverflow.com/a/6856593/2788187
def get_shards(tests, n):
    result = [[] for i in range(n)]
    sums = {i: 0 for i in range(n)}
    c = 0
    for test in tests:
        for i in sums:
            if c == sums[i]:
                result[i].append(test)
                break
        sums[i] += get_replay_data(test)['estimated_duration']
        c = min(sums.values())
    return result


def parse_result_txt_file(path: pathlib.Path):
    if platform.system() == 'Windows':
        # Windows has a tough time reading this file, sometimes resulting in a permission
        # denied error. I suspect MSVC's `std::filesystem::rename` is not atomic like it
        # claims to be. Or maybe the problem lies with Python's mtime.
        for _ in range(0, 10):
            try:
                lines = path.read_text().splitlines()
                if _ != 0:
                    logging.warning('finally was able to read it')
                break
            except:
                logging.exception(f'could not read {path}')
                sleep(0.1)
    else:
        lines = path.read_text().splitlines()

    result = {}
    for line in lines:
        key, value = line.split(': ', 1)
        if value == 'true':
            value = True
        elif value == 'false':
            value = False
        elif value.isdigit():
            value = int(value)
        else:
            try:
                value = float(value)
            except:
                pass

        result[key] = value

    return result


tests.sort(key=lambda test: -get_replay_data(test)['estimated_duration'])

if args.shard and args.print_shards:
    ss = 1
    for shard in get_shards(tests, num_shards):
        total_duration = sum(get_replay_data(test)['estimated_duration'] for test in shard)
        print(ss, total_duration, ' '.join(test.name for test in shard))
        ss += 1

if args.shard:
    tests = get_shards(tests, num_shards)[shard_index - 1]
    if not tests:
        print('nothing to run for this shard')
        exit(0)

if args.ci:
    total_frames = sum(get_replay_data(test)['frames'] for test in tests)
    total_frames_limited = sum(get_replay_data(test)['frames_limited'] for test in tests)
    frames_limited_ratio = total_frames_limited / total_frames
    if frames_limited_ratio < 1:
        print(f'\nframes limited: {frames_limited_ratio * 100:.2f}%')

if args.print_shards:
    exit(0)


def run_replay_test(replay_file: pathlib.Path, output_dir: pathlib.Path) -> RunResult:
    result = RunResult(name=replay_file.name, directory=str(output_dir.relative_to(test_results_dir)))

    # TODO: fix this common-ish error, and whatever else is causing random failures.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    exe_name = 'zelda.exe' if os.name == 'nt' else 'zelda'
    exe_path = pathlib.Path(f'{args.build_folder}/{exe_name}').absolute()

    exe_args = [
        exe_path,
        f'-{mode}', replay_file,
        '-v1' if args.throttle_fps else '-v0',
        '-replay-exit-when-done',
        '-replay-output-dir', output_dir,
    ]

    frame_arg = get_arg_for_replay(replay_file, grouped_frame_arg, is_int=True)
    if frame_arg is not None:
        exe_args.extend(['-frame', str(frame_arg)])

    snapshot_arg = get_arg_for_replay(replay_file, grouped_snapshot_arg)
    if snapshot_arg is not None:
        exe_args.extend(['-snapshot', snapshot_arg])

    replay_data = get_replay_data(replay_file)
    frames = replay_data['frames']
    frames_limited = replay_data['frames_limited']

    # Cap the duration in CI, in case it somehow never ends.
    do_timeout = True if args.ci else False

    if frame_arg is not None and frame_arg < frames:
        frames_limited = frame_arg
    if frames_limited != frames:
        print(f"(-frame {frames_limited}, only doing {100 * frames_limited / frames:.2f}%) ", end='', flush=True)
        exe_args.extend(['-frame', str(frames_limited)])

    allegro_log_path = None
    max_start_attempts = 5
    for i in range(0, max_start_attempts):
        allegro_log_path = output_dir / 'allegro.log'
        p = None
        try:
            result_path = output_dir / replay_file.with_suffix('.zplay.result.txt').name
            if result_path.exists():
                result_path.unlink()

            def on_result_updated(w: ReplayResultUpdatedHandler):
                if not sys.stdout.isatty():
                    return

                w.update_result()
                if not w:
                    return

                last_frame = w.result['frame']
                num_frames = w.result['replay_log_frames']
                duration = w.result['duration']
                fps = w.result['fps']
                print_progress_str(
                    f'{time_format(duration)}, {fps} fps, {last_frame} / {num_frames}')

            watcher = ReplayResultUpdatedHandler(result_path, on_result_updated)

            start = timer()
            p = subprocess.Popen(exe_args,
                                 cwd=args.build_folder,
                                 env={
                                     **os.environ,
                                     'ALLEGRO_LEGACY_TRACE': str(allegro_log_path),
                                 },
                                 stdout=open(output_dir / 'stdout.txt', 'w'),
                                 stderr=open(output_dir / 'stderr.txt', 'w'),
                                 encoding='utf-8',
                                 text=True)

            # Wait for .zplay.result.txt creation.
            while True:
                if do_timeout and timer() - start > 60:
                    raise ReplayTimeoutException('timed out waiting for replay to start')

                watcher.update_result()
                if watcher.result:
                    break

                sleep(0.1)

            test_results.zc_version = watcher.result['zc_version']

            # .zplay.result.txt should be updated every second.
            while watcher.observer.is_alive():
                if do_timeout and timer() - watcher.modified_time > 30:
                    watcher.observer.stop()
                    watcher.update_result()
                    last_frame = watcher.result['frame']
                    raise ReplayTimeoutException(f'timed out, replay got stuck around frame {last_frame}')

                if p.poll() != None:
                    watcher.observer.stop()
                    break

                sleep(0.1)

            p.wait()
            watcher.observer.stop()
            watcher.update_result()
            result.duration = watcher.result['duration']
            result.fps = int(watcher.result['fps'])

            result.success = watcher.result['stopped'] and watcher.result['success']
            if p.returncode != 0 and p.returncode != ASSERT_FAILED_EXIT_CODE:
                print(f'replay failed with unexpected code {p.returncode}')
            break
        except ReplayTimeoutException:
            # Will try again.
            logging.exception('replay timed out')
            p.terminate()
            p.wait()
        except KeyboardInterrupt:
            exit(1)
        except:
            logging.exception('replay encountered an error')
            return result
        finally:
            if p:
                p.wait()
            clear_progress_str()

    if not args.update and p.returncode == ASSERT_FAILED_EXIT_CODE:
        roundtrip_path = output_dir / f'{replay_file.name}.roundtrip'
        if roundtrip_path.exists():
            with open(replay_file) as f:
                fromlines = f.readlines()
            with open(roundtrip_path) as f:
                tolines = f.readlines()
            diff_iter = difflib.context_diff(
                fromlines, tolines,
                replay_file.name,
                roundtrip_path.name,
                n=3)
            trimmed_diff_lines = [x for _, x in zip(range(100), diff_iter)]
            result.diff = ''.join(trimmed_diff_lines)
        else:
            result.diff = 'missing roundtrip file, cannnot diff'

    stderr_path = output_dir / 'stderr.txt'
    if not result.success and stderr_path.exists():
        # TODO make this part of .result.txt
        failing_frame_match = re.match(
            r'.*expected:\n.*?(\d+)', stderr_path.read_text(), re.DOTALL)
        if failing_frame_match:
            result.failing_frame = int(failing_frame_match.group(1))
        else:
            print('could not find failing frame')

    return result


if test_results_dir.exists():
    shutil.rmtree(test_results_dir)
test_results_dir.mkdir(parents=True)

is_ci = 'CI' in os.environ
test_results = ReplayTestResults(
    runs_on=runs_on,
    arch=arch,
    ci=is_ci,
    git_ref=os.environ.get('GITHUB_REF') if is_ci else None,
    workflow_run_id=os.environ.get('GITHUB_RUN_ID') if is_ci else None,
    zc_version='unknown',
    time=datetime.now(timezone.utc).isoformat(),
    runs=[],
)


print(f'running {len(tests)} replays\n')
iteration_count = 0
for i in range(args.retries + 1):
    if i == 0:
        tests_remaining = tests
    else:
        tests_remaining = [replays_dir / r.name for r in test_results.runs[-1] if not r.success]
    if not tests_remaining:
        break
    if i != 0:
        print('\nretrying failures...\n')

    runs_dir = test_results_dir / str(i)
    runs: List[RunResult] = []
    test_results.runs.append(runs)
    for test in tests_remaining:
        print(f'= {test.relative_to(replays_dir)} ... ', end='', flush=True)
        run_dir = runs_dir / test.with_suffix('').name
        run_dir.mkdir(parents=True)
        result = run_replay_test(test, run_dir)
        status_emoji = '✅' if result.success else '❌'

        message = f'{status_emoji} {time_format(result.duration)}'
        if result.fps != None:
            message += f', {result.fps} fps'
        print(message)

        # Only print on failure and last attempt.
        if not result.success and i == args.retries:
            print('\ndiff:')
            print(result.diff)

        runs.append(result)


test_results_path.write_text(test_results.to_json())

if mode == 'assert':
    failing_tests = [r.name for r in test_results.runs[-1] if not r.success]

    if len(failing_tests) == 0:
        print('all replay tests passed')
    else:
        print(f'{len(failing_tests)} replay tests failed')

        if replays_dir == script_dir / 'replays':
            print('\nto collect baseline artifacts and then generate a report, run the following commands:\n')
            print(f'python {script_dir}/run_test_workflow.py --test_results {test_results_dir} --token <1>')
            print(f'python {script_dir}/compare_replays.py --workflow_run <2> --local {test_results_dir}')
            print('\n')
            print('<1>: github personal access token, with write actions access')
            print('<2>: workflow run id printed from the previous command')

        exit(1)
