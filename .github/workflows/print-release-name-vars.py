import argparse
import subprocess
import time
import os
import re


def set_action_output(output_name, value):
    print(f'{output_name}={value}')
    if 'GITHUB_OUTPUT' in os.environ:
        with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
            print('{0}={1}'.format(output_name, value), file=f)


parser = argparse.ArgumentParser()
parser.add_argument('--github-org', required=True)
parser.add_argument('--version-type', required=True, choices=['stable', 'nightly'])
args = parser.parse_args()

previous_release_version = subprocess.check_output(
    'git describe --tags --abbrev=0 --match "*.*.*"', shell=True, encoding='utf-8')
major, minor, patch = map(int, re.search(r'^(\d+)\.(\d+)\.(\d+)', previous_release_version).groups())

version_meta = []

if args.version_type == 'stable':
    minor += 1
    patch = 0
    release_version = f'{major}.{minor}.{patch}'
    release_name = f'{major}.{minor}'
if args.version_type == 'nightly':
    patch += 1
    today = time.strftime("%Y-%m-%d")
    release_version = f'{major}.{minor}.{patch}-nightly'
    version_meta.append(today)
    release_name = f'{major}.{minor}.{patch} Nightly {today}'

if args.github_org != 'ZQuestClassic':
    version_meta.append(args.github_org)
if version_meta:
    release_version += '+' + '.'.join(version_meta)

set_action_output('release-version', release_version)
set_action_output('release-name', release_name)
set_action_output('previous-release-version', previous_release_version)
