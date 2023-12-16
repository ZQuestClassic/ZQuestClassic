# Debugs problems with JIT by comparing against non-JIT execution, and finds the first instance
# of an instruction resulting in unexpected registers/stack.

import argparse
import os
import sys
import json
import shutil
from pathlib import Path
import subprocess

# TODO :(
# from ..tests.common import ReplayTestResults
from typing import List, Literal, Tuple, Optional, Any
from dataclasses import dataclass, field
import dataclasses

@dataclass
class RunResult:
    name: str
    directory: str
    success: bool = False
    stopped: bool = False
    exit_code: int = None
    duration: float = None
    fps: int = None
    frame: int = None
    num_frames: int = None
    failing_frame: int = None
    unexpected_gfx_frames: List[int] = None
    unexpected_gfx_segments: List[Tuple[int, int]] = None
    unexpected_gfx_segments_limited: List[Tuple[int, int]] = None
    exceptions: List[str] = field(default_factory=list) 
    # Only for compare report.
    snapshots: List[Any] = None

@dataclass
class ReplayTestResults:
    runs_on: Literal['windows-2022', 'macos-12', 'ubuntu-22.04']
    arch: Literal['x64', 'win32', 'intel']
    ci: bool
    workflow_run_id: Optional[int]
    git_ref: Optional[str]
    zc_version: str
    time: str
    runs: List[List[RunResult]]
    # Only for compare report.
    label: str = None

    def __post_init__(self):
        if self.runs and isinstance(self.runs[0][0], dict):
            deserialized = []
            for runs in self.runs:
                for run in runs:
                    # Old property, will error when creating dataclass.
                    run.pop('diff', None)
                deserialized.append([RunResult(**run) for run in runs])
            self.runs = deserialized

    def to_json(self, indent=2):
        as_dict = dataclasses.asdict(self)
        return json.dumps(as_dict, indent=indent)

parser = argparse.ArgumentParser(
    description='Helps debug why JIT is not producing expected results.')
parser.add_argument('--build_folder',
                    default='build/Release',
                    help='The location of the build folder. ex: build/Release')
parser.add_argument('--replay_path', required=True)

args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

build_folder = Path(args.build_folder)
replay_path = Path(args.replay_path)
test_results_folder = root_dir / '.tmp/jit_runtime_debug_test_results'
script_debug_folder = build_folder / 'zscript-debug'

def clear_dir(dir: Path):
    if dir.exists():
        shutil.rmtree(dir)

def run_replay(args: List[str]):
    clear_dir(script_debug_folder)
    clear_dir(test_results_folder)
    return subprocess.run([
        sys.executable,
        root_dir / 'tests/run_replay_tests.py',
        '--build_folder', build_folder,
        '--test_results_folder', test_results_folder,
        '--no_report_on_failure',
        *args,
    ])

p = run_replay([
    '--jit',
    # TODO: find which frame script _first_ fail, and only print state of each instruction on that frame (and the one prior?)
    # '--extra_args', '-script-runtime-debug 1',
    '--filter', replay_path,
])
if p.returncode == 0:
    print('replay passed with JIT, nothing to debug')
    exit(0)

test_results_path = test_results_folder / 'test_results.json'
test_results_json = json.loads(test_results_path.read_text('utf-8'))
test_results = ReplayTestResults(**test_results_json)
failing_runs = [r for r in test_results.runs[-1] if not r.success]
# exit(0)

for run in failing_runs:
    print('processing', run.name)
    frame = next((f for f in [run.failing_frame, run.frame] if f), 0)

    # Get baseline
    p = run_replay([
        '--no-jit',
        '--frame', str(frame),
        '--extra_args', '-script-runtime-debug 2',
        # TODO should really have replay path on RunResult
        '--filter', run.name,
    ])
    if p.returncode:
        print('replay unexpectedly failed without JIT, cannot collect baseline')
        exit(1)

    script_debug_baseline_folder = root_dir / '.tmp/jit_runtime_zscript_debug_baseline'
    clear_dir(script_debug_baseline_folder)
    shutil.move(script_debug_folder, script_debug_baseline_folder)

    # Get failing JIT
    p = run_replay([
        '--jit',
        '--frame', str(frame),
        '--extra_args', '-script-runtime-debug 2',
        # TODO should really have replay path on RunResult
        '--filter', run.name,
    ])

    debug_files = list(script_debug_baseline_folder.rglob('debug-*.txt'))
    debug_files.sort(key=lambda x: int(x.stem.split('debug-')[1]))
    for baseline_path in debug_files:
        rel = baseline_path.relative_to(script_debug_baseline_folder)
        jit_path = script_debug_folder / rel

        baseline_text = baseline_path.read_text()
        jit_text = jit_path.read_text()
        if baseline_text != jit_text:
            print('\n=== found difference for', rel)
            print('baseline path:', baseline_path)
            print('jit path:', jit_path)
            print('diff:\n')

            baseline_lines = baseline_text.splitlines()
            jit_lines = jit_text.splitlines()

            bad_index = None
            for i in range(len(baseline_lines)):
                baseline = baseline_lines[i]
                jit = jit_lines[i]
                if baseline != jit:
                    bad_index = i
                    break
            
            start = max(0, bad_index - 10)
            end = min(len(baseline_lines), bad_index + 10)
            for i in range(start, end):
                baseline = baseline_lines[i]
                jit = jit_lines[i]
                if baseline != jit:
                    print(f'-{baseline}')
                    print(f'+{jit}')
                else:
                    print(baseline)
