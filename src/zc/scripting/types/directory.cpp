#include "zc/scripting/types/directory.h"

#include "base/util.h"
#include "zc/ffscript.h"
#include "zc/scripting/common.h"
#include "zc/scripting/script_object.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

UserDataContainer<user_dir, MAX_USER_DIRS> user_dirs = {script_object_type::dir, "directory"};

void do_loaddirectory()
{
	int32_t arrayptr = get_register(sarg1);
	string user_path;
	ArrayH::getString(arrayptr, user_path, 2048);

	std::string resolved_path;
	if (auto r = parse_user_path(user_path, false); !r)
	{
		scripting_log_error_with_context("Error: {}", r.error());
		return;
	} else resolved_path = r.value();

	if (checkPath(resolved_path.c_str(), true))
	{
		ri->directoryref = user_dirs.get_free();
		if(!ri->directoryref) return;

		user_dir* d = checkDir(ri->directoryref, true);
		set_register(sarg1, ri->directoryref);
		d->setPath(resolved_path.c_str());
		return;
	}

	scripting_log_error_with_context("Path '{}' points to a file; must point to a directory!", resolved_path);
	ri->directoryref = 0;
	set_register(sarg1, 0);
}

void do_directory_get()
{
	int32_t indx = get_register(sarg1) / 10000L;
	int32_t arrayptr = get_register(sarg2);

	if (auto dir = checkDir(GET_REF(directoryref), true))
	{
		char buf[2048] = {0};
		set_register(sarg1, dir->get(indx, buf) ? 10000L : 0L);
		if(ArrayH::setArray(arrayptr, string(buf)) == SH::_Overflow)
			scripting_log_error_with_context("Array is not large enough");
	}
	else set_register(sarg1, 0L);
}

void do_directory_reload()
{
	if (auto dir = checkDir(GET_REF(directoryref), true))
		dir->refresh();
}

void do_directory_free()
{
	if (auto dir = checkDir(GET_REF(directoryref), true))
		free_script_object(dir->id);
}

}

user_dir::~user_dir()
{
	if (list)
	{
		list->clear();
		free(list);
		list = NULL;
	}
}

void user_dir::setPath(const char* buf)
{
	if(!list)
	{
		list = (FLIST *) calloc(1, sizeof(FLIST));
	}
	filepath = std::string(buf) + "/";
	util::regulate_path(filepath);
	list->load(filepath.c_str());
}

void user_dir::refresh()
{
	if(list)
		list->load(filepath.c_str());
	else setPath(filepath.c_str());
}

int32_t user_dir::size()
{
	return list->size;
}

bool user_dir::get(int32_t index, char* buf)
{
	return list->get(index, buf);
}

user_dir* checkDir(uint32_t id,  bool skipError)
{
	return user_dirs.check(id, skipError);
}

void dirs_init()
{
	user_dirs.clear();
}

std::optional<int32_t> directory_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case DIRECTORYSIZE:
		{
			if (auto dir = checkDir(GET_REF(directoryref), true))
				ret = dir->size() * 10000L;
			break;
		}

		default: return std::nullopt;
	}

	return ret;
}

bool directory_set_register(int32_t reg, int32_t value)
{
	return false;
}

std::optional<int32_t> directory_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case LOADDIRECTORYR:
		{
			do_loaddirectory();
			break;
		}
		case DIRECTORYGET:
		{
			do_directory_get();
			break;
		}
		case DIRECTORYRELOAD:
		{
			do_directory_reload();
			break;
		}
		case DIRECTORYFREE:
		{
			do_directory_free();
			break;
		}
		case DIRECTORYOWN:
		{
			if (auto dir = checkDir(GET_REF(directoryref)))
				own_script_object(dir, type, i);
			break;
		}

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}
