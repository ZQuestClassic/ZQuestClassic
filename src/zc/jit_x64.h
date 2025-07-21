#ifndef ZC_JIT_X64_H_
#define ZC_JIT_X64_H_

#include "base/zdefs.h"
#include <cstdint>

typedef int32_t (*CompiledFunction)(int32_t* registers, int32_t* global_registers,
								  int32_t* stack, uint32_t* stack_index, uint32_t* pc,
								  intptr_t* call_stack_rets, uint32_t* call_stack_ret_index,
								  uint32_t* wait_index, uint32_t start_pc);

struct JittedFunctionHandle
{
	CompiledFunction compiled_fn;
};

struct JittedScriptHandle
{
	JittedFunctionHandle* fn;
	script_data* script;
	refInfo* ri;
	intptr_t call_stack_rets[100];
	uint32_t call_stack_ret_index;
};

#endif
