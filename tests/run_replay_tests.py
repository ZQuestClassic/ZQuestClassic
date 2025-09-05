# Install python packages:
#   python -m pip install -r requirements.txt

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
import atexit
import os
import platform
import shutil
import subprocess
import sys
import time

from argparse import ArgumentTypeError
from pathlib import Path
from time import sleep

import cutie

from common import get_recent_release_tag, get_release_platform
from compare_replays import (
    collect_many_test_results_from_ci,
    collect_many_test_results_from_dir,
    create_compare_report,
    start_webserver,
)
from github import Github
from replays import (
    Replay,
    RunReplayTestsProgress,
    RunResult,
    configure_estimate_multiplier,
    estimate_fps,
    load_replays,
    run_replays,
)
from run_test_workflow import (
    collect_baseline_from_test_results,
    get_args_for_collect_baseline_from_test_results,
)

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = script_dir / 'replays'
is_ci = 'CI' in os.environ

sys.path.append(str((root_dir / 'scripts').absolute()))
import archives

from github_helpers import infer_gha_platform


def dir_path(path):
    if not os.path.isfile(path) and (os.path.isdir(path) or not os.path.exists(path)):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


ASSERT_FAILED_EXIT_CODE = 120

if os.name == 'nt':
    sys.stdout.reconfigure(encoding='utf-8')

parser = argparse.ArgumentParser()
parser.add_argument(
    '--build_folder',
    type=dir_path,
    default='build/Release',
    help='The folder containing the exe files',
    metavar='DIRECTORY',
)
parser.add_argument(
    '--build_type',
    default='Release',
    help='How to treat the build, for purposes of timeouts and duration estimates',
)
parser.add_argument(
    '--test_results_folder',
    type=dir_path,
    help='Where to save the replay test artifacts. By default, somewhere in .tmp',
)
parser.add_argument(
    '--filter',
    action='append',
    metavar='FILEPATH',
    help='Specify a file to run, instead of running all. Can be supplied multiple times.',
)
parser.add_argument(
    '--max_duration',
    action='append',
    metavar='SECONDS',
    help='The maximum time, in seconds, the replay will test for.',
)
parser.add_argument(
    '--throttle_fps', action='store_true', help='Supply this to cap the replay\'s FPS'
)
parser.add_argument(
    '--retries',
    type=int,
    default=0,
    help='The number of retries (default 0) to give each replay',
)
parser.add_argument(
    '-c',
    '--concurrency',
    type=int,
    help='How many replays to run concurrently. If not value not provided, will be set based on the number of available CPU cores (unless --show or --no-headless is used, in which case the value will be set to 1).',
)
parser.add_argument(
    '--jit',
    action=argparse.BooleanOptionalAction,
    default=True,
    help='Enables JIT compilation',
)
parser.add_argument(
    '--debugger',
    action=argparse.BooleanOptionalAction,
    default=is_ci,
    help='Run in debugger (uses lldb)',
)
parser.add_argument(
    '--headless',
    action=argparse.BooleanOptionalAction,
    default=True,
    help='Run without display or sound',
)
parser.add_argument(
    '--show',
    action=argparse.BooleanOptionalAction,
    default=False,
    help='Alias for --no-headless and --throttle_fps',
)
parser.add_argument('--emoji', action=argparse.BooleanOptionalAction, default=True)
parser.add_argument(
    '--no_console', action='store_true', help='Prevent the debug console from opening'
)
parser.add_argument(
    '--baseline_version',
    help='The ZC version to use for generating the baseline for the compare report',
)
parser.add_argument(
    '--no_report_on_failure',
    action='store_true',
    help='Do not prompt to create compare report',
)
parser.add_argument('--not_interactive', action='store_true')
parser.add_argument('--extra_args')
parser.add_argument(
    '--for_dev_server', action=argparse.BooleanOptionalAction, default=False
)


mode_group = parser.add_argument_group('Mode', 'The playback mode')
exclgroup = mode_group.add_mutually_exclusive_group()

exclgroup.add_argument(
    '--replay',
    action='store_true',
    help='Play back the replay, without updating or asserting.',
)
exclgroup.add_argument(
    '--update',
    action='store_true',
    help='Update the replays, accepting any graphical changes (unless --frame is also set, see comment at top of run_replay_tests.py).',
)
exclgroup.add_argument(
    '--assert',
    dest='assertmode',
    action='store_true',
    help='Play back the replays in assert mode. This is the default behavior if no mode is specified.',
)

int_group = parser.add_argument_group(
    'Internal', 'Use these only if you know what they do.'
)

