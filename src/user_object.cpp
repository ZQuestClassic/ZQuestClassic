#include "base/zdefs.h"
#include "user_object.h"
#include "zc/ffscript.h"

void push_ri();
void pop_ri();
extern refInfo* ri;
extern script_data* curscript;
extern int32_t(*stack)[MAX_SCRIPT_REGISTERS];
extern byte curScriptType;
extern word curScriptNum;
extern int32_t curScriptIndex;
extern bool script_funcrun;
extern std::string* destructstr;

void destroy_object_arr(int32_t ptr);
script_data* load_scrdata(int32_t type, word script, int32_t i);

void scr_func_exec::clear()
{
	pc = type = i = 0;
	script = 0; thiskey = 0;
	name.clear();
}

scr_func_exec& scr_func_exec::operator=(scr_func_exec const& other)
{
	pc = other.pc;
	type = other.type;
	i = other.i;
	script = other.script;
	name = other.name;
	thiskey = other.thiskey;
	return *this;
}

void scr_func_exec::execute()
{
	static int32_t static_stack[MAX_SCRIPT_REGISTERS];
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid())
		return;
	
	ffscript *zas = &sc_data->zasm[pc-1];
	if(!(zas->command == STARTDESTRUCTOR && zas->strptr && name == *(zas->strptr)))
	{
		if(validate())
			zas = &sc_data->zasm[pc-1];
		else return;
	}
	
	if(zas->command == STARTDESTRUCTOR && zas->strptr && name == *(zas->strptr))
	{
		push_ri();
		refInfo newRI;
		ri = &newRI;
		ri->pc = pc;
		ri->thiskey = thiskey;
		ri->sp--;
		
		curscript = sc_data;
		stack = &static_stack;
		curScriptType = type;
		curScriptNum = script;
		curScriptIndex = i;
		memset(static_stack, 0, sizeof(int32_t)*MAX_SCRIPT_REGISTERS);
		//
		std::string* oldstr = destructstr;
		destructstr = &name;
		script_funcrun = true;
		run_script_int(false);
		script_funcrun = false;
		destructstr = oldstr;
		//
		pop_ri();
	}
}
bool scr_func_exec::validate()
{
	script_data* sc_data = load_scrdata(type,script,i);
	if(!pc || !sc_data || !sc_data->valid())
		return false;
	
	ffscript &zas = sc_data->zasm[pc-1];
	if(zas.command == STARTDESTRUCTOR && zas.strptr && name == *(zas.strptr))
		return true; //validated!
	dword q = 0;
	while(true)
	{
		ffscript& zas = sc_data->zasm[q];
		if(zas.command == 0xFFFF)
		{
			zprint2("Destructor for class '%s' expected, but not found!\n", name.c_str());
			return false;
		}
		else if(zas.command == STARTDESTRUCTOR
			&& zas.strptr && name == *(zas.strptr))
		{
			pc = q+1;
			return true; //validated!
		}
	}
}
void user_object::prep(dword pc, int32_t type, word script, int32_t i)
{
	if(!pc) return;
	ffscript &zas = curscript->zasm[pc-1];
	if(zas.command == STARTDESTRUCTOR && zas.strptr)
	{
		destruct.pc = pc;
		destruct.type = type;
		destruct.script = script;
		destruct.i = i;
		destruct.thiskey = ri->thiskey;
		destruct.name = *zas.strptr;
	}
	else zprint2("Destructor for object not found?\n");
}
void user_object::load_arrays(std::map<int32_t,ZScriptArray>& arrs)
{
	for(auto it = arrs.begin(); it != arrs.end(); ++it)
	{
		auto ind = it->first;
		objectRAM[ind] = it->second;
	}
}
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

void user_object::clear(bool destructor)
{
	if(destructor)
		destruct.execute();
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
	owned_type = -1;
	owned_i = 0;
	owned_vars = 0;
}

user_object& user_object::operator=(user_object const& other)
{
	reserved = other.reserved;
	owned_type = other.owned_type;
	owned_i = other.owned_i;
	data = other.data;
	owned_vars = other.owned_vars;
	destruct = other.destruct;
	return *this;
}

saved_user_object& saved_user_object::operator=(saved_user_object const& other)
{
	obj = other.obj;
	object_index = other.object_index;
	held_arrays = other.held_arrays;
	return *this;
}

