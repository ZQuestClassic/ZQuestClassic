#include "zc/jit.h"

JittedFunctionHandle* jit_compile_script(zasm_script* script)
{
	return nullptr;
}

int jit_run_script(JittedScriptHandle* jitted_script)
{
	return RUNSCRIPT_ERROR;
}

JittedScriptHandle* jit_create_script_handle_impl(script_data *script, refInfo* ri, JittedFunctionHandle* fn)
{
	return nullptr;
}

void jit_release(JittedFunctionHandle* fn)
{
}
