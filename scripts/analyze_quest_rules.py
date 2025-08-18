# python -m pip install git+ssh://git@github.com/connorjclark/zquest-data.git@2e9c43

# To update the files in docs folder:
#   python scripts/analyze_quest_rules.py > docs/quest_database.md
#   python scripts/analyze_quest_rules.py --no_compat > docs/quest_database_no_compat.md

import argparse
import logging
import os
import subprocess
import sys

from pathlib import Path

from joblib import Memory
from zquest import constants
from zquest.extract import ZeldaClassicReader
from zquest.section_utils import SECTION_IDS

logging.getLogger('zc').setLevel(logging.FATAL)

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/cache_analyze_replay_tests'
memory = Memory(tmp_dir / 'memory', verbose=0)

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument(
    '--quest_database', type=Path, default=root_dir / '.tmp/database/quests'
)
parser.add_argument('--no_compat', action='store_true')
parser.add_argument('--analyze_replay_tests', action='store_true')
parser.add_argument('--build_folder', default='build/Release')
args = parser.parse_args()

build_folder = Path(args.build_folder).absolute()

quest_rules = {
    qr: qr_name for qr, qr_name in enumerate(constants.quest_rules) if qr_name
}

quest_database: list[Path] = None
if args.quest_database:
    quest_database = list(args.quest_database.rglob('*.qst'))

    def get_pzc_id_number(path: Path):
        first_part = path.relative_to(args.quest_database).parts[0]
        return int(first_part) if first_part.isdigit() else -1

    # Sort by ids, which is the first directory in each path.
    quest_database = sorted(quest_database, key=get_pzc_id_number)


def get_post_compat_rules(path: Path):
    tmp_path = tmp_dir / 'tmp.qst'
    exe_name = 'zeditor.exe' if os.name == 'nt' else 'zeditor'
    args = [
        build_folder / exe_name,
        '-copy-qst',
        path,
        tmp_path,
    ]
    output = subprocess.run(
        args,
        cwd=build_folder,
        encoding='utf-8',
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if output.returncode != 0:
        raise Exception(f'got error: {output.returncode}\n{output.stdout}')

    reader = ZeldaClassicReader(
        str(tmp_path),
        {
            'only_sections': [SECTION_IDS.RULES],
        },
    )
    reader.read_qst()
    tmp_path.unlink()
    return reader.get_quest_rules()


@memory.cache
def read_qst_impl(path: Path):
    reader = ZeldaClassicReader(
        str(path),
        {
            'only_sections': [SECTION_IDS.RULES],
        },
    )
    reader.read_qst()

    return reader.header, reader.get_quest_rules(), get_post_compat_rules(path)


def read_qst(path: Path):
    try:
        return read_qst_impl(path)
    except Exception as e:
        # TODO: some quests fail to decode. ex: 73/thequest.qst
        print('error reading quest!', file=sys.stderr)
        print(e, file=sys.stderr)
        return None


def get_qst_rules(path: Path):
    res = read_qst(path)
    if not res:
        return None

    _, qrs_bf, qrs_compat_bf = res
    if args.no_compat:
        return qrs_bf
    else:
        return qrs_compat_bf


def trim_nul_str(s: str):
    index = s.find('\0')
    if index == -1:
        return s
    return s[0:index]


def print_quest_rules(qsts: list[Path]):
    for qst in qsts:
        qrs_bf = get_qst_rules(qst)
        header = read_qst(qst)[0]
        print(f'\n## {trim_nul_str(header.title)}')
        print('```')
        print('qst:', qst.relative_to(args.quest_database))
        print(f'title: {trim_nul_str(header.title)}')
        print(f'author: {trim_nul_str(header.author)}')
        if header.build:
            print(f'zc version: 0x{header.zelda_version:02x} build {header.build}')
        else:
            print(f'zc version: 0x{header.zelda_version:02x}')
        print('qrs:', ', '.join(qrs_bf.get_values()))
        print('```')


def count_quest_rules(qsts: list[Path]):
    counts = {qr: 0 for qr in quest_rules.keys()}
    for qst in qsts:
        qrs_bf = get_qst_rules(qst)
        qrs = qrs_bf.get_indices()
        for qr in qrs:
            counts[qr] += 1
    return counts


def determine_missing_coverage(covered_qrs: set[int], uncovered_qsts: set[Path]):
    qst_to_uncovered_count = {}
    for qst in uncovered_qsts:
        qrs_bf = get_qst_rules(qst)
        if not qrs_bf:
            continue

        uncovered_qrs = set(qrs_bf.get_indices()) - covered_qrs
        qst_to_uncovered_count[qst] = len(uncovered_qrs)

    if not qst_to_uncovered_count:
        return []

    max_count = max(qst_to_uncovered_count.values())
    if max_count == 0:
        return []

    candidates = [
        qst for qst, count in qst_to_uncovered_count.items() if count == max_count
    ]
    return candidates


# Prune the quests the can't yet be read.
if quest_database:
    num_quests = len(quest_database)
    max_digits = len(str(num_quests))

    valid_quests = []
    for qst in quest_database:
        index = quest_database.index(qst)
        progress = f'({index + 1:{max_digits}d}/{num_quests})'
        rel_name = qst.relative_to(args.quest_database)
        print(f'{progress} reading {rel_name}', file=sys.stderr)

        if get_qst_rules(qst):
            valid_quests.append(qst)
    quest_database = valid_quests


if args.analyze_replay_tests:
    qsts = list((root_dir / 'tests/replays').rglob('*.qst'))
    counts = count_quest_rules(qsts)

    initial_covered_qrs = set(qr for qr, count in counts.items() if count > 0)
    covered_count = len(initial_covered_qrs)
    print(f'qrs covered by tests:  {covered_count} / {len(quest_rules)}')
    print('')

    for qr, count in sorted(counts.items(), key=lambda x: -x[1]):
        print(f'{count:<3d}', quest_rules[qr])

    print('')

    if not quest_database:
        print('missing quest_database arg')
        exit(1)

    uncovered_qsts = set(quest_database)
    covered_qrs = initial_covered_qrs
    iterations = 0
    num_qrs = len(quest_rules)
    critical_path = []
    print('\ndetermining which quests best cover missing qrs...')
    while len(covered_qrs) < num_qrs:
        print(f'[{iterations}] qr coverage so far: {len(covered_qrs)} / {num_qrs}')
        candidates = determine_missing_coverage(covered_qrs, uncovered_qsts)
        if not candidates:
            break

        for candidate in candidates:
            print(f'\t{candidate.relative_to(args.quest_database)}')

        critical_path.append(candidates[0])
        uncovered_qsts.remove(candidates[0])
        covered_qrs = covered_qrs.union(get_qst_rules(candidates[0]).get_indices())
        iterations += 1

    print('\ncritical path:')
    for qst in critical_path:
        new_qr_count = len(set(get_qst_rules(qst).get_indices()) - initial_covered_qrs)
        print(f'\t({new_qr_count}) {qst.relative_to(args.quest_database)}')

    exit(0)


if args.no_compat:
    print(
        'the quest rules listed are what the engine considers the QRs to be, after all versioning upgrades\n'
    )

print(f'number of quests: {len(quest_database)}')

print('\n# QR frequency\n')
counts = count_quest_rules(quest_database)
print('```')
for qr, count in sorted(counts.items(), key=lambda x: -x[1]):
    print(f'{count:<3d}', quest_rules[qr])
print('```')

print('\n# Quests\n')
print_quest_rules(quest_database)
