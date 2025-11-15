#ifndef ZC_SCRIPTING_COMMON_H_
#define ZC_SCRIPTING_COMMON_H_

#include "base/expected.h"

#include <string>

expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file);

#endif
