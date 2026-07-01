#!/usr/bin/env python3

"""
Measure ZC engine performance across engine configurations.

Runs a representative set of benchmarks one at a time (no concurrency, so
timings are not contended), each under one or more engine "modes", and reports
per benchmark and in aggregate:

  * time to completion  - total wall-clock time for the zplayer process
  * time to precompile  - JIT script precompilation time (engine-reported)

The default modes isolate what the JIT and the ZASM optimizer each buy you:

  jit          JIT on,  precompile on,  zasm-optimize on
  nojit        JIT off,                 zasm-optimize on   (interpreter)
  nojit-noopt  JIT off,                 zasm-optimize off  (interpreter, raw zasm)

Benchmarks are the replay set plus a synthetic "maths" run (playground's
maths.zs, a tight arithmetic loop). The JIT speedup shows up on script-heavy
replays - yuurand_riviere is the clearest - and is negligible on graphics- or
gameplay-bound ones. Short replays (e.g. the crucible_quest shorts) are useful
for measuring precompile cost, but too short to amortize it.

Examples:
  # Full comparison (all three modes) over the default benchmarks.
  python scripts/measure_performance.py

  # Just JIT vs interpreter, 3 runs each (medians reported).
  python scripts/measure_performance.py --mode jit --mode nojit --runs 3

  # A custom subset of replays (substring match), a specific build.
  python scripts/measure_performance.py --build_folder build/RelWithDebInfo \\
      --filter yuurand --filter z3

  # Only the maths benchmark, JIT vs no-JIT.
  python scripts/measure_performance.py --no-replays --mode jit --mode nojit

  # Write per-run rows to a CSV for later analysis.
  python scripts/measure_performance.py --csv .tmp/perf.csv
"""

import argparse
import atexit
import csv
import os
import re
import statistics
import subprocess
import sys
import time

from pathlib import Path

script_dir = Path(__file__).resolve().parent
root_dir = script_dir.parent
replays_dir = root_dir / 'tests' / 'replays'

# name -> (jit, precompile, optimize_zasm)
MODES = {
    'jit': dict(jit=True, precompile=True, optimize=True),
    'nojit': dict(jit=False, precompile=False, optimize=True),
    'nojit-noopt': dict(jit=False, precompile=False, optimize=False),
}
DEFAULT_MODES = ['jit', 'nojit', 'nojit-noopt']

# Paths are relative to tests/replays/.
DEFAULT_REPLAYS = [
    'classic_1st/classic_1st_lvl1.zplay',
    'crucible_quest/crucible_quest_short_1.zplay',
    'crucible_quest/crucible_quest_short_2.zplay',
    'freedom_in_chains.zplay',
    'hell_awaits/hell_awaits_1_of_2.zplay',
    'hero_of_dreams/hero_of_dreams_1_of_8.zplay',
    'scripting/100_rooms_of_wisdom.zplay',
    'stellar_seas/stellar_seas_randomizer.zplay',
    # TODO: Include when working on WASM jit.
    # 'terror_of_necromancy_demo6/terror_of_necromancy_demo6_05_of_54.zplay',
    'yuurand/yuurand_riviere.zplay',
    'z3/z3.zplay',
]

# Synthetic CPU-bound benchmark: playground's maths.zs runs on dmap 0 / screen 5.
MATHS_BENCHMARK = dict(
    kind='test',
    name='maths (playground)',
    qst='playground/playground.qst',
    dmap=0,
    screen=5,
    frames=10000,
)

RE_PRECOMPILE = re.compile(r'\[jit\] finished precompiling scripts, took (\d+) ms')
RE_LOAD = re.compile(r'Time to load (?:replay|qst): (\d+) ms')
RE_SCRIPT = re.compile(r'total time in scripting engine: ([\d.]+) ms')
# From -frame-timings (per-frame processing cost). Anchored to line starts so
# they don't collide with other "... ms" lines (e.g. script timings' avg).
RE_FRAME_AVG = re.compile(r'^avg: ([\d.]+) ms', re.M)
RE_FRAME_P95 = re.compile(r'^p95: ([\d.]+) ms', re.M)
RE_FRAME_P99 = re.compile(r'^p99: ([\d.]+) ms', re.M)
RE_FRAME_MAX = re.compile(r'^max: ([\d.]+) ms', re.M)


