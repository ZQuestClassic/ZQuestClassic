#ifndef ZC_JIT_WASM_H_
#define ZC_JIT_WASM_H_

#include "base/zdefs.h"
#include "zc/zasm_utils.h"
#include <cstdint>

struct JittedScript
{
	int module_id;
	std::map<pc_t, uint32_t> pc_to_block_id;
};

struct JittedScriptInstance
{
	JittedScript* j_script;
	script_data* script;
	refInfo* ri;
	uint32_t handle_id;
	pc_t call_stack_rets[MAX_CALL_FRAMES];
	pc_t call_stack_ret_index;
	// nth WaitX instruction last execution stopped at. If 0, then the script has not run yet.
	uint32_t wait_index;
	bool should_wait;
	// If true, run_script_int is being called to execute exactly [uncompiled_command_count] commands.
	bool sequence_mode;
	int32_t uncompiled_command_count;

	~JittedScriptInstance();
};

#endif
