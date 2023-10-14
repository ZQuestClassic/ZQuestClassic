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
build_dir: Path = Path.cwd() / args.build_folder
packages_dir = build_dir / 'packages'

extras = [
    resources_dir / 'Addons',
    resources_dir / 'utilities',
]


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


def glob_files_2(base_dir: Path, pattern: str):
    files = list(base_dir.rglob(pattern))
    if not files:
        raise Exception(f'nothing matched pattern: {pattern}')
    return [f for f in files if f.is_file()]


def files(base_dir: Path, files: List[str]):
    new_files = []
    for f in files:
        path = base_dir / f
        if not path.exists():
            raise Exception(f'file missing: {f}')
        new_files.append(path)
    return new_files


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

        if type(file) is tuple:
            path, dir = file
            files_flat.append((path, dir))
            continue

        if file.is_dir():
            for dir_path, dirs, dir_files in os.walk(file):
                for dir_file in dir_files:
                    path = Path(os.path.join(dir_path, dir_file))
                    if path.is_file():
                        files_flat.append((path, file.parent))
        else:
            files_flat.append((file, None))

    for src, folder in files_flat:
        if folder:
            dest = dest_dir / src.relative_to(folder)
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


def do_web_packaging():
    # An emscripten `.data` file (one for zplayer and zeditor) contains data that will be mapped to
    # the runtimes virtual filesystem before the application starts. Only the most important files
    # go into this, as having it be too big make startup time slow.
    # For everything else, they are downloaded as needed: they will end up in a `./files` folder and
    # via ZC.pathToUrl will be mapped to a URL (see emscripten_utils.cpp `em_init_fs`)
    # zplayer will use zplayer.data, but zeditor will use both zplayer.data and zquest.data

    all_files = glob_files_2(resources_dir, '*')
    ignore_files = [
        *files(resources_dir, [
            'docs/ghost',
            'docs/tango',
            'docs/ZScript_Additions.txt',
        ]),
        *glob_files_2(resources_dir, '**/*.pdf'),
    ]
    for e in extras:
        ignore_files.extend(glob_files_2(e, '*'))
    all_files = list(set(all_files) - set(ignore_files))

    zplayer_data_files = files(resources_dir, [
        'ag.cfg',
        'allegro5.cfg',
        'assets/cursor.bmp',
        'assets/dungeon.mid',
        'assets/ending.mid',
        'assets/gameover.mid',
        'assets/gui_pal.bmp',
        'assets/level9.mid',
        'assets/overworld.mid',
        'assets/title.mid',
        'assets/triforce.mid',
        'base_config/zc.cfg',
        'base_config/zcl.cfg',
        'base_config/zquest.cfg',
        'base_config/zscript.cfg',
        'Classic.nsf',
        'modules/classic.zmod',
        'modules/classic/classic_fonts.dat',
        'modules/classic/default.qst',
        'modules/classic/title_gfx.dat',
        'modules/classic/zelda.nsf',
        'sfx.dat',
        'zc_web.cfg',
        'zc.png',
        'zquest_web.cfg',
    ])
    zeditor_data_files = files(resources_dir, [
        'docs/zquest.txt',
        'docs/zstrings.txt',
        'modules/classic/classic_zquest.dat',
    ])
    lazy_files = list(set(all_files) - set(zplayer_data_files) - set(zeditor_data_files))

    zplayer_data_files = [(f, resources_dir) for f in zplayer_data_files]
    zeditor_data_files = [(f, resources_dir) for f in zeditor_data_files]
    lazy_files = [(f, resources_dir) for f in lazy_files]

    for pkg in ['web_zplayer_data', 'web_zeditor_data', 'web_lazy_files']:
        f = packages_dir / pkg
        if f.exists():
            shutil.rmtree(f)

    copy_files_to_package(zplayer_data_files, packages_dir / 'web_zplayer_data')
    copy_files_to_package(zeditor_data_files, packages_dir / 'web_zeditor_data')
    copy_files_to_package(lazy_files, packages_dir / 'web_lazy_files')

    emcc_dir = Path(shutil.which('emcc')).parent
    subprocess.check_call([
        'python',
        emcc_dir / 'tools/file_packager.py',
        build_dir / 'zplayer.data',
        '--no-node',
        '--preload', f'{packages_dir}/web_zplayer_data@/',
        '--preload', f'{root_dir}/timidity/zc.cfg@/etc/zc.cfg',
        '--preload', f'{root_dir}/timidity/ultra.cfg@/etc/ultra.cfg',
        '--preload', f'{root_dir}/timidity/ppl160.cfg@/etc/ppl160.cfg',
        '--preload', f'{root_dir}/timidity/freepats.cfg@/etc/freepats.cfg',
        '--preload', f'{root_dir}/timidity/soundfont-pats/oot.cfg@/etc/oot.cfg',
        '--preload', f'{root_dir}/timidity/soundfont-pats/2MGM.cfg@/etc/2MGM.cfg',
        '--use-preload-cache',
        f'--js-output={build_dir / "zplayer.data.js"}',
    ])
    subprocess.check_call([
        'python',
        emcc_dir / 'tools/file_packager.py',
        build_dir / 'zeditor.data',
        '--no-node',
        '--preload', f'{packages_dir}/web_zeditor_data@/',
        '--use-preload-cache',
        f'--js-output={build_dir / "zeditor.data.js"}',
    ])
    if 'ZC_PACKAGE_REPLAYS' in os.environ:
        subprocess.check_call([
            'python',
            emcc_dir / 'tools/file_packager.py',
            build_dir / 'replays.data',
            '--no-node',
            '--preload', f'{root_dir}/tests/replays@/test_replays',
            '--use-preload-cache',
            f'--js-output={build_dir / "replays.data.js"}',
        ])

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
elif args.cfg_os == 'web':
    do_web_packaging()
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
