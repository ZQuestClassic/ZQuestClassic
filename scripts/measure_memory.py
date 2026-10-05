#!/usr/bin/env python3

"""
Measure peak memory usage of zplayer and zeditor across a set of large quests.

For each quest:

  * zplayer  - plays the quest's replay (headless, in -replay mode rather
               than -assert, so it keeps going if it desyncs), for realistic
               in-game usage.
  * zeditor  - loads the quest in the editor (headless -export-strings, which
               does a full editor quest load and then exits).

The metric is the peak physical memory footprint on macOS (what Activity
Monitor reports as "Memory"), and the peak resident set size elsewhere. With
--runs, the highest of the runs is reported.

Examples:
  # Measure the default quests with build/Release.
  python scripts/measure_memory.py

  # Save a baseline, change something, rebuild, then compare against it.
  python scripts/measure_memory.py --json .tmp/memory_before.json
  python scripts/measure_memory.py --baseline .tmp/memory_before.json

  # Faster: cap every replay at 20000 frames, only the yuurand quest.
  python scripts/measure_memory.py --max-frames 20000 --filter yuurand

  # Approximate a 1 GB device, where the JIT compile memory budget is ~32 MB.
  python scripts/measure_memory.py --zplayer-args="-jit-compile-memory-budget-mb 32"
"""

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

script_dir = Path(__file__).resolve().parent
root_dir = script_dir.parent

# Paths are relative to the repo root. Every quest needs a replay. Quests whose
# qst or replay file is missing (e.g. ones only in a local .tmp folder, like
# replay uploads) are skipped. If the qst path recorded in a replay doesn't
# resolve next to it, `qst` is used instead (see `prepare_replay`).
QUESTS = [
    {
        'name': 'yuurand',
        'qst': 'tests/replays/yuurand/yuurand.qst',
        'replay': 'tests/replays/yuurand/yuurand.zplay',
    },
    {
        'name': 'stellar_seas_randomizer',
        'qst': 'tests/replays/stellar_seas/stellar_seas_randomizer.qst',
        'replay': 'tests/replays/stellar_seas/stellar_seas_randomizer.zplay',
    },
    {
        'name': 'terror_of_necromancy_demo6',
        'qst': 'tests/replays/terror_of_necromancy_demo6/terror_of_necromancy_demo6.qst',
        'replay': 'tests/replays/terror_of_necromancy_demo6/terror_of_necromancy_demo6_05_of_54.zplay',
    },
    {
        'name': 'crucible_quest',
        'qst': 'tests/replays/crucible_quest/crucible_quest.qst',
        'replay': 'tests/replays/crucible_quest/crucible_quest.zplay',
    },
    {
        'name': 'hollow_forest',
        'qst': 'tests/replays/hollow_forest.qst',
        'replay': 'tests/replays/hollow_forest.zplay',
    },
    {
        'name': 'The ROSEMASTER',
        'qst': '.tmp/replay_uploads/219D5C45D53EE92C96821EF3821BB201/219D5C45D53EE92C96821EF3821BB201.qst',
        'replay': '.tmp/replay_uploads/219D5C45D53EE92C96821EF3821BB201/a3174566-e01c-4c74-b627-a472963f1ff6.zplay',
    },
]


def find_exe(build_folder: Path, name: str) -> Path:
    ext = '.exe' if os.name == 'nt' else ''
    exe = build_folder / f'{name}{ext}'
    if exe.exists():
        return exe
    found = next(iter(sorted(build_folder.glob(f'**/{name}{ext}'))), None)
    if found:
        return found
    print(f'error: could not find {name} under {build_folder}', file=sys.stderr)
    print(
        f'       pass --build_folder pointing at a folder containing {name}.',
        file=sys.stderr,
    )
    sys.exit(1)


def metric_name() -> str:
    return 'peak footprint' if sys.platform == 'darwin' else 'peak RSS'


