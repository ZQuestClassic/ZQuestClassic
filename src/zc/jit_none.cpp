#include "zc/jit.h"

JittedScript* jit_compile_script(zasm_script* script)
{
	return nullptr;
}

int jit_run_script(JittedScriptInstance* j_instance)
{
	return RUNSCRIPT_ERROR;
}

JittedScriptInstance* jit_create_script_impl(script_data *script, refInfo* ri, JittedScript* j_script, bool just_initialized)
{
	return nullptr;
}

void jit_profiler_increment_function_back_edge(JittedScriptInstance* j_instance, pc_t pc)
{
}

void jit_release(JittedScript* j_script)
{
}
