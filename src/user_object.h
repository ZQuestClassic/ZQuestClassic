#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include "allegro/debug.h"
#include "base/check.h"
#include "base/zc_array.h"
#include "base/general.h"
#include <map>
#include <vector>

enum class ScriptType;
struct zasm_script;
class sprite;

// Only script arrays and custom user objects can be restored right now.
// All other object types are set to NULL.
bool can_restore_object_type(script_object_type type);

struct user_abstract_obj
{
	virtual bool operator==(user_abstract_obj const&) const = default;

	int32_t ref_count;
	script_object_type type;
	uint32_t id;
	ScriptType owned_type;
	int32_t owned_i;
	int32_t owned_sprite_id;
	bool global;

	virtual ~user_abstract_obj() = default;

	virtual void get_retained_ids(std::vector<uint32_t>& ids) {};
	virtual void restore_references() {};

	void disown()
	{
		owned_type = ScriptType::None;
		owned_i = 0;
		owned_sprite_id = 0;
	}

	bool is_owned()
	{
		return owned_type != ScriptType::None || owned_sprite_id;
	}

	void clear()
	{
		disown();
	}

	void set_owned_by_script(ScriptType type, int32_t i);
	void set_owned_by_sprite(sprite* sprite);
	bool script_own_clear(ScriptType type, int32_t i);
	bool script_own_clear_any();
	bool script_own_clear_cont();
	bool sprite_own_clear(int32_t id);
};

struct ArrayOwner : user_abstract_obj
{
	ArrayOwner();
	bool specOwned;
	bool specCleared;
	void reset();
	void reown(ScriptType ty, int32_t i);
	void reown(sprite* spr);
};

struct script_array : public user_abstract_obj
{
	struct internal_array_id
	{
		bool operator==(const internal_array_id&) const = default;

		int zasm_var;
		int ref;

		bool matches(ScriptType script_type, int32_t uid) const;
		bool matches(ScriptType script_type) const;
	};

	ZScriptArray arr;
	std::optional<internal_array_id> internal_id;
	bool internal_expired;

#ifdef IS_PLAYER
	void get_retained_ids(std::vector<uint32_t>& ids);
	void restore_references();
#endif
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
	bool validate(const zasm_script* zasm_script);
};
struct user_object : public user_abstract_obj
{
	std::vector<int32_t> data;
	size_t owned_vars;
	std::vector<script_object_type> var_types;
	scr_func_exec destruct;
	zasm_script* script;

#ifdef IS_PLAYER
	void get_retained_ids(std::vector<uint32_t>& ids);
	void restore_references();
#endif
	
	void prep(dword pc, ScriptType type, word script, int32_t i);
	
	void load_arrays(const std::map<int32_t,ZScriptArray>& arrs);
	void save_arrays(std::map<int32_t,ZScriptArray>& arrs);

	bool isMemberObjectType(size_t index)
	{
		if (index < owned_vars)
			return var_types.size() > index && var_types[index] != script_object_type::none;

		if (index < data.size())
			return true;
		
		NOTREACHED();
		return false;
	}
};
struct saved_user_object
{
	bool operator==(const saved_user_object&) const = default;

	user_object obj;
	std::map<int32_t,ZScriptArray> held_arrays;
};

#endif
