# Runs a command for every quest defined in the database mainfest
#
# Example:
#
#   python scripts/run_for_every_qst.py ./build/Debug/zplayer -extract-zasm %s

import argparse
import json
import os
import subprocess

from pathlib import Path
from typing import List

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp'

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--starting_index', type=int, default=0)
parser.add_argument(
    '--quest_database', type=Path, default=tmp_dir / 'database/manifest.json'
)
parser.add_argument('command', nargs=argparse.REMAINDER)
args = parser.parse_args()

db = json.loads(args.quest_database.read_text())
quests: List[Path] = []
for entry in db.values():
    if not entry['id'].startswith('quests/'):
        continue
    # This quest is from 1.92b182, which usually we can load by not this one. There's unexpected bytes
    # prior to the "Zelda Classic Quest File" string.
    if entry['id'] == 'quests/purezc/73':
        continue

    qst_path = args.quest_database.parent / entry['defaultPath']
    if not qst_path.exists():
        print(f'does not exist: {qst_path}')
        print('make sure you\'ve cloned the database:')
        print('  cd scripts/database')
        print('  OFFICIAL_SYNC=1 npm run collect')
        # TODO: would be better if we just downloaded things as we go.
        exit(1)
    quests.append(qst_path)

for index, path in enumerate(quests):
    if index < args.starting_index:
        continue

    print(f'= {index} {path}', flush=True)
    p = subprocess.call([arg.replace('%s', str(path)) for arg in args.command])
    if p != 0:
        print(f'Error: {p}')
