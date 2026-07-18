import argparse
import os
import re
import subprocess
import sys

from dataclasses import dataclass, field
from pathlib import Path

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
parser.add_argument(
    '--to-ref',
    help='Git ref to use as the range endpoint, if different from --to '
    '(which is still used for the displayed version/URLs). Useful when the '
    'release tag does not exist yet.',
)
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

# What follows is a bunch of code to try and ensure some consistency in the changelog text.

word_replacements = {
    r'\bcset\b': 'CSet',
    r'\bdmap\b': 'DMap',
    r'\bgui\b': 'GUI',
    r'\bjit\b': 'JIT',
    r'\bqr\b': 'QR',
    r'\bscc\b': 'SCC',
    r'\bUnder [cC]ombo\b': 'Undercombo',
    r'\bunder combo\b': 'Undercombo',
    r'\bzasm\b': 'ZASM',
    r'\bzc\b': 'ZC',
    r'\bZQ\b': 'ZQ',
    r'\bzscript\b': 'ZScript',
    # keep lowercase
    r'\beweapon\b': 'eweapon',
    r'\blweapon\b': 'lweapon',
    r'\bzfix\b': 'zfix',
}

# Pre-compile the regex patterns once.
compiled_word_patterns = []


def normalize_text(text: str) -> str:
    if not text:
        return text

    if not compiled_word_patterns:
        for pattern, replacement in word_replacements.items():
            dynamic_pattern = rf'(?<!\.)(?<![/\\])(?:{pattern})(?!\.\w)'
            combined_pattern = rf'(https?://[^\s)]+)|({dynamic_pattern})'

            # Store the compiled regex and its target replacement string together
            compiled_word_patterns.append(
                (re.compile(combined_pattern, flags=re.IGNORECASE), replacement)
            )

    # Split lines but keep the exact original line endings (\n, \r\n, etc.)
    lines = text.splitlines(keepends=True)
    new_lines = []
    in_code_block = False

    for line in lines:
        # Toggle state if we hit a Markdown code block fence
        if line.strip().startswith('```'):
            in_code_block = not in_code_block
            new_lines.append(line)
            continue

        # If we are inside a code block, safely pass the line through untouched
        if in_code_block:
            new_lines.append(line)
            continue

        # If we are outside a code block, apply all our normalizations
        normalized_line = line
        for compiled_regex, replacement in compiled_word_patterns:

            # Using a default argument (rep=replacement) safely binds the current loop's
            # replacement value to the function, avoiding Python's late-binding loop gotchas.
            def replacer(match, rep=replacement):
                if match.group(1):
                    return match.group(1)  # It's a URL
                else:
                    return rep  # It's our target word

            normalized_line = compiled_regex.sub(replacer, normalized_line)

        new_lines.append(normalized_line)

    return ''.join(new_lines)


def wrap_in_backticks(text: str) -> str:
    # Group 1: A single-quoted string containing at least one whitespace character.
    ignore_pattern = r"('[^']*?\s[^']*?')"

    # Group 2: Optional opening quote.
    # Group 3: The actual code syntax (functions, arrays, pointers).
    # Note: We now use \2 to check the closing quote, because Group 1 is taken by the ignore_pattern.
    code_pattern = r"(?<!`)([']?)(?<!\w)([@~]?(?:[a-zA-Z_]\w*::)*[a-zA-Z_]\w*(?:\([^)]*\)|\[[^\]]*\])|[@~]?(?:[a-zA-Z_]\w*::)*(?:[a-zA-Z_]\w*)?->[a-zA-Z_]\w*(?:\([^)]*\)|\[[^\]]*\])?)\2(?:(?=s\b|ed\b|ing\b)|(?!\w))(?!`)"

    # Combine them with an OR (|) operator
    pattern = f"{ignore_pattern}|{code_pattern}"

    def replacer(match):
        if match.group(1):
            # We matched a quoted string with spaces. Return it exactly as is!
            return match.group(1)
        elif match.group(3):
            # We matched a code snippet. Wrap it in backticks, dropping the single quotes.
            return f"`{match.group(3)}`"
        return match.group(0)

    # Pass our replacer function instead of a static string
    return re.sub(pattern, replacer, text)


def maybe_capitalize_first_word(text: str) -> str:
    # Capitalization logic with code-protection.
    if text and text[0].isalpha():
        protected_lowercase_words = {
            'sprintf',
            'printf',
            'trace',
            'itemdata',
            'mapdata',
            'npcdata',
            'combodata',
            'subdata',
        }

        # Extract the very first word to inspect it
        first_word_match = re.match(r'^([a-zA-Z0-9_]+)', text)
        if first_word_match:
            first_word = first_word_match.group(1)

            # Check for obvious code traits
            is_snake_case = '_' in first_word
            is_camel_case = bool(re.search(r'[a-z][A-Z]', first_word))
            is_protected = first_word.lower() in protected_lowercase_words

            # Only capitalize if it looks like a normal English word
            if not (is_snake_case or is_camel_case or is_protected):
                text = text[0].upper() + text[1:]

    return text


