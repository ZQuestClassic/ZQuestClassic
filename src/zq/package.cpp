#include "zq/package.h"
#include "base/util.h"
#include "base/zc_alleg.h"
#include <allegro5/allegro.h>
#include <fmt/format.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

#ifdef _WIN32

#pragma pack( push )
#pragma pack( 2 )
typedef struct GRPICONDIRENTRY
{
    BYTE  bWidth;
    BYTE  bHeight;
    BYTE  bColorCount;
    BYTE  bReserved;
    WORD  wPlanes;
    WORD  wBitCount;
    DWORD dwBytesInRes;
    WORD  nId;
} GRPICONDIRENTRY;
typedef struct GRPICONDIR
{
    WORD idReserved;
    WORD idType;
    WORD idCount;
    GRPICONDIRENTRY idEntries[];
} GRPICONDIR;

#pragma pack( pop )


static bool set_icon(std::wstring exe_path, std::wstring icon_path)
{
	std::ifstream file(icon_path, std::ios::binary);
	std::vector<char> icon((std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());

	HANDLE exe = BeginUpdateResourceW(exe_path.c_str(), FALSE);
	if (!exe)
		return false;

	size_t szIconHeader = sizeof(GRPICONDIR) + sizeof(GRPICONDIRENTRY[1]);
	GRPICONDIR *icon_header = (GRPICONDIR*)malloc(szIconHeader);
	if (!icon_header)
		return false;

	icon_header->idReserved = 0;
	icon_header->idType = 1;
	icon_header->idCount = 1;
	icon_header->idEntries[0].bWidth = 0;
	icon_header->idEntries[0].bHeight = 0;
	icon_header->idEntries[0].bColorCount = 0;
	icon_header->idEntries[0].bReserved = 0;
	icon_header->idEntries[0].wPlanes = 0;
	icon_header->idEntries[0].wBitCount = 32;
	icon_header->idEntries[0].dwBytesInRes = icon.size();
	icon_header->idEntries[0].nId = 1;

	bool success = UpdateResourceW(exe, 
		(LPCWSTR)RT_GROUP_ICON,
		(LPCWSTR)MAKEINTRESOURCE(42),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPVOID)icon_header,
		szIconHeader);
	free(icon_header);
	if (!success)
	{
		EndUpdateResource(exe, FALSE);
		return false;
	}

	success = UpdateResourceW(exe, 
		(LPCWSTR)RT_ICON,
		(LPCWSTR)MAKEINTRESOURCE(1),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		icon.data(),
		icon.size());
	if (!success)
	{
		EndUpdateResource(exe, FALSE);
		return false;
	}

	return EndUpdateResource(exe, FALSE);
}
#endif

std::optional<std::string> package_create(std::string quest_path_, std::string package_name)
{
#ifdef _WIN32
	auto root_dir = fs::path("");
	auto quest_path = fs::path(quest_path_);
	auto quest_dir = quest_path.parent_path();
	auto package_dir = root_dir / "packages" / package_name;
	auto extra_dir = root_dir / "packages" / (package_name + "_extra");
	auto data_dir = package_dir / "data";

	std::error_code ec;
	fs::remove_all(package_dir, ec);
	if (ec)
		return fmt::format("Failed to clear {}: {}", package_dir.string(), std::strerror(ec.value()));

	fs::create_directories(data_dir, ec);
	if (ec)
		return fmt::format("Failed to create {}: {}", data_dir.string(), std::strerror(ec.value()));

	fs::copy(quest_path, data_dir);
	fs::copy(root_dir / "zplayer.exe", data_dir / "zplayer.exe");
	fs::copy(root_dir / "zconsole.exe", data_dir / "zconsole.exe");
	fs::copy(root_dir / "allegro5.cfg", data_dir / "allegro5.cfg");
	fs::copy(root_dir / "sfx.dat", data_dir / "sfx.dat");
	fs::copy(root_dir / "base_config", data_dir / "base_config", fs::copy_options::recursive);
	fs::copy(root_dir / "customfonts", data_dir / "customfonts", fs::copy_options::recursive);
	fs::copy(root_dir / "modules", data_dir / "modules", fs::copy_options::recursive);
	fs::copy(root_dir / "assets", data_dir / "assets", fs::copy_options::recursive);
	fs::copy(root_dir / "licenses", data_dir / "licenses", fs::copy_options::recursive);
	if (fs::exists(root_dir / "crashpad_handler.exe"))
		fs::copy(root_dir / "crashpad_handler.exe", data_dir / "crashpad_handler.exe");

	// Copy all .dlls
	ALLEGRO_FS_ENTRY *entry = al_create_fs_entry("");
	if (!(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR))
		return "Failed to copy dlls";
	if (!al_open_directory(entry)) {
		al_trace("Error opening directory: %s\n", al_get_fs_entry_name(entry));
		return "Failed to copy dlls";
	}
	ALLEGRO_FS_ENTRY *next;
	while (true)
	{
		next = al_read_directory(entry);
		if (!next)
			break;
		
		const char* name = al_get_fs_entry_name(next);
		auto path = root_dir / name;
		if (path.extension() == ".dll")
		{
			fs::copy(path, data_dir / path.filename());
		}
		al_destroy_fs_entry(next);
	}
	al_close_directory(entry);

	std::ofstream out(data_dir / "zc_args.txt", std::ios::binary);
	out << "-only " << std::quoted(quest_path.filename().string()) << " ";
	out << "-window-title " << std::quoted(package_name);
	out.close();

	if (fs::exists(extra_dir))
	{
		fs::copy(extra_dir, data_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
	}

	auto exe_path = package_dir / fs::path(package_name).replace_extension("exe");
	fs::copy(root_dir / "zstandalone.exe", exe_path);

	auto icon_path = extra_dir / "icon.png";
	// TODO: support .ico
	// https://stackoverflow.com/questions/20729156/find-out-number-of-icons-in-an-icon-resource-using-win32-api
	// https://en.wikipedia.org/wiki/ICO_(file_format)
	// https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083
	// Basically, I need to extract each icon inside a .ico container to a RT_ICON.
	// if (!fs::exists(icon_path))
	// 	icon_path = extra_dir / "icon.ico";
	if (!fs::exists(icon_path))
		icon_path = root_dir / "assets/zc/ZC_Icon.png";
	if (fs::exists(icon_path))
	{
		set_icon(exe_path.wstring(), icon_path.wstring());
		if (icon_path == extra_dir / "icon.png")
			fs::copy(icon_path, data_dir, fs::copy_options::overwrite_existing);
	}

	return std::nullopt;
#else
	return "Not implemented";
#endif
}
