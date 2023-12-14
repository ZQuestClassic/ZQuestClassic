# To run a bisect:
#   python scripts/bisect_builds.py --token $GH_PAT --good 2.55-alpha-108 --bad 2.55-alpha-109
#   python scripts/bisect_builds.py --token $GH_PAT --bad 2.55-alpha-108 --good 2.55-alpha-109
#
# You can automate running a command on each bisect script, like this:
#   -c '%zq'
#   -c '%zc -test "quests/Z1 Recreations/classic_1st.qst" 0 119'
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
import tempfile
from dataclasses import dataclass
from joblib import Memory
from typing import List, Optional
from pathlib import Path
from github import Github, WorkflowRun
# from ..tests.common import download_artifact

parser = argparse.ArgumentParser(
    description='Runs a bisect using prebuilt releases.')
parser.add_argument('--good')
parser.add_argument('--bad')
parser.add_argument('--token', required=True)
parser.add_argument(
    '--test_builds', action=argparse.BooleanOptionalAction, default=True)
parser.add_argument('--list_releases', action='store_true')
parser.add_argument('--download_release')
parser.add_argument('--channel')
parser.add_argument('-c', '--command',
                    help='command to run on each step. \'%%zc\' is replaced with the path to the player, \'%%zq\' is the editor, and \'%%zl\' is the launcher')
parser.add_argument('--check_return_code',
                    action=argparse.BooleanOptionalAction, default=False)

args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
test_builds_dir = root_dir / '.tmp/test_builds'
memory = Memory(root_dir / '.tmp/bisect_builds', verbose=0)

gh = Github(args.token)
repo = gh.get_repo('ZQuestClassic/ZQuestClassic')

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


@dataclass
class Revision:
    tag: str  # also could be a hash
    commit_count: int
    workflow_run: Optional[WorkflowRun.WorkflowRun] = None


@memory.cache
def get_release_commit_count(tag: str):
    return int(subprocess.check_output(['git', 'rev-list', '--count', tag], encoding='utf-8'))


@memory.cache
def has_release_package(tag: str):
    try:
        url = get_release_package_url(tag)
        return bool(url)
    except:
        return False


@memory.cache
def get_workflow_run_artifact_names(run_id: int):
    run = repo.get_workflow_run(run_id)
    return list(run.get_artifacts())


def get_releases():
    revisions: List[Revision] = []

    # TODO maybe use: git tag --merged=main 'nightly*' '2.55-alpha-???' --sort=committerdate
    tags = subprocess.check_output(
        ['git', '-P', 'tag', '--sort=committerdate', '--merged=main'], encoding='utf-8').splitlines()

    for tag in tags:
        commit_count = get_release_commit_count(tag)
        if channel == 'windows' and commit_count < 6252:
            # Every release after this commit will have binaries, but before only some do.
            if not has_release_package(tag):
                continue
        if channel == 'mac' and commit_count < 6384:
            continue
        if channel == 'linux' and commit_count < 7404:
            continue
        revisions.append(Revision(tag, commit_count))

    return revisions


def get_test_builds(releases: List[Revision]):
    revisions: List[Revision] = []

    good_commit_count = get_release_commit_count(args.good)
    bad_commit_count = get_release_commit_count(args.bad)
    min_commit_count = min(good_commit_count, bad_commit_count)
    max_commit_count = max(good_commit_count, bad_commit_count)

    ci_workflow = repo.get_workflow('ci.yml')
    main_runs = ci_workflow.get_runs(branch='main', status='success')
    for run in main_runs:
        commit_count = get_release_commit_count(run.head_sha)
        if commit_count < min_commit_count:
            break
        if commit_count > max_commit_count:
            continue
        if any(r for r in releases if r.commit_count == commit_count):
            continue
        if any(r for r in revisions if r.commit_count == commit_count):
            continue

        artifacts = get_workflow_run_artifact_names(run.id)
        if not artifacts or any(a for a in artifacts if a.expired):
            continue

        revisions.append(
            Revision(run.head_sha, commit_count, workflow_run=run))

    revisions.sort(key=lambda x: x.commit_count)

    return revisions


