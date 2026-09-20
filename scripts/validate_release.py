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



def git(*cmd: str) -> str:
    return subprocess.check_output(['git', *cmd], encoding='utf-8').strip()


def git_ok(*cmd: str) -> bool:
    return subprocess.run(['git', *cmd], capture_output=True).returncode == 0


def check_cherry_pick_trailers(version: str):
    """Every "(cherry picked from commit X)" trailer added since the previous
    release must name a commit that exists on main. A pick made from a local main
    that was later rebased cites an id nobody else can resolve (2.55.16 shipped
    three of those)."""
    main_ref = next((r for r in ['origin/main', 'main'] if git_ok('rev-parse', '--verify', '-q', r)), None)
    if not main_ref:
        fail('no main branch available to validate cherry-pick trailers against')

    major, minor, patch = (int(x) for x in version.split('.'))
    prev_tags = [
        t
        for t in git('tag', '--list', f'{major}.{minor}.*').split('\n')
        if re.fullmatch(rf'{major}\.{minor}\.\d+', t) and int(t.split('.')[-1]) < patch
    ]
    if not prev_tags:
        return
    prev_tag = max(prev_tags, key=lambda t: int(t.split('.')[-1]))

    bad = []
    log = git('log', '--format=%H%x00%s%x00%b%x01', f'{prev_tag}..HEAD')
    for entry in filter(None, log.split('\x01')):
        sha, subject, body = entry.strip('\n').split('\x00', 2)
        for cited in re.findall(r'\(cherry picked from commit ([0-9a-f]{40})\)', body):
            if not git_ok('merge-base', '--is-ancestor', cited, main_ref):
                bad.append(f'  {sha[:10]} {subject}\n    cites {cited[:10]}, which is not on {main_ref}')
    if bad:
        fail(
            f'cherry-pick trailers since {prev_tag} reference commits missing from {main_ref} '
            f'(main not pushed yet, or the pick cites a pre-rebase id? fix the trailer with '
            f'git rebase before releasing):\n' + '\n'.join(bad)
        )


check_cherry_pick_trailers(args.version)

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