def find_zplayer(build_folder: Path) -> Path:
    ext = '.exe' if os.name == 'nt' else ''
    # Direct candidates first, then a recursive search (build layouts vary).
    candidates = [build_folder / f'zplayer{ext}', build_folder / f'zelda{ext}']
    for c in candidates:
        if c.exists():
            return c
    found = next(iter(sorted(build_folder.glob(f'**/zplayer{ext}'))), None)
    if found:
        return found
    print(f'error: could not find zplayer under {build_folder}', file=sys.stderr)
    print(
        '       pass --build_folder pointing at a folder containing zplayer.',
        file=sys.stderr,
    )
    sys.exit(1)


def resolve_replays(args) -> list[Path]:
    if args.replay:
        paths = [
            Path(r) if Path(r).is_absolute() else replays_dir / r for r in args.replay
        ]
    elif args.filter:
        all_replays = sorted(replays_dir.rglob('*.zplay'))
        paths = []
        for f in args.filter:
            matched = [
                t for t in all_replays if f in t.relative_to(replays_dir).as_posix()
            ]
            if not matched:
                print(f'warning: filter "{f}" matched no replays', file=sys.stderr)
            paths.extend(matched)
        seen = set()  # de-dupe, preserve order
        paths = [p for p in paths if not (p in seen or seen.add(p))]
    else:
        paths = [replays_dir / r for r in DEFAULT_REPLAYS]

    existing = []
    for p in paths:
        if p.exists():
            existing.append(p)
        else:
            print(f'warning: replay not found, skipping: {p}', file=sys.stderr)
    return existing


def build_benchmarks(args) -> list[dict]:
    benchmarks = []
    if args.maths:
        benchmarks.append(dict(MATHS_BENCHMARK))
    if args.replays:
        for p in resolve_replays(args):
            rel = (
                p.relative_to(replays_dir).as_posix()
                if p.is_relative_to(replays_dir)
                else p.name
            )
            benchmarks.append(dict(kind='replay', name=rel, path=p))
    return benchmarks


def safe_name(name: str) -> str:
    return re.sub(r'[^A-Za-z0-9._-]', '_', name)


def parse_result_file(path: Path) -> dict:
    result = {}
    if not path.exists():
        return result
    for line in path.read_text(errors='replace').splitlines():
        if ': ' in line:
            key, _, value = line.partition(': ')
            result[key.strip()] = value.strip()
    return result


def build_flags(mode: dict, args) -> list:
    """Engine flags shared by the native and web invocations."""
    flags = ['-optimize-zasm'] if mode['optimize'] else ['-no-optimize-zasm']
    flags += (
        ['-jit'] + (['-jit-precompile'] if mode['precompile'] else [])
        if mode['jit']
        else ['-no-jit']
    )
    # Suppress script error logging so heavy per-frame errors (e.g. maths.zs's
    # every-frame divide-by-zero) don't dominate the timing.
    if args.disable_logging:
        flags += ['-experimental-disable-script-error-logs']
    # Isolate scripting-engine time (the fairest jit-vs-nojit metric).
    flags += ['-script-timings']
    # Per-frame processing cost distribution. (On web this dumps a line per slow
    # frame to the console at exit, but that's post-run so it doesn't skew timing.)
    flags += ['-frame-timings']
    return flags


