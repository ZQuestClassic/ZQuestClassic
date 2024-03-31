#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include <map>
#include "base/zc_array.h"
#include "base/general.h"

enum class ScriptType;

enum class script_object_type
{
	bitmap,
	dir,
	file,
	object,
	paldata,
	rng,
	stack,
	websocket,
};

struct user_abstract_obj
{
	virtual bool operator==(user_abstract_obj const&) const = default;
	int32_t ref_count;
	script_object_type type;
	uint32_t id;
	ScriptType owned_type;
	int32_t owned_i;

	virtual ~user_abstract_obj() = default;

	void disown()
	{
		owned_type = ScriptType::None;
		owned_i = 0;
	}
	void clear()
	{
		disown();
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
	void reset();
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
	std::vector<int32_t> data;
	size_t owned_vars;
	std::vector<bool> var_is_object;
	scr_func_exec destruct;
	bool global;
	bool fake;
	
	void prep(dword pc, ScriptType type, word script, int32_t i);
	
	void load_arrays(std::map<int32_t,ZScriptArray>& arrs);
	void save_arrays(std::map<int32_t,ZScriptArray>& arrs);
	void setGlobal(bool value)
	{
		global = value;
	}
	bool isGlobal() const
	{
		return global;
	}
	bool isMemberObjectType(size_t index)
	{
		if (index < owned_vars)
		{
			return var_is_object.size() > index && var_is_object[index];
		}
		return false;
	}
	
	#ifdef IS_PLAYER
	void clear_nodestruct();
	~user_object();
	#endif
};
struct saved_user_object
{
	bool operator==(const saved_user_object&) const = default;

	user_object obj;
	std::map<int32_t,ZScriptArray> held_arrays;
};

#endif //ZC_USER_OBJECT