def get_revisions(use_test_builds: bool):
    revisions = get_releases()

    if use_test_builds:
        revisions.extend(get_test_builds(revisions))
        revisions.sort(key=lambda x: x.commit_count)

    return revisions


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
    if dest.exists() and list(dest.glob('*')):
        return dest

    dest.mkdir(parents=True, exist_ok=True)
    print(f'downloading release {tag}')

    r = requests.get(url)
    if channel == 'mac':
        (dest / 'ZQuestClassic.dmg').write_bytes(r.content)
        subprocess.check_call(['hdiutil', 'attach', '-mountpoint',
                              str(dest / 'zc-mounted'), str(dest / 'ZQuestClassic.dmg')], stdout=subprocess.DEVNULL)
        zc_app_path = next((dest / 'zc-mounted').glob('*.app'))
        shutil.copytree(zc_app_path, dest / zc_app_path.name)
        subprocess.check_call(['hdiutil', 'unmount', str(
            dest / 'zc-mounted')], stdout=subprocess.DEVNULL)
        (dest / 'ZQuestClassic.dmg').unlink()
    elif url.endswith('.tar.gz'):
        tf = tarfile.open(fileobj=io.BytesIO(r.content), mode='r')
        tf.extractall(dest, filter='data')
        tf.close()
    else:
        zip = zipfile.ZipFile(io.BytesIO(r.content))
        zip.extractall(dest)
        zip.close()

    print(f'finished downloading {tag}')
    return dest


# TODO: share this code from common.
# See:
# - https://nightly.link/
# - https://github.com/actions/upload-artifact/issues/51
def download_artifact(gh, repo, artifact, dest):
    auth_header = gh._Github__requester._Requester__authorizationHeader
    if auth_header == None:
        url = f'https://nightly.link/{repo}/actions/artifacts/{artifact.id}.zip'
        r = requests.get(url)
    else:
        url = artifact.archive_download_url
        r = requests.get(url, headers={'Authorization': auth_header})

    zip = zipfile.ZipFile(io.BytesIO(r.content))
    zip.extractall(dest)
    zip.close()


def download_test_build(workflow_run: WorkflowRun):
    dest: Path = test_builds_dir / workflow_run.head_sha
    if dest.exists() and list(dest.glob('*')):
        return dest

    dest.mkdir(parents=True, exist_ok=True)
    print(f'downloading test build {workflow_run.head_sha}')

    found_artifact = None
    for artifact in workflow_run.get_artifacts():
        name = artifact.name
        if channel == 'mac' and 'macos' in name:
            found_artifact = artifact
        elif channel == 'windows' and 'windows' in name and 'x64' in name:
            found_artifact = artifact
        elif channel == 'linux' and ('ubuntu' in name or 'linux' in name):
            found_artifact = artifact
        if found_artifact:
            break

    if not found_artifact:
        raise Exception(
            f'could not find artifact for workflow run {workflow_run.id}')

    download_dir = tempfile.TemporaryDirectory()
    download_artifact(gh, 'ZQuestClassic/ZQuestClassic',
                      found_artifact, download_dir.name)
    # Build artifacts have a single file, which is an archive.
    archive_path = next(Path(download_dir.name).glob('*'))

    if channel == 'mac':
        dmg_path = archive_path
        subprocess.check_call(['hdiutil', 'attach', '-mountpoint',
                              str(dest / 'zc-mounted'), str(dmg_path)], stdout=subprocess.DEVNULL)
        zc_app_path = next((dest / 'zc-mounted').glob('*.app'))
        shutil.copytree(zc_app_path, dest / zc_app_path.name)
        subprocess.check_call(['hdiutil', 'unmount', str(
            dest / 'zc-mounted')], stdout=subprocess.DEVNULL)
        (dest / 'ZQuestClassic.dmg').unlink(missing_ok=True)
    elif archive_path.suffix.endswith('.tar.gz'):
        tf = tarfile.open(name=archive_path, mode='r')
        tf.extractall(dest, filter='data')
        tf.close()
    else:
        zip = zipfile.ZipFile(archive_path)
        zip.extractall(dest)
        zip.close()

    return dest


