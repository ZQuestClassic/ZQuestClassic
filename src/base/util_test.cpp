#include "base/util.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <filesystem>
#include <fstream>
#include <limits>

namespace fs = std::filesystem;

// A number too long for the fixed buffer these used to copy into.
static const std::string long_number = std::string(200, '1');

static void test_ffparse2()
{
	assertEqual(util::ffparse2("3"), 30000);
	assertEqual(util::ffparse2("1.5"), 15000);
	assertEqual(util::ffparse2("-2.25"), -22500);
	assertEqual(util::ffparse2("0.0001"), 1);
	assertEqual(util::ffparse2("214748.3647"), 2147483647);
	assertEqual(util::ffparse2("-214748.3648"), std::numeric_limits<int32_t>::min());

	// Extra decimal places are dropped, not rounded.
	assertEqual(util::ffparse2("1.23456"), 12345);

	// Out of range values saturate rather than wrapping around.
	assertEqual(util::ffparse2("999999"), 2147480000);
	assertEqual(util::ffparse2("-999999"), -2147480000);
	assertEqual(util::ffparse2(long_number), 2147480000);
	assertEqual(util::ffparse2("-" + long_number), -2147480000);

	// With `do_except`, the same inputs are rejected instead.
	assertThrows(util::ffparse2("999999", true));
	assertThrows(util::ffparse2("1.23456", true));
	assertThrows(util::ffparse2(long_number, true));
	assertThrows(util::ffparse2("12x", true));
	assertThrows(util::ffparse2("1.2.3", true));
}

static void test_unstringify_vector()
{
	std::vector<int32_t> vec;

	std::vector<int32_t> expected = {1, 2, 30};
	util::unstringify_vector(vec, "{1,2,30}", false);
	assertEqual(vec, expected);

	util::unstringify_vector(vec, std::string("{1,2,30}"), false);
	assertEqual(vec, expected);

	expected = {15000, -22500};
	util::unstringify_vector(vec, "{1.5,-2.25}", true);
	assertEqual(vec, expected);

	// An element longer than the buffer this used to copy into. The value
	// saturates; what matters is that the parse stays in bounds and the
	// elements after it are still read.
	expected = {2147480000, 70000, 220000};
	std::string long_element = "{" + long_number + ",7,22}";
	util::unstringify_vector(vec, long_element, true);
	assertEqual(vec, expected);
}

static void test_escape_characters()
{
	// Round-tripping has to survive a backslash sitting in front of what would
	// otherwise look like an escape.
	for (std::string original : {
			 std::string("plain text"),
			 std::string("C:\\new\\table"),
			 std::string("quote \" and 'tick'"),
			 std::string("real\nnewline\tand tab"),
			 std::string("trailing backslash \\"),
			 std::string(""),
		 })
	{
		assertEqual(util::unescape_characters(util::escape_characters(original)), original);
	}

	// Escapes it doesn't know are left alone.
	assertEqual(util::unescape_characters("a\\zb"), std::string("a\\zb"));
}

static void test_disallow_escapes()
{
	// Drops the control characters...
	assertEqual(util::disallow_escapes(util::escape_characters("real\nnewline")), std::string("realnewline"));
	assertEqual(util::disallow_escapes(util::escape_characters("a\tb\vc")), std::string("abc"));

	// ...but keeps everything else, backslashes included.
	assertEqual(util::disallow_escapes(util::escape_characters("C:\\new\\table")), std::string("C:\\new\\table"));
	assertEqual(util::disallow_escapes(util::escape_characters("has \"quotes\"")), std::string("has \"quotes\""));
	assertEqual(util::disallow_escapes(util::escape_characters("plain text")), std::string("plain text"));
}

static void test_escape_string()
{
	std::string tricky = "say \"hi\"\\path\ttab\x01";

	// Quotes and backslashes must be escaped, or the result can't be read back.
	assertEqual(util::escape_string(tricky), std::string("\"say \\\"hi\\\"\\\\path\\ttab\\x01\""));

	// Both overloads have to agree.
	assertEqual(util::escape_string(tricky.c_str()), util::escape_string(tricky));

	assertEqual(util::unescape_string(util::escape_string(tricky)), tricky);
	assertEqual(util::unescape_string(util::escape_string(tricky).c_str()), tricky);
}

