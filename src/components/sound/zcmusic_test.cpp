#include "components/sound/zcmusic.h"
#include "test_runner/assert.h"
#include "test_runner/test_runner.h"

#ifdef _WIN32
#include "base/win_encoding.h"
#endif

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static void test_find_for_quest_non_ascii()
{
	// A quest with "Thème.ogg" in its music folder. Escaped because MSVC reads source files in
	// the ANSI code page.
	fs::path dir = fs::temp_directory_path() / "zc_zcmusic_test";
	std::error_code ec;
	fs::remove_all(dir, ec);
	fs::create_directories(dir / "music");
	std::ofstream(dir / "music" / fs::path(u8"Th\u00e8me.ogg")) << "x";
	std::string quest_path = (dir / "quest.qst").string();

	auto paths = zcmusic_find_for_quest("Th\xc3\xa8me.ogg", quest_path.c_str());
	assertEqual(paths.size(), (size_t)1);
	assertTrue(paths[0].ends_with("Th\xc3\xa8me.ogg"));

	// The name in cp1252, as an editor before src/windows.manifest saved it: never a crash. On
	// Windows it is read in the system's ANSI code page; elsewhere it names a different file.
	paths = zcmusic_find_for_quest("Th\xe8me.ogg", quest_path.c_str());
#ifdef _WIN32
	if (util::system_ansi_code_page() == 1252)
	{
		assertEqual(paths.size(), (size_t)1);
		assertTrue(paths[0].ends_with("Th\xc3\xa8me.ogg"));
	}
#else
	assertTrue(paths.empty());
#endif

	assertTrue(zcmusic_find_for_quest("missing.ogg", quest_path.c_str()).empty());
	assertTrue(zcmusic_find_for_quest("", quest_path.c_str()).empty());

	fs::remove_all(dir, ec);
}

TestResults test_zcmusic(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "find_for_quest_non_ascii", test_find_for_quest_non_ascii },
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
