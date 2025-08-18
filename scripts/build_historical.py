import os
import shutil
import subprocess
import traceback
import zipfile

from pathlib import Path

import archives
import git_helpers

from archives import Revision

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent


def run_command(args: list[str], **kwargs):
    p = subprocess.run(args, capture_output=True, encoding='utf-8', **kwargs)
    if p.returncode != 0:
        cmd = ' '.join(args)
        raise Exception(f'Failed command {cmd}\n\n{p.stdout}')


def local_build_error(revision: Revision):
    if not revision.is_local_build:
        return False

    dest = revision.dir()
    if not dest.exists():
        return False

    if (dest / 'error.txt').exists():
        return dest / 'error.txt'

    return False


def has_built_locally(revision: Revision):
    if not revision.is_local_build:
        return False

    dest = revision.dir()
    if not dest.exists():
        return False

    if (dest / 'error.txt').exists():
        return False

    return list(dest.glob('*'))


def try_run(args, **kwargs):
    try:
        subprocess.check_output(args, **kwargs)
    except subprocess.TimeoutExpired:
        return
    except Exception as e:
        ex = "".join(traceback.format_exception_only(e)).strip()
        log = kwargs['cwd'] / 'allegro.log'
        if log.exists():
            ex += '\n' + log.read_text('utf-8')
        raise Exception(ex)


def get_most_recent_rev(revision: Revision, release_platform: str):
    most_recent_rev = None
    for rev in archives.get_revisions(
        release_platform, 'main', include_test_builds=True
    ):
        if rev.commit_count == -1:
            continue
        if rev.is_local_build:
            continue
        if rev.commit_count >= revision.commit_count:
            return rev
        if most_recent_rev == None or most_recent_rev.commit_count < rev.commit_count:
            most_recent_rev = rev
    return most_recent_rev


