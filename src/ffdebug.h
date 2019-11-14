//With _DEBUGPRINTSCOMMAND from zdefs.h enabled, ffdebug.h will print out each ASM command
//processed with the value stored in each register supplied to it before the command acts on those registers
//~Joe123

#include "ffscript.h"
#include "zdefs.h"

#ifndef _FFDEBUG_H
#define _FFDEBUG_H

namespace ffdebug
{
	void print_disassembly(const word scommand);
}

#endif

