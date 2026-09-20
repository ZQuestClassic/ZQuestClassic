# Runs the updater within a copy of a release package, and asserts files
# were actually updated. Must first run the package step.

import functools
import hashlib
import http.server
import io
import os
import platform
import shutil
import subprocess
import sys
import threading
import unittest
import zipfile

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/test_updater'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

is_mac = platform.system() == 'Darwin'
is_linux = platform.system() == 'Linux'


def make_zip(entries):
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, 'w', zipfile.ZIP_STORED) as z:
        for name, data in entries:
            z.writestr(name, data)
    return buf.getvalue()


def corrupt_zip_entry(zip_bytes: bytes, data: bytes):
    """Flips a byte of a stored entry's data, so the CRC recorded for it no longer matches."""
    b = bytearray(zip_bytes)
    b[b.index(data)] ^= 0xFF
    return bytes(b)


def snapshot_files(folder: Path):
    """Content hash of every file, keyed by relative path."""
    result = {}
    for path in folder.rglob('*'):
        if path.is_file() and path.name != 'updater.log':
            result[path.relative_to(folder).as_posix()] = hashlib.sha1(
                path.read_bytes()
            ).hexdigest()
    return result


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        pass


def serve_folder(folder: Path):
    """Serves a folder over http on a free localhost port. Returns (server, base url)."""
    handler = functools.partial(QuietHandler, directory=str(folder))
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server, f'http://127.0.0.1:{server.server_address[1]}'


