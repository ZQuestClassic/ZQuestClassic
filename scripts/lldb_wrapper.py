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


def print_thread(thread):
    print(thread, file=sys.stderr)
    for frame in thread.frames:
        print('   ', frame, file=sys.stderr)
    print(file=sys.stderr)


if process.exit_state < 0:
    state = lldb.SBDebugger.StateAsCString(process.GetState())
    print(
        f'\nprocess crashed with exit state: {process.exit_state} (process state: {state})',
        file=sys.stderr,
    )

    # Threads with a stop reason (signal, exception, ...) are the interesting ones, so
    # print them before the rest.
    stopped_threads = [
        t
        for t in process.threads
        if t.GetStopReason() not in (lldb.eStopReasonNone, lldb.eStopReasonInvalid)
    ]
    stopped_ids = {t.GetThreadID() for t in stopped_threads}
    other_threads = [t for t in process.threads if t.GetThreadID() not in stopped_ids]

    print('printing backtrace\n', file=sys.stderr)
    for thread in stopped_threads + other_threads:
        print_thread(thread)

    # The replay test runner only shows the tail of stderr, so repeat the stopped
    # threads at the end.
    if stopped_threads:
        print('==== stopped threads (repeated) ====\n', file=sys.stderr)
        for thread in stopped_threads:
            print_thread(thread)

sys.exit(process.exit_state)
