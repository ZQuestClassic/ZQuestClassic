import argparse
import os
import platform
import re
import shutil
import subprocess
import sys
import time

from pathlib import Path
from typing import List, Tuple

system = platform.system()

parser = argparse.ArgumentParser(
    description='Packages the build binaries and necessary runtime files for distribution.'
)
parser.add_argument(
    '--build_folder', help='The location of the build folder. ex: build/Release'
)
parser.add_argument(
    '--package_folder',
    help='The location of the package folder. defaults to <build_folder>/packages/zc',
)
parser.add_argument(
    '--extras',
    action='store_true',
    help='package the extras instead of the main package',
)
parser.add_argument(
    '--clean_first',
    action='store_true',
    help='Delete package folder before copying over files',
)
parser.add_argument(
    '--skip_binaries',
    action='store_true',
    help='Copy on the runtime resource files, not any of the program entrypoints or shared libraries',
)
parser.add_argument(
    '--copy_to_build_folder',
    action='store_true',
    help='Copy to the provided build folder instead of an isolated package folder',
)
parser.add_argument(
    '--keep_existing_files',
    action='store_true',
    help='Only copy files that do not yet exist at the destination. For local development',
)
parser.add_argument(
    '--skip_archive', action='store_true', help='Skip the compression step'
)
parser.add_argument('--version', help='Used to name the changelog generated')
parser.add_argument('--cfg_os')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
resources_dir = root_dir / 'resources'
resources_extra_dir = root_dir / 'resources-extra'
build_dir: Path = Path.cwd() / args.build_folder
packages_dir = build_dir / 'packages'
package_dir = Path(args.package_folder) if args.package_folder else packages_dir / 'zc'


def binary_file(base_dir: Path, path: str):
    as_path = base_dir / path
    if system == 'Windows':
        return (base_dir, as_path.with_suffix('.exe'))
    else:
        return (base_dir, as_path)


def glob(base_dir: Path, pattern: str):
    files = list(base_dir.glob(pattern))
    if not files:
        raise Exception(f'nothing matched pattern: {pattern}')
    return [(base_dir, f) for f in files if f.is_file()]


def glob_maybe(base_dir: Path, pattern: str):
    files = list(base_dir.glob(pattern))
    return [(base_dir, f) for f in files if f.is_file()]


def files(base_dir: Path, files: List[str] = None):
    if files == None:
        return glob(base_dir, '**/*')

    new_files: List[Tuple[Path, Path]] = []
    for f in files:
        path = base_dir / f
        new_files.append((base_dir, path))
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
                x.strip().lower() for x in directive.split('=')
            ]

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


def copy_files_to_package(
    files: List[Tuple[Path, Path]],
    dest_dir: Path,
    exclude_files: List[Tuple[Path, Path]] = None,
):
    if exclude_files:
        files = list(set(files) - set(exclude_files))

    cfg_os = args.cfg_os or system_to_cfg_os(system)

    for folder, path in files:
        if not path.is_absolute():
            raise Exception(f'expected absolute path, got: {path}')
        if not folder.is_dir():
            raise Exception(f'expected {folder} to be a directory')
        if not path.is_file():
            raise Exception(f'expected {path} to be a file')

        if folder:
            dest = dest_dir / path.relative_to(folder)
        else:
            dest = dest_dir / path.name

        if args.keep_existing_files and dest.exists():
            continue

        dest.parent.mkdir(parents=True, exist_ok=True)
        if path.name == '.gitkeep':
            continue

        if path.parent.name == 'base_config' or path.name == 'allegro5.cfg':
            dest.write_text(preprocess_base_config(path.read_text(), cfg_os))
        else:
            shutil.copy2(path, dest, follow_symlinks=False)


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
    shutil.copy2(root_dir / 'LICENSE', output_dir / 'zquest_classic.LICENSE.txt')
    shutil.copy2(root_dir / 'AUTHORS', output_dir / 'zquest_classic.AUTHORS.txt')

    # Collect third party licences.
    third_party_dirs = []
    third_party_dirs.extend((root_dir / 'third_party').glob('*'))
    cmake_deps_dir = build_dir / '_deps'
    if not cmake_deps_dir.exists():
        cmake_deps_dir = build_dir.parent / '_deps'
    third_party_dirs.extend(cmake_deps_dir.glob('*-src'))

    files_to_copy = [
        'COPYING',
        'LICENSE',
        'LICENSE-BSD',
        'LICENSE-MIT',
        'LICENSE.MIT',
        'LICENSE-Boost',
        'AUTHORS',
        'CREDITS',
        'README',
    ]

    for third_party_dir in third_party_dirs:
        if third_party_dir.is_file():
            continue

        name = third_party_dir.name.replace('-src', '').replace('_external', '')
        dir = output_dir / name
        dir.mkdir(exist_ok=True)

        if name == 'asio':
            third_party_dir = third_party_dir / name

        for file in files_to_copy:
            # Kinda big.
            if name == 'allegro_legacy' and file == 'AUTHORS':
                (dir / file).write_text(
                    'https://github.com/NewCreature/Allegro-Legacy/blob/master/AUTHORS'
                )
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


