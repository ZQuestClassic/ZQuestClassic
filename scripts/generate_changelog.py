import argparse
import os
import re
import subprocess

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

parser = argparse.ArgumentParser()
parser.add_argument('--format', choices=['plaintext', 'markdown'], default='plaintext')
parser.add_argument('--from')
parser.add_argument('--to', default='HEAD')
parser.add_argument(
    '--to-ref',
    help='Git ref to use as the range endpoint, if different from --to '
    '(which is still used for the displayed version/URLs). Useful when the '
    'release tag does not exist yet.',
)
parser.add_argument('--for-nightly', type=str2bool, default=False)

args = parser.parse_args()

commit_url_prefix = 'https://github.com/ZQuestClassic/ZQuestClassic/commit'

overrides = dict()
overrides_squashes = dict()


def parse_override_file(file: Path):
    last_override = None
    current_squash_list = []

    for line in file.read_text().splitlines():
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
            raise Exception('expected full hashes')
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


# Trailing "Key: Value" lines with one of these keys are metadata tags, not part
# of the changelog body. Any other "Key: Value" line (ex: a prose "Affected: ..."
# or "NOTE: ...") is left in the body.
tag_keys = frozenset(
    {'Discord', 'Context', 'See', 'Co-authored-by', 'Signed-off-by', 'Agent'}
)


def parse_for_tags(body: str) -> tuple[dict[str, str], str]:
    """
    Extract trailing metadata-tag lines from a commit body.

    Tags are a contiguous block of "Key: Value" lines at the very end of the
    body whose key is one of `tag_keys` (ex: "Discord: ...", "Co-authored-by:
    ..."). Scanning upward from the bottom, the first line that isn't such a tag
    marks the end of the block. Returns a (tags, remaining_body) tuple; the tags
    are stripped out of the returned body so they aren't rendered in the
    changelog.
    """
    tag_pattern = re.compile(r'^([A-Za-z][\w-]*): (.+)$')

    lines = body.splitlines()
    first_tag = len(lines)
    for i in range(len(lines) - 1, -1, -1):
        match = tag_pattern.match(lines[i])
        if match and match.group(1) in tag_keys:
            first_tag = i
        else:
            break

    tags: dict[str, str] = {}
    for line in lines[first_tag:]:
        key, value = tag_pattern.match(line).groups()
        tags[key] = value

    remaining = '\n'.join(lines[:first_tag]).strip()
    return tags, remaining


# A link to the discord channel where a change was discussed. Historically shared
# a few different ways: a "Discord:"/"Context:"/"See:" tag, or a trailing prose
# "See <url>." sentence.
discord_url_pattern = r'https://discord\.com/\S+'


def extract_discord_link(tags: dict[str, str], body: str) -> tuple[str | None, str]:
    """
    Find the Discord discussion link for a commit, if any, and return it along
    with the body (with any trailing prose reference removed). Handles both the
    "Key: <url>" tag forms (already pulled out into `tags`) and an older prose
    "See <url>." / "Context <url>." sentence at the end of the body.
    """
    for key in ('Discord', 'Context', 'See'):
        value = tags.get(key)
        if value and re.match(f'{discord_url_pattern}$', value):
            return value, body

    match = re.search(
        rf'(?:^|\n)[ \t]*(?:See|Context)\s+({discord_url_pattern}?)\.?\s*\Z',
        body,
        flags=re.IGNORECASE,
    )
    if match:
        return match.group(1).rstrip('.'), body[: match.start()].rstrip()

    return None, body


def parse_commit_body(body: str) -> tuple[str, str | None]:
    """
    Turn a raw commit body into its changelog body and Discord link.

    Tags usually come last, after "end changelog" and the technical details,
    but some commits put them just before "end changelog" instead. Both
    places are checked.
    """
    # Drop the cherry-pick trailer first: it isn't a "Key: Value" tag, so it
    # would otherwise block parse_for_tags from reaching the tags above it.
    body = re.sub(
        r'^\(cherry picked from commit .+\)$', '', body, flags=re.MULTILINE
    ).strip()
    # Pull out trailing tags (Discord, Co-authored-by, Signed-off-by, Agent,
    # etc.) so they don't render in the changelog body.
    tags, body = parse_for_tags(body)
    discord, body = extract_discord_link(tags, body)

    m = re.search(r'end changelog', body, re.IGNORECASE)
    if m:
        body = body[0 : m.start()].strip()
        # Only tags before "end changelog" can be shown; the rest are technical
        # details.
        tags, body = parse_for_tags(body)
        inner_discord, body = extract_discord_link(tags, body)
        discord = discord or inner_discord

    # A trailing "See:"/"Context:" that doesn't point at Discord (ex: a
    # GitHub link) is still a useful reference, so keep it visible in the
    # body rather than dropping it as a metadata tag.
    kept_tags = [
        f'{key}: {value}'
        for key, value in tags.items()
        if key in ('See', 'Context') and not re.match(f'{discord_url_pattern}$', value)
    ]
    if kept_tags:
        body = f'{body}\n\n' + '\n'.join(kept_tags)
        body = body.strip()

    return body, discord


