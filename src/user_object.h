#ifndef ZC_USER_OBJECT
#define ZC_USER_OBJECT

#include "allegro/debug.h"
#include "base/check.h"
#include "base/zc_array.h"
#include "base/general.h"
#include "components/zasm/pc.h"
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <vector>

enum class ScriptType;
struct zasm_script;
struct ScriptEngineData;
class sprite;

struct NamedScriptEngineData
{
	std::string name;
	std::unique_ptr<ScriptEngineData> data;
};

extern std::vector<NamedScriptEngineData*> active_object_dtor_script_datas;

// Only script arrays and custom user objects can be restored right now.
// All other object types are set to NULL.
bool can_restore_object_type(script_object_type type);

struct script_object_base
{
	virtual bool operator==(script_object_base const&) const = default;

	int32_t ref_count;
	script_object_type type;
	uint32_t id;
	ScriptType owned_type;
	int32_t owned_i;
	int32_t owned_sprite_id;
	bool global;

	virtual ~script_object_base() = default;

	virtual void get_retained_ids([[maybe_unused]] std::vector<uint32_t>& ids) {};
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

struct ArrayOwner : script_object_base
{
	ArrayOwner();
	bool specOwned;
	bool specCleared;
	void reset();
	void reown(ScriptType ty, int32_t i);
	void reown(sprite* spr);
};

struct script_array : public script_object_base
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
	std::vector<script_object_type> untyped_array_types;
	std::optional<internal_array_id> internal_id;
	bool internal_expired;

#ifdef IS_PLAYER
	void get_retained_ids(std::vector<uint32_t>& ids);
	void restore_references();
	bool holds_untyped_object(int index) const;
	script_object_type get_type_in_untyped_array(int index) const;
	void set_type_in_untyped_array(int index, script_object_type type);
	void insert_type_in_untyped_array(int index);
	void remove_type_in_untyped_array(int index);
	void truncate_types_in_untyped_array(size_t size);
#endif
};

#define MAX_USER_STACKS 256
#define USERSTACK_MAX_SIZE 2147483647
struct user_stack : public script_object_base
{
	std::deque<int32_t> theStack;
	// Aligned with theStack: which entries hold script objects, and their type.
	// Reference counts are managed by the opcode handlers (see STACKPUSHBACK etc).
	std::deque<script_object_type> object_types;

	void get_retained_ids(std::vector<uint32_t>& ids)
	{
		for (size_t i = 0; i < theStack.size(); i++)
			if (object_types[i] != script_object_type::none)
				ids.push_back(theStack[i]);
	}

#ifdef IS_PLAYER
	void restore_references();
#endif

	int32_t size()
	{
		return theStack.size();
	}
	bool full()
	{
		return theStack.size() == USERSTACK_MAX_SIZE;
	}
	bool holds_object(int32_t index)
	{
		return unsigned(index) < object_types.size() && object_types[index] != script_object_type::none;
	}
	int32_t get(int32_t index)
	{
		if(unsigned(index) >= theStack.size()) return 0;
		return theStack[index];
	}
	bool set(int32_t index, int32_t val, script_object_type type)
	{
		if(unsigned(index) >= theStack.size()) return false;
		theStack[index] = val;
		object_types[index] = type;
		return true;
	}
	bool push_back(int32_t val, script_object_type type)
	{
		if(full()) return false;
		theStack.push_back(val);
		object_types.push_back(type);
		return true;
	}
	bool push_front(int32_t val, script_object_type type)
	{
		if(full()) return false;
		theStack.push_front(val);
		object_types.push_front(type);
		return true;
	}
	int32_t pop_back()
	{
		if(theStack.empty()) return 0;
		int32_t val = theStack.back();
		theStack.pop_back();
		object_types.pop_back();
		return val;
	}
	int32_t pop_front()
	{
		if(theStack.empty()) return 0;
		int32_t val = theStack.front();
		theStack.pop_front();
		object_types.pop_front();
		return val;
	}
	int32_t peek_back()
	{
		if(theStack.empty()) return 0;
		return theStack.back();
	}
	int32_t peek_front()
	{
		if(theStack.empty()) return 0;
		return theStack.front();
	}
	void clearStack()
	{
		theStack.clear();
		theStack.shrink_to_fit();
		object_types.clear();
		object_types.shrink_to_fit();
	}
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
struct user_object : public script_object_base
{
	std::vector<int32_t> data;
	size_t owned_vars;
	std::vector<script_object_type> var_types;
	scr_func_exec destruct;
	zasm_script* script;
	pc_t ctor_pc;

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
