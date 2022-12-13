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
import json
import re
import difflib
import pathlib
import functools
from types import SimpleNamespace
from time import sleep
from timeit import default_timer as timer
from typing import List
from common import infer_gha_platform

ASSERT_FAILED_EXIT_CODE = 120

if os.name == 'nt':
    sys.stdout.reconfigure(encoding='utf-8')

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', default='build/Debug')
parser.add_argument('--filter', action='append')
parser.add_argument('--max_duration', type=int)
parser.add_argument('--throttle_fps', action='store_true')
parser.add_argument('--update', action='store_true')
parser.add_argument('--snapshot', action='append')
parser.add_argument('--retries', type=int, default=0)
parser.add_argument('--frame', action='append')
parser.add_argument('--ci')
parser.add_argument('--shard')
parser.add_argument('--print_shards', action='store_true')
parser.add_argument('--replay', action='store_true')
args = parser.parse_args()

if args.replay and args.update:
    raise Exception('only one of --update or --replay may be used')

mode = 'assert'
if args.update:
    mode = 'update'
elif args.replay:
    mode = 'replay'


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


is_windows_ci = args.ci and 'windows' in args.ci
script_dir = os.path.dirname(os.path.realpath(__file__))
replays_dir = pathlib.Path(os.path.join(script_dir, 'replays'))
test_results_path = replays_dir / 'test_results.json'
tests = list(replays_dir.glob('*.zplay'))
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
        'solid.zplay' if is_windows_ci else None,
    ]
    tests = [t for t in tests if t.name not in skip_in_ci]

if args.shard:
    shard_index, num_shards = (int(s) for s in args.shard.split('/'))
    if shard_index > num_shards or shard_index <= 0 or num_shards <= 0:
        raise ValueError('invalid --shard')


def time_format(seconds: int):
    if seconds is not None:
        seconds = int(seconds)
        m = seconds // 60
        s = seconds % 3600 % 60
        if m > 0:
            return '{}m {:02d}s'.format(m, s)
        elif s > 0:
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
        'first_quest_layered.zplay': 1200,
        'nes-remastered.zplay': 1400,
        'stellar_seas_randomizer.zplay': 150,
    }
    if file.name in estimated_fps_overrides:
        estimated_fps = estimated_fps_overrides[file.name]

    frames_limited = frames
    if args.ci and file.name == 'first_quest_layered.zplay':
        frames_limited = 1_000_000
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


def save_test_results():
    runs_on, arch = args.ci.split('_') if args.ci else infer_gha_platform()
    json_result = {
        'runs_on': runs_on,
        'arch': arch,
    }
    if os.environ.get('CI'):
        json_result['ci'] = True
        json_result['ref'] = os.environ.get('GITHUB_REF')
        json_result['run_id'] = os.environ.get('GITHUB_RUN_ID')

    json_result['replays'] = []
    for replay, test_result in test_results.items():
        json_result['replays'].append({
            'name': replay.name,
            'success': test_result.success,
            'failing_frame': test_result.failing_frame,
        })

    test_results_path.write_text(json.dumps(json_result, indent=2))


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
    print(f'\nframes limited: {frames_limited_ratio * 100:.2f}%')

if args.print_shards:
    exit(0)


