#include "base/zdefs.h"
#include "user_object.h"
#include "zc/ffscript.h"

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

void destroy_object_arr(int32_t ptr);
script_data* load_scrdata(ScriptType type, word script, int32_t i);


void user_abstract_obj::own(ScriptType type, int32_t i)
{
	owned_type = type;
	owned_i = i;
}
bool user_abstract_obj::own_clear(ScriptType type, int32_t i)
{
	if(owned_type == type && owned_i == i)
	{
		free_obj();
		return true;
	}
	return false;
}
bool user_abstract_obj::own_clear_any()
{
	if(owned_type != ScriptType::None || owned_i != 0)
	{
		free_obj();
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
			free_obj();
			return true;
		}
		else if(owned_i > 0 && owned_i < NUMSCRIPTSGENERIC)
		{
			static const word mask = (1<<GENSCR_ST_RELOAD)|(1<<GENSCR_ST_CONTINUE);
			auto& genscr = user_genscript::get(owned_i);
			if((genscr.exitState|genscr.reloadState) & mask)
			{
				free_obj();
				return true;
			}
		}
	}
	return false;
}

ArrayOwner::ArrayOwner() : user_abstract_obj(),
	specOwned(false), specCleared(false)
{}

void ArrayOwner::clear()
{
	user_abstract_obj::clear();
	specOwned = false;
	specCleared = false;
}

void ArrayOwner::reown(ScriptType ty, int32_t i)
{
	clear();
	own(ty,i);
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
	if(validate())
	{
		// Either it did, or it was auto-correctable
		push_ri(); //Store the prior script state
		// Setup the refInfo/stack/global vars for the destructor script state
		refInfo newRI = refInfo();
		ri = &newRI;
		ri->pc = pc;
		ri->thiskey = thiskey;
		ri->sp--;
		
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
		script_funcrun = true;
		run_script_int(false);
		script_funcrun = false; //If this ever is able to be set elsewhere, this should restore old value...
		destructstr = oldstr;
		//
		pop_ri(); //restore the prior script state
	}
}
bool scr_func_exec::validate()
{
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid()) // Destructor is null or points to bad script
		return false;
	
	ffscript &zas = sc_data->zasm[pc-1];
	if(zas.command == STARTDESTRUCTOR && zas.strptr && name == *(zas.strptr))
		return true; //validated! Destructor already points to correct line
	dword q = 0;
	while(true)
	{
		ffscript& zas = sc_data->zasm[q];
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

//Prepare the object's destructor
void user_object::prep(dword pc, ScriptType type, word script, int32_t i)
{
	if(!pc) return; //destructor is null
	ffscript &zas = curscript->zasm[pc-1];
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
void user_object::load_arrays(std::map<int32_t,ZScriptArray>& arrs)
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

void user_object::clear_nodestruct()
{
	disown();
	if(data.size() > owned_vars) //owns arrays!
	{
		for(auto ind = owned_vars; ind < data.size(); ++ind)
		{
			auto arrptr = data.at(ind)/10000;
			destroy_object_arr(arrptr);
		}
	}
	data.clear();
	reserved = false;
	owned_vars = 0;
}
void user_object::clear()
{
	destruct.execute();
	clear_nodestruct();
}

