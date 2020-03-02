#ifndef ZASM_EXPORT_H
#define ZASM_EXPORT_H

#include "parser/Compiler.h"
#include "zdefs.h"

namespace ZScript
{
	class Opcode;
}

std::string varToString(long arg);
std::string getOpcodeString(ffscript const& line);
std::pair<zasm_meta, std::vector<ZScript::Opcode*>> disassemble_script(script_data const* script);

#endif

