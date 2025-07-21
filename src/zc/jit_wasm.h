#ifndef ZC_JIT_WASM_H_
#define ZC_JIT_WASM_H_

#include "base/zdefs.h"
#include "zc/zasm_utils.h"
#include <cstdint>

struct JittedFunctionHandle
{
	int module_id;
};

struct JittedScriptHandle
{
	JittedFunctionHandle* fn;
	script_data* script;
	refInfo* ri;
	uint32_t handle_id;
	pc_t call_stack_rets[100];
	pc_t call_stack_ret_index;

	~JittedScriptHandle();
};

#endif
