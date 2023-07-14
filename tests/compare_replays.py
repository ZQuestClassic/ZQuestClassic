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
is_ci = 'CI' in os.environ


def hash_image(filename):
    return hashlib.sha256(Image.open(filename).tobytes()).hexdigest()


# A `test_results` represents an invocation of run_replay_tests.py, including:
#  - what platform it ran on
#  - if run in CI, the workflow run id and git ref
#  - a list of replays and any snapshots they produced
# It's mostly the same as ReplayTestResults, except only one array of ReplayRuns, and
# with some additional properties: label and snapshots.
# We re-use the ReplayTestResults dataclass for convenience.
def collect_test_results_from_dir(directory: Path) -> ReplayTestResults:
    test_results_json = json.loads(
        (directory/'test_results.json').read_text('utf-8'))
    test_results = ReplayTestResults(**test_results_json)

    label_parts = [
        test_results.runs_on,
        test_results.arch,
    ]
    if test_results.ci:
        label_parts.extend([
            f'run_id {test_results.workflow_run_id}',
            test_results.git_ref,
        ])
    test_results.label = ' '.join(label_parts)

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
        run.snapshots = snapshots

    fake_single_run = [run for run in replay_runs if run.snapshots]
    test_results.runs = [fake_single_run]

    return test_results


# `directory` can include just one test run (a folder with test_results.json);
# or many. For the latter, they will be grouped into a single test run per
# platform (so that sharding does not generate multiple test runs).
def collect_many_test_results_from_dir(directory: Path) -> List[ReplayTestResults]:
    test_runs: List[ReplayTestResults] = []

    for test_results_path in directory.rglob('test_results.json'):
        test_run_dir = test_results_path.parent
        test_runs.append(collect_test_results_from_dir(test_run_dir))

    if len(test_runs) == 0:
        raise Exception('found no test runs')

    if len(test_runs) == 1:
        return test_runs

    runs_by_platform = {}
    for test_results in test_runs:
        key = (test_results.runs_on, test_results.arch)
        if key not in runs_by_platform:
            runs_by_platform[key] = test_results
            continue

        runs_by_platform[key].runs[0].extend(test_results.runs[0])

    return runs_by_platform.values()


def collect_many_test_results_from_ci(gh: Github, repo: str, workflow_run_id: str) -> List[ReplayTestResults]:
    workflow_dir = get_gha_artifacts_with_retry(gh, repo, workflow_run_id)
    return collect_many_test_results_from_dir(workflow_dir)


def create_compare_report(test_runs: List[ReplayTestResults]):
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    def count_images():
        hashes = set()
        for test_results in test_runs:
            for run in test_results.runs[0]:
                for snapshot in run.snapshots:
                    hashes.add(snapshot['hash'])
        return len(hashes)

    if is_ci:
        image_count = count_images()
        if image_count > 4450:
            print(
                f'found {image_count} images, which is too many to upload to surge')
            baseline_test_results = test_runs[0]
            for test_results in test_runs[1:]:
                for run in test_results.runs[0]:
                    filtered_snapshots = []
                    for snapshot in run.snapshots:
                        frame = snapshot['frame']
                        baseline_replay_data = next(
                            (d for d in baseline_test_results.runs[0] if d.name == run.name), None)
                        if not baseline_replay_data:
                            continue

                        baseline_snapshot = next(
                            (s for s in baseline_replay_data.snapshots if s['frame'] == frame), None)
                        if not baseline_snapshot:
                            continue

                        filtered_snapshots.append(snapshot)

                    run.snapshots = filtered_snapshots

            image_count = count_images()
            print(f'reduced to {image_count} images')

    snapshots_dir = out_dir / 'snapshots'
    snapshots_dir.mkdir()
    for test_results in test_runs:
        for run in test_results.runs[0]:
            for snapshot in run.snapshots:
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

    # Remove unneeded data.
    for test_run in test_runs:
        for runs in test_run.runs:
            for run in runs:
                run.unexpected_gfx_frames = None
                run.unexpected_gfx_segments = None
                run.unexpected_gfx_segments_limited = None

    test_runs_as_json = ',\n'.join(test_run.to_json(indent=0) for test_run in test_runs)
    test_runs_as_json = '[\n' + test_runs_as_json + '\n]'
    result = html.replace(
        '// JAVASCRIPT', f'const __TEST_RUNS__ = {test_runs_as_json}\n  {js}')
    result = result.replace('// DEPS', deps)
    result = result.replace('/* CSS */', css)
    out_path = Path(f'{out_dir}/index.html')
    out_path.write_text(result)
    print(f'report written to {out_path}')


def start_webserver():
    from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler

    class Serv(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path == '/':
                self.path = '/index.html'
            path = out_dir / self.path[1:]

            file_to_open = None
            try:
                if path.exists():
                    file_to_open = open(path, 'rb')
                    self.send_response(200)
                    self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
                    self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
            except:
                pass

            if not file_to_open:
                self.send_response(404)

            self.end_headers()
            if file_to_open:
                print(path)
                self.wfile.write(file_to_open.read())
                file_to_open.close()

    port = 8000
    httpd = ThreadingHTTPServer(('localhost', port), Serv)
    print(f'View report at: http://localhost:{port}')
    httpd.serve_forever()


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
            test_runs = collect_many_test_results_from_ci(gh, args.repo, run_id)

            print('found:')
            for test_results in test_runs:
                print(f' - {test_results.label}')
            all_test_runs.extend(test_runs)
    if args.local:
        for directory in args.local:
            print(f'=== collecting test runs from {directory}')
            test_runs = collect_many_test_results_from_dir(directory)

            print('found:')
            for test_results in test_runs:
                print(f' - {test_results.label}')
            all_test_runs.extend(test_runs)

    create_compare_report(all_test_runs)
    if not is_ci:
        start_webserver()
