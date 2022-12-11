# Given various sources of snapshots (either from local filesystem, or
# downloaded from a GitHub Actiosn workflow run), generates an HTML report
# presenting the snapshots frame-by-frame, and their differences.
#
# You must run a local web server for the HTML to work:
#     cd tests/compare-tracks
#     python -m http.server 8000

# TODO: many things could use better names!

import argparse
from argparse import ArgumentTypeError
import requests
import io
import os
import re
import json
import zipfile
import shutil
from pathlib import Path
from itertools import groupby
from github import Github


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

# See:
# - https://nightly.link/
# - https://github.com/actions/upload-artifact/issues/51
if args.workflow_run and not args.token:
    print('no token detected, will download artifacts via nightly.link')


if not args.workflow_run and not args.local:
    raise ArgumentTypeError(
        'must provide at least one --workflow_run or --local')


gh = Github(args.token)
repo = gh.get_repo(args.repo)
script_dir = os.path.dirname(os.path.realpath(__file__))
tracks_dir = f'{script_dir}/compare-tracks'


def download_artifact(artifact, dest):
    if args.token:
        r = requests.get(artifact.archive_download_url, headers={
            'authorization': f'Bearer {args.token}',
        })
    else:
        url = f'https://nightly.link/{args.repo}/actions/artifacts/{artifact.id}.zip'
        r = requests.get(url)

    zip = zipfile.ZipFile(io.BytesIO(r.content))
    zip.extractall(dest)
    zip.close()


def get_replay_from_snapshot_path(path):
    return path.name[:path.name.index('.zplay') + len('.zplay')]


def collect_replay_data_from_directory(directory):
    replay_data = []
    all_snapshots = sorted(directory.rglob('*.png'))
    for replay, snapshots in groupby(all_snapshots, get_replay_from_snapshot_path):
        replay_data.append({
            'replay': replay,
            'snapshots': [{
                'url': str(s.relative_to(tracks_dir)),
                'frame': int(re.match(r'.*\.zplay\.(\d+)', s.name).group(1)),
                'unexpected': 'unexpected' in s.name,
            } for s in snapshots],
            'source': str(directory.relative_to(tracks_dir)),
        })
        replay_data[-1]['snapshots'].sort(key=lambda s: s['frame'])

    return replay_data


def collect_replay_data_from_workflow_run(run_id):
    replay_data = []
    workflow_dir = Path(f'{tracks_dir}/gha-{run_id}')
    run = repo.get_workflow_run(run_id)

    for artifact in run.get_artifacts():
        if artifact.name.startswith('replays-'):
            # strip 'x-of-y'
            # name_without_part = re.match(
            #     r'(.*)-\d+-of-\d+', artifact.name).group(1)
            # dest = workflow_dir / name_without_part
            dest = workflow_dir / artifact.name
            dest.mkdir(parents=True, exist_ok=True)
            if next(dest.glob('*'), None) is None:
                download_artifact(artifact, dest)
            replay_data.extend(collect_replay_data_from_directory(dest))

    return replay_data


# TODO: push args.* to same array in argparse so that order is preserved.
# first should always be baseline. For now, assume it is the workflow option.

all_replay_data = []
if args.workflow_run:
    for run_id in args.workflow_run:
        all_replay_data.extend(collect_replay_data_from_workflow_run(run_id))
if args.local:
    local_index = 0
    for directory in args.local:
        dest = Path(f'{tracks_dir}/local-{local_index}')
        if dest.exists():
            shutil.rmtree(dest)
        dest.mkdir(parents=True)
        for file in directory.glob('*.png'):
            shutil.copy(file, dest)
        all_replay_data.extend(collect_replay_data_from_directory(dest))
        local_index += 1


html = Path(f'{script_dir}/compare-resources/compare.html').read_text('utf-8')
css = Path(f'{script_dir}/compare-resources/compare.css').read_text('utf-8')
js = Path(f'{script_dir}/compare-resources/compare.js').read_text('utf-8')
deps = Path(f'{script_dir}/compare-resources/pixelmatch.js').read_text('utf-8')

result = html.replace(
    '// JAVASCRIPT', f'const data = {json.dumps(all_replay_data, indent=2)}\n  {js}')
result = result.replace('// DEPS', deps)
result = result.replace('/* CSS */', css)
out_path = Path(f'{tracks_dir}/index.html')
out_path.write_text(result)
print(f'report written to {out_path}')
