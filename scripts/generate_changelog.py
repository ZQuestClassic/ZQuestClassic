import argparse
import os
import re
import subprocess
import sys

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List

from git_hooks.common import valid_scopes, valid_types

release_oneliners = {
    '2.55.0': 'The one that is official.',
    '2.55-alpha-120': 'The one with crumbling floors, moving platforms, and ExDoors.',
    '2.55-alpha-119': 'The one with subscreen scripts and an autocombo drawing mode.',
    '2.55-alpha-118': 'The one with the bug fixes.',
    '2.55-alpha-117': 'The one with the subscreen rewrite, software updater, music mixing, and individual save files.',
    '2.55-alpha-116': 'The one with custom Guys, quest package exports, and a ZScript VS Code extension.',
    '2.55-alpha-114': 'The one with trigger groups, newer player movement, and bomb flowers.',
}


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0', ''):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

parser = argparse.ArgumentParser()
parser.add_argument('--format', choices=['plaintext', 'markdown'], default='plaintext')
parser.add_argument('--from')
parser.add_argument('--to', default='HEAD')
parser.add_argument('--for-nightly', type=str2bool, default=False)
parser.add_argument('--version')
parser.add_argument('--generate-all', action='store_true')
parser.add_argument('--generate-cherrypicks', action='store_true')

args = parser.parse_args()

commit_url_prefix = 'https://github.com/ZQuestClassic/ZQuestClassic/commit'

overrides = dict()
overrides_squashes = dict()

# "dropped" commits (ex: "fix!: bug is gone now") are excluded from stable changelogs. Commits are "dropped" from the stable
# changelog if they fix a regression that occured after the last stable (during a nightly release), and so are not relevant
# to mention. However, they are relevant for nightly releases, so let's show 'em in that case.
should_drop_commits = not args.for_nightly


def parse_override_file(file: Path):
    last_override = None
    current_squash_list = []

    for line in file.read_text().splitlines():
        if line.startswith('# !'):
            continue

        line = line.rstrip()

        if (
            last_override
            and last_override[0] in ['reword', 'section']
            and not re.match(r'^(reword|section|subject|squash|drop|pick)', line)
        ):
            if line == '=end':
                last_override = None
                continue

            last_override[1] += f'\n{line}'
            continue

        if not line:
            continue

        parts = line.split(' ', 2)
        if len(parts) == 3:
            type, hash, rest = parts
        else:
            type, hash, rest = [*parts, None]

        if len(hash) != 40:
            raise Exception(f'expected full hashes. got: {hash}')
        if hash in overrides:
            raise Exception(f'hash already present: {hash}')

        if type == 'squash':
            current_squash_list.append(hash)
        elif last_override and last_override[0] == 'squash':
            overrides_squashes[hash] = current_squash_list
            current_squash_list = []

        overrides[hash] = last_override = [type, rest]

    if last_override and last_override[0] == 'squash':
        raise Exception('squash into what?')


@dataclass
class Commit:
    type: str
    scope: str
    short_hash: str
    hash: str
    subject: str
    oneline: str
    body: str
    squashed_commits: List['Commit'] = field(default_factory=lambda: [])

    def scope_and_oneline(self):
        if self.scope:
            return f'{self.scope}: {self.oneline}'
        else:
            return self.oneline


def parse_scope_and_type(subject: str):
    match = re.match(r'(\w+)\((\w+)\)!: (.+)', subject)
    if match:
        type, scope, oneline = match.groups()
        return type, scope, oneline, True

    match = re.match(r'(\w+)!: (.+)', subject)
    if match:
        type, oneline = match.groups()
        return type, None, oneline, True

    match = re.match(r'(\w+)\((\w+)\)<?: (.+)', subject)
    if match:
        type, scope, oneline = match.groups()
        return type, scope, oneline, False

    match = re.match(r'(\w+)<?: (.+)', subject)
    if match:
        type, oneline = match.groups()
        return type, None, oneline, False

    return 'misc', None, subject, False


def get_type_index(type: str):
    if type in valid_types:
        return valid_types.index(type)
    else:
        return -1


def get_scope_index(scope: str):
    if scope in valid_scopes:
        return valid_scopes.index(scope)
    else:
        return -1


