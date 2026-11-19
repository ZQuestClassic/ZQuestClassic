# Re-runs every crash in .tmp/fuzz/artifacts/<target>/ and groups them by where they crashed,
# so each distinct bug shows up once. See docs/fuzzing.md.
#
#   python scripts/fuzzing/triage.py <target> [--show]

import argparse
import os
import re
import subprocess

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

root = Path(__file__).parent.parent.parent
parser = argparse.ArgumentParser()
parser.add_argument('target')
parser.add_argument('--build_folder', default=root / 'build_fuzz', type=Path)
parser.add_argument(
    '--show', action='store_true', help='Print the full report of one crash per group'
)
parser.add_argument(
    '--frames', type=int, default=3, help='Number of stack frames that identify a group'
)
parser.add_argument('-j', '--jobs', type=int, default=max(1, os.cpu_count() // 2))
parser.add_argument(
    '--min_severity',
    choices=['low', 'medium', 'high'],
    default='low',
    help='Only list groups at least this severe (see severity())',
)
args = parser.parse_args()

artifacts = root / '.tmp/fuzz/artifacts' / args.target
env = {
    **os.environ,
    'ZC_FUZZ': args.target,
    'ZC_FUZZ_ZASM_QST': str(root / 'tests/replays/playground/playground.qst'),
    'ASAN_OPTIONS': 'detect_leaks=0:allocator_may_return_null=1:symbolize=1',
}
for symbolizer in [
    '/opt/homebrew/opt/llvm/bin/llvm-symbolizer',
    '/usr/local/opt/llvm/bin/llvm-symbolizer',
]:
    if 'ASAN_SYMBOLIZER_PATH' not in env and Path(symbolizer).exists():
        env['ASAN_SYMBOLIZER_PATH'] = symbolizer

# Frames from these are the sanitizer/libFuzzer/abort machinery, not where the bug is.
ignored_frame = re.compile(
    r'^(__asan|__sanitizer|__interceptor|fuzzer::|_sigtramp|abort|__assert|CheckFailed|'
    r'LogMessage|std::|unsigned long std::|terminate|wrap_|strlen|memcpy|memmove|_Zn|malloc)'
)
ignored_location = re.compile(r'(libsystem|libc\+\+abi|c\+\+/v1/|libclang_rt)')
frame_re = re.compile(r'^\s+#\d+ 0x[0-9a-f]+ in (.+?) (/\S+|\(\S+)')


SEVERITIES = ['low', 'medium', 'high']


# How likely a crash is to be exploitable, from its ASan report. A rough guide to what to look at
# first, not a verdict:
#   high: a write out of bounds, a use after free or a double free
#   medium: a read out of bounds (can leak memory), or a bad access away from null
#   low: null derefs, asserts and CHECKs, stack overflows from recursion, timeouts, running out
#        of memory - crashes, but not ones an attacker can do more with
def severity(kind: str, out: str) -> str:
    if re.match(r'(heap-use-after-free|use-after-poison|double-free|stack-use-after)', kind):
        return 'high'
    if 'overflow' in kind and kind != 'stack-overflow' or kind.endswith('underflow'):
        return 'high' if re.search(r'^WRITE of size', out, re.M) else 'medium'
    if kind in ('SEGV', 'BUS'):
        m = re.search(r'(?:SEGV|BUS) on unknown address (0x[0-9a-f]+|\(nil\))', out)
        address = int(m.group(1), 16) if m and m.group(1).startswith('0x') else 0
        if address < 0x10000 or 'hint: address points to the zero page' in out:
            return 'low'
        return 'high' if 'caused by a WRITE' in out else 'medium'
    if kind in ('bad-free', 'alloc-dealloc-mismatch', 'new-delete-type-mismatch', 'memcpy-param-overlap'):
        return 'medium'
    return 'low'


def run(crash: Path) -> tuple[str, str]:
    p = subprocess.run(
        [
            './zplayer',
            '-timeout=30',
            '-rss_limit_mb=4096',
            '-malloc_limit_mb=2048',
            crash,
        ],
        cwd=args.build_folder,
        env=env,
        capture_output=True,
        text=True,
        errors='replace',
    )
    out = p.stderr + p.stdout
    kind = 'no crash (fixed, or flaky)'
    if m := re.search(r'ERROR: AddressSanitizer: ([\w-]+)', out):
        kind = m.group(1)
    elif m := re.search(r'ERROR: libFuzzer: ([\w-]+)', out):
        kind = m.group(1)
    elif 'CHECK failed' in out or 'Assertion failed' in out:
        kind = 'assert'
    if m := re.search(r'(CHECK failed[^\n]*|Assertion failed[^\n]*)', out):
        kind += f' [{m.group(1)[:120]}]'

    frames = []
    for line in out.splitlines():
        # Only the crashing stack, not where the thread was created or memory was allocated.
        if frames and not line.strip():
            break
        if m := frame_re.match(line):
            func, loc = m.groups()
            if ignored_frame.search(func) or ignored_location.search(loc):
                continue
            loc = re.sub(r'^.*/(src|third_party|_deps)/', r'\1/', loc)
            frames.append(f'{func[:80]} {loc}')
            if len(frames) >= args.frames:
                break
    return f'[{severity(kind, out)}] {kind}\n    ' + '\n    '.join(frames), out


crashes = [
    c for c in sorted(artifacts.glob('*')) if c.is_file() and not c.name.startswith('.')
]
groups: dict[str, list[tuple[Path, str]]] = {}
with ThreadPoolExecutor(args.jobs) as pool:
    for crash, (key, out) in zip(crashes, pool.map(run, crashes)):
        groups.setdefault(key, []).append((crash, out))

def group_severity(key: str) -> int:
    return SEVERITIES.index(key[1 : key.index(']')])


for key, crashes in sorted(
    groups.items(), key=lambda kv: (-group_severity(kv[0]), -len(kv[1]))
):
    if group_severity(key) < SEVERITIES.index(args.min_severity):
        continue
    smallest = min(crashes, key=lambda c: c[0].stat().st_size)
    print(f'=== {len(crashes)}x {key}')
    print(f'    smallest: {smallest[0]} ({smallest[0].stat().st_size} bytes)')
    if args.show:
        report = smallest[1]
        start = report.find('==ERROR')
        if start == -1:
            start = max(0, report.find('CHECK failed') - 200)
        print(report[start : start + 6000])
    print()
