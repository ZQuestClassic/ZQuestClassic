# Downloads artifacts from GitHub Actions.

import argparse
import os

from pathlib import Path

from github import Github

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

os.sys.path.append(str((root_dir / 'scripts').absolute()))
from github_helpers import get_gha_artifacts

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', default='ZQuestClassic/ZQuestClassic')
    parser.add_argument('--token', required=True)
    parser.add_argument('--workflow_run', type=int, required=True)

    args = parser.parse_args()
    gh = Github(args.token)

    path = get_gha_artifacts(gh, args.repo, args.workflow_run)
    print(f'saved to {path}')
