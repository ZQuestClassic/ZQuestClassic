import difflib
import os
import subprocess
import unittest

from pathlib import Path
from typing import List

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
test_builds_dir = root_dir / '.tmp/test_builds'


def get_recent_release_tag(args: List[str]):
    command = f'git describe --tags --abbrev=0 ' + ' '.join(args)
    return subprocess.check_output(command.split(' '), encoding='utf-8').strip()


class ZCTestCase(unittest.TestCase):
    def expect_snapshot(self, expected_path: Path, actual: str, update: bool):
        expected = None
        if expected_path.exists():
            expected = expected_path.read_text()

        if update:
            if expected != actual:
                print(f'updating snapshot {expected_path.name}')
                expected_path.parent.mkdir(parents=True, exist_ok=True)
                expected_path.write_text(actual)
        else:
            if expected == None:
                expected_path.parent.mkdir(parents=True, exist_ok=True)
                expected_path.write_text(actual)
            else:
                if expected != actual:
                    lines = list(
                        difflib.context_diff(
                            expected.splitlines(keepends=True),
                            actual.splitlines(keepends=True),
                        )
                    )
                    # Limit to 1000 lines.
                    lines = lines[:1000]
                    self.fail(''.join(lines))