def find_path(dir: Path, one_of: List[str]):
    for p in one_of:
        if (dir / p).exists():
            return dir / p

    raise Exception(f'could not find one of {one_of} in {dir}')


def get_revision_binaries(revision: Revision):
    if revision.workflow_run:
        dir = download_test_build(revision.workflow_run)
    else:
        dir = download_release(revision.tag)

    binaries = {'dir': dir}
    if channel == 'mac':
        zc_app_path = next(dir.glob('*.app'))
        binaries['zc'] = find_path(zc_app_path / 'Contents/Resources', ['zplayer', 'zelda'])
        binaries['zq'] = find_path(zc_app_path / 'Contents/Resources', ['zeditor', 'zquest'])
        binaries['zl'] = zc_app_path / 'Contents/MacOS/zlauncher'
    elif channel == 'windows':
        binaries['zc'] = find_path(dir, ['zplayer.exe', 'zelda.exe'])
        binaries['zq'] = find_path(dir, ['zeditor.exe', 'zquest.exe'])
        binaries['zl'] = dir / 'zlauncher.exe'
    elif channel == 'linux':
        binaries['zc'] = find_path(dir, ['zplayer', 'zelda'])
        binaries['zq'] = find_path(dir, ['zeditor', 'zquest'])
        binaries['zl'] = dir / 'zlauncher'

    return binaries


def AskIsGoodBuild():
    while True:
        prompt = ('Revision is '
                  '[(g)ood/(b)ad/(u)nknown/(q)uit]: ')
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
    revs = revisions[lower_rev:upper_rev+1]
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
            'roughly %d steps left.' % (revs[lower].tag, min_str, revs[upper].tag,
                                        max_str, int(upper - lower).bit_length()))

        lower_tag = revs[lower].tag
        upper_tag = revs[upper].tag
        print(
            f'changelog of current range: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}')

        print(f'checking {rev.tag}')
        binaries = get_revision_binaries(rev)

        down_pivot = int((pivot - lower) / 2) + lower
        up_pivot = int((upper - pivot) / 2) + pivot

        if args.command:
            cmd = args.command
            cmd = cmd.replace('%zc', f'"{binaries["zc"]}"')
            cmd = cmd.replace('%zq', f'"{binaries["zq"]}"')
            cmd = cmd.replace('%zl', f'"{binaries["zl"]}"')
            print(f'running command: {cmd}')
            p = subprocess.Popen(cmd, cwd=binaries['dir'], shell=system != 'Windows')
            if args.check_return_code:
                retcode = p.communicate()
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
        if (answer == 'g' and good_rev < bad_rev) or (answer == 'b' and bad_rev < good_rev):
            lower = pivot
            pivot = up_pivot
        elif (answer == 'b' and good_rev < bad_rev) or (answer == 'g' and bad_rev < good_rev):
            upper = pivot
            pivot = down_pivot
        elif answer == 'u':
            skipped.append(pivot)

        rev = revs[pivot]

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
        print(state, revs[i].tag)
    print()

    lower_tag = revs[lower].tag
    upper_tag = revs[upper].tag
    if good_rev > bad_rev:
        print(DONE_MESSAGE_GOOD_MAX % (lower_tag, upper_tag))
    else:
        print(DONE_MESSAGE_GOOD_MIN % (lower_tag, upper_tag))

    print(
        f'changelog: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}')


if args.download_release:
    download_release(args.download_release)
    exit(0)

if args.list_releases:
    releases = get_releases()
    for release in releases:
        print(f'@{release.commit_count} {release.tag}')
    exit(0)

revisions = get_revisions(args.test_builds)
run_bisect(revisions)
