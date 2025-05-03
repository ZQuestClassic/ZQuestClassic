# Run as a script to download the database.

import argparse
import json
import os

from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path

import boto3

from botocore import UNSIGNED
from botocore.client import Config

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
default_db_path = root_dir / '.tmp/database'


@dataclass
class Release:
    id: str
    name: str
    resources: list[str]
    resource_hashes: list[str]


@dataclass
class Quest:
    data: dict
    id: str
    name: str
    releases: list[Release]
    default_path: Path


def _should_ignore_key(key: str) -> bool:
    if key in ['cache.json']:
        return True

    # qst.gz files are compressed for the web build. Never needed in a local copy of the database.
    if key.endswith('.qst.gz'):
        return True

    return False


class Database:
    path: Path
    data: dict
    quests: list[Quest]

    def __init__(self, path: Path = default_db_path):
        print(f'[database] init {path}')
        self.path = path
        s3 = boto3.resource(
            's3',
            region_name='nyc3',
            endpoint_url='https://nyc3.digitaloceanspaces.com',
            config=Config(signature_version=UNSIGNED),
        )
        self.bucket = s3.Bucket('zc-data')
        try:
            print('[database] downloading bucket object summaries')
            self._object_summaries = [
                x for x in self.bucket.objects.all() if not _should_ignore_key(x.key)
            ]
        except e as Exception:
            self._object_summaries = None
            print(e)
            print('Could not access bucket, operating in offline mode')

        self._load_manifest()

    def _load_manifest(self):
        self.data = json.loads(self.download('manifest.json').read_text())

        self.quests = []
        for entry in self.data.values():
            if entry['type'] != 'quests':
                continue

            # This quest is from 1.92b182, which usually we can load by not this one. There's unexpected bytes
            # prior to the "Zelda Classic Quest File" string.
            if entry['id'] == 'quests/purezc/73':
                continue

            releases = []
            for release_data in entry['releases']:
                name = release_data['name']
                id = f'{entry["id"]}/{name}'
                resources = release_data['resources']
                resource_hashes = release_data['resourceHashes']

                # This currently contains an .exe and is banned on pzc.
                if id == 'quests/purezc/822/r01':
                    continue

                # TODO: figure out how manifest.json got bad encoded string
                if id == 'quests/purezc/240/r01':
                    resources[1] = 'SWIIKézikönyv.txt'

                releases.append(
                    Release(
                        id=id,
                        name=name,
                        resources=resources,
                        resource_hashes=resource_hashes,
                    )
                )

            if not releases:
                continue

            id = entry['id']
            name = entry['name']
            default_path = self.path / entry['defaultPath']
            self.quests.append(
                Quest(
                    data=entry,
                    id=id,
                    name=name,
                    default_path=default_path,
                    releases=releases,
                )
            )

    def download(self, key: str) -> Path:
        """
        Download the file for the given key, and return the local path.

        Does nothing if already downloaded and is the latest version.
        """

        path = self.path / key

        if not self._object_summaries:
            if not path.exists():
                raise Exception(f'cannot download {key} in offline mode')
            return path

        if path.exists():
            object_summary = next(x for x in self._object_summaries if x.key == key)
            local_mtime = os.path.getmtime(path)
            local_datetime = datetime.fromtimestamp(local_mtime, tz=timezone.utc)
            if object_summary.last_modified > local_datetime:
                print(f'[database] updating {key}')
                self.bucket.download_file(key, path)
        else:
            print(f'[database] downloading {key}')
            path.parent.mkdir(exist_ok=True, parents=True)
            self.bucket.download_file(key, path)

        return path

    def download_all(self, prefix=''):
        for object_summary in self._object_summaries:
            if object_summary.key.startswith(prefix):
                self.download(object_summary.key)

    def download_latest_release(self, quest: Quest):
        release = quest.releases[0]
        self.download_release(release)

    def download_release(self, release: Release):
        for resource in release.resources:
            # I'm pretty sure no quest needs these files to run.
            # They are big/there's many of them, so skip if just needing
            # to download a release for runtime execution.
            ext = resource.split('.')[-1]
            if ext in ['jpg', 'bmp', 'png', 'gif', 'zip']:
                continue
            if ext in ['gz']:
                continue

            key = f'{release.id}/{resource}'
            self.download(key)

    def get_qst_keys_by_hash(self) -> dict[str, (str, Release)]:
        qst_key_by_hash = {}

        for quest in self.quests:
            for release in quest.releases:
                for index, resource_hash in enumerate(release.resource_hashes):
                    resource = release.resources[index]
                    if resource.endswith('.qst'):
                        key = f'{quest.id}/{release.name}/{resource}'
                        qst_key_by_hash[resource_hash] = (key, release)

        return qst_key_by_hash


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '--path',
        type=Path,
        default=default_db_path,
        help='Directory to download database to',
    )
    parser.add_argument(
        '--prefix',
        type=str,
        help='Download only objects matching this prefix',
    )
    args = parser.parse_args()

    db = Database(args.path)
    db.download_all(args.prefix or '')