def run_replay_test(replay_file):
    result = SimpleNamespace(success=False, failing_frame=None, log=None, diff=None, fps=None)

    # TODO: fix this common-ish error, and whatever else is causing random failures.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    exe_name = 'zelda.exe' if os.name == 'nt' else 'zelda'
    exe_path = f'{args.build_folder}/{exe_name}'

    exe_args = [
        exe_path,
        f'-{mode}', replay_file,
        '-v1' if args.throttle_fps else '-v0',
        '-replay-exit-when-done',
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

    # Cap the length of a replay in CI.
    timeout = None
    if args.ci:
        timeout = max(60 + replay_data['estimated_duration'] * 3, 60 * 5)

    if frame_arg is not None and frame_arg < frames:
        frames_limited = frame_arg
    if frames_limited != frames:
        print(f"(-frame {frames_limited}, only doing {100 * frames_limited / frames:.2f}%) ", end='', flush=True)
        exe_args.extend(['-frame', str(frames_limited)])

    def fill_log(process_result, allegro_log_path):
        allegro_log = None
        if allegro_log_path and allegro_log_path.exists():
            allegro_log = allegro_log_path.read_text()
        return {
            'stdout': process_result.stdout,
            'stderr': process_result.stderr,
            'allegro': allegro_log,
        }

    allegro_log_path = None
    max_attempts = 5
    for i in range(0, max_attempts):
        allegro_log_path = pathlib.Path(args.build_folder) / f'{replay_file.stem}.{i}.log'
        start = timer()
        try:
            process_result = subprocess.run(exe_args,
                                            cwd=args.build_folder if os.name == 'nt' else None,
                                            env={
                                                **os.environ,
                                                'ALLEGRO_LEGACY_TRACE': allegro_log_path.name
                                            },
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            text=True,
                                            timeout=timeout)
            result.log = fill_log(process_result, allegro_log_path)
            if 'Replay is active' in process_result.stdout:
                # TODO: we only know the fps if the replay succeeded.
                if process_result.returncode == 0:
                    result.fps = int(frames_limited / (timer() - start))
                    result.success = True
                elif process_result.returncode != ASSERT_FAILED_EXIT_CODE:
                    print(f'process failed with unexpected code {process_result.returncode}')
                break
            if i != max_attempts - 1:
                print('did not start correctly, trying again...')
                sleep(1)
        except subprocess.TimeoutExpired as e:
            result.log = fill_log(e, allegro_log_path)
            result.log['stdout'] = f'{e}\n\n{result.log["stdout"]}'
            return result
        finally:
            if allegro_log_path:
                allegro_log_path.unlink(missing_ok=True)

    if not args.update and process_result.returncode == 120:
        roundtrip_path = pathlib.Path(f'{replay_file}.roundtrip')
        if os.path.exists(roundtrip_path):
            with open(replay_file) as f:
                fromlines = f.readlines()
            with open(roundtrip_path) as f:
                tolines = f.readlines()
            diff_iter = difflib.context_diff(
                fromlines, tolines,
                str(replay_file.relative_to(replays_dir)),
                str(roundtrip_path.relative_to(replays_dir)),
                n=3)
            trimmed_diff_lines = [x for _, x in zip(range(100), diff_iter)]
            result.diff = ''.join(trimmed_diff_lines)
        else:
            result.diff = 'missing roundtrip file, cannnot diff'

    if not result.success:
        failing_frame_match = re.match(r'.*expected:\n.*?(\d+)', result.log['stderr'], re.DOTALL)
        if failing_frame_match:
            result.failing_frame = int(failing_frame_match.group(1))
        else:
            print('could not find failing frame')

    return result


test_results = {}
for test in tests:
    test_results[test] = SimpleNamespace(success=None, failing_frame=None, log=None, diff=None, fps=None)

print(f'running {len(tests)} replays\n')
iteration_count = 0
for i in range(args.retries + 1):
    if all(r.success for r in test_results.values()):
        break

    if i != 0:
        print('\nretrying failures...\n')

    for test in [t for t in tests if not test_results[t].success]:
        print(f'= {test.relative_to(replays_dir)} ... ', end='', flush=True)
        start = timer()
        result = test_results[test] = run_replay_test(test)
        duration = timer() - start
        status_emoji = '✅' if result.success else '❌'

        message = f'{status_emoji} {time_format(duration)}'
        if result.fps != None:
            message += f', {result.fps} fps'
        print(message)

        # Only print on failure and last attempt.
        if not test_results[test].success and i == args.retries:
            print('\nallegro:')
            print(result.log['allegro'])
            print('stdout:')
            print(result.log['stdout'])
            print('\nstderr:')
            print(result.log['stderr'])
            print('\ndiff:')
            print(result.diff)


save_test_results()

if mode == 'assert':
    failing_tests = [test for test, r in test_results.items() if not r.success]

    if len(failing_tests) == 0:
        print('all replay tests passed')
    else:
        print(f'{len(failing_tests)} replay tests failed')

        print('\nto collect baseline artifacts and then generate a report, run the following commands:\n')
        print(f'python {script_dir}/run_test_workflow.py --test_results {replays_dir} --token <1>')
        print(f'python {script_dir}/compare_replays.py --workflow_run <2> --local {replays_dir}')
        print('\n')
        print('<1>: github personal access token, with write actions access')
        print('<2>: workflow run id printed from the previous command')

        exit(1)
