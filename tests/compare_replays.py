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
import hashlib
from pathlib import Path
from github import Github
from common import get_gha_artifacts_with_retry, ReplayTestResults, RunResult
from typing import List
from PIL import Image
import hashlib


def dir_path(path):
    if os.path.isdir(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


script_dir = os.path.dirname(os.path.realpath(__file__))
out_dir = Path(f'{script_dir}/compare-report')


def hash_image(filename):
    return hashlib.sha256(Image.open(filename).tobytes()).hexdigest()


# A `test_run` represents an invocation of run_replay_tests.py, including:
#  - what platform it ran on
#  - if run in CI, the workflow run id and git ref
#  - a list of replays and any snapshots they produced
# TODO remove older `test_run` concept and use `test_results` directly in the JS instead.
def collect_test_run_from_dir(directory: Path):
    test_results_json = json.loads(
        (directory/'test_results.json').read_text('utf-8'))
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

    snapshot_paths = list(directory.rglob('*.zplay*.png'))
    if not snapshot_paths:
        print(f'{directory} has no snapshots')

    # Only process the last run of each replay.
    replay_runs: List[RunResult] = []
    for runs in reversed(test_results.runs):
        for run in runs:
            if any(r for r in replay_runs if r.name == run.name):
                continue
            replay_runs.append(run)

    test_run['replays'] = []
    for run in replay_runs:
        snapshots = [{
            'path': s,
            'frame': int(re.match(r'.*\.zplay\.(\d+)', s.name).group(1)),
            'unexpected': 'unexpected' in s.name,
            'hash': hash_image(s.absolute()),
        } for s in (directory/run.directory).rglob('*.zplay*.png')]
        if not snapshots:
            continue

        snapshots.sort(key=lambda s: s['frame'])
        test_run['replays'].append({
            'name': run.name,
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


def collect_test_runs_from_ci(gh: Github, repo: str, workflow_run_id: str):
    workflow_dir = get_gha_artifacts_with_retry(gh, repo, workflow_run_id)
    return collect_test_runs_from_dir(workflow_dir)


def create_compare_report(test_runs):
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    def count_images():
        hashes = set()
        for test_run in test_runs:
            for replay_data in test_run['replays']:
                for snapshot in replay_data['snapshots']:
                    hashes.add(snapshot['hash'])
        return len(hashes)

    if 'CI' in os.environ:
        image_count = count_images()
        if image_count > 4450:
            print(
                f'found {image_count} images, which is too many to upload to surge')
            baseline_test_run = test_runs[0]
            for test_run in test_runs[1:]:
                for replay_data in test_run['replays']:
                    filtered_snapshots = []
                    for snapshot in replay_data['snapshots']:
                        frame = snapshot['frame']
                        baseline_replay_data = next(
                            (d for d in baseline_test_run['replays'] if d['name'] == replay_data['name']), None)
                        if not baseline_replay_data:
                            continue

                        baseline_snapshot = next(
                            (s for s in baseline_replay_data['snapshots'] if s['frame'] == frame), None)
                        if not baseline_snapshot:
                            continue

                        filtered_snapshots.append(snapshot)

                    replay_data['snapshots'] = filtered_snapshots

            image_count = count_images()
            print(f'reduced to {image_count} images')

    snapshots_dir = out_dir / 'snapshots'
    snapshots_dir.mkdir()
    for test_run in test_runs:
        for replay_data in test_run['replays']:
            for snapshot in replay_data['snapshots']:
                hashsum = snapshot['hash']
                ext = snapshot['path'].suffix
                filename = f'{hashsum}{ext}'
                dest = snapshots_dir / filename
                if not dest.exists():
                    shutil.copy2(snapshot['path'].absolute(), dest)
                snapshot['path'] = str(dest.relative_to(out_dir))

    html = Path(
        f'{script_dir}/compare-resources/compare.html').read_text('utf-8')
    css = Path(
        f'{script_dir}/compare-resources/compare.css').read_text('utf-8')
    js = Path(f'{script_dir}/compare-resources/compare.js').read_text('utf-8')
    deps = Path(
        f'{script_dir}/compare-resources/pixelmatch.js').read_text('utf-8')

    result = html.replace(
        '// JAVASCRIPT', f'const testRuns = {json.dumps(test_runs, indent=2)}\n  {js}')
    result = result.replace('// DEPS', deps)
    result = result.replace('/* CSS */', css)
    out_path = Path(f'{out_dir}/index.html')
    out_path.write_text(result)
    print(f'report written to {out_path}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--workflow_run', type=int, action='append')
    parser.add_argument('--local', type=dir_path, action='append')
    parser.add_argument('--repo', default='ArmageddonGames/ZQuestClassic')
    parser.add_argument('--token')
    args = parser.parse_args()

    if not args.workflow_run and not args.local:
        raise ArgumentTypeError(
            'must provide at least one --workflow_run or --local')

    # TODO: push args.* to same array in argparse so that order is preserved.
    # first should always be baseline. For now, assume it is the workflow option.

    all_test_runs = []
    if args.workflow_run:
        print(f'collecting test runs from CI of {args.repo}')
        gh = Github(args.token)
        for run_id in args.workflow_run:
            print(f'=== collecting test runs from workflow run {run_id}')
            test_runs = collect_test_runs_from_ci(gh, args.repo, run_id)

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

    create_compare_report(all_test_runs)
