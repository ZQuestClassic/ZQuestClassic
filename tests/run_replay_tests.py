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
from argparse import ArgumentTypeError
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
from dataclasses import dataclass
from common import ReplayTestResults, RunResult
from time import sleep
from timeit import default_timer as timer
from typing import List
from common import infer_gha_platform
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

script_dir = pathlib.Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = script_dir / 'replays'


def dir_path(path):
    if not os.path.isfile(path) and (os.path.isdir(path) or not os.path.exists(path)):
        return pathlib.Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


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
parser.add_argument('--build_folder', type=dir_path, default='build/Release',
    help='The folder containing the exe files', metavar='DIRECTORY')
parser.add_argument('--test_results_folder', type=dir_path,
    help='Where to save the replay test artifacts. By default, somewhere in .tmp')
parser.add_argument('--filter', action='append', metavar='FILEPATH',
    help='Specify a file to run, instead of running all. Can be supplied multiple times.')
parser.add_argument('--max_duration', action='append', metavar='SECONDS',
    help='The maximum time, in seconds, the replay will test for.')
parser.add_argument('--throttle_fps', action='store_true',
    help='Supply this to cap the replay\'s FPS')
parser.add_argument('--retries', type=int, default=0,
    help='The number of retries (default 0) to give each replay')
parser.add_argument('--jit', action=argparse.BooleanOptionalAction, default=True,
    help='Enables JIT compilation')


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


def group_arg(raw_values: List[str], allow_concat=False):
    default_arg = None
    arg_by_replay = {}
    if raw_values:
        for raw_value in raw_values:
            if '=' in raw_value:
                for replay, value in [raw_value.split('=')]:
                    replay_full_path = replays_dir / replay
                    if replay_full_path not in tests:
                        raise Exception(f'unknown test {replay}')
                    if replay_full_path in arg_by_replay:
                        if not allow_concat:
                            raise Exception('cannot include of the same key')
                        arg_by_replay[replay_full_path] += f' {value}'
                    else:
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


is_mac_ci = args.ci and 'mac' in args.ci
is_web = bool(list(args.build_folder.glob('*.wasm')))
is_web_ci = is_web and args.ci
is_coverage = args.build_folder.name == 'Coverage'
if args.test_results_folder:
    test_results_dir = args.test_results_folder.absolute()
else:
    test_results_dir = root_dir / f'.tmp/test_results/{int(time.time())}'
