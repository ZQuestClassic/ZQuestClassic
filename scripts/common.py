import platform
import shlex
import subprocess

from pathlib import Path
from typing import List


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
        arg = arg.replace('%zc', f'"{binaries["zc"]}"')
        arg = arg.replace('%zq', f'"{binaries["zq"]}"')
        arg = arg.replace('%zl', f'"{binaries["zl"]}"')
        arg = arg.replace('%zs', f'"{binaries["zs"]}"')
        args2.append(arg)

    print(f'running command: {shlex.join(args2)}')
    shell = True if platform.system() == 'Windows' else args2[0] != 'bash'
    return subprocess.Popen(args2, cwd=cwd, shell=shell)


def find_path(dir: Path, one_of: List[str]):
    for p in one_of:
        if (dir / p).exists():
            return dir / p

    raise Exception(f'could not find one of {one_of} in {dir}')
