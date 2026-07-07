import argparse
import os
import re
import subprocess

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

parser = argparse.ArgumentParser()
parser.add_argument('--version', required=True)

args = parser.parse_args()


def fail(reason: str):
    print(f'blocking release for reason: {reason}')
    exit(1)


if not re.match(r'2\.55\.\d+', args.version):
    fail('expected version to match 2.55.x')

zdefs_content = (root_dir / 'src/base/zdefs.h').read_text()
if '#define V_COMPATRULE       65 ' not in zdefs_content:
    fail('unexpected change to V_COMPATRULE')

versionsig_content = (root_dir / 'src/metadata/versionsig.h').read_text()
patch = args.version.split('.')[-1]
if f'#define V_ZC_THIRD {patch} ' not in versionsig_content:
    fail(f'expected V_ZC_THIRD to equal {patch}')

most_recent_commit_message = subprocess.check_output(
    'git log -1 --pretty=format:%s'.split(' '), encoding='utf-8'
).strip()

expected_commit_message = f'misc!: bump to {args.version}'
if most_recent_commit_message != expected_commit_message:
    fail(f'expected most recent commit message to be: {expected_commit_message}')

most_recent_files_changed = (
    subprocess.check_output(
        'git log -1 --name-only --pretty=format:'.split(' '), encoding='utf-8'
    )
    .strip()
    .split('\n')
)
most_recent_files_changed.sort()

expected_files_changed = ['src/metadata/versionsig.h']
did_bump = 'src/metadata/versionsig.h' in most_recent_files_changed
did_changelog = next(
    (
        f
        for f in most_recent_files_changed
        if 'changelogs/' in f and f.endswith(f'{args.version}.txt')
    ),
    None,
)
if not did_bump or not did_changelog or len(most_recent_files_changed) != 2:
    fail(
        f'expected most recent commit message to change src/metadata/versionsig.h and add new changelog file'
    )

print('ok')
