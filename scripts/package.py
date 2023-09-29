import argparse
import os
import shutil
import platform
import shutil
import subprocess
import sys
from typing import List, Tuple
from pathlib import Path

system = platform.system()

parser = argparse.ArgumentParser(
    description='Packages the build binaries and necessary runtime files for distribution.')
parser.add_argument('--build_folder',
                    help='The location of the build folder. ex: build/Release')
parser.add_argument('--extras', action='store_true',
                    help='package the extras instead of the main package')
parser.add_argument('--clean_first', action='store_true',
                    help='Delete package folder before copying over files')
parser.add_argument('--skip_binaries', action='store_true',
                    help='Copy on the runtime resource files, not any of the program entrypoints or shared libraries')
parser.add_argument('--copy_to_build_folder', action='store_true',
                    help='Copy to the provided build folder instead of an isolated package folder')
parser.add_argument('--keep_existing_files', action='store_true',
                    help='Only copy files that do not yet exist at the destination. For local development')
parser.add_argument('--skip_archive', action='store_true',
                    help='Skip the compression step')
parser.add_argument('--cfg_os')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
resources_dir = root_dir / 'resources'
build_dir = Path.cwd() / args.build_folder
packages_dir = build_dir / 'packages'


def binary_file(path: Path):
    if system == 'Windows':
        return path.with_suffix('.exe')
    else:
        return path


def glob_files(base_dir: Path, pattern: str):
    files = list(base_dir.glob(pattern))
    if not files:
        raise Exception(f'nothing matched pattern: {pattern}')
    return files


def system_to_cfg_os(system: str):
    if system == 'Windows':
        return 'windows'
    elif system == 'Darwin':
        return 'mac'
    elif system == 'Linux':
        return 'linux'


def preprocess_base_config(config_text: str, cfg_os: str):
    new_config_lines = []
    for line in config_text.splitlines():
        if '#?' not in line:
            new_config_lines.append(line)
            continue

        statement_text, directives_text = line.split('#?')
        key, default_value = [x.strip() for x in statement_text.split('=')]
        value = default_value

        directives = [x.strip().lower() for x in directives_text.split(';')]
        for directive in directives:
            condition_text, directive_value = [
                x.strip().lower() for x in directive.split('=')]

            should_negate = False
            if condition_text[0] == '!':
                condition_text = condition_text[1:]
                should_negate = True
            is_match = condition_text == cfg_os

            if should_negate:
                is_match = not is_match
            if is_match:
                value = directive_value
                break

        new_config_lines.append(f'{key} = {value}')

    return '\n'.join(new_config_lines)


def copy_files_to_package(files: List[Path], dest_dir: Path):
    cfg_os = args.cfg_os or system_to_cfg_os(system)
    files_flat: List[Tuple[Path, Path]] = []
    for file in files:
        if not file:
            continue

        if file.is_dir():
            for dir_path, dirs, dir_files in os.walk(file):
                for dir_file in dir_files:
                    path = Path(os.path.join(dir_path, dir_file))
                    if path.is_file():
                        files_flat.append((path, file))
        else:
            files_flat.append((file, None))

    for src, folder in files_flat:
        if folder:
            dest = dest_dir / src.relative_to(folder.parent)
        else:
            dest = dest_dir / src.name

        if args.keep_existing_files and dest.exists():
            continue

        dest.parent.mkdir(parents=True, exist_ok=True)
        if src.parent.name == 'base_config' or src.name == 'allegro5.cfg':
            dest.write_text(preprocess_base_config(src.read_text(), cfg_os))
        else:
            shutil.copy2(src, dest, follow_symlinks=False)


def prepare_package(package_dir: Path):
    if args.clean_first and package_dir.exists():
        shutil.rmtree(package_dir)
    package_dir.mkdir(parents=True, exist_ok=True)


def archive_package(package_dir: Path):
    dest = shutil.make_archive(
        base_name=package_dir,
        format='zip' if system == 'Windows' else 'gztar',
        root_dir=package_dir,
    )
    print(f'archived {dest}')


