# Debugs problems with JIT by comparing against non-JIT execution, and finds the first instance
# of an instruction resulting in unexpected registers/stack.
#
# To debug a replay that is failing only in JIT, run:
#   python scripts/jit_runtime_debug.py --replay_path tests/replays/stellar_seas_randomizer.zplay

import argparse
import dataclasses
import json
import os
import shutil
import subprocess
import sys

from dataclasses import dataclass, field
from pathlib import Path

# TODO :(
# from ..tests.replays import ReplayTestResults
from typing import Literal, Optional


@dataclass
class RunResult:
    name: str
    directory: str
    path: str
    success: bool = False
    stopped: bool = False
    exit_code: int = None
    duration: float = None
    fps: int = None
    frame: int = None
    num_frames: int = None
    failing_frame: int = None
    unexpected_gfx_frames: list[int] = None
    unexpected_gfx_segments: list[tuple[int, int]] = None
    unexpected_gfx_segments_limited: list[tuple[int, int]] = None
    exceptions: list[str] = field(default_factory=list)
    # Only for compare report.
    snapshots: list = None


@dataclass
class ReplayTestResults:
    runs_on: Literal['windows-2022', 'macos-13', 'ubuntu-22.04']
    arch: Literal['x64', 'win32', 'intel']
    ci: bool
    workflow_run_id: Optional[int]
    git_ref: Optional[str]
    zc_version: str
    time: str
    runs: list[list[RunResult]]
    # Only for compare report.
    label: str = None

    def __post_init__(self):
        if self.runs and isinstance(self.runs[0][0], dict):
            deserialized = []
            for runs in self.runs:
                for run in runs:
                    # Old property, will error when creating dataclass.
                    run.pop('diff', None)
                # New property.
                if 'path' not in run:
                    run['path'] = ''
                deserialized.append([RunResult(**run) for run in runs])
            self.runs = deserialized

    def to_json(self, indent=2):
        as_dict = dataclasses.asdict(self)
        return json.dumps(as_dict, indent=indent)


parser = argparse.ArgumentParser(
    description='Helps debug why JIT is not producing expected results.'
)
parser.add_argument(
    '--build_folder',
    default='build/Release',
    help='The location of the build folder. ex: build/Release',
)
parser.add_argument(
    '--baseline_build_folder',
    help='The location of the build folder to use to collect non-JIT baseline. Useful to speed things up by giving a non-web build',
)
parser.add_argument('--replay_path', required=True)
parser.add_argument('--test_this_script', action='store_true')

args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

build_folder = Path(args.build_folder)
if args.baseline_build_folder:
    baseline_build_folder = Path(args.baseline_build_folder)
else:
    baseline_build_folder = build_folder
replay_path = Path(args.replay_path)


def find_index_containing(lines, str):
    for i, line in enumerate(lines):
        if str in line:
            return i

    raise Exception(f'did not find {str}')


def find_last_index_containing(lines, str, n):
    count = 0
    for i, line in reversed(list(enumerate(lines))):
        if str in line:
            if count == n:
                return i
            count += 1

    raise Exception(f'did not find {str}')


def clear_dir(dir: Path):
    if dir.exists():
        shutil.rmtree(dir)


def run_replay(args: list[str]):
    return subprocess.run(
        [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--no_report_on_failure',
            *args,
        ]
    )


def load_test_results(path: Path):
    test_results_json = json.loads(path.read_text('utf-8'))
    return ReplayTestResults(**test_results_json)


def copy_and_trim_zplay(from_path: Path, to_path: Path, frame_limit: int):
    lines = from_path.read_text().splitlines()
    for i, line in enumerate(lines):
        if line.startswith('M'):
            if line.startswith('M qst '):
                qst_path = line.replace('M qst ', '')
                lines[i] = f'M qst {from_path.parent / qst_path}'
            elif line.startswith('M sav '):
                sav_path = line.replace('M sav ', '')
                lines[i] = f'M sav {from_path.parent / sav_path}'
            continue

        frame = int(line.split(' ')[1])
        if frame > frame_limit:
            lines = lines[:i]
            lines.append('')
            break

    to_path.write_text('\n'.join(lines))


# Get the roundtrip file, cut it at the first failure, then find the last line denoting a script starting.
# Then find the previous time that script was started. That frame is what we want.
def find_frame_where_script_broke(test_results_folder: Path):
    test_results = load_test_results(test_results_folder / 'test_results.json')
    test_run = test_results.runs[-1][0]
    roundtrip_path = next(
        (test_results_folder / test_run.directory).glob('*.roundtrip')
    )
    roundtrip_lines = roundtrip_path.read_text().splitlines()
    first_failure_index = find_index_containing(roundtrip_lines, '«')
    first_failure = roundtrip_lines[first_failure_index]
    print('first failure:', first_failure)
    if 'result:' in first_failure or 'trace:' in first_failure:
        # If the result of a script was different than expected, then the issue is not a
        # prior frame but instead in the same frame as the failure frame.
        type, frame, data = first_failure.split(' ', 2)
        return int(frame)

    roundtrip_lines = roundtrip_lines[0:first_failure_index]
    last_index = find_last_index_containing(
        roundtrip_lines, '=== running script type', 0
    )
    needle = roundtrip_lines[last_index].split('===')[1]
    second_last_index = find_last_index_containing(roundtrip_lines, needle, 1)
    type, frame, data = roundtrip_lines[second_last_index].split(' ', 2)
    return int(frame)


