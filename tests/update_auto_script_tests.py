# Generates and updates replay tests for the scripts in `tests/scripts/playground/auto`
#
# The replay tests this script generates are `tests/replays/playground/auto_*.zplay`
#
# When adding or modifying existing scripts in the `auto` folder, you must
# run this script. See tests/scripts/README.md for more information.
#
# To just run all the auto replays tests:
#
# python tests/update_auto_script_tests.py --test

import argparse
import json
import os
import re
import shutil
import subprocess
import sys

from dataclasses import dataclass
from pathlib import Path

from lib.replay_helpers import parse_result_txt_file
from replays import ReplayTestResults

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
auto_tests_dir = script_dir / 'scripts/playground/auto'
auto_replays_dir = root_dir / 'tests/replays/playground'
qst_path = script_dir / 'replays/playground/playground.qst'
tmp_dir = root_dir / '.tmp/update_auto_script_tests'
is_ci = 'CI' in os.environ

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


@dataclass
class Test:
    script_path: Path
    replay_path: Path

    def name(self):
        return self.script_path.stem


tests = [
    Test(path, root_dir / f'{auto_replays_dir}/auto_{path.stem}.zplay')
    for path in auto_tests_dir.rglob('*.zs')
    if path.name != 'test_runner.zs' and path.name != 'auto.zs'
]
tests_missing_replays = [t for t in tests if not t.replay_path.exists()]


def compile():
    print('recompiling playground.qst')

    # TODO: set this via CLI
    include_paths = [
        str(root_dir / 'resources/include'),
        str(root_dir / 'resources/headers'),
        str(root_dir / 'tests/scripts/playground'),
        str(root_dir / 'tests/scripts'),
    ]
    (run_target.get_build_folder() / 'includepaths.txt').write_text(
        ';'.join(include_paths)
    )

    log = run_target.get_build_folder() / 'allegro.log'
    if log.exists():
        log.unlink()
    args = [
        '-headless',
        '-smart-assign',
        qst_path,
    ]
    try:
        run_target.check_run('zeditor', args)
    except Exception as e:
        e.add_note(log.read_text())
        raise e


def create_replay(test: Test):
    print(
        f'creating replay for {test.script_path.relative_to(root_dir)}: {test.replay_path.relative_to(root_dir)}'
    )

    log = run_target.get_build_folder() / 'allegro.log'
    if log.exists():
        log.unlink()
    args = [
        '-test',
        qst_path,
        '5',
        '0',
        '-record',
        test.replay_path,
        '-replay-name',
        test.name(),
        '-replay-script-trace',
    ]
    try:
        run_target.check_run('zplayer', args)
    except Exception as e:
        e.add_note(log.read_text())
        raise e

    content = re.sub(
        r'M qst .*\n',
        'M qst playground.qst\n',
        test.replay_path.read_text(),
    )
    content = re.sub(
        r'M version .*\n',
        'M version latest\n',
        content,
    )
    test.replay_path.write_text(content)


def run_replays(tests: list[Test], update=False):
    if not tests:
        return

    if update:
        print('updating auto script test replays')
    else:
        print('running auto script test replays')

    output_dir = tmp_dir / 'replays_output'
    shutil.rmtree(output_dir, ignore_errors=True)

    args = [
        sys.executable,
        root_dir / 'tests/run_replay_tests.py',
        '--build_folder',
        run_target.get_build_folder(),
        '--test_results',
        output_dir,
        '--retries',
        '2',
        '--no-jit',
    ]
    if update:
        args.append('--update')
    args.extend(t.replay_path for t in tests)

    output = subprocess.run(args, stdout=subprocess.PIPE, encoding='utf-8')
    test_results_path = output_dir / 'test_results.json'
    if not test_results_path.exists():
        print(output.stdout)
        raise Exception('could not find test_results.json')

    if update:
        output.check_returncode()
        return

    if output.returncode:
        test_results_json = json.loads(test_results_path.read_text('utf-8'))
        results = ReplayTestResults(**test_results_json)
        results.print_failures(output_dir)
        exit(output.returncode)


def validate(tests: list[Test]):
    print('validating auto script tests')

    failed_verification = False
    for test in tests:
        content = test.replay_path.read_text()
        test_lines = [l for l in content.splitlines() if '[Test]' in l]
        failed_lines = [l for l in test_lines if '[Test] failed' in l]

        errors = []
        if not any(l for l in test_lines if '[Test] started' in l):
            errors.append('missing call to Test::Init()')
        for line in failed_lines:
            errors.append(line)
        if not any(l for l in test_lines if '[Test] done' in l):
            errors.append('test ended abnormally')

        if errors:
            failed_verification = True
            print(f'Test {test.name()} failed:')
            print('\n'.join(errors))
            print()

    if failed_verification:
        exit(1)


def update_auto_script_tests():
    auto_zh = '// Generated by tests/update_auto_script_tests.py\n\n'
    auto_zh += '\n'.join(sorted([f'#include "auto/{t.name()}.zs"' for t in tests]))
    auto_zh += '\n'
    (auto_tests_dir / 'auto.zs').write_text(auto_zh)

    if is_ci:
        if tests_missing_replays:
            names = ', '.join(t.name() for t in tests_missing_replays)
            raise Exception(
                f'No replay found for auto test: {names}\nYou must run `python tests/update_auto_script_tests.py`'
            )

        validate(tests)
        run_replays(tests)
        return

    compile()
    for test in tests_missing_replays:
        create_replay(test)
    run_replays([t for t in tests if t not in tests_missing_replays], update=True)
    validate(tests)

    print(
        f'done - verify that there are no unexpected changes to the replays in {auto_replays_dir.relative_to(root_dir)}'
    )


parser = argparse.ArgumentParser(
    description='Generates and updates replay tests for the scripts in `tests/scripts/playground/auto`'
)
parser.add_argument(
    '--test',
    action=argparse.BooleanOptionalAction,
    default=False,
    help='Runs all the auto replay tests, without updating',
)

args = parser.parse_args()
if args.test:
    run_replays(tests)
else:
    update_auto_script_tests()
