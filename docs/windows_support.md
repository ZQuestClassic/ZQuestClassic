# Windows version support

The oldest supported Windows version is Windows 7 (see the README). This file lists the code
that exists only for older Windows versions, grouped by the minimum version that makes it
unnecessary, so it can be removed when that minimum goes up.

When adding a workaround for an older Windows version, list it here and mention this file in a
comment at the workaround.

## Windows 8 minimum

- sentry-native's `_WIN32_WINNT` (root `CMakeLists.txt`): set to Windows 7, because sentry
  targets the Windows version CMake runs on and then imports `WaitOnAddress`. Drop the
  override.

- `io_manager::ProcessReadFile` (`src/base/process_management.cpp`): loads
  `GetOverlappedResultEx` (Windows 8+) at runtime, and without it falls back to a blocking
  `ReadFile` with no timeout. Call `GetOverlappedResultEx` directly and drop the fallback.

## Windows 10 1607 minimum

- `GetDebuggerDpiScale` (`src/zc/debugger/debugger_gui.cpp`): loads `GetDpiForWindow` and
  `GetDpiForSystem` from user32 at runtime, because importing them directly stops the exe from
  loading on Windows 7. Without them the debugger UI isn't scaled. Call them directly.

## Windows 10 1903 minimum

`src/windows.manifest` sets the process code page to UTF-8, so the narrow-char APIs (`argv`,
`fopen`, `std::ifstream`, `CreateProcessA`, ...) take the UTF-8 strings the rest of ZC uses.
Older versions ignore that setting and use the ANSI code page, so there a string may be in
either encoding, and paths with characters outside ASCII can fail. Keep the manifest; these can
go:

- `util::utf8_to_wide` (`src/base/win_encoding.cpp`): reads strings that aren't valid UTF-8 in
  the system's ANSI code page. Keep the fallback only while `util::legacy_path_to_utf8` is
  needed for saves written before the manifest; otherwise always convert from UTF-8.
- `create_command_line` (`src/base/process_management.cpp`): converts each part of a command
  line on its own, since parts can come from different code pages. Converting the whole line
  at once would work too (optional).
- `common_main_setup` (`src/base/zapp.cpp`): changes into the exe folder with
  `fs::current_path` and a wide path, because `chdir` would read the UTF-8 path in the ANSI
  code page. Plain `chdir` would work too (optional).
- Places that still fail on non-ASCII paths before 1903 (allegro_legacy's `_al_win_open`,
  `fopen` in the quest decoders, `gme_load_file`, ...) don't need fixing.

## Windows 11 minimum

Everything above (Windows 11 is newer than 1903). Nothing needs Windows 11 specifically yet.