def run_measured(cmd: list, cwd: Path) -> dict:
    """Runs cmd to completion and returns its peak memory (in bytes) and exit code."""
    start = time.time()
    if sys.platform == 'darwin':
        # /usr/bin/time -l reports the peak physical footprint, which (unlike
        # max RSS) excludes clean, shared pages such as the mapped binary.
        p = subprocess.run(
            ['/usr/bin/time', '-l', *map(str, cmd)],
            cwd=cwd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            text=True,
            errors='replace',
        )
        m = re.search(r'^\s*(\d+)\s+peak memory footprint', p.stderr, re.M)
        if not m:
            m = re.search(r'^\s*(\d+)\s+maximum resident set size', p.stderr, re.M)
        peak = int(m.group(1)) if m else None
        code = p.returncode
    elif os.name == 'posix':
        p = subprocess.Popen(
            [str(c) for c in cmd],
            cwd=cwd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        _, status, rusage = os.wait4(p.pid, 0)
        p.returncode = os.waitstatus_to_exitcode(status)
        # Linux reports ru_maxrss in kilobytes.
        peak = rusage.ru_maxrss * 1024
        code = p.returncode
    else:
        print(
            'error: measuring memory is not supported on this platform', file=sys.stderr
        )
        sys.exit(1)

    return {'peak': peak, 'exit_code': code, 'seconds': time.time() - start}


def prepare_replay(replay: Path, qst: Path, output_dir: Path) -> Path:
    """
    zplayer looks for a replay's qst relative to the replay. Replays recorded
    by users (like replay uploads) have a path from their own machine, so in
    that case play a copy of the replay with `qst` linked in at that path. The
    replay's qst metadata stays the same, since compat checks may compare it.
    """
    qst_meta = None
    for line in replay.read_text(errors='replace').splitlines():
        if not line.startswith('M '):
            break
        if line.startswith('M qst '):
            qst_meta = line[len('M qst ') :].strip()
            break

    if not qst_meta or Path(qst_meta).is_absolute() or (replay.parent / qst_meta).exists():
        return replay

    replay_copy = output_dir / 'replay' / replay.name
    linked_qst = replay_copy.parent / qst_meta
    linked_qst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(replay, replay_copy)
    os.symlink(qst.absolute(), linked_qst)
    return replay_copy


def measure_player(args, zplayer: Path, quest: dict) -> dict:
    output_dir = Path(tempfile.mkdtemp(prefix='zc_measure_memory_'))
    try:
        replay = prepare_replay(root_dir / quest['replay'], root_dir / quest['qst'], output_dir)
        cmd = [
            zplayer.absolute(),
            '-headless',
            '-replay',
            replay.absolute(),
            '-replay-exit-when-done',
            '-replay-output-dir',
            output_dir,
            *args.zplayer_args.split(),
        ]
        if args.max_frames:
            cmd += ['-frame', str(args.max_frames)]
        result = run_measured(cmd, zplayer.parent)
        result['frames'] = None
        result_file = output_dir / f'{replay.name}.result.txt'
        if result_file.exists():
            m = re.search(r'^frame: (\d+)', result_file.read_text(), re.M)
            if m:
                result['frames'] = int(m.group(1))
        return result
    finally:
        shutil.rmtree(output_dir, ignore_errors=True)


def measure_editor(zeditor: Path, quest: dict) -> dict:
    output_dir = Path(tempfile.mkdtemp(prefix='zc_measure_memory_'))
    try:
        cmd = [
            zeditor.absolute(),
            '-headless',
            '-export-strings',
            (root_dir / quest['qst']).absolute(),
            output_dir / 'strings.tsv',
        ]
        return run_measured(cmd, zeditor.parent)
    finally:
        shutil.rmtree(output_dir, ignore_errors=True)


def max_result(results: list[dict]) -> dict:
    peaks = [r['peak'] for r in results if r['peak'] is not None]
    return {
        'peak_mb': max(peaks) / 1024 / 1024 if peaks else None,
        'frames': results[0].get('frames'),
        'failed': any(r['exit_code'] != 0 for r in results),
        'seconds': max(r['seconds'] for r in results),
    }


def fmt_mb(mb) -> str:
    return '-' if mb is None else f'{mb:.1f}'


def print_table(headers: list[str], rows: list[list[str]], numeric: set[int]):
    widths = [
        max(len(str(r[i])) for r in [headers] + rows) for i in range(len(headers))
    ]

    def line(cells):
        parts = []
        for i, c in enumerate(cells):
            parts.append(
                str(c).rjust(widths[i]) if i in numeric else str(c).ljust(widths[i])
            )
        return '| ' + ' | '.join(parts) + ' |'

    print(line(headers))
    print('|' + '|'.join('-' * (w + 2) for w in widths) + '|')
    for r in rows:
        print(line(r))


def print_results(results: dict, apps: list[str]):
    headers = ['Quest']
    for app in apps:
        headers += [f'{app} (MB)']
    if 'zplayer' in apps:
        headers += ['Replay frames']
    rows = []
    for name, r in results.items():
        row = [name]
        for app in apps:
            v = r.get(app)
            cell = fmt_mb(v and v['peak_mb'])
            if v and v['failed']:
                cell += ' !'
            row.append(cell)
        if 'zplayer' in apps:
            frames = r['zplayer']['frames']
            row.append('-' if frames is None else str(frames))
        rows.append(row)
    print_table(headers, rows, numeric=set(range(1, len(headers))))


def print_comparison(results: dict, baseline: dict, apps: list[str]):
    for app in apps:
        print(f'\n{app} {baseline["metric"]} (MB):\n')
        rows = []
        total_before = total_after = 0
        for name, r in results.items():
            before = baseline['results'].get(name, {}).get(app)
            after = r.get(app)
            if (
                not before
                or not after
                or before['peak_mb'] is None
                or after['peak_mb'] is None
            ):
                continue
            delta = after['peak_mb'] - before['peak_mb']
            pct = delta / before['peak_mb'] * 100
            total_before += before['peak_mb']
            total_after += after['peak_mb']
            rows.append(
                [
                    name,
                    fmt_mb(before['peak_mb']),
                    fmt_mb(after['peak_mb']),
                    f'{delta:+.1f}',
                    f'{pct:+.1f}%',
                ]
            )
        if total_before:
            delta = total_after - total_before
            rows.append(
                [
                    'Total',
                    fmt_mb(total_before),
                    fmt_mb(total_after),
                    f'{delta:+.1f}',
                    f'{delta / total_before * 100:+.1f}%',
                ]
            )
        print_table(
            ['Quest', 'Before', 'After', 'Change', '%'], rows, numeric={1, 2, 3, 4}
        )


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.split('\n\n')[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split('\n\n', 1)[1],
    )
    parser.add_argument(
        '--build_folder', type=Path, default=root_dir / 'build' / 'Release'
    )
    parser.add_argument(
        '--filter',
        action='append',
        default=[],
        help='Only quests whose name contains this (case-insensitive).',
    )
    parser.add_argument(
        '--runs',
        type=int,
        default=1,
        help='Runs per measurement; the highest is reported.',
    )
    parser.add_argument(
        '--jobs',
        type=int,
        default=max(1, (os.cpu_count() or 2) // 2),
        help='Processes to run at once.',
    )
    parser.add_argument(
        '--max-frames', type=int, help='Stop each replay at this frame.'
    )
    parser.add_argument(
        '--zplayer-args',
        default='',
        help='Extra arguments for zplayer, e.g. "-no-jit".',
    )
    parser.add_argument('--no-player', action='store_true')
    parser.add_argument('--no-editor', action='store_true')
    parser.add_argument(
        '--json',
        type=Path,
        help='Write results to this file (usable later as --baseline).',
    )
    parser.add_argument(
        '--baseline',
        type=Path,
        help='Results from a previous --json run to compare against.',
    )
    args = parser.parse_args()

    apps = []
    if not args.no_player:
        apps.append('zplayer')
    if not args.no_editor:
        apps.append('zeditor')
    exes = {app: find_exe(args.build_folder, app) for app in apps}

    quests = [
        q
        for q in QUESTS
        if not args.filter or any(f.lower() in q['name'].lower() for f in args.filter)
    ]
    missing = []
    for q in quests:
        for path in (q['qst'], q['replay']):
            if not (root_dir / path).exists():
                print(f'skipping {q["name"]}: {path} not found', file=sys.stderr)
                missing.append(q)
                break
    quests = [q for q in quests if q not in missing]
    if not quests:
        print('error: no quests to measure', file=sys.stderr)
        sys.exit(1)

    jobs = []
    for q in quests:
        for app in apps:
            for _ in range(args.runs):
                jobs.append((q, app))

    def run_job(job):
        q, app = job
        if app == 'zplayer':
            return measure_player(args, exes[app], q)
        return measure_editor(exes[app], q)

    print(
        f'measuring {len(quests)} quests ({len(jobs)} runs, {args.jobs} at a time) ...',
        file=sys.stderr,
    )
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        raw = list(pool.map(run_job, jobs))

    results = {}
    for (q, app), r in zip(jobs, raw):
        results.setdefault(q['name'], {}).setdefault(app, []).append(r)
    results = {
        name: {app: max_result(rs) for app, rs in by_app.items()}
        for name, by_app in results.items()
    }

    print(
        f'\n{metric_name()}, {platform.system()} {platform.machine()}, {args.build_folder}\n'
    )
    print_results(results, apps)
    if any(r[app]['failed'] for r in results.values() for app in apps if app in r):
        print(
            '\n! = the process exited with an error'
        )

    if args.baseline:
        baseline = json.loads(args.baseline.read_text())
        if baseline['metric'] != metric_name():
            print(
                f'\nwarning: baseline measured {baseline["metric"]}, not {metric_name()}',
                file=sys.stderr,
            )
        print_comparison(results, baseline, apps)

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(
                {
                    'metric': metric_name(),
                    'platform': f'{platform.system()} {platform.machine()}',
                    'build_folder': str(args.build_folder),
                    'max_frames': args.max_frames,
                    'results': results,
                },
                indent=2,
            )
        )


if __name__ == '__main__':
    main()
