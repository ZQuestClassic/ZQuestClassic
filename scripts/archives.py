# To download a specific build:
#   python scripts/archives.py download 2.55-alpha-109
#
# For more, see --help

import argparse
import functools
import io
import os
import re
import shutil
import subprocess
import tarfile
import textwrap
import zipfile

from dataclasses import dataclass
from pathlib import Path
from typing import Literal

import common
import git_helpers
import requests
import s3_helpers

from github import Github
from joblib import Memory


def _get_historical():
    import build_historical

    return build_historical


script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
archives_dir = root_dir / '.tmp/archives'
memory = Memory(root_dir / '.tmp/memory_archives', verbose=0)
bucket_url = 'https://zc-archives.nyc3.cdn.digitaloceanspaces.com'


@dataclass
class Revision:
    tag: str  # also could be a full sha hash
    commit_count: int
    type: Literal['release', 'test', 'local']
    is_local_build = False

    def dir(self):
        return archives_dir / self.type / self.tag

    def binaries(self, release_platform: str):
        if self.is_local_build:
            try:
                dir = _get_historical().build_locally(self, release_platform)
            except KeyboardInterrupt:
                exit(1)
            except Exception as e:
                print(e)
                return None
        else:
            dir = _download(self, release_platform)

        return create_binary_paths(dir, release_platform)


# For a given branch, assign a number (commit count) to every commit. Commits are
# numbered in order from oldest to newest - the older the commit, the smaller
# the commit count.
@functools.cache
def get_commit_counts(branch: str):
    commit_counts = {}
    shas = subprocess.check_output(
        ['git', 'log', '--format=format:%H', '--reverse', '--first-parent', branch],
        encoding='utf-8',
    ).splitlines()
    for i, sha in enumerate(shas):
        commit_counts[sha] = i
    return commit_counts


def get_release_commit_count(branch: str, sha: str):
    commit_counts = get_commit_counts(branch)
    if sha not in commit_counts:
        raise Exception(f'no commit count found: {sha}')
    return commit_counts[sha]


@memory.cache
def get_release_commit_count_of_tag(branch: str, tag: str):
    sha = git_helpers.rev_parse(tag)
    try:
        return get_release_commit_count(branch, sha)
    except:
        pass

    sha = git_helpers.tag_to_sha_on_branch(tag, branch)
    try:
        return get_release_commit_count(branch, sha)
    except:
        pass

    return None


def get_release_commit_count_of_tag_or_raise(branch: str, tag: str):
    commit_count = get_release_commit_count_of_tag(branch, tag)
    if commit_count == None:
        raise Exception(f'can not find {branch} commit for tag: {tag}')
    return commit_count


@memory.cache
def has_release_package(tag: str, release_platform: str):
    try:
        url = get_gh_release_package_url(tag, release_platform)
        return bool(url)
    except:
        return False


def revision_count_supports_platform(
    revision_count: int, platform_str: str, branch: str
):
    if platform_str == 'windows':
        return True
    if (
        platform_str == 'mac'
        and revision_count
        < get_release_commit_count_of_tag_or_raise(branch, '2.55-alpha-108')
    ):
        return False
    if (
        platform_str == 'linux'
        and revision_count
        < get_release_commit_count_of_tag_or_raise(branch, '2.55-alpha-112')
    ):
        return False
    return True


@functools.cache
def get_download_urls(release_platform: str):
    keys = s3_helpers.list_bucket(bucket_url)

    keys_by_commitish = {}
    for key in keys:
        commitish, filename = key.split('/', 2)
        if commitish not in keys_by_commitish:
            keys_by_commitish[commitish] = []
        keys_by_commitish[commitish].append(key)

    urls_by_commitish = {}
    for commitish, keys in keys_by_commitish.items():
        key = None
        if release_platform == 'windows':
            if len(keys) == 1:
                key = keys[0]
            else:
                keys = [k for k in keys if 'windows' in k]
                if len(keys) == 1:
                    key = keys[0]
                else:
                    key = next((k for k in keys if 'x64' in k), None) or next(
                        (k for k in keys if 'x86' in k), None
                    )
        elif release_platform == 'mac':
            key = next((k for k in keys if k.endswith('.dmg')), None)
        elif release_platform == 'linux':
            key = next((k for k in keys if k.startswith('linux')), None)
        if key:
            urls_by_commitish[commitish] = f'{bucket_url}/{key}'

    return urls_by_commitish


def get_local_builds(branch: str, revisions=[]):
    commit_counts = get_commit_counts(branch)
    for sha, commit_count in commit_counts.items():
        if not next((r for r in revisions if r.commit_count == commit_count), None):
            r = Revision(sha, commit_count, type='local')
            r.is_local_build = True
            revisions.append(r)

    return revisions


