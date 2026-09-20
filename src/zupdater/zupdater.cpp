#include "base/about.h"
#include "base/util.h"
#include "base/zc_alleg.h"
#include "base/zapp.h"
#include "allegro5/allegro_native_dialog.h"
#include <filesystem>
#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <fmt/format.h>

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

namespace fs = std::filesystem;

static bool headless;
static std::string platform = getReleasePlatform();
static std::string channel = getReleaseChannel();
static std::string current_version = getReleaseTag();

static std::ofstream out;

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

	// Only an explicit "yes" (1) may start an update. A dialog that could not be
	// shown returns 0, and that must not be taken for consent.
	int ret = al_show_native_message_box(all_get_display(), "ZQuest Classic Updater", "", msg.c_str(), NULL, ALLEGRO_MESSAGEBOX_YES_NO);
	return ret == 1;
}

// Returns the latest release of the configured channel.
static std::tuple<std::string, std::string> get_next_release()
{
	std::string json_url = fmt::format("https://zquestclassic.com/releases/{}.json", channel);

	struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);
	if (!chunk.memory)
		return {};
	// The parser below reads this buffer as a NUL-terminated string, and the write
	// callback never runs at all for an empty response body.
	chunk.memory[0] = 0;
	chunk.size = 0;

	CURL *curl_handle = curl_easy_init();
	if (!curl_handle)
	{
		free(chunk.memory);
		return {};
	}

	curl_easy_setopt(curl_handle, CURLOPT_URL, json_url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);
	CURLcode res = curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		free(chunk.memory);
		return {};
	}

	std::string tag_name, asset_url;
	// A response that parses but does not have the expected schema must not be
	// read as a release. Every field is checked before it is used, and the
	// non-throwing conversions are taken throughout, so nothing here can escape.
	std::error_code ec;
	auto data = JSON::Load(chunk.memory, ec);
	if (!ec && data.IsObject())
	{
		std::error_code conv_ec;
		if (data.hasKey("tagName") && data["tagName"].IsString())
			tag_name = data["tagName"].ToString(conv_ec);

		if (data.hasKey("assets") && data["assets"].IsArray())
		{
			for (auto& asset_json : data["assets"].ArrayRange())
			{
				if (!asset_json.IsObject())
					continue;
				if (!asset_json.hasKey("name") || !asset_json["name"].IsString())
					continue;
				if (!asset_json.hasKey("url") || !asset_json["url"].IsString())
					continue;

				if (asset_json["name"].ToString(conv_ec).find(platform) != std::string::npos)
				{
					asset_url = asset_json["url"].ToString(conv_ec);
					break;
				}
			}
		}
	}

	free(chunk.memory);

	return {tag_name, asset_url};
}

static bool download_file(std::string url, fs::path dest, std::string& error)
{
	CURL *curl = curl_easy_init();
	if (!curl)
	{
		error = "Failed to init curl";
		return false;
	}

	// Download to a temporary path and only move it into place once the whole
	// transfer succeeded. An interrupted download - or an HTTP error page, which
	// curl writes to the file just like any other body - would otherwise be left
	// at `dest`, where -cache reuses it forever.
	fs::path temp_dest = dest;
	temp_dest += ".part";

	FILE *fp = fopen(temp_dest.string().c_str(), "wb");
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
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	CURLcode res = curl_easy_perform(curl);

	long response_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	curl_easy_cleanup(curl);
	fclose(fp);

	std::error_code ec;
	if (res != CURLE_OK)
	{
		error = curl_easy_strerror(res);
		fs::remove(temp_dest, ec);
		return false;
	}

	if (response_code >= 400)
	{
		error = fmt::format("download failed with status {}", response_code);
		fs::remove(temp_dest, ec);
		return false;
	}

	fs::rename(temp_dest, dest, ec);
	if (ec)
	{
		error = ec.message();
		fs::remove(temp_dest, ec);
		return false;
	}

	return true;
}

struct ZipEntry
{
	unsigned int index;
	std::string name;
};

// Rejects anything that would extract outside the destination folder.
static bool is_safe_archive_path(const std::string& name)
{
	if (name.empty())
		return false;
	// Zip entries always use '/'. A backslash is a separator on Windows, so an
	// entry containing one could climb out of the destination folder there.
	if (name.find('\\') != std::string::npos)
		return false;
	// Ex: "C:/Windows/...". `dest / name` discards `dest` entirely for these.
	if (name.size() >= 2 && name[1] == ':')
		return false;

	fs::path path(name);
	if (path.is_absolute() || path.has_root_name() || path.has_root_directory())
		return false;
	for (const auto& part : path)
	{
		if (part == "..")
			return false;
	}

	return true;
}

static bool open_zip_file(mz_zip_archive& archive, std::vector<ZipEntry>& files, fs::path zip_path, std::string& error)
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

		if (!is_safe_archive_path(info.m_filename))
		{
			error = fmt::format("unsafe path in archive: {}", info.m_filename);
			mz_zip_reader_end(&archive);
			return false;
		}

		files.push_back({i, info.m_filename});
	}

	return true;
}

// Every path written is appended to `extracted`, before it is written, so a failed
// install can clean up after itself.
static bool unzip_file(mz_zip_archive& archive, const std::vector<ZipEntry>& files, fs::path dest, std::vector<fs::path>& extracted, std::string& error)
{
	for (const auto& file : files)
	{
		fs::path dest_path = dest / file.name;
		if (dest_path.has_parent_path())
			fs::create_directories(dest_path.parent_path());

		extracted.push_back(dest_path);
		if (!mz_zip_reader_extract_to_file(&archive, file.index, dest_path.string().c_str(), 0))
		{
			error = mz_zip_get_error_string(archive.m_last_error);
			mz_zip_reader_end(&archive);
			return false;
		}
	}

	mz_zip_reader_end(&archive);
	return true;
}

