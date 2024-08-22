#include "zc/scripting/types/user_object.h"

#include "base/general.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/scripting/script_object.h"
#include "zscriptversion.h"
#include <optional>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;
extern vector<int32_t> *sargvec;
extern string *sargstr;

static UserDataContainer<user_object, MAX_USER_OBJECTS> user_objects = {script_object_type::object, "object"};

user_object* create_user_object(uint32_t id)
{
	auto& vec = next_script_object_id_freelist;
	vec.erase(std::remove(vec.begin(), vec.end(), id), vec.end());
	auto object = new user_object{};
	register_script_object(object, script_object_type::object, id);
	return object;
}

std::vector<user_object*> get_user_objects()
{
	std::vector<user_object*> result;
	for (auto id : script_object_ids_by_type[user_objects.type])
	{
		result.push_back(&user_objects[id]);
	}
	return result;
}

static int32_t get_object_arr(size_t sz)
{
	if(sz > 214748) return 0;
	int32_t free_ptr = 1;
	auto it = objectRAM.begin();
	if(it != objectRAM.end())
	{
		if(it->first == 1)
		{
			for(free_ptr = 2; ; ++free_ptr)
			{
				if(objectRAM.find(free_ptr) == objectRAM.end())
					break;
			}
		}
	}
	ZScriptArray arr{};
	arr.Resize(sz);
	arr.setValid(true);
	objectRAM[free_ptr] = arr;
	
	return -free_ptr;
}

static void do_constructclass(ScriptType type, word script, int32_t i)
{
	if (!sargvec) return;
	
	size_t num_vars = sargvec->at(0);
	size_t total_vars = num_vars + sargvec->size()-1;
	auto destr_pc = ri->d[rEXP1];

	if (auto obj = user_objects.create())
	{
		// Before gc/reference counting, allocating a custom object automatically assigns
		// ownership to the current script. Not needed anymore, but important to keep
		// doing for compat.
		if (!ZScriptVersion::gc())
			own_script_object(obj, type, i);
		obj->owned_vars = num_vars;
		for(size_t q = 0; q < total_vars; ++q)
		{
			if(q < num_vars)
			{
				obj->data.push_back(0);
			}
			else
			{
				size_t sz = sargvec->at(q-num_vars+1);
				if(auto id = get_object_arr(sz))
					obj->data.push_back(10000*id);
				else obj->data.push_back(0); //nullptr
			}
		}
		set_register(sarg1, obj->id);
		ri->thiskey = obj->id;
		obj->prep(destr_pc,type,script,i);
	}
	else set_register(sarg1, 0);
}

static void do_readclass()
{
	dword id = get_register(sarg1);
	ri->d[rEXP1] = 0;
	int32_t ind = sarg2;
	if (auto obj = user_objects.check(id, nullptr, true))
	{
		if(unsigned(ind) >= obj->data.size())
		{
			Z_scripterrlog("Script tried to read position '%d' out of bounds on a '%d' size object (%d).", ind, obj->data.size(), id);
		}
		else
		{
			ri->d[rEXP1] = obj->data.at(ind);
		}
	}
}

static void do_writeclass()
{
	dword id = get_register(sarg1);
	int32_t ind = sarg2;
	if (auto obj = user_objects.check(id, nullptr, true))
	{
		if(unsigned(ind) >= obj->data.size())
		{
			Z_scripterrlog("Script tried to write position '%d' out of bounds on a '%d' size object (%d).", ind, obj->data.size(), id);
		}
		else
		{
			bool is_object = obj->isMemberObjectType(ind);
			if (is_object)
				script_object_ref_dec(obj->data[ind]);
			obj->data[ind] = ri->d[rEXP1];
			if (is_object)
				script_object_ref_inc(obj->data[ind]);
		}
	}
}

static void do_freeclass()
{
	// Deleting is no longer needed. To keep reference counting simpler, simply do nothing on delete.
	ri->d[rEXP1] = 0;
}

void user_object_init()
{
	for (auto id : script_object_ids_by_type[user_objects.type])
	{
		user_objects[id].clear_nodestruct();
	}
}

std::optional<int32_t> user_object_run_command(word command)
{
	extern int32_t sarg1;
	extern int32_t sarg2;
	extern int32_t sarg3;
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	word script = curScriptNum;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case ZCLASS_CONSTRUCT:
		{
			do_constructclass(type,script,i);
			break;
		}
		case ZCLASS_READ:
		{
			do_readclass();
			break;
		}
		case ZCLASS_WRITE:
		{
			do_writeclass();
			break;
		}
		case ZCLASS_FREE:
		{
			do_freeclass();
			break;
		}
		case ZCLASS_OWN:
		{
			if (auto obj = user_objects.check(get_register(sarg1), nullptr, true))
			{
				obj->setGlobal(false);
				own_script_object(obj, type, i);
			}
			break;
		}
		case STARTDESTRUCTOR:
		{
			zprint2("STARTDESTRUCTOR: %s\n", sargstr->c_str());
			//This opcode's EXISTENCE indicates the first opcode
			//of a user_object destructor function.
			break;
		}
		case ZCLASS_GLOBALIZE:
		{
			if (auto obj = user_objects.check(get_register(sarg1), nullptr, true))
			{
				obj->setGlobal(true);
				own_script_object(obj, ScriptType::None, 0);
			}
			break;
		}
		case ZCLASS_MARK_TYPE:
		{
			auto& vec = *sargvec;
			assert(vec.size() % 2 == 0);

			uint32_t id = ri->thiskey;
			if (auto obj = user_objects.check(id, "ZCLASS_MARK_TYPE"))
			{
				for (size_t i = 0; i < vec.size(); i += 2)
				{
					int index = vec[i];
					assert(vec[i + 1] >= 0 && vec[i + 1] <= (int)script_object_type::last);
					auto type = (script_object_type)vec[i + 1];
					if (index >= obj->owned_vars)
					{
						int ptr = -obj->data[index] / 10000;
						if (ptr)
						{
							ZScriptArray& a = objectRAM.at(ptr);
							a.setObjectType(type);
						}
					}
					else
					{
						if (obj->var_types.size() <= index)
							obj->var_types.resize(index + 1);
						obj->var_types[index] = type;
					}
				}
			}
			break;
		}
		case OBJ_OWN_CLASS:
		{
			int id = get_register(sarg1);
			user_object* obj = user_objects.check(id);
			if(!obj) break;
			ScriptType own_type = (ScriptType)sarg2;
			int32_t own_i = get_own_i(own_type);
			own_script_object(obj, own_type, own_i);
			break;
		}
		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}
