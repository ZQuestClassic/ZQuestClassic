# Runs a command for every quest in the database
#
# Examples:
#
#   python scripts/run_for_every_qst.py ./build/Release/zplayer -extract-zasm %s
#   python scripts/run_for_every_qst.py ./build/Release/zplayer -load-and-quit %s
#   python scripts/run_for_every_qst.py echo %s

import argparse
import os
import subprocess

from pathlib import Path
from typing import List

import database

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp'

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--starting_index', type=int, default=0)
parser.add_argument('--quest_database', type=Path, default=tmp_dir / 'database')
parser.add_argument('command', nargs=argparse.REMAINDER)
args = parser.parse_args()

db = database.Database(args.quest_database)

count = 0
for index, quest in enumerate(db.quests):
    if index < args.starting_index:
        continue
    count += 1
print(f'Running against {count} qst files')

errors = []
for index, quest in enumerate(db.quests):
    if index < args.starting_index:
        continue

    db.download_latest_release(quest)

    path = quest.default_path
    print(f'= {index} {path}', flush=True)
    retcode = subprocess.call([arg.replace('%s', str(path)) for arg in args.command])
    if retcode != 0:
        print(f'Error: {retcode}')
        errors.append(f'{path} ({retcode})')

if errors:
    print(f'Running the command on the following quests had errors:')
    for error in errors:
        print(f'  {error}')
    exit(1)
