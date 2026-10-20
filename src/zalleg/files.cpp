#include "zalleg/files.h"
#include "base/util.h"
#include "core/fonts.h"
#include "zalleg/zsys.h"
#include <string>
#include <cstddef>
#include <optional>
#include <functional>
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

enum class FileMode
{
	Save,
	Open,
	Folder,
};

#ifdef HAS_NFD
// Set once the native dialog fails to open (e.g. Linux without xdg-desktop-portal), after which
// only the built-in dialog is used.
static bool native_dialog_failed;

static bool init_dialog()
{
	static bool initialized, tried;
	if (!initialized)
	{
		if (tried)
			return false;

		tried = true;
		if (NFD_Init() != NFD_OKAY)
			return false;
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

// TODO: while this function supports loading utf8 filenames (i think), the rest of ZC doesn't (at least not on Windows).
// One example of an issue is gme_load_file which uses fopen.
static nfdresult_t open_native_dialog_impl(FileMode mode, std::string initial_path, std::vector<nfdfilteritem_t> filters, std::string& path)
{
#ifdef _WIN32
	// Windows NFD requires absolute paths.
	std::string initial_path_str;

	if (initial_path.empty())
	{
		auto u8_str = fs::current_path().u8string();
		initial_path_str = std::string(u8_str.begin(), u8_str.end());
	}
	else
	{
		try
		{
			fs::path p(reinterpret_cast<const char8_t*>(initial_path.c_str()));
			auto u8_str = fs::weakly_canonical(p).u8string();
			initial_path_str = std::string(u8_str.begin(), u8_str.end());
		}
		catch (const fs::filesystem_error&)
		{
			auto u8_str = fs::current_path().u8string();
			initial_path_str = std::string(u8_str.begin(), u8_str.end());
		}
	}
	const char* initial_path_ = initial_path_str.c_str();
#else
	const char* initial_path_ = initial_path.c_str();
#endif
	nfdu8char_t* outPath;
	nfdresult_t result;

	if (mode == FileMode::Folder)
		result = NFD_PickFolderU8(&outPath, initial_path_);
	else if (mode == FileMode::Save)
		result = NFD_SaveDialogU8(&outPath, filters.data(), filters.size(), initial_path_, nullptr);
	else
		result = NFD_OpenDialogU8(&outPath, filters.data(), filters.size(), initial_path_);

	if (result == NFD_OKAY)
	{
		path = reinterpret_cast<const char*>(outPath);
		NFD_FreePathU8(outPath);
	}

	return result;
}

// Returns false if the native dialog could not be shown, in which case the caller should fall back
// to the built-in dialog. Otherwise, `path` is the selection, or std::nullopt if the user cancelled.
static bool open_native_dialog(FileMode mode, std::string initial_path, std::vector<filteritem_t>& filters, std::optional<std::string>& path)
{
	if (native_dialog_failed)
		return false;

	NFD_ClearError();
	nfdresult_t result = init_dialog() ? NFD_OKAY : NFD_ERROR;
	std::string selected_path;
	if (result == NFD_OKAY)
	{
		std::vector<nfdfilteritem_t> filters_nfd;
		for (auto& filter : filters)
			filters_nfd.push_back({filter.text.c_str(), filter.ext.c_str()});

#ifdef __APPLE__
		__block nfdresult_t block_result;
		__block std::string block_path;
		dispatch_sync(dispatch_get_main_queue(), ^{
			block_result = open_native_dialog_impl(mode, initial_path, filters_nfd, block_path);
		});
		result = block_result;
		selected_path = block_path;
#else
		result = open_native_dialog_impl(mode, initial_path, filters_nfd, selected_path);
#endif
	}

	if (result == NFD_ERROR)
	{
		if (auto error = NFD_GetError())
			Z_error("Error: %s", error);
		Z_message("Native file dialog unavailable, using the built-in file dialog instead\n");
		native_dialog_failed = true;
		jwin_file_select_disable_explorer();
		return false;
	}

	if (result == NFD_OKAY && !selected_path.empty())
		path = selected_path;
	else
		path = std::nullopt;
	return true;
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

static int getname(std::string prompt, std::string ext, EXT_LIST *list, char* path)
{
    int sel = 0;
    if (list == NULL)
        return jwin_file_select_ex(prompt.c_str(), path, ext.c_str(), 2048, -1, -1, get_zc_font(font_lfont));
    else
        return jwin_file_browse_ex(prompt.c_str(), path, list, &sel, 2048, -1, -1, get_zc_font(font_lfont));
}

#define FS_EXPLORER 6

// Shows the native dialog when requested (and available), otherwise the built-in one, whose
// "File Explorer" button switches to the native dialog. If the native dialog cannot be shown, the
// built-in one is used instead.
//
// `show_builtin_dialog` is given a buffer holding the initial path, writes the selection to it,
// and returns the dialog's result (0 for cancel).
static std::optional<std::string> prompt_for_path(FileMode mode, std::string ext, EXT_LIST *list, std::string initial_path, const std::function<int(char*)>& show_builtin_dialog)
{
	bool native = use_native_file_dialog;
	while (true)
	{
#ifdef HAS_NFD
		if (native)
		{
			auto filters = create_filter_list(ext, list);
			std::optional<std::string> path;
			if (open_native_dialog(mode, initial_path, filters, path))
				return path;
		}
#endif

		char path[4096];
		strcpy(path, initial_path.c_str());
		int ret = show_builtin_dialog(path);
		if (ret == FS_EXPLORER)
		{
			native = true;
			continue;
		}
		if (!ret)
			return std::nullopt;
		return path;
	}
}

std::optional<std::string> prompt_for_existing_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (!usefilename)
		trim_filename(initial_path);

	return prompt_for_path(FileMode::Open, ext, list, initial_path, [&](char* path) {
		return getname(prompt, ext, list, path);
	});
}

std::optional<std::string> prompt_for_existing_folder(std::string prompt, std::string initial_path, std::string ext)
{
	return prompt_for_path(FileMode::Folder, ext, nullptr, initial_path, [&](char* path) {
		return jwin_dfile_select_ex(prompt.c_str(), path, ext.c_str(), 2048, -1, -1, get_zc_font(font_lfont));
	});
}

std::optional<std::string> prompt_for_new_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename)
{
	if (!usefilename)
		trim_filename(initial_path);

	return prompt_for_path(FileMode::Save, ext, list, initial_path, [&](char* path) {
		return getname(prompt, ext, list, path);
	});
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
