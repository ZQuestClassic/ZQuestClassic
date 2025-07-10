#ifndef ZC_SCRIPTING_USER_OBJECT_H_
#define ZC_SCRIPTING_USER_OBJECT_H_

#include "base/ints.h"

#include <optional>
#include <vector>

struct user_object;

void register_existing_user_object(user_object* object);
std::vector<user_object*> get_user_objects();

void user_object_init();
std::optional<int32_t> user_object_run_command(word command);

#endif
