#include "test_runner/assert.h"
#include "test_runner/test_runner.h"
#include "zc/scripting/common.h"
#include "zc/zc_sys.h"

#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

static void test_parse_user_path_non_ascii()
{
	std::string sep(1, (char)fs::path::preferred_separator);

	// "Créateur/クエスト.txt", escaped because MSVC reads source files in the ANSI code page.
	// Bytes >= 0x80 used to be taken for control characters where char is signed (like Windows).
	std::string dir = "Cr\xc3\xa9" "ateur";
	std::string name = "\xe3\x82\xaf\xe3\x82\xa8\xe3\x82\xb9\xe3\x83\x88.txt";
	auto r = parse_user_path(dir + "/" + name, true);
	if (!r)
		throw std::runtime_error(r.error());
	assertTrue(r->ends_with(dir + sep + name));

	// Not valid UTF-8 (like "Créateur.txt" in cp1252): never a crash. On Windows it is read in the
	// system's ANSI code page; elsewhere the bytes are used as they are.
	assertTrue(parse_user_path("Cr\xe9" "ateur.txt", true).has_value());

	// Still rejected.
	assertTrue(!parse_user_path("a\x01" "b.txt", true));
	assertTrue(!parse_user_path("../a.txt", true));
	assertTrue(!parse_user_path("a/CON", true));
}

static void test_parse_user_path_invalid_character()
{
	// The error names the character that was rejected.
	auto r = parse_user_path("ab|c.txt", true);
	assertTrue(!r);
	assertTrue(r.error().ends_with("invalid character |"));
}

TestResults test_parse_user_path(bool verbose)
{
	char old_qst_files_path[sizeof(qst_files_path)];
	std::memcpy(old_qst_files_path, qst_files_path, sizeof(qst_files_path));
	std::strcpy(qst_files_path, "files");

	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "non_ascii", test_parse_user_path_non_ascii },
		{ "invalid_character", test_parse_user_path_invalid_character },
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

	std::memcpy(qst_files_path, old_qst_files_path, sizeof(qst_files_path));
	return tr;
}
