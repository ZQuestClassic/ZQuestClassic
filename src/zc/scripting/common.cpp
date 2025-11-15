#include "base/expected.h"
#include "zc/zc_sys.h"

#include <fmt/format.h>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

// Gotten from 'https://fileinfo.com/filetypes/executable'
std::set<std::string> banned_extensions = {".xlm",".caction",".8ck", ".actc",".a6p", ".m3g",".run",".workflow",".otm",".apk",".fxp",".73k",".0xe",".exe",".cmd",".jsx",".scar",".wcm",".jar",".ebs2",".ipa",".xap",".ba_",".ac",".bin",".vlx",".icd",".elf",".xbap",".89k",".widget",".a7r",".ex_",".zl9",".cgi",".scr",".coffee",".ahk",".plsc",".air",".ear",".app",".scptd",".xys",".hms",".cyw",".ebm",".pwc",".xqt",".msl",".seed",".vexe",".ebs",".mcr",".gpu",".celx",".wsh",".frs",".vxp",".action",".com",".out",".gadget",".command",".script",".rfu",".tcp",".widget",".ex4",".bat",".cof",".phar",".rxe",".scb",".ms",".isu",".fas",".mlx",".gpe",".mcr",".mrp",".u3p",".js",".acr",".epk",".exe1",".jsf",".rbf",".rgs",".vpm",".ecf",".hta",".dld",".applescript",".prg",".pyc",".spr",".nexe",".server",".appimage",".pyo",".dek",".mrc",".fpi",".rpj",".iim",".vbs",".pif",".mel",".scpt",".csh",".paf",".ws",".mm",".acc",".ex5",".mac",".plx",".snap",".ps1",".vdo",".mxe",".gs",".osx",".sct",".wiz",".x86",".e_e",".fky",".prg",".fas",".azw2",".actm",".cel",".tiapp",".thm",".kix",".wsf",".vbe",".lo",".ls",".tms",".ezs",".ds",".n",".esh",".vbscript",".arscript",".qit",".pex",".dxl",".wpm",".s2a",".sca",".prc",".shb",".rbx",".jse",".beam",".udf",".mem",".kx",".ksh",".rox",".upx",".ms",".mam",".btm",".es",".asb",".ipf",".mio",".sbs",".hpf",".ita",".eham",".ezt",".dmc",".qpx",".ore",".ncl",".exopc",".smm",".pvd",".ham",".wpk"};

// If the path is valid, returns an absolute path under the quest "Files" directory.
expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file)
{
	// First check for non-portable path characters.
	static const char* invalid_chars = "<>|?*&^$#\":";
	if (auto index = user_path.find_first_of(invalid_chars) != std::string::npos)
	{
		return make_unexpected(fmt::format("Bad path: {} - invalid character {}", user_path, user_path[index]));
	}
	for (char c : user_path)
	{
		if (c < 32)
			return make_unexpected(fmt::format("Bad path: {} - invalid control character {:#x}", user_path, c));
	}

	// Any leading slashes are ignored.
	// This makes path always relative.
	const char* path = user_path.c_str();
	while (path[0] == '/' || path[0] == '\\')
		path++;

	// Normalize `user_path` and check if it accesses a parent path.
	auto files_path = fs::absolute(fs::path(qst_files_path));
	auto normalized_path = fs::path(path).lexically_normal();
	if (!normalized_path.empty() && normalized_path.begin()->string() == "..")
	{
		return make_unexpected(fmt::format("Bad path: {} (resolved to {}) - cannot access filesystem outside {} (too many ..?)",
			path, normalized_path.string(), files_path.string()));
	}

	auto resolved_path = files_path / normalized_path;

	// The above should be enough to guarantee that `resolved_path` is within
	// the quest "Files" folder, but check to be safe.
	auto mismatch_pair = std::mismatch(
		resolved_path.begin(), resolved_path.end(),
		files_path.begin(), files_path.end());
	bool is_subpath = mismatch_pair.second == files_path.end();
	if (!is_subpath)
	{
		return make_unexpected(fmt::format("Bad path: {} (resolved to {}) - cannot access filesystem outside {}",
			user_path, resolved_path.string(), files_path.string()));
	}

	// Any extension other than banned ones, including no extension, is allowed.
	if (is_file && resolved_path.has_extension())
	{
		auto ext = resolved_path.extension().string();
		if (banned_extensions.find(ext) != banned_extensions.end())
			return make_unexpected(fmt::format("Bad path: {} - banned extension", user_path));
	}

	if (is_file && !resolved_path.has_filename())
		return make_unexpected(fmt::format("Bad path: {} - missing filename", user_path));

	// https://stackoverflow.com/a/31976060/2788187
	if (is_file)
	{
		static auto banned_fnames = {
			"..", ".", "AUX", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6",
			"COM7", "COM8", "COM9", "CON", "LPT1", "LPT2", "LPT3", "LPT4",
			"LPT5", "LPT6", "LPT7", "LPT8", "LPT9", "NUL", "PRN",
		};

		auto stem = resolved_path.stem().string();
		auto fname = resolved_path.filename().string();
		bool banned = std::find(std::begin(banned_fnames), std::end(banned_fnames), stem) != std::end(banned_fnames);
		banned |= fname.ends_with(".") || fname.ends_with(" ");

		if (banned)
			return make_unexpected(fmt::format("Bad path: {} - banned filename", user_path));
	}

	return resolved_path.string();
}
