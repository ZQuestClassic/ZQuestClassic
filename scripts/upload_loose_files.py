# Upload loose quest files to the database.

import argparse
import hashlib
import json
import os

from datetime import datetime
from pathlib import Path

import run_target

from database import Database, Quest, Release


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

    print('Warning: could not find title for qst: {path}. Using filename')
    return path.stem


def upload_loose_file(bucket, path: Path, key: str):
    if not key.startswith('quests/loose/'):
        raise Exception(f'unexpected key: {key}')

    print(f'uploading: {key}')
    bucket.upload_file(path, key, ExtraArgs={'ACL': 'public-read'})


def handle_upload_loose_quests(quests_path: Path):
    if os.environ.get('ZC_DATABASE_SKIP_MANIFEST_UPDATE'):
        raise Exception('cannot skip manifest update')

    db = Database()
    bucket = db.get_writable_bucket()
    hashes = list(db.get_qst_keys_by_hash().keys())
    now_date_string = datetime.now().strftime("%d %b %Y")
    num_added = 0

    for path in quests_path.glob('*'):
        if path.name.startswith('.'):
            continue

        if path.is_file():
            assert path.suffix == '.qst'
            quest_path = path
            dir_path = path.parent
        else:
            qsts = list(path.rglob('*.qst'))
            assert len(qsts) == 1
            quest_path = qsts[0]
            dir_path = path

        md5_hash = calculate_md5(quest_path)
        if md5_hash in hashes:
            print(f'skipping known quest: {quest_path.name}')
            continue

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
            default_path=f'{db_id}/r01/{resources[0]}',
            releases=releases,
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


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '--quests',
        type=dir_path,
        required=True,
        help='For all qst files in the given folder, upload any that are not currently in the database. Tracks them as "loose" / private files, only to be used for purposes of replay uploads',
    )
    args = parser.parse_args()

    handle_upload_loose_quests(args.quests)
