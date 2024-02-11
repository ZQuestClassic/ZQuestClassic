# TODO: use virtualenv. For now, manually install these packages:
#   python -m pip install cutie PyGithub==1.58.2 requests watchdog discord.py Pillow intervaltree
# For Windows, install this:
#   python -m pip install windows-curses

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
#    ./zplayer -record path_to_file.zplay -test path_to_game.qst dmap screen
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
import heapq
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

    # Check if file is only partially written.
    if 'stopped' not in result:
        return None

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

        try:
            new_result = parse_result_txt_file(self.path)
            if not new_result:
                return
            self.result = new_result
        except:
            logging.warning('could not read result txt file')
            return

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
parser.add_argument('-c', '--concurrency', type=int,
    help='How many replays to run concurrently. If not value not provided, will be set based on the number of available CPU cores (unless --show or --no-headless is used, in which case the value will be set to 1).')
parser.add_argument('--jit', action=argparse.BooleanOptionalAction, default=True,
    help='Enables JIT compilation')
parser.add_argument('--debugger', action=argparse.BooleanOptionalAction, default=is_ci,
    help='Run in debugger (uses lldb)')
parser.add_argument('--headless', action=argparse.BooleanOptionalAction, default=True,
    help='Run without display or sound')
parser.add_argument('--show', action=argparse.BooleanOptionalAction, default=False,
    help='Alias for --no-headless and --throttle_fps')
parser.add_argument('--emoji', action=argparse.BooleanOptionalAction, default=True)
parser.add_argument('--no_console', action='store_true',
    help='Prevent the debug console from opening')


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
int_group.add_argument('--prune_test_results', action='store_true')

parser.add_argument('replays', nargs='*',
    help='If provided, will only run these replays rather than those in tests/replays')

args = parser.parse_args()

if args.show:
    args.headless = False
    args.throttle_fps = True

if args.replays:
    tests = [pathlib.Path(x) for x in args.replays]
    replays_dir = tests[0].parent
else:
    tests = list(replays_dir.rglob('*.zplay'))

mode = 'assert'
if args.update:
    mode = 'update'
    if args.frame:
        args.headless = False
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
    assert group_arg(['first_quest_layered/first_quest_layered_01_of_18.zplay=10']) == ({
        test_r('first_quest_layered/first_quest_layered_01_of_18.zplay'): '10'
    }, None)
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
is_linux_ci = args.ci and 'ubuntu' in args.ci
is_web = bool(list(args.build_folder.glob('**/play/index.html')))
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

if args.concurrency:
    concurrency = args.concurrency
else:
    if is_web or not args.headless:
        concurrency = 1
    elif is_ci:
        concurrency = os.cpu_count()
    else:
        concurrency = max(1, os.cpu_count() - 4)
    print(f'found {os.cpu_count()} cpus, setting concurrency to {concurrency}')

