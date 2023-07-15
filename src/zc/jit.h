#include "base/zdefs.h"

#ifndef _ZC_JIT_H_
#define _ZC_JIT_H_

struct JittedScriptHandle;

bool jit_is_enabled();
void jit_set_enabled(bool enabled);
JittedScriptHandle *jit_create_script_handle(script_data *script, refInfo* ri);
void jit_delete_script_handle(JittedScriptHandle* jitted_script);
void jit_reinit(JittedScriptHandle* jitted_script);
int jit_run_script(JittedScriptHandle *jitted_script);
void jit_startup();
void jit_poll();
void jit_shutdown();

#endif