int_group.add_argument('--snapshot', action='append')
int_group.add_argument('--frame', action='append')
int_group.add_argument('--ci', nargs='?', help='Special arg meant for CI behaviors')
int_group.add_argument('--shard')
int_group.add_argument('--print_shards', action='store_true')
int_group.add_argument('--prune_test_results', action='store_true')

parser.add_argument(
    'replays',
    nargs='*',
    help='If provided, will only run these replays rather than those in tests/replays',
)
parser.add_argument(
    '--root_replays_folder',
    type=dir_path,
    help='Replays run names are made by taking the relative path to this folder. Only used if `replays` positional arg is used',
)

args = parser.parse_args()

if args.show:
    args.headless = False
    args.throttle_fps = True

if args.replays:
    tests = [Path(x) for x in args.replays]
    if args.root_replays_folder:
        replays_dir = args.root_replays_folder
    elif len(tests) > 1:
        replays_dir = Path(os.path.commonpath(tests))
    else:
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
    args.assertmode = True  # default true, not handled by argparse

if args.ci and '_' in args.ci:
    runs_on, arch = args.ci.split('_')
else:
    runs_on, arch = infer_gha_platform()


class ReplayTimeoutException(Exception):
    pass


def group_arg(raw_values: list[str], allow_concat=False):
    default_arg = None
    arg_by_replay = {}
    if raw_values:
        for raw_value in raw_values:
            if '=' in raw_value:
                for replay, value in [raw_value.split('=')]:
                    replay_full_path = replays_dir / replay

                    # If absolute path can't be found, use a looser match of just the filename.
                    if replay_full_path not in tests:
                        for test in tests:
                            if test.name == Path(replay).name:
                                replay_full_path = test
                                break

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
    assert group_arg(['classic_1st.zplay=10']) == (
        {test_r('classic_1st.zplay'): '10'},
        None,
    )
    assert group_arg(['first_quest_layered/first_quest_layered_01_of_18.zplay=10']) == (
        {test_r('first_quest_layered/first_quest_layered_01_of_18.zplay'): '10'},
        None,
    )
    assert group_arg(['1', 'classic_1st.zplay=10']) == (
        {test_r('classic_1st.zplay'): '10'},
        '1',
    )
    test_expect_error(lambda: group_arg(['1', 'no_exist.zplay=10']))
    test_expect_error(
        lambda: group_arg(['1', 'classic_1st.zplay=10', 'classic_1st.zplay=20'])
    )
    assert group_arg(
        ['3', 'classic_1st.zplay=10', 'classic_1st.zplay=20'], allow_concat=True
    ) == ({test_r('classic_1st.zplay'): '10 20'}, '3')
    assert group_arg(['3', 'classic_1st.zplay=10', 'credits.zplay=20']) == (
        {
            test_r('classic_1st.zplay'): '10',
            test_r('credits.zplay'): '20',
        },
        '3',
    )
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
if is_web and args.update:
    raise Exception('--update currently not supported for web build')
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

