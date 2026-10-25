#include "base/win_encoding.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

// Escaped because MSVC reads source files in the ANSI code page.
// "C:\Créateur\クエスト.qst"
static const std::string utf8_path = "C:\\Cr\xc3\xa9" "ateur\\\xe3\x82\xaf\xe3\x82\xa8\xe3\x82\xb9\xe3\x83\x88.qst";

static void test_legacy_path_to_utf8()
{
	std::string path = utf8_path;
	util::legacy_path_to_utf8(path);
	assertEqual(path, utf8_path);

#ifdef _WIN32
	// "C:\Créateur\quest.qst" in cp1252, as a pre-manifest version on a Western system saved it.
	if (util::system_ansi_code_page() == 1252)
	{
		path = "C:\\Cr\xe9" "ateur\\quest.qst";
		util::legacy_path_to_utf8(path);
		assertEqual(path, std::string("C:\\Cr\xc3\xa9" "ateur\\quest.qst"));
	}
#endif
}

static void test_utf8_to_wide()
{
#ifdef _WIN32
	std::wstring wide = util::utf8_to_wide(utf8_path);
	assertTrue(wide == L"C:\\Cr\u00e9ateur\\\u30af\u30a8\u30b9\u30c8.qst");
	assertEqual(util::wide_to_utf8(wide), utf8_path);
	assertTrue(util::utf8_to_wide("").empty());
	assertEqual(util::wide_to_utf8(L""), std::string());

	if (util::system_ansi_code_page() == 1252)
		assertTrue(util::utf8_to_wide("Cr\xe9" "ateur") == L"Cr\u00e9ateur");
#endif
}

TestResults test_win_encoding(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "legacy_path_to_utf8", test_legacy_path_to_utf8 },
		{ "utf8_to_wide", test_utf8_to_wide },
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
