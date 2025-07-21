#ifndef ZC_JIT_H_
#define ZC_JIT_H_

#include "base/compiler.h"
#include "base/zdefs.h"

#define JIT_NONE 0
#define JIT_X64 1
#define JIT_WASM 2

#if ZC_JIT_BACKEND == JIT_NONE
	#include "zc/jit_none.h"
#elif ZC_JIT_BACKEND == JIT_X64
	#include "zc/jit_x64.h"
#elif ZC_JIT_BACKEND == JIT_WASM
	#include "zc/jit_wasm.h"
#endif

ZC_FORMAT_PRINTF(1, 2)
void jit_printf(const char *format, ...);
bool jit_is_enabled();
void jit_set_enabled(bool enabled);
JittedScriptHandle* jit_create_script_handle(script_data *script, refInfo* ri);
void jit_startup();
void jit_poll();
void jit_shutdown();

// The following vary per backend.

// Compile the entire ZASM script at once.
JittedFunctionHandle* jit_compile_script(zasm_script* script);
JittedScriptHandle* jit_create_script_handle_impl(script_data *script, refInfo* ri, JittedFunctionHandle* fn);
int jit_run_script(JittedScriptHandle* jitted_script);
void jit_release(JittedFunctionHandle* fn);

#endif
