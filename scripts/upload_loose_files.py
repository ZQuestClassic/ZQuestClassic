# Upload loose quest files to the database.

import argparse
import hashlib
import json
import os

from datetime import datetime
from pathlib import Path

import requests
import run_target

from database import Database, Quest, Release

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

os.sys.path.append(str((root_dir / 'tests/lib').absolute()))
import replay_helpers


def dir_path(path):
    if not os.path.isfile(path) and (os.path.isdir(path) or not os.path.exists(path)):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')


def calculate_md5(path: Path) -> str:
    md5_hash = hashlib.md5()
    with open(path, 'rb') as file:
        for chunk in iter(lambda: file.read(4096), b''):
            md5_hash.update(chunk)
    return md5_hash.hexdigest().upper()


def get_qst_title(path: Path) -> str:
    p = run_target.run('zplayer', ['-load-and-quit', path])
    if p.returncode:
        raise Exception(p.stderr)

    lines = p.stdout.splitlines()
    for line in lines:
        if line.startswith('Title:'):
            return line.replace('Title:', '').strip()

    print(f'Warning: could not find title for qst: {path}. Using filename')
    return path.stem


def upload_loose_file(bucket, path: Path, key: str):
    if not key.startswith('quests/loose/'):
        raise Exception(f'unexpected key: {key}')

    print(f'uploading: {key}')
    bucket.upload_file(path, key, ExtraArgs={'ACL': 'public-read'})


def put_file_contents(url: str, path: Path) -> requests.Response:
    """
    Sends a PUT request to the specified URL with the contents of a file as the body.

    Args:
        url: The destination URL.
        path: The local path to the file you want to upload.

    Returns:
        The Response object.
    """
    try:
        with open(path, 'rb') as file_data:
            response = requests.put(url, data=file_data)
        response.raise_for_status()
        print(f"Success: Uploaded to {url} with status code {response.status_code}")
        return response

    except requests.exceptions.HTTPError as e:
        print(f"HTTP Error: {e}")
        print(f"Server response: {e.response.text}")
        raise
    except requests.exceptions.RequestException as e:
        print(f"HTTP Request failed: {e}")
        raise


def handle_upload_loose_quests_and_replays(loose_files_path: Path):
    if os.environ.get('ZC_DATABASE_SKIP_MANIFEST_UPDATE'):
        raise Exception('cannot skip manifest update')

    db = Database()
    bucket = db.get_writable_bucket()
    hashes = list(db.get_qst_keys_by_hash().keys())
    now_date_string = datetime.now().strftime("%d %b %Y")
    num_added = 0

    for path in loose_files_path.glob('*'):
        if path.name.startswith('.'):
            continue

        if path.is_file():
            if path.suffix.lower() != '.qst':
                continue

            quest_path = path
            dir_path = path.parent
        else:
            qsts = list(path.rglob('*.qst'))
            if len(qsts) != 1:
                continue

            quest_path = qsts[0]
            dir_path = path

        md5_hash = calculate_md5(quest_path)
        if md5_hash in hashes:
            print(f'skipping known quest: {quest_path.name}')
            continue

        hashes.append(md5_hash)

        music = []
        music_paths = []

        if path.is_file():
            resources = [quest_path.name]
            resource_hashes = [md5_hash]
        else:
            resources = [str(quest_path.relative_to(path).as_posix())]
            resource_hashes = [md5_hash]

            for resource_path in path.rglob('*'):
                if resource_path.name.startswith('.'):
                    continue

                if resource_path.suffix.lower() == '.zplay':
                    continue

                if not resource_path.is_file():
                    continue

                if resource_path.name == quest_path.name:
                    continue

                if resource_path.suffix.lower() in [
                    '.gbs',
                    '.gym',
                    '.it',
                    '.mod',
                    '.mp3',
                    '.nsf',
                    '.ogg',
                    '.s3m',
                    '.spc',
                    '.vgm',
                    '.xm',
                ]:
                    music.append(resource_path.name)
                    music_paths.append(resource_path)
                    continue

                resources.append(str(resource_path.relative_to(path).as_posix()))
                resource_hashes.append(calculate_md5(resource_path))

        if len(resources) != len(set(resources)):
            print(resources)
            raise Exception('resources are not all unique')

        db_id = f'quests/loose/{md5_hash}'
        releases = [
            Release(
                data={},
                id=f'{db_id}/r01',
                name='r01',
                resources=resources,
                resource_hashes=resource_hashes,
            ),
        ]
        quest = Quest(
            data={
                'approval': False,
                'dateAdded': now_date_string,
                'music': music,
            },
            id=db_id,
            name=get_qst_title(quest_path),
            releases=releases,
            default_path=f'{db_id}/r01/{resources[0]}',
            rating_score=None,
        )

        quest_json = {
            'type': 'quests',
            'id': quest.id,
            'name': quest.name,
            'defaultPath': quest.default_path,
            'releases': [
                {
                    'name': r.name,
                    'resources': r.resources,
                    'resourceHashes': r.resource_hashes,
                }
                for r in quest.releases
            ],
            **quest.data,
        }

        print(json.dumps(quest_json, indent=2))
        db.data[quest.id] = quest_json
        num_added += 1

        for release in quest.releases:
            for resource in release.resources:
                path = f'{dir_path}/{resource}'
                key = f'{release.id}/{resource}'
                upload_loose_file(bucket, path, key)

        for music_path in music_paths:
            path = music_path
            key = f'{quest.id}/music/{music_path.name}'
            upload_loose_file(bucket, path, key)

    if num_added:
        manifest_path = db.path / 'manifest.json'
        manifest_path.write_text(json.dumps(db.data, indent=2))
        bucket.upload_file(
            manifest_path,
            'manifest.json',
            ExtraArgs={'ACL': 'public-read', 'ContentType': 'application/json'},
        )

    for path in loose_files_path.rglob('*.zplay'):
        meta = replay_helpers.read_replay_meta(path)
        uuid = meta['uuid']
        qst_hash = meta['qst_hash']
        if qst_hash not in hashes:
            print(f'skipping {path}, unknown qst')
            continue

        print(f'uploading {path} ...')
        put_file_contents(f'https://api.zquestclassic.com/api/v1/replays/{uuid}', path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '--dir',
        type=dir_path,
        required=True,
        help='For all qst and zplay files in the given folder, upload any that are not currently in the database. Tracks the quests as "loose" / private files, only to be used for purposes of replay uploads',
    )
    args = parser.parse_args()

    handle_upload_loose_quests_and_replays(args.dir)