def collect_licenses(package_dir: Path):
    output_dir = package_dir / 'licenses'
    output_dir.mkdir(exist_ok=True, parents=True)
    shutil.copy2(root_dir / 'license.txt', output_dir / 'zquest_classic.LICENSE.txt')
    shutil.copy2(root_dir / 'AUTHORS', output_dir / 'zquest_classic.AUTHORS.txt')

    # Collect third party licences.
    third_party_dirs = []
    third_party_dirs.extend((root_dir / 'third_party').glob('*'))
    cmake_deps_dir = build_dir / '_deps'
    if not cmake_deps_dir.exists():
        cmake_deps_dir = build_dir.parent / '_deps'
    third_party_dirs.extend(cmake_deps_dir.glob('*-src'))

    files_to_copy = ['LICENSE', 'AUTHORS', 'CREDITS', 'README']

    for third_party_dir in third_party_dirs:
        if third_party_dir.is_file():
            continue

        name = third_party_dir.name.replace('-src', '').replace('_external', '')
        dir = output_dir / name
        dir.mkdir(exist_ok=True)

        for file in files_to_copy:
            # Kinda big.
            if name == 'allegro_legacy' and file == 'AUTHORS':
                (dir / file).write_text('https://github.com/NewCreature/Allegro-Legacy/blob/master/AUTHORS')
                continue

            variants = [file]
            for file in list(variants):
                variants.append(file.lower())
            for file in list(variants):
                variants.append(f'{file}.txt')
                variants.append(f'{file}.md')
            file = next((f for f in variants if (third_party_dir / f).exists()), None)
            if file:
                shutil.copy2(third_party_dir / file, dir / file)



def do_packaging(package_dir: Path, files, include_licenses=False):
    prepare_package(package_dir)
    copy_files_to_package(files, package_dir)
    if include_licenses:
        collect_licenses(package_dir)
    print(f'packaged {package_dir}')
    if not args.skip_archive:
        archive_package(package_dir)


extras = [
    resources_dir / 'Addons',
    resources_dir / 'utilities',
]

if 'TEST' in os.environ:
    import unittest

    tc = unittest.TestCase()
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0', 'windows'), 'ignore_monitor_scale = 0')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1', 'windows'), 'ignore_monitor_scale = 1')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? !windows = 1', 'windows'), 'ignore_monitor_scale = 0')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1', 'mac'), 'ignore_monitor_scale = 0')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'mac'), 'ignore_monitor_scale = 2')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'windows'), 'ignore_monitor_scale = 1')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'linux'), 'ignore_monitor_scale = 0')
    tc.assertEqual(preprocess_base_config('ignore_monitor_scale = no #? windows = yes', 'windows'), 'ignore_monitor_scale = yes')
elif args.extras:
    do_packaging(packages_dir / 'extras', extras)
else:
    # Generate changelog for changes since last stable release.
    try:
        last_stable = subprocess.check_output(
            'git describe --tags --abbrev=0 --match "2.55-*"', shell=True, encoding='utf-8').strip()
        changelog = subprocess.check_output([
            sys.executable, script_dir / 'generate_changelog.py',
            '--from', last_stable,
            '--to', 'HEAD',
        ], encoding='utf-8').strip()
    except Exception as e:
        changelog = None
        print(e)

    nightly_changelog_path = root_dir / 'changelogs/nightly.txt'
    if changelog:
        nightly_changelog_path.write_text(f'Changes since {last_stable}\n\n{changelog}')
    elif nightly_changelog_path.exists():
        nightly_changelog_path.unlink()

    files = [
        *glob_files(resources_dir, '*'),
        root_dir / 'changelogs',
    ]
    files = list(set(files) - set(extras))

    if args.copy_to_build_folder:
        copy_files_to_package(files, build_dir)
        exit(0)

    if not args.skip_binaries:
        files.extend([
            binary_file(build_dir / 'zplayer'),
            binary_file(build_dir / 'zeditor'),
            binary_file(build_dir / 'zscript'),
            binary_file(build_dir / 'zlauncher'),
            binary_file(build_dir / 'zconsole') if system == 'Windows' else None,
            binary_file(build_dir / 'zstandalone') if system == 'Windows' else None,
            binary_file(build_dir / 'zupdater'),
            *(glob_files(build_dir, '*.dll') if system == 'Windows' else []),
            *(glob_files(build_dir, '*.so*') if system == 'Linux' else []),
            *(glob_files(build_dir, '*.dylib') if system == 'Darwin' else []),
        ])

        crashpad_binary = binary_file(build_dir / 'crashpad_handler')
        if crashpad_binary.exists():
            files.append(crashpad_binary)

        if system == 'Linux' and 'PACKAGE_DEBUG_INFO' in os.environ:
            files += glob_files(build_dir, '*.debug')

    do_packaging(packages_dir / 'zc', files, include_licenses=True)
