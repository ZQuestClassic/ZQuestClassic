#ifndef ZC_SCRIPTING_DIRECTORY_H_
#define ZC_SCRIPTING_DIRECTORY_H_

#include "base/jwinfsel.h"
#include "user_object.h"

#include <optional>
#include <string>

#define MAX_USER_DIRS 256

struct user_dir : public user_abstract_obj
{
	FLIST* list;
	std::string filepath;

	~user_dir();

	void setPath(const char* buf);
	void refresh();
	int32_t size();
	bool get(int32_t index, char* buf);
};

void dirs_init();
user_dir* checkDir(uint32_t id,  bool skipError = false);

std::optional<int32_t> directory_get_register(int32_t reg);
bool directory_set_register(int32_t reg, int32_t value);
std::optional<int32_t> directory_run_command(word command);

#endif
