#include "base/zdefs.h"

#ifndef _ZC_JIT_H_
#define _ZC_JIT_H_

struct JittedScriptHandle;

void jit_printf(const char *format, ...);
bool jit_is_enabled();
void jit_set_enabled(bool enabled);
JittedScriptHandle *jit_create_script_handle(script_data *script, refInfo* ri);
void jit_delete_script_handle(JittedScriptHandle* jitted_script);
void jit_reinit(JittedScriptHandle* jitted_script);
int jit_run_script(JittedScriptHandle *jitted_script);
void jit_startup();
void jit_poll();
void jit_shutdown();

typedef int32_t (*JittedFunction)(int32_t *registers, int32_t *global_registers,
								  int32_t *stack, uint32_t *stack_index, uint32_t *pc,
								  intptr_t *call_stack_rets, uint32_t *call_stack_ret_index,
								  uint32_t *wait_index);
JittedFunction jit_compile_script(script_data* script);
void jit_release(JittedFunction function);

#endif
