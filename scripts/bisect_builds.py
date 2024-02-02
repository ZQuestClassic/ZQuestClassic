# To run a bisect:
#   python scripts/bisect_builds.py --token $GH_PAT --good 2.55-alpha-108 --bad 2.55-alpha-109
#   python scripts/bisect_builds.py --token $GH_PAT --bad 2.55-alpha-108 --good 2.55-alpha-109
#
# You can automate running a command on each bisect script, like this:
#   -c '%zq'
#   -c '%zc -test "quests/Z1 Recreations/classic_1st.qst" 0 119'
#
# To download a specific build:
#   python scripts/bisect_builds.py --token $GH_PAT --download_release 2.55-alpha-109
#
# Use the '--local_builds' flag to build additional commits locally. This will take much longer, so
# run without first to get a more narrow range.

import argparse
import os
import io
import subprocess
import platform
import requests
import zipfile
import tarfile
import traceback
import shutil
from dataclasses import dataclass
from joblib import Memory
from typing import List
from pathlib import Path
from github import Github

parser = argparse.ArgumentParser(
    description='Runs a bisect using prebuilt releases.')
parser.add_argument('--good')
parser.add_argument('--bad')
parser.add_argument('--token', required=True)
parser.add_argument(
    '--local_builds', action=argparse.BooleanOptionalAction, default=False,
    help='Includes all commits and builds locally if prebuilt binaries are not present. Uses a temporary checkout at .tmp/local_build_working_dir')
parser.add_argument('--list_releases', action='store_true')
parser.add_argument('--download_release', help='Downloads the given release tag')
parser.add_argument('--channel')
parser.add_argument('--single', help='Runs `--command` for the given version (can be release tag or commit sha), downloading or building as necessary')
parser.add_argument('-c', '--command',
                    help='command to run on each step. \'%%zc\' is replaced with the path to the player, \'%%zq\' is the editor, and \'%%zl\' is the launcher')
parser.add_argument('--check_return_code',
                    action=argparse.BooleanOptionalAction, default=False)
parser.add_argument('--backfill_local_builds',
                    action=argparse.BooleanOptionalAction, default=False)

args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
releases_dir = root_dir / '.tmp/releases'
local_builds_dir = root_dir / '.tmp/local_builds'
memory = Memory(root_dir / '.tmp/bisect_builds', verbose=0)

gh = Github(args.token)
repo = gh.get_repo('ZQuestClassic/ZQuestClassic')

system = platform.system()
if args.channel:
    channel = args.channel
elif system == 'Darwin':
    channel = 'mac'
elif system == 'Windows':
    channel = 'windows'
elif system == 'Linux':
    channel = 'linux'
else:
    raise Exception(f'unexpected system: {system}')

commit_counts = {}
shas = subprocess.check_output(
    ['git', 'log', '--format=format:%H', '--reverse', '--first-parent'], encoding='utf-8').splitlines()
for i, sha in enumerate(shas):
    commit_counts[sha] = i

@dataclass
class Revision:
    tag: str  # also could be a hash
    commit_count: int
    is_local_build = False


@memory.cache
def git_rev_parse(commitish: str):
    return subprocess.check_output(['git', 'rev-parse', commitish], encoding='utf-8').strip()


@memory.cache
def git_is_ancestor(maybe_ancestor_sha: str, sha: str):
    return subprocess.call(['git', 'merge-base', '--is-ancestor', maybe_ancestor_sha, sha], encoding='utf-8') == 0


@memory.cache
def git_tag_to_sha_on_main(tag: str):
    args = f'git rev-list {tag}..main --ancestry-path --merges --reverse'.split(' ')
    out = subprocess.check_output(args, encoding='utf-8')
    lines = out.splitlines()
    if not lines:
        return git_rev_parse(tag)
    return lines[0]


def get_release_commit_count(sha: str):
    if sha not in commit_counts:
        raise Exception(f'no commit count found: {sha}')
    return commit_counts[sha]