test_results_path = test_results_dir / 'test_results.json'
grouped_max_duration_arg = group_arg(args.max_duration)
grouped_snapshot_arg = group_arg(args.snapshot, allow_concat=True)
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
    if not last_step:
        raise Exception(f'no content found in {file.name}')
    if not re.match(r'^. \d+ ', last_step):
        raise Exception(f'unexpected content found in {file.name}:\n  {last_step}\nAre you sure this is a zplay file?')

    frames = int(last_step.split(' ')[1])

    # Based on speed found on Windows 64-bit in CI. Should be manually updated occasionally.
    estimated_fps = 1500
    estimated_fps_overrides = {
        'classic_1st_lvl1.zplay': 3000,
        'classic_1st.zplay': 3000,
        'demosp253.zplay': 1600,
        'dreamy_cambria.zplay': 1100,
        'first_quest_layered.zplay': 2500,
        'freedom_in_chains.zplay': 1300,
        'hell_awaits.zplay': 2800,
        'hero_of_dreams.zplay': 2400,
        'hollow_forest.zplay': 500,
        'lands_of_serenity.zplay': 1700,
        'link_to_the_zelda.zplay': 2100,
        'nes-remastered.zplay': 3000,
        'new2013.zplay': 2800,
        'solid.zplay': 1400,
        'ss_jenny.zplay': 1500,
        'stellar_seas_randomizer.zplay': 500,
        'yuurand.zplay': 650,
    }
    if file.name in estimated_fps_overrides:
        estimated_fps = estimated_fps_overrides[file.name]
    if is_mac_ci:
        estimated_fps /= 2
    if is_web_ci:
        estimated_fps /= 30
    elif is_web:
        estimated_fps /= 11
    if is_coverage:
        estimated_fps /= 10

    frames_limited = frames
    frame_arg = get_arg_for_replay(file, grouped_frame_arg, is_int=True)
    if frame_arg is not None and frame_arg < frames_limited:
        frames_limited = frame_arg

    estimated_duration = frames_limited / estimated_fps
    max_duration_arg = get_arg_for_replay(file, grouped_max_duration_arg, is_int=True)
    if max_duration_arg and estimated_duration > max_duration_arg:
        frames_limited = estimated_fps * max_duration_arg
        estimated_duration = max_duration_arg

    return {
        'frames': frames,
        'frames_limited': round(frames_limited),
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


@dataclass
class StartReplayArgs:
    replay_path: pathlib.Path
    output_dir: pathlib.Path
    frame: int

class CLIPlayerInterface:
    p = None

    def start_replay(self, player_args: StartReplayArgs):
        self.p = None
        replay_path = player_args.replay_path
        output_dir = player_args.output_dir
        frame = player_args.frame

        # TODO: fix this common-ish error, and whatever else is causing random failures.
        # Assertion failed: (mutex), function al_lock_mutex, file threads.Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
        # Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
        exe_name = 'zelda.exe' if os.name == 'nt' else 'zelda'
        exe_path = (args.build_folder / exe_name).absolute()

        exe_args = [
            exe_path,
            f'-{mode}', replay_path,
            '-v1' if args.throttle_fps else '-v0',
            '-replay-exit-when-done',
            '-replay-output-dir', output_dir,
        ]

        snapshot_arg = get_arg_for_replay(replay_path, grouped_snapshot_arg)
        if snapshot_arg is not None:
            exe_args.extend(['-snapshot', snapshot_arg])

        if frame != None:
            exe_args.extend(['-frame', str(frame)])

        if args.jit:
            exe_args.append('-jit')

        allegro_log_path = output_dir / 'allegro.log'
        self.p = subprocess.Popen(exe_args,
                                  cwd=args.build_folder,
                                  env={
                                      **os.environ,
                                      'ALLEGRO_LEGACY_TRACE': str(allegro_log_path),
                                  },
                                  stdout=open(output_dir / 'stdout.txt', 'w'),
                                  stderr=open(output_dir / 'stderr.txt', 'w'),
                                  encoding='utf-8',
                                  text=True)

    def wait_for_finish(self):
        if not self.p:
            return

        self.p.wait()

    def get_exit_code(self):
        if not self.p:
            return -1

        return self.p.returncode

    def poll(self):
        if not self.p:
            return

        return self.p.poll()

    def stop(self):
        if not self.p:
            return

        self.p.terminate()
        self.p.wait()


class WebPlayerInterface:
    p = None

    def start_replay(self, player_args: StartReplayArgs):
        self.p = None
        replay_path = player_args.replay_path
        output_dir = player_args.output_dir
        frame = player_args.frame

        replay_path = replay_path.relative_to(root_dir / 'tests/replays')

        url = f'zelda.html?{mode}=test_replays/{replay_path}&replayExitWhenDone&showFps'

        snapshot_arg = get_arg_for_replay(replay_path, grouped_snapshot_arg)
        if snapshot_arg is not None:
            url += f'&snapshot={snapshot_arg}'

        if frame != None:
            url += f'&frame={frame}'

        if args.throttle_fps:
            url += f'&v1'
        else:
            url += f'&v0'

        exe_args = [
            'node', root_dir / 'web/tests/run_replay.js',
            args.build_folder,
            output_dir,
            url,
        ]

        self.p = subprocess.Popen(exe_args)

    def wait_for_finish(self):
        if not self.p:
            return

        self.p.wait()

    def get_exit_code(self):
        if not self.p:
            return -1

        return self.p.returncode

    def poll(self):
        if not self.p:
            return

        return self.p.poll()

    def stop(self):
        if not self.p:
            return

        self.p.terminate()
        self.p.wait()


def run_replay_test(replay_file: pathlib.Path, output_dir: pathlib.Path) -> RunResult:
    result = RunResult(name=replay_file.name, directory=output_dir.relative_to(test_results_dir).as_posix())
    roundtrip_path = output_dir / f'{replay_file.name}.roundtrip'
    allegro_log_path = output_dir / 'allegro.log'
    result_path = output_dir / replay_file.with_suffix('.zplay.result.txt').name

    replay_data = get_replay_data(replay_file)
    frames = replay_data['frames']
    frames_limited = replay_data['frames_limited']
    frame_arg = get_arg_for_replay(replay_file, grouped_frame_arg, is_int=True)
    if frame_arg is None and frames_limited < frames:
        frame_arg = frames_limited
    if frame_arg is not None and frame_arg != frames:
        print(f"(-frame {frame_arg}, only doing {100 * frame_arg / frames:.2f}%) ", end='', flush=True)

    # Cap the duration in CI, in case it somehow never ends.
    do_timeout = True if args.ci else False
    timeout = 60
    if replay_file.name == 'yuurand.zplay':
        timeout = 180
    if is_coverage:
        timeout *= 5

    if is_web:
        player_interface = WebPlayerInterface()
    else:
        player_interface = CLIPlayerInterface()

    max_start_attempts = 5
    for _ in range(0, max_start_attempts):
        watcher = None
        try:
            if result_path.exists():
                result_path.unlink()
            if roundtrip_path.exists():
                roundtrip_path.unlink()
            if allegro_log_path.exists():
                allegro_log_path.unlink()

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
            player_interface.start_replay(StartReplayArgs(
                replay_path=replay_file,
                output_dir=output_dir,
                frame=frame_arg,
            ))

            # Wait for .zplay.result.txt creation.
            while True:
                if do_timeout and timer() - start > timeout:
                    raise ReplayTimeoutException('timed out waiting for replay to start')

                watcher.update_result()
                if watcher.result:
                    break

                sleep(0.1)

            test_results.zc_version = watcher.result['zc_version']

            # .zplay.result.txt should be updated every second.
            while watcher.observer.is_alive():
                if do_timeout and timer() - watcher.modified_time > timeout:
                    watcher.update_result()
                    last_frame = watcher.result['frame']
                    raise ReplayTimeoutException(f'timed out, replay got stuck around frame {last_frame}')

                if player_interface.poll() != None:
                    break

                sleep(0.1)

            player_interface.wait_for_finish()
            watcher.update_result()
            result.duration = watcher.result['duration']
            result.fps = int(watcher.result['fps'])

            result.success = watcher.result['stopped'] and watcher.result['success']
            if not result.success:
                result.failing_frame = watcher.result['failing_frame']
            else:
                result.failing_frame = None
            exit_code = player_interface.get_exit_code()
            if exit_code != 0 and exit_code != ASSERT_FAILED_EXIT_CODE:
                print(f'replay failed with unexpected code {exit_code}')
            # .zplay files are updated in-place, but lets also copy over to the test output folder.
            # This makes it easy to upload an archive of updated replays in CI.
            if mode == 'update' and watcher.result['changed']:
                (test_results_dir / 'updated').mkdir()
                shutil.copy2(replay_file, test_results_dir / 'updated' / replay_file.name)
            break
        except ReplayTimeoutException:
            print('\nSTDOUT:\n\n', (output_dir / 'stdout.txt').read_text())
            print('\n\nSTDERR:\n\n', (output_dir / 'stderr.txt').read_text())
            if allegro_log_path.exists():
                print('\n\nALLEGRO LOG:\n\n', allegro_log_path.read_text())

            # Will try again.
            logging.exception('replay timed out')
            player_interface.stop()
        except KeyboardInterrupt:
            exit(1)
        except:
            print('\nSTDOUT:\n\n', (output_dir / 'stdout.txt').read_text())
            print('\n\nSTDERR:\n\n', (output_dir / 'stderr.txt').read_text())
            if allegro_log_path.exists():
                print('\n\nALLEGRO LOG:\n\n', allegro_log_path.read_text())

            logging.exception('replay encountered an error')
            return result
        finally:
            if watcher:
                watcher.observer.stop()
            if player_interface:
                player_interface.wait_for_finish()
            clear_progress_str()

    if not args.update and player_interface.get_exit_code() == ASSERT_FAILED_EXIT_CODE:
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

if 'CI' in os.environ:
    # Only keep the images of the last run of each replay.
    replay_runs: List[RunResult] = []
    for runs in reversed(test_results.runs):
        for run in runs:
            if any(r for r in replay_runs if r.name == run.name):
                continue
            replay_runs.append(run)

    for runs in test_results.runs:
        for run in runs:
            if run not in replay_runs:
                for png in (test_results_dir / run.directory).glob('*.png'):
                    png.unlink()

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