def build_locally(revision: Revision, release_platform: str):
    """
    Attempts to build any commit of the repo. Here be dragons.

    ZC has a long history of not caring about reproducible builds. There are many, many pitfalls to handle.
    """

    def rm(path: Path):
        if path.exists():
            path.unlink()

    dest = revision.dir()
    if local_build_error(revision) or has_built_locally(revision):
        return dest

    print(f'building locally: {revision}')
    local_build_working_dir = root_dir / '.tmp/local_build_working_dir'

    # Not sure this can be cherry-picked within this range, so just skip for now.
    if (
        archives.get_release_commit_count('0f162d21d47cec49da6276120959be8393c4f80a')
        <= revision.commit_count
        < archives.get_release_commit_count('68fd510de75f14b57221e677c7b361df3dee493c')
    ):
        raise Exception('cannot build this range')

    # Don't know how to build before cmake.
    if revision.commit_count < archives.get_release_commit_count(
        '43313872ea176fa01a015bc09b30cbb6ea638fde'
    ):
        raise Exception('cannot build makefile')

    if not local_build_working_dir.exists():
        local_build_working_dir.mkdir(parents=True)
        print('cloning repo ... this will take awhile')
        run_command(
            [
                'git',
                'clone',
                'https://github.com/ZQuestClassic/ZQuestClassic.git',
                local_build_working_dir,
            ]
        )
    else:
        run_command(['git', 'fetch'], cwd=local_build_working_dir)

    run_command(f'git checkout -- .'.split(' '), cwd=local_build_working_dir)
    run_command(f'git clean -fd'.split(' '), cwd=local_build_working_dir)
    run_command(
        f'git checkout -f {revision.tag}'.split(' '), cwd=local_build_working_dir
    )

    # TODO Before cmake, the makefile was garbage ...
    if not (local_build_working_dir / 'CMakeLists.txt').exists():
        raise Exception('too old')

    supports_64bit = (
        'libs/win32' not in (local_build_working_dir / 'CMakeLists.txt').read_text()
    )
    if not supports_64bit:
        if not release_platform == 'windows':
            raise Exception('unsupported')

    how_to_package = None
    if (local_build_working_dir / 'scripts/package.py').exists():
        how_to_package = 'package.py'
    elif (local_build_working_dir / 'output/_auto/buildpack.bat').exists():
        how_to_package = 'bat'
    else:
        how_to_package = 'cp'

    # Fix broken commits.
    fixups = [
        ('855b797', '2afbe43', None),
        ('1737042', 'd4a7747', None),
        ('0c688f550', '03deb57e9', None),
        ('0a5842ebe', '90d7e592b', None),
        ('a4b44040c', 'd082c5719', None),
        ('0aa0553fe', '68b3edb52', 'src/parser/SemanticAnalyzer.cpp'),
    ]
    for first_bad, fix_commit, file in fixups:
        if not git_helpers.is_ancestor(first_bad, revision.tag):
            continue
        if git_helpers.is_ancestor(fix_commit, revision.tag):
            continue

        if file:
            run_command(
                f'git show {fix_commit} -- {file} | git apply'.split(' '),
                cwd=local_build_working_dir,
                shell=True,
            )
        else:
            run_command(
                f'git cherry-pick {fix_commit} -n -m1'.split(' '),
                cwd=local_build_working_dir,
            )

    build_folder = 'build'

    arch_args = []
    if release_platform == 'windows' and not supports_64bit:
        arch_args = ['-A', 'win32']
        build_folder += '_win32'

    if release_platform == 'windows':
        if revision.commit_count >= archives.get_release_commit_count(
            '5432f8ca67bfe371ba7407838a73d455ea75131b'
        ):
            cmake_generator_args = ['-G', 'Visual Studio 17 2022']
            build_folder += '_vs2022'
        elif revision.commit_count >= archives.get_release_commit_count(
            '7ab5d08254f22c3bc6d67ae3ccaafe1e664d59d4'
        ):
            cmake_generator_args = ['-G', 'Visual Studio 16 2019']
            build_folder += '_vs2019'
        else:
            cmake_generator_args = ['-G', 'Visual Studio 15 2017']
            build_folder += '_vs2017'
    else:
        cmake_generator_args = ['-G', 'Ninja Multi-Config']

    cmake_help = subprocess.check_output(['cmake', '--help'], encoding='utf-8')
    cmake_gen = cmake_generator_args[1]
    if cmake_gen not in cmake_help:
        err = f'Cannot configure with {cmake_gen}, as it is missing.'
        if release_platform == 'windows':
            err += ' Install it from https://visualstudio.microsoft.com/vs/older-downloads/ . Be sure to select the "C++ Desktop" addon, otherwise you won\'t get MSVC'
            if '2017' in cmake_gen:
                err += '\nYou will also need to install the "C++ MFC" component, otherwise afxres.h will not be found'
        print(err)
        raise Exception(err)

    if (local_build_working_dir / build_folder / 'Release').exists():
        shutil.rmtree(local_build_working_dir / build_folder / 'Release')

    print('configuring ...')
    args = [
        'cmake',
        '-B',
        build_folder,
        '-S',
        '.',
        '-DCMAKE_BUILD_TYPE=Release',
        *arch_args,
        *cmake_generator_args,
    ]
    run_command(args, cwd=local_build_working_dir)

    try:
        run_command(
            'cp src/metadata/*.h.sig src/metadata/sigs/'.split(' '),
            cwd=local_build_working_dir,
        )
    except:
        pass

    print('building ...')
    args = [
        'cmake',
        '--build',
        build_folder,
        '--config',
        'Release',
        '--parallel',
    ]
    run_command(args, cwd=local_build_working_dir)

    print('packaging ...')
    bat_script = local_build_working_dir / 'output/_auto/buildpack.bat'
    if how_to_package == 'bat':
        package_dir = local_build_working_dir / 'output/_auto/buildpack'
        if package_dir.exists():
            shutil.rmtree(package_dir)
            package_dir.mkdir()
        subprocess.check_call(
            f'echo N | "{bat_script}"',
            cwd=local_build_working_dir / 'output/_auto',
            shell=True,
        )
        for path in (local_build_working_dir / build_folder / 'Release').glob('*'):
            shutil.copyfile(path, package_dir / path.name)
    elif how_to_package == 'cp':
        package_dir = root_dir / '.tmp/local_build_package_dir'
        if package_dir.exists():
            shutil.rmtree(package_dir)
        package_dir.mkdir(parents=True)

        cp_as_folder = ['docs', 'themes', 'include', 'modules']
        cp_as_files = ['common', 'package', 'config']

        for name in cp_as_folder:
            if (local_build_working_dir / 'output' / name).exists():
                run_command(
                    f'cp -r output/{name} {package_dir}/{name}'.split(' '),
                    cwd=local_build_working_dir,
                )
        for name in cp_as_files:
            p = local_build_working_dir / 'output' / name
            if p.exists():
                if p.is_dir():
                    run_command(
                        f'cp -r output/{name}/* {package_dir}'.split(' '),
                        cwd=local_build_working_dir,
                    )
                else:
                    run_command(
                        f'cp output/{name} {package_dir}'.split(' '),
                        cwd=local_build_working_dir,
                    )
        for path in (local_build_working_dir / build_folder / 'Release').glob('*'):
            if path.suffix in ['.dll', '.exe']:
                shutil.copyfile(path, package_dir / path.name)
        for path in local_build_working_dir.glob('bin/win32/*.dll'):
            if 'debug' not in path.name:
                shutil.copyfile(path, package_dir / path.name)
        for path in local_build_working_dir.glob('libs/win32/*.dll'):
            if 'debug' not in path.name:
                shutil.copyfile(path, package_dir / path.name)

        if 'win32/alleg44.lib' in (
            local_build_working_dir / 'CMakeLists.txt'
        ).read_text('utf-8'):
            need = 'alleg44.dll'
            rm(package_dir / 'alleg42.dll')
        else:
            need = 'alleg42.dll'
            rm(package_dir / 'alleg44.dll')
        if not (package_dir / need).exists():
            most_recent_rev = get_most_recent_rev(revision, release_platform)
            if not most_recent_rev:
                raise Exception('could not find recent build to steal files from')
            rls_dir = archives.download(most_recent_rev, release_platform)
            shutil.copyfile(rls_dir / need, package_dir / need)

        # Modules require tons of handholding to get right. Oh, and at some point the files needed to run the program were no longer
        # included in the source tree. Best we can do is grab the most recent known official build's files.
        # Sigh ...
        if (package_dir / 'modules/classic/classic').exists():
            run_command(
                'mv modules/classic modules/tmp && mv modules/tmp/* modules'.split(' '),
                cwd=package_dir,
                shell=True,
            )
            (package_dir / 'modules/tmp').rmdir()
        if (package_dir / 'modules/default/default').exists():
            run_command(
                'mv modules/default modules/tmp && mv modules/tmp/* modules'.split(' '),
                cwd=package_dir,
                shell=True,
            )
            (package_dir / 'modules/tmp').rmdir()

        zc_cfg_path = package_dir / 'zc.cfg'
        if zc_cfg_path.exists():
            zc_cfg_lines = zc_cfg_path.read_text().splitlines()
            zc_cfg_module = next(
                (l for l in zc_cfg_lines if l.startswith('current_module')), None
            )
            if zc_cfg_module:
                module_path = zc_cfg_module.split('=')[1].strip()
                if not module_path.startswith('modules/'):
                    for zmod_path in (package_dir / 'modules').rglob('*.zmod'):
                        shutil.move(
                            package_dir / 'modules' / zmod_path,
                            package_dir / zmod_path.name,
                        )

                if (package_dir / module_path).exists():
                    mod_text = (package_dir / module_path).read_text('utf-8')
                    needs_to_steal = False
                    if (
                        'modules/classic/classic_1st.qst' in mod_text
                        and not (
                            package_dir / 'modules/classic/classic_1st.qst'
                        ).exists()
                    ):
                        needs_to_steal = True
                    if (
                        'modules/classic/classic_fonts.dat' in mod_text
                        and not (
                            package_dir / 'modules/classic/classic_fonts.dat'
                        ).exists()
                    ):
                        needs_to_steal = True
                else:
                    needs_to_steal = True

                if needs_to_steal:
                    most_recent_rev = get_most_recent_rev(revision, release_platform)
                    if not most_recent_rev:
                        raise Exception(
                            'could not find recent build to steal files from'
                        )
                    rls_dir = archives.download(most_recent_rev, release_platform)
                    shutil.copytree(
                        rls_dir / 'modules', package_dir / 'modules', dirs_exist_ok=True
                    )

        # Fullscreen mode in older builds is often broken on modern Windows.
        for path in ['zc.cfg', 'zquest.cfg']:
            path = package_dir / path
            if path.exists():
                txt = path.read_text('utf-8').replace(
                    'fullscreen = 1', 'fullscreen = 0'
                )
                path.write_text(txt)
            else:
                if path.name == 'zc.cfg':
                    path.write_text('[zeldadx]\nfullscreen = 0')
                elif path.name == 'zquest.cfg':
                    path.write_text('[zquest]\nfullscreen = 0')

    elif how_to_package == 'package.py':
        args = [
            'cmake',
            '--build',
            build_folder,
            '--config',
            'Release',
            '-t',
            'package',
        ]
        run_command(args, cwd=local_build_working_dir)
        package_dir = local_build_working_dir / build_folder / 'Release/packages/zc'
    else:
        raise Exception('unknown package strategy: ' + how_to_package)

    placeholders = ['docs/shield_block_flags.txt']

    if revision.commit_count < archives.get_release_commit_count(
        'a7bc1e8f88cd283c9d4dbba39520fe4f23295c59'
    ):
        placeholders.extend(['1st.qst', '2nd.qst', '3rd.qst', '4th.qst', '5th.qst'])

        if (
            not (package_dir / 'zelda.dat').exists()
            or not (package_dir / '1st.qst').exists()
        ):
            most_recent_rev = get_most_recent_rev(revision, release_platform)
            if most_recent_rev:
                rls_dir = archives.download(most_recent_rev, release_platform)
                for path in (rls_dir / 'modules/classic').glob('*.*'):
                    shutil.copyfile(
                        path, package_dir / path.name.replace('classic_', '')
                    )

    for path in placeholders:
        if not (package_dir / path).exists():
            (package_dir / path).parent.mkdir(parents=True, exist_ok=True)
            (package_dir / path).write_text('PLACEHOLDER')

    if (package_dir / 'Addons').exists():
        shutil.rmtree(package_dir / 'Addons')

    readme_txt = (
        'This build is meant for testing only. It is likely to be broken in some way.\n'
    )
    if release_platform == 'windows':
        arch = 'x64' if supports_64bit else 'x86'
        readme_txt += f'windows-{arch}\n'
    (package_dir / 'README_test_build_only.txt').write_text(readme_txt)

    if dest.exists():
        shutil.rmtree(dest)
    dest.parent.mkdir(exist_ok=True)
    shutil.copytree(package_dir, dest)

    print('sanity checking binaries ...')
    try_folder = root_dir / '.tmp/local_builds_try'
    if try_folder.exists():
        shutil.rmtree(try_folder)
    shutil.copytree(package_dir, try_folder)
    binaries = archives.create_binary_paths(try_folder, release_platform)
    kill_console = lambda: subprocess.run(
        ['taskkill', '/F', '/IM', 'ZConsole.exe'],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    kill_console()
    try_run([binaries['zc']], cwd=try_folder, timeout=1)
    kill_console()
    try_run([binaries['zq']], cwd=try_folder, timeout=1)

    return dest


def backfill(release_platform: str):
    if release_platform != 'windows':
        raise Exception('not supported')

    step = 20
    last = None
    skip_until = None
    failures_in_row = 0
    revs = archives.get_local_builds()
    # Skip revisions without a modules folder for now.
    revs = [
        r
        for r in revs
        if r.commit_count
        >= archives.get_release_commit_count('a7bc1e8f88cd283c9d4dbba39520fe4f23295c59')
    ]
    # Skip the oldest commits for now.
    revs = [r for r in revs if r.commit_count >= 1605]
    for rev in revs:
        if not rev.is_local_build or has_built_locally(rev):
            failures_in_row = 0
            last = rev.commit_count
            skip_until = None
            continue

        if skip_until != None:
            if skip_until > rev.commit_count:
                continue
            skip_until = None

        if last != None and rev.commit_count - last < step:
            continue

        if not local_build_error(rev):
            try:
                build_locally(rev)
            except KeyboardInterrupt:
                exit(1)
            except Exception as e:
                ex = "".join(traceback.format_exception_only(e)).strip()
                print('FAIL', rev)
                print(ex)
                rev.dir().mkdir(exist_ok=True, parents=True)
                (rev.dir() / 'error.txt').write_text(ex)

        if local_build_error(rev):
            failures_in_row += 1
            skip_until = rev.commit_count + 5 * failures_in_row
        else:
            failures_in_row = 0
            last = rev.commit_count

    largest_gap = 0
    largest_gap_revs = []
    last_rev = revs[0]
    for rev in revs:
        if has_built_locally(rev) or not rev.is_local_build:
            gap = rev.commit_count - last_rev.commit_count
            if gap > largest_gap:
                largest_gap = gap
                largest_gap_revs = [last_rev, rev]
            last_rev = rev

    print('largest gap in revisions:')
    print(largest_gap, largest_gap_revs)
    for rev in revs:
        if (
            largest_gap_revs[0].commit_count
            < rev.commit_count
            < largest_gap_revs[1].commit_count
        ):
            error_path = local_build_error(rev)
            if error_path:
                print(error_path)

    local_archive_dir = root_dir / '.tmp/local_archives'
    print(f'zipping to .tmp/local_archives ...')
    for rev in revs:
        if not has_built_locally(rev):
            continue

        print(rev)
        package_dir = build_locally(rev)
        variant_name = (
            (package_dir / 'README_test_build_only.txt').read_text().splitlines()[1]
        )
        archive_path = local_archive_dir / rev.tag / f'{variant_name}.zip'
        archive_path.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(archive_path, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
            for dirpath, dirnames, filenames in os.walk(package_dir):
                for cur_filename in filenames:
                    cur_filepath = os.path.join(dirpath, cur_filename)
                    rel_path = os.path.relpath(cur_filepath, package_dir)
                    zf.write(cur_filepath, arcname=rel_path)
