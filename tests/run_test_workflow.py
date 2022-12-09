# Runs test workflow and downloads replay test artifacts.
# Supports directly defining a test workflow run; and using
# test_results.json files to run just failures from run_replay_tests.py.

import argparse
from argparse import ArgumentTypeError
import os
import io
import platform
import json
import requests
import zipfile
from time import sleep
from typing import List, Optional
from pathlib import Path
# pip install PyGithub
from github import Github, GithubException

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))


def arg_path(path):
    if os.path.exists(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid path')


parser = argparse.ArgumentParser()
parser.add_argument('--repo', default='ArmageddonGames/ZQuestClassic')
parser.add_argument('--token', required=True)

# Specify workflow parameters.
parser.add_argument('--commit', default='main')
parser.add_argument('--runs_on')
parser.add_argument('--arch')
parser.add_argument('--extra_args', default='')

# Automatically start multiple workflow runs based on test failures.
parser.add_argument('--test_results', type=arg_path)
parser.add_argument('--failing_workflow_run', type=int)

args = parser.parse_args()
gh = Github(args.token)

if args.test_results is not None and args.failing_workflow_run is not None:
    raise ArgumentTypeError(
        'can only choose one of --test_results or --failing_workflow_run')


def infer_gha_platform(ci: Optional[str] = ''):
    if ci:
        return ci.split('_')

    system = platform.system()
    if system == 'Windows':
        runs_on = 'windows-2022'
        arch = 'x64' if platform.architecture()[0] == '64bit' else 'win32'
    elif system == 'Darwin':
        runs_on = 'macos-12'
        arch = 'intel'
    return runs_on, arch


def download_artifact(artifact, dest):
    url = f'https://nightly.link/{args.repo}/actions/artifacts/{artifact.id}.zip'
    r = requests.get(url)
    zip = zipfile.ZipFile(io.BytesIO(r.content))
    zip.extractall(dest)
    zip.close()


def set_action_output(output_name, value):
    if 'GITHUB_OUTPUT' in os.environ:
        with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
            print('{0}={1}'.format(output_name, value), file=f)


def find_baseline_commit():
    repo = gh.get_repo(args.repo)
    ci_workflow = repo.get_workflow('ci.yml')
    main_runs = ci_workflow.get_runs(branch='main')
    most_recent_ok = next(
        (r for r in main_runs if r.conclusion == 'success'), None)
    if not most_recent_ok:
        raise Exception(
            'could not find recent successful workflow run to use as baseline')

    # GitHub currently does not support dispatching a workflow run for a specific commit (even if it is on the main branch...)
    # But! It can do refs. So let's make a dummy branch. ugh.
    # See:
    # - https://docs.github.com/en/rest/actions/workflows?apiVersion=2022-11-28#create-a-workflow-dispatch-event
    # - https://github.com/benc-uk/workflow-dispatch/issues/15
    sha = most_recent_ok.head_sha
    print(f'\nusing baseline sha: {sha}')
    dummy_branch = f'compare-baseline-{sha[:8]}'
    ref = f'refs/heads/{dummy_branch}'
    try:
        repo.create_git_ref(ref, sha)
    except GithubException as e:
        if e.data['message'] != 'Reference already exists':
            raise e

    return dummy_branch


def start_test_workflow_run(branch: str, runs_on: str, arch: str, extra_args: List[str]):
    repo = gh.get_repo(args.repo)
    test_workflow = repo.get_workflow('test.yml')

    existing_run_ids = [
        w.id for w in test_workflow.get_runs(branch=branch)]
    inputs = {
        'runs-on': runs_on,
        'arch': arch,
        'extra-args': ' '.join(extra_args),
    }
    print(f'starting run with inputs: {inputs}')
    for key, value in inputs.items():
        print(f'{key}: {value}')

    workflow_run_started = test_workflow.create_dispatch(branch, inputs)
    if not workflow_run_started:
        print('failed to workflow run')
        return

    while True:
        for w in test_workflow.get_runs(branch=branch):
            # TODO this is just a good guess. should do https://stackoverflow.com/a/69500478/2788187
            if w.id not in existing_run_ids:
                print(f'run started: {w.id}')
                return w.id

        print('waiting for run to start')
        sleep(5)


def poll_workflow_runs(run_ids: List[int]):
    repo = gh.get_repo(args.repo)
    runs = {}
    while True:
        for id in run_ids:
            if id in runs and runs[id].conclusion:
                continue

            w = runs[id] = repo.get_workflow_run(id)

            if w.conclusion != None:
                print(f'[{id}] run finished: {w.conclusion}')
                break

        if all(w.conclusion for w in runs.values()):
            print('all runs have finished')
            return

        print('waiting for run to finish')
        sleep(20)


def collect_baseline_from_test_results(test_results_paths: List[Path]):
    baseline_branch = find_baseline_commit()

    # CI runs replays sharded, so need to group the test results by platform.
    results_grouped_by_platform = {}
    for path in test_results_paths:
        test_results = json.loads(path.read_text('utf-8'))
        runs_on, arch = infer_gha_platform(test_results['ci'])

        key = (runs_on, arch)
        if key in results_grouped_by_platform:
            results_for_platform = results_grouped_by_platform[key]
        else:
            results_for_platform = results_grouped_by_platform[key] = []

        results_for_platform.append(test_results)

    run_ids = []
    for key, results_for_platform in results_grouped_by_platform.items():
        (runs_on, arch) = key
        extra_args = []

        failing_replays = []
        for test_results in results_for_platform:
            for replay_result in test_results['replays']:
                if not replay_result['success']:
                    failing_replays.append(replay_result)

        for replay_result in failing_replays:
            replay_file = replay_result['replay']
            failing_frame = replay_result['failing_frame']
            if failing_frame == None:
                print(f'no failing_frame for {replay_file} {key}, skipping')
                continue

            extra_args.append(f'--filter {replay_file}')
            extra_args.append(
                f'--snapshot {replay_file}={max(0, failing_frame-60)}-{failing_frame+60}')

        if not extra_args:
            print(f'all failing replays were invalid for {runs_on} {arch}')
            continue

        run_id = start_test_workflow_run(
            baseline_branch, runs_on, arch, extra_args)
        run_ids.append(run_id)

    print(f'all runs started: {", ".join([str(id) for id in run_ids])}')
    poll_workflow_runs(run_ids)
    print('all runs finished')

    baseline_run_id_args = ' '.join(f'--workflow_run {id}' for id in run_ids)
    set_action_output('baseline_run_id_args', baseline_run_id_args)


def collect_baseline_from_run_test_results(run_id: int):
    repo = gh.get_repo(args.repo)
    run = repo.get_workflow_run(run_id)
    replay_artifacts = [
        r for r in run.get_artifacts() if r.name.startswith('replays-')]
    if not replay_artifacts:
        raise Exception(f'no replay artifacts found for run {run_id}')

    # TODO use this for compare_replays.py too
    gha_cache_dir = script_dir / '.gha-cache-dir'
    workflow_run_dir = gha_cache_dir / str(run_id)
    workflow_run_dir.mkdir(exist_ok=True, parents=True)

    test_results_paths = []
    for artifact in replay_artifacts:
        artifact_dir = workflow_run_dir / str(artifact.name)
        if not artifact_dir.exists():
            print(f'downloading artifact: {artifact.name}')
            download_artifact(artifact, artifact_dir)
        test_results_path = next(artifact_dir.glob('test_results.json'), None)
        if test_results_path:
            test_results_paths.append(test_results_path)
        else:
            print(f'missing test_results.json for {artifact.name}')

    if not test_results_paths:
        raise Exception('found no test_results.json files')

    collect_baseline_from_test_results(test_results_paths)


if args.test_results:
    test_results_paths = []
    if args.test_results.is_dir():
        test_results_paths = list(args.test_results.rglob('test_results.json'))
    else:
        test_results_paths = [args.test_results]
    collect_baseline_from_test_results(test_results_paths)
elif args.failing_workflow_run:
    collect_baseline_from_run_test_results(args.failing_workflow_run)
else:
    extra_args = []
    if args.extra_args:
        extra_args = args.extra_args.split(' ')

    if args.runs_on and args.arch:
        runs_on, arch = args.runs_on, args.arch
    else:
        print('--runs_on and --arch not defined')
        runs_on, arch = infer_gha_platform()
        print(f'inferred from current machine: {runs_on} {arch}')

    run_id = start_test_workflow_run(args.commit, runs_on, arch, extra_args)
    poll_workflow_runs([run_id])