def get_revisions(
    release_platform: str,
    channel: str,
    include_test_builds=True,
    may_build_locally=False,
):
    revisions: list[Revision] = []

    if channel == 'main':
        branch = 'main'
        tags = subprocess.check_output(
            [
                'git',
                '-P',
                'tag',
                '--merged=main',
                '--sort=committerdate',
                'nightly*',
                '2.55-alpha-???',
                '3.*',
            ],
            encoding='utf-8',
        ).splitlines()
    elif channel == '2.55':
        branch = 'releases/2.55'
        tags = subprocess.check_output(
            [
                'git',
                '-P',
                'tag',
                '--merged=releases/2.55',
                '--sort=committerdate',
                'nightly*',
                '2.55-alpha-???',
                '2.55.*',
            ],
            encoding='utf-8',
        ).splitlines()
    else:
        raise Exception(f'unknown channel: {channel}')

    ignore_commit_counts = []

    for tag in tags:
        commit_count = get_release_commit_count_of_tag(branch, tag)

        if commit_count != None and not revision_count_supports_platform(
            commit_count, release_platform, branch
        ):
            ignore_commit_counts.append(commit_count)
            continue

        # Every release after this one will have binaries, but before only some do.
        if commit_count != None and commit_count < get_release_commit_count_of_tag(
            branch, '2.55-alpha-107'
        ):
            if not has_release_package(tag, release_platform):
                ignore_commit_counts.append(commit_count)
                continue

        if commit_count == None:
            commit_count = -1

        revisions.append(Revision(tag, commit_count, type='release'))

    if include_test_builds:
        urls_by_commitish = get_download_urls(release_platform)
        for commitish in urls_by_commitish.keys():
            if any(r for r in revisions if r.tag == commitish):
                continue

            commit_count = get_release_commit_count_of_tag(branch, commitish)
            if commit_count == None or commit_count in ignore_commit_counts:
                continue

            revisions.append(Revision(commitish, commit_count, type='test'))

    if may_build_locally:
        revisions = get_local_builds(branch, revisions)
        revisions = [r for r in revisions if not _get_historical().local_build_error(r)]

    revisions = [r for r in revisions if r.commit_count != -1]
    revisions.sort(key=lambda x: x.commit_count)
    return revisions


def create_binary_paths(dir: Path, release_platform: str):
    binaries = {'dir': dir}

    if release_platform == 'mac':
        zc_app_path = next(dir.glob('*.app'))
        binaries['zc'] = common.find_path(
            zc_app_path / 'Contents/Resources', ['zplayer', 'zelda']
        )
        binaries['zq'] = common.find_path(
            zc_app_path / 'Contents/Resources', ['zeditor', 'zquest']
        )
        binaries['zl'] = zc_app_path / 'Contents/Resources/zlauncher'
        binaries['zs'] = common.find_path(
            zc_app_path / 'Contents/Resources', ['zscript']
        )
    elif release_platform == 'windows':
        binaries['zc'] = common.find_path(dir, ['zplayer.exe', 'zelda.exe'])
        binaries['zq'] = common.find_path(dir, ['zeditor.exe', 'zquest.exe'])
        binaries['zl'] = dir / 'zlauncher.exe'
        binaries['zs'] = common.find_path(dir, ['zscript.exe'])
    elif release_platform == 'linux':
        binaries['zc'] = common.find_path(dir, ['bin/zplayer', 'zplayer', 'zelda'])
        binaries['zq'] = common.find_path(dir, ['bin/zeditor', 'zeditor', 'zquest'])
        binaries['zl'] = common.find_path(dir, ['bin/zlauncher', 'zlauncher'])
        binaries['zs'] = common.find_path(dir, ['bin/zscript', 'zscript'])

    return binaries


def get_repo():
    token = os.environ.get('GH_PAT', None)
    if not token:
        raise Exception('token required - set GH_PAT env variable')

    gh = Github(token)
    return gh.get_repo('ZQuestClassic/ZQuestClassic')


# TODO: remove this when uploading releases to s3 bucket is automated.
def get_gh_release_package_url(tag: str, release_platform: str):
    repo = get_repo()
    release = repo.get_release(tag)
    assets = list(release.get_assets())

    asset = None
    if release_platform == 'mac':
        asset = next((asset for asset in assets if asset.name.endswith('.dmg')), None)
    elif release_platform == 'windows':
        if len(assets) == 1:
            asset = assets[0]
        else:
            assets = [asset for asset in assets if 'windows' in asset.name]
            asset = next(
                (asset for asset in assets if 'x64' in asset.name), None
            ) or next((asset for asset in assets if 'x86' in asset.name), None)
    elif release_platform == 'linux':
        asset = next(
            asset
            for asset in assets
            if asset.name.endswith('.tar.gz') or asset.name.endswith('.tgz')
        )

    if not asset:
        raise Exception(f'could not find package url for {tag}')

    return asset.browser_download_url


