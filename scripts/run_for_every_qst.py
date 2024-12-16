# Runs a command for every quest defined in the database mainfest
#
# Example:
#
#   python scripts/run_for_every_qst.py ./build/Debug/zplayer -extract-zasm %s

import argparse
import os
import subprocess

from pathlib import Path
from typing import List

import db_helpers

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

db = db_helpers.Database(args.quest_database)
quests = db.get_quests()
quest_paths = [db.get_quest_path(quest) for quest in quests]

for index, path in enumerate(quest_paths):
    if index < args.starting_index:
        continue

    print(f'= {index} {path}', flush=True)
    p = subprocess.call([arg.replace('%s', str(path)) for arg in args.command])
    if p != 0:
        print(f'Error: {p}')
