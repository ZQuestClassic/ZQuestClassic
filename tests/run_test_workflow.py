# Runs test workflow and downloads replay test artifacts.
# Supports directly defining a test workflow run; and using
# test_results.json files to run just failures from run_replay_tests.py.

import argparse
import json
import os

from argparse import ArgumentTypeError
from pathlib import Path
from time import sleep
from typing import List

import intervaltree

from github import Github, GithubException, PaginatedList, WorkflowRun
from replays import ReplayTestResults
from workflow_job import WorkflowJob

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

os.sys.path.append(str((root_dir / 'scripts').absolute()))
from github_helpers import extract_tars, get_gha_artifacts


def arg_path(path):
    if os.path.exists(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid path')


def set_action_output(output_name, value):
    if 'GITHUB_OUTPUT' in os.environ:
        with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
            print('{0}={1}'.format(output_name, value), file=f)


def find_baseline(gh: Github, repo_str: str):
    def is_passing_workflow_run(r: WorkflowRun.WorkflowRun):
        if r.conclusion == 'success':
            return True

        # TODO: remove when https://github.com/PyGithub/PyGithub/pull/1951 is released.
        jobs = PaginatedList.PaginatedList(
            WorkflowJob,
            r._requester,
            r.jobs_url,
            dict(),
            list_item='jobs',
        )

        ignore_jobs = ['web', 'compare']
        if all(
            job.conclusion == 'success' for job in jobs if job.name not in ignore_jobs
        ):
            return True

        return False

    repo = gh.get_repo(repo_str)
    ci_workflow = repo.get_workflow('ci.yml')
    main_runs = ci_workflow.get_runs(branch='main')
    most_recent_ok = next(
        (r for r in main_runs if is_passing_workflow_run(r)),
        None,
    )
    if not most_recent_ok:
        raise Exception(
            'could not find recent successful workflow run to use as baseline'
        )

    return create_compare_git_ref(gh, repo_str, most_recent_ok.head_sha)


def create_compare_git_ref(gh: Github, repo_str: str, sha: str):
    # GitHub currently does not support dispatching a workflow run for a specific commit (even if it is on the main branch...)
    # But! It can do refs. So let's make a dummy branch. ugh.
    # See:
    # - https://docs.github.com/en/rest/actions/workflows?apiVersion=2022-11-28#create-a-workflow-dispatch-event
    # - https://github.com/benc-uk/workflow-dispatch/issues/15
    print(f'\nusing baseline sha: {sha}')
    repo = gh.get_repo(repo_str)
    dummy_branch = f'compare-baseline-{sha[:8]}'
    ref = f'refs/heads/{dummy_branch}'
    try:
        repo.create_git_ref(ref, sha)
    except GithubException as e:
        # This error happens when running from fork, since lack write permissions.
        if e.data['message'] == 'Resource not accessible by integration':
            print(e)
            print('assuming baseline is head of main branch ...')
            return 'main'
        if e.data['message'] != 'Reference already exists':
            raise e

    return dummy_branch


def start_test_workflow_run(
    gh: Github,
    repo_str: str,
    branch: str,
    runs_on: str,
    arch: str,
    compiler: str,
    extra_args: List[str],
):
    repo = gh.get_repo(repo_str)
    test_workflow = repo.get_workflow('test.yml')

    existing_run_ids = [w.id for w in test_workflow.get_runs(branch=branch)]
    inputs = {
        'runs-on': runs_on,
        'arch': arch,
        'compiler': compiler,
        'extra-args': ' '.join(f'"{x}"' if ' ' in x else x for x in extra_args),
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


def poll_workflow_run(gh: Github, repo_str: str, run_id: int):
    repo = gh.get_repo(repo_str)

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


# Returns a list of args to give to `run_replay_tests.py`, to collect snapshots of the failing replay
# tests provided by `test_results_paths`
def get_args_for_collect_baseline_from_test_results(
    test_results_paths: List[Path],
) -> List[str]:
    failing_segments_by_replay = {}
    name_to_path = {}
    for path in test_results_paths:
        test_results_json = json.loads(path.read_text('utf-8'))
        test_results = ReplayTestResults(**test_results_json)

        for run in test_results.runs[-1]:
            if run.success:
                continue

            if run.failing_frame == None:
                print(f'{path}: no failing_frame for {run.name}, skipping (it probably crashed)')
                continue

            name_to_path[run.name] = run.path

            if run.name not in failing_segments_by_replay:
                failing_segments_by_replay[run.name] = []
            # Capture the very first frame (this covers non-gfx failures).
            failing_segments_by_replay[run.name].append(
                [run.failing_frame, run.failing_frame]
            )
            # ...and all unexpected gfx segments (but, the limited variant).
            if run.unexpected_gfx_segments_limited:
                failing_segments_by_replay[run.name].extend(
                    run.unexpected_gfx_segments_limited
                )

    args = []
    last_args = []
    for replay_name, failing_segments in failing_segments_by_replay.items():
        replay_path = Path(name_to_path[replay_name])
        if (script_dir / 'replays' / replay_name).exists():
            args.append(f'--filter={replay_name}')
        else:
            last_args.append(name_to_path[replay_name])
        ranges = []
        for start, end in failing_segments:
            # Add some context around these snapshot ranges.
            ranges.append([max(0, start - 60), end + 60])
        tree = intervaltree.IntervalTree.from_tuples(ranges)
        tree.merge_overlaps(strict=False)
        intervals_str = ' '.join(f'{i.begin}-{i.end}' for i in tree.items())
        args.append(f'--snapshot={replay_name}={intervals_str}')
        max_frame = max([segment[1] for segment in ranges])
        args.append(f'--frame={replay_name}={max_frame}')

    args.extend(last_args)

    if not args:
        raise Exception('all failing replays were invalid')

    return args


# Collect all the test failures described by the provided test_results.json
# files, and dispatch and wait for a workflow run to finish using a baseline
# branch.
# Returns the workflow run id, after job finishes.
def collect_baseline_from_test_results(
    gh: Github, repo: str, baseline_branch: str, test_results_paths: List[Path]
) -> int:
    extra_args = get_args_for_collect_baseline_from_test_results(test_results_paths)
    if not baseline_branch:
        baseline_branch = find_baseline(gh, repo)

    # For baseline purposes, only need to run on a single platform.
    run_id = start_test_workflow_run(
        gh, repo, baseline_branch, 'ubuntu-22.04', 'x64', 'clang', extra_args
    )
    poll_workflow_run(gh, repo, run_id)
    print('run finished')
    return run_id


def collect_baseline_from_failing_workflow_run(
    gh: Github, repo: str, baseline_branch: str, run_id: int
):
    workflow_run_dir = get_gha_artifacts(gh, repo, run_id)
    test_results_paths = list(workflow_run_dir.rglob('test_results.json'))
    return collect_baseline_from_test_results(
        gh, repo, baseline_branch, test_results_paths
    )


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', default='ZQuestClassic/ZQuestClassic')
    parser.add_argument('--token', required=True)

    # Specify workflow parameters.
    parser.add_argument('--commit')
    parser.add_argument('--runs_on', default='ubuntu-22.04')
    parser.add_argument('--arch', default='x64')
    parser.add_argument('--compiler')
    parser.add_argument('--extra_args', default='')

    # Automatically start multiple workflow runs based on test failures.
    parser.add_argument('--test_results', type=arg_path)
    parser.add_argument('--failing_workflow_run', type=int)

    args = parser.parse_args()
    gh = Github(args.token)

    if args.test_results is not None and args.failing_workflow_run is not None:
        raise ArgumentTypeError(
            'can only choose one of --test_results or --failing_workflow_run'
        )

    # This could be a commit sha or a branch.
    if args.commit:
        if len(args.commit) == 40:
            branch = create_compare_git_ref(gh, args.repo, args.commit)
        else:
            branch = args.commit
    else:
        branch = find_baseline(gh, args.repo)

    if args.test_results:
        test_results_paths = []
        if args.test_results.is_dir():
            extract_tars(args.test_results)
            test_results_paths = list(args.test_results.rglob('test_results.json'))
        else:
            test_results_paths = [args.test_results]
        baseline_run_id = collect_baseline_from_test_results(
            gh, args.repo, branch, test_results_paths
        )
        set_action_output('baseline_run_id', baseline_run_id)
    elif args.failing_workflow_run:
        baseline_run_id = collect_baseline_from_failing_workflow_run(
            gh, args.repo, branch, args.failing_workflow_run
        )
        set_action_output('baseline_run_id', baseline_run_id)
    else:
        extra_args = []
        if args.extra_args:
            extra_args = args.extra_args.split(' ')

        if args.compiler:
            compiler = args.compiler
        elif args.runs_on.startswith('windows'):
            compiler = 'msvc'
        elif args.runs_on.startswith('mac'):
            compiler = 'clang'
        elif args.runs_on.startswith('ubuntu'):
            compiler = 'clang'

        run_id = start_test_workflow_run(
            gh, args.repo, branch, args.runs_on, args.arch, compiler, extra_args
        )
        poll_workflow_run(gh, args.repo, run_id)