def download(tag_or_sha: Revision, release_platform: str):
    if len(tag_or_sha) == 40:
        type = 'test'
    else:
        type = 'release'
    revision = Revision(tag_or_sha, -1, type)
    return _download(revision, release_platform)


def _download(revision: Revision, release_platform: str):
    dest = revision.dir()
    if dest.exists() and list(dest.glob('*')):
        return dest

    tag = revision.tag
    prefix = f'{bucket_url}/{tag}/'

    if release_platform == 'windows':
        urls = [
            f'{prefix}windows-x64.zip',
            f'{prefix}windows-x86.zip',
        ]
    elif release_platform == 'mac':
        urls = [
            f'{prefix}mac.dmg',
        ]
    elif release_platform == 'linux':
        urls = [
            f'{prefix}linux.tar.gz',
        ]
    else:
        raise Exception(f'unexpected release_platform: {release_platform}')

    found_url = None
    for url in urls:
        print(f'downloading {url}', file=os.sys.stderr)
        r = requests.get(url)
        if not r.ok:
            print(f'not found: {url}', file=os.sys.stderr)
            continue

        found_url = url
        break

    if not found_url:
        print(f'Not found on {bucket_url}, falling back to using the GitHub API')
        url = get_gh_release_package_url(revision.tag, release_platform)
        r = requests.get(url)

    dest.mkdir(parents=True, exist_ok=True)

    if release_platform == 'mac':
        (dest / 'ZQuestClassic.dmg').write_bytes(r.content)
        subprocess.check_call(
            [
                'hdiutil',
                'attach',
                '-mountpoint',
                str(dest / 'zc-mounted'),
                str(dest / 'ZQuestClassic.dmg'),
            ],
            stdout=subprocess.DEVNULL,
        )
        zc_app_path = next((dest / 'zc-mounted').glob('*.app'))
        shutil.copytree(zc_app_path, dest / zc_app_path.name)
        subprocess.check_call(
            ['hdiutil', 'unmount', str(dest / 'zc-mounted')], stdout=subprocess.DEVNULL
        )
        (dest / 'ZQuestClassic.dmg').unlink()
    elif url.endswith('.tar.gz'):
        tf = tarfile.open(fileobj=io.BytesIO(r.content), mode='r')
        tf.extractall(dest, filter='data')
        tf.close()
    else:
        zip = zipfile.ZipFile(io.BytesIO(r.content))
        zip.extractall(dest)
        zip.close()

    print(f'finished downloading {tag}', file=os.sys.stderr)
    return dest


class CLI:
    def __init__(self):
        parser = argparse.ArgumentParser(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            description='Download and run any release build. Also supports various builds inbetween releases',
            epilog=textwrap.dedent(
                '''
			Command template:
				- %zc is replaced with the path to the player
				- %zq is the editor
				- %zl is the launcher
				- %zs is the zscript parser
			
			Example command to open the player in test mode: %zc -test someqst.qst 0 118
			'''
            ),
        )
        subparsers = parser.add_subparsers(dest='command')

        base = argparse.ArgumentParser(add_help=False)
        base.add_argument('--platform', default=common.get_release_platform())
        base.add_argument('--channel', default='main')

        list_cmd = subparsers.add_parser('list', parents=[base])
        list_cmd.add_argument(
            '--test_builds',
            action=argparse.BooleanOptionalAction,
            default=True,
            help='Includes pre-built builds not associated with official releases',
        )

        download_cmd = subparsers.add_parser(
            'download',
            parents=[base],
            help='Downloads the given release (tag or commit sha) to .tmp/archives',
        )
        download_cmd.add_argument('tag_or_sha')

        run_cmd = subparsers.add_parser(
            'run',
            parents=[base],
            help='Runs a command for the given version (can be release tag or commit sha), downloading or building as necessary',
        )
        run_cmd.add_argument('tag_or_sha')
        run_cmd.add_argument('command_args', nargs=argparse.REMAINDER, default='%zl')

        backfill_cmd = subparsers.add_parser('backfill_local_builds')

        args = parser.parse_args()
        if not args.command:
            parser.print_help()
            exit(1)
        getattr(self, args.command)(args)

    def download(self, args):
        print(download(args.tag_or_sha, args.platform))

    def list(self, args):
        revisions = get_revisions(
            args.platform, args.channel, include_test_builds=args.test_builds
        )
        for revision in revisions:
            if revision.commit_count == -1:
                print(f'@? {revision.tag}')
            else:
                print(f'@{revision.commit_count} {revision.tag}')

    def run(self, args):
        binary_dir = download(args.tag_or_sha, args.platform)
        binaries = create_binary_paths(binary_dir, args.platform)
        command_args = args.command_args or ['%zl']
        p = common.run_zc_command(binaries, command_args)
        exit(p.wait())

    def backfill_local_builds(self, args):
        _get_historical().backfill(args.platform)


if __name__ == '__main__':
    CLI()
