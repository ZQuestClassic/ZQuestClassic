#ifndef ZC_JIT_H_
#define ZC_JIT_H_

#include "base/compiler.h"
#include "base/worker_pool.h"
#include "base/zdefs.h"
#include "zc/zasm_utils.h"

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
ZC_FORMAT_PRINTF(1, 2)
void jit_error(const char *format, ...);
bool jit_is_enabled();
void jit_set_enabled(bool enabled);
bool jit_log_is_enabled();
bool jit_should_precompile();
JittedScriptInstance* jit_create_script_instance(script_data* script, refInfo* ri, bool just_initialized);
void jit_profiler_increment_function_back_edge(JittedScriptInstance* j_instance, pc_t pc);
void jit_startup(bool precompile);
void jit_shutdown();

// The following vary per backend.

JittedScript* jit_compile_script(zasm_script* script);
JittedScriptInstance* jit_create_script_impl(script_data* script, refInfo* ri, JittedScript* j_script, bool just_initialized);
int jit_run_script(JittedScriptInstance* j_instance);
void jit_release(JittedScript* j_script);

#endif
