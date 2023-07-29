# TODO: use virtualenv. For now, manually install these packages:
#   python -m pip install cutie PyGithub==1.58.2 requests watchdog discord.py Pillow intervaltree

# For more information, see replay.h

# Before this will work, you need to install git-lfs:
#     https://git-lfs.github.com/
# You should also run:
#     git config diff.lfs.textconv cat

# Default usage runs all the replays in assert mode:
#   python tests/run_replay_tests.py
#
# If any fail, a comparison report is generated for you.

# To create a new replay test, run:
#    ./zelda -record path_to_file.zplay -test path_to_game.qst dmap screen
#
# When done recording, just do Quit from the system menu.

# To run a specific test, run:
#   python tests/run_replay_tests.py --filter classic_1st.zplay
#
# Add `--frame <frame #>` to run the test only up to the specified frame.
# Instead of the command line, you can also use the ZC > Replay menu.

# To run just the tests containing a substring, run:
#   python tests/run_replay_tests.py --filter playground
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
from time import sleep
from timeit import default_timer as timer
from typing import List
from github import Github
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import cutie

from common import ReplayTestResults, RunResult, infer_gha_platform, download_release, maybe_get_downloaded_revision
from run_test_workflow import collect_baseline_from_test_results, get_args_for_collect_baseline_from_test_results
from compare_replays import create_compare_report, start_webserver, collect_many_test_results_from_dir, collect_many_test_results_from_ci

script_dir = pathlib.Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = script_dir / 'replays'
is_ci = 'CI' in os.environ


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
        if key == 'unexpected_gfx_frames':
            value = [int(x) for x in value.split(', ')]
        elif key == 'unexpected_gfx_segments' or key == 'unexpected_gfx_segments_limited':
            segments = []
            for pair in value.split(' '):
                if '-' in pair:
                    start, end = pair.split('-')
                else:
                    start = int(pair)
                    end = start
                segments.append([int(start), int(end)])
            value = segments
        elif value == 'true':
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
parser.add_argument('--build_type', default='Release',
    help='How to treat the build, for purposes of timeouts and duration estimates')
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
parser.add_argument('--debugger', action=argparse.BooleanOptionalAction, default=is_ci,
    help='Run in debugger (uses lldb)')
parser.add_argument('--headless', action=argparse.BooleanOptionalAction, default=True,
    help='Run without display or sound')


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


# Lazy unit testing.
def test_r(replay: str):
    return replays_dir / replay

def test_expect_error(cb):
    try:
        cb()
    except:
        return
    raise Exception('expected error')

if 'LAZY_TEST' in os.environ:
    assert group_arg([]) == ({}, None)
    assert group_arg(['1']) == ({}, '1')
    test_expect_error(lambda: group_arg(['1', '2']))
    assert group_arg(['classic_1st.zplay=10']) == ({test_r('classic_1st.zplay'): '10'}, None)
    assert group_arg(['1', 'classic_1st.zplay=10']) == ({test_r('classic_1st.zplay'): '10'}, '1')
    test_expect_error(lambda: group_arg(['1', 'no_exist.zplay=10']))
    test_expect_error(lambda: group_arg(['1', 'classic_1st.zplay=10', 'classic_1st.zplay=20']))
    assert group_arg(['3', 'classic_1st.zplay=10', 'classic_1st.zplay=20'], allow_concat=True) == ({test_r('classic_1st.zplay'): '10 20'}, '3')
    assert group_arg(['3', 'classic_1st.zplay=10', 'credits.zplay=20']) == ({
        test_r('classic_1st.zplay'): '10',
        test_r('credits.zplay'): '20',
    }, '3')
    print('LAZY_TEST done! exiting')
    exit(0)


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
is_coverage = args.build_folder.name == 'Coverage' or args.build_type == 'Coverage'
is_asan = args.build_folder.name == 'Asan' or args.build_type == 'Asan'
if args.test_results_folder:
    test_results_dir = args.test_results_folder.absolute()
else:
    test_results_dir = root_dir / f'.tmp/test_results/{int(time.time())}'
test_results_path = test_results_dir / 'test_results.json'
grouped_max_duration_arg = group_arg(args.max_duration)
grouped_snapshot_arg = group_arg(args.snapshot, allow_concat=True)
grouped_frame_arg = group_arg(args.frame)


