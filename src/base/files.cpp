#include "base/files.h"
#include "base/fonts.h"
#include "base/zsys.h"
#include <string>
#include <cstddef>
#include <optional>
#include <filesystem>
#include <fmt/format.h>

#ifdef HAS_NFD
#include "nfd.h"
#endif

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#include <dispatch/queue.h>
#endif

namespace fs = std::filesystem;

char temppath[4096];

static bool use_native_file_dialog;

void set_always_use_native_file_dialog(bool active)
{
	use_native_file_dialog = active;
}

#ifdef HAS_NFD
static bool init_dialog()
{
	static bool initialized, tried;
	if (!initialized)
	{
		if (tried)
			return false;

		tried = true;
		auto result = NFD_Init();
		if (result != NFD_OKAY)
		{
			Z_error("Error: %s", NFD_GetError());
			return false;
		}
		atexit(NFD_Quit);
		initialized = true;
	}

	return true;
}

// In allegro 4 `parse_extension_string` allows , ; and space, despite only documenting that ; is supported.
// Convert to `,` which is what NFD expects.
// Allegro also ignores a leading dot.
static void normalize_extension_string(std::string& str)
{
	util::replchar(str, ';', ',');
	util::replchar(str, ' ', ',');
	if (str.starts_with('.'))
		str = str.substr(1);
}

struct filteritem_t
{
	std::string text, ext;
};

static std::vector<filteritem_t> create_filter_list(std::string ext, EXT_LIST *list)
{
	if (list == nullptr)
	{
		normalize_extension_string(ext);
		return {{"", ext}};
	}

	std::vector<filteritem_t> result;
	for (EXT_LIST* cur = list; cur->ext != nullptr; cur++)
	{
		std::string ext = cur->ext;
		normalize_extension_string(ext);
		result.push_back({cur->text, ext});
	}
	return result;
}

enum class FileMode
{
	Save,
	Open,
	Folder,
};

static std::optional<std::string> open_native_dialog_impl(FileMode mode, std::string initial_path, std::vector<nfdfilteritem_t> filters)
{
#ifdef _WIN32
	// Note: on Windows the last folder used is always the initial path - see https://github.com/btzy/nativefiledialog-extended/issues/132#issuecomment-1993512443
	// It also doesn't like paths of the form `./tilesets` (the leading `./`), and must be an absolute path if set ... for now lets just ignore this thing
	// entirely on Windows.
	// At least with NFD, there's no way to simply "open at this folder" due to this behavior.
	fs::path current_path = fs::current_path();
	std::string initial_path_str = current_path.string();
	const char* initial_path_ = initial_path_str.c_str();
#else
	const char* initial_path_ = initial_path.c_str();
#endif
	nfdchar_t *outPath;
	nfdresult_t result;

	if (mode == FileMode::Folder)
		result = NFD_PickFolder(&outPath, initial_path_);
	else if (mode == FileMode::Save)
		result = NFD_SaveDialog(&outPath, filters.data(), filters.size(), initial_path_, nullptr);
	else
		result = NFD_OpenDialog(&outPath, filters.data(), filters.size(), initial_path_);

	if (result == NFD_OKAY)
	{
		std::string path = outPath;
		NFD_FreePath(outPath);
		return path;
	}

	return std::nullopt;
}

static std::optional<std::string> open_native_dialog(FileMode mode, std::string initial_path, std::vector<filteritem_t>& filters)
{
	NFD_ClearError();
	if (!init_dialog())
		return std::nullopt;

	std::vector<nfdfilteritem_t> filters_nfd;
	for (auto& filter : filters)
		filters_nfd.push_back({filter.text.c_str(), filter.ext.c_str()});

#ifdef __APPLE__
	__block std::string path;
	dispatch_sync(dispatch_get_main_queue(), ^{
		if (auto r = open_native_dialog_impl(mode, initial_path, filters_nfd))
			path = *r;
	});
#else
	std::string path;
	if (auto r = open_native_dialog_impl(mode, initial_path, filters_nfd))
		path = *r;
#endif

	if (path.empty())
	{
		if (NFD_GetError())
			Z_error("Error: %s", NFD_GetError());
		return std::nullopt;
	}

	return path;
}
#endif

