#include "base/zdefs.h"
#include "user_object.h"
#include "zasm/table.h"
#include "zc/ffscript.h"
#include "zc/scripting/script_object.h"
#include "zscriptversion.h"

void push_ri();
void pop_ri();
extern refInfo* ri;
extern script_data* curscript;
extern int32_t(*stack)[MAX_SCRIPT_REGISTERS];
extern bounded_vec<word, int32_t>* ret_stack;
extern ScriptType curScriptType;
extern word curScriptNum;
extern int32_t curScriptIndex;
extern bool script_funcrun;
extern std::string* destructstr;

script_data* load_scrdata(ScriptType type, word script, int32_t i);

bool can_restore_object_type(script_object_type type)
{
	return type == script_object_type::object || type == script_object_type::array;
}

void user_abstract_obj::own(ScriptType type, int32_t i)
{
	owned_type = type;
	owned_i = i;
}
bool user_abstract_obj::own_clear(ScriptType type, int32_t i)
{
	if(owned_type == type && owned_i == i)
	{
		return true;
	}
	return false;
}
bool user_abstract_obj::own_clear_any()
{
	if(owned_type != ScriptType::None || owned_i != 0)
	{
		return true;
	}
	return false;
}
bool user_abstract_obj::own_clear_cont()
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

ArrayOwner::ArrayOwner() : user_abstract_obj(),
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
	own(ty,i);
}

bool script_array::internal_array_id::matches(ScriptType script_type, int32_t uid) const
{
	return ref == uid && matches(script_type);
}

bool script_array::internal_array_id::matches(ScriptType script_type) const
{
	int zasm_var_ref = get_register_ref_dependency(zasm_var).value_or(0);
	switch (script_type)
	{
		case ScriptType::Ewpn: return zasm_var_ref == REFEWPN;
		case ScriptType::FFC: return zasm_var_ref == REFFFC;
		case ScriptType::Generic: return zasm_var_ref == REFGENERICDATA;
		case ScriptType::GenericFrozen: return zasm_var_ref == REFGENERICDATA;
		case ScriptType::ItemSprite: return zasm_var_ref == REFITEM;
		case ScriptType::Lwpn: return zasm_var_ref == REFLWPN;
		case ScriptType::NPC: return zasm_var_ref == REFNPC;
		case ScriptType::Screen: return zasm_var_ref == REFSCREENDATA;
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
}

void script_array::restore_references()
{
	if (!arr.HoldsObjects())
		return;

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

void scr_func_exec::clear()
{
	pc = i = 0;
	type = ScriptType::None;
	script = 0; thiskey = 0;
	name.clear();
}

void scr_func_exec::execute()
{
	static int32_t static_stack[MAX_SCRIPT_REGISTERS];
	static bounded_vec<word, int32_t> static_ret_stack;
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid())
		return;
	
	// Check that the line number points to the correct destructor
	if(validate(sc_data->zasm_script.get()))
	{
		// Either it did, or it was auto-correctable
		push_ri(); //Store the prior script state
		// Setup the refInfo/stack/global vars for the destructor script state
		refInfo newRI = refInfo();
		ri = &newRI;
		ri->pc = pc;
		ri->thiskey = thiskey;
		
		curscript = sc_data;
		stack = &static_stack;
		ret_stack = &static_ret_stack;
		curScriptType = type;
		curScriptNum = script;
		curScriptIndex = i;
		memset(static_stack, 0, sizeof(int32_t)*MAX_SCRIPT_REGISTERS);
		static_ret_stack.clear();
		// Run  the destructor script
		std::string* oldstr = destructstr;
		destructstr = &name;
		bool old_funcrun = script_funcrun;
		script_funcrun = true;
		
		run_script_int(false);
		
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
