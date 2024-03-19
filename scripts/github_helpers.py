import io
import os
import platform
import tarfile
import zipfile

from pathlib import Path
from time import sleep

import requests

from github import Github

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent


def infer_gha_platform():
    system = platform.system()
    if system == 'Windows':
        runs_on = 'windows-2022'
        arch = 'x64' if platform.architecture()[0] == '64bit' else 'win32'
    elif system == 'Darwin':
        runs_on = 'macos-13'
        arch = 'intel'
    elif system == 'Linux':
        runs_on = 'ubuntu-22.04'
        arch = 'x64'
    else:
        raise Exception(f'unexpected platform: {system}')
    return runs_on, arch


# See:
# - https://nightly.link/
# - https://github.com/actions/upload-artifact/issues/51
def _download_artifact(gh, repo, artifact, dest):
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


def extract_tars(dir: Path):
    for tar_path in dir.rglob('*.tar'):
        print(f'extracting from {tar_path}')
        extract_dir = tar_path.with_suffix('')
        if not extract_dir.exists():
            with tarfile.open(tar_path) as tar:
                tar.extractall(path=extract_dir)
            tar_path.unlink()


def get_gha_artifacts(gh: Github, repo_str: str, run_id: int) -> Path:
    if not isinstance(run_id, int):
        raise Exception('run_id must be an integer')

    gha_cache_dir = root_dir / '.tmp/gha_artifacts'
    workflow_run_dir = gha_cache_dir / str(run_id)
    if workflow_run_dir.exists() and (workflow_run_dir / '.complete').exists():
        return workflow_run_dir

    repo = gh.get_repo(repo_str)
    run = repo.get_workflow_run(run_id)
    replay_artifacts = [r for r in run.get_artifacts() if r.name.startswith('replays-')]
    if not replay_artifacts:
        raise Exception(f'no replay artifacts found for run {run_id}')

    workflow_run_dir.mkdir(exist_ok=True, parents=True)

    test_results_paths = []
    for artifact in replay_artifacts:
        artifact_dir = workflow_run_dir / str(artifact.name)
        if not artifact_dir.exists():
            print(f'downloading artifact: {artifact.name}')
            _download_artifact(gh, repo_str, artifact, artifact_dir)
            extract_tars(artifact_dir)
        test_results_path = next(artifact_dir.rglob('test_results.json'), None)
        if test_results_path:
            test_results_paths.append(test_results_path)
        else:
            print(f'missing test_results.json for {artifact.name}')

    if not test_results_paths:
        raise Exception(f'run {run_id}: found no test_results.json files')

    (workflow_run_dir / '.complete').write_text('')
    return workflow_run_dir


# It's possible that the artifacts are not available very shortly after a workflow finishes.
# Pretty rare, though.
def get_gha_artifacts_with_retry(gh: Github, repo_str: str, run_id: int) -> Path:
    NUM_TRIES = 5
    WAIT_TIME = 20
    for i in range(0, NUM_TRIES):
        try:
            if i != 0:
                print(f'failed to download artifacts, trying again in {WAIT_TIME}s')
                sleep(WAIT_TIME)
            return get_gha_artifacts(gh, repo_str, run_id)
        except Exception as e:
            if i == NUM_TRIES - 1:
                raise e