static void trim_filename(std::string& path)
{
	if (path.empty())
		return;

	int i = path.size();
	while (i >= 0 && path[i] != '\\' && path[i] != '/')
		path[i--] = 0;
}

static int getname_nogo(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
    int sel = 0;
    strcpy(temppath, initial_path.c_str());
    if (list == NULL)
        return jwin_file_select_ex(prompt.c_str(), temppath, ext.c_str(), 2048, -1, -1, get_zc_font(font_lfont));
    else
        return jwin_file_browse_ex(prompt.c_str(), temppath, list, &sel, 2048, -1, -1, get_zc_font(font_lfont));
}

static int getname(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	extern BITMAP *tmp_scr;
    blit(screen,tmp_scr,0,0,0,0,screen->w,screen->h);
    int ret = getname_nogo(prompt,ext,list,initial_path,usefilename);
    blit(tmp_scr,screen,0,0,0,0,screen->w,screen->h);
    return ret;
}

#define FS_EXPLORER 6

std::optional<std::string> prompt_for_existing_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (!usefilename)
		trim_filename(initial_path);

	if (!use_native_file_dialog)
	{
		int ret = getname(prompt, ext, list, initial_path, usefilename);
		if (ret != FS_EXPLORER)
		{
			if (!ret)
				return std::nullopt;
			return temppath;
		}
	}

#ifdef HAS_NFD
	auto filters = create_filter_list(ext, list);
	return open_native_dialog(FileMode::Open, initial_path, filters);
#else
	return std::nullopt;
#endif
}

std::optional<std::string> prompt_for_existing_folder(std::string prompt, std::string initial_path, std::string ext)
{
	if (!use_native_file_dialog)
	{
		extern BITMAP *tmp_scr;
		blit(screen,tmp_scr,0,0,0,0,screen->w,screen->h);

		char path[2048];
		strcpy(path, initial_path.c_str());
		int ret = jwin_dfile_select_ex(prompt.c_str(), path, ext.c_str(), 2048, -1, -1, get_zc_font(font_lfont));
		if (ret != FS_EXPLORER)
		{
			if (!jwin_dfile_select_ex(prompt.c_str(), path, ext.c_str(), 2048, -1, -1, get_zc_font(font_lfont)))
			{
				blit(tmp_scr,screen,0,0,0,0,screen->w,screen->h);
				return std::nullopt;
			}

			blit(tmp_scr,screen,0,0,0,0,screen->w,screen->h);
			return path;
		}
	}

#ifdef HAS_NFD
	std::vector<filteritem_t> filters;
	return open_native_dialog(FileMode::Folder, initial_path, filters);
#else
	return std::nullopt;
#endif
}

std::optional<std::string> prompt_for_new_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (!usefilename)
		trim_filename(initial_path);

	if (!use_native_file_dialog)
	{
		int ret = getname(prompt, ext, list, initial_path, usefilename);
		if (ret != FS_EXPLORER)
		{
			if (!ret)
				return std::nullopt;
			return temppath;
		}
	}

#ifdef HAS_NFD
	auto filters = create_filter_list(ext, list);
	return open_native_dialog(FileMode::Save, initial_path, filters);
#else
	return std::nullopt;
#endif
}

bool prompt_for_existing_file_compat(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (auto result = prompt_for_existing_file(prompt, ext, list, initial_path, usefilename))
	{
		strcpy(temppath, result->c_str());
		return true;
	}

	return false;
}

bool prompt_for_new_file_compat(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (auto result = prompt_for_new_file(prompt, ext, list, initial_path, usefilename))
	{
		strcpy(temppath, result->c_str());
		return true;
	}

	return false;
}
