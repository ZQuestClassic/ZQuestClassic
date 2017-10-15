#include "../precompiled.h"
#include "ZAsmCommands.h"
#include "ZAsmCommandTable.h"

#include <map>

using namespace std;
using namespace ZAsm;

CommandDef::CommandDef(
		CommandId id, string const& name,
		ArgumentType firstArg, ArgumentType secondArg)
	: id_(id), name_(name), firstArg_(firstArg), secondArg_(secondArg)
{}

int CommandDef::getArgCount() const
{
	return (firstArg_ != ArgNone ? 1 : 0)
		+ (secondArg_ != ArgNone ? 1 : 0);
}

map<CommandId, CommandDef*>& commandsById()
{
	static map<CommandId, CommandDef*> m;
	static bool initialized = false;
	if (!initialized)
	{
		// eg. m[CommandId(0)] = &ZAsm::Cmd_SETV;
#		define COMMAND(ARG0, ARG1, ID, NAME) \
		m[CommandId(ID)] = &ZAsm::Cmd_##NAME;
		ZASM_COMMAND_TABLE
#		undef COMMAND
		initialized = true;
	}
	return m;
}

CommandDef* ZAsm::getCommandDef(CommandId id)
{
	if (id < 0 || id >= ZASM_COMMAND_COUNT) return NULL;
	return commandsById()[id];
}

map<string, CommandDef*>& commandsByName()
{
	static map<string, CommandDef*> m;
	static bool initialized = false;
	if (!initialized)
	{
		// eg. m["SETV"] = &ZAsm::Cmd_SETV;
#		define COMMAND(ARG0, ARG1, ID, NAME) \
		m[#NAME] = &ZAsm::Cmd_##NAME;
		ZASM_COMMAND_TABLE
#		undef COMMAND
		initialized = true;
	}
	return m;
}

CommandDef* ZAsm::getCommandDef(std::string const& name)
{
	map<string, CommandDef*>& m = commandsByName();
	map<string, CommandDef*>::const_iterator it = m.find(name);
	if (it == m.end()) return NULL;
	return it->second;
}

// Declare CommandDef constants. Example:
// CommandDef ZAsm::Cmd_SETV(
//     CommandId(0), "SETV",
//     CommandDef::ArgRegister, CommandDef::ArgValue);
#	define COMMAND(ARG0, ARG1, ID, NAME) \
	CommandDef ZAsm::Cmd_##NAME( \
			CommandId(ID), #NAME, CommandDef::ARG0, CommandDef::ARG1);
	ZASM_COMMAND_TABLE
#	undef COMMAND

#undef ZASM_COMMAND_TABLE
#undef ZASM_COMMAND_COUNT
