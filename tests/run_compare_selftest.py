# Self-test for the CI compare report pipeline.
#
# Dispatches a Test workflow run of a single replay on the given branch, with a zplayer debug flag
# (-replay-debug-corrupt-frame) that corrupts one frame so the replay fails. Once that run fails,
# this runs the same command the compare workflow runs (`ci.py compare-replays`) against it:
# that dispatches a baseline Test run on a compare-baseline-* branch and builds the HTML report in
# tests/compare-report.
#
# Nothing is published unless --publish is given (otherwise SURGE_TOKEN and
# REPLAY_FAILURE_DISCORD_WEBHOOK are ignored).
#
# Examples:
#   python tests/run_compare_selftest.py --repo connorjclark/ZeldaClassic --branch main
#   python tests/run_compare_selftest.py --repo ZQuestClassic/ZQuestClassic --branch releases/2.55
#   # Skip the dispatch and build a report for an existing failed CI or Test run:
#   python tests/run_compare_selftest.py --repo ZQuestClassic/ZQuestClassic --run-id 123456789

import argparse
import os
import shutil
import subprocess
import sys
import time

from pathlib import Path

from github import Github

script_dir = Path(__file__).parent.absolute()
root_dir = script_dir.parent
sys.path.append(str(script_dir))

from run_test_workflow import start_test_workflow_run

parser = argparse.ArgumentParser(
    description='Check that a compare report gets generated for a failing replay run.'
)
parser.add_argument('--repo', default='ZQuestClassic/ZQuestClassic')
parser.add_argument(
    '--branch', default='main', help='Branch to run the failing replay on'
)
parser.add_argument('--replay', default='classic_1st_lvl1.zplay')
parser.add_argument(
    '--corrupt-frame', type=int, default=300, help='Frame of the replay to corrupt'
)
parser.add_argument('--runs-on', default='ubuntu-22.04')
parser.add_argument('--arch', default='x64')
parser.add_argument('--compiler', default='clang')
parser.add_argument(
    '--run-id', type=int, help='Use this existing failed run instead of dispatching one'
)
parser.add_argument(
    '--publish',
    action='store_true',
    help='Deploy the report to surge and notify Discord, if those env vars are set',
)
parser.add_argument(
    '--token', help='GitHub token (default: $GITHUB_TOKEN, then `gh auth token`)'
)
args = parser.parse_args()


def get_token():
    if args.token:
        return args.token
    if os.environ.get('GITHUB_TOKEN'):
        return os.environ['GITHUB_TOKEN']
    return subprocess.check_output(['gh', 'auth', 'token'], text=True).strip()


def wait_for_run(repo, run_id):
    start = time.time()
    last_status = None
    while True:
        run = repo.get_workflow_run(run_id)
        if run.status != last_status:
            print(f'[{int(time.time() - start)}s] run {run_id}: {run.status}')
            last_status = run.status
        if run.status == 'completed':
            return run
        time.sleep(30)


token = get_token()
gh = Github(token)
repo = gh.get_repo(args.repo)

if args.run_id:
    run_id = args.run_id
    run = repo.get_workflow_run(run_id)
    print(f'using existing run: {run.html_url}')
else:
    # Stop shortly after the corrupted frame, to keep the runs short.
    stop_frame = args.corrupt_frame + 60
    extra_args = [
        f'--filter={args.replay}',
        f'--frame={args.replay}={stop_frame}',
        f'--extra_args=-replay-debug-corrupt-frame {args.corrupt_frame}',
    ]
    run_id = start_test_workflow_run(
        gh, args.repo, args.branch, args.runs_on, args.arch, args.compiler, extra_args
    )
    if not run_id:
        sys.exit('failed to dispatch the test workflow')
    print(f'dispatched: https://github.com/{args.repo}/actions/runs/{run_id}')
    run = wait_for_run(repo, run_id)

if run.conclusion != 'failure':
    sys.exit(
        f'run {run_id} concluded "{run.conclusion}", expected "failure". '
        'If it passed, the branch probably lacks the -replay-debug-corrupt-frame flag.'
    )

# ci.py downloads the failing run's artifacts here; clear out any from a previous run.
shutil.rmtree(root_dir / 'test-results', ignore_errors=True)
report_dir = root_dir / 'tests/compare-report'
shutil.rmtree(report_dir, ignore_errors=True)

env = os.environ.copy()
env['GITHUB_TOKEN'] = token
if not args.publish:
    env.pop('SURGE_TOKEN', None)
    env.pop('REPLAY_FAILURE_DISCORD_WEBHOOK', None)

print('running compare-replays (this dispatches and waits for a baseline run) ...')
subprocess.check_call(
    [
        sys.executable,
        '-Xutf8',
        str(root_dir / '.github/workflows/ci.py'),
        'compare-replays',
        '--repo',
        args.repo,
        '--run-id',
        str(run_id),
        '--ref-name',
        run.head_branch,
        '--actor',
        'compare-selftest',
    ],
    cwd=root_dir,
    env=env,
)

index = report_dir / 'index.html'
if not index.exists():
    sys.exit(f'no report was generated at {index}')
print(f'report generated: {index}')
