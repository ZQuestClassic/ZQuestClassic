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
    lldb.eLaunchFlagStopAtEntry,
    False,
    err,
)

def format_thread(thread):
    return '\n'.join([str(thread), *[f'    {frame}' for frame in thread.frames], ''])


def print_thread(thread):
    print(format_thread(thread), file=sys.stderr)


# lldb passes SIGPIPE through without stopping, so a process it kills dies with no backtrace.
# Stopping on it also stops for a SIGPIPE the process ignores (it then gets EPIPE and goes on), so
# keep the backtrace, pass the signal on, and only print it if the process dies from it.
sigpipe_backtrace = None
if process.IsValid():
    signals = process.GetUnixSignals()
    sigpipe = signals.GetSignalNumberFromName('SIGPIPE')
    signals.SetShouldStop(sigpipe, True)
    process.Continue()
    while process.GetState() == lldb.eStateStopped:
        thread = process.GetSelectedThread()
        if (
            thread.GetStopReason() != lldb.eStopReasonSignal
            or thread.GetStopReasonDataAtIndex(0) != sigpipe
        ):
            break
        sigpipe_backtrace = format_thread(thread)
        process.Continue()
sys.stdout.write(Path(stdout_f).read_text())
sys.stderr.write(Path(stderr_f).read_text())
os.unlink(stdout_f)
os.unlink(stderr_f)

if (
    sigpipe_backtrace
    and process.GetState() == lldb.eStateExited
    and 'signal' in (process.GetExitDescription() or '')
):
    print(
        f'\nprocess was killed by SIGPIPE ({process.GetExitDescription()})',
        file=sys.stderr,
    )
    print('printing backtrace of the thread that got it\n', file=sys.stderr)
    print(sigpipe_backtrace, file=sys.stderr)

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