def apply_test_filter(filter: str):
    filter_as_path = pathlib.Path(filter)
    if (os.curdir / filter_as_path).exists():
        filter_as_path = filter_as_path.absolute()
    if filter_as_path.is_absolute():
        filter_as_path = filter_as_path.relative_to(replays_dir)

    filtered = []
    for test in tests:
        if str(test.relative_to(replays_dir)) == str(filter_as_path):
            filtered.append(test)
        if filter in str(test.relative_to(replays_dir)):
            filtered.append(test)
    return filtered


if args.filter:
    filtered_tests = set()
    for filter in args.filter:
        some_tests = apply_test_filter(filter)
        if not some_tests:
            raise Exception(f'bad filter: {filter}')
        filtered_tests.update(some_tests)
    tests = list(filtered_tests)

if args.ci:
    skip_in_ci = [
        'solid.zplay',
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
    if is_asan:
        estimated_fps /= 15

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
        'estimated_duration': estimated_duration,
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


tests.sort(key=lambda test: -get_replay_data(test)['estimated_duration'])

if args.shard and args.print_shards:
    ss = 1
    format_template = "{: <5} {: <10} {: <20}"
    print(format_template.format('shard', 'dur (s)', 'replays'), '\n')
    for shard in get_shards(tests, num_shards):
        total_duration = sum(get_replay_data(test)['estimated_duration'] for test in shard)
        row = [
            str(ss),
            str(round(total_duration)),
            ' '.join(test.name for test in shard),
        ]
        print(format_template.format(*row))
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
        exe_path = args.build_folder / exe_name
        if not exe_path.exists():
            print(f'could not find executable at: {exe_path}\nYou may need to set the --build_folder arg (defaults to build/Release)')
            os._exit(1)

        exe_args = [
            exe_path.absolute(),
            f'-{mode}', replay_path,
            '-v1' if args.throttle_fps else '-v0',
            '-replay-exit-when-done',
            '-replay-output-dir', output_dir,
        ]

        if args.debugger:
            exe_args = [sys.executable, root_dir / 'scripts/run_target.py', 'zelda'] + exe_args[1:]

        snapshot_arg = get_arg_for_replay(replay_path, grouped_snapshot_arg)
        if snapshot_arg is not None:
            exe_args.extend(['-snapshot', snapshot_arg])

        if frame != None:
            exe_args.extend(['-frame', str(frame)])

        if args.jit:
            exe_args.append('-jit')

        if args.headless:
            exe_args.append('-headless')

        # Allegro seems to be using free'd memory when shutting down the sound system.
        # For now, just disable sound in CI or when using Asan/Coverage.
        if args.headless and (is_asan or is_coverage or is_ci or mode == 'assert'):
            exe_args.append('-s')

        allegro_log_path = output_dir / 'allegro.log'
        self.p = subprocess.Popen(exe_args,
                                  cwd=args.build_folder,
                                  env={
                                      **os.environ,
                                      'ALLEGRO_LEGACY_TRACE': str(allegro_log_path),
                                      'BUILD_FOLDER': str(args.build_folder.absolute()),
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
    # ...but not for Coverage/Asan, which is unpredictably slow.
    if is_coverage:
        do_timeout = False
    if is_asan:
        do_timeout = False

    timeout = 60
    if replay_file.name == 'yuurand.zplay':
        timeout = 180
    if is_web:
        timeout *= 2

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
                progress_str = f'{time_format(duration)}, {fps} fps, {last_frame} / {num_frames}'
                if w.result.get('success') == False:
                    failing_frame = w.result.get('failing_frame', None)
                    if failing_frame != None:
                        progress_str += f', failure on frame {failing_frame}'
                    else:
                        progress_str += ', failure'
                print_progress_str(progress_str)

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

                retcode = player_interface.poll()
                if retcode != None:
                    raise ReplayTimeoutException(f'process finished before replay started, exit code: {retcode}')

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
            on_result_updated(watcher)
            result.duration = watcher.result['duration']
            result.fps = int(watcher.result['fps'])
            result.frame = int(watcher.result['frame'])

            result.success = watcher.result['stopped'] and watcher.result['success']
            if not result.success:
                result.failing_frame = watcher.result.get('failing_frame', None)
                result.unexpected_gfx_frames = watcher.result.get('unexpected_gfx_frames', None)
                result.unexpected_gfx_segments = watcher.result.get('unexpected_gfx_segments', None)
                result.unexpected_gfx_segments_limited = watcher.result.get('unexpected_gfx_segments_limited', None)
            else:
                result.failing_frame = None
                result.unexpected_gfx_frames = None
                result.unexpected_gfx_segments = None
                result.unexpected_gfx_segments_limited = None
            exit_code = player_interface.get_exit_code()
            result.exit_code = exit_code
            if exit_code != 0 and exit_code != ASSERT_FAILED_EXIT_CODE:
                print(f'\nreplay failed with unexpected code {exit_code}')
            # .zplay files are updated in-place, but lets also copy over to the test output folder.
            # This makes it easy to upload an archive of updated replays in CI.
            if mode == 'update' and watcher.result['changed']:
                (test_results_dir / 'updated').mkdir(exist_ok=True)
                shutil.copy2(replay_file, test_results_dir / 'updated' / replay_file.name)
            break
        except ReplayTimeoutException:
            # Will try again.
            logging.exception('replay timed out')
            player_interface.stop()
        except KeyboardInterrupt:
            exit(1)
        except:
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
        if not result.success:
            if result.failing_frame != None:
                message += f', failure on frame {result.failing_frame}'
        print(message)

        # Only print on failure and last attempt.
        if not result.success and i == args.retries:
            def print_nicely(title: str, path: pathlib.Path):
                if not path.exists():
                    return

                title = f' {title} '
                length = len(title) * 2
                print()
                print('=' * length)
                print(title.center(length, '='))
                print('=' * length)
                print()
                sys.stdout.buffer.write(path.read_bytes())

            print_nicely('STDOUT', run_dir / 'stdout.txt')
            print_nicely('STDERR', run_dir / 'stderr.txt')
            print_nicely('ALLEGRO LOG', run_dir / 'allegro.log')
            print('\ndiff:')
            print(result.diff)

        runs.append(result)


test_results_path.write_text(test_results.to_json())

if is_ci:
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


def prompt_for_gh_auth():
    print('Select the GitHub repo:')
    repos = ['ArmageddonGames/ZQuestClassic', 'connorjclark/ZeldaClassic']
    selected_index = cutie.select(repos)
    print()
    repo = repos[selected_index]

    default_env_var = ['GH_PAT', 'GH_PAT_connorjclark'][selected_index]
    if default_env_var in os.environ:
        token = os.environ[default_env_var]
    else:
        print(f'Default environment variable for token (${default_env_var}) not set')
        print('Note: you can create a token here: https://github.com/settings/personal-access-tokens/new')
        print('Necessary permissions are: Actions (read and write), Contents (read and write), Metadata (read)')
        token = input('Enter a GitHub PAT token: ').strip()
        print()

    return Github(token), repo


def get_recent_release_tag(match: str):
    command = f'git describe --tags --abbrev=0 --match {match} main'
    return subprocess.check_output(command.split(' '), encoding='utf-8').strip()


def prompt_to_create_compare_report():
    if not cutie.prompt_yes_or_no('Would you like to generate a compare report?', default_is_yes=True):
        return
    print()

    # TODO: support filtering the failing tests
    # runs = [r for r in test_results.runs[-1] if not r.success]
    # print('Select the tests to generate a report for (to use all, just press Enter)')
    # selected_test_indices = cutie.select_multiple([r.name for r in runs],
    #                                               ticked_indices=list(range(0, len(runs))))
    # runs = [r for r in runs if runs.index(r) in selected_test_indices]
    # print(selected_test_names)

    test_runs = []

    print('How should we get the baseline run?')
    selected_index = cutie.select([
        'Collect from disk',
        'Run locally',
        'Run new job in GitHub Actions (requires token)',
        'Collect from finished job in GitHub Actions (requires token)',
    ])
    print()

    local_baseline_dir = root_dir / '.tmp/local-baseline'

    if selected_index == 0:
        gha_cache_dir = root_dir / 'tests/.gha-cache-dir'
        options = []
        if local_baseline_dir.exists():
            options.append(local_baseline_dir)
        options.extend(gha_cache_dir.glob('*'))
        options.sort(key=os.path.getmtime, reverse=True)
        print('Select a folder to use for the baseline:')
        selected_index = cutie.select(
            [p.relative_to(root_dir) for p in options])
        print()
        test_runs.extend(collect_many_test_results_from_dir(options[selected_index]))
    elif selected_index == 1:
        most_recent_nightly = get_recent_release_tag('nightly-*')
        most_recent_alpha = get_recent_release_tag('2.55-alpha-*')
        print('Select a release build to use: ')
        selected_index = cutie.select([
            # TODO
            # 'Most recent passing build from CI (requires token)',
            f'Most recent nightly ({most_recent_nightly}) (requires token)',
            f'Most recent alpha ({most_recent_alpha}) (requires token)',
        ])
        print()

        if selected_index == 0:
            tag = most_recent_nightly
        elif selected_index == 1:
            tag = most_recent_alpha

        system = platform.system()
        if system == 'Darwin':
            channel = 'mac'
        elif system == 'Windows':
            channel = 'windows'
        elif system == 'Linux':
            channel = 'linux'
        else:
            raise Exception(f'unexpected system: {system}')

        build_dir = maybe_get_downloaded_revision(tag)
        if not build_dir:
            gh, repo = prompt_for_gh_auth()
            build_dir = download_release(gh, repo, channel, tag)
        if channel == 'mac':
            build_dir = build_dir / 'ZeldaClassic.app/Contents/Resources'

        command_args = [
            sys.executable,
            str(root_dir / 'tests/run_replay_tests.py'),
            '--replay',
            '--build_folder', str(build_dir),
            '--test_results_folder', str(local_baseline_dir),
            *get_args_for_collect_baseline_from_test_results([test_results_path]),
        ]
        print(f'Collecting baseline locally: {" ".join(command_args)}')
        subprocess.check_call(command_args)
        test_runs.extend(collect_many_test_results_from_dir(local_baseline_dir))
    elif selected_index == 2:
        gh, repo = prompt_for_gh_auth()
        baseline_run_id = collect_baseline_from_test_results(gh, repo, [test_results_path])
        print(f'GitHub Actions job is done, the workflow run id is: {baseline_run_id}')
        test_runs.extend(collect_many_test_results_from_ci(gh, repo, baseline_run_id))
        print('Note: now that you\'ve done this, this will be the first ".gha-cache-dir" option listed in the "Collect from disk" option')
    elif selected_index == 3:
        gh, repo = prompt_for_gh_auth()
        baseline_run_id = cutie.get_number('Enter a workflow run id: ', allow_float=False)
        test_runs.extend(collect_many_test_results_from_ci(gh, repo, baseline_run_id))

    test_runs.extend(collect_many_test_results_from_dir(test_results_dir))

    create_compare_report(test_runs)
    start_webserver()


def is_known_failure_test(run: RunResult):
    if run.success:
        return False

    if run.name == 'triggers.zplay' and run.unexpected_gfx_segments == [[10817, 11217]]:
        print('!!! filtering out known replay test failure !!!')
        print('dithered lighting is off-by-some only during scrolling, and doubled-up for a single frame')
        return True

    return False


def should_consider_failure(run: RunResult):
    if is_known_failure_test(run):
        return False

    if not run.success:
        return True

    # Currently there are failing exit codes when otherwise everything succeeded.
    # Maybe bad code in program termination? For now, ignore (unless under Asan where
    # we specifically care about the exit code).
    if is_asan and run.exit_code != 0:
        return True

    return False


failing_replays = [r.name for r in test_results.runs[-1] if should_consider_failure(r)]
if mode == 'assert':
    if not failing_replays:
        print('all replay tests passed')
    else:
        print(f'{len(failing_replays)} replay tests failed')
        if not is_ci and sys.stdout.isatty() and replays_dir == script_dir / 'replays':
            prompt_to_create_compare_report()
        exit(1)
else:
    # We should still return a failing exit code if any replay failed to run, or had an
    # rng desync, or a bad exit code. Graphical differences won't count as failure
    # here - this is good for non-Release builds (coverage, asan). Don't want to fail if some
    # visual-only floating point drawing produces different visual results, but we do want to
    # fail if the replay does not finish.
    if not failing_replays:
        print('all replays ran successfully')
    else:
        print(f'{len(failing_replays)} replays did not finish')
        exit(1)