def do_packaging(
    package_dir: Path,
    files: List[Tuple[Path, Path]],
    exclude_files: List[Tuple[Path, Path]] = None,
    include_licenses=False,
):
    prepare_package(package_dir)
    copy_files_to_package(files, package_dir, exclude_files=exclude_files)
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

    all_files = glob(resources_dir, '**/*')
    ignore_files = [
        *files(
            resources_dir,
            [
                'docs/ghost',
                'docs/tango',
                'docs/ZScript_Additions.txt',
            ],
        ),
        *glob(resources_dir, '**/*.pdf'),
    ]

    zplayer_data_files = files(
        resources_dir,
        [
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
            'assets/zc/ZC_Forever_HD.mp3',
            'assets/zc/ZC_Icon_Medium_Player.png',
            'assets/zc/ZC_Logo.png',
            'base_config/zc.cfg',
            'base_config/zcl.cfg',
            'base_config/zquest.cfg',
            'base_config/zscript.cfg',
            'Classic.nsf',
            'modules/classic/classic_fonts.dat',
            'modules/classic/default.qst',
            'modules/classic/title_gfx.dat',
            'modules/classic/zelda.nsf',
            'sfx.dat',
            'zc_web.cfg',
            'zc.png',
            'zquest_web.cfg',
        ],
    )
    zeditor_data_files = [
        *files(
            resources_dir,
            [
                'assets/zc/ZC_Icon_Medium_Editor.png',
                'docs/zquest.txt',
                'docs/zstrings.txt',
            ],
        ),
        *glob(resources_dir, 'assets/editor/**/*'),
    ]
    lazy_files = list(
        set(all_files) - set(zplayer_data_files) - set(zeditor_data_files)
    )

    for pkg in ['web_zplayer_data', 'web_zeditor_data', 'web_lazy_files']:
        f = packages_dir / pkg
        if f.exists():
            shutil.rmtree(f)

    copy_files_to_package(zplayer_data_files, packages_dir / 'web_zplayer_data')
    copy_files_to_package(zeditor_data_files, packages_dir / 'web_zeditor_data')
    copy_files_to_package(
        lazy_files, packages_dir / 'web_lazy_files', exclude_files=ignore_files
    )
    if 'ZC_PACKAGE_REPLAYS' in os.environ:
        shutil.copytree(
            root_dir / 'tests/replays', packages_dir / 'web_lazy_files/test_replays'
        )
        # For run_replay_tests.py to copy to when a replay is not from `tests/replays`
        (packages_dir / 'web_lazy_files/test_replays/tmp.zplay').write_text('')

    emcc_dir = Path(shutil.which('emcc')).parent
    subprocess.check_call(
        [
            'python',
            emcc_dir / 'tools/file_packager.py',
            build_dir / 'zplayer.data',
            '--no-node',
            '--preload',
            f'{packages_dir}/web_zplayer_data@/',
            '--preload',
            f'{root_dir}/timidity/zc.cfg@/etc/zc.cfg',
            '--preload',
            f'{root_dir}/timidity/ultra.cfg@/etc/ultra.cfg',
            '--preload',
            f'{root_dir}/timidity/ppl160.cfg@/etc/ppl160.cfg',
            '--preload',
            f'{root_dir}/timidity/freepats.cfg@/etc/freepats.cfg',
            '--preload',
            f'{root_dir}/timidity/soundfont-pats/oot.cfg@/etc/oot.cfg',
            '--preload',
            f'{root_dir}/timidity/soundfont-pats/2MGM.cfg@/etc/2MGM.cfg',
            '--use-preload-cache',
            f'--js-output={build_dir / "zplayer.data.js"}',
        ]
    )
    subprocess.check_call(
        [
            'python',
            emcc_dir / 'tools/file_packager.py',
            build_dir / 'zeditor.data',
            '--no-node',
            '--preload',
            f'{packages_dir}/web_zeditor_data@/',
            '--use-preload-cache',
            f'--js-output={build_dir / "zeditor.data.js"}',
        ]
    )

    zscript_playground_data_files = [
        *glob(resources_dir, 'include/**/*'),
        *glob(resources_dir, 'headers/**/*'),
        *files(resources_dir, ['base_config/zscript.cfg']),
    ]
    copy_files_to_package(zscript_playground_data_files, packages_dir / 'web_zscript_playground_data')
    subprocess.check_call(
        [
            'python',
            emcc_dir / 'tools/file_packager.py',
            build_dir / 'zscript-playground.data',
            '--no-node',
            '--preload',
            f'{packages_dir}/web_zscript_playground_data@/',
            '--use-preload-cache',
            f'--js-output={build_dir / "zscript-playground.data.js"}',
        ]
    )
    text = (build_dir / 'zscript-playground.data.js').read_text() + '\nexport default Module;'
    (build_dir / 'zscript-playground.data.js').write_text(text)


