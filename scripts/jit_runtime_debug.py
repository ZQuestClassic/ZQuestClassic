# Debugs problems with JIT by comparing against non-JIT execution, and finds the first instance
# of an instruction resulting in unexpected registers/stack.
#
# To debug a replay that is failing only in JIT, run:
#   python scripts/jit_runtime_debug.py --replay_path tests/replays/stellar_seas_randomizer.zplay

import argparse
import json
import os
import re
import shutil
import signal
import subprocess
import sys

from pathlib import Path

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../tests'))
from replays import ReplayTestResults

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


def clear_dir(dir: Path):
    if dir.exists():
        shutil.rmtree(dir)


def run_replay(args: list[str], timeout_s=1200, attempts=2):
    # A hung web run (the harness has been observed blocking forever after the
    # engine exits) must not stall a whole bisect: kill the process group after
    # a generous timeout and retry once.
    cmd = [
        sys.executable,
        str(root_dir / 'tests/run_replay_tests.py'),
        '--no_report_on_failure',
        *[str(a) for a in args],
    ]
    for attempt in range(attempts):
        p = subprocess.Popen(cmd, start_new_session=True)
        try:
            p.wait(timeout=timeout_s)
            return p
        except subprocess.TimeoutExpired:
            print(f'replay run timed out after {timeout_s}s; killing (attempt {attempt + 1}/{attempts})')
            try:
                os.killpg(os.getpgid(p.pid), signal.SIGKILL)
            except ProcessLookupError:
                pass
            p.wait()
            kill_orphans()
    raise Exception(f'replay run timed out {attempts} times')


def kill_orphans():
    # Clean up what the group kill can miss (detached browser, webserver holding
    # port 8000) - but only processes actually orphaned by that kill (ppid 1), so
    # a concurrent replay session or dev server on this machine is left alone.
    ps = subprocess.run(
        ['ps', '-eo', 'pid=,ppid=,command='], capture_output=True, text=True
    ).stdout
    for line in ps.splitlines():
        parts = line.split(None, 2)
        if len(parts) != 3:
            continue
        pid, ppid, command = parts
        if ppid == '1' and ('Chrome for Testing' in command or 'webserver.mjs' in command):
            try:
                os.kill(int(pid), signal.SIGKILL)
            except (ProcessLookupError, PermissionError):
                pass


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


def run_replay_folder(
    folder: Path, build: Path, jit_flag: str, extra: str, replay: Path, update=False
):
    """Runs one replay and returns (returncode, its RunResult)."""
    clear_dir(folder)
    a = [
        '--test_results_folder',
        folder,
        '--build_folder',
        build,
        jit_flag,
        '--extra_args',
        extra,
    ]
    if update:
        a.append('--update')
    a.append(replay)
    p = run_replay(a)
    run = load_test_results(folder / 'test_results.json').runs[-1][0]
    return p.returncode, run


force_bug = ' -jit-runtime-debug-test-force-bug' if args.test_this_script else ''
tmp = root_dir / '.tmp/jit_runtime_debug'
tmp.mkdir(parents=True, exist_ok=True)

# 1. Reproduce the failure with the JIT on.
rc, detect = run_replay_folder(
    tmp / 'detect',
    build_folder,
    '--jit',
    f'-jit-precompile -replay-save-result-every-frame -replay-fail-assert-instant{force_bug}',
    replay_path,
)
if rc == 0:
    print('replay passed with JIT, nothing to debug')
    exit(0)

# 2. Trim the replay to the failing frame - every subsequent run stops there.
frame = next((f for f in [detect.failing_frame, detect.frame] if f), 0)
if not frame:
    # A hard crash (e.g. the renderer process died) reports no failing frame;
    # trimming to frame 0 would leave nothing to bisect. This tool only handles
    # value miscompiles, where the replay asserts at a recorded frame.
    print('no failing frame was recorded (a hard crash, not a value divergence?)')
    print('this tool needs a replay that fails an assert; cannot proceed')
    exit(1)
trimmed = tmp / 'trimmed.zplay'
print(f'failed around frame {frame}, trimming replay there')
copy_and_trim_zplay(Path(detect.path), trimmed, frame)


# 3. Bisect for the FIRST run_script call whose jitted execution differs from the
# interpreter - not the first that trips a replay assert (a benign early divergence is
# more useful to see than the later point it happens to break the replay).
#
# Record the interpreter's per-script state as the baseline, then ask, per candidate:
# does jitting calls [0, hi) make any script's state diverge from it? -jit-run-hi H lets
# calls [0, H) use the JIT and forces the rest to the interpreter (state is checkpointed
# after every run_script, so this is a clean split), and the answer is monotonic in H, so
# it bisects.
rc, _ = run_replay_folder(
    tmp / 'baseline_state',
    baseline_build_folder,
    '--no-jit',
    '-script-runtime-debug 1',
    trimmed,
    update=True,
)
if rc:
    print('replay unexpectedly failed without JIT, cannot collect baseline')
    exit(1)


