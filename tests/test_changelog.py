# Tests for scripts/generate_changelog.py and its override files.

import os
import re
import subprocess
import unittest

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
overrides_dir = root_dir / 'scripts' / 'changelog_overrides'

override_types = ['subject', 'reword', 'squash', 'pick', 'drop', 'section']
override_line_re = re.compile(rf"^({'|'.join(override_types)}) ([0-9a-f]{{7,40}})\b")


def git(*args):
    return subprocess.check_output(
        ['git', *args], cwd=root_dir, text=True, stderr=subprocess.DEVNULL
    ).strip()


def collect_override_hashes():
    """Yield (file, line_number, hash) for every override entry."""
    for file in sorted(overrides_dir.glob('*.md')):
        for line_number, line in enumerate(file.read_text().splitlines(), 1):
            m = override_line_re.match(line)
            if m:
                yield file, line_number, m.group(2)


class TestChangelog(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None

    def test_override_hashes_are_reachable(self):
        # Amending or rebasing a commit after adding it to an override file
        # leaves the old hash as a dangling object in the author's local repo,
        # so generate_changelog.py keeps working there — but a fresh clone (like
        # the publish workflow's) only has reachable objects, and `git log -1
        # <hash>` fails with "bad object". Require every override hash to be
        # reachable from some branch so the stale entry is caught before it
        # breaks a release.
        try:
            git('rev-parse', '--git-dir')
        except (subprocess.CalledProcessError, FileNotFoundError):
            raise unittest.SkipTest('not a git checkout')

        if git('rev-parse', '--is-shallow-repository') == 'true':
            raise unittest.SkipTest('shallow clone: full history unavailable')

        reachable = set(git('rev-list', '--branches', '--remotes', 'HEAD').split())
        self.assertGreater(len(reachable), 1000, 'unexpectedly small history')

        bad = []
        for file, line_number, hash in collect_override_hashes():
            if len(hash) < 40:
                # Short hashes are normalized to 40 characters by
                # generate_changelog.py; resolve manually here.
                try:
                    hash = git('rev-parse', '--verify', f'{hash}^{{commit}}')
                except subprocess.CalledProcessError:
                    bad.append(
                        f'{file.relative_to(root_dir)}:{line_number}: unknown commit {hash}'
                    )
                    continue

            if hash not in reachable:
                bad.append(f'{file.relative_to(root_dir)}:{line_number}: {hash}')

        self.assertFalse(
            bad,
            'Override hashes not reachable from any branch. The commit was\n'
            'likely amended or rebased after the override was written; update\n'
            'the entry to the hash that actually landed (or delete it if the\n'
            'landed commit is already hidden via "!"):\n' + '\n'.join(bad),
        )


if __name__ == '__main__':
    unittest.main()
