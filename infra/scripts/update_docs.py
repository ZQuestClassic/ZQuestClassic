#!/usr/bin/env python3

import json
import os
import shutil
import subprocess
import sys
import tarfile
import urllib.request

from pathlib import Path

# Configuration
LATEST_JSON_URL = "https://zquestclassic.com/releases/latest.json"
LAST_VERSION_FILE = Path.home() / ".zc-docs-last-update-version"
TMP_BUILD_DIR = Path.home() / "zc-docs-tmp-build"
# This script is at infra/scripts/update_docs.py, so REPO_ROOT is 3 levels up.
REPO_ROOT = Path(__file__).resolve().parent.parent.parent
UPDATE_SH = REPO_ROOT / "docs-www" / "update.sh"
DEST_HTML_DIR = "/var/www/docs.zquestclassic.com/"


def get_latest_info():
    print(f"Fetching latest release info from {LATEST_JSON_URL}...")
    with urllib.request.urlopen(LATEST_JSON_URL) as response:
        return json.loads(response.read().decode())


def get_last_version():
    if LAST_VERSION_FILE.exists():
        return LAST_VERSION_FILE.read_text().strip()
    return None


def save_last_version(version):
    LAST_VERSION_FILE.write_text(version)


def main():
    try:
        info = get_latest_info()
    except Exception as e:
        print(f"Error fetching latest info: {e}")
        return

    latest_version = info['tagName']
    last_version = get_last_version()

    if latest_version == last_version:
        print(f"Already up to date: {latest_version}")
        return

    print(f"New version detected: {latest_version} (last: {last_version})")

    if TMP_BUILD_DIR.exists():
        print(f"Cleaning up old {TMP_BUILD_DIR}...")
        shutil.rmtree(TMP_BUILD_DIR)

    TMP_BUILD_DIR.mkdir(parents=True, exist_ok=True)

    asset = next(a for a in info['assets'] if 'linux' in a['name'])
    linux_url = asset['url']
    print(f"Downloading {linux_url}...")
    tarball_path = TMP_BUILD_DIR / "linux.tar.gz"

    try:
        urllib.request.urlretrieve(linux_url, tarball_path)
    except Exception as e:
        print(f"Error downloading tarball: {e}")
        return

    print("Extracting...")
    try:
        with tarfile.open(tarball_path, "r:gz") as tar:
            tar.extractall(path=TMP_BUILD_DIR)
    except Exception as e:
        print(f"Error extracting tarball: {e}")
        return

    # Set up environment for update.sh
    env = os.environ.copy()
    bin_dir = next(TMP_BUILD_DIR.glob("**/bin"), None)
    if not bin_dir:
        # Fallback to TMP_BUILD_DIR/bin if glob fails for some reason
        bin_dir = TMP_BUILD_DIR / "bin"

    env["BUILD_FOLDER"] = str(bin_dir)

    print(f"Running update.sh with BUILD_FOLDER={env['BUILD_FOLDER']}...")
    try:
        # We run this from REPO_ROOT to ensure relative paths in update.sh and
        # generate_zscript_api_docs.py work correctly.
        subprocess.check_call(["bash", str(UPDATE_SH)], env=env, cwd=str(REPO_ROOT))
    except subprocess.CalledProcessError as e:
        print(f"Error running update.sh: {e}")
        return

    print(f"rsyncing to {DEST_HTML_DIR}...")
    source_html = REPO_ROOT / "docs-www" / "build" / "html"
    if not source_html.exists():
        print(f"Error: source HTML directory does not exist: {source_html}")
        return

    try:
        # Use -a (archive), -v (verbose), -z (compress), --delete
        subprocess.check_call(
            ["rsync", "-avz", "--delete", f"{source_html}/", DEST_HTML_DIR]
        )
    except subprocess.CalledProcessError as e:
        print(f"Error during rsync: {e}")
        return

    save_last_version(latest_version)

    print("Cleaning up...")
    shutil.rmtree(TMP_BUILD_DIR)
    print("Update complete!")


if __name__ == "__main__":
    main()
