#ifndef ZC_JIT_NONE_H_
#define ZC_JIT_NONE_H_

#include <cstdint>

struct JittedScript {};
struct JittedScriptInstance
{
	bool should_wait;
	// If true, run_script_int is being called to execute exactly [uncompiled_command_count] commands.
	bool sequence_mode;
	int32_t uncompiled_command_count;
};

#endif
