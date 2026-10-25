#ifndef BASE_WIN_ENCODING_H_
#define BASE_WIN_ENCODING_H_

#include <string>
#include <string_view>

// Text encoding for the Windows API. ZC's strings are UTF-8, but Windows takes UTF-16 (the
// wide-char APIs) or the process code page (the narrow ones). src/windows.manifest makes that
// UTF-8 on Windows 10 1903+. Before that it is the system's ANSI code page, so strings from
// narrow APIs (or saved by an older version) can be in either. See docs/windows_support.md.

namespace util
{
	// Converts a path saved on Windows before src/windows.manifest (which can be in the ANSI
	// code page) to UTF-8. Valid UTF-8 is left as is, so the rare ANSI string that is also valid
	// UTF-8 (like cp1252 "Ã©") is not converted. Does nothing on other platforms.
	void legacy_path_to_utf8(std::string& path);

#ifdef _WIN32
	// Strings that aren't valid UTF-8 are read in the system's ANSI code page instead.
	std::wstring utf8_to_wide(std::string_view str);
	std::string wide_to_utf8(std::wstring_view str);
	// Unlike GetACP, this is not UTF-8 under src/windows.manifest.
	unsigned int system_ansi_code_page();
#endif
}

#endif
