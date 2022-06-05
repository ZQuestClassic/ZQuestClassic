#include "zapp.h"
#include <filesystem>
#include <unistd.h>

#if __APPLE__
#include "osx/FolderManager.h"
#endif

void common_main_setup(int argc, char **argv)
{
    // This allows for opening a binary from Finder and having ZC be in its expected
    // working directory (the same as the binary). Only used when not inside an application bundle,
    // and only for testing purposes really. See comment about `SKIP_APP_BUNDLE` in buildpack_osx.sh
#ifdef __APPLE__
    if (!is_in_osx_application_bundle() && argc > 0) {
        chdir(std::filesystem::path(argv[0]).parent_path().c_str());
    }
#endif
}

bool is_in_osx_application_bundle()
{
#ifdef __APPLE__
    return std::filesystem::current_path().string().find("/ZeldaClassic.app/") != std::string::npos;
#else
    return false;
#endif
}

// On windows, the working directory is the same as where the .exe is, and user-files (zc.sav, cfg, quests)
// are allowed to be written in the same directory.
// On OSX, the working directory is the application bundle Resources folder, but we are not allowed to write
// inside the application bundle. Instead, we write to the application support folder.
// To make these two platforms work, any file path that is meant for user-files should use this function.
// On OSX it'll be prefixed with the application support folder, otherwise it will just return the path
// (making it a relative path, like normal).
std::string get_user_data_directory()
{
#ifdef __APPLE__
    if (is_in_osx_application_bundle())
    {
        fm::FolderManager folderManager;
        std::string app_support_folder = (char *)folderManager.pathForDirectory(fm::NSApplicationSupportDirectory, fm::NSUserDomainMask);
        return app_support_folder + "/com.zeldaclassic.www";
    }
#endif

    return "";
}

std::string get_user_data_path(std::string path)
{
    std::string dir = get_user_data_directory();
    if (dir.empty()) return path;
    return dir + "/" + path;
}