@memory.cache
def get_release_commit_count_of_tag(tag: str):
    sha = git_rev_parse(tag)
    try:
        return get_release_commit_count(sha)
    except:
        pass

    sha = git_tag_to_sha_on_main(tag)
    try:
        return get_release_commit_count(sha)
    except:
        pass

    raise Exception(f'can not find main commit for tag: {tag}')


@memory.cache
def has_release_package(tag: str):
    try:
        url = get_release_package_url(tag)
        return bool(url)
    except:
        return False


def revision_count_supports_platform(revision_count: int, platform_str: str):
    if platform_str == 'windows' and revision_count < 6252:
        return True
    if platform_str == 'mac' and revision_count < 6384:
        return False
    if platform_str == 'linux' and revision_count < 7404:
        return False
    return True


def get_releases():
    revisions: List[Revision] = []

    # TODO maybe use: git tag --merged=main 'nightly*' '2.55-alpha-???' --sort=committerdate
    tags = subprocess.check_output(
        ['git', '-P', 'tag', '--sort=committerdate', '--merged=main'], encoding='utf-8').splitlines()

    for tag in tags:
        try:
            commit_count = get_release_commit_count_of_tag(tag)
        except:
            continue

        if not revision_count_supports_platform(commit_count, channel):
            continue

        # Every release after this one will have binaries, but before only some do.
        if commit_count < get_release_commit_count_of_tag('2.55-alpha-107'):
            if not has_release_package(tag):
                continue

        revisions.append(Revision(tag, commit_count))

    return revisions


def get_local_builds(revisions = []):
    for sha, commit_count in commit_counts.items():
        if not next((r for r in revisions if r.commit_count == commit_count), None):
            r = Revision(sha, commit_count)
            r.is_local_build = True
            revisions.append(r)

    return revisions


def get_revisions(may_build_locally: bool):
    revisions = get_releases()
    if may_build_locally:
        revisions = get_local_builds(revisions)
    revisions.sort(key=lambda x: x.commit_count)
    return revisions


def get_release_package_url(tag):
    release = repo.get_release(tag)
    assets = list(release.get_assets())

    asset = None
    if channel == 'mac':
        asset = next(asset for asset in assets if asset.name.endswith('.dmg'))
    elif channel == 'windows':
        if len(assets) == 1:
            asset = assets[0]
        else:
            assets = [asset for asset in assets if 'windows' in asset.name]
            asset = next((asset for asset in assets if 'x64' in asset.name), None) or \
                next((asset for asset in assets if 'x86' in asset.name), None)
    elif channel == 'linux':
        asset = next(asset for asset in assets if asset.name.endswith(
            '.tar.gz') or asset.name.endswith('.tgz'))

    if not asset:
        raise Exception(f'could not find package url for {tag}')

    return asset.browser_download_url


def download_release(tag: str):
    url = get_release_package_url(tag)
    dest = releases_dir / tag
    if dest.exists() and list(dest.glob('*')):
        return dest

    dest.mkdir(parents=True, exist_ok=True)
    print(f'downloading release {tag}')

    r = requests.get(url)
    if channel == 'mac':
        (dest / 'ZQuestClassic.dmg').write_bytes(r.content)
        subprocess.check_call(['hdiutil', 'attach', '-mountpoint',
                              str(dest / 'zc-mounted'), str(dest / 'ZQuestClassic.dmg')], stdout=subprocess.DEVNULL)
        zc_app_path = next((dest / 'zc-mounted').glob('*.app'))
        shutil.copytree(zc_app_path, dest / zc_app_path.name)
        subprocess.check_call(['hdiutil', 'unmount', str(
            dest / 'zc-mounted')], stdout=subprocess.DEVNULL)
        (dest / 'ZQuestClassic.dmg').unlink()
    elif url.endswith('.tar.gz'):
        tf = tarfile.open(fileobj=io.BytesIO(r.content), mode='r')
        tf.extractall(dest, filter='data')
        tf.close()
    else:
        zip = zipfile.ZipFile(io.BytesIO(r.content))
        zip.extractall(dest)
        zip.close()

    print(f'finished downloading {tag}')
    return dest


