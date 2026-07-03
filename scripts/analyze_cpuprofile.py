# Summarizes a V8 .cpuprofile (as written by web/tests/run_replay.js with
# PROFILE=1): top functions by self time, with wasm frames grouped by module.
#
#   PROFILE=1 python tests/run_replay_tests.py --build_folder build_emscripten/Release <replay>
#   python scripts/analyze_cpuprofile.py <test-results-dir>/0/<name>/profile.cpuprofile

import argparse
import collections
import json

from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('profile', type=Path)
parser.add_argument('--top', type=int, default=40)
parser.add_argument(
    '--filter', default=None, help='only show frames whose name contains this'
)
args = parser.parse_args()

profile = json.loads(args.profile.read_text())
nodes = {n['id']: n for n in profile['nodes']}

# Self time per node = samples attributed to it. hitCount is present on nodes;
# total sample time comes from timeDeltas.
total_us = sum(profile.get('timeDeltas', [])) or 1
total_hits = sum(n.get('hitCount', 0) for n in profile['nodes']) or 1
us_per_hit = total_us / total_hits


def frame_name(n):
    cf = n['callFrame']
    name = cf.get('functionName') or '(anonymous)'
    url = cf.get('url', '')
    if url.startswith('wasm://'):
        # e.g. wasm://wasm/zplayer.wasm-00c8451a => zplayer.wasm
        mod = url.split('/')[-1].split('-')[0]
        return f'[{mod}] {name}'
    return name


self_us = collections.Counter()
for n in profile['nodes']:
    hits = n.get('hitCount', 0)
    if hits:
        self_us[frame_name(n)] += hits * us_per_hit

# Total time per top-level bucket.
bucket_us = collections.Counter()
for name, us in self_us.items():
    bucket = name.split(']')[0] + ']' if name.startswith('[') else '(js)'
    bucket_us[bucket] += us

print(f'profile wall time: {total_us / 1e6:.2f}s, {total_hits} samples')
print('\nself time by module:')
for bucket, us in bucket_us.most_common():
    print(f'  {bucket:30} {us / 1e6:8.2f}s  ({100 * us / total_us:.1f}%)')

print(f'\ntop {args.top} functions by self time:')
shown = 0
for name, us in self_us.most_common():
    if args.filter and args.filter not in name:
        continue
    print(f'  {us / 1e6:8.3f}s  {100 * us / total_us:5.1f}%  {name}')
    shown += 1
    if shown >= args.top:
        break
