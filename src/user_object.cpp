#include "base/general.h"
#include "core/zdefs.h"
#include "user_object.h"
#include "components/zasm/table.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/script_object.h"
#include "zc/zscriptversion.h"
#include <algorithm>
#include <memory>

void push_ri();
void pop_ri();
extern refInfo* ri;
extern script_data* curscript;
extern int32_t(*stack)[MAX_STACK_SIZE];
extern int32_t(*ret_stack)[MAX_CALL_FRAMES];
extern ScriptType curScriptType;
extern word curScriptNum;
extern int32_t curScriptIndex;
extern bool script_funcrun;
extern std::string* destructstr;

script_data* load_scrdata(ScriptType type, word script, int32_t i);

bool can_restore_object_type(script_object_type type)
{
	return type == script_object_type::object || type == script_object_type::array || type == script_object_type::stack;
}

// About object ownership.
//
// Two things can "own" an object:
//
// 1) a script
// 2) a sprite
//
// Only one thing may own an object. Updating the ownership of an object automatically removes any
// previous ownership.
//
// Prior to 3.0 (ZScriptVersion::gc()), when a script or sprite object is removed, all objects
// it owns are deleted. Starting in 3.0, instead just a reference is deleted (and the object is
// only deleted if there are no more references to it).
//
// Only scripts can assign object ownership.
//
// Ownership can be assigned to a script in a few ways. For all the following cases, the currently
// running script is what will "own" the object.
//
// - the global OwnObject / OwnArray functions 
// - various `Own()` functions on script objects that are not sprites (ex: bitmap, paldata)
// - (prior to 3.0) when an class instance is constructed
//
// Ownership can be assigned to a sprite in just one way:
//
// - various `Own(object)` functions on scripts objects that are sprites (ex: lweapon::Own(file b)`
//
// When a script or sprite ends, the functions "destroySprite" / "deallocateAllScriptOwned" etc.
// handle removing the reference.

// For the script types whose RunScript "index" identifies a dmap/subscreen context rather than a
// distinct script-engine slot (DMap, OnMap, passive/active subscreen, engine subscreen), there is
// a single shared slot keyed at index 0. Script ownership must normalize the index the same way
// `get_script_engine_data` does, or owned objects end up keyed by the dmap index while cleanup
// looks at index 0 -- leaking them. Keep this list in sync with `get_script_engine_data`.
static int32_t normalize_script_owner_index(ScriptType type, int32_t i)
{
	switch (type)
	{
	case ScriptType::DMap:
	case ScriptType::OnMap:
	case ScriptType::ScriptedPassiveSubscreen:
	case ScriptType::ScriptedActiveSubscreen:
	case ScriptType::EngineSubscreen:
		return 0;
	default:
		return i;
	}
}

void script_object_base::set_owned_by_script(ScriptType type, int32_t i)
{
	owned_type = type;
	owned_i = normalize_script_owner_index(type, i);
	owned_sprite_id = 0;
}
void script_object_base::set_owned_by_sprite(sprite* sprite)
{
	owned_sprite_id = sprite->getUID();
	owned_type = ScriptType::None;
	owned_i = 0;
}
bool script_object_base::script_own_clear(ScriptType type, int32_t i)
{
	if(owned_type == type && owned_i == normalize_script_owner_index(type, i))
	{
		return true;
	}
	return false;
}
bool script_object_base::script_own_clear_any()
{
	if(owned_type != ScriptType::None || owned_i != 0)
	{
		return true;
	}
	return false;
}
bool script_object_base::script_own_clear_cont()
{
	if(owned_type != ScriptType::None || owned_i != 0)
	{
		if(owned_type != ScriptType::Generic)
		{
			return true;
		}
		else if(owned_i > 0 && owned_i < NUMSCRIPTSGENERIC)
		{
			static const word mask = (1<<GENSCR_ST_RELOAD)|(1<<GENSCR_ST_CONTINUE);
			auto& genscr = user_genscript::get(owned_i);
			if((genscr.exitState|genscr.reloadState) & mask)
			{
				return true;
			}
		}
	}
	return false;
}
bool script_object_base::sprite_own_clear(int32_t id)
{
	if (id && id == owned_sprite_id)
	{
		return true;
	}
	return false;
}

ArrayOwner::ArrayOwner() : script_object_base(),
	specOwned(false), specCleared(false)
{}

void ArrayOwner::reset()
{
	specOwned = false;
	specCleared = false;
}

void ArrayOwner::reown(ScriptType ty, int32_t i)
{
	reset();
	set_owned_by_script(ty,i);
}

void ArrayOwner::reown(sprite* spr)
{
	reset();
	set_owned_by_sprite(spr);
}

bool script_array::internal_array_id::matches(ScriptType script_type, int32_t uid) const
{
	return ref == uid && matches(script_type);
}

