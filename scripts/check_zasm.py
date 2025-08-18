# Counts the usage of every ZASM command and register in the quest database and tests folder.
#
# Results: https://gist.github.com/connorjclark/f3c4ce1094d431d6f22a7e7670080ede
#
# First, you should run these commands to populate `build/Release/zscript-debug/zasm`:
#
#   python scripts/run_for_every_qst.py --starting_index 235 ./build/Release/zplayer -extract-zasm %s
#   find tests/replays -name '*.qst' -exec ./build/Release/zplayer -extract-zasm $PWD/{} \;

import argparse
import json
import os
import re
import subprocess

from pathlib import Path

import run_target

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp'

command_counts = {}
register_counts = {}

for line in run_target.check_run(
    'zscript', ['-print-zasm-commands']
).stdout.splitlines():
    id, name = line.split(' ')
    command_counts[name] = 0
for line in run_target.check_run(
    'zscript', ['-print-zasm-registers']
).stdout.splitlines():
    id, name = line.split(' ')
    register_counts[name] = 0

zasm_dir = root_dir / 'build/Release/zscript-debug/zasm'
for file in zasm_dir.rglob('*.txt'):
    content = file.read_text()
    for line in content.splitlines():
        m = re.match(r'\s*\d+: ([^ ]+) ([^[]+)', line)
        if not m:
            continue

        command, args = m.groups()

        if command not in command_counts:
            print('?', command)
            continue
        command_counts[command] += 1

        args = args.strip()
        if args:
            for i, arg in enumerate(re.split(r'\s+', args)):
                if '<' in arg or '>' in arg or '=' in arg:
                    continue
                if arg[0].isdigit():
                    continue
                if arg[0] in '-':
                    continue
                if arg[0] in '{"':
                    break
                if arg not in register_counts:
                    print('?', arg)
                    continue

                register_counts[arg] += 1

                if args[0] == 'WRITEPODSTRING' and i == 1:
                    break

print('--- commands ---')
command_counts = sorted(command_counts.items(), key=lambda x: x[1], reverse=True)
for command, i in command_counts:
    print(command, i)

print('\n--- registers ---')
register_counts = sorted(register_counts.items(), key=lambda x: x[1], reverse=True)
for register, i in register_counts:
    print(register, i)
