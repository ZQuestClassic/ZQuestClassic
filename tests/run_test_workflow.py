# Runs test workflow and downloads replay test artifacts.
# Supports directly defining a test workflow run; and using
# test_results.json files to run just failures from run_replay_tests.py.

import argparse
from argparse import ArgumentTypeError
import os
import json
from time import sleep
from typing import List
from pathlib import Path
# pip install PyGithub
from github import Github, GithubException
from common import infer_gha_platform, get_gha_artifacts, ReplayTestResults

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
parser.add_argument('--compiler')
parser.add_argument('--extra_args', default='')

# Automatically start multiple workflow runs based on test failures.
parser.add_argument('--test_results', type=arg_path)
parser.add_argument('--failing_workflow_run', type=int)

args = parser.parse_args()
gh = Github(args.token)

if args.test_results is not None and args.failing_workflow_run is not None:
    raise ArgumentTypeError(
        'can only choose one of --test_results or --failing_workflow_run')


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


def start_test_workflow_run(branch: str, runs_on: str, arch: str, compiler: str, extra_args: List[str]):
    repo = gh.get_repo(args.repo)
    test_workflow = repo.get_workflow('test.yml')

    existing_run_ids = [
        w.id for w in test_workflow.get_runs(branch=branch)]
    inputs = {
        'runs-on': runs_on,
        'arch': arch,
        'compiler': compiler,
        'extra-args': ' '.join(extra_args),
    }
    print(f'starting run with inputs: {inputs}')
    for key, value in inputs.items():
        print(f'{key}: {value}')

    workflow_run_started = test_workflow.create_dispatch(branch, inputs)
    if not workflow_run_started:
        print('failed to start workflow run')
        return

    while True:
        for w in test_workflow.get_runs(branch=branch):
            # TODO this is just a good guess. should do https://stackoverflow.com/a/69500478/2788187
            if w.id not in existing_run_ids:
                print(f'run started: {w.id}')
                return w.id

        print('waiting for run to start')
        sleep(5)


def poll_workflow_run(run_id: int):
    repo = gh.get_repo(args.repo)

    retries_left = 2
    while True:
        w = repo.get_workflow_run(run_id)
        if w.conclusion == None:
            print('waiting for run to finish')
            sleep(20)
            continue

        print(f'[{run_id}] run finished: {w.conclusion}')

        if w.conclusion != 'success':
            if retries_left == 0:
                raise Exception('failed to run workflow too many times')

            print('re-running workflow')
            retries_left -= 1
            if not w.rerun():
                raise Exception('failed to re-run workflow')

            continue

        break


# Collect all the test failures described by the provided test_results.json
# files, and dispatch and wait for a workflow run to finish using a baseline
# commit.
def collect_baseline_from_test_results(test_results_paths: List[Path]):
    failing_frames_by_replay = {}
    for path in test_results_paths:
        test_results_json = json.loads(path.read_text('utf-8'))
        test_results = ReplayTestResults(**test_results_json)

        for run in test_results.runs[-1]:
            if run.success:
                continue

            if run.failing_frame == None:
                print(f'{path}: no failing_frame for {run.name}, skipping')
                continue

            if run.name not in failing_frames_by_replay:
                failing_frames_by_replay[run.name] = []
            failing_frames_by_replay[run.name].append(run.failing_frame)

    extra_args = []
    for replay_name, failing_frames in failing_frames_by_replay.items():
        for failing_frame in set(failing_frames):
            extra_args.append(f'--filter {replay_name}')
            extra_args.append(
                f'--snapshot {replay_name}={max(0, failing_frame-60)}-{failing_frame+60}')
            extra_args.append(f'--frame {replay_name}={failing_frame+60}')

    if not extra_args:
        raise Exception('all failing replays were invalid')

    # For baseline purposes, only need to run on a single platform.
    run_id = start_test_workflow_run(
        find_baseline_commit(), 'ubuntu-22.04', 'x64', 'clang', extra_args)
    poll_workflow_run(run_id)
    print('run finished')
    set_action_output('baseline_run_id', run_id)


def collect_baseline_from_failing_workflow_run(run_id: int):
    workflow_run_dir = get_gha_artifacts(gh, args.repo, run_id)
    test_results_paths = list(workflow_run_dir.rglob('test_results.json'))
    collect_baseline_from_test_results(test_results_paths)


if args.test_results:
    test_results_paths = []
    if args.test_results.is_dir():
        test_results_paths = list(args.test_results.rglob('test_results.json'))
    else:
        test_results_paths = [args.test_results]
    collect_baseline_from_test_results(test_results_paths)
elif args.failing_workflow_run:
    collect_baseline_from_failing_workflow_run(args.failing_workflow_run)
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

    run_id = start_test_workflow_run(
        args.commit, runs_on, arch, args.compiler, extra_args)
    poll_workflow_run(run_id)
