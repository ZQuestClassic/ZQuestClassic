#!/usr/bin/env python3

import argparse
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.request


def main():
    parser = argparse.ArgumentParser(
        description="Download and extract a github repo tarball"
    )
    parser.add_argument("url", help="URL of the archive to download")
    parser.add_argument(
        "dest", help="Destination directory name for extracted contents"
    )

    args = parser.parse_args()

    if os.path.exists(args.dest):
        shutil.rmtree(args.dest)

    print(f"Downloading {args.url} ...")

    try:
        with tempfile.TemporaryDirectory() as tmpdir:
            tar_path = os.path.join(tmpdir, "repo.tar.gz")
            urllib.request.urlretrieve(args.url, tar_path)

            print("Extracting ...")
            with tarfile.open(tar_path, "r") as tar:
                tar.extractall(path=tmpdir)

            os.remove(tar_path)

            extracted_dirs = [
                d for d in os.listdir(tmpdir) if os.path.isdir(os.path.join(tmpdir, d))
            ]
            if len(extracted_dirs) != 1:
                print(
                    f"Error: Expected exactly one extracted directory, found: {extracted_dirs}",
                    file=sys.stderr,
                )
                sys.exit(1)

            extracted_dir = os.path.join(tmpdir, extracted_dirs[0])
            shutil.move(extracted_dir, args.dest)

    except Exception as e:
        print(f"Error downloading or extracting: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Successfully downloaded and extracted to {args.dest}")


if __name__ == "__main__":
    main()
