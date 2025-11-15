#ifndef ZC_SCRIPTING_FILE_H_
#define ZC_SCRIPTING_FILE_H_

#include "../../user_object.h"

#include <cstdio>
#include <optional>
#include <string>

#define MAX_USER_FILES 256

struct user_file : public user_abstract_obj
{
	FILE* file;
	std::string filepath;

	~user_file();

	void close();
	int32_t do_remove();
	void setPath(const char* buf);
};

void files_init();
user_file* checkFile(int32_t ref, bool req_file = false, bool skipError = false);

std::optional<int32_t> file_get_register(int32_t reg);
bool file_set_register(int32_t reg, int32_t value);
std::optional<int32_t> file_run_command(word command);

#endif