def normalize_oneline(text: str) -> str:
    if not text:
        return text

    # Wrap function calls, array accesses, or pointer accesses in backticks.
    text = wrap_in_backticks(text)
    text = maybe_capitalize_first_word(text)
    if text.endswith('.'):
        text = text[:-1]
    text = normalize_text(text)

    return text


def parse_override_file(file: Path):
    last_override = None
    current_squash_list = []

    # Read lines keeping trailing whitespace/newlines to perfectly restore formatting.
    original_lines = file.read_text().splitlines(keepends=True)
    new_lines = []
    file_modified = False

    for original_line in original_lines:
        if original_line.startswith('# !'):
            new_lines.append(original_line)
            continue

        line = original_line.rstrip()

        if (
            last_override
            and last_override[0] in ['reword', 'section']
            and not re.match(r'^(reword|section|subject|squash|drop|pick)', line)
        ):
            if line == '=end':
                last_override = None
                new_lines.append(original_line)
                continue

            last_override[1] += f'\n{line}'
            new_lines.append(original_line)
            continue

        if not line:
            new_lines.append(original_line)
            continue

        parts = line.split(' ', 2)
        if len(parts) == 3:
            type, hash, rest = parts
        else:
            type, hash, rest = [*parts, None]

        # Normalize short hashes to full 40-character hashes.
        if len(hash) != 40:
            try:
                # --verify ensures the hash uniquely identifies exactly 1 commit
                full_hash = subprocess.check_output(
                    ['git', 'rev-parse', '--verify', hash],
                    stderr=subprocess.STDOUT,
                    text=True,
                ).strip()
            except subprocess.CalledProcessError as e:
                raise Exception(
                    f"Short hash does not uniquely identify 1 commit: '{hash}'.\nGit error: {e.output.strip()}"
                )

            if len(full_hash) != 40:
                raise Exception(
                    f"Expected a 40-character hash for '{hash}', but got: '{full_hash}'"
                )

            # Safely replace the short hash with the full hash in the original string,
            # leveraging the known 'type' prefix to prevent accidental replacements elsewhere in the line.
            original_line = original_line.replace(
                f'{type} {hash}', f'{type} {full_hash}', 1
            )
            hash = full_hash
            file_modified = True

        # Normalize drop subjects to match the real commit subject.
        if type == 'drop':
            try:
                real_subject = subprocess.check_output(
                    ['git', 'log', '-1', '--format=%s', hash],
                    stderr=subprocess.STDOUT,
                    text=True,
                ).strip()
            except subprocess.CalledProcessError as e:
                raise Exception(
                    f"Failed to fetch subject for '{hash}'.\nGit error: {e.output.strip()}"
                )

            if rest != real_subject:
                # Capture the original trailing whitespace (like '\n') to keep formatting intact.
                trailing_chars = original_line[len(original_line.rstrip()) :]
                original_line = f"{type} {hash} {real_subject}{trailing_chars}"
                rest = real_subject
                file_modified = True

        if hash in overrides:
            raise Exception(f'hash already present: {hash}')

        if type == 'squash':
            current_squash_list.append(hash)
        elif last_override and last_override[0] == 'squash':
            overrides_squashes[hash] = current_squash_list
            current_squash_list = []

        overrides[hash] = last_override = [type, rest]
        new_lines.append(original_line)

    if last_override and last_override[0] == 'squash':
        raise Exception('squash into what?')

    # Write the formatted output back to the file if any hashes were expanded
    if file_modified:
        file.write_text(''.join(new_lines))


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
    squashed_commits: list['Commit'] = field(default_factory=lambda: [])

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

    drop = subject.startswith('Revert')
    return 'misc', None, subject, drop


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


def split_text_into_logical_markdown_chunks(text: str) -> list[str]:
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
    commits_by_type: dict[str, list[Commit]], format: str, from_sha: str, to_sha: str
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

            commits_by_scope: dict[str, list[Commit]] = {}
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

                if c.discord:
                    # Set off the tag with a blank line before and after, matching
                    # how a body renders. The trailing '\n' supplies the "after".
                    if lines[-1] != '' and not lines[-1].endswith('\n'):
                        lines.append('')
                    lines.append(f'  Discord: {c.discord}\n')
            lines.append('')

    if lines[-1] != '':
        lines.append('')
    return '\n'.join(lines)


