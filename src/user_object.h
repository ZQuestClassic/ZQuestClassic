#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include <map>
#include "base/zc_array.h"
typedef ZCArray<int32_t> ZScriptArray;

#define MAX_USER_OBJECTS 214748
struct scr_func_exec
{
	dword pc;
	dword thiskey;
	int32_t type, i;
	word script;
	std::string name;
	
	scr_func_exec(){clear();}
	void clear();
	void execute();
	bool validate();
};
struct user_object
{
	bool reserved;
	int32_t owned_type, owned_i;
	std::vector<int32_t> data;
	size_t owned_vars;
	scr_func_exec destruct;
	
	user_object() : reserved(false), owned_type(-1), owned_i(0),
		owned_vars(0)
	{}
	
	void prep(dword pc, int32_t type, word script, int32_t i);
	
	void clear(bool destructor = true);
	
	void disown()
	{
		owned_type = -1;
		owned_i = 0;
	}
	void load_arrays(std::map<int32_t,ZScriptArray>& arrs);
	void save_arrays(std::map<int32_t,ZScriptArray>& arrs);
	bool isGlobal() const
	{
		return owned_type == -1 && owned_i == 0;
	}
	
	void own(int32_t type, int32_t i)
	{
		owned_type = type;
		owned_i = i;
	}
	void own_clear(int32_t type, int32_t i)
	{
		if(owned_type == type && owned_i == i)
			clear();
	}
	void own_clear_any()
	{
		if(owned_type != -1 || owned_i != 0)
			clear();
	}
};
struct saved_user_object
{
	int32_t object_index;
	user_object obj;
	std::map<int32_t,ZScriptArray> held_arrays;
};
extern user_object script_objects[MAX_USER_OBJECTS];

#endif //ZC_USER_OBJECT

