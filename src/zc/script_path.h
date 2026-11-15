#ifndef ZC_SCRIPT_PATH_H_
#define ZC_SCRIPT_PATH_H_

#include "base/expected.h"
#include <string>

// If the path is valid, returns an absolute path under the quest "Files" directory.
expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file);
// For quests without qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE: the path stays relative to
// the current directory as it always has been, but must not leave it.
expected<std::string, std::string> parse_legacy_user_path(std::string user_path);

#endif
