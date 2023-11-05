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
from common import get_gha_artifacts_with_retry, extract_tars, ReplayTestResults, RunResult
from typing import List
from PIL import Image
import intervaltree


def dir_path(path):
    if os.path.isdir(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


script_dir = os.path.dirname(os.path.realpath(__file__))
out_dir = Path(f'{script_dir}/compare-report')
is_ci = 'CI' in os.environ


def hash_image(filename):
    return hashlib.md5(Image.open(filename).tobytes()).hexdigest()


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
            'hash': hash_image(s.absolute()),
        } for s in (directory/run.directory).rglob('*.zplay*.png')]
        if not snapshots:
            continue

        for snapshot in snapshots:
            if 'unexpected' in snapshot['path'].name:
                snapshot['unexpected'] = True

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

    extract_tars(directory)
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

    did_prune = False
    if is_ci:
        # Surge has an unknown limit on deployment size. Let's use number of images as a proxy.
        max_image_count = 8500
        init_image_count = count_images()
        if init_image_count > max_image_count:
            print(
                f'found {init_image_count} images, which is too many to upload to surge')

            image_budget = max_image_count
            hashes_to_keep = set()
            replay_names = [r.name for r in test_runs[0].runs[0]]
            print('! num_replays', len(replay_names))

            replay_index = 0
            for replay_name in replay_names:
                # Give each replay a portion of the remaining budget.
                replay_budget = int(image_budget / (len(replay_names) - replay_index))
                replay_index += 1

                print('! replay_name', replay_name)
                print('! replay_budget', replay_budget)
                runs: List[RunResult] = []
                for test_results in test_runs:
                    for run in test_results.runs[0]:
                        if run.name == replay_name:
                            runs.append(run)

                # Get up to 60 frames around the start of every failing gfx segment.
                segments = []
                for run in runs:
                    if run.unexpected_gfx_segments:
                        for begin, _ in run.unexpected_gfx_segments:
                            segments.append([begin - 30, begin + 30])
                tree = intervaltree.IntervalTree.from_tuples(segments)
                tree.merge_overlaps(strict=False)
                segments = [[x.begin, x.end] for x in tree.items()]
                print('! segments', len(segments))

                # Each segment gets part of the image budget.
                segment_budget = int(replay_budget / len(segments))

                # That budget should never be less than 30 frames. If it is,
                # drop some segments.
                while segment_budget < 30 and len(segments) > 1:
                    print('! removed a segment')
                    segments.pop()
                    segment_budget = int(replay_budget / len(segments))

                print('! segment_budget', segment_budget)
                segment_index = 0
                for begin, end in segments:
                    # Again, give each segment a portion of the remaining budget.
                    budget = int(replay_budget / (len(segments) - segment_index))
                    segment_index += 1

                    # Start in the middle of the range (which is the failure point).
                    i = int((begin + end) / 2)
                    l = i
                    r = i + 1
                    frames = [i]
                    while l >= begin and r <= end:
                        if l >= begin:
                            frames.append(l)
                            l -= 1
                        if r <= end:
                            frames.append(r)
                            r += 1

                    for frame in frames:
                        for run in runs:
                            snapshots = (s for s in run.snapshots
                                         if s['frame'] == frame and s['hash'] not in hashes_to_keep)
                            for snapshot in snapshots:
                                hashes_to_keep.add(snapshot['hash'])
                                budget -= 1
                                image_budget -= 1
                                replay_budget -= 1
                                if budget == 0:
                                    break
                            if budget == 0:
                                break
                        if budget == 0:
                                break # lol

            # Finally, apply the filter.
            for test_results in test_runs:
                for run in test_results.runs[0]:
                    run.snapshots = [s for s in run.snapshots if s['hash'] in hashes_to_keep]

            final_image_count = count_images()
            if init_image_count != final_image_count:
                did_prune = True
                print(f'reduced to {final_image_count} images')

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
    js = f'const __TEST_RUNS__ = {test_runs_as_json}\n  {js}'

    if did_prune:
        run_ids = list(dict.fromkeys(t.workflow_run_id for t in test_runs))
        args = ' '.join(f'--workflow_run {id}' for id in run_ids)
        cmd = f'python tests/compare_replays.py {args}'
        msg = f'The full report was too large to upload, so it has been reduced. To see the full report run this command locally:\\n\\t{cmd}'
        js += f'\nconsole.log("{msg}")'

    result = html.replace('// JAVASCRIPT', js)
    result = result.replace('// DEPS', deps)
    result = result.replace('/* CSS */', css)
    out_path = Path(f'{out_dir}/index.html')
    out_path.write_text(result)
    print(f'report written to {out_path}')


# TODO use webserver.py
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
    parser.add_argument('--repo', default='ZQuestClassic/ZQuestClassic')
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
