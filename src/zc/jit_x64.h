#ifndef ZC_JIT_X64_H_
#define ZC_JIT_X64_H_

#include "base/general.h"
#include "base/zdefs.h"
#include "zc/zasm_utils.h"
#include <cstdint>
#include <map>

typedef int32_t (*CompiledFunction)(int32_t* registers, int32_t* global_registers,
								  int32_t* stack, uint32_t* stack_index, uint32_t* pc,
								  uintptr_t* call_stack_rets, uint32_t* call_stack_ret_index,
								  uint32_t* wait_index, uint32_t start_pc);

struct JittedFunctionHandle
{
	CompiledFunction compiled_fn;
	std::map<pc_t, uintptr_t> call_pc_to_return_address;
	std::map<pc_t, uint32_t> wait_frame_pc_to_index;
};

struct JittedScriptHandle
{
	JittedFunctionHandle* fn;
	script_data* script;
	refInfo* ri;
	uintptr_t call_stack_rets[MAX_CALL_FRAMES];
	uint32_t call_stack_ret_index;
	// nth WaitX instruction last execution stopped at. If 0, then the script has not run yet.
	uint32_t wait_index;
};

#endif
