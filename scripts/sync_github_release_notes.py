#!/usr/bin/env python3
"""
Keep GitHub release notes in sync with scripts/generate_changelog.py.

For each release (optionally filtered with --version_pattern), this regenerates
the release notes the same way the release workflow does and compares them with
what's currently published on GitHub.

Default behavior only reads from GitHub and prints a diff for every release
whose notes are out of date - nothing is modified.

Pass --update_release_notes to actually push the regenerated notes. Each release
is shown as a diff first and requires confirmation before it is updated.

Releases are enumerated (and ordered, to find each one's previous release) via
scripts/archives.py, which covers the main and 2.55 release channels. Requires
the `gh` CLI to be installed and authenticated, and the release tags to be
present locally (run `git fetch --tags` if some are missing).
"""

import argparse
import fnmatch
import json
import subprocess
import sys
import tempfile

from difflib import unified_diff
from pathlib import Path

import archives

REPO = 'ZQuestClassic/ZQuestClassic'

script_dir = Path(__file__).resolve().parent
root_dir = script_dir.parent
generate_changelog_script = script_dir / 'generate_changelog.py'

# ANSI SGR codes used with color().
BOLD = '1'
RED = '31'
GREEN = '32'
YELLOW = '33'
CYAN = '36'


def color(text: str, code: str) -> str:
    if not sys.stdout.isatty():
        return text
    return f'\033[{code}m{text}\033[0m'


def git(*args: str) -> str:
    return subprocess.check_output(
        ['git', *args], cwd=root_dir, encoding='utf-8', stderr=subprocess.DEVNULL
    ).strip()


def gh_json(*args: str):
    output = subprocess.check_output(['gh', *args], cwd=root_dir, encoding='utf-8')
    return json.loads(output)


def is_nightly(tag: str) -> bool:
    """
    True for the old dated nightly builds (nightly-YYYY-MM-DD). These aren't
    "official" releases, so official releases skip past them when picking a
    previous version to compare against (matching the release pipeline, which
    finds the previous version with `git describe --match "*.*.*"
    --match "2.55-alpha-1??"` -- neither pattern matches a nightly tag).
    """
    return tag.startswith('nightly')


def build_release_index() -> dict[str, tuple[str | None, int]]:
    """
    Map each release tag to (previous_release_tag, commit_count).

    Release tags come from archives.get_revisions, which enumerates the main and
    2.55 release channels and orders them by commit count. Because releases are
    ordered, the previous release is (almost) the prior entry -- which is how
    release notes have always been sequenced (each covers the commits since the
    release before it). The exception: an official (non-nightly) release compares
    against the previous official release, skipping any nightly builds in between
    (ex: 2.55.0's previous is 2.55-alpha-120, not the nightly just before it).
    Ancient releases that predate this tooling aren't part of these channels and
    are therefore left alone.
    """
    index: dict[str, tuple[str | None, int]] = {}
    for channel in ('main', '2.55'):
        revisions = archives.get_revisions(
            'windows', channel, include_test_builds=False
        )
        for i, rev in enumerate(revisions):
            previous = None
            for prior in reversed(revisions[:i]):
                if not is_nightly(rev.tag) and is_nightly(prior.tag):
                    continue
                previous = prior.tag
                break
            index.setdefault(rev.tag, (previous, rev.commit_count))
    return index


def get_published_release(tag: str) -> tuple[str, bool] | None:
    """The (body, is_prerelease) of a release, or None if it has no GitHub release."""
    try:
        result = gh_json(
            'release', 'view', tag, '--repo', REPO, '--json', 'body,isPrerelease'
        )
    except subprocess.CalledProcessError:
        return None
    return result['body'], result['isPrerelease']


def generate_notes(tag: str, previous_tag: str, commit: str, prerelease: bool) -> str:
    """Regenerate release notes, reproducing the release workflow's invocation."""
    output = subprocess.check_output(
        [
            sys.executable,
            str(generate_changelog_script),
            '--format',
            'markdown',
            '--from',
            previous_tag,
            # Reproduce the release workflow: --to names the version (for the
            # header/URLs) while --to-ref is the actual commit range endpoint.
            '--to',
            tag,
            '--to-ref',
            commit,
            # --for-nightly drives generate_changelog's should_drop_commits:
            # prerelease/nightly notes keep "dropped" (!) commits, stable notes
            # omit them. isPrerelease is the same signal the release workflow fed
            # to --for-nightly, so passing it here reproduces that decision.
            f'--for-nightly={prerelease}',
        ],
        cwd=root_dir,
        encoding='utf-8',
    )
    return output