def get_type_label(type: str):
    match type:
        case 'feat':
            return 'Features'
        case 'fix':
            return 'Bug Fixes'
        case 'docs':
            return 'Documentation'
        case 'chore':
            return 'Chores'
        case 'refactor':
            return 'Refactors'
        case 'test':
            return 'Tests'
        case 'ci':
            return 'CI'
        case 'misc':
            return 'Misc.'
        case _:
            return type.capitalize()


def get_scope_label(scope: str):
    match scope:
        case 'zc':
            return 'Player'
        case 'zq':
            return 'Editor'
        case 'std_zh':
            return 'ZScript Standard Library (std.zh)'
        case 'zscript':
            return 'ZScript'
        case 'vscode':
            return 'Visual Studio Code Extension'
        case 'launcher':
            return 'ZLauncher'
        case 'zconsole':
            return 'ZConsole'
        case 'zupdater':
            return 'ZUpdater'
        case _:
            return scope.capitalize()


def split_text_into_logical_markdown_chunks(text: str) -> List[str]:
    """
    Splits the input into separate chunks, such that each is a logical markdown chunk
    (ex: list, code block, paragraph). Allows for constructing a pretty markdown result from
    git commit messages, without inheriting random line breaks.

    Helps with generating pretty markdown on GitHub.
    You should test changes to this by previewing in a "releases" page markdown preview. Other
    places on GitHub (like gists) render slightly differently!
    """
    lines = []

    inside_paragraph_block = False
    current_paragraph_block = ''

    inside_code_block = False
    current_code_block = ''

    inside_list_element = False
    current_list_element = ''

    for line in text.splitlines():
        if inside_paragraph_block:
            current_paragraph_block += line + ' '
            if line == '':
                lines.append(current_paragraph_block)
                inside_paragraph_block = False
                current_paragraph_block = ''
            continue

        if inside_code_block:
            current_code_block += line + '\n'
            if line == '```':
                lines.append(current_code_block)
                inside_code_block = False
                current_code_block = ''
            continue

        if inside_list_element:
            current_list_element += line + '\n'
            if line == '':
                lines.append(current_list_element)
                lines.append('&nbsp;')
                inside_list_element = False
                current_list_element = ''
            continue

        if line.startswith('```'):
            current_code_block += line + '\n'
            inside_code_block = True
            continue

        line_stripped = line.strip()
        if (
            line_stripped.startswith('-')
            or line_stripped.startswith('*')
            or re.match(r'^\d+\.', line_stripped)
        ):
            current_list_element += line + '\n'
            inside_list_element = True
            continue

        if line:
            inside_paragraph_block = True
        current_paragraph_block += line + ' '

    if current_paragraph_block:
        lines.append(current_paragraph_block)
    if current_code_block:
        lines.append(current_code_block)
    if current_list_element:
        lines.append(current_list_element)

    return lines