def _execute_and_parse(cmd: list, log_path: Path, result_path: Path, args) -> dict:
    """Run one subprocess (native zplayer or the web harness) and parse metrics."""
    # Clear any stale result so a failed run doesn't read the previous attempt's numbers.
    result_path.unlink(missing_ok=True)

    start = time.perf_counter()
    timed_out = False
    returncode = None
    with open(log_path, 'wb') as log:
        try:
            proc = subprocess.run(
                cmd, stdout=log, stderr=subprocess.STDOUT, timeout=args.max_seconds
            )
            returncode = proc.returncode
        except subprocess.TimeoutExpired:
            timed_out = True
    wall_ms = (time.perf_counter() - start) * 1000

    log_text = log_path.read_text(errors='replace')
    result = parse_result_file(result_path)
    precompile_match = RE_PRECOMPILE.search(log_text)
    load_match = RE_LOAD.search(log_text)
    script_match = RE_SCRIPT.search(log_text)

    def as_int(key):
        return int(result[key]) if result.get(key, '').lstrip('-').isdigit() else None

    def frame_stat(rx):
        m = rx.search(log_text)
        return float(m.group(1)) if m else None

    return {
        'wall_ms': wall_ms,
        'precompile_ms': int(precompile_match.group(1)) if precompile_match else None,
        'load_ms': int(load_match.group(1)) if load_match else None,
        'script_ms': float(script_match.group(1)) if script_match else None,
        'frame_avg_ms': frame_stat(RE_FRAME_AVG),
        'frame_p95_ms': frame_stat(RE_FRAME_P95),
        'frame_p99_ms': frame_stat(RE_FRAME_P99),
        'frame_max_ms': frame_stat(RE_FRAME_MAX),
        'engine_ms': as_int('duration'),
        'frames': as_int('frame'),
        'fps': as_int('fps'),
        'success': result.get('success') == 'true',
        'timed_out': timed_out,
        'returncode': returncode,
    }


def run_once(exe: Path, bench: dict, mode_name: str, out_dir: Path, args) -> dict:
    """Run a single benchmark once under one mode; collect timing metrics.

    Web runs are retried on failure (browser/webserver flakes) up to
    --web-retries times; native runs are not retried.
    """
    mode = MODES[mode_name]
    tag = f'{safe_name(bench["name"])}.{mode_name}'
    log_path = out_dir / f'{tag}.log'
    flags = build_flags(mode, args)
    # Cap replay length to keep the long ones tractable (the maths 'test'
    # benchmark has its own frame count and is left alone).
    frame_cap = (
        ['-frame', str(args.frame)] if args.frame and bench['kind'] == 'replay' else []
    )

    if args.web:
        # The web build runs in a browser via the puppeteer harness. Only replay
        # benchmarks are supported (run_replay.js is replay-only); the maths
        # 'test' benchmark is filtered out for web in main().
        result_path = out_dir / f'{tag}.result.txt'
        url = f'play/?replay=/host{bench["path"].resolve().as_posix()}'
        extra = ' '.join(['-replay-exit-when-done', '-headless'] + frame_cap + flags)
        cmd = [
            'node',
            str(root_dir / 'web' / 'tests' / 'run_replay.js'),
            args.server_url,
            str(out_dir),
            url,
            extra,
            str(result_path),
        ]
    elif bench['kind'] == 'replay':
        cmd = (
            [str(exe), '-replay', str(bench['path'].resolve())]
            + [
                '-replay-exit-when-done',
                '-replay-output-dir',
                str(out_dir),
                '-headless',
            ]
            + frame_cap
            + flags
        )
        result_path = out_dir / f'{bench["path"].name}.result.txt'
    else:  # 'test' -> load a quest at dmap/screen and record N frames
        record_path = out_dir / f'{tag}.rec.zplay'
        cmd = (
            [
                str(exe),
                '-v0',
                '-test',
                str((replays_dir / bench['qst']).resolve()),
                str(bench['dmap']),
                str(bench['screen']),
                '-record',
                str(record_path),
                '-frame',
                str(bench['frames']),
            ]
            + [
                '-replay-exit-when-done',
                '-replay-output-dir',
                str(out_dir),
                '-headless',
            ]
            + flags
        )
        result_path = out_dir / f'{record_path.name}.result.txt'

    # Retry web flakes (browser/webserver races), but not native runs and not
    # timeouts (a timeout just burns another --max-seconds).
    attempts = 1 + (args.web_retries if args.web else 0)
    for attempt in range(attempts):
        r = _execute_and_parse(cmd, log_path, result_path, args)
        r['attempts'] = attempt + 1
        if r['success'] or r['timed_out'] or attempt == attempts - 1:
            return r
        print(
            f' [flake, retry {attempt + 1}/{attempts - 1}]',
            end='', file=sys.stderr, flush=True,
        )
    return r


