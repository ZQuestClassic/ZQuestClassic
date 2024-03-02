# All this does is call the given process with the provided arguments, and returns
# with the same exit code when finished. If the process crashes, prints a backtrace.

import os
import sys
import tempfile

from pathlib import Path

import import_lldb

exe = sys.argv[1]
args = sys.argv[2:]

if sys.argv[1] == '__TEST__':
    exe = 'lldb'

debugger = lldb.SBDebugger.Create()
# Block until target process exits.
debugger.SetAsync(False)
debugger.SetUseColor(True)
target = debugger.CreateTargetWithFileAndArch(exe, lldb.LLDB_ARCH_DEFAULT)
if not target:
    print('failed to create target')
    exit(1)

if sys.argv[1] == '__TEST__':
    exit(0)

launch_info = lldb.SBLaunchInfo(args)
launch_info.SetWorkingDirectory(os.getcwd())
err = lldb.SBError()
stdout_f = os.path.join(tempfile.gettempdir(), os.urandom(24).hex())
stderr_f = os.path.join(tempfile.gettempdir(), os.urandom(24).hex())

process = target.Launch(
    debugger.GetListener(),
    args,
    None,
    None,
    stdout_f,
    stderr_f,
    os.getcwd(),
    0,
    False,
    err,
)
sys.stdout.write(Path(stdout_f).read_text())
sys.stderr.write(Path(stderr_f).read_text())
os.unlink(stdout_f)
os.unlink(stderr_f)

if process.exit_state < 0:
    print(f'\nprocess crashed with exit state: {process.exit_state}', file=sys.stderr)
    print('printing backtrace\n', file=sys.stderr)
    for thread in process.threads:
        print(thread, file=sys.stderr)
        for frame in thread.frames:
            print('   ', frame, file=sys.stderr)
        print(file=sys.stderr)

sys.exit(process.exit_state)
