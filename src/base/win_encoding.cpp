#include "base/win_encoding.h"

#ifdef _WIN32
#include <windows.h>

static bool is_valid_utf8(std::string_view str)
{
	return str.empty() || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), (int)str.size(), nullptr, 0);
}

namespace util
{
	void legacy_path_to_utf8(std::string& path)
	{
		if (!is_valid_utf8(path))
			path = wide_to_utf8(utf8_to_wide(path));
	}

	std::wstring utf8_to_wide(std::string_view str)
	{
		UINT code_page = is_valid_utf8(str) ? CP_UTF8 : system_ansi_code_page();
		int len = MultiByteToWideChar(code_page, 0, str.data(), (int)str.size(), nullptr, 0);
		std::wstring result(len, L'\0');
		if (len)
			MultiByteToWideChar(code_page, 0, str.data(), (int)str.size(), result.data(), len);
		return result;
	}

	std::string wide_to_utf8(std::wstring_view str)
	{
		int len = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
		std::string result(len, '\0');
		if (len)
			WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), result.data(), len, nullptr, nullptr);
		return result;
	}

	unsigned int system_ansi_code_page()
	{
		static UINT code_page = []{
			DWORD result;
			if (GetLocaleInfoEx(LOCALE_NAME_SYSTEM_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
					(LPWSTR)&result, sizeof(result) / sizeof(WCHAR)))
				return (UINT)result;
			return GetACP();
		}();
		return code_page;
	}
}
#else
namespace util
{
	void legacy_path_to_utf8(std::string&)
	{
	}
}
#endif
