import argparse
import subprocess
import time


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0', ''):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


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
    if args.github_org != 'ArmageddonGames':
        return f'{args.github_org}-{tag_name}'
    else:
        return tag_name


if args.full_release:
    release_tag = maybe_add_org_prefix(
        f'2.55-{args.version_type}-{args.number}')
    release_name = f'2.55 {args.version_type.capitalize()} {args.number}'
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

print(f'::set-output name=release-tag::{release_tag}')
print(f'::set-output name=release-name::{release_name}')
