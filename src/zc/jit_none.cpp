#include "zc/jit.h"

JittedFunction jit_compile_script(zasm_script* script)
{
	return nullptr;
}

int jit_run_script(JittedScriptHandle *jitted_script)
{
	return RUNSCRIPT_ERROR;
}

JittedScriptHandle *jit_create_script_handle_impl(script_data *script, refInfo* ri, JittedFunction fn)
{
	return nullptr;
}

void jit_reinit(JittedScriptHandle *jitted_script)
{
}

void jit_delete_script_handle(JittedScriptHandle *jitted_script)
{
}

void jit_release(JittedFunction fn)
{
}