if is_web:
    print('starting webserver')
    webserver_p = subprocess.Popen([
        'python', root_dir / 'scripts/webserver.py',
        '--dir', args.build_folder / 'packages/web',
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    while webserver_p.poll() == None:
        if 'Served by' in webserver_p.stdout.readline():
            break
    print('webserver started')


def apply_test_filter(filter: str):
    filter_as_path = pathlib.Path(filter)

    exact_match = next((t for t in tests if t == filter_as_path.absolute()), None)
    if exact_match:
        return [exact_match]

    exact_match = next((t for t in tests if t == replays_dir / filter_as_path), None)
    if exact_match:
        return [exact_match]

    if filter_as_path.is_relative_to(replays_dir):
        filter_as_path = filter_as_path.relative_to(replays_dir)

    filtered = []
    for test in tests:
        rel = test.relative_to(replays_dir)
        if filter_as_path.as_posix() in rel.as_posix():
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

skip_tests = [
    'solid.zplay',
    # TODO: regression from `30ebe53 fix: passive (non-button) items not respecting "Usable as a Bunny" flag`
    'nargads_trail_crystal_crusades_11_of_24.zplay',
    'nargads_trail_crystal_crusades_19_of_24.zplay',
    'nargads_trail_crystal_crusades_20_of_24.zplay',
]
tests = [t for t in tests if t.name not in skip_tests]

if args.shard:
    shard_index, num_shards = (int(s) for s in args.shard.split('/'))
    if shard_index > num_shards or shard_index <= 0 or num_shards <= 0:
        raise ValueError('invalid --shard')


def time_format(ms: int):
    if ms is not None:
        seconds = int(ms / 1000)
        m = seconds // 60
        s = seconds % 60
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


def get_replay_qst(replay_path: pathlib.Path):
    with replay_path.open('r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line.startswith('M'):
                break

            _ , key, value = line.split(' ', 2)
            if key == 'qst':
                return value

    return None


def get_replay_name(replay_file: pathlib.Path):
    if replay_file.is_relative_to(replays_dir):
        return replay_file.relative_to(replays_dir).as_posix()
    else:
        return replay_file.name


@functools.cache
def read_replay_meta(path: pathlib.Path):
    meta = {}
    with path.open('r', encoding='utf-8') as f:
        while True:
            line = f.readline()
            if not line.startswith('M'):
                break
            _ , key, value = line.strip().split(' ', 2)
            meta[key] = value
    if not meta:
        raise Exception(f'invalid replay {path}')
    return meta


@functools.cache
def get_replay_data(file):
    name = get_replay_name(file)
    meta = read_replay_meta(file)

    if 'frames' in meta:
        frames = int(meta['frames'])
    else:
        # TODO: delete this when all replay tests have `M frames`
        last_step = read_last_contentful_line(file)
        if not last_step:
            raise Exception(f'no content found in {name}')
        if not re.match(r'^. \d+ ', last_step):
            raise Exception(f'unexpected content found in {name}:\n  {last_step}\nAre you sure this is a zplay file?')
        frames = int(last_step.split(' ')[1])

    if meta['qst'] == 'playground.qst' and meta['version'] != 'latest':
        raise Exception(f'all playground.qst replays must set version to "latest": {file}')

    # Based on speed found on Windows 64-bit in CI. Should be manually updated occasionally.
    # NOTE: this are w/o any concurrency, so real numbers in CI today are expected to be lower. Maybe just remove this?
    estimated_fps = 1500
    estimated_fps_overrides = {
        'quests/Z1 Recreations/classic_1st.qst': 3000,
        'demosp253.qst': 1600,
        'dreamy_cambria.qst': 1100,
        'first_quest_layered.qst': 2500,
        'freedom_in_chains.qst': 1300,
        'hell_awaits.qst': 2800,
        'hero_of_dreams.qst': 2400,
        'hollow_forest.qst': 500,
        'lands_of_serenity.qst': 1700,
        'link_to_the_zelda.qst': 2100,
        'nes_remastered.qst': 3000,
        'new2013.qst': 2800,
        'solid.qst': 1400,
        'ss_jenny.qst': 1500,
        'stellar_seas_randomizer.qst': 500,
        'yuurand.qst': 650,
    }
    qst = get_replay_qst(file)
    if qst in estimated_fps_overrides:
        estimated_fps = estimated_fps_overrides[qst]
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
        'frame_arg': frame_arg,
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
        exe_path = args.build_folder / ('zplayer.exe' if os.name == 'nt' else 'zplayer')
        if not exe_path.exists():
            exe_path = args.build_folder / ('zelda.exe' if os.name == 'nt' else 'zelda')
        if not exe_path.exists():
            print(f'could not find executable at: {args.build_folder}\nYou may need to set the --build_folder arg (defaults to build/Release)')
            os._exit(1)

        exe_args = [
            exe_path.absolute(),
            f'-{mode}', replay_path,
            '-v1' if args.throttle_fps else '-v0',
            '-replay-exit-when-done',
            '-replay-output-dir', output_dir,
        ]

        if args.debugger:
            exe_args = [sys.executable, root_dir / 'scripts/run_target.py', exe_path.stem] + exe_args[1:]

        snapshot_arg = get_arg_for_replay(replay_path, grouped_snapshot_arg)
        if snapshot_arg is not None:
            exe_args.extend(['-snapshot', snapshot_arg])

        if frame != None:
            exe_args.extend(['-frame', str(frame)])

        if args.jit:
            exe_args.append('-jit')
            exe_args.append('-jit-fatal-compile-error')

        if args.headless:
            exe_args.append('-headless')
        elif is_mac_ci or is_linux_ci:
            exe_args.append('-s')
        
        if args.no_console:
            exe_args.append('-no_console')
        
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

        url = f'play/?{mode}=test_replays/{replay_path}&replayExitWhenDone&showFps'

        if args.headless:
            url += f'&headless'

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
            'http://localhost:8000',
            output_dir,
            url,
        ]

        self.p = subprocess.Popen(exe_args,
                                  stdout=open(output_dir / 'stdout.txt', 'w'),
                                  stderr=open(output_dir / 'stderr.txt', 'w'))

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


def run_replay_test(key: int, replay_file: pathlib.Path, output_dir: pathlib.Path) -> RunResult:
    name = get_replay_name(replay_file)
    result = RunResult(name=name, directory=output_dir.relative_to(test_results_dir).as_posix())
    roundtrip_path = output_dir / f'{name}.roundtrip'
    allegro_log_path = output_dir / 'allegro.log'
    result_path = output_dir / replay_file.with_suffix('.zplay.result.txt').name

    replay_data = get_replay_data(replay_file)
    frame_arg = replay_data['frame_arg']

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
                if not w:
                    return

                w.update_result()
                if not w.result:
                    return

                result.duration = w.result['duration']
                result.fps = int(w.result['fps'])
                result.frame = int(w.result['frame'])
                result.num_frames = int(w.result['replay_log_frames'])
                result.stopped = w.result['stopped']
                result.success = w.result['stopped'] and w.result['success']

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
                    watcher.update_result()
                    if not watcher.result:
                        raise Exception(f'process finished before replay started, exit code: {retcode}')

                yield (key, 'status', result)

            test_results.zc_version = watcher.result['zc_version']

            # .zplay.result.txt should be updated every second.
            while watcher.observer.is_alive():
                watcher.update_result()

                if player_interface.poll() != None:
                    break

                # Don't apply timeout until beyond the first frame, since JIT may take a moment for big scripts.
                if watcher.result['frame'] == 0:
                    continue

                if do_timeout and timer() - watcher.modified_time > timeout:
                    last_frame = watcher.result['frame']
                    raise ReplayTimeoutException(f'timed out, replay got stuck around frame {last_frame}')

                yield (key, 'status', result)

            player_interface.wait_for_finish()
            on_result_updated(watcher)

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
                result.exceptions.append(f'replay failed with unexpected code {exit_code}')
            # .zplay files are updated in-place, but lets also copy over to the test output folder.
            # This makes it easy to upload an archive of updated replays in CI.
            if mode == 'update' and watcher.result['changed']:
                (test_results_dir / 'updated').mkdir(exist_ok=True)
                shutil.copy2(replay_file, test_results_dir / 'updated' / replay_file.name)
            break
        except ReplayTimeoutException as e:
            # Will try again.
            result.exceptions.append(str(e))
            player_interface.stop()
        except KeyboardInterrupt:
            exit(1)
        except GeneratorExit:
            return
        except BaseException as e:
            result.exceptions.append(str(e))
            on_result_updated(watcher)
            yield (key, 'finish', result)
            return
        finally:
            if watcher:
                watcher.observer.stop()
            if player_interface:
                player_interface.wait_for_finish()
            clear_progress_str()

    yield (key, 'finish', result)


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


def run_replay_tests(tests: List[str], runs_dir: pathlib.Path) -> List[RunResult]:
    def get_replay_log_name(path: pathlib.Path):
        name = get_replay_name(path)
        replay_data = get_replay_data(path)
        frames = replay_data['frames']
        frames_limited = replay_data['frames_limited']
        if frames_limited is not None and frames_limited != frames:
            percent = 100 * frames_limited / frames
            return f'{name} ({percent:4.1f}%)'
        return name

    global global_i
    global_i = 0

    results: List[RunResult] = []
    pending_tests = [*tests]
    active_tests = []
    active_results = []

    replay_finished = {get_replay_name(t):False for t in tests}
    replay_log_names = {get_replay_name(t):get_replay_log_name(t) for t in tests}
    longest_log_name_len = max(len(get_replay_log_name(test)) for test in tests)
    estimates = {get_replay_name(t):get_replay_data(t) for t in tests}
    print_emoji = args.emoji

    use_curses = False
    if sys.stdout.isatty() and not is_ci:
        try:
            import curses
            import atexit
            use_curses = True
            # TODO: I can't figured out how to use emoji w/ curses.
            print_emoji = False
        except:
            if platform.system() == 'Windows':
                print('for a better experience, install the "windows-curses" package via pip')
            pass

    def on_exit():
        curses.endwin()

    if use_curses:
        scr = curses.initscr()
        curses.start_color()
        curses.use_default_colors()
        curses.init_pair(1, curses.COLOR_RED, -1)
        curses.init_pair(2, curses.COLOR_GREEN, -1)
        curses.init_pair(3, curses.COLOR_YELLOW, -1)
        atexit.register(on_exit)

    def get_eta():
        if not active_results:
            return ''

        # https://stackoverflow.com/a/49721962/2788187
        def compute_time(tasks, X):
            threads = [0] * X
            for t in tasks:
                heapq.heappush(threads, heapq.heappop(threads) + t)
            return max(threads)

        frames_so_far = sum(r.frame for r in results if r.frame) + sum(r.frame for r in active_results if r.frame)
        durs_so_far = (sum(r.duration for r in results if r.frame) + sum(r.duration for r in active_results if r.frame)) / 1000
        avg_fps = frames_so_far / durs_so_far if durs_so_far else 0
        avg_fps_established = frames_so_far > 5000 and avg_fps
        if not avg_fps_established:
            return 'ETA ...'

        durs = []

        # For actives tests, divide the frames remaining by the running average FPS.
        # If average FPS has not been established yet, use the rough estimate FPS.
        for result in active_results:
            estimate = estimates[result.name]
            if result.frame and result.num_frames and result.fps:
                if result.frame > 10000 or result.frame/result.num_frames > 0.1:
                    # Some replays tend to get much slower as they go on (ex: hero_of_dreams),
                    # which results in the estimate crawling up before it comes back down.
                    # Maybe a memory leak?
                    fps = result.fps
                else:
                    fps = estimate['estimated_fps']
                frames_left = result.num_frames - result.frame
                durs.append(frames_left / fps)
                continue

            # Test is still starting up.
            if avg_fps_established:
                durs.append(estimate['frames_limited'] / avg_fps)
            else:
                durs.append(estimate['estimated_duration'])

        # For pending tests, divide the frames to be run by the running average FPS.
        # If average FPS has not been established yet, use the rough estimate duration.
        for test in pending_tests:
            estimate = estimates[get_replay_name(test)]
            overhead = 10
            if avg_fps_established:
                durs.append(overhead + estimate['frames_limited'] / avg_fps)
            else:
                durs.append(overhead + estimate['estimated_duration'])

        s = compute_time(durs, concurrency)
        # pathlib.Path('.tmp/out.txt').write_text(f'{frames_so_far} / {durs_so_far} / {len(active_results)} = {avg_fps}\n' + ', '.join(str(int(d)) for d in durs))
        if s < 1:
            eta = 'ETA ~1s'
        elif s < 5:
            eta = 'ETA ~5s'
        elif s < 30:
            eta = 'ETA ~30s'
        elif s < 60:
            eta = 'ETA ~1m'
        else:
            eta = f'ETA {int(s / 60)}m'

        if avg_fps_established:
            return f'{eta} | {int(avg_fps)} fps'
        else:
            return eta
    
    def get_status_msg(r: RunResult):
        if replay_finished[r.name]:
            if print_emoji:
                symbol = '✅' if result.success else '❌'
            else:
                # Can't figure out how to enable UTF-8 for VS output log.
                if args.emoji:
                    symbol = '✔' if r.success else '𐳼'
                else:
                    # Not even the above check/x work in any Windows terminal, so use plaintext
                    # if explictly opted out of emoji.
                    symbol = 'PASS' if r.success else 'FAIL'
            color = 2 if r.success else 1
        else:
            symbol = '◐◓◑◒'[global_i % 4] if args.emoji else 'WAIT'
            color = 3

        name = replay_log_names[r.name].ljust(longest_log_name_len + 1, ' ')
        parts = [name]

        if r.frame:
            parts.append(time_format(r.duration).rjust(7, ' '))
            if r.fps != None:
                parts.append(f'{r.fps} fps  '.rjust(10, ' '))
            if not replay_finished[r.name]:
                last_frame = r.frame
                num_frames = r.num_frames
                parts.append(f'{last_frame} / {num_frames}')

        if r.failing_frame != None:
            parts.append(f'failure on frame {r.failing_frame}')

        if r.exceptions:
            parts.append(' | '.join(r.exceptions))

        return (symbol, color, ' '.join(parts))

    def get_summary_msg():
        num_failing = len([r for r in results if not r.success])
        return ' | '.join([
            f'{len(results)}/{len(tests)}',
            f'{num_failing} failing' if num_failing else 'OK',
            get_eta(),
        ])

    while pending_tests or active_tests:
        while pending_tests and len(active_tests) < concurrency:
            test = pending_tests.pop(0)
            test_index = tests.index(test)
            run_dir = runs_dir / test.with_suffix('').name
            run_dir.mkdir(parents=True)
            active_tests.append(run_replay_test(test_index, test, run_dir))

        next_active_tests = []
        active_results = []
        for active_test in active_tests:
            test_index, type, result = next(active_test, None)

            if type == 'status':
                next_active_tests.append(active_test)
                active_results.append(result)
            elif type == 'finish':
                replay_finished[result.name] = True
                results.append(result)
                if not use_curses:
                    symbol, _, text = get_status_msg(result)
                    clear_progress_str()
                    print(f'{symbol} {text}')
        active_tests = next_active_tests

        if use_curses:
            rows, cols = curses.LINES, curses.COLS
            for _ in range(4):
                lines = []
                for result in active_results:
                    lines.append(get_status_msg(result))
                    if len(lines) > rows - 1:
                        break
                for result in results:
                    if not result.success:
                        lines.append(get_status_msg(result))
                    if len(lines) > rows - 1:
                        break
                for result in results:
                    if result.success:
                        lines.append(get_status_msg(result))
                    if len(lines) > rows - 1:
                        break
                for test in pending_tests:
                    lines.append(('…', 3, replay_log_names[get_replay_name(test)]))
                    if len(lines) > rows - 1:
                        break

                rows, cols = scr.getmaxyx()
                lines = lines[:rows-1]

                scr.erase()
                if rows >= 1 and rows < 3 and cols < 40 and cols >= 15:
                    scr.addstr(0, 0, 'term too small')
                elif rows >= 3 and cols >= 40:
                    for i, msg in enumerate(lines):
                        symbol, color, text = msg
                        scr.addstr(i, 0, symbol, curses.color_pair(color))
                        scr.addstr(i, len(symbol), ' ' + text)
                    scr.addstr(len(lines), 0, get_summary_msg()[:cols-1])
                scr.refresh()

                sleep(0.5)
                global_i += 1
        else:
            print_progress_str(get_summary_msg())
            sleep(1)

    if use_curses:
        scr.clear()
        curses.endwin()
        atexit.unregister(on_exit)

    clear_progress_str()
    print_emoji = args.emoji

    if use_curses:
        for result in results:
            symbol, _, text = get_status_msg(result)
            print(f'{symbol} {text}')

    return [r for r in results if r]


def prompt_for_gh_auth():
    print('Select the GitHub repo:')
    repos = ['ZQuestClassic/ZQuestClassic', 'connorjclark/ZeldaClassic']
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
    command = f'git describe --tags --abbrev=0 --match {match}'
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
            zc_app_path = next(build_dir.glob('*.app'))
            build_dir = zc_app_path / 'Contents/Resources'

        if local_baseline_dir.exists():
            shutil.rmtree(local_baseline_dir)
        command_args = [
            sys.executable,
            str(root_dir / 'tests/run_replay_tests.py'),
            '--replay',
            '--build_folder', str(build_dir),
            '--test_results_folder', str(local_baseline_dir),
            '--retries=2',
            *get_args_for_collect_baseline_from_test_results([test_results_path]),
        ]
        if not args.jit:
            command_args.append('--no-jit')
        if args.no_console:
            command_args.append('--no_console')
        print(f'Collecting baseline locally: {" ".join(command_args)}')
        subprocess.check_call(command_args)
        test_runs.extend(collect_many_test_results_from_dir(local_baseline_dir))
    elif selected_index == 2:
        gh, repo = prompt_for_gh_auth()
        baseline_run_id = collect_baseline_from_test_results(gh, repo, '', [test_results_path])
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


if test_results_dir.exists() and next(test_results_dir.rglob('test_results.json'), None):
    test_results_path = next(test_results_dir.rglob('test_results.json'))
    print('found existing test results at provided path')
    if is_ci:
        print('this is unexpected')
        exit(1)
    prompt_to_create_compare_report()
    exit(0)

if test_results_dir.exists():
    shutil.rmtree(test_results_dir)
test_results_dir.mkdir(parents=True)

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

    results = run_replay_tests(tests_remaining, runs_dir)
    test_results.runs.append(results)

    for result in results:
        test = result.name
        run_dir = runs_dir / result.directory

        # Only print on failure and last attempt.
        if (not result.success or result.exceptions) and i == args.retries:
            print(f'failure: {result.name}')

            if result.exceptions:
                print(f'  EXCEPTION: {" | ".join(result.exceptions)}')

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


if args.prune_test_results:
    # Only keep the last run of each replay.
    replay_runs: List[RunResult] = []
    for runs in reversed(test_results.runs):
        for run in runs:
            if any(r for r in replay_runs if r.name == run.name):
                continue
            replay_runs.append(run)

    for runs in test_results.runs:
        for run in runs:
            if run not in replay_runs:
                shutil.rmtree(test_results_dir / run.directory)

    test_results.runs = [replay_runs]

    # These are huge and not necessary for the compare report.
    for file in test_results_dir.rglob('*.zplay.roundtrip'):
        file.unlink()

test_results_path.write_text(test_results.to_json())


def should_consider_failure(run: RunResult):
    if not run.success:
        return True

    # Currently there are failing exit codes when otherwise everything succeeded.
    # Maybe bad code in program termination? For now, ignore (unless under Asan where
    # we specifically care about the exit code).
    if is_asan and run.exit_code != 0:
        return True

    return False

if is_web:
    webserver_p.kill()

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
