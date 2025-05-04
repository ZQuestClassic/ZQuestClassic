# Downloads all the replays uploaded via the replay upload service,
# copies them to a new replay file that can be updated without modifying
# the original, and enables running all of them via run_replay_tests.py.
#
# This requires an auth key placed at `~/.aws/credentials`
# (or `C:\Users\USERNAME\.aws\credentials`) under `[zc-replays]`.
# Ask Connor to get a personal key.
#
# Note this currently accepts any graphical changes, and will only report
# an error if a replay fails to update due to behavioral changes (ex: rng
# desync, not loading into a screen on the expected frame, etc).
#
# To see a list of all uploaded replays grouped by qst, run:
#
#   python scripts/replay_uploads.py
#
# To attempt to update the local copy of all replays using the current local build, run:
#
#   python scripts/replay_uploads.py --update
#
# These replays can be used to validate changes - but only by exluding the
# "known bad" replays which currently fail to update for some unknown reason.
# For example:
#
#   You have a change you want more validation for. Go to the commit just
#   before, build, and run `python scripts/replay_uploads.py --update --update_known_good`
#   (some will fail to update, that is ok). Then, with your new change, run
#   `python scripts/replay_uploads.py --assert --only_known_good`. If
#   anything fails, generate a compare report and investigate.
#
# To debug why a replay can't be updated, run:
#
#   python scripts/replay_uploads.py --debug 50C00A5197148F3D1587A16509DCC89F/0be80dd3-c719-4203-a314-fec7e9557a7f.zplay
#
# To view a specific replay, run:
#
#   python tests/run_replay_tests.py $PWD/.tmp/replay_uploads/D2905F6A4976CBC02C3ACC92AD6CB12C/c4c21082-dc31-49e0-9f35-d18bfb87c841-updated.zplay --show
#
# TODO: consider running in CI (see the "For example" above)

import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys

from pathlib import Path

import archives
import boto3
import common
import database

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = root_dir / '.tmp/replay_uploads'

os.sys.path.append(str((root_dir / 'tests/lib').absolute()))
os.sys.path.append(str((root_dir / 'tests').absolute()))
import replay_helpers

from replays import ReplayTestResults

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument(
    '--update', action='store_true', help='Update all the local replay copies'
)
parser.add_argument(
    '--assert',
    action='store_true',
    help='Assert all the local replay copies',
    dest='assertmode',
)
parser.add_argument(
    '--only_known_good',
    action='store_true',
    help='Filter out all replays that are not known to be good / update-able',
)
parser.add_argument(
    '--update_known_good',
    action='store_true',
    help='Update the checked-in file for which replays are update-able',
)
parser.add_argument('--debug', help='Debug why the given replay cannot be updated')
args = parser.parse_args()

bucket = None


def connect_bucket():
    global bucket

    bucket_name = 'zc-replays'
    boto3.setup_default_session(profile_name='zc-replays')
    s3 = boto3.resource(
        's3',
        region_name='nyc3',
        endpoint_url='https://nyc3.digitaloceanspaces.com',
    )
    bucket = s3.Bucket(bucket_name)


def prepare_and_get_replay_keys():
    connect_bucket()

    db = database.Database()
    get_qst_keys_by_hash = db.get_qst_keys_by_hash()
    all_keys = list(_.key for _ in bucket.objects.all())
    # Note: currently no plans to actual upload updated replays back to bucket.
    original_keys = [key for key in all_keys if '-updated.zplay' not in key]

    print('All replays downloaded from the upload bucket\n')
    print(f'Directory: {replays_dir}')
    print(f'Total: {len(original_keys)}')

    # Download everything.
    for key in all_keys:
        download_replay(key)

    # Group replays by qst_hash.
    keys_by_qst_hash = {}
    for key in original_keys:
        qst_hash = key.split('/')[0]
        if qst_hash not in keys_by_qst_hash:
            keys_by_qst_hash[qst_hash] = []
        keys_by_qst_hash[qst_hash].append(key)

    # Copy qst files.
    for qst_hash in keys_by_qst_hash.keys():
        qst_key, qst_release = get_qst_keys_by_hash[qst_hash]
        db.download_release(qst_release)
        qst_path = db.download(qst_key)

        qst_replays_dir = replays_dir / qst_hash
        copied_qst_path = qst_replays_dir / f'{qst_hash}.qst'
        if not copied_qst_path.exists():
            shutil.copyfile(qst_path, copied_qst_path)

    for key in original_keys:
        init_updated_replay(key)

    return original_keys


def download_replay(key: str) -> Path:
    path = replays_dir / key
    if path.exists():
        return path

    print(f'downloading: {key}')
    path.parent.mkdir(exist_ok=True, parents=True)
    bucket.download_file(key, path)

    return path


def get_updated_key(key: str):
    return key.replace('.zplay', '-updated.zplay')


