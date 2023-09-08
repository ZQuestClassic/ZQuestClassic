#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include <map>
#include "base/zc_array.h"
typedef ZCArray<int32_t> ZScriptArray;

enum class ScriptType;

#define MAX_USER_OBJECTS 214748
struct scr_func_exec
{
	bool operator==(const scr_func_exec&) const = default;

	dword pc;
	dword thiskey;
	ScriptType type;
	int32_t i;
	word script;
	std::string name;
	
	scr_func_exec(){clear();}
	void clear();
	void execute();
	bool validate();
};
struct user_object
{
	bool operator==(const user_object&) const = default;

	bool reserved;
	// TODO: here and every other `owned_type`; can we replace -1 with ScriptType::None ?
	ScriptType owned_type;
	int32_t owned_i;
	std::vector<int32_t> data;
	size_t owned_vars;
	scr_func_exec destruct;
	
	user_object() : reserved(false), owned_type((ScriptType)-1), owned_i(0),
		owned_vars(0)
	{}
	
	void prep(dword pc, ScriptType type, word script, int32_t i);
	
	void clear(bool destructor = true);
	
	void disown()
	{
		owned_type = (ScriptType)-1;
		owned_i = 0;
	}
	void load_arrays(std::map<int32_t,ZScriptArray>& arrs);
	void save_arrays(std::map<int32_t,ZScriptArray>& arrs);
	bool isGlobal() const
	{
		return owned_type == (ScriptType)-1 && owned_i == 0;
	}
	
	void own(ScriptType type, int32_t i)
	{
		owned_type = type;
		owned_i = i;
	}
	void own_clear(ScriptType type, int32_t i)
	{
		if(owned_type == type && owned_i == i)
			clear();
	}
	void own_clear_any()
	{
		if(owned_type != (ScriptType)-1 || owned_i != 0)
			clear();
	}
};
struct saved_user_object
{
	bool operator==(const saved_user_object&) const = default;

	int32_t object_index;
	user_object obj;
	std::map<int32_t,ZScriptArray> held_arrays;
};
extern user_object script_objects[MAX_USER_OBJECTS];

#endif //ZC_USER_OBJECT

