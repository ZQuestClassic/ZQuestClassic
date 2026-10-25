#include "base/process_management.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>

namespace fs = std::filesystem;

static std::string to_utf8(const fs::path& path)
{
	auto u8_str = path.u8string();
	return std::string(u8_str.begin(), u8_str.end());
}

// A child process should get the exact UTF-8 bytes of its arguments, even for characters
// outside the ANSI code page (on Windows, this needs CreateProcessW and the UTF-8 code page
// from windows.manifest). zplayer echoes the path back when it fails to load it.
static void test_non_ascii_args()
{
	std::string zplayer = locate_zapp_file(ZPLAYER_FILE);
	if (!fs::exists(fs::path(reinterpret_cast<const char8_t*>(zplayer.c_str()))))
	{
		if (std::getenv("CI"))
			throw std::runtime_error(fmt::format("missing {}", zplayer));
		fmt::println("  skipping: missing {}", zplayer);
		return;
	}

	// "Créateur クエスト", escaped because MSVC reads source files in the ANSI code page.
	std::string path = to_utf8(fs::temp_directory_path() / u8"Cr\u00e9ateur \u30af\u30a8\u30b9\u30c8" / "missing.qst");
	std::string output;
	assertTrue(run_and_get_output(zplayer, {"-headless", "-dump-qrs", path}, output));
	if (output.find(fmt::format("failed to load {}", path)) == std::string::npos)
		throw std::runtime_error(fmt::format("expected the path {} in output:\n{}", path, output));
}

TestResults test_process_management(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "non_ascii_args", test_non_ascii_args },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}
