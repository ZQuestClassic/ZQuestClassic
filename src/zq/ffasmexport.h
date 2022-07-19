#ifndef ZASM_EXPORT_H
#define ZASM_EXPORT_H

#include "parser/Compiler.h"
#include "base/zdefs.h"

namespace ZScript
{
	class Opcode;
	struct disassembled_script_data;
}
using ZScript::disassembled_script_data;

std::string varToString(int32_t arg);
std::string getOpcodeString(ffscript const& line);
disassembled_script_data disassemble_script(script_data const* script);
void write_script(FILE* dest, disassembled_script_data const& data);
void write_script(FILE* dest, script_data const* script);


#endif

