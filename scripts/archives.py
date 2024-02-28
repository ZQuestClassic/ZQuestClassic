# To download a specific build:
#   python scripts/archives.py 2.55-alpha-109
#
# For more, see --help

from dataclasses import dataclass
from github import Github
from joblib import Memory
from pathlib import Path
from typing import List, Union, Literal
import argparse
import build_historical
import common
import functools
import git_helpers
import io
import os
import re
import requests
import shutil
import subprocess
import tarfile
import textwrap
import zipfile

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
archives_dir = root_dir / '.tmp/archives'
memory = Memory(root_dir / '.tmp/memory_archives', verbose=0)
bucket_url = 'https://zc-archives.nyc3.cdn.digitaloceanspaces.com'

@dataclass
class Revision:
	tag: str  # also could be a full sha hash
	commit_count: int
	type: Union[Literal['release'], Literal['test'], Literal['local']]
	is_local_build = False

	def dir(self):
		return archives_dir / self.type / self.tag

	def binaries(self, channel: str):
		if self.is_local_build:
			try:
				dir = build_historical.build_locally(self)
			except KeyboardInterrupt:
				exit(1)
			except:
				return None
		else:
			dir = _download(self, channel)

		return create_binary_paths(dir, channel)


@functools.cache
def get_commit_counts():
	commit_counts = {}
	shas = subprocess.check_output(
		['git', 'log', '--format=format:%H', '--reverse', '--first-parent'], encoding='utf-8').splitlines()
	for i, sha in enumerate(shas):
		commit_counts[sha] = i
	return commit_counts


def get_release_commit_count(sha: str):
	commit_counts = get_commit_counts()
	if sha not in commit_counts:
		raise Exception(f'no commit count found: {sha}')
	return commit_counts[sha]


@memory.cache
def get_release_commit_count_of_tag(tag: str):
	sha = git_helpers.rev_parse(tag)
	try:
		return get_release_commit_count(sha)
	except:
		pass

	sha = git_helpers.tag_to_sha_on_main(tag)
	try:
		return get_release_commit_count(sha)
	except:
		pass

	raise Exception(f'can not find main commit for tag: {tag}')


@memory.cache
def has_release_package(tag: str, channel: str):
	try:
		url = get_gh_release_package_url(tag, channel)
		return bool(url)
	except:
		return False


def revision_count_supports_platform(revision_count: int, platform_str: str):
	if platform_str == 'windows':
		return True
	if platform_str == 'mac' and revision_count < get_release_commit_count_of_tag('2.55-alpha-108'):
		return False
	if platform_str == 'linux' and revision_count < get_release_commit_count_of_tag('2.55-alpha-112'):
		return False
	return True


@functools.cache
def get_download_urls(channel: str):
	keys_by_commitish = {}
	def get_download_urls_impl(marker: str):
		url = f'{bucket_url}?max-keys=1000'
		if marker:
			url += f'&marker={marker}'
		archives_xml = requests.get(url).text

		keys = re.compile(r'<Key>(.*?)</Key>').findall(archives_xml)
		for key in keys:
			commitish, filename = key.split('/', 2)
			if commitish not in keys_by_commitish:
				keys_by_commitish[commitish] = []
			keys_by_commitish[commitish].append(key)

		match = re.compile(r'<NextMarker>(.*?)</NextMarker>').search(archives_xml)
		if match:
			return match.group(1)
		return None

	marker = ''
	while True:
		marker = get_download_urls_impl(marker)
		if not marker:
			break

	urls_by_commitish = {}
	for commitish, keys in keys_by_commitish.items():
		key = None
		if channel == 'windows':
			if len(keys) == 1:
				key = keys[0]
			else:
				keys = [k for k in keys if 'windows' in k]
				if len(keys) == 1:
					key = keys[0]
				else:
					key = next((k for k in keys if 'x64' in k), None) or \
						next((k for k in keys if 'x86' in k), None)
		elif channel == 'mac':
			key = next((k for k in keys if k.endswith('.dmg')), None)
		elif channel == 'linux':
			key = next((k for k in keys if k.startswith('linux')), None)
		if key:
			urls_by_commitish[commitish] = f'{bucket_url}/{key}'

	return urls_by_commitish


