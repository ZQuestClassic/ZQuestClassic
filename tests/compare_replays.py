# Given various test run results (either from local filesystem, or
# downloaded from a GitHub Actiosn workflow run), generates an HTML report
# presenting the snapshots frame-by-frame, and their differences.
#
# You must run a local web server for the HTML to work:
#     cd tests/compare-report
#     python -m http.server 8000

import argparse
from argparse import ArgumentTypeError
import os
import re
import json
import shutil
import json
from pathlib import Path
from itertools import groupby
from github import Github
from common import get_gha_artifacts, ReplayTestResults


def dir_path(path):
    if os.path.isdir(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


parser = argparse.ArgumentParser()
parser.add_argument('--workflow_run', type=int, action='append')
parser.add_argument('--local', type=dir_path, action='append')
parser.add_argument('--repo', default='ArmageddonGames/ZQuestClassic')
parser.add_argument('--token')
args = parser.parse_args()


if not args.workflow_run and not args.local:
    raise ArgumentTypeError(
        'must provide at least one --workflow_run or --local')


script_dir = os.path.dirname(os.path.realpath(__file__))
out_dir = Path(f'{script_dir}/compare-report')


def get_replay_from_snapshot_path(path):
    return path.name[:path.name.index('.zplay') + len('.zplay')]


# A `test_run` represents an invocation of run_replay_tests.py, including:
#  - what platform it ran on
#  - if run in CI, the workflow run id and git ref
#  - a list of replays and any snapshots they produced
# TODO remove older `test_run` concept and use `test_results` directly in the JS instead.
def collect_test_run_from_dir(directory: Path):
    test_results_json = json.loads((directory/'test_results.json').read_text('utf-8'))
    test_results = ReplayTestResults(**test_results_json)
    test_run = {
        'label': '',
        'runs_on': test_results.runs_on,
        'arch': test_results.arch,
        'ci': False,
    }
    if test_results.ci:
        test_run['ci'] = True
        test_run['ref'] = test_results.git_ref
        test_run['run_id'] = test_results.workflow_run_id

    label_parts = [
        test_run['runs_on'],
        test_run['arch'],
    ]
    if test_run['ci']:
        label_parts.extend([
            f'run_id {test_run["run_id"]}',
            test_run['ref'],
        ])
    test_run['label'] = ' '.join(label_parts)

    snapshot_paths = sorted(directory.rglob('*.zplay*.png'))
    if len(snapshot_paths) == 0:
        print(f'{directory} has no snapshots')

    test_run['replays'] = []
    for replay, snapshots in groupby(snapshot_paths, get_replay_from_snapshot_path):
        # For now, only collect the last run for a replay.
        last_run_dir_for_replay = None
        for runs in reversed(test_results.runs):
            run = next((r for r in runs if r.name == replay), None)
            if run:
                last_run_dir_for_replay = run
                break
        if last_run_dir_for_replay:
            snapshots = [
                s for s in snapshots if last_run_dir_for_replay.directory in str(s)]

        snapshots = [{
            'path': s,
            'frame': int(re.match(r'.*\.zplay\.(\d+)', s.name).group(1)),
            'unexpected': 'unexpected' in s.name,
        } for s in snapshots]
        snapshots.sort(key=lambda s: s['frame'])
        test_run['replays'].append({
            'name': replay,
            'snapshots': snapshots,
        })

    return test_run


# `directory` can include just one test run (a folder with test_results.json);
# or many. For the latter, they will be grouped into a single test run per
# platform (so that sharding does not generate multiple test runs).
def collect_test_runs_from_dir(directory: Path):
    test_runs = []

    for test_results_path in directory.rglob('test_results.json'):
        test_run_dir = test_results_path.parent
        test_runs.append(collect_test_run_from_dir(test_run_dir))

    if len(test_runs) == 0:
        raise Exception('found no test runs')

    if len(test_runs) == 1:
        return test_runs

    runs_by_platform = {}
    for test_run in test_runs:
        key = (test_run['runs_on'], test_run['arch'])
        if key not in runs_by_platform:
            runs_by_platform[key] = test_run
            continue

        runs_by_platform[key]['replays'].extend(test_run['replays'])

    return runs_by_platform.values()


if out_dir.exists():
    shutil.rmtree(out_dir)
out_dir.mkdir(parents=True)

# TODO: push args.* to same array in argparse so that order is preserved.
# first should always be baseline. For now, assume it is the workflow option.

all_test_runs = []
if args.workflow_run:
    print(f'collecting test runs from CI of {args.repo}')
    gh = Github(args.token)
    for run_id in args.workflow_run:
        print(f'=== collecting test runs from workflow run {run_id}')
        workflow_dir = get_gha_artifacts(gh, args.repo, run_id)
        test_runs = collect_test_runs_from_dir(workflow_dir)

        print('found:')
        for test_run in test_runs:
            print(f' - {test_run["label"]}')
        all_test_runs.extend(test_runs)
if args.local:
    for directory in args.local:
        print(f'=== collecting test runs from {directory}')
        test_runs = collect_test_runs_from_dir(directory)

        print('found:')
        for test_run in test_runs:
            print(f' - {test_run["label"]}')
        all_test_runs.extend(test_runs)


for i, test_run in enumerate(all_test_runs):
    test_run_out_dir = out_dir / str(i)
    test_run_out_dir.mkdir()

    for replay_data in test_run['replays']:
        for snapshot in replay_data['snapshots']:
            dest = test_run_out_dir / snapshot['path'].name
            shutil.copy2(snapshot['path'].absolute(), dest)
            snapshot['path'] = str(dest.relative_to(out_dir))

html = Path(f'{script_dir}/compare-resources/compare.html').read_text('utf-8')
css = Path(f'{script_dir}/compare-resources/compare.css').read_text('utf-8')
js = Path(f'{script_dir}/compare-resources/compare.js').read_text('utf-8')
deps = Path(f'{script_dir}/compare-resources/pixelmatch.js').read_text('utf-8')

result = html.replace(
    '// JAVASCRIPT', f'const testRuns = {json.dumps(all_test_runs, indent=2)}\n  {js}')
result = result.replace('// DEPS', deps)
result = result.replace('/* CSS */', css)
out_path = Path(f'{out_dir}/index.html')
out_path.write_text(result)
print(f'report written to {out_path}')