def init_updated_replay(key: str):
    original_replay_path = download_replay(key)
    updated_replay_path = original_replay_path.with_name(
        original_replay_path.name.replace('.zplay', '-updated.zplay')
    )
    if updated_replay_path.exists():
        return

    lines = original_replay_path.read_text().splitlines()
    for i, line in enumerate(lines):
        if line.startswith('M qst '):
            qst_hash = key.split('/')[0]
            lines[i] = f'M qst {qst_hash}.qst'
            break

    updated_replay_path.write_text('\n'.join(lines))


def run_replays_process(args):
    test_results_folder = root_dir / '.tmp/replay_uploads_test_results'
    if test_results_folder.exists():
        shutil.rmtree(test_results_folder)
    subprocess.run(
        [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            '--test_results_folder',
            test_results_folder,
            '--root_replays_folder',
            replays_dir,
            *args,
        ]
    )
    test_results_json = json.loads(
        (test_results_folder / 'test_results.json').read_text('utf-8')
    )
    test_results = ReplayTestResults(**test_results_json)
    return test_results


def run_replays_process_with_version(zc_version, args):
    release_platform = common.get_release_platform()
    build_folder = archives.download(zc_version, release_platform)
    if release_platform == 'mac':
        zc_app_path = next(build_folder.glob('*.app'))
        build_folder = zc_app_path / 'Contents/Resources'
    return run_replays_process([*args, '--build_folder', build_folder])


def run_replays():
    if args.update:
        mode = 'update'
    elif args.assertmode:
        mode = 'assert'

    original_keys = prepare_and_get_replay_keys()
    exclude_keys = [
        # This replay has an RNG desync even when played in the same release it was made in.
        # Warrants further investigation.
        'DFA6082440977B8C2C0BD323D17068C4/f064251f-0507-44ef-8cc3-42c0c71dd979.zplay'
    ]
    original_keys = [key for key in original_keys if key not in exclude_keys]
    if args.only_known_good:
        known_good_path = Path(script_dir / 'replay_uploads_known_good_replays.json')
        known_good = json.loads(known_good_path.read_text())
        original_keys = [k for k in original_keys if k in known_good]
    updated_keys = [get_updated_key(key) for key in original_keys]
    # Note: shouldn't actually download anything b/c updated replays are all local.
    updated_paths = [download_replay(key) for key in updated_keys]

    test_results = run_replays_process([f'--{mode}', *updated_paths])

    if args.update_known_good:
        known_good = []
        for run in test_results.runs[0]:
            if run.success:
                index = updated_paths.index(Path(run.path))
                known_good.append(original_keys[index])
        known_good.sort()
        known_good_path = Path(script_dir / 'replay_uploads_known_good_replays.json')
        known_good_path.write_text(json.dumps(known_good, indent=2))

    failing_runs = []
    for run in test_results.runs[-1]:
        if not run.success:
            if '0FF17D261771D591F9B3BF1919A39D88' in run.path:
                print(run)
            failing_runs.append(run)

    if failing_runs:
        print('These replays failed (to debug, use `--debug replay/path.zplay`):\n')

        for run in failing_runs:
            meta = replay_helpers.read_replay_meta(Path(run.path))
            title = meta.get('qst_title', meta['qst'])
            print(Path(run.path).relative_to(replays_dir))
            print(title)
            print()
    else:
        print('All replays passed')


def print_info():
    original_keys = prepare_and_get_replay_keys()

    replays_by_qst_hash = {}
    frames = 0
    for key in original_keys:
        path = download_replay(key)
        meta = replay_helpers.read_replay_meta(path)
        frames += meta['frames']

        qst_hash = meta['qst_hash']
        if qst_hash not in replays_by_qst_hash:
            replays_by_qst_hash[qst_hash] = []
        replays_by_qst_hash[qst_hash].append(path)

    print(f'Frames: {frames}')
    print('Hours: {0:.1f}'.format(frames / 60 / 60 / 60))
    print()

    for qst_hash, paths in replays_by_qst_hash.items():
        paths.sort(key=lambda p: -replay_helpers.read_replay_meta(p)['frames'])

        title = replay_helpers.read_replay_meta(paths[0]).get('qst_title', qst_hash)
        print(f'{title.strip()} ({len(paths)})')
        for path in paths:
            meta = replay_helpers.read_replay_meta(path)
            frames = meta['frames']
            print(f'\t{path.relative_to(replays_dir)} ({frames} frames)')
        print()


