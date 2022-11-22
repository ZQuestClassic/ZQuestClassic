# For more information, see replay.h

# Before this will work, you need to install git-lfs:
#     https://git-lfs.github.com/

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
import difflib
import pathlib
import shutil
from time import sleep
from timeit import default_timer as timer

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', default='build/Debug')
parser.add_argument('--filter')
parser.add_argument('--throttle_fps', action='store_true')
parser.add_argument('--update', action='store_true')
parser.add_argument('--snapshot')
parser.add_argument('--retries', type=int, default=0)
parser.add_argument('--frame', type=int)
parser.add_argument('--ci')
args = parser.parse_args()

script_dir = os.path.dirname(os.path.realpath(__file__))
replays_dir = os.path.join(script_dir, 'replays')
tests = list(pathlib.Path(replays_dir).glob('*.zplay'))
if args.filter:
    tests = [t for t in tests if args.filter in str(
        t.relative_to(replays_dir))]
    if len(tests) == 0:
        print('no tests matched filter')
        exit(1)
if args.ci:
    skip_in_ci = [
        # None!
    ]
    tests = [t for t in tests if t.name not in skip_in_ci]


def read_last_contentful_line(file):
    f = pathlib.Path(test).open('rb')
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


def run_replay_test(replay_file):
    # TODO: fix this common-ish error, and whatever else is causing random failures.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    exe_name = 'zelda.exe' if os.name == 'nt' else 'zelda'
    exe_path = f'{args.build_folder}/{exe_name}'

    mode = 'assert'
    if args.update:
        mode = 'update'

    exe_args = [
        exe_path,
        f'-{mode}', replay_file,
        '-v1' if args.throttle_fps else '-v0',
        '-replay-exit-when-done',
    ]
    if args.frame is not None:
        exe_args.extend(['-frame', str(args.frame)])
    if args.snapshot is not None:
        exe_args.extend(['-snapshot', args.snapshot])
    
    last_step = read_last_contentful_line(replay_file)
    num_frames = int(last_step.split(' ')[1])
    num_frames_checked = num_frames

    # Cap the length of a replay in CI.
    if args.ci:
        max_duration = 4 * 60
        fps = 800
        estimated_duration = num_frames / fps
        if estimated_duration > max_duration:
            num_frames_checked = fps * max_duration
            exe_args.extend(['-frame', str(num_frames_checked)])
            print(f"-frame {num_frames_checked}, only doing {100 * num_frames_checked / num_frames:.2f}% ... ", end='', flush=True)
            estimated_duration = max_duration
        timeout = 5 + estimated_duration * 1.2
    else:
        timeout = None

    fps = None
    for _ in range(0, 5):
        try:
            start = timer()
            process_result = subprocess.run(exe_args,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            text=True,
                                            timeout=timeout)
            if 'Replay is active' in process_result.stdout:
                # TODO: we only know the fps if the replay succeeded.
                if process_result.returncode == 0:
                    fps = int(num_frames_checked / (timer() - start))
                break
            print('did not start correctly, trying again...')
            sleep(1)
        except subprocess.TimeoutExpired as e:
            return False, f'{e}\n\n{e.stdout}', e.stderr, None

    diff = None
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
            diff = ''.join(trimmed_diff_lines)
        else:
            diff = 'missing roundtrip file, cannnot diff'

    return process_result.returncode == 0, process_result.stdout, process_result.stderr, diff, fps


test_states = {}
for test in tests:
    test_states[test] = False

    # qst files need to be relative to the build folder, so copy them over.
    maybe_qst_path = test.with_suffix('.qst')
    if maybe_qst_path.exists():
        shutil.copy2(maybe_qst_path, args.build_folder)

print(f'running {len(tests)} replay tests\n')
iteration_count = 0
for i in range(args.retries + 1):
    if all(test_states.values()):
        break

    if i != 0:
        print('\nretrying failures...\n')

    for test in [t for t in tests if not test_states[t]]:
        print(f'= {test.relative_to(replays_dir)} ... ', end='', flush=True)
        test_states[test], stdout, stderr, diff, fps = run_replay_test(test)
        status_emoji = '✅' if test_states[test] else '❌'
        if fps != None:
            print(f'{status_emoji} {fps} fps')
        else:
            print(status_emoji)

        # Only print on failure and last attempt.
        if not test_states[test] and i == args.retries:
            print('stdout:')
            print(stdout)
            print('\nstderr:')
            print(stderr)
            print('\ndiff:')
            print(diff)


num_failures = sum(not state for state in test_states.values())
if num_failures == 0:
    print('all replay tests passed')
else:
    print(f'{num_failures} replay tests failed')
    exit(1)
