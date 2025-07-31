#ifndef ZC_JIT_WASM_H_
#define ZC_JIT_WASM_H_

#include "base/zdefs.h"
#include "zc/zasm_utils.h"
#include <cstdint>

struct JittedFunctionHandle
{
	int module_id;
	std::map<pc_t, uint32_t> pc_to_block_id;
};

struct JittedScriptHandle
{
	JittedFunctionHandle* fn;
	script_data* script;
	refInfo* ri;
	uint32_t handle_id;
	pc_t call_stack_rets[MAX_CALL_FRAMES];
	pc_t call_stack_ret_index;
	// nth WaitX instruction last execution stopped at. If 0, then the script has not run yet.
	uint32_t wait_index;

	~JittedScriptHandle();
};

#endif