class TestUpdater(unittest.TestCase):
    def setUp(self):
        # TODO: support mac
        if is_mac:
            self.skipTest('mac not supported')
        # TODO: works, but in CI there is a problem caching the downloaded release
        if is_linux:
            self.skipTest('linux not supported in CI')

    def get_package_folder(self):
        build_folder = run_target.get_build_folder()
        if 'CI' in os.environ:
            package_folder = build_folder
            if is_mac:
                package_folder = next(
                    p for p in package_folder.parents if p.name.endswith('.app')
                )
        elif is_mac:
            package_folder = build_folder / 'packages/zc-mac/ZQuest Classic.app'
        else:
            package_folder = build_folder / 'packages/zc'

        if not package_folder or not package_folder.exists():
            raise Exception('must run package_zc target')

        return package_folder

    def fresh_package(self, package_folder: Path):
        """Copies the package to a scratch folder. Returns the copy, and the folder inside it
        that the apps live in."""
        fresh_package_dir = tmp_dir / 'fresh'
        shutil.rmtree(fresh_package_dir, ignore_errors=True)
        fresh_package_dir.mkdir(exist_ok=True, parents=True)
        shutil.copytree(
            package_folder,
            fresh_package_dir / package_folder.name,
            dirs_exist_ok=True,
        )
        fresh_package_folder = fresh_package_dir / package_folder.name

        if is_mac:
            resource_folder = fresh_package_folder / 'Contents/Resources'
        else:
            resource_folder = fresh_package_folder
        return fresh_package_folder, resource_folder

    def test_updater(self):
        package_folder = self.get_package_folder()

        for app in ['zupdater', 'zlauncher']:
            with self.subTest(msg=f'w/ {app}'):
                fresh_package_folder, resource_folder = self.fresh_package(package_folder)

                files = [
                    resource_folder / 'zc.png',
                    resource_folder / 'base_config/zc.cfg',
                    resource_folder / run_target.get_exe_name('zplayer'),
                    resource_folder / run_target.get_exe_name('zeditor'),
                    resource_folder / run_target.get_exe_name('zlauncher'),
                    resource_folder / run_target.get_exe_name('zscript'),
                    resource_folder / run_target.get_exe_name('zupdater'),
                ]
                if is_mac:
                    files.append(
                        fresh_package_folder / 'Contents/libs/libzcsound.dylib'
                    )

                for file in files:
                    os.utime(file, (0, 0))

                run_args = {
                    'timeout': 120,
                    'env': {
                        **os.environ,
                        'ZC_UPDATER_CACHE_FOLDER': str(tmp_dir / 'cache'),
                    },
                }

                before_version = run_target.check_run(
                    'zplayer', ['-version'], resource_folder, **run_args
                ).stdout.strip()
                before_mod_times = [f.stat().st_mtime for f in files]

                updater_log_path = resource_folder / 'updater.log'
                try:
                    if app == 'zupdater':
                        p = run_target.run(
                            'zupdater',
                            ['-headless', '-cache'],
                            resource_folder,
                            **run_args,
                        )
                    elif app == 'zlauncher':
                        p = run_target.run(
                            'zlauncher', ['-update'], resource_folder, **run_args
                        )
                except subprocess.TimeoutExpired as e:
                    if updater_log_path.exists():
                        print(updater_log_path.read_text('utf-8'))
                    raise e

                updater_log = ''
                if updater_log_path.exists():
                    updater_log = updater_log_path.read_text('utf-8')

                if p.returncode:
                    raise Exception(
                        f'failed with exit code: {p.returncode}\n{p.stdout}\nupdater.log:\n{updater_log}'
                    )
                print(updater_log)

                after_version = run_target.check_run(
                    'zplayer', ['-version'], resource_folder, **run_args
                ).stdout.strip()
                after_mod_times = [f.stat().st_mtime for f in files]

                files_not_updated = ''
                for i in range(len(before_mod_times)):
                    if before_mod_times[i] == after_mod_times[i]:
                        files_not_updated += f'file did not update: {files[i]}\n'
                if files_not_updated:
                    raise Exception(files_not_updated)

                if before_version == after_version:
                    raise Exception('`zplayer -version` did not update')

    def test_failed_update_leaves_install_untouched(self):
        """A download that is not a release, or that cannot be fully extracted, must leave
        the install exactly as it was, and must not be kept for the next attempt."""
        package_folder = self.get_package_folder()

        serve_dir = tmp_dir / 'serve'
        shutil.rmtree(serve_dir, ignore_errors=True)
        serve_dir.mkdir(parents=True)
        (serve_dir / 'notzip.zip').write_bytes(b'<html>this is not a release</html>')
        # The first entry replaces a real file, so there is something to roll back
        # when the second entry fails its CRC check.
        bad_data = b'the bytes of this entry get corrupted' * 10
        rollback_zip = corrupt_zip_entry(
            make_zip([('base_config/zc.cfg', b'# replaced'), ('zc.png', bad_data)]),
            bad_data,
        )
        self.assertEqual(zipfile.ZipFile(io.BytesIO(rollback_zip)).testzip(), 'zc.png')
        (serve_dir / 'rollback.zip').write_bytes(rollback_zip)

        cache_folder = tmp_dir / 'cache-failures'
        shutil.rmtree(cache_folder, ignore_errors=True)
        run_args = {
            'timeout': 120,
            'env': {
                **os.environ,
                'ZC_UPDATER_CACHE_FOLDER': str(cache_folder),
            },
        }

        server, base_url = serve_folder(serve_dir)
        try:
            for name in ['notzip.zip', 'rollback.zip', 'missing.zip']:
                with self.subTest(msg=name):
                    _, resource_folder = self.fresh_package(package_folder)
                    before = snapshot_files(resource_folder)

                    p = run_target.run(
                        'zupdater',
                        ['-headless', '-cache', '-install', '-asset-url', f'{base_url}/{name}'],
                        resource_folder,
                        **run_args,
                    )
                    updater_log = (resource_folder / 'updater.log').read_text('utf-8')
                    self.assertNotEqual(p.returncode, 0, f'{p.stdout}\nupdater.log:\n{updater_log}')
                    self.assertIn('[fatal] Failed:', updater_log)

                    # Includes .updater-active-files: nothing may be left in there.
                    self.assertEqual(snapshot_files(resource_folder), before)
                    self.assertEqual([f.name for f in cache_folder.rglob('*') if f.is_file()], [])

                    run_target.check_run('zplayer', ['-version'], resource_folder, **run_args)
        finally:
            server.shutdown()


if __name__ == '__main__':
    unittest.main()
