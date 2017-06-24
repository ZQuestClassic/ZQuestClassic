#ifndef ZASMDEFS_H
#define ZASMDEFS_H

#include "../zdefs.h"

struct zasm
{
	uint16_t command;
	int32_t arg1;
	int32_t arg2;
};

struct ZAsmScript
{
	ZAsmScript() : version(ZASM_VERSION), type(SCRIPT_NONE)
	{
		name_len = 21;
		name = new char[21];
		strcpy(name, "Uninitialized Script");
		commands_len = 1;
		commands = new zasm[1];
		commands[0].command = 0xFFFF;
	}

	~ZAsmScript()
	{
		delete[] name;
		delete[] commands;
	}

	ZAsmScript &operator=(const ZAsmScript &other)
	{
		version = other.version;
		type = other.type;
		name_len = other.name_len;
		delete[] name;
		name = new char[name_len];
		strcpy(name, other.name);
		commands_len = other.commands_len;
		delete[] commands;
		commands = new zasm[commands_len];
		for (int i = 0; i < commands_len; i++)
			commands[i] = other.commands[i];

		return *this;
	}

	ZAsmScript(const ZAsmScript &other)
	{
		version = other.version;
		type = other.type;
		name_len = other.name_len;
		name = new char[name_len];
		strcpy(name, other.name);
		commands_len = other.commands_len;
		commands = new zasm[commands_len];
		for (int i = 0; i < commands_len; i++)
			commands[i] = other.commands[i];
	}

	// Version of ZASM this script was compiled for
	int16_t version;

	// Type of the script (SCRIPT_GLOBAL, e.g.)
	int16_t type;

	// Name of the script, if the script was compiled from ZScript
	// For debugging and logging errors, etc.
	int16_t name_len;
	char *name;

	// The ZASM itself
	int32_t commands_len;
	zasm *commands;

};


struct GameScripts
{
        GameScripts()
        {
                // A fixed number of these (for now?)
                globalscripts.resize(NUMSCRIPTGLOBAL);
        }

        std::vector<ZAsmScript> globalscripts;
        std::vector<ZAsmScript> ffscripts;
        std::vector<ZAsmScript> itemscripts;
        std::vector<ZAsmScript> guyscripts;
        std::vector<ZAsmScript> wpnscripts;
        std::vector<ZAsmScript> linkscripts;
        std::vector<ZAsmScript> screenscripts;
};

#endif