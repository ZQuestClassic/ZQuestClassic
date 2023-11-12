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
parser.add_argument('--version-override')
args = parser.parse_args()


def parse_version(version):
    major, minor, patch = map(int, re.search(r'^(\d+)\.(\d+)\.(\d+)', version).groups())
    return (major, minor, patch)


def make_version_name(version, date):
    major, minor, patch = parse_version(version)
    if patch == 0:
        return f'{major}.{minor}'
    return f'{major}.{minor}.{patch} Nightly {date}'

# TODO: remove this first block when ready for 3.0
if True:
    if args.version_type == 'stable' or args.version_override:
        raise Exception('TODO: not yet')
    
    date = time.strftime('%Y-%m-%d')
    n = 1
    try:
        previous_release_version = subprocess.check_output(
            'git describe --tags --abbrev=0 --match "3.0.0-prerelease.*"', shell=True, encoding='utf-8')
        n, = map(int, re.search(r'prerelease\.(\d+)', previous_release_version).groups())
        n += 1
    except:
        previous_release_version = subprocess.check_output(
            f'git describe --tags --abbrev=0 --match "*.*.*" --match "2.55-alpha-1??"', shell=True, encoding='utf-8')
    major, minor, patch = (3, 0, 0)
    version_meta = [date]
    release_version = f'{major}.{minor}.{patch}-prerelease.{n}'
    release_version += '+' + '.'.join(version_meta)
    release_name = f'3.0 Prerelease {n} {date}'

    set_action_output('release-version', release_version)
    set_action_output('release-name', release_name)
    set_action_output('previous-release-version', previous_release_version)
    exit(0)

if args.version_override:
    release_version = args.version_override
    try:
        date = subprocess.check_output(
            f'git log -1 --format=%cs {release_version}', shell=True, encoding='utf-8').strip()
    except:
        date = time.strftime('%Y-%m-%d')
    major, minor, patch = parse_version(release_version)
    if (args.version_type == 'stable' and patch != 0) or args.version_type == 'nightly' and patch == 0:
        raise Exception('incorrect version type')

    previous_release_version = subprocess.check_output(
        f'git describe --tags --abbrev=0 --match "*.*.*" --match "2.55-alpha-1??" --exclude {release_version}', shell=True, encoding='utf-8')
else:
    date = time.strftime('%Y-%m-%d')
    previous_release_version = subprocess.check_output(
        'git describe --tags --abbrev=0 --match "*.*.*"', shell=True, encoding='utf-8')
    major, minor, patch = parse_version(previous_release_version)
    version_meta = []

    if args.version_type == 'stable':
        minor += 1
        patch = 0
        release_version = f'{major}.{minor}.{patch}'
    if args.version_type == 'nightly':
        patch += 1
        release_version = f'{major}.{minor}.{patch}-nightly'
        version_meta.append(date)

    if args.github_org != 'ZQuestClassic':
        version_meta.append(args.github_org)
    if version_meta:
        release_version += '+' + '.'.join(version_meta)

set_action_output('release-version', release_version)
set_action_output('release-name', make_version_name(release_version, date))
set_action_output('previous-release-version', previous_release_version)
