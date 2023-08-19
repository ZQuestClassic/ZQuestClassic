# Runs the updater within a copy of a release package, and asserts files
# were actually updated. Must first run the package step.

import platform
import os
import sys
import unittest
import shutil
import subprocess
from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
tmp_dir = root_dir / '.tmp/test_updater'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestUpdater(unittest.TestCase):
	def test_updater(self):
		is_mac = platform.system() == 'Darwin'
		# TODO: support mac
		if is_mac:
			return

		build_folder = run_target.get_build_folder()
		if 'CI' in os.environ:
			package_folder = build_folder
			if is_mac:
				package_folder = next(p for p in package_folder.parents if p.name.endswith('.app'))
		elif is_mac:
			package_folder = build_folder / 'packages/zc-mac/ZQuestClassic.app'
		else:
			package_folder = build_folder / 'packages/zc'

		if not package_folder or not package_folder.exists():
			raise Exception('must run package.py or package_mac.sh first')

		for app in ['zupdater', 'zlauncher']:
			with self.subTest(msg=f'w/ {app}'):
				fresh_package_dir = tmp_dir / 'fresh'
				shutil.rmtree(fresh_package_dir, ignore_errors=True)
				fresh_package_dir.mkdir(exist_ok=True, parents=True)
				shutil.copytree(package_folder, fresh_package_dir / package_folder.name, dirs_exist_ok=True)
				fresh_package_folder = fresh_package_dir / package_folder.name

				if is_mac:
					resource_folder = fresh_package_folder / 'Contents/Resources'
				else:
					resource_folder = fresh_package_folder

				files = [
					resource_folder / 'zc.png',
					resource_folder / 'base_config/zc.cfg',
					resource_folder / run_target.get_exe_name('zelda'),
					resource_folder / run_target.get_exe_name('zquest'),
					resource_folder / run_target.get_exe_name('zlauncher'),
					resource_folder / run_target.get_exe_name('zscript'),
					# TODO: include after first release has the updater.
					# resource_folder / run_target.get_exe_name('zupdater'),
				]
				if is_mac:
					files.append(fresh_package_folder / 'Contents/libs/libzcsound.dylib')
				
				for file in files:
					os.utime(file, (0, 0))

				run_args = {
					'timeout': 120,
					'env': {
						**os.environ,
						'ZC_UPDATER_CACHE_FOLDER': str(tmp_dir / 'cache'),
					},
				}

				before_version = run_target.check_run('zelda', ['-version'], resource_folder, **run_args).stdout.strip()
				before_mod_times = [f.stat().st_mtime for f in files]

				updater_log_path = resource_folder / 'updater.log'
				try:
					if app == 'zupdater':
						p = run_target.run('zupdater', ['-headless', '-cache'], resource_folder, **run_args)
					elif app == 'zlauncher':
						p = run_target.run('zlauncher', ['-update'], resource_folder, **run_args)
				except subprocess.TimeoutExpired as e:
					if updater_log_path.exists():
						print(updater_log_path.read_text('utf-8'))
					raise e

				updater_log = ''
				if updater_log_path.exists():
					updater_log = updater_log_path.read_text('utf-8')

				if p.returncode:
					raise Exception(f'failed with exit code: {p.returncode}\n{p.stdout}\nupdater.log:\n{updater_log}')
				print(updater_log)

				after_version = run_target.check_run('zelda', ['-version'], resource_folder, **run_args).stdout.strip()
				after_mod_times = [f.stat().st_mtime for f in files]

				files_not_updated = ''
				for i in range(len(before_mod_times)):
					if before_mod_times[i] == after_mod_times[i]:
						files_not_updated += f'file did not update: {files[i]}\n'
				if files_not_updated:
					raise Exception(files_not_updated)

				if before_version == after_version:
					raise Exception('`zelda -version` did not update')


if __name__ == '__main__':
	unittest.main()