def normalize(text: str) -> list[str]:
    """Normalize line endings and trailing blank lines for a stable diff."""
    text = text.replace('\r\n', '\n').replace('\r', '\n')
    return text.rstrip('\n').split('\n')


def print_diff(tag: str, published: list[str], generated: list[str]) -> None:
    diff = unified_diff(
        published,
        generated,
        fromfile=f'{tag} (published)',
        tofile=f'{tag} (generated)',
        lineterm='',
    )
    for line in diff:
        if line.startswith('+'):
            print(color(line, GREEN))
        elif line.startswith('-'):
            print(color(line, RED))
        elif line.startswith('@'):
            print(color(line, CYAN))
        else:
            print(line)


def update_notes(tag: str, generated: str) -> None:
    with tempfile.NamedTemporaryFile(
        'w', suffix='.md', encoding='utf-8', delete=False
    ) as f:
        f.write(generated)
        notes_path = f.name
    subprocess.check_call(
        ['gh', 'release', 'edit', tag, '--repo', REPO, '--notes-file', notes_path],
        cwd=root_dir,
    )
    Path(notes_path).unlink(missing_ok=True)


def confirm(tag: str) -> bool:
    try:
        answer = input(color(f'Update release notes for {tag}? [y/N] ', BOLD))
    except EOFError:
        return False
    return answer.strip().lower() in ('y', 'yes')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '--version_pattern',
        help='Only consider releases whose tag matches this glob (ex: "2.55.*"). '
        'If omitted, every release is checked.',
    )
    parser.add_argument(
        '--update_release_notes',
        action='store_true',
        help='Update out-of-date release notes on GitHub (asks for confirmation '
        'after showing each diff). Without this flag, nothing is modified.',
    )
    args = parser.parse_args()

    print('Enumerating releases...', file=sys.stderr)
    index = build_release_index()
    # Newest first.
    tags = sorted(index, key=lambda t: index[t][1], reverse=True)
    if args.version_pattern:
        tags = [t for t in tags if fnmatch.fnmatch(t, args.version_pattern)]

    if not tags:
        print('No releases matched.', file=sys.stderr)
        return 1

    print(f'Checking {len(tags)} release(s)...\n')

    up_to_date = 0
    out_of_date = 0
    updated = 0
    skipped: list[str] = []

    for tag in tags:
        previous_tag, _ = index[tag]
        if not previous_tag:
            # The very first release in a channel has nothing to diff against.
            skipped.append(tag)
            continue

        try:
            commit = git('rev-parse', f'{tag}^{{commit}}')
        except subprocess.CalledProcessError:
            print(
                color(
                    f'! {tag}: tag not found locally (try `git fetch --tags`)', YELLOW
                )
            )
            skipped.append(tag)
            continue

        published_release = get_published_release(tag)
        if published_release is None:
            print(color(f'! {tag}: no GitHub release found; skipping', YELLOW))
            skipped.append(tag)
            continue
        published, prerelease = published_release

        generated = generate_notes(tag, previous_tag, commit, prerelease)

        published_lines = normalize(published)
        generated_lines = normalize(generated)

        if published_lines == generated_lines:
            up_to_date += 1
            print(color(f'✓ {tag}: up to date', GREEN))
            continue

        out_of_date += 1
        print(color(f'✗ {tag}: out of date (vs {previous_tag})', RED))
        print_diff(tag, published_lines, generated_lines)
        print()

        if args.update_release_notes:
            if confirm(tag):
                update_notes(tag, generated)
                updated += 1
                print(color(f'  updated {tag}', GREEN))
            else:
                print(f'  skipped {tag}')
            print()

    print('\n--- Summary ---')
    print(f'up to date:  {up_to_date}')
    print(f'out of date: {out_of_date}')
    if args.update_release_notes:
        print(f'updated:     {updated}')
    if skipped:
        print(f'skipped:     {len(skipped)} ({", ".join(skipped)})')

    return 0


if __name__ == '__main__':
    sys.exit(main())
