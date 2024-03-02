import platform
import subprocess

from pathlib import Path
from typing import List


def get_channel():
    system = platform.system()
    if system == 'Darwin':
        return 'mac'
    elif system == 'Windows':
        return 'windows'
    elif system == 'Linux':
        return 'linux'
    else:
        raise Exception(f'unexpected system: {system}')


def run_zc_command(binaries, command):
    cwd = binaries['dir']
    if (cwd / 'zc.sav').exists():
        (cwd / 'zc.sav').unlink()

    cmd = command
    cmd = cmd.replace('%zc', f'"{binaries["zc"]}"')
    cmd = cmd.replace('%zq', f'"{binaries["zq"]}"')
    cmd = cmd.replace('%zl', f'"{binaries["zl"]}"')
    print(f'running command: {cmd}')
    return subprocess.Popen(cmd, cwd=cwd, shell=platform.system() != 'Windows')


def find_path(dir: Path, one_of: List[str]):
    for p in one_of:
        if (dir / p).exists():
            return dir / p

    raise Exception(f'could not find one of {one_of} in {dir}')