def diverges_with_hi(hi: int) -> bool:
    rc, run = run_replay_folder(
        tmp / 'bisect',
        build_folder,
        '--jit',
        f'-jit-precompile -script-runtime-debug 1 -jit-run-hi {hi}{force_bug}',
        trimmed,
    )
    roundtrip = next((tmp / 'bisect' / run.directory).glob('*.roundtrip'), None)
    if roundtrip is None:
        # A .roundtrip is only written when an assert fails, so a clean pass
        # has none. A FAILED run with none died before writing it - a hard
        # crash under this JIT split is as diverging as a value mismatch, so
        # bisect toward it instead of silently counting the crash as a match.
        if rc != 0:
            print(f'-jit-run-hi {hi} crashed without a roundtrip; treating as divergence')
            return True
        return False
    return '«' in roundtrip.read_text()


print('bisecting which run_script call first diverges from the interpreter when jitted')
hi = 1
while not diverges_with_hi(hi):
    hi *= 2
    if hi > (1 << 30):
        raise Exception('could not isolate a diverging run_script call')
lo = hi // 2  # the previous power of two matched the interpreter (lo=0 when hi==1)
while hi - lo > 1:
    mid = (lo + hi) // 2
    if diverges_with_hi(mid):
        hi = mid
    else:
        lo = mid
failing_call = hi - 1
print(f'run_script call #{failing_call} is the first to diverge from the interpreter')


# 4. Find the frame that call runs on, to scope the per-instruction debug to it.
rc, run = run_replay_folder(
    tmp / 'find_frame',
    build_folder,
    '--jit',
    f'-jit-precompile -jit-run-hi {failing_call + 1} -jit-run-log-call {failing_call}{force_bug}',
    trimmed,
)
call_frame = None
for name in ['stdout.txt', 'stderr.txt', 'allegro.log']:
    log = tmp / 'find_frame' / run.directory / name
    if log.exists():
        m = re.search(r'\[jit-run-call\] call=\d+ frame=(\d+)', log.read_text())
        if m:
            call_frame = int(m.group(1))
            break
if call_frame is None:
    raise Exception('could not determine the frame of the failing call')
print(f'call #{failing_call} runs on frame {call_frame}')


# 5. Instruction-level diff, but only for that one call. The interpreter baseline
# (--update) records the expected per-instruction state at that frame; the JIT run
# repeats it with ONLY the diverging call jitted (-jit-run-lo/-jit-run-hi).
debug = f'-script-runtime-debug 2 -script-runtime-debug-frame {call_frame}'
rc, _ = run_replay_folder(
    tmp / 'baseline', baseline_build_folder, '--no-jit', debug, trimmed, update=True
)
if rc:
    print('replay unexpectedly failed without JIT, cannot collect baseline')
    exit(1)

rc, run = run_replay_folder(
    tmp / 'jit_debug',
    build_folder,
    '--jit',
    f'{debug} -jit-precompile -replay-fail-assert-instant'
    f' -jit-run-lo {failing_call} -jit-run-hi {failing_call + 1}{force_bug}',
    trimmed,
)

roundtrip_path = next((tmp / 'jit_debug' / run.directory).glob('*.roundtrip'), None)
if roundtrip_path is None:
    raise Exception(
        f'the isolated run produced no .roundtrip (exit code {rc}); call'
        f' #{failing_call} likely crashes instead of desyncing - check the logs'
        f' in {tmp / "jit_debug" / run.directory}'
    )
roundtrip_lines = roundtrip_path.read_text().splitlines()
bad_line_index = next((i for i, l in enumerate(roundtrip_lines) if '«' in l), None)
if bad_line_index is None:
    raise Exception('did not find marker: «')

print('found first bad script failure:\n')
print(roundtrip_lines[bad_line_index - 2])
print(roundtrip_lines[bad_line_index - 1])
but_was, shoulda_been = roundtrip_lines[bad_line_index].split('«')
print(f'shoulda been:\n\t{shoulda_been.strip()}\nbut was:\n\t{but_was.strip()}')

# Surface the ZScript source location of the miscompiled instruction.
source_match = re.search(r'(\S+\.z[sh]:\d+)', roundtrip_lines[bad_line_index])
if source_match:
    print(f'\nmiscompiled instruction is at: {source_match.group(1)}')

print('\ntip: copy/paste this in an editor with word wrap off')
print()
print(f'for more, see {roundtrip_path}')
