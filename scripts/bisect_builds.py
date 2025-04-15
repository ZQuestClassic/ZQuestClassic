# To run a bisect:
#   python scripts/bisect_builds.py --good 2.55-alpha-108 --bad 2.55-alpha-109
#   python scripts/bisect_builds.py --bad 2.55-alpha-108 --good 2.55-alpha-109
#
# You can automate running a command on each bisect script by giving the command after `--`, like this:
#   python scripts/bisect_builds.py ... -- %zq
#   python scripts/bisect_builds.py ... -- %zc -test "/absolute/path/to/quest.qst" 0 119
#
# Use the '--local_builds' flag to build additional commits locally. This will take much longer, so
# run without first to get a more narrow range.

import argparse
import os
import sys

from pathlib import Path
from typing import List

import archives
import common

from archives import Revision
from joblib import Memory

parser = argparse.ArgumentParser(description='Runs a bisect using prebuilt releases.')
parser.add_argument('--good')
parser.add_argument('--bad')
parser.add_argument(
    '--test_builds',
    action=argparse.BooleanOptionalAction,
    default=True,
    help='Includes pre-built builds not associated with official releases',
)
parser.add_argument(
    '--local_builds',
    action=argparse.BooleanOptionalAction,
    default=False,
    help='Includes all commits and builds locally if prebuilt binaries are not present. Uses a temporary checkout at .tmp/local_build_working_dir',
)
parser.add_argument('--platform', default=common.get_release_platform())
parser.add_argument('--channel', default='main')
parser.add_argument(
    '--',
    dest='command',
    nargs='+',
    help='command to run on each step. \'%%zc\' is replaced with the path to the player, \'%%zq\' is the editor, and \'%%zl\' is the launcher',
)
parser.add_argument(
    '--check_return_code', action=argparse.BooleanOptionalAction, default=False
)

# Grab all the arguments after '--'.
argv = sys.argv[1:]
idx = argv.index('--') if '--' in argv else -1
unparsed = argv[idx + 1 :] if idx >= 0 else None

# Give argparse the rest.
argv = argv[:idx] if idx >= 0 else argv
args = parser.parse_args(argv)

# Replace the contents of the dummy argument with the unparsed args.
args.command = unparsed

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
archives_dir = root_dir / '.tmp/archives'
memory = Memory(root_dir / '.tmp/bisect_builds', verbose=0)

if args.channel == 'main':
    branch = 'main'
elif args.channel == '2.55':
    branch = 'releases/2.55'
commit_counts = archives.get_commit_counts(branch)


def AskIsGoodBuild():
    while True:
        prompt = 'Revision is ' '[(g)ood/(b)ad/(u)nknown/(q)uit]: '
        response = input(prompt)
        if response in ('g', 'b', 'u', 'q'):
            return response


def run_bisect(revisions: List[Revision]):
    tags = [r.tag for r in revisions]
    if args.bad not in tags:
        raise Exception(f'did not find release {args.bad}')
    if args.good not in tags:
        raise Exception(f'did not find release {args.good}')

    bad_rev = tags.index(args.bad)
    good_rev = tags.index(args.good)
    lower_rev = min(good_rev, bad_rev)
    upper_rev = max(good_rev, bad_rev)
    revs = revisions[lower_rev : upper_rev + 1]
    lower = 0
    upper = len(revs) - 1
    pivot = upper // 2
    rev = revs[pivot]
    skipped = []
    goods = [upper if good_rev > bad_rev else 0]
    bads = [upper if good_rev <= bad_rev else 0]

    while upper - lower > 1:
        if pivot in skipped:
            for i in range(0, upper - lower):

                def check(k):
                    if k <= lower or k >= upper:
                        return False
                    return k not in skipped

                if check(pivot - i):
                    print(pivot - i)
                    pivot = pivot - i
                    rev = revs[pivot]
                    break
                if check(pivot + i):
                    print(pivot + i)
                    pivot = pivot + i
                    rev = revs[pivot]
                    break
        if pivot in skipped:
            print('skipped all options')
            break

        if bad_rev < good_rev:
            min_str, max_str = 'bad', 'good'
        else:
            min_str, max_str = 'good', 'bad'
        print(
            '=== Bisecting range [%s (%s), %s (%s)], '
            'roughly %d steps left.'
            % (
                revs[lower].tag,
                min_str,
                revs[upper].tag,
                max_str,
                int(upper - lower).bit_length(),
            )
        )

        lower_tag = revs[lower].tag
        upper_tag = revs[upper].tag
        print(
            f'changelog of current range: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}'
        )

        print(f'checking {rev}')
        binaries = rev.binaries(args.platform)

        down_pivot = int((pivot - lower) / 2) + lower
        up_pivot = int((upper - pivot) / 2) + pivot

        if not binaries:
            answer = 'u'
        elif args.command:
            p = common.run_zc_command(binaries, args.command)
            if args.check_return_code:
                retcode = p.wait()
                answer = 'g' if retcode == 0 else 'b'
                print(f'code: {retcode}, answer: {answer}')
            else:
                answer = AskIsGoodBuild()
                p.terminate()
        else:
            answer = AskIsGoodBuild()

        if answer == 'q':
            raise SystemExit()

        if answer == 'g':
            goods.append(pivot)
        elif answer == 'b':
            bads.append(pivot)
        if (answer == 'g' and good_rev < bad_rev) or (
            answer == 'b' and bad_rev < good_rev
        ):
            lower = pivot
            pivot = up_pivot
        elif (answer == 'b' and good_rev < bad_rev) or (
            answer == 'g' and bad_rev < good_rev
        ):
            upper = pivot
            pivot = down_pivot
        elif answer == 'u':
            skipped.append(pivot)

        rev = revs[pivot]

    DONE_MESSAGE_GOOD_MIN = (
        'You are probably looking for a change made after %s ('
        'known good), but no later than %s (first known bad).'
    )
    DONE_MESSAGE_GOOD_MAX = (
        'You are probably looking for a change made after %s ('
        'known bad), but no later than %s (first known good).'
    )
    print('bisect finished!\n')

    for i in range(lower, upper + 1):
        state = ' ' * 7
        if i in skipped:
            state = 'SKIPPED'
        if i in goods:
            state = 'GOOD   '
        if i in bads:
            state = 'BAD    '
        print(state, revs[i].tag)
    print()

    lower_tag = revs[lower].tag
    upper_tag = revs[upper].tag
    if good_rev > bad_rev:
        print(DONE_MESSAGE_GOOD_MAX % (lower_tag, upper_tag))
    else:
        print(DONE_MESSAGE_GOOD_MIN % (lower_tag, upper_tag))

    print(
        f'changelog: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}'
    )


revisions = archives.get_revisions(
    args.platform,
    args.channel,
    include_test_builds=args.test_builds,
    may_build_locally=args.local_builds,
)
run_bisect(revisions)
