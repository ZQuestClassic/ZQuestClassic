# For more information, see replay.h

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
import difflib
import pathlib

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', default='build/Debug')
parser.add_argument('--filter')
parser.add_argument('--throttle_fps', action='store_true')
parser.add_argument('--update', action='store_true')
parser.add_argument('--retries', type=int, default=0)
parser.add_argument('--frame', type=int)
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


def run_replay_test(replay_file):
    # TODO: fix this common-ish error, and whatever else is causing random failures.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    # Assertion failed: (mutex), function al_lock_mutex, file threads.c, line 324.
    attempts = 0
    while attempts <= args.retries:
        attempts += 1
        exe_name = 'zelda.exe' if os.name == 'nt' else 'zelda'
        exe_path = f'{args.build_folder}/{exe_name}'
        exe_args = [
            exe_path,
            '-update' if args.update else '-assert', replay_file,
            '-v1' if args.throttle_fps else '-v0',
        ]
        if args.frame is not None:
            exe_args.extend(['-frame', str(args.frame)])

        process = subprocess.Popen(exe_args,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   text=True)
        stdout, stderr = process.communicate()

        print('❌' if process.returncode != 0 else '✅')

        if process.returncode != 0:
            print('stdout:')
            print(stdout)
            print('\nstderr:')
            print(stderr)
            continue
        else:
            break

    if not args.update and process.returncode == 120:
        roundtrip_path = pathlib.Path(f'{replay_file}.roundtrip')
        if os.path.exists(roundtrip_path):
            with open(replay_file) as f:
                fromlines = f.readlines()
            with open(roundtrip_path) as f:
                tolines = f.readlines()
            sys.stdout.writelines(difflib.context_diff(
                fromlines, tolines,
                str(replay_file.relative_to(replays_dir)),
                str(roundtrip_path.relative_to(replays_dir))))
        else:
            print('missing roundtrip file, cannnot print diff')

    return process.returncode == 0


num_failures = 0
print(f'running {len(tests)} replay tests')
for test in tests:
    print(f'= {test.relative_to(replays_dir)} ... ', end='', flush=True)
    if not run_replay_test(test):
        num_failures += 1

if num_failures == 0:
    print('all replay tests passed')
else:
    print(f'{num_failures} replay tests failed')
    exit(1)
