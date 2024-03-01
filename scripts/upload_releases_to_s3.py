# Uploads GitHub releases to an s3 bucket.

from github import Github
from pathlib import Path
import argparse
import os
import re
import requests
import subprocess

BUCKET = 'zc-archives'
BUCKET_URL = 'https://zc-archives.nyc3.cdn.digitaloceanspaces.com'

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/gh-releases'
tmp_dir.mkdir(exist_ok=True, parents=True)

parser = argparse.ArgumentParser(description='Upload builds from GitHub to s3 bucket.')
parser.add_argument('--token')
args = parser.parse_args()


def get_commits_in_bucket(bucket_url: str):
    commitishes = set()

    def get_download_urls_impl(marker: str):
        url = f'{bucket_url}?max-keys=1000'
        if marker:
            url += f'&marker={marker}'
        archives_xml = requests.get(url).text

        keys = re.compile(r'<Key>(.*?)</Key>').findall(archives_xml)
        for key in keys:
            commitish, filename = key.split('/', 2)
            commitishes.add(commitish)

        match = re.compile(r'<NextMarker>(.*?)</NextMarker>').search(archives_xml)
        if match:
            return match.group(1)
        return None

    marker = ''
    while True:
        marker = get_download_urls_impl(marker)
        if not marker:
            break

    return list(commitishes)


gh = Github(args.token)
repo = gh.get_repo('ZQuestClassic/ZQuestClassic')
release = repo.get_release(repo)
releases_already_uploaded = get_commits_in_bucket(BUCKET_URL)

for release in repo.get_releases():
    if release.tag_name in releases_already_uploaded:
        continue
    if release.created_at.year < 2024:
        continue

    print(f'uploading {release.tag_name}')
    rls_dir = tmp_dir / release.tag_name
    rls_dir.mkdir(exist_ok=True, parents=True)
    for asset in release.get_assets():
        if 'web' in asset.name:
            continue

        asset_path = rls_dir / asset.name.replace(f'{release.tag_name}-', '')
        if not asset_path.exists():
            asset_path.write_bytes(requests.get(asset.browser_download_url).content)

    subprocess.check_call(['s3cmd', 'sync', '--acl-public', rls_dir, f's3://{BUCKET}/'])
