#include "base/zdefs.h"

#ifndef ZC_JIT_H_
#define ZC_JIT_H_

struct JittedScriptHandle;
typedef void* JittedFunction;

void jit_printf(const char *format, ...);
bool jit_is_enabled();
void jit_set_enabled(bool enabled);
JittedScriptHandle *jit_create_script_handle(script_data *script, refInfo* ri);
void jit_startup();
void jit_poll();
void jit_shutdown();

// The following vary per backend.

// Compile the entire ZASM script at once.
JittedFunction jit_compile_script(zasm_script* script);
JittedScriptHandle *jit_create_script_handle_impl(script_data *script, refInfo* ri, JittedFunction fn);
int jit_run_script(JittedScriptHandle *jitted_script);
void jit_reinit(JittedScriptHandle* jitted_script);
void jit_delete_script_handle(JittedScriptHandle* jitted_script);
void jit_release(JittedFunction fn);

#endif
