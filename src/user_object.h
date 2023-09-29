#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include <map>
#include "base/zc_array.h"
#include "base/general.h"

enum class ScriptType;

struct user_abstract_obj
{
	virtual bool operator==(user_abstract_obj const&) const = default;
	ScriptType owned_type;
	int32_t owned_i;
	
	user_abstract_obj() : owned_type(ScriptType::None), owned_i(0)
	{}
	user_abstract_obj(ScriptType owned_type, int32_t owned_i)
		: owned_type(owned_type), owned_i(owned_i)
	{}
	
	void disown()
	{
		owned_type = ScriptType::None;
		owned_i = 0;
	}
	virtual void clear()
	{
		disown();
	}
	virtual void free_obj()
	{
		clear();
	}
	
	void own(ScriptType type, int32_t i);
	bool own_clear(ScriptType type, int32_t i);
	bool own_clear_any();
	bool own_clear_cont();
};

struct ArrayOwner : user_abstract_obj
{
	ArrayOwner();
	bool specOwned;
	bool specCleared;
	virtual void clear() override;
	void reown(ScriptType ty, int32_t i);
};

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
struct user_object : public user_abstract_obj
{
	bool reserved;
	std::vector<int32_t> data;
	size_t owned_vars;
	scr_func_exec destruct;
	
	user_object() : user_abstract_obj(),
		reserved(false), owned_vars(0)
	{}
	
	void prep(dword pc, ScriptType type, word script, int32_t i);
	
	void load_arrays(std::map<int32_t,ZScriptArray>& arrs);
	void save_arrays(std::map<int32_t,ZScriptArray>& arrs);
	bool isGlobal() const
	{
		return owned_type == ScriptType::None && owned_i == 0;
	}
	
	#ifdef IS_PLAYER
	void clear_nodestruct();
	virtual void clear() override;
	#endif
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

