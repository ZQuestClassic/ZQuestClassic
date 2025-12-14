import platform
import shlex
import subprocess

from pathlib import Path


def get_release_platform() -> ['mac', 'windows', 'linux']:
    system = platform.system()
    if system == 'Darwin':
        return 'mac'
    elif system == 'Windows':
        return 'windows'
    elif system == 'Linux':
        return 'linux'
    else:
        raise Exception(f'unexpected system: {system}')


def run_zc_command(binaries, args):
    cwd = binaries['dir']
    if (cwd / 'zc.sav').exists():
        (cwd / 'zc.sav').unlink()

    args2 = []
    for arg in args:
        arg = arg.replace('%zc', str(binaries["zc"]))
        arg = arg.replace('%zq', str(binaries["zq"]))
        arg = arg.replace('%zl', str(binaries["zl"]))
        arg = arg.replace('%zs', str(binaries["zs"]))
        args2.append(arg)

    print(f'running command: {shlex.join(args2)}')
    return subprocess.Popen(args2, cwd=cwd, shell=False)


def find_path(dir: Path, one_of: list[str], missing_ok=False):
    for p in one_of:
        if (dir / p).exists():
            return dir / p

    if not missing_ok:
        raise Exception(f'could not find one of {one_of} in {dir}')
