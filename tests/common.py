import platform
import os
import io
import requests
import zipfile
from pathlib import Path
from github import Github

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))


def infer_gha_platform():
    system = platform.system()
    if system == 'Windows':
        runs_on = 'windows-2022'
        arch = 'x64' if platform.architecture()[0] == '64bit' else 'win32'
    elif system == 'Darwin':
        runs_on = 'macos-12'
        arch = 'intel'
    else:
        raise Exception(f'unexpected platform: {system}')
    return runs_on, arch


# See:
# - https://nightly.link/
# - https://github.com/actions/upload-artifact/issues/51
def download_artifact(repo, artifact, dest):
    url = f'https://nightly.link/{repo}/actions/artifacts/{artifact.id}.zip'
    r = requests.get(url)
    zip = zipfile.ZipFile(io.BytesIO(r.content))
    zip.extractall(dest)
    zip.close()


def get_gha_artifacts(gh: Github, repo_str: str, run_id: int) -> Path:
    gha_cache_dir = script_dir / '.gha-cache-dir'
    workflow_run_dir = gha_cache_dir / str(run_id)
    if workflow_run_dir.exists() and (workflow_run_dir / '.complete').exists():
        return workflow_run_dir

    repo = gh.get_repo(repo_str)
    run = repo.get_workflow_run(run_id)
    replay_artifacts = [
        r for r in run.get_artifacts() if r.name.startswith('replays-')]
    if not replay_artifacts:
        raise Exception(f'no replay artifacts found for run {run_id}')

    workflow_run_dir.mkdir(exist_ok=True, parents=True)

    test_results_paths = []
    for artifact in replay_artifacts:
        artifact_dir = workflow_run_dir / str(artifact.name)
        if not artifact_dir.exists():
            print(f'downloading artifact: {artifact.name}')
            download_artifact(repo_str, artifact, artifact_dir)
        test_results_path = next(artifact_dir.glob('test_results.json'), None)
        if test_results_path:
            test_results_paths.append(test_results_path)
        else:
            print(f'missing test_results.json for {artifact.name}')

    if not test_results_paths:
        raise Exception(f'run {run_id}: found no test_results.json files')

    (workflow_run_dir / '.complete').write_text('')
    return workflow_run_dir
