#ifndef SCRIPTING_ZASM_COMMANDS_H
#define SCRIPTING_ZASM_COMMANDS_H
#include "ZAsmCommandTable.h"

#include <string>

namespace ZAsm
{
	enum CommandId
	{
		// Create command id enum, eg. CmdId_SETV = 0,
#		define COMMAND(ARG0, ARG1, ID, NAME) CmdId_##NAME = ID,
		ZASM_COMMAND_TABLE
#		undef COMMAND
	};

	class CommandDef
	{
	public:
		enum ArgumentType {ArgNone, ArgRegister, ArgValue, ArgLabel};

		CommandDef(CommandId id, std::string const& name,
		           ArgumentType firstArg = ArgNone,
		           ArgumentType secondArg = ArgNone);
		
		CommandId getId() const {return id_;}
		std::string const& getName() const {return name_;}
		bool hasFirstArg() const {return firstArg_ != ArgNone;}
		ArgumentType getFirstArg() const {return firstArg_;}
		bool hasSecondArg() const {return secondArg_ != ArgNone;}
		ArgumentType getSecondArg() const {return secondArg_;}
		int getArgCount() const;

	private:
		CommandId id_;
		std::string name_;
		ArgumentType firstArg_;
		ArgumentType secondArg_;
	};

	// Lookup command definition.
	CommandDef* getCommandDef(CommandId id);
	CommandDef* getCommandDef(std::string const& name);
	
	// Declare CommandDef constants. Example:
	// extern CommandDef Cmd_SETV;
#	define COMMAND(ARG0, ARG1, ID, NAME) \
	extern CommandDef Cmd_##NAME;
	ZASM_COMMAND_TABLE
#	undef COMMAND
}

#undef ZASM_COMMAND_TABLE
#undef ZASM_COMMAND_COUNT
#endif