bool script_array::internal_array_id::matches(ScriptType script_type) const
{
	int zasm_var_ref = get_register_ref_dependency(zasm_var).value_or(0);

	// Handle sprite base class.
	if (zasm_var_ref == REFSPRITE)
		return script_type == ScriptType::Ewpn || script_type == ScriptType::Lwpn || script_type == ScriptType::FFC || script_type == ScriptType::ItemSprite || script_type == ScriptType::NPC;

	switch (script_type)
	{
		case ScriptType::Ewpn: return zasm_var_ref == REFEWPN;
		case ScriptType::FFC: return zasm_var_ref == REFFFC;
		case ScriptType::Generic: return zasm_var_ref == REFGENERICDATA;
		case ScriptType::GenericFrozen: return zasm_var_ref == REFGENERICDATA;
		case ScriptType::ItemSprite: return zasm_var_ref == REFITEM;
		case ScriptType::Lwpn: return zasm_var_ref == REFLWPN;
		case ScriptType::NPC: return zasm_var_ref == REFNPC;
		case ScriptType::Screen: return zasm_var_ref == REFSCREEN;
	}

	return false;
}

void script_array::get_retained_ids(std::vector<uint32_t>& ids)
{
	if (arr.HoldsObjects())
	{
		for (int i = 0; i < arr.Size(); i++)
			ids.push_back(arr[i]);
	}
	else if (arr.MaybeHoldsObjects())
	{
		if (internal_id)
		{
			if (internal_expired) return;

			std::vector<int> values = zasm_array_get_all(internal_id->zasm_var, internal_id->ref);
			int size = std::min(values.size(), untyped_array_types.size());
			for (int i = 0; i < size; i++)
			{
				if (untyped_array_types[i] != script_object_type::none)
					ids.push_back(values[i]);
			}

			return;
		}

		int size = std::min(arr.Size(), untyped_array_types.size());
		for (int i = 0; i < size; i++)
		{
			if (untyped_array_types[i] != script_object_type::none)
				ids.push_back(arr[i]);
		}
	}
}

void script_array::restore_references()
{
	if (!arr.HoldsObjects() && !arr.MaybeHoldsObjects())
		return;

	if (internal_expired)
		return;

	if (arr.HoldsObjects())
	{
		if (!can_restore_object_type(arr.ObjectType()))
		{
			for (int i = 0; i < arr.Size(); i++)
				arr[i] = 0;
			return;
		}

		for (int i = 0; i < arr.Size(); i++)
		{
			if (script_objects.contains(arr[i]))
				script_object_ref_inc(arr[i]);
			else
				arr[i] = 0;
		}
	}
	else if (arr.MaybeHoldsObjects())
	{
		for (int i = 0; i < arr.Size(); i++)
		{
			if (!holds_untyped_object(i))
				continue;

			if (can_restore_object_type(get_type_in_untyped_array(i)) && script_objects.contains(arr[i]))
				script_object_ref_inc(arr[i]);
			else
				arr[i] = 0;
		}
	}
}

#ifdef IS_PLAYER
void user_stack::restore_references()
{
	for (size_t i = 0; i < theStack.size(); i++)
	{
		if (object_types[i] == script_object_type::none)
			continue;

		if (can_restore_object_type(object_types[i]) && script_objects.contains(theStack[i]))
			script_object_ref_inc(theStack[i]);
		else
		{
			theStack[i] = 0;
			object_types[i] = script_object_type::none;
		}
	}
}
#endif

bool script_array::holds_untyped_object(int index) const
{
	if (index >= untyped_array_types.size())
		return false;

	return untyped_array_types[index] != script_object_type::none;
}

script_object_type script_array::get_type_in_untyped_array(int index) const
{
	if (index >= untyped_array_types.size())
		return script_object_type::none;

	return untyped_array_types[index];
}

void script_array::set_type_in_untyped_array(int index, script_object_type type)
{
	if (index >= untyped_array_types.size())
		untyped_array_types.resize(index + 1);
	untyped_array_types[index] = type;
}

// Keeps the positional type bookkeeping aligned when an element is inserted.
void script_array::insert_type_in_untyped_array(int index)
{
	if (index >= 0 && index < untyped_array_types.size())
		untyped_array_types.insert(untyped_array_types.begin() + index, script_object_type::none);
}

// Keeps the positional type bookkeeping aligned when an element is removed.
void script_array::remove_type_in_untyped_array(int index)
{
	if (index >= 0 && index < untyped_array_types.size())
		untyped_array_types.erase(untyped_array_types.begin() + index);
}

// Drops the positional type bookkeeping beyond the array's new size, so that
// growing the array later doesn't treat fresh elements as stale objects.
void script_array::truncate_types_in_untyped_array(size_t size)
{
	if (untyped_array_types.size() > size)
		untyped_array_types.resize(size);
}

void scr_func_exec::clear()
{
	pc = i = 0;
	type = ScriptType::None;
	script = 0; thiskey = 0;
	name.clear();
}

std::vector<NamedScriptEngineData*> active_object_dtor_script_datas;