def stringify_changelog(
    commits_by_type: Dict[str, List[Commit]], format: str, to_sha: str
) -> str:
    lines = []
    oneliner = release_oneliners.get(to_sha, None)
    is_release = to_sha.startswith('2.55') or to_sha.startswith('3')

    if format == 'markdown':
        if oneliner:
            lines.append(f'{oneliner}\n')
        if is_release:
            lines.append(
                f'[View release notes and find downloads on the website](https://zquestclassic.com/releases/{to_sha})\n'
            )

        for type, commits in commits_by_type.items():
            if type == 'CustomSection':
                lines.append('# Sectioned Changes')
                for c in commits:
                    lines.append(f'<details>\n<summary>{c.oneline}</summary>\n')
                    lines.append(f'{c.body}\n')
                    if c.squashed_commits:
                        lines.append('   >&nbsp;')
                        lines.append('   >Relevant changes:')
                        for squashed in c.squashed_commits:
                            # TODO: also show body?
                            link = f'[`{squashed.short_hash}`]({commit_url_prefix}/{squashed.hash})'
                            lines.append(f'   > - {squashed.subject} {link}')
                    lines.append('</details>')
                continue

            label = get_type_label(type)
            lines.append(f'# {label}\n')

            commits_by_scope: Dict[str, List[Commit]] = {}
            for commit in commits:
                by_scope = commits_by_scope.get(commit.scope, [])
                commits_by_scope[commit.scope] = by_scope
                by_scope.append(commit)

            for scope, commits in commits_by_scope.items():
                if scope:
                    label = get_scope_label(scope)
                    lines.append(f'### {label}\n')
                for c in commits:
                    if c.squashed_commits:
                        lines.append(f'- {c.oneline}')
                    else:
                        link = f'[`{c.short_hash}`]({commit_url_prefix}/{c.hash})'
                        lines.append(f'- {c.oneline} {link}')
                    if c.body:
                        lines.append('   &nbsp;')
                        for l in split_text_into_logical_markdown_chunks(c.body):
                            if l:
                                for l2 in l.splitlines():
                                    lines.append(f'   >{l2}')
                                lines.append(f'   >')

                    if c.squashed_commits:
                        lines.append('   >&nbsp;')
                        lines.append('   >Relevant changes:')
                        for squashed in c.squashed_commits:
                            # TODO: also show body?
                            link = f'[`{squashed.short_hash}`]({commit_url_prefix}/{squashed.hash})'
                            lines.append(f'   > - {squashed.subject} {link}')
                lines.append('')
    elif format == 'plaintext':
        if oneliner:
            lines.append(f'{oneliner}\n')
        if is_release:
            lines.append(f'https://zquestclassic.com/releases/{to_sha}\n')
            if to_sha == '2.55.0':
                lines.append(
                    'View a summary of what\'s new in 2.55: https://zquestclassic.com/docs/2.55/\n'
                )

        for type, commits in commits_by_type.items():
            if type == 'CustomSection':
                for c in commits:
                    lines.append(f'{c.body}\n')
                    if c.squashed_commits:
                        lines.append('\n  Relevant changes:')
                        for squashed in c.squashed_commits:
                            # TODO: also show body?
                            lines.append('    ' + squashed.subject)
                continue
            label = get_type_label(type)
            lines.append(f'# {label}\n')
            prev_had_body = False
            for c in commits:
                if c.body and not prev_had_body:
                    lines.append('')
                if 'SHOW_SUBJECT' in os.environ:
                    lines.append(c.subject)
                else:
                    lines.append(f'{c.scope_and_oneline()}')
                if 'SHOW_HASH' in os.environ:
                    lines[-1] = c.hash + ' ' + lines[-1]
                if c.body:
                    lines.append('  ' + c.body.replace('\n', '\n  ') + '\n')
                    prev_had_body = True

                if c.squashed_commits:
                    lines.append('\n  Relevant changes:')
                    for squashed in c.squashed_commits:
                        # TODO: also show body?
                        lines.append('    ' + squashed.subject)
            lines.append('')

    if lines[-1] != '':
        lines.append('')
    return '\n'.join(lines)


