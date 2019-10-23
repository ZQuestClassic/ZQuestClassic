#ifndef ZASM_EXPORT_H
#define ZASM_EXPORT_H

#include "parser/Compiler.h"
#include "zdefs.h"

std::string varToString(long arg);
std::string getOpcodeString(ffscript const& line);
std::vector<ZScript::Opcode*> disassemble_script(ffscript const* script);

#endif

