import os
import subprocess
from typing import List, Optional
from pathlib import Path
import sys
import platform
import shutil

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

DEBUG_METHOD_NONE = 0
DEBUG_METHOD_WRAP = 1
DEBUG_METHOD_CORE = 2


def get_debug_method():
    debug_method = _get_debug_method()
    if debug_method['method'] == DEBUG_METHOD_NONE and platform.system() != 'Windows':
        print('WARNING: if there is a crash, you will not see a backtrace', file=sys.stderr)
    return debug_method


def _get_debug_method():
    def can_use_python_lldb():
        if 'ZC_DISABLE_LLDB' in os.environ:
            return False

        # If not installed, can't use!
        if not shutil.which('lldb'):
            print('WARNING: could not find lldb', file=sys.stderr)
            return False

        # Using the python extension of lldb is tricky to set up... so check if it even works first.
        try:
            if platform.system() == 'Darwin':
                lldb_wrapper_args = ['xcrun', 'python3', script_dir / 'lldb_wrapper.py', '__TEST__']
            else:
                lldb_wrapper_args = [sys.executable, script_dir / 'lldb_wrapper.py', '__TEST__']
            subprocess.check_call(lldb_wrapper_args)
        except Exception as e:
            print(e, file=sys.stderr)
            print('WARNING: lldb is installed, but could not import the Python extension', file=sys.stderr)
            return False

        return True

    if 'ZC_DISABLE_DEBUGGER' in os.environ:
        return {'method': DEBUG_METHOD_NONE}

    if platform.system() == 'Windows':
        return {'method': DEBUG_METHOD_NONE}

    # Best to run executable under lldb, so we can print a backtrace without needing to write/read a really large
    # core dump file.
    if can_use_python_lldb():
        return {'method': DEBUG_METHOD_WRAP}

    # In case lldb via Python is not available, fallback to processing the core dump file.
    debugger_tool = None
    if shutil.which('lldb'):
        debugger_tool = 'lldb'
    elif shutil.which('gdb'):
        debugger_tool = 'gdb'

    if debugger_tool:
        return {'method': DEBUG_METHOD_CORE, 'debugger': debugger_tool}

    return {'method': DEBUG_METHOD_NONE}


def get_exe_name(target_name: str):
    return f'{target_name}.exe' if os.name == 'nt' else target_name


def get_build_folder():
    build_folder = None
    if 'BUILD_FOLDER' in os.environ:
        build_folder = Path(os.environ['BUILD_FOLDER']).absolute()
    else:
        # Select the build folder with the most recently modified binaries.
        possible_folders = [
            root_dir / 'build/RelWithDebInfo',
            root_dir / 'build/Release',
            root_dir / 'build/Debug',
        ]
        targets = ['zelda', 'zquest', 'zlauncher', 'zscript']
        def get_mtime(path: Path):
            if path.exists():
                return path.stat().st_mtime
            return -1
        build_folder = max(possible_folders, key=lambda f: max(get_mtime(f/t) for t in targets))

    if not build_folder.exists():
        raise Exception(f'build folder does not exist: {build_folder}')

    return build_folder