# Finds the most recent ZC version that can update the replay, does that update, and prints a
# command to run to see a compare report against the current local build.
def debug_replay(replay_path: Path):
    if '-updated' not in replay_path.name:
        replay_path = replay_path.with_stem(f'{replay_path.stem}-updated')

    replay_path = replays_dir / replay_path
    meta = replay_helpers.read_replay_meta(replay_path)
    title = meta.get('qst_title', meta['qst'])
    zc_version_updated = meta['zc_version_updated']
    print(f'title: {title}')
    print(f'zc_version_updated: {zc_version_updated}')

    tmp_path = replay_path.with_stem(f'{replay_path.stem}-tmp')
    shutil.copyfile(replay_path, tmp_path)

    # Validate that this replay can even be updated in the version that created it.
    test_results = run_replays_process_with_version(
        zc_version_updated, [tmp_path, '--update']
    )
    if not test_results.runs[-1][0].success:
        print('cannot update the replay even in the same ZC version it was created in')
        tmp_path.unlink()
        sys.exit(1)

    shutil.copyfile(replay_path, tmp_path)
    test_results = run_replays_process([tmp_path, '--update'])
    if test_results.runs[-1][0].success:
        print(test_results.runs[-1][0])
        print(
            'was able to update the replay with the current build - nothing to debug.'
        )
        tmp_path.unlink()
        sys.exit()

    # Reduce the replay to just the part that is failing.
    stopped_frame = test_results.runs[-1][0].frame
    lines = tmp_path.read_text().splitlines()
    for i, line in enumerate(lines):
        type, frame, _ = line.split(' ', 2)
        if type == 'M':
            continue

        frame = int(frame)
        # Go 1s beyond where it fails.
        if frame > stopped_frame + 60:
            lines = lines[: i + 1]
            break

    tmp_trimmed_path = replay_path.with_stem(f'{replay_path.stem}-tmp-trimmed')
    tmp_trimmed_path.write_text('\n'.join(lines))
    shutil.copyfile(tmp_trimmed_path, tmp_path)

    # Bisect to find the newest version that can update this replay.

    # First, assume and validate the good/bad versions.
    if zc_version_updated == '2.55.7':
        good_version = '3.0.0-prerelease.76+2024-10-19'
    elif zc_version_updated == '2.55.8':
        good_version = '3.0.0-prerelease.85+2025-01-10'
    elif zc_version_updated == '2.55.9':
        good_version = '3.0.0-prerelease.95+2025-03-25'
    else:
        good_version = '3.0.0-prerelease.1+2023-11-12'
    bad_version = archives.get_revisions(common.get_release_platform(), 'main')[-1].tag
    print(f'assuming good: {good_version}, bad: {bad_version}')

    test_results = run_replays_process_with_version(
        good_version, [tmp_path, '--update']
    )
    if not test_results.runs[-1][0].success:
        print(
            f'was not able to update the replay with the assumed "good" version: {good_version}.'
        )
        tmp_path.unlink()
        tmp_trimmed_path.unlink()
        sys.exit(1)

    shutil.copyfile(tmp_trimmed_path, tmp_path)
    test_results = run_replays_process_with_version(bad_version, [tmp_path, '--update'])
    if test_results.runs[-1][0].success:
        print(
            f'was able to update the replay with the assumed "bad" version: {bad_version}.'
        )
        tmp_path.unlink()
        sys.exit(1)

    print('Running bisect...')
    output = subprocess.check_output(
        [
            sys.executable,
            root_dir / 'scripts/bisect_builds.py',
            '--good',
            good_version,
            '--bad',
            bad_version,
            '--check_return_code',
            '--',
            'bash',
            '-c',
            f'cp {tmp_trimmed_path} {tmp_path} && %zc -v0 -headless -replay-exit-when-done -update {tmp_path}',
        ],
        encoding='utf-8',
        stderr=subprocess.STDOUT,
    )

    bisect_done = False
    last_good_version = None
    for line in output.splitlines():
        print(line)
        if bisect_done:
            # print(line)
            if line.startswith('GOOD'):
                last_good_version = line[4:].strip()
        elif line.strip() == 'bisect finished!':
            bisect_done = True

    print('\nBisect done.')
    print(f'Last version that can update replay: {last_good_version}')

    shutil.copyfile(tmp_trimmed_path, tmp_path)
    test_results = run_replays_process_with_version(
        last_good_version, [tmp_path, '--update']
    )
    if not test_results.runs[-1][0].success:
        print(
            f'was unable to update the replay with the calculated "last good" version: {last_good_version}.'
        )
        tmp_path.unlink()
        tmp_trimmed_path.unlink()
        sys.exit(1)

    debug_replay_path = replay_path.with_stem(f'{replay_path.stem}-debug')
    shutil.copyfile(tmp_path, debug_replay_path)
    print(f'Replayed updated at: {debug_replay_path}')

    print('\nTo see a compare report, run:')
    print(
        f'  python tests/run_replay_tests.py --baseline_version {last_good_version} {debug_replay_path}'
    )

    tmp_path.unlink()
    tmp_trimmed_path.unlink()


if args.update or args.assertmode:
    run_replays()
elif args.debug:
    debug_replay(Path(args.debug))
else:
    print_info()
    print('exiting (use --assert or --update to run replays)')
    sys.exit()
