#include "../precompiled.h"
#include "ZAsmCommands.h"

#include "ZAsmCommandTable.h"

#include <map>
#include <vector>

using namespace std;
using namespace ZAsm;

CommandDef::CommandDef(
		int id, string const& name,
		ArgumentType firstArg, ArgumentType secondArg)
	: id_(id), name_(name), firstArg_(firstArg), secondArg_(secondArg)
{}

int CommandDef::getArgCount() const
{
	return (firstArg_ != ArgNone ? 1 : 0)
		+ (secondArg_ != ArgNone ? 1 : 0);
}

CommandDef const** commandArray()
{
	static CommandDef const* a[] = {
		// eg. &ZAsm::Cmd_SETV,
#		define X(ARG0, ARG1, NAME) &ZAsm::Cmd_##NAME,
		ZASM_COMMAND_TABLE	
#		undef X
	};
	return a;
}

CommandDef const* ZAsm::getCommandDef(int id)
{
	if (id < 0 || id >= ZAsm::CommandCount) return NULL;
	return commandArray()[id];
}

map<string, CommandDef const*>& commandMap()
{
	static map<string, CommandDef const*> m;
	static bool initialized = false;
	if (!initialized)
	{
		// eg. m["SETV"] = &ZAsm::Cmd_SETV;
#		define X(ARG0, ARG1, NAME) \
		m[#NAME] = &ZAsm::Cmd_##NAME ;
		ZASM_COMMAND_TABLE
#		undef X
		initialized = true;
	}
	return m;
}

CommandDef const* ZAsm::getCommandDef(std::string const& name)
{
	map<string, CommandDef const*>& m = commandMap();
	map<string, CommandDef const*>::const_iterator it = m.find(name);
	if (it == m.end()) return NULL;
	return it->second;
}

/*
// Define CommandDef constants. Example:
// CommandDef const ZAsm::Cmd_SETV(
//   ZAsm::CmdId_SETV, "SETV",
//   CommandDef::ArgRegister, CommandDef::ArgValue);
#define X(ARG0, ARG1, NAME) \
	CommandDef const ZAsm::Cmd_ ## NAME ( \
			ZAsm::CmdId_ ## NAME , #NAME, \
			CommandDef:: ## ARG0 , CommandDef:: ARG1 );
ZASM_COMMAND_TABLE
#undef X
*/

#undef ZASM_COMMAND_TABLE

