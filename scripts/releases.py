# Copys GitHub releases from one repo to another.

import os
import requests
import subprocess
from pathlib import Path
from github import Github

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/gh-releases'
tmp_dir.mkdir(exist_ok=True, parents=True)

gh = Github(os.environ['GH_PAT'])
repo = gh.get_repo('ZQuestClassic/ZQuestClassic')
repo_new = gh.get_repo('ZQuestClassic/ZQuestClassic-mirror')

# all_releases = [r.tag_name for r in repo.get_releases()]
# for tag_name in all_releases:
#     date = subprocess.check_output([
#         'git',
#         'log', '-1',
#         '--date=format-local:%Y-%m-%d %H:%M',
#         '--format=%ad',
#         tag_name,
#     ], encoding='utf-8').strip()
#     command = f'GIT_COMMITTER_DATE="{date}" git tag -a -f -m {tag_name} {tag_name} {tag_name}'
#     print(command)
# exit(0)

current_releases = [r.tag_name for r in repo_new.get_releases()]

# print(set(current_releases) - set(all_releases))
# print(set(all_releases) - set(current_releases))
# exit(0)

for r in repo.get_releases():
    print(r.tag_name)

    if r.tag_name not in current_releases:
        r_new = repo_new.create_git_release(r.tag_name, r.title, r.body, r.draft, r.prerelease)
        existing_assets = []
    else:
        r_new = repo_new.get_release(r.tag_name)
        existing_assets = [a.name for a in r_new.assets]

    for asset in r.assets:
        if asset.name in existing_assets:
            continue

        print(' ', asset.name)
        ext = ''.join(Path(asset.browser_download_url.split('/')[-1]).suffixes)
        asset_path = tmp_dir / f'{asset.id}{ext}'
        if not asset_path.exists():
            asset_path.write_bytes(requests.get(asset.browser_download_url).content)

        r_new.upload_asset(str(asset_path), asset.label or '', asset.content_type, asset.name)