def generate_changelog(from_sha: str, to_sha: str) -> str:
    commits_text = subprocess.check_output(
        f'git log {from_sha}..{to_sha} --reverse --format="%h %H %s"',
        shell=True,
        encoding='utf-8',
    ).strip()

    commits: List[Commit] = []
    for commit_text in commits_text.splitlines():
        short_hash, hash, subject = commit_text.split(' ', 2)
        if hash in overrides and overrides[hash][0] == 'drop':
            continue

        # Remove GitHub PR number. The commit URL will link to this anyway.
        subject = re.sub(r' \(#\d+\)$', '', subject).strip()

        body = subprocess.check_output(
            f'git log -1 {hash} --format="%b"', shell=True, encoding='utf-8'
        ).strip()
        m = re.search(r'end changelog', body, re.IGNORECASE)
        if m:
            body = body[0 : m.start()].strip()
        body = re.sub(r'^Co-authored-by: .+', '', body, flags=re.MULTILINE).strip()
        body = re.sub(r'^\(cherry picked from commit .+\)$', '', body, flags=re.MULTILINE).strip()
        type, scope, oneline, drop = parse_scope_and_type(subject)
        if drop and should_drop_commits:
            continue
        commits.append(Commit(type, scope, short_hash, hash, subject, oneline, body))

    # Replace commit messages with overrides.
    manual_squashes = dict()
    manual_squash_hash = None
    for commit in commits:
        hash = commit.hash
        reparse = True
        if hash in overrides and overrides[hash][0] in ['subject', 'squash']:
            commit.subject = overrides[hash][1]
        elif hash in overrides and overrides[hash][0] == 'reword':
            lines = overrides[hash][1].splitlines()
            commit.subject = lines[0]
            commit.body = '\n'.join(lines[1:])
        elif hash in overrides and overrides[hash][0] == 'section':
            commit.type = 'CustomSection'
            commit.oneline = overrides[hash][1].splitlines()[0].strip()
            commit.body = overrides[hash][1].strip()
            if commit.body[0] != '#':
                commit.body = f'# {commit.body}'
            continue
        else:
            reparse = False
        if commit.hash in overrides_squashes:
            manual_squash_hash = None
        elif manual_squash_hash:
            match = re.match(r'(\w+(?:\(\w+\))?)<(: .+)', commit.subject)
            if match:
                commit.subject = ''.join(filter(None, match.groups()))
                if manual_squash_hash in manual_squashes:
                    manual_squashes[manual_squash_hash].append(commit.hash)
                else:
                    manual_squashes[manual_squash_hash] = [commit.hash]
            else:
                manual_squash_hash = commit.hash
        else:
            manual_squash_hash = commit.hash

        if reparse:
            type, scope, oneline, drop = parse_scope_and_type(commit.subject)
            commit.type = type
            commit.scope = scope
            commit.oneline = oneline

    # Squash commits.
    squashed_hashes = []
    for commit in commits:
        squash_list = None
        add_self = False
        if commit.hash in overrides_squashes:
            squash_list = overrides_squashes[commit.hash]
            add_self = True
        elif commit.hash in manual_squashes:
            squash_list = manual_squashes[commit.hash]
        else:
            continue

        squashed_hashes.extend(squash_list)
        commit.squashed_commits = [c for c in commits if c.hash in squash_list]
        if add_self:
            commit.squashed_commits.insert(0, Commit(**commit.__dict__))
        commit.squashed_commits.sort(
            key=lambda c: (get_type_index(c.type), get_scope_index(c.scope))
        )
    commits = [c for c in commits if c.hash not in squashed_hashes]

    commits_by_type: Dict[str, List[Commit]] = {}
    for type in valid_types:
        commits_by_type[type] = []
    commits_by_type['CustomSection'] = []

    for commit in commits:
        by_type = commits_by_type.get(commit.type, [])
        commits_by_type[commit.type] = by_type
        by_type.append(commit)

    to_remove = []
    for type, commits in commits_by_type.items():
        if not commits:
            to_remove.append(type)

        commits.sort(key=lambda c: get_scope_index(c.scope))

    for key in to_remove:
        del commits_by_type[key]

    changelog = stringify_changelog(commits_by_type, args.format, to_sha)
    # 125k is the maximum length for the api call to update the body. Give a 2k buffer.
    if len(changelog) > 123000:
        print(
            f'WARNING: changelog may be too big to publish to GitHub. Length: {len(changelog)}',
            file=sys.stderr,
        )
    return changelog


for path in (script_dir / 'changelog_overrides').rglob('*.md'):
    if path.name == 'cherrypicks-3.0.md' and args.for_nightly:
        continue
    if path.name != 'README.md':
        parse_override_file(path)

if args.generate_all:

    def get_tags(branch, tag_filter):
        return (
            subprocess.check_output(
                f'git tag --list --sort=committerdate --merged {branch} {tag_filter}',
                shell=True,
                encoding='utf-8',
            )
            .strip()
            .splitlines()
        )

    def process_tag(tag: str, prev_tag: str):
        print(f'{prev_tag} .. {tag}')
        changelog = generate_changelog(prev_tag, tag)
        date = subprocess.check_output(
            f'git show --no-patch --format=%ci {tag}', shell=True, encoding='utf-8'
        ).split(' ')[0]
        date = date.replace('-', '_')
        (root_dir / 'changelogs' / f'{date}-{tag}.txt').write_text(changelog)

    def process_tags(tags: list[str]):
        for i, tag in enumerate(tags):
            if i == 0:
                continue

            if tag in [
                '2.55-alpha-113',
                '2.55-alpha-112',
                '2.55-alpha-111',
                '2.55-alpha-110',
            ]:
                continue

            # This one was manually created.
            if tag == '2.55-alpha-114':
                continue

            process_tag(tag, tags[i - 1])

    tags_255 = get_tags('main', '"2.55-alpha-11?" "2.55-alpha-12?"') + get_tags(
        'releases/2.55', '"2.55.*"'
    )
    process_tags(tags_255)

    # Not needed yet.
    # tags_3 = get_tags('main', '"3.*"')

    exit(0)