static void test_is_relative_path()
{
	assertTrue(util::is_relative_path(""));
	assertTrue(util::is_relative_path("file.qst"));
	assertTrue(util::is_relative_path("quests/file.qst"));
	assertTrue(util::is_relative_path("./file.qst"));
	assertTrue(util::is_relative_path("..\\file.qst"));

	// Absolute paths from either platform are recognized no matter which
	// platform is running.
	assertTrue(!util::is_relative_path("/home/user/file.qst"));
	assertTrue(!util::is_relative_path("\\file.qst"));
	assertTrue(!util::is_relative_path("C:\\Users\\user\\file.qst"));
	assertTrue(!util::is_relative_path("c:/Users/user/file.qst"));
	assertTrue(!util::is_relative_path("c:file.qst"));

	// Only a leading letter followed by a colon counts as a drive.
	assertTrue(!util::is_relative_path("a:b/file.qst"));
	assertTrue(util::is_relative_path("dir/c:file.qst"));
	assertTrue(util::is_relative_path("1:file.qst"));
}

static void test_md5_file()
{
	fs::path dir = fs::temp_directory_path() / "zc_util_test";
	fs::remove_all(dir);
	fs::create_directories(dir);
	fs::path file = dir / "file.bin";

	// Larger than the streaming buffer, with a partial chunk at the end, and
	// bytes that would trip up a text-mode read.
	std::string data;
	data.reserve(200'000);
	for (int i = 0; i < 200'000; i++)
		data.push_back((char)(i * 31 + i / 251));
	std::ofstream(file, std::ios::binary) << data;

	assertTrue(util::md5_file(file) == util::md5_hash_bytes(data));
	assertTrue(!util::md5_file(dir / "nope.bin").has_value());

	fs::remove_all(dir);
}

static void test_checkPath()
{
	fs::path dir = fs::temp_directory_path() / "zc_util_test";
	fs::remove_all(dir);
	fs::create_directories(dir);
	fs::path file = dir / "file.txt";
	std::ofstream(file) << "hi";

	assertTrue(util::checkPath(dir.string().c_str(), true));
	assertTrue(!util::checkPath(dir.string().c_str(), false));
	assertTrue(util::checkPath(file.string().c_str(), false));
	assertTrue(!util::checkPath(file.string().c_str(), true));

	assertTrue(!util::checkPath((dir / "nope").string().c_str(), true));
	assertTrue(!util::checkPath((dir / "nope").string().c_str(), false));

	// Neither a directory nor a regular file. On Windows this path simply
	// doesn't exist, which gives the same answer.
	assertTrue(!util::checkPath("/dev/null", true));
	assertTrue(!util::checkPath("/dev/null", false));

	fs::remove_all(dir);
}

static void test_nearest_existing_directory()
{
	fs::path dir = fs::temp_directory_path() / "zc_util_test";
	fs::remove_all(dir);
	fs::create_directories(dir);

	assertEqual(util::nearest_existing_directory(dir).string(), dir.string());
	assertEqual(util::nearest_existing_directory(dir / "a" / "b" / "c").string(), dir.string());

	fs::remove_all(dir);
	assertNotEqual(util::nearest_existing_directory(dir).string(), dir.string());
}

TestResults test_util(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "ffparse2", test_ffparse2 },
		{ "unstringify_vector", test_unstringify_vector },
		{ "escape_characters", test_escape_characters },
		{ "disallow_escapes", test_disallow_escapes },
		{ "escape_string", test_escape_string },
		{ "is_relative_path", test_is_relative_path },
		{ "md5_file", test_md5_file },
		{ "checkPath", test_checkPath },
		{ "nearest_existing_directory", test_nearest_existing_directory },
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
