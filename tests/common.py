import difflib
import platform
import os
import io
import json
import subprocess
import shutil
import tarfile
from time import sleep
from dataclasses import dataclass, field
import dataclasses
from typing import List, Literal, Tuple, Optional, Any
import requests
import unittest
import zipfile
from pathlib import Path
from github import Github

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
test_builds_dir = root_dir / '.tmp/test_builds'


@dataclass
class RunResult:
    name: str
    directory: str
    success: bool = False
    stopped: bool = False
    exit_code: int = None
    duration: float = None
    fps: int = None
    frame: int = None
    num_frames: int = None
    failing_frame: int = None
    unexpected_gfx_frames: List[int] = None
    unexpected_gfx_segments: List[Tuple[int, int]] = None
    unexpected_gfx_segments_limited: List[Tuple[int, int]] = None
    exceptions: List[str] = field(default_factory=list) 
    # Only for compare report.
    snapshots: List[Any] = None


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
    # Only for compare report.
    label: str = None

    def __post_init__(self):
        if self.runs and isinstance(self.runs[0][0], dict):
            deserialized = []
            for runs in self.runs:
                for run in runs:
                    # Old property, will error when creating dataclass.
                    run.pop('diff', None)
                deserialized.append([RunResult(**run) for run in runs])
            self.runs = deserialized

    def to_json(self, indent=2):
        as_dict = dataclasses.asdict(self)
        return json.dumps(as_dict, indent=indent)


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
def download_artifact(gh, repo, artifact, dest):
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
            download_artifact(gh, repo_str, artifact, artifact_dir)
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

# TODO: remove copy of these methods in bisect_builds.py
def get_release_package_url(gh: Github, repo_str: str, channel: str, tag: str):
    repo = gh.get_repo(repo_str)
    release = repo.get_release(tag)
    assets = list(release.get_assets())

    asset = None
    if channel == 'mac':
        asset = next(asset for asset in assets if asset.name.endswith('.dmg'))
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


def maybe_get_downloaded_revision(tag: str) -> Optional[Path]:
    dir = None
    if (releases_dir / tag).exists():
        dir = releases_dir / tag
    elif (test_builds_dir / tag).exists():
        dir = test_builds_dir / tag
    if dir and list(dir.glob('*')):
        return dir
    return None


def download_release(gh: Github, repo_str: str, channel: str, tag: str):
    dest = releases_dir / tag
    if dest.exists() and list(dest.glob('*')):
        return dest

    dest.mkdir(parents=True, exist_ok=True)
    print(f'downloading release {tag}')

    url = get_release_package_url(gh, repo_str, channel, tag)
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

    print(f'finished downloading {tag}')
    return dest

class ZCTestCase(unittest.TestCase):
    def expect_snapshot(self, expected_path: Path, actual: str, update: bool):
        expected = None
        if expected_path.exists():
            expected = expected_path.read_text()

        if update:
            if expected != actual:
                print(f'updating snapshot {expected_path.name}')
                expected_path.parent.mkdir(parents=True, exist_ok=True)
                expected_path.write_text(actual)
        else:
            if expected == None:
                expected_path.parent.mkdir(parents=True, exist_ok=True)
                expected_path.write_text(actual)
            else:
                if expected != actual:
                    diff = difflib.context_diff(expected.splitlines(keepends=True), actual.splitlines(keepends=True))
                    self.fail(''.join(diff))
