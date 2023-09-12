# Downloads artifacts from GitHub Actions.

import argparse
from github import Github
from common import get_gha_artifacts

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', default='ZQuestClassic/ZQuestClassic')
    parser.add_argument('--token', required=True)
    parser.add_argument('--workflow_run', type=int, required=True)

    args = parser.parse_args()
    gh = Github(args.token)

    path = get_gha_artifacts(gh, args.repo, args.workflow_run)
    print(f'saved to {path}')