if args.generate_cherrypicks:
    # Find all the commits in main that were cherry-picked to releases/2.55,
    # and create an override file dropping each one. This override file is only used
    # when generating the 3.0 changelog.
    #
    # There are a few different ways to find these commits, so the resulting override
    # file will group by those methods.
    #
    # Go read https://stackoverflow.com/a/2937724/2788187.

    def get_subject(sha):
        _, subject = (
            subprocess.check_output(
                f'git log -1 --format="%H %s" {sha}',
                shell=True,
                encoding='utf-8',
            )
            .strip()
            .split(' ', 1)
        )
        return subject

    def get_commits(from_sha, to_sha):
        lines = (
            subprocess.check_output(
                f'git log {from_sha}..{to_sha} --format="%H %s"',
                shell=True,
                encoding='utf-8',
            )
            .strip()
            .splitlines()
        )

        commits = []
        for line in lines:
            sha, subject = line.split(' ', 1)
            commits.append((sha, subject))

        return commits

    shas_seen = []

    # cherry-pick'd with the -x option (cherry picked from commit ...)
    output = (
        subprocess.check_output(
            'git log main..releases/2.55 --grep "cherry picked from commit"',
            shell=True,
            encoding='utf-8',
        )
        .strip()
    )
    shas = re.findall(r'cherry picked from commit ([a-zA-Z0-9]+)', output)
    commits_cherry_picked = []
    for sha in shas:
        shas_seen.append(sha)
        commits_cherry_picked.append((sha, get_subject(sha)))

    # Find all the commits that have the same content (no conflict).
    lines = (
        subprocess.check_output(
            'git cherry releases/2.55 main',
            shell=True,
            encoding='utf-8',
        )
        .strip()
        .splitlines()
    )

    shas = []
    for line in lines:
        if line.startswith('-'):
            sha = line[2:]
            shas.append(sha)

    commits_same_content = []
    for sha in shas:
        if sha in shas_seen:
            continue

        shas_seen.append(sha)
        commits_same_content.append((sha, get_subject(sha)))

    # Find commits with same subject line.
    commits_3 = get_commits(
        '2.55.0', 'main'
    )  # TODO: eventually, change 'main' here to '3.0.0'
    commits_255_subjects = [s[1] for s in get_commits('2.55.0', '2.55.8')]

    commits_same_subject = []
    for sha, subject in commits_3:
        if sha in shas_seen:
            continue

        if subject in commits_255_subjects:
            commits_same_subject.append((sha, subject))
            shas_seen.append(sha)

    lines = [
        '# ! Generated by: python scripts/generate_changelog.py --generate-cherrypicks',
        '# ! These are all the commits in the main branch that were cherry-picked to 2.55.x, and then a 2.55.x release was first to have them in an official release',
        '',
    ]

    lines.append('# ! cherry-pick -x')
    for sha, subject in commits_cherry_picked:
        lines.append(f'drop {sha} {subject}')

    lines.append('\n# ! same content')
    for sha, subject in commits_same_content:
        lines.append(f'drop {sha} {subject}')

    lines.append('\n# ! same subject')
    for sha, subject in commits_same_subject:
        lines.append(f'drop {sha} {subject}')

    text = '\n'.join(lines)
    (script_dir / 'changelog_overrides/cherrypicks-3.0.md').write_text(text)
    exit(0)


from_sha = getattr(args, 'from', None)
if from_sha:
    branch = from_sha
else:
    branch = subprocess.check_output(
        'git describe --tags --abbrev=0', shell=True, encoding='utf-8'
    ).strip()

if args.version:
    if args.format == 'markdown':
        print(
            f'To download this release, [visit the ZQuest Classic website](https://zquestclassic.com/releases/{args.version}/) or see the bottom of this page.\n'
        )
    else:
        print(f'https://zquestclassic.com/releases/{args.version}/\n')

if args.for_nightly:
    print(f'The following are the changes since {branch}:\n\n')
    print(generate_changelog(branch, args.to))

    previous_stable_release_tag = subprocess.check_output(
        'git describe --tags --abbrev=0 --match "*.*.*" --match "2.55-alpha-1??" --exclude "*-nightly"',
        shell=True,
        encoding='utf-8',
    ).strip()
    if previous_stable_release_tag != branch:
        print('-------')
        print(f'The following are the changes since {previous_stable_release_tag}:\n\n')
        print('<details>\n<summary>Expand changelog</summary>\n')
        print(generate_changelog(previous_stable_release_tag, args.to))
        print('\n</details>')
else:
    print(generate_changelog(branch, args.to))
