#include "base/about.h"
#include "base/util.h"
#include "base/version.h"
#include "zalleg/zalleg.h"
#include "base/zapp.h"
#include "allegro5/allegro_native_dialog.h"
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <fmt/format.h>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "miniz.h"

using json = nlohmann::json;

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

namespace fs = std::filesystem;

static bool headless;
static std::string repo = getRepo();
static std::string platform = getReleasePlatform();
static std::string channel = getReleaseChannel();
static std::string current_version = getVersionString();

std::ofstream out("updater.log", std::ios::binary);

[[noreturn]] static void fatal(std::string msg)
{
	if (headless)
		printf("[fatal] %s\n", msg.c_str());
	else
		al_show_native_message_box(all_get_display(), "ZQuest Classic: I AM ERROR", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_ERROR);

	out << "[fatal] " << msg.c_str() << '\n';
	out.close();
	zapp_exit(1);
}

[[noreturn]] static void done(std::string msg)
{
	if (headless)
		printf("[done] %s\n", msg.c_str());
	else
		al_show_native_message_box(all_get_display(), "ZQuest Classic Updater", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL);

	out << "[done] " << msg.c_str() << '\n';
	out.close();
	zapp_exit(0);
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

// Returns the latest release of the configured channel.
static std::tuple<std::string, std::string> get_next_release()
{
	std::string json_url = fmt::format("https://zquestclassic.com/releases/{}.json", channel);

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

	std::string tag_name, asset_url;
	if (auto data = json::parse(chunk.memory, nullptr, false); !data.is_discarded())
	{
		tag_name = data["tagName"].template get<std::string>();
		for (auto& asset_json : data["assets"])
		{
			std::string name = asset_json["name"].template get<std::string>();
			if (name.find(platform) != std::string::npos)
			{
				asset_url = asset_json["url"].template get<std::string>();
				break;
			}
		}
	}

	free(chunk.memory);

	return {tag_name, asset_url};
}

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

static bool install_release(std::string asset_url, bool use_cache, std::string& error)
{
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

	auto [new_version, asset_url] = get_next_release();
	if (new_version.empty() || asset_url.empty())
	{
		fatal("Could not find next version");
	}

	if (used_switch(argc, argv, "-print-next-release"))
	{
		printf("tag_name %s\n", new_version.c_str());
		printf("asset_url %s\n", asset_url.c_str());
		zapp_exit(0);
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
void zprint2(const char * const format,...)
{
	char buf[8192];
	
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, 8192, format, ap);
	va_end(ap);

	printf("%s", buf);
}
