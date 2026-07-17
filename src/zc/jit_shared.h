#ifndef ZC_JIT_SHARED_H_
#define ZC_JIT_SHARED_H_

// Shared data structures and driver for the native JIT backends (x64 and a64).
//
// The compilation pipeline, script/instance lifecycle, hot-function profiling,
// and the run loop are architecture independent and live in jit_shared.cpp.
// The per-architecture backends (jit_x64.cpp, jit_a64.cpp) provide only the
// code generation, exposed through jit_backend_compile_function.
//
// This is the runtime layer, included engine-wide via jit.h - keep it to
// plain types and declarations. The codegen policy the two backends share
// while *emitting* code (register cache, emit loop, etc.) lives in
// jit_codegen_shared.h, which only the backend translation units include.

#include "core/zdefs.h"
#include "components/zasm/pc.h"
#include "zc/zasm_utils.h"
#include <cstdint>
#include <deque>
#include <map>
#include <optional>

class ScriptDebugHandle;
struct JittedScriptInstance;

// Result of executing one compiled function, returned to the run loop.
enum JitExecResult
{
	EXEC_RESULT_UNKNOWN = 0,
	EXEC_RESULT_CONTINUE = 1,
	EXEC_RESULT_CALL = 2,
	EXEC_RESULT_RETURN = 3,
	EXEC_RESULT_EXIT = 4,
};

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
	// Used by the x64 backend to save the callee-saved registers it pins; the
	// a64 backend uses virtual registers and leaves this untouched.
	uint64_t saved_regs[4];
	// x64 only: the native address to jump to on entry when resuming after a
	// call or wait. The a64 backend resumes by comparing pc instead.
	uintptr_t resume_address;
};

using JittedFunctionImpl = int (*)(JittedExecutionContext* ctx);

struct JittedFunction
{
	JittedFunctionImpl exec;
	pc_t id;
	std::map<pc_t, uintptr_t> pc_to_resume_address;
};

struct JittedScript
{
	StructuredZasm structured_zasm;
	ZasmCFG cfg;
	ZasmLiveness liveness;
	std::vector<std::vector<pc_t>> block_predecessors;
	std::vector<pc_t> function_start_pcs;
	std::vector<JittedFunction> compiled_functions;
	std::deque<JittedFunction> pending_compiled_jit_functions;
	std::unique_ptr<ScriptDebugHandle> debug_handle;
	ALLEGRO_MUTEX* mutex;
	std::map<pc_t, uintptr_t> pc_to_resume_address;
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
	// Only used by the wasm JIT (RUNGENFRZSCR yield); always -1 for the native
	// backends (they run RUNGENFRZSCR inline via the interpreter, which can
	// block for the nested frames).
	int32_t frozen_dest_reg = -1;
};

// Provided by the per-architecture backend (jit_x64.cpp / jit_a64.cpp): compile
// one function to native code, or std::nullopt if it should stay interpreted.
std::optional<JittedFunction> jit_backend_compile_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn);

#endif