def get_local_builds(revisions = []):
	commit_counts = get_commit_counts()
	for sha, commit_count in commit_counts.items():
		if not next((r for r in revisions if r.commit_count == commit_count), None):
			r = Revision(sha, commit_count, type='local')
			r.is_local_build = True
			revisions.append(r)

	return revisions


def get_revisions(channel: str, include_test_builds = True, may_build_locally = False):
	revisions: List[Revision] = []

	# TODO maybe use: git tag --merged=main 'nightly*' '2.55-alpha-???' --sort=committerdate
	tags = subprocess.check_output(
		['git', '-P', 'tag', '--sort=committerdate'], encoding='utf-8').splitlines()

	for tag in tags:
		if not any(tag.startswith(pre) for pre in ['2.55', '3', 'nightly']):
			continue

		try:
			commit_count = get_release_commit_count_of_tag(tag)
		except:
			commit_count = -1

		if commit_count != -1 and not revision_count_supports_platform(commit_count, channel):
			continue

		# Every release after this one will have binaries, but before only some do.
		if commit_count != -1 and commit_count < get_release_commit_count_of_tag('2.55-alpha-107'):
			if not has_release_package(tag, channel):
				continue

		revisions.append(Revision(tag, commit_count, type='release'))

	if include_test_builds:
		urls_by_commitish = get_download_urls(channel)
		for commitish in urls_by_commitish.keys():
			if any(r for r in revisions if r.tag == commitish):
				continue

			commit_count = get_release_commit_count_of_tag(commitish)
			revisions.append(Revision(commitish, commit_count, type='test'))

	if may_build_locally:
		revisions = get_local_builds(revisions)
		revisions = [r for r in revisions if not build_historical.local_build_error(r)]

	revisions = [r for r in revisions if r.commit_count != -1]
	revisions.sort(key=lambda x: x.commit_count)
	return revisions


def create_binary_paths(dir: Path, channel: str):
	binaries = {'dir': dir}

	if channel == 'mac':
		zc_app_path = next(dir.glob('*.app'))
		binaries['zc'] = common.find_path(zc_app_path / 'Contents/Resources', ['zplayer', 'zelda'])
		binaries['zq'] = common.find_path(zc_app_path / 'Contents/Resources', ['zeditor', 'zquest'])
		binaries['zl'] = zc_app_path / 'Contents/MacOS/zlauncher'
	elif channel == 'windows':
		binaries['zc'] = common.find_path(dir, ['zplayer.exe', 'zelda.exe'])
		binaries['zq'] = common.find_path(dir, ['zeditor.exe', 'zquest.exe'])
		binaries['zl'] = dir / 'zlauncher.exe'
	elif channel == 'linux':
		binaries['zc'] = common.find_path(dir, ['zplayer', 'zelda'])
		binaries['zq'] = common.find_path(dir, ['zeditor', 'zquest'])
		binaries['zl'] = dir / 'zlauncher'

	return binaries


def get_repo():
	token = os.environ.get('GH_PAT', None)
	if not token:
		raise Exception('token required - set GH_PAT env variable')

	gh = Github(token)
	return gh.get_repo('ZQuestClassic/ZQuestClassic')


# TODO: remove this when uploading releases to s3 bucket is automated.
def get_gh_release_package_url(tag: str, channel: str):
	repo = get_repo()
	release = repo.get_release(tag)
	assets = list(release.get_assets())

	asset = None
	if channel == 'mac':
		asset = next((asset for asset in assets if asset.name.endswith('.dmg')), None)
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


