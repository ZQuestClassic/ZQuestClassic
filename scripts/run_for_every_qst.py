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
# 235 is the first qst with scripts
parser.add_argument('--starting_index', type=int, default=0)
parser.add_argument(
    '--quest_database', type=Path, default=tmp_dir / 'database/manifest.json'
)
parser.add_argument('--build_folder', default='build/Release')
parser.add_argument('command', nargs=argparse.REMAINDER)
args = parser.parse_args()

build_folder = Path(args.build_folder).absolute()

db = json.loads(args.quest_database.read_text())
quests: List[Path] = []
for entry in db.values():
    if not entry['id'].startswith('quests/'):
        continue

    quests.append(entry)

for index, quest in enumerate(quests):
    if index < args.starting_index:
        continue

    path = args.quest_database.parent / quest['defaultPath']
    print(f'= {index} {path}', flush=True)
    p = subprocess.call([arg.replace('%s', str(path)) for arg in args.command])
    if p != 0:
        print(f'Error: {p}')
