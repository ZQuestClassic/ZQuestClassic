import platform
import os
import io
import json
from time import sleep
from dataclasses import dataclass
import dataclasses
from typing import List, Literal, Dict, Optional, Any
import requests
import zipfile
from pathlib import Path

# So that `PyGithub` is not necessary to simply run `run_replay_tests.py`.
try:
    from github import Github
except:
    Github = {}

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))


@dataclass
class RunResult:
    name: str
    directory: str
    success: bool = False
    duration: float = None
    fps: int = None
    failing_frame: int = None
    unexpected_gfx_frames: List[int] = None
    diff: str = None


@dataclass
class ReplayTestResults:
    runs_on: Literal['windows-2022', 'macos-12', 'ubuntu-22.04']
    arch: Literal['x64', 'win32', 'intel']
    ci: bool
    workflow_run_id: Optional[int]
    git_ref: Optional[str]
    zc_version: str
    time: str
    runs: List[List[RunResult]]

    def __post_init__(self):
        if self.runs and isinstance(self.runs[0][0], dict):
            deserialized = []
            for runs in self.runs:
                deserialized.append([RunResult(**run) for run in runs])
            self.runs = deserialized

    def to_json(self):
        as_dict = dataclasses.asdict(self)
        return json.dumps(as_dict, indent=2)


def infer_gha_platform():
    system = platform.system()
    if system == 'Windows':
        runs_on = 'windows-2022'
        arch = 'x64' if platform.architecture()[0] == '64bit' else 'win32'
    elif system == 'Darwin':
        runs_on = 'macos-12'
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
