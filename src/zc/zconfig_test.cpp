// Checks that per-quest settings survive a round trip through the config file,
// whose parser ends section and key names at characters like '#' and '='.
//
// Run with `zplayer -test-zc <tests dir>`.

#include "allegro/config.h"
#include "test_runner/test_runner.h"
#include "zconfig.h"
#include <filesystem>
#include <fmt/format.h>

TestResults test_qst_cfg_header([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	auto dir = std::filesystem::temp_directory_path() / "zc_qst_cfg_header_test";
	std::filesystem::create_directories(dir);
	std::string cfg_path = (dir / "test.cfg").string();
	std::filesystem::remove(cfg_path);

	const char* quest_paths[] = {
		"#Quests/My Quest/quest.qst",
		"Quests/key=value/quest.qst",
		"Quests/[Beta] Quest #2.qst",
	};

	push_config_state();
	set_config_file(cfg_path.c_str());
	for (auto quest_path : quest_paths)
	{
		std::string header = qst_cfg_header_from_path((dir / quest_path).string());
		// Per-quest settings use the header both as a section (e.g. Test Init
		// Data) and inside a key (e.g. the quest's control scheme).
		set_config_string(header.c_str(), "value", "section");
		set_config_string("Controls", fmt::format("qst_controls__{}", header).c_str(), "key");
	}
	flush_config_file();

	// Reload from disk, so the names go through the parser.
	set_config_file(cfg_path.c_str());
	for (auto quest_path : quest_paths)
	{
		std::string header = qst_cfg_header_from_path((dir / quest_path).string());
		std::string as_section = get_config_string(header.c_str(), "value", "");
		std::string as_key = get_config_string("Controls", fmt::format("qst_controls__{}", header).c_str(), "");

		tr.total++;
		if (as_section != "section" || as_key != "key")
		{
			fmt::println("failed: {}: header \"{}\" read back section=\"{}\" key=\"{}\"", quest_path, header, as_section, as_key);
			tr.failed++;
		}
	}
	pop_config_state();

	std::filesystem::remove_all(dir);
	return tr;
}