def generate_changelog(from_sha: str, to_sha: str, to_ref: str = None) -> str:
    # to_sha names the release (used for the displayed version/URLs); to_ref is
    # the actual git range endpoint, which may differ when the tag doesn't exist
    # yet (ex: during a release, where to_ref is the built commit).
    to_ref = to_ref or to_sha
    commits_text = subprocess.check_output(
        f'git log {from_sha}..{to_ref} --reverse --format="%h %H %s"',
        shell=True,
        encoding='utf-8',
    ).strip()

    commits: list[Commit] = []
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
        # Drop the cherry-pick trailer first: it isn't a "Key: Value" tag, so it
        # would otherwise block parse_for_tags from reaching the tags above it.
        body = re.sub(
            r'^\(cherry picked from commit .+\)$', '', body, flags=re.MULTILINE
        ).strip()
        # Pull out trailing tags (Discord, Co-authored-by, Signed-off-by, Agent,
        # etc.) so they don't render in the changelog body.
        tags, body = parse_for_tags(body)
        discord, body = extract_discord_link(tags, body)
        # A trailing "See:"/"Context:" that doesn't point at Discord (ex: a
        # GitHub link) is still a useful reference, so keep it visible in the
        # body rather than dropping it as a metadata tag.
        kept_tags = [
            f'{key}: {value}'
            for key, value in tags.items()
            if key in ('See', 'Context')
            and not re.match(f'{discord_url_pattern}$', value)
        ]
        if kept_tags:
            body = f'{body}\n\n' + '\n'.join(kept_tags)
            body = body.strip()

        # Check if the subject is being overridden before testing for drops.
        subject_to_parse = subject
        did_override_subject = False
        if hash in overrides:
            if overrides[hash][0] in ['subject', 'squash', 'pick']:
                subject_to_parse = overrides[hash][1]
                did_override_subject = True
            elif overrides[hash][0] == 'reword':
                subject_to_parse = overrides[hash][1].splitlines()[0]
                did_override_subject = True

        type, scope, oneline, drop = parse_scope_and_type(subject_to_parse)

        if drop and should_drop_commits:
            continue

        if not did_override_subject:
            oneline = normalize_oneline(oneline)
        body = normalize_text(body)

        commits.append(
            Commit(type, scope, short_hash, hash, subject, oneline, body, discord)
        )

    # Replace commit messages with overrides.
    manual_squashes = dict()
    manual_squash_hash = None
    for commit in commits:
        hash = commit.hash
        reparse = True
        if hash in overrides and overrides[hash][0] in ['subject', 'squash', 'pick']:
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

    commits_by_type: dict[str, list[Commit]] = {}
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

    changelog = stringify_changelog(commits_by_type, args.format, from_sha, to_sha)
    # 125k is the maximum length for the api call to update the body. Give a 2k buffer.
    if len(changelog) > 123000:
        print(
            f'WARNING: changelog may be too big to publish to GitHub. Length: {len(changelog)}',
            file=sys.stderr,
        )
    return changelog


# Stale hashes in the override files (from amending/rebasing a commit after
# writing its override) keep working locally, where the old object still
# exists, but break in a fresh clone. Run the reachability test here for local
# dev.
if 'CI' not in os.environ:
    test_result = subprocess.run(
        [sys.executable, str(root_dir / 'tests' / 'test_changelog.py')],
        capture_output=True,
        text=True,
    )
    if test_result.returncode != 0:
        print(test_result.stderr, file=sys.stderr)
        sys.exit(1)

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
    output = subprocess.check_output(
        'git log main..releases/2.55 --grep "cherry picked from commit"',
        shell=True,
        encoding='utf-8',
    ).strip()
    shas = re.findall(r'cherry picked from commit ([a-zA-Z0-9]+)', output)
    commits_cherry_picked = []
    for sha in shas:
        if sha == '3dbae6acb1da74dbf649f2f15213855c104e04e6':
            sha = '4541c666fa80b63fafe9c16a1f363d9054019216'
        if sha in shas_seen:
            continue

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
    # Compare against everything shipped on the release branch: start from the
    # point where releases/2.55 diverged from main (commits cherry-picked into
    # 2.55.0 itself would otherwise be missed), and end at the latest released
    # 2.55.x tag.
    release_branch_point = subprocess.check_output(
        'git merge-base main releases/2.55',
        shell=True,
        encoding='utf-8',
    ).strip()
    latest_255_tag = subprocess.check_output(
        'git describe --tags --abbrev=0 releases/2.55',
        shell=True,
        encoding='utf-8',
    ).strip()
    commits_255_subjects = [
        s[1] for s in get_commits(release_branch_point, latest_255_tag)
    ]

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

print(generate_changelog(branch, args.to, args.to_ref))
