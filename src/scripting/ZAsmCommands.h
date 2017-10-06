#ifndef SCRIPTING_ZASM_COMMANDS_H
#define SCRIPTING_ZASM_COMMANDS_H

#include "ZAsmCommandTable.h"

#include <string>

namespace ZAsm
{
	class CommandDef
	{
	public:
		enum ArgumentType {ArgNone, ArgRegister, ArgValue, ArgLabel};

		CommandDef(int id, std::string const& name,
		           ArgumentType firstArg = ArgNone,
		           ArgumentType secondArg = ArgNone);
		
		int getId() const {return id_;}
		std::string getName() const {return name_;}
		bool hasFirstArg() const {return firstArg_ != ArgNone;}
		ArgumentType getFirstArg() const {return firstArg_;}
		bool hasSecondArg() const {return secondArg_ != ArgNone;}
		ArgumentType getSecondArg() const {return secondArg_;}
		int getArgCount() const;

	private:
		int id_;
		std::string name_;
		ArgumentType firstArg_;
		ArgumentType secondArg_;
	};

	// Lookup command definition.
	CommandDef const* getCommandDef(int id);
	CommandDef const* getCommandDef(std::string const& name);
	
	// Create command id enum, eg. CmdId_SETV
#	define X(ARG0, ARG1, NAME) CmdId_##NAME,
	enum CommandId
	{
		ZASM_COMMAND_TABLE
		CommandCount
	};
#	undef X

	// Declare CommandDef constants. Example:
	// static CommandDef const Cmd_SETV;
#	define X(ARG0, ARG1, NAME) \
	CommandDef const Cmd_##NAME( \
			CmdId_##NAME, #NAME, \
			CommandDef::ARG0, CommandDef::ARG1);
	ZASM_COMMAND_TABLE
#	undef X


}

#undef ZASM_COMMAND_TABLE
#endif
