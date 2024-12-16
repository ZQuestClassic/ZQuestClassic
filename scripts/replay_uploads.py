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
# To view a specific replay, run:
#
#   python tests/run_replay_tests.py $PWD/.tmp/replay_uploads/D2905F6A4976CBC02C3ACC92AD6CB12C/c4c21082-dc31-49e0-9f35-d18bfb87c841-updated.zplay --show
#
# TODO: upload the `-updated.zplay` copies to the s3 bucket
# TODO: consider running in CI; requires incorporating with compare report generation

import argparse
import os
import shlex
import shutil
import subprocess
import sys

from pathlib import Path

import boto3
import database

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
replays_dir = root_dir / '.tmp/replay_uploads'

os.sys.path.append(str((root_dir / 'tests/lib').absolute()))
import replay_helpers

bucket_name = 'zc-replays'

boto3.setup_default_session(profile_name='zc-replays')
s3 = boto3.resource(
    's3',
    region_name='nyc3',
    endpoint_url='https://nyc3.digitaloceanspaces.com',
)
bucket = s3.Bucket(bucket_name)

db = database.Database()
get_qst_keys_by_hash = db.get_qst_keys_by_hash()
all_keys = list(_.key for _ in bucket.objects.all())
original_keys = [key for key in all_keys if '-updated.zplay' not in key]


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

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument(
    '--update', action='store_true', help='Update all the replay copies (default)'
)
parser.add_argument(
    '--assert', action='store_true', help='Assert all the replays', dest='assertmode'
)
args = parser.parse_args()

print('All replays downloaded from the upload bucket\n')
print(f'Directory: {replays_dir}')
print(f'Total: {len(original_keys)}')

frames = 0
for key in original_keys:
    path = download_replay(key)
    meta = replay_helpers.read_replay_meta(path)
    frames += meta['frames']

print(f'Frames: {frames}')
print('Hours: {0:.1f}'.format(frames / 60 / 60 / 60))

mode = None
if args.update:
    mode = 'update'
elif args.assertmode:
    mode = 'assert'
else:
    mode = 'update'

if mode:
    updated_keys = [get_updated_key(key) for key in original_keys]
    exclude_keys = [
        # This replay has an RNG desync even when played in the same release it was made in.
        # Warrants further investigation.
        'DFA6082440977B8C2C0BD323D17068C4/f064251f-0507-44ef-8cc3-42c0c71dd979-updated.zplay'
    ]
    updated_keys = [key for key in updated_keys if key not in exclude_keys]
    updated_paths = [download_replay(key) for key in updated_keys]

    subprocess.run(
        [
            sys.executable,
            root_dir / 'tests/run_replay_tests.py',
            f'--{mode}',
            *updated_paths,
        ]
    )
