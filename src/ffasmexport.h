#ifndef ZASM_EXPORT_H
#define ZASM_EXPORT_H

#include "parser/Compiler.h"
#include "zdefs.h"

namespace ZScript
{
	class Opcode;
	struct disassembled_script_data;
}
using ZScript::disassembled_script_data;

std::string varToString(long arg);
std::string getOpcodeString(ffscript const& line);
disassembled_script_data disassemble_script(script_data const* script);

#endif

