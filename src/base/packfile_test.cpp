// Round-trips strings through the length-prefixed string readers and writers.
//
// Run with `zplayer -test-zc <tests dir>`.

#include "test_runner/test_runner.h"
#include "base/packfile.h"
#include <filesystem>
#include <fmt/format.h>
#include <string>

namespace fs = std::filesystem;

static void check(TestResults& tr, const char* what, bool ok)
{
	tr.total++;
	if (!ok)
	{
		fmt::println("failed: {}", what);
		tr.failed++;
	}
}

static bool round_trip_lstr(std::string const& str, std::string& result)
{
	auto path = (fs::temp_directory_path() / "zc_packfile_test.bin").string();

	PACKFILE* f = pack_fopen(path.c_str(), F_WRITE);
	if (!f)
		return false;
	bool wrote = p_putlstr(str, f);
	pack_fclose(f);
	if (!wrote)
		return false;

	f = pack_fopen(path.c_str(), F_READ);
	if (!f)
		return false;
	bool read = p_getlstr(&result, f);
	pack_fclose(f);
	fs::remove(path);
	return read;
}

TestResults test_packfile([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	std::string result;
	check(tr, "short string", round_trip_lstr("hello", result) && result == "hello");

	// Longer than a 16-bit length can hold.
	std::string long_str(70000, 'x');
	long_str.back() = 'y';
	check(tr, "string past 64 KB", round_trip_lstr(long_str, result) && result == long_str);

	// A damaged length should fail to read, not ask for gigabytes of memory.
	auto path = (fs::temp_directory_path() / "zc_packfile_test_bad.bin").string();
	bool read_bad = false;
	if (PACKFILE* f = pack_fopen(path.c_str(), F_WRITE))
	{
		p_iputl(0xFFFFFFFF, f);
		pack_fclose(f);
		if ((f = pack_fopen(path.c_str(), F_READ)))
		{
			read_bad = !p_getlstr(&result, f);
			pack_fclose(f);
		}
	}
	check(tr, "huge length fails", read_bad);
	fs::remove(path);

	return tr;
}
