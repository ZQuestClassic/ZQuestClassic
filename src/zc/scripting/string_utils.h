#ifndef ZC_SCRIPTING_STRING_UTILS_H_
#define ZC_SCRIPTING_STRING_UTILS_H_

#include "base/ints.h"
#include <cstdint>
#include <functional>
#include <string>

const char* zs_formatter(const char* format, int32_t arg, int32_t mindig, dword flags);
std::string zs_sprintf(const char* format, int32_t num_args, std::function<int32_t(int32_t,int32_t)> arg_getter);

void do_sprintf(const bool v, const bool varg);
void do_sprintfarr();

#endif