def find_path(dir: Path, one_of: List[str]):
    for p in one_of:
        if (dir / p).exists():
            return dir / p

    raise Exception(f'could not find one of {one_of} in {dir}')


def create_binary_paths(dir: Path):
    binaries = {'dir': dir}

    if channel == 'mac':
        zc_app_path = next(dir.glob('*.app'))
        binaries['zc'] = find_path(zc_app_path / 'Contents/Resources', ['zplayer', 'zelda'])
        binaries['zq'] = find_path(zc_app_path / 'Contents/Resources', ['zeditor', 'zquest'])
        binaries['zl'] = zc_app_path / 'Contents/MacOS/zlauncher'
    elif channel == 'windows':
        binaries['zc'] = find_path(dir, ['zplayer.exe', 'zelda.exe'])
        binaries['zq'] = find_path(dir, ['zeditor.exe', 'zquest.exe'])
        binaries['zl'] = dir / 'zlauncher.exe'
    elif channel == 'linux':
        binaries['zc'] = find_path(dir, ['zplayer', 'zelda'])
        binaries['zq'] = find_path(dir, ['zeditor', 'zquest'])
        binaries['zl'] = dir / 'zlauncher'

    return binaries



def run_command(args: List[str], **kwargs):
    p = subprocess.run(args, capture_output=True, encoding='utf-8', **kwargs)
    if p.returncode != 0:
        cmd = ' '.join(args)
        raise Exception(f'Failed command {cmd}\n\n{p.stdout}')


def local_build_error(revision: Revision):
    if not revision.is_local_build:
        return False

    sha = revision.tag
    dest: Path = local_builds_dir / sha
    if not dest.exists():
        return False

    if (dest/'error.txt').exists():
        return dest/'error.txt'

    return False
    

def has_built_locally(revision: Revision):
    if not revision.is_local_build:
        return False
    
    sha = revision.tag
    dest: Path = local_builds_dir / sha
    if not dest.exists():
        return False

    if (dest/'error.txt').exists():
        return False

    return list(dest.glob('*'))


def try_run(args, **kwargs):
    try:
        subprocess.check_output(args, **kwargs)
    except subprocess.TimeoutExpired:
        return
    except Exception as e:
        ex = "".join(traceback.format_exception_only(e)).strip()
        log = kwargs['cwd']/'allegro.log'
        if log.exists():
            ex += '\n' + log.read_text('utf-8')
        raise Exception(ex)


def get_most_recent_rev(revision: Revision):
    most_recent_rev = None
    for rev in get_revisions(False):
        if rev.is_local_build:
            continue
        if rev.commit_count >= revision.commit_count:
            return rev
        if most_recent_rev == None or most_recent_rev.commit_count < rev.commit_count:
            most_recent_rev = rev
    return most_recent_rev