def _run(target_name: str, args: List, build_folder: Optional[str] = None):
    if not build_folder:
        build_folder = get_build_folder()
    exe_name = get_exe_name(target_name)
    args = [build_folder / exe_name] + args
    preexec_fn = None

    debug_method = get_debug_method()
    if debug_method['method'] == DEBUG_METHOD_WRAP:
        # The reason why wrapping with a lldb Python script is preferred to just running `lldb exe` directly,
        # is that there is no way to retain the exit code of the executable with lldb commands.
        if platform.system() == 'Darwin':
            args = ['xcrun', 'python3', script_dir / 'lldb_wrapper.py', *args]
        else:
            args = [sys.executable, script_dir / 'lldb_wrapper.py', *args]
    elif debug_method['method'] == DEBUG_METHOD_CORE:
        crash_dir = root_dir / '.tmp/crashes'
        crash_dir.mkdir(exist_ok=True, parents=True)

        if platform.system() != 'Windows':
            import resource
            preexec_fn = lambda: resource.setrlimit(resource.RLIMIT_CORE, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))

        if platform.system() == 'Darwin':
            current_corefile_cfg = subprocess.check_output('sysctl -n kern.corefile'.split(' '), encoding='utf-8').strip()
            if current_corefile_cfg != f'{crash_dir}/core.%N.%P':
                cmd = f'sudo sysctl -w kern.corefile={crash_dir}/core.%N.%P'
                sys.stderr.write(f'Running "{cmd}"...\n')
                os.system(cmd)

        if platform.system() == 'Linux':
            # TODO: this is particularly bad for ubuntu systems, which uses apport.
            current_corefile_cfg = subprocess.check_output('sysctl -n kernel.core_pattern'.split(' '), encoding='utf-8').strip()
            if current_corefile_cfg != f'{crash_dir}/core.%e.%p':
                cmd = f'sudo sysctl -w kernel.core_pattern={crash_dir}/core.%e.%p'
                sys.stderr.write(f'Running "{cmd}"...\n')
                os.system(cmd)

    p = subprocess.Popen(args, cwd=build_folder, encoding='utf-8',
                         preexec_fn=preexec_fn,
                         stdout=sys.stdout,
                         stderr=sys.stderr)
    p.wait()

    if debug_method['method'] == DEBUG_METHOD_CORE and p.returncode < 0:
        expected_name = f'core.{exe_name}.{p.pid}'
        crash_path = crash_dir / expected_name
        if crash_path.exists():
            sys.stderr.write(f'Program crashed. Found core dump: {crash_path}\nPrinting backtrace...\n\n')

            backtrace = None
            if debug_method['debugger'] == 'lldb':
                backtrace = subprocess.check_output([
                    'lldb',
                    '--batch',
                    '-c', crash_path,
                    '-o', 'settings set use-color true',
                    '-o', 'thread backtrace all',
                ], stderr=subprocess.STDOUT, encoding='utf-8')
            elif debug_method['debugger'] == 'gdb':
                backtrace = subprocess.check_output([
                    'gdb',
                    '--batch',
                    '-c', crash_path,
                    '-ex', 'bt',
                ], stderr=subprocess.STDOUT, encoding='utf-8')
            if 'ZC_KEEP_CRASHES' not in os.environ:
                crash_path.unlink()

            if backtrace:
                sys.stderr.write('\n')
                sys.stderr.write(backtrace)
                sys.stderr.write('\n')
        else:
            print('WARNING: could not find core dump. Can\'t print backtrace.', file=sys.stderr)
            print(f'\ndebug_method: {debug_method}', file=sys.stderr)

    return p

def run(target_name: str, args: List, build_folder: Optional[Path] = None, **kwargs):
    """
    Runs target (ec: zelda, zscript, zquest, zlauncher), from env.BUILD_FOLDER or the provided build_folder.

    Returns subprocess.CompletedProcess[str], even if command fails.

    If there is a crash, a backtrace is printed to stderr. Note, this is not supported for Windows.
    """
    env = None
    if build_folder:
        env = {
            **os.environ,
            'BUILD_FOLDER': str(build_folder),
        }

    # Spawn a new process because it makes it simpler to get colored output.
    return subprocess.run([
        sys.executable,
        script_dir / 'run_target.py',
        target_name,
        *args,
    ], capture_output=True, encoding='utf-8', env=env, **kwargs)


def check_run(target_name: str, args: List, build_folder: Optional[Path] = None, **kwargs):
    """
    Same as run_target.run, but raises an exception on non-zero exit code.
    """
    p = run(target_name, args, build_folder=build_folder, **kwargs)
    if p.returncode != 0:
        cmd_str = ' '.join([target_name, *[str(a) for a in args]])
        parts = [
            f'got error running command: {cmd_str}',
            f'exit code: {p.returncode}',
            'STDOUT', p.stdout,
            'STDERR', p.stderr
        ]
        raise Exception('\n\n'.join(parts))
    return p

if __name__ == '__main__':
    p = _run(sys.argv[1], sys.argv[2:])
    sys.exit(p.returncode)
