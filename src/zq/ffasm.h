#ifndef _FFASM_H_
#define _FFASM_H_

#include "base/zdefs.h"
#include <utility>
#include <string>
#include <list>

extern script_command command_list[];
extern script_variable variable_list[];

int32_t set_argument(char const* argbuf, script_data **script, int32_t com, int32_t argument);
int32_t parse_script_section(char const* combuf, char const* arg1buf, char const* arg2buf, script_data **script, int32_t com, int32_t &retcode, std::vector<int32_t> *vptr = nullptr, std::string *sptr = nullptr);
int32_t parse_script(script_data **script);
int32_t parse_script_file(script_data **script, const char *path, bool report_success);
int32_t parse_script_file(script_data **script, FILE* fscript, bool report_success);
int32_t parse_script_string(script_data **script, std::string const& scriptstr, bool report_success);
int32_t ffparse(char *string);

#endif