void scr_func_exec::execute()
{
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid())
		return;

	// Check that the line number points to the correct destructor
	if(validate(sc_data->zasm_script.get()))
	{
		// Either it did, or it was auto-correctable
		push_ri(); //Store the prior script state
		// Setup the refInfo/stack/global vars for the destructor script state
		NamedScriptEngineData script_data{};
		script_data.name = "~" + name;
		script_data.data = std::make_unique<ScriptEngineData>();
		refInfo& newRI = script_data.data->ref;
		ri = &newRI;
		ri->pc = pc;
		ri->thiskey = thiskey;
		ri->debugger_stack_frames.push_back({
			.stack_frame_base = (uint16_t)(ri->sp),
			.this_ptr = ri->thiskey,
		});

		curscript = sc_data;
		stack = &script_data.data->stack;
		ret_stack = &script_data.data->ret_stack;
		curScriptType = type;
		curScriptNum = script;
		curScriptIndex = i;
		// Run  the destructor script
		std::string* oldstr = destructstr;
		destructstr = &name;
		bool old_funcrun = script_funcrun;
		script_funcrun = true;

		active_object_dtor_script_datas.push_back(&script_data);
		run_script_int(); // TODO: doesn't use JIT...
		active_object_dtor_script_datas.pop_back();

		script_funcrun = old_funcrun;
		destructstr = oldstr;
		//
		pop_ri(); //restore the prior script state
	}
}
bool scr_func_exec::validate(const zasm_script* zasm_script)
{
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid()) // Destructor is null or points to bad script
		return false;
	
	const ffscript &zas = zasm_script->zasm[pc-1];
	if(zas.command == STARTDESTRUCTOR && zas.strptr && name == *(zas.strptr))
		return true; //validated! Destructor already points to correct line
	dword q = 0;
	while(true)
	{
		ffscript& zas = sc_data->zasm_script->zasm[q];
		if(zas.command == 0xFFFF) //Ran out of script to check
		{
			zprint2("Destructor for class '%s' expected, but not found!\n", name.c_str());
			return false;
		}
		else if(zas.command == STARTDESTRUCTOR && zas.strptr && name == *(zas.strptr))
		{ //Found the correct destructor
			pc = q+1;
			return true; //validated!
		}
	}
}

#ifdef IS_PLAYER
void user_object::get_retained_ids(std::vector<uint32_t>& ids)
{
	for (int i = 0; i < owned_vars; i++)
	{
		if (isMemberObjectType(i))
			ids.push_back(data[i]);
	}

	if (ZScriptVersion::gc_arrays())
	{
		for (int i = owned_vars; i < data.size(); i++)
			ids.push_back(data[i]);
	}
	else
	{
		for (int i = owned_vars; i < data.size(); i++)
		{
			auto ptr = data[i]/10000;
			if (ptr == 0)
				continue;

			auto& aptr = objectRAM.at(-ptr);
			if (!aptr.HoldsObjects())
				continue;

			for (int i = 0; i < aptr.Size(); i++)
				ids.push_back(aptr[i]);
		}
	}
}
#endif

void user_object::restore_references()
{
	for (int i = 0; i < owned_vars; i++)
	{
		if (!isMemberObjectType(i))
			continue;

		if (can_restore_object_type(var_types[i]) && script_objects.contains(data[i]))
			script_object_ref_inc(data[i]);
		else
			data[i] = 0;
	}

	if (ZScriptVersion::gc_arrays())
	{
		for (int i = owned_vars; i < data.size(); i++)
		{
			if (script_objects.contains(data[i]))
				script_object_ref_inc(data[i]);
		}
	}
}

//Prepare the object's destructor
void user_object::prep(dword pc, ScriptType type, word script, int32_t i)
{
	if(!pc) return; //destructor is null
	ffscript &zas = curscript->zasm_script->zasm[pc-1];
	if(zas.command == STARTDESTRUCTOR && zas.strptr) //Destructor is valid
	{
		destruct.pc = pc;
		destruct.type = type;
		destruct.script = script;
		destruct.i = i;
		destruct.thiskey = ri->thiskey;
		destruct.name = *zas.strptr;
	}
	else zprint2("Destructor for object not found?\n"); //Should never occur
}
//Load object arrays (from save file)
void user_object::load_arrays(const std::map<int32_t,ZScriptArray>& arrs)
{
	for(auto it = arrs.begin(); it != arrs.end(); ++it)
	{
		auto ind = it->first;
		objectRAM[ind] = it->second;
	}
}
//Save object arrays (to save file)
void user_object::save_arrays(std::map<int32_t,ZScriptArray>& arrs)
{
	if(data.size() > owned_vars) //owns arrays!
	{
		for(auto ind = owned_vars; ind < data.size(); ++ind)
		{
			auto arrptr = (data.at(ind)/-10000);
			if(arrptr > 0)
			{
				arrs[arrptr] = objectRAM.find(arrptr)->second;
			}
		}
	}
}