_sha_is_commit_cache: dict[str, bool] = {}


def is_commit_sha(token: str) -> bool:
    """Return True if `token` resolves to a real commit in this repo."""
    if token not in _sha_is_commit_cache:
        result = subprocess.run(
            ['git', 'rev-parse', '--verify', '--quiet', f'{token}^{{commit}}'],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        _sha_is_commit_cache[token] = result.returncode == 0
    return _sha_is_commit_cache[token]


def linkify_commit_shas(text: str) -> str:
    """
    Turn bare git SHA references into links to the commit on GitHub.

    Ex: "Regressed in 2.55-alpha-120 (acb7c84)." links the "acb7c84" part.
    Only hex tokens that resolve to an actual commit are linked; text inside a
    URL or an inline `code` span is left untouched.
    """
    pattern = re.compile(r'(`[^`]*`|https?://[^\s)]+)|\b([0-9a-f]{7,40})\b')

    def replacer(match):
        if match.group(1):
            return match.group(1)  # Inline `code` or a URL; leave it alone.
        token = match.group(2)
        if is_commit_sha(token):
            return f'[{token}]({commit_url_prefix}/{token})'
        return token

    return pattern.sub(replacer, text)


@dataclass
class Commit:
    type: str
    scope: str
    short_hash: str
    hash: str
    subject: str
    oneline: str
    body: str
    discord: str | None = None
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
    You should test change to this by previewing in a "releases" page markdown preview. Other
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

        if line.strip().startswith('-') or line.strip().startswith('*'):
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
    commits_by_type: Dict[str, List[Commit]], format: str, from_sha: str, to_sha: str
) -> str:
    lines = []
    oneliner = release_oneliners.get(to_sha, None)
    is_release = to_sha.startswith('2.55') or to_sha.startswith('3')

    if format == 'markdown':
        if oneliner:
            lines.append(f'{oneliner}\n')
        if is_release:
            lines.append(
                f'# [Download from the website](https://zquestclassic.com/releases/{to_sha}), or at the bottom of this page.\n'
            )
            lines.append(
                f'**Full Changelog:** https://github.com/ZQuestClassic/ZQuestClassic/compare/{from_sha}...{to_sha}\n'
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
                    discord_link = f' [Discord]({c.discord})' if c.discord else ''
                    if c.squashed_commits:
                        lines.append(f'- {c.oneline}{discord_link}')
                    else:
                        link = f'[`{c.short_hash}`]({commit_url_prefix}/{c.hash})'
                        lines.append(f'- {c.oneline} {link}{discord_link}')
                    if c.body:
                        lines.append('   &nbsp;')
                        for l in split_text_into_logical_markdown_chunks(c.body):
                            if l:
                                # Don't linkify SHAs inside fenced code blocks.
                                if not l.lstrip().startswith('```'):
                                    l = linkify_commit_shas(l)
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

                if c.discord:
                    # Set off the tag with a blank line before and after, matching
                    # how a body renders. The trailing '\n' supplies the "after".
                    if lines[-1] != '' and not lines[-1].endswith('\n'):
                        lines.append('')
                    lines.append(f'  Discord: {c.discord}\n')
            lines.append('')

    return '\n'.join(lines)


def generate_changelog(from_sha: str, to_sha: str, to_ref: str = None) -> str:
    to_ref = to_ref or to_sha
    commits_text = subprocess.check_output(
        f'git log {from_sha}...{to_ref} --reverse --format="%h %H %s"',
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
        body, discord = parse_commit_body(body)

        type, scope, oneline, drop = parse_scope_and_type(subject)
        if drop:
            continue
        commits.append(
            Commit(type, scope, short_hash, hash, subject, oneline, body, discord)
        )

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

    return stringify_changelog(commits_by_type, args.format, from_sha, to_sha)


for path in (script_dir / 'changelog_overrides').rglob('*.md'):
    if path.name != 'README.md':
        parse_override_file(path)

from_sha = getattr(args, 'from', None)
if from_sha:
    branch = from_sha
else:
    branch = subprocess.check_output(
        'git describe --tags --abbrev=0', shell=True, encoding='utf-8'
    ).strip()

if args.for_nightly:
    print(f'The following are the changes since {branch}:\n\n')
    print(generate_changelog(branch, args.to, args.to_ref))

    previous_full_release_tag = subprocess.check_output(
        'git describe --tags --abbrev=0 --match "2.55-*"', shell=True, encoding='utf-8'
    ).strip()
    if previous_full_release_tag != branch:
        print('-------')
        print(f'The following are the changes since {previous_full_release_tag}:\n\n')
        print('<details>\n<summary>Expand changelog</summary>\n')
        print(generate_changelog(previous_full_release_tag, args.to, args.to_ref))
        print('\n</details>')
else:
    print(generate_changelog(branch, args.to, args.to_ref))