# TODO: move to replays.py
if is_web:
    print('starting webserver')
    webserver_p = subprocess.Popen(
        [
            'node',
            root_dir / 'scripts/webserver.mjs',
            args.build_folder / 'packages/web',
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    while webserver_p.poll() == None:
        if 'Server running' in webserver_p.stdout.readline():
            break
    if webserver_p.poll() is not None:
        raise Exception(f'Could not start webserver: {webserver_p.stderr.read()}')
    print('webserver started')


def apply_test_filter(filter: str):
    filter_as_path = Path(filter)

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
    # TODO: regression from `30ebe53 fix: passive (non-button) items not respecting "Usable as a Bunny" flag`
    'nargads_trail_crystal_crusades_11_of_24.zplay',
    'nargads_trail_crystal_crusades_19_of_24.zplay',
    'nargads_trail_crystal_crusades_20_of_24.zplay',
]
# TODO: weird failures in CI starting in https://github.com/ZQuestClassic/ZQuestClassic/pull/1137
if is_web:
    skip_tests.append('garbage_collection.zplay')
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


estimated_durations = {}
configure_estimate_multiplier(args.build_folder, args.build_type)
replays = load_replays(tests, replays_dir)

for replay in replays:
    frames = replay.frames
    frame_arg = get_arg_for_replay(replay.path, grouped_frame_arg, is_int=True)
    if frame_arg and frame_arg < replay.frames:
        frames = replay.frame_arg = frame_arg

    fps = estimate_fps(replay)
    estimated_duration = frames / fps
    max_duration_arg = get_arg_for_replay(
        replay.path, grouped_max_duration_arg, is_int=True
    )
    if max_duration_arg and estimated_duration > max_duration_arg:
        estimated_duration = max_duration_arg
        replay.frame_arg = fps * max_duration_arg
    estimated_durations[replay.name] = estimated_duration

    snapshot_arg = get_arg_for_replay(replay.path, grouped_snapshot_arg)
    if snapshot_arg:
        replay.snapshot_arg = snapshot_arg

replays.sort(key=lambda replay: -estimated_durations[replay.name])


# https://stackoverflow.com/a/6856593/2788187
def get_shards(replays: list[Replay], n: int) -> list[list[Replay]]:
    result = [[] for i in range(n)]
    sums = {i: 0 for i in range(n)}
    c = 0
    for replay in replays:
        for i in sums:
            if c == sums[i]:
                result[i].append(replay)
                break
        sums[i] += estimated_durations[replay.name]
        c = min(sums.values())
    return result


if args.shard and args.print_shards:
    ss = 1
    format_template = "{: <5} {: <10} {: <20}"
    print(format_template.format('shard', 'dur (s)', 'replays'), '\n')
    for shard in get_shards(replays, num_shards):
        total_duration = sum(estimated_durations[replay.name] for replay in shard)
        row = [
            str(ss),
            str(round(total_duration)),
            ' '.join(test.name for test in shard),
        ]
        print(format_template.format(*row))
        ss += 1

if args.shard:
    replays = get_shards(replays, num_shards)[shard_index - 1]
    if not replays:
        print('nothing to run for this shard')
        exit(0)

total_frames = sum(r.frames for r in replays)
total_frames_limited = sum(r.frames_limited() for r in replays)
frames_limited_ratio = total_frames_limited / total_frames
if frames_limited_ratio < 1:
    print(f'\nframes limited: {frames_limited_ratio * 100:.2f}%')

if args.print_shards:
    exit(0)


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
        print(
            'Note: you can create a token here: https://github.com/settings/personal-access-tokens/new'
        )
        print(
            'Necessary permissions are: Actions (read and write), Contents (read and write), Metadata (read)'
        )
        token = input('Enter a GitHub PAT token: ').strip()
        print()

    return Github(token), repo


def prompt_to_create_compare_report():
    if not cutie.prompt_yes_or_no(
        'Would you like to generate a compare report?', default_is_yes=True
    ):
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
    selected_index = None

    if args.baseline_version:
        selected_index = 1
    else:
        print('How should we get the baseline run?')
        selected_index = cutie.select(
            [
                'Collect from disk',
                'Run locally',
                'Run new job in GitHub Actions (requires token)',
                'Collect from finished job in GitHub Actions (requires token)',
            ]
        )
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
        selected_index = cutie.select([p.relative_to(root_dir) for p in options])
        print()
        test_runs.extend(collect_many_test_results_from_dir(options[selected_index]))
    elif selected_index == 1:
        if args.baseline_version:
            tag = args.baseline_version
            print(f'using baseline version: {tag}\n')
        else:
            # Get latest tags.
            subprocess.run(
                ['git', 'fetch', 'upstream'],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )

            most_recent_nightly = get_recent_release_tag(
                ['--match', '*.*.*-nightly*', '--match', '*.*.*-prerelease*']
            )
            try:
                archives_255_output = subprocess.check_output(
                    [
                        'python',
                        script_dir / '../scripts/archives.py',
                        'list',
                        '--channel',
                        '2.55',
                    ],
                    encoding='utf-8',
                ).strip()
                most_recent_stable = archives_255_output.splitlines()[-1].split(' ')[1]
            except e as Exception:
                print('error finding latest stable version, using 2.55.9 instead')
                print(e)
                most_recent_stable = '2.55.9'

            print('Select a release build to use: ')
            selected_index = cutie.select(
                [
                    # TODO
                    # 'Most recent passing build from CI',
                    f'Most recent nightly ({most_recent_nightly})',
                    f'Most recent stable ({most_recent_stable})',
                ]
            )
            print()

            if selected_index == 0:
                tag = most_recent_nightly
            elif selected_index == 1:
                tag = most_recent_stable

        release_platform = get_release_platform()
        build_dir = archives.download(tag, release_platform)
        if release_platform == 'mac':
            zc_app_path = next(build_dir.glob('*.app'))
            build_dir = zc_app_path / 'Contents/Resources'

        if local_baseline_dir.exists():
            shutil.rmtree(local_baseline_dir)
        command_args = [
            sys.executable,
            str(root_dir / 'tests/run_replay_tests.py'),
            '--replay',
            '--build_folder',
            str(build_dir),
            '--test_results_folder',
            str(local_baseline_dir),
            '--root_replays_folder',
            str(replays_dir),
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
        baseline_run_id = collect_baseline_from_test_results(
            gh, repo, '', [test_results_path]
        )
        print(f'GitHub Actions job is done, the workflow run id is: {baseline_run_id}')
        test_runs.extend(collect_many_test_results_from_ci(gh, repo, baseline_run_id))
        print(
            'Note: now that you\'ve done this, this will be the first ".gha-cache-dir" option listed in the "Collect from disk" option'
        )
    elif selected_index == 3:
        gh, repo = prompt_for_gh_auth()
        baseline_run_id = cutie.get_number(
            'Enter a workflow run id: ', allow_float=False
        )
        test_runs.extend(collect_many_test_results_from_ci(gh, repo, baseline_run_id))

    test_runs.extend(collect_many_test_results_from_dir(test_results_dir))

    create_compare_report(test_runs)
    start_webserver()


def is_known_failure_test(run: RunResult):
    if run.success:
        return False

    # Example:
    # if run.name == 'triggers.zplay' and run.unexpected_gfx_segments == [[11154, 11217]]:
    #     print('!!! filtering out known replay test failure !!!')
    #     print('dithered lighting is off-by-some only during scrolling')
    #     return True

    return False


if test_results_dir.exists() and next(
    test_results_dir.rglob('test_results.json'), None
):
    test_results_path = next(test_results_dir.rglob('test_results.json'))
    print('found existing test results at provided path')
    if is_ci:
        print('this is unexpected')
        exit(1)
    prompt_to_create_compare_report()
    exit(0)


def on_exit():
    import curses

    curses.endwin()


class ProgressDisplay:
    def __init__(self, replays: list[Replay], use_curses: bool, emoji: bool):
        self.use_curses = use_curses
        self.emoji = emoji
        self.replay_log_names = {r.name: self.get_replay_log_name(r) for r in replays}
        self.longest_log_name_len = max(len(n) for n in self.replay_log_names.values())
        if use_curses:
            import curses

            self.curses = curses

            self.scr = curses.initscr()
            curses.start_color()
            curses.use_default_colors()
            curses.init_pair(1, curses.COLOR_RED, -1)
            curses.init_pair(2, curses.COLOR_GREEN, -1)
            curses.init_pair(3, curses.COLOR_YELLOW, -1)
            atexit.register(on_exit)

            self.anim_clock = 0

    def __del__(self):
        if self.use_curses:
            self.scr.clear()
            self.curses.endwin()
            atexit.unregister(on_exit)
        else:
            clear_progress_str()

    def get_replay_log_name(self, replay: Replay):
        name = replay.name
        frames = replay.frames
        frames_limited = replay.frames_limited()
        if frames_limited is not None and frames_limited != frames:
            percent = 100 * frames_limited / frames
            return f'{name} ({percent:4.1f}%)'
        return name

    def get_status_msg(self, result: RunResult, finished: bool):
        if finished:
            if self.emoji:
                symbol = '✅' if result.success else '❌'
            else:
                # Can't figure out how to enable UTF-8 for VS output log.
                if args.emoji:
                    symbol = '✔' if result.success else '𐳼'
                else:
                    # Not even the above check/x work in any Windows terminal, so use plaintext
                    # if explictly opted out of emoji.
                    symbol = 'PASS' if result.success else 'FAIL'
            color = 2 if result.success else 1
        else:
            symbol = '◐◓◑◒'[self.anim_clock % 4] if args.emoji else 'WAIT'
            color = 3

        name = self.replay_log_names[result.name].ljust(
            self.longest_log_name_len + 1, ' '
        )
        parts = [name]

        if result.frame:
            parts.append(time_format(result.duration).rjust(7, ' '))
            if result.fps != None:
                parts.append(f'{result.fps} fps  '.rjust(10, ' '))
            if not finished:
                last_frame = result.frame
                num_frames = result.num_frames
                parts.append(f'{last_frame} / {num_frames}')

        if result.failing_frame != None:
            parts.append(f'failure on frame {result.failing_frame}')

        if result.exceptions:
            parts.append(' | '.join(result.exceptions))

        return (symbol, color, ' '.join(parts))

    def on_update(self, progress: RunReplayTestsProgress):
        if not self.use_curses:
            for name, status in progress.status.items():
                if status == 'finish':
                    result = next(r for r in progress.results if r.name == name)
                    symbol, _, text = self.get_status_msg(result, True)
                    clear_progress_str()
                    print(f'{symbol} {text}')
            print_progress_str(progress.summary)
            return

        import curses

        scr = self.scr
        rows, cols = curses.LINES, curses.COLS
        for _ in range(4):
            lines = []
            for result in progress.active:
                line = self.get_status_msg(result, False)
                lines.append(line)
                if len(lines) > rows - 1:
                    break
            for result in progress.results:
                if not result.success:
                    line = self.get_status_msg(result, True)
                    lines.append(line)
                if len(lines) > rows - 1:
                    break
            for result in progress.results:
                if result.success:
                    line = self.get_status_msg(result, True)
                    lines.append(line)
                if len(lines) > rows - 1:
                    break
            for replay in progress.pending:
                lines.append(('…', 3, self.replay_log_names[replay.name]))
                if len(lines) > rows - 1:
                    break

            rows, cols = scr.getmaxyx()
            lines = lines[: rows - 1]

            scr.erase()
            if rows >= 1 and rows < 3 and cols < 40 and cols >= 15:
                scr.addstr(0, 0, 'term too small')
            elif rows >= 3 and cols >= 40:
                for i, msg in enumerate(lines):
                    symbol, color, text = msg
                    scr.addstr(i, 0, symbol, curses.color_pair(color))
                    scr.addstr(i, len(symbol), ' ' + text)
                scr.addstr(len(lines), 0, progress.summary[: cols - 1])
            scr.refresh()

            self.anim_clock += 1
            sleep(0.25)


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


extra_args = [
    *(args.extra_args.split(' ') if args.extra_args else []),
    '-v1' if args.throttle_fps else '-v0',
]
if args.no_console:
    extra_args.append('-no_console')
# Allegro seems to be using free'd memory when shutting down the sound system.
# For now, just disable sound in CI or when using Asan/Coverage.
if args.headless and (is_asan or is_coverage or is_ci or mode == 'assert'):
    extra_args.append('-s')
# CI has issues with `install_sound`.
elif not args.headless and (is_mac_ci or is_linux_ci):
    extra_args.append('-s')

interactive = not args.not_interactive
print_emoji = args.emoji
use_curses = False
if sys.stdout.isatty() and not is_ci and interactive:
    try:
        import curses

        use_curses = True
        # TODO: I can't figured out how to use emoji w/ curses.
        print_emoji = False
    except:
        # TODO add to requirements.txt
        if platform.system() == 'Windows':
            print(
                'for a better experience, install the "windows-curses" package via pip'
            )
display = ProgressDisplay(replays, use_curses, print_emoji)


def on_update(progress: RunReplayTestsProgress):
    if args.for_dev_server:
        test_results = progress.all_results
        pending_results = [
            RunResult(str(t.path.relative_to(replays_dir)), '', str(t))
            for t in progress.pending
        ]
        test_results.runs.append(progress.active + pending_results + progress.results)
        test_results_path.write_text(test_results.to_json())
        test_results.runs.pop()

    display.on_update(progress)


# Cap the duration in CI, in case it somehow never ends.
timeout = is_ci
# ...but not for Coverage/Asan, which is unpredictably slow.
if is_coverage or is_asan:
    timeout = False

test_results = run_replays(
    replays,
    mode=mode,
    runs_on=runs_on,
    arch=arch,
    build_folder=args.build_folder,
    test_results_dir=test_results_dir,
    extra_args=extra_args,
    debugger=args.debugger,
    headless=args.headless,
    jit=args.jit,
    concurrency=concurrency,
    prune_test_results=args.prune_test_results,
    timeout=timeout,
    retries=args.retries,
    on_update=on_update,
)
del display

for result in test_results.runs[-1]:
    run_dir = test_results_dir / result.directory

    # Only print on failure and last attempt.
    if not result.success or result.exceptions:
        print(f'failure: {result.name}')

        if result.exceptions:
            title = 'EXCEPTIONS'
            length = len(title) * 2
            print()
            print('=' * length)
            print(title.center(length, '='))
            print('=' * length)
            print()
            for exception in result.exceptions:
                print(exception)

        def print_nicely(title: str, path: Path):
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

        print_nicely('STDERR', run_dir / 'stderr.txt')
        print_nicely('STDOUT', run_dir / 'stdout.txt')
        print_nicely('ALLEGRO LOG', run_dir / 'allegro.log')
        print()

if is_web:
    webserver_p.kill()

failing_replays = [r.name for r in test_results.runs[-1] if should_consider_failure(r)]
if mode == 'assert':
    if not failing_replays:
        print('all replay tests passed')
    else:
        print(f'{len(failing_replays)} replay tests failed')
        if not is_ci and not args.no_report_on_failure and interactive and sys.stdout.isatty():
            prompt_to_create_compare_report()
        exit(2)
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
        exit(2)
