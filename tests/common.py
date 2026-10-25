import concurrent.futures
import difflib
import json
import os
import platform
import subprocess
import sys
import unittest

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
test_builds_dir = root_dir / '.tmp/test_builds'


def get_test_concurrency() -> int:
    return max(1, (os.cpu_count() or 1) - 1)


def submit_all(fn, items):
    """
    Submit fn(item) for every item to a thread pool (sized to the machine),
    returning the open executor and a dict mapping each item to its Future.

    The work runs concurrently while the caller consumes results in whatever
    (deterministic) order it likes via future.result(), which re-raises any
    exception from the worker at the point it's consumed. The caller is
    responsible for keeping the executor alive until all results are read,
    e.g. by using it as a context manager.
    """
    executor = concurrent.futures.ThreadPoolExecutor(max_workers=get_test_concurrency())
    futures = {item: executor.submit(fn, item) for item in items}
    return executor, futures


# TODO: this is a duplicated function
def get_release_platform() -> ['mac', 'windows', 'linux']:
    system = platform.system()
    if system == 'Darwin':
        return 'mac'
    elif system == 'Windows':
        return 'windows'
    elif system == 'Linux':
        return 'linux'
    else:
        raise Exception(f'unexpected system: {system}')


def get_recent_release_tag(args: list[str]):
    command = f'git describe --tags --abbrev=0 ' + ' '.join(args)
    return subprocess.check_output(command.split(' '), encoding='utf-8').strip()


def parse_json(json_str: str, context: str):
    try:
        return json.loads(json_str)
    except Exception as e:
        print(f'could not parse json:\n{json_str}\n\n{context}', file=sys.stderr)
        raise e


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
