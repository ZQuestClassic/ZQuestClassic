#include "base/about.h"
#include "base/process_management.h"
#include "base/zc_alleg.h"
#include "base/zapp.h"
#include "allegro5/allegro_native_dialog.h"
#include <algorithm>
#include <filesystem>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <fmt/format.h>

namespace fs = std::filesystem;

static bool headless;
static std::string repo = getRepo();
static std::string channel = getReleaseChannel();
static std::string current_version = getReleaseTag();

std::ofstream out("updater.log", std::ios::binary);

void zprint2(const char * const format, ...)
{
	char buf[8192];

	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	al_trace("%s", buf);
}

static int32_t used_switch(int32_t argc,char *argv[],const char *s)
{
    // assumes a switch won't be in argv[0]
    for(int32_t i=1; i<argc; i++)
        if(stricmp(argv[i],s)==0)
            return i;
            
    return 0;
}

[[noreturn]] static void fatal(std::string msg)
{
	if (headless)
		printf("[fatal] %s\n", msg.c_str());
	else
		al_show_native_message_box(all_get_display(), "ZQuest Classic: I AM ERROR", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_ERROR);

	out << "[fatal] " << msg.c_str() << '\n';
	out.close();
	exit(1);
}

[[noreturn]] static void done(std::string msg)
{
	if (headless)
		printf("[done] %s\n", msg.c_str());
	else
		al_show_native_message_box(all_get_display(), "ZQuest Classic Updater", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL);

	out << "[done] " << msg.c_str() << '\n';
	out.close();
	exit(0);
}

static bool prompt(std::string msg)
{
	out << "[prompt] " << msg.c_str() << '\n';
	if (headless)
	{
		printf("[prompt] %s\n", msg.c_str());
		return true;
	}

	int ret = al_show_native_message_box(all_get_display(), "ZQuest Classic Updater", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_YES_NO);
	return ret != 2;
}

static std::string get_output(std::string file, const std::vector<std::string>& args, std::string fatal_error_msg = "")
{
	std::string output;
	if (!run_and_get_output(file, args, output))
	{
		if (fatal_error_msg.empty()) fatal("Failed to launch: " + file);
		fatal(fatal_error_msg);
	}
	output.erase(output.find_last_not_of("\t\n\v\f\r ") + 1);
	return output;
}

static std::pair<std::string, std::map<std::string, std::string>> get_output_map(std::string file, const std::vector<std::string>& args, std::string fatal_error_msg = "")
{
	std::string output = get_output(file, args, fatal_error_msg);
	return {output, parse_output_map(output)};
}

static bool is_in_osx_application_bundle()
{
#ifdef __APPLE__
    return fs::current_path().string().find("/ZQuestClassic.app/") != std::string::npos;
#else
    return false;
#endif
}

static void require_python()
{
	std::string py_version = get_output(PYTHON, {"--version"}, "Python3 is required to run the updater");
	if (!py_version.starts_with("Python 3"))
	{
		fatal("Python3 is required, but found " + py_version);
	}
}

static std::tuple<std::string, std::string> get_next_release()
{
	auto [next_release_output, next_release_map] = get_output_map(PYTHON, {
		"tools/updater.py",
		"--repo", repo,
		"--channel", channel,
		"--print-next-release",
	});
	if (!next_release_map.contains("tag_name") || !next_release_map.contains("asset_url"))
	{
		fatal("Could not find next version: " + next_release_output);
	}
	std::string new_version = next_release_map["tag_name"];
	std::string asset_url = next_release_map["asset_url"];
	return {new_version, asset_url};
}

static bool install_release(std::string asset_url, bool use_cache, std::string& error)
{
	std::vector<std::string> args = {
		"tools/updater.py",
		"--repo", repo,
		"--channel", channel,
		"--asset-url", asset_url,
	};
	if (use_cache) args.push_back("--cache");
	auto updater_output = get_output(PYTHON, args);
	if (updater_output.empty())
	{
		fatal("Failed to launch updater.py");
	}
	printf("updater.py:\n%s\n", updater_output.c_str());

	bool success = updater_output.find("success!") != std::string::npos;
	if (!success)
		error = updater_output;
	return success;
}

int32_t main(int32_t argc, char* argv[])
{
	common_main_setup(App::updater, argc, argv);

	bool cache = used_switch(argc, argv, "-cache") > 0;
	headless = used_switch(argc, argv, "-headless") > 0;

#ifdef __APPLE__
	if (!is_in_osx_application_bundle() && argc > 0)
	{
        chdir(fs::path(argv[0]).parent_path().c_str());
    }
#endif

	require_python();

	auto [new_version, asset_url] = get_next_release();
	if (new_version.empty() || asset_url.empty())
	{
		fatal("Could not find next version");
	}

	if (current_version == new_version)
	{
		std::string msg = fmt::format("Already on latest version: {}. Would you like to continue anyway?", new_version);
		if (!prompt(msg))
			return 0;
	}
	else
	{
		std::string msg = fmt::format("Would you like to upgrade from {} to {}? This should take less than a minute.", current_version, new_version);
		if (!prompt(msg))
			return 0;
	}

	std::string error;
	bool success = install_release(asset_url, cache, error);
	if (success)
		done("Done!");
	else
		fatal("Failed: " + error);

	return 0;
}
END_OF_MAIN()

// TODO: make this not needed to compile...
bool DragAspect = false;
double aspect_ratio = 0;
int window_min_width = 0, window_min_height = 0;
