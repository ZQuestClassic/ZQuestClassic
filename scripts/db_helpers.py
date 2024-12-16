import json
import os

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
default_manifest_path = root_dir / '.tmp/database/manifest.json'


class Database:
    path: Path
    data: dict

    def __init__(self, path: Path = default_manifest_path):
        # TODO: should download if does not exist.
        self.path = path.parent
        self.data = json.loads(path.read_text())

    def get_quests(self) -> list[dict]:
        quests = []

        for entry in self.data.values():
            if not entry['id'].startswith('quests/'):
                continue

            # This quest is from 1.92b182, which usually we can load by not this one. There's unexpected bytes
            # prior to the "Zelda Classic Quest File" string.
            if entry['id'] == 'quests/purezc/73':
                continue

            qst_path = self.path / entry['defaultPath']
            if not qst_path.exists():
                print(f'does not exist: {qst_path}')
                print('make sure you\'ve cloned the database:')
                print('  cd scripts/database')
                print('  OFFICIAL_SYNC=1 npm run collect')
                # TODO: would be better if we just downloaded things as we go.
                exit(1)

            quests.append(entry)

        return quests

    def get_quest_path(self, quest: dict) -> Path:
        return self.path / quest['defaultPath']

    def get_qst_paths_by_hash(self) -> dict[str, Path]:
        qst_path_by_hash = {}

        for quest in self.get_quests():
            for release in quest['releases']:
                for index, resource_hash in enumerate(release['resourceHashes']):
                    resource = release['resources'][index]
                    if resource.endswith('.qst'):
                        qst_path_by_hash[resource_hash] = (
                            self.path / quest['id'] / release['name'] / resource
                        )

        return qst_path_by_hash
