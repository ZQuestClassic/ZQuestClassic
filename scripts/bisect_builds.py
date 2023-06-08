# To run a bisect:
#   python scripts/bisect_builds.py --token $GH_PAT --good 2.55-alpha-108 --bad 2.55-alpha-109
#   python scripts/bisect_builds.py --token $GH_PAT --bad 2.55-alpha-108 --good 2.55-alpha-109
#
# You can automate running a command on each bisect script, like this:
#   -c '%zq'
#   -c '%zc -test modules/classic/classic_1st.qst 0 119'
#
# To download a specific build:
#   python scripts/bisect_builds.py --token $GH_PAT --download_release 2.55-alpha-109

import argparse
import os
import io
import subprocess
import platform
import requests
import zipfile
import tarfile
import shutil
from joblib import Memory
from typing import List
from pathlib import Path
from github import Github

parser = argparse.ArgumentParser(
    description='Runs a bisect using prebuild releases.')
parser.add_argument('--good')
parser.add_argument('--bad')
parser.add_argument('--token', required=True)
parser.add_argument('--list_releases', action='store_true')
parser.add_argument('--download_release')
parser.add_argument('--channel')
parser.add_argument('-c', '--command',
                    help='command to run on each step. \'%zc\' is replaced with the path to the player, \'%zq\' is the editor, and \'%zl\' is the launcher')

args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
memory = Memory(root_dir / '.tmp/bisect_builds', verbose=0)

gh = Github(args.token)
repo = gh.get_repo('ArmageddonGames/ZQuestClassic')

system = platform.system()
if args.channel:
    channel = args.channel
elif system == 'Darwin':
    channel = 'mac'
elif system == 'Windows':
    channel = 'windows'
elif system == 'Linux':
    channel = 'linux'
else:
    raise Exception(f'unexpected system: {system}')


@memory.cache
def get_release_commit_count(tag: str):
    return int(subprocess.check_output(['git', 'rev-list', '--count', tag], encoding='utf-8'))


def get_releases():
    releases = []

    # TODO maybe use: git tag --merged=main 'nightly*' '2.55-alpha-???' --sort=committerdate
    tags = subprocess.check_output(
        ['git', '-P', 'tag', '--sort=committerdate', '--merged=main'], encoding='utf-8').splitlines()

    for tag in tags:
        commit_count = get_release_commit_count(tag)
        if channel == 'windows' and commit_count < 6252:
            # Every release after this commit will have binaries, but before only some do.
            try:
                get_release_package_url(tag)
            except:
                continue
        if channel == 'mac' and commit_count < 6384:
            continue
        if channel == 'linux' and commit_count < 7404:
            continue
        releases.append({
            'tag': tag,
            'commit_count': commit_count,
        })

    return releases


def get_release_package_url(tag):
    release = repo.get_release(tag)
    assets = list(release.get_assets())

    asset = None
    if channel == 'mac':
        asset = next(asset for asset in assets if asset.name.endswith('.dmg'))
    elif channel == 'windows':
        if len(assets) == 1:
            asset = assets[0]
        else:
            assets = [asset for asset in assets if 'windows' in asset.name]
            asset = next((asset for asset in assets if 'x64' in asset.name), None) or \
                next((asset for asset in assets if 'x86' in asset.name), None)
    elif channel == 'linux':
        asset = next(asset for asset in assets if asset.name.endswith(
            '.tar.gz') or asset.name.endswith('.tgz'))

    if not asset:
        raise Exception(f'could not find package url for {tag}')

    return asset.browser_download_url


def download_release(tag: str):
    url = get_release_package_url(tag)
    dest = releases_dir / tag
    if dest.exists():
        return dest

    dest.mkdir(parents=True)
    print(f'downloading {tag}')

    r = requests.get(url)
    if channel == 'mac':
        (dest / 'ZeldaClassic.dmg').write_bytes(r.content)
        subprocess.check_call(['hdiutil', 'attach', '-mountpoint',
                              str(dest / 'zc-mounted'), str(dest / 'ZeldaClassic.dmg')], stdout=subprocess.DEVNULL)
        shutil.copytree(dest / 'zc-mounted/ZeldaClassic.app',
                        dest / 'ZeldaClassic.app')
        subprocess.check_call(['hdiutil', 'unmount', str(
            dest / 'zc-mounted')], stdout=subprocess.DEVNULL)
        (dest / 'ZeldaClassic.dmg').unlink()
    elif url.endswith('.tar.gz'):
        tf = tarfile.open(fileobj=io.BytesIO(r.content), mode='r:gz')
        tf.extractall(dest)
        tf.close()
    else:
        zip = zipfile.ZipFile(io.BytesIO(r.content))
        zip.extractall(dest)
        zip.close()

    print(f'finished downloading {tag}')
    return dest


