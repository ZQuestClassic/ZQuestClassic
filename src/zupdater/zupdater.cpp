#include "base/about.h"
#include "base/process_management.h"
#include "base/util.h"
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

#ifndef UPDATER_USES_PYTHON
#include <curl/curl.h>
#include "json/json.h"
#include "miniz.h"

using giri::json::JSON;

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

#endif

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
#ifdef UPDATER_USES_PYTHON
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
#else
	std::string json_url = fmt::format("https://api.github.com/repos/{}/releases", repo);

	struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;

	CURL *curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, json_url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	CURLcode res = curl_easy_perform(curl_handle);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		free(chunk.memory);
		return {};
	}

	curl_easy_cleanup(curl_handle);

	std::error_code ec;
	auto json_all = JSON::Load(chunk.memory, ec);

	// There is no "get latest prerelease" API, so must get all the recent releases.
	auto& json = json_all[0];

	std::string new_version, asset_url;
	if (!ec)
	{
		new_version = json["tag_name"].ToString();
		for (auto& asset_json : json["assets"].ArrayRange())
		{
			if (asset_json["name"].ToString().find(channel) != std::string::npos)
			{
				asset_url = asset_json["browser_download_url"].ToString();
				break;
			}
		}
	}

	free(chunk.memory);

	return {new_version, asset_url};
#endif
}

#ifndef UPDATER_USES_PYTHON

static bool download_file(std::string url, fs::path dest, std::string& error)
{
	CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
	if (!curl)
	{
		error = "Failed to init curl";
		return false;
	}

	fp = fopen(dest.string().c_str(), "wb");
	if (!fp)
	{
		error = "Failed to open file";
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	fclose(fp);
	if (res != CURLE_OK) {
		error = curl_easy_strerror(res);
		return false;
	}

	return true;
}

static bool open_zip_file(mz_zip_archive& archive, std::vector<std::string>& files, fs::path zip_path, std::string& error)
{
	memset(&archive, 0, sizeof(archive));
	files.clear();

	if (!mz_zip_reader_init_file(&archive, zip_path.string().c_str(), 0))
	{
		error = mz_zip_get_error_string(archive.m_last_error);
		return false;
	}

	mz_zip_archive_file_stat info;
	for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&archive); ++i)
	{
		if (!mz_zip_reader_file_stat(&archive, i, &info))
		{
			error = mz_zip_get_error_string(archive.m_last_error);
			mz_zip_reader_end(&archive);
			return false;
		}

		if (mz_zip_reader_is_file_a_directory(&archive, i))
			continue;

		files.push_back(info.m_filename);
	}

	return true;
}

static bool unzip_file(mz_zip_archive& archive, fs::path dest, std::string& error)
{
	mz_zip_archive_file_stat info;
	for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&archive); ++i)
	{
		if (!mz_zip_reader_file_stat(&archive, i, &info))
		{
			error = mz_zip_get_error_string(archive.m_last_error);
			mz_zip_reader_end(&archive);
			return false;
		}

		if (mz_zip_reader_is_file_a_directory(&archive, i))
			continue;

		fs::path dest_path = dest / info.m_filename;
		if (dest_path.has_parent_path())
			fs::create_directories(dest_path.parent_path());

		if (!mz_zip_reader_extract_to_file(&archive, i, dest_path.string().c_str(), 0))
		{
			error = mz_zip_get_error_string(archive.m_last_error);
			mz_zip_reader_end(&archive);
			return false;
		}
	}

	mz_zip_reader_end(&archive);
	return true;
}

#endif

static bool install_release(std::string asset_url, bool use_cache, std::string& error)
{
#ifdef UPDATER_USES_PYTHON
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

	bool success = updater_output.find("Success!") != std::string::npos;
	if (!success)
		error = updater_output;
	return success;
#else
	fs::path root_dir = "";
	if (!fs::exists(root_dir / "base_config"))
	{
		error = "Unexpected root directory";
		return false;
	}

	fs::path cache_folder = root_dir / ".updater-cache";
	if (std::getenv("ZC_UPDATER_CACHE_FOLDER") != nullptr)
		cache_folder = std::getenv("ZC_UPDATER_CACHE_FOLDER");

	std::string cache_name = asset_url;
	util::sanitize(cache_name);
	fs::path zip_path = cache_folder / cache_name;
	if (!use_cache || !fs::exists(zip_path))
	{
		fs::create_directory(cache_folder);
		if (!download_file(asset_url, zip_path, error))
			return false;
	}

	mz_zip_archive archive;
	std::vector<std::string> files;
	if (!open_zip_file(archive, files, zip_path, error))
		return false;

	// Windows locks the filesystem of loaded binaries. We can rename the currently loaded binary files
	// so our news one can go to the right place. zapp.cpp will clean these up on startup.
	fs::path active_files_dir = root_dir / ".updater-active-files";
	fs::create_directory(active_files_dir);

	for (auto& file : files)
	{
		fs::path old_path = root_dir / file;
		if (!fs::exists(old_path))
			continue;

		fs::path new_path = active_files_dir / file;
		fs::create_directories(new_path.parent_path());
		fs::rename(old_path, new_path);
	}

	if (!unzip_file(archive, root_dir, error))
		return false;

	return true;
#endif
}

int32_t main(int32_t argc, char* argv[])
{
	common_main_setup(App::updater, argc, argv);

	bool cache = used_switch(argc, argv, "-cache") > 0;
	headless = used_switch(argc, argv, "-headless") > 0;

	if (used_switch(argc, argv, "-install"))
	{
		int asset_url_arg = used_switch(argc, argv, "-asset-url");
		if (!asset_url_arg)
			fatal("Missing required -asset-url");

		std::string error;
		bool success = install_release(argv[asset_url_arg + 1], cache, error);
		if (success)
			done("Success!");
		else
			fatal("Failed: " + error);
	}

#ifdef __APPLE__
	if (!is_in_osx_application_bundle() && argc > 0)
	{
        chdir(fs::path(argv[0]).parent_path().c_str());
    }
#endif

#ifdef UPDATER_USES_PYTHON
	require_python();
#endif

	auto [new_version, asset_url] = get_next_release();
	if (new_version.empty() || asset_url.empty())
	{
		fatal("Could not find next version");
	}

	if (used_switch(argc, argv, "-print-next-release"))
	{
		printf("tag_name %s\n", new_version.c_str());
		printf("asset_url %s\n", asset_url.c_str());
		exit(0);
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
		done("Success!");
	else
		fatal("Failed: " + error);

	return 0;
}
END_OF_MAIN()

// TODO: make this not needed to compile...
bool DragAspect = false;
double aspect_ratio = 0;
int window_min_width = 0, window_min_height = 0;