// Each release archive is tens of megabytes and nothing else ever cleans this
// folder up, so only the one just installed is worth keeping. Best effort, and
// nothing here may throw: the install already succeeded by the time this runs.
static void prune_cache(const fs::path& cache_folder, const fs::path& keep)
{
	std::error_code ec;
	for (fs::directory_iterator it(cache_folder, ec); !ec && it != fs::directory_iterator(); it.increment(ec))
	{
		std::error_code ignored;
		if (it->path() != keep && it->is_regular_file(ignored))
			fs::remove(it->path(), ignored);
	}
}

static bool install_release_impl(std::string asset_url, bool use_cache, std::string& error)
{
	fs::path root_dir = "";
	if (!fs::exists(root_dir / "base_config"))
	{
		error = "Unexpected root directory";
		return false;
	}

	fs::path cache_folder = root_dir / ".updater-cache";
	const char* cache_folder_override = std::getenv("ZC_UPDATER_CACHE_FOLDER");
	if (cache_folder_override)
		cache_folder = cache_folder_override;

	std::string cache_name = asset_url;
	util::sanitize(cache_name);
	fs::path zip_path = cache_folder / cache_name;

	mz_zip_archive archive;
	std::vector<ZipEntry> files;
	bool opened = false;

	// A cached archive is never trusted: one that does not open is thrown away and
	// downloaded again, otherwise a single bad file wedges the updater for good.
	if (use_cache && fs::exists(zip_path))
	{
		std::string cache_error;
		opened = open_zip_file(archive, files, zip_path, cache_error);
		if (!opened)
		{
			std::error_code ec;
			fs::remove(zip_path, ec);
		}
	}

	if (!opened)
	{
		fs::create_directories(cache_folder);
		if (!download_file(asset_url, zip_path, error))
			return false;

		if (!open_zip_file(archive, files, zip_path, error))
		{
			std::error_code ec;
			fs::remove(zip_path, ec);
			return false;
		}
	}

	// Windows locks the filesystem of loaded binaries. We can rename the currently loaded binary files
	// so our new ones can go to the right place. zapp.cpp will clean these up on startup.
	fs::path active_files_dir = root_dir / ".updater-active-files";

	// Every file moved aside, and every file written, so that a failure part way
	// through can put the installation back the way it was. zapp.cpp deletes
	// .updater-active-files on the next startup of any app, so without a rollback
	// the only copy of each displaced file is lost and the install is unrecoverable.
	std::vector<std::pair<fs::path, fs::path>> moved;
	std::vector<fs::path> extracted;

	// Undoes the install and fills in `error`. A file that cannot be put back is
	// still in .updater-active-files, but only until the next app startup, so the
	// error tells the user to rescue it before then.
	auto fail = [&](std::string why) {
		std::error_code ec;
		for (auto it = extracted.rbegin(); it != extracted.rend(); ++it)
			fs::remove(*it, ec);

		int unrestored = 0;
		for (auto it = moved.rbegin(); it != moved.rend(); ++it)
		{
			fs::rename(it->second, it->first, ec);
			if (ec)
			{
				out << "[rollback] could not restore " << it->first.string() << ": " << ec.message() << '\n';
				unrestored++;
			}
		}

		error = why;
		if (unrestored)
			error += fmt::format(" ({} original files could not be restored, see updater.log."
				" Copy them out of .updater-active-files before starting any ZQuest Classic app, or they will be deleted)", unrestored);
		return false;
	};

	try
	{
		fs::create_directories(active_files_dir);

		for (auto& file : files)
		{
			fs::path old_path = root_dir / file.name;
			if (!fs::exists(old_path))
				continue;

			fs::path new_path = active_files_dir / file.name;
			fs::create_directories(new_path.parent_path());
			fs::rename(old_path, new_path);
			moved.push_back({old_path, new_path});
		}

		if (!unzip_file(archive, files, root_dir, extracted, error))
		{
			// The archive opened, but one of its entries would not extract. Keeping
			// it would make every later attempt fail the same way.
			std::error_code ec;
			fs::remove(zip_path, ec);
			return fail(error);
		}
	}
	catch (const std::exception& e)
	{
		mz_zip_reader_end(&archive);
		return fail(e.what());
	}

	// A folder named by ZC_UPDATER_CACHE_FOLDER is not the updater's to clean.
	if (!cache_folder_override)
		prune_cache(cache_folder, use_cache ? zip_path : fs::path());

	return true;
}

static bool install_release(std::string asset_url, bool use_cache, std::string& error)
{
	// The filesystem calls here throw rather than reporting through `error`, and an
	// uncaught one in a /SUBSYSTEM:WINDOWS binary is a silent crash.
	try
	{
		return install_release_impl(asset_url, use_cache, error);
	}
	catch (const std::exception& e)
	{
		if (error.empty())
			error = e.what();
		return false;
	}
}

int32_t main(int32_t argc, char* argv[])
{
	common_main_setup(App::updater, argc, argv);
	// common_main_setup moves into the install folder, which is where this belongs.
	out.open("updater.log", std::ios::binary);

	bool cache = used_switch(argc, argv, "-cache") > 0;
	headless = used_switch(argc, argv, "-headless") > 0;

	if (used_switch(argc, argv, "-install"))
	{
		int asset_url_arg = used_switch(argc, argv, "-asset-url");
		if (!asset_url_arg)
			fatal("Missing required -asset-url");
		if (asset_url_arg + 1 >= argc)
			fatal("Missing value for -asset-url");

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