def build_locally(revision: Revision):
    """
    Attempts to build any commit of the repo. Here be dragons.

    ZC has a long history of not caring about reproducible builds. There are many, many pitfalls to handle.
    """

    def rm(path: Path):
        if path.exists():
            path.unlink()

    sha = revision.tag
    dest: Path = local_builds_dir / sha
    if local_build_error(revision) or has_built_locally(revision):
        return dest

    print(f'building locally: {revision}')
    local_build_working_dir = root_dir / '.tmp/local_build_working_dir'

    # Not sure this can be cherry-picked within this range, so just skip for now.
    if get_release_commit_count('0f162d21d47cec49da6276120959be8393c4f80a') <= revision.commit_count < get_release_commit_count('68fd510de75f14b57221e677c7b361df3dee493c'):
        raise Exception('cannot build this range')

    # Don't know how to build before cmake.
    if revision.commit_count < get_release_commit_count('43313872ea176fa01a015bc09b30cbb6ea638fde'):
        raise Exception('cannot build makefile')

    if not local_build_working_dir.exists():
        local_build_working_dir.mkdir(parents=True)
        print('cloning repo ... this will take awhile')
        run_command(['git', 'clone', 'https://github.com/ZQuestClassic/ZQuestClassic.git', local_build_working_dir])
    else:
        run_command(['git', 'fetch'], cwd=local_build_working_dir)

    run_command(f'git checkout -- .'.split(' '), cwd=local_build_working_dir)
    run_command(f'git clean -fd'.split(' '), cwd=local_build_working_dir)
    run_command(f'git checkout -f {sha}'.split(' '), cwd=local_build_working_dir)

    # TODO Before cmake, the makefile was garbage ...
    if not (local_build_working_dir/'CMakeLists.txt').exists():
        raise Exception('too old')

    supports_64bit = 'libs/win32' not in (local_build_working_dir/'CMakeLists.txt').read_text()
    if not supports_64bit:
        if not channel == 'windows':
            raise Exception('unsupported')

    how_to_package = None
    if (local_build_working_dir/'scripts/package.py').exists():
        how_to_package = 'package.py'
    elif (local_build_working_dir/'output/_auto/buildpack.bat').exists():
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
        if not git_is_ancestor(first_bad, revision.tag):
            continue
        if git_is_ancestor(fix_commit, revision.tag):
            continue

        if file:
            run_command(f'git show {fix_commit} -- {file} | git apply'.split(' '), cwd=local_build_working_dir, shell=True)
        else:
            run_command(f'git cherry-pick {fix_commit} -n -m1'.split(' '), cwd=local_build_working_dir)

    build_folder = 'build'

    arch_args = []
    if channel == 'windows' and not supports_64bit:
        arch_args = ['-A', 'win32']
        build_folder += '_win32'

    if revision.commit_count >= get_release_commit_count('5432f8ca67bfe371ba7407838a73d455ea75131b'):
        cmake_generator_args = ['-G', 'Visual Studio 17 2022']
        build_folder += '_vs2022'
    elif revision.commit_count >= get_release_commit_count('7ab5d08254f22c3bc6d67ae3ccaafe1e664d59d4'):
        cmake_generator_args = ['-G', 'Visual Studio 16 2019']
        build_folder += '_vs2019'
    else:
        cmake_generator_args = ['-G', 'Visual Studio 15 2017']
        build_folder += '_vs2017'

    cmake_help = subprocess.check_output('cmake --help', encoding='utf-8')
    cmake_gen = cmake_generator_args[1]
    if cmake_gen not in cmake_help:
        err = f'Cannot configure with {cmake_gen}, as it is missing. Install it from https://visualstudio.microsoft.com/vs/older-downloads/ . Be sure to select the "C++ Desktop" addon, otherwise you won\'t get MSVC'
        if '2017' in cmake_gen:
            err += '\nYou will also need to install the "C++ MFC" component, otherwise afxres.h will not be found'
        print(err)
        raise Exception(err)

    if (local_build_working_dir/build_folder/'Release').exists():
        shutil.rmtree(local_build_working_dir/build_folder/'Release')

    print('configuring ...')
    args = [
        'cmake',
        '-B', build_folder,
        '-S', '.',
        '-DCMAKE_BUILD_TYPE=Release',
        *arch_args,
        *cmake_generator_args,
    ]
    run_command(args, cwd=local_build_working_dir)

    try:
        run_command('cp src/metadata/*.h.sig src/metadata/sigs/'.split(' '), cwd=local_build_working_dir)
    except:
        pass

    print('building ...')
    args = [
        'cmake',
        '--build', build_folder,
        '--config', 'Release',
        '--parallel',
    ]
    run_command(args, cwd=local_build_working_dir)

    print('packaging ...')
    bat_script = local_build_working_dir / 'output/_auto/buildpack.bat'
    if how_to_package == 'bat':
        package_dir = local_build_working_dir/'output/_auto/buildpack'
        if package_dir.exists():
            shutil.rmtree(package_dir)
            package_dir.mkdir()
        subprocess.check_call(f'echo N | "{bat_script}"', cwd=local_build_working_dir/'output/_auto', shell=True)
        for path in (local_build_working_dir/build_folder/'Release').glob('*'):
            shutil.copyfile(path, package_dir/path.name)
    elif how_to_package == 'cp':
        package_dir = root_dir/'.tmp/local_build_package_dir'
        if package_dir.exists():
            shutil.rmtree(package_dir)
        package_dir.mkdir(parents=True)

        cp_as_folder = ['docs', 'themes', 'include', 'modules']
        cp_as_files = ['common', 'package', 'config']

        for name in cp_as_folder:
            if (local_build_working_dir/'output'/name).exists():
                run_command(f'cp -r output/{name} {package_dir}/{name}'.split(' '), cwd=local_build_working_dir)
        for name in cp_as_files:
            p = local_build_working_dir/'output'/name
            if p.exists():
                if p.is_dir():
                    run_command(f'cp -r output/{name}/* {package_dir}'.split(' '), cwd=local_build_working_dir)
                else:
                    run_command(f'cp output/{name} {package_dir}'.split(' '), cwd=local_build_working_dir)
        for path in (local_build_working_dir/build_folder/'Release').glob('*'):
            if path.suffix in ['.dll', '.exe']:
                shutil.copyfile(path, package_dir/path.name)
        for path in local_build_working_dir.glob('bin/win32/*.dll'):
            if 'debug' not in path.name:
                shutil.copyfile(path, package_dir/path.name)
        for path in local_build_working_dir.glob('libs/win32/*.dll'):
            if 'debug' not in path.name:
                shutil.copyfile(path, package_dir/path.name)

        if 'win32/alleg44.lib' in (local_build_working_dir/'CMakeLists.txt').read_text('utf-8'):
            need = 'alleg44.dll'
            rm(package_dir/'alleg42.dll')
        else:
            need = 'alleg42.dll'
            rm(package_dir/'alleg44.dll')
        if not (package_dir/need).exists():
            most_recent_rev = get_most_recent_rev(revision)
            if not most_recent_rev:
                raise Exception('could not find recent build to steal files from')
            rls_dir = download_release(most_recent_rev.tag)
            shutil.copyfile(rls_dir/need, package_dir/need)

        # Modules require tons of handholding to get right. Oh, and at some point the files needed to run the program were no longer
        # included in the source tree. Best we can do is grab the most recent known official build's files.
        # Sigh ...
        if (package_dir/'modules/classic/classic').exists():
            run_command('mv modules/classic modules/tmp && mv modules/tmp/* modules'.split(' '), cwd=package_dir, shell=True)
            (package_dir/'modules/tmp').rmdir()
        if (package_dir/'modules/default/default').exists():
            run_command('mv modules/default modules/tmp && mv modules/tmp/* modules'.split(' '), cwd=package_dir, shell=True)
            (package_dir/'modules/tmp').rmdir()

        zc_cfg_path = package_dir/'zc.cfg'
        if zc_cfg_path.exists():
            zc_cfg_lines = zc_cfg_path.read_text().splitlines()
            zc_cfg_module = next((l for l in zc_cfg_lines if l.startswith('current_module')), None)
            if zc_cfg_module:
                module_path = zc_cfg_module.split('=')[1].strip()
                if not module_path.startswith('modules/'):
                    for zmod_path in (package_dir/'modules').rglob('*.zmod'):
                        shutil.move(package_dir/'modules'/zmod_path, package_dir/zmod_path.name)

                if (package_dir/module_path).exists():
                    mod_text = (package_dir/module_path).read_text('utf-8')
                    needs_to_steal = False
                    if 'modules/classic/classic_1st.qst' in mod_text and not (package_dir/'modules/classic/classic_1st.qst').exists():
                        needs_to_steal = True
                    if 'modules/classic/classic_fonts.dat' in mod_text and not (package_dir/'modules/classic/classic_fonts.dat').exists():
                        needs_to_steal = True
                else:
                    needs_to_steal = True

                if needs_to_steal:
                    most_recent_rev = get_most_recent_rev(revision)
                    if not most_recent_rev:
                        raise Exception('could not find recent build to steal files from')
                    rls_dir = download_release(most_recent_rev.tag)
                    shutil.copytree(rls_dir/'modules', package_dir/'modules', dirs_exist_ok=True)

        # Fullscreen mode in older builds is often broken on modern Windows.
        for path in ['zc.cfg', 'zquest.cfg']:
            path = package_dir/path
            if path.exists():
                txt = path.read_text('utf-8').replace('fullscreen = 1', 'fullscreen = 0')
                path.write_text(txt)
            else:
                if path.name == 'zc.cfg':
                    path.write_text('[zeldadx]\nfullscreen = 0')
                elif path.name == 'zquest.cfg':
                    path.write_text('[zquest]\nfullscreen = 0')

    elif how_to_package == 'package.py':
        args = [
            'cmake',
            '--build', build_folder,
            '--config', 'Release',
            '-t', 'package',
        ]
        run_command(args, cwd=local_build_working_dir)
        package_dir = local_build_working_dir/build_folder/'Release/packages/zc'
    else:
        raise Exception('unknown package strategy: ' + how_to_package)

    placeholders = ['docs/shield_block_flags.txt']

    if revision.commit_count < get_release_commit_count('a7bc1e8f88cd283c9d4dbba39520fe4f23295c59'):
        placeholders.extend(['1st.qst', '2nd.qst', '3rd.qst', '4th.qst', '5th.qst'])

        if not (package_dir/'zelda.dat').exists() or not (package_dir/'1st.qst').exists():
            most_recent_rev = get_most_recent_rev(revision)
            if most_recent_rev:
                rls_dir = download_release(most_recent_rev.tag)
                for path in (rls_dir/'modules/classic').glob('*.*'):
                    shutil.copyfile(path, package_dir/path.name.replace('classic_', ''))

    for path in placeholders:
        if not (package_dir/path).exists():
            (package_dir/path).parent.mkdir(parents=True, exist_ok=True)
            (package_dir/path).write_text('PLACEHOLDER')
    
    if (package_dir/'Addons').exists():
        shutil.rmtree(package_dir/'Addons')

    readme_txt = 'This build is meant for testing only. It is likely to be broken in some way.\n'
    if channel == 'windows':
        arch = 'x64' if supports_64bit else 'x86'
        readme_txt += f'windows-{arch}\n'
    (package_dir/'README_test_build_only.txt').write_text(readme_txt)

    if dest.exists():
        shutil.rmtree(dest)
    dest.parent.mkdir(exist_ok=True)
    shutil.copytree(package_dir, dest)

    print('sanity checking binaries ...')
    try_folder = root_dir/'.tmp/local_builds_try'
    if try_folder.exists():
        shutil.rmtree(try_folder)
    shutil.copytree(package_dir, try_folder)
    binaries = create_binary_paths(try_folder)
    kill_console = lambda: subprocess.run(['taskkill', '/F', '/IM', 'ZConsole.exe'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    kill_console()
    try_run([binaries['zc']], cwd=try_folder, timeout=1)
    kill_console()
    try_run([binaries['zq']], cwd=try_folder, timeout=1)

    return dest


def get_revision_binaries(revision: Revision):
    if revision.is_local_build:
        try:
            dir = build_locally(revision)
        except KeyboardInterrupt:
            exit(1)
        except:
            return None
    else:
        dir = download_release(revision.tag)

    return create_binary_paths(dir)


def AskIsGoodBuild():
    while True:
        prompt = ('Revision is '
                  '[(g)ood/(b)ad/(u)nknown/(q)uit]: ')
        response = input(prompt)
        if response in ('g', 'b', 'u', 'q'):
            return response


def run_bisect(revisions: List[Revision]):
    tags = [r.tag for r in revisions]
    if args.bad not in tags:
        raise Exception(f'did not find release {args.bad}')
    if args.good not in tags:
        raise Exception(f'did not find release {args.good}')

    bad_rev = tags.index(args.bad)
    good_rev = tags.index(args.good)
    lower_rev = min(good_rev, bad_rev)
    upper_rev = max(good_rev, bad_rev)
    revs = revisions[lower_rev:upper_rev+1]
    lower = 0
    upper = len(revs) - 1
    pivot = upper // 2
    rev = revs[pivot]
    skipped = []
    goods = [upper if good_rev > bad_rev else 0]
    bads = [upper if good_rev <= bad_rev else 0]

    while upper - lower > 1:
        if pivot in skipped:
            for i in range(0, upper - lower):
                def check(k):
                    if k <= lower or k >= upper:
                        return False
                    return k not in skipped

                if check(pivot - i):
                    print(pivot - i)
                    pivot = pivot - i
                    rev = revs[pivot]
                    break
                if check(pivot + i):
                    print(pivot + i)
                    pivot = pivot + i
                    rev = revs[pivot]
                    break
        if pivot in skipped:
            print('skipped all options')
            break

        if bad_rev < good_rev:
            min_str, max_str = 'bad', 'good'
        else:
            min_str, max_str = 'good', 'bad'
        print(
            '=== Bisecting range [%s (%s), %s (%s)], '
            'roughly %d steps left.' % (revs[lower].tag, min_str, revs[upper].tag,
                                        max_str, int(upper - lower).bit_length()))

        lower_tag = revs[lower].tag
        upper_tag = revs[upper].tag
        print(
            f'changelog of current range: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}')

        print(f'checking {rev.tag}')
        binaries = get_revision_binaries(rev)

        down_pivot = int((pivot - lower) / 2) + lower
        up_pivot = int((upper - pivot) / 2) + pivot

        if not binaries:
            answer = 'u'
        elif args.command:
            cmd = args.command
            cmd = cmd.replace('%zc', f'"{binaries["zc"]}"')
            cmd = cmd.replace('%zq', f'"{binaries["zq"]}"')
            cmd = cmd.replace('%zl', f'"{binaries["zl"]}"')
            print(f'running command: {cmd}')
            p = subprocess.Popen(cmd, cwd=binaries['dir'], shell=system != 'Windows')
            if args.check_return_code:
                retcode = p.communicate()
                answer = 'g' if retcode == 0 else 'b'
                print(f'code: {retcode}, answer: {answer}')
            else:
                answer = AskIsGoodBuild()
                p.terminate()
        else:
            answer = AskIsGoodBuild()

        if answer == 'q':
            raise SystemExit()

        if answer == 'g':
            goods.append(pivot)
        elif answer == 'b':
            bads.append(pivot)
        if (answer == 'g' and good_rev < bad_rev) or (answer == 'b' and bad_rev < good_rev):
            lower = pivot
            pivot = up_pivot
        elif (answer == 'b' and good_rev < bad_rev) or (answer == 'g' and bad_rev < good_rev):
            upper = pivot
            pivot = down_pivot
        elif answer == 'u':
            skipped.append(pivot)

        rev = revs[pivot]

    DONE_MESSAGE_GOOD_MIN = ('You are probably looking for a change made after %s ('
                             'known good), but no later than %s (first known bad).')
    DONE_MESSAGE_GOOD_MAX = ('You are probably looking for a change made after %s ('
                             'known bad), but no later than %s (first known good).')
    print('bisect finished!\n')

    for i in range(lower, upper + 1):
        state = ' ' * 7
        if i in skipped:
            state = 'SKIPPED'
        if i in goods:
            state = 'GOOD   '
        if i in bads:
            state = 'BAD    '
        print(state, revs[i].tag)
    print()

    lower_tag = revs[lower].tag
    upper_tag = revs[upper].tag
    if good_rev > bad_rev:
        print(DONE_MESSAGE_GOOD_MAX % (lower_tag, upper_tag))
    else:
        print(DONE_MESSAGE_GOOD_MIN % (lower_tag, upper_tag))

    print(
        f'changelog: https://github.com/ZQuestClassic/ZQuestClassic/compare/{lower_tag}...{upper_tag}')


if args.download_release:
    download_release(args.download_release)
    exit(0)

if args.list_releases:
    releases = get_releases()
    for release in releases:
        print(f'@{release.commit_count} {release.tag}')
    exit(0)

# Clear all builds that failed.
# for rev in get_revisions(True):
#     if local_build_error(rev):
#         shutil.rmtree(local_builds_dir/rev.tag)
# exit(0)

# Build a specific revision.
# revs = get_revisions(True)
# for sha in ['697aca5ea4787bd42c014292a90d624ad719f6cc']:
#     rev = next(r for r in revs if r.tag == sha)
#     if local_build_error(rev):
#        shutil.rmtree(local_builds_dir/rev.tag)
#     try:
#         build_locally(rev)
#     except KeyboardInterrupt:
#         exit(1)
#     except Exception as e:
#         ex = "".join(traceback.format_exception_only(e)).strip()
#         print('FAIL', rev)
#         print(ex)
#         (local_builds_dir/rev.tag).mkdir(exist_ok=True, parents=True)
#         (local_builds_dir/rev.tag/'error.txt').write_text(ex)
# exit(0)

if args.backfill_local_builds:
    if channel != 'windows':
        raise Exception('not supported')

    step = 20
    last = None
    skip_until = None
    failures_in_row = 0
    revs = get_revisions(may_build_locally=True)
    # Skip revisions without a modules folder for now.
    revs = [r for r in revs if r.commit_count >= get_release_commit_count('a7bc1e8f88cd283c9d4dbba39520fe4f23295c59')]
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
                (local_builds_dir/rev.tag).mkdir(exist_ok=True, parents=True)
                (local_builds_dir/rev.tag/'error.txt').write_text(ex)

        if local_build_error(rev):
            failures_in_row += 1
            skip_until = rev.commit_count + 5*failures_in_row
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
        if largest_gap_revs[0].commit_count < rev.commit_count < largest_gap_revs[1].commit_count:
            dest: Path = local_builds_dir / sha
            error_path = local_build_error(rev)
            if error_path:
                print(error_path)

    local_archive_dir = root_dir/'.tmp/local_archives'
    print(f'zipping to .tmp/local_archives ...')
    for rev in revs:
        if not has_built_locally(rev):
            continue

        print(rev)
        package_dir = build_locally(rev)
        variant_name = (package_dir/'README_test_build_only.txt').read_text().splitlines()[1]
        archive_path = local_archive_dir/rev.tag/f'{variant_name}.zip'
        archive_path.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(archive_path, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
            for dirpath, dirnames, filenames in os.walk(package_dir):
                for cur_filename in filenames:
                    cur_filepath = os.path.join(dirpath, cur_filename)
                    rel_path = os.path.relpath(cur_filepath, package_dir)
                    zf.write(cur_filepath, arcname=rel_path)

    exit(0)

if args.single:
    revisions = get_revisions(may_build_locally=True)
    revision = next(r for r in revisions if r.tag == args.single)
    binaries = get_revision_binaries(revision)
    if not binaries:
        raise Exception('failed to find revision')

    cmd = args.command
    cmd = cmd.replace('%zc', f'"{binaries["zc"]}"')
    cmd = cmd.replace('%zq', f'"{binaries["zq"]}"')
    cmd = cmd.replace('%zl', f'"{binaries["zl"]}"')
    print(f'running command: {cmd}')
    p = subprocess.Popen(cmd, cwd=binaries['dir'], shell=system != 'Windows')
    if args.check_return_code:
        retcode = p.communicate()
        exit(retcode)
    else:
        p.terminate()
        exit(0)


revisions = get_revisions(may_build_locally=args.local_builds or args.backfill_local_builds)
run_bisect([r for r in revisions if not local_build_error(r)])