def median_runs(runs: list[dict]) -> dict:
    def med(key):
        vals = [r[key] for r in runs if r.get(key) is not None]
        return statistics.median(vals) if vals else None

    return {
        'wall_ms': med('wall_ms'),
        'precompile_ms': med('precompile_ms'),
        'script_ms': med('script_ms'),
        'frame_avg_ms': med('frame_avg_ms'),
        'frame_p95_ms': med('frame_p95_ms'),
        'frame_p99_ms': med('frame_p99_ms'),
        'frame_max_ms': med('frame_max_ms'),
        'engine_ms': med('engine_ms'),
        'fps': med('fps'),
        'frames': next(
            (r['frames'] for r in runs if r.get('frames') is not None), None
        ),
        'success': all(r['success'] and not r['timed_out'] for r in runs),
        'timed_out': any(r['timed_out'] for r in runs),
    }


def fmt_s(ms):
    return f'{ms / 1000:.2f}' if ms is not None else '-'


def fmt_int(v):
    return str(int(v)) if v is not None else '-'


def fmt_speedup(numerator_ms, denominator_ms):
    # None (a mode didn't report script time) and 0.0 (a scriptless replay) are
    # different situations; only '-' means missing data.
    if numerator_ms is None or denominator_ms is None:
        return '-'
    if denominator_ms == 0:
        return 'n/a'
    return f'{numerator_ms / denominator_ms:.2f}x'