def download(tag_or_sha: Revision, channel: str):
	if len(tag_or_sha) == 40:
		type = 'test'
	else:
		type = 'release'
	revision = Revision(tag_or_sha, -1, type)
	return _download(revision, channel)


def _download(revision: Revision, channel: str):
	dest = revision.dir()
	if dest.exists() and list(dest.glob('*')):
		return dest

	commitish = revision.tag
	if channel == 'windows':
		urls = [
			f'{bucket_url}/{commitish}/windows-x64.zip',
			f'{bucket_url}/{commitish}/windows-x86.zip',
		]
	elif channel == 'mac':
		urls = [
			f'{bucket_url}/{commitish}/mac.dmg',
		]
	elif channel == 'linux':
		urls = [
			f'{bucket_url}/{commitish}/linux.tar.gz',
		]
	else:
		raise Exception(f'unexpected channel: {channel}')

	found_url = None
	for url in urls:
		r = requests.get(url)
		if not r.ok:
			print(f'not found: {url}', file=os.sys.stderr)
			continue

		found_url = url
		break

	# Fall back to using the GitHub API.
	if not found_url:
		found_url = get_gh_release_package_url(revision.tag, channel)

	dest.mkdir(parents=True, exist_ok=True)
	print(f'downloading {found_url}', file=os.sys.stderr)

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

	print(f'finished downloading {commitish}', file=os.sys.stderr)
	return dest


class CLI:
	def __init__(self):
		parser = argparse.ArgumentParser(
			formatter_class=argparse.RawDescriptionHelpFormatter,
			description='Download and run any release build. Also supports various builds inbetween releases',
			epilog=textwrap.dedent('''
			Command template:
				- %zc is replaced with the path to the player
				- %zq is the editor
				- %zl is the launcher
			
			Example command to open the player in test mode: %zc -test someqst.qst 0 118
			'''))
		subparsers = parser.add_subparsers(dest='command')

		base = argparse.ArgumentParser(add_help=False)
		base.add_argument('--channel', default=common.get_channel())

		list_cmd = subparsers.add_parser('list', parents=[base])
		list_cmd.add_argument(
			'--test_builds', action=argparse.BooleanOptionalAction, default=True,
			help='Includes pre-built builds not associated with official releases')
		list_cmd.add_argument('command_args', nargs=argparse.REMAINDER)

		download_cmd = subparsers.add_parser('download', parents=[base], help='Downloads the given release (tag or commit sha) to .tmp/archives')
		download_cmd.add_argument('tag_or_sha')

		run_cmd = subparsers.add_parser('run', parents=[base], help='Runs a command for the given version (can be release tag or commit sha), downloading or building as necessary')
		run_cmd.add_argument('tag_or_sha')
		run_cmd.add_argument('command_args', nargs=argparse.REMAINDER)

		backfill_cmd = subparsers.add_parser('backfill_local_builds')

		args = parser.parse_args()
		if not args.command:
			parser.print_help()
			exit(1)
		getattr(self, args.command)(args)

	def download(self, args):
		print(download(args.tag_or_sha, args.channel))

	def list(self, args):
		revisions = get_revisions(args.channel, include_test_builds=args.test_builds)
		for revision in revisions:
			if revision.commit_count == -1:
				print(f'@? {revision.tag}')
			else:
				print(f'@{revision.commit_count} {revision.tag}')

	def run(self, args):
		revisions = get_revisions(args.channel, include_test_builds=args.test_builds, may_build_locally=True)
		revision = next(r for r in revisions if r.tag == args.tag_or_sha)
		binaries = revision.binaries(args.channel)
		if not binaries:
			raise Exception('failed to find revision')

		p = common.run_zc_command(binaries, ' '.join(args.command_args))
		exit(p.wait())

	def backfill_local_builds(self, args):
		build_historical.backfill(args.channel)


if __name__ == '__main__':
	CLI()