extra_args = (
    '-jit-precompile -replay-save-result-every-frame -replay-fail-assert-instant'
)
if args.test_this_script:
    extra_args += ' -jit-runtime-debug-test-force-bug'
test_results_folder_0 = root_dir / '.tmp/jit_runtime_debug_test_results_0'
clear_dir(test_results_folder_0)
p = run_replay(
    [
        '--test_results_folder',
        test_results_folder_0,
        '--build_folder',
        build_folder,
        '--jit',
        '--extra_args',
        extra_args,
        replay_path,
    ]
)
if p.returncode == 0:
    print('replay passed with JIT, nothing to debug')
    exit(0)

test_results = load_test_results(test_results_folder_0 / 'test_results.json')
failing_runs = [r for r in test_results.runs[-1] if not r.success]

for run in failing_runs:
    print('processing', run.name)
    frame = next((f for f in [run.failing_frame, run.frame] if f), 0)

    # Copy the failing replay, and update it with script-level runtime debugging.
    test_zplay_path = root_dir / '.tmp/jit_runtime_debug_test_replay.zplay'
    print(f'failed at frame {frame}, so cutting replay there')
    copy_and_trim_zplay(Path(run.path), test_zplay_path, frame)

    extra_args = '-script-runtime-debug 1'
    test_results_folder_1 = root_dir / '.tmp/jit_runtime_debug_test_results_1'
    clear_dir(test_results_folder_1)
    p = run_replay(
        [
            '--test_results_folder',
            test_results_folder_1,
            '--build_folder',
            baseline_build_folder,
            '--no-jit',
            '--extra_args',
            extra_args,
            '--update',
            test_zplay_path,
        ]
    )
    if p.returncode:
        print('replay unexpectedly failed without JIT, cannot collect baseline')
        exit(1)

    # Get failing JIT
    extra_args = '-script-runtime-debug 1 -jit-precompile -replay-save-result-every-frame -replay-fail-assert-instant'
    if args.test_this_script:
        extra_args += ' -jit-runtime-debug-test-force-bug'
    test_results_folder_2 = root_dir / '.tmp/jit_runtime_debug_test_results_2'
    clear_dir(test_results_folder_2)
    p = run_replay(
        [
            '--test_results_folder',
            test_results_folder_2,
            '--build_folder',
            build_folder,
            '--jit',
            '--frame',
            str(frame),
            '--extra_args',
            extra_args,
            test_zplay_path,
        ]
    )

    # The script runtime state was bad at failing_frame, which means execution in some previous
    # frame is where things broke. We need to find the last time the bad script ran.
    frame = find_frame_where_script_broke(test_results_folder_2)
    print(f'script seems to have broken at frame: {frame}')

    # Update the replay again, but with instruction-level runtime debugging.
    extra_args = f'-script-runtime-debug 2 -script-runtime-debug-frame {frame}'
    test_results_folder_3 = root_dir / '.tmp/jit_runtime_debug_test_results_3'
    clear_dir(test_results_folder_3)
    p = run_replay(
        [
            '--test_results_folder',
            test_results_folder_3,
            '--build_folder',
            baseline_build_folder,
            '--no-jit',
            '--extra_args',
            extra_args,
            '--update',
            test_zplay_path,
        ]
    )
    if p.returncode:
        print('replay unexpectedly failed without JIT, cannot collect baseline')
        exit(1)

    # Get failing JIT
    extra_args = f'-script-runtime-debug 2 -script-runtime-debug-frame {frame} -jit-precompile -replay-fail-assert-instant'
    if args.test_this_script:
        extra_args += ' -jit-runtime-debug-test-force-bug'
    test_results_folder_4 = root_dir / '.tmp/jit_runtime_debug_test_results_4'
    clear_dir(test_results_folder_4)
    p = run_replay(
        [
            '--test_results_folder',
            test_results_folder_4,
            '--build_folder',
            build_folder,
            '--jit',
            # '--frame', str(frame),
            '--extra_args',
            extra_args,
            test_zplay_path,
        ]
    )

    test_results = load_test_results(test_results_folder_4 / 'test_results.json')
    test_run = test_results.runs[-1][0]
    roundtrip_path = next(
        (test_results_folder_4 / test_run.directory).glob('*.roundtrip')
    )
    roundtrip_lines = roundtrip_path.read_text().splitlines()
    bad_line_index = None
    for i, line in enumerate(roundtrip_lines):
        if '«' in line:
            bad_line_index = i
            break
    if bad_line_index == None:
        raise Exception('did not find marker: «')

    print('found first bad script failure:\n')
    print(roundtrip_lines[bad_line_index - 2])
    print(roundtrip_lines[bad_line_index - 1])
    but_was, shoulda_been = roundtrip_lines[bad_line_index - 0].split('«')
    print(f'shoulda been:\n\t{shoulda_been.strip()}\nbut was:\n\t{but_was.strip()}')
    print('\ntip: copy/paste this in an editor with word wrap off')
    print()
    print(f'for more, see {roundtrip_path}')

    # Only bother showing one bad thing at a time.
    break
