#include "zc/jit.h"
#include "components/zasm/pc.h"

void jit_startup_impl()
{
}

JittedScript* jit_compile_script([[maybe_unused]] zasm_script* script)
{
	return nullptr;
}

int jit_run_script([[maybe_unused]] JittedScriptInstance* j_instance)
{
	return RUNSCRIPT_ERROR;
}

bool jit_can_start_script()
{
	return true;
}

JittedScriptInstance* jit_create_script_impl([[maybe_unused]] script_data *script, [[maybe_unused]] refInfo* ri, [[maybe_unused]] JittedScript* j_script)
{
	return nullptr;
}

void jit_profiler_increment_function_back_edge([[maybe_unused]] JittedScriptInstance* j_instance, [[maybe_unused]] pc_t pc)
{
}

void jit_release([[maybe_unused]] JittedScript* j_script)
{
}
