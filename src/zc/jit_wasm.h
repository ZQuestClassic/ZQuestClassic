#ifndef ZC_JIT_WASM_H_
#define ZC_JIT_WASM_H_

#include "core/zdefs.h"
#include "components/zasm/pc.h"
#include <cstdint>

struct JittedScript
{
	int module_id;
};

struct JittedScriptInstance
{
	JittedScript* j_script;
	script_data* script;
	refInfo* ri;
	int handle_id;
	pc_t call_stack_rets[MAX_CALL_FRAMES];
	pc_t call_stack_ret_index;
	// nth WaitX instruction last execution stopped at. If 0, then the script has not run yet.
	uint32_t wait_index;
	bool should_wait;
	// If true, run_script_int is being called to execute exactly [uncompiled_command_count] commands.
	bool sequence_mode;
	int32_t uncompiled_command_count;
	// When the JIT yields to run a frozen generic script (RUNGENFRZSCR), this holds
	// the register the result of runGenericFrozenEngine should be written to before
	// resuming. -1 means no frozen script is pending (a normal return/wait).
	int32_t frozen_dest_reg = -1;

	~JittedScriptInstance();
};

#endif
