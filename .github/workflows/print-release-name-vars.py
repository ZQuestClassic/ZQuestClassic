import argparse
import subprocess
import time
import os


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0', ''):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


def set_action_output(output_name, value):
    if 'GITHUB_OUTPUT' in os.environ:
        with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
            print('{0}={1}'.format(output_name, value), file=f)


parser = argparse.ArgumentParser()
parser.add_argument('--github_org', required=True)
parser.add_argument('--full_release', required=True, type=str2bool)
parser.add_argument('--version_type', required=True)
parser.add_argument('--number')
args = parser.parse_args()

if args.number:
    args.number = int(args.number)

release_tag = ''
release_name = ''


def maybe_add_org_prefix(tag_name):
    if args.github_org != 'ZQuestClassic':
        return f'{args.github_org}-{tag_name}'
    else:
        return tag_name


if args.full_release:
    release_tag = maybe_add_org_prefix(
        f'2.55.{args.number}')
    release_name = f'2.55.{args.number}'
else:
    today = time.strftime("%Y-%m-%d")

    i = 1
    while True:
        release_tag = maybe_add_org_prefix(
            f'nightly-{today}')
        release_name = f'Nightly {today}'
        if i != 1:
            release_tag += f'-{i}'
            release_name += f' ({i})'
        proc = subprocess.run(['git', 'rev-parse', release_tag])
        if proc.returncode == 0:
            i += 1
        else:
            break

if args.full_release:
    previous_release_tag = subprocess.check_output(
        'git describe --tags --abbrev=0 --match "2.55-*"', shell=True, encoding='utf-8')
else:
    previous_release_tag = subprocess.check_output(
        'git describe --tags --abbrev=0', shell=True, encoding='utf-8')

set_action_output('release-tag', release_tag)
set_action_output('release-name', release_name)
set_action_output('previous-release-tag', previous_release_tag)
