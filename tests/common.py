import difflib
import os
import json
import subprocess
from dataclasses import dataclass, field
import dataclasses
from typing import List, Literal, Tuple, Optional, Any
import unittest
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
    path: str
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
                # New property.
                if 'path' not in run:
                    run['path'] = ''
                deserialized.append([RunResult(**run) for run in runs])
            self.runs = deserialized

    def to_json(self, indent=2):
        as_dict = dataclasses.asdict(self)
        return json.dumps(as_dict, indent=indent)


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


def get_recent_release_tag(args: List[str]):
    command = f'git describe --tags --abbrev=0 ' + ' '.join(args)
    return subprocess.check_output(command.split(' '), encoding='utf-8').strip()

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
