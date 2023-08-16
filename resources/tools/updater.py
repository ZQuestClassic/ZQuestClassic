import json
import os
import io
import time
import platform
import subprocess
import shutil
import tarfile
import zipfile
from urllib.request import urlopen
from pathlib import Path
import argparse

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
resources_dir = script_dir.parent


parser = argparse.ArgumentParser(
	description='Download and unzip latest release from GitHub')
parser.add_argument('--channel', default='none', choices=['mac', 'linux', 'windows-x86', 'windows-x64', 'none'])
parser.add_argument('--repo', default='ArmageddonGames/ZQuestClassic')
parser.add_argument('--print-next-release', action='store_true')
parser.add_argument('--asset-url')
parser.add_argument('--cache', action='store_true')
parser.add_argument('--cache-folder', default=resources_dir / '.updater-cache')

args = parser.parse_args()

if 'ZC_UPDATER_CACHE_FOLDER' in os.environ:
	args.cache_folder = Path(os.environ['ZC_UPDATER_CACHE_FOLDER'])

if args.channel == 'none':
	system = platform.system()
	if system == 'Darwin':
		args.channel = 'mac'
	elif system == 'Windows':
		args.channel = 'windows-x64'
	elif system == 'Linux':
		args.channel = 'linux'

if args.repo == 'none':
	args.repo = 'ArmageddonGames/ZQuestClassic'


def get_release_json(url):
	print(f'calling gh api {url}')
	attempts = 0
	while attempts < 10:
		try:
			response = urlopen(url)
			return json.loads(response.read())
		except Exception as e:
			print(e)
			attempts += 1

			# Find when rate limit is gone.
			limits = json.loads(urlopen('https://api.github.com/rate_limit').read())
			limit = limits['resources']['core']
			print(f'gh rate limit: {json.dumps(limit, indent=2)}')

			time_until_reset = limit['reset'] - time.time()
			if time_until_reset < 0 or limit['remaining'] > 0:
				# Weird but ok, let's try again.
				continue
			if time_until_reset > 10:
				print(f'GitHub API reached. Try again in {time_until_reset / 60} minutes')
				exit(1)

			print(f'gh api rate limit hit, waiting {time_until_reset} seconds')
			time.sleep(time_until_reset)

	raise Exception('timed out')


def download_release(url: str, dest: Path):
	print(f'downloading {url} to {dest}')

	if args.cache:
		url_filename = url.split('/')[-1]
		cache_filename = f'{args.repo.replace("/", "-")}-{url_filename}'
		cache_folder = Path(args.cache_folder)
		cache_folder.mkdir(parents=True, exist_ok=True)
		cache_path = cache_folder / cache_filename
		print(f'cache_path {cache_path}')

	content = None
	if args.cache and cache_path.exists():
		print('restored from cache')
		content = io.BytesIO(cache_path.read_bytes())
	else:
		with urlopen(url) as resp:
			content = io.BytesIO(resp.read())

		if args.cache:
			print('saved to cache')
			cache_path.write_bytes(content.getvalue())

	if args.channel == 'mac':
		# In case it already exists somehow.
		shutil.rmtree(dest / 'ZeldaClassic.app', ignore_errors=True)

		(dest / 'ZeldaClassic.dmg').write_bytes(content.getvalue())
		subprocess.check_call(['hdiutil', 'attach', '-mountpoint',
								str(dest / 'zc-mounted'), str(dest / 'ZeldaClassic.dmg')], stdout=subprocess.DEVNULL)
		shutil.copytree(dest / 'zc-mounted/ZeldaClassic.app',
						dest / 'ZeldaClassic.app')
		subprocess.check_call(['hdiutil', 'unmount', str(
			dest / 'zc-mounted')], stdout=subprocess.DEVNULL)
		(dest / 'ZeldaClassic.dmg').unlink()

		app_bundle_path = next(
			(p for p in dest.parents if p.name.endswith('.app')), None)
		if app_bundle_path:
			print(f'app_bundle_path {app_bundle_path}')
			shutil.copytree(dest / 'ZeldaClassic.app',
							app_bundle_path, dirs_exist_ok=True)
		else:
			raise Exception('only support updating an app bundle')

		shutil.rmtree(dest / 'ZeldaClassic.app')
	elif url.endswith('.tar.gz'):
		tf = tarfile.open(fileobj=content, mode='r')
		# Act like `install` by first unlinking existing files. This is necessary to prevent
		# errors when deleting binaries currently in use.
		# See https://stackoverflow.com/a/7779703/2788187
		for name in tf.getnames():
			if (dest / name).is_file():
				os.unlink(dest / name)
		tf.extractall(dest, filter='data')
		tf.close()
	else:
		zip = zipfile.ZipFile(content)
		# Windows locks the filesystem of loaded binaries. We can rename the currently loaded binary files
		# so our news one can go to the right place. zapp.cpp will clean these up on startup.
		active_files_dir = resources_dir / '.updater-active-files'
		active_files_dir.mkdir(exist_ok=True)
		for name in zip.namelist():
			original = dest / name
			if original.is_file():
				(active_files_dir / name).parent.mkdir(exist_ok=True, parents=True)
				original.rename(active_files_dir / name)
		zip.extractall(dest)
		zip.close()

	# In CI only keep the latest release in cache.
	if args.cache and 'CI' in os.environ:
		for f in cache_folder.glob('*'):
			if f != cache_path:
				f.unlink()

	print('finished downloading release')


def get_next_release():
	data = get_release_json(f'https://api.github.com/repos/{args.repo}/releases')
	rls = data[0]
	tag_name = rls['tag_name']
	asset = next((a for a in rls['assets'] if args.channel in a['name']), None)
	asset_url = asset['browser_download_url']
	if not asset:
		print('no asset found matching this platform')
		exit(1)

	return (tag_name, asset_url)


if args.print_next_release:
	tag_name, asset_url = get_next_release()
	print(f'tag_name {tag_name}')
	print(f'asset_url {asset_url}')
	exit(0)


if args.channel == 'mac':
	print('updater current does not support mac')
	exit(1)

# Allow caller to provide the asset url, saving an API call.
if args.asset_url:
	asset_url = args.asset_url
else:
	tag_name, asset_url = get_next_release()

download_release(asset_url, resources_dir)
print('success!')