def fmt_dur(seconds):
    seconds = int(seconds)
    return f'{seconds // 60}m{seconds % 60:02d}s' if seconds >= 60 else f'{seconds}s'


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '--build_folder',
        type=Path,
        default=root_dir / 'build' / 'Release',
        help='folder containing zplayer (default: build/Release)',
    )
    parser.add_argument(
        '--mode',
        action='append',
        choices=list(MODES),
        default=[],
        help=f'engine mode to run (repeatable); default: {" ".join(DEFAULT_MODES)}',
    )
    parser.add_argument(
        '--filter',
        action='append',
        default=[],
        help='substring match on a replay path (repeatable); overrides the default replay set',
    )
    parser.add_argument(
        '--replay',
        action='append',
        default=[],
        help='explicit replay path, absolute or relative to tests/replays (repeatable)',
    )
    parser.add_argument(
        '--replays',
        action=argparse.BooleanOptionalAction,
        default=True,
        help='include replay benchmarks (default: on; --no-replays for maths only)',
    )
    parser.add_argument(
        '--maths',
        action=argparse.BooleanOptionalAction,
        default=True,
        help='include the synthetic maths.zs benchmark (default: on)',
    )
    parser.add_argument(
        '--runs',
        type=int,
        default=1,
        help='times to run each benchmark; medians are reported (default: 1)',
    )
    parser.add_argument(
        '--frame',
        type=int,
        default=50000,
        help='cap each replay to this many frames (default: 50000; 0 = uncapped). '
        'Keeps long replays tractable; the maths benchmark is unaffected',
    )
    parser.add_argument(
        '--max-seconds',
        type=int,
        default=600,
        help='per-run wall-clock timeout in seconds (default: 600)',
    )
    parser.add_argument(
        '--web-retries',
        type=int,
        default=2,
        help='retry a failed web run this many times (browser/webserver flakes); '
        'native runs are never retried (default: 2)',
    )
    parser.add_argument(
        '--disable-logging',
        action=argparse.BooleanOptionalAction,
        default=True,
        help='pass -experimental-disable-script-error-logs to suppress script error '
        'spam (default: on; --no-disable-logging to keep script error logging)',
    )
    parser.add_argument(
        '--output-dir',
        type=Path,
        default=root_dir / '.tmp' / 'perf',
        help='where run output/logs are written (default: .tmp/perf)',
    )
    parser.add_argument(
        '--csv',
        type=Path,
        default=None,
        help='also write per-run rows to this CSV file',
    )
    args = parser.parse_args()

    modes = args.mode or DEFAULT_MODES

    # A web build has packages/web/play/index.html; it runs in a browser via the
    # puppeteer harness rather than as a native subprocess.
    web_hits = list(args.build_folder.glob('**/packages/web/play/index.html'))
    args.web = bool(web_hits)
    web_root = web_hits[0].parent.parent if web_hits else None
    exe = None if args.web else find_zplayer(args.build_folder)

    benchmarks = build_benchmarks(args)
    if args.web:
        for b in [b for b in benchmarks if b['kind'] != 'replay']:
            print(
                f'note: skipping "{b["name"]}" on web build (only replays supported)',
                file=sys.stderr,
            )
        benchmarks = [b for b in benchmarks if b['kind'] == 'replay']
    if not benchmarks:
        print('error: no benchmarks to run', file=sys.stderr)
        sys.exit(1)

    # Resolve to absolute: zplayer chdirs during qst load, so a relative
    # -replay-output-dir would be interpreted against the wrong directory and
    # the result file would land next to the source replay instead.
    args.output_dir = args.output_dir.resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    # For the web build, start the static server the puppeteer harness talks to.
    webserver = None
    if args.web:
        args.server_url = 'http://localhost:8000'
        server_log = args.output_dir / 'webserver.log'
        webserver = subprocess.Popen(
            ['node', str(root_dir / 'scripts' / 'webserver.mjs'), str(web_root)],
            stdout=open(server_log, 'wb'),
            stderr=subprocess.STDOUT,
        )
        for _ in range(60):
            if server_log.exists() and 'Server running' in server_log.read_text(
                errors='replace'
            ):
                break
            if webserver.poll() is not None:
                print(
                    f'error: webserver exited early; see {server_log}', file=sys.stderr
                )
                sys.exit(1)
            time.sleep(0.5)
        else:
            webserver.terminate()
            print('error: webserver did not become ready', file=sys.stderr)
            sys.exit(1)
        atexit.register(webserver.terminate)  # always stop the server on exit

    print(f'target:     {"web (" + str(web_root) + ")" if args.web else exe}')
    if args.web:
        print(f'server:     {args.server_url}')
    print(f'modes:      {", ".join(modes)}')
    print(f'benchmarks: {len(benchmarks)}   runs each: {args.runs}')
    if args.web:
        print(
            'note:       web wall(s) includes browser startup; see engine totals below'
        )
    print()

    # results[bench_name][mode_name] = median dict
    results = {b['name']: {} for b in benchmarks}
    csv_rows = []
    total_runs = len(benchmarks) * len(modes) * args.runs
    idx_w = len(str(total_runs))
    done = 0
    t_start = time.perf_counter()
    for bench in benchmarks:
        for mode_name in modes:
            runs = []
            for i in range(args.runs):
                done += 1
                label = f'{bench["name"]} [{mode_name}]'
                if args.runs > 1:
                    label += f' run {i + 1}/{args.runs}'
                # Progress goes to stderr so it stays real-time (stdout is block
                # buffered when redirected) and doesn't pollute the result table.
                print(
                    f'[{done:>{idx_w}}/{total_runs}] {label} ... ',
                    end='',
                    file=sys.stderr,
                    flush=True,
                )
                r = run_once(exe, bench, mode_name, args.output_dir, args)
                runs.append(r)
                csv_rows.append(
                    {'benchmark': bench['name'], 'mode': mode_name, 'run': i, **r}
                )
                elapsed = time.perf_counter() - t_start
                eta = elapsed / done * (total_runs - done)
                status = (
                    'TIMEOUT' if r['timed_out'] else ('ok' if r['success'] else 'FAIL')
                )
                wall = f'{r["wall_ms"] / 1000:.1f}s' if r['wall_ms'] else '-'
                print(
                    f'{wall} {status}  (elapsed {fmt_dur(elapsed)}, eta {fmt_dur(eta)})',
                    file=sys.stderr,
                    flush=True,
                )
            results[bench['name']][mode_name] = median_runs(runs)
    print(
        f'all {total_runs} runs done in {fmt_dur(time.perf_counter() - t_start)}\n',
        file=sys.stderr,
        flush=True,
    )

    # ---- comparison table: one row per benchmark, wall(s) per mode ----
    name_w = max(len('benchmark'), max(len(b['name']) for b in benchmarks))
    mode_w = max(11, max(len(m + '(s)') for m in modes))  # fit the widest mode header
    # Spanning label over the per-mode wall-time columns.
    span_w = len(modes) * (mode_w + 1)
    banner = '|' + 'wall time (includes precompile)'.center(span_w - 2) + '|'
    print(f'{"":<{name_w}} {"":>8}{banner}')
    header = f'{"benchmark":<{name_w}} {"frames":>8}'
    for m in modes:
        header += f' {m + "(s)":>{mode_w}}'
    if 'jit' in modes:
        header += f' {"precomp(ms)":>12}'
        if 'nojit' in modes:
            header += f' {"script x":>10}'
    print(header)
    print('-' * len(header))

    # Aggregate only over benchmarks that succeeded in ALL compared modes, so a
    # single failed/flaky run can't fabricate a speedup by comparing different
    # replay sets per mode. Every benchmark still shows in the table.
    complete = {
        b['name']
        for b in benchmarks
        if all(
            results[b['name']][m]['success'] and not results[b['name']][m]['timed_out']
            for m in modes
        )
    }
    excluded = [b['name'] for b in benchmarks if b['name'] not in complete]

    mode_totals = {m: 0.0 for m in modes}
    mode_engine_totals = {m: 0.0 for m in modes}
    mode_script_totals = {m: 0.0 for m in modes}
    precomp_total = 0.0
    for bench in benchmarks:
        by_mode = results[bench['name']]
        is_complete = bench['name'] in complete
        frames = next(
            (by_mode[m]['frames'] for m in modes if by_mode[m]['frames'] is not None),
            None,
        )
        row = f'{bench["name"]:<{name_w}} {fmt_int(frames):>8}'
        for m in modes:
            agg = by_mode[m]
            cell = (
                'TIMEOUT'
                if agg['timed_out']
                else (
                    'FAIL ' + fmt_s(agg['wall_ms'])
                    if not agg['success']
                    else fmt_s(agg['wall_ms'])
                )
            )
            row += f' {cell:>{mode_w}}'
            if is_complete:
                if agg['wall_ms'] is not None:
                    mode_totals[m] += agg['wall_ms']
                if agg['engine_ms'] is not None:
                    mode_engine_totals[m] += agg['engine_ms']
                if agg['script_ms'] is not None:
                    mode_script_totals[m] += agg['script_ms']
        if 'jit' in modes:
            pc = by_mode['jit']['precompile_ms']
            if is_complete:
                precomp_total += pc or 0
            row += f' {fmt_int(pc):>12}'
            if 'nojit' in modes:
                # Scripting-engine time is the tightest jit-vs-nojit comparison.
                js, ns = by_mode['jit']['script_ms'], by_mode['nojit']['script_ms']
                row += f' {fmt_speedup(ns, js):>10}'
        print(row)

    print('-' * len(header))
    total_row = f'{"TOTAL":<{name_w}} {"":>8}'
    for m in modes:
        total_row += f' {fmt_s(mode_totals[m]):>{mode_w}}'
    if 'jit' in modes:
        total_row += f' {fmt_int(precomp_total):>12}'
        if 'nojit' in modes:
            jt, nt = mode_script_totals['jit'], mode_script_totals['nojit']
            total_row += f' {fmt_speedup(nt, jt):>10}'
    print(total_row)

    print()
    print(
        f'totals/speedups over {len(complete)}/{len(benchmarks)} benchmarks '
        f'complete in all modes'
        + (f'; excluded (a mode failed): {", ".join(excluded)}' if excluded else '')
    )
    for m in modes:
        print(f'total wall time [{m}]:    {mode_totals[m] / 1000:8.2f} s')
    # Engine time (replay-loop duration) excludes process/browser startup, so it's
    # a fairer cross-mode comparison - especially on web.
    for m in modes:
        print(f'total engine time [{m}]:  {mode_engine_totals[m] / 1000:8.2f} s')
    # Script time (run_script only) isolates the scripting engine - the tightest
    # jit-vs-nojit comparison, undiluted by rendering/frame overhead.
    for m in modes:
        print(f'total script time [{m}]:  {mode_script_totals[m] / 1000:8.2f} s')
    if 'jit' in modes and 'nojit' in modes:
        if mode_engine_totals['jit']:
            print(
                f'engine jit speedup:       {mode_engine_totals["nojit"] / mode_engine_totals["jit"]:.2f}x'
            )
        if mode_script_totals['jit']:
            print(
                f'script jit speedup:       {mode_script_totals["nojit"] / mode_script_totals["jit"]:.2f}x'
            )
    if 'jit' in modes:
        print(f'total jit precompile:     {precomp_total / 1000:8.2f} s')

    # ---- per-frame processing cost (from -frame-timings; native only) ----
    have_frame_data = any(
        results[b['name']][m].get('frame_avg_ms') is not None
        for b in benchmarks
        for m in modes
    )
    if have_frame_data:
        print()
        print('frame time (ms) - max is usually frame 0 (load + precompile)')
        fw = 8
        fheader = (
            f'{"benchmark":<{name_w}} {"mode":<12}'
            f'{"avg":>{fw}}{"p95":>{fw}}{"p99":>{fw}}{"max":>{fw}}'
        )
        print(fheader)
        print('-' * len(fheader))
        for bench in benchmarks:
            for m in modes:
                agg = results[bench['name']][m]
                if agg.get('frame_avg_ms') is None:
                    continue
                fmt_f = lambda v: f'{v:.1f}' if v is not None else '-'
                print(
                    f'{bench["name"]:<{name_w}} {m:<12}'
                    f'{fmt_f(agg["frame_avg_ms"]):>{fw}}{fmt_f(agg["frame_p95_ms"]):>{fw}}'
                    f'{fmt_f(agg["frame_p99_ms"]):>{fw}}{fmt_f(agg["frame_max_ms"]):>{fw}}'
                )

    if args.csv:
        args.csv.parent.mkdir(parents=True, exist_ok=True)
        fields = [
            'benchmark',
            'mode',
            'run',
            'wall_ms',
            'precompile_ms',
            'load_ms',
            'script_ms',
            'frame_avg_ms',
            'frame_p95_ms',
            'frame_p99_ms',
            'frame_max_ms',
            'engine_ms',
            'frames',
            'fps',
            'success',
            'timed_out',
            'returncode',
        ]
        with open(args.csv, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fields)
            writer.writeheader()
            for row in csv_rows:
                writer.writerow({k: row.get(k) for k in fields})
        print(f'\nwrote per-run CSV: {args.csv}')

    # Non-zero exit if any run failed, so the script is usable in automation.
    all_ok = all(
        results[b['name']][m]['success'] and not results[b['name']][m]['timed_out']
        for b in benchmarks
        for m in modes
    )
    sys.exit(0 if all_ok else 1)


if __name__ == '__main__':
    main()