if 'TEST' in os.environ:
    import unittest

    tc = unittest.TestCase()
    tc.assertEqual(
        preprocess_base_config('ignore_monitor_scale = 0', 'windows'),
        'ignore_monitor_scale = 0',
    )
    tc.assertEqual(
        preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1', 'windows'),
        'ignore_monitor_scale = 1',
    )
    tc.assertEqual(
        preprocess_base_config('ignore_monitor_scale = 0 #? !windows = 1', 'windows'),
        'ignore_monitor_scale = 0',
    )
    tc.assertEqual(
        preprocess_base_config('ignore_monitor_scale = 0 #? windows = 1', 'mac'),
        'ignore_monitor_scale = 0',
    )
    tc.assertEqual(
        preprocess_base_config(
            'ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'mac'
        ),
        'ignore_monitor_scale = 2',
    )
    tc.assertEqual(
        preprocess_base_config(
            'ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'windows'
        ),
        'ignore_monitor_scale = 1',
    )
    tc.assertEqual(
        preprocess_base_config(
            'ignore_monitor_scale = 0 #? windows = 1 ; mac = 2', 'linux'
        ),
        'ignore_monitor_scale = 0',
    )
    tc.assertEqual(
        preprocess_base_config('ignore_monitor_scale = no #? windows = yes', 'windows'),
        'ignore_monitor_scale = yes',
    )
elif args.extras:
    do_packaging(packages_dir / 'extras', glob(resources_extra_dir, '**/*'))
elif args.cfg_os == 'web':
    do_web_packaging()
else:
    # May already exist from build cache.
    nightly_changelog_path = root_dir / 'changelogs/nightly.txt'
    if nightly_changelog_path.exists():
        nightly_changelog_path.unlink()

    # Generate changelog for changes since last stable release.
    # For nightly releases, this changelog is saved as `changelogs/nightly.txt` and includes all changes since the last stable release.
    # For stable releases, this changelog is the same as we would save to `resources/changelogs/DATE-TAG.txt` in source control, except
    # that hasn't happened yet so it's done here for the release job.
    changelog = None
    if args.version:
        major, minor, patch = map(
            int, re.search(r'^(\d+)\.(\d+)\.(\d+)', args.version).groups()
        )
        is_stable_release = patch == '0'
        # Tag either already exists (we are re-publishing for some reason), or doesn't yet.
        try:
            date = subprocess.check_output(
                f'git log -1 --format=%cs {args.version}', shell=True, encoding='utf-8'
            ).strip()
            date = date.replace('-', '_')
        except:
            date = time.strftime("%Y_%m_%d")

        try:
            last_stable = subprocess.check_output(
                f'git describe --tags --abbrev=0 --match "*.*.0" --match "2.55-alpha-1??" --exclude {args.version}',
                shell=True,
                encoding='utf-8',
            ).strip()
            changelog = subprocess.check_output(
                [
                    sys.executable,
                    script_dir / 'generate_changelog.py',
                    '--from',
                    last_stable,
                    '--to',
                    'HEAD',
                    '--version',
                    args.version,
                ],
                encoding='utf-8',
            ).strip()
        except Exception as e:
            changelog = None
            print(e)

        if is_stable_release:
            new_changelog_path = root_dir / f'changelogs/${date}-{args.version}.txt'
        else:
            new_changelog_path = root_dir / 'changelogs/nightly.txt'

        if changelog:
            if is_stable_release:
                new_changelog_path.write_text(changelog)
            else:
                new_changelog_path.write_text(
                    f'Changes since {last_stable}\n\n{changelog}'
                )
        elif new_changelog_path.exists():
            new_changelog_path.unlink()

    zc_files = [
        *glob(resources_dir, '**/*'),
        *glob(root_dir, 'changelogs/**/*'),
    ]

    if args.copy_to_build_folder:
        copy_files_to_package(zc_files, build_dir)
        exit(0)

    if not args.skip_binaries:
        zc_files.extend(
            [
                binary_file(build_dir, 'zplayer'),
                binary_file(build_dir, 'zeditor'),
                binary_file(build_dir, 'zscript'),
                binary_file(build_dir, 'zlauncher'),
                binary_file(build_dir, 'zupdater'),
                *(glob(build_dir, '*.dll') if system == 'Windows' else []),
                *(glob(build_dir, '*.so*') if system == 'Linux' else []),
                *(glob(build_dir, '*.dylib') if system == 'Darwin' else []),
            ]
        )

        if system == 'Windows':
            zc_files.append(binary_file(build_dir, 'zconsole'))
            zc_files.append(binary_file(build_dir, 'zstandalone'))

        crashpad_binary = binary_file(build_dir, 'crashpad_handler')
        if crashpad_binary[1].exists():
            zc_files.append(crashpad_binary)

        if system == 'Linux' and 'PACKAGE_DEBUG_INFO' in os.environ:
            zc_files += glob_maybe(build_dir, '*.debug')

    do_packaging(package_dir, zc_files, include_licenses=True)
