#ifndef BASE_CRASH_BACKTRACE_H_
#define BASE_CRASH_BACKTRACE_H_

// On Windows, makes a crash print a backtrace: to the file named by the ZC_CRASH_BACKTRACE_FILE
// environment variable, or to stderr. Covers unhandled exceptions (access violations etc.) and
// abort(). Does nothing on other platforms, where scripts/run_target.py re-runs a crashed
// program under lldb or gdb instead.
//
// Symbols come from the .pdb files next to the executables, so a package needs them (see
// ZC_KEEP_SYMBOLS in packaging/CMakeLists.txt) to show function names and lines.
void install_crash_backtrace_handler();

#endif
