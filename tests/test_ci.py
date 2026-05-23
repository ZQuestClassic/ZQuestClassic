import os
import platform
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/test_ci'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

CI = 'CI' in os.environ
is_mac = platform.system() == 'Darwin'
is_windows = platform.system() == 'Windows'
is_linux = platform.system() == 'Linux'


class TestCi(unittest.TestCase):
    def setUp(self):
        self.maxDiff = None
        self.build_folder = run_target.get_build_folder()
        self.ci_script = root_dir / '.github' / 'workflows' / 'ci.py'

        if tmp_dir.exists():
            shutil.rmtree(tmp_dir)
        tmp_dir.mkdir(parents=True)

    def run_ci(self, args: list, env=None):
        cmd = [sys.executable, str(self.ci_script)] + args
        print(f"Running: {' '.join(cmd)}")
        return subprocess.check_call(cmd, env=env)

    def test_ci(self):
        if 'CI' in os.environ:
            self.skipTest('this test is only for local development')

        packages_dir = tmp_dir / 'packages'
        extracted_dir = tmp_dir / 'extracted-package'

        self.run_ci(
            ['package', '--repo', 'dummy/repo', '--packages-dir', str(packages_dir)],
        )

        package = next(packages_dir.glob('*.*'))
        print(f'extracting package: {package}')

        self.run_ci(
            [
                'extract-package',
                '--repo',
                'dummy/repo',
                '--package',
                str(package),
                '--build-folder',
                str(extracted_dir),
            ],
        )

        print('running replay tests')

        self.run_ci(
            [
                'replay-tests',
                '--repo',
                'dummy/repo',
                '--build-folder',
                str(extracted_dir),
                '--extra-args=--filter=circle.zplay',
            ],
        )

        # TODO: add a test for the compare command


if __name__ == '__main__':
    unittest.main()
