#ifndef ZC_JIT_X64_H_
#define ZC_JIT_X64_H_

#include "base/zdefs.h"
#include "zc/zasm_utils.h"
#include <cstdint>
#include <deque>
#include <map>

class ScriptDebugHandle;
struct JittedScriptInstance;

struct JittedExecutionContext
{
	JittedScriptInstance* j_instance;
	int32_t ret_code;
	int32_t* registers;
	int32_t* global_registers;
	int32_t* stack_base;
	uint32_t sp;
	pc_t pc;
	pc_t call_pc;
	uint64_t saved_regs[4];
};

using JittedFunctionImpl = int (*)(JittedExecutionContext* ctx);
using DispatcherStub = int (*)(uintptr_t target_address, uint32_t stack_size, JittedExecutionContext* ctx);

struct JittedFunction
{
	JittedFunctionImpl exec;
	pc_t id;
	std::map<pc_t, intptr_t> pc_to_address;
	std::map<pc_t, uint32_t> pc_to_stack_size;
};

struct JittedScript
{
	StructuredZasm structured_zasm;
	ZasmCFG cfg;
	std::map<pc_t, pc_t> start_pc_to_fn_id;
	std::vector<JittedFunction> compiled_functions;
	std::deque<JittedFunction> pending_compiled_jit_functions;
	std::unique_ptr<ScriptDebugHandle> debug_handle;
	ALLEGRO_MUTEX* mutex;
	std::map<pc_t, uintptr_t> pc_to_address;
	std::map<pc_t, uint32_t> pc_to_stack_size;
	DispatcherStub dispatcher_stub;
	std::map<pc_t, pc_t> pc_to_containing_function_id_cache;
	// Tracks how many times functions are called in the interpeter.
	std::vector<int> profiler_function_call_count;
	// Tracks how many times functions execute a loop iteration in the interpeter.
	std::vector<int> profiler_function_back_edge_count;
	std::vector<bool> functions_requested_to_be_compiled;
};

struct JittedScriptInstance
{
	JittedScript* j_script;
	script_data* script;
	refInfo* ri;
	bool should_wait;
	// If true, run_script_int is being called to execute exactly [uncompiled_command_count] commands.
	bool sequence_mode;
	int32_t uncompiled_command_count;
};

#endif