def get_release_binaries(tag: str):
    dir = download_release(tag)

    binaries = {'dir': dir}
    if channel == 'mac':
        binaries['zc'] = dir / 'ZeldaClassic.app/Contents/Resources/zelda'
        binaries['zq'] = dir / 'ZeldaClassic.app/Contents/Resources/zquest'
        binaries['zl'] = dir / 'ZeldaClassic.app/Contents/MacOS/zlauncher'
    elif channel == 'windows':
        binaries['zc'] = dir / 'zelda.exe'
        binaries['zq'] = dir / 'zquest.exe'
        binaries['zl'] = dir / 'zlauncher.exe'
    elif channel == 'linux':
        binaries['zc'] = dir / 'zelda'
        binaries['zq'] = dir / 'zquest'
        binaries['zl'] = dir / 'zlauncher'

    return binaries


def AskIsGoodBuild():
    while True:
        prompt = ('Revision is '
                  '[(g)ood/(b)ad/(u)nknown/(q)uit]: ')
        response = input(prompt)
        if response in ('g', 'b', 'u'):
            return response
        if response == 'q':
            raise SystemExit()


def run_bisect(releases: List):
    tags = [r['tag'] for r in releases]
    if args.bad not in tags:
        raise Exception(f'did not find release {args.bad}')
    if args.good not in tags:
        raise Exception(f'did not find release {args.good}')

    bad_rev = tags.index(args.bad)
    good_rev = tags.index(args.good)
    lower_rev = min(good_rev, bad_rev)
    upper_rev = max(good_rev, bad_rev)
    revs = releases[lower_rev:upper_rev+1]
    lower = 0
    upper = len(revs) - 1
    pivot = upper // 2
    release = revs[pivot]
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
                    release = revs[pivot]
                    break
                if check(pivot + i):
                    print(pivot + i)
                    pivot = pivot + i
                    release = revs[pivot]
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
            'roughly %d steps left.' % (revs[lower]['tag'], min_str, revs[upper]['tag'],
                                        max_str, int(upper - lower).bit_length()))

        tag = release['tag']
        print(f'checking {tag}')
        binaries = get_release_binaries(tag)

        down_pivot = int((pivot - lower) / 2) + lower
        up_pivot = int((upper - pivot) / 2) + pivot

        if args.command:
            cmd = args.command
            cmd = cmd.replace('%zc', f'"{binaries["zc"]}"')
            cmd = cmd.replace('%zq', f'"{binaries["zq"]}"')
            cmd = cmd.replace('%zl', f'"{binaries["zl"]}"')
            print(f'running command: {cmd}')
            retcode = subprocess.call(cmd, cwd=binaries['dir'], shell=True)
            print(f'code: {retcode}')

        answer = AskIsGoodBuild()
        if answer == 'g':
            goods.append(pivot)
        elif answer == 'b':
            bads.append(pivot)
        if (answer == 'g' and good_rev < bad_rev) or (answer == 'b' and bad_rev < good_rev):
            lower = pivot
            pivot = up_pivot
        elif (answer == 'b' and good_rev < bad_rev) or (answer == 'g' and bad_rev < good_rev):
            upper = pivot
            pivot = down_pivot
        elif answer == 'u':
            skipped.append(pivot)

        release = revs[pivot]

    DONE_MESSAGE_GOOD_MIN = ('You are probably looking for a change made after %s ('
                             'known good), but no later than %s (first known bad).')
    DONE_MESSAGE_GOOD_MAX = ('You are probably looking for a change made after %s ('
                             'known bad), but no later than %s (first known good).')
    print('bisect finished!\n')

    for i in range(lower, upper + 1):
        state = ' ' * 7
        if i in skipped:
            state = 'SKIPPED'
        if i in goods:
            state = 'GOOD   '
        if i in bads:
            state = 'BAD    '
        print(state, revs[i]['tag'])
    print()

    lower_tag = revs[lower]['tag']
    upper_tag = revs[upper]['tag']
    if good_rev > bad_rev:
        print(DONE_MESSAGE_GOOD_MAX % (lower_tag, upper_tag))
    else:
        print(DONE_MESSAGE_GOOD_MIN % (lower_tag, upper_tag))

    print(
        f'changelog: https://github.com/ArmageddonGames/ZQuestClassic/compare/{lower_tag}...{upper_tag}')


releases = get_releases()

if args.list_releases:
    for release in releases:
        print(f'@{release["commit_count"]} {release["tag"]}')
    exit(0)

if args.download_release:
    download_release(args.download_release)
    exit(0)

run_bisect(releases)
