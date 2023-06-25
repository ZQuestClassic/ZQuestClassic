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
    else:
        print(f'[DEBUG] {output_name}={value}')


parser = argparse.ArgumentParser()
parser.add_argument('--github_org', required=True)
parser.add_argument('--branch', required=True)
parser.add_argument('--full_release', required=True, type=str2bool)
parser.add_argument('--version_type', required=True)
parser.add_argument('--number')
args = parser.parse_args()

if args.number:
    args.number = int(args.number)

release_tag = ''
release_name = ''


def maybe_add_rls_prefix(tag_name):
    parts = []
    if args.github_org != 'ArmageddonGames':
        parts.append(args.github_org)
    if args.branch != 'main':
        parts.append(args.branch)
    parts.append(tag_name)
    return '-'.join(parts)


if args.full_release:
    release_tag = maybe_add_rls_prefix(
        f'2.55-{args.version_type}-{args.number}')
    release_name = f'2.55 {args.version_type.capitalize()} {args.number}'
else:
    today = time.strftime("%Y-%m-%d")

    i = 1
    while True:
        release_tag = maybe_add_rls_prefix(
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

previous_release_tag = None
try:
    prefix = maybe_add_rls_prefix('')
    tags = subprocess.check_output(
        'git tag --sort committerdate', shell=True, encoding='utf-8').splitlines()
    tags = [tag for tag in tags if tag.startswith(prefix)]
    previous_release_tag = tags[-1]
except Exception as err:
    print(err)
    print('failed to find previous tag, trying backup')

# Just in case the complex thing above fails, fall back to this.
# This works as expected in majority of cases, but fails for some merge / non-linear histories in branches.
# Good luck understanding this: https://stackoverflow.com/a/57697016/2788187
if not previous_release_tag:
    previous_release_tag = subprocess.check_output(
        'git describe --tags --abbrev=0', shell=True, encoding='utf-8')

set_action_output('release-tag', release_tag)
set_action_output('release-name', release_name)
set_action_output('previous-release-tag', previous_release_tag)
