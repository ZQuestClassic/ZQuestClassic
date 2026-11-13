import io
import os
import platform
import tarfile
import zipfile

from pathlib import Path
from time import sleep

import requests

from requests.adapters import HTTPAdapter
from urllib3.util import Retry

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

API_URL = 'https://api.github.com'


class GitHubError(Exception):
    def __init__(self, status: int, data: dict):
        super().__init__(f'{status} {data}')
        self.status = status
        # The response body. For an error from GitHub, has a "message".
        self.data = data


# A minimal client for GitHub's REST API. Results are the parsed JSON, as
# documented at https://docs.github.com/en/rest
class GitHub:
    def __init__(self, token: str | None = None):
        self.token = token
        self.session = requests.Session()
        # The session reuses connections, which GitHub may have closed while
        # idle (say, between polls of a workflow run). Retry those, and
        # GitHub's occasional 5xx errors, for requests that are safe to repeat.
        retry = Retry(
            total=5,
            backoff_factor=1,
            status_forcelist=[500, 502, 503, 504],
            raise_on_status=False,
        )
        self.session.mount('https://', HTTPAdapter(max_retries=retry))
        self.session.headers['Accept'] = 'application/vnd.github+json'
        self.session.headers['X-GitHub-Api-Version'] = '2022-11-28'
        if token:
            self.session.headers['Authorization'] = f'Bearer {token}'

    # `path` is relative to the API root, or a full URL (many results have
    # URLs for related resources).
    def request(self, method: str, path: str, **kwargs) -> requests.Response:
        url = path if path.startswith('https://') else f'{API_URL}/{path}'
        response = self.session.request(method, url, timeout=60, **kwargs)
        if response.status_code >= 400:
            try:
                data = response.json()
            except ValueError:
                data = {'message': response.text}
            raise GitHubError(response.status_code, data)
        return response

    def get(self, path: str, **params):
        return self.request('GET', path, params=params).json()

    def post(self, path: str, body: dict | None = None) -> requests.Response:
        return self.request('POST', path, json=body)

    # Yields every item of a list, fetching more pages only as needed. Some
    # lists are wrapped in an object: `list_item` is the key they are under.
    def paginate(self, path: str, list_item: str | None = None, **params):
        params = {'per_page': 100, **params}
        while path:
            response = self.request('GET', path, params=params)
            data = response.json()
            yield from data[list_item] if list_item else data
            # The URL of the next page already has the params.
            path = response.links.get('next', {}).get('url')
            params = None


def is_arm64_windows():
    if platform.system() != 'Windows':
        return False
    if platform.machine() == 'ARM64':
        return True

    # An x64 python running under emulation may report the emulated
    # architecture, so ask Windows for the machine's own.
    import ctypes

    IMAGE_FILE_MACHINE_ARM64 = 0xAA64
    try:
        kernel32 = ctypes.windll.kernel32
        process_machine = ctypes.c_ushort()
        native_machine = ctypes.c_ushort()
        if kernel32.IsWow64Process2(
            kernel32.GetCurrentProcess(),
            ctypes.byref(process_machine),
            ctypes.byref(native_machine),
        ):
            return native_machine.value == IMAGE_FILE_MACHINE_ARM64
    except (AttributeError, OSError):
        # Older than Windows 10 1709.
        pass
    return False


def infer_gha_platform():
    system = platform.system()
    if system == 'Windows':
        if is_arm64_windows():
            runs_on = 'windows-11-arm'
            arch = 'arm64'
        else:
            runs_on = 'windows-2022'
            arch = 'x64' if platform.architecture()[0] == '64bit' else 'win32'
    elif system == 'Darwin':
        runs_on = 'macos-15-intel'
        arch = 'intel'
    elif system == 'Linux':
        if platform.machine() in ('aarch64', 'arm64'):
            runs_on = 'ubuntu-22.04-arm'
            arch = 'aarch64'
        else:
            runs_on = 'ubuntu-22.04'
            arch = 'x64'
    else:
        raise Exception(f'unexpected platform: {system}')
    return runs_on, arch


# See:
# - https://nightly.link/
# - https://github.com/actions/upload-artifact/issues/51
def _download_artifact(gh: GitHub, repo, artifact, dest):
    if not gh.token:
        url = f'https://nightly.link/{repo}/actions/artifacts/{artifact["id"]}.zip'
        r = requests.get(url)
    else:
        r = gh.request('GET', artifact['archive_download_url'])

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


def get_gha_artifacts(gh: GitHub, repo_str: str, run_id: int) -> Path:
    if not isinstance(run_id, int):
        raise Exception('run_id must be an integer')

    gha_cache_dir = root_dir / '.tmp/gha_artifacts'
    workflow_run_dir = gha_cache_dir / str(run_id)
    if workflow_run_dir.exists() and (workflow_run_dir / '.complete').exists():
        return workflow_run_dir

    artifacts = gh.paginate(
        f'repos/{repo_str}/actions/runs/{run_id}/artifacts', list_item='artifacts'
    )
    replay_artifacts = [a for a in artifacts if a['name'].startswith('replays-')]
    if not replay_artifacts:
        raise Exception(f'no replay artifacts found for run {run_id}')

    workflow_run_dir.mkdir(exist_ok=True, parents=True)

    test_results_paths = []
    for artifact in replay_artifacts:
        artifact_dir = workflow_run_dir / artifact['name']
        if not artifact_dir.exists():
            print(f'downloading artifact: {artifact["name"]}')
            _download_artifact(gh, repo_str, artifact, artifact_dir)
            extract_tars(artifact_dir)
        test_results_path = next(artifact_dir.rglob('test_results.json'), None)
        if test_results_path:
            test_results_paths.append(test_results_path)
        else:
            print(f'missing test_results.json for {artifact["name"]}')

    if not test_results_paths:
        raise Exception(f'run {run_id}: found no test_results.json files')

    (workflow_run_dir / '.complete').write_text('')
    return workflow_run_dir


# It's possible that the artifacts are not available very shortly after a workflow finishes.
# Pretty rare, though.
def get_gha_artifacts_with_retry(gh: GitHub, repo_str: str, run_id: int) -> Path:
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
